#pragma once

#include <vulkan/vulkan_core.h>

#include <string_view>

namespace odfaeg {
    class VulkanWindow;

    class Instance final {
      public:
        explicit Instance(const VulkanWindow &window, std::string_view application_name, std::uint32_t application_version);
        ~Instance();

      private:
#ifdef NDEBUG
		[[nodiscard]] VkDebugUtilsMessengerEXT createDebugMessenger();
        void populateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT &debugInfo);
#endif

      private:
        VkInstance m_instance{nullptr};

#ifdef NDEBUG
        VkDebugUtilsMessengerEXT m_debug_messenger{nullptr};
#endif
    };
}  // namespace odfaeg
