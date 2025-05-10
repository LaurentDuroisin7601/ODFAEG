#pragma once

#include <GLFW/glfw3.h>

#include <memory>
#include <string>
#include <string_view>

#include "odfaeg/Math/vec2f.h"

namespace odfaeg {
    class VulkanWindow final {
      public:
        struct WindowSpec final {
            WindowSpec() = delete;
            explicit WindowSpec(const math::Vec2f &extent, std::string_view title) : m_extent(extent), m_title(title) {}

            math::Vec2f m_extent;
            std::string m_title;
        };

      public:
        explicit VulkanWindow(WindowSpec &&spec);
        ~VulkanWindow() noexcept;

        [[nodiscard]] auto getWindow() const noexcept { return m_window; }

      private:
        GLFWwindow *m_window{nullptr};
        WindowSpec m_spec;
    };
}  // namespace odfaeg
