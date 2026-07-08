module;
#include <windows.h>
#include <vulkan/vulkan.hpp>
#include <odfaeg/config.hpp>
#if defined(ODFAEG_SYSTEM_WINDOWS)
#include <vulkan/vulkan_win32.h>
#else if defined(ODFAEG_SYSTEM_LINUX)
#include <X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>
#endif
#include <set>
#include <algorithm>
#include <iostream>
//import odfaeg.graphic.instance;
module odfaeg.graphic.instance;
import odfaeg.graphic.debug;
namespace odfaeg {
    namespace graphic {
        Instance::Instance() {

            instance = VK_NULL_HANDLE;
            //std::cout << "CTOR Instance() : " <<instance<<std::endl;
        }
        void Instance::createInstance() {
            //std::cout<<"create instance is null ?"<<instance<<std::endl;
            if (instance == VK_NULL_HANDLE) {
                if (enableValidationLayers && !checkValidationLayerSupport()) {
                    throw std::runtime_error("validation layers requested, but not available!");
                }
                //std::cout<<"create instance"<<std::endl;
                VkApplicationInfo appInfo{};
                appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
                appInfo.pApplicationName = "ODFAEG Vulkan Application";
                appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
                appInfo.pEngineName = "ODFAEG";
                appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
                appInfo.apiVersion = VK_API_VERSION_1_3;
                VkInstanceCreateInfo createInfo{};
                createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
                createInfo.pApplicationInfo = &appInfo;
                auto extensions = getRequiredExtensions();
                createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
                createInfo.ppEnabledExtensionNames = extensions.data();

                VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
                //std::cout<<"enable validation"
                if (enableValidationLayers) {
                    //std::cout << "enable validation layers" << std::endl;                    
                    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                    createInfo.ppEnabledLayerNames = validationLayers.data();
                    const char* setting_debug_action[] = { "VK_DBG_LAYER_ACTION_LOG_MSG" };
                    const int32_t setting_printf_buffer_size = 1048576;
                    const VkBool32 setting_enable_message_limit = VK_FALSE;
                    const char* setting_debug_action_level[] = { "info" };
                    const VkLayerSettingEXT settings[] = {
                        {"VK_LAYER_KHRONOS_validation", "enable_message_limit", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &setting_enable_message_limit},
                        {"VK_LAYER_KHRONOS_validation", "debug_action", VK_LAYER_SETTING_TYPE_STRING_EXT, 1, &setting_debug_action},
                        {"VK_LAYER_KHRONOS_validation", "printf_buffer_size", VK_LAYER_SETTING_TYPE_UINT32_EXT, 1, &setting_printf_buffer_size},
                        {"VK_LAYER_KHRONOS_validation", "report_flags", VK_LAYER_SETTING_TYPE_STRING_EXT, 1, &setting_debug_action_level}
                    };

                    VkValidationFeatureEnableEXT enables[] = { VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT
                                                             };
                    /*VkValidationFeatureDisableEXT disabled[] = {
                        VK_VALIDATION_FEATURE_DISABLE_GPU_ASSISTED_EXT,
                        VK_VALIDATION_FEATURE_DISABLE_SHADER_VALIDATION_CACHE_EXT,
                        VK_VALIDATION_FEATURE_DISABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,
                        VK_VALIDATION_FEATURE_DISABLE_BEST_PRACTICES_EXT,
                        VK_VALIDATION_FEATURE_DISABLE_SYNCHRONIZATION_VALIDATION_EXT
                    };  */      
                    VkValidationFeaturesEXT features = {};
                    features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
                    features.enabledValidationFeatureCount = sizeof(enables) / sizeof(enables[0]);
                    features.pEnabledValidationFeatures = enables;
                    /*features.disabledValidationFeaturesCount = sizeof(disabled) / sizeof(disabled[0]);
                    features.pDisabledValidationFeatures = disabled;*/

                    const VkLayerSettingsCreateInfoEXT layer_settings_create_info = {
                    VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, &features,
                    static_cast<uint32_t>(std::size(settings)), settings };

                    populateDebugMessengerCreateInfo(debugCreateInfo);
                    features.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
                    createInfo.pNext = &layer_settings_create_info;                    
                    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
                        throw std::runtime_error("Failed to create vulkan instance");
                    }                    
                }
                else {
                    //std::cout<<"create instance"<<std::endl;
                    createInfo.enabledLayerCount = 0;
                    createInfo.pNext = nullptr;
                    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
                        throw std::runtime_error("Failed to create vulkan instance");
                    }
                }
                setupDebugMessenger();
            }
        }
        std::vector<const char*> Instance::getRequiredExtensions() {
            //std::cout<<"get extensions"<<std::endl;
            std::vector<const char*> extensions;
            if (enableValidationLayers) {
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                //extensions.push_back(VK_EXT_LAYER_SETTINGS_EXTENSION_NAME);
            }
            extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
            #if defined(ODFAEG_SYSTEM_WINDOWS)
                extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
            #else if defined(ODFAEG_SYSTEM_LINUX)
                //std::cout<<"add linux extension name"<<std::endl;
                extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
            #endif    
            return extensions;
        }
        bool Instance::checkValidationLayerSupport() {
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
        void Instance::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
            createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            createInfo.pfnUserCallback = debugCallback;
        }
        void Instance::setupDebugMessenger() {
            if (!enableValidationLayers) {
                return;
            }
            VkDebugUtilsMessengerCreateInfoEXT createInfo;
            populateDebugMessengerCreateInfo(createInfo);

            if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
                throw std::runtime_error("failed to set up debug messenger!");
            }
        }

        void Instance::cleanup() {

            if (instance != VK_NULL_HANDLE) {

                if (enableValidationLayers) {
                    DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
                }


                vkDestroyInstance(instance, nullptr);
            }
        }

        VkInstance Instance::getInstance() {
            return instance;
        }

        void Instance::setInstance(VkInstance instance) {
            this->instance = instance;
        }

        Instance::~Instance() {
            std::cout<<"destroy instance"<<std::endl;
            cleanup();
        }
    }
}
