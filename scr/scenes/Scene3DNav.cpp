#include "Scene3DNav.hpp"
#include "../graphics/AssetsManager.hpp"

#include "imgui.h"


namespace iris::scene{

    Scene3DNav::Scene3DNav(graphics::ForwardRenderer &renderer, graphics::Window& window) : app::Scene(renderer, window){

    }

    void Scene3DNav::update() {
        ImGuiIO& io = ImGui::GetIO();

        m_camera.update(utils::Timer::getDeltaTime());
        double mouseX = graphics::Window::m_sMouseInfo.m_xPos;
        double mouseY = graphics::Window::m_sMouseInfo.m_yPos;
        auto rayDir = glm::normalize(glm::vec3(m_camera.getCameraRay(mouseX, mouseY)));

        for(auto & renderObject : m_renderObjects){
            renderObject.updateInfo();
            renderObject.getBoundingBox().update(utils::Timer::getDeltaTime(), renderObject.modelMatrix());
        }

        graphics::Window::m_sMouseInfo.m_isLeftButtonPressedLastFrame = false;
    }

    void Scene3DNav::draw() {
        update();

        auto cmd = m_rRenderer.beginFrame();
        drawUi();

        m_rRenderer.renderScene(cmd, m_renderObjects, m_sceneData, m_camera);
        m_rRenderer.endFrame(cmd);
    }

    void Scene3DNav::loadScene() {
        initObjects();
        initLights();
    }

