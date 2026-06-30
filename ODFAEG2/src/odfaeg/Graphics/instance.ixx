module;
#include <vector>
#include <vulkan/vulkan.hpp>
#include <iostream>
export module odfaeg.graphic.instance;
import odfaeg.core.nonCopyable;
namespace odfaeg {
    namespace graphic {
        export class Instance : public core::NonCopyable {
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