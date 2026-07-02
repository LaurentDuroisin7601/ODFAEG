module;
#include <deque>
#include <odfaeg/config.hpp>
#include <iostream>
#include <memory>
import odfaeg.graphic.gpuContext;
module odfaeg.graphic.gpuContext;
namespace odfaeg {
    namespace graphic {
        GPUContext::GPUContext() : device(inst) {

        }
        GPUContext& GPUContext::instance() {
            static GPUContext ctx;
            return ctx;
        }
        Pipeline& GPUContext::getComputePipeline(Shader& shader) {
			if (computePipelines.size() <= shader.getId()) {
				for (unsigned int i = computePipelines.size(); i < shader.getId() + 1; i++) {
					computePipelines.emplace_back(device);
				}
			}
        	return computePipelines[shader.getId()];
			//std::cout<<"compute : "<<computePipelines[shader.getId()].getHandle()<<","<<descriptorSetLayouts[shader.getId()][0].getHandle()<<std::endl;
		}
    	std::deque<std::deque<Pipeline>>& GPUContext::getGraphicsPipeline(Shader& shader) {
        	//std::cout<<"shader id : "<<shader.getId()<<std::endl;
        	if (graphicsPipeline.size() <=  shader.getId()) {
        		graphicsPipeline.resize(shader.getId() + 1);
        	}
        	return graphicsPipeline[shader.getId()];
        }
		Pipeline& GPUContext::getGraphicsPipeline(PrimitiveType primType, Shader& shader, BlendMode blendMode, unsigned int depthStencilId) {
			blendMode.updateIds();
			if (graphicsPipeline.size() <=  shader.getId()) {
				graphicsPipeline.resize(shader.getId() + 1);
			}
			if(graphicsPipeline[shader.getId()].size() <= primType) {
				graphicsPipeline[shader.getId()].resize(primType + 1);
			}
			for (unsigned int j = 0; j < primType+1; j++) {

				if (graphicsPipeline[shader.getId()][j].size() <= depthStencilId * blendMode.nbBlendModes + blendMode.id) {
					for (unsigned int k = graphicsPipeline[shader.getId()][j].size(); k < depthStencilId * blendMode.nbBlendModes + blendMode.id + 1; k++) {
						graphicsPipeline[shader.getId()][j].emplace_back(device);
					}
				}

			}			
        	//std::cout<<"pipeline ids : "<<shader.getId()<<","<<primType<<","<<depthStencilId * blendMode.nbBlendModes + blendMode.id<<std::endl;
			return graphicsPipeline[shader.getId()][primType][depthStencilId * blendMode.nbBlendModes + blendMode.id];
		}
    	DescriptorPool&  GPUContext::getDescriptorPool(Shader& shader, unsigned int nbShaderBindings, unsigned int setBinding) {
			if (descriptorPools.size() <= shader.getId()) {
				descriptorPools.resize(shader.getId() + 1);
			}
			//std::cout<<"shader id  : "<<shader.getId()+1<<std::endl;			
			//std::cout<<"size : "<<descriptorPools[i].size()<<std::endl;
			if (descriptorPools[shader.getId()].size() <= setBinding) {
				for (unsigned int j = descriptorPools[shader.getId()].size() ; j < setBinding + 1; j++) {
					//std::cout<<"create descriptor pool"<<std::endl;
					descriptorPools[shader.getId()].emplace_back(device);
				}
			}			
			descriptorPools[shader.getId()][setBinding].setNbBindings(nbShaderBindings, MAX_FRAMES_IN_FLIGHT);
			return descriptorPools[shader.getId()][setBinding];
		}
    	std::deque<DescriptorSetLayout>& GPUContext::getDescriptorSetLayout(Shader& shader) {
			if (descriptorSetLayouts.size() <= shader.getId()) {
				descriptorSetLayouts.resize(shader.getId() + 1);
			}
	        return descriptorSetLayouts[shader.getId()];
        }
		DescriptorSetLayout& GPUContext::getDescriptorSetLayout(Shader& shader, unsigned int nbShaderBindings, bool bindless, unsigned int setBinding) {
			if (descriptorSetLayouts.size() <= shader.getId()) {
				descriptorSetLayouts.resize(shader.getId() + 1);
			}
			if(descriptorSetLayouts[shader.getId()].size() <= setBinding) {
				for (unsigned int j = descriptorSetLayouts[shader.getId()].size(); j < setBinding + 1; j++) {
					descriptorSetLayouts[shader.getId()].emplace_back(device);
				}				
			}
			descriptorSetLayouts[shader.getId()][setBinding].setNbBindings(nbShaderBindings, bindless);
			return descriptorSetLayouts[shader.getId()][setBinding];
		}
		std::deque<DescriptorSet>& GPUContext::getDescriptorSets(Shader& shader, unsigned int nbShaderBindings, unsigned int nbDescriptorSetsPerFrame, unsigned int setBinding) {
			//std::cout<<"size 1 : "<<descriptorSets.size()<<" "<<shader.getId()<<std::endl;
			if (descriptorSets.size() <= shader.getId()) {
				descriptorSets.resize(shader.getId() + 1);
			}			
			if (descriptorSets[shader.getId()].size() <= setBinding) {
				descriptorSets[shader.getId()].resize(setBinding + 1);
			}				
			for (unsigned int j = 0; j < setBinding + 1; j++) {
				if (descriptorSets[shader.getId()][j].size() < nbDescriptorSetsPerFrame) {
					for (unsigned int k = descriptorSets[shader.getId()][j].size(); k < nbDescriptorSetsPerFrame; k++) {
						descriptorSets[shader.getId()][j].emplace_back(device);
					}
				}
			}
			//std::cout<<"nb bindiings ?  : "<<nbShaderBindings<<",shader id : "<<shader.getId()<<std::endl;
			for (unsigned int i = 0; i < descriptorSets[shader.getId()][setBinding].size(); i++) {
				descriptorSets[shader.getId()][setBinding][i].setNbBindings(nbShaderBindings);
			}
			return descriptorSets[shader.getId()][setBinding];
		}
    	std::deque<std::deque<DescriptorSet>>& GPUContext::getDescriptorSets(Shader& shader) {
			if (descriptorSets.size() <= shader.getId()) {
				descriptorSets.resize(shader.getId()+1);
			}
        	return descriptorSets[shader.getId()];
        }
    	std::deque<Buffer>& GPUContext::getSharedBuffers(unsigned int bufferID) {
			if (sharedBuffers.size() <= bufferID) {
				sharedBuffers.resize(bufferID + 1);
			}
        	return sharedBuffers[bufferID];
        }
    	std::deque<Texture>& GPUContext::getSharedTextures(unsigned int textureID) {
        	if (sharedTextures.size() <= textureID) {
        		sharedTextures.resize(textureID + 1);
        	}
        	return sharedTextures[textureID];
        }
    	std::deque<Image>& GPUContext::getSharedImage(unsigned int imageID) {
        	if (sharedImages.size() <= imageID) {
        		sharedImages.resize(imageID + 1);
        	}
        	return sharedImages[imageID];
        }
    	std::deque<VertexBuffer>& GPUContext::getSharedVertexBuffer(unsigned int vertexBufferID) {
        	if (sharedVertexBuffers.size() <= vertexBufferID) {
        		sharedVertexBuffers.resize(vertexBufferID + 1);
        	}
        	return sharedVertexBuffers[vertexBufferID];
        }
    	Device& GPUContext::getDevice() {
	        return device;
        }
		Instance& GPUContext::getInstance() {
			return inst;
		}
    	std::deque<Fence>& GPUContext::getSharedFence(unsigned int fenceID) {
	        if (sharedFences.size() <= fenceID) {
		        sharedFences.resize(fenceID + 1);
	        }
        	return sharedFences[fenceID];
        }
    	std::deque<Semaphore>& GPUContext::getSharedSemaphore(unsigned int semaphoreID) {
	        if (sharedSemaphores.size() <= semaphoreID) {
	        	sharedSemaphores.resize(semaphoreID + 1);
	        }
        	return sharedSemaphores[semaphoreID];
        }
    }
}