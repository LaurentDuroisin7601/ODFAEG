#include "odfaeg/Graphics/Vulkan/device.hpp"

#include <array>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include "odfaeg/Graphics/Vulkan/instance.hpp"

namespace odfaeg {
    namespace config {
        static constexpr std::array device_extensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    }

    namespace {
        [[nodiscard]] QueueFamilyIndices find_queue_families(VkPhysicalDevice physical, VkSurfaceKHR surface) {
            QueueFamilyIndices indices;

            std::uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physical, &queueFamilyCount, nullptr);

            std::vector<VkQueueFamilyProperties> familyProperties(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(physical, &queueFamilyCount, familyProperties.data());

            int i = 0;
            for (const auto &familyProperty : familyProperties) {
                if (familyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    indices.graphics_family = i;
                }

                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(physical, i, surface, &presentSupport);

                if (presentSupport) {
                    indices.present_family = i;
                }

                ++i;
            }

            return indices;
        }

        [[nodiscard]] bool is_device_suitable(VkPhysicalDevice physical, VkSurfaceKHR surface) {
            auto indices = find_queue_families(physical, surface);
            return indices.graphics_family.has_value() && indices.present_family.has_value();
        }

        [[nodiscard]] bool check_device_extensions_support(VkPhysicalDevice physical) {
            std::uint32_t extensionCount = 0;
            vkEnumerateDeviceExtensionProperties(physical, nullptr, &extensionCount, nullptr);

            std::vector<VkExtensionProperties> extensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(physical, nullptr, &extensionCount, extensions.data());

            std::unordered_set<std::string> requiredExtensions{};

            for (const auto &extension : extensions) {
                requiredExtensions.erase(extension.extensionName);
            }

            return requiredExtensions.empty();
        }
    }  // namespace

    Device::Device(std::shared_ptr<Instance> instance) : m_instance(std::move(instance)) {
        m_physical = pickPhysicalDevice();
        m_device = createLogicalDevice();
		m_indices = find_queue_families(m_physical, m_instance->getWindowSurface());
    }

    Device::~Device() noexcept { vkDestroyDevice(m_device, nullptr); }

    [[nodiscard]] VkDevice Device::createLogicalDevice() {
        auto indices = find_queue_families(m_physical, m_instance->getWindowSurface());

        std::vector<VkDeviceQueueCreateInfo> queueInfos;
        std::set<std::uint32_t> uniqueQueueFamilies = {indices.graphics_family.value(), indices.present_family.value()};

        float queuePriority = 1.0f;
        for (std::uint32_t queueFamily : uniqueQueueFamilies) {
            queueInfos.emplace_back(VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0, queueFamily, 1, &queuePriority);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo deviceInfo{};
        deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        deviceInfo.queueCreateInfoCount = queueInfos.size();
        deviceInfo.pQueueCreateInfos = queueInfos.data();
        deviceInfo.pEnabledFeatures = &deviceFeatures;

        if (check_device_extensions_support(m_physical) != 0) {
            throw std::runtime_error("the physical device does not support every extensions needed for this application!");
        } else {
            deviceInfo.enabledExtensionCount = config::device_extensions.size();
            deviceInfo.ppEnabledExtensionNames = config::device_extensions.data();
        }

        deviceInfo.enabledLayerCount = 0;

        VkDevice device = nullptr;
        if (vkCreateDevice(m_physical, &deviceInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        } else {
            vkGetDeviceQueue(device, indices.graphics_family.value(), 0, &m_graphics_queue);
			vkGetDeviceQueue(device, indices.present_family.value(), 0, &m_present_queue);

            return device;
        }
    }

    [[nodiscard]] VkPhysicalDevice Device::pickPhysicalDevice() const {
        std::uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_instance->getInstance(), &deviceCount, nullptr);

        std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
        vkEnumeratePhysicalDevices(m_instance->getInstance(), &deviceCount, physicalDevices.data());

        VkPhysicalDevice chosenGpu = [&]() -> VkPhysicalDevice {
            for (const auto &gpu : physicalDevices) {
                if (is_device_suitable(gpu, m_instance->getWindowSurface())) {
                    return gpu;
                }
            }
            return nullptr;
        }();

        if (chosenGpu != nullptr) {
            return chosenGpu;
        } else {
            throw std::runtime_error("there is no gpu available that is suitable!");
        }
    }
}  // namespace odfaeg
