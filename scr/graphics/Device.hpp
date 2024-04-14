#ifndef IRIS_DEVICE_HPP
#define IRIS_DEVICE_HPP

#include "Window.hpp"

#include <vector>
#include <glm/glm.hpp>
#include <iostream>
#include <optional>
#include <string>
#include <vk_mem_alloc.h>
#include <functional>
#include <memory>

namespace iris::graphics{
    struct AllocatedBuffer {
        VkBuffer m_buffer;
        VmaAllocation m_allocation;
    };

    struct AllocatedImage {
        VkImage m_image;
        VmaAllocation m_allocation;
    };

    struct Texture{
        std::string m_name;

        AllocatedImage m_allocatedImage;
        VkImageView m_imageView;
    };

    struct UploadContext {
        VkFence m_uploadFence;
        VkCommandPool m_commandPool;
        VkCommandBuffer m_commandBuffer;
    };

    struct QueueFamilyIndices{
        std::optional<uint32_t> m_graphicsFamily;
        std::optional<uint32_t> m_presentFamily;

        [[nodiscard]] bool isComplete() const {
            return m_graphicsFamily.has_value() && m_presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR m_capabilities;
        std::vector<VkSurfaceFormatKHR> m_formats;
        std::vector<VkPresentModeKHR> m_presentModes;
    };

    class Device {
    public:
#ifdef NDEBUG
        const bool m_EnableValidationLayers = false;
#else
        const bool m_cEnableValidationLayers = true;
#endif

        explicit Device(Window& window);
        ~Device();

        // Not copyable or movable
        Device(const Device &) = delete;
        Device &operator=(const Device &) = delete;
        Device(Device &&) = delete;
        Device &operator=(Device &&) = delete;

        [[nodiscard]] VkCommandPool getCommandPool() const { return m_commandPool; }
        [[nodiscard]] VkDevice getDevice() const { return m_device; }
        [[nodiscard]] VkPhysicalDevice getPhysicalDevice() const { return m_chosenGpu; }
        [[nodiscard]] VkSurfaceKHR getSurface() const { return m_surface; }
        [[nodiscard]] VkQueue getGraphicsQueue() const { return m_graphicsQueue; }
        [[nodiscard]] VkQueue getPresentQueue() const { return m_presentQueue; }
        [[nodiscard]] VkInstance getInstance() const { return m_instance; }

        [[nodiscard]] uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
        [[nodiscard]] VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
        [[nodiscard]] VkPhysicalDeviceProperties getPhysicalDeviceProperties() const { return m_chosenGpuProperties; }
        [[nodiscard]] VkSampleCountFlagBits getMaxUsableSampleCount() const;

        [[nodiscard]] QueueFamilyIndices getQueueFamilyIndices() const { return m_queueFamilyIndices; }
        [[nodiscard]] SwapChainSupportDetails getSwapChainSupport() const;

        void createImageWithInfo(const VkImageCreateInfo &imageInfo,
                                               VkMemoryPropertyFlags properties,
                                               VkImage &image,
                                               VkDeviceMemory &imageMemory);

        AllocatedBuffer createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
        void destroyBuffer(AllocatedBuffer& buffer);
        void destroyImage(AllocatedImage& image);
        void copyToBuffer(void * src, AllocatedBuffer& dst, size_t size);

        void immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);
        AllocatedImage loadTexture(const std::string& filePath);

        VmaAllocator& getAllocator() { return m_allocator; }
    private:
        Window& m_rWindow;

        VmaAllocator m_allocator{};

        UploadContext m_uploadContext{};

        VkInstance m_instance{};
        VkSurfaceKHR m_surface{};

        // physical device
        VkPhysicalDevice m_chosenGpu{};
        VkPhysicalDeviceProperties m_chosenGpuProperties{};

        // logical device
        VkDevice m_device{};
        VkQueue m_graphicsQueue{};
        VkQueue m_presentQueue{};
        QueueFamilyIndices m_queueFamilyIndices;

        VkCommandPool m_commandPool{};

        void createInstance();
        void createSurface();
        void chosePhysicalDevice();
        void createLogicalDevice();
        void createCommandPool();

        void initiateUploadContext();

        [[nodiscard]] std::vector<const char*> getRequiredInstanceExtensions() const;
        void findQueueFamilies();

        const std::vector<const char*> m_cValidationLayers = { "VK_LAYER_KHRONOS_validation" };
        const std::vector<const char*> m_cDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    };
}

#endif //IRIS_DEVICE_HPP
