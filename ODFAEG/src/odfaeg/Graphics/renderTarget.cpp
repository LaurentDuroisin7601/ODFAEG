#include "../../../include/odfaeg/Graphics/renderTarget.h"
#include "../../../include/odfaeg/Graphics/drawable.h"
//#include "../../../include/odfaeg/Graphics/glExtensions.hpp"
#ifndef VULKAN
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include "glCheck.h"
#include "GlDebug.hpp"
#endif
namespace
{
    #ifdef VULKAN

    #else
    // Convert an sf::BlendMode::Factor constant to the corresponding OpenGL constant.
    sf::Uint32 factorToGlConstant(sf::BlendMode::Factor blendFactor)
    {
        switch (blendFactor)
        {
            default:
            case sf::BlendMode::Zero:             return GL_ZERO;
            case sf::BlendMode::One:              return GL_ONE;
            case sf::BlendMode::SrcColor:         return GL_SRC_COLOR;
            case sf::BlendMode::OneMinusSrcColor: return GL_ONE_MINUS_SRC_COLOR;
            case sf::BlendMode::DstColor:         return GL_DST_COLOR;
            case sf::BlendMode::OneMinusDstColor: return GL_ONE_MINUS_DST_COLOR;
            case sf::BlendMode::SrcAlpha:         return GL_SRC_ALPHA;
            case sf::BlendMode::OneMinusSrcAlpha: return GL_ONE_MINUS_SRC_ALPHA;
            case sf::BlendMode::DstAlpha:         return GL_DST_ALPHA;
            case sf::BlendMode::OneMinusDstAlpha: return GL_ONE_MINUS_DST_ALPHA;
        }
    }


    // Convert an sf::BlendMode::BlendEquation constant to the corresponding OpenGL constant.
    sf::Uint32 equationToGlConstant(sf::BlendMode::Equation blendEquation)
    {
        switch (blendEquation)
        {
            default:
            case sf::BlendMode::Add:             return GL_FUNC_ADD_EXT;
            case sf::BlendMode::Subtract:        return GL_FUNC_SUBTRACT_EXT;
        }
    }
    #endif // VULKAN
}
namespace odfaeg {
    namespace graphic {
        using namespace sf;
        #ifdef VULKAN
        RenderTarget::RenderTarget(window::VkSettup& vkSettup) : vkSettup(vkSettup),
        m_defaultView(), m_view() {
        }
        RenderTarget::~RenderTarget() {
            cleanup();
            vkDestroyRenderPass(vkSettup.getDevice(), renderPass, nullptr);
        }
        void RenderTarget::initialize() {
            m_defaultView = View (static_cast<float>(getSize().x), static_cast<float>(getSize().y), -static_cast<float>(getSize().y) - 200, static_cast<float>(getSize().y)+200);
            m_defaultView.reset(physic::BoundingBox(0, 0, -static_cast<float>(getSize().y) - 200,static_cast<float>(getSize().x), static_cast<float>(getSize().y),static_cast<float>(getSize().y)+200));
            m_view = m_defaultView;
            const std::string defaultVertexShader = R"(#version 450
                                                        vec2 positions[3] = vec2[](
                                                            vec2(0.0, -0.5),
                                                            vec2(0.5, 0.5),
                                                            vec2(-0.5, 0.5)
                                                        );
                                                        void main() {
                                                            gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
                                                        })";
            const std::string defaultFragmentShader = R"(#version 450
                                                          #extension GL_ARB_separate_shader_objects : enable

                                                          layout(location = 0) out vec4 outColor;

