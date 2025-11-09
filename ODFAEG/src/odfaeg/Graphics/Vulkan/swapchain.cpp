#include "odfaeg/Graphics/Vulkan/swapchain.hpp"

#include <limits>
#include <span>
#include <stdexcept>

#include "odfaeg/Graphics/Vulkan/device.hpp"
#include "odfaeg/Graphics/Vulkan/instance.hpp"
#include "odfaeg/Graphics/Vulkan/vulkan_window.hpp"

namespace odfaeg {
    namespace {
        [[nodiscard]] VkSurfaceFormatKHR choose_surface_format(std::span<const VkSurfaceFormatKHR> availableFormats) {
            for (const auto &availableFormat : availableFormats) {
                if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
                    return availableFormat;
                }
            }
            return *availableFormats.begin();
        }

        [[nodiscard]] VkPresentModeKHR choose_present_mode(std::span<const VkPresentModeKHR> availablePresentModes) {
            for (const auto &availablePresentMode : availablePresentModes) {
                if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                    return availablePresentMode;
                }
            }
            return VK_PRESENT_MODE_FIFO_KHR;
        }

        [[nodiscard]] VkExtent2D choose_extent(VkSurfaceCapabilitiesKHR capabilities, GLFWwindow *window) {
            if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max() ||
                capabilities.currentExtent.height != std::numeric_limits<std::uint32_t>::max()) {
                return capabilities.currentExtent;
            } else {
                int width = 0, height = 0;
                glfwGetFramebufferSize(window, &width, &height);

                VkExtent2D extent = {static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height)};
                extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, extent.width));
                extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, extent.height));

                return extent;
            }
        }
    }  // namespace

    Swapchain::Swapchain(std::shared_ptr<Instance> instance, std::shared_ptr<Device> device, const VulkanWindow &window)
        : m_instance(std::move(instance)), m_device(std::move(device)) {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->getPhysicalDEvice(), m_instance->getWindowSurface(), &m_capabilities);

        // query surface formats
        auto surfaceFormatCount = 0u;
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_device->getPhysicalDEvice(), m_instance->getWindowSurface(), &surfaceFormatCount, nullptr);

        m_formats.resize(surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_device->getPhysicalDEvice(), m_instance->getWindowSurface(), &surfaceFormatCount, m_formats.data());

        // query surface present modes
        auto surfacePresentModeCount = 0u;
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_device->getPhysicalDEvice(), m_instance->getWindowSurface(), &surfacePresentModeCount, nullptr);

        m_present_modes.resize(surfacePresentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_device->getPhysicalDEvice(), m_instance->getWindowSurface(), &surfacePresentModeCount, m_present_modes.data());

        m_swapchain = createSwapchain(window);
    }

    Swapchain::~Swapchain() noexcept { vkDestroySwapchainKHR(m_device->getLogicalDevice(), m_swapchain, nullptr); }

    [[nodiscard]] VkSwapchainKHR Swapchain::createSwapchain(const VulkanWindow &window) {
        auto surfaceFormat = choose_surface_format(m_formats);
        auto presentMode = choose_present_mode(m_present_modes);
        auto surfaceExtent = choose_extent(m_capabilities, window.getWindow());

        m_image_count = m_capabilities.minImageCount + 1;
        if (m_capabilities.maxImageCount > 0 && m_image_count > m_capabilities.maxImageCount) {
            m_image_count = m_capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR swapchainInfo{};
        swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

        swapchainInfo.surface = m_instance->getWindowSurface();

        swapchainInfo.minImageCount = m_image_count;
        swapchainInfo.imageFormat = surfaceFormat.format;
        swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
        swapchainInfo.imageExtent = surfaceExtent;
        swapchainInfo.imageArrayLayers = 1;
        swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        const std::array familyIndices = {m_device->getFamilyIndices().graphics_family.value(), m_device->getFamilyIndices().present_family.value()};
        if (m_device->getFamilyIndices().graphics_family != m_device->getFamilyIndices().present_family) {
            swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchainInfo.queueFamilyIndexCount = 1;
            swapchainInfo.pQueueFamilyIndices = familyIndices.data();
        } else {
            swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        swapchainInfo.preTransform = m_capabilities.currentTransform;
        swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainInfo.presentMode = presentMode;
        swapchainInfo.clipped = VK_TRUE;

        VkSwapchainKHR swapchain = nullptr;
        if (vkCreateSwapchainKHR(m_device->getLogicalDevice(), &swapchainInfo, nullptr, &swapchain) != VK_SUCCESS) {
            return swapchain;
        } else {
        	throw std::runtime_error("failed to create swapchain!");
        }
    }
}  // namespace odfaeg
