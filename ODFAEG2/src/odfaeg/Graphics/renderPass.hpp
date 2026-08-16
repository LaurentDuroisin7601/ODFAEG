#ifndef ODFAEG_RENDERPASS_HPP
#define ODFAEG_RENDERPASS_HPP
#include <vulkan/vulkan.hpp>
#include "device.hpp"
#include "../Core/nonCopyable.hpp"
namespace odfaeg {
	namespace graphic {
		class RenderPass : public core::NonCopyable {
		public:
			RenderPass(Device& device);
			RenderPass(RenderPass&& other) noexcept;
			RenderPass& operator=(RenderPass&& other) noexcept;
			void create(VkFormat format);
            void create(VkFormat format, std::uint32_t viewMask);
			void create(VkFormat format, VkImageLayout layout, VkSampleCountFlagBits msaaSamples, bool resolvePass=false);
			void create(VkFormat format, VkImageLayout layout, std::uint32_t viewMask);
			void create(VkFormat format, VkFormat depthStencilFormat, VkImageLayout layout,VkSampleCountFlagBits mesaaSamples, bool resolvePass = false);
			void create(VkFormat format, VkFormat depthStencilFormat, std::uint32_t viewMask);
			void cleanup();
			VkRenderPass getHandle();
			~RenderPass();
		private:
			VkRenderPass renderPass;
			Device& device;
		};
	}
}
#endif