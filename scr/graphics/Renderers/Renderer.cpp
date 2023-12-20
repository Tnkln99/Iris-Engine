#include "Renderer.hpp"

namespace iris::graphics{

    Renderer::Renderer(Device &device, Window &window) : m_rDevice{device}, m_rWindow{window} {
        auto extent = m_rWindow.getExtent();
    }

    Renderer::~Renderer() = default;
}
