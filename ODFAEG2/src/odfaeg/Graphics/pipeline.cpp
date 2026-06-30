module;
#include <deque>
#include <vulkan/vulkan.hpp>
#include <odfaeg/config.hpp>
#include <iostream>
#include <memory>
import odfaeg.graphic.pipeline;
module odfaeg.graphic.pipeline;
import odfaeg.graphic.vertex;
import odfaeg.graphic.blendMode;
namespace odfaeg {
	namespace graphic {	
		namespace {
			VkBlendFactor factorToVkConstant(odfaeg::graphic::BlendMode::Factor blendFactor) {
				switch (blendFactor) {
				default:
				case odfaeg::graphic::BlendMode::Zero:             return VK_BLEND_FACTOR_ZERO;
				case odfaeg::graphic::BlendMode::One:              return VK_BLEND_FACTOR_ONE;
				case odfaeg::graphic::BlendMode::SrcColor:         return VK_BLEND_FACTOR_SRC_COLOR;
				case odfaeg::graphic::BlendMode::OneMinusSrcColor: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
				case odfaeg::graphic::BlendMode::DstColor:         return VK_BLEND_FACTOR_DST_COLOR;
				case odfaeg::graphic::BlendMode::OneMinusDstColor: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
				case odfaeg::graphic::BlendMode::SrcAlpha:         return VK_BLEND_FACTOR_SRC_ALPHA;
				case odfaeg::graphic::BlendMode::OneMinusSrcAlpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				case odfaeg::graphic::BlendMode::DstAlpha:         return VK_BLEND_FACTOR_DST_ALPHA;
				case odfaeg::graphic::BlendMode::OneMinusDstAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
				}
			}
			VkBlendOp equationToVkConstant(odfaeg::graphic::BlendMode::Equation blendEquation) {
				switch (blendEquation)
				{
				default:
				case odfaeg::graphic::BlendMode::Add:             return VK_BLEND_OP_ADD;
				case odfaeg::graphic::BlendMode::Subtract:        return VK_BLEND_OP_SUBTRACT;
				}
			}
		}
		Pipeline::Pipeline(Device& device) : device(device) {
			pipeline = VK_NULL_HANDLE;
		}
		Pipeline::Pipeline(Pipeline&& other) noexcept : device(other.device) {			
			pipeline = other.pipeline;
			pipelineLayout = other.pipelineLayout;
		}
		Pipeline& Pipeline::operator= (Pipeline&& other) noexcept {
			if (this != &other) {
				cleanup();
				pipeline = other.pipeline;
				pipelineLayout = other.pipelineLayout;
				other.pipeline = VK_NULL_HANDLE;
			}
			return *this;
		}
		void Pipeline::createGraphicPipeline(Shader& shader, PrimitiveType primitveType, std::deque<DescriptorSetLayout>& setLayouts, VkPipelineRenderingCreateInfo renderingCreateInfo, VkPipelineDepthStencilStateCreateInfo depthStencil, BlendMode blendMode,
			VkCullModeFlags cullMode, VkPolygonMode polygonMode, std::vector<VkPushConstantRange> pushConstants) {
			if (pipeline != VK_NULL_HANDLE) {
				cleanup();
			}
			shader.createShaderModules();
			VkShaderModule vertexShaderModule = VK_NULL_HANDLE;
			VkShaderModule fragmentShaderModule = VK_NULL_HANDLE;
			VkShaderModule geometryShaderModule = VK_NULL_HANDLE;
			vertexShaderModule = shader.getVertexShaderModule();
			fragmentShaderModule = shader.getFragmentShaderModule();
			geometryShaderModule = shader.getGeometryShaderModule();
			std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
			if (vertexShaderModule != VK_NULL_HANDLE) {
				VkPipelineShaderStageCreateInfo vertexShaderStageInfo{};
				vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
				vertexShaderStageInfo.module = vertexShaderModule;
				vertexShaderStageInfo.pName = "main";
				shaderStages.push_back(vertexShaderStageInfo);				
			}
			if (geometryShaderModule != VK_NULL_HANDLE) {
				VkPipelineShaderStageCreateInfo geometryShaderStageInfo{};
				geometryShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				geometryShaderStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
				geometryShaderStageInfo.module = geometryShaderModule;
				geometryShaderStageInfo.pName = "main";
				shaderStages.push_back(geometryShaderStageInfo);
			}
			if (fragmentShaderModule != VK_NULL_HANDLE) {
				VkPipelineShaderStageCreateInfo fragmentShaderStageInfo{};
				fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
				fragmentShaderStageInfo.module = fragmentShaderModule;
				fragmentShaderStageInfo.pName = "main";
				shaderStages.push_back(fragmentShaderStageInfo);
			}
			VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
			auto bindingDescription = Vertex::getBindingDescription();
			auto attributeDescriptions = Vertex::getAttributeDescriptions();
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputInfo.vertexBindingDescriptionCount = 1;
			vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
			vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
			vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
			VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
			inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			VkPrimitiveTopology modes[] = { VK_PRIMITIVE_TOPOLOGY_POINT_LIST, VK_PRIMITIVE_TOPOLOGY_LINE_LIST, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN };
			inputAssembly.topology = modes[primitveType];
			inputAssembly.primitiveRestartEnable =
				(inputAssembly.topology == VK_PRIMITIVE_TOPOLOGY_LINE_STRIP ||
					inputAssembly.topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP ||
					inputAssembly.topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN)
				? VK_TRUE : VK_FALSE;
			VkPipelineViewportStateCreateInfo viewportState{};
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.viewportCount = MAX_SCISSORS_AND_VIEWPORTS;
			viewportState.scissorCount = MAX_SCISSORS_AND_VIEWPORTS;
			std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
			};
			VkPipelineDynamicStateCreateInfo dynamicState{};
			dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
			dynamicState.pDynamicStates = dynamicStates.data();

