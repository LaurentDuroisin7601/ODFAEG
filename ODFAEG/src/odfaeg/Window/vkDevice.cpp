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

            // Check if VK_KHR_maintenance4 is supported


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
                VkPhysicalDeviceFeatures2 physical_features2 = {};
                physical_features2.sType =  VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

                VkPhysicalDeviceVulkan11Features features11 = {};
                features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
                physical_features2.pNext = &features11;
                vkGetPhysicalDeviceFeatures2(device, &physical_features2);

                VkPhysicalDeviceVulkan12Features features12 = {};
                features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
                physical_features2.pNext = &features12;
                vkGetPhysicalDeviceFeatures2(device, &physical_features2);
                return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy && features11.shaderDrawParameters == VK_TRUE && features12.runtimeDescriptorArray == VK_TRUE;
            } else {
                QueueFamilyIndices  indices = findQueueFamilies(device, surface);
                VkPhysicalDeviceFeatures supportedFeatures;
                vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
                VkPhysicalDeviceFeatures2 physical_features2 = {};

                VkPhysicalDeviceVulkan11Features features11 = {};
                features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
                physical_features2.pNext = &features11;
                vkGetPhysicalDeviceFeatures2(device, &physical_features2);

                VkPhysicalDeviceVulkan12Features features12 = {};
                features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
                physical_features2.pNext = &features12;
                vkGetPhysicalDeviceFeatures2(device, &physical_features2);
                return indices.isGraphicComplete() && indices.isComputeComplete() && supportedFeatures.samplerAnisotropy && features11.shaderDrawParameters == VK_TRUE && features12.runtimeDescriptorArray == VK_TRUE;
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
                    //std::cout<<"found graphic : "<<i<<std::endl;
                    indices.graphicsFamily = i;
                }
                if (!(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
                    (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
                    //std::cout<<"found compute : "<<i<<std::endl;
                    indices.computeFamily = i;
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
                i++;
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



                VkPhysicalDeviceNestedCommandBufferFeaturesEXT maintenance7Features {
                    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_FEATURES_EXT,
                    .nestedCommandBuffer = VK_TRUE
                };

                VkPhysicalDeviceMaintenance4Features maintenance4Features {
                    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES,
                    .pNext = &maintenance7Features,
                    .maintenance4 = VK_TRUE
                };

                VkPhysicalDeviceSynchronization2Features synchronization2Features {
                    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR,
                    .pNext = &maintenance4Features,
                    .synchronization2 = VK_TRUE

                };

                VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT physcialDeviceFragmentShaderInterlock{};
                physcialDeviceFragmentShaderInterlock.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT;
                physcialDeviceFragmentShaderInterlock.pNext = &synchronization2Features;
                physcialDeviceFragmentShaderInterlock.fragmentShaderPixelInterlock = VK_TRUE;

                VkPhysicalDeviceFeatures deviceFeatures{};
                deviceFeatures.samplerAnisotropy = VK_TRUE;
                deviceFeatures.fragmentStoresAndAtomics = VK_TRUE;
                VkPhysicalDeviceVulkan11Features features11 = {};
                features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
                features11.pNext = &physcialDeviceFragmentShaderInterlock;
                VkPhysicalDeviceVulkan12Features features12 = {};
                features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
                features12.pNext = &features11;

                VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT restartFeatures = {};
                restartFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVE_TOPOLOGY_LIST_RESTART_FEATURES_EXT;
                restartFeatures.primitiveTopologyListRestart = VK_TRUE;
                restartFeatures.pNext = &features12;

                VkPhysicalDeviceFeatures2 physical_features21;
                physical_features21.sType =  VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
                physical_features21.pNext = &restartFeatures;
                physical_features21.features = deviceFeatures;
                vkGetPhysicalDeviceFeatures2(physicalDevice, &physical_features21);



                PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2KHR>(vkGetInstanceProcAddr(vkSettup.getInstance(), "vkGetPhysicalDeviceProperties2KHR"));
                if (!vkGetPhysicalDeviceProperties2KHR) {
                    throw core::Erreur(0, "Could not get a valid function pointer for vkGetPhysicalDeviceProperties2KHR", 1);
                }
                VkPhysicalDeviceProperties2KHR devicePropsEXT2{};
                pushDescriptorProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR;
                devicePropsEXT2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
                devicePropsEXT2.pNext = &pushDescriptorProps;
                vkGetPhysicalDeviceProperties2KHR(physicalDevice, &devicePropsEXT2);




                VkDeviceCreateInfo createInfo{};
                createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
                createInfo.pNext = &physical_features21;

                createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
                createInfo.pQueueCreateInfos = queueCreateInfos.data();

                createInfo.pEnabledFeatures = nullptr;

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
                vkGetDeviceQueue(device, indices.computeFamily.value(), 0, &computeQueue);
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
        VkQueue Device::getComputeQueue() {
            return computeQueue;
        }
        VkQueue Device::getPresentQueue() {
            return presentQueue;
        }
        Device::~Device() {
            //////std::cout<<"destory device"<<std::endl;
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
