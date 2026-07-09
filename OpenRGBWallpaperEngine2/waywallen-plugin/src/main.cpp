#include <arpa/inet.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <filesystem>
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
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <fcntl.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <waywallen-bridge/bridge.h>
#include <waywallen-bridge/ipc_v1.h>
#include <waywallen-bridge/pool.h>

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

class EGLRenderContext {
public:
    EGLRenderContext(std::size_t width, std::size_t height) {
        display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (display_ == EGL_NO_DISPLAY) {
            throw std::runtime_error("eglGetDisplay failed");
        }

        EGLint major = 0;
        EGLint minor = 0;
        if (!eglInitialize(display_, &major, &minor)) {
            throw std::runtime_error("eglInitialize failed");
        }

        if (!eglBindAPI(EGL_OPENGL_ES_API)) {
            throw std::runtime_error("eglBindAPI failed");
        }

        const EGLint configAttribs[] = {
            EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_NONE,
        };

        EGLConfig config = nullptr;
        EGLint configCount = 0;
        if (!eglChooseConfig(display_, configAttribs, &config, 1, &configCount) || configCount == 0) {
            throw std::runtime_error("eglChooseConfig failed");
        }

        const EGLint pbufferAttribs[] = {
            EGL_WIDTH, static_cast<EGLint>(width),
            EGL_HEIGHT, static_cast<EGLint>(height),
            EGL_NONE,
        };
        surface_ = eglCreatePbufferSurface(display_, config, pbufferAttribs);
        if (surface_ == EGL_NO_SURFACE) {
            throw std::runtime_error("eglCreatePbufferSurface failed");
        }

        const EGLint contextAttribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE,
        };
        context_ = eglCreateContext(display_, config, EGL_NO_CONTEXT, contextAttribs);
        if (context_ == EGL_NO_CONTEXT) {
            throw std::runtime_error("eglCreateContext failed");
        }

        if (!eglMakeCurrent(display_, surface_, surface_, context_)) {
            throw std::runtime_error("eglMakeCurrent failed");
        }
    }

    ~EGLRenderContext() {
        if (display_ != EGL_NO_DISPLAY) {
            eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            if (context_ != EGL_NO_CONTEXT) {
                eglDestroyContext(display_, context_);
            }
            if (surface_ != EGL_NO_SURFACE) {
                eglDestroySurface(display_, surface_);
            }
            eglTerminate(display_);
        }
    }

    EGLDisplay display() const { return display_; }

private:
    EGLDisplay display_ = EGL_NO_DISPLAY;
    EGLSurface surface_ = EGL_NO_SURFACE;
    EGLContext context_ = EGL_NO_CONTEXT;
};

class WaywallenBridge {
public:
    WaywallenBridge(std::string ipcPath, std::size_t width, std::size_t height)
        : ipcPath_(std::move(ipcPath)), width_(width), height_(height), eglContext_(width, height) {}

    ~WaywallenBridge() {
        stop();
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
        running_ = true;
        eventThread_ = std::thread([this] { eventLoop(); });
    }

    void stop() {
        running_ = false;
        if (eventThread_.joinable()) {
            eventThread_.join();
        }
        if (fd_ >= 0) {
            ::close(fd_);
            fd_ = -1;
        }
        if (pool_ != nullptr) {
            ww_bridge_pool_destroy(pool_);
            pool_ = nullptr;
        }
    }

    void renderFrame(const RGBFrame& frame) {
        if (!running_ || pool_ == nullptr) return;

        const auto slotIndex = nextSlot_++ % std::max<std::uint32_t>(1u, slotCount_);
        ww_pool_slot_t slot{};
        if (ww_bridge_pool_acquire_slot(pool_, slotIndex, &slot) != 0) {
            return;
        }

        renderRgbFrame(frame, slot);
        ww_bridge_pool_submit_slot(pool_, fd_, slotIndex, -1);
        ww_bridge_pool_wait_slot_release(pool_, slotIndex, 50);
    }

private:
    static int openRenderNode() {
        const std::filesystem::path driPath("/dev/dri");
        std::error_code ec;
        if (std::filesystem::exists(driPath, ec)) {
            for (const auto& entry : std::filesystem::directory_iterator(driPath, ec)) {
                const std::string name = entry.path().filename().string();
                if (name.rfind("renderD", 0) == 0) {
                    const int fd = ::open(entry.path().c_str(), O_RDWR | O_CLOEXEC);
                    if (fd >= 0) return fd;
                }
            }
        }
        return ::open("/dev/dri/renderD128", O_RDWR | O_CLOEXEC);
    }

