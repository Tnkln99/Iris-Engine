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

        std::unique_ptr<DescriptorPool> m_GlobalPool{};

        std::unique_ptr<DescriptorSetLayout> m_GlobalSetLayout{};
        std::unique_ptr<DescriptorSetLayout> m_SingleTexturedSetLayout{};

        std::vector<VkDescriptorSet> m_CameraDescriptorSets{};
        std::vector<AllocatedBuffer> m_UboCameraBuffers;

        Camera m_Camera{};
        std::vector<RenderObject> m_RenderObjects{};

        void loadScene();
        void loadModels();
        void loadImages();
        void initDescriptorSets();
        void initMaterials();
        void initObjects();
    };
}



#endif //IRIS_SCENE_HPP
