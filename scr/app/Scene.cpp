#include "Scene.hpp"
#include "../graphics/Initializers.hpp"
#include "../graphics/Debugger.hpp"
#include "../graphics/AssetsManager.hpp"

#include <cassert>

namespace iris::app{

    Scene::Scene(ForwardRenderer &renderer) : m_rRenderer{renderer} {
        m_rRenderer.init();
        utils::Timer::init();

        m_sceneData.m_ambientLightColor = {1.f, 1.f, 1.f, .02f};
    }


    Scene::~Scene() {
        m_renderObjects.clear();
    }

    void Scene::update() {

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
        AssetsManager::storeMaterialInstance("NonTextured", m_rRenderer.createMaterialInstance("DefaultMeshNonTextured"));

        RenderObject texturedStar01{};
        texturedStar01.setModel(AssetsManager::getModel("Star"));
        texturedStar01.m_pMaterialInstance = AssetsManager::getMaterialInstance("RedColor");
        texturedStar01.m_transform.m_translation = {-0.3f, 0.2f, 0.0f};
        texturedStar01.m_transform.m_scale = {0.5f, 0.5f, 0.5f};
        m_renderObjects.push_back(texturedStar01);


        RenderObject texturedStar02{};
        texturedStar02.setModel(AssetsManager::getModel("Star"));
        texturedStar02.m_pMaterialInstance =  AssetsManager::getMaterialInstance("StarTextured");
        texturedStar02.m_transform.m_translation = {0.3f, 0.2f, 0.0f};
        texturedStar02.m_transform.m_scale = {0.5f, 0.5f, 0.5f};
        texturedStar02.m_transform.m_rotation = {180.0f, 0.0f, 0.0f};
        m_renderObjects.push_back(texturedStar02);

        RenderObject nonTexturedStar{};
        nonTexturedStar.setModel(AssetsManager::getModel("Star"));
        nonTexturedStar.m_pMaterialInstance =  AssetsManager::getMaterialInstance("NonTextured");
        nonTexturedStar.m_transform.m_translation = {0.0f, 0.2f, 0.6f};
        nonTexturedStar.m_transform.m_scale = {0.5f, 0.5f, 0.5f};
        nonTexturedStar.m_transform.m_rotation = {90.0f, 180.0f, -45.0f};
        m_renderObjects.push_back(nonTexturedStar);
        float xOffset = -1.2f;
        for(int i = 0; i < 9; i++){
            RenderObject square{};
            square.setModel(AssetsManager::getModel("Square"));
            square.m_pMaterialInstance =  AssetsManager::getMaterialInstance("NonTextured");
            square.m_transform.m_translation = {xOffset, 0.8f, 0.0f};
            square.m_transform.m_scale = {0.1f, 0.1f, 0.1f};
            square.m_transform.m_rotation = {0.0f, 45.0f, 90.0f};
            m_renderObjects.push_back(square);
            xOffset += 0.3f;
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