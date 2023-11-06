#ifndef IRIS_SCENE_HPP
#define IRIS_SCENE_HPP

#include "Renderer.hpp"
#include "Pipeline.hpp"
#include "Descriptors.hpp"
#include "Objects.hpp"


namespace iris::graphics{
    class Scene {
    public:
        Scene(Device& device, Renderer& renderer);
        ~Scene();

        void draw();
    private:
        Device& m_rDevice;
        Renderer& m_rRenderer;

        std::unique_ptr<DescriptorPool> m_pGlobalPool{};

        std::unique_ptr<DescriptorSetLayout> m_pGlobalSetLayout{};
        std::unique_ptr<DescriptorSetLayout> m_pSingleTexturedSetLayout{};

        std::vector<VkDescriptorSet> m_cameraDescriptorSets{};
        std::vector<AllocatedBuffer> m_uboCameraBuffers;

        Camera m_camera{};
        std::vector<RenderObject> m_renderObjects{};

        void loadScene();
        void loadModels();
        void loadImages();
        void initDescriptorSets();
        void initMaterials();
        void initObjects();
    };
}



#endif //IRIS_SCENE_HPP
