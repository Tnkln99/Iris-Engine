#include "Engine.hpp"
#include "graphics/AssetsManager.hpp"

namespace iris {

    Engine::Engine() {
        loadModels();
        loadImages();
        m_renderer.loadRenderer();
        m_scene.loadScene();
    }


    Engine::~Engine() {

    }

    void Engine::run() {
        while(!m_window.shouldCloseWindow()){
            m_window.pollWindowEvents();

            m_scene.draw();
        }

        m_renderer.postRender();
        graphics::AssetsManager::clear(m_device);
    }

    void Engine::loadModels() {
        graphics::AssetsManager::loadModel(m_device, "Star", "../assets/star/Star.obj");
        graphics::AssetsManager::loadModel(m_device, "Plane", "../assets/plane/Plane.obj");
        graphics::AssetsManager::loadModel(m_device, "Square", "../assets/Square.obj");
    }

    void Engine::loadImages() {
        graphics::AssetsManager::loadTexture(m_device, "StarAmbient", "../assets/star/Ambient.png");
        graphics::AssetsManager::loadTexture(m_device, "StarDiffuse", "../assets/star/Diffuse.png");
        graphics::AssetsManager::loadTexture(m_device, "StarSpecular", "../assets/star/Specular.png");
        graphics::AssetsManager::loadTexture(m_device, "T_Obstacle", "../assets/Obstacle.png");
        graphics::AssetsManager::loadTexture(m_device, "T_Target", "../assets/Target.png");
        graphics::AssetsManager::loadTexture(m_device, "T_Man", "../assets/Man.png");
        graphics::AssetsManager::loadTexture(m_device, "T_Walkable", "../assets/Walkable.png");
        graphics::AssetsManager::loadTexture(m_device, "T_Road", "../assets/Road.png");
        graphics::AssetsManager::loadTexture(m_device, "T_Explored", "../assets/RoadV2.png");
    }
}
