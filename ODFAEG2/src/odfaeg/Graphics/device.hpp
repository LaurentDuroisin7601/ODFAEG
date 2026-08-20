#ifndef ODFAEG_DEVICE_HPP
#define ODFAEG_DEVICE_HPP
#include <vulkan/vulkan.hpp>
#include <optional>
#include <vector>
#include <set>
#include <vk_mem_alloc.h>
#include <iostream>
#include "instance.hpp"
#include "../Core/nonCopyable.hpp"
#include "debug.hpp"
namespace odfaeg {
    namespace graphic {

        class Device : public core::NonCopyable {
        public:
            struct SwapChainSupportDetails {
                VkSurfaceCapabilitiesKHR capabilities;
                std::vector<VkSurfaceFormatKHR> formats;
                std::vector<VkPresentModeKHR> presentModes;
            };
            static const std::vector<const char*> deviceExtensions;
            static const std::vector<const char*> deviceMeshExtensions;
            static const std::vector<const char*> deviceRTExtensions;
            struct QueueFamilyIndices {
                std::optional<uint32_t> graphicsFamily, computeFamily, presentFamily;
                bool isComplete() {
                    return graphicsFamily.has_value() && computeFamily.has_value() && presentFamily.has_value();
                }
                bool isGraphicComplete() {
                    return graphicsFamily.has_value();
                }
                bool isComputeComplete() {
                    return computeFamily.has_value();
                }
            };
            Device(Instance& Instance);
            QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface=VK_NULL_HANDLE);
            bool checkDeviceExtensionSupport(VkPhysicalDevice device);
            bool checkDeviceMeshExtensionSupport(VkPhysicalDevice device);
            bool checkDeviceRTExtensionSupport(VkPhysicalDevice device);
            bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
            SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
            void pickupPhysicalDevice(VkSurfaceKHR surface=VK_NULL_HANDLE);
            void createLogicalDevice(VkSurfaceKHR surface=VK_NULL_HANDLE);
            void createAllocator();
            VmaAllocator getAllocator();
            VkDevice getDevice();
            VkPhysicalDevice getPhysicalDevice();
            VkQueue getQueue(uint32_t queueFamily, unsigned int queueIndex);


            void createInstance();
            Instance& getInstance();
            void cleanup();
            bool areMeshShadersSupported();
            VkSampleCountFlagBits getMaxUsableSampleCount();
            VkSampleCountFlagBits getMsaaSamples();
            ~Device();

        private:
            VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
            VkPhysicalDevicePushDescriptorPropertiesKHR pushDescriptorProps{};
            VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT fragmentShaderInterlockProps;
            Instance& instance;
            VmaAllocator allocator;
            VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
            VkDevice device;
            std::vector<std::vector<VkQueue>> queues;
            bool meshSupported, rtSupported;
        };
    }
}
#endif