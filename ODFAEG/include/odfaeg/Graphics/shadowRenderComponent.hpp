#ifndef ODFAEG_SHADOW_RENDER_COMPONENT_HPP
#define ODFAEG_SHADOW_RENDER_COMPONENT_HPP
#include "GL/glew.h"
#include "renderWindow.h"
#include "renderTexture.h"
#include "sprite.h"
#include "entityManager.h"
#include "heavyComponent.h"
#include "2D/ambientLight.h"
#include "model.h"
#include "rectangleShape.h"

/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace graphic {
        #ifdef VULKAN
        class ODFAEG_GRAPHICS_API ShadowRenderComponent : public HeavyComponent {
            public :
                enum ShadowDepthStencilID {
                    SHADOWNODEPTHNOSTENCIL, SHADOWNBDEPTHSTENCIL
                };
                struct alignas(16) GLMatrix4f {
                    float data[16]; // row-major ou column-major, selon ton convention
                };
                struct DrawArraysIndirectCommand {
                    unsigned int  count;
                    unsigned int  instanceCount;
                    unsigned int  firstIndex;
                    unsigned int  baseInstance;
                };
                struct DrawElementsIndirectCommand {
                    unsigned index_count;
                    unsigned instance_count;
                    unsigned first_index;       // cf parametre offset de glDrawElements()
                    unsigned vertex_base;
                    unsigned instance_base;
                };
                struct ModelData {
                    math::Matrix4f worldMat;
                    math::Matrix4f shadowProjMat;
                };
                struct alignas(16) MaterialData {
                    math::Vec2f uvScale;
                    math::Vec2f uvOffset;
                    unsigned int textureIndex;
                    unsigned int layer;
                    uint32_t _padding[2];
                };
                struct IndirectRenderingPC {
                    math::Matrix4f projectionMatrix;
                    math::Matrix4f viewMatrix;
                };
                struct LightIndirectRenderingPC {
                    math::Matrix4f projectionMatrix;
                    math::Matrix4f viewMatrix;
                };
                struct LayerPC {
                    math::Vec4f resolution;
                    unsigned int nbLayers;
                };
                struct ResolutionPC {
                    math::Vec4f resolution;
                };
                struct ShadowUBO {
                    GLMatrix4f projectionMatrix;
                    GLMatrix4f viewMatrix;
                    GLMatrix4f lprojectionMatrix;
                    GLMatrix4f lviewMatrix;
                };
                ShadowRenderComponent (RenderWindow& window, int layer, std::string expression,window::ContextSettings settings = window::ContextSettings(0, 0, 4, 3, 0), bool useThread = true);
                void loadTextureIndexes();
                void launchRenderer();
                void drawNextFrame();

                void drawInstanced();
                void drawInstancedIndexed();

                void onVisibilityChanged(bool visible);
                std::vector<Entity*> getEntities();
                void draw(RenderTarget& target, RenderStates states);
                void draw(Drawable& drawable, RenderStates states) {
                }
                void pushEvent(window::IEvent event, RenderWindow& rw);
                bool needToUpdate();
                View& getView();
                int getLayer();
                void setExpression(std::string expression);
                std::string getExpression();
                void setView(View view);
                bool loadEntitiesOnComponent(std::vector<Entity*> vEntities);
                void clear();
                RenderTexture* getFrameBuffer();
                void createDescriptorsAndPipelines();
                ~ShadowRenderComponent();
                bool isSomethingDrawn;

            private :
                GLMatrix4f toVulkanMatrix(const math::Matrix4f& mat) {
                    GLMatrix4f flat;
                    for (int col = 0; col < 4; ++col)
                        for (int row = 0; row < 4; ++row)
                            flat.data[col * 4 + row] = mat[col][row];
                    return flat;
                }
                std::vector<VkSemaphore> offscreenFinishedSemaphore, offscreenDepthFinishedSemaphore, offscreenAlphaFinishedSemaphore, copyFinishedSemaphore;
                std::array<unsigned int, 1> values, values2, copyValues;
                RenderWindow& window;
                LayerPC layerPC;
                ResolutionPC resolutionPC;
                IndirectRenderingPC indirectRenderingPC;
                LightIndirectRenderingPC lightIndirectRenderingPC;
                VkCommandPool commandPool, secondaryBufferCommandPool;
                VkCommandBuffer copyModelDataBufferCommandBuffer, copyMaterialDataBufferCommandBuffer, copyDrawBufferCommandBuffer, copyDrawIndexedBufferCommandBuffer,
                copyVbBufferCommandBuffer, copyVbIndexedBufferCommandBuffer, depthCommandBuffer, alphaCommandBuffer, stencilCommandBuffer, shadowCommandBuffer;
                ShadowUBO shadowUBODatas;
                void createCommandPool();
                unsigned int align(unsigned int offset);
                uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
                void resetBuffers();
                void fillBuffersMT();
                void fillIndexedBuffersMT();
                void fillShadowBuffersMT();
                void fillShadowIndexedBuffersMT();
                void drawBuffers();
                void createDescriptorPool(RenderStates states);
                void createDescriptorPool(unsigned int p, RenderStates states);
                void createDescriptorSetLayout(RenderStates states);
                void createDescriptorSets(RenderStates states);
                void updateDescriptorSets(unsigned int p, RenderStates states);
                void allocateDescriptorSets(RenderStates states);
                void allocateDescriptorSets(unsigned int p, RenderStates states);
                void compileShaders();
                void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
                void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
                void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandBuffer cmd);
                void createImageView();
                void createSampler();
                void createCommandBuffersIndirect(unsigned int p, unsigned int nbIndirectCommands, unsigned int stride, ShadowDepthStencilID depthStencilID, RenderStates currentStates);
                void recordCommandBufferIndirect(unsigned int p, unsigned int nbIndirectCommands, unsigned int stride, ShadowDepthStencilID depthStencilID, unsigned int vertexOffset, unsigned int indexOffset, unsigned int uboOffset, unsigned int modelDataOffset, unsigned int materialDataOffset, unsigned int drawCommandOffset, RenderStates currentStates, VkCommandBuffer commandBuffer);
                Batcher batcher, shadowBatcher, normalBatcher, normalShadowBatcher, batcherIndexed, shadowBatcherIndexed, normalBatcherIndexed, normalShadowBatcherIndexed, normalStencilBuffer; /**> A group of faces using the same materials and primitive type.*/
                std::vector<Instance> m_instances, m_normals, m_instancesIndexed, m_normalsIndexed; /**> Instances to draw. (Instanced rendering.) */
                std::vector<Instance> m_shadow_instances, m_shadow_normals, m_shadow_instances_indexed, m_shadow_normalsIndexed;
                std::vector<Instance> m_stencil_buffer;
                std::vector<Entity*> visibleEntities; /**> Entities loaded*/
                RenderTexture stencilBuffer; /**> the stencil buffer.*/
                RenderTexture shadowMap; /**> the shadow map.*/
                RenderTexture depthBuffer;
                RenderTexture alphaBuffer;
                Sprite stencilBufferTile, shadowTile, depthBufferTile, alphaBufferSprite; /**> the stencil and shadow map buffer.*/
                Shader buildShadowMapShader; /**> the shader to generate the stencil buffer.*/
                Shader perPixShadowShader; /**> the shader to generate the shadow map.*/
                Shader depthGenShader;
                Shader sBuildAlphaBufferShader;
                View view; /**> the view of the component.*/
                std::string expression;
                bool update, datasReady, needToUpdateDS;
                VkBuffer modelDataBuffer, materialDataBuffer, modelDataStagingBuffer, materialDataStagingBuffer;
                VkDeviceMemory modelDataStagingBufferMemory, materialDataStagingBufferMemory;
                VkDeviceSize maxVboIndirectSize, maxModelDataSize, maxMaterialDataSize;
                VkBuffer vboIndirect, vboIndirectStagingBuffer, vboIndexedIndirectStagingBuffer;
                VkDeviceMemory vboIndirectMemory, vboIndirectStagingBufferMemory, vboIndexedIndirectStagingBufferMemory;

                std::array<VkBuffer, Batcher::nbPrimitiveTypes> modelDataBufferMT = {};
                std::array<VkDeviceMemory, Batcher::nbPrimitiveTypes> modelDataBufferMemoryMT = {};
                std::array<VkBuffer, Batcher::nbPrimitiveTypes> materialDataBufferMT = {};
                std::array<VkDeviceMemory, Batcher::nbPrimitiveTypes> materialDataBufferMemoryMT = {};
                std::array<VkBuffer, Batcher::nbPrimitiveTypes> drawCommandBufferMT = {};
                std::array<VkDeviceMemory, Batcher::nbPrimitiveTypes> drawCommandBufferMemoryMT = {};
                std::array<VkBuffer, Batcher::nbPrimitiveTypes> drawCommandBufferIndexedMT = {};
                std::array<VkDeviceMemory, Batcher::nbPrimitiveTypes> drawCommandBufferIndexedMemoryMT = {};
                std::array<std::vector<DrawArraysIndirectCommand>, Batcher::nbPrimitiveTypes> drawArraysIndirectCommands = {};
                std::array<std::vector<DrawElementsIndirectCommand>, Batcher::nbPrimitiveTypes> drawElementsIndirectCommands = {};

                std::vector<VkBuffer> modelDataShaderStorageBuffers;
                std::vector<VkDeviceMemory> modelDataShaderStorageBuffersMemory;
                std::vector<VkBuffer> materialDataShaderStorageBuffers;
                std::vector<VkDeviceMemory> materialDataShaderStorageBuffersMemory;
                std::vector<VkBuffer> shadowUBO;
                std::vector<VkDeviceMemory> shadowUBOMemory;
                VkImage depthTextureImage;
                VkImageView depthTextureImageView;
                VkSampler depthTextureSampler;
                VkDeviceMemory depthTextureImageMemory;
                VkImage alphaTextureImage;
                VkImageView alphaTextureImageView;
                VkSampler alphaTextureSampler;
                VkDeviceMemory alphaTextureImageMemory;
                VkImage stencilTextureImage;
                VkImageView stencilTextureImageView;
                VkSampler stencilTextureSampler;
                VkDeviceMemory stencilTextureImageMemory;
                window::Device& vkDevice;
                VertexBuffer vb, vb2;
                std::array<VertexBuffer ,Batcher::nbPrimitiveTypes> vbBindlessTex, vbBindlessTexIndexed;
                RectangleShape quad;
                std::array<std::vector<ModelData>, Batcher::nbPrimitiveTypes> modelDatas;
                std::array<std::vector<MaterialData>, Batcher::nbPrimitiveTypes> materialDatas;
                std::array<unsigned int, Batcher::nbPrimitiveTypes> totalBufferSizeModelData, maxBufferSizeModelData, maxAlignedSizeModelData, oldTotalBufferSizeModelData;
                std::array<unsigned int, Batcher::nbPrimitiveTypes> totalBufferSizeMaterialData, maxBufferSizeMaterialData, maxAlignedSizeMaterialData, oldTotalBufferSizeMaterialData;
                std::array<unsigned int, Batcher::nbPrimitiveTypes> totalVertexCount, totalVertexIndexCount, totalIndexCount, totalBufferSizeDrawCommand, totalBufferSizeIndexedDrawCommand, maxBufferSizeDrawCommand, maxBufferSizeIndexedDrawCommand;
                std::array<unsigned int, Batcher::nbPrimitiveTypes> currentModelOffset, currentMaterialOffset;
                std::array<std::vector<unsigned int>, Batcher::nbPrimitiveTypes> modelDataOffsets, materialDataOffsets, drawCommandBufferOffsets, nbDrawCommandBuffer, drawIndexedCommandBufferOffsets, nbIndexedDrawCommandBuffer;
                std::atomic<bool> commandBufferReady = false;
                std::condition_variable cv;
                std::array<bool, Batcher::nbPrimitiveTypes> needToUpdateDSs;
                unsigned int alignment;
                std::mutex mtx;
                bool useThread;
        };

        #else
        /**
          * \file OITRenderComponent.h
          * \class OITRenderComponent
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          * \brief represent a component used to render the entities of a scene.
          */
        class ODFAEG_GRAPHICS_API ShadowRenderComponent : public HeavyComponent {
            public :
                struct uint64_to_uint128 {
                    uint64_t handle;
                    uint64_t padding;
                };
                struct Samplers {
                    uint64_to_uint128 tex[200];
                };
                struct DrawArraysIndirectCommand {
                    unsigned int  count;
                    unsigned int  instanceCount;
                    unsigned int  firstIndex;
                    unsigned int  baseInstance;
                };
                struct DrawElementsIndirectCommand {
                        unsigned index_count;
                        unsigned instance_count;
                        unsigned first_index;       // cf parametre offset de glDrawElements()
                        unsigned vertex_base;
                        unsigned instance_base;
                };
                struct ModelData {
                    math::Matrix4f worldMat;
                    math::Matrix4f shadowProjMat;
                };
                struct MaterialData {
                    unsigned int textureIndex;
                    unsigned int layer;
                };
                ShadowRenderComponent (RenderWindow& window, int layer, std::string expression,window::ContextSettings settings = window::ContextSettings(0, 0, 4, 3, 0));
                void loadTextureIndexes();
                void drawNextFrame();

                void drawInstanced();
                void drawInstancedIndexed();

                void onVisibilityChanged(bool visible);
                std::vector<Entity*> getEntities();
                void draw(RenderTarget& target, RenderStates states);
                void draw(Drawable& drawable, RenderStates states) {
                }
                void pushEvent(window::IEvent event, RenderWindow& rw);
                bool needToUpdate();
                View& getView();
                int getLayer();
                const Texture& getStencilBufferTexture();
                const Texture& getShadowMapTexture();
                Sprite& getFrameBufferTile ();
                Sprite& getDepthBufferTile();
                void setExpression(std::string expression);
                std::string getExpression();
                void setView(View view);
                bool loadEntitiesOnComponent(std::vector<Entity*> vEntities);
                void clear();
                RenderTexture* getFrameBuffer();
                ~ShadowRenderComponent();
            private :
                Batcher batcher, shadowBatcher, normalBatcher, normalShadowBatcher, batcherIndexed, shadowBatcherIndexed, normalBatcherIndexed, normalShadowBatcherIndexed, normalStencilBuffer; /**> A group of faces using the same materials and primitive type.*/
                std::vector<Instance> m_instances, m_normals, m_instancesIndexed, m_normalsIndexed; /**> Instances to draw. (Instanced rendering.) */
                std::vector<Instance> m_shadow_instances, m_shadow_normals, m_shadow_instances_indexed, m_shadow_normalsIndexed;
                std::vector<Instance> m_stencil_buffer;
                std::vector<Entity*> visibleEntities; /**> Entities loaded*/
                RenderTexture stencilBuffer; /**> the stencil buffer.*/
                RenderTexture shadowMap; /**> the shadow map.*/
                RenderTexture depthBuffer;
                RenderTexture alphaBuffer;
                Sprite stencilBufferTile, shadowTile, depthBufferTile, alphaBufferSprite; /**> the stencil and shadow map buffer.*/
                Shader buildShadowMapShader; /**> the shader to generate the stencil buffer.*/
                Shader perPixShadowShader; /**> the shader to generate the shadow map.*/
                Shader depthGenShader;
                Shader sBuildAlphaBufferShader, debugShader;
                View view; /**> the view of the component.*/
                std::string expression;
                bool update, datasReady;
                unsigned int ubo, clearBuf, alphaTex, clearBuf2, depthTex, stencilTex, clearBuf3, vboIndirect, clearBuf4, frameBufferTex, atomicBuffer, modelDataBuffer, materialDataBuffer;
                VertexBuffer vb, vb2;
                std::array<VertexBuffer ,Batcher::nbPrimitiveTypes> vbBindlessTex;
                RectangleShape quad;
         };
         #endif
    }
}
#endif


