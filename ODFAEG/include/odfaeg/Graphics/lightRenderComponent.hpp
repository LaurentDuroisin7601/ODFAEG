#ifndef ODFAEG_LIGHT_RENDER_COMPONENT_HPP
#define ODFAEG_LIGHT_RENDER_COMPONENT_HPP
#include "../config.hpp"
#ifndef VULKAN
#include "glCheck.h"
#include <GL/glew.h>
#endif
#include "renderWindow.h"
#include "renderTexture.h"
#include "sprite.h"
#include "entityManager.h"
#include "heavyComponent.h"
#include "2D/ambientLight.h"
#include "rectangleShape.h"


/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace graphic {
        /**
          * \file OITRenderComponent.h
          * \class OITRenderComponent
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          * \brief represent a component used to render the entities of a scene.
          */
        #ifdef VULKAN
        class ODFAEG_GRAPHICS_API LightRenderComponent : public HeavyComponent {
                public :
                enum LightDepthStencilID {
                    LIGHTNODEPTHNOSTENCIL, LIGHTNBDEPTHSTENCIL
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
                    unsigned indexCount;
                    unsigned instanceCount;
                    unsigned firstIndex;       // cf parametre offset de glDrawElements()
                    unsigned baseVertex;
                    unsigned baseInstance;
                };
                struct ModelData {
                    math::Matrix4f worldMat;
                };
                struct alignas(16) MaterialData {
                    math::Vec2f uvScale;
                    math::Vec2f uvOffset;
                    unsigned int textureIndex;
                    unsigned int bumpTextureIndex;
                    unsigned int layer;
                    float specularIntensity;
                    float specularPower;
                    unsigned int padding1;
                    unsigned int padding2;
                    unsigned int padding3;
                    math::Vec4f lightCenter;
                    math::Vec4f lightColor;

                };
                struct IndirectRenderingPC {
                    math::Matrix4f projMatrix;
                    math::Matrix4f viewMatrix;
                };
                struct LightIndirectRenderingPC {
                    math::Matrix4f projMatrix;
                    math::Matrix4f viewMatrix;
                    math::Matrix4f viewportMatrix;
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
                struct MaxSpecPC {
                    math::Vec4f resolution;
                    float maxP;
                    float maxM;
                };
                LightRenderComponent (RenderWindow& window, int layer, std::string expression,window::ContextSettings settings = window::ContextSettings(0, 0, 4, 3, 0), bool useThread = true);
                void createDescriptorsAndPipelines();
                void loadTextureIndexes() {}
                void launchRenderer();
                void stopRenderer();
                void onVisibilityChanged(bool visible);
                void pushEvent(window::IEvent event, RenderWindow& rw);
                bool needToUpdate();
                std::string getExpression();
                void clear();
                bool loadEntitiesOnComponent(std::vector<Entity*> vEntities);
                void setView(View view);
                void setExpression(std::string expression);
                void drawNextFrame();
                void drawDepthLightInstances();
                void drawLightInstances();
                void drawInstances();
                void drawIndexedInstances();
                std::vector<Entity*> getEntities();
                void draw(RenderTarget& target, RenderStates states);
                void draw(Drawable& drawable, RenderStates states) {
                }
                View& getView();
                int getLayer();
                RenderTexture* getFrameBuffer();
                ~LightRenderComponent();
            private :
                GLMatrix4f toVulkanMatrix(const math::Matrix4f& mat) {
                    GLMatrix4f flat;
                    for (int col = 0; col < 4; ++col)
                        for (int row = 0; row < 4; ++row)
                            flat.data[col * 4 + row] = mat[col][row];
                    return flat;
                }
                void createCommandPool();
                unsigned int align(unsigned int offset);
                uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
                void resetBuffers();
                void fillBuffersMT();
                void fillIndexedBuffersMT();
                void fillLightBuffersMT();
                void fillLightBuffersIndexedMT();
                void drawBuffers();
                void createDescriptorPool(unsigned int p, RenderStates states);
                void createDescriptorPool(RenderStates states);
                void createDescriptorSetLayout(RenderStates states);
                void updateDescriptorSets(unsigned int currentFrame, unsigned int p, RenderStates states, bool lightDepth = false);
                void createDescriptorSets(RenderStates states, bool lightDepth = false);
                void allocateDescriptorSets(unsigned int p, RenderStates states);
                void allocateDescriptorSets(RenderStates states);
                void compileShaders();
                void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
                void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
                void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandBuffer cmd);
                void createImageView();
                void createSampler();
                void createCommandBuffersIndirect(unsigned int p, unsigned int nbIndirectCommands, unsigned int stride, LightDepthStencilID depthStencilID, RenderStates currentStates, bool lightDepth=false);
                void recordCommandBufferIndirect(unsigned int currentFrame, unsigned int p, unsigned int nbIndirectCommands, unsigned int stride, LightDepthStencilID depthStencilID, unsigned int vertexOffset, unsigned int indexOffset, unsigned int uboOffset, unsigned int modelDataOffset, unsigned int materialDataOffset, unsigned int drawCommandOffset, RenderStates currentStates, VkCommandBuffer commandBuffer, bool lightDepth = false);
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
                Batcher batcher, lightBatcher, lightIndexedBatcher, normalBatcher, indexedBatcher, normalIndexedBatcher; /**> A group of faces using the same materials and primitive type.*/
                std::vector<Instance> m_instances, m_normals, m_instancesIndexed, m_normalsIndexed; /**> Instances to draw. (Instanced rendering.) */
                std::vector<Instance> m_light_instances, m_light_instances_indexed; /**> Instances to draw. (Instanced rendering.) */
                std::vector<Entity*> visibleEntities; /**> Entities loaded*/
                RenderTexture depthBuffer; /**> the stencil buffer.*/
                RenderTexture bumpTexture;
                RenderTexture specularTexture;
                RenderTexture lightMap;
                RenderTexture lightDepthBuffer;
                RenderTexture alphaBuffer;
                Sprite  depthBufferTile, bumpMapTile, specularBufferTile, lightMapTile; /**> the stencil and shadow map buffer.*/
                Shader depthBufferGenerator; /**> the shader to generate the stencil buffer.*/
                Shader specularTextureGenerator;
                Shader bumpTextureGenerator;
                Shader lightMapGenerator;
                Shader buildAlphaBufferGenerator;
                View view; /**> the view of the component.*/
                std::string expression;
                bool update, datasReady, needToUpdateDS, isSomethingDrawn;

                std::array<VertexBuffer ,Batcher::nbPrimitiveTypes> vbBindlessTex, vbBindlessTexIndexed;
                VertexBuffer vb;
                std::vector<VkSemaphore> offscreenLightDepthAlphaFinishedSemaphore, offscreenFinishedSemaphore, copyFinishedSemaphore;
                std::array<unsigned int, MAX_FRAMES_IN_FLIGHT> valuesFinished, valuesLightDepthAlpha, copyValues;
                RenderWindow& window;
                VkCommandPool commandPool, secondaryBufferCommandPool;
                std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> copyModelDataBufferCommandBuffer, copyMaterialDataBufferCommandBuffer, copyDrawBufferCommandBuffer, copyDrawIndexedBufferCommandBuffer,
                copyVbBufferCommandBuffer, copyVbIndexedBufferCommandBuffer, lightDepthCommandBuffer,depthCommandBuffer, alphaCommandBuffer, specularCommandBuffer,
                bumpCommandBuffer, lightCommandBuffer;
                std::array<VkImage, MAX_FRAMES_IN_FLIGHT> lightDepthTextureImage;
                std::array<VkImageView, MAX_FRAMES_IN_FLIGHT> lightDepthTextureImageView;
                std::array<VkSampler, MAX_FRAMES_IN_FLIGHT> lightDepthTextureSampler;
                std::array<VkDeviceMemory, MAX_FRAMES_IN_FLIGHT> lightDepthTextureImageMemory;
                std::array<VkImage, MAX_FRAMES_IN_FLIGHT> depthTextureImage;
                std::array<VkImageView, MAX_FRAMES_IN_FLIGHT> depthTextureImageView;
                std::array<VkSampler, MAX_FRAMES_IN_FLIGHT> depthTextureSampler;
                std::array<VkDeviceMemory, MAX_FRAMES_IN_FLIGHT> depthTextureImageMemory;
                std::array<VkImage, MAX_FRAMES_IN_FLIGHT> alphaTextureImage;
                std::array<VkImageView, MAX_FRAMES_IN_FLIGHT> alphaTextureImageView;
                std::array<VkSampler, MAX_FRAMES_IN_FLIGHT> alphaTextureSampler;
                std::array<VkDeviceMemory, MAX_FRAMES_IN_FLIGHT> alphaTextureImageMemory;
                std::array<std::vector<ModelData>, Batcher::nbPrimitiveTypes> modelDatas;
                std::array<std::vector<MaterialData>, Batcher::nbPrimitiveTypes> materialDatas;
                window::Device& vkDevice;
                IndirectRenderingPC indirectRenderingPC;
                LightIndirectRenderingPC lightIndirectRenderingPC;
                LayerPC layerPC;
                ResolutionPC resolutionPC;
                MaxSpecPC maxSpecPC;
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
        };
        #else
        class ODFAEG_GRAPHICS_API LightRenderComponent : public HeavyComponent {
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
                };
                struct MaterialData {
                    unsigned int textureIndex;
                    unsigned int layer;
                    float specularIntensity;
                    float specularPower;
                    math::Vec3f lightCenter;
                    math::Vec3f lightColor;
                };
                LightRenderComponent (RenderWindow& window, int layer, std::string expression,window::ContextSettings settings = window::ContextSettings(0, 0, 4, 3, 0));
                void loadTextureIndexes();
                void onVisibilityChanged(bool visible);
                void pushEvent(window::IEvent event, RenderWindow& rw);
                bool needToUpdate();
                std::string getExpression();
                void clear();
                Sprite& getDepthBufferTile();
                Sprite& getspecularTile ();
                Sprite& getBumpTile();
                Sprite& getLightTile();
                const Texture& getDepthBufferTexture();
                const Texture& getSpecularTexture();
                const Texture& getbumpTexture();
                const Texture& getLightMapTexture();
                bool loadEntitiesOnComponent(std::vector<Entity*> vEntities);
                void setView(View view);
                void setExpression(std::string expression);
                void drawNextFrame();
                void drawDepthLightInstances();
                void drawLightInstances();
                void drawInstances();
                std::vector<Entity*> getEntities();
                void draw(RenderTarget& target, RenderStates states);
                void draw(Drawable& drawable, RenderStates states) {
                }
                View& getView();
                int getLayer();
                RenderTexture* getFrameBuffer();
                ~LightRenderComponent();
            private :
                Batcher batcher, lightBatcher, normalBatcher; /**> A group of faces using the same materials and primitive type.*/
                std::vector<Instance> m_instances, m_normals; /**> Instances to draw. (Instanced rendering.) */
                std::vector<Instance> m_light_instances; /**> Instances to draw. (Instanced rendering.) */
                std::vector<Entity*> visibleEntities; /**> Entities loaded*/
                RenderTexture depthBuffer; /**> the stencil buffer.*/
                RenderTexture normalMap; /**> the shadow map.*/
                RenderTexture bumpTexture;
                RenderTexture specularTexture;
                RenderTexture lightMap;
                RenderTexture lightDepthBuffer;
                RenderTexture alphaBuffer;
                Sprite  depthBufferTile, bumpMapTile, specularBufferTile, lightMapTile; /**> the stencil and shadow map buffer.*/
                Shader depthBufferGenerator; /**> the shader to generate the stencil buffer.*/
                Shader specularTextureGenerator;
                Shader bumpTextureGenerator;
                Shader lightMapGenerator;
                Shader buildAlphaBufferGenerator, debugShader;
                View view; /**> the view of the component.*/
                std::string expression;
                bool update, datasReady;
                unsigned int  ubo, lightDepthTex, alphaTex, depthTex, clearBuf, clearBuf2, clearBuf3, clearBuf4, vboIndirect, frameBufferTex, modelDataBuffer, materialDataBuffer;
                std::array<VertexBuffer ,Batcher::nbPrimitiveTypes> vbBindlessTex;
                VertexBuffer vb;
                std::vector<float> matrices;
                RectangleShape quad;
        };
        #endif

    }
}

#endif
