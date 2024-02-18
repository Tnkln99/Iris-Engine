#include "Scene.hpp"
#include "../graphics/AssetsManager.hpp"

#include <imgui.h>

namespace iris::app{

    Scene::Scene(ForwardRenderer &renderer, Window& window) : m_rRenderer{renderer}, m_rWindow{window} {
        m_rRenderer.init();
        utils::Timer::init();

        m_sceneData.m_ambientLightColor = {1.f, 1.f, 1.f, .02f};
    }


    Scene::~Scene() {
        m_renderObjects.clear();
    }

    void Scene::update() {
        ImGuiIO& io = ImGui::GetIO();

        m_camera.update(utils::Timer::getDeltaTime());
        double mouseX = Window::m_sMouseInfo.m_xPos;
        double mouseY = Window::m_sMouseInfo.m_yPos;
        auto rayDir = glm::normalize(glm::vec3(m_camera.getCameraRay(mouseX, mouseY)));
        for(auto & renderObject : m_renderObjects){
            renderObject.updateInfo();
            renderObject.getBoundingBox().update(utils::Timer::getDeltaTime(), renderObject.modelMatrix());
            if ( renderObject.getBoundingBox().rayIntersectsBox(m_camera.m_transform.m_translation, rayDir) && !io.WantCaptureMouse){
                renderObject.getBoundingBox().m_show = true;
                if(Window::m_sMouseInfo.m_isLeftButtonPressedLastFrame){
                    renderObject.m_pMaterialInstance = AssetsManager::getMaterialInstance("MI_StarTextured");
                }
            }
            else{
                renderObject.getBoundingBox().m_show = false;
            }
        }
        Window::m_sMouseInfo.m_isLeftButtonPressedLastFrame = false;
    }

    void Scene::draw() {
        update();
        m_rRenderer.renderScene(m_renderObjects, m_sceneData, m_camera);
    }

    void Scene::loadScene() {
        initObjects();
        initLights();
    }

    void Scene::initObjects() {
        AssetsManager::storeMaterialInstance("MI_RedTextured", m_rRenderer.createMaterialInstance("M_Textured", "RedColor", "RedColor", "RedColor"));
        AssetsManager::storeMaterialInstance("MI_StarTextured", m_rRenderer.createMaterialInstance("M_Textured", "StarAmbient", "StarDiffuse", "StarSpecular"));
        AssetsManager::storeMaterialInstance("MI_FrameTextured", m_rRenderer.createMaterialInstance("M_Textured", "T_Frame", "T_Frame", "T_Frame"));
        AssetsManager::storeMaterialInstance("MI_Default", m_rRenderer.createMaterialInstance("M_Default"));

        //RenderObject texturedStar01{};
        //texturedStar01.setModel(AssetsManager::getModel("Star"));
        //texturedStar01.m_pMaterialInstance = AssetsManager::getMaterialInstance("MI_RedTextured");
        //texturedStar01.m_transform.m_translation = {-20.0f, 0.2f, 0.0f};
        //texturedStar01.m_transform.m_scale = {10.0f, 10.0f, 10.0f};
        //m_renderObjects.push_back(texturedStar01);

        float yOffset = -20.0f;
        for(int i = 0; i < 20; i++){
            float xOffset = -20.0f;
            for(int j = 0; j < 20; j++){
                RenderObject square{};
                square.setModel(AssetsManager::getModel("Square"));
                square.m_pMaterialInstance = AssetsManager::getMaterialInstance("MI_FrameTextured");
                square.m_transform.m_translation = {xOffset, yOffset, 0.0f};
                square.m_transform.m_scale = {1.0f, 1.0f, 1.0f};
                m_renderObjects.push_back(square);
                xOffset += 2.0f;
            }
            yOffset += 2.0f;
        }

    }

    void Scene::initLights() {
        m_PointLights.emplace_back(glm::vec3(0,0,1), glm::vec4(1,1,1,1));
        //m_PointLights.emplace_back(glm::vec3(-1,1,-1), glm::vec4(1,1,1,1));
        for(int i = 0; i < m_PointLights.size(); i++){
            m_sceneData.m_lights[i] = m_PointLights[i].m_gpuLightData;
        }
        m_sceneData.m_numLights = m_PointLights.size();
    }
}