#ifndef IRIS_SWAPCHAIN_HPP
#define IRIS_SWAPCHAIN_HPP

#include "Device.hpp"

namespace iris::graphics{
    class Swapchain {
    public:
        explicit Swapchain(Device& device, VkExtent2D extent);
        ~Swapchain();

        Swapchain(const Swapchain &) = delete;
        Swapchain& operator=(const Swapchain &) = delete;

        const int m_cMaxImagesOnFlight = 2;

        unsigned int getImagesCount() { return m_swapchainImages.size(); }
        VkExtent2D getExtent() { return m_swapChainExtent; }

        uint32_t acquireNextImage(int currentFrame);
        void submitCommandBuffers(const VkCommandBuffer *buffers, int currentFrameIndex);

        [[nodiscard]] VkFormat getSwapchainImageFormat() const { return m_swapchainImageFormat; }
        [[nodiscard]] VkFormat findDepthFormat() const {
            return m_rDevice.findSupportedFormat(
                    {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        }

        void createFramebuffers(VkRenderPass renderPass);
        VkFramebuffer getFrameBuffer(int index) { return m_swapchainFramebuffers[index]; }
    private:
        Device& m_rDevice;

        VkSwapchainKHR m_swapchain{};
        uint32_t m_swapchainImageIndex{};

        std::vector<VkFramebuffer> m_swapchainFramebuffers{};

        // depth attachments
        std::vector<VkImage> m_depthImages;
        std::vector<VkImageView> m_depthImageViews;
        std::vector<VkDeviceMemory> m_depthImageMemories;
        VkFormat m_swapChainDepthFormat;

        // color attachments for forward rendering
        std::vector<VkImage> m_swapchainImages;
        std::vector<VkImageView> m_swapchainImageViews;
        VkFormat m_swapchainImageFormat;

        // color attachments for deferred rendering

        VkExtent2D m_windowExtent{};
        VkExtent2D m_swapChainExtent{};

        std::vector<VkSemaphore> m_presentSemaphores;
        std::vector<VkSemaphore> m_renderSemaphores;
        std::vector<VkFence> m_inFlightFences;
        std::vector<VkFence> m_imagesInFlight;

        void createSwapchain();
        void createImageViews();
        void createDepthResources();
        void createSyncObjects();

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
    };
}


#endif //IRIS_SWAPCHAIN_HPP
