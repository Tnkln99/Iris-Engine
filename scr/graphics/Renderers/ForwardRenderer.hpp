#ifndef IRIS_FORWARDRENDERER_HPP
#define IRIS_FORWARDRENDERER_HPP

#include "Renderer.hpp"
#include "../Initializers.hpp"
#include "../Debugger.hpp"


namespace iris::graphics{
    class ForwardRenderer : public Renderer {
    public:
        ForwardRenderer(Device& device, Window& window);
        ~ForwardRenderer() override;

        ForwardRenderer(const ForwardRenderer &) = delete;
        ForwardRenderer &operator=(const ForwardRenderer &) = delete;

        void init() override;

        VkCommandBuffer beginFrame() override;
        void endFrame(VkCommandBuffer cmd) override;
        void postRender() override;

        VkRenderPass getRenderPass(){ return m_renderPass; }
    private:
        VkRenderPass m_renderPass{};
        void createRenderPass();

        void createCommandBuffers() override;
        void freeCommandBuffers() override;
    };
}

#endif //IRIS_FORWARDRENDERER_HPP
