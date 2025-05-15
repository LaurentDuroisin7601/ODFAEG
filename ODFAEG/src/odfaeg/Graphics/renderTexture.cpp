////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2013 Laurent Gomila (laurent.gom@gmail.com)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "../../../include/odfaeg/config.hpp"
#ifndef VULKAN
#include "GL/glew.h"
#endif
#include "../../../include/odfaeg/Graphics/renderTexture.h"
#ifndef VULKAN

#include <SFML/OpenGL.hpp>
#include "glCheck.h"
#include "renderTextureImplFBO.h"
#include "renderTextureImplDefault.h"
#endif

namespace odfaeg
{
    namespace graphic {
        #ifdef VULKAN
        RenderTexture::RenderTexture(window::Device& vkDevice) : RenderTarget(vkDevice), vkDevice(vkDevice), m_texture(vkDevice) {
        }
        bool RenderTexture::create(unsigned int width, unsigned int height) {
            vkDevice.createInstance();
            vkDevice.pickupPhysicalDevice(VK_NULL_HANDLE);
            vkDevice.createLogicalDevice(VK_NULL_HANDLE);
            m_texture.create(width, height);
            getDepthTexture().createDepthTexture(width, height);
            createRenderPass();
            createFramebuffers();
            createSyncObjects();
            m_size.x = width;
            m_size.y = height;
            RenderTarget::initialize();
            currentFrame = 0;
            return true;
        }
        bool RenderTexture::createCubeMap(unsigned int width, unsigned int height) {
            vkDevice.createInstance();
            vkDevice.pickupPhysicalDevice(VK_NULL_HANDLE);
            vkDevice.createLogicalDevice(VK_NULL_HANDLE);
            m_texture.createCubeMap(width, height);
            getDepthTexture().createDepthTexture(width, height);
            createRenderPass();
            createFramebuffers();
            createSyncObjects();
            m_size.x = width;
            m_size.y = height;
            RenderTarget::initialize();
            currentFrame = 0;
            return true;
        }
        void RenderTexture::createSyncObjects() {
            inFlightFences.resize(getMaxFramesInFlight());

            VkFenceCreateInfo fenceInfo{};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            VkSemaphoreCreateInfo semaphoreInfo{};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            for (size_t i = 0; i < getMaxFramesInFlight(); i++) {
                if (vkCreateFence(vkDevice.getDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {

                    throw core::Erreur(0, "échec de la création des objets de synchronisation pour une frame!", 1);
                }
            }
        }
        VkSurfaceKHR RenderTexture::getSurface() {
            return VK_NULL_HANDLE;
        }
        VkExtent2D RenderTexture::getSwapchainExtents() {
            VkExtent2D actualExtent = {
                static_cast<uint32_t>(m_texture.getSize().x),
                static_cast<uint32_t>(m_texture.getSize().y)
            };
            return actualExtent;
        }
        VkFormat RenderTexture::getSwapchainImageFormat() {
            return m_texture.getFormat();
        }
        std::vector<VkImage> RenderTexture::getSwapchainImages() {
            std::vector<VkImage> images;
            images.push_back(m_texture.getImage());
            return images;
        }
        uint32_t RenderTexture::getCurrentFrame() {
            return 0;
        }
        const int RenderTexture::getMaxFramesInFlight() {
            return 1;
        }
        const Texture& RenderTexture::getTexture() const {
            return m_texture;
        }
        sf::Vector2u RenderTexture::getSize() const {
            return m_size;
        }
        std::vector<VkFramebuffer> RenderTexture::getSwapchainFrameBuffers() {
            return swapChainFramebuffers;
        }
        VkRenderPass RenderTexture::getRenderPass() {
            return renderPass;
        }
        void RenderTexture::createFramebuffers() {
            swapChainFramebuffers.resize(getSwapchainImages().size());
            for (size_t i = 0; i < getSwapchainImages().size(); i++) {
                std::array<VkImageView, 2> attachments = {
                    m_texture.getImageView(),
                    getDepthTexture().getImageView()
                };

                VkFramebufferCreateInfo framebufferInfo{};
                framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferInfo.renderPass = renderPass;
                framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());;
                framebufferInfo.pAttachments = attachments.data();
                framebufferInfo.width = getSwapchainExtents().width;
                framebufferInfo.height = getSwapchainExtents().height;
                framebufferInfo.layers = 1;

                if (vkCreateFramebuffer(vkDevice.getDevice(), &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to create framebuffer!", 1);
                }
            }
        }
        void RenderTexture::createRenderPass() {
            VkAttachmentDescription colorAttachment{};
            colorAttachment.format =    getSwapchainImageFormat();
            colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkAttachmentDescription depthAttachment{};
            depthAttachment.format = getDepthTexture().findDepthFormat();
            depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
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
            if (vkCreateRenderPass(vkDevice.getDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to create render pass!", 1);
            }

        }
        void RenderTexture::clear(const sf::Color& color) {
             //std::cout<<"render texture clear begin command buffer"<<std::endl;
             clearColor = color;
             VkClearColorValue clearValue = {clearColor.r / 255.f, clearColor.g / 255.f, clearColor.b / 255.f, clearColor.a / 255.f};
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
             for (unsigned int i = 0; i < getCommandBuffers().size(); i++) {
                vkResetCommandBuffer(getCommandBuffers()[i], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                if (vkBeginCommandBuffer(getCommandBuffers()[i], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }
                VkImageMemoryBarrier presentToClearBarrier {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .pNext = nullptr,
                    .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
                    .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                    .oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image = getSwapchainImages()[i],
                    .subresourceRange = imageRange
                };
                VkImageMemoryBarrier clearToPresentBarrier {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .pNext = nullptr,
                    .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                    .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
                    .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image = getSwapchainImages()[i],
                    .subresourceRange = imageRange
                };
                vkCmdPipelineBarrier(getCommandBuffers()[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &presentToClearBarrier);
                vkCmdClearColorImage(getCommandBuffers()[i], getSwapchainImages()[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearValue, 1, &imageRange);
                vkCmdPipelineBarrier(getCommandBuffers()[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &clearToPresentBarrier);
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
                vkCmdPipelineBarrier(getCommandBuffers()[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &depthStencilToClearBarrier);
                vkCmdClearDepthStencilImage(getCommandBuffers()[i], getDepthTexture().getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearDepthStencilValue, 1, &imageRange2);
                vkCmdPipelineBarrier(getCommandBuffers()[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &clearToDepthStencilBarrier);
                /*if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }*/
             }
        }
        void RenderTexture::display() {
            if (getCommandBuffers().size() > 0) {
                //std::cout<<"render texture end command buffer"<<std::endl;

                for (unsigned int i = 0; i < getCommandBuffers().size(); i++) {
                    if (vkEndCommandBuffer(getCommandBuffers()[i]) != VK_SUCCESS) {
                        throw core::Erreur(0, "failed to record command buffer!", 1);
                    }
                }
                vkWaitForFences(vkDevice.getDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

                VkSubmitInfo submitInfo{};
                submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &getCommandBuffers()[getCurrentFrame()];

                vkResetFences(vkDevice.getDevice(), 1, &inFlightFences[currentFrame]);
                if (vkQueueSubmit(vkDevice.getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "échec de l'envoi d'un command buffer!", 1);
                }
                vkDeviceWaitIdle(vkDevice.getDevice());
            }
        }
        RenderTexture::~RenderTexture() {
            //RenderTarget::cleanup();
            std::cout<<"destroy render texture"<<std::endl;
            for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
                vkDestroyFramebuffer(vkDevice.getDevice(), swapChainFramebuffers[i], nullptr);
            }
            vkDestroyRenderPass(vkDevice.getDevice(), renderPass, nullptr);
            for (size_t i = 0; i < getMaxFramesInFlight(); i++) {
                vkDestroyFence(vkDevice.getDevice(), inFlightFences[i], nullptr);
            }
        }
        #else
        ////////////////////////////////////////////////////////////
        RenderTexture::RenderTexture() :
        m_impl(NULL),
        m_context(NULL)
        {

        }


        ////////////////////////////////////////////////////////////
        RenderTexture::~RenderTexture()
        {
            //delete m_context;
            if (m_impl)
                delete m_impl;
            if (m_context)
                delete m_context;
        }


        ////////////////////////////////////////////////////////////
        bool RenderTexture::create(unsigned int width, unsigned int height, window::ContextSettings settings, unsigned int textureType, bool useSeparateContext, unsigned int precision, unsigned int format, unsigned int type)
        {

            if (useSeparateContext) {
                m_context = new window::Context(settings, width, height);
                m_settings = m_context->getSettings();
            } else {
                m_settings = settings;
            }
            RenderTarget::setVersionMajor(m_settings.versionMajor);
            RenderTarget::setVersionMinor(m_settings.versionMinor);
            // Create the texture
            if (textureType == GL_TEXTURE_2D) {
                if(!m_texture.create(width, height))
                {
                    std::cerr<< "Impossible to create render texture (failed to create the target texture)" << std::endl;
                    return false;
                }
            } else if (textureType = GL_TEXTURE_CUBE_MAP) {
                if(!m_texture.createCubeMap(width, height))
                {
                    std::cerr<< "Impossible to create render texture (failed to create the target texture)" << std::endl;
                    return false;
                }
            }
            // We disable smoothing by default for render textures
            setSmooth(false);

            // Create the implementation
            delete m_impl;
            if (priv::RenderTextureImplFBO::isAvailable())
            {
                // Use frame-buffer object (FBO)
                m_impl = new priv::RenderTextureImplFBO;
            }
            else
            {
                std::cout<<"FBO not avalaible"<<std::endl;
                // Use default implementation
                m_impl = new priv::RenderTextureImplDefault;
            }

            // Initialize the render texture
            if (!m_impl->create(width, height, (m_context) ? m_context->getSettings() : settings, m_texture.m_texture))
                return false;
            // We can now initialize the render target part
            RenderTarget::initialize(m_impl->getFramebufferId());
            return true;
        }


        ////////////////////////////////////////////////////////////
        void RenderTexture::setSmooth(bool smooth)
        {
            m_texture.setSmooth(smooth);
        }


        ////////////////////////////////////////////////////////////
        bool RenderTexture::isSmooth() const
        {
            return m_texture.isSmooth();
        }


        ////////////////////////////////////////////////////////////
        void RenderTexture::setRepeated(bool repeated)
        {
            m_texture.setRepeated(repeated);
        }


        ////////////////////////////////////////////////////////////
        bool RenderTexture::isRepeated() const
        {
            return m_texture.isRepeated();
        }


        ////////////////////////////////////////////////////////////
        bool RenderTexture::setActive(bool active)
        {
            if (m_context)
                return m_impl && m_context->setActive(active);
            else
                return true;
        }


        ////////////////////////////////////////////////////////////
        void RenderTexture::display()
        {
            // Update the target texture
            if (setActive(true))
            {
                m_impl->updateTexture(m_texture.m_texture);
                m_texture.m_pixelsFlipped = true;
            }
        }


        ////////////////////////////////////////////////////////////
        sf::Vector2u RenderTexture::getSize() const
        {
            return m_texture.getSize();
        }


        ////////////////////////////////////////////////////////////
        const Texture& RenderTexture::getTexture() const
        {
            return m_texture;
        }
        const window::ContextSettings& RenderTexture::getSettings() const {
              return m_settings;
        }
        bool RenderTexture::activate(bool active) {
            return setActive(active);
        }
        void RenderTexture::bind() {
            if (m_impl)
                m_impl->bind();
        }
        void RenderTexture::setLinkedListIds(unsigned int atomicBuffer, unsigned int linkedListBuffer, unsigned int headPtrTex, unsigned int clearBuff) {
            m_atomicBuffer = atomicBuffer;
            m_linkedListBuffer = linkedListBuffer;
            m_headPtrTex = headPtrTex;
            m_clearBuff = clearBuff;
        }
        unsigned int RenderTexture::getAtomicBuffer() {
            return m_atomicBuffer;
        }
        unsigned int RenderTexture::getLinkedListBuffer() {
            return m_linkedListBuffer;
        }
        unsigned int RenderTexture::getHeadPtrTex() {
            return m_headPtrTex;
        }
        unsigned int RenderTexture::getClearBuff() {
            return m_clearBuff;
        }
        void RenderTexture::selectCubemapFace(int cubemapFace) {
            if(m_impl && m_texture.isCubemap())
                m_impl->selectCubemapFace(cubemapFace, m_texture.getNativeHandle());
        }
        #endif // VULKAN
    }
}
