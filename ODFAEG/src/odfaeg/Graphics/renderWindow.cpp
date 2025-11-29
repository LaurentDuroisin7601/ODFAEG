#include "../../../include/odfaeg/openGL.hpp"
#include "../../../include/odfaeg/Graphics/renderWindow.h"

#ifndef VULKAN
#include "glCheck.h"
#endif


namespace odfaeg {
    namespace graphic {

        #ifdef VULKAN
        ////////////////////////////////////////////////////////////
        RenderWindow::RenderWindow(window::VideoMode mode, const core::String& title, window::Device& vkDevice, std::uint32_t style,  const window::ContextSettings& settings) : RenderTarget(vkDevice), vkDevice(vkDevice)
        {
            // Don't call the base class constructor because it contains virtual function calls
            create(mode, title, style, settings);
        }
         ////////////////////////////////////////////////////////////
        RenderWindow::RenderWindow(window::WindowHandle handle,  window::Device& vkDevice, const window::ContextSettings& settings) : vkDevice(vkDevice), RenderTarget(vkDevice)
        {
            // Don't call the base class constructor because it contains virtual function calls
            create(handle, settings);
        }
        void RenderWindow::cleanup() {
            vkDeviceWaitIdle(vkDevice.getDevice());
            cleanupSwapchain();
            for (unsigned int i = 0; i < renderPasses.size(); i++) {
                vkDestroyRenderPass(vkDevice.getDevice(), renderPasses[i], nullptr);
            }
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                vkDestroySemaphore(vkDevice.getDevice(), renderFinishedSemaphores[i], nullptr);
                vkDestroySemaphore(vkDevice.getDevice(), imageAvailableSemaphores[i], nullptr);
                vkDestroyFence(vkDevice.getDevice(), inFlightFences[i], nullptr);
            }
            vkDestroySurfaceKHR(vkDevice.getInstance(), surface, nullptr);
        }
        void RenderWindow::createSurface() {
             surface = window::Window::createSurface(vkDevice.getInstance());


        }
        void RenderWindow::createSwapChain() {
            window::Device::SwapChainSupportDetails swapChainSupport = vkDevice.querySwapChainSupport(vkDevice.getPhysicalDevice(), surface);

            VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
            VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
            VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
            uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
            if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
                imageCount = swapChainSupport.capabilities.maxImageCount;
            }

            VkSwapchainCreateInfoKHR createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            createInfo.surface = surface;

            createInfo.minImageCount = imageCount;
            createInfo.imageFormat = surfaceFormat.format;
            createInfo.imageColorSpace = surfaceFormat.colorSpace;
            createInfo.imageExtent = extent;
            createInfo.imageArrayLayers = 1;
            createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

            window::Device::QueueFamilyIndices indices = vkDevice.findQueueFamilies(vkDevice.getPhysicalDevice(), surface);
            uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

            if (indices.graphicsFamily != indices.presentFamily) {
                createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                createInfo.queueFamilyIndexCount = 2;
                createInfo.pQueueFamilyIndices = queueFamilyIndices;
            }
            else {
                createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            }

            createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
            createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            createInfo.presentMode = presentMode;
            createInfo.clipped = VK_TRUE;

