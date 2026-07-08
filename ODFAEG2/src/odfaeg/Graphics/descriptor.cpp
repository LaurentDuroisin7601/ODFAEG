module;
#include <deque>
#include <vulkan/vulkan.hpp>
#include <vector>
#include <odfaeg/config.hpp>
#include <iostream>
#include <memory>
//import odfaeg.graphic.descriptor;
module odfaeg.graphic.descriptor;
namespace odfaeg {
	namespace graphic {
		DescriptorPool::DescriptorPool(Device& device) : device(device) {
			descriptorPool = VK_NULL_HANDLE;
		}
		DescriptorPool::DescriptorPool(DescriptorPool&& other) noexcept : device (other.device) {			
			poolSizes = other.poolSizes;
			maxSets = other.maxSets;
			descriptorPool = other.descriptorPool;
			other.descriptorPool = VK_NULL_HANDLE;
		}

		void DescriptorPool::setNbBindings(unsigned int nbBindings, unsigned int maxSets) {
			poolSizes.resize(nbBindings);
			this->maxSets = maxSets;
		}
		DescriptorPool& DescriptorPool::operator=(DescriptorPool&& other) noexcept {
			if (this != &other) {	
				cleanup();
				poolSizes = other.poolSizes;
				maxSets = other.maxSets;
				descriptorPool = other.descriptorPool;
				other.descriptorPool = VK_NULL_HANDLE;
			}
			return *this;
		}
		void DescriptorPool::updatePoolSize(unsigned int binding, VkDescriptorType descriptorType, unsigned int descriptorCount) {
			poolSizes[binding].type = descriptorType;
			poolSizes[binding].descriptorCount = descriptorCount;
		}
		void DescriptorPool::update() {
			if (descriptorPool != VK_NULL_HANDLE) {
				cleanup();
			}
			VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
			poolInfo.pPoolSizes = poolSizes.data();
			poolInfo.maxSets = maxSets;

			if (vkCreateDescriptorPool(device.getDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
				throw std::runtime_error("echec de la creation de la pool de descripteurs!");
			}
			//std::cout<<"update descriptor pool : "<<descriptorPool<<std::endl;
		}
		void DescriptorPool::cleanup() {
			if (descriptorPool != nullptr) {
				vkDestroyDescriptorPool(device.getDevice(), descriptorPool, nullptr);
			}
		}
		VkDescriptorPool DescriptorPool::getHandle() {
			return descriptorPool;
		}
		Device& DescriptorPool::getDevice() {
			return device;
		}
		DescriptorPool::~DescriptorPool() {
			cleanup();
		}
		DescriptorSetLayout::DescriptorSetLayout(Device& device) : device(device) {

			descriptorSetLayout = VK_NULL_HANDLE;
		}
		DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& other) noexcept : device(other.device) {			
			layoutBindings = other.layoutBindings;
			bindlessFlags = other.bindlessFlags;
			bindless = other.bindless;
			descriptorSetLayout = other.descriptorSetLayout;
			other.descriptorSetLayout = VK_NULL_HANDLE;
		}
		DescriptorSetLayout& DescriptorSetLayout::operator=(DescriptorSetLayout&& other) noexcept {
			if (this != &other) {
				cleanup();
				layoutBindings = other.layoutBindings;
				bindlessFlags = other.bindlessFlags;
				bindless = other.bindless;
				descriptorSetLayout = other.descriptorSetLayout;
				other.descriptorSetLayout = VK_NULL_HANDLE;
			}
			return *this;
		}
		void DescriptorSetLayout::setNbBindings(unsigned int nbBindings, bool bindless) {
			layoutBindings.resize(nbBindings);
			if (bindless) {
				bindlessFlags.resize(nbBindings, 0);
			}
			this->bindless = bindless;
		}
		void DescriptorSetLayout::updateLayout(unsigned int binding, VkDescriptorType descriptorType, unsigned int descriptorCount, VkShaderStageFlags stageFlags, VkDescriptorBindingFlags bindlessFlags) {
			layoutBindings[binding].binding = binding;
			layoutBindings[binding].descriptorCount = descriptorCount;
			layoutBindings[binding].descriptorType = descriptorType;
			layoutBindings[binding].pImmutableSamplers = nullptr;
			layoutBindings[binding].stageFlags = stageFlags;
			if (bindless) {
				if (binding == layoutBindings.size() - 1) {
					this->bindlessFlags[binding] = bindlessFlags;
				}
			}
		}	
		void DescriptorSetLayout::update() {
			if (descriptorSetLayout != VK_NULL_HANDLE) {
				cleanup();
			}
			VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
			if (bindless) {				
				bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
				bindingFlagsInfo.bindingCount = static_cast<uint32_t>(bindlessFlags.size());
				//std::cout<<"bindings  flags: size "<<layoutBindings.size()<<std::endl;
				bindingFlagsInfo.pBindingFlags = bindlessFlags.data();
				layoutInfo.pNext = &bindingFlagsInfo;
			}
			layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());;
			layoutInfo.pBindings = layoutBindings.data();
			//std::cout<<"bindings : "<<layoutBindings.size()<<std::endl;

