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

        unsigned int getImagesCount() { return m_SwapchainImages.size(); }
        VkExtent2D getExtent() { return m_SwapChainExtent; }

        uint32_t acquireNextImage(int currentFrame);
        void submitCommandBuffers(const VkCommandBuffer *buffers, int currentFrameIndex);

        [[nodiscard]] VkFormat getSwapchainImageFormat() const { return m_SwapchainImageFormat; }
        [[nodiscard]] VkFormat findDepthFormat() const {
            return m_rDevice.findSupportedFormat(
                    {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        }

        void createFramebuffers(VkRenderPass renderPass);
        VkFramebuffer getFrameBuffer(int index) { return m_SwapchainFramebuffers[index]; }
    private:
        Device& m_rDevice;

        VkSwapchainKHR m_Swapchain{};
        uint32_t m_SwapchainImageIndex{};

        std::vector<VkFramebuffer> m_SwapchainFramebuffers{};

        std::vector<VkImage> m_DepthImages;
        std::vector<VkImageView> m_DepthImageViews;
        std::vector<VkDeviceMemory> m_DepthImageMemories;

        std::vector<VkImage> m_SwapchainImages;
        std::vector<VkImageView> m_SwapchainImageViews;

        VkExtent2D m_WindowExtent{};

        VkFormat m_SwapchainImageFormat;
        VkFormat m_SwapChainDepthFormat;
        VkExtent2D m_SwapChainExtent{};

        std::vector<VkSemaphore> m_PresentSemaphores;
        std::vector<VkSemaphore> m_RenderSemaphores;
        std::vector<VkFence> inFlightFences;
        std::vector<VkFence> imagesInFlight;

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
