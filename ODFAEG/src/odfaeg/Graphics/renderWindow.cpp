#include <SFML/OpenGL.hpp>
#include "../../../include/odfaeg/Graphics/renderWindow.h"
#ifndef VULKAN
#include "glCheck.h"
#endif


namespace odfaeg {
    namespace graphic {
        using namespace sf;
        #ifdef VULKAN
        ////////////////////////////////////////////////////////////
        RenderWindow::RenderWindow(VideoMode mode, const String& title, window::VkSettup& vkSettup, Uint32 style,  const window::ContextSettings& settings) : RenderTarget(vkSettup), vkSettup(vkSettup)
        {
            // Don't call the base class constructor because it contains virtual function calls
            create(mode, title, style, settings);
        }
         ////////////////////////////////////////////////////////////
        RenderWindow::RenderWindow(WindowHandle handle,  window::VkSettup& vkSettup, const window::ContextSettings& settings) : vkSettup(vkSettup), RenderTarget(vkSettup)
        {
            // Don't call the base class constructor because it contains virtual function calls
            create(handle, settings);
        }
        void RenderWindow::cleanup() {
            cleanupSwapchain();
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                vkDestroySemaphore(vkSettup.getDevice(), renderFinishedSemaphores[i], nullptr);
                vkDestroySemaphore(vkSettup.getDevice(), imageAvailableSemaphores[i], nullptr);
                vkDestroyFence(vkSettup.getDevice(), inFlightFences[i], nullptr);
            }
            vkDestroySurfaceKHR(vkSettup.getInstance(), surface, nullptr);
        }
        void RenderWindow::createSurface(GLFWwindow* window) {
             std::cout<<"create window surface."<<std::endl;
             if (glfwCreateWindowSurface(vkSettup.getInstance(), getWindow(), nullptr, &surface) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to create window surface!", 1);
             }
        }
        void RenderWindow::createSwapChain() {
            window::VkSettup::SwapChainSupportDetails swapChainSupport = vkSettup.querySwapChainSupport(vkSettup.getPhysicalDevice(), surface);

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
            createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            window::VkSettup::QueueFamilyIndices indices = vkSettup.findQueueFamilies(vkSettup.getPhysicalDevice(), surface);
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

            if (vkCreateSwapchainKHR(vkSettup.getDevice(), &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to create swap chain!", 1);
            }
            vkGetSwapchainImagesKHR(vkSettup.getDevice(), swapChain, &imageCount, nullptr);
            swapChainImages.resize(imageCount);
            vkGetSwapchainImagesKHR(vkSettup.getDevice(), swapChain, &imageCount, swapChainImages.data());

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
                glfwGetFramebufferSize(getWindow(), &width, &height);

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
            if (vkCreateImageView(vkSettup.getDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
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
        void RenderWindow::createSyncObjects() {
            imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
            renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
            inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
            imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

            VkSemaphoreCreateInfo semaphoreInfo{};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            VkFenceCreateInfo fenceInfo{};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                if (vkCreateSemaphore(vkSettup.getDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                    vkCreateSemaphore(vkSettup.getDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                    vkCreateFence(vkSettup.getDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {

                    throw core::Erreur(0, "échec de la création des objets de synchronisation pour une frame!", 1);
                }
            }
        }
        size_t RenderWindow::getCurrentFrame() {
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
        VkExtent2D RenderWindow::getSwapchainExtents() {
            return swapChainExtent;
        }
        VkSurfaceKHR RenderWindow::getSurface() {
            return surface;
        }
        const int RenderWindow::getMaxFramesInFlight() {
            return MAX_FRAMES_IN_FLIGHT;
        }
        void RenderWindow::drawVulkanFrame() {
            if (getCommandBuffers().size() > 0) {
                vkWaitForFences(vkSettup.getDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
                uint32_t imageIndex;
                vkAcquireNextImageKHR(vkSettup.getDevice(), swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
                // Vérifier si une frame précédente est en train d'utiliser cette image (il y a une fence à attendre)
                if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
                    vkWaitForFences(vkSettup.getDevice(), 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
                }
                 // Marque l'image comme étant à nouveau utilisée par cette frame
                imagesInFlight[imageIndex] = inFlightFences[currentFrame];

                VkSubmitInfo submitInfo{};
                submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

                VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
                VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
                submitInfo.waitSemaphoreCount = 1;
                submitInfo.pWaitSemaphores = waitSemaphores;
                submitInfo.pWaitDstStageMask = waitStages;
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &getCommandBuffers()[imageIndex];
                VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
                submitInfo.signalSemaphoreCount = 1;
                submitInfo.pSignalSemaphores = signalSemaphores;
                vkResetFences(vkSettup.getDevice(), 1, &inFlightFences[currentFrame]);
                if (vkQueueSubmit(vkSettup.getGraphicQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "échec de l'envoi d'un command buffer!", 1);
                }
                VkPresentInfoKHR presentInfo{};
                presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

                presentInfo.waitSemaphoreCount = 1;
                presentInfo.pWaitSemaphores = signalSemaphores;
                VkSwapchainKHR swapChains[] = {swapChain};
                presentInfo.swapchainCount = 1;
                presentInfo.pSwapchains = swapChains;
                presentInfo.pImageIndices = &imageIndex;
                presentInfo.pResults = nullptr; // Optionnel
                vkQueuePresentKHR(vkSettup.getPresentQueue(), &presentInfo);
                currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
                vkDeviceWaitIdle(vkSettup.getDevice());
            }
        }
        void RenderWindow::cleanupSwapchain() {
            for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
                vkDestroyFramebuffer(vkSettup.getDevice(), swapChainFramebuffers[i], nullptr);
            }
            for (size_t i = 0; i < swapChainImageViews.size(); i++) {
                vkDestroyImageView(vkSettup.getDevice(), swapChainImageViews[i], nullptr);
            }

            vkDestroySwapchainKHR(vkSettup.getDevice(), swapChain, nullptr);
        }
        void RenderWindow::recreateSwapchain() {
            vkDeviceWaitIdle(vkSettup.getDevice());

            cleanupSwapchain();

            createSwapChain();
            createImageViews();
            createFramebuffers();
        }
        ////////////////////////////////////////////////////////////
        RenderWindow::~RenderWindow()
        {
            RenderTarget::cleanup();
            cleanup();
        }
        ////////////////////////////////////////////////////////////
        Vector2u RenderWindow::getSize() const
        {
            return Window::getSize();
        }
        ////////////////////////////////////////////////////////////
        void RenderWindow::onCreate()
        {
            // Just initialize the render target part

            vkSettup.createInstance();
            vkSettup.setupDebugMessenger();
            createSurface(getWindow());
            vkSettup.pickupPhysicalDevice(surface);
            vkSettup.createLogicalDevice(surface);
            createSwapChain();
            createImageViews();
            RenderTarget::initialize();
            createFramebuffers();
            createSyncObjects();
            currentFrame = 0;
        }


        ////////////////////////////////////////////////////////////
        void RenderWindow::onResize()
        {
            // Update the current view (recompute the viewport, which is stored in relative coordinates)

            setView(getView());
            recreateSwapchain();
            vkDestroyRenderPass(vkSettup.getDevice(), renderPass, nullptr);
            createRenderPass();
        }
        void RenderWindow::createFramebuffers() {
            swapChainFramebuffers.resize(swapChainImageViews.size());

            for (size_t i = 0; i < swapChainImageViews.size(); i++) {
                VkImageView attachments[] = {
                    swapChainImageViews[i]
                };

                VkFramebufferCreateInfo framebufferInfo{};
                framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferInfo.renderPass = renderPass;
                framebufferInfo.attachmentCount = 1;
                framebufferInfo.pAttachments = attachments;
                framebufferInfo.width = swapChainExtent.width;
                framebufferInfo.height = swapChainExtent.height;
                framebufferInfo.layers = 1;

                if (vkCreateFramebuffer(vkSettup.getDevice(), &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to create framebuffer!", 1);
                }
            }
        }
        size_t RenderWindow::getcurrentFrame() {
            return currentFrame;
        }
        #else
        ////////////////////////////////////////////////////////////
        RenderWindow::RenderWindow()
        {
            // Nothing to do
        }

        ////////////////////////////////////////////////////////////
        RenderWindow::RenderWindow(VideoMode mode, const String& title, Uint32 style, const window::ContextSettings& settings)
        {
            // Don't call the base class constructor because it contains virtual function calls
            create(mode, title, style, settings);
        }




        ////////////////////////////////////////////////////////////
        RenderWindow::RenderWindow(WindowHandle handle, const window::ContextSettings& settings)
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
        Vector2u RenderWindow::getSize() const
        {
            return Window::getSize();
        }

        ////////////////////////////////////////////////////////////
        Image RenderWindow::capture()
        {
            Image image;
            if (setActive())
            {
                int width = static_cast<int>(getSize().x);
                int height = static_cast<int>(getSize().y);

                // copy rows one by one and flip them (OpenGL's origin is bottom while SFML's origin is top)
                std::vector<Uint8> pixels(width * height * 4);
                for (int i = 0; i < height; ++i)
                {
                    Uint8* ptr = &pixels[i * width * 4];
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


