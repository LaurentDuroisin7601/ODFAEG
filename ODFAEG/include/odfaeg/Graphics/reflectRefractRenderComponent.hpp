#ifndef ODFAEG_REFLECT_REFRACT_RENDER_COMPONENT
#define ODFAEG_REFLECT_REFRACT_RENDER_COMPONENT

#include "GL/glew.h"
#include "../openGL.hpp"

#include "heavyComponent.h"
#include "renderTexture.h"
#include "sprite.h"
#include "rectangleShape.h"
#include "world.h"
#include "perPixelLinkedListRenderComponent.hpp"
#include "3D/cube.h"
#include "3D/skybox.hpp"
namespace odfaeg {
    namespace graphic {
        #ifdef VULKAN
        class ODFAEG_GRAPHICS_API ReflectRefractRenderComponent : public HeavyComponent {
            public :
            struct GLMatrix4f {
                float data[16];
            };
            enum DepthStencilID {
                NODEPTHNOSTENCIL, DEPTHNOSTENCIL, NBDEPTHSTENCIL
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
                GLMatrix4f worldMat;
            };
            struct alignas(16) MaterialData {
                math::Vec2f uvScale;
                math::Vec2f uvOffset;
                unsigned int textureIndex;
                unsigned int layer;
                unsigned int materialType;
                uint32_t padding;
            };
            struct AtomicCounterSSBO {
                unsigned int count[6];
                unsigned int maxNodeCount;
            };
            struct IndirectRenderingPC  {
                GLMatrix4f projMatrix;
                GLMatrix4f viewMatrix;
            };
            struct LinkedList2PC  {
                GLMatrix4f projMatrix;
                GLMatrix4f viewMatrix;
                GLMatrix4f worldMat;
            };
            struct BuildDepthPC {
                unsigned int nbLayers;
            };
            struct BuildAlphaPC {
                math::Vec4f resolution;
                unsigned int nbLayers;
            };
            struct BuildFrameBufferPC {
                math::Vec4f cameraPos;
                math::Vec4f resolution;
            };
            struct MatricesData {
                GLMatrix4f projMatrix;
                GLMatrix4f viewMatrix;
            };
            struct UniformBufferObject {
                MatricesData matrices[6];

            };
            ReflectRefractRenderComponent (RenderWindow& window, int layer, std::string expression, window::ContextSettings settings, bool useThread = true);
            void loadTextureIndexes();
            void createDescriptorsAndPipelines();
            std::vector<Entity*> getEntities();
            bool loadEntitiesOnComponent(std::vector<Entity*> visibleEntities);
            void loadSkybox(Entity* skybox);
            bool needToUpdate();
            /**
            * \fn void clearBufferBits()
            * \brief clear the buffer bits of the component.
            */
            void clear ();
            /**
            * \fn void setBackgroundColor(Color color)
            * \brief set the background color of the component. (TRansparent by default)
            * \param Color color : the color.
            */
            void setBackgroundColor(Color color) {}
            /**
            * \fn void drawNextFrame()
            * \brief draw the next frame of the component.
            */
            void resetBuffers();
            void drawNextFrame();
            void fillBufferReflMT();
            void fillBufferReflIndexedMT();
            void fillNonReflBufferMT();
            void fillNonReflIndexedBufferMT();
            void fillReflEntityBufferMT(Entity* reflectEntity);
            void fillIndexedReflEntityBufferMT(Entity* reflectEntity);
            void drawDepthReflInst();
            void drawDepthReflIndexedInst();
            void drawAlphaInst();
            void drawAlphaIndexedInst();
            void drawEnvReflInst();
            void drawEnvReflIndexedInst();
            void drawReflInst(Entity* reflectEntity);
            void drawReflIndexedInst(Entity* reflectEntity);
            void setExpression (std::string expression);
            /**
            * \fn draw(Drawable& drawable, RenderStates states = RenderStates::Default);
            * \brief draw a drawable object onto the component.
            * \param Drawable drawable : the drawable object to draw.
            * \param RenderStates states : the render states.
            */
            void draw(Drawable& drawable, RenderStates states = RenderStates::Default) {}
            /**
            * \fn void draw(RenderTarget& target, RenderStates states)
            * \brief draw the frame on a render target.
            * \param RenderTarget& target : the render target.
            * \param RenderStates states : the render states.
            */
            void draw(RenderTarget& target, RenderStates states);
            std::string getExpression();
            void pushEvent(window::IEvent event, RenderWindow& window) {}
            void setView(View view);
            View& getView();
            int getLayer();
            RenderTexture* getFrameBuffer();
            ~ReflectRefractRenderComponent();
            void onVisibilityChanged(bool visible) {}
            private :
            GLMatrix4f toVulkanMatrix(const math::Matrix4f& mat) {
                GLMatrix4f flat;
                for (int col = 0; col < 4; ++col)
                    for (int row = 0; row < 4; ++row)
                        flat.data[col * 4 + row] = mat[col][row];
                return flat;
            }
            unsigned int align(unsigned int currentOffset);
            unsigned int alignUBO (unsigned int blocSize);
            void createCommandPool();
            VkCommandBuffer beginSingleTimeCommands();
            void endSingleTimeCommands(VkCommandBuffer commandBuffer);
            void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
            void createComputeDescriptorPool();
            void createComputeDescriptorSetLayout();
            void createComputeDescriptorSet();
            void createComputePipeline();
            void createDescriptorPool(RenderStates states);
            void createDescriptorPool(unsigned int p, RenderStates states);
            void createDescriptorSetLayout(RenderStates states);
            void createDescriptorSets(RenderStates states);
            void allocateDescriptorSets(RenderStates states);
            void allocateDescriptorSets(unsigned int p, RenderStates states);
            void updateDescriptorSets(unsigned int p, RenderStates states);
            void compileShaders();
            void createUniformBuffers();
            void createUniformBuffersMT();
            void updateUniformBuffer(uint32_t currentImage, UniformBufferObject ubo);
            void updateUniformBuffer(uint32_t currentImage, std::vector<UniformBufferObject> ubo);
            void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
            void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
            void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandBuffer cmd);
            void createImageView();
            void createSampler();
            void createCommandBuffersIndirect(unsigned int p, unsigned int nbIndirectCommands, unsigned int stride, DepthStencilID depthStencilID, RenderStates currentStates);
            void recordCommandBufferIndirect(unsigned int p, unsigned int nbIndirectCommands, unsigned int stride, DepthStencilID depthStencilID, unsigned int vertexOffset, unsigned int indexOffset, unsigned int uboOffset, unsigned int modelDataOffset, unsigned int materialDataOffset, unsigned int drawCommandOffset, RenderStates currentStates, VkCommandBuffer commandBuffer);
            void createCommandBufferVertexBuffer(RenderStates currentStates);
            void recordCommandBufferVertexBuffer(RenderStates currentStates, VkCommandBuffer commandBuffer);
            void drawBuffers();
            bool isCommandBufferReady();
            unsigned int maxNodes;
            uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
            float squareSize;
            RectangleShape quad;
            Batcher batcher, normalBatcher, reflBatcher, reflNormalBatcher, indexedBatcher, normalIndexedBatcher, reflIndexedBatcher, reflNormalIndexedBatcher, skyboxBatcher;
            Color backgroundColor; /**> The background color.*/
            std::vector<Instance> m_instances, m_normals, m_reflInstances, m_reflNormals, m_indexed, m_normalIndexed, m_reflIndexed, m_reflNormalIndexed, m_skyboxInstance; /**> Instances to draw. (Instanced rendering.) */
            std::vector<Entity*> visibleEntities;
            Entity* skybox;
            RenderTexture depthBuffer, alphaBuffer, reflectRefractTex, environmentMap;
            Shader clearHeadptrComputeShader, sBuildDepthBuffer, sBuildAlphaBuffer, sReflectRefract, sLinkedList, sLinkedList2, skyboxShader;
            View view;
            std::string expression;
            bool update;
            Sprite depthBufferSprite, reflectRefractTexSprite, alphaBufferSprite;
            std::array<VertexBuffer ,Batcher::nbPrimitiveTypes> vbBindlessTex;
            std::array<VertexBuffer ,Batcher::nbPrimitiveTypes> vbBindlessTexIndexed;
            VertexBuffer vb, vb2;
            math::Vec3f dirs[6];
            math::Vec3f ups[6];
            std::vector<Entity*> rootEntities;
            VkCommandPool commandPool, secondaryBufferCommandPool;

