module;
#include <vulkan/vulkan.hpp>
//import odfaeg.graphic.semaphore;
module odfaeg.graphic.semaphore;
namespace odfaeg {
	namespace graphic {	
		Semaphore::Semaphore(Device& device)  : device(device) {
			semaphore = VK_NULL_HANDLE;
		}
		void Semaphore::create(bool timeline, std::uint64_t value) {
			if (semaphore != VK_NULL_HANDLE) {
				cleanup();
			}
			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			VkSemaphoreTypeCreateInfo timelineCreateInfo{};
			if (timeline) {
				timelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
				timelineCreateInfo.pNext = nullptr;
				timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
				timelineCreateInfo.initialValue = value;
				semaphoreInfo.pNext = &timelineCreateInfo;			}

			if (vkCreateSemaphore(device.getDevice(), &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS) {
				throw std::runtime_error("failed to create semaphore");
			}				
			
		}
		void Semaphore::incrementValue() {
			value++;
		}
		std::uint64_t& Semaphore::getValue() {
			return value;
		}
		void Semaphore::cleanup() {
			if (semaphore != VK_NULL_HANDLE) {
				vkDestroySemaphore(device.getDevice(), semaphore, nullptr);
			}
			semaphore = VK_NULL_HANDLE;
		}
		VkSemaphore& Semaphore::getHandle() {
			return semaphore;
		}
		bool Semaphore::isTimeline() {
			return timeline;
		}
		Semaphore::~Semaphore() {
			cleanup();
		}
	}
}
