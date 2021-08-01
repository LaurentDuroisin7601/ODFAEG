#pragma once

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <memory>
#include <optional>

#include "odfaeg/Core/crtp.hpp"

namespace odfaeg {
    class Instance;

    struct QueueFamilyIndices final {
        std::optional<std::uint32_t> graphics_family;
        std::optional<std::uint32_t> present_family;
    };

    enum class QueueFamilyType {
        GRAPHICS,
        PRESENT,
    };

    class Device final : public NoCopy, public NoMove {
      public:
        explicit Device(std::shared_ptr<Instance> instance);
        ~Device() noexcept;

        [[nodiscard]] auto getLogicalDevice() const { return m_device; }
        [[nodiscard]] auto getPhysicalDEvice() const { return m_physical; }

        [[nodiscard]] const QueueFamilyIndices &getFamilyIndices() const { return m_indices; }

      private:
        [[nodiscard]] VkDevice createLogicalDevice();
        [[nodiscard]] VkPhysicalDevice pickPhysicalDevice() const;

      private:
        std::shared_ptr<Instance> m_instance;

        VkDevice m_device{nullptr};
        VkPhysicalDevice m_physical{nullptr};

        VkQueue m_graphics_queue{nullptr};
        VkQueue m_present_queue{nullptr};

        QueueFamilyIndices m_indices;
    };
}  // namespace odfaeg
