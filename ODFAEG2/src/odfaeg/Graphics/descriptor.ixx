module;
#include <deque>
#include <vulkan/vulkan.hpp>
#include <vector>
#include <memory>
export module odfaeg.graphic.descriptor;
import odfaeg.graphic.buffer;
import odfaeg.graphic.image;
import odfaeg.core.nonCopyable;
import odfaeg.graphic.device; 
import odfaeg.graphic.texture;
import odfaeg.graphic.vertexBuffer;
namespace odfaeg {
	namespace graphic {
		export class DescriptorPool : public core::NonCopyable {
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
		export class DescriptorSetLayout : public core::NonCopyable {
		public:
			DescriptorSetLayout(Device& device);
			DescriptorSetLayout(DescriptorSetLayout&& other) noexcept;
			DescriptorSetLayout& operator= (DescriptorSetLayout&& other) noexcept;
			void setNbBindings(unsigned int nbBindings, bool bindless);
 			void updateLayout(unsigned int binding, VkDescriptorType descriptorType, unsigned int descriptorCount, VkShaderStageFlags, VkDescriptorBindingFlags bindlessFlags=0);
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
		export class DescriptorSet : public core::NonCopyable {
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
			void updateDescriptorSet();
			VkDescriptorSet getHandle();
			void setHandle(VkDescriptorSet descriptorSet);
		private:
			std::vector<std::vector<VkDescriptorBufferInfo>> bufferInfos;
			std::vector<std::vector<VkDescriptorImageInfo>>	imageInfos;
			std::vector<VkWriteDescriptorSet> descriptorWrites;			
			VkDescriptorSet descriptorSet;
			Device& device;
		};
	}
}
