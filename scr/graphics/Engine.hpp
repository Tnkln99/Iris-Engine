#ifndef IRIS_ENGINE_HPP
#define IRIS_ENGINE_HPP

#include "Device.hpp"
#include "Renderers/Rasteriser.hpp"
#include "Scene.hpp"

namespace iris::graphics {
    class Engine {
    public:
        Engine();
        ~Engine();

        Engine(const Engine &) = delete;
        Engine &operator=(const Engine &) = delete;

        void run();
    private:
        Window m_window{800, 600, "Iris Engine"};
        Device m_device{m_window};
        Rasteriser m_renderer{m_device, m_window};

        Scene m_viewport{m_device, m_renderer};
    };
}


#endif //IRIS_ENGINE_HPP
