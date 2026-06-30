module;
#include <vulkan/vulkan.hpp>
#include <optional>
#include <vector>
#include <vk_mem_alloc.h>
export module odfaeg.graphic.device;
import odfaeg.graphic.instance;
import odfaeg.core.nonCopyable;
namespace odfaeg {
    namespace graphic {

        export class Device : public core::NonCopyable {
        public:
            struct SwapChainSupportDetails {
                VkSurfaceCapabilitiesKHR capabilities;
                std::vector<VkSurfaceFormatKHR> formats;
                std::vector<VkPresentModeKHR> presentModes;
            };
            static const std::vector<const char*> deviceExtensions;
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
            ~Device();

        private:
            VkPhysicalDevicePushDescriptorPropertiesKHR pushDescriptorProps{};
            VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT fragmentShaderInterlockProps;
            Instance& instance;
            VmaAllocator allocator;
            VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
            VkDevice device;
            std::vector<std::vector<VkQueue>> queues;
        };
    }
}