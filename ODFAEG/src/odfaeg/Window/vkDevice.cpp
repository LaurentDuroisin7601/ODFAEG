#include "../../../include/odfaeg/Window/vkDevice.hpp"
#ifdef VULKAN
namespace odfaeg {
    namespace window {
        Device::Device (VkSettup& vkSettup) : vkSettup(vkSettup) {
            physicalDevice = VK_NULL_HANDLE;
            device = VK_NULL_HANDLE;
        }
        bool Device::checkDeviceExtensionSupport(VkPhysicalDevice device) {
            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

            std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

            for (const auto& extension : availableExtensions) {
                requiredExtensions.erase(extension.extensionName);
            }

            return requiredExtensions.empty();
        }
        void Device::pickupPhysicalDevice(VkSurfaceKHR surface) {
            if (physicalDevice == VK_NULL_HANDLE) {
                uint32_t deviceCount = 0;
                vkEnumeratePhysicalDevices(vkSettup.getInstance(), &deviceCount, nullptr);

                if (deviceCount == 0) {
                    throw core::Erreur(0, "failed to find GPUs with Vulkan support!", 1);
                }
                std::vector<VkPhysicalDevice> devices(deviceCount);
                vkEnumeratePhysicalDevices(vkSettup.getInstance(), &deviceCount, devices.data());
                for (const auto& device : devices) {
                    if (isDeviceSuitable(device, surface)) {
                        physicalDevice = device;
                        break;
                    }
                }

                if (physicalDevice == VK_NULL_HANDLE) {
                    throw core::Erreur(0, "failed to find a suitable GPU!", 1);
                }
            }
        }
        bool Device::isDeviceSuitable(VkPhysicalDevice device,  VkSurfaceKHR surface) {
            QueueFamilyIndices indices = findQueueFamilies(device, surface);
            if (surface != VK_NULL_HANDLE) {
                bool extensionsSupported = checkDeviceExtensionSupport(device);
                bool swapChainAdequate;
                if (extensionsSupported) {
                    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
                    swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
                } else {
                    swapChainAdequate = true;
                }
                VkPhysicalDeviceFeatures supportedFeatures;
                vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
                return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
            } else {
                QueueFamilyIndices  indices = findQueueFamilies(device, surface);
                VkPhysicalDeviceFeatures supportedFeatures;
                vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
                return indices.isGraphicComplete() && supportedFeatures.samplerAnisotropy;
            }
        }
        Device::SwapChainSupportDetails Device::querySwapChainSupport(VkPhysicalDevice device,  VkSurfaceKHR surface) {
            SwapChainSupportDetails details;

            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

            uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

            if (formatCount != 0) {
                details.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
            }

            uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

            if (presentModeCount != 0) {
                details.presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
            }

            return details;
        }
        Device::QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
            QueueFamilyIndices indices;
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

            int i = 0;
            for (const auto& queueFamily : queueFamilies) {
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    indices.graphicsFamily = i;
                    if (surface == VK_NULL_HANDLE) {
                        break;
                    }
                }
                if (surface != VK_NULL_HANDLE) {
                    VkBool32 presentSupport = false;
                    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

                    if (presentSupport) {
                        indices.presentFamily = i;
                    }

                    if (indices.isComplete()) {
                        break;
                    }
                }
            }
            return indices;
        }
         void Device::createLogicalDevice(VkSurfaceKHR surface) {
            if (device == VK_NULL_HANDLE) {
                std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
                std::set<uint32_t> uniqueQueueFamilies;
                 QueueFamilyIndices indices;
                if (surface != VK_NULL_HANDLE) {
                    indices = findQueueFamilies(physicalDevice, surface);
                    uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
                } else {
                    indices = findQueueFamilies(physicalDevice, surface);
                    uniqueQueueFamilies = {indices.graphicsFamily.value()};
                }
                float queuePriority = 1.0f;
                for (uint32_t queueFamily : uniqueQueueFamilies) {
                    VkDeviceQueueCreateInfo queueCreateInfo{};
                    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                    queueCreateInfo.queueFamilyIndex = queueFamily;
                    queueCreateInfo.queueCount = 1;
                    queueCreateInfo.pQueuePriorities = &queuePriority;
                    queueCreateInfos.push_back(queueCreateInfo);
                }
                VkPhysicalDeviceFeatures deviceFeatures{};
                deviceFeatures.samplerAnisotropy = VK_TRUE;

                VkDeviceCreateInfo createInfo{};
                createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

                createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
                createInfo.pQueueCreateInfos = queueCreateInfos.data();

                createInfo.pEnabledFeatures = &deviceFeatures;

                createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
                createInfo.ppEnabledExtensionNames = deviceExtensions.data();

                if (enableValidationLayers) {
                    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                    createInfo.ppEnabledLayerNames = validationLayers.data();
                } else {
                    createInfo.enabledLayerCount = 0;
                }

                if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to create logical device!", 1);
                }
                vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
                if (surface != VK_NULL_HANDLE) {
                    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
                }
            }
        }
        VkDevice Device::getDevice() {
            return device;
        }
        VkPhysicalDevice Device::getPhysicalDevice() {
            return physicalDevice;
        }
        VkQueue Device::getGraphicsQueue() {
            return graphicsQueue;
        }
        VkQueue Device::getPresentQueue() {
            return presentQueue;
        }
        void Device::setCommandPool(VkCommandPool commandPool) {
            this->commandPool = commandPool;
        }
        VkCommandPool Device::getCommandPool() {
            return commandPool;
        }
        Device::~Device() {
            vkDestroyDevice(device, nullptr);
        }
        void Device::createInstance() {
            vkSettup.createInstance();
        }
        VkInstance Device::getInstance() {
            return vkSettup.getInstance();
        }
    }
}
#endif
