#ifndef ODFAEG_REFLECT_REFRACT_RENDER_COMPONENT
#define ODFAEG_REFLECT_REFRACT_RENDER_COMPONENT

#include "GL/glew.h"
#include <SFML/OpenGL.hpp>

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
            struct AtomicCounterSSBO {
                unsigned int count[6];
                unsigned int maxNodeCount;
            };
            struct IndirectRenderingPC  {
                math::Matrix4f projMatrix;
                math::Matrix4f viewMatrix;
            };
            struct LinkedList2PC  {
                math::Matrix4f projMatrix;
                math::Matrix4f viewMatrix;
                math::Matrix4f worldMat;
            };
            struct BuildDepthPC {
                unsigned int nbLayers;
            };
            struct BuildAlphaPC {
                math::Vec3f resolution;
                unsigned int nbLayers;
            };
            struct BuildFrameBufferPC {
                math::Vec3f cameraPos;
                math::Vec3f resolution;
            };
            struct UniformBufferObject {
                math::Matrix4f projMatrix[6];
                math::Matrix4f viewMatrix[6];
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
            * \fn void setBackgroundColor(sf::Color color)
            * \brief set the background color of the component. (TRansparent by default)
            * \param sf::Color color : the color.
            */
            void setBackgroundColor(sf::Color color) {}
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
            void createCommandPool();
            VkCommandBuffer beginSingleTimeCommands();
            void endSingleTimeCommands(VkCommandBuffer commandBuffer);
            void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
            void createDescriptorPool(RenderStates states);
            void createDescriptorSetLayout(RenderStates states);
            void createDescriptorSets(RenderStates states);
            void allocateDescriptorSets(RenderStates states);
            void compileShaders();
            void createUniformBuffers();
            void updateUniformBuffer(uint32_t currentImage, UniformBufferObject ubo);
            void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
            void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
            void createImageView();
            void createSampler();
            void createCommandBuffersIndirect(unsigned int p, unsigned int nbIndirectCommands, unsigned int stride, DepthStencilID depthStencilID, RenderStates currentStates);
            void createCommandBufferVertexBuffer(RenderStates currentStates);
            unsigned int maxNodes;
            uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
            float squareSize;
            RectangleShape quad;
            Batcher batcher, normalBatcher, reflBatcher, reflNormalBatcher, rvBatcher, normalRvBatcher, skyboxBatcher;
            sf::Color backgroundColor; /**> The background color.*/
            std::vector<Instance> m_instances, m_normals, m_reflInstances, m_reflNormals, m_skyboxInstance; /**> Instances to draw. (Instanced rendering.) */
            std::vector<Entity*> visibleEntities;
            Entity* skybox;
            RenderTexture depthBuffer, alphaBuffer, reflectRefractTex, environmentMap;
            Shader sBuildDepthBuffer, sBuildAlphaBuffer, sReflectRefract, sLinkedList, sLinkedList2, skyboxShader;
            View view;
            std::string expression;
            bool update;
            Sprite depthBufferSprite, reflectRefractTexSprite, alphaBufferSprite;
            std::array<VertexBuffer ,Batcher::nbPrimitiveTypes> vbBindlessTex;
            VertexBuffer vb, vb2;
            math::Vec3f dirs[6];
            math::Vec3f ups[6];
            std::vector<Entity*> rootEntities;
            VkCommandPool commandPool;
            std::vector<VkCommandBuffer> commandBuffers;
            VkBuffer modelDataBuffer, materialDataBuffer, modelDataStagingBuffer, materialDataStagingBuffer;
            VkDeviceMemory modelDataStagingBufferMemory, materialDataStagingBufferMemory;
            VkDeviceSize maxVboIndirectSize, maxModelDataSize, maxMaterialDataSize;
            VkBuffer vboIndirect, vboIndirectStagingBuffer;
            VkDeviceMemory vboIndirectMemory, vboIndirectStagingBufferMemory;
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
            math::Vec3f resolution;
            LinkedList2PC linkedList2PC;
            IndirectRenderingPC indirectRenderingPC;
            BuildDepthPC buildDepthPC;
            BuildAlphaPC buildAlphaPC;
            BuildFrameBufferPC buildFrameBufferPC;
            UniformBufferObject ubo;
            bool needToUpdateDS;
            std::vector<Entity*> vEntities;
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
            * \fn void setBackgroundColor(sf::Color color)
            * \brief set the background color of the component. (TRansparent by default)
            * \param sf::Color color : the color.
            */
            void setBackgroundColor(sf::Color color);
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
            void onVisibilityChanged(bool visible) {}
            void pushEvent(window::IEvent event, RenderWindow& window);
            void setView(View view);
            View& getView();
            RenderTexture* getFrameBuffer();
            ~ReflectRefractRenderComponent();
        private :
            RectangleShape quad;
            Batcher batcher, normalBatcher, reflBatcher, reflNormalBatcher, rvBatcher, normalRvBatcher, skyboxBatcher;
            sf::Color backgroundColor; /**> The background color.*/
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
        };
        #endif
    }
}
#endif // ODFAEG_REFLECT_REFRACT_RENDER_COMPONENT
