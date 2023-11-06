#include "Engine.hpp"
#include "AssetsManager.hpp"

namespace iris::graphics {

    Engine::Engine() {

    }


    Engine::~Engine() {

    }

    void Engine::run() {
        while(!m_window.shouldCloseWindow()){
            m_window.pollWindowEvents();

            m_viewport.draw();
        }

        m_renderer.postRender();
        AssetsManager::clear(m_device);
    }
}
