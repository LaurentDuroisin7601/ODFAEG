#pragma once

#include <vulkan/vulkan_core.h>

#include <memory>
#include <vector>

#include "odfaeg/Core/crtp.hpp"

namespace odfaeg {
    class Instance;
    class Device;

    class VulkanWindow;

    class Swapchain final : public NoCopy, public NoMove {
      public:
        explicit Swapchain(std::shared_ptr<Instance> instance, std::shared_ptr<Device> device, const VulkanWindow &window);
        ~Swapchain() noexcept;

      private:
        [[nodiscard]] VkSwapchainKHR createSwapchain(const VulkanWindow &window);

      private:
        std::shared_ptr<Instance> m_instance{nullptr};
        std::shared_ptr<Device> m_device{nullptr};

        VkSwapchainKHR m_swapchain{nullptr};
        VkSurfaceCapabilitiesKHR m_capabilities;

        std::vector<VkSurfaceFormatKHR> m_formats;
        std::vector<VkPresentModeKHR> m_present_modes;

        std::uint32_t m_image_count;
        std::vector<VkImage> m_images;
        std::vector<VkImageView> m_image_views;
    };
}  // namespace odfaeg
