#include <vulkan/vulkan.hpp>
#include "device.hpp"
namespace odfaeg {
	namespace graphic {
		export class Semaphore {
		public:
			Semaphore(Device& device);
			void create(bool timeline = false, std::uint64_t value=0);
			void incrementValue();
			std::uint64_t& getValue();
			void cleanup();
			bool isTimeline();
			VkSemaphore& getHandle();
			~Semaphore();
		private:
			VkSemaphore semaphore;
			std::uint64_t value;
			bool timeline;
			Device& device;
		};
	}
}
#include "semaphore.inl"

