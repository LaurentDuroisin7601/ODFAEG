#ifndef DEVICE_HPP
#define DEVICE_HPP
#include "vkSettup.hpp"
#ifdef VULKAN
namespace odfaeg {
    namespace window {

        class ODFAEG_WINDOW_API Device {
        public :
            struct SwapChainSupportDetails {
                VkSurfaceCapabilitiesKHR capabilities;
                std::vector<VkSurfaceFormatKHR> formats;
                std::vector<VkPresentModeKHR> presentModes;
            };
            const std::vector<const char*> deviceExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME, VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME, VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME, VK_KHR_MAINTENANCE1_EXTENSION_NAME, VK_KHR_MAINTENANCE_4_EXTENSION_NAME, VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME, VK_KHR_MULTIVIEW_EXTENSION_NAME, VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME
            };
            struct QueueFamilyIndices {
                std::optional<uint32_t> graphicsFamily, presentFamily;
                bool isComplete() {
                    return graphicsFamily.has_value() && presentFamily.has_value();
                }
                bool isGraphicComplete() {
                    return graphicsFamily.has_value();
                }
            };
            Device (VkSettup& vkSettup);
            QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
            bool checkDeviceExtensionSupport (VkPhysicalDevice device);
            bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
            SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
            void pickupPhysicalDevice(VkSurfaceKHR surface);
            void createLogicalDevice(VkSurfaceKHR surface);
            VkDevice getDevice();
            VkPhysicalDevice getPhysicalDevice();
            VkQueue getGraphicsQueue();
            VkQueue getPresentQueue();

            void createInstance();
            VkInstance getInstance();
            ~Device();

        private :
            VkPhysicalDevicePushDescriptorPropertiesKHR pushDescriptorProps{};
            VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT fragmentShaderInterlockProps;
            VkSettup& vkSettup;
            VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
            VkDevice device;
            VkQueue graphicsQueue, presentQueue;


        };
    }
}
#endif // VK_DEVICE_HPP
#endif
