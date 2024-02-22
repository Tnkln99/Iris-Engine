#ifndef IRIS_ENGINE_HPP
#define IRIS_ENGINE_HPP

#include "graphics/Device.hpp"
#include "graphics/Renderers/ForwardRenderer.hpp"
#include "scenes/Scene2DNav.hpp"
#include "scenes/Scene3DNav.hpp"

namespace iris {
    class Engine {
    public:
        Engine();
        ~Engine();

        Engine(const Engine &) = delete;
        Engine &operator=(const Engine &) = delete;

        void run();
    private:
        graphics::Window m_window{1000, 1000, "Iris Engine"};
        graphics::Device m_device{m_window};
        graphics::ForwardRenderer m_renderer{m_device, m_window};

        scene::Scene3DNav m_scene{m_renderer, m_window};

        void loadModels();
        void loadImages();
    };
}


#endif //IRIS_ENGINE_HPP
