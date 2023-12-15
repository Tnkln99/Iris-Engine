#include "Engine.hpp"
#include "AssetsManager.hpp"

namespace iris::graphics {

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
        AssetsManager::clear(m_device);
    }

    void Engine::loadModels() {
        AssetsManager::loadModel(m_device, "Star", "../assets/models/Star/Star.obj");
        AssetsManager::loadModel(m_device, "Plane", "../assets/models/Plane/Plane.obj");
    }

    void Engine::loadImages() {
        AssetsManager::loadTexture(m_device, "StarAmbient", "../assets/models/Star/Ambient.png");
        AssetsManager::loadTexture(m_device, "StarDiffuse", "../assets/models/Star/Diffuse.png");
        AssetsManager::loadTexture(m_device, "StarSpecular", "../assets/models/Star/Specular.png");
    }
}
