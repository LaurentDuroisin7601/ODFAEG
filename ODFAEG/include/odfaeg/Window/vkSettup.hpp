#ifndef VK_SETUP_HPP
#define VK_SETUP_HPP
#include "vkDebug.hpp"
#include "../Core/erreur.h"

#ifdef VULKAN
#include <GLFW/glfw3.h>
#include <set>
#include <algorithm>
#include <iostream>
namespace odfaeg {
    namespace window {
        class VkSettup {
        public :
            const std::vector<const char*> deviceExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
            };
            struct QueueFamilyIndices {
                bool hasGfValue = false;
                bool hasPfValue = false;
                uint32_t graphicsFamily;
                uint32_t presentFamily;
                bool createSurface;
                bool isComplete() {
                    return hasGfValue && (!createSurface ||  createSurface && hasPfValue);
                }
            };

            struct SwapChainSupportDetails {
                VkSurfaceCapabilitiesKHR capabilities;
                std::vector<VkSurfaceFormatKHR> formats;
                std::vector<VkPresentModeKHR> presentModes;
            };
            VkSettup();
            void setCreateSurface (bool createSurface, GLFWwindow* window);
            void initVulkan();
            void recreateSwapchain();
            VkExtent2D getSwapchainExtends();
            VkDevice getDevice();
            VkFormat getSwapchainImageFormat();
            std::vector<VkImageView>& getSwapChainImageViews();
            VkPhysicalDevice getPhysicalDevice();
            QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
            void setCommandBuffers(std::vector<VkCommandBuffer> commandBuffers);
            void drawFrame();
            void cleanup();
            ~VkSettup();
        private :
            void cleanupSwapchain();
            static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback (VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
                if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
                    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
                }
                return VK_FALSE;
            }
            void createInstance();
            bool checkValidationLayerSupport();
            void setupDebugMessenger();
            void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
            void pickupPhysicalDevice();
            void createLogicalDevice();
            void createSurface();
            void createSwapChain();
            void createImageViews();
            void createSyncObjects();
            VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
            bool checkDeviceExtensionSupport (VkPhysicalDevice device);
            bool isDeviceSuitable(VkPhysicalDevice device);
            SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
            VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
            VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
            VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
            std::vector<const char*> getRequiredExtensions();
            VkInstance instance;
            VkDebugUtilsMessengerEXT debugMessenger;
            VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
            VkDevice device;
            VkQueue graphicsQueue, presentQueue;
            bool mustCreateSurface;
            VkSurfaceKHR surface;
            VkSwapchainKHR swapChain;

            std::vector<VkImage> swapChainImages;
            VkFormat swapChainImageFormat;
            VkExtent2D swapChainExtent;
            std::vector<VkImageView> swapChainImageViews;
            std::vector<VkCommandBuffer> commandBuffers;
            std::vector<VkSemaphore> imageAvailableSemaphores;
            std::vector<VkSemaphore> renderFinishedSemaphores;
            std::vector<VkFence> inFlightFences;
            std::vector<VkFence> imagesInFlight;
            const int MAX_FRAMES_IN_FLIGHT = 2;
            size_t currentFrame;
            GLFWwindow* window;
        };
    }
}
#endif
#endif // VK_SETUP_HPP
