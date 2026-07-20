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
#include <odfaeg/config.hpp>
import odfaeg.graphic.shadowRenderer;
module odfaeg.graphic.shadowRenderer;
import odfaeg.graphic.camera;
import odfaeg.graphic.descriptor;
import odfaeg.math.maths;
import odfaeg.graphic.gpuContext;
import odfaeg.graphic.blendMode;
import odfaeg.entity.primitiveType;
import odfaeg.graphic.device;
import odfaeg.graphic.material;
import odfaeg.core.delegate;
import odfaeg.window.command;
import odfaeg.graphic.renderStates;
import odfaeg.graphic.texture;
import odfaeg.physic.boundingBox;
import odfaeg.entity.gameObject;
namespace odfaeg {
    namespace graphic {
        ShadowRenderer::ShadowRenderer(RenderTarget& parentRenderer, RenderTexture& sceneColorTexture, unsigned int layer, std::string typesToRenderExpression, int windowId, bool useThread) : IRenderer(windowId), parentRenderer(parentRenderer),
        sceneColorTexture(sceneColorTexture),
        shadowMap(GPUContext::instance().getDevice(), true), 
        shadowMapPL(GPUContext::instance().getDevice(), true),
        shadowPassCSMShader(GPUContext::instance().getDevice()),
        shadowPassPLShader(GPUContext::instance().getDevice()), 
        shadowMappingShader(GPUContext::instance().getDevice()),      
        shadowPassCommandPool(GPUContext::instance().getDevice()),
        shadowPassPLCommandPool(GPUContext::instance().getDevice()), 
        shadowMappingCommandPool(GPUContext::instance().getDevice()),
        lightSpaceMatricesStaggingBuffer(GPUContext::instance().getDevice()),
        lightSpaceMatricesStaggingBufferFinal(GPUContext::instance().getDevice()),
        lightViewsPLMatricesStaggingBuffer(GPUContext::instance().getDevice()),      
        dirLightsStaggingBufferFinal(GPUContext::instance().getDevice()),
        pointLightsStaggingBufferFinal(GPUContext::instance().getDevice()),
        commandPool(GPUContext::instance().getDevice()),
        threadPool(6),
        useThread(useThread)
        {
            rendererReady.store(false);    
            //shadowMapPL.createCubeMap(std::max(parentRenderer.getSize().x(), parentRenderer.getSize().y()), true, false);              
            //std::cout<<"pl created"<<std::endl;
            //std::cout<<"addess : "<<&parentRenderer<<std::endl;
            shadowMap.create(SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, NB_CASCADES+1, true, true);
            shadowMapPL.createCubeMap(SHADOW_MAP_SIZE, true, false);  
            physic::BoundingBox viewport = physic::BoundingBox(0, 0, parentRenderer.getCamera().getViewport().getPosition().z(), SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, parentRenderer.getCamera().getViewport().getSize().z());
            physic::BoundingBox viewportPL = physic::BoundingBox(0, 0, parentRenderer.getCamera().getViewport().getSize().z(), SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, parentRenderer.getCamera().getViewport().getSize().z());
            shadowMap.getCamera().setViewport(viewport);
            shadowMapPL.getCamera().setViewport(viewportPL);
            std::string shaderDir = std::string(ODFAEG_INSTALL_DIR) + "/Shader";
            if (!shadowPassCSMShader.loadFromFile(shaderDir + "/shadowPassCSM.vert", shaderDir + "/shadowPassCSM.frag")) {
                throw std::runtime_error("Failed to load shadow pass csm shader");
            }                
            if (!shadowPassPLShader.loadFromFile(shaderDir + "/shadowPassPL.vert", shaderDir + "/shadowPassPL.frag")) {
                throw std::runtime_error("Failed to load shadow pass csm shader");
            }
            if (!shadowMappingShader.loadFromFile(shaderDir + "/shadowMapping.vert", shaderDir + "/shadowMapping.frag")) {
                throw std::runtime_error("Failed to load shadow mapping csm shader");
            }   
            shadowCascadeLevels = computeSplits(NB_CASCADES, parentRenderer.getCamera().getViewport().getPosition().z(), parentRenderer.getCamera().getViewport().getSize().z(), 0.5f);                 
            CascadeData cascadeData;            
            for (unsigned int i = 0; i < shadowCascadeLevels.size(); i++) {
                cascadeData.cascadePlaneDistances[i].x() = shadowCascadeLevels[i];
                //std::cout<<"cascade data : "<<cascadeData.cascadePlaneDistances[i]<<std::endl;
            }
            cascadeData.cascadeCount = NB_CASCADES;  
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                cascadePlaneDistancesBuffer.emplace_back(GPUContext::instance().getDevice());
                cascadePlaneDistancesBuffer.back().create(sizeof(CascadeData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
                cascadePlaneDistancesBuffer.back().update(&cascadeData, sizeof(CascadeData));
            }  
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                lightSpaceMatricesBuffer.emplace_back(GPUContext::instance().getDevice());
                lightSpaceMatricesBuffer.back().create(sizeof(math::Matrix4f), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
            }    
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                lightSpaceMatricesBufferFinal.emplace_back(GPUContext::instance().getDevice());
                lightSpaceMatricesBufferFinal.back().create(sizeof(math::Matrix4f), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
            }   
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                lightViewsPLMatricesBuffer.emplace_back(GPUContext::instance().getDevice());
                lightViewsPLMatricesBuffer.back().create(sizeof(math::Matrix4f), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
            }   
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                dirLightsBufferFinal.emplace_back(GPUContext::instance().getDevice());
                dirLightsBufferFinal.back().create(sizeof(DirLight), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
            }  
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                pointLightsBufferFinal.emplace_back(GPUContext::instance().getDevice());
                pointLightsBufferFinal.back().create(sizeof(PointLight), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
            }          
            //shadowMap.setCamera(parentRenderer.getCamera());
            //shadowMapPL.setCamera(parentRenderer.getCamera());
            
            createCommandPools();
            unsigned int maxNodesDir = 20 * SHADOW_MAP_WIDTH * SHADOW_MAP_HEIGHT;
            shadowPassCSMFragPC.maxNodes = maxNodesDir;
            unsigned int nodeSize = 5 * sizeof(float) + sizeof(unsigned int);
            commandPool.beginRecordCommandBuffer(0);
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT*NB_CASCADES+1; i++) {
                headPtrsDirStorageImage.emplace_back(GPUContext::instance().getDevice());
                linkedListDirBuffer.emplace_back(GPUContext::instance().getDevice());
                nodeCounterDirBuffer.emplace_back(GPUContext::instance().getDevice());
                headPtrsDirStorageImage.back().create(SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, 1, VK_IMAGE_TYPE_2D, VK_FORMAT_R32_UINT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VMA_MEMORY_USAGE_GPU_ONLY,
                    1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL);
                Texture::transitionImageLayout(headPtrsDirStorageImage.back(), commandPool.getHandle(0), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
                headPtrsDirStorageImage.back().createImageView(VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R32_UINT, VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1, 1);
                //headPtrsStorageImage.back().createSampler(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 1, false, false);
                nodeCounterDirBuffer.back().create(sizeof(std::uint32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
                linkedListDirBuffer.back().create(maxNodesDir * nodeSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);                
            }        
            unsigned int maxNodesPoint = 20 * SHADOW_MAP_SIZE * SHADOW_MAP_SIZE;
            shadowPassPLFragPC.maxNodes = maxNodesPoint;    
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT*6; i++) {
                headPtrsPointStorageImage.emplace_back(GPUContext::instance().getDevice());
                linkedListPointBuffer.emplace_back(GPUContext::instance().getDevice());
                nodeCounterPointBuffer.emplace_back(GPUContext::instance().getDevice());
                headPtrsPointStorageImage.back().create(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1, VK_IMAGE_TYPE_2D, VK_FORMAT_R32_UINT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VMA_MEMORY_USAGE_GPU_ONLY,
                    1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL);
                Texture::transitionImageLayout(headPtrsPointStorageImage.back(), commandPool.getHandle(0), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
                headPtrsPointStorageImage.back().createImageView(VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R32_UINT, VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1, 1);
                //headPtrsStorageImage.back().createSampler(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 1, false, false);
                nodeCounterPointBuffer.back().create(sizeof(std::uint32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
                linkedListPointBuffer.back().create(maxNodesPoint * nodeSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);                
            } 
            commandPool.endRecordCommandBuffer(0);
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandPool.getHandle(0);
            Device::QueueFamilyIndices indices = GPUContext::instance().getDevice().findQueueFamilies(GPUContext::instance().getDevice().getPhysicalDevice(), VK_NULL_HANDLE);
            if (vkQueueSubmit(GPUContext::instance().getDevice().getQueue(indices.graphicsFamily.value(), 0), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
                throw std::runtime_error("�chec de l'envoi d'un command buffer!");
            }
            vkDeviceWaitIdle(GPUContext::instance().getDevice().getDevice());
            createDescriptorsAndPipelines();
            window::Command rendererReadyCmd(core::FastDelegate<bool>(&ShadowRenderer::isRendererReady, this), core::FastDelegate<void>(&ShadowRenderer::drawNextFrame, this));
            getEventListener().connect("RendererReady",rendererReadyCmd); 
            if (useThread) {
                //std::cout<<"lanch"<<std::endl;
                getEventListener().launch();
            }           
            if (GPUContext::instance().getSharedSemaphore(0).empty()) {
                GPUContext::instance().getSharedSemaphore(0).emplace_back(GPUContext::instance().getDevice());
                GPUContext::instance().getSharedSemaphore(0)[0].create(true, 0);
            }
            needToUpdateDescriptorSets = true;
            needToUpdateDirLightsMatrices = needToUpdatePointLightsMatrices = false;
            stop.store(false);
            rendererReady.store(true);            
        }          
        void ShadowRenderer::addDirectionnalLight(DirLight dirLight) {
            dirLights.push_back(dirLight);
            needToUpdateDirLightsMatrices = true;
        }
        void ShadowRenderer::addPonctualLight(PointLight pointLight) {
            pointLights.push_back(pointLight);
            needToUpdatePointLightsMatrices = true;
        }
        void ShadowRenderer::computePointLightMatrices() {
            Camera pointLightCamera;
            pointLightCamera.setPerspective(90, 1, 1, 25);            
            shadowPassPLVertPC.lightProjMatrix = pointLightCamera.getProjMatrix().getMatrix().transpose();
            for (unsigned int l = 0; l < pointLights.size(); l++) {  
                ViewPLMatrix viewPLMatrices;
                math::Vec3f lightPos = pointLights[l].pos;              
                pointLightCamera.setCenter(lightPos);
                pointLightCamera.lookAt(-1, 0, 0, math::Vec3f(0, -1, 0));
                viewPLMatrices.viewsPLMatrices[0] = pointLightCamera.getViewMatrix().getMatrix().transpose();
                pointLightCamera.lookAt(1, 0, 0, math::Vec3f(0, -1, 0));
                viewPLMatrices.viewsPLMatrices[1] = pointLightCamera.getViewMatrix().getMatrix().transpose();
                pointLightCamera.lookAt(0, -1, 0, math::Vec3f(0, 0, -1));
                viewPLMatrices.viewsPLMatrices[2] = pointLightCamera.getViewMatrix().getMatrix().transpose();
                pointLightCamera.lookAt(0, 1, 0, math::Vec3f(0, 0, 1));
                viewPLMatrices.viewsPLMatrices[3] = pointLightCamera.getViewMatrix().getMatrix().transpose();
                pointLightCamera.lookAt(0, 0, -1, math::Vec3f(0, -1, 0));
                viewPLMatrices.viewsPLMatrices[4] = pointLightCamera.getViewMatrix().getMatrix().transpose();
                pointLightCamera.lookAt(0, 0, 1, math::Vec3f(0, -1, 0));
                viewPLMatrices.viewsPLMatrices[5] = pointLightCamera.getViewMatrix().getMatrix().transpose(); 
                lightViewsPLMatrices.push_back(viewPLMatrices);   
                pointLights[l].far_plane = 25;             
            }
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(GPUContext::instance().getDevice().getPhysicalDevice(), &props); 
            uint32_t minAlign = props.limits.minStorageBufferOffsetAlignment;  
            uint32_t lightViewPLMatrixAlignSize = (sizeof(ViewPLMatrix) + minAlign - 1) & ~(minAlign - 1);
            lightViewsPLMatricesStaggingBuffer.create(sizeof(ViewPLMatrix) * lightViewsPLMatrices.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
            for (unsigned int l = 0; l < pointLights.size(); l++) {
                lightViewsPLMatricesStaggingBuffer.update(lightViewsPLMatrices.data(), sizeof(ViewPLMatrix), l * lightViewPLMatrixAlignSize);
            }
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {                
                lightViewsPLMatricesBuffer[i].create(sizeof(ViewPLMatrix) * lightViewsPLMatrices.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY); 
                lightViewsPLMatricesBuffer[i].setRange(sizeof(ViewPLMatrix));                             
            } 
            pointLightsStaggingBufferFinal.create(sizeof(PointLight) * pointLights.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
            pointLightsStaggingBufferFinal.update(pointLights.data(), sizeof(PointLight) * pointLights.size());                                         
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {                
                pointLightsBufferFinal[i].create(sizeof(PointLight) * pointLights.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);                              
            }
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                commandPool.beginRecordCommandBuffer(i);                
                Buffer::copyBuffer(lightViewsPLMatricesStaggingBuffer, lightViewsPLMatricesBuffer[i], sizeof(ViewPLMatrix)*lightViewsPLMatrices.size(), commandPool.getHandle(i)); 
                Buffer::copyBuffer(pointLightsStaggingBufferFinal, pointLightsBufferFinal[i], sizeof(PointLight)*pointLights.size(), commandPool.getHandle(i));
                commandPool.endRecordCommandBuffer(i);
            }
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = commandPool.getHandles().size();
            submitInfo.pCommandBuffers = commandPool.getHandles().data();
            Device::QueueFamilyIndices indices = GPUContext::instance().getDevice().findQueueFamilies(GPUContext::instance().getDevice().getPhysicalDevice(), VK_NULL_HANDLE);
            if (vkQueueSubmit(GPUContext::instance().getDevice().getQueue(indices.graphicsFamily.value(), 0), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
                throw std::runtime_error("Echec de l'envoi d'un command buffer!");
            }
            vkDeviceWaitIdle(GPUContext::instance().getDevice().getDevice());
        }
        void ShadowRenderer::computeDirLightMatrices() {            
            for (unsigned int l = 0; l < dirLights.size(); l++) {
                /*std::cout<<"update light"<<std::endl;
                int i;
                std::cin>>i;*/
                LightSpaceMatrix lightSpaceMatrices;
                for (size_t i = 1; i < shadowCascadeLevels.size(); i++)
                {

                    lightSpaceMatrices.lightSpaceMatrices[i-1] = getLightSpaceMatrix(dirLights[l].dir, shadowCascadeLevels[i-1], shadowCascadeLevels[i]);
                    //std::cout<<fLightSpaceMatrices.back()<<std::endl;
                }            
                lightSpaceMatrices.lightSpaceMatrices[NB_CASCADES] = lightSpaceMatrices.lightSpaceMatrices[NB_CASCADES-1];  
                fLightSpaceMatrices.push_back(lightSpaceMatrices);
                dirLights[l].far_plane = shadowCascadeLevels[NB_CASCADES];                           
            }   
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(GPUContext::instance().getDevice().getPhysicalDevice(), &props); 
            uint32_t minAlign = props.limits.minStorageBufferOffsetAlignment;  
            uint32_t lightSpaceMatrixAlignSize = (sizeof(LightSpaceMatrix) + minAlign - 1) & ~(minAlign - 1);   
            lightSpaceMatricesStaggingBuffer.create(sizeof(LightSpaceMatrix) * fLightSpaceMatrices.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
            for (unsigned int l = 0; l < dirLights.size(); l++) {
                lightSpaceMatricesStaggingBuffer.update(fLightSpaceMatrices.data(), sizeof(LightSpaceMatrix), l * lightSpaceMatrixAlignSize);
            }            
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {                
                lightSpaceMatricesBuffer[i].create(sizeof(LightSpaceMatrix) * fLightSpaceMatrices.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);  
                lightSpaceMatricesBuffer[i].setRange(sizeof(LightSpaceMatrix));                           
            }            
            lightSpaceMatricesStaggingBufferFinal.create(sizeof(LightSpaceMatrix) * fLightSpaceMatrices.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
            lightSpaceMatricesStaggingBufferFinal.update(fLightSpaceMatrices.data(), sizeof(LightSpaceMatrix)*fLightSpaceMatrices.size());
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {                
                lightSpaceMatricesBufferFinal[i].create(sizeof(LightSpaceMatrix) * fLightSpaceMatrices.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);                              
            }            
            dirLightsStaggingBufferFinal.create(sizeof(DirLight) * dirLights.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
            dirLightsStaggingBufferFinal.update(dirLights.data(), sizeof(DirLight) * dirLights.size());                        
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {                
                dirLightsBufferFinal[i].create(sizeof(DirLight) * dirLights.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);                              
            }  
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                commandPool.beginRecordCommandBuffer(i);
                Buffer::copyBuffer(lightSpaceMatricesStaggingBuffer, lightSpaceMatricesBuffer[i], sizeof(LightSpaceMatrix)*fLightSpaceMatrices.size(), commandPool.getHandle(i));
                Buffer::copyBuffer(lightSpaceMatricesStaggingBufferFinal, lightSpaceMatricesBufferFinal[i], sizeof(LightSpaceMatrix)*fLightSpaceMatrices.size(), commandPool.getHandle(i));                
                Buffer::copyBuffer(dirLightsStaggingBufferFinal, dirLightsBufferFinal[i], sizeof(DirLight)*dirLights.size(), commandPool.getHandle(i));
                commandPool.endRecordCommandBuffer(i);
            }
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = commandPool.getHandles().size();
            submitInfo.pCommandBuffers = commandPool.getHandles().data();
            Device::QueueFamilyIndices indices = GPUContext::instance().getDevice().findQueueFamilies(GPUContext::instance().getDevice().getPhysicalDevice(), VK_NULL_HANDLE);
            if (vkQueueSubmit(GPUContext::instance().getDevice().getQueue(indices.graphicsFamily.value(), 0), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
                throw std::runtime_error("Echec de l'envoi d'un command buffer!");
            }
            vkDeviceWaitIdle(GPUContext::instance().getDevice().getDevice());
        }
        void ShadowRenderer::createDescriptorsAndPipelines() {
            DescriptorSetLayout& shadowPassLayout = GPUContext::instance().getDescriptorSetLayout(shadowPassCSMShader, 7, true);
            shadowPassLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES, VK_SHADER_STAGE_VERTEX_BIT);
            shadowPassLayout.updateLayout(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES, VK_SHADER_STAGE_VERTEX_BIT);
            shadowPassLayout.updateLayout(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_VERTEX_BIT);
            shadowPassLayout.updateLayout(3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_FRAGMENT_BIT);
            shadowPassLayout.updateLayout(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_FRAGMENT_BIT);
            shadowPassLayout.updateLayout(5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_FRAGMENT_BIT);
            shadowPassLayout.updateLayout(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
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
            VkPushConstantRange vertexPCRange;
            vertexPCRange.offset = 0;
            vertexPCRange.size = sizeof(ShadowPassCSMVertPC);
            vertexPCRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            pushConstants.push_back(vertexPCRange);
            VkPushConstantRange fragmentPCRange;
            fragmentPCRange.offset = sizeof(ShadowPassCSMVertPC);
            fragmentPCRange.size = sizeof(ShadowPassCSMFragPC);
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
              
                GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), shadowPassCSMShader, blendMode, RenderTarget::DEPTHNOSTENCIL).createGraphicPipeline(shadowPassCSMShader, static_cast<entity::PrimitiveType>(i), GPUContext::instance().getDescriptorSetLayout(shadowPassCSMShader), renderingCreateInfo, shadowMap.getDepthStencilInfos()[RenderTarget::DEPTHNOSTENCIL], blendMode, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, pushConstants);
                /*std::cout<<"ids : "<<i<<","<<shadowPassCSMShader.getId()<<","<<RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id<<std::endl;
                std::cout<<"pipeline at creation : "<<shadowPassCSMPipeline[i][RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id]->getHandle()<<std::endl;
                std::cout<<"pipeline layout at creation : "<<shadowPassCSMPipeline[i][RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id]->getLayout()<<std::endl;*/
                //std::cout<<"pipeline created"<<std::endl;
            }            
            DescriptorSetLayout& shadowPassPLLayout = GPUContext::instance().getDescriptorSetLayout(shadowPassPLShader, 7, true);
            shadowPassPLLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES, VK_SHADER_STAGE_VERTEX_BIT);
            shadowPassPLLayout.updateLayout(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES, VK_SHADER_STAGE_FRAGMENT_BIT);
            shadowPassPLLayout.updateLayout(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_VERTEX_BIT);            
            shadowPassPLLayout.updateLayout(3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_FRAGMENT_BIT);
            shadowPassPLLayout.updateLayout(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_FRAGMENT_BIT);
            shadowPassPLLayout.updateLayout(5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_FRAGMENT_BIT);
            shadowPassPLLayout.updateLayout(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
            shadowPassPLLayout.update();
            
            renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
            renderingCreateInfo.colorAttachmentCount = 0;
            renderingCreateInfo.pColorAttachmentFormats = nullptr;
            renderingCreateInfo.depthAttachmentFormat = shadowMapPL.getDepthStencilTexture().getFormat();
            renderingCreateInfo.viewMask = shadowMapPL.getViewMask();
            
            pushConstants.clear();           
            vertexPCRange.offset = 0;
            vertexPCRange.size = sizeof(ShadowPassPLVertPC);                        
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
              
                GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), shadowPassPLShader, blendMode, RenderTarget::DEPTHNOSTENCIL).createGraphicPipeline(shadowPassPLShader, static_cast<entity::PrimitiveType>(i), GPUContext::instance().getDescriptorSetLayout(shadowPassPLShader), renderingCreateInfo, shadowMap.getDepthStencilInfos()[RenderTarget::DEPTHNOSTENCIL], blendMode, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, pushConstants);
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
            vertexPCRange.size = sizeof(ShadowMappingVertPC);            
            pushConstants.push_back(vertexPCRange);
            fragmentPCRange.offset = sizeof(ShadowMappingVertPC);
            fragmentPCRange.size = sizeof(ShadowMappingFragPC);            
            pushConstants.push_back(fragmentPCRange);
            DescriptorSetLayout& shadowMappingLayout = GPUContext::instance().getDescriptorSetLayout(shadowMappingShader, 12);
            shadowMappingLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES, VK_SHADER_STAGE_VERTEX_BIT);            
            shadowMappingLayout.updateLayout(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_FRAGMENT_BIT);
            shadowMappingLayout.updateLayout(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_FRAGMENT_BIT);
            shadowMappingLayout.updateLayout(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_FRAGMENT_BIT);
            shadowMappingLayout.updateLayout(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_FRAGMENT_BIT);
            shadowMappingLayout.updateLayout(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
            shadowMappingLayout.updateLayout(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
            shadowMappingLayout.updateLayout(7, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, RenderTexture::NB_SWAPCHAIN_IMAGES, VK_SHADER_STAGE_FRAGMENT_BIT);
            shadowMappingLayout.updateLayout(8, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_FRAGMENT_BIT);
            shadowMappingLayout.updateLayout(9, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_FRAGMENT_BIT);
            shadowMappingLayout.updateLayout(10, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_FRAGMENT_BIT);
            shadowMappingLayout.updateLayout(11, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_FRAGMENT_BIT);
            shadowMappingLayout.update();            
            blendMode.updateIds();           
            for (unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {                
                /*if (shadowPassCSMPipeline[i].size() <= RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id) {
                    for (unsigned int j = 0; j < RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id+1; j++) {
                        shadowPassCSMPipeline[i].emplace_back(std::make_unique<Pipeline>(GPUContext::instance().getDevice()));
                    }
                }
                shadowPassCSMPipeline[i][RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id]->createGraphicPipeline(shadowPassCSMShader, static_cast<PrimitiveType>(i), GPUContext::instance().getDescriptorSetLayout(shadowPassCSMShader), renderingCreateInfo, shadowMap.getDepthStencilInfos()[RenderTarget::DEPTHNOSTENCIL], blendMode, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, pushConstants);*/
              
                GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), shadowMappingShader, blendMode, RenderTarget::DEPTHNOSTENCIL).createGraphicPipeline(shadowMappingShader, static_cast<entity::PrimitiveType>(i), GPUContext::instance().getDescriptorSetLayout(shadowMappingShader), renderingCreateInfo, shadowMap.getDepthStencilInfos()[RenderTarget::DEPTHNOSTENCIL], blendMode, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, pushConstants);
                //std::cout<<"ids : "<<i<<","<<shadowMappingPLShader.getId()<<","<<RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id<<std::endl;
                /*std::cout<<"pipeline at creation : "<<shadowPassCSMPipeline[i][RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id]->getHandle()<<std::endl;
                std::cout<<"pipeline layout at creation : "<<shadowPassCSMPipeline[i][RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id]->getLayout()<<std::endl;*/
                //std::cout<<"pipeline created"<<std::endl;
            }
            DescriptorPool& shadowPassPool = GPUContext::instance().getDescriptorPool(shadowPassCSMShader, 7);
            shadowPassPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES);
            shadowPassPool.updatePoolSize(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES);
            shadowPassPool.updatePoolSize(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,MAX_FRAMES_IN_FLIGHT);
            shadowPassPool.updatePoolSize(3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,MAX_FRAMES_IN_FLIGHT);
            shadowPassPool.updatePoolSize(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,MAX_FRAMES_IN_FLIGHT);
            shadowPassPool.updatePoolSize(5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,MAX_FRAMES_IN_FLIGHT);
            shadowPassPool.updatePoolSize(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,MAX_TEXTURES);
            shadowPassPool.update();
            DescriptorPool& shadowPassPLPool = GPUContext::instance().getDescriptorPool(shadowPassPLShader, 7);
            shadowPassPLPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES);
            shadowPassPLPool.updatePoolSize(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES);
            shadowPassPLPool.updatePoolSize(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT);
            shadowPassPLPool.updatePoolSize(3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,MAX_FRAMES_IN_FLIGHT);
            shadowPassPLPool.updatePoolSize(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,MAX_FRAMES_IN_FLIGHT);
            shadowPassPLPool.updatePoolSize(5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,MAX_FRAMES_IN_FLIGHT);
            shadowPassPLPool.updatePoolSize(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,MAX_TEXTURES);
            shadowPassPLPool.update();
            DescriptorPool& shadowMappingPool = GPUContext::instance().getDescriptorPool(shadowMappingShader, 12);
            shadowMappingPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES);
            shadowMappingPool.updatePoolSize(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT);
            shadowMappingPool.updatePoolSize(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT);            
            shadowMappingPool.updatePoolSize(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT);
            shadowMappingPool.updatePoolSize(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT);
            shadowMappingPool.updatePoolSize(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
            shadowMappingPool.updatePoolSize(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
            shadowMappingPool.updatePoolSize(7, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, RenderTexture::NB_SWAPCHAIN_IMAGES);
            shadowMappingPool.updatePoolSize(8, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_FRAMES_IN_FLIGHT);
            shadowMappingPool.updatePoolSize(9, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT);
            shadowMappingPool.updatePoolSize(10, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_FRAMES_IN_FLIGHT);
            shadowMappingPool.updatePoolSize(11, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT);
            shadowMappingPool.update();
            /*shadowPassCSMSets.resize(1);
            shadowPassCSMSets[0].emplace_back(std::make_unique<DescriptorSet>(GPUContext::instance().getDevice()));
            shadowPassCSMSets[0][0]->setNbBindings(2);*/
            DescriptorSet::allocate(shadowPassPool, shadowPassLayout, GPUContext::instance().getDescriptorSets(shadowPassCSMShader, 7, 1), MAX_TEXTURES);
            DescriptorSet::allocate(shadowPassPLPool, shadowPassPLLayout, GPUContext::instance().getDescriptorSets(shadowPassPLShader, 7, 1), MAX_TEXTURES);
            /*shadowMappingCSMSets.resize(1);
            shadowMappingCSMSets[0].emplace_back(std::make_unique<DescriptorSet>(GPUContext::instance().getDevice()));*/            
            DescriptorSet::allocate(shadowMappingPool, shadowMappingLayout, GPUContext::instance().getDescriptorSets(shadowMappingShader, 12, 1));
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
        math::Matrix4f ShadowRenderer::getLightSpaceMatrix(math::Vec3f lightDir, const float nearPlane, const float farPlane) {
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
            commandPool.create(queueFamilyIndices.graphicsFamily.value());
            commandPool.createCommandBuffers(true, MAX_FRAMES_IN_FLIGHT);
            shadowPassCommandPool.create(queueFamilyIndices.graphicsFamily.value());
            shadowPassCommandPool.createCommandBuffers(false, MAX_FRAMES_IN_FLIGHT);
            shadowPassPLCommandPool.create(queueFamilyIndices.graphicsFamily.value());
            shadowPassPLCommandPool.createCommandBuffers(false, MAX_FRAMES_IN_FLIGHT);
            shadowMappingCommandPool.create(queueFamilyIndices.graphicsFamily.value());            
            shadowMappingCommandPool.createCommandBuffers(false, MAX_FRAMES_IN_FLIGHT);
        }
        void ShadowRenderer::updateDescriptorSets() {
            //std::cout<<"update descriptor sets"<<std::endl;
            bool hasDiffuseTexture = GPUContext::instance().getSharedTextures(entity::SubMesh::DIFFUSE).size() != 0;
            DescriptorSet& shadowPassDescriptorSet = GPUContext::instance().getDescriptorSets(shadowPassCSMShader, (hasDiffuseTexture) ? 7 : 6, 1)[0];
            //std::cout<<"size : "<<shadowPassCSMSets.size()<<","<<shadowPassCSMSets[0].size()<<std::endl;
            //std::cout<<"id 2 : "<<(RenderTarget::OUTPUT_MODELS+shadowMap.getId()*RenderTarget::NB_BUFFERS)<<std::endl;
            shadowPassDescriptorSet.updateBufferInfos(0, GPUContext::instance().getSharedBuffers(RenderTarget::OUTPUT_MODELS+shadowMap.getId()*RenderTarget::NB_BUFFERS), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            shadowPassDescriptorSet.updateBufferInfos(1, lightSpaceMatricesBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);
            shadowPassDescriptorSet.updateBufferInfos(2, GPUContext::instance().getSharedBuffers(RenderTarget::OUTPUT_MATERIALS+parentRenderer.getId()*RenderTarget::NB_BUFFERS), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            shadowPassDescriptorSet.updateImageInfos(3, headPtrsDirStorageImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
            shadowPassDescriptorSet.updateBufferInfos(4, nodeCounterDirBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            shadowPassDescriptorSet.updateBufferInfos(5, linkedListDirBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            if (hasDiffuseTexture) {
                shadowPassDescriptorSet.updateImageInfos(6, GPUContext::instance().getSharedTextures(entity::SubMesh::DIFFUSE), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            }
            shadowPassDescriptorSet.updateDescriptorSet();   
            DescriptorSet& shadowPassPLDescriptorSet = GPUContext::instance().getDescriptorSets(shadowPassPLShader, (hasDiffuseTexture) ? 7 : 6, 1)[0];
            shadowPassPLDescriptorSet.updateBufferInfos(0, GPUContext::instance().getSharedBuffers(RenderTarget::OUTPUT_MODELS+shadowMapPL.getId()*RenderTarget::NB_BUFFERS), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            shadowPassPLDescriptorSet.updateBufferInfos(1, lightViewsPLMatricesBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);
            shadowPassPLDescriptorSet.updateBufferInfos(2, GPUContext::instance().getSharedBuffers(RenderTarget::OUTPUT_MATERIALS+parentRenderer.getId()*RenderTarget::NB_BUFFERS), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            shadowPassPLDescriptorSet.updateImageInfos(3, headPtrsPointStorageImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
            shadowPassPLDescriptorSet.updateBufferInfos(4, nodeCounterPointBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            shadowPassPLDescriptorSet.updateBufferInfos(5, linkedListPointBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            if (hasDiffuseTexture) {
                shadowPassPLDescriptorSet.updateImageInfos(6, GPUContext::instance().getSharedTextures(entity::SubMesh::DIFFUSE), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            }
            shadowPassPLDescriptorSet.updateDescriptorSet();
            
            DescriptorSet& shadowMappingDescriptorSet = GPUContext::instance().getDescriptorSets(shadowMappingShader, 12, 1)[0];
            //shadowMappingCSMSets[0][0]->setNbBindings((hasDiffuseTexture) ? 6 : 5);
            shadowMap.getDepthStencilTexture().getImage(0).setLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
            shadowMapPL.getDepthStencilTexture().getImage(0).setLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
            shadowMappingDescriptorSet.updateBufferInfos(0, GPUContext::instance().getSharedBuffers(RenderTarget::OUTPUT_MODELS+parentRenderer.getId()*RenderTarget::NB_BUFFERS), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            shadowMappingDescriptorSet.updateBufferInfos(1, lightSpaceMatricesBufferFinal, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            shadowMappingDescriptorSet.updateBufferInfos(2, cascadePlaneDistancesBuffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
            shadowMappingDescriptorSet.updateBufferInfos(3, dirLightsBufferFinal, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            shadowMappingDescriptorSet.updateBufferInfos(4, pointLightsBufferFinal, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            shadowMappingDescriptorSet.updateImageInfos(5, shadowMap.getDepthStencilTexture(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            shadowMappingDescriptorSet.updateImageInfos(6, shadowMapPL.getDepthStencilTexture(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            shadowMap.getDepthStencilTexture().getImage(0).setLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
            shadowMapPL.getDepthStencilTexture().getImage(0).setLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
            shadowMappingDescriptorSet.updateImageInfos(7, sceneColorTexture.getTexture(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);            
            shadowMappingDescriptorSet.updateImageInfos(8, headPtrsDirStorageImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
            shadowMappingDescriptorSet.updateBufferInfos(9, linkedListDirBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            shadowMappingDescriptorSet.updateImageInfos(10, headPtrsDirStorageImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
            shadowMappingDescriptorSet.updateBufferInfos(11, linkedListDirBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            shadowMappingDescriptorSet.updateDescriptorSet();
        }
        void ShadowRenderer::clear() {
            shadowMap.clear();
            VkClearColorValue clearColor;
            clearColor.uint32[0] = 0xffffffff;
            VkImageSubresourceRange subresRange = {};
            subresRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subresRange.levelCount = 1;
            subresRange.layerCount = 1;
            for (unsigned int i = 0; i < NB_CASCADES+1; i++) {
                vkCmdClearColorImage(shadowMap.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), headPtrsDirStorageImage[i*MAX_FRAMES_IN_FLIGHT+parentRenderer.getCurrentFrame()].getHandle(), VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &subresRange);
                vkCmdFillBuffer(shadowMap.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), nodeCounterDirBuffer[i*MAX_FRAMES_IN_FLIGHT+parentRenderer.getCurrentFrame()].getHandle(), 0, sizeof(uint32_t), 0u);
            }
            shadowMapPL.clear(); 
            for (unsigned int i = 0; i < 6; i++) {
                vkCmdClearColorImage(shadowMapPL.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), headPtrsPointStorageImage[i*MAX_FRAMES_IN_FLIGHT+parentRenderer.getCurrentFrame()].getHandle(), VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &subresRange);
                vkCmdFillBuffer(shadowMapPL.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), nodeCounterPointBuffer[i*MAX_FRAMES_IN_FLIGHT+parentRenderer.getCurrentFrame()].getHandle(), 0, sizeof(uint32_t), 0u);
            }
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
                //waitSemaphores.push_back(shadowMap.getSemaphores()[0].getHandle());
                waitValues.push_back(GPUContext::instance().getSharedSemaphore(0)[0].getValue());
                //waitValues.push_back(shadowMap.getSemaphores()[0].getValue());    
                semaphoreWaitInfo.semaphoreCount = waitSemaphores.size();
                semaphoreWaitInfo.pSemaphores = waitSemaphores.data();
                semaphoreWaitInfo.pValues = waitValues.data();
                //vkWaitSemaphores(GPUContext::instance().getDevice().getDevice(), &semaphoreWaitInfo, UINT64_MAX);
                //std::cout<<"frame : "<<renderFrame<<" ready!"<<std::endl;
                if (needToUpdateDirLightsMatrices) {
                    computeDirLightMatrices();
                    needToUpdateDirLightsMatrices = false;
                    needToUpdateDescriptorSets = true;
                }
                if (needToUpdatePointLightsMatrices) {
                    computePointLightMatrices();
                    needToUpdatePointLightsMatrices = false;
                    needToUpdateDescriptorSets = true;
                }
                if (needToUpdateDescriptorSets) {
                    //std::cout<<"update ds"<<std::endl;
                    updateDescriptorSets();
                    needToUpdateDescriptorSets = false;
                }                         
                jobFences[renderFrame].reset(3);                
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
                    for (unsigned int i = 0; i < GPUContext::instance().getDescriptorSets(shadowPassCSMShader).size(); i++) {
                        sets.push_back(GPUContext::instance().getDescriptorSets(shadowPassCSMShader)[i][0].getHandle());
                    }
                    blendMode.updateIds();
                    //std::cout<<"begin record cmd : "<<renderFrame<<std::endl;
                    shadowPassCommandPool.beginRecordCommandBuffer(renderFrame, inheritanceInfo);
                    shadowPassCSMVertPC.currentFrame = renderFrame;
                    VkPhysicalDeviceProperties props;
                    vkGetPhysicalDeviceProperties(GPUContext::instance().getDevice().getPhysicalDevice(), &props); 
                    uint32_t minAlign = props.limits.minStorageBufferOffsetAlignment;  
                    uint32_t lightSpaceMatrixAlignSize = (sizeof(LightSpaceMatrix) + minAlign - 1) & ~(minAlign - 1);   
                    for (unsigned int l = 0; l < dirLights.size(); l++) {
                        for (unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {
                            shadowPassCSMVertPC.primitiveType = i;
                            //std::cout<<"ids : "<<i<<","<<shadowPassCSMShader.getId()<<","<<RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id<<std::endl;
                            std::vector<uint32_t> offsetLightSpaceMats;
                            for (unsigned int j = 0; j < MAX_FRAMES_IN_FLIGHT; j++) {
                              offsetLightSpaceMats.push_back(l * lightSpaceMatrixAlignSize);
                            }                            
                            //std::cout<<"register binds"<<std::endl;
                            vkCmdBindPipeline(shadowPassCommandPool.getHandle(renderFrame), VK_PIPELINE_BIND_POINT_GRAPHICS, GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), shadowPassCSMShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getHandle());
                            //std::cout<<"registered bind pipeline"<<std::endl;
                            vkCmdPushConstants(shadowPassCommandPool.getHandle(renderFrame), GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), shadowPassCSMShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ShadowPassCSMVertPC), &shadowPassCSMVertPC);
                            //std::cout<<"registed bind push constants"<<std::endl;
                            vkCmdBindDescriptorSets(shadowPassCommandPool.getHandle(renderFrame), VK_PIPELINE_BIND_POINT_GRAPHICS, GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), shadowPassCSMShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getLayout(), 0, sets.size(), sets.data(), offsetLightSpaceMats.size(), offsetLightSpaceMats.data());
                            //std::cout<<"registered bind decriptor sets"<<std::endl;
                            shadowMap.draw(shadowPassCommandPool, static_cast<entity::PrimitiveType>(i), states);
                            //std::cout<<"registered"<<std::endl;
                        }
                    }
                    shadowPassCommandPool.endRecordCommandBuffer(renderFrame);
                    //std::cout<<"render frame : "<<renderFrame<<std::endl;
                    jobFences[renderFrame].jobDone();
                });               
                threadPool.enqueue([this, renderFrame] {
                    VkCommandBufferInheritanceRenderingInfo inheritanceRenderingInfo{};
                    inheritanceRenderingInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO;
                    inheritanceRenderingInfo.colorAttachmentCount = 0;
                    inheritanceRenderingInfo.pColorAttachmentFormats = nullptr; // tableau de VkFormat
                    inheritanceRenderingInfo.depthAttachmentFormat = shadowMapPL.getDepthStencilTexture().getFormat();    // VK_FORMAT_D32_SFLOAT, etc.                    
                    inheritanceRenderingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
                    inheritanceRenderingInfo.viewMask = shadowMapPL.getViewMask();
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
                    states.shader = &shadowPassPLShader;
                    BlendMode blendMode;
                    std::vector<VkDescriptorSet> sets;
                    for (unsigned int i = 0; i < GPUContext::instance().getDescriptorSets(shadowPassPLShader).size(); i++) {
                        sets.push_back(GPUContext::instance().getDescriptorSets(shadowPassPLShader)[i][0].getHandle());
                    }
                    
                    //std::cout<<"begin record cmd : "<<renderFrame<<std::endl;
                    shadowPassPLCommandPool.beginRecordCommandBuffer(renderFrame, inheritanceInfo);
                    shadowPassPLVertPC.currentFrame = renderFrame;  
                    VkPhysicalDeviceProperties props;
                    vkGetPhysicalDeviceProperties(GPUContext::instance().getDevice().getPhysicalDevice(), &props); 
                    uint32_t minAlign = props.limits.minStorageBufferOffsetAlignment;  
                    uint32_t lightViewPLMatrixAlignSize = (sizeof(ViewPLMatrix) + minAlign - 1) & ~(minAlign - 1); 
                    for (unsigned int l = 0; l < pointLights.size(); l++) { 
                        shadowPassPLFragPC.lightPos = pointLights[l].pos;
                        shadowPassPLFragPC.far_plane = pointLights[l].far_plane;
                        
                        for (unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {
                            shadowPassPLVertPC.primitiveType = i;                            
                            //std::cout<<"ids : "<<i<<","<<shadowPassPLShader.getId()<<","<<RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id<<std::endl;
                            std::vector<uint32_t> offsetLightViewMatPLs;
                            for (unsigned int j = 0; j < MAX_FRAMES_IN_FLIGHT; j++) {
                                offsetLightViewMatPLs.push_back(l * lightViewPLMatrixAlignSize);
                            }
                            vkCmdBindPipeline(shadowPassPLCommandPool.getHandle(renderFrame), VK_PIPELINE_BIND_POINT_GRAPHICS, GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), shadowPassPLShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getHandle());
                            //std::cout<<"registered bind pipeline"<<std::endl;
                            vkCmdPushConstants(shadowPassPLCommandPool.getHandle(renderFrame), GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), shadowPassPLShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ShadowPassPLVertPC), &shadowPassPLVertPC);
                            vkCmdPushConstants(shadowPassPLCommandPool.getHandle(renderFrame), GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), shadowPassPLShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(ShadowPassPLVertPC), sizeof(ShadowPassPLFragPC), &shadowPassPLFragPC);
                            //std::cout<<"registed bind push constants"<<std::endl;
                            vkCmdBindDescriptorSets(shadowPassPLCommandPool.getHandle(renderFrame), VK_PIPELINE_BIND_POINT_GRAPHICS, GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), shadowPassPLShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getLayout(), 0, sets.size(), sets.data(), offsetLightViewMatPLs.size(), offsetLightViewMatPLs.data());
                            //std::cout<<"registered bind decriptor sets"<<std::endl;
                            shadowMapPL.draw(shadowPassPLCommandPool, static_cast<entity::PrimitiveType>(i), states);
                            //std::cout<<"registered"<<std::endl;
                        }
                    }
                    shadowPassPLCommandPool.endRecordCommandBuffer(renderFrame);
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
                    states.shader = &shadowMappingShader;
                    BlendMode blendMode;
                    std::vector<VkDescriptorSet> sets;
                    for (unsigned int i = 0; i < GPUContext::instance().getDescriptorSets(shadowMappingShader).size(); i++) {
                        sets.push_back(GPUContext::instance().getDescriptorSets(shadowMappingShader)[i][0].getHandle());
                    }                    
                    blendMode.updateIds();
                    shadowMappingCommandPool.beginRecordCommandBuffer(renderFrame, inheritanceInfo);  
                    shadowMappingVertPC.projMatrix = parentRenderer.getCamera().getProjMatrix().getMatrix().transpose();
                    shadowMappingVertPC.viewMatrix = parentRenderer.getCamera().getViewMatrix().getMatrix().transpose();
                    shadowMappingVertPC.currentFrame = parentRenderer.getCurrentFrame();  
                    shadowMappingFragPC.view = parentRenderer.getCamera().getViewMatrix().getMatrix();
                    shadowMappingFragPC.nbDirLights = dirLights.size();  
                    shadowMappingFragPC.nbPointLights = pointLights.size();                         
                    //std::cout << "sizeof(ViewProjMatPC) = " << sizeof(ViewProjMatPC) << std::endl;
                    for (unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {
                        shadowMappingVertPC.primitiveType = i;
                        //std::cout<<"ids : "<<i<<","<<shadowMappingPLShader.getId()<<","<<RenderTarget::DEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id<<std::endl;
                        vkCmdBindPipeline(shadowMappingCommandPool.getHandle(renderFrame), VK_PIPELINE_BIND_POINT_GRAPHICS, GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), shadowMappingShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getHandle());
                        vkCmdPushConstants(shadowMappingCommandPool.getHandle(renderFrame), GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), shadowMappingShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ShadowMappingVertPC), &shadowMappingVertPC);
                        vkCmdPushConstants(shadowMappingCommandPool.getHandle(renderFrame), GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), shadowMappingShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(ShadowMappingVertPC), sizeof(ShadowMappingFragPC), &shadowMappingFragPC);
                        //std::cout<<"bind sets"<<std::endl;
                        vkCmdBindDescriptorSets(shadowMappingCommandPool.getHandle(renderFrame), VK_PIPELINE_BIND_POINT_GRAPHICS, GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), shadowMappingShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getLayout(), 0, sets.size(), sets.data(), 0, nullptr);
                        parentRenderer.draw(shadowMappingCommandPool, static_cast<entity::PrimitiveType>(i), states);
                    }
                    shadowMappingCommandPool.endRecordCommandBuffer(renderFrame);
                    //std::cout<<"render frame : "<<renderFrame<<std::endl;
                    jobFences[renderFrame].jobDone();
                });                                            
                jobFences[renderFrame].wait();                             
            }
            
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
                shadowMapPL.applyComputeGraphicsBarrier();
                shadowMapPL.beginRendering(true);
                vkCmdExecuteCommands(shadowMapPL.getCommandPool().getHandle(shadowMapPL.getCurrentFrame()), 1, &shadowPassPLCommandPool.getHandle(parentRenderer.getCurrentFrame()));
                shadowMapPL.endRendering(); 
                shadowMapPL.submit(true);
                shadowMapPL.display();
                //std::cout<<"shadow map drawn"<<std::endl;
                                          
                parentRenderer.applyComputeGraphicsBarrier();
                VkMemoryBarrier memoryBarrier{};
                memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                memoryBarrier.pNext = VK_NULL_HANDLE;
                memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;  
                vkCmdPipelineBarrier(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                
                Texture::transitionImageLayout(shadowMap.getDepthStencilTexture().getImage(0), parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);      
                Texture::transitionImageLayout(shadowMapPL.getDepthStencilTexture().getImage(0), parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
                parentRenderer.beginRendering(true);
                vkCmdExecuteCommands(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), 1, &shadowMappingCommandPool.getHandle(parentRenderer.getCurrentFrame()));
                parentRenderer.endRendering(); 
                Texture::transitionImageLayout(shadowMapPL.getDepthStencilTexture().getImage(0), parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);     
                Texture::transitionImageLayout(shadowMap.getDepthStencilTexture().getImage(0), parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);               
            }
            //std::cout<<"drawn"<<std::endl;
        }
        bool ShadowRenderer::isRendererReady() {
            return rendererReady.load();
        }
    }
}
