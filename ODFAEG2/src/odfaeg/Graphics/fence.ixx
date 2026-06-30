module;
#include <vulkan/vulkan.hpp>
#include <vector>
export module odfaeg.graphic.fence;
import odfaeg.graphic.device;
namespace odfaeg{
	namespace graphic {
		export class Fence {
		public:
			Fence(Device& device);			
			void create(unsigned int fenceCount=1, bool signaled = false);
			VkFence& getHandle(unsigned int frame=0);
			void waitForFences(bool waitAll, uint64_t timeout);
			void resetFences();
			void cleanup();
			~Fence();
		private:
			Device& device;
			std::vector<VkFence> fences;
		};
	}
}
