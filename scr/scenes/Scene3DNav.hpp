#ifndef IRIS_SCENE3DNAV_HPP
#define IRIS_SCENE3DNAV_HPP

#include "../app/Scene.hpp"
#include "../ai/NavigationArea3D.hpp"


namespace iris::scene{
    class Scene3DNav : public app::Scene{
    public:
        Scene3DNav(graphics::ForwardRenderer& renderer, graphics::Window& window);
        ~Scene3DNav() = default;

        void loadScene() override;
        void update() override;
        void draw() override;
    private:
        ai::NavigationArea3D m_navArea{};

        // this will stay on scene
        void initObjects() override;
        void initLights() override;
        void drawUi() override;
    };
}


#endif //IRIS_SCENE3DNAV_HPP
