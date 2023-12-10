#ifndef IRIS_SCENE_HPP
#define IRIS_SCENE_HPP

#include "Renderers/ForwardRenderer.hpp"
#include "Pipeline.hpp"
#include "Descriptors.hpp"
#include "Objects.hpp"


namespace iris::graphics{
    struct GpuSceneData{
        glm::mat4 m_projectionMatrix;
        glm::mat4 m_viewMatrix;
        glm::vec4 m_ambientLightColor; // w is intesity
        int m_numLights;
        PointLight::GpuPointLightData m_lights[10];
    };

    class Scene {
    public:
        Scene(Device& device, ForwardRenderer& renderer);
        ~Scene();

        void loadScene();
        void draw();
    private:
        Device& m_rDevice;
        ForwardRenderer& m_rRenderer;

        std::unique_ptr<DescriptorPool> m_pGlobalPool{};

        std::unique_ptr<DescriptorSetLayout> m_pGlobalSetLayout{};
        std::unique_ptr<DescriptorSetLayout> m_pTexturedSetLayout{};

        std::vector<VkDescriptorSet> m_sceneDescriptorSets{};
        std::vector<AllocatedBuffer> m_uboSceneBuffers;
        GpuSceneData m_sceneData{};

        Camera m_camera{};
        std::vector<RenderObject> m_renderObjects{};

        // maybe to renderers
        void initDescriptorSets();
        void initMaterials();

        // this will stay on scene
        void initObjects();
        void initLights();
    };
}



#endif //IRIS_SCENE_HPP
