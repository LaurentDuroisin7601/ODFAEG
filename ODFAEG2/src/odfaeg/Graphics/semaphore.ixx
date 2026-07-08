module;
#include <vulkan/vulkan.hpp>
export module odfaeg.graphic.semaphore;
import odfaeg.graphic.device;
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

