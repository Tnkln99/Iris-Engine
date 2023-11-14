#include "Renderer.hpp"

namespace iris::graphics{

    Renderer::Renderer(Device &device, Window &window) : m_rDevice{device}, m_rWindow{window} {
        auto extent = m_rWindow.getExtent();
        m_pSwapchain = std::make_unique<Swapchain>(device, extent);
    }

    Renderer::~Renderer() = default;
}
