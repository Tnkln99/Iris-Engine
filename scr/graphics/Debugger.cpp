#include "Debugger.hpp"

#include <iostream>
#include <sstream>
#include <cassert>

namespace iris::graphics{
    PFN_vkCreateDebugUtilsMessengerEXT Debugger::m_sVkCreateDebugUtilsMessengerEXT;
    PFN_vkDestroyDebugUtilsMessengerEXT Debugger::m_sVkDestroyDebugUtilsMessengerEXT;
    VkDebugUtilsMessengerEXT Debugger::m_sDebugUtilsMessenger;

    VKAPI_ATTR VkBool32 VKAPI_CALL Debugger::debugUtilsMessengerCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData)
    {
        // Select prefix depending on flags passed to the callback
        std::string prefix("");

        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
            prefix = "VERBOSE: ";
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
            prefix = "INFO: ";
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            prefix = "WARNING: ";
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            prefix = "ERROR: ";
        }


        // Display message to default output (console/logcat)
        std::stringstream debugMessage;
        debugMessage << prefix << std::endl
        << "[" << pCallbackData->messageIdNumber << "]["<< pCallbackData->pMessageIdName << "] : "
        << pCallbackData->pMessage << std::endl << std::endl;

#if defined(__ANDROID__)
        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
				LOGE("%s", debugMessage.str().c_str());
			} else {
				LOGD("%s", debugMessage.str().c_str());
			}
#else
        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            std::cerr << debugMessage.str() << "\n";
        } else {
            std::cout << debugMessage.str() << "\n";
        }
        fflush(stdout);
#endif


        // The return value of this callback controls whether the Vulkan call that caused the validation message will be aborted or not
        // We return VK_FALSE as we DON'T want Vulkan calls that cause a validation message to abort
        // If you instead want to have calls abort, pass in VK_TRUE and the function will return VK_ERROR_VALIDATION_FAILED_EXT
        return VK_FALSE;
    }

    void Debugger::setupDebugging(VkInstance instance)
    {

        Debugger::m_sVkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
        Debugger::m_sVkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

        VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCI{};
        debugUtilsMessengerCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugUtilsMessengerCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugUtilsMessengerCI.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        debugUtilsMessengerCI.pfnUserCallback = debugUtilsMessengerCallback;

        vkCheck(m_sVkCreateDebugUtilsMessengerEXT(instance, &debugUtilsMessengerCI, nullptr, &m_sDebugUtilsMessenger), "Failed to set up debug messenger!");
    }

    void Debugger::freeDebugCallback(VkInstance instance)
    {
        if (Debugger::m_sDebugUtilsMessenger != VK_NULL_HANDLE)
        {
            m_sVkDestroyDebugUtilsMessengerEXT(instance, Debugger::m_sDebugUtilsMessenger, nullptr);
        }
    }

    void Debugger::vkCheck(VkResult result, const char *message) {
        if(result != VK_SUCCESS){
            std::cout << message << std::endl;
            assert(0 && "Vulkan error");
        }
    }
}