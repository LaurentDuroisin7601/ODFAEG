#ifndef VK_SETUP_HPP
#define VK_SETUP_HPP
#include "vkDebug.hpp"
#include "../Core/erreur.h"
#ifdef VULKAN
#include <GLFW/glfw3.h>
#include <set>
#include <algorithm>
#include <iostream>
#include <optional>
#include "export.hpp"
namespace odfaeg {
    namespace window {
        class ODFAEG_WINDOW_API VkSettup {
        public :




            VkSettup();
            void createInstance();
            void setupDebugMessenger();
            VkInstance getInstance();
            void cleanup();

            ~VkSettup();
        private :
            static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback (VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
                if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
                    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
                }
                return VK_FALSE;
            }
            bool checkValidationLayerSupport();
            void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);



            std::vector<const char*> getRequiredExtensions();
            VkInstance instance;
            VkDebugUtilsMessengerEXT debugMessenger;
            VkCommandPool commandPool;
            VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
            VkDevice device;
            VkQueue graphicsQueue;
        };
    }
}
#endif
#endif // VK_SETUP_HPP
