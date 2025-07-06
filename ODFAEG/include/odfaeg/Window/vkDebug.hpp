#ifndef VK_DEBUG_HPP
#define VK_DEBUG_HPP
#include "../config.hpp"
#ifdef VULKAN
#include <vulkan/vulkan.hpp>
#include "export.hpp"
#include <vector>
namespace odfaeg {
    namespace window {
        #ifdef ODFAEG_DEBUG
        const bool enableValidationLayers = true;
        #else
        const bool enableValidationLayers = false;
        #endif
        const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };
        ODFAEG_WINDOW_API VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
        ODFAEG_WINDOW_API void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    }
}
#endif
#endif // VK_DEBUG_HPP
