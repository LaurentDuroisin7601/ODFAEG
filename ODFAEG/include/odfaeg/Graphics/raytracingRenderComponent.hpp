#ifndef ODFAEG_GRAPHIC_RAYTRACING_RENDER_COMPONENT
#define ODFAEG_GRAPHIC_RAYTRACING_RENDER_COMPONENT
#include "heavyComponent.h"
#include "renderTexture.h"
#include "sprite.h"
#include "rectangleShape.h"
#include "2D/ambientLight.h"
#include "2D/ponctualLight.h"
#include "../config.hpp"

namespace odfaeg {
    namespace graphic {
        #ifdef VULKAN
        class ODFAEG_GRAPHICS_API RaytracingRenderComponent : public HeavyComponent {
        public :
            struct Vertex {
                math::Vec3f position;
                math::Vec4f colour;
                math::Vec2f texCoords;
                math::Vec3f normal;
            };
            struct GeometryOffset {
                uint32_t vertexOffset;
                uint32_t indexOffset;
            };
            struct MaterialData {
                math::Matrix4f textureMatrix;
                uint32_t materialIndex;
            };
            RaytracingRenderComponent (RenderWindow& window, int layer, std::string expression, window::ContextSettings settings);
                 /**
            * \fn bool loadEntitiesOnComponent(std::vector<Entity*> visibleEntities)
            * \brief load the given entities onto the component.
            * \param std::vector<Entity*> visibleEntities : the entities to load.
            * \return if the loading was sucessfull.
            */
            void loadTextureIndexes() {}
            std::vector<Entity*> getEntities();
            bool loadEntitiesOnComponent(std::vector<Entity*> visibleEntities);
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
            * \fn int getLayer()C:\Users\Laurent\Windows\ODFAEG\include\odfaeg\Math\triangle.h
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
            RenderTexture* getFrameBuffer();
            ~RaytracingRenderComponent();
        private :
            struct RayTracingScratchBuffer
            {
                uint64_t deviceAddress = 0;
                VkBuffer handle = VK_NULL_HANDLE;
                VkDeviceMemory memory = VK_NULL_HANDLE;
            };

            // Ray tracing acceleration structure
            struct AccelerationStructure {
                VkAccelerationStructureKHR handle;
                uint64_t deviceAddress = 0;
                VkDeviceMemory memory;
                VkBuffer buffer;
            };

            struct StorageImage {
                VkDeviceMemory memory;
                VkImage image;
                VkImageView view;
                VkFormat format;
            } storageImage;

            struct UniformData {
                math::Matrix4f viewInverse;
                math::Matrix4f projInverse;
            } uniformData;
            VkTransformMatrixKHR toVkTransformMatrixKHR (math::Matrix4f matrix);
            void compileShaders();
            void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
            void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
            void createUniformBuffers();
            VkDeviceSize maxTriangleOffsetBufferSize, maxTriangleBufferSize, maxVertexBufferSize, maxIndexBufferSize, maxTransformBufferSize, maxInstanceBufferSize, maxIndexTriangleBufferSize,
            maxMaterialBufferSize;
            RayTracingScratchBuffer createScratchBuffer(VkDeviceSize size);
            uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
            void deleteScratchBuffer(RayTracingScratchBuffer& scratchBuffer);
            void createAccelerationStructureBuffer(AccelerationStructure &accelerationStructure, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo);
            uint64_t getBufferDeviceAddress(VkBuffer buffer);
            void createStorageImage();
            void createTrianglesBuffers();
            void createBottomLevelAccelerationStructure();
            void createTopLevelAccelerationStructure();
            void createShaderBindingTable();
            void createDescriptorPool();
            void createDescriptorSetLayout();
            void allocateDescriptorSets();
            void createDescriptorSets();
            void createRayTracingPipeline();
            void updateUniformBuffers();
            VkPhysicalDeviceRayTracingPipelinePropertiesKHR  rayTracingPipelineProperties{};
            VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};


