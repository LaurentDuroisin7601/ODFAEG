#include "../../../include/odfaeg/Graphics/renderTarget.h"
#include "../../../include/odfaeg/Graphics/drawable.h"
#include "../../../include/odfaeg/Graphics/batcher.h"
//#include "../../../include/odfaeg/Graphics/glExtensions.hpp"
#ifndef VULKAN
#include <GL/glew.h>
#include <ODFAEG/OpenGL.hpp>
#include "glCheck.h"
#endif

namespace
{
    #ifndef VULKAN


    // Convert an odfaeg::graphic::BlendMode::Factor constant to the corresponding OpenGL constant.
    std::uint32_t factorToGlConstant(odfaeg::graphic::BlendMode::Factor blendFactor)
    {
        switch (blendFactor)
        {
            default:
            case odfaeg::graphic::BlendMode::Zero:             return GL_ZERO;
            case odfaeg::graphic::BlendMode::One:              return GL_ONE;
            case odfaeg::graphic::BlendMode::SrcColor:         return GL_SRC_COLOR;
            case odfaeg::graphic::BlendMode::OneMinusSrcColor: return GL_ONE_MINUS_SRC_COLOR;
            case odfaeg::graphic::BlendMode::DstColor:         return GL_DST_COLOR;
            case odfaeg::graphic::BlendMode::OneMinusDstColor: return GL_ONE_MINUS_DST_COLOR;
            case odfaeg::graphic::BlendMode::SrcAlpha:         return GL_SRC_ALPHA;
            case odfaeg::graphic::BlendMode::OneMinusSrcAlpha: return GL_ONE_MINUS_SRC_ALPHA;
            case odfaeg::graphic::BlendMode::DstAlpha:         return GL_DST_ALPHA;
            case odfaeg::graphic::BlendMode::OneMinusDstAlpha: return GL_ONE_MINUS_DST_ALPHA;
        }
    }


    // Convert an odfaeg::graphic::BlendMode::BlendEquation constant to the corresponding OpenGL constant.
    std::uint32_t equationToGlConstant(odfaeg::graphic::BlendMode::Equation blendEquation)
    {
        switch (blendEquation)
        {
            default:
            case odfaeg::graphic::BlendMode::Add:             return GL_FUNC_ADD_EXT;
            case odfaeg::graphic::BlendMode::Subtract:        return GL_FUNC_SUBTRACT_EXT;
        }
    }
    #else
    VkBlendFactor factorToVkConstant(odfaeg::graphic::BlendMode::Factor blendFactor) {
        switch(blendFactor) {
            default:
            case odfaeg::graphic::BlendMode::Zero:             return VK_BLEND_FACTOR_ZERO;
            case odfaeg::graphic::BlendMode::One:              return VK_BLEND_FACTOR_ONE;
            case odfaeg::graphic::BlendMode::SrcColor:         return VK_BLEND_FACTOR_SRC_COLOR;
            case odfaeg::graphic::BlendMode::OneMinusSrcColor: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
            case odfaeg::graphic::BlendMode::DstColor:         return VK_BLEND_FACTOR_DST_COLOR;
            case odfaeg::graphic::BlendMode::OneMinusDstColor: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
            case odfaeg::graphic::BlendMode::SrcAlpha:         return VK_BLEND_FACTOR_SRC_ALPHA;
            case odfaeg::graphic::BlendMode::OneMinusSrcAlpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            case odfaeg::graphic::BlendMode::DstAlpha:         return VK_BLEND_FACTOR_DST_ALPHA;
            case odfaeg::graphic::BlendMode::OneMinusDstAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        }
    }
    VkBlendOp equationToVkConstant(odfaeg::graphic::BlendMode::Equation blendEquation) {
        switch (blendEquation)
        {
            default:
            case odfaeg::graphic::BlendMode::Add:             return VK_BLEND_OP_ADD;
            case odfaeg::graphic::BlendMode::Subtract:        return VK_BLEND_OP_SUBTRACT;
        }
    }
    #endif // VULKAN
}
namespace odfaeg {
    namespace graphic {

        #ifdef VULKAN
        unsigned int RenderTarget::nbRenderTargets = 0;

        RenderTarget::RenderTarget(window::Device& vkDevice, bool useSecondaryCmds) : vkDevice(vkDevice), defaultShader(vkDevice),
        m_defaultView(), m_view(), id(nbRenderTargets), depthTestEnabled(false), stencilTestEnabled(false), depthTexture(nullptr),
        useSecondaryCmds(useSecondaryCmds) {
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


            m_defaultView = View (static_cast<float>(getSize().x()), static_cast<float>(getSize().y()), -static_cast<float>(getSize().y()) - 200, static_cast<float>(getSize().y())+200);
            m_defaultView.reset(physic::BoundingBox(0, 0, -static_cast<float>(getSize().y()) - 200,static_cast<float>(getSize().x()), static_cast<float>(getSize().y()),static_cast<float>(getSize().y())+200));
            m_view = m_defaultView;
            const std::string defaultVertexShader = R"(#version 450
                                                       #extension GL_EXT_debug_printf : enable
                                                       struct DrawableData {
                                                            mat4 projMatrix;
                                                            mat4 viewMatrix;
                                                            mat4 modelMatrix;
                                                            vec2 uvScale;
                                                            vec2 uvOffset;
                                                            uint textureID;
                                                            uint pad[3];
                                                        };
                                                        layout(location = 0) in vec3 inPosition;
                                                        layout(location = 1) in vec4 inColor;
                                                        layout(location = 2) in vec2 inTexCoord;
                                                        layout(location = 3) in vec3 normals;
                                                        layout(location = 4) in int drawableDataID;

                                                        layout(location = 0) out vec4 fragColor;
                                                        layout(location = 1) out vec2 fragTexCoord;
                                                        layout(location = 2) out vec3 normal;
                                                        layout(location = 3) out uint outTextureID;

                                                        layout (std430, set = 0, binding = 0) buffer DrawableDataSSBO {
                                                            DrawableData drawableData[];
                                                        };


                                                        void main() {
                                                             //debugPrintfEXT("matrix: %v4f \n %v4f \n %v4f \n %v4f", drawableData[drawableDataID].projMatrix[0], drawableData[drawableDataID].projMatrix[1], drawableData[drawableDataID].projMatrix[2], drawableData[drawableDataID].projMatrix[3]);
                                                             //debugPrintfEXT("drawable id : %i", drawableDataID);
                                                             gl_PointSize = 2.0f;

                                                             gl_Position =  vec4(inPosition, 1.0) * drawableData[drawableDataID].modelMatrix * drawableData[drawableDataID].viewMatrix * drawableData[drawableDataID].projMatrix;
                                                             //debugPrintfEXT("position : %v4f", gl_Position);
                                                             fragColor = inColor;
                                                             fragTexCoord = inTexCoord * drawableData[drawableDataID].uvScale + drawableData[drawableDataID].uvOffset;
                                                             normal = normals;
                                                             outTextureID = drawableData[drawableDataID].textureID;
                                                        }
                                                        )";
             const std::string defaultFragmentShader = R"(#version 450
                                                          #extension GL_ARB_separate_shader_objects : enable
                                                          #extension GL_EXT_nonuniform_qualifier : enable
                                                          #extension GL_EXT_debug_printf : enable
                                                          const uint MAX_TEXTURES = 128;
                                                          layout(set = 0, binding = 1) uniform sampler2D textures[MAX_TEXTURES];
                                                          layout(location = 0) in vec4 fragColor;
                                                          layout(location = 1) in vec2 fragTexCoord;
                                                          layout(location = 2) in vec3 normal;
                                                          layout(location = 3) in flat uint inTextureID;
                                                          layout(location = 0) out vec4 outColor;
                                                          void main() {
                                                             //debugPrintfEXT("fragment shader");
                                                             outColor = (inTextureID == 0) ? fragColor : texture(textures[inTextureID-1], fragTexCoord) * fragColor;
                                                          })";
             if (!defaultShader.loadFromMemory(defaultVertexShader, defaultFragmentShader)) {
                  throw core::Erreur (0, "Failed to load default shader", 1);
             }
             ////////std::cout<<"size : "<<Shader::getNbShaders()<<std::endl;

             createCommandPool();
             createCommandBuffers();
             for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                 VertexBuffer vb(vkDevice);
                 vb.setPrimitiveType(static_cast<PrimitiveType>(i));

