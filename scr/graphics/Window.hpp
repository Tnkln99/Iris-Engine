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
            double xPos;
            double yPos;
        } m_sMouseInfo{};
        inline static struct KeyInfo{
            int key;
            int scancode;
            int action;
            int mods;
        } m_sKeyInfo{};
    private:
        GLFWwindow * m_pWindow;

        int m_Width;
        int m_Height;
        std::string m_Name;

        static void mouseCallback(GLFWwindow* window, double xPos, double yPos);
        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    };
}


#endif //IRIS_WINDOW_HPP
