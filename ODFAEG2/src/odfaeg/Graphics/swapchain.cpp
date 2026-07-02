module;
#include <vulkan/vulkan.hpp>
#include <iostream>
import odfaeg.graphic.swapchain;
module odfaeg.graphic.swapchain;
namespace odfaeg {
	namespace graphic {
		Swapchain::Swapchain(Device& device) : device(device) {
            swapchain = VK_NULL_HANDLE;
		}
        Swapchain::Swapchain(Swapchain&& other) noexcept : device(other.device) {
            swapchain = other.swapchain;
            other.swapchain = VK_NULL_HANDLE;
        }
        Swapchain& Swapchain::operator=(Swapchain&& other) noexcept {
            if (this != &other) {
                swapchain = other.swapchain;
                other.swapchain = VK_NULL_HANDLE;
            }
            return *this;
        }
        void Swapchain::create(VkSurfaceKHR surface, bool verticalSynch, int width, int height) {
            if (swapchain != VK_NULL_HANDLE) {
                cleanup();
            }
            Device::SwapChainSupportDetails swapchainSupport = device.querySwapChainSupport(device.getPhysicalDevice(), surface);
            VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats);
            VkPresentModeKHR presentMode = chooseSwapPresentMode(swapchainSupport.presentModes, verticalSynch);
            VkExtent2D extent = chooseSwapExtent(swapchainSupport.capabilities, width, height);
            uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
            minImagesCount = imageCount;
            if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount) {
                imageCount = swapchainSupport.capabilities.maxImageCount;
            }

            VkSwapchainCreateInfoKHR createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            createInfo.surface = surface;

            createInfo.minImageCount = imageCount;
            createInfo.imageFormat = surfaceFormat.format;
            createInfo.imageColorSpace = surfaceFormat.colorSpace;
            createInfo.imageExtent = extent;
            createInfo.imageArrayLayers = 1;
            createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

            Device::QueueFamilyIndices indices = device.findQueueFamilies(device.getPhysicalDevice(), surface);
            uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

            if (indices.graphicsFamily.value() != indices.presentFamily.value()) {
                createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                createInfo.queueFamilyIndexCount = 2;
                createInfo.pQueueFamilyIndices = queueFamilyIndices;
            }
            else {
                createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            }

            createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
            createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            createInfo.presentMode = presentMode;
            createInfo.clipped = VK_TRUE;

            if (vkCreateSwapchainKHR(device.getDevice(), &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
                throw std::runtime_error("failed to create swap chain!");
            }
            vkGetSwapchainImagesKHR(device.getDevice(), swapchain, &imageCount, nullptr);
            std::vector<VkImage> vkSwapchainImages;
            vkSwapchainImages.resize(imageCount);

            swapchainImages.reserve(imageCount);
            vkGetSwapchainImagesKHR(device.getDevice(), swapchain, &imageCount, vkSwapchainImages.data());            
            for (unsigned int i = 0; i < imageCount; i++) {                
                swapchainImages.emplace_back(device);
                swapchainImages[i].setHandle(vkSwapchainImages[i]);
            }
            swapchainImageFormat = surfaceFormat.format;
            swapchainExtents = extent;
        }
        VkSurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
            for (const auto& availableFormat : availableFormats) {
                if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    return availableFormat;
                }
            }

            return availableFormats[0];
        }
        VkPresentModeKHR Swapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, bool verticalSynch) {
            for (const auto& availablePresentMode : availablePresentModes) {
                if (!verticalSynch && availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                    return availablePresentMode;
                }
            }

            return VK_PRESENT_MODE_FIFO_KHR;
        }
        VkExtent2D Swapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, int width, int height) {
            if (capabilities.currentExtent.width != UINT32_MAX) {
                return capabilities.currentExtent;
            }
            else {                

                VkExtent2D actualExtent = {
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height)
                };

                actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
                actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

                return actualExtent;
            }
        }
        void Swapchain::createImageViews() {            

            for (uint32_t i = 0; i < swapchainImages.size(); i++) {
                swapchainImages[i].createImageView(VK_IMAGE_VIEW_TYPE_2D, swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1, 1);
            }
        }
        VkFormat& Swapchain::getSwapchainImageFormat() {
            return swapchainImageFormat;
        }
        std::vector<Image>& Swapchain::getSwapchainImages() {
            return swapchainImages;
        }        
        VkExtent2D Swapchain::getSwapchainExtents() {
            return swapchainExtents;
        }
        VkSwapchainKHR Swapchain::getHandle() {
            return swapchain;
        }
        void Swapchain::cleanup() {
            if (swapchain != VK_NULL_HANDLE) {
                for (unsigned int i = 0; i < swapchainImages.size(); i++) {
                    swapchainImages[i].cleanup();
                }
                swapchainImages.clear();
                vkDestroySwapchainKHR(device.getDevice(), swapchain, nullptr);
                swapchain = VK_NULL_HANDLE;
            }
        }
        uint32_t Swapchain::getMinImagesCount() {
            return minImagesCount;
        }
        Swapchain::~Swapchain() {
            cleanup();
        }
	}
}