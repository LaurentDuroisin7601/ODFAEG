#ifndef ODFAEG_DEBUG_HPP
#define ODFAEG_DEBUG_HPP
#include <vulkan/vulkan.hpp>
#include <odfaeg/config.hpp>
namespace odfaeg {
    namespace graphic {
        #ifdef ODFAEG_DEBUG
            const bool enableValidationLayers = true;
        #else
            const bool enableValidationLayers = true;
        #endif
        const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };
        VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
        void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    }
}
#endif