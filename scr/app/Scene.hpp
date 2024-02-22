#ifndef IRIS_SCENE_HPP
#define IRIS_SCENE_HPP

#include "../graphics/renderers/ForwardRenderer.hpp"
#include "../graphics/Pipeline.hpp"

namespace iris::app{
    class Scene {
    public:
        explicit Scene(graphics::ForwardRenderer& renderer, graphics::Window& window);
        ~Scene();

        virtual void loadScene() = 0;
        virtual void update() = 0;
        virtual void draw() = 0;
    protected:
        graphics::ForwardRenderer& m_rRenderer;
        graphics::Window& m_rWindow;

        app::GpuSceneData m_sceneData{};
        app::Camera m_camera{m_rWindow};
        std::vector<app::RenderObject> m_renderObjects{};
        std::vector<app::PointLight> m_PointLights{};


        virtual void initObjects() = 0;
        virtual void initLights() = 0;
        virtual void drawUi() = 0;
    };
}

#endif //IRIS_SCENE_HPP
