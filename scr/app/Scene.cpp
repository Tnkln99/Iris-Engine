#include "Scene.hpp"
#include "../graphics/AssetsManager.hpp"

#include <imgui.h>

namespace iris::app{

    Scene::Scene(graphics::ForwardRenderer &renderer, graphics::Window& window) : m_rRenderer{renderer}, m_rWindow{window} {
        m_rRenderer.init();
        utils::Timer::init();

        m_sceneData.m_ambientLightColor = {1.f, 1.f, 1.f, .02f};
    }


    Scene::~Scene() {
        m_renderObjects.clear();
    }

    void Scene::update() {
        m_renderObjects.clear();
        ImGuiIO& io = ImGui::GetIO();

        m_camera.update(utils::Timer::getDeltaTime());
        double mouseX = graphics::Window::m_sMouseInfo.m_xPos;
        double mouseY = graphics::Window::m_sMouseInfo.m_yPos;
        auto rayDir = glm::normalize(glm::vec3(m_camera.getCameraRay(mouseX, mouseY)));
        // mouse button logic
        for(auto & tile : m_navigationArea.m_tiles){
            tile.second.updateInfo();
            tile.second.getBoundingBox().update(utils::Timer::getDeltaTime(), tile.second.modelMatrix());
            if ( tile.second.getBoundingBox().rayIntersectsBox(m_camera.m_transform.m_translation, rayDir)
                    && !io.WantCaptureMouse && m_paintType != ai::NavigationTile2D::TileType::NONE){
                tile.second.getBoundingBox().m_show = true;
                if(graphics::Window::m_sMouseInfo.m_isLeftButtonPressedLastFrame){
                    if (m_navigationArea.m_startTileIndex == tile.first)
                        continue;
                    if (m_navigationArea.m_targetTileIndex == tile.first)
                        continue;
                    tile.second.changeType(m_paintType);
                    if(m_paintType == ai::NavigationTile2D::TileType::START){
                        if ( m_navigationArea.m_startTileIndex != -1)
                            m_navigationArea.m_tiles[m_navigationArea.m_startTileIndex].changeType(ai::NavigationTile2D::TileType::WALKABLE);
                        m_navigationArea.m_startTileIndex = tile.first;
                    }

                    if(m_paintType == ai::NavigationTile2D::TileType::TARGET){
                        if ( m_navigationArea.m_targetTileIndex != -1)
                            m_navigationArea.m_tiles[m_navigationArea.m_targetTileIndex].changeType(ai::NavigationTile2D::TileType::WALKABLE);
                        m_navigationArea.m_targetTileIndex = tile.first;
                    }
                }
            }
            else{
                tile.second.getBoundingBox().m_show = false;
            }
            m_renderObjects.push_back(tile.second);
        }


        //m_star01.updateInfo();
        //m_star01.getBoundingBox().update(utils::Timer::getDeltaTime(), m_star01.modelMatrix());
        //if (m_star01.getBoundingBox().rayIntersectsBox(m_camera.m_transform.m_translation, rayDir) && !io.WantCaptureMouse){
        //    std::cout << "Star intersects" << std::endl;
        //    m_star01.getBoundingBox().m_show = true;
        //    if(Window::m_sMouseInfo.m_isLeftButtonPressedLastFrame){
        //        m_star01.m_pMaterialInstance = AssetsManager::getMaterialInstance("MI_StarTextured");
        //    }
        //}
        //else{
        //    m_star01.getBoundingBox().m_show = false;
        //}
        //m_renderObjects.push_back(m_star01);

        graphics::Window::m_sMouseInfo.m_isLeftButtonPressedLastFrame = false;
    }

    void Scene::draw() {
        update();

        auto cmd = m_rRenderer.beginFrame();
        drawUi();
        m_rRenderer.renderScene(cmd, m_renderObjects, m_sceneData, m_camera);
        m_rRenderer.endFrame(cmd);
    }

    void Scene::loadScene() {
        initObjects();
        initLights();
    }

    void Scene::initObjects() {
        graphics::AssetsManager::storeMaterialInstance("MI_Obstacle", m_rRenderer.createMaterialInstance("M_Textured", "T_Obstacle", "T_Obstacle", "T_Obstacle"));
        graphics::AssetsManager::storeMaterialInstance("MI_StarTextured", m_rRenderer.createMaterialInstance("M_Textured", "StarAmbient", "StarDiffuse", "StarSpecular"));
        graphics::AssetsManager::storeMaterialInstance("MI_Man", m_rRenderer.createMaterialInstance("M_Textured", "T_Man", "T_Man", "T_Man"));
        graphics::AssetsManager::storeMaterialInstance("MI_Target", m_rRenderer.createMaterialInstance("M_Textured", "T_Target", "T_Target", "T_Target"));
        graphics::AssetsManager::storeMaterialInstance("MI_Walkable", m_rRenderer.createMaterialInstance("M_Textured", "T_Walkable", "T_Walkable", "T_Walkable"));
        graphics::AssetsManager::storeMaterialInstance("MI_Road", m_rRenderer.createMaterialInstance("M_Textured", "T_Road", "T_Road", "T_Road"));
        graphics::AssetsManager::storeMaterialInstance("MI_Explored", m_rRenderer.createMaterialInstance("M_Textured", "T_Explored", "T_Explored", "T_Explored"));
        graphics::AssetsManager::storeMaterialInstance("MI_Bush", m_rRenderer.createMaterialInstance("M_Textured", "T_Bush", "T_Bush", "T_Bush"));
        graphics::AssetsManager::storeMaterialInstance("MI_ExploredBush", m_rRenderer.createMaterialInstance("M_Textured", "T_ExploredBush", "T_ExploredBush", "T_ExploredBush"));
        graphics::AssetsManager::storeMaterialInstance("MI_RoadBush", m_rRenderer.createMaterialInstance("M_Textured", "T_RoadBush", "T_RoadBush", "T_RoadBush"));
        graphics::AssetsManager::storeMaterialInstance("MI_Default", m_rRenderer.createMaterialInstance("M_Default"));

        //m_star01.setModel(AssetsManager::getModel("Star"));
        //m_star01.m_pMaterialInstance = AssetsManager::getMaterialInstance("MI_Obstacle");
        //m_star01.m_transform.m_translation = {-20.0f, 0.2f, 0.0f};
        //m_star01.m_transform.m_scale = {20.0f, 20.0f, 20.0f};

        m_navigationArea.loadArea();
    }

