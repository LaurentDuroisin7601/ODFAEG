namespace odfaeg {
    namespace graphic {
        LightningRenderer::LightningRenderer(RenderTarget& parentRenderer, unsigned int layer, std::string typesToRenderExpression, int windowId, bool useThread) :
        parentRenderer(parentRenderer),
        pbrShader(GPUContext::instance().getDevice()),
        irradianceShader(GPUContext::instance().getDevice()),
        prefilterShader(GPUContext::instance().getDevice()),
        brdfShader(GPUContext::instance().getDevice()),
        backgroundShader(GPUContext::instance().getDevice()),
        environmentMap(GPUContext::instance().getDevice()),
        irradianceTexture(GPUContext::instance().getDevice()),
        prefilterTexture(GPUContext::instance().getDevice()),
        brdfLUT(GPUContext::instance().getDevice()),
        lightStaggingBuffer(GPUContext::instance().getDevice()),
        commandPool(GPUContext::instance().getDevice()),
        pbrCommandPool(GPUContext::instance().getDevice()),
        ndcCubeVB(GPUContext::instance().getDevice(), entity::Triangles),
        fullScreenQuad(GPUContext::instance().getDevice(), entity::Triangles),
        threadPool(6),
        typesToRenderExpression(typesToRenderExpression)
        {
            rendererReady.store(false); 
            Camera camera = parentRenderer.getCamera();
            camera.setPerspective(90.0f, 1.0f, 0.1f, 10.0f);
            camera.setViewport(physic::BoundingBox(0, 0, 0.1f, 32, 32, 10.f));            
            environmentMap.createCubeMap(1024);
            irradianceTexture.createCubeMap(32);
            irradianceTexture.setCamera(camera);
            camera.setViewport(physic::BoundingBox(0, 0, 0.1f, 128, 128, 10.f));
            prefilterTexture.createCubeMap(128, 5, 1, false, true);           
            prefilterTexture.setCamera(camera);
            camera.setViewport(physic::BoundingBox(0, 0, 0.1f, 512, 512, 10.f));
            brdfLUT.create(512, 512);
            viewsUBO.emplace_back(GPUContext::instance().getDevice());
            viewsUBO.back().create(6*sizeof(math::Matrix4f), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
            camera.setCenter(math::Vec3f(0, 0, 0));
            std::array<math::Matrix4f, 6> views;
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
            viewsUBO.back().update(views.data(), views.size()*sizeof(math::Matrix4f));
            std::string shaderDir = std::string(ODFAEG_INSTALL_DIR) + "/Shader";
            if (!pbrShader.loadFromFile(shaderDir+"/pbr.vert", shaderDir+"/pbr.frag")) {
                throw std::runtime_error("Failed to load pbr shader!");
            }
            if (!irradianceShader.loadFromFile(shaderDir+"/cubemap.vert", shaderDir+"/irrandiance_convulsion.frag")) {
                throw std::runtime_error("Failed to load irrandiance shader!");
            }
            if (!prefilterShader.loadFromFile(shaderDir+"/cubemap.vert", shaderDir+"/prefilter.frag")) {
                throw std::runtime_error("Failed to load prefilter shader");
            }
            if (!brdfShader.loadFromFile(shaderDir+"/brdf.vert", shaderDir+"/brdf.frag")) {
                throw std::runtime_error("Failed to load brdf shader");
            }
            entity::Cube cube(math::Vec3f(-1, -1, -1), 2, 2, 2);
            
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
            createCommandPools();
            window::Command rendererReadyCmd(core::FastDelegate<bool>(&LightningRenderer::isRendererReady, this), core::FastDelegate<void>(&LightningRenderer::drawNextFrame, this));
            listener.connect("RendererReady",rendererReadyCmd); 
            if (useThread) {
                //std::cout<<"lanch"<<std::endl;
                listener.launch();
            }  
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                lightsBuffer.emplace_back(GPUContext::instance().getDevice());
                lightsBuffer.back().create(sizeof(Light), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
            }            
            stop.store(false);
            rendererReady.store(true);    
        }
        void LightningRenderer::addLight(Light light) {
            lights.push_back(light);
            needToUpdateLightsBuffer = true;
        }
        void LightningRenderer::setEnvironmentMap(Texture& environmentMap) {
            this->environmentMap.copyFrom(environmentMap);
            updateDescriptorSets();
            std::vector<VkDescriptorSet> sets;
            for (unsigned int i = 0; i < GPUContext::instance().getDescriptorSets(irradianceShader).size(); i++) {
                            //std::cout<<"set : "<<linkedListSets[i][0].getHandle()<<std::endl;
                sets.push_back(GPUContext::instance().getDescriptorSets(irradianceShader)[i][0].getHandle());
            }
            irradianceTexture.clear();
            BlendMode blendMode;
            vkCmdBindPipeline(irradianceTexture.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS,GPUContext::instance().getGraphicsPipeline(entity::Triangles, irradianceShader, blendMode, RenderTarget::NODEPTHNOSTENCIL).getHandle());
            vkCmdBindDescriptorSets(irradianceTexture.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS, GPUContext::instance().getGraphicsPipeline(entity::Triangles, irradianceShader, blendMode, RenderTarget::NODEPTHNOSTENCIL).getLayout(), 0, sets.size(), sets.data(), 0, nullptr);
            math::Matrix4f projMatrix = irradianceTexture.getCamera().getProjMatrix().getMatrix().transpose();
            vkCmdPushConstants(irradianceTexture.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), GPUContext::instance().getGraphicsPipeline(entity::Triangles, irradianceShader, blendMode, RenderTarget::NODEPTHNOSTENCIL).getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(math::Matrix4f), &projMatrix);
            RenderStates states;
            states.shader = &irradianceShader;    
            irradianceTexture.beginRendering();          
            irradianceTexture.draw(irradianceTexture.getCommandPool(), ndcCubeVB, states);
            irradianceTexture.endRendering();
            irradianceTexture.submit(true);                    
            prefilterTexture.clear(); 
            states.shader = &prefilterShader;        
            unsigned int maxMipLevels = 5;
            sets.clear();
            for (unsigned int i = 0; i < GPUContext::instance().getDescriptorSets(prefilterShader).size(); i++) {
                            //std::cout<<"set : "<<linkedListSets[i][0].getHandle()<<std::endl;
                sets.push_back(GPUContext::instance().getDescriptorSets(irradianceShader)[i][0].getHandle());
            }
            for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
            {
               
                float roughness = (float)mip / (float)(maxMipLevels - 1);
                vkCmdBindPipeline(prefilterTexture.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS,GPUContext::instance().getGraphicsPipeline(entity::Triangles, prefilterShader, blendMode, RenderTarget::NODEPTHNOSTENCIL).getHandle());
                vkCmdBindDescriptorSets(prefilterTexture.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS, GPUContext::instance().getGraphicsPipeline(entity::Triangles, prefilterShader, blendMode, RenderTarget::NODEPTHNOSTENCIL).getLayout(), 0, sets.size(), sets.data(), 0, nullptr);
                projMatrix = prefilterTexture.getCamera().getProjMatrix().getMatrix().transpose();
                vkCmdPushConstants(prefilterTexture.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), GPUContext::instance().getGraphicsPipeline(entity::Triangles, prefilterShader, blendMode, RenderTarget::NODEPTHNOSTENCIL).getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(math::Matrix4f), &projMatrix);
                vkCmdPushConstants(prefilterTexture.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), GPUContext::instance().getGraphicsPipeline(entity::Triangles, prefilterShader, blendMode, RenderTarget::NODEPTHNOSTENCIL).getLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(math::Matrix4f), sizeof(float), &roughness);
                VkRenderingInfo renderingInfo = {};
                VkRenderingAttachmentInfo depthAttachmentInfo = {
                    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .imageView = prefilterTexture.getDepthStencilTexture().getDepthViews()[mip].getHandle(),
                    .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .clearValue = {.depthStencil{1.f, 0}}
                };
                renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
                renderingInfo.renderArea = {
                    .offset { .x=0, .y=0 },
                    .extent = prefilterTexture.getExtents()
                };
                renderingInfo.pDepthAttachment = &depthAttachmentInfo;
                renderingInfo.layerCount = prefilterTexture.getDepthStencilTexture().getLayerCount();
                VkRenderingAttachmentInfo colorAttachmentInfo;
                colorAttachmentInfo = {
                    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .imageView = prefilterTexture.getTexture().getImageViews()[mip].getHandle(),
                    .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .clearValue = {.color = {0.0f, 0.0f, 0.0f, 1.0f}}
                };
                renderingInfo.colorAttachmentCount = 1;
                renderingInfo.pColorAttachments = &colorAttachmentInfo;
                renderingInfo.viewMask = prefilterTexture.getViewMask(); 
                vkCmdBeginRendering(prefilterTexture.getCommandPool().getHandle(parentRenderer.getCurrentFrame()),&renderingInfo);
                prefilterTexture.draw(prefilterTexture.getCommandPool(), ndcCubeVB, states);
                vkCmdEndRendering(prefilterTexture.getCommandPool().getHandle(parentRenderer.getCurrentFrame())); 
            }
            prefilterTexture.submit(true);
            states.shader = &brdfShader;           
            brdfLUT.clear();  
            sets.clear();
            for (unsigned int i = 0; i < GPUContext::instance().getDescriptorSets(brdfShader).size(); i++) {
                        //std::cout<<"set : "<<linkedListSets[i][0].getHandle()<<std::endl;
                sets.push_back(GPUContext::instance().getDescriptorSets(brdfShader)[i][0].getHandle());
            }
            vkCmdBindPipeline(brdfLUT.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS,GPUContext::instance().getGraphicsPipeline(entity::Triangles, brdfShader, blendMode, RenderTarget::NODEPTHNOSTENCIL).getHandle());
            vkCmdBindDescriptorSets(brdfLUT.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS, GPUContext::instance().getGraphicsPipeline(entity::Triangles, brdfShader, blendMode, RenderTarget::NODEPTHNOSTENCIL).getLayout(), 0, sets.size(), sets.data(), 0, nullptr); 
            brdfLUT.beginRendering();
            brdfLUT.draw(brdfLUT.getCommandPool(), fullScreenQuad, states);
            brdfLUT.submit(true);
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
                Buffer::copyBuffer(lightStaggingBuffer, lightsBuffer[i], sizeof(Light)*lights.size(), commandPool.getHandle(i));
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
            DescriptorSetLayout& irradianceSetLayout = GPUContext::instance().getDescriptorSetLayout(irradianceShader, 2);
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
            renderingCreateInfo.pColorAttachmentFormats = &irradianceTexture.getImageFormat();
            renderingCreateInfo.depthAttachmentFormat = irradianceTexture.getDepthStencilTexture().getFormat();
            GPUContext::instance().getGraphicsPipeline(entity::Triangles, irradianceShader, blendMode,0).createGraphicPipeline(irradianceShader, entity::Triangles, GPUContext::instance().getDescriptorSetLayout(irradianceShader), renderingCreateInfo,parentRenderer.getDepthStencilInfos()[RenderTarget::NODEPTHNOSTENCIL], blendMode, VK_SAMPLE_COUNT_1_BIT, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, pushConstants);
            
            DescriptorSetLayout& prefilterSetLayout = GPUContext::instance().getDescriptorSetLayout(prefilterShader, 2);
            prefilterSetLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
            prefilterSetLayout.updateLayout(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
            prefilterSetLayout.update();
            pushConstants.clear();
            vsPushConstant.offset = 0;
            vsPushConstant.size = sizeof(math::Matrix4f);            
            pushConstants.push_back(vsPushConstant);              
            fsPushConstant.offset = sizeof(math::Matrix4f);
            fsPushConstant.size = sizeof(float);
            pushConstants.push_back(fsPushConstant);
            
            
            renderingCreateInfo.colorAttachmentCount = 1;
            renderingCreateInfo.pColorAttachmentFormats = &prefilterTexture.getImageFormat();
            renderingCreateInfo.depthAttachmentFormat = prefilterTexture.getDepthStencilTexture().getFormat();
            GPUContext::instance().getGraphicsPipeline(entity::Triangles, prefilterShader, blendMode,0).createGraphicPipeline(prefilterShader, entity::Triangles, GPUContext::instance().getDescriptorSetLayout(prefilterShader), renderingCreateInfo,parentRenderer.getDepthStencilInfos()[RenderTarget::NODEPTHNOSTENCIL], blendMode, VK_SAMPLE_COUNT_1_BIT, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, pushConstants);

            
            
            renderingCreateInfo.colorAttachmentCount = 1;
            renderingCreateInfo.pColorAttachmentFormats = &parentRenderer.getImageFormat();
            renderingCreateInfo.depthAttachmentFormat = parentRenderer.getDepthStencilTexture().getFormat();
            GPUContext::instance().getGraphicsPipeline(entity::Triangles, brdfShader, blendMode,0).createGraphicPipeline(brdfShader, entity::Triangles, GPUContext::instance().getDescriptorSetLayout(brdfShader), renderingCreateInfo,parentRenderer.getDepthStencilInfos()[RenderTarget::NODEPTHNOSTENCIL], blendMode, GPUContext::instance().getDevice().getMsaaSamples(), VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL);
            DescriptorSetLayout& pbrDescriptorLayout = GPUContext::instance().getDescriptorSetLayout(pbrShader, 7, true);
            pbrDescriptorLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT*NB_PRIMITIVE_TYPES, VK_SHADER_STAGE_VERTEX_BIT);
            pbrDescriptorLayout.updateLayout(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT*NB_PRIMITIVE_TYPES, VK_SHADER_STAGE_FRAGMENT_BIT); 
            pbrDescriptorLayout.updateLayout(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT*NB_PRIMITIVE_TYPES, VK_SHADER_STAGE_FRAGMENT_BIT);  
            pbrDescriptorLayout.updateLayout(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);                     
            pbrDescriptorLayout.updateLayout(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
            pbrDescriptorLayout.updateLayout(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
            pbrDescriptorLayout.updateLayout(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
            pbrDescriptorLayout.update();

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
            pbrDescriptorLayout5.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
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
            pushConstants.clear();
            vsPushConstant.offset = 0;
            vsPushConstant.size = sizeof(PbrVertPC);            
            pushConstants.push_back(vsPushConstant);            
            fsPushConstant.offset = sizeof(pbrVertPC);
            fsPushConstant.size = sizeof(math::Vec4f);
            pushConstants.push_back(fsPushConstant);
            
            
            renderingCreateInfo.colorAttachmentCount = 1;
            renderingCreateInfo.pColorAttachmentFormats = &parentRenderer.getImageFormat();
            renderingCreateInfo.depthAttachmentFormat = parentRenderer.getDepthStencilTexture().getFormat();
            for (unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {
                GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), pbrShader, blendMode,0).createGraphicPipeline(pbrShader, static_cast<entity::PrimitiveType>(i), GPUContext::instance().getDescriptorSetLayout(pbrShader), renderingCreateInfo,parentRenderer.getDepthStencilInfos()[RenderTarget::DEPTHNOSTENCIL], blendMode, GPUContext::instance().getDevice().getMsaaSamples(), VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, pushConstants); 
            }          
            
            DescriptorPool& irradiancePool = GPUContext::instance().getDescriptorPool(irradianceShader, 2);
            irradiancePool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
            irradiancePool.updatePoolSize(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
            irradiancePool.update();
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
            pbrPool.updatePoolSize(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
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

            DescriptorSet::allocate(irradiancePool, irradianceSetLayout, GPUContext::instance().getDescriptorSets(irradianceShader, 2, 1));
            DescriptorSet::allocate(prefilterPool, prefilterSetLayout, GPUContext::instance().getDescriptorSets(prefilterShader, 2, 1));
            DescriptorSet::allocate(pbrPool, pbrDescriptorLayout, GPUContext::instance().getDescriptorSets(pbrShader, 7, 1), MAX_TEXTURES);
            DescriptorSet::allocate(pbrPool2, pbrDescriptorLayout2, GPUContext::instance().getDescriptorSets(pbrShader, 1, 1, 1), MAX_TEXTURES);
            DescriptorSet::allocate(pbrPool3, pbrDescriptorLayout3, GPUContext::instance().getDescriptorSets(pbrShader, 1, 1, 2), MAX_TEXTURES);
            DescriptorSet::allocate(pbrPool4, pbrDescriptorLayout4, GPUContext::instance().getDescriptorSets(pbrShader, 1, 1, 3), MAX_TEXTURES);
            DescriptorSet::allocate(pbrPool5, pbrDescriptorLayout5, GPUContext::instance().getDescriptorSets(pbrShader, 1, 1, 4), MAX_TEXTURES);
            DescriptorSet::allocate(pbrPool6, pbrDescriptorLayout6, GPUContext::instance().getDescriptorSets(pbrShader, 1, 1, 5), MAX_TEXTURES);
            DescriptorSet::allocate(pbrPool7, pbrDescriptorLayout7, GPUContext::instance().getDescriptorSets(pbrShader, 1, 1, 6), MAX_TEXTURES);

        }
        void LightningRenderer::createCommandPools() {
            Device::QueueFamilyIndices queueFamilyIndices = GPUContext::instance().getDevice().findQueueFamilies(GPUContext::instance().getDevice().getPhysicalDevice());
            commandPool.create(queueFamilyIndices.graphicsFamily.value());
            commandPool.createCommandBuffers(true, MAX_FRAMES_IN_FLIGHT);
            commandPool.beginRecordCommandBuffer(0);
            pbrCommandPool.create(queueFamilyIndices.graphicsFamily.value());
            pbrCommandPool.createCommandBuffers(false, MAX_FRAMES_IN_FLIGHT);
        }
        void LightningRenderer::updateDescriptorSets() {
           DescriptorSet& irradianceDescriptorSet = GPUContext::instance().getDescriptorSets(irradianceShader, 2, 1)[0];
           irradianceDescriptorSet.updateBufferInfos(0, viewsUBO, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
           irradianceDescriptorSet.updateImageInfos(1, environmentMap, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
           irradianceDescriptorSet.updateDescriptorSet();
           DescriptorSet& prefilterDescriptorSet = GPUContext::instance().getDescriptorSets(prefilterShader, 2, 1)[0];
           prefilterDescriptorSet.updateBufferInfos(0, viewsUBO, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
           prefilterDescriptorSet.updateImageInfos(1, environmentMap, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
           prefilterDescriptorSet.updateDescriptorSet(); 
           DescriptorSet& pbrDescriptorSet =  GPUContext::instance().getDescriptorSets(pbrShader, 2, 1)[0];
           bool hasDiffuseTexture = GPUContext::instance().getSharedTextures(entity::SubMesh::DIFFUSE).size() != 0;
           pbrDescriptorSet.updateBufferInfos(0,  GPUContext::instance().getSharedBuffers(RenderTarget::OUTPUT_MODELS+parentRenderer.getId()*RenderTarget::NB_BUFFERS), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
           pbrDescriptorSet.updateBufferInfos(1,  GPUContext::instance().getSharedBuffers(RenderTarget::OUTPUT_MATERIALS+parentRenderer.getId()*RenderTarget::NB_BUFFERS), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
           pbrDescriptorSet.updateBufferInfos(2, lightsBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);
           pbrDescriptorSet.updateImageInfos(3, irradianceTexture.getTexture(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
           pbrDescriptorSet.updateImageInfos(4, prefilterTexture.getTexture(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
           pbrDescriptorSet.updateImageInfos(5, brdfLUT.getTexture(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
           if (hasDiffuseTexture) {
					//std::cout<<"textures : "<<Texture::getAllTextures().size()<<std::endl;
				pbrDescriptorSet.updateImageInfos(6, GPUContext::instance().getSharedTextures(entity::SubMesh::DIFFUSE), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
           }
           pbrDescriptorSet.updateDescriptorSet();
           bool hasSpecularTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::SPECULAR).size() != 0;
           if (hasSpecularTextures) {
               DescriptorSet& specularPbrSet = GPUContext::instance().getDescriptorSets(pbrShader, 1, 1, 1)[0];
               specularPbrSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::SPECULAR), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
               specularPbrSet.updateDescriptorSet();
           }
           bool hasNormalTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::NORMAL).size() != 0;
           if (hasNormalTextures) {
               DescriptorSet& normalPbrSet = GPUContext::instance().getDescriptorSets(pbrShader,  1, 1, 2)[0];
               normalPbrSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::NORMAL), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
               normalPbrSet.updateDescriptorSet();
           }

           bool hasMetalnessTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::METALNESS).size() != 0;
           if (hasMetalnessTextures) {
               DescriptorSet& metalnessPbrSet = GPUContext::instance().getDescriptorSets(pbrShader,  1, 1, 3)[0];
               metalnessPbrSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::METALNESS), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
               metalnessPbrSet.updateDescriptorSet();
           }
           bool hasRoughnessTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::ROUGHNESS).size() != 0;
           if (hasRoughnessTextures) {
               DescriptorSet& roughnessPbrSet = GPUContext::instance().getDescriptorSets(pbrShader,  1, 1, 4)[0];
               roughnessPbrSet .updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::ROUGHNESS), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
               roughnessPbrSet .updateDescriptorSet();
           }
           bool hasAOTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::AO).size() != 0;
           if (hasAOTextures) {
               DescriptorSet& aoPbrSet = GPUContext::instance().getDescriptorSets(pbrShader, 1, 1, 5)[0];
               aoPbrSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::AO), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
               aoPbrSet.updateDescriptorSet();
           }
           bool hasEmissiveTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::EMISSIVE).size() != 0;
           if (hasEmissiveTextures) {
               DescriptorSet& emissivePbrSet = GPUContext::instance().getDescriptorSets(pbrShader, 1, 1, 6)[0];
               emissivePbrSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::EMISSIVE), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
               emissivePbrSet.updateDescriptorSet();
           } 
        }
        void LightningRenderer::clear() {            
            registerFramesJob[parentRenderer.getCurrentFrame()].store(true);
            cv.notify_one();
        }
        void LightningRenderer::drawNextFrame() {
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
                if (needToUpdateLightsBuffer) {
                    updateBuffers();
                    needToUpdateLightsBuffer = false;
                    needToUpdateDescriptorSets = true;
                }
                if (needToUpdateDescriptorSets) {
                    //std::cout<<"update ds"<<std::endl;
                    updateDescriptorSets();
                    needToUpdateDescriptorSets = false;
                }                         
                jobFence[renderFrame].reset(1);                
                threadPool.enqueue([this, renderFrame] {
                    VkCommandBufferInheritanceRenderingInfo inheritanceRenderingInfo{};
                    inheritanceRenderingInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO;
                    inheritanceRenderingInfo.colorAttachmentCount = 1;
                    inheritanceRenderingInfo.pColorAttachmentFormats = &parentRenderer.getImageFormat(); // tableau de VkFormat
                    inheritanceRenderingInfo.depthAttachmentFormat = parentRenderer.getDepthStencilTexture().getFormat();    // VK_FORMAT_D32_SFLOAT, etc.                    
                    inheritanceRenderingInfo.rasterizationSamples = GPUContext::instance().getDevice().getMsaaSamples();
                    
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
                    states.shader = &pbrShader;
                    BlendMode blendMode;
                    std::vector<VkDescriptorSet> sets;
                    for (unsigned int i = 0; i < GPUContext::instance().getDescriptorSets(pbrShader).size(); i++) {
                        sets.push_back(GPUContext::instance().getDescriptorSets(pbrShader)[i][0].getHandle());
                    } 
                    VkPhysicalDeviceProperties props;
                    vkGetPhysicalDeviceProperties(GPUContext::instance().getDevice().getPhysicalDevice(), &props); 
                    uint32_t minAlign = props.limits.minStorageBufferOffsetAlignment;  
                    uint32_t lightAlignSize = (sizeof(Light) + minAlign - 1) & ~(minAlign - 1);                      
                    for (unsigned int l = 0; l < lights.size(); l++) {
                        std::vector<uint32_t> offsetLights;
                        for (unsigned int j = 0; j < MAX_FRAMES_IN_FLIGHT; j++) {
                           offsetLights.push_back(l * lightAlignSize);
                        } 
                        for (unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {
                            vkCmdBindPipeline(pbrCommandPool.getHandle(renderFrame), VK_PIPELINE_BIND_POINT_GRAPHICS, GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), pbrShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getHandle());
                            //std::cout<<"registered bind pipeline"<<std::endl;
                            pbrVertPC.projMatrix = parentRenderer.getCamera().getProjMatrix().getMatrix().transpose();
                            pbrVertPC.viewMatrix = parentRenderer.getCamera().getViewMatrix().getMatrix().transpose();
                            pbrVertPC.primitiveType = i;
                            pbrVertPC.currentFrame = parentRenderer.getCurrentFrame();
                            vkCmdPushConstants(pbrCommandPool.getHandle(renderFrame), GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), pbrShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PbrVertPC), &pbrVertPC);
                            math::Vec4f cameraPos = parentRenderer.getCamera().getCenter();
                            vkCmdPushConstants(pbrCommandPool.getHandle(renderFrame), GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), pbrShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PbrVertPC), sizeof(math::Vec4f), &cameraPos);
                            //std::cout<<"registed bind push constants"<<std::endl;
                            vkCmdBindDescriptorSets(pbrCommandPool.getHandle(renderFrame), VK_PIPELINE_BIND_POINT_GRAPHICS, GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), pbrShader, blendMode, RenderTarget::DEPTHNOSTENCIL).getLayout(), 0, sets.size(), sets.data(), offsetLights.size(), offsetLights.data());
                            //std::cout<<"registered bind decriptor sets"<<std::endl;
                            parentRenderer.draw(pbrCommandPool, static_cast<entity::PrimitiveType>(i), states);
                        } 
                    }
                    jobFence[renderFrame].jobDone();
                });
                jobFence[renderFrame].wait();
            }
            commandBuffersReady[renderFrame].store(true);
            cv.notify_all();
        }
        void LightningRenderer::draw() {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] {
                    //std::cout<<"draw frame : "<<frameBuffer.getCurrentFrame()<<std::endl;
                return commandBuffersReady[parentRenderer.getCurrentFrame()].load() || stop.load();
            });
            //std::cout<<"buffers ready!"<<std::endl;
            commandBuffersReady[parentRenderer.getCurrentFrame()].store(false);
            if (!stop.load()) {
                parentRenderer.applyComputeGraphicsBarrier();
                parentRenderer.beginRendering(true);
                vkCmdExecuteCommands(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), 1, &pbrCommandPool.getHandle(parentRenderer.getCurrentFrame()));
                parentRenderer.endRendering();
            }
        }
        bool LightningRenderer::isRendererReady() {
            return rendererReady.load();
        }
    }
}