			VkPipelineRasterizationStateCreateInfo rasterizer{};
			rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer.depthClampEnable = VK_FALSE;
			rasterizer.rasterizerDiscardEnable = VK_FALSE;
			rasterizer.polygonMode = polygonMode;
			rasterizer.lineWidth = 1.0f;
			rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
			rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

			VkPipelineMultisampleStateCreateInfo multisampling{};
			multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			bool hasColorAttachment = renderingCreateInfo.colorAttachmentCount > 0;
			VkPipelineColorBlendAttachmentState colorBlendAttachment{};			
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = VK_FALSE;


			colorBlendAttachment.srcColorBlendFactor = factorToVkConstant(blendMode.colorSrcFactor);
			colorBlendAttachment.dstColorBlendFactor = factorToVkConstant(blendMode.colorDstFactor);
			colorBlendAttachment.colorBlendOp = equationToVkConstant(blendMode.colorEquation);


			colorBlendAttachment.srcAlphaBlendFactor = factorToVkConstant(blendMode.alphaSrcFactor);
			colorBlendAttachment.dstAlphaBlendFactor = factorToVkConstant(blendMode.alphaDstFactor);
			colorBlendAttachment.alphaBlendOp = equationToVkConstant(blendMode.alphaEquation);

			VkPipelineColorBlendStateCreateInfo colorBlending{};
			colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlending.logicOpEnable = VK_FALSE;
			colorBlending.logicOp = VK_LOGIC_OP_COPY;
			colorBlending.attachmentCount = 1;
			colorBlending.pAttachments = &colorBlendAttachment;						
			colorBlending.blendConstants[0] = 0.0f;
			colorBlending.blendConstants[1] = 0.0f;
			colorBlending.blendConstants[2] = 0.0f;
			colorBlending.blendConstants[3] = 0.0f;
			VkPipelineLayoutCreateInfo pipelineLayoutInfos{};
			pipelineLayoutInfos.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
			for (unsigned int i = 0; i < setLayouts.size(); i++) {
				descriptorSetLayouts.push_back(setLayouts[i].getHandle());
			}
			pipelineLayoutInfos.setLayoutCount = descriptorSetLayouts.size();
			pipelineLayoutInfos.pSetLayouts = descriptorSetLayouts.data();
			if (pushConstants.size() > 0) {
				pipelineLayoutInfos.pPushConstantRanges = pushConstants.data();
				pipelineLayoutInfos.pushConstantRangeCount = pushConstants.size();
			}
			if (vkCreatePipelineLayout(device.getDevice(), &pipelineLayoutInfos, nullptr, &pipelineLayout) != VK_SUCCESS) {

				throw std::runtime_error("failed to create pipeline layout!");
			}
			//std::cout<<"piepline layout : "<<shaderStages.size()<<std::endl;
			VkGraphicsPipelineCreateInfo pipelineInfo{};
			pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineInfo.pNext = &renderingCreateInfo;
			pipelineInfo.stageCount = shaderStages.size();
			pipelineInfo.pStages = shaderStages.data();
			pipelineInfo.pVertexInputState = &vertexInputInfo;
			pipelineInfo.pInputAssemblyState = &inputAssembly;
			pipelineInfo.pViewportState = &viewportState;
			pipelineInfo.pRasterizationState = &rasterizer;
			pipelineInfo.pMultisampleState = &multisampling;
			pipelineInfo.pColorBlendState = (hasColorAttachment) ? &colorBlending : nullptr;
			pipelineInfo.pDynamicState = &dynamicState;
			pipelineInfo.layout = pipelineLayout;
			pipelineInfo.renderPass = VK_NULL_HANDLE;
			pipelineInfo.subpass = 0;
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
			pipelineInfo.pDepthStencilState = &depthStencil;
			//std::cout<<"create graphics pipeline"<<std::endl;
			if (vkCreateGraphicsPipelines(device.getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
				throw std::runtime_error("failed to create graphics pipeline!");
			}
			//std::cout<<"pipeline : "<<pipeline<<std::endl;
			shader.cleanupShaderModules();
		}
		void Pipeline::createGraphicPipeline(Shader& shader, PrimitiveType primitveType, std::deque<DescriptorSetLayout>& setLayouts, RenderPass& renderPass, VkPipelineDepthStencilStateCreateInfo depthStencil, BlendMode blendMode,
			VkCullModeFlags cullMode, VkPolygonMode polygonMode, std::vector<VkPushConstantRange> pushConstants) {
			if (pipeline != VK_NULL_HANDLE) {
				cleanup();
			}
			shader.createShaderModules();
			VkShaderModule vertexShaderModule = VK_NULL_HANDLE;
			VkShaderModule fragmentShaderModule = VK_NULL_HANDLE;
			VkShaderModule geometryShaderModule = VK_NULL_HANDLE;
			vertexShaderModule = shader.getVertexShaderModule();
			fragmentShaderModule = shader.getFragmentShaderModule();
			geometryShaderModule = shader.getGeometryShaderModule();
			std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
			if (vertexShaderModule != VK_NULL_HANDLE) {
				VkPipelineShaderStageCreateInfo vertexShaderStageInfo{};
				vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
				vertexShaderStageInfo.module = vertexShaderModule;
				vertexShaderStageInfo.pName = "main";
				shaderStages.push_back(vertexShaderStageInfo);
			}
			if (geometryShaderModule != VK_NULL_HANDLE) {
				VkPipelineShaderStageCreateInfo geometryShaderStageInfo{};
				geometryShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				geometryShaderStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
				geometryShaderStageInfo.module = geometryShaderModule;
				geometryShaderStageInfo.pName = "main";
				shaderStages.push_back(geometryShaderStageInfo);
			}
			if (fragmentShaderModule != VK_NULL_HANDLE) {
				VkPipelineShaderStageCreateInfo fragmentShaderStageInfo{};
				fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
				fragmentShaderStageInfo.module = fragmentShaderModule;
				fragmentShaderStageInfo.pName = "main";
				shaderStages.push_back(fragmentShaderStageInfo);
			}
			VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
			auto bindingDescription = Vertex::getBindingDescription();
			auto attributeDescriptions = Vertex::getAttributeDescriptions();
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputInfo.vertexBindingDescriptionCount = 1;
			vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
			vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
			vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
			VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
			inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			VkPrimitiveTopology modes[] = { VK_PRIMITIVE_TOPOLOGY_POINT_LIST, VK_PRIMITIVE_TOPOLOGY_LINE_LIST, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN };
			inputAssembly.topology = modes[primitveType];
			inputAssembly.primitiveRestartEnable =
				(inputAssembly.topology == VK_PRIMITIVE_TOPOLOGY_LINE_STRIP ||
					inputAssembly.topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP ||
					inputAssembly.topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN)
				? VK_TRUE : VK_FALSE;
			VkPipelineViewportStateCreateInfo viewportState{};
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.viewportCount = MAX_SCISSORS_AND_VIEWPORTS;
			viewportState.scissorCount = MAX_SCISSORS_AND_VIEWPORTS;
			std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
			};
			VkPipelineDynamicStateCreateInfo dynamicState{};
			dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
			dynamicState.pDynamicStates = dynamicStates.data();