                                                          void main() {
                                                              outColor = vec4(1.0, 0.0, 0.0, 1.0);
                                                          })";
             defaultShader.setVkSettup(&vkSettup);
             if (!defaultShader.loadFromMemory(defaultVertexShader, defaultFragmentShader)) {
                  throw core::Erreur (0, "Failed to load default shader", 1);
             }
        }
        void RenderTarget::clear(const sf::Color& color) {
            cleanup();
            clearColor = color;
        }
        void RenderTarget::clearDepth() {
        }
        void RenderTarget::setView(View view)
        {
            m_view = view;
        }
        View& RenderTarget::getView() {
            return m_view;
        }
        View& RenderTarget::getDefaultView() {
            return m_defaultView;
        }
         math::Vec3f RenderTarget::mapPixelToCoords(const math::Vec3f& point)
        {
            return mapPixelToCoords(point, getView());
        }


        math::Vec3f RenderTarget::mapPixelToCoords(const math::Vec3f& point, View& view)
        {
            ViewportMatrix vpm;
            vpm.setViewport(math::Vec3f(view.getViewport().getPosition().x, view.getViewport().getPosition().y, 0)
                                        ,math::Vec3f(view.getViewport().getWidth(), view.getViewport().getHeight(), 1));
            math::Vec3f coords = vpm.toNormalizedCoordinates(point);
            coords = view.getProjMatrix().unProject(coords);
            coords = coords.normalizeToVec3();
            coords = view.getViewMatrix().inverseTransform(coords);
            return coords;
        }

        math::Vec3f RenderTarget::mapCoordsToPixel(const math::Vec3f& point)
        {
            return mapCoordsToPixel(point, getView());
        }


        math::Vec3f RenderTarget::mapCoordsToPixel(const math::Vec3f& point, View& view) {
            ViewportMatrix vpm;
            vpm.setViewport(math::Vec3f(view.getViewport().getPosition().x, view.getViewport().getPosition().y, 0),
            math::Vec3f(view.getViewport().getWidth(), view.getViewport().getHeight(), 1));
            math::Vec3f coords = view.getViewMatrix().transform(point);
            coords = view.getProjMatrix().project(coords);
            coords = coords.normalizeToVec3();
            coords = vpm.toViewportCoordinates(coords);
            return coords;
        }
        void RenderTarget::draw(Drawable& drawable, RenderStates states)
        {
            drawable.draw(*this, states);
        }
        void RenderTarget::draw(const Vertex* vertices, unsigned int vertexCount, sf::PrimitiveType type,
                      RenderStates states) {
             createGraphicPipeline(vertices, vertexCount, type, states);
             createCommandPool();
             createCommandBuffers();
        }
        void RenderTarget::createRenderPass() {
            VkAttachmentDescription colorAttachment{};
            colorAttachment.format =    vkSettup.getSwapchainImageFormat();
            colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            VkAttachmentReference colorAttachmentRef{};
            colorAttachmentRef.attachment = 0;
            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &colorAttachmentRef;

            VkRenderPassCreateInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount = 1;
            renderPassInfo.pAttachments = &colorAttachment;
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &subpass;
            VkSubpassDependency dependency{};
            dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            dependency.dstSubpass = 0;
            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.srcAccessMask = 0;
            dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            renderPassInfo.dependencyCount = 1;
            renderPassInfo.pDependencies = &dependency;
            if (vkCreateRenderPass(vkSettup.getDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to create render pass!", 1);
            }

        }
        void RenderTarget::createGraphicPipeline(const Vertex* vertices, unsigned int vertexCount, sf::PrimitiveType type,
                      RenderStates states = RenderStates::Default) {
            defaultShader.createShaderModules();
            VkShaderModule vertShaderModule = defaultShader.getVertexShaderModule();
            VkShaderModule fragShaderModule = defaultShader.getFragmentShaderModule();
            VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
            vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vertShaderStageInfo.module = vertShaderModule;
            vertShaderStageInfo.pName = "main";

            VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
            fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragShaderStageInfo.module = fragShaderModule;
            fragShaderStageInfo.pName = "main";

            VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

            VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
            vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputInfo.vertexBindingDescriptionCount = 0;
            vertexInputInfo.vertexAttributeDescriptionCount = 0;

            VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
            inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            inputAssembly.primitiveRestartEnable = VK_FALSE;

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = vkSettup.getSwapchainExtends().width;
            viewport.height = vkSettup.getSwapchainExtends().height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = vkSettup.getSwapchainExtends();

            VkPipelineViewportStateCreateInfo viewportState{};
            viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.viewportCount = 1;
            viewportState.pViewports = &viewport;
            viewportState.scissorCount = 1;
            viewportState.pScissors = &scissor;

            VkPipelineRasterizationStateCreateInfo rasterizer{};
            rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer.depthClampEnable = VK_FALSE;
            rasterizer.rasterizerDiscardEnable = VK_FALSE;
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer.lineWidth = 1.0f;
            rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
            rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
            rasterizer.depthBiasEnable = VK_FALSE;

            VkPipelineMultisampleStateCreateInfo multisampling{};
            multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.sampleShadingEnable = VK_FALSE;
            multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineColorBlendAttachmentState colorBlendAttachment{};
            colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            colorBlendAttachment.blendEnable = VK_FALSE;

            VkPipelineColorBlendStateCreateInfo colorBlending{};
            colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlending.logicOpEnable = VK_FALSE;
            colorBlending.logicOp = VK_LOGIC_OP_COPY;
            colorBlending.attachmentCount = 1;
            colorBlending.pAttachments = &colorBlendAttachment;
            colorBlending.blendConstants[0] = 0.0f;
            colorBlending.blendConstants[1] = 0.0f;
            colorBlending.blendConstants[2] = 0.0f;
            colorBlending.blendConstants[3] = 0.0f;

            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = 0;
            pipelineLayoutInfo.pushConstantRangeCount = 0;

            if (vkCreatePipelineLayout(vkSettup.getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to create pipeline layout!", 1);
            }
            VkGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount = 2;
            pipelineInfo.pStages = shaderStages;
            pipelineInfo.pVertexInputState = &vertexInputInfo;
            pipelineInfo.pInputAssemblyState = &inputAssembly;
            pipelineInfo.pViewportState = &viewportState;
            pipelineInfo.pRasterizationState = &rasterizer;
            pipelineInfo.pMultisampleState = &multisampling;
            pipelineInfo.pColorBlendState = &colorBlending;
            pipelineInfo.layout = pipelineLayout;
            pipelineInfo.renderPass = renderPass;
            pipelineInfo.subpass = 0;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

            if (vkCreateGraphicsPipelines(vkSettup.getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to create graphics pipeline!", 1);
            }
            defaultShader.cleanupShaderModules();
        }
        void RenderTarget::createCommandPool() {
            window::VkSettup::QueueFamilyIndices queueFamilyIndices = vkSettup.findQueueFamilies(vkSettup.getPhysicalDevice());

            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
            poolInfo.flags = 0; // Optionel
            if (vkCreateCommandPool(vkSettup.getDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
                throw core::Erreur(0, "échec de la création d'une command pool!", 1);
            }
        }
        void RenderTarget::createCommandBuffers() {
            commandBuffers.resize(swapChainFramebuffers.size());

            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = commandPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

            if (vkAllocateCommandBuffers(vkSettup.getDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to allocate command buffers!", 1);
            }

            for (size_t i = 0; i < commandBuffers.size(); i++) {
                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

                if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }

                VkRenderPassBeginInfo renderPassInfo{};
                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                renderPassInfo.renderPass = renderPass;
                renderPassInfo.framebuffer = swapChainFramebuffers[i];
                renderPassInfo.renderArea.offset = {0, 0};
                renderPassInfo.renderArea.extent = vkSettup.getSwapchainExtends();

                VkClearValue clrColor = {clearColor.r / 255.f,clearColor.g / 255.f, clearColor.b / 255.f, clearColor.a / 255.f};
                renderPassInfo.clearValueCount = 1;
                renderPassInfo.pClearValues = &clrColor;

                vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

                    vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

                    vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

                vkCmdEndRenderPass(commandBuffers[i]);

                if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }

            }
            vkSettup.setCommandBuffers(commandBuffers);
        }
        void RenderTarget::cleanup() {
            vkDestroyCommandPool(vkSettup.getDevice(), commandPool, nullptr);
            vkDestroyPipeline(vkSettup.getDevice(), graphicsPipeline, nullptr);
            vkDestroyPipelineLayout(vkSettup.getDevice(), pipelineLayout, nullptr);
        }
        #else
        ////////////////////////////////////////////////////////////
        RenderTarget::RenderTarget() :
        m_defaultView(), m_view(), m_cache()
        {
            m_cache.glStatesSet = false;
            m_vao = m_versionMajor = m_versionMinor = 0;
            enableAlphaTest = true;
            enableCubeMap = false;
            m_cache.vboPointerSets = false;
        }
        void RenderTarget::setVersionMajor (unsigned int versionMajor) {
            m_versionMajor = versionMajor;
        }
        void RenderTarget::setVersionMinor (unsigned int versionMinor) {
            m_versionMinor = versionMinor;
        }
        unsigned int RenderTarget::getVersionMajor() {
            return m_versionMajor;
        }
        unsigned int RenderTarget::getVersionMinor() {
            return m_versionMinor;
        }
        ////////////////////////////////////////////////////////////
        RenderTarget::~RenderTarget()
        {
            if (m_versionMajor > 3 || m_versionMajor == 3 && m_versionMinor >= 3 && m_vao) {
                glCheck(glDeleteVertexArrays(1, &m_vao));
            }
        }


        ////////////////////////////////////////////////////////////
        void RenderTarget::clear(const Color& color)
        {
            if (activate(true))
            {
                applyTexture(nullptr);
                glCheck(glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferId));
                glCheck(glClearColor(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f));
                glCheck(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
            }
        }

        void RenderTarget::clearDepth() {
            glCheck(glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferId));
            glCheck(glClear(GL_DEPTH_BUFFER_BIT));
        }        ////////////////////////////////////////////////////////////
        void RenderTarget::setView(View view)
        {
            m_view = view;
            m_cache.viewChanged = true;
        }


        ////////////////////////////////////////////////////////////
        View& RenderTarget::getView()
        {
            return m_view;
        }


        ////////////////////////////////////////////////////////////
        View& RenderTarget::getDefaultView()
        {
            return m_defaultView;
        }
        math::Vec3f RenderTarget::mapPixelToCoords(const math::Vec3f& point)
        {
            return mapPixelToCoords(point, getView());
        }


        math::Vec3f RenderTarget::mapPixelToCoords(const math::Vec3f& point, View& view)
        {
            ViewportMatrix vpm;
            vpm.setViewport(math::Vec3f(view.getViewport().getPosition().x, view.getViewport().getPosition().y, 0)
                                        ,math::Vec3f(view.getViewport().getWidth(), view.getViewport().getHeight(), 1));
            math::Vec3f coords = vpm.toNormalizedCoordinates(point);
            coords = view.getProjMatrix().unProject(coords);
            coords = coords.normalizeToVec3();
            coords = view.getViewMatrix().inverseTransform(coords);
            return coords;
        }

        math::Vec3f RenderTarget::mapCoordsToPixel(const math::Vec3f& point)
        {
            return mapCoordsToPixel(point, getView());
        }


        math::Vec3f RenderTarget::mapCoordsToPixel(const math::Vec3f& point, View& view) {
            ViewportMatrix vpm;
            vpm.setViewport(math::Vec3f(view.getViewport().getPosition().x, view.getViewport().getPosition().y, 0),
            math::Vec3f(view.getViewport().getWidth(), view.getViewport().getHeight(), 1));
            math::Vec3f coords = view.getViewMatrix().transform(point);
            coords = view.getProjMatrix().project(coords);
            if (coords.w == 0) {
                coords.w = view.getSize().z * 0.5;
            }
            coords = coords.normalizeToVec3();
            coords = vpm.toViewportCoordinates(coords);
            return coords;
        }
        void RenderTarget::drawInstanced(VertexBuffer& vertexBuffer, enum sf::PrimitiveType type, unsigned int start, unsigned int nb, unsigned int nbInstances, RenderStates states, unsigned int vboMatrix1, unsigned int vboMatrix2) {
            if (vertexBuffer.getVertexCount() == 0) {
                return;
            }

            if (activate(true))
            {
                if (!m_cache.glStatesSet)
                    resetGLStates();
                // Apply the view
                if (m_cache.viewChanged)
                    applyCurrentView();

                if (states.blendMode != m_cache.lastBlendMode)
                    applyBlendMode(states.blendMode);

                // Apply the texture
                sf::Uint64 textureId = states.texture ? states.texture->getNativeHandle() : 0;
                if (textureId != m_cache.lastTextureId)
                    applyTexture(states.texture);
                // Apply the shader
                if (states.shader)
                    applyShader(states.shader);
                if (m_versionMajor > 3 || m_versionMajor == 3 && m_versionMinor >= 3)
                    glCheck(glBindVertexArray(m_vao));
                if (m_cache.lastVboBuffer != &vertexBuffer) {
                    if (m_versionMajor > 3 || m_versionMajor == 3 && m_versionMinor >= 3) {
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboVertexBuffer));
                        glCheck(glEnableVertexAttribArray(0));
                        glCheck(glEnableVertexAttribArray(1));
                        glCheck(glEnableVertexAttribArray(2));
                        glCheck(glVertexAttribPointer(0, 3,GL_FLOAT,GL_FALSE,sizeof(Vertex), (GLvoid*) 0));
                        glCheck(glVertexAttribPointer(1, 4,GL_UNSIGNED_BYTE,GL_TRUE,sizeof(Vertex),(GLvoid*) 12));
                        glCheck(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) 16));
                        glCheck(glEnableVertexAttribArray(3));
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboNormalBuffer));
                        glCheck(glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(sf::Vector3f), (GLvoid*) 0));
                        glCheck(glDisableVertexAttribArray(0));
                        glCheck(glDisableVertexAttribArray(1));
                        glCheck(glDisableVertexAttribArray(2));
                        glCheck(glDisableVertexAttribArray(3));
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));

                        /*va_list args;
                        va_start(args, n);
                        for (unsigned int i = 0; i < n; i++) {
                            unsigned int vboMatrices = va_arg(args, unsigned int);
                            for (unsigned int j = 0; j < 4; j++) {
                                glCheck(glEnableVertexAttribArray(i * 4 + j + 3));
                                glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboMatrices));
                                glCheck(glVertexAttribPointer(i * 4 + j + 3, 4, GL_FLOAT, GL_FALSE, sizeof(math::Matrix4f),
                                                        (const GLvoid*)(sizeof(GLfloat) * i * 4)));
                                glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                                glCheck(glVertexAttribDivisor(i * 4 + j + 3, 1));
                                glCheck(glDisableVertexAttribArray(i * 4 + j + 3));
                            }
                        }
                        va_end(args);*/
                        if (vboMatrix1 != 0) {
                            for (unsigned int i = 0; i < 4; i++) {
                                glCheck(glEnableVertexAttribArray(i + 4));
                                glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboMatrix1));
                                glCheck(glVertexAttribPointer(i + 4, 4, GL_FLOAT, GL_FALSE, sizeof(math::Matrix4f),
                                                        (const GLvoid*)(sizeof(GLfloat) * i * 4)));
                                glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                                glCheck(glVertexAttribDivisor(i + 4, 1));
                                glCheck(glDisableVertexAttribArray(i + 4));
                            }
                        }
                        if (vboMatrix2 != 0) {
                            for (unsigned int i = 0; i < 4; i++) {
                                glCheck(glEnableVertexAttribArray(i + 8));
                                glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboMatrix2));
                                glCheck(glVertexAttribPointer(i + 8, 4, GL_FLOAT, GL_FALSE, sizeof(math::Matrix4f),
                                                        (const GLvoid*)(sizeof(GLfloat) * i * 4)));
                                glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                                glCheck(glVertexAttribDivisor(i + 8, 1));
                                glCheck(glDisableVertexAttribArray(i + 8));
                            }
                        }
                        glCheck(glEnableVertexAttribArray(12));
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboTextureIndexesBuffer));
                        glCheck(glVertexAttribIPointer(12, 1, GL_UNSIGNED_INT, sizeof(GLuint), (GLvoid*) 0));
                        glCheck(glDisableVertexAttribArray(12));
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                    }
                    m_cache.lastVboBuffer = &vertexBuffer;

                }
                if (m_versionMajor > 3 || m_versionMajor == 3 && m_versionMinor >= 3) {
                    glCheck(glEnableVertexAttribArray(0));
                    glCheck(glEnableVertexAttribArray(1));
                    glCheck(glEnableVertexAttribArray(2));
                    glCheck(glEnableVertexAttribArray(3));
                    if (vboMatrix1 != 0) {
                        for (unsigned int i = 0; i < 4 ; i++) {
                            glCheck(glEnableVertexAttribArray(4 + i));
                        }
                    }
                    if (vboMatrix2 != 0) {
                        for (unsigned int i = 0; i < 4 ; i++) {
                            glCheck(glEnableVertexAttribArray(8 + i));
                        }
                    }
                    glCheck(glEnableVertexAttribArray(12));
                    static const GLenum modes[] = {GL_POINTS, GL_LINES, GL_LINE_STRIP, GL_TRIANGLES,
                                                       GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_QUADS};
                    GLenum mode = modes[type];
                    glCheck(glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferId));
                    glCheck(glDrawArraysInstanced(mode,start,nb,nbInstances));
                    glCheck(glDisableVertexAttribArray(0));
                    glCheck(glDisableVertexAttribArray(1));
                    glCheck(glDisableVertexAttribArray(2));
                    glCheck(glDisableVertexAttribArray(3));
                    if (vboMatrix1 != 0) {
                        for (unsigned int i = 0; i < 4 ; i++) {
                            glCheck(glDisableVertexAttribArray(4 + i));
                        }
                    }
                    if (vboMatrix2 != 0) {
                        for (unsigned int i = 0; i < 4 ; i++) {
                            glCheck(glDisableVertexAttribArray(8 + i));
                        }
                    }
                    glCheck(glDisableVertexAttribArray(12));
                    glCheck(glBindVertexArray(0));
                }
            }
            applyTexture(nullptr);
            applyShader(nullptr);
        } //////////////////////////////////////////////////////////
        void RenderTarget::draw(Drawable& drawable, RenderStates states)
        {
            drawable.draw(*this, states);
        }


        ////////////////////////////////////////////////////////////
        void RenderTarget::draw(const Vertex* vertices, unsigned int vertexCount,
                                PrimitiveType type, RenderStates states)
        {
            // Nothing to draw?
            if (!vertices || (vertexCount == 0))
                return;

            if (activate(true))
            {

                if (!m_cache.glStatesSet)
                    resetGLStates();
                // Apply the view
                if (m_cache.viewChanged)
                    applyCurrentView();

                if (states.blendMode != m_cache.lastBlendMode)
                    applyBlendMode(states.blendMode);

                // Apply the texture
                sf::Uint64 textureId = states.texture ? states.texture->getNativeHandle() : 0;
                if (textureId != m_cache.lastTextureId)
                    applyTexture(states.texture);
                // Apply the shader
                if (states.shader)
                    applyShader(states.shader);
                bool useVertexCache = (vertexCount <= StatesCache::VertexCacheSize);
                if (useVertexCache)
                {

                    // Pre-transform the vertices and store them into the vertex cache
                    for (unsigned int i = 0; i < vertexCount; ++i)
                    {

                        Vertex& vertex = m_cache.vertexCache[i];
                        math::Vec3f pos (vertices[i].position.x, vertices[i].position.y, vertices[i].position.z);
                        math::Vec3f finalpos = states.transform.transform(pos);

                        vertex.position = sf::Vector3f(finalpos.x, finalpos.y, finalpos.z);
                        vertex.color = vertices[i].color;
                        vertex.texCoords = vertices[i].texCoords;
                    }
                    // Since vertices are transformed, we must use an identity transform to render them
                    states.transform.reset3D();
                    applyTransform(states.transform);
                }
                else
                {
                    TransformMatrix tm = states.transform;
                    applyTransform(tm);
                }
                if (useVertexCache)
                {
                    // ... and if we already used it previously, we don't need to set the pointers again
                    if (!m_cache.useVertexCache)
                        vertices = m_cache.vertexCache;
                    else
                        vertices = nullptr;
                }
                if (vertices) {
                    const char* data = reinterpret_cast<const char*>(vertices);
                    glEnableClientState(GL_COLOR_ARRAY);
                    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                    glEnableClientState(GL_VERTEX_ARRAY);
                    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), data + 0 );
                    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), data + 12);
                    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), data + 16);
                    glDisableClientState(GL_COLOR_ARRAY);
                    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                    glDisableClientState(GL_VERTEX_ARRAY);
                }
                glEnableClientState(GL_COLOR_ARRAY);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glEnableClientState(GL_VERTEX_ARRAY);
                // Find the OpenGL primitive type
                static const GLenum modes[] = {GL_POINTS, GL_LINES, GL_LINE_STRIP, GL_TRIANGLES,
                                                   GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_QUADS};
                GLenum mode = modes[type];
                // Draw the primitives
                //std::cout<<"frame buffer id : "<<m_framebufferId<<std::endl;
                glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferId);
                glDrawArrays(mode, 0, vertexCount);
                glDisableClientState(GL_COLOR_ARRAY);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                glDisableClientState(GL_VERTEX_ARRAY);
                m_cache.useVertexCache = useVertexCache;
                applyTexture(nullptr);
                applyShader(nullptr);
            }
        }
        void RenderTarget::drawVertexBuffer(VertexBuffer& vertexBuffer, RenderStates states) {
            if (vertexBuffer.getVertexCount() == 0) {
                return;
            }

            if (activate(true))
            {

                if (!m_cache.glStatesSet)
                    resetGLStates();
                // Apply the view
                if (m_cache.viewChanged)
                    applyCurrentView();

                if (states.blendMode != m_cache.lastBlendMode)
                    applyBlendMode(states.blendMode);

                // Apply the texture
                sf::Uint64 textureId = states.texture ? states.texture->getNativeHandle() : 0;
                if (textureId != m_cache.lastTextureId)
                    applyTexture(states.texture);
                // Apply the shader
                if (states.shader)
                    applyShader(states.shader);
                if (m_versionMajor > 3 || m_versionMajor == 3 && m_versionMinor >= 3)
                    glCheck(glBindVertexArray(m_vao));
                if (m_cache.lastVboBuffer != &vertexBuffer) {
                    if (m_versionMajor > 3 || m_versionMajor == 3 && m_versionMinor >= 3) {
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboVertexBuffer));
                        glCheck(glEnableVertexAttribArray(0));
                        glCheck(glEnableVertexAttribArray(1));
                        glCheck(glEnableVertexAttribArray(2));
                        glCheck(glVertexAttribPointer(0, 3,GL_FLOAT,GL_FALSE,sizeof(Vertex), (GLvoid*) 0));
                        glCheck(glVertexAttribPointer(1, 4,GL_UNSIGNED_BYTE,GL_TRUE,sizeof(Vertex),(GLvoid*) 12));
                        glCheck(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) 16));
                        glCheck(glEnableVertexAttribArray(3));
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboNormalBuffer));
                        glCheck(glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(sf::Vector3f), (GLvoid*) 0));
                        glCheck(glEnableVertexAttribArray(4));
                        //std::cout<<"vbo texture index buffer : "<<vertexBuffer.vboTextureIndexesBuffer<<std::endl;
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboTextureIndexesBuffer));

                        glCheck(glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT/*,GL_FALSE*/,sizeof(GLuint),(GLvoid*) 0));
                        glCheck(glDisableVertexAttribArray(0));
                        glCheck(glDisableVertexAttribArray(1));
                        glCheck(glDisableVertexAttribArray(2));
                        glCheck(glDisableVertexAttribArray(3));
                        glCheck(glDisableVertexAttribArray(4));

                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                    } else {
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboVertexBuffer));
                        glCheck(glEnableClientState(GL_COLOR_ARRAY));
                        glCheck(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
                        glCheck(glEnableClientState(GL_VERTEX_ARRAY));
                        glCheck(glVertexPointer(3, GL_FLOAT, sizeof(Vertex), (GLvoid*) 0 ));
                        glCheck(glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), (GLvoid*) 12));
                        glCheck(glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex),(GLvoid*) 16));
                        glCheck(glEnableClientState(GL_NORMAL_ARRAY));
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboNormalBuffer));
                        glCheck(glNormalPointer(GL_FLOAT, sizeof(sf::Vector3f), (GLvoid*) 0));
                        glCheck(glDisableClientState(GL_COLOR_ARRAY));
                        glCheck(glDisableClientState(GL_TEXTURE_COORD_ARRAY));
                        glCheck(glDisableClientState(GL_VERTEX_ARRAY));
                        glCheck(glDisableClientState(GL_NORMAL_ARRAY));
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                    }
                    m_cache.lastVboBuffer = &vertexBuffer;
                }
                if (m_versionMajor > 3 || m_versionMajor == 3 && m_versionMinor >= 3) {
                    glCheck(glEnableVertexAttribArray(0));
                    glCheck(glEnableVertexAttribArray(1));
                    glCheck(glEnableVertexAttribArray(2));
                    glCheck(glEnableVertexAttribArray(3));
                    glCheck(glEnableVertexAttribArray(4));
                } else {
                    glCheck(glEnableClientState(GL_COLOR_ARRAY));
                    glCheck(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
                    glCheck(glEnableClientState(GL_VERTEX_ARRAY));
                    glCheck(glEnableClientState(GL_NORMAL_ARRAY));
                }

                // Find the OpenGL primitive type
                static const GLenum modes[] = {GL_POINTS, GL_LINES, GL_LINE_STRIP, GL_TRIANGLES,
                                                   GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_QUADS};
                GLenum mode = modes[vertexBuffer.getPrimitiveType()];
                if (vertexBuffer.m_indexes.size() > 0) {
                    glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer.vboIndexBuffer));
                    glCheck(glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferId));
                    glCheck(glDrawElements(mode, vertexBuffer.m_indexes.size(), GL_UNSIGNED_INT, (GLvoid*) 0));
                    glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
                } else {
                    //std::cout<<"draw arrays"<<std::endl;
                    //std::cout<<"frame buffer id : "<<m_framebufferId<<std::endl;
                    glCheck(glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferId));
                    glCheck(glDrawArrays(mode, 0, vertexBuffer.getVertexCount()));
                }
                if (m_versionMajor > 3 || m_versionMajor == 3 && m_versionMinor >= 3) {
                    glCheck(glDisableVertexAttribArray(0));
                    glCheck(glDisableVertexAttribArray(1));
                    glCheck(glDisableVertexAttribArray(2));
                    glCheck(glDisableVertexAttribArray(3));
                    glCheck(glDisableVertexAttribArray(4));
                    glCheck(glBindVertexArray(0));
                } else {
                    glCheck(glDisableClientState(GL_COLOR_ARRAY));
                    glCheck(glDisableClientState(GL_TEXTURE_COORD_ARRAY));
                    glCheck(glDisableClientState(GL_VERTEX_ARRAY));
                    glCheck(glDisableClientState(GL_NORMAL_ARRAY));
                }
            }
            applyTexture(nullptr);
            applyShader(nullptr);
        }
        ////////////////////////////////////////////////////////////
        void RenderTarget::pushGLStates()
        {
            if (activate(true))
            {
                glCheck(glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS));
                glCheck(glPushAttrib(GL_ALL_ATTRIB_BITS));
                glCheck(glMatrixMode(GL_MODELVIEW));
                glCheck(glPushMatrix());
                glCheck(glMatrixMode(GL_PROJECTION));
                glCheck(glPushMatrix());
                glCheck(glMatrixMode(GL_TEXTURE));
                glCheck(glPushMatrix());
            }

            resetGLStates();
        }


        ////////////////////////////////////////////////////////////
        void RenderTarget::popGLStates()
        {
            if (activate(true))
            {
                glCheck(glMatrixMode(GL_PROJECTION));
                glCheck(glPopMatrix());
                glCheck(glMatrixMode(GL_MODELVIEW));
                glCheck(glPopMatrix());
                glCheck(glMatrixMode(GL_TEXTURE));
                glCheck(glPopMatrix());
                glCheck(glPopClientAttrib());
                glCheck(glPopAttrib());
            }
        }
        void RenderTarget::setAlphaTestEnable(bool enabled) {
            enableAlphaTest = enabled;

        }
        ////////////////////////////////////////////////////////////
        void RenderTarget::resetGLStates()
        {
            if (activate(true))
            {

                #ifdef ODFAEG_DEBUG
                // make sure that the user didn't leave an unchecked OpenGL error
                GLenum error = glGetError();
                if (error != GL_NO_ERROR)
                {
                    err() << "OpenGL error (" << error << ") detected in user code, "
                          << "you should check for errors with glGetError()"
                          << std::endl;        }

                #endif
                // Make sure that extensions are initialized
                if (GL_ARB_multitexture)
                {
                    //glCheck(glClientActiveTexture(GL_TEXTURE0));
                    glCheck(glActiveTexture(GL_TEXTURE0));
                }
                /*glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
                glFrontFace(GL_CW);*/
                glCheck(glDisable(GL_LIGHTING));
                glCheck(glEnable(GL_DEPTH_TEST));

                if (enableAlphaTest) {
                    glCheck(glEnable(GL_ALPHA_TEST));
                } else {
                    glCheck(glDisable(GL_ALPHA_TEST));
                }
                glCheck(glAlphaFunc(GL_GREATER, 0.f));
                glCheck(glDepthFunc(GL_GREATER));
                if (!enableCubeMap) {
                    glCheck(glEnable(GL_TEXTURE_2D));
                    glCheck(glDisable(GL_TEXTURE_CUBE_MAP));
                } else {
                    glCheck(glEnable(GL_TEXTURE_CUBE_MAP));
                    glCheck(glDisable(GL_TEXTURE_2D));
                }
                glCheck(glEnable(GL_BLEND));
                glCheck(glClearDepth(0));
                glCheck(glDepthMask(GL_TRUE));
                glCheck(glDisable(GL_SCISSOR_TEST));

                m_cache.glStatesSet = true;

                // Apply the default SFML states
                applyBlendMode(BlendAlpha);
                TransformMatrix tm;
                applyTransform(tm);
                applyTexture(nullptr);
                if (Shader::isAvailable())
                    applyShader(nullptr);
                m_cache.useVertexCache = false;

                // Set the default view
                setView(getView());
            }
        }


        ////////////////////////////////////////////////////////////
        void RenderTarget::initialize(unsigned int framebufferId)
        {
            m_framebufferId = framebufferId;
            //std::cout<<"version : "<<m_versionMajor<<"."<<m_versionMinor<<std::endl;
            if (m_versionMajor > 3 || m_versionMajor == 3 && m_versionMinor >= 3) {
                GLuint vaoID;
                glCheck(glGenVertexArrays(1, &vaoID));
                m_vao = static_cast<unsigned int>(vaoID);
            }
            #ifdef ODFAEG_DEBUG
            if (m_versionMajor >= 4) {
                std::cout<<"load Debug functions"<<std::endl;
                OpenGL::LoadDebugFunctions();
                OpenGL::InitialiseDebugFunctions();
            }
            #endif
            // Setup the default and current views
            m_defaultView = View (static_cast<float>(getSize().x), static_cast<float>(getSize().y), -static_cast<float>(getSize().y) - 200, static_cast<float>(getSize().y)+200);
            m_defaultView.reset(physic::BoundingBox(0, 0, -static_cast<float>(getSize().y) - 200,static_cast<float>(getSize().x), static_cast<float>(getSize().y),static_cast<float>(getSize().y)+200));
            m_view = m_defaultView;

            // Set GL states only on first draw, so that we don't pollute user's states
            m_cache.glStatesSet = false;
            m_cache.lastVboBuffer = nullptr;
        }


        ////////////////////////////////////////////////////////////
        void RenderTarget::applyCurrentView()
        {
            // Set the viewport
            physic::BoundingBox viewport = getView().getViewport();
            glCheck(glViewport(viewport.getPosition().x, viewport.getPosition().y, viewport.getWidth(), viewport.getHeight()));
            // Set the projection matrix
            glCheck(glMatrixMode(GL_PROJECTION));
            //float* projMatrix = getView().getProjMatrix().getGlMatrix();
            glCheck(glLoadMatrixf(getView().getProjMatrix().getMatrix().transpose().toGlMatrix().data()));
            //delete projMatrix;
            //float* viewMatrix = getView().getViewMatrix().getGlMatrix();
            glCheck(glMultMatrixf(getView().getViewMatrix().getMatrix().transpose().toGlMatrix().data()));
            //delete viewMatrix;

            // Go back to model-view mode
            getView().updated();
            glCheck(glMatrixMode(GL_MODELVIEW));
        }
        ////////////////////////////////////////////////////////////
        void RenderTarget::applyBlendMode(const BlendMode& mode)
        {

            // Apply the blend mode, falling back to the non-separate versions if necessary

            if (glBlendFuncSeparateEXT) {
                glCheck(glBlendFuncSeparate(
                factorToGlConstant(mode.colorSrcFactor), factorToGlConstant(mode.colorDstFactor),
                factorToGlConstant(mode.alphaSrcFactor), factorToGlConstant(mode.alphaDstFactor)));

            } else {
                    glCheck(glBlendFunc(
                    factorToGlConstant(mode.colorSrcFactor),
                    factorToGlConstant(mode.colorDstFactor)));
            }

            if (glBlendFuncSeparateEXT)
            {
                glCheck(glBlendEquationSeparate(
                    equationToGlConstant(mode.colorEquation),
                    equationToGlConstant(mode.alphaEquation)));
            } else {
                glCheck(glBlendEquation(equationToGlConstant(mode.colorEquation)));
            }

            m_cache.lastBlendMode = mode;
        }
        ////////////////////////////////////////////////////////////
        void RenderTarget::applyTexture(const Texture* texture)
        {
            Texture::bind(texture, Texture::Pixels);
            m_cache.lastTextureId = texture ? texture->m_cacheId : 0;

        }
        void RenderTarget::applyTransform(TransformMatrix& tm) {
            glCheck(glLoadIdentity());
            //float* matrix = tm.getGlMatrix();
            glCheck(glMultMatrixf(tm.getMatrix().transpose().toGlMatrix().data()));
            //delete matrix;
        }
        ////////////////////////////////////////////////////////////
        void RenderTarget::applyShader(const Shader* shader)
        {
            Shader::bind(shader);
        }
        void RenderTarget::setEnableCubeMap(bool enableCubeMap) {
            this->enableCubeMap = enableCubeMap;
            m_cache.glStatesSet = false;
        }
        #endif
    }
}

