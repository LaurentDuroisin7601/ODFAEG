module;
#include <vulkan/vulkan.hpp>
export module odfaeg.graphic.frameBuffer;
import odfaeg.graphic.device;
import odfaeg.graphic.image;
import odfaeg.graphic.renderPass;
import odfaeg.core.nonCopyable;
namespace odfaeg {
	namespace graphic {
		export class FrameBuffer : public core::NonCopyable {
		public:
			FrameBuffer(Device& device);
			FrameBuffer(FrameBuffer&& other) noexcept;
			FrameBuffer& operator=(FrameBuffer&& other) noexcept;
			void create(RenderPass& renderPass, ImageView& imageViews, uint32_t width, uint32_t height);
			void create(RenderPass& renderPass, ImageView& imageViews, ImageView& depthbufferImageViews, unsigned int width, unsigned int height);
			void cleanup();
			VkFramebuffer getHandle();
			~FrameBuffer();
		private:
			VkFramebuffer frameBuffer;
			Device& device;
		};
	}
}