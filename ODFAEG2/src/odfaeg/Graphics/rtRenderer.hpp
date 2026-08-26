#ifndef ODFAEG_RT_RENDERER
#define ODFAEG_RT_RENDERER
#include <string>
#include <odfaeg/config.hpp>
#include <deque>
#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.h"
#include "../Math/maths.hpp"
#include "../Math/matrix.hpp"
#include "gpuContext.hpp"
#include "renderTarget.hpp"
#include "../Physics/boundingBox.hpp"
#include "renderTexture.hpp"
#include "descriptor.hpp"
#include "device.hpp"
#include "camera.hpp"
#include "mesh.hpp"
#include "buffer.hpp"
#include "iRenderer.hpp"
#include "../Entity/pointLight.hpp"
#include "../Entity/directionnalLight.hpp"

namespace odfaeg {
    namespace graphic {
        class RTRenderer {
            public :
                struct DirLight {
                    int id;
                    alignas(16) math::Vec3f dir;
                    unsigned int color;
                    math::Matrix4f lightSpace[NB_CASCADES+1];
                };
                struct PointLight {
                    int id;
                    alignas(16) math::Vec3f position;
                    unsigned int color;
                    math::Matrix4f lightSpace[6];
                };
                struct RayGenPC {
                    unsigned int currentFrame;
                    unsigned int pointLightCount;
                    unsigned int dirLightCount;
                    unsigned int hasGeometry;
                };
                struct GeometryOffset {
                    uint32_t vertexOffset;
                    uint32_t indexOffset;
                    uint32_t materialOffset;
                    uint32_t tlasOffset;
                };
                struct MaterialData {
                    unsigned int diffuseTextureIndex;
                    unsigned int specularTextureIndex;
                    unsigned int normalTextureIndex;
                    unsigned int metalnessTextureIndex;
                    unsigned int roughnessTextureIndex;
                    unsigned int aoTextureIndex;
                    unsigned int emissiveTextureIndex;
                    unsigned int materialType;
                    unsigned int materialSet;
                    unsigned int nbVertices;
                    unsigned int nbIndexes;
                    int instanceGroupId;
                    unsigned int vertsInstanceSet;
                    unsigned int materialId;
                    unsigned int nbBuffers;
                    int reflectable;
                    int refractable;
                    int opaque;
                };	
                struct UBOData {
                    math::Matrix4f viewInverse;
                    math::Matrix4f projInverse;
                };
                RTRenderer(RenderTarget& parentRenderer, Texture& environmentMap, RenderTexture& frameBuffer,
                    RenderTexture& cmsShadowMaps, RenderTexture& plShadowMaps, unsigned int layer, std::string typesToRenderExpression, int windowId=-1, bool usethread = false);
                void clear();
                void drawNextFrame();
                void draw();
                VkTransformMatrixKHR toVulkanTransformMatrix (math::Matrix4f matrix);
                void createDescriptorsAndPipelines();
                void createShaderBindingTable();
                void updateBLAS();
                void updateTLAS();                
                void addPointLight(entity::PointLight pointLight);
                void addDirectionnalLight(entity::DirectionnalLight directionnalLight);
            private :
                VkTransformMatrixKHR toVulkanMatrix (math::Matrix4f matrix);
                RayGenPC rayGenPC;
                void updateBuffers();
                void updateDescriptorSets();
                void createCommandPool();
                void loadExtensionsFuncPtr();
                std::deque<Image> storageImage;
                VkPhysicalDeviceRayTracingPipelinePropertiesKHR  rayTracingPipelineProperties{};
                Buffer transformMatrixBuffer, materialBuffer, geometryOffsetBuffer;
                Buffer transformMatrixStaggingBuffer, materialStaggingBuffer, geometryOffsetStaggingBuffer;
                Buffer instancesBuffer, instancesStaggingBuffer;
                Buffer localInstancesBuffer;
                Buffer raygenShaderBT, raymissShaderBT, rayhitShaderBT, dirLightStaggingBuffer, pointLightStaggingBuffer;
                std::deque<Buffer> dirLightsBuffer, pointLightsBuffer;
                std::deque<Buffer> bottomLevelASBuffers, topLevelASBuffers;
                std::deque<Buffer> ubo;
                std::vector<GeometryOffset> geometryOffsets;
                std::vector<PointLight> pointLights;
                std::vector<DirLight> dirLights;
                std::vector<VkAccelerationStructureKHR> bottomLevelAS, topLevelAS, topLocalAS; 
                bool needToUpdateBLAS, needToUpdateTLAS, needToUpdateDescriptorSets;
                Shader rtShader;
                CommandPool commandPool;
                RenderTarget& parentRenderer;
                Texture& environmentMap;
                RenderTexture& csmShadowMaps, &plShadowMaps, &frameBuffer;
                std::string typesToRenderExpression;
                bool needToUpdateBuffers;
                int instancesGroupCount, singleInstancesCount, shaderGroupCount;                     
                PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR{ nullptr };
                PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR{ nullptr };
                PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR{ nullptr };
                PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR{ nullptr };
                PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR{ nullptr };
                PFN_vkBuildAccelerationStructuresKHR vkBuildAccelerationStructuresKHR{ nullptr };
                PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR{ nullptr };
                PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR{ nullptr };                
        };        
    }
}
#endif