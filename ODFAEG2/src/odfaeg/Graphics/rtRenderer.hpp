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
namespace odfaeg {
    namespace graphic {
        class RTRenderer {
            public :
                struct GeometryOffset {
                    uint32_t vertexOffset;
                    uint32_t indexOffset;
                    uint32_t materialOffset;
                }
                struct MaterialData {
                    math::Vec2f uvScale;
                    math::Vec2f uvOffset;
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
                    unsigned int padding;
                };	
                struct UBOData {
                    math::Matrix4f viewInverse;
                    math::Matrix4f projInverse;
                };
                RTRenderer(RenderTarget& parentRenderer, unsigned int layer, std::string typesToRenderExpression, int windowId=-1, bool usethread = false);
                void clear();
                void drawNextFrame();
                void draw();
                VkTransformMatrixKHR toVulkanTransformMatrix (math::Matrix4f matrix);
                void createDescriptorsAndPipelines();
                void createShaderBindingTable();
                void updateBLAS();
                void updateTLAS();
                private :
                    std::deque<Buffer> transformMatrixBuffer, materialBuffer, geometryOffsetBuffer;
                    Buffer transformMatrixStaggingBuffer, materialStaggingBuffer, geometryOffsetStaggingBuffer;
                    Buffer raygenBindingTable, raymissBindingTable, rayhitBindingTable;
                    Buffer bottomLevelASBuffers, topLevelASBuffers;
                    std::deque<VkAccelerationStructureKHR> bottomLevelAS, topLevelAS; 
                    bool needToUpdateDS, needToUpdateTLAS, needToUpdateBuffers;
                    Shader rtShader;
                    CommandPool commandPool;
                    RenderTarget& parentRenderer;                                       
            };
        };
    }
}
#endif