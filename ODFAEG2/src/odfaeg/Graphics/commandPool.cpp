module;
#include <iostream>
#include <ostream>
#include <vulkan/vulkan.hpp>
import odfaeg.graphic.commandPool;
module odfaeg.graphic.commandPool;

namespace odfaeg {
	namespace graphic {
		CommandPool::CommandPool(Device& device) : device(device) {
			commandPool = VK_NULL_HANDLE;
		}
		CommandPool::CommandPool(CommandPool&& other) noexcept : device(other.device) {
			commandPool = other.commandPool;
			other.commandPool = VK_NULL_HANDLE;
			commandBuffers = other.commandBuffers;
		}
		CommandPool& CommandPool::operator=(CommandPool&& other) noexcept {
			if (this != &other) {
				cleanup();
				commandPool = other.commandPool;
				other.commandPool = VK_NULL_HANDLE;
				commandBuffers = other.commandBuffers;
			}
			return *this;
		}
		void CommandPool::create(uint32_t queueFamilyIndex) {
			if (commandPool != VK_NULL_HANDLE) {
				cleanup();
			}
			VkCommandPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.flags =  VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			poolInfo.queueFamilyIndex = queueFamilyIndex;
			if (vkCreateCommandPool(device.getDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
				throw std::runtime_error("Echec de la cr�ation d'une command pool!");
			}
			
		}		
		void CommandPool::createCommandBuffers(bool primary, unsigned int commandBufferCount) {
			if (commandBuffers.size() > 0) {
				vkFreeCommandBuffers(device.getDevice(), commandPool, commandBuffers.size(), commandBuffers.data());
				recordingStates.clear();
				commandBuffers.clear();
			}
			commandBuffers.resize(commandBufferCount);
			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = commandPool;
			allocInfo.level = (primary) ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
			allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

			if (vkAllocateCommandBuffers(device.getDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate command buffers!");
			}
			recordingStates.resize(commandBufferCount, false);
		}
		void CommandPool::beginRecordCommandBuffer(unsigned int frame) {
			//std::cout<<"is recording ? "<<recordingStates[frame]<<std::endl;
			if (!recordingStates[frame]) {
				recordingStates[frame] = true;
				vkResetCommandBuffer(commandBuffers[frame], 0);
				VkCommandBufferBeginInfo beginInfo{};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;				

				//std::cout<<"begin record command buffer : "<<commandBuffers[frame]<<std::endl;
				if (vkBeginCommandBuffer(commandBuffers[frame], &beginInfo) != VK_SUCCESS) {
					throw std::runtime_error("failed to begin recording command buffer!");
				}

			}
		}
		void CommandPool::beginRecordCommandBuffer(unsigned int frame, VkCommandBufferInheritanceInfo inheritanceInfo) {
			if (!recordingStates[frame]) {
				VkCommandBufferBeginInfo beginInfo{};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
				beginInfo.pInheritanceInfo = &inheritanceInfo;
				vkResetCommandBuffer(commandBuffers[frame], 0);
				//std::cout<<"begin record command buffer : "<<commandBuffers[frame]<<std::endl;
				if (vkBeginCommandBuffer(commandBuffers[frame], &beginInfo) != VK_SUCCESS) {
					throw std::runtime_error("failed to begin recording command buffer!");
				}
				recordingStates[frame] = true;
			}
		}
		void CommandPool::endRecordCommandBuffer(unsigned int frame) {
			//std::cout<<"is recording ? "<<recordingStates[frame]<<std::endl;
			if (recordingStates[frame]) {
				//std::cout<<"end record command buffer : "<<commandBuffers[frame]<<std::endl;
				if (vkEndCommandBuffer(commandBuffers[frame]) != VK_SUCCESS) {
					throw std::runtime_error("failed to record command buffer!");
				}
				recordingStates[frame] = false;
			}
		}
		VkCommandPool CommandPool::getHandle() {
			return commandPool;
		}
		VkCommandBuffer& CommandPool::getHandle(int frame) {
			return commandBuffers[frame];
		}
		std::vector<VkCommandBuffer>& CommandPool::getHandles() {
			return commandBuffers;
		}
		void CommandPool::cleanup() {
			if (commandPool != VK_NULL_HANDLE) {
				vkDeviceWaitIdle(device.getDevice());
				if (commandBuffers.size() > 0) {
					vkFreeCommandBuffers(device.getDevice(), commandPool, commandBuffers.size(), commandBuffers.data());
				}
				for (unsigned int i = 0; i < commandBuffers.size(); i++) {
					commandBuffers[i] = VK_NULL_HANDLE;
				}
				commandBuffers.clear();
				recordingStates.clear();
				vkDestroyCommandPool(device.getDevice(), commandPool, nullptr);
				commandPool = VK_NULL_HANDLE;
			}
		}
		Device& CommandPool::getDevice() {
			return device;
		}
		CommandPool::~CommandPool() {
			cleanup();
		}
	}
}