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

namespace iris::graphics{
    struct AllocatedBuffer {
        VkBuffer buffer;
        VmaAllocation allocation;
    };

    struct AllocatedImage {
        VkImage image;
        VmaAllocation allocation;
    };

    struct UploadContext {
        VkFence uploadFence;
        VkCommandPool commandPool;
        VkCommandBuffer commandBuffer;
    };

    struct QueueFamilyIndices{
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        [[nodiscard]] bool isComplete() const {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class Device {
    public:
#ifdef NDEBUG
        const bool m_EnableValidationLayers = false;
#else
        const bool m_EnableValidationLayers = true;
#endif

        explicit Device(Window& window);
        ~Device();

        // Not copyable or movable
        Device(const Device &) = delete;
        Device &operator=(const Device &) = delete;
        Device(Device &&) = delete;
        Device &operator=(Device &&) = delete;

        [[nodiscard]] VkCommandPool getCommandPool() const { return m_CommandPool; }
        [[nodiscard]] VkDevice getDevice() const { return m_Device; }
        [[nodiscard]] VkPhysicalDevice getPhysicalDevice() const { return m_ChosenGpu; }
        [[nodiscard]] VkSurfaceKHR getSurface() const { return m_Surface; }
        [[nodiscard]] VkQueue getGraphicsQueue() const { return m_GraphicsQueue; }
        [[nodiscard]] VkQueue getPresentQueue() const { return m_PresentQueue; }
        [[nodiscard]] VkInstance getInstance() const { return m_Instance; }

        [[nodiscard]] uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
        [[nodiscard]] VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
        [[nodiscard]] VkPhysicalDeviceProperties getPhysicalDeviceProperties() const { return m_ChosenGpuProperties; }
        [[nodiscard]] VkSampleCountFlagBits getMaxUsableSampleCount() const;

        [[nodiscard]] QueueFamilyIndices getQueueFamilyIndices() const { return m_QueueFamilyIndices; }
        [[nodiscard]] SwapChainSupportDetails getSwapChainSupport() const;

        void createImageWithInfo(const VkImageCreateInfo &imageInfo,
                                               VkMemoryPropertyFlags properties,
                                               VkImage &image,
                                               VkDeviceMemory &imageMemory);

        AllocatedBuffer createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
        void destroyBuffer(AllocatedBuffer& buffer);
        void copyToBuffer(void * src, AllocatedBuffer& dst, size_t size);

        void immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);
    private:
        Window& m_rWindow;

        VmaAllocator m_Allocator{};

        UploadContext m_UploadContext{};

        VkInstance m_Instance{};
        VkSurfaceKHR m_Surface{};

        // physical device
        VkPhysicalDevice m_ChosenGpu{};
        VkPhysicalDeviceProperties m_ChosenGpuProperties{};

        // logical device
        VkDevice m_Device{};
        VkQueue m_GraphicsQueue{};
        VkQueue m_PresentQueue{};
        QueueFamilyIndices m_QueueFamilyIndices;

        VkCommandPool m_CommandPool{};

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
