#ifndef IRIS_RENDERER_HPP
#define IRIS_RENDERER_HPP

#include "Device.hpp"
#include "Swapchain.hpp"

#include <memory>

namespace iris::graphics{
    class Renderer {
    public:
        Renderer(Device& device, Window& window);
        ~Renderer();

        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;

        VkCommandBuffer beginFrame();
        void endFrame(VkCommandBuffer cmd);

        VkRenderPass getRenderPass(){ return m_RenderPass; }
        int getMaximumFramesInFlight(){ return m_pSwapchain->m_cMaxImagesOnFlight; }
        int getCurrentFrame(){ return m_FrameCount % m_pSwapchain->m_cMaxImagesOnFlight;}
        VkExtent2D getSwapchainExtent(){ return m_pSwapchain->getExtent(); }

        void postRender();
    private:
        Device& m_rDevice;
        Window& m_rWindow;
        std::unique_ptr<Swapchain> m_pSwapchain;

        int m_FrameCount{0};

        std::vector<VkCommandBuffer> m_CommandBuffers{};
        VkRenderPass m_RenderPass{};


        void createCommandBuffers();
        void freeCommandBuffers();

        void createRenderPass();
    };

}

#endif //IRIS_RENDERER_HPP