                 vertexBuffer.push_back(std::move(vb));
             }



             vkCmdPushDescriptorSetKHR = (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(vkDevice.getDevice(), "vkCmdPushDescriptorSetKHR");
             if (!vkCmdPushDescriptorSetKHR) {
                throw core::Erreur(0, "Could not get a valid function pointer for vkCmdPushDescriptorSetKHR", 1);
             }
             for (unsigned int i = 0; i < scissors.size(); i++) {
                scissors[i].offset = {0, 0};
                scissors[i].extent = getSwapchainExtents();
                viewports[i].x = m_view.getViewport().getPosition().x();
                viewports[i].y = m_view.getViewport().getPosition().y();
                viewports[i].width = m_view.getViewport().getSize().x();
                viewports[i].height = m_view.getViewport().getSize().y();
                viewports[i].minDepth = 0.0f;
                viewports[i].maxDepth = 1.0f;
             }
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
        void RenderTarget::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandBuffer cmd) {
            //std::cout<<"opy buffers"<<std::endl;
            if (srcBuffer != nullptr && dstBuffer != nullptr) {
                VkBufferCopy copyRegion{};
                copyRegion.size = size;
                vkCmdCopyBuffer(cmd, srcBuffer, dstBuffer, 1, &copyRegion);
            }
        }
        std::array<VkRect2D, 2>& RenderTarget::getScissors() {
            return scissors;
        }
        void RenderTarget::createDescriptorsAndPipelines() {
             RenderStates states;
             states.shader = &defaultShader;
             createDescriptorPool(states);
             createDescriptorSetLayout(states);
             allocateDescriptorSets(states);


             BlendMode none = BlendNone;
             BlendMode alpha = BlendAlpha;
             BlendMode add = BlendAdd;
             BlendMode multiply = BlendMultiply;
             std::vector<BlendMode> blendModes;
             blendModes.push_back(none);
             blendModes.push_back(alpha);
             blendModes.push_back(add);
             blendModes.push_back(multiply);
             none.updateIds();
             for (unsigned int b = 0; b < blendModes.size(); b++) {
                 states.blendMode = blendModes[b];
                 /*////std::cout<<"blendmode id : "<<states.blendMode.id<<std::endl;
                 system("PAUSE");*/
                 states.shader = &defaultShader;
                 for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes - 1; i++) {
                    createGraphicPipeline(static_cast<PrimitiveType>(i), states, 0, NB_DEPTH_STENCIL);
                 }
            }
            depthTestEnabled = true;
            if ((Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders() > depthStencil.size()) {
                depthStencil.resize((Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders());
            }
            for (unsigned int i = 0; i < (Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders(); i++) {
                depthStencil[i].resize(1);
            }
            for (unsigned int i = 0; i < (Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders(); i++) {
                for (unsigned int j = 0; j < 1; j++) {
                    if (NB_DEPTH_STENCIL*none.nbBlendModes > pipelineLayoutInfo[i][j].size()) {
                        depthStencil[i][j].resize(NB_DEPTH_STENCIL*none.nbBlendModes);
                    }
                }
            }
            for (unsigned int b = 0; b < blendModes.size(); b++) {
                 states.blendMode = blendModes[b];
                 /*////std::cout<<"blendmode id : "<<states.blendMode.id<<std::endl;
                 system("PAUSE");*/
                 states.shader = &defaultShader;
                 for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes - 1; i++) {
                    depthStencil[defaultShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][1*none.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
                    depthStencil[defaultShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][1*none.nbBlendModes+states.blendMode.id].depthWriteEnable = VK_TRUE;
                    createGraphicPipeline(static_cast<PrimitiveType>(i), states, 1, NB_DEPTH_STENCIL);
                 }
            }
            depthTestEnabled = false;
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
         math::Vec4f RenderTarget::mapPixelToCoords(math::Vec4f point)
        {
            return mapPixelToCoords(point, getView());
        }


        math::Vec4f RenderTarget::mapPixelToCoords(math::Vec4f point, View& view)
        {
            point[3] = 1;
            ViewportMatrix vpm;
            vpm.setViewport(math::Vec3f(view.getViewport().getPosition().x(), view.getViewport().getPosition().y(), 0)
                                        ,math::Vec3f(view.getViewport().getWidth(), view.getViewport().getHeight(), 1));
            math::Vec4f coords = vpm.toNormalizedCoordinates(point);
            coords = view.getProjMatrix().unProject(coords);
            coords = coords.normalizeToVec3();
            coords = view.getViewMatrix().inverseTransform(coords);
            return coords;
        }

        math::Vec4f RenderTarget::mapCoordsToPixel(math::Vec4f point)
        {
            return mapCoordsToPixel(point, getView());
        }


        math::Vec4f RenderTarget::mapCoordsToPixel(math::Vec4f point, View& view) {
            point[3] = 1;
            ViewportMatrix vpm;
            vpm.setViewport(math::Vec3f(view.getViewport().getPosition().x(), view.getViewport().getPosition().y(), 0),
            math::Vec3f(view.getViewport().getWidth(), view.getViewport().getHeight(), 1));
            //////std::cout<<"viewport matrix : "<<vpm.getMatrix()<<std::endl;
            math::Vec4f coords = view.getViewMatrix().transform(point);
            //////std::cout<<"view matrix : "<<view.getViewMatrix().getMatrix()<<std::endl<<coords<<std::endl;
            coords = view.getProjMatrix().project(coords);
            //////std::cout<<"projection matrix : "<<view.getProjMatrix().getMatrix()<<std::endl<<coords<<std::endl;
            coords = coords.normalizeToVec3();
            //////std::cout<<"n coords : "<<coords<<std::endl;
            coords = vpm.toViewportCoordinates(coords);
            //////std::cout<<"vp coords : "<<coords<<std::endl;
            return coords;
        }
        void RenderTarget::draw(Drawable& drawable, RenderStates states)
        {
            ////std::cout<<"check ubo drawable"<<std::endl;
            ////std::cout<<"draw drawable"<<std::endl;
            drawable.draw(*this, states);
            ////std::cout<<"drawable drawn"<<std::endl;
        }
        void RenderTarget::draw(const Vertex* vertices, unsigned int vertexCount, PrimitiveType type,
                      RenderStates states) {
             ////std::cout<<"draw vertices"<<std::endl;
             if (states.shader == nullptr) {
                states.shader = &defaultShader;
             }
             DrawableData datas;
             datas.projMatrix = toVulkanMatrix(m_view.getProjMatrix().getMatrix());
             datas.viewMatrix = toVulkanMatrix(m_view.getViewMatrix().getMatrix());
             datas.modelMatrix = toVulkanMatrix(states.transform.getMatrix());
             datas.textureID = (states.texture != nullptr) ? states.texture->getId() : 0;
             datas.uvScale = (states.texture != nullptr) ? math::Vec2f(1.f / states.texture->getSize().x(), 1.f / states.texture->getSize().y()) : math::Vec2f(0, 0);
             datas.uvOffset = math::Vec2f(0, 0);
             drawableData.push_back(datas);
             if (type == Quads) {
                type = Triangles;
                for (unsigned int i = 0; i < vertexCount; i++) {
                    vertexBuffer[type].append(vertices[i]);
                    vertexBuffer[type][vertexBuffer[type].getVertexCount()-1].padding = drawableData.size() - 1;
                }
                vertexBuffer[type].addIndex(vertexBuffer[type].getVertexCount()-4);
                vertexBuffer[type].addIndex(vertexBuffer[type].getVertexCount()-3);
                vertexBuffer[type].addIndex(vertexBuffer[type].getVertexCount()-2);
                vertexBuffer[type].addIndex(vertexBuffer[type].getVertexCount()-4);
                vertexBuffer[type].addIndex(vertexBuffer[type].getVertexCount()-2);
                vertexBuffer[type].addIndex(vertexBuffer[type].getVertexCount()-1);

             } else {
                 for (unsigned int i = 0; i < vertexCount; i++) {
                    vertexBuffer[type].append(vertices[i]);
                    vertexBuffer[type].addIndex(vertexBuffer[type].getVertexCount()-1);
                    vertexBuffer[type][vertexBuffer[type].getVertexCount()-1].padding = drawableData.size() - 1;
                 }
             }
             if (type == TriangleFan || type == TriangleStrip || type == LineStrip)
                vertexBuffer[type].addIndex(0xFFFF);


             VkDeviceSize bufferSize = sizeof(DrawableData) * drawableData.size();
             if (bufferSize > maxDrawableDataSize[getCurrentFrame()]) {

                if (stagingDrawableData[getCurrentFrame()] != nullptr) {
                    vkDestroyBuffer(vkDevice.getDevice(), stagingDrawableData[getCurrentFrame()], nullptr);
                    vkFreeMemory(vkDevice.getDevice(), stagingDrawableDataMemory[getCurrentFrame()], nullptr);
                }

                createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingDrawableData[getCurrentFrame()], stagingDrawableDataMemory[getCurrentFrame()]);

                if (drawableDataSSBO[getCurrentFrame()] != nullptr) {
                    vkDestroyBuffer(vkDevice.getDevice(), drawableDataSSBO[getCurrentFrame()], nullptr);
                    vkFreeMemory(vkDevice.getDevice(), drawableDataSSBOMemory[getCurrentFrame()], nullptr);
                }

                createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, drawableDataSSBO[getCurrentFrame()], drawableDataSSBOMemory[getCurrentFrame()]);
                maxDrawableDataSize[getCurrentFrame()] = bufferSize;
                updateDescriptorSets(states);
             }

             if (useSecondaryCmds)
                beginRecordSecondaryCommandBuffers();
             else {
                beginRecordCommandBuffers();
                if (!firstDraw && isFirstSubmit()) {
                    registerClearCommands(clearColor);
                }
             }

             if (secondaryCommandsOnRecordedState[getCurrentFrame()]) {
                copyBuffer(stagingDrawableData[getCurrentFrame()], drawableDataSSBO[getCurrentFrame()], bufferSize, secondaryCommandBuffers[getCurrentFrame()]);
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vertexBuffer[p].getVertexCount() > 0)
                        vertexBuffer[p].update(getCurrentFrame(), secondaryCommandBuffers[getCurrentFrame()]);
                }
             } else if(commandsOnRecordedState[getCurrentFrame()]) {
                copyBuffer(stagingDrawableData[getCurrentFrame()], drawableDataSSBO[getCurrentFrame()], bufferSize, commandBuffers[getCurrentFrame()]);
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vertexBuffer[p].getVertexCount() > 0)
                        vertexBuffer[p].update(getCurrentFrame(), commandBuffers[getCurrentFrame()]);
                }
             }

             if (!useSecondaryCmds)
                beginRenderPass();

             if (secondaryCommandsOnRecordedState[getCurrentFrame()]) {
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vertexBuffer[p].getVertexCount() > 0)
                        recordCommandBuffers(secondaryCommandBuffers[getCurrentFrame()], vertexBuffer[p], static_cast<PrimitiveType>(p), states);
                }
             } else if(commandsOnRecordedState[getCurrentFrame()]) {
                 for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vertexBuffer[p].getVertexCount() > 0)
                        recordCommandBuffers(commandBuffers[getCurrentFrame()], vertexBuffer[p], static_cast<PrimitiveType>(p), states);
                 }
             }
             if (!useSecondaryCmds)
                endRenderPass();
             if (secondaryCommandsOnRecordedState[getCurrentFrame()])
                endRecordSecondaryCommandBuffers();
             else if (commandsOnRecordedState[getCurrentFrame()])
                endRecordCommandBuffers();
            firstDraw = false;
        }
        void RenderTarget::drawVertexBuffer(VertexBuffer& vb, RenderStates states) {


             if (secondaryCommandsOnRecordedState[getCurrentFrame()])
                recordCommandBuffers(secondaryCommandBuffers[getCurrentFrame()], vb, vb.getPrimitiveType(), states);
             else if(commandsOnRecordedState[getCurrentFrame()])
                recordCommandBuffers(commandBuffers[getCurrentFrame()], vb, vb.getPrimitiveType(), states);
             /*//////std::cout<<"drawn"<<std::endl;
             system("PAUSE");*/

        }
        void RenderTarget::drawIndirectCount(VkCommandBuffer& cmd, unsigned int i, unsigned int nbIndirectCommands, unsigned int stride, VertexBuffer& vertexBuffer, VkBuffer vboIndirect, VkBuffer vboCount, unsigned int depthStencilId, RenderStates states) {
            Shader* shader = const_cast<Shader*>(states.shader);
            ////////std::cout<<"draw indirect depth stencil id :"<<depthStencilId<<std::endl;
            unsigned int descriptorId = shader->getId();
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+vertexBuffer.getPrimitiveType()][id][depthStencilId]);
            VkBuffer vertexBuffers[] = {vertexBuffer.getVertexBuffer(getCurrentFrame())};
            VkDeviceSize offsets[] = {0, 0};
            vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+vertexBuffer.getPrimitiveType()][id][depthStencilId], 0, 1, &descriptorSets[descriptorId][getCurrentFrame()], 0, nullptr);

            applyViewportAndScissor(cmd);
            if(vertexBuffer.getIndicesSize() > 0) {
                vkCmdBindIndexBuffer(cmd, vertexBuffer.getIndexBuffer(getCurrentFrame()), 0, VK_INDEX_TYPE_UINT16);
            }
            if(vertexBuffer.getIndicesSize() > 0) {
                vkCmdDrawIndexedIndirectCount(cmd, vboIndirect, 0, vboCount, 0, nbIndirectCommands, stride);
            } else {
                vkCmdDrawIndirectCount(cmd, vboIndirect, 0, vboCount, 0, nbIndirectCommands, stride);
            }
        }
        void RenderTarget::drawIndirect(VkCommandBuffer& cmd, unsigned int currentFrame, unsigned int nbIndirectCommands, unsigned int stride, VertexBuffer& vertexBuffer, VkBuffer vboIndirect, unsigned int depthStencilId, RenderStates states, unsigned int descriptorSetCustomID, unsigned int vertexOffset, unsigned int drawCommandOffset, std::vector<unsigned int> dynamicBufferOffsets, unsigned int indexOffset, unsigned int id) {
            states.blendMode.updateIds();
            Shader* shader = const_cast<Shader*>(states.shader);
            unsigned int blendModeId = states.blendMode.id;
            unsigned int nbBlendMode = states.blendMode.nbBlendModes;
            ////////std::cout<<"draw indirect depth stencil id :"<<depthStencilId<<std::endl;
            unsigned int descriptorId = descriptorSetCustomID * shader->getNbShaders() + shader->getId();
            /*if(shader->getId()* (Batcher::nbPrimitiveTypes - 1)+vertexBuffer.getPrimitiveType() >= graphicsPipeline.size()
               || id >= graphicsPipeline[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+vertexBuffer.getPrimitiveType()].size()
               || depthStencilId*nbBlendMode+blendModeId >= graphicsPipeline[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+vertexBuffer.getPrimitiveType()][id].size()
               || graphicsPipeline[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+vertexBuffer.getPrimitiveType()][id][depthStencilId*nbBlendMode+blendModeId] == nullptr)
                std::cout<<"erreur"<<std::endl;*/
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+vertexBuffer.getPrimitiveType()][id][depthStencilId*nbBlendMode+blendModeId]);

            VkBuffer vertexBuffers[] = {vertexBuffer.getVertexBuffer(currentFrame)};
            VkDeviceSize offsets[] = {0, 0};
            vkCmdBindVertexBuffers(cmd, vertexOffset, 1, vertexBuffers, offsets);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+vertexBuffer.getPrimitiveType()][id][depthStencilId*nbBlendMode+blendModeId], 0, 1, &descriptorSets[descriptorId][currentFrame], dynamicBufferOffsets.size(), dynamicBufferOffsets.data());
            ////std::cout<<"pipeline id draw : "<<depthStencilId*nbBlendMode+blendModeId<<std::endl;
            /*if (depthStencil[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+vertexBuffer.getPrimitiveType()][id][depthStencilId*nbBlendMode+blendModeId].back.compareOp == VK_COMPARE_OP_NOT_EQUAL)
                //std::cout<<"not equale"<<std::endl;*/
            applyViewportAndScissor(cmd);
            if(vertexBuffer.getIndicesSize() > 0) {
                vkCmdBindIndexBuffer(cmd, vertexBuffer.getIndexBuffer(currentFrame), indexOffset, VK_INDEX_TYPE_UINT16);
            }
            if(vertexBuffer.getIndicesSize() > 0) {
                //std::cout<<"draw indexed indirect"<<std::endl;
                vkCmdDrawIndexedIndirect(cmd, vboIndirect, drawCommandOffset, nbIndirectCommands, stride);
            } else {
                vkCmdDrawIndirect(cmd, vboIndirect, drawCommandOffset, nbIndirectCommands, stride);
            }
        }
        void RenderTarget::drawVertexBuffer(VkCommandBuffer& cmd, unsigned int currentFrame, VertexBuffer& vertexBuffer, unsigned int depthStencilId, RenderStates states, std::vector<unsigned int> dynamicOffsets, unsigned int instanceCount, unsigned int customDescriptorSetId, unsigned int id) {
            ////////std::cout<<"vertex stencil id :"<<depthStencilId<<std::endl;
            states.blendMode.updateIds();
            Shader* shader = const_cast<Shader*>(states.shader);
            unsigned int blendModeId = states.blendMode.id;
            unsigned int nbBlendMode = states.blendMode.nbBlendModes;
            unsigned int descriptorId = customDescriptorSetId * shader->getNbShaders() + shader->getId();
            /*if(shader->getId()* (Batcher::nbPrimitiveTypes - 1)+vertexBuffer.getPrimitiveType() >= graphicsPipeline.size()
               || id >= graphicsPipeline[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+vertexBuffer.getPrimitiveType()].size()
               || depthStencilId*nbBlendMode+blendModeId >= graphicsPipeline[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+vertexBuffer.getPrimitiveType()][id].size()
               || graphicsPipeline[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+vertexBuffer.getPrimitiveType()][id][depthStencilId*nbBlendMode+blendModeId] == nullptr)
                std::cout<<"erreur"<<std::endl;*/
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+vertexBuffer.getPrimitiveType()][id][depthStencilId*nbBlendMode+blendModeId]);
            VkBuffer vertexBuffers[] = {vertexBuffer.getVertexBuffer(currentFrame)};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
            /*//////std::cout<<"draw pipeline id : "<<shader->getId() * (Batcher::nbPrimitiveTypes - 1)+vertexBuffer.getPrimitiveType()<<","<<id<<","<<depthStencilId<<std::endl;
            system("PAUSE");*/
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+vertexBuffer.getPrimitiveType()][id][depthStencilId*nbBlendMode+blendModeId], 0, 1, &descriptorSets[descriptorId][currentFrame], dynamicOffsets.size(), dynamicOffsets.data());

            applyViewportAndScissor(cmd);
            if(vertexBuffer.getIndicesSize() > 0) {
                vkCmdBindIndexBuffer(cmd, vertexBuffer.getIndexBuffer(currentFrame), 0, VK_INDEX_TYPE_UINT16);
            }
            if(vertexBuffer.getIndicesSize() > 0) {
                vkCmdDrawIndexed(cmd, static_cast<uint32_t>(vertexBuffer.getIndicesSize()), instanceCount, 0, 0, 0);
            } else {
                vkCmdDraw(cmd, static_cast<uint32_t>(vertexBuffer.getSize()), instanceCount, 0, 0);
            }
        }


        void RenderTarget::createDescriptorSetLayout(RenderStates states) {
            Shader* shader = const_cast<Shader*>(states.shader);
            if (shader->getNbShaders() > descriptorSetLayout.size())
                descriptorSetLayout.resize(shader->getNbShaders());

            std::vector<VkDescriptorBindingFlags> bindingFlags(2, 0); // 2 binding, flags par défaut à 0
            bindingFlags[1] = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                  VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT; // seulement pour sampler[]

            VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
            bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
            bindingFlagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
            bindingFlagsInfo.pBindingFlags = bindingFlags.data();

            VkDescriptorSetLayoutBinding ssboLayoutBinding{};
            ssboLayoutBinding.binding = 0;
            ssboLayoutBinding.descriptorCount = 1;
            ssboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            ssboLayoutBinding.pImmutableSamplers = nullptr;
            ssboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            VkDescriptorSetLayoutBinding samplerLayoutBinding{};
            samplerLayoutBinding.binding = 1;
            samplerLayoutBinding.descriptorCount = MAX_TEXTURES;
            samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            samplerLayoutBinding.pImmutableSamplers = nullptr;
            samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            std::array<VkDescriptorSetLayoutBinding, 2> bindings = {ssboLayoutBinding, samplerLayoutBinding};

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.pNext = &bindingFlagsInfo;
            layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
            layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());;
            layoutInfo.pBindings = bindings.data();

            if (vkCreateDescriptorSetLayout(vkDevice.getDevice(), &layoutInfo, nullptr, &descriptorSetLayout[shader->getId()]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor set layout!");
            }
        }
        void RenderTarget::allocateDescriptorSets(RenderStates states) {
            Shader* shader = const_cast<Shader*>(states.shader);
            if (shader->getNbShaders() > descriptorSets.size()) {
                descriptorSets.resize(shader->getNbShaders());

                for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                    descriptorSets[i].resize(MAX_FRAMES_IN_FLIGHT);
                }
            }
            unsigned int descriptorId = shader->getId();
            std::vector<uint32_t> variableCounts(MAX_FRAMES_IN_FLIGHT, MAX_TEXTURES);

            VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
            variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
            variableCountInfo.descriptorSetCount = static_cast<uint32_t>(variableCounts.size());
            variableCountInfo.pDescriptorCounts = variableCounts.data();
            std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout[descriptorId]);
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.pNext = &variableCountInfo;
            allocInfo.descriptorPool = descriptorPool[descriptorId];
            allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
            allocInfo.pSetLayouts = layouts.data();
            if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, descriptorSets[descriptorId].data()) != VK_SUCCESS) {
                throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
            }
        }
        void RenderTarget::createDescriptorPool(RenderStates states) {
            Shader* shader = const_cast<Shader*>(states.shader);
            if (shader->getNbShaders() > descriptorPool.size())
                descriptorPool.resize(shader->getNbShaders()*nbRenderTargets);
            unsigned int descriptorId = shader->getId();
            std::array<VkDescriptorPoolSize, 2> poolSizes{};
            poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            poolSizes[0].descriptorCount = static_cast<uint32_t>(getMaxFramesInFlight());
            poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSizes[1].descriptorCount = MAX_TEXTURES * static_cast<uint32_t>(getMaxFramesInFlight());


            VkDescriptorPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            poolInfo.pPoolSizes = poolSizes.data();
            poolInfo.maxSets = static_cast<uint32_t>(getMaxFramesInFlight());
            if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool[descriptorId]) != VK_SUCCESS) {
                throw std::runtime_error("echec de la creation de la pool de descripteurs!");
            }
        }
        void RenderTarget::updateDescriptorSets(RenderStates states) {
            Shader* shader = const_cast<Shader*>(states.shader);
            std::vector<Texture*> allTextures = Texture::getAllTextures();
            std::vector<VkDescriptorImageInfo> imageInfos{};
            imageInfos.resize (allTextures.size());
            for (unsigned int j = 0; j < imageInfos.size(); j++) {
                imageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfos[j].imageView = const_cast<Texture*>(allTextures[j])->getImageView();
                imageInfos[j].sampler = const_cast<Texture*>(allTextures[j])->getSampler();
            }
            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
            VkDescriptorBufferInfo bufferInfo;
            bufferInfo.buffer = drawableDataSSBO[getCurrentFrame()];
            bufferInfo.offset = 0;
            bufferInfo.range = drawableData.size() * sizeof(DrawableData);
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSets[shader->getId()][getCurrentFrame()];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSets[shader->getId()][getCurrentFrame()];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = imageInfos.size();
            descriptorWrites[1].pImageInfo = imageInfos.data();

            vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
        void RenderTarget::createGraphicPipeline(PrimitiveType type,
                      RenderStates states, unsigned int depthStencilId, unsigned int nbDepthStencil, unsigned int id) {
            Shader* shader = const_cast<Shader*>(states.shader);
            states.blendMode.updateIds();
            //system("PAUSE");

            unsigned int blendModeId = states.blendMode.id;
            unsigned int nbBlendMode = states.blendMode.nbBlendModes;
            if ((Batcher::nbPrimitiveTypes - 1) * shader->getNbShaders() > graphicsPipeline.size()) {
                graphicsPipeline.resize((Batcher::nbPrimitiveTypes - 1) * shader->getNbShaders());
                pipelineLayoutInfo.resize((Batcher::nbPrimitiveTypes - 1) * shader->getNbShaders());
                pipelineLayout.resize((Batcher::nbPrimitiveTypes - 1) * shader->getNbShaders());
                depthStencil.resize((Batcher::nbPrimitiveTypes - 1) * shader->getNbShaders());
            }
            for (unsigned int i = 0; i < (Batcher::nbPrimitiveTypes - 1) * shader->getNbShaders(); i++) {
                if ((id + 1) > graphicsPipeline[i].size()) {
                    graphicsPipeline[i].resize(id+1);
                    pipelineLayoutInfo[i].resize(id+1);
                    pipelineLayout[i].resize(id+1);
                    depthStencil[i].resize(id+1);
                }
            }
            for (unsigned int i = 0; i < (Batcher::nbPrimitiveTypes - 1) * shader->getNbShaders(); i++) {
                for (unsigned int j = 0; j < (id + 1); j++) {
                    if (nbDepthStencil * nbBlendMode > graphicsPipeline[i][j].size()) {
                        graphicsPipeline[i][j].resize(nbDepthStencil*nbBlendMode);
                        pipelineLayoutInfo[i][j].resize(nbDepthStencil*nbBlendMode);
                        pipelineLayout[i][j].resize(nbDepthStencil*nbBlendMode);
                        depthStencil[i][j].resize(nbDepthStencil*nbBlendMode);
                    }
                }
            }

            unsigned int descriptorId = shader->getId();
            //std::cout<<"descriptor set layout id : "<<descriptorId<<std::endl;


            ////////std::cout<<"rt dl size : "<<descriptorSetLayout.size()<<std::endl;
            /*//////std::cout<<"descriptor id : "<<descriptorId<<std::endl;*/
            ////////std::cout<<"pipeline id : "<<pipelineId<<std::endl;


            /*for (unsigned int i = 0; i < pipelinesId.size(); i++) {
                if (pipelinesId[i] == pipelineId) {
                    //////std::cout<<"Erreur"<<std::endl;
                    system("PAUSE");
                }
            }
            pipelinesId.push_back(pipelineId);*/

            VkShaderModule vertShaderModule;
            VkShaderModule fragShaderModule;
            VkShaderModule geomShaderModule=nullptr;

            if (states.shader == nullptr) {
                defaultShader.createShaderModules();
                vertShaderModule = defaultShader.getVertexShaderModule();
                fragShaderModule = defaultShader.getFragmentShaderModule();
                states.shader = &defaultShader;
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
                geomShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                geomShaderStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
                geomShaderStageInfo.module = geomShaderModule;
                geomShaderStageInfo.pName = "main";

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
            inputAssembly.primitiveRestartEnable =
            (inputAssembly.topology == VK_PRIMITIVE_TOPOLOGY_LINE_STRIP ||
             inputAssembly.topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP ||
             inputAssembly.topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN)
            ? VK_TRUE : VK_FALSE;



            VkPipelineViewportStateCreateInfo viewportState{};
            viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.viewportCount = viewports.size();
            //viewportState.pViewports = &viewport;
            viewportState.scissorCount = scissors.size();
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


            colorBlendAttachment.srcColorBlendFactor = factorToVkConstant(states.blendMode.colorSrcFactor);
            colorBlendAttachment.dstColorBlendFactor = factorToVkConstant(states.blendMode.colorDstFactor);
            colorBlendAttachment.colorBlendOp = equationToVkConstant(states.blendMode.colorEquation);


            colorBlendAttachment.srcAlphaBlendFactor = factorToVkConstant(states.blendMode.alphaSrcFactor);
            colorBlendAttachment.dstAlphaBlendFactor = factorToVkConstant(states.blendMode.alphaDstFactor);
            colorBlendAttachment.alphaBlendOp = equationToVkConstant(states.blendMode.alphaEquation);

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



            pipelineLayoutInfo[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId*nbBlendMode+blendModeId].sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId*nbBlendMode+blendModeId].setLayoutCount = 1;
            pipelineLayoutInfo[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId*nbBlendMode+blendModeId].pSetLayouts = &descriptorSetLayout[descriptorId];

            //std::cout<<"ids : "<<shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type<<","<<id<<","<<depthStencilId*nbBlendMode+blendModeId<<std::endl;
            if (pipelineLayout[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId*nbBlendMode+blendModeId] != nullptr) {
                //std::cout<<"destroy pipeline layout ids : "<<shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type<<","<<id<<","<<depthStencilId*nbBlendMode+blendModeId<<std::endl;
                vkDestroyPipelineLayout(vkDevice.getDevice(), pipelineLayout[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId*nbBlendMode+blendModeId], nullptr);
            }
            /*std::cout<<"create pipeline layout ids : "<<shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type<<","<<id<<","<<depthStencilId*nbBlendMode+blendModeId<<std::endl;
            std::cout<<"sizes : "<<pipelineLayout.size()<<","<<pipelineLayout[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type].size()<<","<<pipelineLayout[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id].size()<<std::endl;*/
            if (vkCreatePipelineLayout(vkDevice.getDevice(), &pipelineLayoutInfo[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId*nbBlendMode+blendModeId], nullptr, &pipelineLayout[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId*nbBlendMode+blendModeId]) != VK_SUCCESS) {

                throw core::Erreur(0, "failed to create pipeline layout!", 1);
            }
            ////////std::cout<<"pipeline layout : "<<pipelineLayout[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId]<<std::endl;

            depthStencil[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId*nbBlendMode+blendModeId].sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depthStencil[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId*nbBlendMode+blendModeId].depthTestEnable = (depthTestEnabled) ? VK_TRUE : VK_FALSE;
            depthStencil[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId*nbBlendMode+blendModeId].depthBoundsTestEnable = VK_FALSE;
            depthStencil[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId*nbBlendMode+blendModeId].minDepthBounds = 0.0f; // Optional
            depthStencil[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId*nbBlendMode+blendModeId].maxDepthBounds = 1.0f; // Optional
            depthStencil[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId*nbBlendMode+blendModeId].stencilTestEnable = (stencilTestEnabled) ? VK_TRUE : VK_FALSE;
            /*if (depthStencil[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId*nbBlendMode+blendModeId].depthTestEnable  == VK_TRUE)
                //std::cout<<"depth test enabled"<<std::endl;*/



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
            pipelineInfo.layout = pipelineLayout[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId*nbBlendMode+blendModeId];
            /*if (getSurface() != VK_NULL_HANDLE)
                //////std::cout<<"render pass ppl rw : "<<getRenderPass()<<std::endl;
            else
                //////std::cout<<"render pass ppl rt : "<<getRenderPass()<<std::endl;*/
            pipelineInfo.renderPass = (depthTestEnabled || stencilTestEnabled) ? getRenderPass(1) : getRenderPass(0);
            pipelineInfo.subpass = 0;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
            pipelineInfo.pDepthStencilState = &depthStencil[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId*nbBlendMode+blendModeId];
            if (graphicsPipeline[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId*nbBlendMode+blendModeId] != nullptr) {
                //std::cout<<"destroy pipeline"<<std::endl;
                vkDestroyPipeline(vkDevice.getDevice(), graphicsPipeline[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId*nbBlendMode+blendModeId], nullptr);

            }
            /*std::cout<<"create graphics pipeline ids : "<<(shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type)<<","<<id<<","<<(depthStencilId*nbBlendMode+blendModeId)<<std::endl;
            std::cout<<"sizes : "<<graphicsPipeline.size()<<","<<graphicsPipeline[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type].size()<<","<<graphicsPipeline[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id].size()<<std::endl;*/
            if (vkCreateGraphicsPipelines(vkDevice.getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+type][id][depthStencilId*nbBlendMode+blendModeId]) != VK_SUCCESS) {
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
            if (vkCreateCommandPool(vkDevice.getDevice(), &poolInfo, nullptr, &secondaryCommandPool) != VK_SUCCESS) {
                throw core::Erreur(0, "échec de la création d'une command pool!", 1);
            }
        }
        void RenderTarget::createCommandBuffers() {
            commandBuffers.resize(getMaxFramesInFlight());
            commandsOnRecordedState.resize(getMaxFramesInFlight(), false);
            secondaryCommandBuffers.resize(getMaxFramesInFlight());
            secondaryCommandsOnRecordedState.resize(getMaxFramesInFlight(), false);

            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = commandPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

            if (vkAllocateCommandBuffers(vkDevice.getDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to allocate command buffers!", 1);
            }
            allocInfo.commandPool = secondaryCommandPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
            allocInfo.commandBufferCount = (uint32_t) secondaryCommandBuffers.size();
            if (vkAllocateCommandBuffers(vkDevice.getDevice(), &allocInfo, secondaryCommandBuffers.data()) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to allocate command buffers!", 1);
            }
            /*if (m_name == "depthBuffer")
                //////std::cout<<"allocate cmd : "<<commandBuffers.size()<<std::endl;*/
        }
        void RenderTarget::beginRecordCommandBuffers() {
            if (!commandsOnRecordedState[getCurrentFrame()]) {
                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                /*if (m_name == "depthBuffer")
                    //////std::cout<<"begin cmd : "<<commandBuffers.size()<<std::endl;*/
                vkResetCommandBuffer(getCommandBuffers()[getCurrentFrame()], 0);
                if (vkBeginCommandBuffer(getCommandBuffers()[getCurrentFrame()], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }
                commandsOnRecordedState[getCurrentFrame()] = true;
            }
        }
        void RenderTarget::beginRecordSecondaryCommandBuffers() {
            if (!secondaryCommandsOnRecordedState[getCurrentFrame()]) {
                useSecondaryCmds = true;
                VkCommandBufferInheritanceInfo inheritanceInfo{};
                inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                inheritanceInfo.renderPass = (depthTestEnabled || stencilTestEnabled) ? getRenderPass(1) : getRenderPass(0);
                inheritanceInfo.framebuffer = (depthTestEnabled || stencilTestEnabled) ? getSwapchainFrameBuffers(1)[getImageIndex()] : getSwapchainFrameBuffers(0)[getImageIndex()];
                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
                beginInfo.pInheritanceInfo = &inheritanceInfo;
                /*if (m_name == "depthBuffer")
                    //////std::cout<<"begin cmd : "<<commandBuffers.size()<<std::endl;*/
                vkResetCommandBuffer(secondaryCommandBuffers[getCurrentFrame()], 0);
                if (vkBeginCommandBuffer(secondaryCommandBuffers[getCurrentFrame()], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }
                secondaryCommandsOnRecordedState[getCurrentFrame()] = true;
            }
        }
        void RenderTarget::endRecordCommandBuffers() {
            if (commandsOnRecordedState[getCurrentFrame()]) {
                if (vkEndCommandBuffer(commandBuffers[getCurrentFrame()]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }
            }
            commandsOnRecordedState[getCurrentFrame()] = false;
        }
        void RenderTarget::endRecordSecondaryCommandBuffers() {
            if (vkEndCommandBuffer(getSecondaryCommandBuffers()[getCurrentFrame()]) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to record command buffer!", 1);
            }
            secondaryCommandsOnRecordedState[getCurrentFrame()] = false;
        }
        void RenderTarget::executeSecondaryCommandBuffers() {
            if (useSecondaryCmds) {
                vkCmdExecuteCommands(getCommandBuffers()[getCurrentFrame()], 1, &getSecondaryCommandBuffers()[getCurrentFrame()]);
                useSecondaryCmds = false;
            }
        }
        void RenderTarget::beginRenderPass() {
            ////std::cout<<"render pass depth ? "<<(depthTestEnabled || stencilTestEnabled)<<std::endl;
            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = (depthTestEnabled || stencilTestEnabled) ? getRenderPass(1) : getRenderPass(0);
            renderPassInfo.framebuffer = (depthTestEnabled || stencilTestEnabled) ? getSwapchainFrameBuffers(1)[getImageIndex()] : getSwapchainFrameBuffers(0)[getImageIndex()];
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = getSwapchainExtents();
            ////////std::cout<<"render pass : "<<(m_view.getViewport().getSize().x == 800 && m_view.getViewport().getSize().y == 800)<<std::endl;

            VkClearValue clrColor = {clearColor.r / 255.f,clearColor.g / 255.f, clearColor.b / 255.f, clearColor.a / 255.f};
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clrColor;


            vkCmdBeginRenderPass(getCommandBuffers()[getCurrentFrame()], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE_AND_SECONDARY_COMMAND_BUFFERS_KHR);
        }
        void RenderTarget::endRenderPass() {
            vkCmdEndRenderPass(getCommandBuffers()[getCurrentFrame()]);
        }

        void RenderTarget::applyViewportAndScissor(VkCommandBuffer cmd) {

            ////////std::cout<<(m_view.getViewport().getSize().x == 800 && m_view.getViewport().getSize().y == 800)<<std::endl;
            vkCmdSetViewport(cmd, 0, viewports.size(), viewports.data());

            vkCmdSetScissor(cmd, 0, scissors.size(), scissors.data());
        }
        void RenderTarget::recordCommandBuffers(VkCommandBuffer cmd, VertexBuffer& vb, PrimitiveType primitiveType, RenderStates states) {
            //std::lock_guard<std::recursive_mutex> lock(rec_mutex);

            Shader* shader = const_cast<Shader*>(states.shader);
            states.blendMode.updateIds();

            /*////std::cout<<"blend mode : "<<states.blendMode.colorSrcFactor<<","<<states.blendMode.colorDstFactor<<std::endl;
            system("PAUSE");*/

            //for (size_t i = 0; i < commandBuffers.size(); i++) {
                /*VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

                if (vkBeginCommandBuffer(commandBuffers[getCurrentFrame()], &beginInfo) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }*/


                /*if (getSurface() != VK_NULL_HANDLE)
                    //////std::cout<<"render pass cmd rw : "<<getRenderPass()<<std::endl;
                else
                    //////std::cout<<"render pass cmd rt : "<<getRenderPass()<<std::endl;*/
                /*std::cout<<"ids : "<<shader->getId()<<","<<vb.getPrimitiveType()<<","<<id<<","<<states.blendMode.id<<std::endl;
                system("PAUSE");*/
                /*if (depthTestEnabled)
                    std::cout<<"depth test enabled"<<std::endl;*/

                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+primitiveType][0][(depthTestEnabled) ? states.blendMode.nbBlendModes + states.blendMode.id : states.blendMode.id]);
                ////////std::cout<<"buffer : "<<this->vertexBuffers[selectedBuffer]->getVertexBuffer()<<std::endl;

                VkBuffer vertexBuffers[] = {vb.getVertexBuffer(getCurrentFrame())};
                VkDeviceSize offsets[] = {0};
                vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
                unsigned int descriptorId = shader->getId();


                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout[shader->getId() * (Batcher::nbPrimitiveTypes - 1)+primitiveType][0][(depthTestEnabled) ? states.blendMode.nbBlendModes + states.blendMode.id : states.blendMode.id], 0, 1, &descriptorSets[shader->getId()][getCurrentFrame()], 0, nullptr);

                applyViewportAndScissor(cmd);
                if(vb.getIndicesSize() > 0) {
                    vkCmdBindIndexBuffer(cmd, vb.getIndexBuffer(getCurrentFrame()), 0, VK_INDEX_TYPE_UINT16);
                }
                if(vb.getIndicesSize() > 0) {
                    vkCmdDrawIndexed(cmd, static_cast<uint32_t>(vb.getIndicesSize()), 1, 0, 0, 0);
                } else {
                    vkCmdDraw(cmd, static_cast<uint32_t>(vb.getSize()), 1, 0, 0);
                }



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
            //////std::cout<<"destroy command buffers : "<<m_name<<std::endl;
            if (commandBuffers.size() > 0) {
                vkFreeCommandBuffers(vkDevice.getDevice(), commandPool, commandBuffers.size(), commandBuffers.data());
                vkDestroyCommandPool(vkDevice.getDevice(), commandPool, nullptr);
            }
            if (secondaryCommandBuffers.size() > 0) {
                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryCommandPool, secondaryCommandBuffers.size(), secondaryCommandBuffers.data());
                vkDestroyCommandPool(vkDevice.getDevice(), secondaryCommandPool, nullptr);
            }
            delete depthTexture;




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
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                vkDestroyBuffer(vkDevice.getDevice(), drawableDataSSBO[i], nullptr);
                vkFreeMemory(vkDevice.getDevice(), drawableDataSSBOMemory[i], nullptr);
                vkDestroyBuffer(vkDevice.getDevice(), stagingDrawableData[i], nullptr);
                vkFreeMemory(vkDevice.getDevice(), stagingDrawableDataMemory[i], nullptr);
            }
        }
        std::vector<VkCommandBuffer>& RenderTarget::getCommandBuffers() {
            return commandBuffers;
        }
        std::vector<VkCommandBuffer>& RenderTarget::getSecondaryCommandBuffers() {
            return secondaryCommandBuffers;
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
        ViewportMatrix RenderTarget::getViewportMatrix(View* view) {
            ViewportMatrix vpm;
            vpm.setViewport(math::Vec3f(view->getViewport().getPosition().x(), view->getViewport().getPosition().y(), 0),
            math::Vec3f(view->getViewport().getWidth(), view->getViewport().getHeight(), 1));
            return vpm;
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
                glCheck(glEnable(GL_STENCIL_TEST));
                glCheck(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
                glCheck(glDisable(GL_STENCIL_TEST));
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
        math::Vec4f RenderTarget::mapPixelToCoords(const math::Vec4f& point)
        {
            return mapPixelToCoords(point, getView());
        }


        math::Vec4f RenderTarget::mapPixelToCoords(const math::Vec4f& point, View& view)
        {
            point[3] = 1;
            ViewportMatrix vpm;
            ////////std::cout<<"point : "<<point<<std::endl;
            vpm.setViewport(math::Vec3f(view.getViewport().getPosition().x(), view.getViewport().getPosition().y(), 0)
                                        ,math::Vec3f(view.getViewport().getWidth(), view.getViewport().getHeight(), 1));
            math::Vec4f coords = vpm.toNormalizedCoordinates(point);
            ////////std::cout<<"ndc : "<<coords<<std::endl;
            coords = view.getProjMatrix().unProject(coords);
            ////////std::cout<<"unproject : "<<coords<<std::endl;
            coords = coords.normalizeToVec3();
            ////////std::cout<<"normalized to vec3 : "<<coords<<std::endl;
            coords = view.getViewMatrix().inverseTransform(coords);
            ////////std::cout<<"inverse view : "<<coords<<std::endl;
            return coords;
        }

        math::Vec4f RenderTarget::mapCoordsToPixel(const math::Vec4f& point)
        {
            return mapCoordsToPixel(point, getView());
        }


        math::Vec4f RenderTarget::mapCoordsToPixel(const math::Vec4f& point, View& view) {
            point[3] = 1;
            ViewportMatrix vpm;
            vpm.setViewport(math::Vec3f(view.getViewport().getPosition().x(), view.getViewport().getPosition().y(), 0),
            math::Vec3f(view.getViewport().getWidth(), view.getViewport().getHeight(), 1));
            math::Vec4f coords = view.getViewMatrix().transform(point);
            coords = view.getProjMatrix().project(coords);


            coords = coords.normalizeToVec3();
            coords = vpm.toViewportCoordinates(coords);
            return coords;
        }
        ViewportMatrix RenderTarget::getViewportMatrix(View* view) {
            ViewportMatrix vpm;
            vpm.setViewport(math::Vec3f(view->getViewport().getPosition().x(), view->getViewport().getPosition().y(), 0),
            math::Vec3f(view->getViewport().getWidth(), view->getViewport().getHeight(), 1));
            return vpm;
        }
        void RenderTarget::drawVBOBindlessIndirect(PrimitiveType type, unsigned int nbIndirectCommands, RenderStates states, unsigned int vboIndirect) {
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
                std::uint64_t textureId = states.texture ? states.texture->getNativeHandle() : 0;
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
         void RenderTarget::drawIndirect(VertexBuffer& vertexBuffer, PrimitiveType type, unsigned int nbIndirectCommands, RenderStates states, unsigned int vboIndirect, unsigned int vboMatrix1, unsigned int vboMatrix2) {
            if (vertexBuffer.getVertexCount() == 0) {
                return;
            }

            if (activate(true))
            {
                if (!m_cache.glStatesSet) {
                    resetGLStates();
                }
                // Apply the view
                if (m_cache.viewChanged)
                    applyCurrentView();

                if (states.blendMode != m_cache.lastBlendMode)
                    applyBlendMode(states.blendMode);

                // Apply the texture
                std::uint64_t textureId = states.texture ? states.texture->getNativeHandle() : 0;
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
                        glCheck(glEnableVertexAttribArray(3));
                        glCheck(glEnableVertexAttribArray(4));
                        glCheck(glEnableVertexAttribArray(5));
                        glCheck(glVertexAttribPointer(0, 3,GL_FLOAT,GL_FALSE,sizeof(Vertex), (GLvoid*) 0));
                        glCheck(glVertexAttribPointer(1, 4,GL_UNSIGNED_BYTE,GL_TRUE,sizeof(Vertex),(GLvoid*) 12));
                        glCheck(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) 16));
                        glCheck(glEnableVertexAttribArray(3));
                        glCheck(glVertexAttribPointer(3, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), (GLvoid*) 24));
                        glCheck(glEnableVertexAttribArray(4));
                        glCheck(glVertexAttribIPointer(4, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs)));
                        glCheck(glEnableVertexAttribArray(5));
                        glCheck(glVertexAttribPointer(5, 4,  GL_FLOAT, GL_FALSE, sizeof(Vertex),(void*)offsetof(Vertex, m_Weights)));
                        glCheck(glDisableVertexAttribArray(0));
                        glCheck(glDisableVertexAttribArray(1));
                        glCheck(glDisableVertexAttribArray(2));
                        glCheck(glDisableVertexAttribArray(3));
                        glCheck(glDisableVertexAttribArray(4));
                        glCheck(glDisableVertexAttribArray(5));
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
                        /*if (vboMatrix1 != 0) {
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
                            glCheck(glVertexAttribPointer(15, 2, GL_FLOAT,GL_FALSE,sizeof(math::Vec2f),(GLvoid*) 0));
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
                        }*/
                    }
                    m_cache.lastVboBuffer = &vertexBuffer;

                }
                if (m_versionMajor > 3 || m_versionMajor == 3 && m_versionMinor >= 3) {
                    glCheck(glEnableVertexAttribArray(0));
                    glCheck(glEnableVertexAttribArray(1));
                    glCheck(glEnableVertexAttribArray(2));
                    glCheck(glEnableVertexAttribArray(3));
                    glCheck(glEnableVertexAttribArray(4));
                    glCheck(glEnableVertexAttribArray(5));
                    /*if (vboMatrix1 != 0) {
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
                    }*/
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
                    glCheck(glDisableVertexAttribArray(4));
                    glCheck(glDisableVertexAttribArray(5));
                    /*if (vboMatrix1 != 0) {
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
                    }*/
                    glCheck(glBindVertexArray(0));
                }
            }
            applyTexture(nullptr);
            applyShader(nullptr);
        }
        void RenderTarget::drawInstanced(VertexBuffer& vertexBuffer, enum PrimitiveType type, unsigned int start, unsigned int nb, unsigned int nbInstances, RenderStates states, unsigned int vboMatrix1, unsigned int vboMatrix2) {
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
                std::uint64_t textureId = states.texture ? states.texture->getNativeHandle() : 0;
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
                        /*if (vboMatrix1 != 0) {
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
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));*/
                    }
                    m_cache.lastVboBuffer = &vertexBuffer;

                }
                if (m_versionMajor > 3 || m_versionMajor == 3 && m_versionMinor >= 3) {
                    glCheck(glEnableVertexAttribArray(0));
                    glCheck(glEnableVertexAttribArray(1));
                    glCheck(glEnableVertexAttribArray(2));
                    glCheck(glEnableVertexAttribArray(3));
                    /*if (vboMatrix1 != 0) {
                        for (unsigned int i = 0; i < 4 ; i++) {
                            glCheck(glEnableVertexAttribArray(4 + i));
                        }
                    }
                    if (vboMatrix2 != 0) {
                        for (unsigned int i = 0; i < 4 ; i++) {
                            glCheck(glEnableVertexAttribArray(8 + i));
                        }
                    }
                    glCheck(glEnableVertexAttribArray(12));*/
                    static const GLenum modes[] = {GL_POINTS, GL_LINES, GL_LINE_STRIP, GL_TRIANGLES,
                                                       GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_QUADS};
                    GLenum mode = modes[type];
                    glCheck(glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferId));
                    glCheck(glDrawArraysInstanced(mode,start,nb,nbInstances));
                    glCheck(glDisableVertexAttribArray(0));
                    glCheck(glDisableVertexAttribArray(1));
                    glCheck(glDisableVertexAttribArray(2));
                    glCheck(glDisableVertexAttribArray(3));
                    /*if (vboMatrix1 != 0) {
                        for (unsigned int i = 0; i < 4 ; i++) {
                            glCheck(glDisableVertexAttribArray(4 + i));
                        }
                    }
                    if (vboMatrix2 != 0) {
                        for (unsigned int i = 0; i < 4 ; i++) {
                            glCheck(glDisableVertexAttribArray(8 + i));
                        }
                    }
                    glCheck(glDisableVertexAttribArray(12));*/
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
            std::vector<GlVertex> glVerts(vertexCount);
            for (unsigned int i = 0; i < vertexCount; i++) {
                glVerts[i].position[0] = vertices[i].position[0];
                glVerts[i].position[1] = vertices[i].position[1];
                glVerts[i].position[2] = vertices[i].position[2];
                glVerts[i].color[0] = vertices[i].color.r;
                glVerts[i].color[1] = vertices[i].color.g;
                glVerts[i].color[2] = vertices[i].color.b;
                glVerts[i].color[3] = vertices[i].color.a;
                glVerts[i].texCoords[0] = vertices[i].texCoords[0];
                glVerts[i].texCoords[1] = vertices[i].texCoords[1];
                glVerts[i].normal[0] = vertices[i].normal[0];
                glVerts[i].normal[1] = vertices[i].normal[1];
                glVerts[i].normal[1] = vertices[i].normal[1];
            }
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
                std::uint64_t textureId = states.texture ? states.texture->getNativeHandle() : 0;
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
                        math::Vec3f pos (vertices[i].position.x(), vertices[i].position.y(), vertices[i].position.z());
                        math::Vec3f finalpos = states.transform.transform(pos);

                        vertex.position = math::Vec3f(finalpos.x(), finalpos.y(), finalpos.z());
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
                        /*////std::cout << "offset position:  " << offsetof(Vertex, position) << std::endl;
////std::cout << "offset color:     " << offsetof(Vertex, color) << std::endl;
////std::cout << "offset texCoords: " << offsetof(Vertex, texCoords) << std::endl;
////std::cout << "offset normal:    " << offsetof(Vertex, normal) << std::endl;*/
                    const char* data = reinterpret_cast<const char*>(&glVerts[0]);
                    glEnableClientState(GL_COLOR_ARRAY);
                    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                    glEnableClientState(GL_VERTEX_ARRAY);
                    glEnableClientState(GL_NORMAL_ARRAY);
                    glVertexPointer(3, GL_FLOAT, sizeof(GlVertex), reinterpret_cast<const void*>(data + 0) );
                    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(GlVertex), reinterpret_cast<const void*>(data + 12));
                    glTexCoordPointer(2, GL_FLOAT, sizeof(GlVertex), reinterpret_cast<const void*>(data + 16));
                    glNormalPointer(GL_FLOAT, sizeof(GlVertex), reinterpret_cast<const void*>(data + 24));
                    glDisableClientState(GL_COLOR_ARRAY);
                    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                    glDisableClientState(GL_VERTEX_ARRAY);
                    glDisableClientState(GL_NORMAL_ARRAY);
                }
                glEnableClientState(GL_COLOR_ARRAY);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glEnableClientState(GL_VERTEX_ARRAY);
                glEnableClientState(GL_NORMAL_ARRAY);
                // Find the OpenGL primitive type
                static const GLenum modes[] = {GL_POINTS, GL_LINES, GL_LINE_STRIP, GL_TRIANGLES,
                                                   GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_QUADS};
                GLenum mode = modes[type];
                // Draw the primitives
                ////////std::cout<<"frame buffer id : "<<m_framebufferId<<std::endl;
                glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferId);
                glDrawArrays(mode, 0, vertexCount);
                /*glDisableClientState(GL_COLOR_ARRAY);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                glDisableClientState(GL_VERTEX_ARRAY);
                glDisableClientState(GL_NORMAL_ARRAY);
                m_cache.useVertexCache = useVertexCache;*/
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
                std::uint64_t textureId = states.texture ? states.texture->getNativeHandle() : 0;
                if (textureId != m_cache.lastTextureId)
                    applyTexture(states.texture);
                // Apply the shader
                if (states.shader)
                    applyShader(states.shader);
                applyTransform(states.transform);
                if (m_versionMajor > 3 || m_versionMajor == 3 && m_versionMinor >= 3)
                    glCheck(glBindVertexArray(m_vao));
                if (m_cache.lastVboBuffer != &vertexBuffer) {
                    ////std::cout<<"versions : "<<m_versionMajor<<","<<m_versionMinor<<std::endl;
                    if (m_versionMajor > 3 || m_versionMajor == 3 && m_versionMinor >= 3) {
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboVertexBuffer));
                        glCheck(glEnableVertexAttribArray(0));
                        glCheck(glEnableVertexAttribArray(1));
                        glCheck(glEnableVertexAttribArray(2));
                        glCheck(glEnableVertexAttribArray(3));
                        glCheck(glEnableVertexAttribArray(4));
                        glCheck(glEnableVertexAttribArray(5));
                        glCheck(glVertexAttribPointer(0, 3,GL_FLOAT,GL_FALSE,sizeof(Vertex), (GLvoid*) 0));
                        glCheck(glVertexAttribPointer(1, 4,GL_UNSIGNED_BYTE,GL_TRUE,sizeof(Vertex),(GLvoid*) 12));
                        glCheck(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) 16));
                        glCheck(glEnableVertexAttribArray(3));
                        glCheck(glVertexAttribPointer(3, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), (GLvoid*) 24));
                        glCheck(glEnableVertexAttribArray(4));
                        glCheck(glVertexAttribIPointer(4, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs)));
                        glCheck(glEnableVertexAttribArray(5));
                        glCheck(glVertexAttribPointer(5, 4,  GL_FLOAT, GL_FALSE, sizeof(Vertex),(void*)offsetof(Vertex, m_Weights)));
                        glCheck(glDisableVertexAttribArray(0));
                        glCheck(glDisableVertexAttribArray(1));
                        glCheck(glDisableVertexAttribArray(2));
                        glCheck(glDisableVertexAttribArray(3));
                        glCheck(glDisableVertexAttribArray(4));
                        glCheck(glDisableVertexAttribArray(5));
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        /*glCheck(glEnableVertexAttribArray(4));
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
                            glCheck(glVertexAttribPointer(7, 2, GL_FLOAT,GL_FALSE,sizeof(math::Vec2f),(GLvoid*) 0));
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
                        ////////std::cout<<"vbo texture index buffer : "<<vertexBuffer.vboTextureIndexesBuffer<<std::endl;
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboTextureIndexesBuffer));

                        glCheck(glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT,GL_FALSE,sizeof(GLuint),(GLvoid*) 0));*/
                        glCheck(glDisableVertexAttribArray(0));
                        glCheck(glDisableVertexAttribArray(1));
                        glCheck(glDisableVertexAttribArray(2));
                        glCheck(glDisableVertexAttribArray(3));
                        glCheck(glDisableVertexAttribArray(4));
                        glCheck(glDisableVertexAttribArray(5));
                       /* glCheck(glDisableVertexAttribArray(4));
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
                        }*/

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
                        glCheck(glNormalPointer(GL_FLOAT, sizeof(math::Vec3f), (GLvoid*) 0));
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
                    glCheck(glEnableVertexAttribArray(5));
                    /*glCheck(glEnableVertexAttribArray(4));
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
                    }*/
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
                    ////////std::cout<<"draw arrays"<<std::endl;
                    ////////std::cout<<"frame buffer id : "<<m_framebufferId<<std::endl;
                    glCheck(glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferId));
                    glCheck(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vboVertexBuffer));
                    glCheck(glDrawArrays(mode, 0, vertexBuffer.getVertexCount()));
                    glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                }
                if (m_versionMajor > 3 || m_versionMajor == 3 && m_versionMinor >= 3) {
                    glCheck(glDisableVertexAttribArray(0));
                    glCheck(glDisableVertexAttribArray(1));
                    glCheck(glDisableVertexAttribArray(2));
                    glCheck(glDisableVertexAttribArray(3));
                    glCheck(glDisableVertexAttribArray(4));
                    glCheck(glDisableVertexAttribArray(5));
                    /*glCheck(glDisableVertexAttribArray(4));
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
                    }*/
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
                    std::cerr << "OpenGL error (" << error << ") detected in user code, "
                          << "you should check for errors with glGetError()"
                          << std::endl;        }

                #endif
                glCheck(glDisable(GL_CULL_FACE));
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
                glCheck(glDepthFunc(GL_ALWAYS));
                /*glEnable(GL_STENCIL_TEST);
                glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
                glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);*/
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
                //glCheck(glDisable(GL_SCISSOR_TEST));
                //glCheck(glClipControl(GL_UPPER_LEFT, GL_ZERO_TO_ONE));

                m_cache.glStatesSet = true;

                // Apply the default ODFAEG states
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
            ////////std::cout<<"version : "<<m_versionMajor<<"."<<m_versionMinor<<std::endl;
            if (m_versionMajor > 3 || m_versionMajor == 3 && m_versionMinor >= 3) {
                GLuint vaoID;
                glCheck(glGenVertexArrays(1, &vaoID));
                m_vao = static_cast<unsigned int>(vaoID);
            }

            // Setup the default and current views
            m_defaultView = View (static_cast<float>(getSize().x()), static_cast<float>(getSize().y()), -static_cast<float>(getSize().y()) - 200, static_cast<float>(getSize().y())+200);
            m_defaultView.reset(physic::BoundingBox(0, 0, -static_cast<float>(getSize().y()) - 200,static_cast<float>(getSize().x()), static_cast<float>(getSize().y()),static_cast<float>(getSize().y())+200));
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
            glCheck(glViewport(viewport.getPosition().x(), viewport.getPosition().y(), viewport.getWidth(), viewport.getHeight()));
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
