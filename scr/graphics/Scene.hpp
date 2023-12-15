#ifndef IRIS_SCENE_HPP
#define IRIS_SCENE_HPP

#include "Renderers/ForwardRenderer.hpp"
#include "Pipeline.hpp"
#include "Descriptors.hpp"
#include "Objects.hpp"


namespace iris::graphics{
    class Scene {
    public:
        explicit Scene(ForwardRenderer& renderer);
        ~Scene();

        void loadScene();
        void update();
        void draw();
    private:
        ForwardRenderer& m_rRenderer;

        GpuSceneData m_sceneData{};
        Camera m_camera{};
        std::vector<RenderObject> m_renderObjects{};
        std::vector<PointLight> m_PointLights{};

        // this will stay on scene
        void initObjects();
        void initLights();
    };
}



#endif //IRIS_SCENE_HPP
