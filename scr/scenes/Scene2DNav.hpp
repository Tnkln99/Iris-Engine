#ifndef IRIS_SCENE2DNAV_HPP
#define IRIS_SCENE2DNAV_HPP

#include "../app/Scene.hpp"
#include "../ai/NavigationArea2D.hpp"


namespace iris::scene{
class Scene2DNav : public app::Scene{
    public:
        Scene2DNav(graphics::ForwardRenderer& renderer, graphics::Window& window);
        ~Scene2DNav() = default;

        void loadScene() override;
        void update() override;
        void draw() override;
    private:
        ai::NavigationArea2D m_navigationArea{};
        app::RenderObject m_star01{};

        // this will stay on scene
        void initObjects() override;
        void initLights() override;
        void drawUi() override;
        ai::NavigationTile2D::TileType m_paintType = ai::NavigationTile2D::TileType::WALKABLE;
    };
}



#endif //IRIS_SCENE2DNAV_HPP
