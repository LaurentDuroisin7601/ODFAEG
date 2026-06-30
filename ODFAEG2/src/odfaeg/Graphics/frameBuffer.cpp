module;
#include <vulkan/vulkan.hpp>
import odfaeg.graphic.frameBuffer;
module odfaeg.graphic.frameBuffer;
namespace odfaeg {
	namespace graphic {
		FrameBuffer::FrameBuffer(Device& device) : device(device) {
			frameBuffer = VK_NULL_HANDLE;
		}	
		FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept : device(other.device) {
			frameBuffer = other.frameBuffer;
			other.frameBuffer = VK_NULL_HANDLE;
		}
		FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) noexcept {
			if (this != &other) {
				frameBuffer = other.frameBuffer;
				other.frameBuffer = VK_NULL_HANDLE;
			}
			return *this;
		}
		void FrameBuffer::create(RenderPass& renderPass, ImageView& imageView, uint32_t width, uint32_t height) {
			if (frameBuffer != VK_NULL_HANDLE) {
				cleanup();
			}
			VkFramebufferCreateInfo framebufferInfo{};
			std::array<VkImageView, 1> attachments = {
				imageView.getHandle()
			};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass.getHandle();
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = width;
			framebufferInfo.height = height;
			framebufferInfo.layers = 1;
			if (vkCreateFramebuffer(device.getDevice(), &framebufferInfo, nullptr, &frameBuffer) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
		void FrameBuffer::create(RenderPass& renderPass, ImageView& imageView, ImageView& depthBufferImageView, unsigned int width, unsigned int height) {
			if (frameBuffer != VK_NULL_HANDLE) {
				cleanup();
			}
			VkFramebufferCreateInfo framebufferInfo{};
			std::array<VkImageView, 2> attachments = {
				imageView.getHandle(),
				depthBufferImageView.getHandle()
			};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass.getHandle();
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = width;
			framebufferInfo.height = height;
			framebufferInfo.layers = 1;
			if (vkCreateFramebuffer(device.getDevice(), &framebufferInfo, nullptr, &frameBuffer) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
		void FrameBuffer::cleanup() {
			if (frameBuffer != VK_NULL_HANDLE) {
				vkDestroyFramebuffer(device.getDevice(), frameBuffer, nullptr);
				frameBuffer = VK_NULL_HANDLE;
			}
		}
		VkFramebuffer FrameBuffer::getHandle() {
			return frameBuffer;
		}
		FrameBuffer::~FrameBuffer() {
			cleanup();
		}
	}
}