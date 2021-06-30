#ifndef VK_DEBUG_HPP
#define VK_DEBUG_HPP
#include "../config.hpp"
#ifdef VULKAN
#include <vulkan/vulkan.hpp>
namespace odfaeg {
    namespace window {
        #ifdef ODFAEG_DEBUG
        const bool enableValidationLayers = false;
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
#endif // VK_DEBUG_HPP
