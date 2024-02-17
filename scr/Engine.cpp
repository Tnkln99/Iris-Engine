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
        AssetsManager::clear(m_device);
    }

    void Engine::loadModels() {
        AssetsManager::loadModel(m_device, "Star", "../assets/star/Star.obj");
        AssetsManager::loadModel(m_device, "Plane", "../assets/plane/Plane.obj");
        AssetsManager::loadModel(m_device, "Square", "../assets/Square.obj");
    }

    void Engine::loadImages() {
        AssetsManager::loadTexture(m_device, "StarAmbient", "../assets/star/Ambient.png");
        AssetsManager::loadTexture(m_device, "StarDiffuse", "../assets/star/Diffuse.png");
        AssetsManager::loadTexture(m_device, "StarSpecular", "../assets/star/Specular.png");
        AssetsManager::loadTexture(m_device, "RedColor", "../assets/Red.jpg");
        AssetsManager::loadTexture(m_device, "T_Frame", "../assets/Frame.jpg");
    }
}
