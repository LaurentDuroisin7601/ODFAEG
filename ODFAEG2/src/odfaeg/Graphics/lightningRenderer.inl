namespace odfaeg {
    namespace graphic {
        LightningRenderer::LightningRenderer(RenderTarget& parentRenderer, unsigned int layer, std::string typesToRenderExpression, int windowId = -1, bool usethread) :
        parentRenderer(parentRenderer),
        pbrShader(GPUContext::instance().getDevice()),
        irradianceShader(GPUContext::instance().getDevice()),
        prefilterShader(GPUContext::instance().getDevice()),
        brdfShader(GPUContext::instance().getDevice()),
        backGroundShader(GPUContext::instance().getDevice()),
        environmentMap(GPUContext::instance().getDevice()),
        irradianceTexture(GPUContext::instance().getDevice()),
        prefilterTexture(GPUContext::instance().getDevice()),
        brdfLUT(GPUContext::instance().getDevice()),
        staggingLightBuffer(GPUContext::instance().getDevice()),
        ndcCubeVB(GPUContext::instance().getDevice(), entity::Triangles),
        fullscreenQuad(GPUContext::instance().getDevice(), entity::Triangles),
        threadPool(6),
        typesToRenderExpression(typesToRenderExpression)
        {
            Camera camera = parentRenderer.getCamera();
            camera.setPerspective(90.0f, , 1.0f, 0.1f, 10.0f);
            camera.setViewport(physic::BoundingBox(0, 0, 0.1f, 32, 32, 10.f));            
            environmentMap.createCubeMap(1024);
            irradianceMap.createCubeMap(32);
            irradianceMap.setCamera(camera);
            camera.setViewport(physic::BoundingBox(0, 0, 0.1f, 128, 128, 10.f));
            prefilterTexture.createCubeMap(128, 5);
            prefilterTexture.generateMipMaps();
            prefilter.setCamera(camera);
            camera.setViewport(physic::BoundingBox(0, 0, 0.1f, 512, 512, 10.f));
            brdfLUT.create(512, 512);
            viewsUBO.emplace_back(GPUContext::getDevice());
            viewsUBO.back().create(6*sizeof(math::Matrix4f), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
            camera.setCenter(math::Vec3f(0, 0, 0));
            std::array<math::vec3f, 6> views;
            camera.lookAt(1, 0, 0, math::Vec3f(0, -1, 0));
            views[0] = camera.getViewMatrix().getMatrix().transpose();
            camera.lookAt(-1, 0, 0, math::Vec3f(0, -1, 0));
            views[1] = camera.getViewMatrix().getMatrix().transpose();
            camera.lookAt(0, 1, 0, math::Vec3f(0, -1, 0));
            views[2] = camera.getViewMatrix().getMatrix().transpose();
            camera.lookAt(0, -1, 0, math::Vec3f(0, -1, 0));
            views[3] = camera.getViewMatrix().getMatrix().transpose();
            camera.lookAt(0, 0, -1, math::Vec3f(0, 0, 1));
            views[4] = camera.getViewMatrix().getMatrix().transpose();
            camera.lookAt(0, 0, 1, math::Vec3f(0, 0, -1));
            views[5] = camera.getViewMatrix().getMatrix().transpose();
            viewsUBO.update(views.data(), views.size()*sizeof(math::Matrix4f));
            std::string shaderDir = std::string(ODFAEG_INSTALL_DIR) + "/Shader";
            if (!pbrShader.loadFromFile(shaderDir+"/pbr.vert", shaderDir+"/pbr.frag")) {
                throw std::runtime_error("Failed to load pbr shader!");
            }
            if (!irrandianceShader.loadFromFile(shaderDir+"/cubemap.vert", shaderDir+"/irrandiance_convulsion.frag")) {
                throw std::runtime_error("Failed to load irrandiance shader!");
            }
            if (!prefilterShader.loadFromFile(shaderDir+"/cubemap.vert", shaderDire+"/prefilter.frag")) {
                throw std::runtime_error("Failed to load prefilter shader");
            }
            if (!brdfShader.loadFromFile(shaderDir+"/brdf.vert", shaderDir+"/brdf.frag")) {
                throw std::runtime_error("Failed to load brdf shader");
            }
            entity::Cube cube(Vec3f(-1, -1, -1), 2, 2, 2);
            
            for (unsigned int i = 0; i < cube.getSubMeshesCount(); i++) {
                for (unsigned int v = 0; v < cube.getSubMeshes()[i].getVertexArray().getVertexCount(); v++) {
                    ndcCubeVB.append(cube.getSubMeshes()[i].getVertexArray()[v]);
                }
                for (unsigned int v = 0; v < cube.getSubMeshes()[i].getVertexArray().getIndexCount(); v++) {
                    ndcCubeVB.addIndex(cube.getSubMeshes()[i].getVertexArray().getIndex(v));
                }
            }
            ndcCubeVB.update();
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
            createCommandPool();
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                lightsBuffer.emplace_back(GPUContext::instance().getDevice());
                lightsBuffer.back().create(sizeof(Light), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
            }
            setEnvironmentMap(environmentMap);
        }
        void LightningRenderer::addLight(Light light) {
            lights.push_back(light);
            needToUpdateBuffers = true;
        }
        void LightningRenderer::setEnvironmentMap(Texture& envMap) {
            environmentMap = envMap;
            updateDescriptorSet(0);
            irradianceTexture.clear();
            vkCmdBindPipeline(irradianceTexture.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS,GPUContext::instance().getGraphicsPipeline(entity::Triangles, irradianceShader, blendMode, RenderTarget::NODEPTHNOSTENCIL).getHandle());
            vkCmdBindDescriptorSets(irradianceTexture.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS, GPUContext::instance().getGraphicsPipeline(entity::Triangles, irradianceShader, blendMode, RenderTarget::NODEPTHNOSTENCIL).getLayout(), 0, sets.size(), sets.data(), 0, nullptr);
            vkCmdPushConstants(irradianceTexture.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), GPUContext::instance().getGraphicsPipeline(entity::Triangles, irradianceShader, blendMode, RenderTarget::NODEPTHNOSTENCIL).getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(math::Matrix4f), &camera.getProMatrix().getMatrix().transpose());
            RenderStates states;
            states.shader = &irradianceShader;    
            irradianceTexture.beginRendering();          
            irradianceTexture.draw(irradianceTexture.getCommandPool(), ndcCubeVB, states);
            irradianceTexture.endRendering();
            irradianceTexture.submit(true);                    
            prefilterTexture.clear(); 
            states.shader = &prefilterShader;        
            unsigned int maxMipLevels = 5;
            for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
            {
                updateDescriptorSet(mip+1);
                sets.clear();
                for (unsigned int i = 0; i < GPUContext::instance().getDescriptorSets(linkedListShader).size(); i++) {
                            //std::cout<<"set : "<<linkedListSets[i][0].getHandle()<<std::endl;
                    sets.push_back(GPUContext::instance().getDescriptorSets(irrandianceShader)[i][0].getHandle());
                }
                vkCmdBindPipeline(prefilterTexture.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS,GPUContext::instance().getGraphicsPipeline(entity::Triangles, prefilterShader, blendMode, RenderTarget::NODEPTHNOSTENCIL).getHandle());
                vkCmdBindDescriptorSets(prefilterTexture.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS, GPUContext::instance().getGraphicsPipeline(entity::Triangles, prefilterShader, blendMode, RenderTarget::NODEPTHNOSTENCIL).getLayout(), 0, sets.size(), sets.data(), 0, nullptr);
                VkRenderingInfo renderingInfo = {};
                VkRenderingAttachmentInfo depthAttachmentInfo = {
                    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .imageView = prefilterTexture.getDepthStencilTexture().getViews()[mip].getHandle(),
                    .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .clearValue = {.depthStencil{1.f, 0}}
                };
                renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
                renderingInfo.renderArea = {
                    .offset { .x=0, .y=0 },
                    .extent = getExtents()
                };
                renderingInfo.pDepthAttachment = &depthAttachmentInfo;
                renderingInfo.layerCount = getDepthStencilTexture().getLayerCount();
                VkRenderingAttachmentInfo colorAttachmentInfo;
                colorAttachmentInfo = {
                    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .imageView = m_textures[0].getImage().getViews()[mip].getHandle(),
                    .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .clearValue = {.color = {0.0f, 0.0f, 0.0f, 1.0f}}
                };
                renderingInfo.colorAttachmentCount = 1;
                renderingInfo.pColorAttachments = &colorAttachmentInfo;
                renderingInfo.viewMask = 6; 
                vkCmdBeginRendering(prefilterTexture.getCommandPool().getHandle(parentRenderer.getCurrentFrame()),&renderingInfo);
                prefilterTexture.draw(prefilterTexture.getCommandPool(), ndcCubeVB, states);
                vkCmdEndRendering(prefilterTexture.getCommandPool().getHandle(parentRenderer.getCurrentFrame())); 
            }
            prefilterTexture.submit(true);
            states.shader = &brdfShader;
            updateDescriptorSet(0);
            brdfLUT.clear();  
            sets.clear();
            for (unsigned int i = 0; i < GPUContext::instance().getDescriptorSets(brdfShader).size(); i++) {
                        //std::cout<<"set : "<<linkedListSets[i][0].getHandle()<<std::endl;
                sets.push_back(GPUContext::instance().getDescriptorSets(brdfShader)[i][0].getHandle());
            }
            vkCmdBindPipeline(brdfLUT.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS,GPUContext::instance().getGraphicsPipeline(entity::Triangles, brdfShader, blendMode, RenderTarget::NODEPTHNOSTENCIL).getHandle());
            vkCmdBindDescriptorSets(brdfLUT.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS, GPUContext::instance().getGraphicsPipeline(entity::Triangles, brdfShader, blendMode, RenderTarget::NODEPTHNOSTENCIL).getLayout(), 0, sets.size(), sets.data(), 0, nullptr); 
            brdfLUT.beginRendering();
            brdfLUT.draw(brdfLUT.getCommandPool(), fullscreenQuad, states)
        }
        void LightningRenderer::updateBuffers() {
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(GPUContext::instance().getDevice().getPhysicalDevice(), &props); 
            uint32_t minAlign = props.limits.minStorageBufferOffsetAlignment;  
            uint32_t lightAlignSize = (sizeof(Light) + minAlign - 1) & ~(minAlign - 1);   
            lightStaggingBuffer.create(sizeof(Light) * lights.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
            for (unsigned int l = 0; l < lights.size(); l++) {
                lightStaggingBuffer.update(lights.data(), sizeof(Light), l * lightAlignSize);
            } 
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                lightsBuffer[i].create(sizeof(Light) * lights.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
                commandPool.beginRecordCommandBuffer(i);
                Buffer::copyBuffer(lightStaggingBuffer, lightBuffer[i], sizeof(Light)*lights.size(), commandPool.getHandle(i));
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
        void LightningRenderer::createDescriptorsAndPipelines() {
            DescriptorSetLayout& irradianceSetLayout = GPUContext::instance().getDescriptorSetLayout(irrandianceShader, 2);
            irradianceSetLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
            irradianceSetLayout.updateLayout(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
            irradianceSetLayout.update();
            std::vector<VkPushConstantRange> pushConstants;
            VkPushConstantRange vsPushConstant;
            vsPushConstant.offset = 0;
            vsPushConstant.size = sizeof(math::Matrix4f);
            vsPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            pushConstants.push_back(vsPushConstant);
            VkPushConstantRange fsPushConstant;            
            fsPushConstant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            pushConstants.push_back(vsPushConstant);
            BlendMode blendMode;
            VkPipelineRenderingCreateInfo renderingCreateInfo = {};
            renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
            renderingCreateInfo.colorAttachmentCount = 1;
            renderingCreateInfo.pColorAttachmentFormats = &parentRenderer.getImageFormat();
            renderingCreateInfo.depthAttachmentFormat = parentRenderer.getDepthStencilTexture().getFormat();
            GPUContext::instance().getGraphicsPipeline(entity::Triangles, irradianceShader, blendMode,0).createGraphicPipeline(irradianceShader, entity::Triangles, GPUContext::instance().getDescriptorSetLayout(irrandianceShader), renderingCreateInfo,parentRenderer.getDepthStencilInfos()[RenderTarget::NODEPTHNOSTENCIL], blendMode, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, pushConstants);
            
            DescriptorSetLayout& prefilterSetLayout = GPUContext::instance().getDescriptorSetLayout(prefilterShader, 2);
            prefilterSetLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
            prefilterSetLayout.updateLayout(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
            prefilterSetLayout.update();
            pushConstants.clear();
            vsPushConstant.offset = 0;
            vsPushConstant.size = sizeof(math::Matrix4f);            
            pushConstants.push_back(vsPushConstant);
            VkPushConstantRange fsPushConstant;    
            fsPushConstant.offset = sizeof(math::Matrix4f);
            fsPushConstant.size = sizeof(float);
            pushConstants.push_back(fsPushConstant);
            
            VkPipelineRenderingCreateInfo renderingCreateInfo = {};
            renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
            renderingCreateInfo.colorAttachmentCount = 1;
            renderingCreateInfo.pColorAttachmentFormats = &parentRenderer.getImageFormat();
            renderingCreateInfo.depthAttachmentFormat = parentRenderer.getDepthStencilTexture().getFormat();
            GPUContext::instance().getGraphicsPipeline(entity::Triangles, prefilterShader, blendMode,0).createGraphicPipeline(prefilterShader, entity::Triangles, GPUContext::instance().getDescriptorSetLayout(irrandianceShader), renderingCreateInfo,parentRenderer.getDepthStencilInfos()[RenderTarget::NODEPTHNOSTENCIL], blendMode, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, pushConstants);

            
            VkPipelineRenderingCreateInfo renderingCreateInfo = {};
            renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
            renderingCreateInfo.colorAttachmentCount = 1;
            renderingCreateInfo.pColorAttachmentFormats = &parentRenderer.getImageFormat();
            renderingCreateInfo.depthAttachmentFormat = parentRenderer.getDepthStencilTexture().getFormat();
            GPUContext::instance().getGraphicsPipeline(entity::Triangles, brdfShader, blendMode,0).createGraphicPipeline(brdfShader, entity::Triangles, GPUContext::instance().getDescriptorSetLayout(irrandianceShader), renderingCreateInfo,parentRenderer.getDepthStencilInfos()[RenderTarget::NODEPTHNOSTENCIL], blendMode, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL);
            DescriptorSetLayout& pbrDescriptorLayout = GPUContext::instance().getDescriptorSetLayout(pbrShader, 7, true);
            pbrDescriptorLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT*NB_PRIMITIVE_TYPES, VK_SHADER_STAGE_VERTEX_BIT);
            pbrDescriptorLayout.updateLayout(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT*NB_PRIMITIVE_TYPES, VK_SHADER_STAGE_FRAGMENT_BIT); 
            pbrDescriptorLayout.updateLayout(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT*NB_PRIMITIVE_TYPES, VK_SHADER_STAGE_FRAGMENT_BIT);  
            pbrDescriptorLayout.updateLayout(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);                     
            pbrDescriptorLayout.updateLayout(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
            pbrDescriptorLayout.updateLayout(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
            pbrDescriptorLayout.updateLayout(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
            pbrDescriptorSetLayout.update();

            DescriptorSetLayout& pbrDescriptorLayout2 = GPUContext::instance().getDescriptorSetLayout(pbrShader, 1, true, 1);
            pbrDescriptorLayout2.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
            pbrDescriptorLayout2.update();
            DescriptorSetLayout& pbrDescriptorLayout3 = GPUContext::instance().getDescriptorSetLayout(pbrShader, 1, true, 2);
            pbrDescriptorLayout3.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
            pbrDescriptorLayout3.update();
            DescriptorSetLayout& pbrDescriptorLayout4 = GPUContext::instance().getDescriptorSetLayout(pbrShader, 1, true, 3);
            pbrDescriptorLayout4.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
            pbrDescriptorLayout4.update();    
            DescriptorSetLayout& pbrDescriptorLayout5 = GPUContext::instance().getDescriptorSetLayout(pbrShader, 1, true, 4);
            pbrDescriptorLayout5.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES, VK_SHADER_STAGE_FRAGMENT_BIT, , VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
            pbrDescriptorLayout5.update();
            DescriptorSetLayout& pbrDescriptorLayout6 = GPUContext::instance().getDescriptorSetLayout(pbrShader, 1, true, 5);
            pbrDescriptorLayout6.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
            pbrDescriptorLayout6.update();
            DescriptorSetLayout& pbrDescriptorLayout7 = GPUContext::instance().getDescriptorSetLayout(pbrShader, 1, true, 6);
            pbrDescriptorLayout7.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
            pbrDescriptorLayout7.update(); 
            vsPushConstant.offset = 0;
            vsPushConstant.size = sizeof(PbrVertPC);            
            pushConstants.push_back(vsPushConstant);
            VkPushConstantRange fsPushConstant;    
            fsPushConstant.offset = sizeof(pbrVertPC);
            fsPushConstant.size = sizeof(math::Vec4f);
            pushConstants.push_back(fsPushConstant);
            
            VkPipelineRenderingCreateInfo renderingCreateInfo = {};
            renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
            renderingCreateInfo.colorAttachmentCount = 1;
            renderingCreateInfo.pColorAttachmentFormats = &parentRenderer.getImageFormat();
            renderingCreateInfo.depthAttachmentFormat = parentRenderer.getDepthStencilTexture().getFormat();
            GPUContext::instance().getGraphicsPipeline(entity::Triangles, pbrShader, blendMode,0).createGraphicPipeline(pbrShader, entity::Triangles, GPUContext::instance().getDescriptorSetLayout(pbrShader), renderingCreateInfo,parentRenderer.getDepthStencilInfos()[RenderTarget::NODEPTHNOSTENCIL], blendMode, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, pushConstants);           
            
            DescriptorPool& irradiancePool = GPUContext::instance().getDescriptorPool(irrandianceShader, 2);
            irrandiancePool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
            irrandiancePool.updatePoolSize(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
            irrandiancePool.update();
            DescriptorPool& prefilterPool = GPUContext::instance().getDescriptorPool(prefilterShader, 2);            
            prefilterPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
            prefilterPool.updatePoolSize(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
            prefilterPool.update();

            DescriptorPool& pbrPool = GPUContext::instance().getDescriptorPool(pbrShader, 7);
            pbrPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT*NB_PRIMITIVE_TYPES);
            pbrPool.updatePoolSize(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT*NB_PRIMITIVE_TYPES); 
            pbrPool.updatePoolSize(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT*NB_PRIMITIVE_TYPES);  
            pbrPool.updatePoolSize(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);                     
            pbrPool.updatePoolSize(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
            pbrPool.updatePoolSize(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
            pbrPool.updatePoolsize(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
            pbrPool.update();

            DescriptorPool& pbrPool2 = GPUContext::instance().getDescriptorPool(pbrShader, 1, 1);
            pbrPool2.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
            pbrPool2.update();
            DescriptorPool& pbrPool3 = GPUContext::instance().getDescriptorPool(pbrShader, 1, 2);
            pbrPool3.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
            pbrPool3.update();
            DescriptorPool& pbrPool4 = GPUContext::instance().getDescriptorPool(pbrShader, 1, 3);
            pbrPool4.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
            pbrPool4.update();
            DescriptorPool& pbrPool5 = GPUContext::instance().getDescriptorPool(pbrShader, 1, 4);
            pbrPool5.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
            pbrPool5.update();
            DescriptorPool& pbrPool6 = GPUContext::instance().getDescriptorPool(pbrShader, 1, 5);
            pbrPool6.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
            pbrPool6.update();
            DescriptorPool& pbrPool7 = GPUContext::instance().getDescriptorPool(pbrShader, 1, 6);
            pbrPool7.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
            pbrPool7.update();

            DescriptorSet::allocate(irrandiancePool, irradianceLayout, GPUContext::instance().getDescriptorSets(irrandianceShader, 2, 1));
            DescriptorSet::allocate(prefilterPool, prefilterLayout, GPUContext::instance().getDescriptorSets(prefilterShader, 2, 1));
            DescriptorSet::allocate(pbrPool, pbrLayout, GPUContext::instance().getDescriptorSets(pbrShader, 7, 1), MAX_TEXTURES);
            DescriptorSet::allocate(pbrPool2, pbrLayout2, GPUContext::instance().getDescriptorSets(pbrShader, 1, 1, 1), MAX_TEXTURES);
            DescriptorSet::allocate(pbrPool3, pbrLayout3, GPUContext::instance().getDescriptorSets(irrandianceShader, 1, 1, 2), MAX_TEXTURES);
            DescriptorSet::allocate(pbrPool4, pbrLayout4, GPUContext::instance().getDescriptorSets(irrandianceShader, 1, 1, 3), MAX_TEXTURES);
            DescriptorSet::allocate(pbrPool5, pbrLayout5, GPUContext::instance().getDescriptorSets(irrandianceShader, 1, 1, 4), MAX_TEXTURES);
            DescriptorSet::allocate(pbrPool6, pbrLayout6, GPUContext::instance().getDescriptorSets(irrandianceShader, 1, 1, 5), MAX_TEXTURES);
            DescriptorSet::allocate(pbrPool7, pbrLayout7, GPUContext::instance().getDescriptorSets(irrandianceShader, 1, 1, 6), MAX_TEXTURES);

        }
        void LightningRenderer::createCommandPool() {
            commandPool.create(queueFamilyIndices.graphicsFamily.value());
            commandPool.createCommandBuffers(true, MAX_FRAMES_IN_FLIGHT);
            commandPool.beginRecordCommandBuffer(0);
            pbrCommandPool.create(queueFamilyIndices.graphicsFamily.value());
            pbrCommandPool.createCommandBuffers(false, MAX_FRAMES_IN_FLIGHT);
        }
        void LightningRenderer::updateDescriptorSet(unsigned int imageViewIndex) {
           DescriptorSet& irradianceDescriptorSet = GPUContext::instance(irradianceShader, 2, 1)[0];
           irradianceDescriptorSet.updateBufferInfos(0, viewsUBO, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
           irradianceDescriptorSet.updateImageInfos(1, environmentMap.getTexture(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
           irradianceDescriptorSet.updateDescriptorSet();
           DescriptorSet& prefilterDescriptorSet = GPUContext::instance(prefilterShader, 2, 1)[0];
           prefilterDescriptorSet.updateBufferInfos(0, viewsUBO, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
           if (imageViewIndex > 0) {
               prefilterDescriptorSet.updateImageInfos(1, environmentMap.getTexture(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageViewIndex-1);
           } else {
               prefilterDescriptorSet.updateImageInfos(1, environmentMap.getTexture(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
           }
           prefilterDescriptorSet.updateDescriptorSet(); 
           DescriptorSet& pbrDescriptorSet =  GPUContext::instance(pbrShader, 2, 1)[0];
           bool hasDiffuseTexture = GPUContext::instance().getSharedTextures(entity::SubMesh::DIFFUSE).size() != 0;
           pbrDescriptorSet.updateBufferInfos(0,  GPUContext::instance().getSharedBuffers(RenderTarget::OUTPUT_MODELS+parentRenderer.getId()*RenderTarget::NB_BUFFERS), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
           pbrDescriptorSet.updateBufferInfos(1,  GPUContext::instance().getSharedBuffers(RenderTarget::OUTPUT_MATERIALS+parentRenderer.getId()*RenderTarget::NB_BUFFERS), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
           pbrDescriptorSet.updateImageInfos(2, lightsBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);
           pbrDescriptorSet.updateImageInfos(3, irradianceTexture.getTexture(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
           pbrDescriptorSet.updateImageInfos(4, prefilterTexture.getTexture(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
           pbrDescriptorSet.updateImageInfos(5, brdfLUTTexture.getTexture(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
           if (hasDiffuseTextures) {
					//std::cout<<"textures : "<<Texture::getAllTextures().size()<<std::endl;
				pbrDescriptorSet.updateImageInfos(6, GPUContext::instance().getSharedTextures(entity::SubMesh::DIFFUSE), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
           }
           pbrDescriptorSet.updateDescriptorSet();
           bool hasSpecularTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::SPECULAR).size() != 0;
           if (hasSpecularTextures) {
               DescriptorSet& specularDefaultRenderingSet = GPUContext::instance().getDescriptorSets(defaultRenderingShader, 1, 1, 1)[0];
               specularDefaultRenderingSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::SPECULAR), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
               specularDefaultRenderingSet.updateDescriptorSet();
           }
           bool hasNormalTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::NORMAL).size() != 0;
           if (hasNormalTextures) {
               DescriptorSet& normalDefaultRenderingSet = GPUContext::instance().getDescriptorSets(defaultRenderingShader,  1, 1, 2)[0];
               normalDefaultRenderingSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::NORMAL), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
               normalDefaultRenderingSet.updateDescriptorSet();
           }

           bool hasMetalnessTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::METALNESS).size() != 0;
           if (hasMetalnessTextures) {
               DescriptorSet& metalnessDefaultRenderingSet = GPUContext::instance().getDescriptorSets(defaultRenderingShader,  1, 1, 3)[0];
               metalnessDefaultRenderingSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::METALNESS), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
               metalnessDefaultRenderingSet.updateDescriptorSet();
           }
           bool hasRoughnessTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::ROUGHNESS).size() != 0;
           if (hasRoughnessTextures) {
               DescriptorSet& roughnessDefaultRenderingSet = GPUContext::instance().getDescriptorSets(defaultRenderingShader,  1, 1, 4)[0];
               roughnessDefaultRenderingSet .updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::ROUGHNESS), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
               roughnessDefaultRenderingSet .updateDescriptorSet();
           }
           bool hasAOTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::AO).size() != 0;
           if (hasAOTextures) {
               DescriptorSet& aoDefaultRenderingSet = GPUContext::instance().getDescriptorSets(defaultRenderingShader, 1, 1, 5)[0];
               aoDefaultRenderingSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::AO), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
               aoDefaultRenderingSet.updateDescriptorSet();
           }
           bool hasEmissiveTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::EMISSIVE).size() != 0;
           if (hasEmissiveTextures) {
               DescriptorSet& emissiveDefaultRenderingSet = GPUContext::instance().getDescriptorSets(defaultRenderingShader, 1, 1, 6)[0];
               emissiveDefaultRenderingSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::EMISSIVE), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
               emissiveDefaultRenderingSet.updateDescriptorSet();
           } 
        }
        void LightingRenderer::drawNextFrame() {
            
        }
    }
}