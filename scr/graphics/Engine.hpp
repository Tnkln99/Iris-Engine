#ifndef IRIS_ENGINE_HPP
#define IRIS_ENGINE_HPP

#include "Device.hpp"
#include "Renderer.hpp"
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
        Window m_Window{800, 600, "Iris Engine"};
        Device m_Device{m_Window};
        Renderer m_Renderer{m_Device, m_Window};

        Scene m_Viewport{m_Device, m_Renderer};
    };
}


#endif //IRIS_ENGINE_HPP
