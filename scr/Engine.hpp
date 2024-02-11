#ifndef IRIS_ENGINE_HPP
#define IRIS_ENGINE_HPP

#include "graphics/Device.hpp"
#include "graphics/Renderers/ForwardRenderer.hpp"
#include "app/Scene.hpp"

namespace iris {
    class Engine {
    public:
        Engine();
        ~Engine();

        Engine(const Engine &) = delete;
        Engine &operator=(const Engine &) = delete;

        void run();
    private:
        graphics::Window m_window{800, 600, "Iris Engine"};
        graphics::Device m_device{m_window};
        graphics::ForwardRenderer m_renderer{m_device, m_window};

        app::Scene m_scene{m_renderer};

        void loadModels();
        void loadImages();
    };
}


#endif //IRIS_ENGINE_HPP