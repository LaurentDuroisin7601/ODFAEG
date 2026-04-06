/*#include "../../../include/odfaeg/Graphics/perPixelLinkedListRenderComponent2.hpp"
namespace odfaeg {
    namespace graphic {
        PerPixelLinkedListDrivenRenderComponent::PerPixelLinkedListDrivenRenderComponent(RenderWindow& window, int layer, std::string expression, window::ContextSettings settings, bool useThread) :
            HeavyComponent(window, math::Vec3f(window.getView().getPosition().x(), window.getView().getPosition().y(), layer),
                          math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0),
                          math::Vec3f(window.getView().getSize().x() + window.getView().getSize().x() * 0.5f, window.getView().getPosition().y() + window.getView().getSize().y() * 0.5f, layer)) {
        }
        void PerPixelLinkedListDrivenRenderComponent::compileShader() {
            std::string computeShaderCode {
                R"(#version 460
                   #extension GL_EXT_debug_printf : enable
                   const int MAX_BONE_INFLUENCE = 4;
                   const int MAX_BONES = 100;
                   const int MAX_FRAMES_IN_FLIGHT = 2;
                   struct Vertex {
                       struct Particle {
                             vec3 position;
                             vec3 velocity;
                             vec3 scale;
                             uint color;
                             uint id;
                             float rotation;
                             float rotationSpeed;
                             uint textureIndex;
                             float passedLifeTime;
                             float totalLifeTime;
                             int padding;
                          };
                          struct Vertex {
                             vec3 position;
                             vec2 texCoords;
                             vec3 normal;
                             int m_BoneIDs[MAX_BONE_INFLUENCE];
                             float m_Weights[MAX_BONE_INFLUENCE];
                             uint color;
                             int entityId;
                             int particleId;
                             int primitiveType;
                             int currentAnimIndex;
                             int padding[3];
                         };
                         struct Quad {
                             Vertex quad[6];
                         };
                         struct AABB {
                             float posX;
                             float posY;
                             float posZ;
                             float sizeX;
                             float sizeY;
                             float sizeZ;
                         };
                         struct Object {
                             AABB globalBounds;
                             int id;
                             int type;
                             int selectedAnimIndex;
                             int currentFrameIndex;
                             int dataBufferId;
                             int visible;
                             int posInVertexBuffer;
                             int posInFinalBoneMatrices;
                             int posInIndexBuffer;
                             int nbIndexes;
                             int nbVertices;
                             int nbInstances;
                             int currentAnimIndex;
                             int posInModelDatas;

                         };
                         struct ModelData {
                             mat4 modelMatrix;
                             mat4 shadowProjMatrix;
                         };
                         struct IndirectDrawCommands {
                             uint  count;
                             uint  instanceCount;
                             uint  firstIndex;
                             uint  baseInstance;
                         };
                         struct IndirectElementsDrawCommands {
                             uint index_count;
                             uint instance_count;
                             uint first_index;       // cf parametre offset de glDrawElements()
                             uint vertex_base;
                             uint instance_base;
                         };
                         layout (std430, set = 0, binding = 0) buffer inputParticleSSBO {
                             Particle particles[];
                         } particlesBuffers;
                         layout (std430, set = 0, binding = 1) buffer inputVerticesSSBO {
                             Vertex vertices[];
                         } inputVertexBuffer[];
                         layout (std430, set = 0, binding = 2) buffer inputVerticesIndexes {
                             uint indexes[];
                         } inputVertexBuffer[];

                         layout (std430, set = 0, binding = 3) buffer inputQuadSSBO {
                             Quad quads[];
                         } quadsBuffers;
                         layout (std430, set = 0, binding = 7) buffer inputModelDataSSBO {
                             ModelData modelsDatas[];
                         } modelDatas;
                         layout (std430, set = 0, binding = 8) buffer inputFinalBonesMatricesSSBO {
                            mat4 finalBonesMatrices[MAX_BONES];
                         } finalBonesMatricesBuffers[];
                         layout (std430, set = 0, binding = 9) buffer outputVerticesSSBO {
                           Vertex vertices[];
                         } outputVertices[];
                         layout (std430, set = 0, binding = 10) buffer outputIndexVerticesSSBO {
                           uint indexes[];
                         } outputIndexVertices[];
                         layout (std430, set= 0, binding = 11) buffer outputInstancesMMSSBO {
                           mat4 modelMatrices[];
                           mat4 shadowProjMatrices[];
                         } outputModelDatas[];

                         layout (std430, set = 0, binding = 12) buffer outputIndirectSSBO {
                             IndirectDrawCommands indirectDrawCmds[];
                         } outputIndirect[];
                         layout (std430, set = 0, binding = 13) buffer outputIndirectElementsSSBO {
                             IndirectElementsDrawCommands indirectElementsDrawCmds[];
                         } outputElementsIndirect[];
                         layout (std430, set = 0, binding = 14) buffer outputIndirectDrawCountSSBO {
                             uint drawCount;
                         } indirectDrawCount;
                         layout (std430, set = 0, binding = 15) buffer countersSSBO {
                            uint posInOutputVertex;
                            uint posInOutputIndex;
                            uint posInModelDatas;
                            uint posInIndirectCommands;
                            uint posInElementsIndirecctCommands
                            uint firstIndex;
                            uint baseInstance;
                            uint baseVertex;
                         } counterInfos;
                         layout (binding = 16) uniform UniformBufferObject {
                             AABB frustrum;
                             float time;
                             int currentFrame;
                             int animTypeId;
                             int particuleTypeId;
                             int boneAnimTypeId;
                             int type;
                             int primitiveType;
                         } ubo;
                         layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
                         void main() {
                            Object object = modelDatas[gl_GlobalInvocationID.x];
                            if (object.type == ubo.type && object.visible == 1 && intersects(object.AABB, ubo.frustrum)) {

                                if (object.nbIndexes == 0) {
                                        for (int i = modelData.posInVertexBuffer; i < object.nbVertices; i++) {
                                            if (object.currentAnimIndex == inputVerticesIndexes[i].currentAnimIndex) {
                                                outputVertices[ubo.currentFrame].vertices[counterInfos.posInOutputVertex] = inputVerticesIndexes[i];
                                                if (object.type == ubo.boneAnimTypeId) {
                                                    vec4 totalPosition = vec4(0.0f);
                                                    for(int j = 0 ; j < MAX_BONE_INFLUENCE ; j++)
                                                    {
                                                      if(inputVerticesIndexes[i] == -1)
                                                         continue;
                                                      if(inputVerticesIndexes[i] >=MAX_BONES)
                                                      {
                                                          totalPosition = vec4(inputVerticesIndexes[i].xyz, 1);
                                                          break;
                                                      }
                                                      totalPosition += inputVerticesIndexes[i].weights[j] * (vec4(inputVerticesIndexes[i].position.xyz, 1) * finalBonesMatricesBuffers.finalBones[object.posInFinalBonesMatrices+MatricesinputVerticesIndexes[i].boneIds[j]]);

                                                    }
                                                    outputVertices[ubo.currentFrame].vertices[counterInfos.posInOutputVertex].position = vec3(totalPosition.xyz);
                                                }
                                            }
                                            if (object.nbInstances == 1) {
                                                outputVertices[ubo.currentFrame].vertices[counterInfos.posInOutputVertex].position = (vec4(outputVertices[ubo.currentFrame].vertices[counterInfos.posInOutputVertex].xyz, 1) * modelData[object.posInModelDatas].modelMatrix).xyz;

                                            }
                                            counterInfos.posInOutputVertex++;
                                        }
                                        if (object.nbInstances == 1)  {
                                            outputModelDatas[ubo.currentFrame].modelMatrices[counterInfos.posInModelDatas] = mat4(1.0);
                                            counterInfos.posInModelDatas++;
                                        } else {
                                            for (int i = 0; i < object.nbInstances; i++) {
                                                outputModelDatas[ubo.currentFrame].modelMatrices[counterInfos.posInModelDatas] = modelData[object.posInModelDatas].modelMatrix;
                                                counterInfos.posInModelDatas++;
                                            }
                                        }
                                        IndirectDrawCommands indirectDrawCmds;
                                        indirectDrawCmds.count = object.nbVertices;
                                        indirectDrawCmds.firstIndex = counterInfos.firstIndex;
                                        indirectDrawCmds.baseInstance = baseInstance;
                                    } else {
                                        for (int i = modelData.posInVertexBuffer; i < object.nbVertices; i++) {
                                            if (object.currentAnimIndex == inputVerticesIndexes[ubo.primitiveType].vertices[i].currentAnimIndex) {
                                                outputVertices[ubo.currentFrame].vertices[counterInfos.posInOutputVertex] = inputVerticesIndexes[i];
                                                if (object.type == ubo.boneAnimTypeId) {
                                                    vec4 totalPosition = vec4(0.0f);
                                                    for(int j = 0 ; j < MAX_BONE_INFLUENCE ; j++)
                                                    {
                                                      if(inputVerticesIndexes[i] == -1)
                                                         continue;
                                                      if(inputVerticesIndexes[i] >=MAX_BONES)
                                                      {
                                                          totalPosition = vec4(inputVerticesIndexes[i].xyz, 1);
                                                          break;
                                                      }
                                                      totalPosition += inputVerticesIndexes[i].weights[j] * (vec4(inputVerticesIndexes[i].position.xyz, 1) * finalBonesMatricesBuffers.finalBones[object.posInFinalBonesMatrices+inputFinalBonesMatrices[i].boneIds[j]]);

                                                    }
                                                    outputIndexVertices[ubo.currentFrame].vertices[counterInfos.posInOutputVertex].position = vec3(totalPosition.xyz);
                                                }
                                            }
                                            if (object.nbInstances == 1) {
                                                outputIndexVertices[ubo.currentFrame].index[counterInfos.posInOutputIndex] = (vec4(outputVertices[ubo.currentFrame].vertices[counterInfos.posInOutputVertex].xyz, 1) * modelData[object.posInModelDatas].modelMatrix).xyz;

                                            }
                                            counterInfos.posInOutputVertex++;
                                        }
                                        for (int i = modelData.posInIndexBuffer; i < object.nbIndexes; i++) {
                                            outputVertices[ubo.currentFrame].vertices[counterInfos.posInOutputVertex] = inputIndexes[i];
                                        }
                                        if (object.nbInstances == 1)  {
                                            outputModelDatas[ubo.currentFrame].modelMatrices[counterInfos.posInModelDatas] = mat4(1.0);
                                            counterInfos.posInModelDatas++;
                                        } else {
                                            for (int i = 0; i < object.nbInstances; i++) {
                                                outputModelDatas[ubo.currentFrame*6+primitiveType].modelMatrices[counterInfos.posInModelDatas] = modelData[object.posInModelDatas].modelMatrix;
                                                counterInfos.posInModelDatas++;
                                            }
                                        }
                                    }
                                }
                            }
                         }
                   };
                )";
            }
        }
    }
}*/