			if (vkCreateDescriptorSetLayout(device.getDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
				throw std::runtime_error("failed to create descriptor set layout!");
			}
		}
		VkDescriptorSetLayout DescriptorSetLayout::getHandle() {
			return descriptorSetLayout;
		}
		void DescriptorSetLayout::cleanup() {
			if (descriptorSetLayout != nullptr) {
				vkDestroyDescriptorSetLayout(device.getDevice(), descriptorSetLayout, nullptr);
			}
		}
		DescriptorSetLayout::~DescriptorSetLayout() {
			cleanup();
		}
		DescriptorSet::DescriptorSet(Device& device) : device(device) {
			descriptorSet = VK_NULL_HANDLE;
		}
		DescriptorSet::DescriptorSet(DescriptorSet&& other) noexcept: device (other.device) {			
			bufferInfos = other.bufferInfos;
			imageInfos = other.imageInfos;
			descriptorWrites = other.descriptorWrites;
			descriptorSet = other.descriptorSet;
			other.descriptorSet = VK_NULL_HANDLE;
		}
		void DescriptorSet::setNbBindings(unsigned int nbBindings) {
			descriptorWrites.resize(nbBindings);
			bufferInfos.resize(nbBindings);
			imageInfos.resize(nbBindings);
			//std::cout<<"nb bindings : "<<nbBindings<<std::endl;
		}
		DescriptorSet& DescriptorSet::operator=(DescriptorSet&& other) noexcept {
			if (this != &other) {
				bufferInfos = other.bufferInfos;
				imageInfos = other.imageInfos;
				descriptorWrites = other.descriptorWrites;
				descriptorSet = other.descriptorSet;
				other.descriptorSet = VK_NULL_HANDLE;
			}
			return *this;
		}
		void DescriptorSet::allocate(DescriptorPool& descriptorPool,
							 DescriptorSetLayout& descriptorSetLayout,
							 std::deque<DescriptorSet>& descriptorSets,
							 unsigned int bindless) {
			//std::cout<<"empty ?"<<std::endl;
			if (descriptorSets.empty()) {
				return; // rien à allouer
			}
			//std::cout<<"empty done"<<std::endl;
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool.getHandle();
			allocInfo.descriptorSetCount = static_cast<uint32_t>(descriptorSets.size());

			std::vector<VkDescriptorSetLayout> layouts(descriptorSets.size(),
													   descriptorSetLayout.getHandle());
			allocInfo.pSetLayouts = layouts.data();
			//std::cout<<"layout : "<<descriptorSetLayout.getHandle()<<std::endl;

			VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
			std::vector<std::uint32_t> variableCounts(descriptorSets.size(),
													 bindless);
			if (bindless > 0) {

				variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
				variableCountInfo.descriptorSetCount = static_cast<uint32_t>(variableCounts.size());
				variableCountInfo.pDescriptorCounts = variableCounts.data();
				allocInfo.pNext = &variableCountInfo;
			}

			std::vector<VkDescriptorSet> sets(descriptorSets.size());

			if (vkAllocateDescriptorSets(descriptorPool.getDevice().getDevice(),
										 &allocInfo,
										 sets.data()) != VK_SUCCESS) {
				throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
			}
			//std::cout<<"set : "<<sets[0]<<std::endl;

			// recopier les handles dans tes wrappers
			for (size_t i = 0; i < descriptorSets.size(); ++i) {
				descriptorSets[i].setHandle(sets[i]); // à implémenter si pas déjà fait
			}
		}
		void DescriptorSet::updateBufferInfos(unsigned int binding, Buffer& buffer, VkDescriptorType descriptorType) {
			bufferInfos[binding].resize(1);
			//std::cout<<"buffer size : "<<buffers.size()<<"update ds write : "<<i<<std::endl;
			bufferInfos[binding][0].buffer = buffer.getHandle();
			bufferInfos[binding][0].offset = buffer.getOffset();
			bufferInfos[binding][0].range = buffer.getRange();
			descriptorWrites[binding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[binding].dstSet = descriptorSet;
			descriptorWrites[binding].dstBinding = binding;
			descriptorWrites[binding].dstArrayElement = 0;
			descriptorWrites[binding].descriptorType = descriptorType;
			descriptorWrites[binding].descriptorCount = bufferInfos[binding].size();
			descriptorWrites[binding].pBufferInfo = bufferInfos[binding].data();
		}
		void DescriptorSet::updateBufferInfos(unsigned int binding, std::deque<Buffer>& buffers, VkDescriptorType descriptorType) {
			bufferInfos[binding].resize(buffers.size());
			for (unsigned int i = 0; i < buffers.size(); i++) {
				//std::cout<<"buffer size : "<<buffers.size()<<"update ds write : "<<i<<std::endl;
				bufferInfos[binding][i].buffer = buffers[i].getHandle();
				bufferInfos[binding][i].offset = buffers[i].getOffset();
				bufferInfos[binding][i].range = buffers[i].getRange();
			}
			descriptorWrites[binding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[binding].dstSet = descriptorSet;
			descriptorWrites[binding].dstBinding = binding;
			descriptorWrites[binding].dstArrayElement = 0;
			descriptorWrites[binding].descriptorType = descriptorType;
			descriptorWrites[binding].descriptorCount = bufferInfos[binding].size();
			descriptorWrites[binding].pBufferInfo = bufferInfos[binding].data();
		}
		void DescriptorSet::updateBufferInfos(unsigned int binding, bool vertices, std::deque<VertexBuffer>& buffers, VkDescriptorType descriptorType) {
			for (unsigned int i = 0; i < buffers.size(); i++) {
				bufferInfos[binding].resize(buffers.size()*buffers[i].getNbBuffers());
				for (unsigned int j = 0; j < buffers[i].getNbBuffers(); j++) {
					//std::cout<<"i :"<<i<<std::endl;
					if (vertices) {
						bufferInfos[binding][i * buffers[i].getNbBuffers() + j].buffer = buffers[i].getVertexBuffer(j).getHandle();
						bufferInfos[binding][i * buffers[i].getNbBuffers() + j].offset = buffers[i].getVertexBuffer(j).getOffset();
						bufferInfos[binding][i * buffers[i].getNbBuffers() + j].range = buffers[i].getVertexBuffer(j).getRange();
					} else {
						bufferInfos[binding][i * buffers[i].getNbBuffers() + j].buffer = buffers[i].getIndexBuffer(j).getHandle();
						bufferInfos[binding][i * buffers[i].getNbBuffers() + j].offset = buffers[i].getIndexBuffer(j).getOffset();
						bufferInfos[binding][i * buffers[i].getNbBuffers() + j].range = buffers[i].getIndexBuffer(j).getRange();
					}
				}
			}
			descriptorWrites[binding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[binding].dstSet = descriptorSet;
			descriptorWrites[binding].dstBinding = binding;
			descriptorWrites[binding].dstArrayElement = 0;
			descriptorWrites[binding].descriptorType = descriptorType;
			descriptorWrites[binding].descriptorCount = bufferInfos[binding].size();
			descriptorWrites[binding].pBufferInfo = bufferInfos[binding].data();
		}
		void DescriptorSet::updateImageInfos(unsigned int binding, std::deque<Texture>& images, VkDescriptorType descriptorType) {
			for (unsigned int i = 0; i < images.size(); i++) {
				imageInfos[binding].resize(images.size()*images[i].getNbBuffers());
				for (unsigned int j = 0; j < images[i].getNbBuffers(); j++) {
					//std::cout<<"add texture info!"<<std::endl;
					imageInfos[binding][i * images[i].getNbBuffers() + j].imageLayout = images[i].getImage(j).getLayout();
					imageInfos[binding][i * images[i].getNbBuffers() + j].imageView = images[i].getImage(j).getImageView().getHandle();
					imageInfos[binding][i * images[i].getNbBuffers() + j].sampler = images[i].getImage(j).getSampler().getHandle();
				}
			}
			descriptorWrites[binding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[binding].dstSet = descriptorSet;
			descriptorWrites[binding].dstBinding = binding;
			descriptorWrites[binding].dstArrayElement = 0;
			descriptorWrites[binding].descriptorType = descriptorType;
			descriptorWrites[binding].descriptorCount = imageInfos[binding].size();
			descriptorWrites[binding].pImageInfo = imageInfos[binding].data();
		}
		void DescriptorSet::updateImageInfos(unsigned int binding, Texture& images, VkDescriptorType descriptorType) {
			imageInfos[binding].resize(images.getNbBuffers());
			for (unsigned int j = 0; j < images.getNbBuffers(); j++) {
				//std::cout<<"add texture info!"<<std::endl;
				imageInfos[binding][j].imageLayout = images.getImage(j).getLayout();
				imageInfos[binding][j].imageView = images.getImage(j).getImageView().getHandle();
				imageInfos[binding][j].sampler = images.getImage(j).getSampler().getHandle();
			}
			descriptorWrites[binding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[binding].dstSet = descriptorSet;
			descriptorWrites[binding].dstBinding = binding;
			descriptorWrites[binding].dstArrayElement = 0;
			descriptorWrites[binding].descriptorType = descriptorType;
			descriptorWrites[binding].descriptorCount = imageInfos[binding].size();
			descriptorWrites[binding].pImageInfo = imageInfos[binding].data();
		}
		void DescriptorSet::updateImageInfos(unsigned int binding, std::deque<Image>& images, VkDescriptorType descriptorType) {
			imageInfos[binding].resize(images.size());
			for (unsigned int i = 0; i < images.size(); i++) {
				imageInfos[binding][i].imageLayout = images[i].getLayout();
				imageInfos[binding][i].imageView = images[i].getImageView().getHandle();
				imageInfos[binding][i].sampler = images[i].getSampler().getHandle();
			}
			descriptorWrites[binding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[binding].dstSet = descriptorSet;
			descriptorWrites[binding].dstBinding = binding;
			descriptorWrites[binding].dstArrayElement = 0;
			descriptorWrites[binding].descriptorType = descriptorType;
			descriptorWrites[binding].descriptorCount = imageInfos[binding].size();
			descriptorWrites[binding].pImageInfo = imageInfos[binding].data();
		}
		void DescriptorSet::updateDescriptorSet() {
			//std::cout<<"update sets : "<<descriptorWrites.size()<<std::endl;
			vkUpdateDescriptorSets(device.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),0, nullptr);
		}
		VkDescriptorSet DescriptorSet::getHandle() {
			return descriptorSet;
		}
		void DescriptorSet::setHandle(VkDescriptorSet descriptorSet) {
			this->descriptorSet = descriptorSet;
		}
	}
}