            VkBuffer modelDataBuffer, materialDataBuffer, modelDataStagingBuffer, materialDataStagingBuffer;
            VkDeviceMemory modelDataStagingBufferMemory, materialDataStagingBufferMemory;
            VkDeviceSize maxVboIndirectSize, maxModelDataSize, maxMaterialDataSize;
            VkBuffer vboIndirect, vboIndirectStagingBuffer, vboCount;
            VkDeviceMemory vboIndirectMemory, vboIndirectStagingBufferMemory, vboCountMemory;
            std::vector<VkBuffer> uniformBuffer;
            std::vector<VkDeviceMemory> uniformBufferMemory;
            std::vector<VkBuffer> linkedListShaderStorageBuffers;
            std::vector<VkDeviceMemory> linkedListShaderStorageBuffersMemory;
            std::vector<VkBuffer> counterShaderStorageBuffers;
            std::vector<VkDeviceMemory> counterShaderStorageBuffersMemory;
            std::vector<VkBuffer> modelDataShaderStorageBuffers;
            std::vector<VkDeviceMemory> modelDataShaderStorageBuffersMemory;
            std::vector<VkBuffer> materialDataShaderStorageBuffers;
            std::vector<VkDeviceMemory> materialDataShaderStorageBuffersMemory;
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
            VkCommandBuffer copyModelDataBufferCommandBuffer, copyMaterialDataBufferCommandBuffer, copyDrawBufferCommandBuffer, copyDrawIndexedBufferCommandBuffer,
            copyVbBufferCommandBuffer, copyVbIndexedBufferCommandBuffer, copyVbEnvPass2BufferCommandBuffer, depthBufferCommandBuffer, alphaBufferCommandBuffer, environmentMapPass2CommandBuffer;
            std::vector<VkCommandBuffer> environmentMapCommandBuffer, reflectRefractCommandBuffer;
            VkImage headPtrTextureImage;
            VkImageView headPtrTextureImageView;
            VkSampler headPtrTextureSampler;
            VkDeviceMemory headPtrTextureImageMemory;
            VkImage depthTextureImage;
            VkImageView depthTextureImageView;
            VkSampler depthTextureSampler;
            VkDeviceMemory depthTextureImageMemory;
            VkImage alphaTextureImage;
            VkImageView alphaTextureImageView;
            VkSampler alphaTextureSampler;
            VkDeviceMemory alphaTextureImageMemory;
            window::Device& vkDevice;
            PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR{ VK_NULL_HANDLE };
            std::vector<VkDescriptorPool>& descriptorPool;
            std::vector<VkDescriptorSetLayout>& descriptorSetLayout;
            std::vector<std::vector<VkDescriptorSet>>& descriptorSets;
            std::array<std::vector<ModelData>, Batcher::nbPrimitiveTypes> modelDatas;
            std::array<std::vector<MaterialData>, Batcher::nbPrimitiveTypes> materialDatas;
            math::Vec4f resolution;
            LinkedList2PC linkedList2PC;
            IndirectRenderingPC indirectRenderingPC;
            BuildDepthPC buildDepthPC;
            BuildAlphaPC buildAlphaPC;
            BuildFrameBufferPC buildFrameBufferPC;
            UniformBufferObject ubo;
            std::vector<UniformBufferObject> ubos;
            bool needToUpdateDS, datasReady;
            std::array<bool, Batcher::nbPrimitiveTypes> needToUpdateDSs;
            std::vector<VkEvent> events;
            VkPipeline computePipeline;
            VkPipelineLayout computePipelineLayout;
            VkDescriptorPool cpDescriptorPool;
            VkDescriptorSetLayout cpDescriptorSetLayout;
            VkDescriptorSet cpDescriptorSet;
            VkFence computeFence;
            VkSemaphore computeSemaphore;
            std::vector<VkSemaphore> renderFinishedSemaphore;
            std::vector<VkSemaphore> clearFinishedSemaphore;
            RenderWindow& window;
            bool isSomethingDrawn;
            std::array<unsigned int, Batcher::nbPrimitiveTypes> currentModelOffset, currentMaterialOffset;
            unsigned int nbReflRefrEntities, alignment, uboAlignment;
            bool useThread;
            std::array<unsigned int, Batcher::nbPrimitiveTypes> totalBufferSizeModelData, maxBufferSizeModelData;
            std::array<unsigned int, Batcher::nbPrimitiveTypes> totalBufferSizeMaterialData, maxBufferSizeMaterialData;
            std::array<unsigned int, Batcher::nbPrimitiveTypes> totalVertexCount, totalVertexIndexCount, totalIndexCount, totalBufferSizeDrawCommand, totalBufferSizeIndexedDrawCommand, maxBufferSizeDrawCommand, maxBufferSizeIndexedDrawCommand;
            std::array<std::vector<unsigned int>, Batcher::nbPrimitiveTypes> vertexOffsets, vertexIndexOffsets, indexOffsets, modelDataOffsets, materialDataOffsets, drawCommandBufferOffsets, nbDrawCommandBuffer, drawIndexedCommandBufferOffsets, nbIndexedDrawCommandBuffer;
            std::mutex mtx;
            std::condition_variable cv;
            std::atomic<bool> commandBufferReady = false;
        };
        #else
        class ODFAEG_GRAPHICS_API ReflectRefractRenderComponent : public HeavyComponent {
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
                unsigned int materialType;
            };
            ReflectRefractRenderComponent (RenderWindow& window, int layer, std::string expression, window::ContextSettings settings);
            void loadTextureIndexes();
            std::vector<Entity*> getEntities();
            bool loadEntitiesOnComponent(std::vector<Entity*> visibleEntities);
            void loadSkybox(Entity* skybox);
            bool needToUpdate();
            /**
            * \fn void clearBufferBits()
            * \brief clear the buffer bits of the component.
            */
            void clear ();
            /**
            * \fn void setBackgroundColor(Color color)
            * \brief set the background color of the component. (TRansparent by default)
            * \param Color color : the color.
            */
            void setBackgroundColor(Color color);
            /**
            * \fn void drawNextFrame()
            * \brief draw the next frame of the component.
            */
            void drawNextFrame();
            void drawDepthReflInst();
            void drawAlphaInst();
            void drawEnvReflInst();
            void drawReflInst(Entity* reflectEntity);
            void setExpression (std::string expression);
            /**
            * \fn draw(Drawable& drawable, RenderStates states = RenderStates::Default);
            * \brief draw a drawable object onto the component.
            * \param Drawable drawable : the drawable object to draw.
            * \param RenderStates states : the render states.
            */
            void draw(Drawable& drawable, RenderStates states = RenderStates::Default);
            /**
            * \fn void draw(RenderTarget& target, RenderStates states)
            * \brief draw the frame on a render target.
            * \param RenderTarget& target : the render target.
            * \param RenderStates states : the render states.
            */
            void draw(RenderTarget& target, RenderStates states);
            std::string getExpression();
            /**
            * \fn int getLayer()
            * \brief get the layer of the component.
            * \return the number of the layer.
            */
            int getLayer();
            /**
            * \fn void setView(View& view)
            * \brief set the view of the component.
            * \param the view of the component.
            */
            /**
            * \fn register an event to the event stack of the component.
            * \param window::IEvent : the event to register.
            * \param Renderwindow : the window generating the event.
            */
            void onVisibilityChanged(bool visible);
            void pushEvent(window::IEvent event, RenderWindow& window);
            void setView(View view);
            View& getView();
            RenderTexture* getFrameBuffer();
            ~ReflectRefractRenderComponent();
        private :
            RectangleShape quad;
            Batcher batcher, normalBatcher, reflBatcher, reflNormalBatcher, rvBatcher, normalRvBatcher, skyboxBatcher;
            Color backgroundColor; /**> The background color.*/
            std::vector<Instance> m_instances, m_normals, m_reflInstances, m_reflNormals, m_skyboxInstance; /**> Instances to draw. (Instanced rendering.) */
            std::vector<Entity*> visibleEntities;
            Entity* skybox;
            RenderTexture depthBuffer, alphaBuffer, reflectRefractTex, environmentMap;
            Shader sBuildDepthBuffer, sBuildAlphaBuffer, sReflectRefract, sLinkedList, sLinkedList2, skyboxShader;
            View view;
            std::string expression;
            bool update, cubeMapCreated;
            unsigned int atomicBuffer, linkedListBuffer, clearBuf, clearBuf2, clearBuf3, headPtrTex, alphaTex, depthTex, ubo, vboIndirect, modelDataBuffer, materialDataBuffer;
            float squareSize;
            Sprite depthBufferSprite, reflectRefractTexSprite, alphaBufferSprite;
            std::array<VertexBuffer ,Batcher::nbPrimitiveTypes> vbBindlessTex;
            VertexBuffer vb, vb2;
            math::Vec3f dirs[6];
            math::Vec3f ups[6];
            std::vector<Entity*> rootEntities;
            bool datasReady;

        };
        #endif
    }
}
#endif // ODFAEG_REFLECT_REFRACT_RENDER_COMPONENT
