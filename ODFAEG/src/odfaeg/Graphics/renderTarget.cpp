#include "../../../include/odfaeg/Graphics/renderTarget.h"
#include "../../../include/odfaeg/Graphics/drawable.h"
#include "../../../include/odfaeg/Graphics/batcher.h"
//#include "../../../include/odfaeg/Graphics/glExtensions.hpp"
#ifndef VULKAN
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include "glCheck.h"
#endif

namespace
{
    #ifndef VULKAN


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
    #else
    VkBlendFactor factorToVkConstant(sf::BlendMode::Factor blendFactor) {
        switch(blendFactor) {
            default:
            case sf::BlendMode::Zero:             return VK_BLEND_FACTOR_ZERO;
            case sf::BlendMode::One:              return VK_BLEND_FACTOR_ONE;
            case sf::BlendMode::SrcColor:         return VK_BLEND_FACTOR_SRC_COLOR;
            case sf::BlendMode::OneMinusSrcColor: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
            case sf::BlendMode::DstColor:         return VK_BLEND_FACTOR_DST_COLOR;
            case sf::BlendMode::OneMinusDstColor: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
            case sf::BlendMode::SrcAlpha:         return VK_BLEND_FACTOR_SRC_ALPHA;
            case sf::BlendMode::OneMinusSrcAlpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            case sf::BlendMode::DstAlpha:         return VK_BLEND_FACTOR_DST_ALPHA;
            case sf::BlendMode::OneMinusDstAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        }
    }
    VkBlendOp equationToVkConstant(sf::BlendMode::Equation blendEquation) {
        switch (blendEquation)
        {
            default:
            case sf::BlendMode::Add:             return VK_BLEND_OP_ADD;
            case sf::BlendMode::Subtract:        return VK_BLEND_OP_SUBTRACT;
        }
    }
    #endif // VULKAN
}
namespace odfaeg {
    namespace graphic {
        using namespace sf;
        #ifdef VULKAN
        unsigned int RenderTarget::nbBuffers = 0;
        std::vector<VertexBuffer*> RenderTarget::vertexBuffers =  std::vector<VertexBuffer*>();
        std::vector<std::vector<VkBuffer>> RenderTarget::uniformBuffers = std::vector<std::vector<VkBuffer>>();
        std::vector<std::vector<VkDeviceMemory>> RenderTarget::uniformBuffersMemory = std::vector<std::vector<VkDeviceMemory>>();
        std::vector<std::vector<std::vector<VkPipelineLayoutCreateInfo>>> RenderTarget::pipelineLayoutInfo = std::vector<std::vector<std::vector<VkPipelineLayoutCreateInfo>>>();
        std::vector<std::vector<std::vector<VkPipelineLayout>>> RenderTarget::pipelineLayout = std::vector<std::vector<std::vector<VkPipelineLayout>>>();
        std::vector<std::vector<std::vector<VkPipelineDepthStencilStateCreateInfo>>> RenderTarget::depthStencil = std::vector<std::vector<std::vector<VkPipelineDepthStencilStateCreateInfo>>>();
        std::vector<std::vector<std::vector<VkPipeline>>> RenderTarget::graphicsPipeline = std::vector<std::vector<std::vector<VkPipeline>>>();
        std::vector<VkDescriptorPool> RenderTarget::descriptorPool = std::vector<VkDescriptorPool>();
        std::vector<VkDescriptorSetLayout> RenderTarget::descriptorSetLayout = std::vector<VkDescriptorSetLayout>();
        std::vector<std::vector<VkDescriptorSet>> RenderTarget::descriptorSets = std::vector<std::vector<VkDescriptorSet>>();
        unsigned int RenderTarget::nbRenderTargets = 0;

