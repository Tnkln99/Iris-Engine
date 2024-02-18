#ifndef IRIS_FORWARDRENDERER_HPP
#define IRIS_FORWARDRENDERER_HPP

#include "Renderer.hpp"
#include "../Initializers.hpp"
#include "../Debugger.hpp"
#include "../Descriptors.hpp"
#include "../../utilities/Timer.hpp"
#include "imgui_impl_vulkan.h"


namespace iris::graphics{
    class ForwardRenderer : public Renderer {
    public:
        ForwardRenderer(Device& device, Window& window);
        ~ForwardRenderer() override;

        ForwardRenderer(const ForwardRenderer &) = delete;
        ForwardRenderer &operator=(const ForwardRenderer &) = delete;

        void init() override;

        VkCommandBuffer beginFrame() override;
        void loadRenderer() override;
        void endFrame(VkCommandBuffer cmd) override;
        void postRender() override;
        void renderScene(std::vector<RenderObject> & renderObjects, GpuSceneData sceneData, Camera & camera) override;

        VkRenderPass getRenderPass(){ return m_renderPass; }

        std::shared_ptr<Material::MaterialInstance> createMaterialInstance(std::string matName, std::string ambientTex,
                                    std::string diffuseTex, std::string specularTex);
        std::shared_ptr<Material::MaterialInstance> createMaterialInstance(std::string matName);
    private:
        void createCommandBuffers() override;
        void freeCommandBuffers() override;

        std::unique_ptr<DescriptorPool> m_pGlobalPool{};

        std::unique_ptr<DescriptorSetLayout> m_pGlobalSetLayout{};
        std::unique_ptr<DescriptorSetLayout> m_pTexturedSetLayout{};

        std::vector<VkDescriptorSet> m_sceneDescriptorSets{};
        std::vector<AllocatedBuffer> m_uboSceneBuffers;

        void initDescriptorSets();
        void initMaterials();

        VkRenderPass m_renderPass{};
        void createRenderPass();

        void initImgui();
        VkDescriptorPool m_imguiPool;
    };
}

#endif //IRIS_FORWARDRENDERER_HPP
