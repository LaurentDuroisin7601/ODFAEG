#ifndef ODFAEG_DESCRIPTOR_HPP
#define ODFAEG_DESCRIPTOR_HPP
#include <deque>
#include <vulkan/vulkan.hpp>
#include <vector>
#include <memory>
#include <odfaeg/config.hpp>
#include <iostream>
#include "buffer.hpp"
#include "image.hpp"
#include "../Core/nonCopyable.hpp"
#include "device.hpp" 
#include "texture.hpp"
#include "vertexBuffer.hpp"
namespace odfaeg {
	namespace graphic {
	    class DescriptorPool : public core::NonCopyable {
		public:
			DescriptorPool(Device& device);
			DescriptorPool(DescriptorPool&& other) noexcept;
			DescriptorPool& operator=(DescriptorPool&& other) noexcept;
			void setNbBindings(unsigned int nbBindings, unsigned int maxSets);
			void updatePoolSize(unsigned int binding, VkDescriptorType descriptorType, unsigned int descriptorCount);
			void update();
			VkDescriptorPool getHandle();
			Device& getDevice();
			void cleanup();
			~DescriptorPool();
		private:
			std::vector<VkDescriptorPoolSize> poolSizes;
			size_t maxSets;
			VkDescriptorPool descriptorPool;
			Device& device;
		};	
		class DescriptorSetLayout : public core::NonCopyable {
		public:
			DescriptorSetLayout(Device& device);
			DescriptorSetLayout(DescriptorSetLayout&& other) noexcept;
			DescriptorSetLayout& operator= (DescriptorSetLayout&& other) noexcept;
			void setNbBindings(unsigned int nbBindings, bool bindless);
 			void updateLayout(unsigned int binding, VkDescriptorType descriptorType, unsigned int descriptorCount, VkShaderStageFlags shaderStageFlags, VkDescriptorBindingFlags bindlessFlags=0);
			void update();
			VkDescriptorSetLayout getHandle();
			void cleanup();
			~DescriptorSetLayout();
		private :
			std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
			std::vector<VkDescriptorBindingFlags> bindlessFlags;
			bool bindless;
			VkDescriptorSetLayout descriptorSetLayout;
			Device& device;
		};	
		class DescriptorSet : public core::NonCopyable {
		public :
			DescriptorSet(Device& device);
			DescriptorSet(DescriptorSet&& descriptorSet) noexcept;
			DescriptorSet& operator=(DescriptorSet&& descriptorSet) noexcept;
			void setNbBindings(unsigned int nbBindings);
			static void allocate(DescriptorPool& desriptorPool, DescriptorSetLayout& descritorSetLayout, std::deque<DescriptorSet>& descriptorSets, unsigned int bindless=0);
			void updateBufferInfos(unsigned int binding, Buffer& buffer, VkDescriptorType descriptorType);
			void updateBufferInfos(unsigned int binding, std::deque<Buffer>& buffers, VkDescriptorType descriptorType);
			void updateBufferInfos(unsigned int binding, bool vertices, std::deque<VertexBuffer>& buffers, VkDescriptorType descriptorType);
			void updateImageInfos(unsigned int binding, std::deque<Image>& images, VkDescriptorType descriptorType);
			void updateImageInfos(unsigned int binding, std::deque<Texture>& images, VkDescriptorType descriptorType);
			void updateImageInfos(unsigned int binding, Texture& images, VkDescriptorType descriptorType);
			void updateImageInfos(unsigned int binding, Texture& images, VkDescriptorType descriptorType, unsigned int imageViewIndex);
			void updateAccelerationStructureInfos(unsigned int binding, std::vector<VkAccelerationStructureKHR> handles);
			void updateDescriptorSet();
			VkDescriptorSet getHandle();
			void setHandle(VkDescriptorSet descriptorSet);
		private:
			std::vector<std::vector<VkDescriptorBufferInfo>> bufferInfos;
			std::vector<std::vector<VkDescriptorImageInfo>>	imageInfos;
			std::vector<VkWriteDescriptorSet> descriptorWrites;
			VkWriteDescriptorSetAccelerationStructureKHR descriptorAccelerationStructureInfo={};			
			VkDescriptorSet descriptorSet;
			Device& device;
		};
	}
}
#endif
