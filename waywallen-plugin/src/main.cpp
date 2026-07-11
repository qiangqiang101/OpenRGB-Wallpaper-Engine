#include <arpa/inet.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <condition_variable>
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
#include <unordered_map>
#include <unistd.h>
#include <vector>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <fcntl.h>

#include <waywallen-bridge/bridge.h>
#include <waywallen-bridge/ipc_v1.h>
#include <waywallen-bridge/pool.h>
#include <waywallen-bridge/probe_vk.h>

#include <vulkan/vulkan.h>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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
    explicit OpenRGBReceiver(std::uint16_t port)
        : port_(port)
    {
    }

    bool waitForFrame(std::chrono::milliseconds timeout)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        return frameCv_.wait_for(lock, timeout, [this] { return !frames_.empty(); });
    }

    void start()
    {
        socket_ = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if(socket_ < 0) throw std::runtime_error("socket failed");

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(port_);

        if(::bind(socket_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
        {
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

    void stop()
    {
        running_ = false;
        if(socket_ >= 0)
        {
            ::shutdown(socket_, SHUT_RDWR);
            ::close(socket_);
            socket_ = -1;
        }
        if(receiverThread_.joinable()) receiverThread_.join();
    }

    void setSettings(const Settings& settings)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        settings_ = settings;
    }

    Settings getSettings() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return settings_;
    }

    std::optional<RGBFrame> popFrame()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if(frames_.empty()) return std::nullopt;
        RGBFrame frame = std::move(frames_.front());
        frames_.pop();
        return frame;
    }
    
    std::optional<RGBFrame> popLatestFrame()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if(frames_.empty()) return std::nullopt;
        RGBFrame frame = std::move(frames_.back());
        while(!frames_.empty()) frames_.pop();
        return frame;
    }

    bool hasSettingsPacket() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return settingsReceived_;
    }

private:
    void receiveLoop()
    {
        std::array<unsigned char, 65535> buffer{};
        while(running_)
        {
            if(socket_ < 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }
            sockaddr_in sender{};
            socklen_t senderSize = sizeof(sender);
            ssize_t len = ::recvfrom(socket_, buffer.data(), buffer.size(), 0,
                                     reinterpret_cast<sockaddr*>(&sender), &senderSize);
            if(len <= 0) continue;
            processPacket(buffer.data(), static_cast<std::size_t>(len));
        }
    }

    static std::pair<std::size_t, std::size_t> matrixSizeFromType(int type, int tier)
    {
        std::pair<std::size_t, std::size_t> (*landscape)(int, int, int) = [](int w, int h, int tier) -> std::pair<std::size_t, std::size_t>
        {
            switch(tier)
            {
                case 0: return {w / 4, h / 4};
                case 1: return {w / 2, h / 2};
                case 2: return {w, h};
                default: return {w * 2, h * 2};
            }
        };
        std::pair<std::size_t, std::size_t> (*portrait)(int, int, int) = [](int w, int h, int tier) -> std::pair<std::size_t, std::size_t>
        {
            switch(tier)
            {
                case 0: return {h / 4, w / 4};
                case 1: return {h / 2, w / 2};
                case 2: return {h, w};
                default: return {h * 2, w * 2};
            }
        };
        switch(type)
        {
            case 0: return landscape(8, 2, tier);
            case 1: return portrait(8, 2, tier);
            case 2: return landscape(8, 6, tier);
            case 3: return portrait(8, 6, tier);
            case 4: return landscape(48, 27, 2);
            case 5: return portrait(10, 8, tier);
            case 6: return landscape(32, 18, tier);
            case 7: return portrait(32, 18, tier);
            case 8: return landscape(32, 20, tier);
            case 9: return portrait(32, 20, tier);
            case 10: return landscape(42, 18, tier);
            case 11: return portrait(42, 18, tier);
            case 12: return landscape(64, 18, tier);
            case 13: return portrait(64, 18, tier);
            default: return {32, 32};
        }
    }

    void processPacket(const unsigned char* data, std::size_t len)
    {
        if(len < 1) return;

        if(data[0] == 0)
        {
            static std::vector<unsigned char> combined;
            if(len >= 3)
            {
                const std::size_t payloadOffset = 3;
                combined.insert(combined.end(), data + payloadOffset, data + len);
                if(data[1] == data[2] - 1)
                {
                    parseRgbPacket(combined.data(), combined.size());
                    combined.clear();
                }
            }
            return;
        }

        if(data[0] == 1 && len >= 19)
        {
            const int matrixType = data[1];
            const int matrixTier = data[2];
            std::pair<std::size_t, std::size_t> sizePair = matrixSizeFromType(matrixType, matrixTier);
            std::size_t w = sizePair.first;
            std::size_t h = sizePair.second;

            Settings nextSettings;
            nextSettings.width = w;
            nextSettings.height = h;
            nextSettings.udpPort = port_;
            nextSettings.backgroundColor = {1.0f, 1.0f, 1.0f, 1.0f};
            nextSettings.backgroundImage = std::string(reinterpret_cast<const char*>(data + 19), len - 19);
            nextSettings.backgroundImage.erase(nextSettings.backgroundImage.find_last_not_of('\0') + 1);
            setSettings(nextSettings);
            settingsReceived_ = true;
            std::cerr << "[openrgb] settings: matrixType=" << matrixType
                      << " tier=" << matrixTier << " -> " << w << "x" << h << std::endl;
        }
    }

    void parseRgbPacket(const unsigned char* data, std::size_t len)
    {        
        if(len < 3) return;

        std::lock_guard<std::mutex> lock(mutex_);
        std::size_t pixelCount = len / 3;
        RGBFrame frame;
        frame.pixels.resize(pixelCount * 3);
        std::memcpy(frame.pixels.data(), data, frame.pixels.size());
        frame.width = settings_.width;
        frame.height = settings_.height;
        frames_.push(std::move(frame));
        frameCv_.notify_one();
    }

    std::uint16_t port_;
    int socket_ = -1;
    bool running_ = false;
    std::thread receiverThread_;
    mutable std::mutex mutex_;
    std::condition_variable frameCv_;
    std::queue<RGBFrame> frames_;
    Settings settings_{};
    bool settingsReceived_ = false;
};

