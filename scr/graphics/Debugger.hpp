#ifndef IRIS_DEBUGGER_HPP
#define IRIS_DEBUGGER_HPP
#include <iostream>
#include <vulkan/vulkan.h>

namespace iris::graphics {

    class Debugger{
    public:
        static PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
        static PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;
        static VkDebugUtilsMessengerEXT debugUtilsMessenger;

        // Load debug function pointers and set debug callback
        static void setupDebugging(VkInstance instance);
        // Clear debug callback
        static void freeDebugCallback(VkInstance instance);

        static VkBool32 debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                             VkDebugUtilsMessageTypeFlagsEXT messageType,
                                             const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                             void *pUserData);

        static void vkCheck(VkResult result, const char* message);
    };



}



#endif //IRIS_DEBUGGER_HPP
