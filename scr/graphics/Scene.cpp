#include "Scene.hpp"
#include "AssetsManager.hpp"

namespace iris::graphics{

    Scene::Scene(Renderer &renderer) : m_rRenderer{renderer} {
        m_rRenderer.init();
        utils::Timer::init();

        m_sceneData.m_ambientLightColor = {1.f, 1.f, 1.f, .02f};
    }


    Scene::~Scene() {
        m_renderObjects.clear();
        m_PointLights.clear();
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
        //RenderObject plane{};
        //plane.m_pModel = AssetsManager::getModel("Plane");
        //plane.m_pMaterial = AssetsManager::getMaterial("DefaultMeshNonTextured");
        //plane.m_transform.m_translation = {0.0f, 0.0f, 0.0f};
        //plane.m_transform.m_scale = {0.5f, 0.5f, 0.5f};
        //m_renderObjects.push_back(plane);

        //RenderObject star{};
        //star.m_pModel = AssetsManager::getModel("Star");
        //star.m_pMaterial = AssetsManager::getMaterial("DefaultMeshNonTextured");
        //star.m_transform.m_translation = {-0.3f, 0.2f, 0.0f};
        //star.m_transform.m_scale = {0.5f, 0.5f, 0.5f};
        //m_renderObjects.push_back(star);

        RenderObject texturedStar{};
        texturedStar.m_pModel = AssetsManager::getModel("Star");
        texturedStar.m_pMaterial = AssetsManager::getMaterial("DefaultMeshTextured");
        texturedStar.m_transform.m_translation = {0.3f, 0.2f, 0.0f};
        texturedStar.m_transform.m_scale = {0.5f, 0.5f, 0.5f};
        texturedStar.m_transform.m_rotation = {180.0f, 0.0f, 0.0f};
        m_renderObjects.push_back(texturedStar);
    }

    void Scene::initLights() {
        m_PointLights.emplace_back(glm::vec3(1,1,1), glm::vec4(1,1,0,1));
        m_PointLights.emplace_back(glm::vec3(-1,1,-1), glm::vec4(0,1,1,1));
        for(int i = 0; i < m_PointLights.size(); i++){
            m_sceneData.m_lights[i] = m_PointLights[i].m_gpuLightData;
        }
        m_sceneData.m_numLights = m_PointLights.size();
    }
}