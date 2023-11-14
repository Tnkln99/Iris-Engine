#ifndef IRIS_RENDERER_HPP
#define IRIS_RENDERER_HPP

#include "../Device.hpp"
#include "../Swapchain.hpp"


namespace iris::graphics{
    class Renderer {
    public:
        Renderer(Device& device, Window& window);
        ~Renderer();

        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;

        virtual VkCommandBuffer beginFrame() = 0;
        virtual void endFrame(VkCommandBuffer cmd) = 0;

        int getMaximumFramesInFlight(){ return m_pSwapchain->m_cMaxImagesOnFlight; }
        int getCurrentFrame(){ return m_frameCount % m_pSwapchain->m_cMaxImagesOnFlight;}
        VkExtent2D getSwapchainExtent(){ return m_pSwapchain->getExtent(); }

        virtual void postRender() = 0;
    private:
        Device& m_rDevice;
        Window& m_rWindow;
        std::unique_ptr<Swapchain> m_pSwapchain;

        int m_frameCount{0};

        std::vector<VkCommandBuffer> m_commandBuffers{};
    };
}


#endif //IRIS_RENDERER_HPP
