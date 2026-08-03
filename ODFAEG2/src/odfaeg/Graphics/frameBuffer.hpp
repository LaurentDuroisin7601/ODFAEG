#ifndef ODFAEG_FRAMEBUFFER_HPP
#define ODFAEG_FRAMEBUFFER_HPP
#include <vulkan/vulkan.hpp>
#include <vector>
#include "device.hpp"
#include "image.hpp"
#include "renderPass.hpp"
#include "../Core/nonCopyable.hpp"
namespace odfaeg {
	namespace graphic {
		class FrameBuffer : public core::NonCopyable {
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
#include "frameBuffer.inl"
#endif