#ifndef IRIS_RASTERISER_HPP
#define IRIS_RASTERISER_HPP

#include "Device.hpp"
#include "Swapchain.hpp"

#include <memory>

namespace iris::graphics{
    class Rasteriser {
    public:
        Rasteriser(Device& device, Window& window);
        ~Rasteriser();

        Rasteriser(const Rasteriser &) = delete;
        Rasteriser &operator=(const Rasteriser &) = delete;

        VkCommandBuffer beginFrame();
        void endFrame(VkCommandBuffer cmd);

        VkRenderPass getRenderPass(){ return m_renderPass; }
        int getMaximumFramesInFlight(){ return m_pSwapchain->m_cMaxImagesOnFlight; }
        int getCurrentFrame(){ return m_frameCount % m_pSwapchain->m_cMaxImagesOnFlight;}
        VkExtent2D getSwapchainExtent(){ return m_pSwapchain->getExtent(); }

        void postRender();
    private:
        Device& m_rDevice;
        Window& m_rWindow;
        std::unique_ptr<Swapchain> m_pSwapchain;

        int m_frameCount{0};

        std::vector<VkCommandBuffer> m_commandBuffers{};
        VkRenderPass m_renderPass{};

        void createCommandBuffers();
        void freeCommandBuffers();

        void createRenderPass();
    };
}

#endif //IRIS_RASTERISER_HPP
