module;
#include <deque>
#include <memory>
#include <vulkan/vulkan.hpp>
export module odfaeg.graphic.pipeline;
import odfaeg.core.nonCopyable;
import odfaeg.graphic.device;
import odfaeg.graphic.shader;
import odfaeg.graphic.primitiveType;
import odfaeg.graphic.descriptor;
import odfaeg.graphic.blendMode;
import odfaeg.graphic.renderPass;
namespace odfaeg {
	namespace graphic {
		export class Pipeline : public core::NonCopyable {
		public :			
			Pipeline(Device& device);
			Pipeline(Pipeline&& pipeline) noexcept;
			Pipeline& operator=(Pipeline&& pipeline) noexcept;			
			void createGraphicPipeline(Shader& shader, PrimitiveType primitveType, std::deque<DescriptorSetLayout>& setLayouts, RenderPass& renderPass, VkPipelineDepthStencilStateCreateInfo depthStencil, BlendMode blendMode,
				VkCullModeFlags cullMode = VK_CULL_MODE_NONE, VkPolygonMode polygoneMode = VK_POLYGON_MODE_FILL, std::vector<VkPushConstantRange> pushConstants = std::vector<VkPushConstantRange>());
			void createGraphicPipeline(Shader& shader, PrimitiveType primitveType, std::deque<DescriptorSetLayout>& setLayouts, VkPipelineRenderingCreateInfo renderingCreateInfo, VkPipelineDepthStencilStateCreateInfo depthStencil, BlendMode blendMode,
            				VkCullModeFlags cullMode = VK_CULL_MODE_NONE, VkPolygonMode polygoneMode = VK_POLYGON_MODE_FILL, std::vector<VkPushConstantRange> pushConstants = std::vector<VkPushConstantRange>());
			void createComputePipeline(Shader& shader, std::deque<DescriptorSetLayout>& setLayouts, std::vector<VkPushConstantRange> pushConstants = std::vector<VkPushConstantRange>());
			void createGraphicPipeline(Shader& shader, std::deque<DescriptorSetLayout>& setLayouts, VkPipelineRenderingCreateInfo renderingCreateInfo, VkPipelineDepthStencilStateCreateInfo depthStencil, BlendMode blendMode,
				VkCullModeFlags cullMode = VK_CULL_MODE_NONE, VkPolygonMode polygoneMode = VK_POLYGON_MODE_FILL, std::vector<VkPushConstantRange> pushConstants = std::vector<VkPushConstantRange>());
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