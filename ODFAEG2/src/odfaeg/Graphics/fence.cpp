module;
#include <vulkan/vulkan.hpp>
//#include <odfaeg/config.hpp>
//import odfaeg.graphic.fence;
module odfaeg.graphic.fence;

namespace odfaeg {
	namespace graphic {
		Fence::Fence(Device& device) : device(device) {

		}
		VkFence& Fence::getHandle(unsigned int frame) {
			return fences[frame];
		}
		void Fence::create(unsigned int fenceCount, bool signaled) {
			if (fences.size() > 0) {
				cleanup();
			}
			fences.resize(fenceCount);
			for (unsigned int i = 0; i < fences.size(); i++) {
				VkFenceCreateInfo fenceInfo{};
				fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				if (signaled)
					fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
				if (vkCreateFence(device.getDevice(), &fenceInfo, nullptr, &fences[i]) != VK_SUCCESS) {
					throw std::runtime_error("failed to create fence");
				}
			}
		}
		void Fence::waitForFences(bool waitAll, uint64_t timeout) {
			if (fences.size() > 0) {
				//std::lock_guard<std::recursive_mutex> lock(getGlobalMutex());
				vkWaitForFences(device.getDevice(), fences.size(), fences.data(), (waitAll) ? VK_TRUE : VK_FALSE, timeout);
			}
		}
		void Fence::resetFences() {
			if (fences.size() > 0) {
				//std::lock_guard<std::recursive_mutex> lock(getGlobalMutex());
				vkResetFences(device.getDevice(), fences.size(), fences.data());
			}
		}
		void Fence::cleanup() {
			for (unsigned int i = 0; i < fences.size(); i++) {
				vkDestroyFence(device.getDevice(), fences[i], nullptr);
			}
			fences.clear();
		}
		Fence::~Fence() {
			cleanup();
		}
	}
}