            if (vkCreateSwapchainKHR(vkDevice.getDevice(), &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to create swap chain!", 1);
            }
            vkGetSwapchainImagesKHR(vkDevice.getDevice(), swapChain, &imageCount, nullptr);
            swapChainImages.resize(imageCount);
            vkGetSwapchainImagesKHR(vkDevice.getDevice(), swapChain, &imageCount, swapChainImages.data());

            swapChainImageFormat = surfaceFormat.format;
            swapChainExtent = extent;
        }
        VkSurfaceFormatKHR RenderWindow::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
            for (const auto& availableFormat : availableFormats) {
                if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    return availableFormat;
                }
            }

            return availableFormats[0];
        }
        VkPresentModeKHR RenderWindow::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
            for (const auto& availablePresentMode : availablePresentModes) {
                if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                    return availablePresentMode;
                }
            }

            return VK_PRESENT_MODE_FIFO_KHR;
        }
        VkExtent2D RenderWindow::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
            if (capabilities.currentExtent.width != UINT32_MAX) {
                return capabilities.currentExtent;
            } else {
                int width, height;
                getFramebufferSize(width, height);

                VkExtent2D actualExtent = {
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height)
                };

                actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
                actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

                return actualExtent;
            }
        }
        VkImageView RenderWindow::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = format;
            viewInfo.subresourceRange.aspectMask = aspectFlags;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = mipLevels;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            VkImageView imageView;
            if (vkCreateImageView(vkDevice.getDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to create texture image view!", 1);
            }

            return imageView;
        }
        void RenderWindow::createImageViews() {
            swapChainImageViews.resize(swapChainImages.size());

            for (uint32_t i = 0; i < swapChainImages.size(); i++) {
                swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
            }
        }
        void RenderWindow::createRenderPass() {
            for (unsigned int i = 0; i < 2; i++ ) {
                if (i == 0) {
                    VkAttachmentDescription colorAttachment{};
                    colorAttachment.format =    swapChainImageFormat;
                    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
                    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

                    VkAttachmentReference colorAttachmentRef{};
                    colorAttachmentRef.attachment = 0;
                    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                    VkSubpassDescription subpass{};
                    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                    subpass.colorAttachmentCount = 1;
                    subpass.pColorAttachments = &colorAttachmentRef;
                    subpass.pDepthStencilAttachment = nullptr;

                    std::array<VkAttachmentDescription, 1> attachments = {colorAttachment};

                    VkRenderPassCreateInfo renderPassInfo{};
                    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
                    renderPassInfo.pAttachments = attachments.data();
                    renderPassInfo.subpassCount = 1;
                    renderPassInfo.pSubpasses = &subpass;
                    VkSubpassDependency dependency{};
                    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
                    dependency.dstSubpass = 0;
                    dependency.srcStageMask =  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                    dependency.srcAccessMask = 0;
                    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    renderPassInfo.dependencyCount = 1;
                    renderPassInfo.pDependencies = &dependency;
                    VkRenderPass renderPass;
                    if (vkCreateRenderPass(vkDevice.getDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
                        throw core::Erreur(0, "failed to create render pass!", 1);
                    }
                    renderPasses.push_back(renderPass);
                } else {
                    VkAttachmentDescription colorAttachment{};
                    colorAttachment.format =    swapChainImageFormat;
                    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
                    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


                    VkAttachmentDescription depthAttachment{};
                    depthAttachment.format = getDepthTexture().findDepthFormat();
                    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
                    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
                    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

                    VkAttachmentReference colorAttachmentRef{};
                    colorAttachmentRef.attachment = 0;
                    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


                    VkAttachmentReference depthAttachmentRef{};
                    depthAttachmentRef.attachment = 1;
                    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

                    VkSubpassDescription subpass{};
                    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                    subpass.colorAttachmentCount = 1;
                    subpass.pColorAttachments = &colorAttachmentRef;
                    subpass.pDepthStencilAttachment = &depthAttachmentRef;

                    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

                    VkRenderPassCreateInfo renderPassInfo{};
                    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
                    renderPassInfo.pAttachments = attachments.data();
                    renderPassInfo.subpassCount = 1;
                    renderPassInfo.pSubpasses = &subpass;
                    VkSubpassDependency dependency{};
                    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
                    dependency.dstSubpass = 0;
                    dependency.srcStageMask =  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
                    dependency.srcAccessMask = 0;
                    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
                    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                    renderPassInfo.dependencyCount = 1;
                    renderPassInfo.pDependencies = &dependency;
                    VkRenderPass renderPass;
                    if (vkCreateRenderPass(vkDevice.getDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
                        throw core::Erreur(0, "failed to create render pass!", 1);
                    }
                    renderPasses.push_back(renderPass);
                }
            }
        }
        void RenderWindow::createSyncObjects() {
            imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
            renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
            inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
            imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

            VkSemaphoreCreateInfo semaphoreInfo{};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            VkFenceCreateInfo fenceInfo{};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                if (vkCreateSemaphore(vkDevice.getDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                    vkCreateSemaphore(vkDevice.getDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                    vkCreateFence(vkDevice.getDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {

                    throw core::Erreur(0, "échec de la création des objets de synchronisation pour une frame!", 1);
                }
            }
        }
        uint32_t RenderWindow::getCurrentFrame() {
            return currentFrame;
        }
        VkFormat RenderWindow::getSwapchainImageFormat() {
            return swapChainImageFormat;
        }
        std::vector<VkImage> RenderWindow::getSwapchainImages() {
            return swapChainImages;
        }
        std::vector<VkImageView> RenderWindow::getSwapChainImageViews() {
            return swapChainImageViews;
        }
        std::vector<VkFramebuffer> RenderWindow::getSwapchainFrameBuffers(unsigned int frameBufferId) {
            return swapChainFramebuffers[frameBufferId];
        }
        VkExtent2D RenderWindow::getSwapchainExtents() {
            return swapChainExtent;
        }
        VkSurfaceKHR RenderWindow::getSurface() {
            return surface;
        }
        const int RenderWindow::getMaxFramesInFlight() {
            return MAX_FRAMES_IN_FLIGHT;
        }
        VkRenderPass RenderWindow::getRenderPass(unsigned int renderPassId) {
            return renderPasses[renderPassId];
        }
        void RenderWindow::clear(const Color& color) {


            vkAcquireNextImageKHR(vkDevice.getDevice(), swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
            firstSubmit = true;
            firstDraw = true;
            registerClearCommands(color);
            for (unsigned int i = 0; i < 7; i++)
                vertexBuffer[i].clear();
            drawableData.clear();

                /*if (vkEndCommandBuffer(commandBuffers[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }*/
             //}
        }
        void RenderWindow::registerClearCommands(const Color& color) {
             clearColor = color;
             VkClearColorValue clearValue  = {clearColor.r / 255.f, clearColor.g / 255.f, clearColor.b / 255.f, clearColor.a / 255.f};
             VkClearDepthStencilValue clearDepthStencilValue = {
                .depth = 0.f,
                .stencil = 0
             };

             VkImageSubresourceRange imageRange = {
                 .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                 .baseMipLevel = 0,
                 .levelCount = 1,
                 .baseArrayLayer = 0,
                 .layerCount = 1
             };
             VkImageSubresourceRange imageRange2 = {
                 .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                 .baseMipLevel = 0,
                 .levelCount = 1,
                 .baseArrayLayer = 0,
                 .layerCount = 1
             };

             //for (unsigned int i = 0; i < getCommandBuffers().size(); i++) {
                beginRecordCommandBuffers();
                VkImageMemoryBarrier presentToClearBarrier {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .pNext = nullptr,
                    .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
                    .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                    .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image = getSwapchainImages()[imageIndex],
                    .subresourceRange = imageRange
                };
                VkImageMemoryBarrier clearToPresentBarrier {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .pNext = nullptr,
                    .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                    .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
                    .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image = getSwapchainImages()[imageIndex],
                    .subresourceRange = imageRange
                };
                vkCmdPipelineBarrier(getCommandBuffers()[currentFrame], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &presentToClearBarrier);
                vkCmdClearColorImage(getCommandBuffers()[currentFrame], getSwapchainImages()[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearValue, 1, &imageRange);
                vkCmdPipelineBarrier(getCommandBuffers()[currentFrame], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &clearToPresentBarrier);
                VkImageMemoryBarrier depthStencilToClearBarrier {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .pNext = nullptr,
                    .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
                    .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                    .oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                    .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image = getDepthTexture().getImage(),
                    .subresourceRange = imageRange2
                };
                VkImageMemoryBarrier clearToDepthStencilBarrier {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .pNext = nullptr,
                    .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                    .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
                    .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image = getDepthTexture().getImage(),
                    .subresourceRange = imageRange2
                };
                vkCmdPipelineBarrier(getCommandBuffers()[currentFrame], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &depthStencilToClearBarrier);
                vkCmdClearDepthStencilImage(getCommandBuffers()[currentFrame], getDepthTexture().getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearDepthStencilValue, 1, &imageRange2);
                vkCmdPipelineBarrier(getCommandBuffers()[currentFrame], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &clearToDepthStencilBarrier);
        }
        const uint32_t& RenderWindow::getImageIndex() {
            return imageIndex;
        }
        bool RenderWindow::isFirstSubmit() {
            return firstSubmit;
        }
        void RenderWindow::submit(bool lastSubmit, std::vector<VkSemaphore> signalSemaphores,
                        std::vector<VkSemaphore> waitSemaphores, std::vector<VkPipelineStageFlags> waitStages,
                        std::vector<uint64_t> signalValues,
                        std::vector<uint64_t> waitValues,
                        std::vector<VkFence> fences) {
            if (getCommandBuffers().size() > 0) {


                if (commandsOnRecordedState[currentFrame]) {
                    if (vkEndCommandBuffer(getCommandBuffers()[currentFrame]) != VK_SUCCESS) {
                        throw core::Erreur(0, "failed to record command buffer!", 1);
                    }
                }
                VkDeviceSize bufferSize = sizeof(DrawableData) * drawableData.size();
                if (bufferSize > 0) {
                    //std::cout<<"size models : "<<bufferSize<<std::endl;
                    void* data;
                    vkMapMemory(vkDevice.getDevice(), stagingDrawableDataMemory[getCurrentFrame()], 0, bufferSize, 0, &data);
                    memcpy(data, drawableData.data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), stagingDrawableDataMemory[getCurrentFrame()]);
                }
                for (unsigned int i = 0; i < 7; i++) {
                    if (vertexBuffer[i].getVertexCount() > 0)
                        vertexBuffer[i].updateStagingBuffers(currentFrame);
                }
                commandsOnRecordedState[currentFrame] = false;
                VkSubmitInfo submitInfo{};
                submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                if (firstSubmit) {
                    waitSemaphores.push_back(imageAvailableSemaphores[currentFrame]);
                    waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                    if (waitValues.size() > 0) {
                        ////////std::cout<<"wait semaphore : "<<semaphore[currentFrame]<<std::endl;
                        waitValues.push_back(0);
                    }
                }
                if (lastSubmit) {
                    signalSemaphores.push_back(renderFinishedSemaphores[currentFrame]);
                    if (signalValues.size() > 0) {
                        signalValues.push_back(0);
                    }
                }
                VkTimelineSemaphoreSubmitInfo timelineInfo{};
                if (signalValues.size() > 0 || waitValues.size() > 0) {
                    timelineInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
                    timelineInfo.waitSemaphoreValueCount=waitValues.size();
                    timelineInfo.pWaitSemaphoreValues = waitValues.data();
                    timelineInfo.signalSemaphoreValueCount=signalValues.size();
                    timelineInfo.pSignalSemaphoreValues = signalValues.data();
                    submitInfo.pNext = &timelineInfo;
                }
                submitInfo.signalSemaphoreCount = signalSemaphores.size();
                submitInfo.pSignalSemaphores = signalSemaphores.data();
                submitInfo.pWaitDstStageMask = waitStages.data();
                submitInfo.waitSemaphoreCount = waitSemaphores.size();
                submitInfo.pWaitSemaphores = waitSemaphores.data();
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &getCommandBuffers()[currentFrame];
                ////////std::cout<<"current frame : "<<currentFrame<<std::endl;
                /*VkSubmitInfo submitInfo{};
                submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

                std::vector<VkSemaphore> waitSemaphores;
                if (firstSubmit)
                    waitSemaphores.push_back(imageAvailableSemaphores[currentFrame]);
                if (semaphore.size() != 0) {
                    ////////std::cout<<"wait semaphore : "<<semaphore[currentFrame]<<std::endl;
                    waitSemaphores.push_back(semaphore[currentFrame]);
                }
                std::vector<VkPipelineStageFlags> waitStages;
                if (firstSubmit)
                    waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT); // pour imageAvailable
                if (semaphore.size() != 0) {
                    //////std::cout<<"wait frame : "<<currentFrame<<std::endl;
                    waitStages.push_back(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT); // pour la sync personnalisée
                }
                submitInfo.waitSemaphoreCount = waitSemaphores.size();
                submitInfo.pWaitSemaphores = waitSemaphores.data();
                submitInfo.pWaitDstStageMask = waitStages.data();
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &getCommandBuffers()[currentFrame];
                VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
                if (lastSubmit) {
                    submitInfo.signalSemaphoreCount = 1;
                    submitInfo.pSignalSemaphores = signalSemaphores;
                } else {
                    submitInfo.signalSemaphoreCount = 0;
                    submitInfo.pSignalSemaphores = nullptr;
                }*/
                firstSubmit = false;
                if (fences.size() > 0) {
                    vkWaitForFences(vkDevice.getDevice(), fences.size(), fences.data(), VK_TRUE, UINT64_MAX);
                    vkResetFences(vkDevice.getDevice(), fences.size(), fences.data());
                }
                if (vkQueueSubmit(vkDevice.getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "échec de l'envoi d'un command buffer!", 1);
                }
                vkWaitForFences(vkDevice.getDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
                vkResetFences(vkDevice.getDevice(), 1, &inFlightFences[currentFrame]);
                for (unsigned int i = 0; i < 7; i++)
                    vertexBuffer[i].clear();
                drawableData.clear();
            }
        }
        void RenderWindow::drawVulkanFrame() {
            if (getCommandBuffers().size() > 0) {
                //for (unsigned int i = 0; i < getCommandBuffers().size(); i++) {

                //}

                // Vérifier si une frame précédente est en train d'utiliser cette image (il y a une fence à attendre)
                VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
                VkPresentInfoKHR presentInfo{};
                presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

                presentInfo.waitSemaphoreCount = 1;
                presentInfo.pWaitSemaphores = signalSemaphores;
                VkSwapchainKHR swapChains[] = {swapChain};
                presentInfo.swapchainCount = 1;
                presentInfo.pSwapchains = swapChains;
                presentInfo.pImageIndices = &imageIndex;
                presentInfo.pResults = nullptr; // Optionnel
                vkQueuePresentKHR(vkDevice.getPresentQueue(), &presentInfo);
                currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
                //////std::cout<<"current frame : "<<currentFrame<<std::endl;
                //vkDeviceWaitIdle(vkDevice.getDevice());
            }
        }
        void RenderWindow::cleanupSwapchain() {
            for (unsigned int j = 0; j < swapChainFramebuffers.size(); j++) {
                for (size_t i = 0; i < swapChainFramebuffers[j].size(); i++) {
                    vkDestroyFramebuffer(vkDevice.getDevice(), swapChainFramebuffers[j][i], nullptr);
                }
            }
            for (size_t i = 0; i < swapChainImageViews.size(); i++) {
                vkDestroyImageView(vkDevice.getDevice(), swapChainImageViews[i], nullptr);
            }
            vkDestroySwapchainKHR(vkDevice.getDevice(), swapChain, nullptr);
        }
        void RenderWindow::recreateSwapchain() {
            vkDeviceWaitIdle(vkDevice.getDevice());

            cleanupSwapchain();

            createSwapChain();
            createImageViews();
            createFramebuffers();
        }
        ////////////////////////////////////////////////////////////
        RenderWindow::~RenderWindow()
        {
            //RenderTarget::cleanup();
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

            vkDevice.createInstance();
            createSurface();
            vkDevice.pickupPhysicalDevice(surface);
            vkDevice.createLogicalDevice(surface);
            createSwapChain();
            createImageViews();
            //if (depthTestEnabled || stencilTestEnabled)
                getDepthTexture().createDepthTexture(swapChainExtent.width, swapChainExtent.height);
            createRenderPass();
            createFramebuffers();
            RenderTarget::initialize();
            createSyncObjects();
            currentFrame = 0;
            imageIndex = 0;
        }


        ////////////////////////////////////////////////////////////
        void RenderWindow::onResize()
        {
            // Update the current view (recompute the viewport, which is stored in relative coordinates)

            setView(getView());
            recreateSwapchain();
        }
        void RenderWindow::createFramebuffers() {
            swapChainFramebuffers.resize(2);
            for (unsigned int j = 0; j < 2; j++) {
                swapChainFramebuffers[j].resize(swapChainImageViews.size());

                for (size_t i = 0; i < swapChainImageViews.size(); i++) {
                    if (j == 0) {
                        std::array<VkImageView, 1> attachments = {
                            swapChainImageViews[i]
                        };

                        VkFramebufferCreateInfo framebufferInfo{};
                        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                        framebufferInfo.renderPass = renderPasses[0];
                        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());;
                        framebufferInfo.pAttachments = attachments.data();
                        framebufferInfo.width = swapChainExtent.width;
                        framebufferInfo.height = swapChainExtent.height;
                        framebufferInfo.layers = 1;
                        if (vkCreateFramebuffer(vkDevice.getDevice(), &framebufferInfo, nullptr, &swapChainFramebuffers[j][i]) != VK_SUCCESS) {
                            throw core::Erreur(0, "failed to create framebuffer!", 1);
                        }
                    } else {
                        std::array<VkImageView, 2> attachments = {
                            swapChainImageViews[i],
                            getDepthTexture().getImageView()
                        };

                        VkFramebufferCreateInfo framebufferInfo{};
                        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                        framebufferInfo.renderPass = renderPasses[1];
                        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());;
                        framebufferInfo.pAttachments = attachments.data();
                        framebufferInfo.width = swapChainExtent.width;
                        framebufferInfo.height = swapChainExtent.height;
                        framebufferInfo.layers = 1;
                        if (vkCreateFramebuffer(vkDevice.getDevice(), &framebufferInfo, nullptr, &swapChainFramebuffers[j][i]) != VK_SUCCESS) {
                            throw core::Erreur(0, "failed to create framebuffer!", 1);
                        }
                    }
                }
            }
        }
        window::Device& RenderWindow::getDevice() {
            return vkDevice;
        }
        #else
        ////////////////////////////////////////////////////////////
        RenderWindow::RenderWindow()
        {
            // Nothing to do
        }

        ////////////////////////////////////////////////////////////
        RenderWindow::RenderWindow(window::VideoMode mode, const core::String& title, std::uint32_t style, const window::ContextSettings& settings)
        {
            // Don't call the base class constructor because it contains virtual function calls

            create(mode, title, style, settings);
        }




        ////////////////////////////////////////////////////////////
        RenderWindow::RenderWindow(window::WindowHandle handle, const window::ContextSettings& settings)
        {
            // Don't call the base class constructor because it contains virtual function calls
            create(handle, settings);
        }


        ////////////////////////////////////////////////////////////
        RenderWindow::~RenderWindow()
        {

        }


        ////////////////////////////////////////////////////////////
        bool RenderWindow::activate(bool active)
        {
            return setActive(active);
        }


        ////////////////////////////////////////////////////////////
        math::Vector2u RenderWindow::getSize() const
        {
            return Window::getSize();
        }

        ////////////////////////////////////////////////////////////
        Image RenderWindow::capture()
        {
            Image image;
            if (setActive())
            {
                int width = static_cast<int>(getSize().x());
                int height = static_cast<int>(getSize().y());

                // copy rows one by one and flip them (OpenGL's origin is bottom while ODFAEG's origin is top)
                std::vector<std::uint8_t> pixels(width * height * 4);
                for (int i = 0; i < height; ++i)
                {
                    std::uint8_t* ptr = &pixels[i * width * 4];
                    glCheck(glReadPixels(0, height - i - 1, width, 1, GL_RGBA, GL_UNSIGNED_BYTE, ptr));
                }

                image.create(width, height, &pixels[0]);
            }

            return image;
        }
        ////////////////////////////////////////////////////////////
        void RenderWindow::onCreate()
        {
            priv::ensureGlewInit();
            RenderTarget::setVersionMajor(getSettings().versionMajor);
            RenderTarget::setVersionMinor(getSettings().versionMinor);
            // Just initialize the render target part
            RenderTarget::initialize(0);
        }


        ////////////////////////////////////////////////////////////
        void RenderWindow::onResize()
        {
            // Update the current view (recompute the viewport, which is stored in relative coordinates)
            setView(getView());
        }
        #endif // VULKAN
    }

} // namespace sf
