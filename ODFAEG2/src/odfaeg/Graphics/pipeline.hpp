#ifndef ODFAEG_PIPELINE_HPP
#define ODFAEG_PIPELINE_HPP
#include <vulkan/vulkan.hpp>
#include <deque>
#include <vulkan/vulkan.hpp>
#include <odfaeg/config.hpp>
#include <iostream>
#include <memory>
#include "../Core/nonCopyable.hpp"
#include "device.hpp"
#include "shader.hpp"
#include "../Entity/primitiveType.hpp"
#include "blendMode.hpp"
#include "renderPass.hpp"
#include "descriptor.hpp"
namespace odfaeg {
	namespace graphic {
		class Pipeline : public core::NonCopyable {			
		public :			
			Pipeline(Device& device);
			Pipeline(Pipeline&& pipeline) noexcept;
			Pipeline& operator=(Pipeline&& pipeline) noexcept;			
			void createGraphicPipeline(Shader& shader, entity::PrimitiveType primitveType, std::deque<DescriptorSetLayout>& setLayouts, RenderPass& renderPass, VkPipelineDepthStencilStateCreateInfo depthStencil, BlendMode blendMode,
				VkSampleCountFlagBits msaaSamples, VkCullModeFlags cullMode = VK_CULL_MODE_NONE, VkPolygonMode polygoneMode = VK_POLYGON_MODE_FILL, std::vector<VkPushConstantRange> pushConstants = std::vector<VkPushConstantRange>());
			void createGraphicPipeline(Shader& shader, entity::PrimitiveType primitveType, std::deque<DescriptorSetLayout>& setLayouts, VkPipelineRenderingCreateInfo renderingCreateInfo, VkPipelineDepthStencilStateCreateInfo depthStencil, BlendMode blendMode,
            				VkSampleCountFlagBits msaaSamples, VkCullModeFlags cullMode = VK_CULL_MODE_NONE, VkPolygonMode polygoneMode = VK_POLYGON_MODE_FILL, std::vector<VkPushConstantRange> pushConstants = std::vector<VkPushConstantRange>());
			void createComputePipeline(Shader& shader, std::deque<DescriptorSetLayout>& setLayouts, std::vector<VkPushConstantRange> pushConstants = std::vector<VkPushConstantRange>());
			void createGraphicPipeline(Shader& shader, std::deque<DescriptorSetLayout>& setLayouts, VkPipelineRenderingCreateInfo renderingCreateInfo, VkPipelineDepthStencilStateCreateInfo depthStencil, BlendMode blendMode,
				VkSampleCountFlagBits msaaSamples, VkCullModeFlags cullMode = VK_CULL_MODE_NONE, VkPolygonMode polygoneMode = VK_POLYGON_MODE_FILL, std::vector<VkPushConstantRange> pushConstants = std::vector<VkPushConstantRange>());
			void createRTPipeline(Shader& shader, std::deque<DescriptorSetLayout>& setLayouts, std::vector<VkPushConstantRange> pushConstants = std::vector<VkPushConstantRange>());
			void cleanup();
			VkPipeline getHandle();
			VkPipelineLayout getLayout();			
			~Pipeline();
		private:
			VkPipeline pipeline;
			Device& device;
			VkPipelineLayout pipelineLayout;
		};
	}
}
#endif