module;
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
export module odfaeg.graphic.buffer;
import odfaeg.core.nonCopyable;
import odfaeg.graphic.device;
namespace odfaeg {
	namespace graphic {
		export class Buffer : public core::NonCopyable {
		public :
			Buffer(Device& device);	
			Buffer(Buffer&& other) noexcept;
			Buffer& operator= (Buffer&& other) noexcept;
			void create(VkDeviceSize size, VkBufferUsageFlags, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags=0);
			void update(const void* srcData, size_t srcDataSize, size_t dstStart = 0);
			void swap(Buffer& buffer);
			size_t getRange();
			size_t getOffset();
			void cleanup();
			VkBuffer getHandle();
			~Buffer();
			static void copyBuffer(Buffer& srcBuffer, Buffer& dstBuffer, VkDeviceSize size, VkCommandBuffer& cmd);
		private :			
			VmaAllocator allocator;
			VmaAllocation memory;
			VkBuffer buffer;
			size_t range;
			size_t offset;			
		};
		void swap(Buffer& a, Buffer& b) noexcept {
			a.swap(b);
		}
	}
}