            VkPhysicalDeviceBufferDeviceAddressFeatures enabledBufferDeviceAddresFeatures{};
            VkPhysicalDeviceRayTracingPipelineFeaturesKHR enabledRayTracingPipelineFeatures{};
            VkPhysicalDeviceAccelerationStructureFeaturesKHR enabledAccelerationStructureFeatures{};
            VkBuffer vertexBuffer, vertexStagingBuffer;
            VkBuffer indexBuffer, indexStagingBuffer;
            VkBuffer transformBuffer, transformStagingBuffer;
            VkDeviceMemory vertexBufferMemory, vertexStagingBufferMemory;
            VkDeviceMemory indexBufferMemory, indexStagingBufferMemory;
            VkDeviceMemory transformBufferMemory, transformStagingBufferMemory;
            Color backgroundColor; /**> The background color.*/
            std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups{};
            VkBuffer raygenShaderBindingTable;
            VkBuffer missShaderBindingTable;
            VkBuffer hitShaderBindingTable;
            VkDeviceMemory raygenShaderBindingTableMemory;
            VkDeviceMemory missShaderBindingTableMemory;
            VkDeviceMemory hitShaderBindingTableMemory;
            VkBuffer triangleBuffer;
            VkBuffer triangleOffsetBuffer;
            VkBuffer indexTriangleBuffer;
            VkBuffer materialBuffer;

            VkDeviceMemory triangleBufferMemory;
            VkDeviceMemory triangleOffsetBufferMemory;
            VkDeviceMemory indexTriangleBufferMemory;
            VkDeviceMemory materialBufferMemory;
            VkBuffer triangleStagingBuffer, triangleOffsetStagingBuffer, indexTriangleStagingBuffer, materialStagingBuffer;
            VkDeviceMemory triangleStagingBufferMemory, triangleOffsetStagingBufferMemory, indexTriangleStagingBufferMemory, materialStagingBufferMemory;
            VkBuffer instanceBuffer, instanceStagingBuffer;
            VkDeviceMemory instanceBufferMemory, instanceStagingBufferMemory;
            std::vector<AccelerationStructure> bottomLevelASs;
            AccelerationStructure topLevelAS{};


            VkBuffer ubo;
            VkDeviceMemory uboMemory;

            VkPipeline pipeline;
            VkPipelineLayout pipelineLayout;
            VkDescriptorPool descriptorPool;
            VkDescriptorSet descriptorSet;
            VkDescriptorSetLayout descriptorSetLayout;

            bool update, needToUpdateDS, datasReady;
            std::string expression;
            int layer;
            std::vector<odfaeg::graphic::Entity*> visibleEntities;
            View view;
            window::Device& vkDevice;
            RenderWindow& window;
            Batcher normalBatcher, instanceBatcher;
            std::vector<Instance> m_normals, m_instances;
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indexes;
            std::vector<GeometryOffset> geometryOffsets;
            Shader raytracingShader;
            std::vector<MaterialData> materialDatas;
            VkCommandPool commandPool;

        };
        #else
        class ODFAEG_GRAPHICS_API RaytracingRenderComponent : public HeavyComponent {
        public :
            struct Triangle {
                math::Matrix4f transform;
                math::Matrix4f textureMatrix;
                math::Vec3f positions[3];
                math::Vec3f colours[3];
                math::Vec3f texCoords[3];
                math::Vec3f normal;
                uint32_t textureIndex;
                uint32_t refractReflect;
                alignas(8) float ratio;
            };
            struct Light {
                math::Vec3f center;
                math::Vec3f color;
                float radius;
            };
            struct uint64_to_uint128 {
                uint64_t handle;
                uint64_t padding;
            };
            struct Samplers {
                uint64_to_uint128 tex[200];
            };
            RaytracingRenderComponent (RenderWindow& window, int layer, std::string expression, window::ContextSettings settings);
                 /**
            * \fn bool loadEntitiesOnComponent(std::vector<Entity*> visibleEntities)
            * \brief load the given entities onto the component.
            * \param std::vector<Entity*> visibleEntities : the entities to load.
            * \return if the loading was sucessfull.
            */
            std::vector<Entity*> getEntities();
            bool loadEntitiesOnComponent(std::vector<Entity*> visibleEntities);
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
            * \fn int getLayer()C:\Users\Laurent\Windows\ODFAEG\include\odfaeg\Math\triangle.h
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
            ~RaytracingRenderComponent();
        private :
            VertexBuffer vb;
            odfaeg::graphic::RectangleShape quad;
            Color backgroundColor; /**> The background color.*/
            odfaeg::graphic::RenderTexture frameBuffer;
            Sprite frameBufferSprite;
            unsigned int frameBufferTex, trianglesSSBO, lightsSSBO, clearBuf, ubo;
            unsigned int atomicBuffer, linkedListBuffer, headPtrTex, clearBuf2;
            odfaeg::graphic::Shader rayComputeShader, quadShader;
            std::vector<Triangle> triangles;
            std::vector<Light> lights;
            bool update;
            std::string expression;
            int layer;
            std::vector<odfaeg::graphic::Entity*> visibleEntities;
            View view;
            Texture external;
        };
        #endif
    }
}
#endif // ODFAEG_GRAPHIC_RAYTRACING_RENDER_COMPONENT
