#ifndef IRIS_SCENE_HPP
#define IRIS_SCENE_HPP

#include "../graphics/Renderers/ForwardRenderer.hpp"
#include "../graphics/Pipeline.hpp"
#include "../graphics/Descriptors.hpp"
#include "../graphics/Objects.hpp"

using namespace iris::graphics;


namespace iris::app{
    class Scene {
    public:
        explicit Scene(ForwardRenderer& renderer, Window& window);
        ~Scene();

        void loadScene();
        void update();
        void draw();
    private:
        ForwardRenderer& m_rRenderer;
        Window& m_rWindow;

        GpuSceneData m_sceneData{};
        graphics::Camera m_camera{m_rWindow};
        std::vector<RenderObject> m_renderObjects{};
        std::vector<PointLight> m_PointLights{};

        // this will stay on scene
        void initObjects();
        void initLights();
    };
}



#endif //IRIS_SCENE_HPP
