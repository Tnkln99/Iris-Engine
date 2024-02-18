#ifndef IRIS_RENDERER_HPP
#define IRIS_RENDERER_HPP

#include "../Device.hpp"
#include "../Swapchain.hpp"
#include "../../app/Objects.hpp"


namespace iris::graphics{
    class Renderer {
    public:
        Renderer(Device& device, Window& window);
        virtual ~Renderer();

        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;

        virtual VkCommandBuffer beginFrame() = 0;
        virtual void loadRenderer() = 0;
        virtual void endFrame(VkCommandBuffer cmd) = 0;
        virtual void postRender() = 0;
        virtual void renderScene(VkCommandBuffer cmd, std::vector<app::RenderObject> & renderObjects, app::GpuSceneData sceneData, app::Camera & camera) = 0;

        virtual void init() = 0;

        int getMaximumFramesInFlight(){ return m_pSwapchain->m_cMaxImagesOnFlight; }
        int getCurrentFrame(){ return m_frameCount % m_pSwapchain->m_cMaxImagesOnFlight;}
        VkExtent2D getSwapchainExtent(){ return m_pSwapchain->getExtent(); }
    protected:
        Device& m_rDevice;
        Window& m_rWindow;
        std::unique_ptr<Swapchain> m_pSwapchain;

        std::vector<VkCommandBuffer> m_commandBuffers{};
        virtual void createCommandBuffers() = 0;
        virtual void freeCommandBuffers() = 0;

        int m_frameCount{0};
    };
}


#endif //IRIS_RENDERER_HPP
