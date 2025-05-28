#include "../../../include/odfaeg/Window/vkSettup.hpp"
#ifdef VULKAN
namespace odfaeg {
    namespace window {
        VkSettup::VkSettup()  {
            instance = VK_NULL_HANDLE;
        }

        void VkSettup::createInstance() {
            if (instance == VK_NULL_HANDLE) {
                if (enableValidationLayers && !checkValidationLayerSupport()) {
                    throw core::Erreur(0, "validation layers requested, but not available!", 1);
                }
                VkApplicationInfo appInfo{};
                appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
                appInfo.pApplicationName = "ODFAEG Vulkan Application";
                appInfo.applicationVersion = VK_MAKE_VERSION (1, 0, 0);
                appInfo.pEngineName = "ODFAEG";
                appInfo.engineVersion = VK_MAKE_VERSION (1, 0, 0);
                appInfo.apiVersion = VK_API_VERSION_1_3;
                VkInstanceCreateInfo createInfo{};
                createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
                createInfo.pApplicationInfo = &appInfo;
                auto extensions = getRequiredExtensions();
                createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
                createInfo.ppEnabledExtensionNames = extensions.data();

                VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
                if (enableValidationLayers) {
                    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                    createInfo.ppEnabledLayerNames = validationLayers.data();
                    const char* setting_debug_action[] = {"VK_DBG_LAYER_ACTION_LOG_MSG"};
                    const int32_t setting_printf_buffer_size = 1048576;
                    const VkBool32 setting_enable_message_limit = VK_FALSE;
                    const VkLayerSettingEXT settings[] = {
                        {"VK_LAYER_KHRONOS_validation", "enable_message_limit", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &setting_enable_message_limit},
                        {"VK_LAYER_KHRONOS_validation", "debug_action", VK_LAYER_SETTING_TYPE_STRING_EXT, 1, &setting_debug_action},
                        {"VK_LAYER_KHRONOS_validation", "printf_buffer_size", VK_LAYER_SETTING_TYPE_UINT32_EXT, 1, &setting_printf_buffer_size}
                    };

                    VkValidationFeatureEnableEXT enables[] = {VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT};
                    VkValidationFeaturesEXT features = {};
                    features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
                    features.enabledValidationFeatureCount = 1;
                    features.pEnabledValidationFeatures = enables;

                    const VkLayerSettingsCreateInfoEXT layer_settings_create_info = {
                    VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, &features,
                    static_cast<uint32_t>(std::size(settings)), settings};

                    populateDebugMessengerCreateInfo(debugCreateInfo);
                    features.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
                    createInfo.pNext = &layer_settings_create_info;
                }
                else {
                    createInfo.enabledLayerCount = 0;

                    createInfo.pNext = nullptr;
                }
                if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
                    throw core::Erreur(0, "Failed to create vulkan instance", 1);
                }
                setupDebugMessenger();
            }
        }
        std::vector<const char*> VkSettup::getRequiredExtensions() {
            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions;
            glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

            if (enableValidationLayers) {
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }
            extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);


            return extensions;
        }
        bool VkSettup::checkValidationLayerSupport() {
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

            for (const char* layerName : validationLayers) {
                bool layerFound = false;

                for (const auto& layerProperties : availableLayers) {
                    if (strcmp(layerName, layerProperties.layerName) == 0) {
                        layerFound = true;
                        break;
                    }
                }

                if (!layerFound) {
                    return false;
                }
            }

            return true;
        }
        void VkSettup::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
            createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            createInfo.pfnUserCallback = debugCallback;
        }
        void VkSettup::setupDebugMessenger () {
            if (!enableValidationLayers) {
                return;
            }
            VkDebugUtilsMessengerCreateInfoEXT createInfo;
            populateDebugMessengerCreateInfo(createInfo);

            if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to set up debug messenger!", 1);
            }
        }

        void VkSettup::cleanup() {



            if (enableValidationLayers) {
                DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
            }


            vkDestroyInstance(instance, nullptr);
        }

        VkInstance VkSettup::getInstance() {
            return instance;
        }
        VkSettup::~VkSettup() {
            cleanup();
        }
    }
}
#endif
