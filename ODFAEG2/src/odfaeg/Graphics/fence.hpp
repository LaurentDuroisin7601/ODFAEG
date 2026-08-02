#ifndef ODFAEG_FENCE_HPP
#define ODFAEG_FENCE_HPP
#include <vulkan/vulkan.hpp>
#include <vector>
namespace odfaeg{
	namespace graphic {
		class Fence {
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
#include "fence.inl"
#endif