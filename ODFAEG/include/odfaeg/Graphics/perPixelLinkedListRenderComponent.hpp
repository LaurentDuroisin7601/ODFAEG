#ifndef ODFAEG_PERPIXEL_LINKEDLIST_RENDER_COMPONENT_HPP
#define ODFAEG_PERPIXEL_LINKEDLIST_RENDER_COMPONENT_HPP
#ifndef VULKAN
#include "GL/glew.h"
#include <SFML/OpenGL.hpp>
#endif // VULKAN

#include "heavyComponent.h"
#include "renderTexture.h"
#include "sprite.h"
#include "rectangleShape.h"
#include "3D/skybox.hpp"
namespace odfaeg {
    namespace graphic {
        #ifdef VULKAN
        class ODFAEG_GRAPHICS_API PerPixelLinkedListRenderComponent : public HeavyComponent {
            public :
            enum DepthStencilID {
                NODEPTHNOSTENCIL, NODEPTHSTENCIL, NODEPTHSTENCILOUTLINE, NBDEPTHSTENCIL
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
                unsigned int materialType;
            };
            struct AtomicCounterSSBO {
                unsigned int count;
                unsigned int maxNodeCount;
            };
            struct IndirectDrawPushConsts {
                math::Matrix4f projMatrix;
                math::Matrix4f viewMatrix;
                math::Vec3f resolution;
                float time;
            };
            struct Ppll2PushConsts {
                math::Matrix4f projMatrix;
                math::Matrix4f viewMatrix;
                math::Matrix4f worldMat;
            };
            PerPixelLinkedListRenderComponent (RenderWindow& window, int layer, std::string expression, window::ContextSettings settings);
            void drawNextFrame();
            bool loadEntitiesOnComponent(std::vector<Entity*> visibleEntities);
            bool needToUpdate();
            void setExpression (std::string expression);
            std::string getExpression();
            void pushEvent(window::IEvent event, RenderWindow& window);
            void setView(View view);
            View& getView();
            void draw(RenderTarget& target, RenderStates states);
            void draw(Drawable& drawable, RenderStates states) {}
            void loadTextureIndexes() {}
            std::vector<Entity*> getEntities();
            virtual ~PerPixelLinkedListRenderComponent();
            private :
            void createCommandPool();
            void createHeadPtrImageView();
            void createHeadPtrSampler();
            void clear();
            VkCommandBuffer beginSingleTimeCommands();
            void endSingleTimeCommands(VkCommandBuffer commandBuffer);
            void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
            void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
            void createDescriptorPool(RenderStates states);
            void createDescriptorSetLayout(RenderStates states);
            void createDescriptorSets(unsigned int p, RenderStates states);
            void allocateDescriptorSets(RenderStates states);
            void createDescriptorPool2(RenderStates states);
            void createDescriptorSetLayout2(RenderStates states);
            void createDescriptorSets2(RenderStates states);
            void allocateDescriptorSets2(RenderStates states);
            void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
            void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
            void compileShaders();
            void drawInstances();
            void drawInstancesIndexed();
            void createCommandBuffersIndirect(unsigned int p, unsigned int nbIndirectCommands, unsigned int stride, DepthStencilID dephStencilID, RenderStates currentStates);
            void createCommandBufferVertexBuffer(RenderStates currentStates);
            uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
            void allocateCommandBuffers();
            View view;
            std::string expression;
            RectangleShape quad;
            unsigned int layer;

