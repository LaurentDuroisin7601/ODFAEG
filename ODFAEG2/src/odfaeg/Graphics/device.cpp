module;
#include <vulkan/vulkan.hpp>
#include <vector>
#include <set>
#include <vk_mem_alloc.h>
#include <iostream>
//import odfaeg.graphic.device;
module odfaeg.graphic.device;
import odfaeg.graphic.debug;
import odfaeg.graphic.instance;
namespace odfaeg {
    namespace graphic {
        const std::vector<const char*> Device::deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
            VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME            
        };
        const std::vector<const char*> Device::deviceMeshExtensions = {            
            VK_EXT_MESH_SHADER_EXTENSION_NAME
        };
        Device::Device(Instance& instance) : instance(instance) {
            //std::cout<<"instance in device: "<<instance.getInstance()<<std::endl;
            physicalDevice = VK_NULL_HANDLE;
            device = VK_NULL_HANDLE;
            meshSupported = false;
        }
        bool Device::areMeshShadersSupported() {
            return false/*meshSupported*/;
        }
        bool Device::checkDeviceMeshExtensionSupport(VkPhysicalDevice device) {
            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

            std::set<std::string> requiredExtensions(deviceMeshExtensions.begin(), deviceMeshExtensions.end());
            for (const auto& extension : availableExtensions) {
                requiredExtensions.erase(extension.extensionName);
            }

            return requiredExtensions.empty();
        }
        bool Device::checkDeviceExtensionSupport(VkPhysicalDevice device) {
            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

            // Check if VK_KHR_maintenance4 is supported
            /*std::cout << "Requested extensions:\n";
            for (auto& e : deviceExtensions)
                std::cout << "  " << e << "\n";

            std::cout << "Available extensions:\n";
            for (auto& e : availableExtensions)
                std::cout << "  " << e.extensionName << "\n";*/
            //std::cout<<"extensions count  : "<<extensionCount<<std::endl;
            std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
            //std::cout<<"extensions count  : "<<extensionCount<<std::endl;
            for (const auto& extension : availableExtensions) {
                requiredExtensions.erase(extension.extensionName);
            }

            return requiredExtensions.empty();
        }
        void Device::pickupPhysicalDevice(VkSurfaceKHR surface) {
            if (physicalDevice == VK_NULL_HANDLE) {
                uint32_t deviceCount = 0;
                vkEnumeratePhysicalDevices(instance.getInstance(), &deviceCount, nullptr);

                if (deviceCount == 0) {
                    throw std::runtime_error("failed to find GPUs with Vulkan support!");
                }
                std::vector<VkPhysicalDevice> devices(deviceCount);
                vkEnumeratePhysicalDevices(instance.getInstance(), &deviceCount, devices.data());
                //std::cout<<"device count : "<<deviceCount<<std::endl;
                for (const auto& device : devices) {
                    //std::cout<<"device count : "<<deviceCount<<std::endl;
                    physicalDevice = device;
                    VkPhysicalDeviceProperties props;
                    vkGetPhysicalDeviceProperties(physicalDevice, &props);
                    if (isDeviceSuitable(device, surface) && props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                        physicalDevice = device;
                        VkPhysicalDeviceProperties props;
                        vkGetPhysicalDeviceProperties(physicalDevice, &props);
                        //std::cout << "GPU: " << props.deviceName << std::endl;
                        //system("PAUSE");
                        break;
                    }
                }
                if (physicalDevice == VK_NULL_HANDLE) {
                    throw std::runtime_error("failed to find a suitable GPU!");
                }
            }
        }
        bool Device::isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
            QueueFamilyIndices indices = findQueueFamilies(device, surface);
            if (!indices.isComplete())
                return false;

            // --- 2. Vérifier les extensions ---
            if (!checkDeviceExtensionSupport(device))
                return false;
            if (checkDeviceMeshExtensionSupport(device))
                meshSupported = true;
            //std::cout<<"mesh supported ? "<<meshSupported<<std::endl;  
            // --- 3. Vérifier le swapchain si surface fournie ---
            if (surface != VK_NULL_HANDLE)
            {
                SwapChainSupportDetails swap = querySwapChainSupport(device, surface);
                if (swap.formats.empty() || swap.presentModes.empty())
                    return false;
            }

            // --- 4. Récupérer les features Vulkan 1.0 ---
            VkPhysicalDeviceFeatures features10{};
            vkGetPhysicalDeviceFeatures(device, &features10);

            // --- 5. Chaînage propre des features 1.1 et 1.2 ---
            VkPhysicalDeviceVulkan12Features features12{};
            features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;