    static std::vector<std::uint8_t> readExact(int socketFd, std::size_t size) {
        std::vector<std::uint8_t> buffer(size);
        std::size_t offset = 0;
        while (offset < size) {
            const ssize_t chunk = ::recv(socketFd, buffer.data() + offset, size - offset, 0);
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
        if (opcode != WW_EVT_IN_INIT) {
            throw std::runtime_error("unexpected control opcode");
        }
    }

    void eventLoop() {
        while (running_) {
            uint16_t opcode = 0;
            uint8_t* body = nullptr;
            size_t bodyLen = 0;
            int fds[8] = {};
            size_t nFds = 0;
            const int rc = ww_bridge_recv_frame(fd_, &opcode, &body, &bodyLen, fds, 8, &nFds);
            if (rc != 0) {
                if (!running_) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            processIncomingMessage(opcode, body, bodyLen, fds, nFds);
            if (body != nullptr) {
                std::free(body);
                body = nullptr;
            }
            for (size_t i = 0; i < nFds; ++i) {
                if (fds[i] >= 0) ::close(fds[i]);
            }
        }
    }

    void processIncomingMessage(uint16_t opcode, const uint8_t* body, size_t bodyLen, const int* fds, size_t nFds) {
        (void)fds;
        (void)nFds;
        switch (opcode) {
            case WW_EVT_IN_INIT: {
                ww_evt_in_init_t init{};
                if (ww_evt_in_init_decode(body, bodyLen, &init) == 0) {
                    ww_evt_in_init_free(&init);
                }
                break;
            }
            case WW_EVT_IN_NEGOTIATE_BUFFERS: {
                ww_evt_in_negotiate_buffers_t directive{};
                if (ww_evt_in_negotiate_buffers_decode(body, bodyLen, &directive) == 0) {
                    handleNegotiateBuffers(directive);
                    ww_evt_in_negotiate_buffers_free(&directive);
                }
                break;
            }
            case WW_EVT_IN_PLAY: {
                ww_evt_in_play_t play{};
                if (ww_evt_in_play_decode(body, bodyLen, &play) == 0) {
                    ww_evt_in_play_free(&play);
                }
                break;
            }
            case WW_EVT_IN_SHUTDOWN: {
                running_ = false;
                break;
            }
            default:
                break;
        }
    }

    void handleNegotiateBuffers(const ww_evt_in_negotiate_buffers_t& directive) {
        if (pool_ != nullptr) {
            ww_bridge_pool_destroy(pool_);
            pool_ = nullptr;
        }

        const int drmFd = openRenderNode();
        if (drmFd < 0) {
            std::cerr << "Failed to open DRM render node: " << std::strerror(errno) << std::endl;
            return;
        }

        ww_pool_egl_gbm_init_t init{};
        init.egl_display = eglContext_.display();
        init.drm_render_fd = drmFd;
        init.get_proc_address = reinterpret_cast<void* (*)(const char*)>(eglGetProcAddress);
        init.drm_render_major = 0;
        init.drm_render_minor = 0;

        if (ww_bridge_pool_create(WW_POOL_BACKEND_EGL_GBM, &init, &pool_) != 0) {
            ::close(drmFd);
            std::cerr << "ww_bridge_pool_create failed" << std::endl;
            return;
        }

        if (ww_bridge_pool_advertise_caps(pool_, fd_, width_, height_, 0) != 0) {
            std::cerr << "ww_bridge_pool_advertise_caps failed" << std::endl;
        }

        slotCount_ = std::max<std::uint32_t>(1u, directive.count);
        std::cout << "waywallen buffer negotiation active with " << slotCount_ << " slots" << std::endl;

        ww_pool_directive_t apply{};
        apply.category = directive.path;
        apply.mem_source = directive.mem_source;
        apply.fourcc = directive.fourcc;
        apply.modifier = directive.modifier;
        apply.plane_count = directive.plane_count;
        apply.sync_mode = directive.sync_mode;
        apply.color = directive.color;
        apply.mem_hint = directive.mem_hint;
        apply.width = static_cast<std::uint32_t>(width_);
        apply.height = static_cast<std::uint32_t>(height_);
        apply.count = slotCount_;
        if (ww_bridge_pool_apply_directive(pool_, fd_, &apply) != 0) {
            std::cerr << "ww_bridge_pool_apply_directive failed" << std::endl;
        }
    }

    void renderRgbFrame(const RGBFrame& frame, const ww_pool_slot_t& slot) {
        if (frame.pixels.empty()) return;

        if (!shaderProgram_) {
            shaderProgram_ = compileShaderProgram();
            if (!shaderProgram_) {
                return;
            }
            positionLocation_ = glGetAttribLocation(shaderProgram_, "aPosition");
            textureLocation_ = glGetUniformLocation(shaderProgram_, "uTexture");
        }
        if (!shaderProgram_) {
            return;
        }

        if (texture_ == 0) {
            glGenTextures(1, &texture_);
            glBindTexture(GL_TEXTURE_2D, texture_);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }

        glBindTexture(GL_TEXTURE_2D, texture_);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        std::vector<std::uint8_t> rgba(frame.pixels.size() / 3 * 4);
        for (std::size_t i = 0; i < frame.pixels.size() / 3; ++i) {
            const std::size_t inputOffset = i * 3;
            const std::size_t outputOffset = i * 4;
            rgba[outputOffset + 0] = frame.pixels[inputOffset + 0];
            rgba[outputOffset + 1] = frame.pixels[inputOffset + 1];
            rgba[outputOffset + 2] = frame.pixels[inputOffset + 2];
            rgba[outputOffset + 3] = 255;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(frame.width), static_cast<GLsizei>(frame.height), 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());

        glBindFramebuffer(GL_FRAMEBUFFER, slot.gl_export_fbo);
        glViewport(0, 0, static_cast<GLsizei>(slot.width), static_cast<GLsizei>(slot.height));
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram_);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_);
        glUniform1i(textureLocation_, 0);

        const GLfloat vertices[] = {
            -1.0f, -1.0f,
             1.0f, -1.0f,
            -1.0f,  1.0f,
             1.0f,  1.0f,
        };
        const GLfloat texCoords[] = {
            0.0f, 0.0f,
            1.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 1.0f,
        };

        glEnableVertexAttribArray(positionLocation_);
        glVertexAttribPointer(positionLocation_, 2, GL_FLOAT, GL_FALSE, 0, vertices);
        glEnableVertexAttribArray(texCoordLocation_);
        glVertexAttribPointer(texCoordLocation_, 2, GL_FLOAT, GL_FALSE, 0, texCoords);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glDisableVertexAttribArray(positionLocation_);
        glDisableVertexAttribArray(texCoordLocation_);
        glFlush();
    }

