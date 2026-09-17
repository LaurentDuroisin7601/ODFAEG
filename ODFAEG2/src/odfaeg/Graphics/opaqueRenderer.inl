namespace odfaeg {
    namespace graphic {       
        OpaqueRenderer::OpaqueRenderer(RenderTarget& parentRenderer, unsigned int layer, std::string typesToRenderExpression, int windowId, bool useThread) :
        parentRenderer(parentRenderer),
        hzShader(GPUContext::instance().getDevice()) {                
            needToUpdateDescriptorSets = true;           
            std::string shaderDir = std::string(ODFAEG_INSTALL_DIR) + "/Shader";
            if (!hzShader.loadFromFile(shaderDir + "/hz.vert", shaderDir + "hz.frag")) {
                throw std::runtime_error("Failed to load shadow pass csm shader");
            }   
            createDescriptorAndPipelines();
        } 
        void OpaqueRenderer::createDescriptorAndPipelines() {
            VkPipelineRenderingCreateInfo renderingCreateInfo = {};
            renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
            renderingCreateInfo.colorAttachmentCount = 1;
            renderingCreateInfo.pColorAttachmentFormats = &parentRenderer.getImageFormat();  
            renderingCreateInfo.depthAttachmentFormat = parentRenderer.getDepthStencilTexture().getFormat();
            DescriptorSetLayout& defaultRenderingLayout = GPUContext::instance().getDescriptorSetLayout(hzShader, 4, true);
            defaultRenderingLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES*MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_VERTEX_BIT);
            defaultRenderingLayout.updateLayout(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES*MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_VERTEX_BIT);
            defaultRenderingLayout.updateLayout(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES*MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_FRAGMENT_BIT);
            defaultRenderingLayout.updateLayout(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
            defaultRenderingLayout.update();
            DescriptorSetLayout& specularDefaultRenderingLayout = GPUContext::instance().getDescriptorSetLayout(hzShader, 1, true, 1);
            specularDefaultRenderingLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES/**MAX_FRAMES_IN_FLIGHT*/, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
            specularDefaultRenderingLayout.update();
            DescriptorSetLayout& normalDefaultRenderingLayout = GPUContext::instance().getDescriptorSetLayout(hzShader, 1, true, 2);
            normalDefaultRenderingLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES/**MAX_FRAMES_IN_FLIGHT*/, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
            normalDefaultRenderingLayout.update();
            DescriptorSetLayout& metalnessDefaultRenderingLayout = GPUContext::instance().getDescriptorSetLayout(hzShader, 1, true, 3);
            metalnessDefaultRenderingLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES/**MAX_FRAMES_IN_FLIGHT*/, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
            metalnessDefaultRenderingLayout.update();
            DescriptorSetLayout& roughnessDefaultRenderingLayout = GPUContext::instance().getDescriptorSetLayout(hzShader, 1, true, 4);
            roughnessDefaultRenderingLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES/**MAX_FRAMES_IN_FLIGHT*/, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
            roughnessDefaultRenderingLayout.update();
            DescriptorSetLayout& aoDefaultRenderingLayout = GPUContext::instance().getDescriptorSetLayout(hzShader, 1, true, 5);
            aoDefaultRenderingLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES/**MAX_FRAMES_IN_FLIGHT*/, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
            aoDefaultRenderingLayout.update();
            DescriptorSetLayout& emissiveDefaultRenderingLayout = GPUContext::instance().getDescriptorSetLayout(hzShader, 1, true, 6);
            emissiveDefaultRenderingLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES/**MAX_FRAMES_IN_FLIGHT*/, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
            emissiveDefaultRenderingLayout.update();
            
            
           
            BlendMode blendMode;
            std::vector<VkPushConstantRange> pushConstants;
            VkPushConstantRange vsPushConstant;
            vsPushConstant.offset = 0;
            vsPushConstant.size = sizeof(RenderTarget::ViewProjMatPC);
            vsPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            pushConstants.push_back(vsPushConstant);
            for (unsigned int p = 0; p < NB_PRIMITIVE_TYPES; p++) {
                GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(p), hzShader, blendMode, RenderTarget::DEPTHNOSTENCIL).createGraphicPipeline( hzShader, static_cast<entity::PrimitiveType>(p),GPUContext::instance().getDescriptorSetLayout(hzShader), renderingCreateInfo, parentRenderer.getDepthStencilInfos()[RenderTarget::DEPTHNOSTENCIL], blendMode, GPUContext::instance().getDevice().getMsaaSamples(), VK_CULL_MODE_NONE, VK_POLYGON_MODE_FILL, pushConstants);
                //std::cout<<"pipeline created in render target : "<<GPUContext::instance().getGraphicsPipeline(static_cast<PrimitiveType>(p), hzShader, blendMode,i).getHandle()<<std::endl;
            }            
            DescriptorPool& defaultRenderingPool = GPUContext::instance().getDescriptorPool(hzShader, 4);
            for (unsigned int i = 0; i < 3; i++) {
                defaultRenderingPool.updatePoolSize(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES * MAX_FRAMES_IN_FLIGHT);
            }
            defaultRenderingPool.updatePoolSize(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
            defaultRenderingPool.update();
            DescriptorPool& specularDefaultRenderingPool = GPUContext::instance().getDescriptorPool(hzShader, 1, 1);
            specularDefaultRenderingPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
            specularDefaultRenderingPool.update();
            DescriptorPool& normalDefaultRenderingPool = GPUContext::instance().getDescriptorPool(hzShader, 1, 2);
            normalDefaultRenderingPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
            normalDefaultRenderingPool.update();
            DescriptorPool& metalnessDefaultRenderingPool = GPUContext::instance().getDescriptorPool(hzShader, 1, 3);
            metalnessDefaultRenderingPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
            metalnessDefaultRenderingPool.update();
            DescriptorPool& roughnessDefaultRenderingPool = GPUContext::instance().getDescriptorPool(hzShader, 1, 4);
            roughnessDefaultRenderingPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
            roughnessDefaultRenderingPool.update();
            DescriptorPool& aoDefaultRenderingPool = GPUContext::instance().getDescriptorPool(hzShader, 1, 5);
            aoDefaultRenderingPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
            aoDefaultRenderingPool.update();
            DescriptorPool& emissiveDefaultRenderingPool = GPUContext::instance().getDescriptorPool(hzShader, 1, 6);
            emissiveDefaultRenderingPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
            emissiveDefaultRenderingPool.update();
                        
            
            DescriptorSet::allocate(defaultRenderingPool, defaultRenderingLayout, GPUContext::instance().getDescriptorSets(hzShader, 1, 1));
        }
        void OpaqueRenderer::updateDescriptorSets() {            
            bool hasDiffuseTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::DIFFUSE).size() != 0;
            DescriptorSet& defaultRenderingSet = GPUContext::instance().getDescriptorSets(hzShader, (hasDiffuseTextures) ? 4 : 3, 1)[0];
            defaultRenderingSet.updateBufferInfos(0, GPUContext::instance().getSharedBuffers(RenderTarget::OUTPUT_MODELS), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            defaultRenderingSet.updateBufferInfos(1, GPUContext::instance().getSharedBuffers(RenderTarget::OUTPUT_MESHES), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            defaultRenderingSet.updateBufferInfos(2, GPUContext::instance().getSharedBuffers(RenderTarget::OUTPUT_MATERIALS), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            if (hasDiffuseTextures ) {
                //std::cout<<"textures : "<<Texture::getAllTextures().size()<<std::endl;
                defaultRenderingSet.updateImageInfos(3, GPUContext::instance().getSharedTextures(entity::SubMesh::DIFFUSE), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            }
            defaultRenderingSet.updateDescriptorSet();
            bool hasSpecularTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::SPECULAR).size() != 0;
            if (hasSpecularTextures) {
                DescriptorSet& specularDefaultRenderingSet = GPUContext::instance().getDescriptorSets(hzShader, 1, 1, 1)[0];
                specularDefaultRenderingSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::SPECULAR), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
                specularDefaultRenderingSet.updateDescriptorSet();
            }
            bool hasNormalTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::NORMAL).size() != 0;
            if (hasNormalTextures) {
                DescriptorSet& normalDefaultRenderingSet = GPUContext::instance().getDescriptorSets(hzShader,  1, 1, 2)[0];
                normalDefaultRenderingSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::NORMAL), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
                normalDefaultRenderingSet.updateDescriptorSet();
            }

            bool hasMetalnessTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::METALNESS).size() != 0;
            if (hasMetalnessTextures) {
                DescriptorSet& metalnessDefaultRenderingSet = GPUContext::instance().getDescriptorSets(hzShader,  1, 1, 3)[0];
                metalnessDefaultRenderingSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::METALNESS), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
                metalnessDefaultRenderingSet.updateDescriptorSet();
            }
            bool hasRoughnessTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::ROUGHNESS).size() != 0;
            if (hasRoughnessTextures) {
                DescriptorSet& roughnessDefaultRenderingSet = GPUContext::instance().getDescriptorSets(hzShader,  1, 1, 4)[0];
                roughnessDefaultRenderingSet .updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::ROUGHNESS), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
                roughnessDefaultRenderingSet .updateDescriptorSet();
            }
            bool hasAOTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::AO).size() != 0;
            if (hasAOTextures) {
                DescriptorSet& aoDefaultRenderingSet = GPUContext::instance().getDescriptorSets(hzShader, 1, 1, 5)[0];
                aoDefaultRenderingSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::AO), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
                aoDefaultRenderingSet.updateDescriptorSet();
            }
            bool hasEmissiveTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::EMISSIVE).size() != 0;
            if (hasEmissiveTextures) {
                DescriptorSet& emissiveDefaultRenderingSet = GPUContext::instance().getDescriptorSets(hzShader, 1, 1, 6)[0];
                emissiveDefaultRenderingSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::EMISSIVE), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
                emissiveDefaultRenderingSet.updateDescriptorSet();
            }			
        }
        void OpaqueRenderer::drawNextFrame() {

        } 
        void OpaqueRenderer::clear() {

        } 
        unsigned int OpaqueRenderer::getLayer() {
            return layer;
        }    
        void OpaqueRenderer::draw() {
            if (needToUpdateDescriptorSets) {
                updateDescriptorSets();
                needToUpdateDescriptorSets  = false;
            }
            BlendMode blendMode;
            RenderStates states;
            states.shader = &hzShader;   
            hzVertPC.projMatrix = parentRenderer.getCamera().getProjMatrix().getMatrix().transpose();
            hzVertPC.projMatrix = parentRenderer.getCamera().getViewMatrix().getMatrix().transpose(); 
            hzVertPC.currentFrame = parentRenderer.getCurrentFrame(); 
            parentRenderer.beginRendering();        
            for(unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {
                hzVertPC.primitiveType = i;
                vkCmdPushConstants(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(i), hzShader, blendMode, RenderTarget::NODEPTHNOSTENCIL).getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(RenderTarget::ViewProjMatPC), &hzVertPC);
                parentRenderer.draw(parentRenderer.getCommandPool(),static_cast<entity::PrimitiveType>(i), states);
            }
            parentRenderer.endRendering();                      
            parentRenderer.getDepthStencilTexture().generateDepthMipmaps();
        }
    }
}