module;
#include <iostream>
#include <ostream>
#include <vulkan/vulkan.hpp>
#include "device.hpp"
#include "../Core/nonCopyable.hpp"
namespace odfaeg {
	namespace graphic {
		class CommandPool : public core::NonCopyable {
		public:
			CommandPool(Device& device);
			CommandPool(CommandPool&& other) noexcept;
			CommandPool& operator=(CommandPool&& commandPool) noexcept;
			void create(uint32_t queueFamilyIndex);
			void reset();
			void createCommandBuffers(bool primary, unsigned int commandBufferCount);
			void beginRecordCommandBuffer(unsigned int frame);
			void beginRecordCommandBuffer(unsigned int frame, VkCommandBufferInheritanceInfo inheritanceInfo);
			void endRecordCommandBuffer(unsigned int frame);
			void cleanup();
			VkCommandPool getHandle();
			VkCommandBuffer& getHandle(int frame);
			std::vector<VkCommandBuffer>& getHandles();
			Device& getDevice();
			~CommandPool();
		private :
			VkCommandPool commandPool;
			std::vector<VkCommandBuffer> commandBuffers;
			std::vector<bool> recordingStates;			
			Device& device;
		};
	}
}
#include "commandPool.inl"