#include "Device.hpp"
#include "Debugger.hpp"
#include "Initializers.hpp"

#include <vector>
#include <set>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace iris::graphics{
     //////////////////////////////
    //Constructor and destructor//
   //////////////////////////////

    Device::Device(Window &window) : m_rWindow{window} {
        createInstance();
        createSurface();
        chosePhysicalDevice();
        createLogicalDevice();
        createCommandPool();

        initiateUploadContext();

        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.physicalDevice = getPhysicalDevice();
        allocatorInfo.device = getDevice();
        allocatorInfo.instance = getInstance();
        vmaCreateAllocator(&allocatorInfo, &m_allocator);
    }

    Device::~Device() {
        vmaDestroyAllocator(m_allocator);

        vkDestroyFence(m_device, m_uploadContext.m_uploadFence, nullptr);
        vkDestroyCommandPool(m_device, m_uploadContext.m_commandPool, nullptr);

        vkDestroyCommandPool(m_device, m_commandPool, nullptr);
        vkDestroyDevice(m_device, nullptr);
        if (m_cEnableValidationLayers) {
            Debugger::freeDebugCallback(m_instance);
        }
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        vkDestroyInstance(m_instance, nullptr);
    }

    ///////////////////////
    //  Private methods //
    /////////////////////

    void Device::createInstance() {
        VkApplicationInfo appInfo = Initializers::createAppInfo("Iris", "Iris Engine");

        auto extensions = getRequiredInstanceExtensions();

        VkInstanceCreateInfo instanceInfo = Initializers::createInstanceInfo(appInfo, extensions, m_cValidationLayers,
                                                                             m_cEnableValidationLayers);
        Debugger::vkCheck(vkCreateInstance(&instanceInfo, nullptr, &m_instance),
                          "failed to create instance");
        Debugger::setupDebugging(m_instance);
    }

    void Device::createSurface() {
        m_rWindow.createWindowSurface(m_instance, &m_surface);
    }

    void Device::chosePhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

        for (const auto &device : devices) {
            auto props = VkPhysicalDeviceProperties{};
            vkGetPhysicalDeviceProperties(device, &props);
            if (props.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                m_chosenGpu = device;
                break;
            }
            else{
                m_chosenGpu = device;
            }
        }

        if (m_chosenGpu == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }

        vkGetPhysicalDeviceProperties(m_chosenGpu, &m_chosenGpuProperties);
        std::cout << "Chosen gpu is : " << m_chosenGpuProperties.deviceName << std::endl;
    }

    void Device::createLogicalDevice() {
        findQueueFamilies();

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {m_queueFamilyIndices.m_graphicsFamily.value(), m_queueFamilyIndices.m_presentFamily.value()};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo = Initializers::createDeiceQueue(queueFamily, 1, &queuePriority);

            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{}; // used to specify features we will use
        VkDeviceCreateInfo createInfo = Initializers::createDeviceInfo(queueCreateInfos,
                                                                       m_cDeviceExtensions,
                                                                       m_cValidationLayers, deviceFeatures,
                                                                       m_cEnableValidationLayers);

        Debugger::vkCheck(vkCreateDevice(m_chosenGpu, &createInfo, nullptr, &m_device), "failed to create logical device");

        vkGetDeviceQueue(m_device, m_queueFamilyIndices.m_graphicsFamily.value(), 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device, m_queueFamilyIndices.m_presentFamily.value(), 0, &m_presentQueue);
    }

    void Device::createCommandPool() {
        VkCommandPoolCreateInfo commandPoolInfo = Initializers::createCommandPoolInfo(m_queueFamilyIndices.m_graphicsFamily.value(),
                                                                                      VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        Debugger::vkCheck(vkCreateCommandPool(m_device, &commandPoolInfo, nullptr, &m_commandPool)
                , "failed to create command pool");
    }


    void Device::initiateUploadContext() {
        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        Debugger::vkCheck(vkCreateFence(m_device, &fenceInfo, nullptr, &m_uploadContext.m_uploadFence),
                          "could not create upload contexts fence");

        VkCommandPoolCreateInfo commandPoolInfo = Initializers::createCommandPoolInfo(m_queueFamilyIndices.m_graphicsFamily.value(), 0);
        Debugger::vkCheck(vkCreateCommandPool(m_device, &commandPoolInfo, nullptr, &m_uploadContext.m_commandPool),
                          "could not create upload contexts command pool");

        //allocate the default command buffer that we will use for the instant commands
        VkCommandBufferAllocateInfo cmdAllocInfo = Initializers::createCommandBufferAllocateInfo(m_uploadContext.m_commandPool, 1);

        Debugger::vkCheck(vkAllocateCommandBuffers(m_device, &cmdAllocInfo, &m_uploadContext.m_commandBuffer),
                          "could not create upload contexts command");
    }

    std::vector<const char *> Device::getRequiredInstanceExtensions() const {
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (m_cEnableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    void Device::findQueueFamilies() {

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_chosenGpu, &queueFamilyCount,
                                                 nullptr); // get the number of queue families

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_chosenGpu, &queueFamilyCount,
                                                 queueFamilies.data()); // get the queue families

        int i = 0;
        for (const auto &queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) { // check if the queue family has the capability of presenting to our window surface
                m_queueFamilyIndices.m_graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(m_chosenGpu, i, m_surface, &presentSupport); // check if the queue family has the capability of presenting to our window surface

            if (presentSupport) {
                m_queueFamilyIndices.m_presentFamily = i;
            }

            if (m_queueFamilyIndices.isComplete()) {
                break;
            }

            i++;
        }
    }

    ///////////////////////
    //  Public methods  //
    /////////////////////

    uint32_t Device::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_chosenGpu, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    VkFormat Device::findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                         VkFormatFeatureFlags features) const {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_chosenGpu, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                       (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }
    }

    VkSampleCountFlagBits Device::getMaxUsableSampleCount() const {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(m_chosenGpu, &physicalDeviceProperties);

        VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts &
                                    physicalDeviceProperties.limits.framebufferDepthSampleCounts;
        if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
        if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
        if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
        if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
        if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
        if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

        return VK_SAMPLE_COUNT_1_BIT;
    }

    SwapChainSupportDetails Device::getSwapChainSupport() const {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_chosenGpu, m_surface, &details.m_capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_chosenGpu, m_surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.m_formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(m_chosenGpu, m_surface, &formatCount, details.m_formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_chosenGpu, m_surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.m_presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                    m_chosenGpu,
                    m_surface,
                    &presentModeCount,
                    details.m_presentModes.data());
        }
        return details;
    }

    void
    Device::createImageWithInfo(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties, VkImage &image,
                                VkDeviceMemory &imageMemory) {
        Debugger::vkCheck(vkCreateImage(m_device, &imageInfo, nullptr, &image)
                , "failed to create image");

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        Debugger::vkCheck(vkAllocateMemory(m_device, &allocInfo, nullptr, &imageMemory),
                          "failed to allocate image memory");

        Debugger::vkCheck(vkBindImageMemory(m_device, image, imageMemory, 0),
                          "failed to bind image memory");
    }

    AllocatedBuffer Device::createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) {
        //allocate vertex buffer
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.pNext = nullptr;

        bufferInfo.size = allocSize;
        bufferInfo.usage = usage;


        VmaAllocationCreateInfo vmaAllocInfo = {};
        vmaAllocInfo.usage = memoryUsage;

        AllocatedBuffer newBuffer{};

        //allocate the buffer
        Debugger::vkCheck(vmaCreateBuffer(m_allocator, &bufferInfo, &vmaAllocInfo,
                                          &newBuffer.m_buffer,
                                          &newBuffer.m_allocation,
                                          nullptr), "Failed to allocate vertex buffer");

        return newBuffer;
    }

    void Device::destroyBuffer(AllocatedBuffer &buffer) {
        vmaDestroyBuffer(m_allocator, buffer.m_buffer, buffer.m_allocation);
    }


    void Device::destroyImage(AllocatedImage &image) {
        vmaDestroyImage(m_allocator, image.m_image, image.m_allocation);
    }

    void Device::copyToBuffer(void *src, AllocatedBuffer &dst, size_t size) {
        void *data;
        vmaMapMemory(m_allocator, dst.m_allocation, &data);
        memcpy(data, src, size);
        vmaUnmapMemory(m_allocator, dst.m_allocation);
    }

    void Device::immediateSubmit(std::function<void(VkCommandBuffer)> &&function) {
        VkCommandBuffer cmd = m_uploadContext.m_commandBuffer;

        //begin the command buffer recording. We will use this command buffer exactly once before resetting, so we tell vulkan that
        VkCommandBufferBeginInfo cmdBeginInfo = Initializers::createCommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        Debugger::vkCheck(vkBeginCommandBuffer(cmd, &cmdBeginInfo),
                          "Failed to begin recording command buffer on immediateSubmit!");

        //execute the function
        function(cmd);

        Debugger::vkCheck(vkEndCommandBuffer(cmd), "Failed to record command buffer on immediateSubmit!");

        VkSubmitInfo submit = {};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.pNext = nullptr;

        submit.waitSemaphoreCount = 0;
        submit.pWaitSemaphores = nullptr;
        submit.pWaitDstStageMask = nullptr;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &cmd;
        submit.signalSemaphoreCount = 0;
        submit.pSignalSemaphores = nullptr;

        //submit command buffer to the queue and execute it.
        // uploadFence will now block until the graphic commands finish execution
        Debugger::vkCheck(vkQueueSubmit(m_graphicsQueue, 1, &submit, m_uploadContext.m_uploadFence)
                , "Failed to submit command buffer on immediateSubmit!");

        vkWaitForFences(m_device, 1, &m_uploadContext.m_uploadFence, true, 9999999999);
        vkResetFences(m_device, 1, &m_uploadContext.m_uploadFence);

        // reset the command buffers inside the command pool
        vkResetCommandPool(m_device, m_uploadContext.m_commandPool, 0);
    }

    AllocatedImage Device::loadTexture(const std::string &filePath) {
        int texWidth, texHeight, texChannels;

        stbi_uc* pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        if (!pixels) {
            std::cout << "Failed to load texture file " << filePath << std::endl;
            exit(1);
        }

        // Flip image vertically
        int width_in_bytes = texWidth * 4;
        unsigned char *top = NULL;
        unsigned char *bottom = NULL;
        unsigned char temp = 0;
        int half_height = texHeight / 2;

        for (int row = 0; row < half_height; row++)
        {
            top = pixels + row * width_in_bytes;
            bottom = pixels + (texHeight - row - 1) * width_in_bytes;
            for (int col = 0; col < width_in_bytes; col++)
            {
                temp = *top;
                *top = *bottom;
                *bottom = temp;
                top++;
                bottom++;
            }
        }

        void* pixel_ptr = pixels;
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        VkFormat image_format = VK_FORMAT_R8G8B8A8_SRGB;

        AllocatedBuffer stagingBuffer = createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

        void* data;
        vmaMapMemory(m_allocator, stagingBuffer.m_allocation, &data);
        memcpy(data, pixel_ptr, static_cast<size_t>(imageSize));
        vmaUnmapMemory(m_allocator, stagingBuffer.m_allocation);

        stbi_image_free(pixels);

        VkExtent3D imageExtent;
        imageExtent.width = static_cast<uint32_t>(texWidth);
        imageExtent.height = static_cast<uint32_t>(texHeight);
        imageExtent.depth = 1;

        VkImageCreateInfo dimg_info = Initializers::createImageInfo(image_format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, imageExtent);

        AllocatedImage outImage{};

        VmaAllocationCreateInfo dimg_allocinfo = {};
        dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        //allocate and create the image
        vmaCreateImage(m_allocator, &dimg_info,
                       &dimg_allocinfo, &outImage.m_image,
                       &outImage.m_allocation, nullptr);

        //transition image to transfer-receiver
        immediateSubmit([&](VkCommandBuffer cmd) {
            VkImageSubresourceRange range;
            range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            range.baseMipLevel = 0;
            range.levelCount = 1;
            range.baseArrayLayer = 0;
            range.layerCount = 1;

            VkImageMemoryBarrier imageBarrier_toTransfer = {};
            imageBarrier_toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

            imageBarrier_toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageBarrier_toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageBarrier_toTransfer.image = outImage.m_image;
            imageBarrier_toTransfer.subresourceRange = range;

            imageBarrier_toTransfer.srcAccessMask = 0;
            imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            //barrier the image into the transfer-receive layout
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                                 0, nullptr,
                                 0, nullptr,
                                 1, &imageBarrier_toTransfer);

            VkBufferImageCopy copyRegion = {};
            copyRegion.bufferOffset = 0;
            copyRegion.bufferRowLength = 0;
            copyRegion.bufferImageHeight = 0;

            copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyRegion.imageSubresource.mipLevel = 0;
            copyRegion.imageSubresource.baseArrayLayer = 0;
            copyRegion.imageSubresource.layerCount = 1;
            copyRegion.imageExtent = imageExtent;

            //copy the buffer into the image
            vkCmdCopyBufferToImage(cmd, stagingBuffer.m_buffer, outImage.m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

            VkImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;

            imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            //barrier the image into the shader readable layout
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toReadable);
        });

        vmaDestroyBuffer(m_allocator, stagingBuffer.m_buffer, stagingBuffer.m_allocation);

        std::cout << "Texture loaded successfully " << filePath << std::endl;

        return outImage;
    }

}
