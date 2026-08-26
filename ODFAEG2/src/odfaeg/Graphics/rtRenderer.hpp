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
                void updateBuffers;
                void updateDescriptorSets();
                void createCommandPool();
                void loadExtensionsFuncPtr();
                std::deque<Image> storageImage;
                VkPhysicalDeviceRayTracingPipelinePropertiesKHR  rayTracingPipelineProperties{};
                Buffer transformMatrixBuffer, materialBuffer, geometryOffsetBuffer;
                Buffer transformMatrixStaggingBuffer, materialStaggingBuffer, geometryOffsetStaggingBuffer;
                Buffer instancesBuffer, instancesStaggingBuffer;
                Buffer raygenShaderBT, raymissShaderBT, rayhitShaderBT, dirLightStaggingBuffer, pointLightStaggingBuffer;
                std::deque<Buffer> dirLightsBuffer, pointLightsBuffer;
                std::deque<Buffer> bottomLevelASBuffers, topLevelASBuffers;
                std::deque<Buffer> ubo;
                std::vector<VkAccelerationStructureKHR> bottomLevelAS, topGlobalAS, topLocalAS; 
                bool needToUpdateBLAS, needToUpdateTLAS, needToUpdateDescriptorSets;
                Shader rtShader;
                CommandPool commandPool;
                RenderTarget& parentRenderer;
                Texture& environmentMap;
                RenderTexture& cmsShadowMaps, &plShadowMaps, &frameBuffer;
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