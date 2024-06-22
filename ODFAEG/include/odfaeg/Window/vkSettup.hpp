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
namespace odfaeg {
    namespace window {
        class VkSettup {
        public :
            struct SwapChainSupportDetails {
                VkSurfaceCapabilitiesKHR capabilities;
                std::vector<VkSurfaceFormatKHR> formats;
                std::vector<VkPresentModeKHR> presentModes;
            };
            const std::vector<const char*> deviceExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
            };
            struct QueueFamilyIndices {
                std::optional<uint32_t> graphicsFamily, presentFamily;
                bool isComplete() {
                    return graphicsFamily.has_value() && presentFamily.has_value();
                }
            };


            VkSettup();
            void createInstance();
            void setupDebugMessenger();
            void pickupPhysicalDevice(VkSurfaceKHR surface);
            void createLogicalDevice(VkSurfaceKHR surface);
            bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
            VkDevice getDevice();
            VkPhysicalDevice getPhysicalDevice();
            QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
            VkQueue getGraphicQueue();
            VkQueue getPresentQueue();
            void setCommandPool(VkCommandPool commandPool);
            VkCommandPool getCommandPool();
            VkInstance getInstance();
            void cleanup();
            SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
            ~VkSettup();
            bool vulkanInitialized;
        private :
            static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback (VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
                if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
                    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
                }
                return VK_FALSE;
            }
            bool checkValidationLayerSupport();
            void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
            bool checkDeviceExtensionSupport (VkPhysicalDevice device);


            std::vector<const char*> getRequiredExtensions();
            VkInstance instance;
            VkDebugUtilsMessengerEXT debugMessenger;
            VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
            VkDevice device;
            VkQueue graphicsQueue, presentQueue;
            VkCommandPool commandPool;
        };
    }
}
#endif
#endif // VK_SETUP_HPP
