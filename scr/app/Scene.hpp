#ifndef IRIS_SCENE_HPP
#define IRIS_SCENE_HPP

#include "../graphics/Renderers/ForwardRenderer.hpp"
#include "../graphics/Pipeline.hpp"
#include "../graphics/Descriptors.hpp"
#include "../ai/NavigationArea2D.hpp"


namespace iris::app{
    class Scene {
    public:
        explicit Scene(graphics::ForwardRenderer& renderer, graphics::Window& window);
        ~Scene();

        void loadScene();
        void update();
        void draw();
    private:
        graphics::ForwardRenderer& m_rRenderer;
        graphics::Window& m_rWindow;

        GpuSceneData m_sceneData{};
        Camera m_camera{m_rWindow};
        std::vector<RenderObject> m_renderObjects{};
        std::vector<PointLight> m_PointLights{};

        ai::NavigationArea2D m_navigationArea{};
        RenderObject m_star01{};

        // this will stay on scene
        void initObjects();
        void initLights();
        void drawUi();
        ai::NavigationTile2D::TileType m_paintType = ai::NavigationTile2D::TileType::WALKABLE;
    };
}



#endif //IRIS_SCENE_HPP
