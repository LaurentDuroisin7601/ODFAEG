module;
#include <vulkan/vulkan.hpp>
export module odfaeg.graphic.commandPool;
import odfaeg.graphic.device;
import odfaeg.core.nonCopyable;
namespace odfaeg {
	namespace graphic {
		export class CommandPool : public core::NonCopyable {
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