#ifndef ODFAEG_SHADOW_RENDER_COMPONENT_HPP
#define ODFAEG_SHADOW_RENDER_COMPONENT_HPP
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
                    GLMatrix4f projectionMatrix;
                    GLMatrix4f viewMatrix;
                };
                struct LightIndirectRenderingPC {
                    GLMatrix4f projectionMatrix;
                    GLMatrix4f viewMatrix;
                };
                struct LayerPC {
                    math::Vec4f resolution;
                    unsigned int nbLayers;
                };
                struct ResolutionPC {
                    math::Vec4f resolution;
                    float near;
                    float far;
                };
                struct ShadowUBO {
                    GLMatrix4f projectionMatrix;
                    GLMatrix4f viewMatrix;
                    GLMatrix4f lprojectionMatrix;
                    GLMatrix4f lviewMatrix;
                    GLMatrix4f viewportMatrix;
                    math::Vec4f lightCenter;
                };
                ShadowRenderComponent (RenderWindow& window, int layer, std::string expression,window::ContextSettings settings = window::ContextSettings(0, 0, 4, 3, 0), bool useThread = true);
                void loadTextureIndexes();
                void launchRenderer();
                void stopRenderer();
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
                Entity* getShadow(Entity* entity);
                std::vector<VkSemaphore> offscreenFinishedSemaphore, offscreenDepthFinishedSemaphore, offscreenAlphaFinishedSemaphore, copyFinishedSemaphore;
                std::array<unsigned int, MAX_FRAMES_IN_FLIGHT> values, values2, copyValues;
                RenderWindow& window;
                LayerPC layerPC;
                ResolutionPC resolutionPC;
                IndirectRenderingPC indirectRenderingPC;
                LightIndirectRenderingPC lightIndirectRenderingPC;
                VkCommandPool commandPool, secondaryBufferCommandPool;
                std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> copyModelDataBufferCommandBuffer, copyMaterialDataBufferCommandBuffer, copyDrawBufferCommandBuffer, copyDrawIndexedBufferCommandBuffer,
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
                void updateDescriptorSets(unsigned int currentFrame, unsigned int p, RenderStates states);
                void allocateDescriptorSets(RenderStates states);
                void allocateDescriptorSets(unsigned int p, RenderStates states);
                void compileShaders();
                void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
                void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
                void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandBuffer cmd);
                void createImageView();
                void createSampler();
                void createCommandBuffersIndirect(unsigned int p, unsigned int nbIndirectCommands, unsigned int stride, ShadowDepthStencilID depthStencilID, RenderStates currentStates);
                void recordCommandBufferIndirect(unsigned int currentFrame, unsigned int p, unsigned int nbIndirectCommands, unsigned int stride, ShadowDepthStencilID depthStencilID, unsigned int vertexOffset, unsigned int indexOffset, unsigned int uboOffset, unsigned int modelDataOffset, unsigned int materialDataOffset, unsigned int drawCommandOffset, RenderStates currentStates, VkCommandBuffer commandBuffer);
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

                std::array<std::array<VkBuffer, MAX_FRAMES_IN_FLIGHT>, Batcher::nbPrimitiveTypes> modelDataBufferMT = {};
                std::array<std::array<VkDeviceMemory, MAX_FRAMES_IN_FLIGHT>, Batcher::nbPrimitiveTypes> modelDataBufferMemoryMT = {};
                std::array<std::array<VkBuffer, MAX_FRAMES_IN_FLIGHT>, Batcher::nbPrimitiveTypes> materialDataBufferMT = {};
                std::array<std::array<VkDeviceMemory, MAX_FRAMES_IN_FLIGHT>, Batcher::nbPrimitiveTypes> materialDataBufferMemoryMT = {};
                std::array<std::array<VkBuffer, MAX_FRAMES_IN_FLIGHT>, Batcher::nbPrimitiveTypes> drawCommandBufferMT = {};
                std::array<std::array<VkDeviceMemory, MAX_FRAMES_IN_FLIGHT>, Batcher::nbPrimitiveTypes> drawCommandBufferMemoryMT = {};
                std::array<std::array<VkBuffer, MAX_FRAMES_IN_FLIGHT>, Batcher::nbPrimitiveTypes> drawCommandBufferIndexedMT = {};
                std::array<std::array<VkDeviceMemory, MAX_FRAMES_IN_FLIGHT>, Batcher::nbPrimitiveTypes> drawCommandBufferIndexedMemoryMT = {};
                std::array<std::vector<DrawArraysIndirectCommand>, Batcher::nbPrimitiveTypes> drawArraysIndirectCommands = {};
                std::array<std::vector<DrawElementsIndirectCommand>, Batcher::nbPrimitiveTypes> drawElementsIndirectCommands = {};
                std::array<std::array<VkBuffer, MAX_FRAMES_IN_FLIGHT>, Batcher::nbPrimitiveTypes>  materialDataStagingBufferMT={}, modelDataStagingBufferMT={}, vboIndirectStagingBufferMT={},
                vboIndexedIndirectStagingBufferMT={};
                std::array<std::array<VkDeviceMemory, MAX_FRAMES_IN_FLIGHT>, Batcher::nbPrimitiveTypes> materialDataStagingBufferMemoryMT={}, modelDataStagingBufferMemoryMT={}, vboIndirectStagingBufferMemoryMT={},
                vboIndexedIndirectStagingBufferMemoryMT={};

                std::vector<VkBuffer> modelDataShaderStorageBuffers;
                std::vector<VkDeviceMemory> modelDataShaderStorageBuffersMemory;
                std::vector<VkBuffer> materialDataShaderStorageBuffers;
                std::vector<VkDeviceMemory> materialDataShaderStorageBuffersMemory;
                std::vector<VkBuffer> shadowUBO;
                std::vector<VkDeviceMemory> shadowUBOMemory;
                std::array<VkImage, MAX_FRAMES_IN_FLIGHT> depthTextureImage;
                std::array<VkImageView, MAX_FRAMES_IN_FLIGHT> depthTextureImageView;
                std::array<VkSampler, MAX_FRAMES_IN_FLIGHT> depthTextureSampler;
                std::array<VkDeviceMemory, MAX_FRAMES_IN_FLIGHT> depthTextureImageMemory;
                std::array<VkImage, MAX_FRAMES_IN_FLIGHT> alphaTextureImage;
                std::array<VkImageView, MAX_FRAMES_IN_FLIGHT> alphaTextureImageView;
                std::array<VkSampler, MAX_FRAMES_IN_FLIGHT> alphaTextureSampler;
                std::array<VkDeviceMemory, MAX_FRAMES_IN_FLIGHT> alphaTextureImageMemory;
                std::array<VkImage, MAX_FRAMES_IN_FLIGHT> stencilTextureImage;
                std::array<VkImageView, MAX_FRAMES_IN_FLIGHT> stencilTextureImageView;
                std::array<VkSampler, MAX_FRAMES_IN_FLIGHT> stencilTextureSampler;
                std::array<VkDeviceMemory, MAX_FRAMES_IN_FLIGHT> stencilTextureImageMemory;
                window::Device& vkDevice;
                VertexBuffer vb, vb2;
                std::array<VertexBuffer ,Batcher::nbPrimitiveTypes> vbBindlessTex, vbBindlessTexIndexed;
                RectangleShape quad;
                std::array<std::vector<ModelData>, Batcher::nbPrimitiveTypes> modelDatas;
                std::array<std::vector<MaterialData>, Batcher::nbPrimitiveTypes> materialDatas;
                std::array<unsigned int, Batcher::nbPrimitiveTypes> totalBufferSizeModelData, maxAlignedSizeModelData, oldTotalBufferSizeModelData;
                std::array<std::array<unsigned int, MAX_FRAMES_IN_FLIGHT>, Batcher::nbPrimitiveTypes> maxBufferSizeModelData;
                std::array<unsigned int, Batcher::nbPrimitiveTypes> totalBufferSizeMaterialData, maxAlignedSizeMaterialData, oldTotalBufferSizeMaterialData;
                std::array<std::array<unsigned int, MAX_FRAMES_IN_FLIGHT>, Batcher::nbPrimitiveTypes> maxBufferSizeMaterialData;
                std::array<unsigned int, Batcher::nbPrimitiveTypes> totalVertexCount, totalVertexIndexCount, totalIndexCount, totalBufferSizeDrawCommand, totalBufferSizeIndexedDrawCommand;
                std::array<std::array<unsigned int, MAX_FRAMES_IN_FLIGHT>, Batcher::nbPrimitiveTypes> maxBufferSizeDrawCommand, maxBufferSizeIndexedDrawCommand;
                std::array<unsigned int, Batcher::nbPrimitiveTypes> currentModelOffset, currentMaterialOffset;
                std::array<std::vector<unsigned int>, Batcher::nbPrimitiveTypes> modelDataOffsets, materialDataOffsets, drawCommandBufferOffsets, nbDrawCommandBuffer, drawIndexedCommandBufferOffsets, nbIndexedDrawCommandBuffer;
                std::array<std::atomic<bool>, MAX_FRAMES_IN_FLIGHT> commandBufferReady = {};
                std::array<std::atomic<bool>, MAX_FRAMES_IN_FLIGHT> registerFrameJob = {true, false};
                std::condition_variable cv;
                std::array<std::array<bool, MAX_FRAMES_IN_FLIGHT>, Batcher::nbPrimitiveTypes> needToUpdateDSs;
                unsigned int alignment;
                std::mutex mtx;
                bool useThread;
                std::atomic<bool> stop = false;
                std::array<unsigned int, MAX_FRAMES_IN_FLIGHT> maxTexturesInUse={0, 0};
                View lightView;
                std::vector<std::unique_ptr<Entity>> shadows;
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


