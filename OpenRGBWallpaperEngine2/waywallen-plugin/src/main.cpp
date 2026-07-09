#include <arpa/inet.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <optional>
#include <queue>
#include <stdexcept>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

namespace {

struct RGBFrame {
    std::vector<std::uint8_t> pixels;
    std::size_t width = 0;
    std::size_t height = 0;
};

struct Settings {
    std::uint16_t udpPort = 8133;
    std::size_t width = 32;
    std::size_t height = 32;
    std::string backgroundImage;
    std::string ipcPath;
    std::array<float, 4> backgroundColor{1.0f, 1.0f, 1.0f, 1.0f};
};

class OpenRGBReceiver {
public:
    explicit OpenRGBReceiver(std::uint16_t port) : port_(port) {}

    void start() {
        socket_ = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (socket_ < 0) throw std::runtime_error("socket failed");

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(port_);

        if (::bind(socket_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            const int err = errno;
            ::close(socket_);
            socket_ = -1;
            std::cerr << "OpenRGB UDP bind failed on port " << port_ << ": "
                      << std::strerror(err) << ". Continuing without listener."
                      << std::endl;
            return;
        }

        running_ = true;
        receiverThread_ = std::thread([this] { receiveLoop(); });
    }

    void stop() {
        running_ = false;
        if (socket_ >= 0) {
            ::shutdown(socket_, SHUT_RDWR);
            ::close(socket_);
            socket_ = -1;
        }
        if (receiverThread_.joinable()) receiverThread_.join();
    }

    void setSettings(const Settings& settings) {
        std::lock_guard<std::mutex> lock(mutex_);
        settings_ = settings;
    }

    std::optional<RGBFrame> popFrame() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (frames_.empty()) return std::nullopt;
        RGBFrame frame = std::move(frames_.front());
        frames_.pop();
        return frame;
    }

private:
    void receiveLoop() {
        std::array<unsigned char, 65535> buffer{};
        while (running_) {
            if (socket_ < 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }
            sockaddr_in sender{};
            socklen_t senderSize = sizeof(sender);
            ssize_t len = ::recvfrom(socket_, buffer.data(), buffer.size(), 0,
                                     reinterpret_cast<sockaddr*>(&sender), &senderSize);
            if (len <= 0) continue;
            processPacket(buffer.data(), static_cast<std::size_t>(len));
        }
    }

    void processPacket(const unsigned char* data, std::size_t len) {
        if (len < 1) return;

        if (data[0] == 0) {
            static std::vector<unsigned char> combined;
            if (len >= 3) {
                const std::size_t payloadOffset = 3;
                combined.insert(combined.end(), data + payloadOffset, data + len);
                if (data[1] == data[2] - 1) {
                    parseRgbPacket(combined.data(), combined.size());
                    combined.clear();
                }
            }
            return;
        }

        if (data[0] == 1 && len >= 19) {
            Settings nextSettings;
            nextSettings.width = 32;
            nextSettings.height = 32;
            nextSettings.udpPort = port_;
            nextSettings.backgroundColor = {1.0f, 1.0f, 1.0f, 1.0f};
            nextSettings.backgroundImage = std::string(reinterpret_cast<const char*>(data + 19), len - 19);
            nextSettings.backgroundImage.erase(nextSettings.backgroundImage.find_last_not_of('\0') + 1);
            setSettings(nextSettings);
        }
    }

    void parseRgbPacket(const unsigned char* data, std::size_t len) {
        if (len < 3) return;
        std::lock_guard<std::mutex> lock(mutex_);
        std::size_t pixelCount = len / 3;
        RGBFrame frame;
        frame.pixels.resize(pixelCount * 3);
        std::memcpy(frame.pixels.data(), data, frame.pixels.size());
        frame.width = settings_.width;
        frame.height = settings_.height;
        frames_.push(std::move(frame));
    }

    std::uint16_t port_;
    int socket_ = -1;
    bool running_ = false;
    std::thread receiverThread_;
    std::mutex mutex_;
    std::queue<RGBFrame> frames_;
    Settings settings_{};
};

class WaywallenBridge {
public:
    explicit WaywallenBridge(std::string ipcPath) : ipcPath_(std::move(ipcPath)) {}

    ~WaywallenBridge() {
        running_ = false;
        if (frameThread_.joinable()) frameThread_.join();
        if (fd_ >= 0) {
            ::close(fd_);
            fd_ = -1;
        }
    }

