#ifndef IRIS_RASTERISER_HPP
#define IRIS_RASTERISER_HPP

#include "Renderer.hpp"
#include "../Initializers.hpp"
#include "../Debugger.hpp"


namespace iris::graphics{
    class Rasteriser : public Renderer {
    public:
        Rasteriser(Device& device, Window& window);
        ~Rasteriser() override;

        Rasteriser(const Rasteriser &) = delete;
        Rasteriser &operator=(const Rasteriser &) = delete;

        VkCommandBuffer beginFrame() override;
        void endFrame(VkCommandBuffer cmd) override;
        void postRender() override;

        VkRenderPass getRenderPass(){ return m_renderPass; }
    private:
        VkRenderPass m_renderPass{};
        void createRenderPass();

        std::vector<VkCommandBuffer> m_commandBuffers{};
        void createCommandBuffers(){
            m_commandBuffers.resize(m_pSwapchain->m_cMaxImagesOnFlight);
            VkCommandBufferAllocateInfo allocInfo = Initializers::createCommandBufferAllocateInfo(m_rDevice.getCommandPool(), static_cast<uint32_t>(m_commandBuffers.size()));
            Debugger::vkCheck(vkAllocateCommandBuffers(m_rDevice.getDevice(), &allocInfo, m_commandBuffers.data()),
                              "Failed to allocate command buffers!");
        }
        void freeCommandBuffers(){
            vkFreeCommandBuffers(
                    m_rDevice.getDevice(),
                    m_rDevice.getCommandPool(),
                    static_cast<uint32_t>(m_commandBuffers.size()),
                    m_commandBuffers.data());
            m_commandBuffers.clear();
        }
    };
}

#endif //IRIS_RASTERISER_HPP
