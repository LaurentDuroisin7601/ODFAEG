module;
#include <iostream>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
import odfaeg.graphic.buffer;
module odfaeg.graphic.buffer;
import odfaeg.graphic.device;
namespace odfaeg {
	namespace graphic {
		Buffer::Buffer(Device& device) : allocator(device.getAllocator()) {	
			buffer = VK_NULL_HANDLE;
			range = 0;
			offset = 0;
		}
		Buffer::Buffer(Buffer&& other) noexcept {
			allocator = other.allocator;
			buffer = other.buffer;
			memory = other.memory;
			allocator = other.allocator;
			range = other.range;
			offset = other.offset;
			other.buffer = VK_NULL_HANDLE;
			other.memory = VK_NULL_HANDLE;
		}
		Buffer& Buffer::operator= (Buffer&& other) noexcept {
			if (this != &other) {
				cleanup();
				allocator = other.allocator;
				buffer = other.buffer;
				memory = other.memory;
				allocator = other.allocator;
				range = other.range;
				offset = other.offset;
				other.buffer = VK_NULL_HANDLE;
				other.memory = VK_NULL_HANDLE;
			}
			return *this;
		}
		void Buffer::create(VkDeviceSize size, VkBufferUsageFlags bufferUsageFlags, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags) {
			if (buffer != VK_NULL_HANDLE) {
				cleanup();
			}
			VkBufferCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			info.size = size;
			info.usage = bufferUsageFlags;
			VmaAllocationCreateInfo alloc{};
			alloc.usage = memoryUsage;
			alloc.flags = flags;
			vmaCreateBuffer(allocator, &info, &alloc, &buffer, &memory, nullptr);
			//std::cout<<"create buffer : "<<buffer<<std::endl;
			range = size;
		}
		void Buffer::update(const void* srcData, size_t srcDataSize, size_t dstStart) {
			if (buffer != VK_NULL_HANDLE && srcDataSize > 0) {
				void* data;
				vmaMapMemory(allocator, memory, &data);
				memcpy(static_cast<std::uint8_t*>(data) + dstStart, srcData, srcDataSize);
				vmaUnmapMemory(allocator, memory);
			}
		}
		void Buffer::swap(Buffer& b) {
			std::swap(buffer, b.buffer);
			std::swap(range, b.range);
			std::swap(offset, b.offset);
		}
		void Buffer::copyBuffer(Buffer& srcBuffer, Buffer& dstBuffer, VkDeviceSize size, VkCommandBuffer& cmd) {
			if (srcBuffer.getHandle() != VK_NULL_HANDLE && dstBuffer.getHandle() != VK_NULL_HANDLE && size > 0 && cmd != VK_NULL_HANDLE) {
				VkBufferCopy copyRegion{};
				copyRegion.size = size;
				vkCmdCopyBuffer(cmd, srcBuffer.getHandle(), dstBuffer.getHandle(), 1, &copyRegion);
			}
		}
		void Buffer::cleanup() {

			if (buffer != VK_NULL_HANDLE) {
				//std::cout<<"destroy buffer : "<<buffer<<std::endl;
				vmaDestroyBuffer(allocator, buffer, memory);
				buffer = VK_NULL_HANDLE;
			}
		}
		VkBuffer Buffer::getHandle(){
			return buffer;
		}
		size_t Buffer::getRange() {
			return range;
		}
		size_t Buffer::getOffset() {
			return offset;
		}
		Buffer::~Buffer() {

			cleanup();
		}
	}
}