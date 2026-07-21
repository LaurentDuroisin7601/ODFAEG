module;
#include <string>
#include <odfaeg/config.hpp>
#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.h"
import odfaeg.core.delegate;
import odfaeg.math.vec;
import odfaeg.math.matrix;
import odfaeg.window.command;
import odfaeg.graphic.gpuContext;
import odfaeg.graphic.renderTarget;
import odfaeg.physic.boundingBox;
import odfaeg.graphic.renderTexture;
import odfaeg.graphic.texture;
import odfaeg.graphic.device;
import odfaeg.entity.vertex;
import odfaeg.entity.gameObject;
import odfaeg.graphic.descriptor;
import odfaeg.graphic.blendMode;
import odfaeg.entity.primitiveType;
import odfaeg.graphic.camera;
module odfaeg.graphic.envMapRenderer;
import odfaeg.graphic.mesh;
import odfaeg.graphic.buffer;
import odfaeg.graphic.renderStates;
namespace odfaeg {
    namespace graphic {
        EnvMapRenderer::EnvMapRenderer(RenderTarget& parentRenderer, unsigned int layer, std::string typesToRenderExpression, bool useThread) : 
          parentRenderer(parentRenderer),
          fullScreenQuad(GPUContext::instance().getDevice()),
          envMap(GPUContext::instance().getDevice()),
          envMapShader(GPUContext::instance().getDevice()),
          envMapQuadShader(GPUContext::instance().getDevice()),
          reflRefrShader(GPUContext::instance().getDevice()),
          commandPool(GPUContext::instance().getDevice()),          
          threadPool(6),
          typesToRenderExpression(typesToRenderExpression),
          layer(layer),
          staggingViewMatricesBuffer(GPUContext::instance().getDevice()),
          useThread(useThread)
        {
            rendererReady.store(false);
            envMap.createCubeMap(ENV_MAP_SIZE, false, false);
            envMap.getCamera().setViewport(physic::BoundingBox(0, 0, parentRenderer.getCamera().getViewport().getPosition().z(), ENV_MAP_SIZE, ENV_MAP_SIZE, parentRenderer.getCamera().getViewport().getSize().z()));
            createCommandPools();
            maxNodes = 20 * ENV_MAP_SIZE * ENV_MAP_SIZE;
            envMapFragPC.maxNodes = maxNodes;
            unsigned int nodeSize = 5 * sizeof(float) + sizeof(unsigned int);
            commandPool.beginRecordCommandBuffer(0);            
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT*6; i++) {
                headPtrsStorageImage.emplace_back(GPUContext::instance().getDevice());
                linkedListBuffer.emplace_back(GPUContext::instance().getDevice());
                nodeCounterBuffer.emplace_back(GPUContext::instance().getDevice());
                headPtrsStorageImage.back().create(ENV_MAP_SIZE, ENV_MAP_SIZE, 1, VK_IMAGE_TYPE_2D, VK_FORMAT_R32_UINT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VMA_MEMORY_USAGE_GPU_ONLY,
                    1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL);
                Texture::transitionImageLayout(headPtrsStorageImage.back(), commandPool.getHandle(0), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
                headPtrsStorageImage.back().createImageView(VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R32_UINT, VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1, 1);
                //headPtrsStorageImage.back().createSampler(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 1, false, false);
                nodeCounterBuffer.back().create(sizeof(std::uint32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
                linkedListBuffer.back().create(maxNodes * nodeSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);                
            }      
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {    
                viewMatricesBuffer.emplace_back(GPUContext::instance().getDevice());
                viewMatricesBuffer.back().create(sizeof(EnvViewMatrix), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
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
            std::string shaderDir = std::string(ODFAEG_INSTALL_DIR) + "/Shader";
            if (envMapShader.loadFromFile(shaderDir + "/envMap.vert", shaderDir + "/envMap.frag")) {
                throw std::runtime_error("Could not load env map shader");
            }
            if (envMapQuadShader.loadFromFile(shaderDir + "linkedListQuad.vert", shaderDir + "linkedListQuad.frag")) {
                throw std::runtime_error("Could not load env linked list quad shader");
            }            
            if (reflRefrShader.loadFromFile(shaderDir + "/reflectRefract.vert", shaderDir + "/reflectRefract.frag")) {
                throw std::runtime_error("Could not load refl/refr shader");
            }
            dirs[0] = math::Vec3f(1, 0, 0);
            dirs[1] = math::Vec3f(-1, 0, 0);
            dirs[2] = math::Vec3f(0, 1, 0);
            dirs[3] = math::Vec3f(0, -1, 0);
            dirs[4] = math::Vec3f(0, 0, 1);
            dirs[5] = math::Vec3f(0, 0, -1);
            ups[0] = math::Vec3f(0, -1, 0);
            ups[1] = math::Vec3f(0, -1, 0);
            ups[2] = math::Vec3f(0, 0, 1);
            ups[3] = math::Vec3f(0, 0, -1);
            ups[4] = math::Vec3f(0, -1, 0);
            ups[5] = math::Vec3f(0, -1, 0);
            createDescriptorsAndPipelines();
            window::Command rendererReadyCmd(core::FastDelegate<bool>(&EnvMapRenderer::isRendererReady, this), core::FastDelegate<void>(&EnvMapRenderer::drawNextFrame, this));
            eventListener.connect("RendererReady",rendererReadyCmd); 
            if (useThread) {
                //std::cout<<"lanch"<<std::endl;
                eventListener.launch();
            } 
            needToUpdateDescriptorSets = true;
            needToUpdateBuffers = false;
            stop.store(false);
            rendererReady.store(true);
        }
        void EnvMapRenderer::addReflRefrGameObject(Mesh* gameObject) {
            /*for (unsigned int i = 0; i < gameObject->getGameObject->getChildren().size(); i++) {
                addReflRefrGameObject(gameObject->getGameObject->getChildren()[i]);
            }*/
            gameObject->populateVertexBuffers();
            reflRefrGameObjects.push_back(gameObject);
            needToUpdateBuffers = true;
            needToUpdateDescriptorSets = true;
        }
        void EnvMapRenderer::createCommandPools() {
            Device::QueueFamilyIndices queueFamilyIndices = GPUContext::instance().getDevice().findQueueFamilies(GPUContext::instance().getDevice().getPhysicalDevice());
            commandPool.create(queueFamilyIndices.graphicsFamily.value());
            commandPool.createCommandBuffers(true, MAX_FRAMES_IN_FLIGHT);            
            for (unsigned int i = 0; i < reflRefrGameObjects.size(); i++) {
                envMapCmdPools.emplace_back(GPUContext::instance().getDevice());
                envMapCmdPools[i].create(queueFamilyIndices.graphicsFamily.value());
                envMapCmdPools[i].createCommandBuffers(true, MAX_FRAMES_IN_FLIGHT);
            }            
            for (unsigned int i = 0; i < reflRefrGameObjects.size(); i++) {
                envMapQuadCmdPools.emplace_back(GPUContext::instance().getDevice());
                envMapQuadCmdPools[i].create(queueFamilyIndices.graphicsFamily.value());
                envMapQuadCmdPools[i].createCommandBuffers(true, MAX_FRAMES_IN_FLIGHT);
            }            
            for (unsigned int i = 0; i < reflRefrGameObjects.size(); i++) {
                reflRefrCmdPools.emplace_back(GPUContext::instance().getDevice());
                reflRefrCmdPools[i].create(queueFamilyIndices.graphicsFamily.value());
                reflRefrCmdPools[i].createCommandBuffers(true, MAX_FRAMES_IN_FLIGHT);
            }
        }
        void EnvMapRenderer::createDescriptorsAndPipelines() {
            DescriptorSetLayout& envMapLayout = GPUContext::instance().getDescriptorSetLayout(envMapShader, 7,true);
            envMapLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_VERTEX_BIT);
            envMapLayout.updateLayout(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_VERTEX_BIT); 
            envMapLayout.updateLayout(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_FRAGMENT_BIT);
            envMapLayout.updateLayout(3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_FRAMES_IN_FLIGHT*6, VK_SHADER_STAGE_FRAGMENT_BIT);
            envMapLayout.updateLayout(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT*6, VK_SHADER_STAGE_FRAGMENT_BIT);
            envMapLayout.updateLayout(5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT*6, VK_SHADER_STAGE_FRAGMENT_BIT);
            envMapLayout.updateLayout(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
            envMapLayout.update();
            std::vector<VkPushConstantRange> pushConstants;
            VkPushConstantRange vertexPCRange;
            vertexPCRange.offset = 0;
            vertexPCRange.size = sizeof(EnvMapVertPC);
            vertexPCRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            pushConstants.push_back(vertexPCRange);
            VkPushConstantRange fragmentPCRange;
            fragmentPCRange.offset = sizeof(EnvMapVertPC);
            fragmentPCRange.size = sizeof(EnvMapFragPC);
            fragmentPCRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            pushConstants.push_back(fragmentPCRange);
            BlendMode blendMode;
            blendMode.updateIds();
            VkPipelineRenderingCreateInfo renderingCreateInfo{};
            renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
            renderingCreateInfo.colorAttachmentCount = 1;
            renderingCreateInfo.pColorAttachmentFormats = &envMap.getTexture().getFormat();
            renderingCreateInfo.depthAttachmentFormat = envMap.getDepthStencilTexture().getFormat();
            renderingCreateInfo.viewMask = envMap.getViewMask();
            for (unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {   
                GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), envMapShader, blendMode, RenderTarget::NODEPTHNOSTENCIL).createGraphicPipeline(envMapShader, static_cast<entity::PrimitiveType>(i), GPUContext::instance().getDescriptorSetLayout(envMapShader), renderingCreateInfo, envMap.getDepthStencilInfos()[RenderTarget::NODEPTHNOSTENCIL], blendMode, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, pushConstants);                
            }
            DescriptorSetLayout& quadLinkedListLayout = GPUContext::instance().getDescriptorSetLayout(envMapQuadShader, 2);
            quadLinkedListLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_FRAGMENT_BIT);
            quadLinkedListLayout.updateLayout(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_FRAGMENT_BIT);
            quadLinkedListLayout.update();
            pushConstants.clear();
            VkPushConstantRange quadPushConstant;
            quadPushConstant.offset = 0;
            quadPushConstant.size = sizeof(unsigned int);
            quadPushConstant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            pushConstants.push_back(quadPushConstant);
            GPUContext::instance().getGraphicsPipeline(entity::PrimitiveType::Triangles, envMapQuadShader, blendMode, 0).createGraphicPipeline(envMapQuadShader, entity::PrimitiveType::Triangles, GPUContext::instance().getDescriptorSetLayout(envMapQuadShader), renderingCreateInfo,parentRenderer.getDepthStencilInfos()[RenderTarget::NODEPTHNOSTENCIL], blendMode, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, pushConstants);            
            DescriptorSetLayout& reflRefrLayout = GPUContext::instance().getDescriptorSetLayout(reflRefrShader, 2);
            reflRefrLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_FRAGMENT_BIT);
            reflRefrLayout.updateLayout(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, parentRenderer.getSwapchainImagesCount(), VK_SHADER_STAGE_FRAGMENT_BIT);
            reflRefrLayout.update();
            renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
            renderingCreateInfo.colorAttachmentCount = 1;
            renderingCreateInfo.pColorAttachmentFormats = &parentRenderer.getImageFormat();
            renderingCreateInfo.depthAttachmentFormat = parentRenderer.getDepthStencilTexture().getFormat();
            pushConstants.clear();
            vertexPCRange.offset = 0;
            vertexPCRange.size = sizeof(ReflRefrVertPC);
            vertexPCRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            pushConstants.push_back(vertexPCRange);            
            fragmentPCRange.offset = sizeof(ReflRefrVertPC);
            fragmentPCRange.size = sizeof(ReflRefrFragPC);
            fragmentPCRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            pushConstants.push_back(fragmentPCRange);
            for (unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {   
                GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), reflRefrShader, blendMode, RenderTarget::DEPTHNOSTENCIL).createGraphicPipeline(reflRefrShader, static_cast<entity::PrimitiveType>(i), GPUContext::instance().getDescriptorSetLayout(reflRefrShader), renderingCreateInfo, parentRenderer.getDepthStencilInfos()[RenderTarget::DEPTHNOSTENCIL], blendMode, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, pushConstants);                
            }
            DescriptorPool& envMapPool = GPUContext::instance().getDescriptorPool(envMapShader, 7);
            envMapPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT);
            envMapPool.updatePoolSize(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT);
            envMapPool.updatePoolSize(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT);
            envMapPool.updatePoolSize(3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_FRAMES_IN_FLIGHT*6);
            envMapPool.updatePoolSize(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT*6);
            envMapPool.updatePoolSize(5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT*6);
            envMapPool.updatePoolSize(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
            envMapPool.update();
            DescriptorPool& quadLinkedListPool = GPUContext::instance().getDescriptorPool(envMapQuadShader, 2);
            quadLinkedListPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_FRAMES_IN_FLIGHT);
            quadLinkedListPool.updatePoolSize(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT);
            quadLinkedListPool.update();
            DescriptorPool& reflRefrPool = GPUContext::instance().getDescriptorPool(reflRefrShader, 2);
            reflRefrPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT);
            reflRefrPool.updatePoolSize(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, parentRenderer.getSwapchainImagesCount());  
            reflRefrPool.update();
            DescriptorSet::allocate(envMapPool, envMapLayout, GPUContext::instance().getDescriptorSets(envMapShader, 6, 1), MAX_TEXTURES);
            DescriptorSet::allocate(quadLinkedListPool, quadLinkedListLayout, GPUContext::instance().getDescriptorSets(envMapQuadShader, 2, 1));
            DescriptorSet::allocate(reflRefrPool, reflRefrLayout, GPUContext::instance().getDescriptorSets(reflRefrShader, 2, 1));          
        }
        void EnvMapRenderer::updateBuffers() { 
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(GPUContext::instance().getDevice().getPhysicalDevice(), &props); 
            uint32_t minAlign = props.limits.minStorageBufferOffsetAlignment;  
            uint32_t viewMatrixAlignSize = (sizeof(EnvViewMatrix) + minAlign - 1) & ~(minAlign - 1);              
            std::vector<EnvViewMatrix> viewMatrices;
            for (unsigned int r = 0; r < reflRefrGameObjects.size(); r++) {
                EnvViewMatrix envViewMatrices;                
                Camera envMapCamera = envMap.getCamera();
                envMapCamera.setCenter(reflRefrGameObjects[r]->getGameObject()->getCenter());
                for (unsigned int m = 0; m < 6; m++) {
                    math::Vec3f target = envMapCamera.getCenter() + dirs[m];
                    envMapCamera.lookAt(target.x(), target.y(), target.z(), ups[m]);    
                    envViewMatrices.viewMatrices[m] = envMapCamera.getViewMatrix().getMatrix().transpose();                                    
                }
                viewMatrices.push_back(envViewMatrices);            
            }
            staggingViewMatricesBuffer.create(sizeof(EnvViewMatrix)*reflRefrGameObjects.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
            for (unsigned int i = 0; i < viewMatrices.size(); i++) {
                staggingViewMatricesBuffer.update(viewMatrices.data(), sizeof(EnvViewMatrix), i * viewMatrixAlignSize);
            }
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {  
                commandPool.beginRecordCommandBuffer(i);  
                viewMatricesBuffer.emplace_back(GPUContext::instance().getDevice());
                viewMatricesBuffer.back().create(sizeof(EnvViewMatrix)*reflRefrGameObjects.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
                viewMatricesBuffer.back().setRange(sizeof(EnvViewMatrix));
                Buffer::copyBuffer(staggingViewMatricesBuffer, viewMatricesBuffer.back(), sizeof(EnvViewMatrix)*viewMatrices.size(), commandPool.getHandle(i));   
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
        void EnvMapRenderer::updateDescriptorSets() {
            bool hasDiffuseTexture = GPUContext::instance().getSharedTextures(entity::SubMesh::DIFFUSE).size() != 0;
            DescriptorSet& envMapSet = GPUContext::instance().getDescriptorSets(envMapShader, (hasDiffuseTexture) ? 6 : 5, 1)[0];
            envMapSet.updateBufferInfos(0, GPUContext::instance().getSharedBuffers(RenderTarget::OUTPUT_MODELS), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            envMapSet.updateBufferInfos(1, viewMatricesBuffer,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);
            envMapSet.updateBufferInfos(2, GPUContext::instance().getSharedBuffers(RenderTarget::OUTPUT_MATERIALS), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            envMapSet.updateImageInfos(3, headPtrsStorageImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
            envMapSet.updateBufferInfos(4, linkedListBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            envMapSet.updateBufferInfos(5, nodeCounterBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            if (hasDiffuseTexture) {
                envMapSet.updateImageInfos(6, GPUContext::instance().getSharedTextures(entity::SubMesh::DIFFUSE), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            }
            envMapSet.updateDescriptorSet();
            DescriptorSet& linkedListQuadSet = GPUContext::instance().getDescriptorSets(envMapQuadShader, 2, 1)[0];
            linkedListQuadSet.updateImageInfos(0, headPtrsStorageImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
            linkedListQuadSet.updateBufferInfos(1, linkedListBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            linkedListQuadSet.updateDescriptorSet();
            DescriptorSet& reflRefrSet = GPUContext::instance().getDescriptorSets(reflRefrShader, 2, 1)[0];
            reflRefrSet.updateBufferInfos(0, GPUContext::instance().getSharedBuffers(RenderTarget::OUTPUT_MATERIALS), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            reflRefrSet.updateImageInfos(1, envMap.getTexture(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            reflRefrSet.updateDescriptorSet();
        } 
        void EnvMapRenderer::clear() {
            parentRenderer.setTypesToRender(typesToRenderExpression, parentRenderer.getCurrentFrame());
            parentRenderer.applyCullingAndBatching();
            envMap.setTypesToRender(typesToRenderExpression, envMap.getCurrentFrame());
            envMap.applyCullingAndBatching();
            cv.notify_one();
        }  
        void EnvMapRenderer::drawNextFrame() {            
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] {
                    //std::cout<<"draw frame : "<<frameBuffer.getCurrentFrame()<<std::endl;
                return registerFramesJob[parentRenderer.getCurrentFrame()].load() || stop.load();
            });
            registerFramesJob[envMap.getCurrentFrame()].store(false);
            if (!stop.load()) {
                if (needToUpdateBuffers) {
                    createCommandPools();
                    updateBuffers();
                    needToUpdateBuffers = false;
                }
                if (needToUpdateDescriptorSets) {
                    //std::cout<<"update ds"<<std::endl;
                    updateDescriptorSets();
                    needToUpdateDescriptorSets = false;
                }
                VkPhysicalDeviceProperties props;
                vkGetPhysicalDeviceProperties(GPUContext::instance().getDevice().getPhysicalDevice(), &props); 
                uint32_t minAlign = props.limits.minStorageBufferOffsetAlignment;  
                uint32_t viewMatrixAlignSize = (sizeof(EnvViewMatrix) + minAlign - 1) & ~(minAlign - 1);              
                jobFence[parentRenderer.getCurrentFrame()].reset(reflRefrCmdPools.size()*2+1);
                for (unsigned int e = 0; e < reflRefrCmdPools.size(); e++) {
                    unsigned int cmp = e;                                  
                    threadPool.enqueue([this, cmp, viewMatrixAlignSize] { 
                        VkCommandBufferInheritanceRenderingInfo inheritanceRenderingInfo{};
                        inheritanceRenderingInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO;
                        inheritanceRenderingInfo.colorAttachmentCount = 1;
                        inheritanceRenderingInfo.pColorAttachmentFormats = &envMap.getImageFormat(),
                        inheritanceRenderingInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;                    
                        inheritanceRenderingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;                      
                        VkCommandBufferInheritanceInfo inheritanceInfo{};
                        inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                        inheritanceInfo.pNext = &inheritanceRenderingInfo;
                        inheritanceInfo.renderPass = VK_NULL_HANDLE;
                        inheritanceInfo.framebuffer = VK_NULL_HANDLE;
                        envMapCmdPools[cmp].beginRecordCommandBuffer(envMap.getCurrentFrame(), inheritanceInfo);
                        BlendMode blendMode;
                        RenderStates states;
                        states.shader = &envMapShader;
                        states.blendMode = blendMode;
                        envMapFragPC.maxNodes = maxNodes;
                        envMapFragPC.currentImageIndex = envMap.getImageIndex();
                        std::vector<uint32_t> offsetEnvViewMatrices;
                        for (unsigned int j = 0; j < MAX_FRAMES_IN_FLIGHT; j++) {
                            offsetEnvViewMatrices.push_back(cmp * viewMatrixAlignSize);
                        }
                        std::vector<VkDescriptorSet> sets;
                        for (unsigned int i = 0; i <  GPUContext::instance().getDescriptorSets(envMapShader).size(); i++) {
                            //std::cout<<"set : "<<linkedListSets[i][0].getHandle()<<std::endl;
                            sets.push_back(GPUContext::instance().getDescriptorSets(envMapShader)[i][0].getHandle());
                        }
                        blendMode.updateIds();
                        for (unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {
                            //std::cout<<"sizes = "<<linkedListPipeline.size()<<","<<linkedListPipeline[i].size()<<",ids : "<<i<<","<<RenderTarget::NODEPTHNOSTENCIL * blendMode.nbBlendModes+blendMode.id<<std::endl;
                            //std::cout<<"bind pipeline : "<<linkedListPipeline[i][RenderTarget::NODEPTHNOSTENCIL * blendMode.nbBlendModes+blendMode.id].getHandle()<<std::endl;
                            vkCmdBindPipeline(envMapCmdPools[cmp].getHandle(envMap.getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS,GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), envMapShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getHandle());
                            //std::cout<<"pipeline bound"<<std::endl;
                            vkCmdBindDescriptorSets(envMapCmdPools[cmp].getHandle(envMap.getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS, GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), envMapShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getLayout(), 0, sets.size(), sets.data(), offsetEnvViewMatrices.size(), offsetEnvViewMatrices.data());
                            vkCmdPushConstants(envMapCmdPools[cmp].getHandle(envMap.getCurrentFrame()), GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), envMapShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(EnvMapVertPC), &envMapVertPC);
                            vkCmdPushConstants(envMapCmdPools[cmp].getHandle(envMap.getCurrentFrame()), GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), envMapShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(EnvMapFragPC), &envMapFragPC);

                            parentRenderer.draw(envMapCmdPools[cmp], static_cast<entity::PrimitiveType>(i), states);
                        }
                        envMapCmdPools[cmp].endRecordCommandBuffer(envMap.getCurrentFrame());                        
                        jobFence[envMap.getCurrentFrame()].jobDone();
                    });                
                    threadPool.enqueue([this, cmp] { 
                        VkCommandBufferInheritanceInfo inheritanceInfo{};
                        inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                        inheritanceInfo.renderPass = VK_NULL_HANDLE;
                        inheritanceInfo.framebuffer = VK_NULL_HANDLE;
                        envMapQuadCmdPools[cmp].beginRecordCommandBuffer(parentRenderer.getCurrentFrame(), inheritanceInfo);
                        BlendMode blendMode;
                        RenderStates states;
                        states.shader = &envMapQuadShader;
                        states.blendMode = blendMode;
                        unsigned int currentFrame = parentRenderer.getCurrentFrame();
                        std::vector<VkDescriptorSet> sets;
                        for (unsigned int i = 0; i < GPUContext::instance().getDescriptorSets(envMapQuadShader).size(); i++) {
                            sets.push_back(GPUContext::instance().getDescriptorSets(envMapQuadShader)[i][0].getHandle());
                        }
                        blendMode.updateIds();
                        //std::cout<<"sizes = "<<quadLinkedListPipeline.size()<<","<<quadLinkedListPipeline[Triangles].size()<<",ids : "<<Triangles<<","<<RenderTarget::NODEPTHNOSTENCIL * blendMode.nbBlendModes+blendMode.id<<std::endl;
                        //std::cout<<"bind pipeline : "<<quadLinkedListPipeline[Triangles][RenderTarget::NODEPTHNOSTENCIL * blendMode.nbBlendModes+blendMode.id].getHandle()<<std::endl;
                        vkCmdBindPipeline(envMapQuadCmdPools[cmp].getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS,GPUContext::instance().getGraphicsPipeline(entity::PrimitiveType::Triangles, envMapQuadShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getHandle());
                        //std::cout<<"pipeline bound"<<std::endl;
                        vkCmdBindDescriptorSets(envMapQuadCmdPools[cmp].getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS, GPUContext::instance().getGraphicsPipeline(entity::PrimitiveType::Triangles, envMapQuadShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getLayout(), 0, sets.size(), sets.data(), 0, nullptr);
                        vkCmdPushConstants(envMapQuadCmdPools[cmp].getHandle(parentRenderer.getCurrentFrame()), GPUContext::instance().getGraphicsPipeline(entity::PrimitiveType::Triangles, envMapQuadShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(unsigned int), &currentFrame);
                        envMap.draw(envMapQuadCmdPools[cmp], fullScreenQuad, states);
                        envMapQuadCmdPools[cmp].endRecordCommandBuffer(parentRenderer.getCurrentFrame());                        
                        jobFence[parentRenderer.getCurrentFrame()].jobDone();
                    });                
                    threadPool.enqueue([this, cmp] {
                        VkCommandBufferInheritanceRenderingInfo inheritanceRenderingInfo{};
                        inheritanceRenderingInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO;
                        inheritanceRenderingInfo.colorAttachmentCount = 1;
                        inheritanceRenderingInfo.pColorAttachmentFormats = &parentRenderer.getImageFormat(),
                        inheritanceRenderingInfo.depthAttachmentFormat = parentRenderer.getDepthStencilTexture().getFormat();                    
                        inheritanceRenderingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;                      
                        VkCommandBufferInheritanceInfo inheritanceInfo{};
                        inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                        inheritanceInfo.pNext = &inheritanceRenderingInfo;
                        inheritanceInfo.renderPass = VK_NULL_HANDLE;
                        inheritanceInfo.framebuffer = VK_NULL_HANDLE;
                        reflRefrCmdPools[cmp].beginRecordCommandBuffer(envMap.getCurrentFrame(), inheritanceInfo);
                        BlendMode blendMode;
                        RenderStates states;
                        states.shader = &reflRefrShader;
                        states.blendMode = blendMode;                        
                        reflRefrFragPC.currentImageIndex = parentRenderer.getImageIndex();
                        std::vector<VkDescriptorSet> sets;
                        for (unsigned int i = 0; i <  GPUContext::instance().getDescriptorSets(reflRefrShader).size(); i++) {
                            //std::cout<<"set : "<<linkedListSets[i][0].getHandle()<<std::endl;
                            sets.push_back(GPUContext::instance().getDescriptorSets(reflRefrShader)[i][0].getHandle());
                        }
                        blendMode.updateIds();
                        //std::cout<<"sizes = "<<linkedListPipeline.size()<<","<<linkedListPipeline[i].size()<<",ids : "<<i<<","<<RenderTarget::NODEPTHNOSTENCIL * blendMode.nbBlendModes+blendMode.id<<std::endl;
                        //std::cout<<"bind pipeline : "<<linkedListPipeline[i][RenderTarget::NODEPTHNOSTENCIL * blendMode.nbBlendModes+blendMode.id].getHandle()<<std::endl;
                        
                        for (unsigned int v = 0; v < reflRefrGameObjects[cmp]->getVertexBuffers().size(); v++) {                                
                            vkCmdBindPipeline(reflRefrCmdPools[cmp].getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS,GPUContext::instance().getGraphicsPipeline(reflRefrGameObjects[cmp]->getVertexBuffers()[v].getPrimitiveType(), envMapShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getHandle());
                            //std::cout<<"pipeline bound"<<std::endl;
                            vkCmdBindDescriptorSets(reflRefrCmdPools[cmp].getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS, GPUContext::instance().getGraphicsPipeline(reflRefrGameObjects[cmp]->getVertexBuffers()[v].getPrimitiveType(), envMapShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getLayout(), 0, sets.size(), sets.data(), 0, nullptr);
                            vkCmdPushConstants(reflRefrCmdPools[cmp].getHandle(parentRenderer.getCurrentFrame()), GPUContext::instance().getGraphicsPipeline(reflRefrGameObjects[cmp]->getVertexBuffers()[v].getPrimitiveType(), reflRefrShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ReflRefrFragPC), &reflRefrFragPC);
                            parentRenderer.draw(reflRefrCmdPools[cmp], reflRefrGameObjects[cmp]->getVertexBuffers()[v], states);
                        }
                        reflRefrCmdPools[cmp].endRecordCommandBuffer(parentRenderer.getCurrentFrame());                   
                        jobFence[parentRenderer.getCurrentFrame()].jobDone();
                    });
                }
                jobFence[parentRenderer.getCurrentFrame()].wait();            
            }            
            commandBuffersReady[parentRenderer.getCurrentFrame()].store(true);
            cv.notify_all();                    
        } 
        void EnvMapRenderer::draw() {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] {
                    //std::cout<<"draw frame : "<<frameBuffer.getCurrentFrame()<<std::endl;
                return commandBuffersReady[parentRenderer.getCurrentFrame()].load() || stop.load();
            });
            commandBuffersReady[parentRenderer.getCurrentFrame()].store(false);
            for (unsigned int e = 0; e < reflRefrGameObjects.size(); e++) {
                envMap.clear();
                envMap.beginRendering();
                vkCmdExecuteCommands(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), 1, &envMapCmdPools[e].getHandle(parentRenderer.getCurrentFrame()));
                envMap.endRendering();
                VkMemoryBarrier memoryBarrier{};
                memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                memoryBarrier.pNext = VK_NULL_HANDLE;
                memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                vkCmdPipelineBarrier(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                envMap.beginRendering();
                vkCmdExecuteCommands(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), 1, &envMapQuadCmdPools[e].getHandle(parentRenderer.getCurrentFrame()));
                envMap.endRendering(); 
                envMap.submit(true);
                envMap.display();                
                vkCmdPipelineBarrier(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);               
                parentRenderer.beginRendering();
                vkCmdExecuteCommands(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), 1, &reflRefrCmdPools[e].getHandle(parentRenderer.getCurrentFrame()));
                parentRenderer.endRendering();
            }           
        }
        unsigned int EnvMapRenderer::getLayer() {
            return layer;
        }
        bool EnvMapRenderer::isRendererReady() {
            return rendererReady.load();
        }
    }
}