    void Scene3DNav::initObjects() {
        graphics::AssetsManager::storeMaterialInstance("MI_Default", m_rRenderer.createMaterialInstance("M_Default"));
        graphics::AssetsManager::storeMaterialInstance("MI_StarTextured", m_rRenderer.createMaterialInstance("M_Textured", "StarAmbient", "StarDiffuse", "StarSpecular"));
        graphics::AssetsManager::storeMaterialInstance("MI_Explored", m_rRenderer.createMaterialInstance("M_Textured", "T_Explored", "T_Explored", "T_Explored"));
        graphics::AssetsManager::storeMaterialInstance("MI_Red", m_rRenderer.createMaterialInstance("M_Textured", "T_Red", "T_Red", "T_Red"));
        graphics::AssetsManager::storeMaterialInstance("MI_Debug", m_rRenderer.createMaterialInstance("DebugBox"));
        m_camera.m_transform.m_translation = glm::vec3(-28, 2.2, 70);


        //app::RenderObject sphere{};
        //sphere.m_transform.m_translation = glm::vec3(-3.0f, -3.0f, -3.0f);
        //sphere.m_pMaterialInstance = graphics::AssetsManager::getMaterialInstance("MI_Explored");
        //sphere.m_transform.m_scale = glm::vec3(2);
        //sphere.setModel(graphics::AssetsManager::getModel("Cube"));
        //sphere.getBoundingBox().update(0, sphere.modelMatrix());
        //m_renderObjects.push_back(sphere);

        for (int i = -4; i < 4; i ++){
            for(int j = -4; j < 4; j++){
                app::RenderObject cube{};
                cube.m_transform.m_translation = glm::vec3(0, j * 2, i * 2);
                cube.m_pMaterialInstance = graphics::AssetsManager::getMaterialInstance("MI_Explored");
                cube.m_transform.m_scale = glm::vec3(2);
                cube.setModel(graphics::AssetsManager::getModel("Cube"));
                cube.getBoundingBox().update(0, cube.modelMatrix());
                m_renderObjects.push_back(cube);
            }
        }

        app::RenderObject cube{};
        cube.m_transform.m_translation = glm::vec3(0);
        cube.m_pMaterialInstance = graphics::AssetsManager::getMaterialInstance("MI_Explored");
        cube.m_transform.m_scale = glm::vec3(2);
        cube.setModel(graphics::AssetsManager::getModel("Cube"));
        cube.getBoundingBox().update(0, cube.modelMatrix());
        m_renderObjects.push_back(cube);

        app::RenderObject cube2{};
        cube2.m_transform.m_translation = glm::vec3(.0f, .0f, -2.0f);
        cube2.m_pMaterialInstance = graphics::AssetsManager::getMaterialInstance("MI_Explored");
        cube2.m_transform.m_scale = glm::vec3(2);
        cube2.setModel(graphics::AssetsManager::getModel("Cube"));
        cube2.getBoundingBox().update(0, cube2.modelMatrix());
        m_renderObjects.push_back(cube2);

        app::RenderObject cube3{};
        cube3.m_transform.m_translation = glm::vec3(.0f, .0f, 2.0f);
        cube3.m_pMaterialInstance = graphics::AssetsManager::getMaterialInstance("MI_Explored");
        cube3.m_transform.m_scale = glm::vec3(2);
        cube3.setModel(graphics::AssetsManager::getModel("Cube"));
        cube3.getBoundingBox().update(0, cube3.modelMatrix());
        m_renderObjects.push_back(cube3);

        app::RenderObject cube4{};
        cube4.m_transform.m_translation = glm::vec3(.0f, .0f, -4.0f);
        cube4.m_pMaterialInstance = graphics::AssetsManager::getMaterialInstance("MI_Explored");
        cube4.m_transform.m_scale = glm::vec3(2);
        cube4.setModel(graphics::AssetsManager::getModel("Cube"));
        cube4.getBoundingBox().update(0, cube4.modelMatrix());
        m_renderObjects.push_back(cube4);

        glm::vec3 location(-15.0f, -15.0f, -15.0f);
        int gridSize = 2; // Size of each grid cell
        int height = 26; // Height of the navigation area
        int width = 26; // Width of the navigation area
        int depth = 26; // Depth of the navigation area
        m_navArea.loadArea(location, gridSize, height, width, depth, m_renderObjects);

        app::RenderObject start{};
        start.m_transform.m_translation = glm::vec3(-16, -5, -5);
        start.m_pMaterialInstance = graphics::AssetsManager::getMaterialInstance("MI_Red");
        start.m_transform.m_scale = glm::vec3(2);
        start.setModel(graphics::AssetsManager::getModel("Star"));
        start.getBoundingBox().update(0, start.modelMatrix());
        m_renderObjects.push_back(start);

        app::RenderObject goal{};
        goal.m_transform.m_translation = glm::vec3(5, 5, 5);
        goal.m_pMaterialInstance = graphics::AssetsManager::getMaterialInstance("MI_Red");
        goal.m_transform.m_scale = glm::vec3(2);
        goal.setModel(graphics::AssetsManager::getModel("Star"));
        goal.getBoundingBox().update(0, goal.modelMatrix());
        m_renderObjects.push_back(goal);

        m_navArea.aStarFindWay(start.m_transform.m_translation, goal.m_transform.m_translation, m_renderObjects);

    }

    void Scene3DNav::initLights() {
        m_PointLights.emplace_back(glm::vec3(16.0f, 16.0f, 16.0f), glm::vec4(1,1,1,1));
        m_PointLights.emplace_back(glm::vec3(-16.0f, -16.0f, -16.0f), glm::vec4(1,1,1,1));
        //m_PointLights.emplace_back(glm::vec3(10,0,1.0), glm::vec4(1,1,1,1));
        //m_PointLights.emplace_back(glm::vec3(10,-10,1.0), glm::vec4(1,1,1,1));
        //m_PointLights.emplace_back(glm::vec3(10,10,1.0), glm::vec4(1,1,1,1));
        //m_PointLights.emplace_back(glm::vec3(-10,-10,1.0), glm::vec4(1,1,1,1));
        //m_PointLights.emplace_back(glm::vec3(-10,10,1.0), glm::vec4(1,1,1,1));
        for(int i = 0; i < m_PointLights.size(); i++){
            m_sceneData.m_lights[i] = m_PointLights[i].m_gpuLightData;
        }
        m_sceneData.m_numLights = m_PointLights.size();
    }

    void Scene3DNav::drawUi() {

    }
}