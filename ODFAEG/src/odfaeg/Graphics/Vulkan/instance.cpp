#include "odfaeg/Graphics/Vulkan/instance.hpp"

#include <GLFW/glfw3.h>
#include <fmt/color.h>

#include <array>
#include <stdexcept>
#include <vector>

#include "odfaeg/Graphics/Vulkan/vulkan_window.hpp"
#include "odfaeg/config.hpp"

namespace odfaeg {
    namespace config {
#ifdef NDEBUG
        static constexpr bool enable_validation_layers = true;
        static constexpr std::array validation_layers = {"VK_LAYER_KHRONOS_validation"};
#else
        static constexpr bool enable_validation_layers = false;
#endif

        static std::vector<const char *> get_required_extensions() {
            std::uint32_t extensionCount = 0;
            const char **glfwExtensions = nullptr;

            glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);

            std::vector<const char *> extensions(glfwExtensions, glfwExtensions + extensionCount);
            if constexpr (enable_validation_layers) {
                extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }

            return extensions;
        }

        static VkResult create_debug_utils_messenger_ext(
            VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pMessenger) {
            if (auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")); func != nullptr) {
                return func(instance, pCreateInfo, pAllocator, pMessenger);
            } else {
                return VK_ERROR_EXTENSION_NOT_PRESENT;
            }
        }

        static bool checkValidationLayerSupport() {
            std::uint32_t layerCount = 0;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

            for (const char *layerName : config::validation_layers) {
                bool layerFound = [&]() {
                    for (const auto &layerProperties : availableLayers) {
                        if (std::strcmp(layerName, layerProperties.layerName) == 0) {
                            return true;
                        }
                    }

                    return false;
                }();

                if (!layerFound != true) {
                    return false;
                }
            }

            return true;
        }

        static void destroy_debug_utils_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks *pAllocator) {
            if (auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")); func != nullptr) {
                func(instance, debug_messenger, pAllocator);
            }
        }

        VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {
            if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
                fmt::print(fmt::fg(fmt::color::crimson) | fmt::emphasis::bold, "{}\n\n", pCallbackData->pMessage);
            }

            return VK_FALSE;
        }
    }  // namespace config

    Instance::Instance(const VulkanWindow &window, std::string_view application_name, std::uint32_t application_version) {
        if constexpr (config::enable_validation_layers) {
            if (!config::checkValidationLayerSupport()) {
                throw std::runtime_error("validation layers are enabled but not available!");
            }
        }

        VkApplicationInfo application_info{};
        application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

        application_info.applicationVersion = application_version;
        application_info.pApplicationName = application_name.data();

        application_info.engineVersion = VK_MAKE_VERSION(ODFAEG_VERSION_MAJOR, ODFAEG_VERSION_MINOR, 0);
        application_info.pEngineName = "OEDFAEG";

        application_info.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo instanceInfo{};
        instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

        instanceInfo.pApplicationInfo = &application_info;

        const auto extensions = config::get_required_extensions();
        instanceInfo.enabledExtensionCount = extensions.size();
        instanceInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
        if constexpr (config::enable_validation_layers) {
            instanceInfo.enabledLayerCount = config::validation_layers.size();
            instanceInfo.ppEnabledLayerNames = config::validation_layers.data();

            populateDebugMessenger(debugInfo);
            instanceInfo.pNext = &debugInfo;
        } else {
            instanceInfo.enabledLayerCount = 0;
            instanceInfo.ppEnabledLayerNames = nullptr;

            instanceInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&instanceInfo, nullptr, &m_instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        } else {
            if constexpr (config::enable_validation_layers) {
                m_debug_messenger = createDebugMessenger();
            }

            glfwCreateWindowSurface(m_instance, window.getWindow(), nullptr, &m_window_surface);
        }
    }

    Instance::~Instance() noexcept {
        if constexpr (config::enable_validation_layers) {
            config::destroy_debug_utils_messenger_ext(m_instance, m_debug_messenger, nullptr);
        }
        vkDestroyInstance(m_instance, nullptr);
    }

#ifdef NDEBUG
    VkDebugUtilsMessengerEXT Instance::createDebugMessenger() {
        VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
        populateDebugMessenger(debugInfo);

        VkDebugUtilsMessengerEXT debugMessenger = nullptr;
        if (config::create_debug_utils_messenger_ext(m_instance, &debugInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to create debug messenger!");
        } else {
            return debugMessenger;
        }
    }

    void Instance::populateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT &debugInfo) {
        debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        // debugInfo.pUserData = config::debugCallback;
    }
#endif
}  // namespace odfaeg
