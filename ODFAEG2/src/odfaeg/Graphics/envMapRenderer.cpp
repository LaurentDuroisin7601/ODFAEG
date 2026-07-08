module;
module odfaeg.Graphics.envMapRenderer;
namespace odfaeg {
    namespace graphic {
        EnvMapRenderer::EnvMapRenderer(RenderTarget& parentRenderer, unsigned int layer, std::string typesToRenderExpression, bool usethread) : 
          fullScreenQuad(GPUContext::instance().getDevice()),
          envMap(GPUContext::instance().getDevice()),
          envMapShader(GPUContext::instance().getDevice()),
          reflRefrShader(GPUContext::instance().getDevice()),
          commandPool(GPUContext::instance().getDevice()),
          envMapCmdPool(GPUContext::instance().getDevice()),
          reflRefrCmdPool(GPUContext::instance().getDevice()),
          threadPool(6),
          typesToRenderExpression(typesToRenderExpression),
          layer(layer)
        {
            rendererReady.store(false);
            envMap.createCubeMap(ENV_MAP_SIZE, false, false);
            envMap.getCamera().setViewport(physic::BoundingBox(0, 0, parentRenderer.getCamera().getViewport().getPosition().z(), ENV_MAP_SIZE, ENV_MAP_SIZE, parentRenderer.getCamera().getViewport().getSize().z()));
            createCommandPools();
            commandPool.beginRecordCommandBuffer(0);
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT*6; i++) {
                headPtrsStorageImage.emplace_back(GPUContext::instance().getDevice());
                linkedListBuffer.emplace_back(GPUContext::instance().getDevice());
                nodeCounterBuffer.emplace_back(GPUContext::instance().getDevice());
                headPtrsStorageImage.back().create(size.x(), size.y(), 1, VK_IMAGE_TYPE_2D, VK_FORMAT_R32_UINT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VMA_MEMORY_USAGE_GPU_ONLY,
                    1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL);
                Texture::transitionImageLayout(headPtrsStorageImage.back(), commandPool.getHandle(0), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
                headPtrsStorageImage.back().createImageView(VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R32_UINT, VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1, 1);
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
            fullScreenQuad.resize(4, 0);
            //All the screen area (in NDC coords)
            fullScreenQuad[0] = Vertex(math::Vec3f(-1.f, -1.f, 0.f));
            fullScreenQuad[1] = Vertex(math::Vec3f(-1.f, 1.f, 0.f));
            fullScreenQuad[2] = Vertex(math::Vec3f(1.f, 1.f, 0.f));
            fullScreenQuad[3] = Vertex(math::Vec3f(1.f, -1.f, 0.f));
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
            if (reflRefrShader.loadFromFile(shaderDir + "/reflectRefract.vert", shaderDir + "/reflectRefract.frag")) {
                throw std::runtime_error("Could not load refl/refr shader");
            }
            createDescriptorsAndPipelines();
        }
        void EnvMapRenderer::createCommandPools() {
            Device::QueueFamilyIndices queueFamilyIndices = GPUContext::instance().getDevice().findQueueFamilies(GPUContext::instance().getDevice().getPhysicalDevice());
            commandPool.create(queueFamilyIndices.graphicsFamily.value());
            commandPool.createCommandBuffers(true, MAX_FRAMES_IN_FLIGHT);
            envMapCmdPool.create(queueFamilyIndices.graphicsFamily.value());
            envMapCmdPool.createCommandBuffers(true, MAX_FRAMES_IN_FLIGHT);
            reflRefrCmdPool.create(queueFamilyIndices.graphicsFamily.value());
            reflRefrCmdPool.createCommandBuffers(true, MAX_FRAMES_IN_FLIGHT);
        }
        void EnvMapRenderer::createDescriptorsAndPipelines() {
            DescriptorSetLayout& envMapLayout = GPUContext::instance().getDescriptorSetLayout(envMapShader, 7,true);
            envMapLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_VERTEX_BIT);
            envMapLayout.updateLayout(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_VERTEX_BIT); 
            envMapLayout.updateLayout(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_FRAGMENT_BIT);
            envMapLayout.updateLayout(3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_FRAMES_IN_FLIGHT*6, VK_SHADER_STAGE_FRAGMENT_BIT);
            envMapLayout.updateLayout(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT*6, VK_SHADER_STAGE_FRAGMENT_BIT);
            envMapLayout.updateLayout(5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT*6, VK_SHADER_STAGE_FRAGMENT_BIT);
            envMapLayout.updateLayout(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
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
                GPUContext::instance().getGraphicsPipeline(static_cast<PrimitiveType>(i), envMapShader, blendMode, RenderTarget::NODEPTHNOSTENCIL).createGraphicPipeline(envMapShader, static_cast<PrimitiveType>(i), GPUContext::instance().getDescriptorSetLayout(envMapShader), renderingCreateInfo, envMap.getDepthStencilInfos()[RenderTarget::NODEPTHNOSTENCIL], blendMode, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, pushConstants);                
            }
            DescriptorSetLayout& reflRefrLayout = GPUContext::instance().getDescriptorSetLayout(reflRefrShader, 2);
            reflRefrLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_FRAGMENT_BIT);
            reflRefrLayout.updateLayout(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, parentRenderer.getSwapchainImages().size(), VK_SHADER_STAGE_FRAGMENT_BIT);
            reflRefrLayout.update();
            renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
            renderingCreateInfo.colorAttachmentCount = 1;
            renderingCreateInfo.pColorAttachmentFormats = &parentRenderer.getTexture().getFormat();
            renderingCreateInfo.depthAttachmentFormat = parentRenderer.getDepthStencilTexture().getFormat();
            pushConstants.clear();
            vertexPCRange.offset = 0;
            vertexPCRange.size = sizeof(ReflRefrVertPC);
            vertexPCRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            pushConstants.push_back(vertexPCRange);            
            fragmentPCRange.offset = sizeof(ReflRefrertPC);
            fragmentPCRange.size = sizeof(ReflRefrFragPC);
            fragmentPCRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            pushConstants.push_back(fragmentPCRange);
            for (unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {   
                GPUContext::instance().getGraphicsPipeline(static_cast<PrimitiveType>(i), reflRefrShader, blendMode, RenderTarget::DEPTHNOSTENCIL).createGraphicPipeline(reflRefrShader, static_cast<PrimitiveType>(i), GPUContext::instance().getDescriptorSetLayout(reflRefrShader), renderingCreateInfo, parentRenderer.getDepthStencilInfos()[RenderTarget::DEPTHNOSTENCIL], blendMode, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, pushConstants);                
            }
        }
    }
}