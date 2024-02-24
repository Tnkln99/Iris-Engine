#include "Scene3DNav.hpp"
#include "../graphics/AssetsManager.hpp"
#include "imgui.h"

#include <vector>
#include <algorithm>


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
        graphics::AssetsManager::storeMaterialInstance("MI_Green", m_rRenderer.createMaterialInstance("M_Textured", "T_Green", "T_Green", "T_Green"));
        graphics::AssetsManager::storeMaterialInstance("MI_Debug", m_rRenderer.createMaterialInstance("DebugBox"));
        m_camera.m_transform.m_translation = glm::vec3(-28, 2.2, 70);


        //app::RenderObject sphere{};
        //sphere.m_transform.m_translation = glm::vec3(-3.0f, -3.0f, -3.0f);
        //sphere.m_pMaterialInstance = graphics::AssetsManager::getMaterialInstance("MI_Explored");
        //sphere.m_transform.m_scale = glm::vec3(2);
        //sphere.setModel(graphics::AssetsManager::getModel("Cube"));
        //sphere.getBoundingBox().update(0, sphere.modelMatrix());
        //m_renderObjects.push_back(sphere);

        for (int i = -5; i < 5; i ++){
            for(int j = -10; j < 10; j++){
                app::RenderObject cube{};
                cube.m_transform.m_translation = glm::vec3(2, j * 2 + 3, i * 2 + 2);
                cube.m_pMaterialInstance = graphics::AssetsManager::getMaterialInstance("MI_Explored");
                cube.m_transform.m_scale = glm::vec3(2);
                cube.setModel(graphics::AssetsManager::getModel("Cube"));
                cube.getBoundingBox().update(0, cube.modelMatrix());
                m_renderObjects.push_back(cube);
            }
        }

        for (int i = -5; i < 5; i ++){
            for(int j = -10; j < 10; j++){
                app::RenderObject cube{};
                cube.m_transform.m_translation = glm::vec3(-4, j * 2 + 3, i * 2 + 10);
                cube.m_pMaterialInstance = graphics::AssetsManager::getMaterialInstance("MI_Explored");
                cube.m_transform.m_scale = glm::vec3(2);
                cube.setModel(graphics::AssetsManager::getModel("Cube"));
                cube.getBoundingBox().update(0, cube.modelMatrix());
                m_renderObjects.push_back(cube);
            }
        }

        app::RenderObject plane{};
        plane.m_transform.m_translation = glm::vec3(-10,10,10);
        plane.m_pMaterialInstance = graphics::AssetsManager::getMaterialInstance("MI_Green");
        plane.m_transform.m_scale = glm::vec3(5);
        plane.setModel(graphics::AssetsManager::getModel("Plane"));
        plane.getBoundingBox().update(0, plane.modelMatrix());
        m_renderObjects.push_back(plane);

        glm::vec3 location(-16.0f, -16.0f, -16.0f);
        int gridSize = 4; // Size of each grid cell
        int height = 52; // Height of the navigation area
        int width = 52; // Width of the navigation area
        int depth = 52; // Depth of the navigation area
        m_navArea.loadArea(location, gridSize, height, width, depth, m_renderObjects);

        app::RenderObject start{};
        start.m_transform.m_translation = glm::vec3(-15, -5, -5);
        start.m_pMaterialInstance = graphics::AssetsManager::getMaterialInstance("MI_StarTextured");
        start.m_transform.m_scale = glm::vec3(2);
        start.m_name = "start";
        start.setModel(graphics::AssetsManager::getModel("Star"));
        start.getBoundingBox().update(0, start.modelMatrix());
        m_renderObjects.push_back(start);

        app::RenderObject goal{};
        goal.m_transform.m_translation = glm::vec3(10, 0, 5);
        goal.m_pMaterialInstance = graphics::AssetsManager::getMaterialInstance("MI_StarTextured");
        goal.m_transform.m_scale = glm::vec3(2);
        goal.m_name = "goal";
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
        ImGui::Begin("Scene3DNav");
        ImGui::Text("Camera Position: x: %f, y: %f, z: %f", m_camera.m_transform.m_translation.x, m_camera.m_transform.m_translation.y, m_camera.m_transform.m_translation.z);
        ImGui::End();


        ImGui::Begin("Settings");

        static bool showGrid = false;
        ImGui::Checkbox("ShowGrid",&showGrid);

        for (auto& rb : m_renderObjects) {
            if(rb.m_name == "grid"){
                rb.m_bRender = showGrid;
            }
            // Skip "path" and "default" objects
            if (rb.m_name == "path" || rb.m_name == "default" || rb.m_name == "grid") continue;

            // Use a TreeNode to represent each RenderObject
            if (ImGui::TreeNode(rb.m_name.c_str())) {
                // Now, inside the tree, we can allow editing of positions
                ImGui::InputFloat("X Position", &(rb.m_transform.m_translation.x));
                ImGui::InputFloat("Y Position", &(rb.m_transform.m_translation.y));
                ImGui::InputFloat("Z Position", &(rb.m_transform.m_translation.z));


                ImGui::TreePop();
            }
        }

        if(ImGui::Button("Find Path")){
            // Use erase-remove idiom to remove all objects with m_name == "path"
            m_renderObjects.erase(std::remove_if(m_renderObjects.begin(), m_renderObjects.end(),
                                                 [](const app::RenderObject& obj) { return obj.m_name == "path"; }),
                                  m_renderObjects.end());

            glm::vec3 startPos;
            glm::vec3 goalPos;

            for(auto & ro : m_renderObjects){
                if(ro.m_name == "goal"){
                    goalPos = ro.m_transform.m_translation;
                }

                if(ro.m_name == "start"){
                    startPos = ro.m_transform.m_translation;
                }
            }


            m_navArea.aStarFindWay(startPos, goalPos, m_renderObjects);
        }



        ImGui::End();

    }
}