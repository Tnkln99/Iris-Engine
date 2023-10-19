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

        std::unique_ptr<Pipeline> m_DefaultPipeline;
        VkPipelineLayout m_DefaultPipelineLayout{};

        std::unique_ptr<DescriptorPool> m_GlobalPool{};
        std::vector<VkDescriptorSet> m_GlobalDescriptorSets;

        std::vector<AllocatedBuffer> m_UboCameraBuffers;

        RenderObject m_BoatObject{};
        Camera m_Camera{};

        void loadScene();
    };
}



#endif //IRIS_SCENE_HPP