			VkPipelineRasterizationStateCreateInfo rasterizer{};
			rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer.depthClampEnable = VK_FALSE;
			rasterizer.rasterizerDiscardEnable = VK_FALSE;
			rasterizer.polygonMode = polygonMode;
			rasterizer.lineWidth = 1.0f;
			rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
			rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

			VkPipelineMultisampleStateCreateInfo multisampling{};
			multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

			VkPipelineColorBlendAttachmentState colorBlendAttachment{};
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = VK_FALSE;


			colorBlendAttachment.srcColorBlendFactor = factorToVkConstant(blendMode.colorSrcFactor);
			colorBlendAttachment.dstColorBlendFactor = factorToVkConstant(blendMode.colorDstFactor);
			colorBlendAttachment.colorBlendOp = equationToVkConstant(blendMode.colorEquation);


			colorBlendAttachment.srcAlphaBlendFactor = factorToVkConstant(blendMode.alphaSrcFactor);
			colorBlendAttachment.dstAlphaBlendFactor = factorToVkConstant(blendMode.alphaDstFactor);
			colorBlendAttachment.alphaBlendOp = equationToVkConstant(blendMode.alphaEquation);

			VkPipelineColorBlendStateCreateInfo colorBlending{};
			colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlending.logicOpEnable = VK_FALSE;
			colorBlending.logicOp = VK_LOGIC_OP_COPY;
			colorBlending.attachmentCount = 1;
			colorBlending.pAttachments = &colorBlendAttachment;
			colorBlending.blendConstants[0] = 0.0f;
			colorBlending.blendConstants[1] = 0.0f;
			colorBlending.blendConstants[2] = 0.0f;
			colorBlending.blendConstants[3] = 0.0f;
			VkPipelineLayoutCreateInfo pipelineLayoutInfos{};
			pipelineLayoutInfos.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
			for (unsigned int i = 0; i < setLayouts.size(); i++) {
				descriptorSetLayouts.push_back(setLayouts[i].getHandle());
			}
			pipelineLayoutInfos.setLayoutCount = descriptorSetLayouts.size();
			pipelineLayoutInfos.pSetLayouts = descriptorSetLayouts.data();
			if (pushConstants.size() > 0) {
				pipelineLayoutInfos.pPushConstantRanges = pushConstants.data();
				pipelineLayoutInfos.pushConstantRangeCount = pushConstants.size();
			}
			if (vkCreatePipelineLayout(device.getDevice(), &pipelineLayoutInfos, nullptr, &pipelineLayout) != VK_SUCCESS) {

				throw std::runtime_error("failed to create pipeline layout!");
			}
			//std::cout<<"shader stages : "<<shaderStages.size()<<std::endl;
			VkGraphicsPipelineCreateInfo pipelineInfo{};
			pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineInfo.stageCount = shaderStages.size();
			pipelineInfo.pStages = shaderStages.data();
			pipelineInfo.pVertexInputState = &vertexInputInfo;
			pipelineInfo.pInputAssemblyState = &inputAssembly;
			pipelineInfo.pViewportState = &viewportState;
			pipelineInfo.pRasterizationState = &rasterizer;
			pipelineInfo.pMultisampleState = &multisampling;
			pipelineInfo.pColorBlendState = &colorBlending;
			pipelineInfo.pDynamicState = &dynamicState;
			pipelineInfo.layout = pipelineLayout;
			pipelineInfo.renderPass = renderPass.getHandle();
			pipelineInfo.subpass = 0;
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
			pipelineInfo.pDepthStencilState = &depthStencil;
			if (vkCreateGraphicsPipelines(device.getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
				throw std::runtime_error("failed to create graphics pipeline!");
			}
			shader.cleanupShaderModules();
		}
		void Pipeline::createComputePipeline(Shader& shader, std::deque<DescriptorSetLayout>& setLayouts, std::vector<VkPushConstantRange> pushConstants) {
			if (pipeline != VK_NULL_HANDLE) {
				cleanup();
			}
			shader.createComputeShaderModule();
			VkShaderModule computeShaderModule = shader.getComputeShaderModule();
			VkPipelineShaderStageCreateInfo shaderStageInfo{};
			shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
			shaderStageInfo.module = computeShaderModule;
			shaderStageInfo.pName = "main";
			VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
			for (unsigned int i = 0; i < setLayouts.size(); i++) {
				descriptorSetLayouts.push_back(setLayouts[i].getHandle());
			}
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
			pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
			if (pushConstants.size() > 0) {
				pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();
				pipelineLayoutInfo.pushConstantRangeCount = pushConstants.size();
			}
			if (vkCreatePipelineLayout(device.getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
				throw std::runtime_error("failed to create compute pipeline layout!");
			}
			VkComputePipelineCreateInfo pipelineInfo{};
			pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
			pipelineInfo.layout = pipelineLayout;
			pipelineInfo.stage = shaderStageInfo;

			if (vkCreateComputePipelines(device.getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
				throw std::runtime_error("failed to create compute pipeline!");
			}
			shader.cleanupComputeShaderModule();
		}
		void Pipeline::cleanup() {
			if (pipeline != VK_NULL_HANDLE) {
				vkDestroyPipelineLayout(device.getDevice(), pipelineLayout, nullptr);
				vkDestroyPipeline(device.getDevice(), pipeline, nullptr);
				pipeline = VK_NULL_HANDLE;
				pipelineLayout = VK_NULL_HANDLE;
			}
		}
		VkPipeline Pipeline::getHandle() {
			return pipeline;
		}
		VkPipelineLayout Pipeline::getLayout() {
			return pipelineLayout;
		}		
		Pipeline::~Pipeline() {
			cleanup();
		}		
	}
}