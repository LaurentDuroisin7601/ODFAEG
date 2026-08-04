#ifndef ODFAEG_INSTANCE_HPP
#define ODFAEG_INSTANCE_HPP
#include <vulkan/vulkan.hpp>
#include <odfaeg/config.hpp>
#if defined(ODFAEG_SYSTEM_WINDOWS)
#include <windows.h>
#include <vulkan/vulkan_win32.h>
#else if defined(ODFAEG_SYSTEM_LINUX)
#include <X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>
#endif
#include <set>
#include <algorithm>
#include <iostream>
#include <vector>
#include <iostream>
#include "../Core/nonCopyable.hpp"
#include "debug.hpp"
namespace odfaeg {
    namespace graphic {
        class Instance : public core::NonCopyable {
        public:




            Instance();
            void createInstance();
            void setupDebugMessenger();
            VkInstance getInstance();
            void setInstance(VkInstance instance);
            void cleanup();


            ~Instance();
        private:
            Instance(const Instance&) = delete;
            Instance& operator=(const Instance&) = delete;
            Instance(Instance&& instance) = delete;
            Instance& operator= (Instance&& instance) = delete;
            static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
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

        };
    }
}
#endif