            window::Device& vkDevice;
            RenderTexture frameBuffer;
            unsigned int maxNodes;
            Sprite frameBufferSprite;
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
            std::vector<VkDescriptorPool>& descriptorPool;
            std::vector<VkDescriptorSetLayout>& descriptorSetLayout;
            std::vector<std::vector<VkDescriptorSet>>& descriptorSets;
            std::array<std::vector<ModelData>, Batcher::nbPrimitiveTypes> modelDatas;
            std::array<std::vector<MaterialData>, Batcher::nbPrimitiveTypes> materialDatas;
            Shader indirectRenderingShader, perPixelLinkedListP2;
            Ppll2PushConsts ppll2PushConsts;
            IndirectDrawPushConsts indirectDrawPushConsts;
            std::array<VertexBuffer ,Batcher::nbPrimitiveTypes> vbBindlessTex;
            VertexBuffer vb;
            VkBuffer vboIndirect;
            VkDeviceMemory vboIndirectMemory;
            std::vector<VkCommandBuffer> commandBuffers;
            std::vector<Instance> m_instances, m_normals, m_instancesIndexed, m_normalsIndexed,
            m_selectedScale, m_selected, m_selectedScaleIndexed, m_selectedIndexed,
            m_selectedScaleInstance, m_selectedInstance, m_selectedScaleInstanceIndexed, m_selectedInstanceIndexed, m_skyboxInstance;
            Batcher batcher, batcherIndexed, normalBatcher, normalBatcherIndexed,
            selectedInstanceBatcher, selectedInstanceScaleBatcher, selectedInstanceIndexBatcher, selectedInstanceIndexScaleBatcher,
            selectedBatcher, selectedScaleBatcher, selectedIndexBatcher, selectedIndexScaleBatcher, skyboxBatcher;
            bool update, firstDraw;
            Entity* skybox;
            std::vector<std::unique_ptr<Entity>> visibleSelectedScaleEntities;
            std::vector<Entity*> visibleEntities;
            math::Vec3f resolution;
            VkCommandPool commandPool;
            PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR{ VK_NULL_HANDLE };
            sf::Clock timeClock;
            std::vector<unsigned int> pipelineIds;
        };
        #else
        class PerPixelLinkedListRenderComponent : public HeavyComponent {
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
                unsigned int materialType;
            };
            PerPixelLinkedListRenderComponent (RenderWindow& window, int layer, std::string expression, window::ContextSettings settings);
            void onVisibilityChanged(bool visible);
            void loadTextureIndexes();

                 /**
            * \fn bool loadEntitiesOnComponent(std::vector<Entity*> visibleEntities)
            * \brief load the given entities onto the component.
            * \param std::vector<Entity*> visibleEntities : the entities to load.
            * \return if the loading was sucessfull.
            */
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
            void drawInstances();
            void drawInstancesIndexed();

            void drawSelectedInstances();
            void drawSelectedInstancesIndexed();
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
            void pushEvent(window::IEvent event, RenderWindow& window);
            void setView(View view);
            View& getView();
            const Texture& getFrameBufferTexture();
            RenderTexture* getFrameBuffer();
            ~PerPixelLinkedListRenderComponent();
            private :
            void compileShaders();
            RectangleShape quad;
            std::vector<std::pair<std::reference_wrapper<Drawable>, RenderStates>> drawables;
            Batcher batcher, batcherIndexed, normalBatcher, normalBatcherIndexed,
            selectedInstanceBatcher, selectedInstanceScaleBatcher, selectedInstanceIndexBatcher, selectedInstanceIndexScaleBatcher,
            selectedBatcher, selectedScaleBatcher, selectedIndexBatcher, selectedIndexScaleBatcher, skyboxBatcher; /**> A group of faces using the same materials and primitive type.*/
            sf::Color backgroundColor; /**> The background color.*/
            std::vector<Instance> m_instances, m_normals, m_instancesIndexed, m_normalsIndexed,
            m_selectedScale, m_selected, m_selectedScaleIndexed, m_selectedIndexed,
            m_selectedScaleInstance, m_selectedInstance, m_selectedScaleInstanceIndexed, m_selectedInstanceIndexed, m_skyboxInstance; /**> Instances to draw. (Instanced rendering.) */

            std::vector<Entity*> visibleEntities;
            std::vector<std::unique_ptr<Entity>> visibleSelectedScaleEntities; /**> Entities loaded*/
            RenderTexture frameBuffer; /**> the frame buffer.*/
            Shader perPixelLinkedListP2, indirectRenderingShader, skyboxShader;
            RenderStates currentStates; /**> the current render states.*/
            View view; /**> the view of the component.*/
            std::string expression;
            bool update;
            GLuint maxNodes;
            sf::Vector3i resolution;
            unsigned int atomicBuffer, linkedListBuffer, clearBuf, headPtrTex, modelDataBuffer, materialDataBuffer, ubo, vboIndirect;
            Sprite frameBufferSprite;
            VertexBuffer vb;
            std::array<VertexBuffer ,Batcher::nbPrimitiveTypes> vbBindlessTex;
            int layer;
            Entity* skybox;
            sf::Clock timeClock;
        };
        #endif
    }
}
#endif // ODFAEG_PERPIXEL_LINKEDLIST_RENDER_COMPONENT_HPP