/* Minimal Vulkan context for uploading RGB frames to DMA-BUF slots. */
class VulkanContext
{
public:
    VulkanContext(std::size_t width, std::size_t height)
        : width_(static_cast<std::uint32_t>(width)),
          height_(static_cast<std::uint32_t>(height))
    {
        createInstance();
        pickPhysicalDevice();
        createDevice();
        createStagingBuffer();
        createCommandPool();
        createCommandBuffer();
    }

    void ensureStagingBuffer(std::uint32_t requiredWidth, std::uint32_t requiredHeight)
    {
        const VkDeviceSize requiredSize = static_cast<VkDeviceSize>(requiredWidth) * requiredHeight * 4;
        if(requiredSize > stagingSize_)
        {
            if(stagingBuffer_ != VK_NULL_HANDLE)
            {
                vkDestroyBuffer(device_, stagingBuffer_, nullptr);
                stagingBuffer_ = VK_NULL_HANDLE;
            }
            if(stagingMemory_ != VK_NULL_HANDLE)
            {
                vkFreeMemory(device_, stagingMemory_, nullptr);
                stagingMemory_ = VK_NULL_HANDLE;
            }
            width_ = requiredWidth;
            height_ = requiredHeight;
            stagingSize_ = requiredSize;
            createStagingBuffer();
        }
    }

    ~VulkanContext()
    {
        if(device_ != VK_NULL_HANDLE)
        {
            if(cmdBuffer_ != VK_NULL_HANDLE)
            {
                vkFreeCommandBuffers(device_, cmdPool_, 1, &cmdBuffer_);
            }
            if(cmdPool_ != VK_NULL_HANDLE)
            {
                vkDestroyCommandPool(device_, cmdPool_, nullptr);
            }
            if(stagingBuffer_ != VK_NULL_HANDLE)
            {
                vkDestroyBuffer(device_, stagingBuffer_, nullptr);
            }
            if(stagingMemory_ != VK_NULL_HANDLE)
            {
                vkFreeMemory(device_, stagingMemory_, nullptr);
            }
            vkDestroyDevice(device_, nullptr);
        }
        if(instance_ != VK_NULL_HANDLE)
        {
            vkDestroyInstance(instance_, nullptr);
        }
    }

    VkInstance instance() const { return instance_; }
    VkPhysicalDevice physicalDevice() const { return physDevice_; }
    VkDevice device() const { return device_; }
    VkQueue queue() const { return queue_; }
    std::uint32_t queueFamilyIndex() const { return queueFamily_; }
    int drmRenderFd() const { return drmFd_; }
    std::uint32_t drmRenderMajor() const { return drmMajor_; }
    std::uint32_t drmRenderMinor() const { return drmMinor_; }
    const std::uint8_t* deviceUUID() const { return deviceUUID_; }
    const std::uint8_t* driverUUID() const { return driverUUID_; }