    void Scene::initLights() {
        m_PointLights.emplace_back(glm::vec3(0,0,1.0), glm::vec4(1,1,1,1));
        m_PointLights.emplace_back(glm::vec3(-10,0,1.0), glm::vec4(1,1,1,1));
        m_PointLights.emplace_back(glm::vec3(10,0,1.0), glm::vec4(1,1,1,1));
        m_PointLights.emplace_back(glm::vec3(10,-10,1.0), glm::vec4(1,1,1,1));
        m_PointLights.emplace_back(glm::vec3(10,10,1.0), glm::vec4(1,1,1,1));
        m_PointLights.emplace_back(glm::vec3(-10,-10,1.0), glm::vec4(1,1,1,1));
        m_PointLights.emplace_back(glm::vec3(-10,10,1.0), glm::vec4(1,1,1,1));
        for(int i = 0; i < m_PointLights.size(); i++){
            m_sceneData.m_lights[i] = m_PointLights[i].m_gpuLightData;
        }
        m_sceneData.m_numLights = m_PointLights.size();
    }

    void Scene::drawUi() {
        static enum { ShowPath, ShowSettings } currentTab = ShowSettings;
        ImGui::SetNextWindowSize(ImVec2(0, 0)); // Suggests ImGui to auto-fit the window size
        ImGui::Begin("Menu");
        static int currentPaint = 0; // Current item index
        static int algoChosen = 0; // Variable to store the chosen item, -1 means no item chosen yet
        static int bushMovementCost = 2;

        switch (currentTab) {
            case ShowPath:
                if(ImGui::Button("Reset")){
                    m_navigationArea.resetPath();
                    m_paintType = ai::NavigationTile2D::TileType::WALKABLE;
                    currentPaint = 0;
                    currentTab = ShowSettings;
                }
                break;
            case ShowSettings:
            {
                const char* paints[] = { "Walkable", "Bush", "Obstacle", "Target", "Start" };
                static int chosenPaint = 0; // Variable to store the chosen item, -1 means no item chosen yet

                // Create a combo box
                if (ImGui::Combo("Paint", &currentPaint, paints, static_cast<int>(ai::NavigationTile2D::TileType::ROAD)))
                {
                    chosenPaint = currentPaint;
                    m_paintType = static_cast<ai::NavigationTile2D::TileType>(chosenPaint);
                }

                // Optionally, display the chosen item
                if (chosenPaint != -1)
                {
                    ImGui::Text("Chosen paint: %s", paints[chosenPaint]);
                }


                const char* algos[] = { "Breadth First Search", "Dijkstra’s Algorithm", "Greedy Best First Search", "A* Search" };
                static int currenAlgo = 0; // Current item index

                // Create a combo box
                if (ImGui::Combo("Algorithmes", &currenAlgo, algos, IM_ARRAYSIZE(algos)))
                {
                    // Item selection logic can be handled here if needed
                    algoChosen = currenAlgo;
                }

                // Optionally, display the chosen item
                if (algoChosen != -1)
                {
                    ImGui::Text("Chosen algo: %s", algos[algoChosen]);
                    if (algoChosen == 1 || algoChosen == 3){
                        // Display the text
                        ImGui::Text("Considering movement cost of normal terrain is 1, movement cost of bushes are:");

                        // Integer input for the bush movement cost
                        ImGui::InputInt("Bushes Movement Cost", &bushMovementCost);
                        if (bushMovementCost <= 1) {
                            bushMovementCost = 2;
                        }
                    }
                }

                if (ImGui::Button("FindPath"))
                {
                    if(m_navigationArea.m_startTileIndex == -1 || m_navigationArea.m_targetTileIndex == -1){
                        ImGui::OpenPopup("MustChosePopUp");
                    }
                    else if (algoChosen == 0){
                        m_navigationArea.breadthFirstSearch();
                        m_paintType = ai::NavigationTile2D::TileType::NONE;
                        currentTab = ShowPath;
                    }
                    else if (algoChosen == 1){
                        m_navigationArea.dijkstra(bushMovementCost);
                        m_paintType = ai::NavigationTile2D::TileType::NONE;
                        currentTab = ShowPath;
                    }
                    else if (algoChosen == 2){
                        m_navigationArea.greedyBestFirstSearch();
                        m_paintType = ai::NavigationTile2D::TileType::NONE;
                        currentTab = ShowPath;
                    }
                    else if (algoChosen == 3){
                        m_navigationArea.aStar(bushMovementCost);
                        m_paintType = ai::NavigationTile2D::TileType::NONE;
                        currentTab = ShowPath;
                    }
                }

                if (ImGui::BeginPopup("MustChoosePopUp"))
                {
                    ImGui::Text("You should choose a start and target tile to find a path.");
                    // Close button inside the popup
                    if (ImGui::Button("Close")) {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }
            }
            break;
        }

        ImGui::End();
    }
}