module;
#include <stdexcept>
#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.h"
#include "odfaeg/config.hpp"
#include <iostream>
//import odfaeg.graphic.linkedListRenderer;
module odfaeg.graphic.linkedListRenderer;
import odfaeg.graphic.gpuContext;
import odfaeg.graphic.renderTarget;
import odfaeg.entity.vertex;
import odfaeg.graphic.blendMode;
import odfaeg.graphic.device;
import odfaeg.core.delegate;
import odfaeg.window.command;
import odfaeg.graphic.material;
import odfaeg.graphic.texture;
import odfaeg.entity.gameObject;
namespace odfaeg {
    namespace graphic {
        LinkedListRenderer::LinkedListRenderer(RenderTarget& parentRenderer, unsigned int layer, std::string typesToRenderExpression, int windowId, bool useThread) : IRenderer(windowId), threadPool(6),
        typesToRenderExpression(typesToRenderExpression),
        layer(layer),
        fullScreenQuad(GPUContext::instance().getDevice(), entity::PrimitiveType::Triangles),
        linkedListShader(GPUContext::instance().getDevice()),
        quadLinkedListShader(GPUContext::instance().getDevice()),
        linkedListCmdPool(GPUContext::instance().getDevice()),
        quadLinkedListCommandPool(GPUContext::instance().getDevice()),
        parentRenderer(parentRenderer), commandPool(GPUContext::instance().getDevice())
        {        
            rendererReady.store(false);
            stop.store(false);
            unsigned int nodeSize = 5 * sizeof(float) + sizeof(unsigned int);
            math::Vector2u size = parentRenderer.getSize();
            //std::cout<<"size : "<<size.x()<<","<<size.y()<<std::endl;
            maxNodes = 20 * size.x() * size.y();
            Device::QueueFamilyIndices queueFamilyIndices = GPUContext::instance().getDevice().findQueueFamilies(GPUContext::instance().getDevice().getPhysicalDevice());
            commandPool.create(queueFamilyIndices.graphicsFamily.value());
            commandPool.createCommandBuffers(true, MAX_FRAMES_IN_FLIGHT);
            commandPool.beginRecordCommandBuffer(0);
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                headPtrsStorageImage.emplace_back(GPUContext::instance().getDevice());
                linkedListBuffer.emplace_back(GPUContext::instance().getDevice());
                nodeCounterBuffer.emplace_back(GPUContext::instance().getDevice());
                headPtrsStorageImage.back().create(size.x(), size.y(), 1, VK_IMAGE_TYPE_2D, VK_FORMAT_R32_UINT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VMA_MEMORY_USAGE_GPU_ONLY,
                    1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL);                
                headPtrsStorageImage.back().createImageView(VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R32_UINT, VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1, 1);
                Texture::transitionImageLayout(headPtrsStorageImage.back(), commandPool.getHandle(0), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
                //headPtrsStorageImage.back().createSampler(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 1, false, false);
                nodeCounterBuffer.back().create(sizeof(std::uint32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
                linkedListBuffer.back().create(maxNodes * nodeSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
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
            std::string shaderDir = std::string(ODFAEG_INSTALL_DIR) + "/Shader";
            if (!linkedListShader.loadFromFile(shaderDir + "/renderTarget.vert", shaderDir + "/linkedList.frag")) {
                throw std::runtime_error("Could not load linked list shader");
            }
            if (!quadLinkedListShader.loadFromFile(shaderDir + "/linkedListQuad.vert", shaderDir + "/linkedListQuad.frag")) {
                throw std::runtime_error("Could not load linked list quad shader");
            }
            fullScreenQuad.resize(4, 0);
            //All the screen area (in NDC coords)
            fullScreenQuad[0] = entity::Vertex(math::Vec3f(-1.f, -1.f, 0.f));
            fullScreenQuad[1] = entity::Vertex(math::Vec3f(-1.f, 1.f, 0.f));
            fullScreenQuad[2] = entity::Vertex(math::Vec3f(1.f, 1.f, 0.f));
            fullScreenQuad[3] = entity::Vertex(math::Vec3f(1.f, -1.f, 0.f));
            fullScreenQuad.addIndex(0);
            fullScreenQuad.addIndex(1);
            fullScreenQuad.addIndex(2);
            fullScreenQuad.addIndex(0);
            fullScreenQuad.addIndex(2);
            fullScreenQuad.addIndex(3);
            fullScreenQuad.update();
            createDescriptorsAndPipelines();
            linkedListCmdPool.create(queueFamilyIndices.graphicsFamily.value());
            linkedListCmdPool.createCommandBuffers(false, MAX_FRAMES_IN_FLIGHT);
            quadLinkedListCommandPool.create(queueFamilyIndices.graphicsFamily.value());
            quadLinkedListCommandPool.createCommandBuffers(false, MAX_FRAMES_IN_FLIGHT);

            window::Command rendererReadyCmd(core::FastDelegate<bool>(&LinkedListRenderer::isRendererReady, this), core::FastDelegate<void>(&LinkedListRenderer::drawNextFrame, this));
            getEventListener().connect("RendererReady",rendererReadyCmd);
            connectSwapchainResizedCommand<LinkedListRenderer>();
            if (useThread) {
                getEventListener().launch();
            }
            if (GPUContext::instance().getSharedSemaphore(0).empty()) {
                GPUContext::instance().getSharedSemaphore(0).emplace_back(GPUContext::instance().getDevice());
                GPUContext::instance().getSharedSemaphore(0)[0].create(true, 0);
            }
            needToUpdateDescriptorSets = true;
            rendererReady.store(true);
        }
        void LinkedListRenderer::createDescriptorsAndPipelines() {
            DescriptorSetLayout& linkedListLayout = GPUContext::instance().getDescriptorSetLayout(linkedListShader, 7, true);
            linkedListLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT*NB_PRIMITIVE_TYPES, VK_SHADER_STAGE_VERTEX_BIT);
            linkedListLayout.updateLayout(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT*NB_PRIMITIVE_TYPES, VK_SHADER_STAGE_VERTEX_BIT);
            linkedListLayout.updateLayout(2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_FRAGMENT_BIT);
            linkedListLayout.updateLayout(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT*NB_PRIMITIVE_TYPES, VK_SHADER_STAGE_FRAGMENT_BIT);
            for (unsigned int i = 4; i < 6; i++) {
                linkedListLayout.updateLayout(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_FRAGMENT_BIT);
            }
            linkedListLayout.updateLayout(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
            linkedListLayout.update();
            BlendMode blendMode;
            std::vector<VkPushConstantRange> pushConstants;
            VkPushConstantRange vsPushConstant;
            vsPushConstant.offset = 0;
            vsPushConstant.size = sizeof(ViewProjMatPC);
            vsPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            pushConstants.push_back(vsPushConstant);
            VkPushConstantRange fsPushConstant;
            fsPushConstant.offset = sizeof(ViewProjMatPC);
            fsPushConstant.size = sizeof(LinkedListPC);
            fsPushConstant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            pushConstants.push_back(fsPushConstant);
            VkPipelineRenderingCreateInfo renderingCreateInfo = {};
            renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
            renderingCreateInfo.colorAttachmentCount = 1;
            renderingCreateInfo.pColorAttachmentFormats = &parentRenderer.getImageFormat();
            renderingCreateInfo.depthAttachmentFormat = parentRenderer.getDepthStencilTexture().getFormat();
            for (unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {
                GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), linkedListShader, blendMode,0).createGraphicPipeline(linkedListShader, static_cast<entity::PrimitiveType>(i), GPUContext::instance().getDescriptorSetLayout(linkedListShader), renderingCreateInfo,parentRenderer.getDepthStencilInfos()[RenderTarget::NODEPTHNOSTENCIL], blendMode, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, pushConstants);
            }
            DescriptorSetLayout& quadLinkedListLayout = GPUContext::instance().getDescriptorSetLayout(quadLinkedListShader, 2);
            quadLinkedListLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_FRAGMENT_BIT);
            quadLinkedListLayout.updateLayout(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_FRAGMENT_BIT);
            quadLinkedListLayout.update();
            pushConstants.clear();
            VkPushConstantRange quadPushConstant;
            quadPushConstant.offset = 0;
            quadPushConstant.size = sizeof(unsigned int);
            quadPushConstant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            pushConstants.push_back(quadPushConstant);
            for (unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {
                GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), quadLinkedListShader, blendMode, 0).createGraphicPipeline(quadLinkedListShader, static_cast<entity::PrimitiveType>(i), GPUContext::instance().getDescriptorSetLayout(quadLinkedListShader), renderingCreateInfo,parentRenderer.getDepthStencilInfos()[RenderTarget::NODEPTHNOSTENCIL], blendMode, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, pushConstants);
            }
            DescriptorPool& linkedListPool = GPUContext::instance().getDescriptorPool(linkedListShader, 7);
            linkedListPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT*NB_PRIMITIVE_TYPES);
            linkedListPool.updatePoolSize(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT*NB_PRIMITIVE_TYPES);
            linkedListPool.updatePoolSize(2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_FRAMES_IN_FLIGHT);
            linkedListPool.updatePoolSize(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT*NB_PRIMITIVE_TYPES);
            for (unsigned int i = 4; i < 6; i++) {
                linkedListPool.updatePoolSize(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT);
            }
            linkedListPool.updatePoolSize(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
            linkedListPool.update();
            DescriptorPool& quadLinkedListPool = GPUContext::instance().getDescriptorPool(quadLinkedListShader, 2);
            quadLinkedListPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_FRAMES_IN_FLIGHT);
            quadLinkedListPool.updatePoolSize(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT);
            quadLinkedListPool.update();
            DescriptorSet::allocate(linkedListPool, linkedListLayout, GPUContext::instance().getDescriptorSets(linkedListShader, 7, 1), MAX_TEXTURES);
            DescriptorSet::allocate(quadLinkedListPool, quadLinkedListLayout, GPUContext::instance().getDescriptorSets(quadLinkedListShader, 2, 1));
            //std::cout<<"pipeline descritpors created"<<std::endl;
        }
        void LinkedListRenderer::updateDescriptorSets() {
            bool hasDiffuseTexture = GPUContext::instance().getSharedTextures(entity::SubMesh::DIFFUSE).size() != 0;
            DescriptorSet& linkedListSet = GPUContext::instance().getDescriptorSets(linkedListShader, (hasDiffuseTexture) ? 7 : 6, 1)[0];
            linkedListSet.updateBufferInfos(0, GPUContext::instance().getSharedBuffers(RenderTarget::OUTPUT_MODELS), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            linkedListSet.updateBufferInfos(1, GPUContext::instance().getSharedBuffers(RenderTarget::OUTPUT_MESHES), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            linkedListSet.updateImageInfos(2, headPtrsStorageImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
            linkedListSet.updateBufferInfos(3, GPUContext::instance().getSharedBuffers(RenderTarget::OUTPUT_MATERIALS), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            linkedListSet.updateBufferInfos(4, linkedListBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            linkedListSet.updateBufferInfos(5, nodeCounterBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            if (hasDiffuseTexture) {
                //std::cout<<"diffuse texture"<<std::endl;
                linkedListSet.updateImageInfos(6, GPUContext::instance().getSharedTextures(entity::SubMesh::DIFFUSE), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            }
            linkedListSet.updateDescriptorSet();
            DescriptorSet& linkedListQuadSet = GPUContext::instance().getDescriptorSets(quadLinkedListShader, 2, 1)[0];
            linkedListQuadSet.updateImageInfos(0, headPtrsStorageImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
            linkedListQuadSet.updateBufferInfos(1, linkedListBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            linkedListQuadSet.updateDescriptorSet();
        }
        void LinkedListRenderer::clear() {
            
            VkClearColorValue clearColor;
            clearColor.uint32[0] = 0xffffffff;
            VkImageSubresourceRange subresRange = {};
            subresRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subresRange.levelCount = 1;
            subresRange.layerCount = 1;
            vkCmdClearColorImage(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), headPtrsStorageImage[parentRenderer.getCurrentFrame()].getHandle(), VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &subresRange);
            vkCmdFillBuffer(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), nodeCounterBuffer[parentRenderer.getCurrentFrame()].getHandle(), 0, sizeof(uint32_t), 0u);
            VkMemoryBarrier memoryBarrier0;
            memoryBarrier0.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
            memoryBarrier0.pNext = VK_NULL_HANDLE;
            memoryBarrier0.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            memoryBarrier0.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            vkCmdPipelineBarrier(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier0, 0, nullptr, 0, nullptr);
            parentRenderer.setTypesToRender(typesToRenderExpression, parentRenderer.getCurrentFrame());
            parentRenderer.applyCullingAndBatching();
            registerFramesJob[parentRenderer.getCurrentFrame()].store(true);
            //std::cout<<"cleared"<<std::endl;
            cv.notify_one();
        }
        void LinkedListRenderer::drawNextFrame() {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] {
                    //std::cout<<"draw frame : "<<frameBuffer.getCurrentFrame()<<std::endl;
                return registerFramesJob[parentRenderer.getCurrentFrame()].load() || stop.load();
            });
            registerFramesJob[parentRenderer.getCurrentFrame()].store(false);
            if (!stop.load()) {
                VkSemaphoreWaitInfo semaphoreWaitInfo = {};
                semaphoreWaitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
                semaphoreWaitInfo.semaphoreCount = 1;
                semaphoreWaitInfo.pSemaphores = &GPUContext::instance().getSharedSemaphore(0)[0].getHandle();
                semaphoreWaitInfo.pValues = &GPUContext::instance().getSharedSemaphore(0)[0].getValue();
                //vkWaitSemaphores(GPUContext::instance().getDevice().getDevice(), &semaphoreWaitInfo, UINT64_MAX);
                //std::cout<<"frame : "<<parentRenderer.getCurrentFrame()<<" ready!"<<std::endl;
                if (needToUpdateDescriptorSets) {
                    //std::cout<<"update ds"<<std::endl;
                    updateDescriptorSets();
                    needToUpdateDescriptorSets = false;
                }
                jobFence[parentRenderer.getCurrentFrame()].reset(2);
                threadPool.enqueue([this] {
                    bool useDepthTest = parentRenderer.useDepthTest();
                    bool useStencilTest = parentRenderer.useStencilTest();
                    parentRenderer.setDepthStencil(false, false);
                    VkCommandBufferInheritanceRenderingInfo inheritanceRenderingInfo{};
                    inheritanceRenderingInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO;
                    inheritanceRenderingInfo.colorAttachmentCount = 1;
                    inheritanceRenderingInfo.pColorAttachmentFormats = &parentRenderer.getImageFormat(),
                    inheritanceRenderingInfo.depthAttachmentFormat = parentRenderer.getDepthStencilTexture().getFormat();    // VK_FORMAT_D32_SFLOAT, etc.                    
                    inheritanceRenderingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;                    
                    VkCommandBufferInheritanceInfo inheritanceInfo{};
                    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                    inheritanceInfo.pNext = &inheritanceRenderingInfo;
                    inheritanceInfo.renderPass = VK_NULL_HANDLE;
                    inheritanceInfo.framebuffer = VK_NULL_HANDLE;
                    linkedListCmdPool.beginRecordCommandBuffer(parentRenderer.getCurrentFrame(), inheritanceInfo);
                    BlendMode blendMode;
                    RenderStates states;
                    states.shader = &linkedListShader;
                    states.blendMode = blendMode;
                    linkedListPC.maxNodes = maxNodes;
                    linkedListPC.currentImageIndex = parentRenderer.getImageIndex();
                    std::vector<VkDescriptorSet> sets;
                    for (unsigned int i = 0; i < GPUContext::instance().getDescriptorSets(linkedListShader).size(); i++) {
                        //std::cout<<"set : "<<linkedListSets[i][0].getHandle()<<std::endl;
                        sets.push_back(GPUContext::instance().getDescriptorSets(linkedListShader)[i][0].getHandle());
                    }
                    blendMode.updateIds();
                    for (unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {
                        //std::cout<<"sizes = "<<linkedListPipeline.size()<<","<<linkedListPipeline[i].size()<<",ids : "<<i<<","<<RenderTarget::NODEPTHNOSTENCIL * blendMode.nbBlendModes+blendMode.id<<std::endl;
                        //std::cout<<"bind pipeline : "<<linkedListPipeline[i][RenderTarget::NODEPTHNOSTENCIL * blendMode.nbBlendModes+blendMode.id].getHandle()<<std::endl;
                        vkCmdBindPipeline(linkedListCmdPool.getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS,GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), linkedListShader, blendMode, RenderTarget::NODEPTHNOSTENCIL).getHandle());
                        //std::cout<<"pipeline bound"<<std::endl;
                        vkCmdBindDescriptorSets(linkedListCmdPool.getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS, GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), linkedListShader, blendMode, RenderTarget::NODEPTHNOSTENCIL).getLayout(), 0, sets.size(), sets.data(), 0, nullptr);
                        viewProjMatPC.projMatrix = parentRenderer.getCamera().getProjMatrix().getMatrix().transpose();
                        viewProjMatPC.viewMatrix = parentRenderer.getCamera().getViewMatrix().getMatrix().transpose();
                        viewProjMatPC.currentFrame = parentRenderer.getCurrentFrame();
                        viewProjMatPC.primitiveType = i; 
                        vkCmdPushConstants(linkedListCmdPool.getHandle(parentRenderer.getCurrentFrame()), GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), linkedListShader, blendMode, RenderTarget::NODEPTHNOSTENCIL).getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ViewProjMatPC), &viewProjMatPC);
                        vkCmdPushConstants(linkedListCmdPool.getHandle(parentRenderer.getCurrentFrame()), GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), linkedListShader, blendMode, RenderTarget::NODEPTHNOSTENCIL).getLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(ViewProjMatPC), sizeof(LinkedListPC), &linkedListPC);
                        parentRenderer.draw(linkedListCmdPool, static_cast<entity::PrimitiveType>(i), states);
                    }
                    linkedListCmdPool.endRecordCommandBuffer(parentRenderer.getCurrentFrame());
                    parentRenderer.setDepthStencil(useDepthTest, useStencilTest);
                    jobFence[parentRenderer.getCurrentFrame()].jobDone();
                });
                threadPool.enqueue([this] {
                    bool useDepthTest = parentRenderer.useDepthTest();
                    bool useStencilTest = parentRenderer.useStencilTest();
                    parentRenderer.setDepthStencil(false, false);
                    VkCommandBufferInheritanceRenderingInfo inheritanceRenderingInfo{};
                    inheritanceRenderingInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO;
                    inheritanceRenderingInfo.colorAttachmentCount = 1;
                    inheritanceRenderingInfo.pColorAttachmentFormats = &parentRenderer.getImageFormat(),
                    inheritanceRenderingInfo.depthAttachmentFormat = parentRenderer.getDepthStencilTexture().getFormat();    // VK_FORMAT_D32_SFLOAT, etc.                    
                    inheritanceRenderingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
                    VkCommandBufferInheritanceInfo inheritanceInfo{};
                    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                    inheritanceInfo.pNext = &inheritanceRenderingInfo;
                    inheritanceInfo.renderPass = VK_NULL_HANDLE;
                    inheritanceInfo.framebuffer = VK_NULL_HANDLE;
                    quadLinkedListCommandPool.beginRecordCommandBuffer(parentRenderer.getCurrentFrame(), inheritanceInfo);
                    BlendMode blendMode;
                    RenderStates states;
                    states.shader = &quadLinkedListShader;
                    states.blendMode = blendMode;
                    unsigned int currentFrame = parentRenderer.getCurrentFrame();
                    std::vector<VkDescriptorSet> sets;
                    for (unsigned int i = 0; i < GPUContext::instance().getDescriptorSets(quadLinkedListShader).size(); i++) {
                        sets.push_back( GPUContext::instance().getDescriptorSets(quadLinkedListShader)[i][0].getHandle());
                    }
                    blendMode.updateIds();
                    //std::cout<<"sizes = "<<quadLinkedListPipeline.size()<<","<<quadLinkedListPipeline[Triangles].size()<<",ids : "<<Triangles<<","<<RenderTarget::NODEPTHNOSTENCIL * blendMode.nbBlendModes+blendMode.id<<std::endl;
                    //std::cout<<"bind pipeline : "<<quadLinkedListPipeline[Triangles][RenderTarget::NODEPTHNOSTENCIL * blendMode.nbBlendModes+blendMode.id].getHandle()<<std::endl;
                    vkCmdBindPipeline(quadLinkedListCommandPool.getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS,GPUContext::instance().getGraphicsPipeline(entity::PrimitiveType::Triangles, quadLinkedListShader, blendMode, RenderTarget::NODEPTHNOSTENCIL).getHandle());
                    //std::cout<<"pipeline bound"<<std::endl;
                    vkCmdBindDescriptorSets(quadLinkedListCommandPool.getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS, GPUContext::instance().getGraphicsPipeline(entity::PrimitiveType::Triangles, quadLinkedListShader, blendMode, RenderTarget::NODEPTHNOSTENCIL).getLayout(), 0, sets.size(), sets.data(), 0, nullptr);
                    vkCmdPushConstants(quadLinkedListCommandPool.getHandle(parentRenderer.getCurrentFrame()), GPUContext::instance().getGraphicsPipeline(entity::PrimitiveType::Triangles, quadLinkedListShader, blendMode, RenderTarget::NODEPTHNOSTENCIL).getLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(unsigned int), &currentFrame);
                    parentRenderer.draw(quadLinkedListCommandPool, fullScreenQuad, states);
                    quadLinkedListCommandPool.endRecordCommandBuffer(parentRenderer.getCurrentFrame());
                    parentRenderer.setDepthStencil(useDepthTest, useStencilTest);
                    jobFence[parentRenderer.getCurrentFrame()].jobDone();
                });
                jobFence[parentRenderer.getCurrentFrame()].wait();
            }
            commandBuffersReady[parentRenderer.getCurrentFrame()].store(true);
            cv.notify_all();
        }
        void LinkedListRenderer::draw() {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] {
                    //std::cout<<"draw frame : "<<frameBuffer.getCurrentFrame()<<std::endl;
                return commandBuffersReady[parentRenderer.getCurrentFrame()].load() || stop.load();
            });
            commandBuffersReady[parentRenderer.getCurrentFrame()].store(false);
            if (!stop.load()) {
                bool useDepthTest = parentRenderer.useDepthTest();
                bool useStencilTest = parentRenderer.useStencilTest();
                parentRenderer.setDepthStencil(false, false);                
                //parentRenderer.applyComputeGraphicsBarrier();
                //std::cout<<"commands : !"<<parentRenderer.getCurrentFrame()<<" registered!"<<std::endl;
                parentRenderer.beginRendering(true);
                vkCmdExecuteCommands(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), 1, &linkedListCmdPool.getHandle(parentRenderer.getCurrentFrame()));
                parentRenderer.endRendering();
                VkMemoryBarrier memoryBarrier{};
                memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                memoryBarrier.pNext = VK_NULL_HANDLE;
                memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                vkCmdPipelineBarrier(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                parentRenderer.beginRendering(true);
                vkCmdExecuteCommands(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), 1, &quadLinkedListCommandPool.getHandle(parentRenderer.getCurrentFrame()));
                parentRenderer.endRendering();
                parentRenderer.setDepthStencil(useDepthTest, useStencilTest);
            }
        }
        bool LinkedListRenderer::isRendererReady() {
            return rendererReady.load();
        }
        unsigned int LinkedListRenderer::getLayer() {
            return layer;
        }    
        void LinkedListRenderer::onSwapchainResized(math::Vector2i newSize) {

        }    
    }
}