    bool uploadToImage(VkImage image, std::uint32_t imgW, std::uint32_t imgH,
                       const std::uint8_t* rgbaData, std::size_t rgbaSize,
                       int* outSyncFd)
    {
        *outSyncFd = -1;

        void* mapped = nullptr;
        if(vkMapMemory(device_, stagingMemory_, 0, VK_WHOLE_SIZE, 0, &mapped) != VK_SUCCESS)
        {
            return false;
        }
        std::memcpy(mapped, rgbaData, std::min(rgbaSize, stagingSize_));
        vkUnmapMemory(device_, stagingMemory_);

        VkSemaphore sem = VK_NULL_HANDLE;
        VkExportSemaphoreCreateInfo exportInfo{};
        exportInfo.sType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO;
        exportInfo.handleTypes = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT;

        VkSemaphoreCreateInfo semInfo{};
        semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semInfo.pNext = &exportInfo;

        if(vkCreateSemaphore(device_, &semInfo, nullptr, &sem) != VK_SUCCESS)
        {
            return true;
        }

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        if(vkBeginCommandBuffer(cmdBuffer_, &beginInfo) != VK_SUCCESS)
        {
            vkDestroySemaphore(device_, sem, nullptr);
            return false;
        }

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        vkCmdPipelineBarrier(cmdBuffer_,
                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0, 0, nullptr, 0, nullptr, 1, &barrier);

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.layerCount = 1;
        region.imageExtent.width = imgW;
        region.imageExtent.height = imgH;
        region.imageExtent.depth = 1;

        vkCmdCopyBufferToImage(cmdBuffer_, stagingBuffer_, image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(cmdBuffer_,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             0, 0, nullptr, 0, nullptr, 1, &barrier);

        if(vkEndCommandBuffer(cmdBuffer_) != VK_SUCCESS)
        {
            vkDestroySemaphore(device_, sem, nullptr);
            return false;
        }

        VkTimelineSemaphoreSubmitInfo timelineInfo{};
        timelineInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
        timelineInfo.signalSemaphoreValueCount = 1;
        uint64_t signalValue = 1;
        timelineInfo.pSignalSemaphoreValues = &signalValue;

        VkSemaphoreSubmitInfo signalInfo{};
        signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        signalInfo.semaphore = sem;
        signalInfo.stageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        signalInfo.value = 1;

        VkSubmitInfo2 submit2{};
        submit2.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submit2.pNext = &timelineInfo;
        submit2.commandBufferInfoCount = 1;
        
        VkCommandBufferSubmitInfo cmdInfo{};
        cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        cmdInfo.commandBuffer = cmdBuffer_;
        submit2.pCommandBufferInfos = &cmdInfo;
        
        submit2.signalSemaphoreInfoCount = 1;
        submit2.pSignalSemaphoreInfos = &signalInfo;

        if(vkQueueSubmit2(queue_, 1, &submit2, VK_NULL_HANDLE) != VK_SUCCESS)
        {
            vkDestroySemaphore(device_, sem, nullptr);
            return false;
        }

        VkSemaphoreGetFdInfoKHR fdInfo{};
        fdInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR;
        fdInfo.semaphore = sem;
        fdInfo.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT;
        if(vkGetSemaphoreFdKHR(device_, &fdInfo, outSyncFd) != VK_SUCCESS)
        {
            *outSyncFd = -1;
        }

        vkDestroySemaphore(device_, sem, nullptr);
        return true;
    }

private:
    void createInstance()
    {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "OpenRGB waywallen";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        const std::vector<const char*> extensions = {
            VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,
            VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME,
            VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
        };

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        if(vkCreateInstance(&createInfo, nullptr, &instance_) != VK_SUCCESS)
        {
            throw std::runtime_error("vkCreateInstance failed");
        }
    }

    void pickPhysicalDevice()
    {
        std::uint32_t count = 0;
        vkEnumeratePhysicalDevices(instance_, &count, nullptr);
        if(count == 0) throw std::runtime_error("no Vulkan devices");

        std::vector<VkPhysicalDevice> devices(count);
        vkEnumeratePhysicalDevices(instance_, &count, devices.data());
        physDevice_ = devices[0];

        VkPhysicalDeviceIDProperties idProps{};
        idProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES;

        VkPhysicalDeviceProperties2 props2{};
        props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        props2.pNext = &idProps;
        vkGetPhysicalDeviceProperties2(physDevice_, &props2);

        std::memcpy(deviceUUID_, idProps.deviceUUID, VK_UUID_SIZE);
        std::memcpy(driverUUID_, idProps.driverUUID, VK_UUID_SIZE);

        VkPhysicalDeviceDrmPropertiesEXT drmProps{};
        drmProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRM_PROPERTIES_EXT;

        VkPhysicalDeviceProperties2 drmProps2{};
        drmProps2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        drmProps2.pNext = &drmProps;
        vkGetPhysicalDeviceProperties2(physDevice_, &drmProps2);

        if(drmProps.hasRender)
        {
            drmMajor_ = drmProps.renderMajor;
            drmMinor_ = drmProps.renderMinor;
        }

        std::uint32_t qCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physDevice_, &qCount, nullptr);
        std::vector<VkQueueFamilyProperties> queues(qCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physDevice_, &qCount, queues.data());

        for(std::uint32_t i = 0; i < qCount; ++i)
        {
            if(queues[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                queueFamily_ = i;
                break;
            }
        }
    }

    void createDevice()
    {
        const float queuePriority = 1.0f;

        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = queueFamily_;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &queuePriority;

        const std::vector<const char*> extensions = {
            VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
            VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME,
            VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME,
            VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME,
            VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
            VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
            VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME,
            VK_KHR_MAINTENANCE1_EXTENSION_NAME,
            VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME,
            VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME,
            VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME,
            VK_EXT_EXTERNAL_MEMORY_DMA_BUF_EXTENSION_NAME,
            VK_EXT_PHYSICAL_DEVICE_DRM_EXTENSION_NAME,
        };

        VkPhysicalDeviceFeatures features{};

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pQueueCreateInfos = &queueInfo;
        createInfo.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        createInfo.pEnabledFeatures = &features;

        if(vkCreateDevice(physDevice_, &createInfo, nullptr, &device_) != VK_SUCCESS)
        {
            throw std::runtime_error("vkCreateDevice failed");
        }

        vkGetDeviceQueue(device_, queueFamily_, 0, &queue_);

        vkGetSemaphoreFdKHR = reinterpret_cast<PFN_vkGetSemaphoreFdKHR>(vkGetDeviceProcAddr(device_, "vkGetSemaphoreFdKHR"));
        vkQueueSubmit2 = reinterpret_cast<PFN_vkQueueSubmit2>(vkGetDeviceProcAddr(device_, "vkQueueSubmit2"));
    }

    void createStagingBuffer()
    {
        stagingSize_ = width_ * height_ * 4;

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = stagingSize_;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if(vkCreateBuffer(device_, &bufferInfo, nullptr, &stagingBuffer_) != VK_SUCCESS)
        {
            throw std::runtime_error("vkCreateBuffer failed");
        }

        VkMemoryRequirements memReqs;
        vkGetBufferMemoryRequirements(device_, stagingBuffer_, &memReqs);

        VkPhysicalDeviceMemoryProperties memProps;
        vkGetPhysicalDeviceMemoryProperties(physDevice_, &memProps);

        std::uint32_t memType = VK_MAX_MEMORY_TYPES;
        for(std::uint32_t i = 0; i < memProps.memoryTypeCount; ++i)
        {
            if((memReqs.memoryTypeBits & (1 << i)) &&
                (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) &&
                (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
            {
                memType = i;
                break;
            }
        }

        if(memType == VK_MAX_MEMORY_TYPES)
        {
            throw std::runtime_error("no suitable memory type for staging buffer");
        }

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReqs.size;
        allocInfo.memoryTypeIndex = memType;

        if(vkAllocateMemory(device_, &allocInfo, nullptr, &stagingMemory_) != VK_SUCCESS)
        {
            throw std::runtime_error("vkAllocateMemory failed");
        }

        vkBindBufferMemory(device_, stagingBuffer_, stagingMemory_, 0);
    }

    void createCommandPool()
    {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamily_;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if(vkCreateCommandPool(device_, &poolInfo, nullptr, &cmdPool_) != VK_SUCCESS)
        {
            throw std::runtime_error("vkCreateCommandPool failed");
        }
    }

    void createCommandBuffer()
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = cmdPool_;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        if(vkAllocateCommandBuffers(device_, &allocInfo, &cmdBuffer_) != VK_SUCCESS)
        {
            throw std::runtime_error("vkAllocateCommandBuffers failed");
        }
    }

    std::uint32_t           width_              = 0;
    std::uint32_t           height_             = 0;
    VkInstance              instance_           = VK_NULL_HANDLE;
    VkPhysicalDevice        physDevice_         = VK_NULL_HANDLE;
    VkDevice                device_             = VK_NULL_HANDLE;
    VkQueue                 queue_              = VK_NULL_HANDLE;
    std::uint32_t           queueFamily_        = 0;
    int                     drmFd_              = -1;
    std::uint32_t           drmMajor_           = 0;
    std::uint32_t           drmMinor_           = 0;
    std::uint8_t            deviceUUID_[VK_UUID_SIZE]{};
    std::uint8_t            driverUUID_[VK_UUID_SIZE]{};
    VkBuffer                stagingBuffer_      = VK_NULL_HANDLE;
    VkDeviceMemory          stagingMemory_      = VK_NULL_HANDLE;
    VkDeviceSize            stagingSize_        = 0;
    VkCommandPool           cmdPool_            = VK_NULL_HANDLE;
    VkCommandBuffer         cmdBuffer_          = VK_NULL_HANDLE;

    PFN_vkGetSemaphoreFdKHR vkGetSemaphoreFdKHR = nullptr;
    PFN_vkQueueSubmit2      vkQueueSubmit2      = nullptr;
};

/* Precomputed circle mask for fast rendering. */
class CircleMaskCache
{
public:
    struct Mask
    {
        std::vector<int> xOffsets;
        std::vector<int> widths;
        int height = 0;
        int radius = 0;
    };

    const Mask& getMask(int radius)
    {
        auto it = cache_.find(radius);
        if(it != cache_.end())
        {
            return it->second;
        }

        Mask mask;
        mask.radius = radius;
        mask.height = radius * 2 + 1;
        
        const int radiusSq = radius * radius;
        
        for(int dy = -radius; dy <= radius; ++dy)
        {
            const int dySq = dy * dy;
            const int remaining = radiusSq - dySq;
            
            if(remaining >= 0)
            {
                const int dx = fastSqrt(remaining);
                mask.xOffsets.push_back(-dx);
                mask.widths.push_back(dx * 2 + 1);
            }
            else
            {
                mask.xOffsets.push_back(0);
                mask.widths.push_back(0);
            }
        }

        cache_[radius] = std::move(mask);
        return cache_[radius];
    }

private:
    static int fastSqrt(int n)
    {
        if(n <= 0) return 0;
        if(n < 2) return 1;
        
        int low = 0, high = n;
        while(low < high)
        {
            const int mid = (low + high + 1) / 2;
            if(mid <= n / mid)
            {
                low = mid;
            }
            else
            {
                high = mid - 1;
            }
        }
        return low;
    }

    std::unordered_map<int, Mask> cache_;
};

/* Load an image from file or HTTP URL. */
static std::pair<std::vector<std::uint8_t>, std::pair<int, int>> loadBackgroundImage(const std::string& path)
{
    std::pair<std::vector<std::uint8_t>, std::pair<int, int>> result;
    
    if(path.empty())
    {
        return result;
    }

    int width = 0, height = 0, channels = 0;
    unsigned char* data = nullptr;
    
    if(path.find("http://") == 0 || path.find("https://") == 0)
    {
        std::string tmpPath = "/tmp/openrgb_bg_" + std::to_string(getpid()) + ".tmp";
        std::string cmd = "curl -sL --max-time 10 --connect-timeout 5 \"" + path + "\" -o \"" + tmpPath + "\"";
        int ret = system(cmd.c_str());
        if(ret != 0 || access(tmpPath.c_str(), R_OK) != 0)
        {
            std::cerr << "[openrgb] failed to download background image from " << path << std::endl;
            return result;
        }
        
        data = stbi_load(tmpPath.c_str(), &width, &height, &channels, 4);
        std::remove(tmpPath.c_str());
    }
    else
    {
        data = stbi_load(path.c_str(), &width, &height, &channels, 4);
    }

    if(!data || width <= 0 || height <= 0)
    {
        std::cerr << "[openrgb] failed to load background image: " << path << std::endl;
        if(data) stbi_image_free(data);
        return result;
    }

    result.first.resize(width * height * 4);
    std::memcpy(result.first.data(), data, width * height * 4);
    result.second = {width, height};
    
    stbi_image_free(data);
    std::cout << "[openrgb] loaded background image: " << path << " (" << width << "x" << height << ")" << std::endl;
    
    return result;
}

/* Render circles on a background image. */
static std::vector<std::uint8_t> renderCirclesOnImage(const RGBFrame& matrix, 
                                                       std::uint32_t displayW, std::uint32_t displayH,
                                                       const std::vector<std::uint8_t>& bgImage,
                                                       int bgW, int bgH)
{
    const std::size_t matrixW = matrix.width;
    const std::size_t matrixH = matrix.height;

    std::vector<std::uint8_t> output(displayW * displayH * 4, 0);

    if(!bgImage.empty() && bgW > 0 && bgH > 0)
    {
        for(std::uint32_t y = 0; y < displayH; ++y)
        {
            const int srcY = static_cast<int>(static_cast<float>(y) * bgH / displayH);
            const int srcYClamped = std::max(0, std::min(bgH - 1, srcY));
            
            for(std::uint32_t x = 0; x < displayW; ++x)
            {
                const int srcX = static_cast<int>(static_cast<float>(x) * bgW / displayW);
                const int srcXClamped = std::max(0, std::min(bgW - 1, srcX));
                
                const std::size_t srcIdx = (static_cast<std::size_t>(srcYClamped) * bgW + srcXClamped) * 4;
                const std::size_t dstIdx = (y * displayW + x) * 4;
                
                output[dstIdx + 0] = bgImage[srcIdx + 0];
                output[dstIdx + 1] = bgImage[srcIdx + 1];
                output[dstIdx + 2] = bgImage[srcIdx + 2];
                output[dstIdx + 3] = 255;
            }
        }
    }
    else
    {
        std::memset(output.data(), 0, output.size());
    }

    if(matrix.pixels.empty() || matrixW == 0 || matrixH == 0)
    {
        return output;
    }

    const std::uint32_t cellW = displayW / static_cast<std::uint32_t>(matrixW);
    const std::uint32_t cellH = displayH / static_cast<std::uint32_t>(matrixH);
    
    const int radiusX = static_cast<int>(cellW * 0.4f);
    const int radiusY = static_cast<int>(cellH * 0.4f);
    
    static thread_local CircleMaskCache cache;
    const auto& maskX = cache.getMask(radiusX);
    const auto& maskY = cache.getMask(radiusY);

    for(std::size_t y = 0; y < matrixH; ++y)
    {
        for(std::size_t x = 0; x < matrixW; ++x)
        {
            const std::size_t idx = y * matrixW + x;
            if(idx * 3 + 2 >= matrix.pixels.size()) break;

            const std::uint8_t r = matrix.pixels[idx * 3 + 0];
            const std::uint8_t g = matrix.pixels[idx * 3 + 1];
            const std::uint8_t b = matrix.pixels[idx * 3 + 2];

            const int cx = (static_cast<int>(x) * static_cast<int>(cellW)) + (cellW / 2);
            const int cy = (static_cast<int>(y) * static_cast<int>(cellH)) + (cellH / 2);

            const int minX = std::max(0, cx - radiusX);
            const int maxX = std::min(static_cast<int>(displayW) - 1, cx + radiusX);
            const int minY = std::max(0, cy - radiusY);
            const int maxY = std::min(static_cast<int>(displayH) - 1, cy + radiusY);

            const int maskHeight = maskX.height;
            const int maskCenterY = maskHeight / 2;
            
            for(int py = minY; py <= maxY; ++py)
            {
                const int maskYIdx = (py - cy) + maskCenterY;
                if(maskYIdx < 0 || maskYIdx >= maskY.height) continue;
                if(maskY.widths[maskYIdx] == 0) continue;

                const int dx = maskX.xOffsets[maskYIdx];
                const int lineWidth = maskX.widths[maskYIdx];
                
                int lineMinX = cx + dx;
                int lineMaxX = lineMinX + lineWidth - 1;
                
                lineMinX = std::max(minX, lineMinX);
                lineMaxX = std::min(maxX, lineMaxX);
                
                if(lineMinX > lineMaxX) continue;

                std::uint8_t* rowPtr = &output[(static_cast<std::size_t>(py) * displayW + lineMinX) * 4];
                const int pixelCount = lineMaxX - lineMinX + 1;
                
                for(int i = 0; i < pixelCount; ++i)
                {
                    rowPtr[i * 4 + 0] = r;
                    rowPtr[i * 4 + 1] = g;
                    rowPtr[i * 4 + 2] = b;
                    rowPtr[i * 4 + 3] = 255;
                }
            }
        }
    }

    return output;
}

/* Render circles on black background (fallback). */
static std::vector<std::uint8_t> renderCirclesOnBlack(const RGBFrame& matrix, std::uint32_t displayW, std::uint32_t displayH)
{
    const std::size_t matrixW = matrix.width;
    const std::size_t matrixH = matrix.height;

    std::vector<std::uint8_t> output(displayW * displayH * 4, 0);

    if(matrix.pixels.empty() || matrixW == 0 || matrixH == 0)
    {
        return output;
    }

    const std::uint32_t cellW = displayW / static_cast<std::uint32_t>(matrixW);
    const std::uint32_t cellH = displayH / static_cast<std::uint32_t>(matrixH);
    
    const int radiusX = static_cast<int>(cellW * 0.4f);
    const int radiusY = static_cast<int>(cellH * 0.4f);
    
    static thread_local CircleMaskCache cache;
    const auto& maskX = cache.getMask(radiusX);
    const auto& maskY = cache.getMask(radiusY);

    for(std::size_t y = 0; y < matrixH; ++y)
    {
        for(std::size_t x = 0; x < matrixW; ++x)
        {
            const std::size_t idx = y * matrixW + x;
            if(idx * 3 + 2 >= matrix.pixels.size()) break;

            const std::uint8_t r = matrix.pixels[idx * 3 + 0];
            const std::uint8_t g = matrix.pixels[idx * 3 + 1];
            const std::uint8_t b = matrix.pixels[idx * 3 + 2];

            const int cx = (static_cast<int>(x) * static_cast<int>(cellW)) + (cellW / 2);
            const int cy = (static_cast<int>(y) * static_cast<int>(cellH)) + (cellH / 2);

            const int minX = std::max(0, cx - radiusX);
            const int maxX = std::min(static_cast<int>(displayW) - 1, cx + radiusX);
            const int minY = std::max(0, cy - radiusY);
            const int maxY = std::min(static_cast<int>(displayH) - 1, cy + radiusY);

            const int maskHeight = maskX.height;
            const int maskCenterY = maskHeight / 2;
            
            for(int py = minY; py <= maxY; ++py)
            {
                const int maskYIdx = (py - cy) + maskCenterY;
                if(maskYIdx < 0 || maskYIdx >= maskY.height) continue;
                if(maskY.widths[maskYIdx] == 0) continue;

                const int dx = maskX.xOffsets[maskYIdx];
                const int lineWidth = maskX.widths[maskYIdx];
                
                int lineMinX = cx + dx;
                int lineMaxX = lineMinX + lineWidth - 1;
                
                lineMinX = std::max(minX, lineMinX);
                lineMaxX = std::min(maxX, lineMaxX);
                
                if(lineMinX > lineMaxX) continue;

                std::uint8_t* rowPtr = &output[(static_cast<std::size_t>(py) * displayW + lineMinX) * 4];
                const int pixelCount = lineMaxX - lineMinX + 1;
                
                for(int i = 0; i < pixelCount; ++i)
                {
                    rowPtr[i * 4 + 0] = r;
                    rowPtr[i * 4 + 1] = g;
                    rowPtr[i * 4 + 2] = b;
                    rowPtr[i * 4 + 3] = 255;
                }
            }
        }
    }

    return output;
}

class WaywallenBridge {
public:
    WaywallenBridge(std::string ipcPath, std::size_t matrixW, std::size_t matrixH)
        : ipcPath_(std::move(ipcPath)), width_(matrixW), height_(matrixH)
    {
    }

    ~WaywallenBridge()
    {
        stop();
    }

    void connectAndHandshake()
    {
        if(ipcPath_.empty()) return;

        std::cerr << "[openrgb] connecting to waywallen IPC at " << ipcPath_ << std::endl;
        fd_ = ::socket(AF_UNIX, SOCK_STREAM, 0);
        
        if(fd_ < 0)
        {
            throw std::runtime_error("socketpair failed");
        }

        sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        std::strncpy(addr.sun_path, ipcPath_.c_str(), sizeof(addr.sun_path) - 1);
        if(::connect(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
        {
            ::close(fd_);
            fd_ = -1;
            throw std::runtime_error("connect failed");
        }
        std::cerr << "[openrgb] connected to waywallen IPC" << std::endl;

        readInitMessage();
        std::cerr << "[openrgb] received Init from daemon" << std::endl;

        ww_pool_vulkan_init_t init{};
        init.instance = vkContext_.instance();
        init.physical_device = vkContext_.physicalDevice();
        init.device = vkContext_.device();
        init.queue = vkContext_.queue();
        init.queue_family_index = vkContext_.queueFamilyIndex();
        init.get_instance_proc_addr = reinterpret_cast<void* (*)(void*, const char*)>(vkGetInstanceProcAddr);
        init.device_uuid = vkContext_.deviceUUID();
        init.driver_uuid = vkContext_.driverUUID();
        init.drm_render_major = vkContext_.drmRenderMajor();
        init.drm_render_minor = vkContext_.drmRenderMinor();
        init.drm_render_fd = -1;
        init.image_usage_flags = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        init.format_feature_flags = VK_FORMAT_FEATURE_TRANSFER_DST_BIT;

        if(ww_bridge_pool_create(WW_POOL_BACKEND_VULKAN, &init, &pool_) != 0)
        {
            throw std::runtime_error("ww_bridge_pool_create failed");
        }
        std::cerr << "[openrgb] Vulkan pool created" << std::endl;

        constexpr std::uint32_t kMaxWidth = 3840;
        constexpr std::uint32_t kMaxHeight = 2160;
        const int capsRet = ww_bridge_pool_advertise_caps(pool_, fd_,
                                          kMaxWidth, kMaxHeight,
                                          WW_MEM_HINT_DEVICE_LOCAL | WW_MEM_HINT_HOST_VISIBLE);
        if(capsRet != 0)
        {
            std::cerr << "[openrgb] ww_bridge_pool_advertise_caps returned " << capsRet << std::endl;
            throw std::runtime_error("ww_bridge_pool_advertise_caps failed");
        }
        std::cerr << "[openrgb] advertise_caps succeeded (Ready+FormatCaps sent)" << std::endl;

        running_ = true;
        eventThread_ = std::thread([this] { eventLoop(); });
        std::cerr << "[openrgb] event thread started" << std::endl;
    }

    void stop()
    {
        running_ = false;
        if(renderThread_.joinable())
        {
            frameCv_.notify_one();
            renderThread_.join();
        }
        if(eventThread_.joinable())
        {
            eventThread_.join();
        }
        if(fd_ >= 0)
        {
            ::close(fd_);
            fd_ = -1;
        }
        if(pool_ != nullptr)
        {
            ww_bridge_pool_destroy(pool_);
            pool_ = nullptr;
        }
    }

    void setBackgroundImage(std::vector<std::uint8_t> image, int w, int h)
    {
        std::lock_guard<std::mutex> lock(bgMutex_);
        bgImage_ = std::move(image);
        bgWidth_ = w;
        bgHeight_ = h;
    }

    void pushFrame(RGBFrame frame)
    {
        std::lock_guard<std::mutex> lock(frameMutex_);
        latestFrame_ = std::move(frame);
        haveFrame_ = true;
        frameCv_.notify_one();
    }

    bool isReady() const
    {
        return poolReady_;
    }

    void startRenderThread()
    {
        renderThread_ = std::thread([this] { renderLoop(); });
    }

    void renderLoop()
    {
        while(running_)
        {
            std::unique_lock<std::mutex> lock(frameMutex_);
            frameCv_.wait(lock, [this] { return !running_ || haveFrame_; });
            if(!running_) break;
            
            RGBFrame frame = std::move(latestFrame_);
            haveFrame_ = false;
            lock.unlock();
            
            renderFrame(std::move(frame));
        }
    }

    void renderFrame(RGBFrame frame)
    {
        if(!running_ || pool_ == nullptr || !poolReady_)
        {
            return;
        }

        const std::uint32_t slotIndex = nextSlot_++ % std::max<std::uint32_t>(1u, slotCount_);
        ww_pool_slot_t slot{};
        if(ww_bridge_pool_acquire_slot(pool_, slotIndex, &slot) != 0)
        {
            return;
        }

        vkContext_.ensureStagingBuffer(slot.width, slot.height);

        int syncFd = -1;
        if(!frame.pixels.empty())
        {
            std::vector<std::uint8_t> rgba;
            
            // Check if we have a background image
            std::vector<std::uint8_t> bgImage;
            int bgW = 0, bgH = 0;
            {
                std::lock_guard<std::mutex> lock(bgMutex_);
                bgImage = bgImage_;
                bgW = bgWidth_;
                bgH = bgHeight_;
            }
            
            if(!bgImage.empty())
            {
                rgba = renderCirclesOnImage(frame, slot.width, slot.height, bgImage, bgW, bgH);
            }
            else
            {
                rgba = renderCirclesOnBlack(frame, slot.width, slot.height);
            }
            
            vkContext_.uploadToImage(reinterpret_cast<VkImage>(slot.vk_image),
                                     slot.width, slot.height,
                                     rgba.data(), rgba.size(), &syncFd);
        }
        else
        {
            std::vector<std::uint8_t> black(slot.width * slot.height * 4, 0);
            vkContext_.uploadToImage(reinterpret_cast<VkImage>(slot.vk_image),
                                     slot.width, slot.height,
                                     black.data(), black.size(), &syncFd);
        }

        const int rc = ww_bridge_pool_submit_slot(pool_, fd_, slotIndex, syncFd);
        if(rc != 0)
        {
            std::cerr << "[openrgb] submit_slot failed: " << rc << std::endl;
        }
    }

private:
    static int openRenderNode()
    {
        const std::filesystem::path driPath("/dev/dri");
        std::error_code ec;
        if(std::filesystem::exists(driPath, ec))
        {
            for(const auto& entry : std::filesystem::directory_iterator(driPath, ec))
            {
                const std::string name = entry.path().filename().string();
                if(name.rfind("renderD", 0) == 0)
                {
                    const int fd = ::open(entry.path().c_str(), O_RDWR | O_CLOEXEC);
                    if(fd >= 0) return fd;
                }
            }
        }
        return ::open("/dev/dri/renderD128", O_RDWR | O_CLOEXEC);
    }

    static std::vector<std::uint8_t> readExact(int socketFd, std::size_t size)
    {
        std::vector<std::uint8_t> buffer(size);
        std::size_t offset = 0;
        while(offset < size)
        {
            const ssize_t chunk = ::recv(socketFd, buffer.data() + offset, size - offset, 0);
            if(chunk <= 0) throw std::runtime_error("recv failed");
            offset += static_cast<std::size_t>(chunk);
        }
        return buffer;
    }

    void readInitMessage()
    {
        const std::vector<std::uint8_t> header = readExact(fd_, 4);
        const std::uint16_t opcode = static_cast<std::uint16_t>(header[0] | (header[1] << 8));
        const std::uint16_t total = static_cast<std::uint16_t>(header[2] | (header[3] << 8));
        if(total < 4) throw std::runtime_error("invalid init frame");
        const std::size_t bodyLen = total - 4;
        if(bodyLen > 0)
        {
            readExact(fd_, bodyLen);
        }
        if(opcode != WW_EVT_IN_INIT)
        {
            throw std::runtime_error("unexpected control opcode");
        }
    }

    void eventLoop()
    {
        while(running_)
        {
            uint16_t opcode = 0;
            uint8_t* body = nullptr;
            size_t bodyLen = 0;
            int fds[8] = {};
            size_t nFds = 0;
            const int rc = ww_bridge_recv_frame(fd_, &opcode, &body, &bodyLen, fds, 8, &nFds);
            if(rc != 0)
            {
                if(!running_) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            processIncomingMessage(opcode, body, bodyLen, fds, nFds);

            if(body != nullptr)
            {
                std::free(body);
                body = nullptr;
            }
            for(size_t i = 0; i < nFds; ++i)
            {
                if(fds[i] >= 0) ::close(fds[i]);
            }
        }
    }

    void processIncomingMessage(uint16_t opcode, const uint8_t* body, size_t bodyLen, const int* fds, size_t nFds)
    {
        (void)fds;
        (void)nFds;
        switch(opcode)
        {
            case WW_EVT_IN_INIT:
            {
                ww_evt_in_init_t init{};
                if(ww_evt_in_init_decode(body, bodyLen, &init) == 0)
                {
                    ww_evt_in_init_free(&init);
                }
                break;
            }
            case WW_EVT_IN_NEGOTIATE_BUFFERS:
            {
                ww_evt_in_negotiate_buffers_t directive{};
                if(ww_evt_in_negotiate_buffers_decode(body, bodyLen, &directive) == 0)
                {
                    handleNegotiateBuffers(directive);
                    ww_evt_in_negotiate_buffers_free(&directive);
                }
                break;
            }
            case WW_EVT_IN_PLAY:
            {
                ww_evt_in_play_t play{};
                if(ww_evt_in_play_decode(body, bodyLen, &play) == 0)
                {
                    ww_evt_in_play_free(&play);
                }
                break;
            }
            case WW_EVT_IN_SHUTDOWN:
            {
                running_ = false;
                break;
            }
            default:
                break;
        }
    }

    void handleNegotiateBuffers(const ww_evt_in_negotiate_buffers_t& directive)
    {
        if(pool_ == nullptr)
        {
            std::cerr << "[openrgb] pool not created before negotiate_buffers" << std::endl;
            return;
        }

        slotCount_ = std::max<std::uint32_t>(1u, directive.count);
        std::cerr << "[openrgb] negotiate_buffers: " << slotCount_ << " slots, fourcc=0x"
                  << std::hex << directive.fourcc << std::dec
                  << " path=" << directive.path
                  << " mem_source=" << directive.mem_source << std::endl;

        constexpr std::uint32_t kMaxWidth = 3840;
        constexpr std::uint32_t kMaxHeight = 2160;

        ww_pool_directive_t apply{};
        apply.category = directive.path;
        apply.mem_source = directive.mem_source;
        apply.fourcc = directive.fourcc;
        apply.modifier = directive.modifier;
        apply.plane_count = directive.plane_count;
        apply.sync_mode = directive.sync_mode;
        apply.color = directive.color;
        apply.mem_hint = directive.mem_hint;
        apply.width = kMaxWidth;
        apply.height = kMaxHeight;
        apply.count = slotCount_;
        if(ww_bridge_pool_apply_directive(pool_, fd_, &apply) == 0)
        {
            poolReady_ = true;
            std::cerr << "[openrgb] buffer pool ready for rendering" << std::endl;
        }
        else
        {
            std::cerr << "[openrgb] ww_bridge_pool_apply_directive failed" << std::endl;
        }
    }

    std::string ipcPath_;
    std::size_t width_ = 0;
    std::size_t height_ = 0;
    int fd_ = -1;
    bool running_ = false;
    bool poolReady_ = false;
    std::thread eventThread_;
    std::thread renderThread_;
    VulkanContext vkContext_{3840, 2160};
    ww_pool_t* pool_ = nullptr;
    std::uint32_t slotCount_ = 1;
    std::uint32_t nextSlot_ = 0;
    
    std::mutex frameMutex_;
    std::condition_variable frameCv_;
    RGBFrame latestFrame_;
    bool haveFrame_ = false;
    
    std::mutex bgMutex_;
    std::vector<std::uint8_t> bgImage_;
    int bgWidth_ = 0;
    int bgHeight_ = 0;
};

} // namespace

static Settings parseArgs(int argc, char** argv)
{
    Settings settings;
    for(int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];
        if(arg == "--udp-port" && i + 1 < argc)
        {
            settings.udpPort = static_cast<std::uint16_t>(std::stoi(argv[++i]));
        }
        else if(arg == "--background-image" && i + 1 < argc)
        {
            settings.backgroundImage = argv[++i];
        }
        else if(arg == "--background-color" && i + 1 < argc)
        {
            settings.backgroundColor = {1.0f, 1.0f, 1.0f, 1.0f};
        }
        else if(arg == "--width" && i + 1 < argc)
        {
            settings.width = static_cast<std::size_t>(std::stoul(argv[++i]));
        }
        else if(arg == "--height" && i + 1 < argc)
        {
            settings.height = static_cast<std::size_t>(std::stoul(argv[++i]));
        }
        else if(arg == "--ipc" && i + 1 < argc)
        {
            settings.ipcPath = argv[++i];
        }
    }
    const char* envPort = std::getenv("OPENRGB_UDP_PORT");
    if(envPort && *envPort) settings.udpPort = static_cast<std::uint16_t>(std::stoi(envPort));
    const char* envImage = std::getenv("OPENRGB_BACKGROUND_IMAGE");
    if(envImage && *envImage) settings.backgroundImage = envImage;
    return settings;
}

int main(int argc, char** argv)
{
    try
    {
        Settings settings = parseArgs(argc, argv);
        std::cout << "OpenRGB waywallen plugin listening on UDP " << settings.udpPort << std::endl;

        OpenRGBReceiver receiver(settings.udpPort);
        receiver.start();

        std::optional<WaywallenBridge> bridge;
        std::optional<std::pair<std::vector<std::uint8_t>, std::pair<int, int>>> bgImage;
        
        // Load background image if specified
        if(!settings.backgroundImage.empty())
        {
            bgImage = loadBackgroundImage(settings.backgroundImage);
        }

        if(!settings.ipcPath.empty())
        {
            std::cout << "Waiting for OpenRGB settings packet to determine matrix size..." << std::endl;
            bool gotSettings = false;
            for(int i = 0; i < 300; ++i)
            {
                if(receiver.hasSettingsPacket())
                {
                    gotSettings = true;
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            if(!gotSettings)
            {
                std::cout << "No settings packet received; using default 32x32" << std::endl;
            }

            Settings actualSettings = receiver.getSettings();
            std::cout << "Using matrix size: " << actualSettings.width << "x" << actualSettings.height << std::endl;
            bridge.emplace(settings.ipcPath, actualSettings.width, actualSettings.height);
            
            // Set background image if loaded
            if(bgImage.has_value())
            {
                bridge->setBackgroundImage(std::move(bgImage->first), bgImage->second.first, bgImage->second.second);
            }
            
            bridge->connectAndHandshake();
            bridge->startRenderThread();
        }

        constexpr std::chrono::milliseconds new_frame_timeout(1000);

        while(true)
        {
            if(bridge.has_value())
            {
                if(receiver.waitForFrame(new_frame_timeout))
                {
                    std::optional<RGBFrame> latestFrame = receiver.popLatestFrame();
                    if(latestFrame.has_value())
                    {
                        bridge->pushFrame(std::move(*latestFrame));
                    }
                }
            }
            else
            {
                std::this_thread::sleep_for(new_frame_timeout);
            }
        }
    }
    catch(const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
}