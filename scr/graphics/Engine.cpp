#include "Engine.hpp"
#include "AssetsManager.hpp"

namespace iris::graphics {

    Engine::Engine() {

    }


    Engine::~Engine() {

    }

    void Engine::run() {
        while(!m_Window.shouldCloseWindow()){
            m_Window.pollWindowEvents();

            m_Viewport.draw();
        }

        m_Renderer.postRender();
        AssetsManager::clear(m_Device);
    }
}
