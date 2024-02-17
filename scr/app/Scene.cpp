#include "Scene.hpp"
#include "../graphics/AssetsManager.hpp"

#include <cassert>

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
        m_camera.update(utils::Timer::getDeltaTime());
        double mouseX = Window::m_sMouseInfo.m_xPos;
        double mouseY = Window::m_sMouseInfo.m_yPos;
        auto rayDir = glm::normalize(glm::vec3(m_camera.getCameraRay(mouseX, mouseY)));
        std::cout << "rayDir: " << rayDir.x << " " << rayDir.y << " " << rayDir.z << std::endl;
        for(auto & renderObject : m_renderObjects){
            renderObject.updateInfo();
            renderObject.getBoundingBox().update(utils::Timer::getDeltaTime(), renderObject.modelMatrix());
            if ( renderObject.getBoundingBox().rayIntersectsBox(m_camera.m_transform.m_translation, rayDir)){
                renderObject.getBoundingBox().m_show = true;
                if(Window::m_sMouseInfo.m_isLeftButtonPressedLastFrame){
                    std::cout << "Clicked on object" << std::endl;
                    renderObject.m_pMaterialInstance = AssetsManager::getMaterialInstance("StarTextured");
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
        AssetsManager::storeMaterialInstance("RedColor", m_rRenderer.createMaterialInstance("DefaultMeshTextured", "RedColor", "RedColor", "RedColor"));
        AssetsManager::storeMaterialInstance("StarTextured", m_rRenderer.createMaterialInstance("DefaultMeshTextured", "StarAmbient", "StarDiffuse", "StarSpecular"));
        AssetsManager::storeMaterialInstance("MI_Frame", m_rRenderer.createMaterialInstance("DefaultMeshTextured", "T_Frame", "T_Frame", "T_Frame"));
        AssetsManager::storeMaterialInstance("NonTextured", m_rRenderer.createMaterialInstance("DefaultMeshNonTextured"));

        float xOffset = -6.0f;
        for(int i = 0; i < 9; i++){
            RenderObject square{};
            square.setModel(AssetsManager::getModel("Square"));
            square.m_pMaterialInstance = AssetsManager::getMaterialInstance("MI_Frame");
            square.m_transform.m_translation = {xOffset, 2.0f, 0.0f};
            square.m_transform.m_scale = {1.0f, 1.0f, 1.0f};
            m_renderObjects.push_back(square);
            xOffset += 2.0f;
        }

    }

    void Scene::initLights() {
        m_PointLights.emplace_back(glm::vec3(1,1,1), glm::vec4(1,1,1,1));
        m_PointLights.emplace_back(glm::vec3(-1,1,-1), glm::vec4(1,1,1,1));
        for(int i = 0; i < m_PointLights.size(); i++){
            m_sceneData.m_lights[i] = m_PointLights[i].m_gpuLightData;
        }
        m_sceneData.m_numLights = m_PointLights.size();
    }
}