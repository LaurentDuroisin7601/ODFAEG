module;
#include <vulkan/vulkan.hpp>
#include "../../../include/odfaeg/config.hpp"
#include <odfaeg/Window/windowHandle.hpp>
#include <iostream>
//import odfaeg.graphic.renderWindow;
module odfaeg.graphic.renderWindow;
import odfaeg.graphic.gpuContext;
import odfaeg.graphic.particleSystemUpdater;
import odfaeg.graphic.morphAnimUpdater;
import odfaeg.graphic.boneAnimUpdater;
namespace odfaeg {
	namespace graphic {
        ////////////////////////////////////////////////////////////
        RenderWindow::RenderWindow(window::VideoMode mode, const core::String& title, Device& device, std::uint32_t style, bool useDepth, bool useStencil) : useDepth(useDepth), useStencil(useStencil), RenderTarget(device, useDepth, useStencil), device(device), swapchain(device)
        {
            //std::cout<<"instance in render window : "<<this->device.getInstance().getInstance()<<std::endl;
            //this->device.getInstance().setInstance(VK_NULL_HANDLE);
            // Don't call the base class constructor because it contains virtual function calls

            create(mode, title, style);
        }
        ////////////////////////////////////////////////////////////
        RenderWindow::RenderWindow(window::WindowHandle handle, Device& device, bool useDepth, bool useStencil) : RenderTarget(device, useDepth, useStencil), device(device), swapchain(device)
        {
            // Don't call the base class constructor because it contains virtual function calls
            create(handle);
        }
	    void RenderWindow::onClose() {
            /*ParticleSystemUpdater::instance().stop();
            MorphAnimUpdater::instance().stop();
            BoneAnimUpdater::instance().stop();
            waitDeviceIdle();*/
        }
        void RenderWindow::cleanup() {
            vkDeviceWaitIdle(device.getDevice());
            cleanupSwapchain();
            if (surface != VK_NULL_HANDLE) {
                //std::cout<<"destroy surface"<<std::endl;
                vkDestroySurfaceKHR(device.getInstance().getInstance(), surface, nullptr);
                surface = VK_NULL_HANDLE;
            }
        }
        void RenderWindow::createSurface() {
            surface = window::Window::createSurface(device.getInstance().getInstance());
        }
        void RenderWindow::createSwapchain() {
            int width, height;
            getFramebufferSize(width, height);
            swapchain.create(surface, isVerticalSynchEnabled(), width, height);
        }        
        void RenderWindow::createImageViews() {
            swapchain.createImageViews();
        }
        void RenderWindow::createRenderPass() {
            if (useDepthTest() || useStencilTest()) {
                renderPasses.reserve(3);
            } else {
                renderPasses.reserve(2);
            }
            renderPasses.emplace_back(device);
            renderPasses[0].create(swapchain.getSwapchainImageFormat(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

            if (useDepthTest() || useStencilTest()) {
                //std::cout<<"create depth texture!"<<std::endl;
                renderPasses.emplace_back(device);
                renderPasses.emplace_back(device);
                renderPasses[1].create(getDepthStencilTexture().getFormat());
                renderPasses[2].create(swapchain.getSwapchainImageFormat(), getDepthStencilTexture().getFormat(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);                
            } else {
                renderPasses.emplace_back(device);
                renderPasses[1].create(getDepthStencilTexture().getFormat());
            }
        }
        void RenderWindow::createSyncObjects() {
            imageAvailableSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
            inFlightFences.reserve(MAX_FRAMES_IN_FLIGHT);
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                imageAvailableSemaphores.emplace_back(device);
                imageAvailableSemaphores.back().create();
                inFlightFences.emplace_back(device);
                inFlightFences.back().create();
            }
            renderFinishedSemaphores.reserve(swapchain.getSwapchainImages().size());
            for (size_t i = 0; i < swapchain.getSwapchainImages().size(); i++) {
                renderFinishedSemaphores.emplace_back(device);
                renderFinishedSemaphores.back().create();
            }
            imagesInFlight.resize(swapchain.getSwapchainImages().size(), VK_NULL_HANDLE);
        }
        uint32_t RenderWindow::getCurrentFrame() {
            return currentFrame;
        }        
        std::vector<FrameBuffer>& RenderWindow::getFrameBuffers(unsigned int frameBufferId) {
            return frameBuffers[frameBufferId];
        }        
        VkSurfaceKHR RenderWindow::getSurface() {
            return surface;
        }        
        RenderPass& RenderWindow::getRenderPass(unsigned int renderPassId) {
            return renderPasses[renderPassId];
        }
        VkExtent2D RenderWindow::getExtents() {
            return swapchain.getSwapchainExtents();
        }
	    VkFormat& RenderWindow::getImageFormat() {
            return swapchain.getSwapchainImageFormat();
        }
        void RenderWindow::clear(const Color& clearColor) {
            //Solve present to present write read hazard.
            /*if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
                vkWaitForFences(device.getDevice(), 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
                vkResetFences(device.getDevice(), 1, &imagesInFlight[imageIndex]);
            }*/
            //std::cout<<"clear : "<<getCurrentFrame()<<std::endl;
            this->clearColor = clearColor;
            VkResult result = vkAcquireNextImageKHR(device.getDevice(), swapchain.getHandle(), UINT64_MAX, imageAvailableSemaphores[currentFrame].getHandle(), inFlightFences[currentFrame].getHandle(), &imageIndex);
            if (result == VK_ERROR_OUT_OF_DATE_KHR) {
                framebufferResized = false;
                recreateSwapchain();
                return; // <-- IMPORTANT : ne pas continuer drawFrame()
            }
            if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
                throw std::runtime_error("failed to acquire swap chain image!");
            }
            //Solve barrier after acquire image write read hazard.
            vkWaitForFences(device.getDevice(), 1, &inFlightFences[currentFrame].getHandle(), VK_TRUE, UINT64_MAX);
            vkResetFences(device.getDevice(), 1, &inFlightFences[currentFrame].getHandle());



            firstSubmit = true;
           
            VkClearColorValue clearValue = { clearColor.r / 255.f, clearColor.g / 255.f, clearColor.b / 255.f, clearColor.a / 255.f };


            VkImageSubresourceRange imageRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            };

            VkImageMemoryBarrier presentToClearBarrier {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .pNext = nullptr,
                .srcAccessMask = 0,
                .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = swapchain.getSwapchainImages()[imageIndex].getHandle(),
                .subresourceRange = imageRange
            };
            VkImageMemoryBarrier clearToPresentBarrier{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .pNext = nullptr,
                .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = swapchain.getSwapchainImages()[imageIndex].getHandle(),
                .subresourceRange = imageRange
            };
            //std::cout<<"begin record command buffer : "<<currentFrame<<std::endl;
            beginRecordCommandBuffer();
            vkCmdPipelineBarrier(getCommandPool().getHandle(currentFrame), VK_PIPELINE_STAGE_NONE, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &presentToClearBarrier);
            vkCmdClearColorImage(getCommandPool().getHandle(currentFrame), swapchain.getSwapchainImages()[imageIndex].getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearValue, 1, &imageRange);
            vkCmdPipelineBarrier(getCommandPool().getHandle(currentFrame), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &clearToPresentBarrier);
            if (useDepthTest() || useStencilTest()) {
                VkClearDepthStencilValue clearDepthStencilValue = {
                    .depth = 1.f,
                    .stencil = 0
                };
                VkImageSubresourceRange imageRange2 = {
                    .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = getDepthStencilTexture().getLayerCount()
                };
                //std::cout<<"clear depth stencil"<<std::endl;
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
                    .image = getDepthStencilTexture().getImage().getHandle(),
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
                    .image = getDepthStencilTexture().getImage().getHandle(),
                    .subresourceRange = imageRange2
                };
                vkCmdPipelineBarrier(getCommandPool().getHandle(currentFrame), VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &depthStencilToClearBarrier);
                vkCmdClearDepthStencilImage(getCommandPool().getHandle(currentFrame), getDepthStencilTexture().getImage().getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearDepthStencilValue, 1, &imageRange2);
                vkCmdPipelineBarrier(getCommandPool().getHandle(currentFrame), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, 0, 0, nullptr, 0, nullptr, 1, &clearToDepthStencilBarrier);
            }
        }       
        uint32_t RenderWindow::getImageIndex() {
            return imageIndex;
        }        
        void RenderWindow::submit(bool lastSubmit, std::vector<VkSemaphore> signalSemaphores,
            std::vector<VkSemaphore> waitSemaphores, std::vector<VkPipelineStageFlags> waitStages,
            std::vector<uint64_t> signalValues,
            std::vector<uint64_t> waitValues,
            std::vector<VkFence> fences, unsigned int queueIndex, bool resetFence, bool resetFences, VkFence fenceToSubmit) {
            if (getCommandPool().getHandles().size() > 0) { 
                
                //std::cout<<"end record command buffer : "<<currentFrame<<std::endl;
                VkSubmitInfo submitInfo{};
                submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                if (firstSubmit) {

                    waitSemaphores.push_back(imageAvailableSemaphores[currentFrame].getHandle());
                    waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                    if (waitValues.size() > 0) {
                        ////////std::cout<<"wait semaphore : "<<semaphore[currentFrame]<<std::endl;
                        waitValues.push_back(0);
                    }
                    //std::cout<<"first submit"<<std::endl;
                }
                if (lastSubmit) {
                    /*for (unsigned int i = 0; i < GPUContext::instance().getSharedSemaphore(0).size(); i++) {
                        signalSemaphores.push_back(GPUContext::instance().getSharedSemaphore(0)[i].getHandle());
                        signalValues.push_back(GPUContext::instance().getSharedSemaphore(0)[i].getValue()+1);
                        GPUContext::instance().getSharedSemaphore(0)[i].incrementValue();
                    }*/
                    signalSemaphores.push_back(renderFinishedSemaphores[imageIndex].getHandle());
                    if (signalValues.size() > 0) {
                        signalValues.push_back(0);
                    }
                    VkImageSubresourceRange imageRange = {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1
                    };
                    VkImageMemoryBarrier toPresent = {
                        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                        .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
                        .oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                        .image = swapchain.getSwapchainImages()[imageIndex].getHandle(),
                        .subresourceRange = imageRange
                    };
                    vkCmdPipelineBarrier(
                        getCommandPool().getHandle(getCurrentFrame()),                        
                        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                        0,
                        0, nullptr,
                        0, nullptr,
                        1, &toPresent
                    );
                    //std::cout<<"last submit"<<std::endl;
                }
                endRecordCommandBuffer();
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
                //std::cout<<"submit infos set : "<<getCommandPool().getHandle(currentFrame)<<std::endl;

                /*if (fences.size() > 0) {
                    vkWaitForFences(device.getDevice(), fences.size(), fences.data(), VK_TRUE, UINT64_MAX);
                    if (resetFences)
                        vkResetFences(device.getDevice(), fences.size(), fences.data());
                }*/
                //std::cout<<"current frame : "<<currentFrame<<std::endl;
                Device::QueueFamilyIndices indices = device.findQueueFamilies(device.getPhysicalDevice(), getSurface());
                if (vkQueueSubmit(device.getQueue(indices.graphicsFamily.value(), queueIndex), 1, &submitInfo, (fenceToSubmit == nullptr) ? inFlightFences[currentFrame].getHandle() : fenceToSubmit) != VK_SUCCESS) {
                    throw std::runtime_error("échec de l'envoi d'un command buffer window!");
                }
                inFlightFences[currentFrame].waitForFences(VK_TRUE, UINT64_MAX);
                inFlightFences[currentFrame].resetFences();
                //std::cout<<"submitted"<<std::endl;
                //Cette image est utilisée par cette frame.
                //imagesInFlight[imageIndex] = inFlightFences[currentFrame].getHandle();
                firstSubmit = false;
                //resetVertexBufferDatas();
            }
        }
        void RenderWindow::drawVulkanFrame() {
            if (getCommandPool().getHandles().size() > 0) {
                //std::cout<<"present : "<<imageIndex<<std::endl;
                //for (unsigned int i = 0; i < getCommandBuffers().size(); i++) {

                //}

                // V�rifier si une frame pr�c�dente est en train d'utiliser cette image (il y a une fence � attendre)
                VkSemaphore waitSemaphores[] = { renderFinishedSemaphores[imageIndex].getHandle()};
                VkPresentInfoKHR presentInfo{};
                presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

                presentInfo.waitSemaphoreCount = 1;
                presentInfo.pWaitSemaphores = waitSemaphores;
                VkSwapchainKHR swapchains[] = { swapchain.getHandle()};
                presentInfo.swapchainCount = 1;
                presentInfo.pSwapchains = swapchains;
                presentInfo.pImageIndices = &imageIndex;
                presentInfo.pResults = nullptr; // Optionnel
                Device::QueueFamilyIndices indices = device.findQueueFamilies(device.getPhysicalDevice(), getSurface());
                VkResult result = vkQueuePresentKHR(device.getQueue(indices.presentFamily.value(), 0), &presentInfo);
                if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
                    framebufferResized  =   false;
                    recreateSwapchain();
                }
                currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
                //std::cout<<"presented"<<std::endl;
                //vkDeviceWaitIdle(vkDevice.getDevice());
            }
        }
        void RenderWindow::cleanupSwapchain() {
            for (unsigned int j = 0; j < frameBuffers.size(); j++) {
                frameBuffers[j].clear();
                for (size_t i = 0; i < frameBuffers[j].size(); i++) {
                    frameBuffers[j][i].cleanup();
                }
            }
            frameBuffers.clear();
            for (unsigned int i = 0; i < renderPasses.size(); i++) {
                renderPasses[i].cleanup();
            }
            renderPasses.clear();
            swapchain.cleanup();
        }
        void RenderWindow::recreateSwapchain() {
            vkDeviceWaitIdle(device.getDevice());
            cleanupSwapchain();
            createSwapchain();
            createImageViews();
            /*createRenderPass();
            createFrameBuffers();*/
        }
        ////////////////////////////////////////////////////////////
        RenderWindow::~RenderWindow()
        {
            //////std::cout<<"destroy window"<<std::endl;
            cleanup();
        }
        ////////////////////////////////////////////////////////////
        math::Vector2u RenderWindow::getSize() const
        {
            return Window::getSize();
        }
        ////////////////////////////////////////////////////////////
        void RenderWindow::onCreate()
        {
            // Just initialize the render target part
            //std::cout<<"create instance!"<<std::endl;
            viewMask = 0;
            device.createInstance();
            //std::cout<<"create surface : "<<device.getInstance().getInstance()<<std::endl;
            createSurface();
            //std::cout<<"pickup physical device!"<<std::endl;
            device.pickupPhysicalDevice(surface);
            //std::cout<<"create device!"<<std::endl;
            device.createLogicalDevice(surface);

            //std::cout<<"create swapchain!"<<std::endl;
            createSwapchain();
            //std::cout<<"create image views!"<<std::endl;
            createImageViews();
            getDepthStencilTexture().createDepthTexture(getExtents().width, getExtents().height);
            //std::cout<<"create dp cmds"<<std::endl;

            //std::cout<<"create render pass!"<<std::endl;
            createRenderPass();
            //std::cout<<"create frame buffers!"<<std::endl;
            createFrameBuffers();
            createSyncObjects();
            //std::cout<<"initialize rt"<<std::endl;
            RenderTarget::initialize();
            currentFrame = 0;
            imageIndex = 0;
            //std::cout<<"window created!"<<std::endl;
        }


        ////////////////////////////////////////////////////////////
        void RenderWindow::onResize()
        {            
            framebufferResized = true;
        }
        void RenderWindow::createFrameBuffers() {
            //std::cout<<"extents : "<<getExtents().width<<", "<<getExtents().height<<std::endl;
            if (useDepthTest() || useStencilTest())
                frameBuffers.resize(3);
            else
                frameBuffers.resize(2);
            frameBuffers[0].reserve(swapchain.getSwapchainImages().size());
            frameBuffers[1].reserve(swapchain.getSwapchainImages().size());
            for (size_t i = 0; i < swapchain.getSwapchainImages().size(); i++) {
                frameBuffers[0].emplace_back(device);
                frameBuffers[0][i].create(renderPasses[0], swapchain.getSwapchainImages()[i].getImageView(), getExtents().width, getExtents().height);
                frameBuffers[1].emplace_back(device);
                frameBuffers[1][i].create(renderPasses[1], getDepthStencilTexture().getImage().getImageView(), getExtents().width, getExtents().height);
            }
            if (useDepthTest() || useStencilTest()) {
                frameBuffers[2].reserve(swapchain.getSwapchainImages().size());
                for (size_t i = 0; i < swapchain.getSwapchainImages().size(); i++) {                    
                    frameBuffers[2].emplace_back(device);
                    frameBuffers[2][i].create(renderPasses[2], swapchain.getSwapchainImages()[i].getImageView(), getDepthStencilTexture().getImage().getImageView(), getExtents().width, getExtents().height);
                }
            }
        }
        void RenderWindow::waitDeviceIdle() {
            vkDeviceWaitIdle(device.getDevice());
        }
        Device& RenderWindow::getDevice() {
            return device;
        }
	    bool RenderWindow::isDepthOnly() {
            return false;
        }
	    std::uint32_t& RenderWindow::getViewMask() {
            return viewMask;
        }
        std::uint32_t RenderWindow::getSwapchainMinImagesCount() {
            return swapchain.getMinImagesCount();
        }
        std::uint32_t RenderWindow::getSwapchainImagesCount() {
            return swapchain.getSwapchainImages().size();
        }
        void RenderWindow::beginRenderPass(bool useSecondaryCommandBuffer) {
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
        void RenderWindow::endRenderPass() {
            vkCmdEndRenderPass(getCommandPool().getHandle(getCurrentFrame()));
        }
	    void RenderWindow::beginRendering(bool secondaryCommandBuffers) {
            VkRenderingAttachmentInfo colorAttachmentInfo = {
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = swapchain.getSwapchainImages()[imageIndex].getImageView().getHandle(),
                .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = {.color = {0.0f, 0.0f, 0.0f, 1.0f}}
            };
            VkRenderingAttachmentInfo depthAttachmentInfo = {
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = getDepthStencilTexture().getImage().getImageView().getHandle(),
                .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = {.depthStencil{1.f, 0}}
            };
            VkRenderingInfo renderingInfo = {
                .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
                .renderArea = {
                    .offset { .x=0, .y=0 },
                    .extent = getExtents()
                },
                .layerCount = getDepthStencilTexture().getLayerCount(),
                .colorAttachmentCount = 1,
                .pColorAttachments = &colorAttachmentInfo,
                .pDepthAttachment = &depthAttachmentInfo
            };
            renderingInfo.flags = (secondaryCommandBuffers) ? VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT : 0;
            vkCmdBeginRendering(getCommandPool().getHandle(getCurrentFrame()),&renderingInfo);
        }        
	    void RenderWindow::endRendering() {
            vkCmdEndRendering(getCommandPool().getHandle(getCurrentFrame()));            
        }       
	}
}