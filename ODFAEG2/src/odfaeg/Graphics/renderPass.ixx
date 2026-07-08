module;
#include <vulkan/vulkan.hpp>
export module odfaeg.graphic.renderPass;
import odfaeg.graphic.device;
import odfaeg.core.nonCopyable;
namespace odfaeg {
	namespace graphic {
		export class RenderPass : public core::NonCopyable {
		public:
			RenderPass(Device& device);
			RenderPass(RenderPass&& other) noexcept;
			RenderPass& operator=(RenderPass&& other) noexcept;
			void create(VkFormat format);
            void create(VkFormat format, std::uint32_t viewMask);
			void create(VkFormat format, VkImageLayout layout);
			void create(VkFormat format, VkImageLayout layout, std::uint32_t viewMask);
			void create(VkFormat format, VkFormat depthStencilFormat, VkImageLayout layout);
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