            VkPhysicalDeviceVulkan11Features features11{};
            features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
            features11.pNext = &features12;

            VkPhysicalDeviceFeatures2 features2{};
            features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            features2.pNext = &features11;

            vkGetPhysicalDeviceFeatures2(device, &features2);

            // --- 6. Vérifier uniquement les features réellement supportées par GTX 1660 SUPER ---
            if (!features10.samplerAnisotropy)
                return false;

            if (!features11.shaderDrawParameters)
                return false;

            // runtimeDescriptorArray n'est PAS supporté sur GTX 16xx → ne pas l'exiger
            // descriptorIndexing est partiellement supporté → OK si tu ne demandes pas tout

            return true;
        }
        Device::SwapChainSupportDetails Device::querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
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
                        //std::cout<<"found present : "<<i<<std::endl;
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
            //std::cout<<"create logical device"<<std::endl;
            if (device == VK_NULL_HANDLE) {
                VkPhysicalDeviceMeshShaderPropertiesEXT meshProps = {};
                meshProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT;

                VkPhysicalDeviceProperties2 props = {};
                props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
                props.pNext = &meshProps;

                vkGetPhysicalDeviceProperties2(physicalDevice, &props);

                /*std::cout << "Max mesh output vertices: "
                        << meshProps.maxMeshOutputVertices << std::endl;

                std::cout << "Max mesh output primitives: "
                        << meshProps.maxMeshOutputPrimitives << std::endl;

                std::cout << "Max mesh workgroup size: "
                        << meshProps.maxMeshWorkGroupSize[0] << ", "
                        << meshProps.maxMeshWorkGroupSize[1] << ", "
                        << meshProps.maxMeshWorkGroupSize[2] << std::endl;*/
                //std::cout<<"create logical device"<<std::endl;
                std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
                std::set<uint32_t> uniqueQueueFamilies;
                QueueFamilyIndices indices;
                if (surface != VK_NULL_HANDLE) {
                    indices = findQueueFamilies(physicalDevice, surface);
                    uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.computeFamily.value(), indices.presentFamily.value() };
                }
                else {
                    indices = findQueueFamilies(physicalDevice, surface);
                    uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.computeFamily.value() };
                }
                uint32_t queueFamilyCount = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
                queues.resize(queueFamilyCount);

                std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
                vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
                std::vector<std::vector<float>> allPriorities;
                for (uint32_t queueFamily : uniqueQueueFamilies) {
                    uint32_t count = queueFamilies[queueFamily].queueCount;
                    allPriorities.emplace_back(count, 1.0f); // cr�e un tableau de N priorit�s
                    VkDeviceQueueCreateInfo queueCreateInfo{};
                    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                    queueCreateInfo.queueFamilyIndex = queueFamily;
                    queueCreateInfo.queueCount = queueFamilies[queueFamily].queueCount;
                    queueCreateInfo.pQueuePriorities = allPriorities.back().data();
                    queueCreateInfos.push_back(queueCreateInfo);
                }
                // 1) On prépare les structs pour QUERY (tout à 0)
                VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeature{};
                dynamicRenderingFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
                dynamicRenderingFeature.dynamicRendering = VK_TRUE;

                VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT restartFeatures{};
                restartFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVE_TOPOLOGY_LIST_RESTART_FEATURES_EXT;
                restartFeatures.pNext = &dynamicRenderingFeature;

                VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT interlockFeatures{};
                interlockFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT;
                interlockFeatures.pNext = &restartFeatures;

                VkPhysicalDeviceSynchronization2Features sync2Features{};
                sync2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
                sync2Features.pNext = &interlockFeatures;

                VkPhysicalDeviceMeshShaderFeaturesEXT meshFeatures{};
                meshFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
                meshFeatures.pNext = &sync2Features;

                VkPhysicalDeviceMaintenance4Features maintenance4Features{};
                maintenance4Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES;
                maintenance4Features.pNext = &meshFeatures;
                
                // ⚠️ PAS de nestedCommandBuffer ici : l’extension n’est PAS dans ta liste → on la vire

                VkPhysicalDeviceVulkan12Features features12{};
                features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
                features12.pNext = &maintenance4Features;

                VkPhysicalDeviceVulkan11Features features11{};
                features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
                features11.pNext = &features12;

                VkPhysicalDeviceFeatures2 features2{};
                features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
                features2.pNext = &features11;

                vkGetPhysicalDeviceFeatures2(physicalDevice, &features2);

                // Features 1.0
                VkPhysicalDeviceFeatures deviceFeatures{};
                deviceFeatures.drawIndirectFirstInstance = VK_TRUE;


                if (features2.features.samplerAnisotropy)
                    deviceFeatures.samplerAnisotropy = VK_TRUE;
                if (features2.features.fragmentStoresAndAtomics)
                    deviceFeatures.fragmentStoresAndAtomics = VK_TRUE;

                if (features2.features.vertexPipelineStoresAndAtomics)
                    deviceFeatures.vertexPipelineStoresAndAtomics = VK_TRUE;

                // Features 1.1 / 1.2
                if (features11.shaderDrawParameters)
                    features11.shaderDrawParameters = VK_TRUE;

                // Interlock
                if (interlockFeatures.fragmentShaderPixelInterlock)
                    interlockFeatures.fragmentShaderPixelInterlock = VK_TRUE;

                // Sync2
                if (sync2Features.synchronization2)
                    sync2Features.synchronization2 = VK_TRUE;

                // Restart
                if (restartFeatures.primitiveTopologyListRestart)
                    restartFeatures.primitiveTopologyListRestart = VK_TRUE;
                if (meshFeatures.meshShader && meshFeatures.taskShader) {
                    meshSupported = true;
                    meshFeatures.meshShader = VK_TRUE;
                    meshFeatures.taskShader = VK_TRUE; 
                    // On ne veut PAS utiliser le shading rate via mesh shader pour l’instant
                    meshFeatures.primitiveFragmentShadingRateMeshShader = VK_FALSE;                   
                }
                // Maintenance4
                if (maintenance4Features.maintenance4)
                    maintenance4Features.maintenance4 = VK_TRUE;               
                VkPhysicalDeviceFeatures2 enabledFeatures2 = features2; // copie
                enabledFeatures2.features = deviceFeatures; // on met nos flags 1.0

                VkDeviceCreateInfo createInfo{};
                createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
                createInfo.pNext = &enabledFeatures2;
                createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
                createInfo.pQueueCreateInfos = queueCreateInfos.data();
                createInfo.pEnabledFeatures = nullptr; // on utilise Features2
                std::vector<const char*> allExtensions = deviceExtensions; 
                if (meshSupported) {
                    //std::cout<<"mesh supported!"<<std::endl;
                                       
                    allExtensions.insert(allExtensions.end(), deviceMeshExtensions.begin(), deviceMeshExtensions.end());
                    createInfo.enabledExtensionCount = static_cast<uint32_t>(allExtensions.size());                    
                    createInfo.ppEnabledExtensionNames = allExtensions.data();
                }
                else {
                    //std::cout<<"mesh not supported!"<<std::endl;
                    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
                    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
                }
                //Device VL deprecated features
                createInfo.enabledLayerCount = 0;
                createInfo.ppEnabledLayerNames = nullptr;                

                if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create logical device!");
                }
                float queueProperty = 1.0f;

                for (uint32_t queueFamily : uniqueQueueFamilies) {
                    queues[queueFamily].resize(queueFamilies[queueFamily].queueCount);
                    for (unsigned int i = 0; i < queues[queueFamily].size(); i++) {
                        vkGetDeviceQueue(device, queueFamily, i, &queues[queueFamily][i]);
                    }
                }
                createAllocator();
            }
        }
        VkDevice Device::getDevice() {
            return device;
        }
        VkPhysicalDevice Device::getPhysicalDevice() {
            return physicalDevice;
        }
        VkQueue Device::getQueue(uint32_t queueFamily, unsigned int queueIndex) {
            return queues[queueFamily][queueIndex];
        }
        void Device::cleanup() {
            if (device != VK_NULL_HANDLE) {

                vkDeviceWaitIdle(device);
                //std::cout<<"destory vma"<<std::endl;
                vmaDestroyAllocator(allocator);
                //std::cout<<"destroy device"<<std::endl;
                vkDestroyDevice(device, nullptr);
            }
        }
        Device::~Device() {
            cleanup();
        }
        void Device::createInstance() {
            instance.createInstance();
        }
        Instance& Device::getInstance() {
            return instance;
        }
        void Device::createAllocator() {
            VmaAllocatorCreateInfo allocatorInfo{};
            allocatorInfo.physicalDevice = physicalDevice;
            allocatorInfo.device = device;
            allocatorInfo.instance = instance.getInstance();
            vmaCreateAllocator(&allocatorInfo, &allocator);
        }
        VmaAllocator Device::getAllocator() {
            return allocator;
        }
    }
}