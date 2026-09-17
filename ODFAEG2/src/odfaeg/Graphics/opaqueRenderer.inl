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
            DescriptorSetLayout& defaultRenderingLayout = GPUContext::instance().getDescriptorSetLayout(hzShader, 1);
            defaultRenderingLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES*MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_VERTEX_BIT);
            
            
            defaultRenderingLayout.update();
            BlendMode blendMode;
            std::vector<VkPushConstantRange> pushConstants;
            VkPushConstantRange vsPushConstant;
            vsPushConstant.offset = 0;
            vsPushConstant.size = sizeof(RenderTarget::ViewProjMatPC);
            vsPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            pushConstants.push_back(vsPushConstant);
            for (unsigned int p = 0; p < NB_PRIMITIVE_TYPES; p++) {
                GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(p), hzShader, blendMode, RenderTarget::DEPTHNOSTENCIL).createGraphicPipeline( hzShader, static_cast<entity::PrimitiveType>(p),GPUContext::instance().getDescriptorSetLayout(hzShader), renderingCreateInfo, parentRenderer.getDepthStencilInfos()[RenderTarget::DEPTHNOSTENCIL], blendMode, GPUContext::instance().getDevice().getMsaaSamples(), VK_CULL_MODE_NONE, VK_POLYGON_MODE_FILL, pushConstants);
                //std::cout<<"pipeline created in render target : "<<GPUContext::instance().getGraphicsPipeline(static_cast<PrimitiveType>(p), defaultRenderingShader, blendMode,i).getHandle()<<std::endl;
            }            
            DescriptorPool& defaultRenderingPool = GPUContext::instance().getDescriptorPool(hzShader, 1);
            defaultRenderingPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES * MAX_FRAMES_IN_FLIGHT);
                        
            defaultRenderingPool.update();
            DescriptorSet::allocate(defaultRenderingPool, defaultRenderingLayout, GPUContext::instance().getDescriptorSets(hzShader, 1, 1));
        }
        void OpaqueRenderer::updateDescriptorSets() {
            DescriptorSet& defaultRenderingSet = GPUContext::instance().getDescriptorSets(hzShader, 1, 1)[0];
            defaultRenderingSet.updateBufferInfos(0, GPUContext::instance().getSharedBuffers(RenderTarget::OUTPUT_MODELS+parentRenderer.getId()*RenderTarget::NB_BUFFERS), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);				
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
            parentRenderer.submit(true);            
            parentRenderer.getDepthStencilTexture().generateDepthMipmaps();
        }
    }
}