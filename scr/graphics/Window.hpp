#ifndef IRIS_WINDOW_HPP
#define IRIS_WINDOW_HPP

#include <string>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace iris::graphics{
    class Window {
    public:
        Window(int width, int height, std::string  windowName);

        bool shouldCloseWindow();
        void pollWindowEvents();

        int getWidth() const;
        int getHeight() const;
        VkExtent2D getExtent();

        void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

        inline static struct MouseInfo{
            double m_xPos;
            double m_yPos;
        } m_sMouseInfo{};
        inline static struct KeyInfo{
            int m_key;
            int m_scancode;
            int m_action;
            int m_mods;
        } m_sKeyInfo{};
    private:
        GLFWwindow * m_pWindow;

        int m_width;
        int m_height;
        std::string m_name;

        static void mouseCallback(GLFWwindow* window, double xPos, double yPos);
        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    };
}


#endif //IRIS_WINDOW_HPP
