#pragma once

#include <vulkan/vulkan_core.h>

#include <string_view>

#include "odfaeg/Core/crtp.hpp"

namespace odfaeg {
    class VulkanWindow;

    class Instance final : public NoCopy, public NoMove {
      public:
        explicit Instance(const VulkanWindow &window, std::string_view application_name, std::uint32_t application_version);
        ~Instance() noexcept;

        [[nodiscard]] auto getInstance() const { return m_instance; }
        [[nodiscard]] auto getWindowSurface() const { return m_window_surface; }

      private:
#ifdef NDEBUG
        [[nodiscard]] VkDebugUtilsMessengerEXT createDebugMessenger();
        void populateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT &debugInfo);
#endif

      private:
        VkInstance m_instance{nullptr};
        VkSurfaceKHR m_window_surface{nullptr};

#ifdef NDEBUG
        VkDebugUtilsMessengerEXT m_debug_messenger{nullptr};
#endif
    };
}  // namespace odfaeg
