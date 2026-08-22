namespace odfaeg {
	namespace graphic {
		Buffer::Buffer(Device& device) : allocator(device.getAllocator()), device(device) {	
			buffer = VK_NULL_HANDLE;
			range = 0;
			offset = 0;
			deviceAddress = 0;
			deviceMemory = VK_NULL_HANDLE;
			vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(device.getDevice(), "vkGetBufferDeviceAddressKHR"));
		}
		Buffer::Buffer(Buffer&& other) noexcept : device(other.device) {
			allocator = other.allocator;
			buffer = other.buffer;
			memory = other.memory;
			allocator = other.allocator;
			range = other.range;
			offset = other.offset;
			other.buffer = VK_NULL_HANDLE;
			other.memory = VK_NULL_HANDLE;
			vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(device.getDevice(), "vkGetBufferDeviceAddressKHR"));
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
		void Buffer::create(VkDeviceSize size, VkBufferUsageFlags bufferUsageFlags, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags, bool dedicatedMemory) {
			this->dedicatedMemory = dedicatedMemory;
			if (buffer != VK_NULL_HANDLE) {
				cleanup();
			}
			if (!dedicatedMemory) {
				VkBufferCreateInfo info{};
				info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				info.size = size;
				info.usage = bufferUsageFlags;
				VmaAllocationCreateInfo alloc{};
				alloc.usage = memoryUsage;
				alloc.flags = flags;
				VkMemoryAllocateFlagsInfo flagsInfo{};
				flagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
				flagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
				vmaCreateBuffer(allocator, &info, &alloc, &buffer, &memory, nullptr);
			} else {
				VkBufferCreateInfo info{};
				info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				info.size = size;
				bufferUsageFlags |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
				info.usage = bufferUsageFlags;
				if(vkCreateBuffer(device.getDevice(), &info, nullptr, &buffer) != VK_SUCCESS) {
					throw std::runtime_error("failed to create dedicated buffer!");
				}
				VkMemoryRequirements memoryRequirements{};
				vkGetBufferMemoryRequirements(device.getDevice(), buffer, &memoryRequirements);

				VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
				memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
				memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

				VkMemoryAllocateInfo memoryAllocateInfo = {};
				memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
				memoryAllocateInfo.allocationSize = memoryRequirements.size;
				memoryAllocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
				if(vkAllocateMemory(device.getDevice(), &memoryAllocateInfo, nullptr, &deviceMemory) != VK_SUCCESS) {
					throw std::runtime_error("failed to allocate dedicated buffer memory");
				}
				if(vkBindBufferMemory(device.getDevice(), buffer, deviceMemory, 0)) {
					throw std::runtime_error("failed to bind scratch buffer memory");
				}
			}
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
		void Buffer::setRange(size_t range) {
			this->range = range;
		}
		void Buffer::cleanup() {
			if (!dedicatedMemory) {
				if (buffer != VK_NULL_HANDLE) {
					//std::cout<<"destroy buffer : "<<buffer<<std::endl;
					vmaDestroyBuffer(allocator, buffer, memory);
					buffer = VK_NULL_HANDLE;
				}
			} else {
				if (buffer != VK_NULL_HANDLE) {
					//std::cout<<"destroy buffer : "<<buffer<<std::endl;
					vkFreeMemory(device.getDevice(), deviceMemory, nullptr);
					vkDestroyBuffer(device.getDevice(), buffer, nullptr);
					buffer = VK_NULL_HANDLE;
				}
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
		void swap(Buffer& a, Buffer& b) noexcept {
			a.swap(b);
		}
		uint32_t Buffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
            VkPhysicalDeviceMemoryProperties memProperties;
            vkGetPhysicalDeviceMemoryProperties(device.getPhysicalDevice(), &memProperties);
            for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
                if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                    return i;
                }
            }
            throw std::runtime_error("aucun type de memoire ne satisfait le buffer!");
        }
		uint64_t Buffer::getDeviceAddress() {
			if (dedicatedMemory) {
				VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{};
				bufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
				bufferDeviceAddressInfo.buffer = buffer;
				return vkGetBufferDeviceAddressKHR(GPUContext::instance().getDevice().getDevice(), &bufferDeviceAddressInfo);            
			}
			return 0;
		}
	}
}
