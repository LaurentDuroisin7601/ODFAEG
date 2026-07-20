module;
#include <vulkan/vulkan.hpp>
#include <iostream>
//import odfaeg.graphic.renderTexture;
module odfaeg.graphic.renderTexture;
namespace odfaeg {
	namespace graphic {
        RenderTexture::RenderTexture(Device& device, bool useDepthTest, bool useStencilTest) : RenderTarget(device, useDepthTest, useStencilTest), device(device),enableDepthTest(useDepthTest), enableStencilTest(useStencilTest) {
            currentFrame = imageIndex = 0;
            firstSubmit = true;
            depthOnly = false;
            viewMask = 0;
            imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
        }
        bool RenderTexture::create(unsigned int width, unsigned int height, unsigned int depth, bool layered, bool depthOnly) {
            //std::cout<<"create!"<<std::endl; 
            device.createInstance();
            device.pickupPhysicalDevice(VK_NULL_HANDLE);
            device.createLogicalDevice(VK_NULL_HANDLE);
            viewMask = (depth == 1) ? 0 : (1 << depth) - 1;
            if (!depthOnly) {              
                
                for (unsigned int i = 0; i < 1; i++) {
                    std::cout<<"create texture"<<std::endl;
                    m_textures.emplace_back(device, NB_SWAPCHAIN_IMAGES);
                    //std::cout<<"images : "<<m_textures.back().getImages().size()<<std::endl;
                    m_textures.back().create(width, height, depth, 1, false, true);
                }
            }
            //std::cout<<"images : "<<m_textures.back().getImages().size()<<std::endl;
            m_size[0] = width;
            m_size[1] = height;
            this->depthOnly = depthOnly;           
            //if (useDepthTest() || useDepthTest() || depthOnly) {
                      
                getDepthStencilTexture().createDepthTexture(getExtents().width, getExtents().height, depth, layered);
            //}
            /*createRenderPass();
            createFramebuffers();*/
            createSyncObjects();


            RenderTarget::initialize();

            return true;
        }
        bool RenderTexture::createCubeMap(unsigned int size, bool depthOnly, bool layered) {
            //std::cout<<"create!"<<std::endl;
            
            
            device.createInstance();
            device.pickupPhysicalDevice(VK_NULL_HANDLE);
            device.createLogicalDevice(VK_NULL_HANDLE);
            if (!depthOnly) {                
                for (unsigned int i = 0; i < 1; i++) {
                    m_textures.emplace_back(device, NB_SWAPCHAIN_IMAGES);
                    m_textures.back().createCubeMap(size, layered, true);
                }  
            }
            m_size[0] = size;
            m_size[1] = size;
            this->depthOnly = depthOnly;
            viewMask = 0b00111111;
            ////std::cout<<"cm dt created"<<std::endl;
            //createRenderPass();
            ////std::cout<<"render pass created"<<std::endl;
            //createFramebuffers();
            ////std::cout<<"frame buffer created"<<std::endl;
            createSyncObjects();
            getDepthStencilTexture().createDepthCubeMap(size, layered);
            
            currentFrame = imageIndex = 0;
            RenderTarget::initialize();
            return true;
        }        
        void RenderTexture::createSyncObjects() {
            inFlightFences.reserve(RT_MAX_FRAMES_IN_FLIGHT);             
            for (size_t i = 0; i < RT_MAX_FRAMES_IN_FLIGHT; i++) {
                inFlightFences.emplace_back(device);
                inFlightFences[i].create();               
            }  
            imageAvailableSemaphores.reserve(1);
            imageAvailableSemaphores.emplace_back(device);
            imageAvailableSemaphores[0].create(true, 0);            
        }
        VkSurfaceKHR RenderTexture::getSurface() {
            return VK_NULL_HANDLE;
        }
        VkExtent2D RenderTexture::getExtents() {
            VkExtent2D actualExtent = {
                static_cast<uint32_t>(m_size.x()),
                static_cast<uint32_t>(m_size.y())
            };
            return actualExtent;
        }       
        uint32_t RenderTexture::getCurrentFrame() {
            return currentFrame;
        }
	    Texture& RenderTexture::getTexture(unsigned int attachmentPoint) {
            return m_textures[attachmentPoint];
        }
        math::Vector2u RenderTexture::getSize() const {
            return m_size;
        }
        std::vector<FrameBuffer>& RenderTexture::getFrameBuffers(unsigned int frameBufferId) {
            return frameBuffers[frameBufferId];
        }
        RenderPass& RenderTexture::getRenderPass(unsigned int renderPassId) {
            return renderPasses[renderPassId];
        }
        std::deque<Texture>& RenderTexture::getTextures() {
            return m_textures;
        }
        void RenderTexture::createFramebuffers() {
            if (!useDepthTest() && !useStencilTest()) {
                frameBuffers.resize(2);
                for (unsigned int j = 0; j < m_textures.size(); j++) {
                    frameBuffers[0].reserve(m_textures[j].getImages().size());
                    frameBuffers[1].reserve(m_textures[j].getImages().size());
                    for (size_t i = 0; i < m_textures[j].getImages().size(); i++) {
                        frameBuffers[0].emplace_back(device);
                        frameBuffers[0][i].create(getRenderPass(0), m_textures[j].getImages()[i].getImageView(), getExtents().width, getExtents().height);
                        frameBuffers[1].emplace_back(device);
                        frameBuffers[1][i].create(getRenderPass(1), m_textures[j].getImages()[i].getImageView(), getExtents().width, getExtents().height);
                    }
                }
            } else {
                frameBuffers.resize(3);
                for (size_t j = 0; j < m_textures.size(); j++) {
                    frameBuffers[0].reserve(m_textures[j].getImages().size());
                    frameBuffers[1].reserve(m_textures[j].getImages().size());
                    frameBuffers[2].reserve(m_textures[j].getImages().size());
                    for (size_t i = 0; i < m_textures[j].getImages().size(); i++) {
                        frameBuffers[0].emplace_back(device);
                        frameBuffers[0][i].create(getRenderPass(0), m_textures[j].getImages()[i].getImageView(), getExtents().width, getExtents().height);
                        frameBuffers[1].emplace_back(device);
                        frameBuffers[1][i].create(getRenderPass(1), m_textures[j].getImages()[i].getImageView(),getDepthStencilTexture().getImage(i).getImageView(), getExtents().width, getExtents().height);
                        frameBuffers[2].emplace_back(device);
                        frameBuffers[2][i].create(getRenderPass(2), getDepthStencilTexture().getImage(i).getImageView(), getExtents().width, getExtents().height);
                    }
                }
            }
        }
        void RenderTexture::createRenderPass() {
            if (!useDepthTest() && !useStencilTest()) {
                renderPasses.reserve(2);
                renderPasses.emplace_back(device);
                renderPasses.emplace_back(device);
                if (viewMask == 0) {
                    renderPasses[0].create(imageFormat, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
                } else {
                    renderPasses[0].create(imageFormat, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, viewMask);
                }
                if (viewMask == 0) {
                    renderPasses[1].create(getDepthStencilTexture().getFormat());
                } else {
                    renderPasses[1].create(getDepthStencilTexture().getFormat(), viewMask);
                }
            } else {
                renderPasses.reserve(3);
                renderPasses.emplace_back(device);
                renderPasses.emplace_back(device);
                renderPasses.emplace_back(device);
                if (viewMask == 0) {
                    renderPasses[0].create(imageFormat, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
                    renderPasses[1].create(imageFormat, getDepthStencilTexture().getFormat());
                    renderPasses[2].create(getDepthStencilTexture().getFormat());
                } else {
                    renderPasses[0].create(imageFormat, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, viewMask);
                    renderPasses[1].create(imageFormat, getDepthStencilTexture().getFormat(), viewMask);
                    renderPasses[2].create(getDepthStencilTexture().getFormat(), viewMask);
                }
            }
        }
        void RenderTexture::clear(const entity::Color& clearColor) {
            firstSubmit = true;
            beginRecordCommandBuffer();
            if (!isDepthOnly()) {
                /*std::cout<<"texture size : "<<m_textures.size()<<std::endl;
                std::cout<<"nb images : "<<m_textures[0].getImages().size()<<std::endl;
                system("PAUSE");*/
                VkClearColorValue clearValue = { clearColor.r / 255.f, clearColor.g / 255.f, clearColor.b / 255.f, clearColor.a / 255.f };

                VkImageSubresourceRange imageRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                };

                
                VkImageMemoryBarrier presentToClearBarrier{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .pNext = nullptr,
                    .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
                    .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                    .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image = m_textures[0].getImage(imageIndex).getHandle(),
                    .subresourceRange = imageRange
                };
                VkImageMemoryBarrier clearToPresentBarrier{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .pNext = nullptr,
                    .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                    .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
                    .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image = m_textures[0].getImage(imageIndex).getHandle(),
                    .subresourceRange = imageRange
                };
                vkCmdPipelineBarrier(getCommandPool().getHandle(currentFrame), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &presentToClearBarrier);
                vkCmdClearColorImage(getCommandPool().getHandle(currentFrame), m_textures[0].getImage(imageIndex).getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearValue, 1, &imageRange);
                vkCmdPipelineBarrier(getCommandPool().getHandle(currentFrame), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &clearToPresentBarrier);
            }
            VkClearDepthStencilValue clearDepthStencilValue = {
               .depth = 1.f,
               .stencil = 0
            };
            //std::cout<<"nb layouts : "<<getDepthStencilTexture().getLayerCount()<<std::endl;
            VkImageSubresourceRange imageRange2 = {
                .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount =  getDepthStencilTexture().getLayerCount()
            };
            VkImageMemoryBarrier depthStencilToClearBarrier{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .pNext = nullptr,
                .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                                 VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = getDepthStencilTexture().getImage(0).getHandle(),
                .subresourceRange = imageRange2
            };
            VkImageMemoryBarrier clearToDepthStencilBarrier{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .pNext = nullptr,
                .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                                 VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = getDepthStencilTexture().getImage(0).getHandle(),
                .subresourceRange = imageRange2
            };
            vkCmdPipelineBarrier(getCommandPool().getHandle(currentFrame), VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &depthStencilToClearBarrier);
            vkCmdClearDepthStencilImage(getCommandPool().getHandle(currentFrame), getDepthStencilTexture().getImage(0).getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearDepthStencilValue, 1, &imageRange2);
            vkCmdPipelineBarrier(getCommandPool().getHandle(currentFrame), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, 0, 0, nullptr, 0, nullptr, 1, &clearToDepthStencilBarrier);
        }        
        uint32_t RenderTexture::getImageIndex() {
            return imageIndex;
        }        
        void RenderTexture::display() {
            currentFrame = (currentFrame + 1) % RT_MAX_FRAMES_IN_FLIGHT;
            imageIndex = (imageIndex + 1) % NB_SWAPCHAIN_IMAGES;
        }        
        void RenderTexture::submit(bool lastSubmit, std::vector<VkSemaphore> signalSemaphores,
            std::vector<VkSemaphore> waitSemaphores, std::vector<VkPipelineStageFlags> waitStages,
            std::vector<uint64_t> signalValues,
            std::vector<uint64_t> waitValues, std::vector<VkFence> fences, unsigned int queueIndex, bool resetFence, bool resetFences, VkFence fenceToSubmit) {
            if (getCommandPool().getHandles().size() > 0) {
                getCommandPool().endRecordCommandBuffer(currentFrame);
                if (firstSubmit) {
                    if (waitValues.size() == 0 && waitSemaphores.size() > 0) {
                        for (unsigned int i = 0; i < waitSemaphores.size(); i++) {
                            waitValues.push_back(0);
                        }
                    }
                    /*waitSemaphores.push_back(imageAvailableSemaphores[0].getHandle());
                    waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                    waitValues.push_back(imageAvailableSemaphores[0].getValue());*/
                    //std::cout<<"wait on value : "<<value<<std::endl;
                    firstSubmit = false;
                }
                if (lastSubmit) {
                    if (signalValues.size() == 0 && signalSemaphores.size() > 0) {
                        for (unsigned int i = 0; i < signalSemaphores.size(); i++) {
                            signalValues.push_back(0);
                        }
                    }
                    /*signalSemaphores.push_back(imageAvailableSemaphores[0].getHandle());                    
                    signalValues.push_back(imageAvailableSemaphores[0].getValue()+1);                
                    imageAvailableSemaphores[0].incrementValue();*/
                }
                VkSubmitInfo submitInfo{};
                submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                VkTimelineSemaphoreSubmitInfo timelineInfo{};
                if (signalValues.size() > 0 || waitValues.size() > 0) {
                    timelineInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
                    timelineInfo.waitSemaphoreValueCount = waitValues.size();
                    timelineInfo.pWaitSemaphoreValues = waitValues.data();
                    timelineInfo.signalSemaphoreValueCount = signalValues.size();
                    timelineInfo.pSignalSemaphoreValues = signalValues.data();
                    submitInfo.pNext = &timelineInfo;
                }
                submitInfo.signalSemaphoreCount = signalSemaphores.size();
                submitInfo.pSignalSemaphores = signalSemaphores.data();
                submitInfo.pWaitDstStageMask = waitStages.data();
                submitInfo.waitSemaphoreCount = waitSemaphores.size();
                submitInfo.pWaitSemaphores = waitSemaphores.data();
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &getCommandPool().getHandle(currentFrame);
                if (fences.size() > 0) {
                    VkResult r = vkWaitForFences(device.getDevice(), fences.size(), fences.data(), VK_TRUE, UINT64_MAX);
                    if (r == -4)
                        printf("wait for fences result : %d\n", r);
                    if (resetFences)
                        vkResetFences(device.getDevice(), fences.size(), fences.data());
                }

                Device::QueueFamilyIndices indices = device.findQueueFamilies(device.getPhysicalDevice(), nullptr);
                //std::cout<<"render texture submit on frame : "<<currentFrame<<std::endl;
                if (vkQueueSubmit(device.getQueue(indices.graphicsFamily.value(), queueIndex), 1, &submitInfo, (fenceToSubmit == nullptr) ? inFlightFences[currentFrame].getHandle() : fenceToSubmit) != VK_SUCCESS) {
                    throw std::runtime_error("�chec de l'envoi d'un graphic command buffer! ");
                }
                std::cout<<"wait on fence : "<<inFlightFences[currentFrame].getHandle()<<std::endl;
                VkResult r2 = vkWaitForFences(device.getDevice(), 1, (fenceToSubmit == nullptr) ? &inFlightFences[currentFrame].getHandle() : &fenceToSubmit, VK_TRUE, UINT64_MAX);
                if (r2 == -4)
                    printf("wait for fence result : %d\n", r2);
                if (resetFence) {  
                    //std::cout<<"reset fence : "<<inFlightFences[currentFrame].getHandle()<<std::endl;                  
                    vkResetFences(device.getDevice(), 1, (fenceToSubmit == nullptr) ? &inFlightFences[currentFrame].getHandle() : &fenceToSubmit);
                }
                //resetVertexBufferDatas();
             }
        }
	    VkFormat& RenderTexture::getImageFormat() {
            return imageFormat;
        }
	    bool RenderTexture::isDepthOnly() {
            return depthOnly;
        }
        RenderTexture::~RenderTexture() {
            vkDeviceWaitIdle(device.getDevice());            
        }
	    std::uint32_t& RenderTexture::getViewMask() {
            return viewMask;
        }
        void RenderTexture::beginRenderPass(bool useSecondaryCommandBuffer) {
            //std::cout<<"render pass depth ? "<<(depthTestEnabled)<<","<<(stencilTestEnabled)<<std::endl;
            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = (useDepthTest() || useStencilTest()) ? renderPasses[1].getHandle() : renderPasses[0].getHandle();
            renderPassInfo.framebuffer = (useDepthTest() || useStencilTest()) ? frameBuffers[1][getImageIndex()].getHandle() : frameBuffers[0][getImageIndex()].getHandle();
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = getExtents();
            ////////std::cout<<"render pass : "<<(m_view.getViewport().getSize().x == 800 && m_view.getViewport().getSize().y == 800)<<std::endl;

            //VkClearValue clrColor = {clearColor.r / 255.f,clearColor.g / 255.f, clearColor.b / 255.f, clearColor.a / 255.f};
            renderPassInfo.clearValueCount = 0;
            renderPassInfo.pClearValues = nullptr;


            vkCmdBeginRenderPass(getCommandPool().getHandle(getCurrentFrame()), &renderPassInfo, (!useSecondaryCommandBuffer) ? VK_SUBPASS_CONTENTS_INLINE : VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
        }
        void RenderTexture::endRenderPass() {
            vkCmdEndRenderPass(getCommandPool().getHandle(getCurrentFrame()));
        }
	    void RenderTexture::beginRendering(bool secondaryCommandBuffers) {
            VkRenderingInfo renderingInfo = {};
            VkRenderingAttachmentInfo depthAttachmentInfo = {
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = getDepthStencilTexture().getImage().getImageView().getHandle(),
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
            if (!depthOnly) {
                colorAttachmentInfo = {
                    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .imageView = m_textures[0].getImage(imageIndex).getImageView().getHandle(),
                    .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .clearValue = {.color = {0.0f, 0.0f, 0.0f, 1.0f}}
                };
                renderingInfo.colorAttachmentCount = 1;
                renderingInfo.pColorAttachments = &colorAttachmentInfo;
            } 
            if (viewMask != 0) {
                renderingInfo.viewMask = viewMask;
            }         
            renderingInfo.flags = (secondaryCommandBuffers) ? VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT : 0;
            vkCmdBeginRendering(getCommandPool().getHandle(currentFrame),&renderingInfo);
        }
	    void RenderTexture::endRendering() {
            vkCmdEndRendering(getCommandPool().getHandle(currentFrame));
        }
        std::vector<Semaphore>& RenderTexture::getSemaphores() {
            return imageAvailableSemaphores;
        }
        std::uint32_t RenderTexture::getSwapchainImagesCount() {
            return NB_SWAPCHAIN_IMAGES;
        }
	}
}