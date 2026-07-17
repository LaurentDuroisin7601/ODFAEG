module;
module odfaeg.Graphics.envMapRenderer;
namespace odfaeg {
    namespace graphic {
        EnvMapRenderer::EnvMapRenderer(RenderTarget& parentRenderer, unsigned int layer, std::string typesToRenderExpression, bool usethread) : 
          fullScreenQuad(GPUContext::instance().getDevice()),
          envMap(GPUContext::instance().getDevice()),
          envMapShader(GPUContext::instance().getDevice()),
          envMapQuadShader(GPUContext::instance().getDevice()),
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
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {    
                viewMatricesBuffer.emplace_back(GPUContext::instance().getDevice());
                viewMatricesBuffer.create(sizeof(math::Matrix4f)*6, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU);
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
        }
        void EnvMapRenderer::createCommandPools() {
            Device::QueueFamilyIndices queueFamilyIndices = GPUContext::instance().getDevice().findQueueFamilies(GPUContext::instance().getDevice().getPhysicalDevice());
            commandPool.create(queueFamilyIndices.graphicsFamily.value());
            commandPool.createCommandBuffers(true, MAX_FRAMES_IN_FLIGHT);
            envMapCmdPools.resize(reflRefGameObjects.size());
            for (unsigned int i = 0; i < envMapCmdPools.size(); i++) {
                envMapCmdPools[i].create(queueFamilyIndices.graphicsFamily.value());
                envMapCmdPools[i].createCommandBuffers(true, MAX_FRAMES_IN_FLIGHT);
            }
            envMapQuadCmdPools.resize(reflRefGameObjects.size());
            for (unsigned int i = 0; i < envMapQuadCmdPools.size(); i++) {
                envMapQuadCmdPools[i].create(queueFamilyIndices.graphicsFamily.value());
                envMapQuadCmdPools[i].createCommandBuffers(true, MAX_FRAMES_IN_FLIGHT);
            }
            reflRefrCmdPools.resize(reflRefGameObjects.size());
            for (unsigned int i = 0; i < reflRefrCmdPools.size(); i++) {
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
            DescriporPool& envMapPool = GPU::instance().getDescriptorPool(envMapShader, 7);
            envMapPool.updatePoolSizes(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT);
            envMapPool.updatePoolSizes(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT);
            envMapPool.updatePoolSizes(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT);
            envMapPool.updatePoolSizes(3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_FRAMES_IN_FLIGHT*6);
            envMapPool.updatePoolSizes(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT*6);
            envMapPool.updatePoolSizes(5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT*6);
            envMapPool.updatePoolSizes(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
            envMapPool.update();
            DescriptorPool& reflRefrPool = GPUContext::instance().getDescriptorSetLayout(reflRefrShader, 2);
            reflRefrPool.updatePoolSizes(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT);
            reflRefrPool.updatePoolSizes(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, parentRenderer.getSwapchainImages().size());  
            reflRefrPool.update();
            DescriptorSet::allocate(envMapPool, envMapLayout, GPUContext::instance().getDescriptorSets(envMapShader, 6, 1), MAX_TEXTURES);
            DescriptorSet::allocate(reflRefrPool, reflRefrLayout, GPUContext::instance().getDescriptorSets(refrReflShader, 2, 1));          
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
                envMapCamera.setCenter(reflRefrGameObjects[r].getCenter());
                for (unsigned int m = 0; m < 6; m++) {
                    math::Vec3f target = envMapCamera.getCenter() + dirs[m];
                    envMapCamera.lookAt(target.x(), target.y(), target.z(), ups[m]);    
                    envViewMatrices.viewMatrix[m] = envMapCamera.getViewMatrix().getMatrix().transpose();                                    
                }
                viewMatrices.push_back(envViewMatrices);            
            }
            staggingViewMatricesBuffer.create(sizeof(EnvViewMatrix)*reflRefrGameObjects.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU);
            for (unsigned int i = 0; i < viewMatrices.size(); i++) {
                staggingViewMatricesBuffer.update(viewMatrices.data(), sizeof(EnvViewMatrix), i * viewMatrixAlignSize);
            }
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {  
                commandPool.beginRecordCommandBuffer(i);  
                viewMatricesBuffer.emplace_back(GPUContext::instance().getDevice());
                viewMatricesBuffer.back().create(sizeof(EnvViewMatrix)*reflRefrGameObjects.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU);
                viewMatricesBuffer.back().setRange(sizeof(EnvViewMatrix));
                Buffer::copyBuffer(staggingViewMatrices, viewMatricesBuffer.back(), sizeof(EnvViewMatrix)*viewMatrices.size(), commandPool.getHandle(i));   
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
            bool hasDiffuseTexture = GPUContext::instance().getSharedTextures(Material::DIFFUSE).size() != 0;
            DescriptorSet& envMapSet = GPUContext::instance().getDescriptorSets(envMapShader, (hasDiffuseTexture) ? 6 : 5, 1)[0];
            envMapSet.updateBufferInfos(0, GPUContext::instance().getSharedBuffers(RenderTarget::OUTPUT_MODELS), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            envMapSet.updateImageInfos(1, viewMatricesBuffer,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);
            envMapSet.updateBufferInfos(2, GPUContext::instance().getSharedBuffers(RenderTarget::OUTPUT_MATERIALS), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            envMapSet.updateBufferInfos(3, headPtrsStorageImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
            envMapSet.updateBufferInfos(4, linkedList, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            envMapSet.updateBufferInfos(5, nodeCounterBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            if (hasDiffuseTexture) {
                envMapSet.updateBufferInfos(6, GPUContext::instance().getSharedTextures(Material::DIFFUSE), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            }
            envMapSet.updateDescriptorSets();
            DescriptorSet& reflRefrSet = GPUContext::instance().getDescriptorSets(reflRefrShader, 2, 1)[0];
            reflRefrSet.updateBufferInfos(0, GPUContext::instance().getSharedBuffers(RenderTarget::OUTPUT_MATERIALS), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_BIT);
            reflRefrSet.updateImageInfos(1, envMap.getTexture(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            reflRefrSet.updateDescriptorSets();
        }   
        void EnvMapRenderer::drawNextFrame() {            
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] {
                    //std::cout<<"draw frame : "<<frameBuffer.getCurrentFrame()<<std::endl;
                return registerFramesJob[parentRenderer.getCurrentFrame()].load() || stop.load();
            });
            registerFramesJob[envMap.getCurrentFrame()].store(false);
            if (!stop.load()) {
                if (needToUpdateDescriptorSets) {
                    //std::cout<<"update ds"<<std::endl;
                    updateDescriptorSets();
                    needToUpdateDescriptorSets = false;
                }
                jobFence[parentRenderer.getCurrentFrame()].reset(reflRefrCmdPools.size()*2+1);
                          );
                          VkPhysicalDeviceProperties props;
                vkGetPhysicalDeviceProperties(GPUContext::instance().getDevice().getPhysicalDevice(), &props); 
                uint32_t minAlign = props.limits.minStorageBufferOffsetAlignment;  
                uint32_t envViewMatrixAlignSize = (sizeof(EnvViewMatrix) + minAlign - 1) & ~(minAlign - 1); 
                for (unsigned int e = 0; e < reflRefrCmdPools.size(); e++) {
                    unsigned int cmp = e;
                    std::array<math::Matrix4f, 6> viewMatrices;
                    Camera envMapCamera = envMap.getCamera();
                    envMapCamera.setCenter(reflRefrGameObjects.getCenter());
                    for (unsigned int i = 0; i < 6; i++) {
                        math::Vec3f target = reflectView.getPosition() + dirs[m];
                        envMapCamera.lookAt(target.x(), target.y(), target.z(), ups[m]);                        
                    }                    
                    threadPool.enqueue([this, cmp] { 
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
                        envMapVertPC.maxNodes = maxNodes;
                        envMapFragPC.currentImageIndex = envMap.getImageIndex();
                        std::vector<uint32_t> offsetEnViewMatrices;
                        for (unsigned int j = 0; j < MAX_FRAMES_IN_FLIGHT; j++) {
                            offsetEnViewMatrices.push_back(e * minEnvViewMatrixAlignSize);
                        }
                        std::vector<VkDescriptorSet> sets;
                        for (unsigned int i = 0; i <  GPUContext::instance().getDescriptorSets(envMapShader).size(); i++) {
                            //std::cout<<"set : "<<linkedListSets[i][0].getHandle()<<std::endl;
                            sets.push_back(GPUContext::instance().getDescriptorSets(envMapShader)[0].getHandle());
                        }
                        blendMode.updateIds();
                        for (unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {
                            //std::cout<<"sizes = "<<linkedListPipeline.size()<<","<<linkedListPipeline[i].size()<<",ids : "<<i<<","<<RenderTarget::NODEPTHNOSTENCIL * blendMode.nbBlendModes+blendMode.id<<std::endl;
                            //std::cout<<"bind pipeline : "<<linkedListPipeline[i][RenderTarget::NODEPTHNOSTENCIL * blendMode.nbBlendModes+blendMode.id].getHandle()<<std::endl;
                            vkCmdBindPipeline(envMapCmdPools[cmd], GPUContext::instance().getGraphicsPipeline(static_cast<PrimitiveType>(i), envMapShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getHandle()), VK_PIPELINE_BIND_POINT_GRAPHICS,GPUContext::instance().getGraphicsPipeline(static_cast<PrimitiveType>(i), envMapShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getHandle());
                            //std::cout<<"pipeline bound"<<std::endl;
                            vkCmdBindDescriptorSets(envMapCmdPools[cmd].getHandle(envMap.getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS, linkedListPipeline[i][RenderTarget::NODEPTHNOSTENCIL*blendMode.nbBlendModes+blendMode.id].getLayout(), 0, sets.size(), sets.data(), offsetEnViewMatrices.data(), offsetEnViewMatrices.size());
                            vkCmdPushConstants(envMapCmdPools[cmd].getHandle(envMap.getCurrentFrame()), GPUContext::instance().getGraphicsPipeline(static_cast<PrimitiveType>(i), envMapShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getHandle()), VK_PIPELINE_BIND_POINT_GRAPHICS,GPUContext::instance().getGraphicsPipeline(static_cast<PrimitiveType>(i), envMapShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getHandle(), sizeof(ReflRefrFragPC), &reflRefrFragPC);
                            parentRenderer.draw(envMapCmdPools[cmd], static_cast<PrimitiveType>(i), states);
                        }
                        envMapCmdPools[cmd].endRecordCommandBuffer(envMap.getCurrentFrame());                        
                        jobFence[envMap.getCurrentFrame()].jobDone();
                    });                
                    threadPool.enqueue([this] { 
                        VkCommandBufferInheritanceInfo inheritanceInfo{};
                        inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                        inheritanceInfo.renderPass = VK_NULL_HANDLE;
                        inheritanceInfo.framebuffer = VK_NULL_HANDLE;
                        envMapQuadCmdPools[cmp].beginRecordCommandBuffer(parentRenderer.getCurrentFrame(), inheritanceInfo);
                        BlendMode blendMode;
                        RenderStates states;
                        states.shader = &quadLinkedListShader;
                        states.blendMode = blendMode;
                        unsigned int currentFrame = parentRenderer.getCurrentFrame();
                        std::vector<VkDescriptorSet> sets;
                        for (unsigned int i = 0; i < GPUContext::getDescriptorSets(envMapQuadShader).size(); i++) {
                            sets.push_back(GPUContext::getDescriptorSets(envMapQuadShader).getHandle());
                        }
                        blendMode.updateIds();
                        //std::cout<<"sizes = "<<quadLinkedListPipeline.size()<<","<<quadLinkedListPipeline[Triangles].size()<<",ids : "<<Triangles<<","<<RenderTarget::NODEPTHNOSTENCIL * blendMode.nbBlendModes+blendMode.id<<std::endl;
                        //std::cout<<"bind pipeline : "<<quadLinkedListPipeline[Triangles][RenderTarget::NODEPTHNOSTENCIL * blendMode.nbBlendModes+blendMode.id].getHandle()<<std::endl;
                        vkCmdBindPipeline(envMapQuadCmdPools[cmp].getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS,quadLinkedListPipeline[Triangles][RenderTarget::NODEPTHNOSTENCIL * blendMode.nbBlendModes+blendMode.id].getHandle());
                        //std::cout<<"pipeline bound"<<std::endl;
                        vkCmdBindDescriptorSets(envMapQuadCmdPools[cmp].getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS, GPUContext::instance().getGraphicsPipeline(static_cast<PrimitiveType>(i), envMapQuadShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getLayout(), 0, sets.size(), sets.data(), 0, nullptr);
                        vkCmdPushConstants(envMapQuadCmdPools[cmp].getHandle(parentRenderer.getCurrentFrame()), GPUContext::instance().getGraphicsPipeline(static_cast<PrimitiveType>(i), envMapQuadShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(unsigned int), &currentFrame);
                        envMap.draw(envMapQuadCmdPools[cmp], fullScreenQuad, states);
                        envMapQuadCmdPools[cmp].endRecordCommandBuffer(parentRenderer.getCurrentFrame());                        
                        jobFence[parentRenderer.getCurrentFrame()].jobDone();
                    });                
                    threadPool.enqueue([this] {
                        VkCommandBufferInheritanceRenderingInfo inheritanceRenderingInfo{};
                        inheritanceRenderingInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO;
                        inheritanceRenderingInfo.colorAttachmentCount = 1;
                        inheritanceRenderingInfo.pColorAttachmentFormats = &parentRenederer.getImageFormat(),
                        inheritanceRenderingInfo.depthAttachmentFormat = parentRenderer.getDepthTexture().getFormat();                    
                        inheritanceRenderingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;                      
                        VkCommandBufferInheritanceInfo inheritanceInfo{};
                        inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                        inheritanceInfo.pNext = &inheritanceRenderingInfo;
                        inheritanceInfo.renderPass = VK_NULL_HANDLE;
                        inheritanceInfo.framebuffer = VK_NULL_HANDLE;
                        reflRefrCmdPool.beginRecordCommandBuffer(envMap.getCurrentFrame(), inheritanceInfo);
                        BlendMode blendMode;
                        RenderStates states;
                        states.shader = &reflRefrShader;
                        states.blendMode = blendMode;
                        reflRefrVertPC.maxNodes = maxNodes;
                        reflRefFragPC.currentImageIndex = parentRenderer.getImageIndex();
                        std::vector<VkDescriptorSet> sets;
                        for (unsigned int i = 0; i <  GPUContext::instance().getDescriptorSets(reflRefrShader).size(); i++) {
                            //std::cout<<"set : "<<linkedListSets[i][0].getHandle()<<std::endl;
                            sets.push_back(GPUContext::instance().getDescriptorSets(reflRefrShader)[0].getHandle());
                        }
                        blendMode.updateIds();
                        for (unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {
                            //std::cout<<"sizes = "<<linkedListPipeline.size()<<","<<linkedListPipeline[i].size()<<",ids : "<<i<<","<<RenderTarget::NODEPTHNOSTENCIL * blendMode.nbBlendModes+blendMode.id<<std::endl;
                            //std::cout<<"bind pipeline : "<<linkedListPipeline[i][RenderTarget::NODEPTHNOSTENCIL * blendMode.nbBlendModes+blendMode.id].getHandle()<<std::endl;
                            vkCmdBindPipeline(GPUContext::instance().getGraphicsPipeline(static_cast<PrimitiveType>(i), reflRefrShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getHandle()), VK_PIPELINE_BIND_POINT_GRAPHICS,GPUContext::instance().getGraphicsPipeline(static_cast<PrimitiveType>(i), envMapShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getHandle());
                            //std::cout<<"pipeline bound"<<std::endl;
                            vkCmdBindDescriptorSets(reflRefrCmdPool.getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS, GPUContext::instance().getGraphicsPipeline(static_cast<PrimitiveType>(i), envMapShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getHandle()), VK_PIPELINE_BIND_POINT_GRAPHICS,GPUContext::instance().getGraphicsPipeline(static_cast<PrimitiveType>(i), envMapShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getLayout(), 0, sets.size(), sets.data(), 0, nullptr);
                            vkCmdPushConstants(reflRefrCmdPool.getHandle(parentRenderer.getCurrentFrame()), GPUContext::instance().getGraphicsPipeline(static_cast<PrimitiveType>(i), reflRefrShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getHandle()), VK_PIPELINE_BIND_POINT_GRAPHICS,GPUContext::instance().getGraphicsPipeline(static_cast<PrimitiveType>(i), envMapShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getHandle(), sizeof(ReflRefrFragPC), &reflRefrFragPC);
                            for (unsigned int s = 0; s < reflRefrGameObjects[cmp].getSubMeshes().size(); s++) {
                                parentRenderer.draw(reflRefrCmdPools[i], reflRefrGameObjects[cmp].getSubMeshes()[s].getVertexBuffer(), states);
                            }
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
                vkCmdExecuteCommands(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), 1, &reflRefrCommandPools[e].getHandle(parentRenderer.getCurrentFrame()));
                parentRenderer.endRendering();
            }           
        }
    }
}