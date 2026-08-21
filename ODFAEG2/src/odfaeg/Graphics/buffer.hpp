#ifndef ODFAEG_BUFFER_HPP
#define ODFAEG_BUFFER_HPP
#include <iostream>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include "../Core/nonCopyable.hpp"
#include "device.hpp"
namespace odfaeg {
	namespace graphic {
		class Buffer : public core::NonCopyable {
		public :
			Buffer(Device& device);	
			Buffer(Buffer&& other) noexcept;
			Buffer& operator= (Buffer&& other) noexcept;
			void create(VkDeviceSize size, VkBufferUsageFlags, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags=0, bool dedicatedMemory = false);
			void update(const void* srcData, size_t srcDataSize, size_t dstStart = 0);
			void swap(Buffer& buffer);
			size_t getRange();
			void setRange(size_t range);
			size_t getOffset();
			void cleanup();
			VkBuffer getHandle();
			uint64_t getDeviceAddress();
			~Buffer();
			static void copyBuffer(Buffer& srcBuffer, Buffer& dstBuffer, VkDeviceSize size, VkCommandBuffer& cmd);
		private :
			uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);			
			VmaAllocator allocator;
			VmaAllocation memory;
			VkBuffer buffer;
			VkDeviceMemory deviceMemory;
			size_t range;
			size_t offset;
			uint64_t deviceAddress;
			bool dedicatedMemory;
			Device& device;			
		};
		void swap(Buffer& a, Buffer& b) noexcept;
	}
}
#endif