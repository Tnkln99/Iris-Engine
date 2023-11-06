#include "Window.hpp"

#include <utility>
#include <iostream>

namespace iris::graphics{
    Window::Window(int width, int height, std::string windowName)
    : m_width{width}, m_height{height}, m_name{std::move(windowName)}
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        m_pWindow = glfwCreateWindow(m_width, m_height,
                                     m_name.c_str(), nullptr, nullptr);

        if(!m_pWindow){
            std::cout<< "failed to create window" << std::endl;
        }

        glfwSetWindowUserPointer(m_pWindow, this);
        glfwSetCursorPosCallback(m_pWindow, mouseCallback);
        glfwSetKeyCallback(m_pWindow, keyCallback);
    }

    bool Window::shouldCloseWindow() {
        return glfwWindowShouldClose(m_pWindow);
    }

    void Window::pollWindowEvents() {
        glfwPollEvents();
    }

    int Window::getWidth() const {
        return m_width;
    }

    int Window::getHeight() const {
        return m_height;
    }

    VkExtent2D Window::getExtent() {
        return {static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height) };
    }

    void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
        if (glfwCreateWindowSurface(instance, m_pWindow, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface");
        }
    }

    void Window::mouseCallback(GLFWwindow *window, double xPos, double yPos) {
        m_sMouseInfo.m_xPos = xPos;
        m_sMouseInfo.m_yPos = yPos;
    }

    void Window::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
        m_sKeyInfo.m_key = key;
        m_sKeyInfo.m_scancode = scancode;
        m_sKeyInfo.m_action = action;
        m_sKeyInfo.m_mods = mods;
    }
}