        RenderTarget::RenderTarget(window::Device& vkDevice) : vkDevice(vkDevice), defaultShader(vkDevice), defaultShader2(vkDevice),
        m_defaultView(), m_view(), id(nbRenderTargets), depthTestEnabled(false), stencilTestEnabled(false), depthTexture(nullptr) {
            nbRenderTargets++;
        }
        RenderTarget::~RenderTarget() {
            nbRenderTargets--;
            cleanup();
        }
        void RenderTarget::enableDepthTest(bool enabled) {
            depthTestEnabled = enabled;
        }
        void RenderTarget::enableStencilTest (bool enabled) {
            stencilTestEnabled = enabled;
        }
        void RenderTarget::initialize() {


            m_defaultView = View (static_cast<float>(getSize().x), static_cast<float>(getSize().y), -static_cast<float>(getSize().y) - 200, static_cast<float>(getSize().y)+200);
            m_defaultView.reset(physic::BoundingBox(0, 0, -static_cast<float>(getSize().y) - 200,static_cast<float>(getSize().x), static_cast<float>(getSize().y),static_cast<float>(getSize().y)+200));
            m_view = m_defaultView;
            const std::string defaultVertexShader = R"(#version 450
                                                        layout(binding = 0) uniform UniformBufferObject {
                                                            mat4 model;
                                                            mat4 view;
                                                            mat4 proj;
                                                        } ubo;
                                                        layout(location = 0) in vec3 inPosition;
                                                        layout(location = 1) in vec4 inColor;
                                                        layout(location = 2) in vec2 inTexCoord;
                                                        layout(location = 3) in vec3 normals;

                                                        layout(location = 0) out vec4 fragColor;
                                                        layout(location = 1) out vec2 fragTexCoord;
                                                        layout(location = 2) out vec3 normal;


                                                        void main() {
                                                            gl_PointSize = 2.0f;
                                                            gl_Position =  vec4(inPosition, 1.0) * ubo.model * ubo.view * ubo.proj;
                                                            fragColor = inColor;
                                                            fragTexCoord = inTexCoord;
                                                            normal = normals;
                                                        }
                                                        )";
            const std::string defaultFragmentShader = R"(#version 450
                                                          #extension GL_ARB_separate_shader_objects : enable
                                                          layout(binding = 1) uniform sampler2D texSampler;
                                                          layout(location = 0) in vec4 fragColor;
                                                          layout(location = 1) in vec2 fragTexCoord;
                                                          layout(location = 2) in vec3 normal;
                                                          layout(location = 0) out vec4 outColor;
                                                          void main() {
                                                             outColor = textureLod(texSampler, fragTexCoord, 0) * fragColor;
                                                          })";
             const std::string defaultFragmentShader2 = R"(#version 450
                                                          #extension GL_ARB_separate_shader_objects : enable
                                                          layout(location = 0) in vec4 fragColor;
                                                          layout(location = 1) in vec2 fragTexCoord;
                                                          layout(location = 2) in vec3 normal;
                                                          layout(location = 0) out vec4 outColor;
                                                          void main() {
                                                             outColor = fragColor;
                                                          })";
             if (!defaultShader.loadFromMemory(defaultVertexShader, defaultFragmentShader)) {
                  throw core::Erreur (0, "Failed to load default shader", 1);
             }
             //std::cout<<"size : "<<Shader::getNbShaders()<<std::endl;
             if (!defaultShader2.loadFromMemory(defaultVertexShader, defaultFragmentShader2)) {
                  throw core::Erreur (0, "Failed to load default shader 2", 1);
             }
             //std::cout<<"size : "<<Shader::getNbShaders()<<std::endl;

             createCommandPool();
             createCommandBuffers();


             RenderStates states;
             states.shader = &defaultShader;
             createDescriptorPool(states);
             createDescriptorSetLayout(states);

             states.shader = &defaultShader2;
             createDescriptorPool(states);
             createDescriptorSetLayout(states);



             states.shader = &defaultShader;
             for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes - 1; i++) {
                createGraphicPipeline(static_cast<sf::PrimitiveType>(i), states);
             }
             states.shader = &defaultShader2;
             for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes - 1; i++) {
                createGraphicPipeline(static_cast<sf::PrimitiveType>(i), states);
             }
             vkCmdPushDescriptorSetKHR = (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(vkDevice.getDevice(), "vkCmdPushDescriptorSetKHR");
             if (!vkCmdPushDescriptorSetKHR) {
                throw core::Erreur(0, "Could not get a valid function pointer for vkCmdPushDescriptorSetKHR", 1);
             }
             if (!vkCmdPushDescriptorSetKHR) {
                throw odfaeg::core::Erreur(0, "Could not get a valid function pointer for vkCmdPushDescriptorSetKHR", 1);
             }
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
            if (drawable.bufferId == -1) {

                createUniformBuffers();
                drawable.bufferId = nbBuffers;
                selectedBuffer = drawable.bufferId;
                nbBuffers++;
                /*system("PAUSE");*/
            } else {
                selectedBuffer = drawable.bufferId;
            }
            drawable.draw(*this, states);
        }
        void RenderTarget::draw(const Vertex* vertices, unsigned int vertexCount, sf::PrimitiveType type,
                      RenderStates states) {
             vertexBuffers[selectedBuffer]->setPrimitiveType(type);
             vertexBuffers[selectedBuffer]->clear();
             for (unsigned int i = 0; i < vertexCount; i++) {
                vertexBuffers[selectedBuffer]->append(vertices[i]);
                //std::cout<<"vertex : "<<vertices[i].position.x<<std::endl;
             }
             sf::PrimitiveType oldType;
             if (type == sf::Quads) {
                for (unsigned int i = 0; i < vertexBuffers[selectedBuffer]->getSize(); i+=4) {
                    vertexBuffers[selectedBuffer]->addIndex(i);
                    vertexBuffers[selectedBuffer]->addIndex(i+1);
                    vertexBuffers[selectedBuffer]->addIndex(i+2);
                    vertexBuffers[selectedBuffer]->addIndex(i);
                    vertexBuffers[selectedBuffer]->addIndex(i+2);
                    vertexBuffers[selectedBuffer]->addIndex(i+3);
                }
                type = sf::Triangles;
                oldType = sf::Quads;
             }
             if (states.shader == nullptr) {
                if (states.texture != nullptr) {
                    states.shader = &defaultShader;
                } else {
                    states.shader = &defaultShader2;
                }
             }
             vertexBuffers[selectedBuffer]->update();
             UniformBufferObject ubo;
             ubo.proj = m_view.getProjMatrix().getMatrix()/*.transpose()*/;
             //ubo.proj.m22 *= -1;
             ubo.view = m_view.getViewMatrix().getMatrix()/*.transpose()*/;
             ubo.model = states.transform.getMatrix()/*.transpose()*/;
             updateUniformBuffer(getCurrentFrame(), ubo);
             recordCommandBuffers(type, states);
             if (oldType == sf::Quads)
                vertexBuffers[selectedBuffer]->clearIndexes();
             /*std::cout<<"drawn"<<std::endl;
             system("PAUSE");*/

        }
        void RenderTarget::drawIndirect(VkCommandBuffer& cmd, unsigned int i, unsigned int nbIndirectCommands, unsigned int stride, VertexBuffer& vertexBuffer, VkBuffer vboIndirect, unsigned int depthStencilId, RenderStates states) {
            Shader* shader = const_cast<Shader*>(states.shader);
            //std::cout<<"draw indirect depth stencil id :"<<depthStencilId<<std::endl;
            unsigned int descriptorId = id * shader->getNbShaders() + shader->getId();
            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = (depthTestEnabled || stencilTestEnabled) ? getRenderPass(1) : getRenderPass(0);
            renderPassInfo.framebuffer = (depthTestEnabled || stencilTestEnabled) ? getSwapchainFrameBuffers(1)[getImageIndex()] : getSwapchainFrameBuffers(0)[getImageIndex()];
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = getSwapchainExtents();

            VkClearValue clrColor = {clearColor.r / 255.f,clearColor.g / 255.f, clearColor.b / 255.f, clearColor.a / 255.f};
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clrColor;

            vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+vertexBuffer.getPrimitiveType()][id][depthStencilId]);
            VkBuffer vertexBuffers[] = {vertexBuffer.getVertexBuffer()};
            VkDeviceSize offsets[] = {0, 0};
            vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+vertexBuffer.getPrimitiveType()][id][depthStencilId], 0, 1, &descriptorSets[descriptorId][getCurrentFrame()], 0, nullptr);

            applyViewportAndScissor();
            if(vertexBuffer.getIndicesSize() > 0) {
                vkCmdBindIndexBuffer(cmd, vertexBuffer.getIndexBuffer(), 0, VK_INDEX_TYPE_UINT16);
            }
            if(vertexBuffer.getIndicesSize() > 0) {
                vkCmdDrawIndexedIndirect(cmd, vboIndirect, 0, nbIndirectCommands, stride);
            } else {
                vkCmdDrawIndirect(cmd, vboIndirect, 0, nbIndirectCommands, stride);
            }
            vkCmdEndRenderPass(cmd);
        }
        void RenderTarget::drawVertexBuffer(VkCommandBuffer& cmd, unsigned int i, VertexBuffer& vertexBuffer, unsigned int depthStencilId, RenderStates states) {
            //std::cout<<"vertex stencil id :"<<depthStencilId<<std::endl;
            Shader* shader = const_cast<Shader*>(states.shader);
            unsigned int descriptorId = id * shader->getNbShaders() + shader->getId();
            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = (depthTestEnabled || stencilTestEnabled) ? getRenderPass(1) : getRenderPass(0);
            renderPassInfo.framebuffer = (depthTestEnabled || stencilTestEnabled) ? getSwapchainFrameBuffers(1)[getImageIndex()] : getSwapchainFrameBuffers(0)[getImageIndex()];
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = getSwapchainExtents();

            VkClearValue clrColor = {clearColor.r / 255.f,clearColor.g / 255.f, clearColor.b / 255.f, clearColor.a / 255.f};
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clrColor;

            vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+vertexBuffer.getPrimitiveType()][id][depthStencilId]);
            VkBuffer vertexBuffers[] = {vertexBuffer.getVertexBuffer()};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+vertexBuffer.getPrimitiveType()][id][depthStencilId], 0, 1, &descriptorSets[descriptorId][getCurrentFrame()], 0, nullptr);

            applyViewportAndScissor();
            if(vertexBuffer.getIndicesSize() > 0) {
                vkCmdBindIndexBuffer(cmd, vertexBuffer.getIndexBuffer(), 0, VK_INDEX_TYPE_UINT16);
            }
            if(vertexBuffer.getIndicesSize() > 0) {
                vkCmdDrawIndexed(cmd, static_cast<uint32_t>(vertexBuffer.getIndicesSize()), 1, 0, 0, 0);
            } else {
                vkCmdDraw(cmd, static_cast<uint32_t>(vertexBuffer.getSize()), 1, 0, 0);
            }

            vkCmdEndRenderPass(cmd);
        }


        void RenderTarget::createDescriptorSetLayout(RenderStates states) {
            Shader* shader = const_cast<Shader*>(states.shader);
            descriptorSetLayout.resize(shader->getNbShaders() * nbRenderTargets);
            unsigned int descriptorId = id * shader->getNbShaders() + shader->getId();
            if (states.shader == &defaultShader) {
                VkDescriptorSetLayoutBinding uboLayoutBinding{};
                uboLayoutBinding.binding = 0;
                uboLayoutBinding.descriptorCount = 1;
                uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                uboLayoutBinding.pImmutableSamplers = nullptr;
                uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                VkDescriptorSetLayoutBinding samplerLayoutBinding{};
                samplerLayoutBinding.binding = 1;
                samplerLayoutBinding.descriptorCount = 1;
                samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                samplerLayoutBinding.pImmutableSamplers = nullptr;
                samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

                VkDescriptorSetLayoutCreateInfo layoutInfo{};
                layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
                layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());;
                layoutInfo.pBindings = bindings.data();

                if (vkCreateDescriptorSetLayout(vkDevice.getDevice(), &layoutInfo, nullptr, &descriptorSetLayout[descriptorId]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create descriptor set layout!");
                }
            } else {
                VkDescriptorSetLayoutBinding uboLayoutBinding{};
                uboLayoutBinding.binding = 0;
                uboLayoutBinding.descriptorCount = 1;
                uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                uboLayoutBinding.pImmutableSamplers = nullptr;
                uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;


                std::array<VkDescriptorSetLayoutBinding, 1> bindings = {uboLayoutBinding};

                VkDescriptorSetLayoutCreateInfo layoutInfo{};
                layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
                layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());;
                layoutInfo.pBindings = bindings.data();

                if (vkCreateDescriptorSetLayout(vkDevice.getDevice(), &layoutInfo, nullptr, &descriptorSetLayout[descriptorId]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create descriptor set layout!");
                }
            }
        }
        void RenderTarget::createDescriptorPool(RenderStates states) {

            Shader* shader = const_cast<Shader*>(states.shader);
            descriptorPool.resize(shader->getNbShaders()*nbRenderTargets);
            unsigned int descriptorId = id * shader->getNbShaders() + shader->getId();
            if (states.shader == &defaultShader) {
                std::array<VkDescriptorPoolSize, 2> poolSizes{};
                poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                poolSizes[0].descriptorCount = static_cast<uint32_t>(getMaxFramesInFlight());
                poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                poolSizes[1].descriptorCount = static_cast<uint32_t>(getMaxFramesInFlight());

                VkDescriptorPoolCreateInfo poolInfo{};
                poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
                poolInfo.pPoolSizes = poolSizes.data();
                poolInfo.maxSets = static_cast<uint32_t>(getMaxFramesInFlight());
                if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool[descriptorId]) != VK_SUCCESS) {
                    throw std::runtime_error("echec de la creation de la pool de descripteurs!");
                }
            } else {
                std::array<VkDescriptorPoolSize, 1> poolSizes{};
                poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                poolSizes[0].descriptorCount = static_cast<uint32_t>(getMaxFramesInFlight());

                VkDescriptorPoolCreateInfo poolInfo{};
                poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
                poolInfo.pPoolSizes = poolSizes.data();
                poolInfo.maxSets = static_cast<uint32_t>(getMaxFramesInFlight());
                if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool[descriptorId]) != VK_SUCCESS) {
                    throw std::runtime_error("echec de la creation de la pool de descripteurs!");
                }
            }
        }
        void RenderTarget::createDescriptorSets(RenderStates states) {
            Shader* shader = const_cast<Shader*>(states.shader);
            for (size_t i = 0; i < getMaxFramesInFlight(); i++) {
                VkDescriptorBufferInfo bufferInfo{};
                bufferInfo.buffer = uniformBuffers[selectedBuffer][i];
                bufferInfo.offset = 0;
                bufferInfo.range = sizeof(UniformBufferObject);
                if (states.shader == &defaultShader) {
                    VkDescriptorImageInfo imageInfo{};
                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    imageInfo.imageView = const_cast<Texture*>(states.texture)->getImageView();
                    imageInfo.sampler = const_cast<Texture*>(states.texture)->getSampler();
                    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

                    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[0].dstSet = descriptorSets[shader->getId()][i];
                    descriptorWrites[0].dstBinding = 0;
                    descriptorWrites[0].dstArrayElement = 0;
                    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    descriptorWrites[0].descriptorCount = 1;
                    descriptorWrites[0].pBufferInfo = &bufferInfo;



                    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[1].dstSet = descriptorSets[shader->getId()][i];
                    descriptorWrites[1].dstBinding = 1;
                    descriptorWrites[1].dstArrayElement = 0;
                    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrites[1].descriptorCount = 1;
                    descriptorWrites[1].pImageInfo = &imageInfo;

                    vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
                }  else {
                    std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

                    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[0].dstSet = descriptorSets[i][shader->getId()];
                    descriptorWrites[0].dstBinding = 0;
                    descriptorWrites[0].dstArrayElement = 0;
                    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    descriptorWrites[0].descriptorCount = 1;
                    descriptorWrites[0].pBufferInfo = &bufferInfo;

                    vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
                }
            }
        }
        void RenderTarget::createGraphicPipeline(sf::PrimitiveType type,
                      RenderStates states, unsigned int depthStencilId, unsigned int nbDepthStencil) {
            Shader* shader = const_cast<Shader*>(states.shader);
            if ((Batcher::nbPrimitiveTypes - 1) * shader->getNbShaders() > graphicsPipeline.size()) {
                graphicsPipeline.resize((Batcher::nbPrimitiveTypes - 1) * shader->getNbShaders());
                pipelineLayoutInfo.resize((Batcher::nbPrimitiveTypes - 1) * shader->getNbShaders());
                pipelineLayout.resize((Batcher::nbPrimitiveTypes - 1) * shader->getNbShaders());
                depthStencil.resize((Batcher::nbPrimitiveTypes - 1) * shader->getNbShaders());
            }
            for (unsigned int i = 0; i < (Batcher::nbPrimitiveTypes - 1) * shader->getNbShaders(); i++) {
                if (nbRenderTargets > graphicsPipeline[i].size()) {
                    graphicsPipeline[i].resize(nbRenderTargets);
                    pipelineLayoutInfo[i].resize(nbRenderTargets);
                    pipelineLayout[i].resize(nbRenderTargets);
                    depthStencil[i].resize(nbRenderTargets);
                }
            }

            for (unsigned int i = 0; i < (Batcher::nbPrimitiveTypes - 1) * shader->getNbShaders(); i++) {
                for (unsigned int j = 0; j < nbRenderTargets; j++) {
                    if (nbDepthStencil > graphicsPipeline[i][j].size()) {
                        graphicsPipeline[i][j].resize(nbDepthStencil);
                        pipelineLayoutInfo[i][j].resize(nbDepthStencil);
                        pipelineLayout[i][j].resize(nbDepthStencil);
                        depthStencil[i][j].resize(nbDepthStencil);
                    }
                }
            }

            unsigned int descriptorId = id * shader->getNbShaders() + shader->getId();
            //std::cout<<"rt dl size : "<<descriptorSetLayout.size()<<std::endl;
            /*std::cout<<"descriptor id : "<<descriptorId<<std::endl;*/
            //std::cout<<"pipeline id : "<<pipelineId<<std::endl;


            /*for (unsigned int i = 0; i < pipelinesId.size(); i++) {
                if (pipelinesId[i] == pipelineId) {
                    std::cout<<"Erreur"<<std::endl;
                    system("PAUSE");
                }
            }
            pipelinesId.push_back(pipelineId);*/

            VkShaderModule vertShaderModule;
            VkShaderModule fragShaderModule;
            VkShaderModule geomShaderModule=nullptr;

            if (states.shader == nullptr) {
                if (states.texture != nullptr) {
                    defaultShader.createShaderModules();
                    vertShaderModule = defaultShader.getVertexShaderModule();
                    fragShaderModule = defaultShader.getFragmentShaderModule();
                    states.shader = &defaultShader;
                } else {
                    defaultShader2.createShaderModules();
                    vertShaderModule = defaultShader2.getVertexShaderModule();
                    fragShaderModule = defaultShader2.getFragmentShaderModule();
                    states.shader = &defaultShader2;
                }
            } else {
                shader->createShaderModules();
                vertShaderModule = shader->getVertexShaderModule();
                fragShaderModule = shader->getFragmentShaderModule();
                if (shader->getGeometryShaderModule() != nullptr) {
                    geomShaderModule = shader->getGeometryShaderModule();
                }
            }

            std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

            if (geomShaderModule == nullptr) {
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

                shaderStages = {vertShaderStageInfo, fragShaderStageInfo};
            } else {
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

                VkPipelineShaderStageCreateInfo geomShaderStageInfo{};
                fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                fragShaderStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
                fragShaderStageInfo.module = geomShaderModule;
                fragShaderStageInfo.pName = "main";

                shaderStages = {vertShaderStageInfo, fragShaderStageInfo, geomShaderStageInfo};
            }

            VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
            auto bindingDescription = Vertex::getBindingDescription();
            auto attributeDescriptions = Vertex::getAttributeDescriptions();
            vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputInfo.vertexBindingDescriptionCount = 1;
            vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
            vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
            vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();


            VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
            inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            VkPrimitiveTopology modes[] = {VK_PRIMITIVE_TOPOLOGY_POINT_LIST, VK_PRIMITIVE_TOPOLOGY_LINE_LIST, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN};

            inputAssembly.topology = modes[type];
            inputAssembly.primitiveRestartEnable = VK_FALSE;



            VkPipelineViewportStateCreateInfo viewportState{};
            viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.viewportCount = 1;
            //viewportState.pViewports = &viewport;
            viewportState.scissorCount = 1;
            //viewportState.pScissors = &scissor;
            std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
            };
            VkPipelineDynamicStateCreateInfo dynamicState{};
            dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
            dynamicState.pDynamicStates = dynamicStates.data();

            VkPipelineRasterizationStateCreateInfo rasterizer{};
            rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer.depthClampEnable = VK_FALSE;
            rasterizer.rasterizerDiscardEnable = VK_FALSE;
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer.lineWidth = 1.0f;
            rasterizer.cullMode = VK_CULL_MODE_NONE;
            rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
            rasterizer.depthBiasEnable = VK_FALSE;

            VkPipelineMultisampleStateCreateInfo multisampling{};
            multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.sampleShadingEnable = VK_FALSE;
            multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineColorBlendAttachmentState colorBlendAttachment{};
            colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            colorBlendAttachment.blendEnable = VK_TRUE;
            sf::BlendMode mode = states.blendMode;
            colorBlendAttachment.srcColorBlendFactor = factorToVkConstant(mode.colorSrcFactor);
            colorBlendAttachment.dstColorBlendFactor = factorToVkConstant(mode.colorDstFactor);
            colorBlendAttachment.colorBlendOp = equationToVkConstant(mode.colorEquation);

            colorBlendAttachment.srcAlphaBlendFactor = factorToVkConstant(mode.alphaSrcFactor);
            colorBlendAttachment.dstAlphaBlendFactor = factorToVkConstant(mode.alphaDstFactor);
            colorBlendAttachment.alphaBlendOp = equationToVkConstant(mode.alphaEquation);

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

            pipelineLayoutInfo[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId].sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId].setLayoutCount = 1;
            pipelineLayoutInfo[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId].pSetLayouts = &descriptorSetLayout[descriptorId];

            if (pipelineLayout[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId] != nullptr) {
                //system("PAUSE");
                vkDestroyPipelineLayout(vkDevice.getDevice(), pipelineLayout[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId], nullptr);
            }
            if (vkCreatePipelineLayout(vkDevice.getDevice(), &pipelineLayoutInfo[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId], nullptr, &pipelineLayout[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId]) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to create pipeline layout!", 1);
            }
            //std::cout<<"pipeline layout : "<<pipelineLayout[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId]<<std::endl;

            depthStencil[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId].sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depthStencil[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId].depthTestEnable = (depthTestEnabled) ? VK_TRUE : VK_FALSE;
            depthStencil[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId].depthWriteEnable = VK_TRUE;
            //depthStencil[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId].depthCompareOp = VK_COMPARE_OP_ALWAYS;
            depthStencil[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId].depthBoundsTestEnable = VK_FALSE;
            depthStencil[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId].minDepthBounds = 0.0f; // Optional
            depthStencil[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId].maxDepthBounds = 1.0f; // Optional
            depthStencil[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId].stencilTestEnable = (stencilTestEnabled) ? VK_TRUE : VK_FALSE;


            VkGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount = shaderStages.size();
            pipelineInfo.pStages = shaderStages.data();
            pipelineInfo.pVertexInputState = &vertexInputInfo;
            pipelineInfo.pInputAssemblyState = &inputAssembly;
            pipelineInfo.pViewportState = &viewportState;
            pipelineInfo.pRasterizationState = &rasterizer;
            pipelineInfo.pMultisampleState = &multisampling;
            pipelineInfo.pColorBlendState = &colorBlending;
            pipelineInfo.pDynamicState = &dynamicState;
            pipelineInfo.layout = pipelineLayout[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId];
            /*if (getSurface() != VK_NULL_HANDLE)
                std::cout<<"render pass ppl rw : "<<getRenderPass()<<std::endl;
            else
                std::cout<<"render pass ppl rt : "<<getRenderPass()<<std::endl;*/
            pipelineInfo.renderPass = (depthTestEnabled || stencilTestEnabled) ? getRenderPass(1) : getRenderPass(0);
            pipelineInfo.subpass = 0;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
            pipelineInfo.pDepthStencilState = &depthStencil[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId];
            if (graphicsPipeline[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId] != nullptr) {
                vkDestroyPipeline(vkDevice.getDevice(), graphicsPipeline[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId], nullptr);
            }
            if (vkCreateGraphicsPipelines(vkDevice.getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId]) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to create graphics pipeline!", 1);
            }
            shader->cleanupShaderModules();
        }
        void RenderTarget::createCommandPool() {

            window::Device::QueueFamilyIndices queueFamilyIndices = vkDevice.findQueueFamilies(vkDevice.getPhysicalDevice(), getSurface());

            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
            poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optionel
            if (vkCreateCommandPool(vkDevice.getDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
                throw core::Erreur(0, "échec de la création d'une command pool!", 1);
            }
        }
        void RenderTarget::createUniformBuffers() {
            VkDeviceSize bufferSize = sizeof(UniformBufferObject);
            std::vector<VkBuffer> ubos;
            std::vector<VkDeviceMemory> ubosMemory;
            ubos.resize(getMaxFramesInFlight());
            ubosMemory.resize(getMaxFramesInFlight());

            for (size_t i = 0; i < getMaxFramesInFlight(); i++) {
                createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, ubos[i], ubosMemory[i]);
                //std::cout<<"uniform buffer : "<<ubos[i]<<std::endl;
            }
            uniformBuffers.push_back(ubos);
            uniformBuffersMemory.push_back(ubosMemory);
            VertexBuffer* vb = new VertexBuffer(vkDevice);
            vertexBuffers.push_back(std::move(vb));
        }
        void RenderTarget::updateUniformBuffer(uint32_t currentImage, UniformBufferObject ubo) {
            void* data;
            vkMapMemory(vkDevice.getDevice(), uniformBuffersMemory[selectedBuffer][currentImage], 0, sizeof(ubo), 0, &data);
                memcpy(data, &ubo, sizeof(ubo));
            vkUnmapMemory(vkDevice.getDevice(), uniformBuffersMemory[selectedBuffer][currentImage]);

        }
        void RenderTarget::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = size;
            bufferInfo.usage = usage;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (vkCreateBuffer(vkDevice.getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
                throw std::runtime_error("failed to create buffer!");
            }

            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(vkDevice.getDevice(), buffer, &memRequirements);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

            if (vkAllocateMemory(vkDevice.getDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate buffer memory!");
            }

            vkBindBufferMemory(vkDevice.getDevice(), buffer, bufferMemory, 0);
        }
        uint32_t RenderTarget::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
            VkPhysicalDeviceMemoryProperties memProperties;
            vkGetPhysicalDeviceMemoryProperties(vkDevice.getPhysicalDevice(), &memProperties);
            for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
                if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                    return i;
                }
            }
            throw std::runtime_error("aucun type de memoire ne satisfait le buffer!");
        }
        void RenderTarget::createCommandBuffers() {
            commandBuffers.resize(getSwapchainImages().size());

            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = commandPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

            if (vkAllocateCommandBuffers(vkDevice.getDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to allocate command buffers!", 1);
            }


        }
        void RenderTarget::beginRecordCommandBuffers() {
            //std::cout<<"render texture begin command buffer"<<std::endl;
            //for (unsigned int i = 0; i < getCommandBuffers().size(); i++) {
                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                if (vkBeginCommandBuffer(getCommandBuffers()[getCurrentFrame()], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }
            //}
        }
        void RenderTarget::applyViewportAndScissor() {
            VkViewport viewport{};
            viewport.x = m_view.getViewport().getPosition().x;
            viewport.y = (float) m_view.getViewport().getSize().y - m_view.getViewport().getPosition().y;
            viewport.width = m_view.getViewport().getSize().x;
            viewport.height = -(float)  m_view.getViewport().getSize().y;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffers[getCurrentFrame()], 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = getSwapchainExtents();
            vkCmdSetScissor(commandBuffers[getCurrentFrame()], 0, 1, &scissor);
        }
        void RenderTarget::recordCommandBuffers(sf::PrimitiveType type, RenderStates states) {
            Shader* shader = const_cast<Shader*>(states.shader);


            //for (size_t i = 0; i < commandBuffers.size(); i++) {
                /*VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

                if (vkBeginCommandBuffer(commandBuffers[getCurrentFrame()], &beginInfo) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }*/
                VkRenderPassBeginInfo renderPassInfo{};
                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                /*if (getSurface() != VK_NULL_HANDLE)
                    std::cout<<"render pass cmd rw : "<<getRenderPass()<<std::endl;
                else
                    std::cout<<"render pass cmd rt : "<<getRenderPass()<<std::endl;*/
                renderPassInfo.renderPass = (depthTestEnabled || stencilTestEnabled) ? getRenderPass(1) : getRenderPass(0);
                renderPassInfo.framebuffer = (depthTestEnabled || stencilTestEnabled) ? getSwapchainFrameBuffers(1)[getImageIndex()] : getSwapchainFrameBuffers(0)[getImageIndex()];
                renderPassInfo.renderArea.offset = {0, 0};
                renderPassInfo.renderArea.extent = getSwapchainExtents();

                VkClearValue clrColor = {clearColor.r / 255.f,clearColor.g / 255.f, clearColor.b / 255.f, clearColor.a / 255.f};
                renderPassInfo.clearValueCount = 1;
                renderPassInfo.pClearValues = &clrColor;


                vkCmdBeginRenderPass(commandBuffers[getCurrentFrame()], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
                vkCmdBindPipeline(commandBuffers[getCurrentFrame()], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][0]);
                //std::cout<<"buffer : "<<this->vertexBuffers[selectedBuffer]->getVertexBuffer()<<std::endl;
                VkBuffer vertexBuffers[] = {this->vertexBuffers[selectedBuffer]->getVertexBuffer()};
                VkDeviceSize offsets[] = {0};
                vkCmdBindVertexBuffers(commandBuffers[getCurrentFrame()], 0, 1, vertexBuffers, offsets);
                VkDescriptorBufferInfo bufferInfo{};
                bufferInfo.buffer = uniformBuffers[selectedBuffer][getCurrentFrame()];
                bufferInfo.offset = 0;
                bufferInfo.range = sizeof(UniformBufferObject);
                if (states.shader == &defaultShader) {
                    VkDescriptorImageInfo imageInfo{};
                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    imageInfo.imageView = const_cast<Texture*>(states.texture)->getImageView();
                    imageInfo.sampler = const_cast<Texture*>(states.texture)->getSampler();
                    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

                    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[0].dstSet = 0;
                    descriptorWrites[0].dstBinding = 0;
                    descriptorWrites[0].dstArrayElement = 0;
                    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    descriptorWrites[0].descriptorCount = 1;
                    descriptorWrites[0].pBufferInfo = &bufferInfo;



                    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[1].dstSet = 0;
                    descriptorWrites[1].dstBinding = 1;
                    descriptorWrites[1].dstArrayElement = 0;
                    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrites[1].descriptorCount = 1;
                    descriptorWrites[1].pImageInfo = &imageInfo;

                    vkCmdPushDescriptorSetKHR(commandBuffers[getCurrentFrame()], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][0], 0, 2, descriptorWrites.data());
                }  else {
                    std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

                    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[0].dstSet = 0;
                    descriptorWrites[0].dstBinding = 0;
                    descriptorWrites[0].dstArrayElement = 0;
                    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    descriptorWrites[0].descriptorCount = 1;
                    descriptorWrites[0].pBufferInfo = &bufferInfo;

                    vkCmdPushDescriptorSetKHR(commandBuffers[getCurrentFrame()], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][0], 0, 1, descriptorWrites.data());
                }
                applyViewportAndScissor();


                if(this->vertexBuffers[selectedBuffer]->getIndicesSize() > 0) {
                    vkCmdBindIndexBuffer(commandBuffers[getCurrentFrame()], this->vertexBuffers[selectedBuffer]->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT16);
                }
                if(this->vertexBuffers[selectedBuffer]->getIndicesSize() > 0) {
                    vkCmdDrawIndexed(commandBuffers[getCurrentFrame()], static_cast<uint32_t>(this->vertexBuffers[selectedBuffer]->getIndicesSize()), 1, 0, 0, 0);
                } else {
                    vkCmdDraw(commandBuffers[getCurrentFrame()], static_cast<uint32_t>(this->vertexBuffers[selectedBuffer]->getSize()), 1, 0, 0);
                }

                vkCmdEndRenderPass(commandBuffers[getCurrentFrame()]);

                /*if (vkEndCommandBuffer(commandBuffers[getCurrentFrame()]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }*/

            //}

        }
        Texture& RenderTarget::getDepthTexture() {
            if (depthTexture == nullptr) {
                depthTexture = new Texture(vkDevice);
            }
            return *depthTexture;
        }
        void RenderTarget::cleanup() {
            std::cout<<"destroy command buffers"<<std::endl;
            if (commandBuffers.size() > 0) {
                vkFreeCommandBuffers(vkDevice.getDevice(), commandPool, commandBuffers.size(), commandBuffers.data());
                vkDestroyCommandPool(vkDevice.getDevice(), commandPool, nullptr);
            }
            delete depthTexture;

            if (nbRenderTargets == 0) {


                for (unsigned int i = 0; i < pipelineLayout.size(); i++) {
                    for (unsigned int j = 0; j < pipelineLayout[i].size(); j++) {
                        for (unsigned int k = 0; k < pipelineLayout[i][j].size(); k++) {
                            vkDestroyPipelineLayout(vkDevice.getDevice(), pipelineLayout[i][j][k], nullptr);

                        }
                    }
                }

                for (unsigned int i = 0; i < graphicsPipeline.size(); i++) {
                    for (unsigned int j = 0; j < graphicsPipeline[i].size(); j++) {
                        for (unsigned int k = 0; k < graphicsPipeline[i][j].size(); k++) {

                            vkDestroyPipeline(vkDevice.getDevice(), graphicsPipeline[i][j][k], nullptr);

                        }
                    }
                }
                for (unsigned int i = 0; i < descriptorSetLayout.size(); i++) {
                    vkDestroyDescriptorSetLayout(vkDevice.getDevice(), descriptorSetLayout[i], nullptr);
                }
                for (unsigned int i = 0; i < descriptorPool.size(); i++) {
                    vkDestroyDescriptorPool(vkDevice.getDevice(), descriptorPool[i], nullptr);
                }
                for (unsigned int i = 0; i < uniformBuffers.size(); i++) {
                    for (size_t j = 0; j < uniformBuffers[i].size(); j++) {
                        vkDestroyBuffer(vkDevice.getDevice(), uniformBuffers[i][j], nullptr);
                        vkFreeMemory(vkDevice.getDevice(), uniformBuffersMemory[i][j], nullptr);
                    }
                }
                for (unsigned int i = 0; i < vertexBuffers.size(); i++) {
                    delete vertexBuffers[i];
                }
            }
        }
        std::vector<VkCommandBuffer>& RenderTarget::getCommandBuffers() {
            return commandBuffers;
        }
        std::vector<std::vector<std::vector<VkPipelineLayoutCreateInfo>>>& RenderTarget::getPipelineLayoutCreateInfo() {
            return pipelineLayoutInfo;
        }
        std::vector<std::vector<std::vector<VkPipelineDepthStencilStateCreateInfo>>>& RenderTarget::getDepthStencilCreateInfo() {
            return depthStencil;
        }
        std::vector<std::vector<std::vector<VkPipelineLayout>>>& RenderTarget::getPipelineLayout() {
            return pipelineLayout;
        }
        std::vector<std::vector<std::vector<VkPipeline>>>& RenderTarget::getGraphicPipeline() {
            return graphicsPipeline;
        }
        std::vector<VkDescriptorPool>& RenderTarget::getDescriptorPool() {
            return descriptorPool;
        }
        std::vector<VkDescriptorSetLayout>& RenderTarget::getDescriptorSetLayout() {
            return descriptorSetLayout;
        }
        std::vector<std::vector<VkDescriptorSet>>& RenderTarget::getDescriptorSet() {
            return descriptorSets;
        }
        unsigned int RenderTarget::getId() {
            return id;
        }
        unsigned int RenderTarget::getNbRenderTargets() {
            return nbRenderTargets;
        }
        void RenderTarget::updateCommandBuffers(VkCommandPool commandPool, std::vector<VkCommandBuffer> commandBuffers) {
            vkFreeCommandBuffers(vkDevice.getDevice(), this->commandPool, this->commandBuffers.size(), this->commandBuffers.data());
            vkDestroyCommandPool(vkDevice.getDevice(), this->commandPool, 0);
            this->commandBuffers = commandBuffers;
            this->commandPool = commandPool;
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
        ViewportMatrix RenderTarget::getViewportMatrix(View* view) {
            ViewportMatrix vpm;
            vpm.setViewport(math::Vec3f(view->getViewport().getPosition().x, view->getViewport().getPosition().y, 0),
            math::Vec3f(view->getViewport().getWidth(), view->getViewport().getHeight(), 1));
            return vpm;
        }
        void RenderTarget::drawVBOBindlessIndirect(sf::PrimitiveType type, unsigned int nbIndirectCommands, RenderStates states, unsigned int vboIndirect) {
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
                static const GLenum modes[] = {GL_POINTS, GL_LINES, GL_LINE_STRIP, GL_TRIANGLES,
                                                       GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_QUADS};
                GLenum mode = modes[type];
                glCheck(glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferId));
                glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vboIndirect));
                glCheck(glMultiDrawArraysIndirect(mode,0,nbIndirectCommands,0));
                if (m_versionMajor > 3 || m_versionMajor == 3 && m_versionMinor >= 3)
                    glCheck(glBindVertexArray(0));
            }
            applyTexture(nullptr);
            applyShader(nullptr);
         }
         void RenderTarget::drawIndirect(VertexBuffer& vertexBuffer, sf::PrimitiveType type, unsigned int nbIndirectCommands, RenderStates states, unsigned int vboIndirect, unsigned int vboMatrix1, unsigned int vboMatrix2) {
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
                        glCheck(glVertexAttribPointer(3, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), (GLvoid*) 24));
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
                        if (vertexBuffer.vboMaterialType != 0) {
                            glCheck(glEnableVertexAttribArray(13));
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboMaterialType));
                            glCheck(glVertexAttribIPointer(13, 1, GL_UNSIGNED_INT, sizeof(GLuint), (GLvoid*) 0));
                            glCheck(glDisableVertexAttribArray(13));
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        }
                        if (vertexBuffer.vboLayer != 0) {
                            glCheck(glEnableVertexAttribArray(14));
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboLayer));
                            glCheck(glVertexAttribIPointer(14, 1, GL_UNSIGNED_INT, sizeof(GLuint), (GLvoid*) 0));
                            glCheck(glDisableVertexAttribArray(14));
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        }
                        if (vertexBuffer.vboSpecular != 0) {
                            glCheck(glEnableVertexAttribArray(15));
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboSpecular));
                            glCheck(glVertexAttribPointer(15, 2, GL_FLOAT,GL_FALSE,sizeof(sf::Vector2f),(GLvoid*) 0));
                            glCheck(glDisableVertexAttribArray(15));
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        }
                        if (vertexBuffer.vboLightInfos != 0) {
                            glCheck(glEnableVertexAttribArray(16));
                            glCheck(glEnableVertexAttribArray(17));
                            glCheck(glVertexAttribPointer(16, 4,GL_FLOAT,GL_FALSE,sizeof(Vertex), (GLvoid*) 0));
                            glCheck(glVertexAttribPointer(17, 4,GL_UNSIGNED_BYTE,GL_TRUE,sizeof(Vertex),(GLvoid*) 16));
                            glCheck(glDisableVertexAttribArray(16));
                            glCheck(glDisableVertexAttribArray(17));
                        }
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
                    if (vertexBuffer.vboMaterialType != 0) {
                        glCheck(glEnableVertexAttribArray(13));
                    }
                    if (vertexBuffer.vboLayer != 0) {
                        glCheck(glEnableVertexAttribArray(14));
                    }
                    if (vertexBuffer.vboSpecular != 0) {
                        glCheck(glEnableVertexAttribArray(15));
                    }
                    if (vertexBuffer.vboLightInfos != 0) {
                        glCheck(glEnableVertexAttribArray(16));
                        glCheck(glEnableVertexAttribArray(17));
                    }
                    static const GLenum modes[] = {GL_POINTS, GL_LINES, GL_LINE_STRIP, GL_TRIANGLES,
                                                       GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_QUADS};
                    GLenum mode = modes[type];
                    glCheck(glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferId));
                    glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vboIndirect));
                    if (vertexBuffer.getIndexes().size() == 0) {
                        glCheck(glMultiDrawArraysIndirect(mode,0,nbIndirectCommands,0));
                    } else {
                        glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer.vboIndexBuffer));
                        glCheck(glMultiDrawElementsIndirect(mode, GL_UNSIGNED_INT, 0, nbIndirectCommands, 0));
                        glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
                    }
                    glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
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
                    if (vertexBuffer.vboMaterialType != 0) {
                        glCheck(glDisableVertexAttribArray(13));
                    }
                    if (vertexBuffer.vboLayer != 0) {
                        glCheck(glDisableVertexAttribArray(14));
                    }
                    if (vertexBuffer.vboSpecular != 0) {
                        glCheck(glDisableVertexAttribArray(15));
                    }
                    if (vertexBuffer.vboLightInfos != 0) {
                        glCheck(glDisableVertexAttribArray(16));
                        glCheck(glDisableVertexAttribArray(17));
                    }
                    glCheck(glBindVertexArray(0));
                }
            }
            applyTexture(nullptr);
            applyShader(nullptr);
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
                        glCheck(glVertexAttribPointer(3, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), (GLvoid*) 24));
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
            }
            applyTexture(nullptr);
            applyShader(nullptr);
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
                        glCheck(glVertexAttribPointer(3, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), (GLvoid*) 24));
                        glCheck(glEnableVertexAttribArray(4));
                        if (vertexBuffer.vboMaterialType != 0) {
                            glCheck(glEnableVertexAttribArray(5));
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboMaterialType));
                            glCheck(glVertexAttribIPointer(5, 1, GL_UNSIGNED_INT, sizeof(GLuint), (GLvoid*) 0));
                            glCheck(glDisableVertexAttribArray(5));
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        }
                        if (vertexBuffer.vboLayer != 0) {
                            glCheck(glEnableVertexAttribArray(6));
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboLayer));
                            glCheck(glVertexAttribIPointer(6, 1, GL_UNSIGNED_INT, sizeof(GLuint), (GLvoid*) 0));
                            glCheck(glDisableVertexAttribArray(6));
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        }
                        if (vertexBuffer.vboSpecular != 0) {
                            glCheck(glEnableVertexAttribArray(7));
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboSpecular));
                            glCheck(glVertexAttribPointer(7, 2, GL_FLOAT,GL_FALSE,sizeof(sf::Vector2f),(GLvoid*) 0));
                            glCheck(glDisableVertexAttribArray(7));
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        }
                        if (vertexBuffer.vboLightInfos != 0) {
                            glCheck(glEnableVertexAttribArray(8));
                            glCheck(glEnableVertexAttribArray(9));
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboLightInfos));
                            glCheck(glVertexAttribPointer(8, 4,GL_FLOAT,GL_FALSE,sizeof(VertexBuffer::LightInfos), (GLvoid*) 0));
                            glCheck(glVertexAttribPointer(9, 4, GL_UNSIGNED_BYTE,GL_TRUE,sizeof(VertexBuffer::LightInfos), (GLvoid*) 16));
                            glCheck(glDisableVertexAttribArray(8));
                            glCheck(glDisableVertexAttribArray(9));
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        }
                        //std::cout<<"vbo texture index buffer : "<<vertexBuffer.vboTextureIndexesBuffer<<std::endl;
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboTextureIndexesBuffer));

                        glCheck(glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT/*,GL_FALSE*/,sizeof(GLuint),(GLvoid*) 0));
                        glCheck(glDisableVertexAttribArray(0));
                        glCheck(glDisableVertexAttribArray(1));
                        glCheck(glDisableVertexAttribArray(2));
                        glCheck(glDisableVertexAttribArray(3));
                        glCheck(glDisableVertexAttribArray(4));
                        if (vertexBuffer.vboMaterialType != 0) {
                            glCheck(glDisableVertexAttribArray(5));
                        }
                        if (vertexBuffer.vboLayer != 0) {
                            glCheck(glDisableVertexAttribArray(6));
                        }
                        if (vertexBuffer.vboSpecular != 0) {
                            glCheck(glDisableVertexAttribArray(7));
                        }
                        if (vertexBuffer.vboLightInfos != 0) {
                            glCheck(glDisableVertexAttribArray(8));
                            glCheck(glDisableVertexAttribArray(9));
                        }

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
                    if (vertexBuffer.vboMaterialType != 0) {
                        glCheck(glEnableVertexAttribArray(5));
                    }
                    if (vertexBuffer.vboLayer != 0) {
                        glCheck(glEnableVertexAttribArray(6));
                    }
                    if (vertexBuffer.vboSpecular != 0) {
                        glCheck(glEnableVertexAttribArray(7));
                    }
                    if (vertexBuffer.vboLightInfos != 0) {
                        glCheck(glEnableVertexAttribArray(8));
                        glCheck(glEnableVertexAttribArray(9));
                    }
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
                    if (vertexBuffer.vboMaterialType != 0) {
                        glCheck(glDisableVertexAttribArray(5));
                    }
                    if (vertexBuffer.vboLayer != 0) {
                        glCheck(glDisableVertexAttribArray(6));
                    }
                    if (vertexBuffer.vboSpecular != 0) {
                        glCheck(glDisableVertexAttribArray(7));
                    }
                    if (vertexBuffer.vboLightInfos != 0) {
                        glCheck(glDisableVertexAttribArray(8));
                        glCheck(glDisableVertexAttribArray(9));
                    }
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
                //glCheck(glClipControl(GL_UPPER_LEFT, GL_NEGATIVE_ONE_TO_ONE));

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
