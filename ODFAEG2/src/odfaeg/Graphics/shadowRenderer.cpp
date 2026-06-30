module;
#include <array>
#include <vector>
#include <string>
#include <vulkan/vulkan.h>
#include <odfaeg/config.hpp>
#include <stdexcept>
#include <iostream>
#include <memory>
#include <mutex>
#include "../../../external/vma/vk_mem_alloc.h"
import odfaeg.graphic.shadowRenderer;
module odfaeg.graphic.shadowRenderer;
import odfaeg.graphic.camera;
import odfaeg.graphic.descriptor;
import odfaeg.math.maths;
import odfaeg.graphic.gpuContext;
import odfaeg.graphic.blendMode;
import odfaeg.graphic.primitiveType;
import odfaeg.graphic.device;
import odfaeg.graphic.material;
import odfaeg.core.delegate;
import odfaeg.window.command;
import odfaeg.graphic.renderStates;
import odfaeg.graphic.texture;
namespace odfaeg {
    namespace graphic {
        ShadowRenderer::ShadowRenderer(RenderTarget& parentRenderer, unsigned int layer, std::string typesToRenderExpression, bool useThread) : parentRenderer(parentRenderer),
        shadowMap(GPUContext::instance().getDevice(), true), 
        shadowMapPL(GPUContext::instance().getDevice(), true),
        shadowPassCSMShader(GPUContext::instance().getDevice()), shadowMappingCSMShader(GPUContext::instance().getDevice()),  
        shadowPassPLShader(GPUContext::instance().getDevice()), shadowMappingPLShader(GPUContext::instance().getDevice()),      
        shadowPassCommandPool(GPUContext::instance().getDevice()),
        shadowMappingCommandPool(GPUContext::instance().getDevice()),   
        shadowPassCSMPipeline(GPUContext::instance().getGraphicsPipeline(shadowPassCSMShader)),
        shadowMappingCSMPipeline(GPUContext::instance().getGraphicsPipeline(shadowMappingCSMShader)), 
        shadowPassCSMSets(GPUContext::instance().getDescriptorSets(shadowPassCSMShader)),
        shadowMappingCSMSets(GPUContext::instance().getDescriptorSets(shadowMappingCSMShader)),
        shadowPassPLPipeline(GPUContext::instance().getGraphicsPipeline(shadowPassPLShader)),
        shadowMappingPLPipeline(GPUContext::instance().getGraphicsPipeline(shadowMappingPLShader)),
        shadowPassPLSets(GPUContext::instance().getDescriptorSets(shadowPassPLShader)),
        shadowMappingPLSets(GPUContext::instance().getDescriptorSets(shadowMappingPLShader)),
        threadPool(6),
        useThread(useThread)
        {
            rendererReady.store(false);    
            //shadowMapPL.createCubeMap(std::max(parentRenderer.getSize().x(), parentRenderer.getSize().y()), true, false);              
            //std::cout<<"pl created"<<std::endl;
            //std::cout<<"addess : "<<&parentRenderer<<std::endl;
            shadowMap.create(parentRenderer.getSize().x(), parentRenderer.getSize().y(), NB_CASCADES+1, true, 0b00011111, true);
            shadowMapPL.createCubeMap(std::max(parentRenderer.getSize().x(), parentRenderer.getSize().y()), true, false);            
            std::vector<float> shadowCascadeLevels = computeSplits(NB_CASCADES, parentRenderer.getCamera().getViewport().getPosition().z(), parentRenderer.getCamera().getViewport().getSize().z(), 0.5f);
            for (size_t i = 1; i < shadowCascadeLevels.size(); i++)
            {

                fLightSpaceMatrices.push_back(getLightSpaceMatrix(shadowCascadeLevels[i-1], shadowCascadeLevels[i]));
                //std::cout<<fLightSpaceMatrices.back()<<std::endl;
            }            
            fLightSpaceMatrices.push_back(fLightSpaceMatrices.back());
            /*int i;
            std::cin>>i;*/
            std::string shaderDir = std::string(ODFAEG_INSTALL_DIR) + "/Shader";
            if (!shadowPassCSMShader.loadFromFile(shaderDir + "/shadowPassCSM.vert", shaderDir + "/shadowPassCSM.frag")) {
                throw std::runtime_error("Failed to load shadow pass csm shader");
            }
            if (!shadowMappingCSMShader.loadFromFile(shaderDir + "/shadowMappingCSM.vert", shaderDir + "/shadowMappingCSM.frag")) {
                throw std::runtime_error("Failed to load shadow mapping csm shader");
            }      
            if (!shadowPassPLShader.loadFromFile(shaderDir + "/shadowPassPL.vert", shaderDir + "/shadowPassPL.frag")) {
                throw std::runtime_error("Failed to load shadow pass csm shader");
            }
            if (!shadowMappingPLShader.loadFromFile(shaderDir + "/shadowMappingPL.vert", shaderDir + "/shadowMappingPL.frag")) {
                throw std::runtime_error("Failed to load shadow mapping csm shader");
            }                   
            CascadeData cascadeData;
            /*std::cout<<"size : "<<shadowCascadeLevels.size()<<std::endl;
            int i;
            std::cin>>i;*/
            for (unsigned int i = 0; i < shadowCascadeLevels.size(); i++) {
                cascadeData.cascadePlaneDistances[i].x() = shadowCascadeLevels[i];
                //std::cout<<"cascade data : "<<cascadeData.cascadePlaneDistances[i]<<std::endl;
            }
            cascadeData.cascadeCount = NB_CASCADES;
            /*int pause;
            std::cin>>pause;*/
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                lightSpaceMatricesBuffer.emplace_back(GPUContext::instance().getDevice());
                lightSpaceMatricesBuffer.back().create(sizeof(math::Matrix4f) * fLightSpaceMatrices.size(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
                lightSpaceMatricesBuffer.back().update(fLightSpaceMatrices.data(), sizeof(math::Matrix4f)*fLightSpaceMatrices.size());                
            }
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                cascadePlaneDistancesBuffer.emplace_back(GPUContext::instance().getDevice());
                cascadePlaneDistancesBuffer.back().create(sizeof(CascadeData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
                cascadePlaneDistancesBuffer.back().update(&cascadeData, sizeof(CascadeData));
            }  
            Camera pointLightCamera;
            math::Vec3f lightPos(10, 10, 0);
            pointLightCamera.setPerspective(90, 1, 1, 25);
            shadowPassPLVertPC.lightProjMatrix = pointLightCamera.getProjMatrix().getMatrix().transpose();
            pointLightCamera.setCenter(lightPos);
            pointLightCamera.lookAt(-1, 0, 0, math::Vec3f(0, -1, 0));
            lightViewsPLMatrices[0] = pointLightCamera.getViewMatrix().getMatrix().transpose();
            pointLightCamera.lookAt(1, 0, 0, math::Vec3f(0, -1, 0));
            lightViewsPLMatrices[1] = pointLightCamera.getViewMatrix().getMatrix().transpose();
            pointLightCamera.lookAt(0, -1, 0, math::Vec3f(0, -1, 0));
            lightViewsPLMatrices[2] = pointLightCamera.getViewMatrix().getMatrix().transpose();
            pointLightCamera.lookAt(0, 1, 0, math::Vec3f(0, -1, 0));
            lightViewsPLMatrices[3] = pointLightCamera.getViewMatrix().getMatrix().transpose();
            pointLightCamera.lookAt(0, 0, -1, math::Vec3f(0, -1, 0));
            lightViewsPLMatrices[4] = pointLightCamera.getViewMatrix().getMatrix().transpose();
            pointLightCamera.lookAt(0, 0, 1, math::Vec3f(0, -1, 0));
            lightViewsPLMatrices[5] = pointLightCamera.getViewMatrix().getMatrix().transpose();
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {                                
                lightViewsPLMatricesBuffer.emplace_back(GPUContext::instance().getDevice());
                lightViewsPLMatricesBuffer.back().create(sizeof(lightViewsPLMatrices), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
                lightViewsPLMatricesBuffer.back().update(lightViewsPLMatrices.data(),sizeof(lightViewsPLMatrices));
            }
            shadowMap.setCamera(parentRenderer.getCamera());
            createDescriptorsAndPipelines();
            createCommandPools();
            
            window::Command rendererReadyCmd(core::FastDelegate<bool>(&ShadowRenderer::isRendererReady, this), core::FastDelegate<void>(&ShadowRenderer::drawNextFrame, this));
            eventListener.connect("RendererReady",rendererReadyCmd); 
            if (useThread) {
                //std::cout<<"lanch"<<std::endl;
                eventListener.launch();
            }           
            if (GPUContext::instance().getSharedSemaphore(0).empty()) {
                GPUContext::instance().getSharedSemaphore(0).emplace_back(GPUContext::instance().getDevice());
                GPUContext::instance().getSharedSemaphore(0)[0].create(true, 0);
            }
            needToUpdateDescriptorSets = true;
            stop.store(false);
            rendererReady.store(true);
            
        }
        void ShadowRenderer::createDescriptorsAndPipelines() {
            DescriptorSetLayout& shadowPassLayout = GPUContext::instance().getDescriptorSetLayout(shadowPassCSMShader, 2);
            shadowPassLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES, VK_SHADER_STAGE_VERTEX_BIT);
            shadowPassLayout.updateLayout(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_VERTEX_BIT);
            shadowPassLayout.update();
            VkPipelineRenderingCreateInfo renderingCreateInfo{};
            renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
            renderingCreateInfo.colorAttachmentCount = 0;
            renderingCreateInfo.pColorAttachmentFormats = nullptr;
            renderingCreateInfo.depthAttachmentFormat = shadowMap.getDepthStencilTexture().getFormat();
            renderingCreateInfo.viewMask = shadowMap.getViewMask();
            BlendMode blendMode;
            blendMode.updateIds();
            std::vector<VkPushConstantRange> pushConstants;
            VkPushConstantRange pushConstant;
            pushConstant.offset = 0;
            pushConstant.size = sizeof(PushConstantData);
            pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            pushConstants.push_back(pushConstant);
            //std::cout<<"create pipelines"<<std::endl;
            //shadowPassCSMPipeline.resize(NB_PRIMITIVE_TYPES);
            for (unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {                
                /*if (shadowPassCSMPipeline[i].size() <= RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id) {
                    for (unsigned int j = 0; j < RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id+1; j++) {
                        shadowPassCSMPipeline[i].emplace_back(std::make_unique<Pipeline>(GPUContext::instance().getDevice()));
                    }
                }
                shadowPassCSMPipeline[i][RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id]->createGraphicPipeline(shadowPassCSMShader, static_cast<PrimitiveType>(i), GPUContext::instance().getDescriptorSetLayout(shadowPassCSMShader), renderingCreateInfo, shadowMap.getDepthStencilInfos()[RenderTarget::DEPTHNOSTENCIL], blendMode, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, pushConstants);*/
              
                GPUContext::instance().getGraphicsPipeline(static_cast<PrimitiveType>(i), shadowPassCSMShader, blendMode, RenderTarget::DEPTHNOSTENCIL).createGraphicPipeline(shadowPassCSMShader, static_cast<PrimitiveType>(i), GPUContext::instance().getDescriptorSetLayout(shadowPassCSMShader), renderingCreateInfo, shadowMap.getDepthStencilInfos()[RenderTarget::DEPTHNOSTENCIL], blendMode, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, pushConstants);
                /*std::cout<<"ids : "<<i<<","<<shadowPassCSMShader.getId()<<","<<RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id<<std::endl;
                std::cout<<"pipeline at creation : "<<shadowPassCSMPipeline[i][RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id]->getHandle()<<std::endl;
                std::cout<<"pipeline layout at creation : "<<shadowPassCSMPipeline[i][RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id]->getLayout()<<std::endl;*/
                //std::cout<<"pipeline created"<<std::endl;
            }
            DescriptorSetLayout& shadowMappingLayout = GPUContext::instance().getDescriptorSetLayout(shadowMappingCSMShader, 6);
            shadowMappingLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES, VK_SHADER_STAGE_VERTEX_BIT);
            shadowMappingLayout.updateLayout(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES, VK_SHADER_STAGE_FRAGMENT_BIT);
            shadowMappingLayout.updateLayout(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_FRAGMENT_BIT);
            shadowMappingLayout.updateLayout(3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_FRAGMENT_BIT);
            shadowMappingLayout.updateLayout(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
            shadowMappingLayout.updateLayout(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
            shadowMappingLayout.update();
            pushConstants.clear();
            VkPushConstantRange vertexPCRange;
            vertexPCRange.offset = 0;
            vertexPCRange.size = sizeof(ViewProjMatPC);
            vertexPCRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            pushConstants.push_back(vertexPCRange);
            VkPushConstantRange fragmentPCRange;
            fragmentPCRange.offset = sizeof(ViewProjMatPC);
            fragmentPCRange.size = sizeof(PushConstantData2);
            fragmentPCRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            pushConstants.push_back(fragmentPCRange);
            //shadowMappingCSMPipeline.resize(NB_PRIMITIVE_TYPES);
            renderingCreateInfo.colorAttachmentCount = 1;
            renderingCreateInfo.pColorAttachmentFormats = &parentRenderer.getImageFormat();
            renderingCreateInfo.depthAttachmentFormat = parentRenderer.getDepthStencilTexture().getFormat();
            for (unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {
                /*if (shadowMappingCSMPipeline[i].size() <= RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id) {
                    for (unsigned int j = 0; j < RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id+1; j++) {
                        shadowMappingCSMPipeline[i].emplace_back(std::make_unique<Pipeline>(GPUContext::instance().getDevice()));
                    }
                }
                shadowMappingCSMPipeline[i][RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id]->createGraphicPipeline(shadowMappingCSMShader, static_cast<PrimitiveType>(i), GPUContext::instance().getDescriptorSetLayout(shadowMappingCSMShader), renderingCreateInfo, parentRenderer.getDepthStencilInfos()[RenderTarget::DEPTHNOSTENCIL], blendMode, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, pushConstants);*/
                GPUContext::instance().getGraphicsPipeline(static_cast<PrimitiveType>(i), shadowMappingCSMShader, blendMode, RenderTarget::DEPTHNOSTENCIL).createGraphicPipeline(shadowMappingCSMShader, static_cast<PrimitiveType>(i), GPUContext::instance().getDescriptorSetLayout(shadowMappingCSMShader), renderingCreateInfo, parentRenderer.getDepthStencilInfos()[RenderTarget::DEPTHNOSTENCIL], blendMode, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, pushConstants);
            }
            DescriptorPool& shadowPassPool = GPUContext::instance().getDescriptorPool(shadowPassCSMShader, 2);
            shadowPassPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES);
            shadowPassPool.updatePoolSize(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,MAX_FRAMES_IN_FLIGHT);
            shadowPassPool.update();
            DescriptorPool& shadowMappingPool = GPUContext::instance().getDescriptorPool(shadowMappingCSMShader, 6);
            shadowMappingPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES);
            shadowMappingPool.updatePoolSize(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES);
            shadowMappingPool.updatePoolSize(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT);
            shadowMappingPool.updatePoolSize(3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT);
            shadowMappingPool.updatePoolSize(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
            shadowMappingPool.updatePoolSize(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
            shadowMappingPool.update();
            /*shadowPassCSMSets.resize(1);
            shadowPassCSMSets[0].emplace_back(std::make_unique<DescriptorSet>(GPUContext::instance().getDevice()));
            shadowPassCSMSets[0][0]->setNbBindings(2);*/
            DescriptorSet::allocate(shadowPassPool, shadowPassLayout, GPUContext::instance().getDescriptorSets(shadowPassCSMShader, 2, 1));
            /*shadowMappingCSMSets.resize(1);
            shadowMappingCSMSets[0].emplace_back(std::make_unique<DescriptorSet>(GPUContext::instance().getDevice()));*/
            
            DescriptorSet::allocate(shadowMappingPool, shadowMappingLayout, GPUContext::instance().getDescriptorSets(shadowMappingCSMShader, 6, 1), MAX_TEXTURES);
            
            DescriptorSetLayout& shadowPassPLLayout = GPUContext::instance().getDescriptorSetLayout(shadowPassPLShader, 2);
            shadowPassPLLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES, VK_SHADER_STAGE_VERTEX_BIT);
            shadowPassPLLayout.updateLayout(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_VERTEX_BIT);
            shadowPassPLLayout.update();
            
            renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
            renderingCreateInfo.colorAttachmentCount = 0;
            renderingCreateInfo.pColorAttachmentFormats = nullptr;
            renderingCreateInfo.depthAttachmentFormat = shadowMapPL.getDepthStencilTexture().getFormat();
            renderingCreateInfo.viewMask = shadowMapPL.getViewMask();
            
            pushConstants.clear();
            vertexPCRange.offset = 0;
            vertexPCRange.size = sizeof(ShadowPassPLVertPC);
            vertexPCRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            pushConstants.push_back(vertexPCRange);
            fragmentPCRange.offset = sizeof(ShadowPassPLVertPC);
            fragmentPCRange.size = sizeof(ShadowPassPLFragPC);
            fragmentPCRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            pushConstants.push_back(fragmentPCRange);
            //std::cout<<"create pipelines"<<std::endl;
            //shadowPassCSMPipeline.resize(NB_PRIMITIVE_TYPES);
            for (unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {                
                /*if (shadowPassCSMPipeline[i].size() <= RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id) {
                    for (unsigned int j = 0; j < RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id+1; j++) {
                        shadowPassCSMPipeline[i].emplace_back(std::make_unique<Pipeline>(GPUContext::instance().getDevice()));
                    }
                }
                shadowPassCSMPipeline[i][RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id]->createGraphicPipeline(shadowPassCSMShader, static_cast<PrimitiveType>(i), GPUContext::instance().getDescriptorSetLayout(shadowPassCSMShader), renderingCreateInfo, shadowMap.getDepthStencilInfos()[RenderTarget::DEPTHNOSTENCIL], blendMode, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, pushConstants);*/
              
                GPUContext::instance().getGraphicsPipeline(static_cast<PrimitiveType>(i), shadowPassPLShader, blendMode, RenderTarget::DEPTHNOSTENCIL).createGraphicPipeline(shadowPassPLShader, static_cast<PrimitiveType>(i), GPUContext::instance().getDescriptorSetLayout(shadowPassPLShader), renderingCreateInfo, shadowMap.getDepthStencilInfos()[RenderTarget::DEPTHNOSTENCIL], blendMode, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, pushConstants);
                /*std::cout<<"ids : "<<i<<","<<shadowPassCSMShader.getId()<<","<<RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id<<std::endl;
                std::cout<<"pipeline at creation : "<<shadowPassCSMPipeline[i][RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id]->getHandle()<<std::endl;
                std::cout<<"pipeline layout at creation : "<<shadowPassCSMPipeline[i][RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id]->getLayout()<<std::endl;*/
                //std::cout<<"pipeline created"<<std::endl;
            }
            renderingCreateInfo.colorAttachmentCount = 1;
            renderingCreateInfo.pColorAttachmentFormats = &parentRenderer.getImageFormat();
            renderingCreateInfo.depthAttachmentFormat = parentRenderer.getDepthStencilTexture().getFormat();
            pushConstants.clear();
            vertexPCRange.offset = 0;
            vertexPCRange.size = sizeof(ViewProjMatPC);
            vertexPCRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            pushConstants.push_back(vertexPCRange);
            fragmentPCRange.offset = sizeof(ViewProjMatPC);
            fragmentPCRange.size = sizeof(ShadowPassPLFragPC);
            fragmentPCRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            pushConstants.push_back(fragmentPCRange);
            DescriptorSetLayout& shadowMappingPLLayout = GPUContext::instance().getDescriptorSetLayout(shadowMappingPLShader, 4);
            shadowMappingLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES, VK_SHADER_STAGE_VERTEX_BIT);
            shadowMappingLayout.updateLayout(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES, VK_SHADER_STAGE_FRAGMENT_BIT);
            shadowMappingLayout.updateLayout(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
            shadowMappingLayout.updateLayout(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
            shadowPassPLLayout.update();
            for (unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {                
                /*if (shadowPassCSMPipeline[i].size() <= RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id) {
                    for (unsigned int j = 0; j < RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id+1; j++) {
                        shadowPassCSMPipeline[i].emplace_back(std::make_unique<Pipeline>(GPUContext::instance().getDevice()));
                    }
                }
                shadowPassCSMPipeline[i][RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id]->createGraphicPipeline(shadowPassCSMShader, static_cast<PrimitiveType>(i), GPUContext::instance().getDescriptorSetLayout(shadowPassCSMShader), renderingCreateInfo, shadowMap.getDepthStencilInfos()[RenderTarget::DEPTHNOSTENCIL], blendMode, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, pushConstants);*/
              
                GPUContext::instance().getGraphicsPipeline(static_cast<PrimitiveType>(i), shadowMappingPLShader, blendMode, RenderTarget::DEPTHNOSTENCIL).createGraphicPipeline(shadowMappingPLShader, static_cast<PrimitiveType>(i), GPUContext::instance().getDescriptorSetLayout(shadowMappingPLShader), renderingCreateInfo, shadowMap.getDepthStencilInfos()[RenderTarget::DEPTHNOSTENCIL], blendMode, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, pushConstants);
                /*std::cout<<"ids : "<<i<<","<<shadowPassCSMShader.getId()<<","<<RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id<<std::endl;
                std::cout<<"pipeline at creation : "<<shadowPassCSMPipeline[i][RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id]->getHandle()<<std::endl;
                std::cout<<"pipeline layout at creation : "<<shadowPassCSMPipeline[i][RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id]->getLayout()<<std::endl;*/
                //std::cout<<"pipeline created"<<std::endl;
            }
            DescriptorPool& shadowPassPLPool = GPUContext::instance().getDescriptorPool(shadowPassPLShader, 2);
            shadowPassPLPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES);
            shadowPassPLPool.updatePoolSize(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,MAX_FRAMES_IN_FLIGHT);
            shadowPassPLPool.update();
            DescriptorPool& shadowMappingPLPool = GPUContext::instance().getDescriptorPool(shadowMappingPLShader, 4);
            shadowMappingPLPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES);
            shadowMappingPLPool.updatePoolSize(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES);
            shadowMappingPLPool.updatePoolSize(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
            shadowMappingPLPool.updatePoolSize(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
            shadowMappingPLPool.update();
            /*shadowPassCSMSets.resize(1);
            shadowPassCSMSets[0].emplace_back(std::make_unique<DescriptorSet>(GPUContext::instance().getDevice()));
            shadowPassCSMSets[0][0]->setNbBindings(2);*/
            DescriptorSet::allocate(shadowPassPLPool, shadowPassPLLayout, GPUContext::instance().getDescriptorSets(shadowPassPLShader, 2, 1));
            /*shadowMappingCSMSets.resize(1);
            shadowMappingCSMSets[0].emplace_back(std::make_unique<DescriptorSet>(GPUContext::instance().getDevice()));*/
            
            DescriptorSet::allocate(shadowMappingPLPool, shadowMappingPLLayout, GPUContext::instance().getDescriptorSets(shadowMappingPLShader, 4, 1), MAX_TEXTURES);
        }
        std::array<math::Vec3f, 8> ShadowRenderer::getFrustrumCornersWordlSpace(math::Matrix4f projView) {
            std::array<math::Vec3f, 8> frustrumCorners = {
                math::Vec3f(-1.f, -1.f, 0.f),
                math::Vec3f(1.f, -1.f, 0.f),
                math::Vec3f(1.f,  1.f, 0.f),
                math::Vec3f(-1.f,  1.f, 0.f),
                math::Vec3f(-1.f, -1.f, 1.f),
                math::Vec3f(1.f, -1.f, 1.f),
                math::Vec3f(1.f,  1.f, 1.f),
                math::Vec3f(-1.f,  1.f, 1.f)
            };
            //std::cout<<"inverse : "<<projView.inverse()<<std::endl;
            for (unsigned int i = 0; i < frustrumCorners.size(); i++) {
                frustrumCorners[i] = projView.inverse() * frustrumCorners[i];
                //std::cout<<"corner : "<<frustrumCorners[i]<<std::endl;
            }
            return frustrumCorners;
        }
        math::Matrix4f ShadowRenderer::getLightSpaceMatrix(const float nearPlane, const float farPlane) {
            Camera camera = parentRenderer.getCamera();
            //std::cout<<"near/far : "<<nearPlane<<","<<farPlane<<std::endl;
            camera.setPerspective(80, shadowMap.getSize().x() / shadowMap.getSize().y(), nearPlane, farPlane);  
            //camera.setUp(math::Vec3f(0, -1, 0));          
            math::Matrix4f projView = camera.getProjMatrix().getMatrix() * camera.getViewMatrix().getMatrix();
            //std::cout<<"porj view : "<<projView<<std::endl;
            auto corners = getFrustrumCornersWordlSpace(projView);
            math::Vec3f center(0.f, 0.f, 0.f);
            for (const auto& v : corners) {
                
                center += v;
                
            }
            //std::cout<<"center : "<<center<<std::endl;
            center /= corners.size();
            //std::cout<<"center / 8 : "<<center<<std::endl;
            math::Vec3f lightDir = math::Vec3f(20.f, 50.f, 20.f);
            Camera lightSpace;
            lightSpace.setCenter(center + lightDir);
            lightSpace.lookAt(center.x(), center.y(), center.z(), math::Vec3f(0.f, -1.f, 0.f));

            float minX = std::numeric_limits<float>::max();
            float maxX = std::numeric_limits<float>::lowest();
            float minY = std::numeric_limits<float>::max();
            float maxY = std::numeric_limits<float>::lowest();
            float minZ = std::numeric_limits<float>::max();
            float maxZ = std::numeric_limits<float>::lowest();
            for (const auto& v : corners)
            {
                const auto trf = lightSpace.getViewMatrix().getMatrix() * v;
                /*std::cout<<"v : "<<v<<std::endl;
                std::cout<<"matrix : "<<lightSpace.getViewMatrix().getMatrix()<<std::endl;*/
                minX = std::min(minX, trf.x());
                maxX = std::max(maxX, trf.x());
                minY = std::min(minY, trf.y());
                maxY = std::max(maxY, trf.y());
                minZ = std::min(minZ, trf.z());
                maxZ = std::max(maxZ, trf.z());
            }
            constexpr float zMult = 10.0f;
            if (minZ < 0)
            {
                minZ *= zMult;
            }
            else
            {
                minZ /= zMult;
            }
            if (maxZ < 0)
            {
                maxZ /= zMult;
            }
            else
            {
                maxZ *= zMult;
            }
            
            lightSpace.setPerspective(minX, maxX, minY, maxY, minZ, maxZ);
           

            return lightSpace.getProjMatrix().getMatrix() * lightSpace.getViewMatrix().getMatrix();
        }
        std::vector<float> ShadowRenderer::computeSplits(int cascadeCount, float nearPlane, float farPlane, float lambda)
        {
            std::vector<float> splits(cascadeCount + 1);
            splits[0] = nearPlane;
            splits[cascadeCount] = farPlane;

            for (int i = 1; i < cascadeCount; i++)
            {
                float p = (float)i / cascadeCount;

                float logSplit = nearPlane * math::Math::power(farPlane / nearPlane, p);
                float uniSplit = nearPlane + (farPlane - nearPlane) * p;

                splits[i] = lambda * logSplit + (1.0f - lambda) * uniSplit;
                //std::cout<<"cascade count : "<<cascadeCount<<",split : "<<splits[i]<<std::endl;
                
            }            

            return splits;
        }
        void ShadowRenderer::createCommandPools() {
            Device::QueueFamilyIndices queueFamilyIndices = GPUContext::instance().getDevice().findQueueFamilies(GPUContext::instance().getDevice().getPhysicalDevice());
            shadowPassCommandPool.create(queueFamilyIndices.graphicsFamily.value());
            shadowPassCommandPool.createCommandBuffers(false, MAX_FRAMES_IN_FLIGHT);
            shadowMappingCommandPool.create(queueFamilyIndices.graphicsFamily.value());            
            shadowMappingCommandPool.createCommandBuffers(false, MAX_FRAMES_IN_FLIGHT);
        }
        void ShadowRenderer::updateDescriptorSets() {
            //std::cout<<"update descriptor sets"<<std::endl;
            DescriptorSet& shadowPassDescriptorSet = GPUContext::instance().getDescriptorSets(shadowPassCSMShader, 2, 1)[0];
            //std::cout<<"size : "<<shadowPassCSMSets.size()<<","<<shadowPassCSMSets[0].size()<<std::endl;
            //std::cout<<"id 2 : "<<(RenderTarget::OUTPUT_MODELS+shadowMap.getId()*RenderTarget::NB_BUFFERS)<<std::endl;
            shadowPassDescriptorSet.updateBufferInfos(0, GPUContext::instance().getSharedBuffers(RenderTarget::OUTPUT_MODELS+shadowMap.getId()*RenderTarget::NB_BUFFERS), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            shadowPassDescriptorSet.updateBufferInfos(1, lightSpaceMatricesBuffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
            shadowPassDescriptorSet.updateDescriptorSet();
            bool hasDiffuseTexture = GPUContext::instance().getSharedTextures(Material::DIFFUSE).size() != 0;
            DescriptorSet& shadowMappingDescriptorSet = GPUContext::instance().getDescriptorSets(shadowMappingCSMShader, (hasDiffuseTexture) ? 6 : 5, 1)[0];
            //shadowMappingCSMSets[0][0]->setNbBindings((hasDiffuseTexture) ? 6 : 5);
            shadowMap.getDepthStencilTexture().getImage(0).setLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
            shadowMap.getDepthStencilTexture().getImage(1).setLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
            shadowMappingDescriptorSet.updateBufferInfos(0, GPUContext::instance().getSharedBuffers(RenderTarget::OUTPUT_MODELS+parentRenderer.getId()*RenderTarget::NB_BUFFERS), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            shadowMappingDescriptorSet.updateBufferInfos(1, GPUContext::instance().getSharedBuffers(RenderTarget::OUTPUT_MATERIALS+parentRenderer.getId()*RenderTarget::NB_BUFFERS), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            shadowMappingDescriptorSet.updateBufferInfos(2, lightSpaceMatricesBuffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
            shadowMappingDescriptorSet.updateBufferInfos(3, cascadePlaneDistancesBuffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
            shadowMappingDescriptorSet.updateImageInfos(4, shadowMap.getDepthStencilTexture(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            shadowMap.getDepthStencilTexture().getImage(0).setLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
            shadowMap.getDepthStencilTexture().getImage(1).setLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
            if (hasDiffuseTexture) {
                shadowMappingDescriptorSet.updateImageInfos(5, GPUContext::instance().getSharedTextures(Material::DIFFUSE), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            }
            shadowMappingDescriptorSet.updateDescriptorSet();

            DescriptorSet& shadowPassPLDescriptorSet = GPUContext::instance().getDescriptorSets(shadowPassPLShader, 2, 1)[0];
            shadowPassPLDescriptorSet.updateBufferInfos(0, GPUContext::instance().getSharedBuffers(RenderTarget::OUTPUT_MODELS+shadowMapPL.getId()*RenderTarget::NB_BUFFERS), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            shadowPassPLDescriptorSet.updateBufferInfos(1, lightViewsPLMatricesBuffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
            shadowPassPLDescriptorSet.updateDescriptorSet();
            DescriptorSet& shadowMappingPLDescriptorSet = GPUContext::instance().getDescriptorSets(shadowMappingPLShader, (hasDiffuseTexture) ? 4 : 3, 1)[0];
            shadowMappingPLDescriptorSet.updateBufferInfos(0, GPUContext::instance().getSharedBuffers(RenderTarget::OUTPUT_MODELS+parentRenderer.getId()*RenderTarget::NB_BUFFERS), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            shadowMappingPLDescriptorSet.updateBufferInfos(1, GPUContext::instance().getSharedBuffers(RenderTarget::OUTPUT_MATERIALS+parentRenderer.getId()*RenderTarget::NB_BUFFERS), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);           
            shadowMappingPLDescriptorSet.updateImageInfos(2, shadowMapPL.getDepthStencilTexture(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); 
            if (hasDiffuseTexture) {
                shadowMappingPLDescriptorSet.updateImageInfos(3, GPUContext::instance().getSharedTextures(Material::DIFFUSE), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); 
            }
        }
        void ShadowRenderer::clear() {
            shadowMap.clear();
            parentRenderer.setTypesToRender(typesToRenderExpression, parentRenderer.getCurrentFrame());
            parentRenderer.applyCullingAndBatching();
            shadowMap.setTypesToRender(typesToRenderExpression, parentRenderer.getCurrentFrame());
            shadowMap.applyCullingAndBatching();
            shadowMapPL.setTypesToRender(typesToRenderExpression, parentRenderer.getCurrentFrame());
            shadowMapPL.applyCullingAndBatching();
            
            //std::cout<<"cleared  :"<<parentRenderer.getCurrentFrame()<<std::endl;
            registerFramesJob[parentRenderer.getCurrentFrame()].store(true);
            cv.notify_one();
        }
        void ShadowRenderer::drawNextFrame() {
            std::unique_lock<std::mutex> lock(mtx);
            //std::cout<<"frame : "<<parentRenderer.getCurrentFrame()<<std::endl;
            cv.wait(lock, [this] {
                    //std::cout<<"draw frame : "<<frameBuffer.getCurrentFrame()<<std::endl;
                return registerFramesJob[parentRenderer.getCurrentFrame()].load() || stop.load();
            });            
            uint32_t renderFrame  = parentRenderer.getCurrentFrame();
            //std::cout<<"draw!"<<std::endl;
            registerFramesJob[renderFrame].store(false);
            
            if (!stop.load()) {
                VkSemaphoreWaitInfo semaphoreWaitInfo = {};
                semaphoreWaitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
                std::vector<VkSemaphore> waitSemaphores;
                std::vector<uint64_t> waitValues;
                waitSemaphores.push_back(GPUContext::instance().getSharedSemaphore(0)[0].getHandle());
                waitSemaphores.push_back(shadowMap.getSemaphores()[0].getHandle());
                waitValues.push_back(GPUContext::instance().getSharedSemaphore(0)[0].getValue());
                waitValues.push_back(shadowMap.getSemaphores()[0].getValue());    
                semaphoreWaitInfo.semaphoreCount = waitSemaphores.size();
                semaphoreWaitInfo.pSemaphores = waitSemaphores.data();
                semaphoreWaitInfo.pValues = waitValues.data();
                vkWaitSemaphores(GPUContext::instance().getDevice().getDevice(), &semaphoreWaitInfo, UINT64_MAX);
                //std::cout<<"frame : "<<renderFrame<<" ready!"<<std::endl;
                if (needToUpdateDescriptorSets) {
                    //std::cout<<"update ds"<<std::endl;
                    updateDescriptorSets();
                    needToUpdateDescriptorSets = false;
                }                
                jobFences[renderFrame].reset(2);                
                threadPool.enqueue([this, renderFrame] {
                    VkCommandBufferInheritanceRenderingInfo inheritanceRenderingInfo{};
                    inheritanceRenderingInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO;
                    inheritanceRenderingInfo.colorAttachmentCount = 0;
                    inheritanceRenderingInfo.pColorAttachmentFormats = nullptr; // tableau de VkFormat
                    inheritanceRenderingInfo.depthAttachmentFormat = shadowMap.getDepthStencilTexture().getFormat();    // VK_FORMAT_D32_SFLOAT, etc.                    
                    inheritanceRenderingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
                    inheritanceRenderingInfo.viewMask = shadowMap.getViewMask();
                    VkCommandBufferInheritanceInfo inheritanceInfo{};
                    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                    inheritanceInfo.pNext = &inheritanceRenderingInfo;
                    inheritanceInfo.renderPass = VK_NULL_HANDLE;
                    inheritanceInfo.subpass = 0;
                    inheritanceInfo.framebuffer = VK_NULL_HANDLE;   
                    inheritanceInfo.occlusionQueryEnable = VK_FALSE;
                    inheritanceInfo.pipelineStatistics = 0;
                    inheritanceInfo.queryFlags = 0;                 
                    RenderStates states;
                    states.shader = &shadowPassCSMShader;
                    BlendMode blendMode;
                    std::vector<VkDescriptorSet> sets;
                    for (unsigned int i = 0; i < shadowPassCSMSets.size(); i++) {
                        sets.push_back(shadowPassCSMSets[i][0].getHandle());
                    }
                    blendMode.updateIds();
                    //std::cout<<"begin record cmd : "<<renderFrame<<std::endl;
                    shadowPassCommandPool.beginRecordCommandBuffer(renderFrame, inheritanceInfo);
                    pc.currentFrame = renderFrame;
                    for (unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {
                        pc.primitiveType = i;
                        //std::cout<<"ids : "<<i<<","<<shadowPassCSMShader.getId()<<","<<RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id<<std::endl;
                        /*std::cout<<"pipeline : "<<shadowPassCSMPipeline[i][RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id].getHandle()<<std::endl;
                        std::cout<<"pipeline layout : "<<shadowPassCSMPipeline[i][RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id].getLayout()<<std::endl; */                      
                        //std::cout<<"register binds"<<std::endl;
                        vkCmdBindPipeline(shadowPassCommandPool.getHandle(renderFrame), VK_PIPELINE_BIND_POINT_GRAPHICS, shadowPassCSMPipeline[i][RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id].getHandle());
                        //std::cout<<"registered bind pipeline"<<std::endl;
                        vkCmdPushConstants(shadowPassCommandPool.getHandle(renderFrame), shadowPassCSMPipeline[i][RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id].getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstantData), &pc);
                        //std::cout<<"registed bind push constants"<<std::endl;
                        vkCmdBindDescriptorSets(shadowPassCommandPool.getHandle(renderFrame), VK_PIPELINE_BIND_POINT_GRAPHICS, shadowPassCSMPipeline[i][RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id].getLayout(), 0, sets.size(), sets.data(), 0, nullptr);
                        //std::cout<<"registered bind decriptor sets"<<std::endl;
                        shadowMap.draw(shadowPassCommandPool, static_cast<PrimitiveType>(i), states);
                        //std::cout<<"registered"<<std::endl;
                    }
                    shadowPassCommandPool.endRecordCommandBuffer(renderFrame);
                    //std::cout<<"render frame : "<<renderFrame<<std::endl;
                    jobFences[renderFrame].jobDone();
                });
                threadPool.enqueue([this, renderFrame] {
                    VkCommandBufferInheritanceRenderingInfo inheritanceRenderingInfo{};
                    inheritanceRenderingInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO;
                    inheritanceRenderingInfo.colorAttachmentCount = 1;
                    inheritanceRenderingInfo.pColorAttachmentFormats = &parentRenderer.getImageFormat(); // tableau de VkFormat
                    inheritanceRenderingInfo.depthAttachmentFormat = parentRenderer.getDepthStencilTexture().getFormat();    // VK_FORMAT_D32_SFLOAT, etc.                    
                    inheritanceRenderingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
                    VkCommandBufferInheritanceInfo inheritanceInfo{};
                    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                    inheritanceInfo.pNext = &inheritanceRenderingInfo;
                    inheritanceInfo.renderPass = VK_NULL_HANDLE;
                    inheritanceInfo.subpass = 0;
                    inheritanceInfo.framebuffer = VK_NULL_HANDLE;
                    inheritanceInfo.occlusionQueryEnable = VK_FALSE;
                    inheritanceInfo.pipelineStatistics = 0;
                    inheritanceInfo.queryFlags = 0;                
                    RenderStates states;
                    states.shader = &shadowMappingCSMShader;
                    BlendMode blendMode;
                    std::vector<VkDescriptorSet> sets;
                    for (unsigned int i = 0; i < shadowMappingCSMSets.size(); i++) {
                        sets.push_back(shadowMappingCSMSets[i][0].getHandle());
                    }
                    blendMode.updateIds();
                    shadowMappingCommandPool.beginRecordCommandBuffer(renderFrame, inheritanceInfo);
                    pc2.lightDir = math::Vec3f(20, 50, 20);
                    pc2.view = parentRenderer.getCamera().getViewMatrix().getMatrix().transpose();
                    pc2.farPlane = parentRenderer.getCamera().getViewport().getSize().z();
                    viewProjMatPC.projMatrix = parentRenderer.getCamera().getProjMatrix().getMatrix().transpose();
                    viewProjMatPC.viewMatrix = parentRenderer.getCamera().getViewMatrix().getMatrix().transpose();
                    viewProjMatPC.currentFrame = parentRenderer.getCurrentFrame();                    
                    //std::cout << "sizeof(ViewProjMatPC) = " << sizeof(ViewProjMatPC) << std::endl;
                    for (unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {
                        viewProjMatPC.primitiveType = i;
                        vkCmdBindPipeline(shadowMappingCommandPool.getHandle(renderFrame), VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMappingCSMPipeline[i][RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id].getHandle());
                        vkCmdPushConstants(shadowMappingCommandPool.getHandle(renderFrame), shadowMappingCSMPipeline[i][RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id].getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ViewProjMatPC), &viewProjMatPC);
                        vkCmdPushConstants(shadowMappingCommandPool.getHandle(renderFrame), shadowMappingCSMPipeline[i][RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id].getLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(ViewProjMatPC), sizeof(PushConstantData2), &pc2);
                        //std::cout<<"bind sets"<<std::endl;
                        vkCmdBindDescriptorSets(shadowMappingCommandPool.getHandle(renderFrame), VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMappingCSMPipeline[i][RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id].getLayout(), 0, sets.size(), sets.data(), 0, nullptr);
                        parentRenderer.draw(shadowMappingCommandPool, static_cast<PrimitiveType>(i), states);
                    }
                    shadowMappingCommandPool.endRecordCommandBuffer(renderFrame);
                    //std::cout<<"render frame : "<<renderFrame<<std::endl;
                    jobFences[renderFrame].jobDone();
                });                
                jobFences[renderFrame].wait();
            }
            //std::cout<<"ready!"<<std::endl;
            commandBuffersReady[renderFrame].store(true);
            cv.notify_all();
        }
        void ShadowRenderer::draw() {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] {
                    //std::cout<<"draw frame : "<<frameBuffer.getCurrentFrame()<<std::endl;
                return commandBuffersReady[parentRenderer.getCurrentFrame()].load() || stop.load();
            });
            //std::cout<<"buffers ready!"<<std::endl;
            commandBuffersReady[parentRenderer.getCurrentFrame()].store(false);
            if (!stop.load()) {
                
                //std::cout<<"commands : !"<<parentRenderer.getCurrentFrame()<<" registered!"<<std::endl;
                //std::cout<<"draw shadow map"<<std::endl;
                shadowMap.applyComputeGraphicsBarrier();
                shadowMap.beginRendering(true);
                vkCmdExecuteCommands(shadowMap.getCommandPool().getHandle(shadowMap.getCurrentFrame()), 1, &shadowPassCommandPool.getHandle(parentRenderer.getCurrentFrame()));
                shadowMap.endRendering(); 
                //std::cout<<"submit shadow map"<<std::endl;
                shadowMap.submit(true); 
                //std::cout<<"shadow map curent frame : "<<shadowMap.getCurrentFrame()<<"window current frame : "<<parentRenderer.getCurrentFrame()<<std::endl;
                shadowMap.display();
                //std::cout<<"shadow map drawn"<<std::endl;
                                          
                parentRenderer.applyComputeGraphicsBarrier();
                VkMemoryBarrier memoryBarrier{};
                memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                memoryBarrier.pNext = VK_NULL_HANDLE;
                memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;  
                vkCmdPipelineBarrier(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                
                Texture::transitionImageLayout(shadowMap.getDepthStencilTexture().getImage(0), parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);      
                parentRenderer.beginRendering(true);
                vkCmdExecuteCommands(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), 1, &shadowMappingCommandPool.getHandle(parentRenderer.getCurrentFrame()));
                parentRenderer.endRendering(); 
                Texture::transitionImageLayout(shadowMap.getDepthStencilTexture().getImage(0), parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);     
                             
            }
            //std::cout<<"drawn"<<std::endl;
        }
        bool ShadowRenderer::isRendererReady() {
            return rendererReady.load();
        }
    }
}