    void connectAndHandshake() {
        if (ipcPath_.empty()) return;
        fd_ = ::socket(AF_UNIX, SOCK_STREAM, 0);
        if (fd_ < 0) throw std::runtime_error("socketpair failed");

        sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        std::strncpy(addr.sun_path, ipcPath_.c_str(), sizeof(addr.sun_path) - 1);
        if (::connect(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            ::close(fd_);
            fd_ = -1;
            throw std::runtime_error("connect failed");
        }

        readInitMessage();
        sendReady();
        running_ = true;
        frameThread_ = std::thread([this] { frameLoop(); });
    }

private:
    static void sendAll(int fd, const void* data, std::size_t size) {
        const auto* ptr = static_cast<const char*>(data);
        std::size_t sent = 0;
        while (sent < size) {
            const ssize_t chunk = ::send(fd, ptr + sent, size - sent, MSG_NOSIGNAL);
            if (chunk <= 0) throw std::runtime_error("send failed");
            sent += static_cast<std::size_t>(chunk);
        }
    }

    static void sendEvent(int fd, std::uint16_t opcode, const std::vector<std::uint8_t>& body, int fdToPass = -1) {
        std::vector<std::uint8_t> frame(4 + body.size());
        frame[0] = static_cast<std::uint8_t>(opcode & 0xff);
        frame[1] = static_cast<std::uint8_t>((opcode >> 8) & 0xff);
        const std::uint16_t total = static_cast<std::uint16_t>(4 + body.size());
        frame[2] = static_cast<std::uint8_t>(total & 0xff);
        frame[3] = static_cast<std::uint8_t>((total >> 8) & 0xff);
        std::copy(body.begin(), body.end(), frame.begin() + 4);

        if (fdToPass >= 0) {
            msghdr msg{};
            iovec iov{};
            iov.iov_base = frame.data();
            iov.iov_len = frame.size();
            msg.msg_iov = &iov;
            msg.msg_iovlen = 1;
            std::array<unsigned char, CMSG_SPACE(sizeof(int))> control{};
            msg.msg_control = control.data();
            msg.msg_controllen = CMSG_SPACE(sizeof(int));

            cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
            cmsg->cmsg_level = SOL_SOCKET;
            cmsg->cmsg_type = SCM_RIGHTS;
            cmsg->cmsg_len = CMSG_LEN(sizeof(int));
            *reinterpret_cast<int*>(CMSG_DATA(cmsg)) = fdToPass;

            const ssize_t sent = ::sendmsg(fd, &msg, MSG_NOSIGNAL);
            if (sent < 0) throw std::runtime_error("sendmsg failed");
            return;
        }

        sendAll(fd, frame.data(), frame.size());
    }

    static std::vector<std::uint8_t> readExact(int fd, std::size_t size) {
        std::vector<std::uint8_t> buffer(size);
        std::size_t offset = 0;
        while (offset < size) {
            const ssize_t chunk = ::recv(fd, buffer.data() + offset, size - offset, 0);
            if (chunk <= 0) throw std::runtime_error("recv failed");
            offset += static_cast<std::size_t>(chunk);
        }
        return buffer;
    }

    void readInitMessage() {
        const auto header = readExact(fd_, 4);
        const std::uint16_t opcode = static_cast<std::uint16_t>(header[0] | (header[1] << 8));
        const std::uint16_t total = static_cast<std::uint16_t>(header[2] | (header[3] << 8));
        if (total < 4) throw std::runtime_error("invalid init frame");
        const std::size_t bodyLen = total - 4;
        if (bodyLen > 0) {
            readExact(fd_, bodyLen);
        }
        if (opcode != 1) {
            throw std::runtime_error("unexpected control opcode");
        }
    }

    void sendReady() {
        std::vector<std::uint8_t> body(8, 0);
        sendEvent(fd_, 1, body);
    }

    void frameLoop() {
        std::uint64_t seq = 0;
        while (running_) {
            std::vector<std::uint8_t> body(28, 0);
            std::uint32_t imageIndex = 0;
            std::uint64_t tsNs = static_cast<std::uint64_t>(
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count());
            std::memcpy(body.data(), &imageIndex, sizeof(imageIndex));
            std::memcpy(body.data() + 4, &seq, sizeof(seq));
            std::memcpy(body.data() + 12, &tsNs, sizeof(tsNs));
            std::uint64_t releasePoint = 0;
            std::memcpy(body.data() + 20, &releasePoint, sizeof(releasePoint));
            const int syncFd = ::eventfd(0, 0);
            if (syncFd < 0) {
                std::cerr << "eventfd failed" << std::endl;
                break;
            }
            try {
                sendEvent(fd_, 3, body, syncFd);
            } catch (const std::exception& ex) {
                std::cerr << "frame emit failed: " << ex.what() << std::endl;
                ::close(syncFd);
                break;
            }
            ::close(syncFd);
            ++seq;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    std::string ipcPath_;
    int fd_ = -1;
    bool running_ = false;
    std::thread frameThread_;
    std::uint64_t frameSeq_ = 0;
};

} // namespace

static Settings parseArgs(int argc, char** argv) {
    Settings settings;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--udp-port" && i + 1 < argc) {
            settings.udpPort = static_cast<std::uint16_t>(std::stoi(argv[++i]));
        } else if (arg == "--background-image" && i + 1 < argc) {
            settings.backgroundImage = argv[++i];
        } else if (arg == "--background-color" && i + 1 < argc) {
            settings.backgroundColor = {1.0f, 1.0f, 1.0f, 1.0f};
        } else if (arg == "--width" && i + 1 < argc) {
            settings.width = static_cast<std::size_t>(std::stoul(argv[++i]));
        } else if (arg == "--height" && i + 1 < argc) {
            settings.height = static_cast<std::size_t>(std::stoul(argv[++i]));
        } else if (arg == "--ipc" && i + 1 < argc) {
            settings.ipcPath = argv[++i];
        }
    }
    const char* envPort = std::getenv("OPENRGB_UDP_PORT");
    if (envPort && *envPort) settings.udpPort = static_cast<std::uint16_t>(std::stoi(envPort));
    const char* envImage = std::getenv("OPENRGB_BACKGROUND_IMAGE");
    if (envImage && *envImage) settings.backgroundImage = envImage;
    return settings;
}

int main(int argc, char** argv) {
    try {
        Settings settings = parseArgs(argc, argv);
        std::cout << "OpenRGB waywallen plugin listening on UDP " << settings.udpPort << std::endl;

        OpenRGBReceiver receiver(settings.udpPort);
        receiver.start();

        std::optional<WaywallenBridge> bridge;
        if (!settings.ipcPath.empty()) {
            bridge.emplace(settings.ipcPath);
            bridge->connectAndHandshake();
        }

        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
}
