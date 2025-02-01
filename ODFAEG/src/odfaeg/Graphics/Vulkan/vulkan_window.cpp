#include "odfaeg/Graphics/Vulkan/vulkan_window.hpp"

#include <GLFW/glfw3.h>

#include <stdexcept>

namespace odfaeg {
    VulkanWindow::VulkanWindow(VulkanWindow::WindowSpec &&spec) : m_spec(std::move(spec)) {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, false);

        m_window = glfwCreateWindow(m_spec.m_extent.x, m_spec.m_extent.y, m_spec.m_title.c_str(), nullptr, nullptr);
        if (m_window == nullptr) {
			throw std::runtime_error("glfw window allocation failed");
        }
    }

    VulkanWindow::~VulkanWindow() noexcept { glfwDestroyWindow(m_window); }
}  // namespace odfaeg