    static GLuint compileShaderProgram() {
        constexpr char kVertexShader[] = R"(
attribute vec2 aPosition;
varying vec2 vTexCoord;
void main() {
    gl_Position = vec4(aPosition, 0.0, 1.0);
    vTexCoord = vec2((aPosition.x + 1.0) * 0.5, (1.0 - aPosition.y) * 0.5);
}
)";

        constexpr char kFragmentShader[] = R"(
precision mediump float;
uniform sampler2D uTexture;
varying vec2 vTexCoord;
void main() {
    gl_FragColor = texture2D(uTexture, vTexCoord);
}
)";

        const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        const GLchar* vertexSource = reinterpret_cast<const GLchar*>(kVertexShader);
        glShaderSource(vertexShader, 1, &vertexSource, nullptr);
        glCompileShader(vertexShader);

        GLint compiled = 0;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLen);
            std::vector<GLchar> infoLog(infoLen);
            glGetShaderInfoLog(vertexShader, infoLen, nullptr, infoLog.data());
            std::cerr << "vertex shader failed: " << infoLog.data() << std::endl;
            glDeleteShader(vertexShader);
            return 0;
        }

        const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        const GLchar* fragmentSource = reinterpret_cast<const GLchar*>(kFragmentShader);
        glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
        glCompileShader(fragmentShader);
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &infoLen);
            std::vector<GLchar> infoLog(infoLen);
            glGetShaderInfoLog(fragmentShader, infoLen, nullptr, infoLog.data());
            std::cerr << "fragment shader failed: " << infoLog.data() << std::endl;
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            return 0;
        }

        const GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glBindAttribLocation(program, 0, "aPosition");
        glLinkProgram(program);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        GLint linked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &linked);
        if (!linked) {
            GLint infoLen = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
            std::vector<GLchar> infoLog(infoLen);
            glGetProgramInfoLog(program, infoLen, nullptr, infoLog.data());
            std::cerr << "program link failed: " << infoLog.data() << std::endl;
            glDeleteProgram(program);
            return 0;
        }

        return program;
    }

    std::string ipcPath_;
    std::size_t width_ = 0;
    std::size_t height_ = 0;
    int fd_ = -1;
    bool running_ = false;
    std::thread eventThread_;
    EGLRenderContext eglContext_{1, 1};
    ww_pool_t* pool_ = nullptr;
    std::uint32_t slotCount_ = 1;
    std::uint32_t nextSlot_ = 0;
    GLuint shaderProgram_ = 0;
    GLint positionLocation_ = 0;
    GLint texCoordLocation_ = 1;
    GLint textureLocation_ = -1;
    GLuint texture_ = 0;
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
            bridge.emplace(settings.ipcPath, settings.width, settings.height);
            bridge->connectAndHandshake();
        }

        while (true) {
            if (bridge.has_value()) {
                if (const auto frame = receiver.popFrame()) {
                    bridge->renderFrame(*frame);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
}
