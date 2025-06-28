#include "../../../include/odfaeg/Graphics/lightRenderComponent.hpp"


#include <memory.h>

using namespace std;
namespace odfaeg {
    namespace graphic {
        #ifdef VULKAN
        LightRenderComponent::LightRenderComponent (RenderWindow& window, int layer, std::string expression,window::ContextSettings settings) :
                HeavyComponent(window, math::Vec3f(window.getView().getPosition().x(), window.getView().getPosition().y(), layer),
                              math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0),
                              math::Vec3f(window.getView().getSize().x() + window.getView().getSize().x() * 0.5f, window.getView().getPosition().y() + window.getView().getSize().y() * 0.5f, layer)),
                view(window.getView()),
                expression(expression),
                vb(window.getDevice()),
                vbBindlessTex {VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()),
                VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice())},
                descriptorSetLayout(window.getDescriptorSetLayout()),
                depthBufferGenerator(window.getDevice()), buildAlphaBufferGenerator(window.getDevice()), specularTextureGenerator(window.getDevice()),bumpTextureGenerator(window.getDevice()), lightMapGenerator(window.getDevice()),
                vkDevice(window.getDevice()),
                descriptorPool(window.getDescriptorPool()),
                descriptorSets(window.getDescriptorSet()),
                depthBuffer(window.getDevice()),
                alphaBuffer(window.getDevice()),
                bumpTexture(window.getDevice()),
                specularTexture(window.getDevice()),
                lightMap(window.getDevice()),
                lightDepthBuffer(window.getDevice()),
                window(window) {
                    vboIndirect = vboIndirectStagingBuffer = modelDataStagingBuffer = materialDataStagingBuffer = nullptr;
                maxVboIndirectSize = maxModelDataSize = maxMaterialDataSize = 0;
                needToUpdateDS = false;
                createCommandPool();
                depthBuffer.create(window.getView().getSize().x(), window.getView().getSize().y());
                depthBuffer.setView(view);
                math::Vec4f resolution(window.getView().getSize().x(), window.getView().getSize().y(),window.getView().getSize().z(), 1);

                depthBufferTile = Sprite(depthBuffer.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0), IntRect(0, 0, window.getView().getSize().x(), window.getView().getSize().y()));
                alphaBuffer.create(window.getView().getSize().x(), window.getView().getSize().y());
                alphaBuffer.setView(view);
                //alphaBuffer.m_name = "alphaBuffer";

                bumpTexture.create(window.getView().getSize().x(), window.getView().getSize().y());
                bumpTexture.setView(view);
                specularTexture.create(window.getView().getSize().x(), window.getView().getSize().y());
                specularTexture.setView(view);
                lightMap.create(window.getView().getSize().x(), window.getView().getSize().y());
                lightMap.setView(view);
                lightDepthBuffer.create(window.getView().getSize().x(), window.getView().getSize().y());
                lightDepthBuffer.setView(view);
                core::FastDelegate<bool> signal (&LightRenderComponent::needToUpdate, this);
                core::FastDelegate<void> slot (&LightRenderComponent::drawNextFrame, this);
                core::Command cmd(signal, slot);
                getListener().connect("UPDATE", cmd);
                compileShaders();

                VkImageCreateInfo imageInfo{};
                imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                imageInfo.imageType = VK_IMAGE_TYPE_2D;
                imageInfo.extent.width = static_cast<uint32_t>(window.getView().getSize().x());
                imageInfo.extent.height = static_cast<uint32_t>(window.getView().getSize().y());
                imageInfo.extent.depth = 1;
                imageInfo.mipLevels = 1;
                imageInfo.arrayLayers = 1;
                imageInfo.format =  VK_FORMAT_R32G32B32A32_SFLOAT;
                imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
                imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
                imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                imageInfo.flags = 0; // Optionnel

                if (vkCreateImage(window.getDevice().getDevice(), &imageInfo, nullptr, &depthTextureImage) != VK_SUCCESS) {
                    throw std::runtime_error("echec de la creation d'une image!");
                }


                VkMemoryRequirements memRequirements;
                vkGetImageMemoryRequirements(window.getDevice().getDevice(), depthTextureImage, &memRequirements);

                VkMemoryAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.allocationSize = memRequirements.size;
                allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

                if (vkAllocateMemory(window.getDevice().getDevice(), &allocInfo, nullptr, &depthTextureImageMemory) != VK_SUCCESS) {
                    throw std::runtime_error("echec de l'allocation de la memoire d'une image!");
                }
                vkBindImageMemory(window.getDevice().getDevice(), depthTextureImage, depthTextureImageMemory, 0);
                //transitionImageLayout(depthTextureImage, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);



                VkImageCreateInfo imageInfo2{};
                imageInfo2.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                imageInfo2.imageType = VK_IMAGE_TYPE_2D;
                imageInfo2.extent.width = static_cast<uint32_t>(window.getView().getSize().x());
                imageInfo2.extent.height = static_cast<uint32_t>(window.getView().getSize().y());
                imageInfo2.extent.depth = 1;
                imageInfo2.mipLevels = 1;
                imageInfo2.arrayLayers = 1;
                imageInfo2.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                imageInfo2.tiling = VK_IMAGE_TILING_OPTIMAL;
                imageInfo2.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                imageInfo2.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
                imageInfo2.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                imageInfo2.samples = VK_SAMPLE_COUNT_1_BIT;
                imageInfo2.flags = 0; // Optionnel
                if (vkCreateImage(window.getDevice().getDevice(), &imageInfo2, nullptr, &alphaTextureImage) != VK_SUCCESS) {
                    throw std::runtime_error("echec de la creation d'une image!");
                }


                vkGetImageMemoryRequirements(window.getDevice().getDevice(), alphaTextureImage, &memRequirements);


                allocInfo.allocationSize = memRequirements.size;
                allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                if (vkAllocateMemory(window.getDevice().getDevice(), &allocInfo, nullptr, &alphaTextureImageMemory) != VK_SUCCESS) {
                    throw std::runtime_error("echec de l'allocation de la memoire d'une image!");
                }
                vkBindImageMemory(window.getDevice().getDevice(), alphaTextureImage, alphaTextureImageMemory, 0);
                createImageView();
                createSampler();
                lightMap.beginRecordCommandBuffers();
                std::vector<VkCommandBuffer> commandBuffers = lightMap.getCommandBuffers();
                unsigned int currentFrame =  lightMap.getCurrentFrame();
                VkImageMemoryBarrier barrier = {};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
                barrier.image = depthTextureImage;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.layerCount = 1;
                vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
                VkImageMemoryBarrier barrier2 = {};
                barrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier2.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier2.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                barrier2.newLayout = VK_IMAGE_LAYOUT_GENERAL;
                barrier2.image = alphaTextureImage;
                barrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier2.subresourceRange.levelCount = 1;
                barrier2.subresourceRange.layerCount = 1;
                vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier2);
                const_cast<Texture&>(lightMap.getTexture()).toShaderReadOnlyOptimal(lightMap.getCommandBuffers()[lightMap.getCurrentFrame()]);
                lightMap.display();

                VkSemaphoreCreateInfo semaphoreInfo{};
                semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                renderFinishedSemaphore.resize(RenderWindow::MAX_FRAMES_IN_FLIGHT);
                for (unsigned int i = 0; i < RenderWindow::MAX_FRAMES_IN_FLIGHT; i++) {
                    if (vkCreateSemaphore(vkDevice.getDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphore[i]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create compute synchronization objects for a frame!");
                    }
                }
                for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                    vbBindlessTex[i].setPrimitiveType(static_cast<PrimitiveType>(i));
                }
                resolutionPC.resolution = resolution;
            }
            void LightRenderComponent::compileShaders() {
                const std::string indirectRenderingVertexShader = R"(#version 460
                                                                             layout (location = 0) in vec3 position;
                                                                             layout (location = 1) in vec4 color;
                                                                             layout (location = 2) in vec2 texCoords;
                                                                             layout (location = 3) in vec3 normals;

                                                                             struct ModelData {
                                                                                mat4 modelMatrix;
                                                                             };
                                                                             struct MaterialData {
                                                                                uint textureIndex;
                                                                                uint layer;
                                                                                float specularIntensity;
                                                                                float specularPower;
                                                                                vec4 lightCenter;
                                                                                vec4 lightColor;
                                                                             };
                                                                             layout (push_constant)uniform PushConsts {
                                                                                 mat4 projectionMatrix;
                                                                                 layout (offset = 64) mat4 viewMatrix;
                                                                             } pushConsts;
                                                                             layout(binding = 4) buffer modelData {
                                                                                 ModelData modelDatas[];
                                                                             };
                                                                             layout(binding = 5) buffer materialData {
                                                                                 MaterialData materialDatas[];
                                                                             };
                                                                             layout (location = 0) out vec2 fTexCoords;
                                                                             layout (location = 1) out vec4 frontColor;
                                                                             layout (location = 2) out uint texIndex;
                                                                             layout (location = 3) out uint layer;
                                                                             layout (location = 4) out vec3 normal;
                                                                             void main() {
                                                                                 ModelData model = modelDatas[gl_InstanceIndex];
                                                                                 MaterialData material = materialDatas[gl_DrawID];
                                                                                 uint textureIndex = material.textureIndex;
                                                                                 uint l = material.layer;
                                                                                 gl_Position = vec4(position, 1.f) * model.modelMatrix * pushConsts.viewMatrix, pushConsts.projectionMatrix;
                                                                                 fTexCoords = texCoords;
                                                                                 frontColor = color;
                                                                                 texIndex = textureIndex;
                                                                                 layer = l;
                                                                             }
                                                                             )";
                        const std::string specularIndirectRenderingVertexShader = R"(#version 460
                                                                                     layout (location = 0) in vec3 position;
                                                                                     layout (location = 1) in vec4 color;
                                                                                     layout (location = 2) in vec2 texCoords;
                                                                                     layout (location = 3) in vec3 normals;

                                                                                     struct ModelData {
                                                                                        mat4 modelMatrix;
                                                                                     };
                                                                                     struct MaterialData {
                                                                                        uint textureIndex;
                                                                                        uint layer;
                                                                                        float specularIntensity;
                                                                                        float specularPower;
                                                                                        vec4 lightCenter;
                                                                                        vec4 lightColor;
                                                                                     };
                                                                                     layout (push_constant)uniform PushConsts {
                                                                                         mat4 projectionMatrix;
                                                                                         layout (offset = 64) mat4 viewMatrix;
                                                                                     } pushConsts;
                                                                                     layout(binding = 4) buffer modelData {
                                                                                     ModelData modelDatas[];
                                                                                     };
                                                                                     layout(binding = 5) buffer materialData {
                                                                                         MaterialData materialDatas[];
                                                                                     };
                                                                                     layout (location = 0) out vec2 fTexCoords;
                                                                                     layout (location = 1) out vec4 frontColor;
                                                                                     layout (location = 2) out uint texIndex;
                                                                                     layout (location = 3) out uint layer;
                                                                                     layout (location = 4) out vec3 normal;
                                                                                     layout (location = 5) out vec2 specular;
                                                                                     void main() {
                                                                                         ModelData model = modelDatas[gl_InstanceIndex];
                                                                                         MaterialData material = materialDatas[gl_DrawID];
                                                                                         uint textureIndex = material.textureIndex;
                                                                                         uint l = material.layer;
                                                                                         gl_Position = vec4(position, 1.f) * model.modelMatrix * pushConsts.viewMatrix * pushConsts.projectionMatrix;
                                                                                         frontColor = color;
                                                                                         texIndex = textureIndex;
                                                                                         layer = l;
                                                                                         specular = vec2(material.specularIntensity, material.specularPower);
                                                                                     }
                                                                                     )";
                        const std::string perPixLightingIndirectRenderingVertexShader = R"(#version 460
                                                                                      layout (location = 0) in vec3 position;
                                                                                      layout (location = 1) in vec4 color;
                                                                                      layout (location = 2) in vec2 texCoords;
                                                                                      layout (location = 3) in vec3 normals;
                                                                                      layout (push_constant)uniform PushConsts {
                                                                                         mat4 projectionMatrix;
                                                                                         layout (offset = 64) mat4 viewMatrix;
                                                                                         layout (offset = 128) mat4 viewportMatrix;
                                                                                     } pushConsts;
                                                                                      struct ModelData {
                                                                                         mat4 modelMatrix;
                                                                                      };
                                                                                      struct MaterialData {
                                                                                         uint textureIndex;
                                                                                         uint layer;
                                                                                         float specularIntensity;
                                                                                         float specularPower;
                                                                                         vec4 lightCenter;
                                                                                         vec4 lightColor;
                                                                                      };
                                                                                      layout(binding = 4) buffer modelData {
                                                                                      ModelData modelDatas[];
                                                                                      };
                                                                                      layout(binding = 5) buffer materialData {
                                                                                          MaterialData materialDatas[];
                                                                                      };
                                                                                      layout (location = 0) out vec2 fTexCoords;
                                                                                      layout (location = 1) out vec4 frontColor;
                                                                                      layout (location = 2) out uint texIndex;
                                                                                      layout (location = 3) out uint layer;
                                                                                      layout (location = 4) out vec3 normal;
                                                                                      layout (location = 5) out vec4 lightPos;
                                                                                      layout (location = 6) out vec4 lightColor;
                                                                                      void main() {
                                                                                         ModelData model = modelDatas[gl_InstanceIndex];
                                                                                         MaterialData material = materialDatas[gl_DrawID];
                                                                                         uint l = material.layer;
                                                                                         gl_Position = vec4(position, 1.f) * model.modelMatrix * pushConsts.viewMatrix * pushConsts.projectionMatrix;
                                                                                         fTexCoords = texCoords;
                                                                                         frontColor = color;
                                                                                         layer = l;
                                                                                         vec4 coords = vec4(material.lightCenter.xyz, 1);
                                                                                         coords = coords * model.modelMatrix * pushConsts.viewMatrix * pushConsts.projectionMatrix;

                                                                                         coords = coords / coords.w;
                                                                                         coords = pushConsts.viewportMatrix * coords;
                                                                                         coords.w = material.lightCenter.w;
                                                                                         lightPos = coords;
                                                                                         lightColor = material.lightColor;
                                                                                      }
                                                                                      )";

                        const std::string depthGenFragShader = R"(#version 460
                                                                          #extension GL_ARB_fragment_shader_interlock : require
                                                                          #extension GL_EXT_nonuniform_qualifier : enable
                                                                          layout (location = 0) in vec2 fTexCoords;
                                                                          layout (location = 1) in vec4 frontColor;
                                                                          layout (location = 2) in flat uint texIndex;
                                                                          layout (location = 3) in flat uint layer;
                                                                          layout (location = 4) in vec3 normal;
                                                                          layout(set = 0, binding = 0) uniform sampler2D textures[];
                                                                          layout(set = 0, binding = 1, rgba32f) uniform coherent image2D depthBuffer;
                                                                          layout (location = 0) out vec4 fColor;
                                                                          layout (push_constant) uniform PushConsts {
                                                                              layout (offset = 128) uint nbLayers;
                                                                          } pushConsts;
                                                                          void main () {
                                                                              vec4 texel = (texIndex != 0) ? frontColor * texture(textures[texIndex-1], fTexCoords.xy) : frontColor;
                                                                              float z = gl_FragCoord.z;
                                                                              float l = layer;
                                                                              beginInvocationInterlockARB();
                                                                              vec4 depth = imageLoad(depthBuffer,ivec2(gl_FragCoord.xy));
                                                                              if (/*l > depth.y() || l == depth.y() &&*/ z > depth.z) {
                                                                                imageStore(depthBuffer,ivec2(gl_FragCoord.xy),vec4(0,l,z,texel.a));
                                                                                memoryBarrier();
                                                                                fColor = vec4(0, l, z, texel.a);
                                                                              } else {
                                                                                fColor = depth;
                                                                              }
                                                                              endInvocationInterlockARB();
                                                                          }
                                                                        )";
                        const std::string buildAlphaBufferFragmentShader = R"(#version 460
                                                                      #extension GL_ARB_fragment_shader_interlock : require
                                                                      #extension GL_EXT_nonuniform_qualifier : enable
                                                                      layout (location = 0) in vec2 fTexCoords;
                                                                      layout (location = 1) in vec4 frontColor;
                                                                      layout (location = 2) in flat uint texIndex;
                                                                      layout (location = 3) in flat uint layer;
                                                                      layout (location = 4) in vec3 normal;
                                                                      layout(set = 0, binding = 1) uniform sampler2D depthBuffer;
                                                                      layout(set = 0, binding = 0) uniform sampler2D textures[];
                                                                      layout(binding = 0, rgba32f) uniform coherent image2D alphaBuffer;
                                                                      layout (location = 0) out vec4 fColor;
                                                                      layout (push_constant) uniform PushConsts {
                                                                          layout (offset = 128) uint nbLayers;
                                                                      } pushConsts;
                                                                      void main() {
                                                                          vec4 texel = (texIndex != 0) ? frontColor * texture(textures[texIndex-1], fTexCoords.xy) : frontColor;
                                                                          float current_alpha = texel.a;
                                                                          vec4 depth = texture (depthBuffer, gl_FragCoord.xy);
                                                                          beginInvocationInterlockARB();
                                                                          vec4 alpha = imageLoad(alphaBuffer,ivec2(gl_FragCoord.xy));
                                                                          float l = layer;
                                                                          float z = gl_FragCoord.z;
                                                                          if (/*l > depth.y() || l == depth.y() &&*/ depth.z >= z && current_alpha > alpha.a) {
                                                                              imageStore(alphaBuffer,ivec2(gl_FragCoord.xy),vec4(0, l, z, current_alpha));
                                                                              memoryBarrier();
                                                                              fColor = vec4(0, l, z, current_alpha);
                                                                          } else {
                                                                              fColor = alpha;
                                                                          }
                                                                          endInvocationInterlockARB();
                                                                      }
                                                                      )";
                        const std::string specularGenFragShader = R"(#version 460
                                                                     #extension GL_EXT_nonuniform_qualifier : enable
                                                                     layout (location = 0) in vec2 fTexCoords;
                                                                     layout (location = 1) in vec4 frontColor;
                                                                     layout (location = 2) in flat uint texIndex;
                                                                     layout (location = 3) in flat uint layer;
                                                                     layout (location = 4) in vec3 normal;
                                                                     layout (location = 5) in vec2 specular;
                                                                     layout (push_constant) uniform PushConsts {
                                                                         float maxP;
                                                                         float maxM;
                                                                     } pushConsts;
                                                                     layout(set = 0, binding = 0) uniform sampler2D textures[];
                                                                     layout(set = 0, binding = 1) uniform sampler2D depthBuffer;
                                                                     layout (location = 0) out vec4 fColor;
                                                                     void main() {
                                                                        vec4 texel = (texIndex != 0) ? frontColor * texture(textures[texIndex-1], fTexCoords.xy) : frontColor;
                                                                        vec4 depth = texture(depthBuffer, gl_FragCoord.xy);
                                                                        float z = gl_FragCoord.z;
                                                                        float intensity = (pushConsts.maxM != 0.f) ? specular.x / pushConsts.maxM : 0.f;
                                                                        float power = (pushConsts.maxP != 0.f) ? specular.y / pushConsts.maxP : 0.f;
                                                                        if (/*layer > depth.y || layer == depth.y &&*/ z > depth.z)
                                                                            fColor = vec4(intensity, power, z, texel.a);
                                                                        else
                                                                            fColor = vec4(specular.x, specular.y, 0, 1);
                                                                     }
                                                                  )";
                        const std::string bumpGenFragShader =    R"(#version 460
                                                                    #extension GL_EXT_nonuniform_qualifier : enable
                                                                 layout(set = 0, binding = 0) uniform sampler2D textures[];
                                                                 layout (location = 0) in vec2 fTexCoords;
                                                                 layout (location = 1) in vec4 frontColor;
                                                                 layout (location = 2) in flat uint texIndex;
                                                                 layout (location = 3) in flat uint layer;
                                                                 layout (location = 4) in vec3 normal;
                                                                 layout(set = 0, binding = 1) uniform sampler2D depthBuffer;
                                                                 layout(set = 0, binding = 2) uniform sampler2D bumpMap;
                                                                 layout (location = 0) out vec4 fColor;
                                                                 void main() {
                                                                     vec4 color = (texIndex != 0) ? texture(textures[texIndex-1], fTexCoords.xy) : vec4(0, 0, 0, 0);
                                                                     vec4 depth = texture(depthBuffer, gl_FragCoord.xy);
                                                                     vec4 bump = texture(bumpMap, gl_FragCoord.xy);
                                                                     if (/*layer > depth.y() || layer == depth.y() &&*/ gl_FragCoord.z > depth.z) {
                                                                        fColor = color;
                                                                     } else {
                                                                        fColor = bump;
                                                                     }
                                                                 }
                                                                 )";
                        const std::string perPixLightingFragmentShader =  R"(#version 460
                                                                             #extension GL_EXT_nonuniform_qualifier : enable
                                                                 layout (location = 0) in vec2 fTexCoords;
                                                                 layout (location = 1) in vec4 frontColor;
                                                                 layout (location = 2) in flat uint texIndex;
                                                                 layout (location = 3) in flat uint layer;
                                                                 layout (location = 4) in vec3 normal;
                                                                 layout (location = 5) in flat vec4 lightColor;
                                                                 layout (location = 6) in flat vec4 lightPos;
                                                                 const vec2 size = vec2(2.0,0.0);
                                                                 const ivec3 off = ivec3(-1,0,1);
                                                                 layout(set = 0, binding = 0) uniform sampler2D depthTexture;
                                                                 layout(set = 0, binding = 1) uniform sampler2D bumpMap;
                                                                 layout(set = 0, binding = 2) uniform sampler2D specularTexture;
                                                                 layout(set = 0, binding = 3) uniform sampler2D alphaMap;
                                                                 layout(set = 0, binding = 6) uniform sampler2D lightMap;
                                                                 layout (location = 0) out vec4 fColor;
                                                                 layout (push_constant) uniform PushConsts {
                                                                     layout (offset = 128) vec4 resolution;
                                                                     layout (offset = 144) float near;
                                                                     layout (offset = 148) float far;
                                                                 } pushConsts;
                                                         void main () {
                                                             vec4 depth = texture(depthTexture, gl_FragCoord.xy);
                                                             vec4 alpha = texture(alphaMap, gl_FragCoord.xy);
                                                             float s01 = textureOffset(depthTexture, gl_FragCoord.xy, off.xy).z;
                                                             float s21 = textureOffset(depthTexture, gl_FragCoord.xy, off.zy).z;
                                                             float s10 = textureOffset(depthTexture, gl_FragCoord.xy, off.yx).z;
                                                             float s12 = textureOffset(depthTexture, gl_FragCoord.xy, off.yz).z;
                                                             vec3 va = normalize (vec3(size.xy, s21 - s01));
                                                             vec3 vb = normalize (vec3(size.yx, s12 - s10));
                                                             vec3 normal = vec3(cross(va, vb));
                                                             vec4 bump = texture(bumpMap, gl_FragCoord.xy);
                                                             vec4 specularInfos = texture(specularTexture, gl_FragCoord.xy);
                                                             vec3 sLightPos = vec3 (lightPos.x, lightPos.y, -lightPos.z * (pushConsts.far - pushConsts.near));
                                                             float radius = lightPos.w;
                                                             vec3 pixPos = vec3 (gl_FragCoord.x, gl_FragCoord.y, -depth.z * (pushConsts.far - pushConsts.near));
                                                             vec4 lightMapColor = texture(lightMap, gl_FragCoord.xy);
                                                             vec3 viewPos = vec3(pushConsts.resolution.x * 0.5f, pushConsts.resolution.y * 0.5f, 0);
                                                             float z = gl_FragCoord.z;
                                                             vec3 vertexToLight = sLightPos - pixPos;
                                                             if (bump.x != 0 || bump.y != 0 || bump.z != 0) {
                                                                 vec3 tmpNormal = (normal.xyz);
                                                                 vec3 tangeant = normalize (vec3(size.xy, s21 - s01));
                                                                 vec3 binomial = normalize (vec3(size.yx, s12 - s10));
                                                                 normal.x = dot(bump.xyz, tangeant);
                                                                 normal.y = dot(bump.xyz, binomial);
                                                                 normal.z = dot(bump.xyz, tmpNormal);
                                                             }
                                                             if (/*layer > depth.y || layer == depth.y &&*/ z > depth.z) {
                                                                 vec4 specularColor = vec4(0, 0, 0, 0);
                                                                 float attenuation = 1.f - length(vertexToLight) / radius;
                                                                 vec3 pixToView = pixPos - viewPos;
                                                                 float normalLength = dot(normal.xyz, vertexToLight);
                                                                 vec3 lightReflect = vertexToLight + 2 * (normal.xyz * normalLength - vertexToLight);
                                                                 float m = specularInfos.r;
                                                                 float p = specularInfos.g;
                                                                 float specularFactor = dot(normalize(pixToView), normalize(lightReflect));
                                                                 specularFactor = pow (specularFactor, p);
                                                                 if (specularFactor > 0) {
                                                                     specularColor = vec4(lightColor.rgb, 1) * m * specularFactor;
                                                                 }
                                                                 if (normal.x != 0 || normal.y != 0 || normal.z != 0 && vertexToLight.z > 0.f) {
                                                                     vec3 dirToLight = normalize(vertexToLight.xyz);
                                                                     float nDotl = max(dot (dirToLight, normal.xyz), 0.0);
                                                                     attenuation *= nDotl;

                                                                 }
                                                                 fColor = vec4(lightColor.rgb, 1) * max(0.0f, attenuation) + specularColor * (1 - alpha.a);
                                                             } else {
                                                                 fColor = lightMapColor;
                                                             }
                                                         }
                                                     )";
                        if (!depthBufferGenerator.loadFromMemory(indirectRenderingVertexShader, depthGenFragShader))
                            throw core::Erreur(50, "Failed to load depth buffer generator shader", 0);
                        if (!specularTextureGenerator.loadFromMemory(specularIndirectRenderingVertexShader, specularGenFragShader))
                            throw core::Erreur(52, "Failed to load specular texture generator shader", 0);

                        if (!bumpTextureGenerator.loadFromMemory(indirectRenderingVertexShader, bumpGenFragShader))
                            throw core::Erreur(53, "Failed to load bump texture generator shader", 0);
                        if (!buildAlphaBufferGenerator.loadFromMemory(indirectRenderingVertexShader, buildAlphaBufferFragmentShader))
                            throw core::Erreur(53, "Failed to load build alpha buffer generator shader", 0);

                        if (!lightMapGenerator.loadFromMemory(perPixLightingIndirectRenderingVertexShader, perPixLightingFragmentShader))
                            throw core::Erreur(54, "Failed to load light map generator shader", 0);
            }
            void LightRenderComponent::createCommandPool() {
                window::Device::QueueFamilyIndices queueFamilyIndices = vkDevice.findQueueFamilies(vkDevice.getPhysicalDevice(), VK_NULL_HANDLE);

                VkCommandPoolCreateInfo poolInfo{};
                poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
                poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optionel
                if (vkCreateCommandPool(vkDevice.getDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
                    throw core::Erreur(0, "�chec de la cr�ation d'une command pool!", 1);
                }
            }
            uint32_t LightRenderComponent::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
                VkPhysicalDeviceMemoryProperties memProperties;
                vkGetPhysicalDeviceMemoryProperties(vkDevice.getPhysicalDevice(), &memProperties);
                for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
                    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                        return i;
                    }
                }
                throw std::runtime_error("aucun type de memoire ne satisfait le buffer!");
            }
            void LightRenderComponent::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
                VkBufferCreateInfo bufferInfo{};
                bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                bufferInfo.size = size;
                bufferInfo.usage = usage;
                bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

                if (vkCreateBuffer(vkDevice.getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create buffer!");
                }

                VkMemoryRequirements memRequirements;
                vkGetBufferMemoryRequirements(vkDevice.getDevice(), buffer, &memRequirements);

                VkMemoryAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.allocationSize = memRequirements.size;
                allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

                if (vkAllocateMemory(vkDevice.getDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
                    throw std::runtime_error("failed to allocate buffer memory!");
                }

                vkBindBufferMemory(vkDevice.getDevice(), buffer, bufferMemory, 0);

            }
            void LightRenderComponent::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
                VkCommandBufferAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                allocInfo.commandPool = commandPool;
                allocInfo.commandBufferCount = 1;

                VkCommandBuffer commandBuffer;
                vkAllocateCommandBuffers(vkDevice.getDevice(), &allocInfo, &commandBuffer);

                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

                vkBeginCommandBuffer(commandBuffer, &beginInfo);

                    VkBufferCopy copyRegion{};
                    copyRegion.size = size;
                    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

                vkEndCommandBuffer(commandBuffer);

                VkSubmitInfo submitInfo{};
                submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &commandBuffer;

                vkQueueSubmit(vkDevice.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
                vkQueueWaitIdle(vkDevice.getGraphicsQueue());

                vkFreeCommandBuffers(vkDevice.getDevice(), commandPool, 1, &commandBuffer);
            }
            void LightRenderComponent::clear() {
                depthBuffer.clear(Color::Transparent);
                std::vector<VkCommandBuffer> commandBuffers = depthBuffer.getCommandBuffers();
                VkClearColorValue clearColor = {0.f, 0.f, 0.f, 0.f};
                VkImageSubresourceRange subresRange = {};
                subresRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                subresRange.levelCount = 1;
                subresRange.layerCount = 1;
                for (unsigned int i = 0; i < commandBuffers.size(); i++) {
                    vkCmdClearColorImage(commandBuffers[i], depthTextureImage, VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &subresRange);
                    VkMemoryBarrier memoryBarrier;
                    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                    memoryBarrier.pNext = VK_NULL_HANDLE;
                    memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                    vkCmdPipelineBarrier(commandBuffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                }
                alphaBuffer.clear(Color::Transparent);
                commandBuffers = alphaBuffer.getCommandBuffers();
                for (unsigned int i = 0; i < commandBuffers.size(); i++) {
                    vkCmdClearColorImage(commandBuffers[i], alphaTextureImage, VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &subresRange);
                    VkMemoryBarrier memoryBarrier;
                    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                    memoryBarrier.pNext = VK_NULL_HANDLE;
                    memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                    vkCmdPipelineBarrier(commandBuffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                }
                specularTexture.clear(Color::Transparent);
                bumpTexture.clear(Color::Transparent);
                lightMap.beginRecordCommandBuffers();
                const_cast<Texture&>(lightMap.getTexture()).toColorAttachmentOptimal(lightMap.getCommandBuffers()[lightMap.getCurrentFrame()]);
                lightMap.display();
                Color ambientColor = g2d::AmbientLight::getAmbientLight().getColor();
                lightMap.clear(ambientColor);
            }
            void LightRenderComponent::createImageView() {
                VkImageViewCreateInfo viewInfo{};
                viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                viewInfo.image = depthTextureImage;
                viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                viewInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                viewInfo.subresourceRange.baseMipLevel = 0;
                viewInfo.subresourceRange.levelCount = 1;
                viewInfo.subresourceRange.baseArrayLayer = 0;
                viewInfo.subresourceRange.layerCount = 1;
                if (vkCreateImageView(vkDevice.getDevice(), &viewInfo, nullptr, &depthTextureImageView) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create head ptr texture image view!");
                }
                VkImageViewCreateInfo viewInfo2{};
                viewInfo2.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                viewInfo2.image = alphaTextureImage;
                viewInfo2.viewType = VK_IMAGE_VIEW_TYPE_2D;
                viewInfo2.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                viewInfo2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                viewInfo2.subresourceRange.baseMipLevel = 0;
                viewInfo2.subresourceRange.levelCount = 1;
                viewInfo2.subresourceRange.baseArrayLayer = 0;
                viewInfo2.subresourceRange.layerCount = 1;
                if (vkCreateImageView(vkDevice.getDevice(), &viewInfo2, nullptr, &alphaTextureImageView) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create head ptr texture image view!");
                }
            }
            void LightRenderComponent::createSampler() {
                VkSamplerCreateInfo samplerInfo{};
                samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                samplerInfo.magFilter = VK_FILTER_LINEAR;
                samplerInfo.minFilter = VK_FILTER_LINEAR;
                samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo.anisotropyEnable = VK_TRUE;
                VkPhysicalDeviceProperties properties{};
                vkGetPhysicalDeviceProperties(vkDevice.getPhysicalDevice(), &properties);
                samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
                samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
                samplerInfo.unnormalizedCoordinates = VK_FALSE;
                samplerInfo.compareEnable = VK_FALSE;
                samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
                samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
                samplerInfo.mipLodBias = 0.0f;
                samplerInfo.minLod = 0.0f;
                samplerInfo.maxLod = 0.0f;
                if (vkCreateSampler(vkDevice.getDevice(), &samplerInfo, nullptr, &depthTextureSampler) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create texture sampler!");
                }
                VkSamplerCreateInfo samplerInfo2{};
                samplerInfo2.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                samplerInfo2.magFilter = VK_FILTER_LINEAR;
                samplerInfo2.minFilter = VK_FILTER_LINEAR;
                samplerInfo2.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo2.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo2.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo2.anisotropyEnable = VK_TRUE;
                vkGetPhysicalDeviceProperties(vkDevice.getPhysicalDevice(), &properties);
                samplerInfo2.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
                samplerInfo2.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
                samplerInfo2.unnormalizedCoordinates = VK_FALSE;
                samplerInfo2.compareEnable = VK_FALSE;
                samplerInfo2.compareOp = VK_COMPARE_OP_ALWAYS;
                samplerInfo2.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
                samplerInfo2.mipLodBias = 0.0f;
                samplerInfo2.minLod = 0.0f;
                samplerInfo2.maxLod = 0.0f;
                if (vkCreateSampler(vkDevice.getDevice(), &samplerInfo2, nullptr, &alphaTextureSampler) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create texture sampler!");
                }
            }
            void LightRenderComponent::createDescriptorsAndPipelines () {
                RenderStates states;
                states.shader = &depthBufferGenerator;
                createDescriptorPool(states);
                createDescriptorSetLayout(states);
                allocateDescriptorSets(states);
                states.shader = &buildAlphaBufferGenerator;
                createDescriptorPool(states);
                createDescriptorSetLayout(states);
                allocateDescriptorSets(states);
                states.shader = &specularTextureGenerator;
                createDescriptorPool(states);
                createDescriptorSetLayout(states);
                allocateDescriptorSets(states);
                states.shader = &bumpTextureGenerator;
                createDescriptorPool(states);
                createDescriptorSetLayout(states);
                allocateDescriptorSets(states);
                states.shader = &lightMapGenerator;
                createDescriptorPool(states);
                createDescriptorSetLayout(states);
                allocateDescriptorSets(states);
                BlendMode none = BlendNone;
                BlendMode alpha = BlendAlpha;
                BlendMode add = BlendAdd;
                BlendMode multiply = BlendMultiply;
                none.updateIds();
                std::vector<BlendMode> blendModes;
                blendModes.push_back(none);
                blendModes.push_back(alpha);
                blendModes.push_back(add);
                blendModes.push_back(multiply);
                std::vector<std::vector<std::vector<VkPipelineLayoutCreateInfo>>>& pipelineLayoutInfo = lightMap.getPipelineLayoutCreateInfo();
                std::vector<std::vector<std::vector<VkPipelineDepthStencilStateCreateInfo>>>& depthStencilCreateInfo = lightMap.getDepthStencilCreateInfo();
                if ((Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders() > pipelineLayoutInfo.size()) {
                    pipelineLayoutInfo.resize((Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders());
                    depthStencilCreateInfo.resize((Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders());
                }
                for (unsigned int i = 0; i < (Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders(); i++) {
                    if (lightMap.getNbRenderTargets() > pipelineLayoutInfo[i].size()) {
                        pipelineLayoutInfo[i].resize(lightMap.getNbRenderTargets());
                        depthStencilCreateInfo[i].resize(lightMap.getNbRenderTargets());
                    }
                }
                for (unsigned int i = 0; i < (Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders(); i++) {
                    for (unsigned int j = 0; j < lightMap.getNbRenderTargets(); j++) {
                        if (NBDEPTHSTENCIL * none.nbBlendModes > pipelineLayoutInfo[i][j].size()) {
                            pipelineLayoutInfo[i][j].resize(NBDEPTHSTENCIL * none.nbBlendModes);
                            depthStencilCreateInfo[i][j].resize(NBDEPTHSTENCIL* none.nbBlendModes);
                        }
                    }
                }
                specularTexture.enableDepthTest(true);
                depthBuffer.enableDepthTest(true);
                alphaBuffer.enableDepthTest(true);
                bumpTexture.enableDepthTest(true);
                states.shader = &depthBufferGenerator;
                for (unsigned int b = 0; b < blendModes.size(); b++) {
                    states.blendMode = blendModes[b];
                    states.blendMode.updateIds();
                    for (unsigned int j = 0; j < NBDEPTHSTENCIL; j++) {
                        for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes - 1; i++) {
                            if (j == 0) {
                                std::array<VkPushConstantRange, 2> push_constants;
                                VkPushConstantRange push_constant;
                                //this push constant range starts at the beginning
                                push_constant.offset = 0;
                                //this push constant range takes up the size of a MeshPushConstants struct
                                push_constant.size = sizeof(IndirectRenderingPC);
                                //this push constant range is accessible only in the vertex shader
                                push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                                push_constants[0] = push_constant;
                                VkPushConstantRange push_constant2;
                                //this push constant range starts at the beginning
                                push_constant2.offset = 128;
                                //this push constant range takes up the size of a MeshPushConstants struct
                                push_constant2.size = sizeof(LayerPC);
                                //this push constant range is accessible only in the vertex shader
                                push_constant2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                                push_constants[1] = push_constant2;
                                pipelineLayoutInfo[depthBufferGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][depthBuffer.getId()][NODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = push_constants.data();
                                pipelineLayoutInfo[depthBufferGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][depthBuffer.getId()][NODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 2;
                               depthStencilCreateInfo[depthBufferGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][depthBuffer.getId()][NODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                               depthStencilCreateInfo[depthBufferGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][depthBuffer.getId()][NODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = {};
                               depthStencilCreateInfo[depthBufferGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][depthBuffer.getId()][NODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back = {};
                               depthBuffer.createGraphicPipeline(static_cast<PrimitiveType>(i), states, NODEPTHNOSTENCIL, NBDEPTHSTENCIL);
                            }
                        }
                    }
                }
                for (unsigned int b = 0; b < blendModes.size(); b++) {
                    states.blendMode = blendModes[b];
                    states.blendMode.updateIds();
                    states.shader = &buildAlphaBufferGenerator;
                    for (unsigned int j = 0; j < NBDEPTHSTENCIL; j++) {
                        for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes - 1; i++) {
                            if (j == 0) {
                                VkPushConstantRange push_constant;
                                //this push constant range takes up the size of a MeshPushConstants struct
                                push_constant.offset = 0;
                                push_constant.size = sizeof(LayerPC);
                                //this push constant range is accessible only in the vertex shader
                                push_constant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                                pipelineLayoutInfo[buildAlphaBufferGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][alphaBuffer.getId()][NODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = &push_constant;
                                pipelineLayoutInfo[buildAlphaBufferGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][alphaBuffer.getId()][NODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 1;
                               depthStencilCreateInfo[buildAlphaBufferGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][alphaBuffer.getId()][NODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                               depthStencilCreateInfo[buildAlphaBufferGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][alphaBuffer.getId()][NODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = {};
                               depthStencilCreateInfo[buildAlphaBufferGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][alphaBuffer.getId()][NODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back = {};
                               alphaBuffer.createGraphicPipeline(static_cast<PrimitiveType>(i), states, NODEPTHNOSTENCIL, NBDEPTHSTENCIL);
                            }
                        }
                    }
                }
                for (unsigned int b = 0; b < blendModes.size(); b++) {
                    states.blendMode = blendModes[b];
                    states.blendMode.updateIds();
                    states.shader = &specularTextureGenerator;
                    for (unsigned int j = 0; j < NBDEPTHSTENCIL; j++) {
                        for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes - 1; i++) {
                            if (j == 0) {
                                VkPushConstantRange push_constant;
                                //this push constant range takes up the size of a MeshPushConstants struct
                                push_constant.offset = 0;
                                push_constant.size = sizeof(IndirectRenderingPC);
                                //this push constant range is accessible only in the vertex shader
                                push_constant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                                pipelineLayoutInfo[specularTextureGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][specularTexture.getId()][NODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = &push_constant;
                                pipelineLayoutInfo[specularTextureGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][specularTexture.getId()][NODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 1;
                               depthStencilCreateInfo[specularTextureGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][specularTexture.getId()][NODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                               depthStencilCreateInfo[specularTextureGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][specularTexture.getId()][NODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = {};
                               depthStencilCreateInfo[specularTextureGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][specularTexture.getId()][NODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back = {};
                               specularTexture.createGraphicPipeline(static_cast<PrimitiveType>(i), states, NODEPTHNOSTENCIL, NBDEPTHSTENCIL);
                            }
                        }
                    }
                }
                for (unsigned int b = 0; b < blendModes.size(); b++) {
                    states.blendMode = blendModes[b];
                    states.blendMode.updateIds();
                    states.shader = &specularTextureGenerator;
                    for (unsigned int j = 0; j < NBDEPTHSTENCIL; j++) {
                        for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes - 1; i++) {
                            if (j == 0) {
                                std::array<VkPushConstantRange, 2> push_constants;
                                VkPushConstantRange push_constant;
                                //this push constant range starts at the beginning
                                push_constant.offset = 0;
                                //this push constant range takes up the size of a MeshPushConstants struct
                                push_constant.size = sizeof(LightIndirectRenderingPC);
                                //this push constant range is accessible only in the vertex shader
                                push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                                push_constants[0] = push_constant;
                                VkPushConstantRange push_constant2;
                                //this push constant range starts at the beginning
                                push_constant2.offset = 128;
                                //this push constant range takes up the size of a MeshPushConstants struct
                                push_constant2.size = sizeof(ResolutionPC);
                                //this push constant range is accessible only in the vertex shader
                                push_constant2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                                push_constants[1] = push_constant2;

                                pipelineLayoutInfo[bumpTextureGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][bumpTexture.getId()][NODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = push_constants.data();
                                pipelineLayoutInfo[bumpTextureGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][bumpTexture.getId()][NODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 2;
                               depthStencilCreateInfo[bumpTextureGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][bumpTexture.getId()][NODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                               depthStencilCreateInfo[bumpTextureGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][bumpTexture.getId()][NODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = {};
                               depthStencilCreateInfo[bumpTextureGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][bumpTexture.getId()][NODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back = {};
                               bumpTexture.createGraphicPipeline(static_cast<PrimitiveType>(i), states, NODEPTHNOSTENCIL, NBDEPTHSTENCIL);
                            }
                        }
                    }
                }
                for (unsigned int b = 0; b < blendModes.size(); b++) {
                    states.blendMode = blendModes[b];
                    states.blendMode.updateIds();
                    states.shader = &lightMapGenerator;
                    for (unsigned int j = 0; j < NBDEPTHSTENCIL; j++) {
                        for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes - 1; i++) {
                            if (j == 0) {
                                VkPushConstantRange push_constant;
                                //this push constant range takes up the size of a MeshPushConstants struct
                                push_constant.offset = 0;
                                push_constant.size = sizeof(LightIndirectRenderingPC);
                                //this push constant range is accessible only in the vertex shader
                                push_constant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                                pipelineLayoutInfo[lightMapGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][lightMap.getId()][NODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = &push_constant;
                                pipelineLayoutInfo[lightMapGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][lightMap.getId()][NODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 1;
                               depthStencilCreateInfo[lightMapGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][lightMap.getId()][NODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                               depthStencilCreateInfo[lightMapGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][lightMap.getId()][NODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = {};
                               depthStencilCreateInfo[lightMapGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][lightMap.getId()][NODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back = {};
                               lightMap.createGraphicPipeline(static_cast<PrimitiveType>(i), states, NODEPTHNOSTENCIL, NBDEPTHSTENCIL);
                            }
                        }
                    }
                }
            }
            void LightRenderComponent::createDescriptorPool(RenderStates states) {
                Shader* shader = const_cast<Shader*>(states.shader);
                if (shader == &depthBufferGenerator) {
                    if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders()*depthBuffer.getNbRenderTargets());
                    unsigned int descriptorId = depthBuffer.getId() * shader->getNbShaders() + shader->getId();
                    std::array<VkDescriptorPoolSize, 4> poolSizes;
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight() * allTextures.size());
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());
                    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[2].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());
                    poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[3].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());

                    if (descriptorPool[descriptorId] != nullptr) {
                        vkDestroyDescriptorPool(vkDevice.getDevice(), descriptorPool[descriptorId], nullptr);
                    }

                    VkDescriptorPoolCreateInfo poolInfo{};
                    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
                    poolInfo.pPoolSizes = poolSizes.data();
                    poolInfo.maxSets = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());
                    if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de la creation de la pool de descripteurs!");
                    }
                } else if (shader == &buildAlphaBufferGenerator) {
                    if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders()*alphaBuffer.getNbRenderTargets());
                    unsigned int descriptorId = alphaBuffer.getId() * shader->getNbShaders() + shader->getId();
                    std::array<VkDescriptorPoolSize, 5> poolSizes;
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight() * allTextures.size());
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight());
                    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[2].descriptorCount = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight());
                    poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[3].descriptorCount = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight());
                    poolSizes[4].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[4].descriptorCount = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight());

                    if (descriptorPool[descriptorId] != nullptr) {
                        vkDestroyDescriptorPool(vkDevice.getDevice(), descriptorPool[descriptorId], nullptr);
                    }

                    VkDescriptorPoolCreateInfo poolInfo{};
                    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
                    poolInfo.pPoolSizes = poolSizes.data();
                    poolInfo.maxSets = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight());
                    if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de la creation de la pool de descripteurs!");
                    }
                } else if (shader == &specularTextureGenerator) {
                    if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders()*specularTexture.getNbRenderTargets());
                    unsigned int descriptorId = specularTexture.getId() * shader->getNbShaders() + shader->getId();
                    std::array<VkDescriptorPoolSize, 4> poolSizes;
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(specularTexture.getMaxFramesInFlight() * allTextures.size());
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(specularTexture.getMaxFramesInFlight());
                    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[2].descriptorCount = static_cast<uint32_t>(specularTexture.getMaxFramesInFlight());
                    poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[3].descriptorCount = static_cast<uint32_t>(specularTexture.getMaxFramesInFlight());
                } else if (shader == &bumpTextureGenerator) {
                    if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders()*bumpTexture.getNbRenderTargets());
                    unsigned int descriptorId = bumpTexture.getId() * shader->getNbShaders() + shader->getId();
                    std::array<VkDescriptorPoolSize, 5> poolSizes;
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(bumpTexture.getMaxFramesInFlight() * allTextures.size());
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(bumpTexture.getMaxFramesInFlight());
                    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[2].descriptorCount = static_cast<uint32_t>(bumpTexture.getMaxFramesInFlight());
                    poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[3].descriptorCount = static_cast<uint32_t>(bumpTexture.getMaxFramesInFlight());
                    poolSizes[4].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[4].descriptorCount = static_cast<uint32_t>(bumpTexture.getMaxFramesInFlight());

                } else {
                    if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders()*lightMap.getNbRenderTargets());
                    unsigned int descriptorId = lightMap.getId() * shader->getNbShaders() + shader->getId();
                    std::array<VkDescriptorPoolSize, 6> poolSizes;
                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(lightMap.getMaxFramesInFlight());
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(lightMap.getMaxFramesInFlight());
                    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[2].descriptorCount = static_cast<uint32_t>(lightMap.getMaxFramesInFlight());
                    poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[3].descriptorCount = static_cast<uint32_t>(lightMap.getMaxFramesInFlight());
                    poolSizes[4].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[4].descriptorCount = static_cast<uint32_t>(lightMap.getMaxFramesInFlight());
                    poolSizes[5].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[5].descriptorCount = static_cast<uint32_t>(lightMap.getMaxFramesInFlight());
                }
            }
            void LightRenderComponent::createDescriptorSetLayout(RenderStates states) {
                Shader* shader = const_cast<Shader*>(states.shader);
                if (shader == &depthBufferGenerator) {
                    if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorSetLayout.size())
                        descriptorSetLayout.resize(shader->getNbShaders()*depthBuffer.getNbRenderTargets());
                    unsigned int descriptorId = depthBuffer.getId() * shader->getNbShaders() + shader->getId();
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
                    samplerLayoutBinding.binding = 0;
                    samplerLayoutBinding.descriptorCount = allTextures.size();
                    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding.pImmutableSamplers = nullptr;
                    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding headPtrImageLayoutBinding;
                    headPtrImageLayoutBinding.binding = 1;
                    headPtrImageLayoutBinding.descriptorCount = 1;
                    headPtrImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    headPtrImageLayoutBinding.pImmutableSamplers = nullptr;
                    headPtrImageLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding modelDataLayoutBinding{};
                    modelDataLayoutBinding.binding = 4;
                    modelDataLayoutBinding.descriptorCount = 1;
                    modelDataLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    modelDataLayoutBinding.pImmutableSamplers = nullptr;
                    modelDataLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                    VkDescriptorSetLayoutBinding materialDataLayoutBinding{};
                    materialDataLayoutBinding.binding = 5;
                    materialDataLayoutBinding.descriptorCount = 1;
                    materialDataLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    materialDataLayoutBinding.pImmutableSamplers = nullptr;
                    materialDataLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                    std::array<VkDescriptorSetLayoutBinding, 4> bindings = {samplerLayoutBinding, headPtrImageLayoutBinding, modelDataLayoutBinding, materialDataLayoutBinding};

                    if (descriptorSetLayout[descriptorId] != nullptr) {
                        vkDestroyDescriptorSetLayout(vkDevice.getDevice(), descriptorSetLayout[descriptorId], nullptr);
                    }

                    VkDescriptorSetLayoutCreateInfo layoutInfo{};
                    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                    //layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
                    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());;
                    layoutInfo.pBindings = bindings.data();

                    if (vkCreateDescriptorSetLayout(vkDevice.getDevice(), &layoutInfo, nullptr, &descriptorSetLayout[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create descriptor set layout!");
                    }
                } else if (shader == &buildAlphaBufferGenerator) {
                    if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorSetLayout.size())
                        descriptorSetLayout.resize(shader->getNbShaders()*alphaBuffer.getNbRenderTargets());
                    unsigned int descriptorId = alphaBuffer.getId() * shader->getNbShaders() + shader->getId();
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
                    samplerLayoutBinding.binding = 0;
                    samplerLayoutBinding.descriptorCount = allTextures.size();
                    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding.pImmutableSamplers = nullptr;
                    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding headPtrImageLayoutBinding;
                    headPtrImageLayoutBinding.binding = 1;
                    headPtrImageLayoutBinding.descriptorCount = 1;
                    headPtrImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    headPtrImageLayoutBinding.pImmutableSamplers = nullptr;
                    headPtrImageLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding samplerLayoutBinding2{};
                    samplerLayoutBinding2.binding = 2;
                    samplerLayoutBinding2.descriptorCount = 1;
                    samplerLayoutBinding2.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding2.pImmutableSamplers = nullptr;
                    samplerLayoutBinding2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding modelDataLayoutBinding{};
                    modelDataLayoutBinding.binding = 4;
                    modelDataLayoutBinding.descriptorCount = 1;
                    modelDataLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    modelDataLayoutBinding.pImmutableSamplers = nullptr;
                    modelDataLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                    VkDescriptorSetLayoutBinding materialDataLayoutBinding{};
                    materialDataLayoutBinding.binding = 5;
                    materialDataLayoutBinding.descriptorCount = 1;
                    materialDataLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    materialDataLayoutBinding.pImmutableSamplers = nullptr;
                    materialDataLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                    std::array<VkDescriptorSetLayoutBinding, 5> bindings = {samplerLayoutBinding, headPtrImageLayoutBinding, samplerLayoutBinding2, modelDataLayoutBinding, materialDataLayoutBinding};

                    if (descriptorSetLayout[descriptorId] != nullptr) {
                        vkDestroyDescriptorSetLayout(vkDevice.getDevice(), descriptorSetLayout[descriptorId], nullptr);
                    }

                    VkDescriptorSetLayoutCreateInfo layoutInfo{};
                    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                    //layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
                    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());;
                    layoutInfo.pBindings = bindings.data();

                    if (vkCreateDescriptorSetLayout(vkDevice.getDevice(), &layoutInfo, nullptr, &descriptorSetLayout[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create descriptor set layout!");
                    }
                } else if (shader == &specularTextureGenerator) {
                    if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorSetLayout.size())
                        descriptorSetLayout.resize(shader->getNbShaders()*specularTexture.getNbRenderTargets());
                    unsigned int descriptorId = specularTexture.getId() * shader->getNbShaders() + shader->getId();
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
                    samplerLayoutBinding.binding = 0;
                    samplerLayoutBinding.descriptorCount = allTextures.size();
                    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding.pImmutableSamplers = nullptr;
                    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding samplerLayoutBinding2{};
                    samplerLayoutBinding2.binding = 2;
                    samplerLayoutBinding2.descriptorCount = 1;
                    samplerLayoutBinding2.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding2.pImmutableSamplers = nullptr;
                    samplerLayoutBinding2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding modelDataLayoutBinding{};
                    modelDataLayoutBinding.binding = 4;
                    modelDataLayoutBinding.descriptorCount = 1;
                    modelDataLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    modelDataLayoutBinding.pImmutableSamplers = nullptr;
                    modelDataLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                    VkDescriptorSetLayoutBinding materialDataLayoutBinding{};
                    materialDataLayoutBinding.binding = 5;
                    materialDataLayoutBinding.descriptorCount = 1;
                    materialDataLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    materialDataLayoutBinding.pImmutableSamplers = nullptr;
                    materialDataLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                    std::array<VkDescriptorSetLayoutBinding, 4> bindings = {samplerLayoutBinding, samplerLayoutBinding2, modelDataLayoutBinding, materialDataLayoutBinding};

                    if (descriptorSetLayout[descriptorId] != nullptr) {
                        vkDestroyDescriptorSetLayout(vkDevice.getDevice(), descriptorSetLayout[descriptorId], nullptr);
                    }

                    VkDescriptorSetLayoutCreateInfo layoutInfo{};
                    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                    //layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
                    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());;
                    layoutInfo.pBindings = bindings.data();

                    if (vkCreateDescriptorSetLayout(vkDevice.getDevice(), &layoutInfo, nullptr, &descriptorSetLayout[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create descriptor set layout!");
                    }
                } else if (shader == &bumpTextureGenerator) {
                    if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorSetLayout.size())
                        descriptorSetLayout.resize(shader->getNbShaders()*bumpTexture.getNbRenderTargets());
                    unsigned int descriptorId = bumpTexture.getId() * shader->getNbShaders() + shader->getId();
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
                    samplerLayoutBinding.binding = 0;
                    samplerLayoutBinding.descriptorCount = allTextures.size();
                    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding.pImmutableSamplers = nullptr;
                    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding samplerLayoutBinding2{};
                    samplerLayoutBinding2.binding = 1;
                    samplerLayoutBinding2.descriptorCount = 1;
                    samplerLayoutBinding2.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding2.pImmutableSamplers = nullptr;
                    samplerLayoutBinding2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding samplerLayoutBinding3{};
                    samplerLayoutBinding2.binding = 2;
                    samplerLayoutBinding2.descriptorCount = 1;
                    samplerLayoutBinding2.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding2.pImmutableSamplers = nullptr;
                    samplerLayoutBinding2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding modelDataLayoutBinding{};
                    modelDataLayoutBinding.binding = 4;
                    modelDataLayoutBinding.descriptorCount = 1;
                    modelDataLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    modelDataLayoutBinding.pImmutableSamplers = nullptr;
                    modelDataLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                    VkDescriptorSetLayoutBinding materialDataLayoutBinding{};
                    materialDataLayoutBinding.binding = 5;
                    materialDataLayoutBinding.descriptorCount = 1;
                    materialDataLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    materialDataLayoutBinding.pImmutableSamplers = nullptr;
                    materialDataLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                    std::array<VkDescriptorSetLayoutBinding, 5> bindings = {samplerLayoutBinding, samplerLayoutBinding2, samplerLayoutBinding3, modelDataLayoutBinding, materialDataLayoutBinding};

                    if (descriptorSetLayout[descriptorId] != nullptr) {
                        vkDestroyDescriptorSetLayout(vkDevice.getDevice(), descriptorSetLayout[descriptorId], nullptr);
                    }

                    VkDescriptorSetLayoutCreateInfo layoutInfo{};
                    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                    //layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
                    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());;
                    layoutInfo.pBindings = bindings.data();

                    if (vkCreateDescriptorSetLayout(vkDevice.getDevice(), &layoutInfo, nullptr, &descriptorSetLayout[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create descriptor set layout!");
                    }
                } else {
                    if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorSetLayout.size())
                        descriptorSetLayout.resize(shader->getNbShaders()*lightMap.getNbRenderTargets());
                    unsigned int descriptorId = lightMap.getId() * shader->getNbShaders() + shader->getId();
                    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
                    samplerLayoutBinding.binding = 0;
                    samplerLayoutBinding.descriptorCount = 1;
                    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding.pImmutableSamplers = nullptr;
                    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding samplerLayoutBinding2{};
                    samplerLayoutBinding2.binding = 1;
                    samplerLayoutBinding2.descriptorCount = 1;
                    samplerLayoutBinding2.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding2.pImmutableSamplers = nullptr;
                    samplerLayoutBinding2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding samplerLayoutBinding3{};
                    samplerLayoutBinding3.binding = 2;
                    samplerLayoutBinding3.descriptorCount = 1;
                    samplerLayoutBinding3.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding3.pImmutableSamplers = nullptr;
                    samplerLayoutBinding3.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding samplerLayoutBinding4{};
                    samplerLayoutBinding4.binding = 3;
                    samplerLayoutBinding4.descriptorCount = 1;
                    samplerLayoutBinding4.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding4.pImmutableSamplers = nullptr;
                    samplerLayoutBinding4.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding modelDataLayoutBinding{};
                    modelDataLayoutBinding.binding = 4;
                    modelDataLayoutBinding.descriptorCount = 1;
                    modelDataLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    modelDataLayoutBinding.pImmutableSamplers = nullptr;
                    modelDataLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                    VkDescriptorSetLayoutBinding materialDataLayoutBinding{};
                    materialDataLayoutBinding.binding = 5;
                    materialDataLayoutBinding.descriptorCount = 1;
                    materialDataLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    materialDataLayoutBinding.pImmutableSamplers = nullptr;
                    materialDataLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                    VkDescriptorSetLayoutBinding samplerLayoutBinding5{};
                    samplerLayoutBinding5.binding = 6;
                    samplerLayoutBinding5.descriptorCount = 1;
                    samplerLayoutBinding5.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding5.pImmutableSamplers = nullptr;
                    samplerLayoutBinding5.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    std::array<VkDescriptorSetLayoutBinding, 7> bindings = {samplerLayoutBinding, samplerLayoutBinding2, samplerLayoutBinding3, samplerLayoutBinding4, modelDataLayoutBinding, materialDataLayoutBinding, samplerLayoutBinding5};

                    if (descriptorSetLayout[descriptorId] != nullptr) {
                        vkDestroyDescriptorSetLayout(vkDevice.getDevice(), descriptorSetLayout[descriptorId], nullptr);
                    }

                    VkDescriptorSetLayoutCreateInfo layoutInfo{};
                    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                    //layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
                    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());;
                    layoutInfo.pBindings = bindings.data();

                    if (vkCreateDescriptorSetLayout(vkDevice.getDevice(), &layoutInfo, nullptr, &descriptorSetLayout[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create descriptor set layout!");
                    }
                }
            }
            void LightRenderComponent::createDescriptorSets(RenderStates states) {
                Shader* shader = const_cast<Shader*>(states.shader);
                if (shader == &depthBufferGenerator) {
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                   unsigned int descriptorId = depthBuffer.getId() * shader->getNbShaders() + shader->getId();
                   for (size_t i = 0; i < depthBuffer.getMaxFramesInFlight(); i++) {
                        std::vector<VkDescriptorImageInfo>	descriptorImageInfos;
                        descriptorImageInfos.resize(allTextures.size());
                        for (unsigned int j = 0; j < allTextures.size(); j++) {
                            descriptorImageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                            descriptorImageInfos[j].imageView = allTextures[j]->getImageView();
                            descriptorImageInfos[j].sampler = allTextures[j]->getSampler();
                        }
                        std::array<VkWriteDescriptorSet, 4> descriptorWrites{};
                        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[0].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[0].dstBinding = 0;
                        descriptorWrites[0].dstArrayElement = 0;
                        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[0].descriptorCount = allTextures.size();
                        descriptorWrites[0].pImageInfo = descriptorImageInfos.data();

                        VkDescriptorImageInfo headPtrDescriptorImageInfo;
                        headPtrDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                        headPtrDescriptorImageInfo.imageView = depthTextureImageView;
                        headPtrDescriptorImageInfo.sampler = depthTextureSampler;

                        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[1].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[1].dstBinding = 1;
                        descriptorWrites[1].dstArrayElement = 0;
                        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                        descriptorWrites[1].descriptorCount = 1;
                        descriptorWrites[1].pImageInfo = &headPtrDescriptorImageInfo;

                        VkDescriptorBufferInfo modelDataStorageBufferInfoLastFrame{};
                        modelDataStorageBufferInfoLastFrame.buffer = modelDataShaderStorageBuffers[i];
                        modelDataStorageBufferInfoLastFrame.offset = 0;
                        modelDataStorageBufferInfoLastFrame.range = maxModelDataSize;

                        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[2].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[2].dstBinding = 4;
                        descriptorWrites[2].dstArrayElement = 0;
                        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                        descriptorWrites[2].descriptorCount = 1;
                        descriptorWrites[2].pBufferInfo = &modelDataStorageBufferInfoLastFrame;

                        VkDescriptorBufferInfo materialDataStorageBufferInfoLastFrame{};
                        materialDataStorageBufferInfoLastFrame.buffer = materialDataShaderStorageBuffers[i];
                        materialDataStorageBufferInfoLastFrame.offset = 0;
                        materialDataStorageBufferInfoLastFrame.range = maxMaterialDataSize;

                        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[3].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[3].dstBinding = 5;
                        descriptorWrites[3].dstArrayElement = 0;
                        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                        descriptorWrites[3].descriptorCount = 1;
                        descriptorWrites[3].pBufferInfo = &materialDataStorageBufferInfoLastFrame;

                        vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
                   }
                } else if (shader == &buildAlphaBufferGenerator) {
                        std::vector<Texture*> allTextures = Texture::getAllTextures();
                        unsigned int descriptorId = alphaBuffer.getId() * shader->getNbShaders() + shader->getId();
                        for (size_t i = 0; i < alphaBuffer.getMaxFramesInFlight(); i++) {
                            std::vector<VkDescriptorImageInfo>	descriptorImageInfos;
                            descriptorImageInfos.resize(allTextures.size());
                            for (unsigned int j = 0; j < allTextures.size(); j++) {
                                descriptorImageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                                descriptorImageInfos[j].imageView = allTextures[j]->getImageView();
                                descriptorImageInfos[j].sampler = allTextures[j]->getSampler();
                            }
                            std::array<VkWriteDescriptorSet, 6> descriptorWrites{};
                            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                            descriptorWrites[0].dstSet = descriptorSets[descriptorId][i];
                            descriptorWrites[0].dstBinding = 0;
                            descriptorWrites[0].dstArrayElement = 0;
                            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                            descriptorWrites[0].descriptorCount = allTextures.size();
                            descriptorWrites[0].pImageInfo = descriptorImageInfos.data();

                            VkDescriptorImageInfo headPtrDescriptorImageInfo;
                            headPtrDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                            headPtrDescriptorImageInfo.imageView = alphaTextureImageView;
                            headPtrDescriptorImageInfo.sampler = alphaTextureSampler;

                            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                            descriptorWrites[1].dstSet = descriptorSets[descriptorId][i];
                            descriptorWrites[1].dstBinding = 1;
                            descriptorWrites[1].dstArrayElement = 0;
                            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                            descriptorWrites[1].descriptorCount = 1;
                            descriptorWrites[1].pImageInfo = &headPtrDescriptorImageInfo;

                            VkDescriptorImageInfo	descriptorImageInfo;
                            descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                            descriptorImageInfo.imageView = depthBuffer.getTexture().getImageView();
                            descriptorImageInfo.sampler = depthBuffer.getTexture().getSampler();

                            descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                            descriptorWrites[2].dstSet = descriptorSets[descriptorId][i];
                            descriptorWrites[2].dstBinding = 2;
                            descriptorWrites[2].dstArrayElement = 0;
                            descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                            descriptorWrites[2].descriptorCount = 1;
                            descriptorWrites[2].pImageInfo = &descriptorImageInfo;

                            descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                            descriptorWrites[3].dstSet = descriptorSets[descriptorId][i];
                            descriptorWrites[3].dstBinding = 3;
                            descriptorWrites[3].dstArrayElement = 0;
                            descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                            descriptorWrites[3].descriptorCount = 1;
                            descriptorWrites[3].pImageInfo = &descriptorImageInfo;

                            VkDescriptorBufferInfo modelDataStorageBufferInfoLastFrame{};
                            modelDataStorageBufferInfoLastFrame.buffer = modelDataShaderStorageBuffers[i];
                            modelDataStorageBufferInfoLastFrame.offset = 0;
                            modelDataStorageBufferInfoLastFrame.range = maxModelDataSize;

                            descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                            descriptorWrites[4].dstSet = descriptorSets[descriptorId][i];
                            descriptorWrites[4].dstBinding = 4;
                            descriptorWrites[4].dstArrayElement = 0;
                            descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                            descriptorWrites[4].descriptorCount = 1;
                            descriptorWrites[4].pBufferInfo = &modelDataStorageBufferInfoLastFrame;

                            VkDescriptorBufferInfo materialDataStorageBufferInfoLastFrame{};
                            materialDataStorageBufferInfoLastFrame.buffer = materialDataShaderStorageBuffers[i];
                            materialDataStorageBufferInfoLastFrame.offset = 0;
                            materialDataStorageBufferInfoLastFrame.range = maxMaterialDataSize;

                            descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                            descriptorWrites[5].dstSet = descriptorSets[descriptorId][i];
                            descriptorWrites[5].dstBinding = 5;
                            descriptorWrites[5].dstArrayElement = 0;
                            descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                            descriptorWrites[5].descriptorCount = 1;
                            descriptorWrites[5].pBufferInfo = &materialDataStorageBufferInfoLastFrame;



                            vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
                    }
                } else if (shader == &specularTextureGenerator) {
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    unsigned int descriptorId = specularTexture.getId() * shader->getNbShaders() + shader->getId();
                    for (size_t i = 0; i < specularTexture.getMaxFramesInFlight(); i++) {
                        std::vector<VkDescriptorImageInfo>	descriptorImageInfos;
                        descriptorImageInfos.resize(allTextures.size());
                        for (unsigned int j = 0; j < allTextures.size(); j++) {
                            descriptorImageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                            descriptorImageInfos[j].imageView = allTextures[j]->getImageView();
                            descriptorImageInfos[j].sampler = allTextures[j]->getSampler();
                        }
                        std::array<VkWriteDescriptorSet, 4> descriptorWrites{};
                        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[0].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[0].dstBinding = 0;
                        descriptorWrites[0].dstArrayElement = 0;
                        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[0].descriptorCount = allTextures.size();
                        descriptorWrites[0].pImageInfo = descriptorImageInfos.data();

                        VkDescriptorImageInfo headPtrDescriptorImageInfo;
                        headPtrDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        headPtrDescriptorImageInfo.imageView = specularTexture.getTexture().getImageView();
                        headPtrDescriptorImageInfo.sampler = specularTexture.getTexture().getSampler();

                        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[1].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[1].dstBinding = 0;
                        descriptorWrites[1].dstArrayElement = 0;
                        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[1].descriptorCount = 1;
                        descriptorWrites[1].pImageInfo = &headPtrDescriptorImageInfo;

                        VkDescriptorBufferInfo modelDataStorageBufferInfoLastFrame{};
                        modelDataStorageBufferInfoLastFrame.buffer = modelDataShaderStorageBuffers[i];
                        modelDataStorageBufferInfoLastFrame.offset = 0;
                        modelDataStorageBufferInfoLastFrame.range = maxModelDataSize;

                        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[2].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[2].dstBinding = 4;
                        descriptorWrites[2].dstArrayElement = 0;
                        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                        descriptorWrites[2].descriptorCount = 1;
                        descriptorWrites[2].pBufferInfo = &modelDataStorageBufferInfoLastFrame;

                        VkDescriptorBufferInfo materialDataStorageBufferInfoLastFrame{};
                        materialDataStorageBufferInfoLastFrame.buffer = materialDataShaderStorageBuffers[i];
                        materialDataStorageBufferInfoLastFrame.offset = 0;
                        materialDataStorageBufferInfoLastFrame.range = maxMaterialDataSize;

                        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[3].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[3].dstBinding = 5;
                        descriptorWrites[3].dstArrayElement = 0;
                        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                        descriptorWrites[3].descriptorCount = 1;
                        descriptorWrites[3].pBufferInfo = &materialDataStorageBufferInfoLastFrame;

                        vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
                    }
                } else if (shader == &bumpTextureGenerator) {
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    unsigned int descriptorId = bumpTexture.getId() * shader->getNbShaders() + shader->getId();
                    for (size_t i = 0; i < bumpTexture.getMaxFramesInFlight(); i++) {
                        std::vector<VkDescriptorImageInfo>	descriptorImageInfos;
                        descriptorImageInfos.resize(allTextures.size());
                        for (unsigned int j = 0; j < allTextures.size(); j++) {
                            descriptorImageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                            descriptorImageInfos[j].imageView = allTextures[j]->getImageView();
                            descriptorImageInfos[j].sampler = allTextures[j]->getSampler();
                        }
                        std::array<VkWriteDescriptorSet, 5> descriptorWrites{};
                        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[0].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[0].dstBinding = 0;
                        descriptorWrites[0].dstArrayElement = 0;
                        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[0].descriptorCount = allTextures.size();
                        descriptorWrites[0].pImageInfo = descriptorImageInfos.data();

                        VkDescriptorImageInfo headPtrDescriptorImageInfo;
                        headPtrDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        headPtrDescriptorImageInfo.imageView = depthBuffer.getTexture().getImageView();
                        headPtrDescriptorImageInfo.sampler = depthBuffer.getTexture().getSampler();

                        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[1].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[1].dstBinding = 1;
                        descriptorWrites[1].dstArrayElement = 0;
                        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[1].descriptorCount = 1;
                        descriptorWrites[1].pImageInfo = &headPtrDescriptorImageInfo;

                        VkDescriptorImageInfo headPtrDescriptorImageInfo2;
                        headPtrDescriptorImageInfo2.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        headPtrDescriptorImageInfo2.imageView = depthBuffer.getTexture().getImageView();
                        headPtrDescriptorImageInfo2.sampler = depthBuffer.getTexture().getSampler();

                        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[2].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[2].dstBinding = 2;
                        descriptorWrites[2].dstArrayElement = 0;
                        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[2].descriptorCount = 1;
                        descriptorWrites[2].pImageInfo = &headPtrDescriptorImageInfo2;

                        VkDescriptorBufferInfo modelDataStorageBufferInfoLastFrame{};
                        modelDataStorageBufferInfoLastFrame.buffer = modelDataShaderStorageBuffers[i];
                        modelDataStorageBufferInfoLastFrame.offset = 0;
                        modelDataStorageBufferInfoLastFrame.range = maxModelDataSize;

                        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[3].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[3].dstBinding = 4;
                        descriptorWrites[3].dstArrayElement = 0;
                        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                        descriptorWrites[3].descriptorCount = 1;
                        descriptorWrites[3].pBufferInfo = &modelDataStorageBufferInfoLastFrame;

                        VkDescriptorBufferInfo materialDataStorageBufferInfoLastFrame{};
                        materialDataStorageBufferInfoLastFrame.buffer = materialDataShaderStorageBuffers[i];
                        materialDataStorageBufferInfoLastFrame.offset = 0;
                        materialDataStorageBufferInfoLastFrame.range = maxMaterialDataSize;

                        descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[4].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[4].dstBinding = 5;
                        descriptorWrites[4].dstArrayElement = 0;
                        descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                        descriptorWrites[4].descriptorCount = 1;
                        descriptorWrites[4].pBufferInfo = &materialDataStorageBufferInfoLastFrame;

                        vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
                    }
                } else {
                    unsigned int descriptorId = lightMap.getId() * shader->getNbShaders() + shader->getId();
                    for (size_t i = 0; i < lightMap.getMaxFramesInFlight(); i++) {
                        std::array<VkWriteDescriptorSet, 7> descriptorWrites{};
                        VkDescriptorImageInfo headPtrDescriptorImageInfo;
                        headPtrDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        headPtrDescriptorImageInfo.imageView = specularTexture.getTexture().getImageView();
                        headPtrDescriptorImageInfo.sampler = depthBuffer.getTexture().getSampler();

                        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[0].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[0].dstBinding = 0;
                        descriptorWrites[0].dstArrayElement = 0;
                        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[0].descriptorCount = 1;
                        descriptorWrites[0].pImageInfo = &headPtrDescriptorImageInfo;

                        VkDescriptorImageInfo headPtrDescriptorImageInfo2;
                        headPtrDescriptorImageInfo2.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        headPtrDescriptorImageInfo2.imageView = specularTexture.getTexture().getImageView();
                        headPtrDescriptorImageInfo2.sampler =  specularTexture.getTexture().getSampler();

                        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[1].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[1].dstBinding = 1;
                        descriptorWrites[1].dstArrayElement = 0;
                        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[1].descriptorCount = 1;
                        descriptorWrites[1].pImageInfo = &headPtrDescriptorImageInfo2;

                        VkDescriptorImageInfo headPtrDescriptorImageInfo3;
                        headPtrDescriptorImageInfo3.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        headPtrDescriptorImageInfo3.imageView = bumpTexture.getTexture().getImageView();
                        headPtrDescriptorImageInfo3.sampler = bumpTexture.getTexture().getSampler();

                        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[2].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[2].dstBinding = 2;
                        descriptorWrites[2].dstArrayElement = 0;
                        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[2].descriptorCount = 1;
                        descriptorWrites[2].pImageInfo = &headPtrDescriptorImageInfo3;

                        VkDescriptorImageInfo headPtrDescriptorImageInfo4;
                        headPtrDescriptorImageInfo4.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        headPtrDescriptorImageInfo4.imageView = alphaBuffer.getTexture().getImageView();
                        headPtrDescriptorImageInfo4.sampler =  alphaBuffer.getTexture().getSampler();

                        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[3].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[3].dstBinding = 3;
                        descriptorWrites[3].dstArrayElement = 0;
                        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[3].descriptorCount = 1;
                        descriptorWrites[3].pImageInfo = &headPtrDescriptorImageInfo4;

                         VkDescriptorBufferInfo modelDataStorageBufferInfoLastFrame{};
                        modelDataStorageBufferInfoLastFrame.buffer = modelDataShaderStorageBuffers[i];
                        modelDataStorageBufferInfoLastFrame.offset = 0;
                        modelDataStorageBufferInfoLastFrame.range = maxModelDataSize;

                        descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[4].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[4].dstBinding = 4;
                        descriptorWrites[4].dstArrayElement = 0;
                        descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                        descriptorWrites[4].descriptorCount = 1;
                        descriptorWrites[4].pBufferInfo = &modelDataStorageBufferInfoLastFrame;

                        VkDescriptorBufferInfo materialDataStorageBufferInfoLastFrame{};
                        materialDataStorageBufferInfoLastFrame.buffer = materialDataShaderStorageBuffers[i];
                        materialDataStorageBufferInfoLastFrame.offset = 0;
                        materialDataStorageBufferInfoLastFrame.range = maxMaterialDataSize;

                        descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[5].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[5].dstBinding = 5;
                        descriptorWrites[5].dstArrayElement = 0;
                        descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                        descriptorWrites[5].descriptorCount = 1;
                        descriptorWrites[5].pBufferInfo = &materialDataStorageBufferInfoLastFrame;

                        VkDescriptorImageInfo headPtrDescriptorImageInfo5;
                        headPtrDescriptorImageInfo5.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        headPtrDescriptorImageInfo5.imageView = lightMap.getTexture().getImageView();
                        headPtrDescriptorImageInfo5.sampler =  lightMap.getTexture().getSampler();

                        descriptorWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[6].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[6].dstBinding = 6;
                        descriptorWrites[6].dstArrayElement = 0;
                        descriptorWrites[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[6].descriptorCount = 1;
                        descriptorWrites[6].pImageInfo = &headPtrDescriptorImageInfo5;
                        vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
                    }
                }
            }
            void LightRenderComponent::allocateDescriptorSets(RenderStates states) {
                Shader* shader = const_cast<Shader*>(states.shader);
                if (shader == &depthBufferGenerator) {
                    if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorSets.size())
                        descriptorSets.resize(shader->getNbShaders()*depthBuffer.getNbRenderTargets());
                    unsigned int descriptorId = depthBuffer.getId() * shader->getNbShaders() + shader->getId();
                    for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                        descriptorSets[i].resize(depthBuffer.getMaxFramesInFlight());
                    }
                    std::vector<VkDescriptorSetLayout> layouts(depthBuffer.getMaxFramesInFlight(), descriptorSetLayout[descriptorId]);
                    VkDescriptorSetAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    allocInfo.descriptorPool = descriptorPool[descriptorId];
                    allocInfo.descriptorSetCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());
                    allocInfo.pSetLayouts = layouts.data();
                    if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, descriptorSets[descriptorId].data()) != VK_SUCCESS) {
                        throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
                    }
                } else if (shader == &buildAlphaBufferGenerator) {
                    if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorSets.size())
                        descriptorSets.resize(shader->getNbShaders()*alphaBuffer.getNbRenderTargets());
                    unsigned int descriptorId = alphaBuffer.getId() * shader->getNbShaders() + shader->getId();
                    for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                        descriptorSets[i].resize(alphaBuffer.getMaxFramesInFlight());
                    }
                    std::vector<VkDescriptorSetLayout> layouts(alphaBuffer.getMaxFramesInFlight(), descriptorSetLayout[descriptorId]);
                    VkDescriptorSetAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    allocInfo.descriptorPool = descriptorPool[descriptorId];
                    allocInfo.descriptorSetCount = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight());
                    allocInfo.pSetLayouts = layouts.data();
                    if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, descriptorSets[descriptorId].data()) != VK_SUCCESS) {
                        throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
                    }

            } else if (shader == &specularTextureGenerator) {
                if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorSets.size())
                    descriptorSets.resize(shader->getNbShaders()*specularTexture.getNbRenderTargets());
                unsigned int descriptorId = specularTexture.getId() * shader->getNbShaders() + shader->getId();
                for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                    descriptorSets[i].resize(specularTexture.getMaxFramesInFlight());
                }
                std::vector<VkDescriptorSetLayout> layouts(specularTexture.getMaxFramesInFlight(), descriptorSetLayout[descriptorId]);
                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = descriptorPool[descriptorId];
                allocInfo.descriptorSetCount = static_cast<uint32_t>(specularTexture.getMaxFramesInFlight());
                allocInfo.pSetLayouts = layouts.data();
                if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, descriptorSets[descriptorId].data()) != VK_SUCCESS) {
                    throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
                }
            } else if (shader == &bumpTextureGenerator) {
                if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorSets.size())
                    descriptorSets.resize(shader->getNbShaders()*bumpTexture.getNbRenderTargets());
                unsigned int descriptorId = bumpTexture.getId() * shader->getNbShaders() + shader->getId();
                for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                    descriptorSets[i].resize(bumpTexture.getMaxFramesInFlight());
                }
                std::vector<VkDescriptorSetLayout> layouts(bumpTexture.getMaxFramesInFlight(), descriptorSetLayout[descriptorId]);
                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = descriptorPool[descriptorId];
                allocInfo.descriptorSetCount = static_cast<uint32_t>(bumpTexture.getMaxFramesInFlight());
                allocInfo.pSetLayouts = layouts.data();
                if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, descriptorSets[descriptorId].data()) != VK_SUCCESS) {
                    throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
                }
            } else {
                if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorSets.size())
                    descriptorSets.resize(shader->getNbShaders()*lightMap.getNbRenderTargets());
                unsigned int descriptorId = lightMap.getId() * shader->getNbShaders() + shader->getId();
                for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                    descriptorSets[i].resize(lightMap.getMaxFramesInFlight());
                }
                std::vector<VkDescriptorSetLayout> layouts(lightMap.getMaxFramesInFlight(), descriptorSetLayout[descriptorId]);
                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = descriptorPool[descriptorId];
                allocInfo.descriptorSetCount = static_cast<uint32_t>(lightMap.getMaxFramesInFlight());
                allocInfo.pSetLayouts = layouts.data();
                if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, descriptorSets[descriptorId].data()) != VK_SUCCESS) {
                    throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
                }
            }
        }
        void LightRenderComponent::drawDepthLightInstances() {
            for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                vbBindlessTex[i].clear();
                modelDatas[i].clear();
                materialDatas[i].clear();
            }
            std::array<std::vector<DrawArraysIndirectCommand>, Batcher::nbPrimitiveTypes> drawArraysIndirectCommands;
            std::array<unsigned int, Batcher::nbPrimitiveTypes> firstIndex, baseInstance;
            for (unsigned int i = 0; i < firstIndex.size(); i++) {
                firstIndex[i] = 0;
            }
            for (unsigned int i = 0; i < baseInstance.size(); i++) {
                baseInstance[i] = 0;
            }
            for (unsigned int i = 0; i < m_light_instances.size(); i++) {
                if (m_light_instances[i].getAllVertices().getVertexCount() > 0) {
                    ////std::cout<<"instance : "<<i<<std::endl;
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    unsigned int p = m_light_instances[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = 0;
                    material.layer = m_light_instances[i].getMaterial().getLayer();
                    material.lightCenter = m_light_instances[i].getMaterial().getLightCenter();
                    Color c = m_light_instances[i].getMaterial().getLightColor();
                    material.lightColor = math::Vec4f(1.f / 255.f * c.r, 1.f / 255.f * c.g, 1.f / 255.f * c.b, 1.f / 255.f * c.a);
                    materialDatas[p].push_back(material);
                    ModelData model;
                    TransformMatrix tm;
                    model.worldMat = tm.getMatrix()/*.transpose()*/;
                    modelDatas[p].push_back(model);
                    unsigned int vertexCount = 0;
                    for (unsigned int j = 0; j < m_light_instances[i].getAllVertices().getVertexCount(); j++) {
                        vertexCount++;
                        vbBindlessTex[p].append(m_light_instances[i].getAllVertices()[j]);
                    }
                    drawArraysIndirectCommand.count = vertexCount;
                    drawArraysIndirectCommand.firstIndex = firstIndex[p];
                    drawArraysIndirectCommand.baseInstance = baseInstance[p];
                    drawArraysIndirectCommand.instanceCount = 1;
                    drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                    firstIndex[p] += vertexCount;
                    baseInstance[p] += 1;
                }
            }
            RenderStates currentStates;
            currentStates.shader = &depthBufferGenerator;
            currentStates.texture = nullptr;
            for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                if (vbBindlessTex[p].getVertexCount() > 0) {
                    vbBindlessTex[p].update();
                    VkDeviceSize bufferSize = sizeof(ModelData) * modelDatas[p].size();
                    if (bufferSize > maxModelDataSize) {
                        if (modelDataStagingBuffer != nullptr) {
                            vkDestroyBuffer(vkDevice.getDevice(), modelDataStagingBuffer, nullptr);
                            vkFreeMemory(vkDevice.getDevice(), modelDataStagingBufferMemory, nullptr);
                        }
                        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, modelDataStagingBuffer, modelDataStagingBufferMemory);
                        for (unsigned int i = 0; i < modelDataShaderStorageBuffers.size(); i++) {
                            vkDestroyBuffer(vkDevice.getDevice(), modelDataShaderStorageBuffers[i], nullptr);
                            vkFreeMemory(vkDevice.getDevice(), modelDataShaderStorageBuffersMemory[i], nullptr);
                        }
                        modelDataShaderStorageBuffers.resize(depthBuffer.getMaxFramesInFlight());
                        modelDataShaderStorageBuffersMemory.resize(depthBuffer.getMaxFramesInFlight());
                        for (unsigned int i = 0; i < depthBuffer.getMaxFramesInFlight(); i++) {
                            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, modelDataShaderStorageBuffers[i], modelDataShaderStorageBuffersMemory[i]);
                        }
                        maxModelDataSize = bufferSize;
                        needToUpdateDS  = true;
                    }


                    void* data;
                    vkMapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, modelDatas[p].data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory);
                    for (unsigned int i = 0; i < depthBuffer.getMaxFramesInFlight(); i++) {
                        copyBuffer(modelDataStagingBuffer, modelDataShaderStorageBuffers[i], bufferSize);
                    }

                    bufferSize = sizeof(MaterialData) * materialDatas[p].size();
                    if (bufferSize > maxMaterialDataSize) {
                        if (materialDataStagingBuffer != nullptr) {
                            vkDestroyBuffer(vkDevice.getDevice(), materialDataStagingBuffer, nullptr);
                            vkFreeMemory(vkDevice.getDevice(), materialDataStagingBufferMemory, nullptr);
                        }
                        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, materialDataStagingBuffer, materialDataStagingBufferMemory);
                        for (unsigned int i = 0; i < materialDataShaderStorageBuffers.size(); i++) {
                            vkDestroyBuffer(vkDevice.getDevice(),materialDataShaderStorageBuffers[i], nullptr);
                            vkFreeMemory(vkDevice.getDevice(), materialDataShaderStorageBuffersMemory[i], nullptr);
                        }
                        materialDataShaderStorageBuffers.resize(depthBuffer.getMaxFramesInFlight());
                        materialDataShaderStorageBuffersMemory.resize(depthBuffer.getMaxFramesInFlight());
                        for (unsigned int i = 0; i < depthBuffer.getMaxFramesInFlight(); i++) {
                            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, materialDataShaderStorageBuffers[i], materialDataShaderStorageBuffersMemory[i]);
                        }
                        maxMaterialDataSize = bufferSize;
                        needToUpdateDS = true;
                    }

                    vkMapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, materialDatas[p].data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory);
                    for (unsigned int i = 0; i < depthBuffer.getMaxFramesInFlight(); i++) {
                        copyBuffer(materialDataStagingBuffer, materialDataShaderStorageBuffers[i], bufferSize);
                    }
                    bufferSize = sizeof(DrawArraysIndirectCommand) * drawArraysIndirectCommands[p].size();

                    if (bufferSize > maxVboIndirectSize) {
                        if (vboIndirectStagingBuffer != nullptr) {
                            vkDestroyBuffer(vkDevice.getDevice(), vboIndirectStagingBuffer, nullptr);
                            vkFreeMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemory, nullptr);
                        }
                        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vboIndirectStagingBuffer, vboIndirectStagingBufferMemory);
                        if (vboIndirect != VK_NULL_HANDLE) {
                            vkDestroyBuffer(vkDevice.getDevice(),vboIndirect, nullptr);
                            vkFreeMemory(vkDevice.getDevice(), vboIndirectMemory, nullptr);
                        }
                        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vboIndirect, vboIndirectMemory);
                        maxVboIndirectSize = bufferSize;
                    }

                    vkMapMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, drawArraysIndirectCommands[p].data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemory);
                    copyBuffer(vboIndirectStagingBuffer, vboIndirect, bufferSize);
                    //createDescriptorSets(p, currentStates);
                    createCommandBuffersIndirect(p, drawArraysIndirectCommands[p].size(), sizeof(DrawArraysIndirectCommand), NODEPTHNOSTENCIL, currentStates);
                }
            }
        }
        void LightRenderComponent::drawLightInstances() {
            for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                vbBindlessTex[i].clear();
                modelDatas[i].clear();
                materialDatas[i].clear();
            }
            std::array<std::vector<DrawArraysIndirectCommand>, Batcher::nbPrimitiveTypes> drawArraysIndirectCommands;
            std::array<unsigned int, Batcher::nbPrimitiveTypes> firstIndex, baseInstance;
            for (unsigned int i = 0; i < firstIndex.size(); i++) {
                firstIndex[i] = 0;
            }
            for (unsigned int i = 0; i < baseInstance.size(); i++) {
                baseInstance[i] = 0;
            }
            for (unsigned int i = 0; i < m_light_instances.size(); i++) {
                if (m_light_instances[i].getAllVertices().getVertexCount() > 0) {
                    ////std::cout<<"instance : "<<i<<std::endl;
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    unsigned int p = m_light_instances[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = 0;
                    material.layer = m_light_instances[i].getMaterial().getLayer();
                    material.lightCenter = m_light_instances[i].getMaterial().getLightCenter();
                    Color c = m_light_instances[i].getMaterial().getLightColor();
                    material.lightColor = math::Vec4f(1.f / 255.f * c.r, 1.f / 255.f * c.g, 1.f / 255.f * c.b, 1.f / 255.f * c.a);
                    materialDatas[p].push_back(material);
                    ModelData model;
                    TransformMatrix tm;
                    model.worldMat = tm.getMatrix().transpose();
                    modelDatas[p].push_back(model);
                    unsigned int vertexCount = 0;
                    for (unsigned int j = 0; j < m_light_instances[i].getAllVertices().getVertexCount(); j++) {
                        vertexCount++;
                        vbBindlessTex[p].append(m_light_instances[i].getAllVertices()[j]);
                    }
                    drawArraysIndirectCommand.count = vertexCount;
                    drawArraysIndirectCommand.firstIndex = firstIndex[p];
                    drawArraysIndirectCommand.baseInstance = baseInstance[p];
                    drawArraysIndirectCommand.instanceCount = 1;
                    drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                    firstIndex[p] += vertexCount;
                    baseInstance[p] += 1;
                }
            }
            for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                vbBindlessTex[i].clear();
                drawArraysIndirectCommands[i].clear();
            }
            for (unsigned int i = 0; i < firstIndex.size(); i++) {
                firstIndex[i] = 0;
            }
            for (unsigned int i = 0; i < baseInstance.size(); i++) {
                baseInstance[i] = 0;
            }
            for (unsigned int i = 0; i < m_light_instances.size(); i++) {
                if (m_light_instances[i].getAllVertices().getVertexCount() > 0) {
                    ////std::cout<<"instance : "<<i<<std::endl;
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    unsigned int p = m_light_instances[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = 0;
                    material.layer = m_light_instances[i].getMaterial().getLayer();
                    material.lightCenter = m_light_instances[i].getMaterial().getLightCenter();
                    Color c = m_light_instances[i].getMaterial().getLightColor();
                    material.lightColor = math::Vec4f(1.f / 255.f * c.r, 1.f / 255.f * c.g, 1.f / 255.f * c.b, 1.f / 255.f * c.a);
                    materialDatas[p].push_back(material);
                    ModelData model;
                    TransformMatrix tm;
                    model.worldMat = tm.getMatrix().transpose();
                    modelDatas[p].push_back(model);
                    unsigned int vertexCount = 0;
                    for (unsigned int j = 0; j < m_light_instances[i].getAllVertices().getVertexCount(); j++) {
                        vertexCount++;
                        vbBindlessTex[p].append(m_light_instances[i].getAllVertices()[j]);
                    }
                    drawArraysIndirectCommand.count = vertexCount;
                    drawArraysIndirectCommand.firstIndex = firstIndex[p];
                    drawArraysIndirectCommand.baseInstance = baseInstance[p];
                    drawArraysIndirectCommand.instanceCount = 1;
                    drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                    firstIndex[p] += vertexCount;
                    baseInstance[p] += 1;
                }
            }
            RenderStates currentStates;
            currentStates.blendMode = BlendAdd;
            currentStates.shader = &lightMapGenerator;
            currentStates.texture = nullptr;
             for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                if (vbBindlessTex[p].getVertexCount() > 0) {
                    vbBindlessTex[p].update();
                    VkDeviceSize bufferSize = sizeof(ModelData) * modelDatas[p].size();
                    if (bufferSize > maxModelDataSize) {
                        if (modelDataStagingBuffer != nullptr) {
                            vkDestroyBuffer(vkDevice.getDevice(), modelDataStagingBuffer, nullptr);
                            vkFreeMemory(vkDevice.getDevice(), modelDataStagingBufferMemory, nullptr);
                        }
                        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, modelDataStagingBuffer, modelDataStagingBufferMemory);
                        for (unsigned int i = 0; i < modelDataShaderStorageBuffers.size(); i++) {
                            vkDestroyBuffer(vkDevice.getDevice(), modelDataShaderStorageBuffers[i], nullptr);
                            vkFreeMemory(vkDevice.getDevice(), modelDataShaderStorageBuffersMemory[i], nullptr);
                        }
                        modelDataShaderStorageBuffers.resize(depthBuffer.getMaxFramesInFlight());
                        modelDataShaderStorageBuffersMemory.resize(depthBuffer.getMaxFramesInFlight());
                        for (unsigned int i = 0; i < depthBuffer.getMaxFramesInFlight(); i++) {
                            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, modelDataShaderStorageBuffers[i], modelDataShaderStorageBuffersMemory[i]);
                        }
                        maxModelDataSize = bufferSize;
                        needToUpdateDS  = true;
                    }


                    void* data;
                    vkMapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, modelDatas[p].data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory);
                    for (unsigned int i = 0; i < depthBuffer.getMaxFramesInFlight(); i++) {
                        copyBuffer(modelDataStagingBuffer, modelDataShaderStorageBuffers[i], bufferSize);
                    }

                    bufferSize = sizeof(MaterialData) * materialDatas[p].size();
                    if (bufferSize > maxMaterialDataSize) {
                        if (materialDataStagingBuffer != nullptr) {
                            vkDestroyBuffer(vkDevice.getDevice(), materialDataStagingBuffer, nullptr);
                            vkFreeMemory(vkDevice.getDevice(), materialDataStagingBufferMemory, nullptr);
                        }
                        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, materialDataStagingBuffer, materialDataStagingBufferMemory);
                        for (unsigned int i = 0; i < materialDataShaderStorageBuffers.size(); i++) {
                            vkDestroyBuffer(vkDevice.getDevice(),materialDataShaderStorageBuffers[i], nullptr);
                            vkFreeMemory(vkDevice.getDevice(), materialDataShaderStorageBuffersMemory[i], nullptr);
                        }
                        materialDataShaderStorageBuffers.resize(depthBuffer.getMaxFramesInFlight());
                        materialDataShaderStorageBuffersMemory.resize(depthBuffer.getMaxFramesInFlight());
                        for (unsigned int i = 0; i < depthBuffer.getMaxFramesInFlight(); i++) {
                            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, materialDataShaderStorageBuffers[i], materialDataShaderStorageBuffersMemory[i]);
                        }
                        maxMaterialDataSize = bufferSize;
                        needToUpdateDS = true;
                    }

                    vkMapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, materialDatas[p].data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory);
                    for (unsigned int i = 0; i < depthBuffer.getMaxFramesInFlight(); i++) {
                        copyBuffer(materialDataStagingBuffer, materialDataShaderStorageBuffers[i], bufferSize);
                    }
                    bufferSize = sizeof(DrawArraysIndirectCommand) * drawArraysIndirectCommands[p].size();

                    if (bufferSize > maxVboIndirectSize) {
                        if (vboIndirectStagingBuffer != nullptr) {
                            vkDestroyBuffer(vkDevice.getDevice(), vboIndirectStagingBuffer, nullptr);
                            vkFreeMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemory, nullptr);
                        }
                        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vboIndirectStagingBuffer, vboIndirectStagingBufferMemory);
                        if (vboIndirect != VK_NULL_HANDLE) {
                            vkDestroyBuffer(vkDevice.getDevice(),vboIndirect, nullptr);
                            vkFreeMemory(vkDevice.getDevice(), vboIndirectMemory, nullptr);
                        }
                        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vboIndirect, vboIndirectMemory);
                        maxVboIndirectSize = bufferSize;
                    }

                    vkMapMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, drawArraysIndirectCommands[p].data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemory);
                    copyBuffer(vboIndirectStagingBuffer, vboIndirect, bufferSize);
                    //createDescriptorSets(p, currentStates);
                    createCommandBuffersIndirect(p, drawArraysIndirectCommands[p].size(), sizeof(DrawArraysIndirectCommand), NODEPTHNOSTENCIL, currentStates);
                }
            }
        }
        void LightRenderComponent::createCommandBuffersIndirect(unsigned int p, unsigned int nbIndirectCommands, unsigned int stride, DepthStencilID depthStencilID, RenderStates currentStates) {

            if (needToUpdateDS) {
                Shader* shader = const_cast<Shader*>(currentStates.shader);
                currentStates.shader = &depthBufferGenerator;
                createDescriptorSets(currentStates);
                currentStates.shader = &buildAlphaBufferGenerator;
                createDescriptorSets(currentStates);
                currentStates.shader = &specularTextureGenerator;
                createDescriptorSets(currentStates);
                currentStates.shader = &bumpTextureGenerator;
                createDescriptorSets(currentStates);
                currentStates.shader = &lightMapGenerator;
                createDescriptorSets(currentStates);
                currentStates.shader = shader;
                needToUpdateDS = false;
            }
            currentStates.blendMode.updateIds();
            Shader* shader = const_cast<Shader*>(currentStates.shader);
            if (shader == &depthBufferGenerator) {

                ////std::cout<<"draw on db"<<std::endl;
                std::vector<VkCommandBuffer> commandBuffers = depthBuffer.getCommandBuffers();
                unsigned int currentFrame = depthBuffer.getCurrentFrame();
                layerPC.nbLayers = GameObject::getNbLayers();
                vkCmdPushConstants(commandBuffers[currentFrame], depthBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][depthBuffer.getId()][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(IndirectRenderingPC), &indirectRenderingPC);
                vkCmdPushConstants(commandBuffers[currentFrame], depthBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][depthBuffer.getId()][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_FRAGMENT_BIT, 128, sizeof(LayerPC), &layerPC);
                depthBuffer.beginRenderPass();
                depthBuffer.drawIndirect(commandBuffers[currentFrame], currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], vboIndirect, depthStencilID,currentStates);
                depthBuffer.endRenderPass();
                depthBuffer.display();
            } else if (shader == &buildAlphaBufferGenerator) {
                const_cast<Texture&>(depthBuffer.getTexture()).toShaderReadOnlyOptimal(alphaBuffer.getCommandBuffers()[alphaBuffer.getCurrentFrame()]);


                std::vector<VkCommandBuffer> commandBuffers = alphaBuffer.getCommandBuffers();
                unsigned int currentFrame = alphaBuffer.getCurrentFrame();
                layerPC.nbLayers = GameObject::getNbLayers();
                /*vkCmdResetEvent(commandBuffers[currentFrame], events[currentFrame],  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                vkCmdSetEvent(commandBuffers[currentFrame], events[currentFrame],  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);*/
                //vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);


                vkCmdPushConstants(commandBuffers[currentFrame], alphaBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][alphaBuffer.getId()][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(IndirectRenderingPC), &indirectRenderingPC);
                vkCmdPushConstants(commandBuffers[currentFrame], alphaBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][alphaBuffer.getId()][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_FRAGMENT_BIT, 128, sizeof(LayerPC), &layerPC);


                VkMemoryBarrier memoryBarrier={};
                memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                //vkCmdWaitEvents(commandBuffers[currentFrame], 1, &events[currentFrame], VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                alphaBuffer.beginRenderPass();
                alphaBuffer.drawIndirect(commandBuffers[currentFrame], currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], vboIndirect, depthStencilID,currentStates);
                alphaBuffer.endRenderPass();
                const_cast<Texture&>(depthBuffer.getTexture()).toColorAttachmentOptimal(alphaBuffer.getCommandBuffers()[alphaBuffer.getCurrentFrame()]);
                alphaBuffer.display();
            } else if (shader == &specularTextureGenerator) {


                std::vector<VkCommandBuffer> commandBuffers = specularTexture.getCommandBuffers();
                unsigned int currentFrame = specularTexture.getCurrentFrame();

                //vkCmdWaitEvents(commandBuffers[currentFrame], 1, &events[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                vkCmdPushConstants(commandBuffers[currentFrame], specularTexture.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][specularTexture.getId()][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(IndirectRenderingPC), &indirectRenderingPC);

                /*std::cout<<"pipeline layout : "<<stencilBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][stencilBuffer.getId()][depthStencilID]<<std::endl;
                system("PAUSE");*/
                specularTexture.beginRenderPass();
                specularTexture.drawIndirect(commandBuffers[currentFrame], currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], vboIndirect, depthStencilID,currentStates);
                specularTexture.endRenderPass();
                specularTexture.display();

            } else if (shader == &bumpTextureGenerator) {

                std::vector<VkCommandBuffer> commandBuffers = bumpTexture.getCommandBuffers();
                unsigned int currentFrame = bumpTexture.getCurrentFrame();

                //vkCmdWaitEvents(commandBuffers[currentFrame], 1, &events[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                vkCmdPushConstants(commandBuffers[currentFrame], bumpTexture.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][bumpTexture.getId()][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(IndirectRenderingPC), &indirectRenderingPC);

                /*std::cout<<"pipeline layout : "<<stencilBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][stencilBuffer.getId()][depthStencilID]<<std::endl;
                system("PAUSE");*/
                bumpTexture.beginRenderPass();
                bumpTexture.drawIndirect(commandBuffers[currentFrame], currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], vboIndirect, depthStencilID,currentStates);
                bumpTexture.endRenderPass();
                bumpTexture.display();
            } else {
                const_cast<Texture&>(specularTexture.getTexture()).toShaderReadOnlyOptimal(lightMap.getCommandBuffers()[lightMap.getCurrentFrame()]);
                const_cast<Texture&>(bumpTexture.getTexture()).toShaderReadOnlyOptimal(lightMap.getCommandBuffers()[lightMap.getCurrentFrame()]);
                const_cast<Texture&>(alphaBuffer.getTexture()).toShaderReadOnlyOptimal(lightMap.getCommandBuffers()[lightMap.getCurrentFrame()]);
                const_cast<Texture&>(depthBuffer.getTexture()).toShaderReadOnlyOptimal(lightMap.getCommandBuffers()[lightMap.getCurrentFrame()]);


                std::vector<VkCommandBuffer> commandBuffers = lightMap.getCommandBuffers();
                unsigned int currentFrame = lightMap.getCurrentFrame();


                vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);
                VkMemoryBarrier memoryBarrier;
                memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                memoryBarrier.pNext = VK_NULL_HANDLE;
                memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                //vkCmdWaitEvents(commandBuffers[currentFrame], 1, &events[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

                vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                vkCmdPushConstants(commandBuffers[currentFrame], lightMap.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][lightMap.getId()][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(LightIndirectRenderingPC), &lightIndirectRenderingPC);
                vkCmdPushConstants(commandBuffers[currentFrame], lightMap.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][lightMap.getId()][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_FRAGMENT_BIT, 128, sizeof(ResolutionPC), &resolutionPC);
                lightMap.beginRenderPass();
                lightMap.drawIndirect(commandBuffers[currentFrame], currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], vboIndirect, depthStencilID,currentStates);
                lightMap.endRenderPass();

                const_cast<Texture&>(alphaBuffer.getTexture()).toColorAttachmentOptimal(lightMap.getCommandBuffers()[lightMap.getCurrentFrame()]);
                const_cast<Texture&>(bumpTexture.getTexture()).toColorAttachmentOptimal(lightMap.getCommandBuffers()[lightMap.getCurrentFrame()]);
                const_cast<Texture&>(depthBuffer.getTexture()).toColorAttachmentOptimal(lightMap.getCommandBuffers()[lightMap.getCurrentFrame()]);
                const_cast<Texture&>(specularTexture.getTexture()).toColorAttachmentOptimal(lightMap.getCommandBuffers()[lightMap.getCurrentFrame()]);
                lightMap.display(true, renderFinishedSemaphore[window.getCurrentFrame()]);
            }
        }
        void LightRenderComponent::drawNextFrame() {
            update = false;
            physic::BoundingBox viewArea = view.getViewVolume();
            math::Vec3f position (viewArea.getPosition().x(),viewArea.getPosition().y(), view.getPosition().z());
            math::Vec3f size (viewArea.getWidth(), viewArea.getHeight(), 0);



           {
               std::lock_guard<std::recursive_mutex> lock(rec_mutex);
               if (datasReady) {
                   datasReady = false;
                   m_instances = batcher.getInstances();
                   m_normals = normalBatcher.getInstances();
                   m_light_instances = lightBatcher.getInstances();
               }

           }
            //glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
            if (!view.isOrtho())
                view.setPerspective(80, view.getViewport().getSize().x() / view.getViewport().getSize().y(), 0.1, view.getViewport().getSize().z());
            math::Matrix4f viewMatrix = view.getViewMatrix().getMatrix()/*.transpose()*/;
            math::Matrix4f projMatrix = view.getProjMatrix().getMatrix()/*.transpose()*/;
            indirectRenderingPC.projMatrix = projMatrix;
            indirectRenderingPC.viewMatrix = viewMatrix;
            layerPC.nbLayers = GameObject::getNbLayers();

            drawDepthLightInstances();
            //drawNormals();
            drawInstances();
            lightIndirectRenderingPC.projMatrix = projMatrix;
            lightIndirectRenderingPC.viewMatrix = viewMatrix;
            lightIndirectRenderingPC.viewportMatrix = lightMap.getViewportMatrix(&lightMap.getView()).getMatrix();



            drawLightInstances();

        }
        void LightRenderComponent::drawInstances() {
            for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                vbBindlessTex[i].clear();
                modelDatas[i].clear();
                materialDatas[i].clear();
            }
            std::array<std::vector<DrawArraysIndirectCommand>, Batcher::nbPrimitiveTypes> drawArraysIndirectCommands;
            std::array<unsigned int, Batcher::nbPrimitiveTypes> firstIndex, baseInstance;
            for (unsigned int i = 0; i < firstIndex.size(); i++) {
                firstIndex[i] = 0;
            }
            for (unsigned int i = 0; i < baseInstance.size(); i++) {
                baseInstance[i] = 0;
            }
            for (unsigned int i = 0; i < m_normals.size(); i++) {
               if (m_normals[i].getAllVertices().getVertexCount() > 0) {
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    unsigned int p = m_normals[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = (m_normals[i].getMaterial().getTexture() != nullptr) ? m_normals[i].getMaterial().getTexture()->getId() : 0;
                    material.layer = m_normals[i].getMaterial().getLayer();
                    materialDatas[p].push_back(material);
                    ModelData model;
                    TransformMatrix tm;
                    model.worldMat = tm.getMatrix().transpose();
                    modelDatas[p].push_back(model);
                    unsigned int vertexCount = 0;
                    for (unsigned int j = 0; j < m_normals[i].getAllVertices().getVertexCount(); j++) {
                        vertexCount++;
                        vbBindlessTex[p].append(m_normals[i].getAllVertices()[j]);
                    }
                    drawArraysIndirectCommand.count = vertexCount;
                    drawArraysIndirectCommand.firstIndex = firstIndex[p];
                    drawArraysIndirectCommand.baseInstance = baseInstance[p];
                    drawArraysIndirectCommand.instanceCount = 1;
                    drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                    firstIndex[p] += vertexCount;
                    baseInstance[p] += 1;
                }
            }
            for (unsigned int i = 0; i < m_instances.size(); i++) {

                if (m_instances[i].getAllVertices().getVertexCount() > 0) {
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    unsigned int p = m_instances[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = (m_instances[i].getMaterial().getTexture() != nullptr) ? m_instances[i].getMaterial().getTexture()->getId() : 0;
                    material.layer = m_instances[i].getMaterial().getLayer();
                    materialDatas[p].push_back(material);
                    std::vector<TransformMatrix*> tm = m_instances[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData model;
                        model.worldMat = tm[j]->getMatrix().transpose();
                        modelDatas[p].push_back(model);
                    }
                    unsigned int vertexCount = 0;
                    if (m_instances[i].getVertexArrays().size() > 0) {
                        Entity* entity = m_instances[i].getVertexArrays()[0]->getEntity();
                        for (unsigned int j = 0; j < m_instances[i].getVertexArrays().size(); j++) {
                            if (entity == m_instances[i].getVertexArrays()[j]->getEntity()) {
                                for (unsigned int k = 0; k < m_instances[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                    vertexCount++;
                                    vbBindlessTex[p].append((*m_instances[i].getVertexArrays()[j])[k]);

                                }
                            }
                        }
                    }
                    drawArraysIndirectCommand.count = vertexCount;
                    drawArraysIndirectCommand.firstIndex = firstIndex[p];
                    drawArraysIndirectCommand.baseInstance = baseInstance[p];
                    drawArraysIndirectCommand.instanceCount = tm.size();
                    drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                    firstIndex[p] += vertexCount;
                    baseInstance[p] += tm.size();
                }
            }
            RenderStates currentStates;
            currentStates.blendMode = BlendNone;
            currentStates.shader = &depthBufferGenerator;
            currentStates.texture = nullptr;
            for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                if (vbBindlessTex[p].getVertexCount() > 0) {
                    vbBindlessTex[p].update();
                    VkDeviceSize bufferSize = sizeof(ModelData) * modelDatas[p].size();
                    if (bufferSize > maxModelDataSize) {
                        if (modelDataStagingBuffer != nullptr) {
                            vkDestroyBuffer(vkDevice.getDevice(), modelDataStagingBuffer, nullptr);
                            vkFreeMemory(vkDevice.getDevice(), modelDataStagingBufferMemory, nullptr);
                        }
                        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, modelDataStagingBuffer, modelDataStagingBufferMemory);
                        for (unsigned int i = 0; i < modelDataShaderStorageBuffers.size(); i++) {
                            vkDestroyBuffer(vkDevice.getDevice(), modelDataShaderStorageBuffers[i], nullptr);
                            vkFreeMemory(vkDevice.getDevice(), modelDataShaderStorageBuffersMemory[i], nullptr);
                        }
                        modelDataShaderStorageBuffers.resize(depthBuffer.getMaxFramesInFlight());
                        modelDataShaderStorageBuffersMemory.resize(depthBuffer.getMaxFramesInFlight());
                        for (unsigned int i = 0; i < depthBuffer.getMaxFramesInFlight(); i++) {
                            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, modelDataShaderStorageBuffers[i], modelDataShaderStorageBuffersMemory[i]);
                        }
                        maxModelDataSize = bufferSize;
                        needToUpdateDS  = true;
                    }


                    void* data;
                    vkMapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, modelDatas[p].data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory);
                    for (unsigned int i = 0; i < depthBuffer.getMaxFramesInFlight(); i++) {
                        copyBuffer(modelDataStagingBuffer, modelDataShaderStorageBuffers[i], bufferSize);
                    }

                    bufferSize = sizeof(MaterialData) * materialDatas[p].size();
                    if (bufferSize > maxMaterialDataSize) {
                        if (materialDataStagingBuffer != nullptr) {
                            vkDestroyBuffer(vkDevice.getDevice(), materialDataStagingBuffer, nullptr);
                            vkFreeMemory(vkDevice.getDevice(), materialDataStagingBufferMemory, nullptr);
                        }
                        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, materialDataStagingBuffer, materialDataStagingBufferMemory);
                        for (unsigned int i = 0; i < materialDataShaderStorageBuffers.size(); i++) {
                            vkDestroyBuffer(vkDevice.getDevice(),materialDataShaderStorageBuffers[i], nullptr);
                            vkFreeMemory(vkDevice.getDevice(), materialDataShaderStorageBuffersMemory[i], nullptr);
                        }
                        materialDataShaderStorageBuffers.resize(depthBuffer.getMaxFramesInFlight());
                        materialDataShaderStorageBuffersMemory.resize(depthBuffer.getMaxFramesInFlight());
                        for (unsigned int i = 0; i < depthBuffer.getMaxFramesInFlight(); i++) {
                            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, materialDataShaderStorageBuffers[i], materialDataShaderStorageBuffersMemory[i]);
                        }
                        maxMaterialDataSize = bufferSize;
                        needToUpdateDS = true;
                    }

                    vkMapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, materialDatas[p].data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory);
                    for (unsigned int i = 0; i < depthBuffer.getMaxFramesInFlight(); i++) {
                        copyBuffer(materialDataStagingBuffer, materialDataShaderStorageBuffers[i], bufferSize);
                    }
                    bufferSize = sizeof(DrawArraysIndirectCommand) * drawArraysIndirectCommands[p].size();

                    if (bufferSize > maxVboIndirectSize) {
                        if (vboIndirectStagingBuffer != nullptr) {
                            vkDestroyBuffer(vkDevice.getDevice(), vboIndirectStagingBuffer, nullptr);
                            vkFreeMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemory, nullptr);
                        }
                        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vboIndirectStagingBuffer, vboIndirectStagingBufferMemory);
                        if (vboIndirect != VK_NULL_HANDLE) {
                            vkDestroyBuffer(vkDevice.getDevice(),vboIndirect, nullptr);
                            vkFreeMemory(vkDevice.getDevice(), vboIndirectMemory, nullptr);
                        }
                        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vboIndirect, vboIndirectMemory);
                        maxVboIndirectSize = bufferSize;
                    }

                    vkMapMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, drawArraysIndirectCommands[p].data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemory);
                    copyBuffer(vboIndirectStagingBuffer, vboIndirect, bufferSize);
                    //createDescriptorSets(p, currentStates);
                    createCommandBuffersIndirect(p, drawArraysIndirectCommands[p].size(), sizeof(DrawArraysIndirectCommand), NODEPTHNOSTENCIL, currentStates);
                }
            }
            currentStates.shader = &buildAlphaBufferGenerator;
            for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                if (vbBindlessTex[p].getVertexCount() > 0) {
                    vbBindlessTex[p].update();
                    VkDeviceSize bufferSize = sizeof(ModelData) * modelDatas[p].size();
                    if (bufferSize > maxModelDataSize) {
                        if (modelDataStagingBuffer != nullptr) {
                            vkDestroyBuffer(vkDevice.getDevice(), modelDataStagingBuffer, nullptr);
                            vkFreeMemory(vkDevice.getDevice(), modelDataStagingBufferMemory, nullptr);
                        }
                        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, modelDataStagingBuffer, modelDataStagingBufferMemory);
                        for (unsigned int i = 0; i < modelDataShaderStorageBuffers.size(); i++) {
                            vkDestroyBuffer(vkDevice.getDevice(), modelDataShaderStorageBuffers[i], nullptr);
                            vkFreeMemory(vkDevice.getDevice(), modelDataShaderStorageBuffersMemory[i], nullptr);
                        }
                        modelDataShaderStorageBuffers.resize(depthBuffer.getMaxFramesInFlight());
                        modelDataShaderStorageBuffersMemory.resize(depthBuffer.getMaxFramesInFlight());
                        for (unsigned int i = 0; i < depthBuffer.getMaxFramesInFlight(); i++) {
                            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, modelDataShaderStorageBuffers[i], modelDataShaderStorageBuffersMemory[i]);
                        }
                        maxModelDataSize = bufferSize;
                        needToUpdateDS  = true;
                    }


                    void* data;
                    vkMapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, modelDatas[p].data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory);
                    for (unsigned int i = 0; i < depthBuffer.getMaxFramesInFlight(); i++) {
                        copyBuffer(modelDataStagingBuffer, modelDataShaderStorageBuffers[i], bufferSize);
                    }

                    bufferSize = sizeof(MaterialData) * materialDatas[p].size();
                    if (bufferSize > maxMaterialDataSize) {
                        if (materialDataStagingBuffer != nullptr) {
                            vkDestroyBuffer(vkDevice.getDevice(), materialDataStagingBuffer, nullptr);
                            vkFreeMemory(vkDevice.getDevice(), materialDataStagingBufferMemory, nullptr);
                        }
                        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, materialDataStagingBuffer, materialDataStagingBufferMemory);
                        for (unsigned int i = 0; i < materialDataShaderStorageBuffers.size(); i++) {
                            vkDestroyBuffer(vkDevice.getDevice(),materialDataShaderStorageBuffers[i], nullptr);
                            vkFreeMemory(vkDevice.getDevice(), materialDataShaderStorageBuffersMemory[i], nullptr);
                        }
                        materialDataShaderStorageBuffers.resize(depthBuffer.getMaxFramesInFlight());
                        materialDataShaderStorageBuffersMemory.resize(depthBuffer.getMaxFramesInFlight());
                        for (unsigned int i = 0; i < depthBuffer.getMaxFramesInFlight(); i++) {
                            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, materialDataShaderStorageBuffers[i], materialDataShaderStorageBuffersMemory[i]);
                        }
                        maxMaterialDataSize = bufferSize;
                        needToUpdateDS = true;
                    }

                    vkMapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, materialDatas[p].data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory);
                    for (unsigned int i = 0; i < depthBuffer.getMaxFramesInFlight(); i++) {
                        copyBuffer(materialDataStagingBuffer, materialDataShaderStorageBuffers[i], bufferSize);
                    }
                    bufferSize = sizeof(DrawArraysIndirectCommand) * drawArraysIndirectCommands[p].size();

                    if (bufferSize > maxVboIndirectSize) {
                        if (vboIndirectStagingBuffer != nullptr) {
                            vkDestroyBuffer(vkDevice.getDevice(), vboIndirectStagingBuffer, nullptr);
                            vkFreeMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemory, nullptr);
                        }
                        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vboIndirectStagingBuffer, vboIndirectStagingBufferMemory);
                        if (vboIndirect != VK_NULL_HANDLE) {
                            vkDestroyBuffer(vkDevice.getDevice(),vboIndirect, nullptr);
                            vkFreeMemory(vkDevice.getDevice(), vboIndirectMemory, nullptr);
                        }
                        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vboIndirect, vboIndirectMemory);
                        maxVboIndirectSize = bufferSize;
                    }

                    vkMapMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, drawArraysIndirectCommands[p].data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemory);
                    copyBuffer(vboIndirectStagingBuffer, vboIndirect, bufferSize);
                    //createDescriptorSets(p, currentStates);
                    createCommandBuffersIndirect(p, drawArraysIndirectCommands[p].size(), sizeof(DrawArraysIndirectCommand), NODEPTHNOSTENCIL, currentStates);
                }
            }

            for (unsigned int i = 0; i < firstIndex.size(); i++) {
                firstIndex[i] = 0;
            }
            for (unsigned int i = 0; i < baseInstance.size(); i++) {
                baseInstance[i] = 0;
            }
            for (unsigned int i = 0; i < modelDatas.size(); i++) {
                modelDatas[i].clear();
            }
            for (unsigned int i = 0; i < materialDatas.size(); i++) {
                materialDatas[i].clear();
            }
            for (unsigned int i = 0; i < drawArraysIndirectCommands.size(); i++) {
                drawArraysIndirectCommands[i].clear();
            }
            for (unsigned int i = 0; i < m_normals.size(); i++) {
               if (m_normals[i].getAllVertices().getVertexCount() > 0) {
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    unsigned int p = m_normals[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = (m_normals[i].getMaterial().getBumpTexture() != nullptr) ? m_normals[i].getMaterial().getBumpTexture()->getId() : 0;
                    material.layer = m_normals[i].getMaterial().getLayer();
                    materialDatas[p].push_back(material);
                    ModelData model;
                    TransformMatrix tm;
                    model.worldMat = tm.getMatrix().transpose();
                    modelDatas[p].push_back(model);
                    unsigned int vertexCount = 0;
                    for (unsigned int j = 0; j < m_normals[i].getAllVertices().getVertexCount(); j++) {
                        vertexCount++;
                        vbBindlessTex[p].append(m_normals[i].getAllVertices()[j]);
                    }
                    drawArraysIndirectCommand.count = vertexCount;
                    drawArraysIndirectCommand.firstIndex = firstIndex[p];
                    drawArraysIndirectCommand.baseInstance = baseInstance[p];
                    drawArraysIndirectCommand.instanceCount = 1;
                    drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                    firstIndex[p] += vertexCount;
                    baseInstance[p] += 1;
                }
            }
            for (unsigned int i = 0; i < m_instances.size(); i++) {
                if (m_instances[i].getAllVertices().getVertexCount() > 0) {
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    unsigned int p = m_instances[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = (m_instances[i].getMaterial().getBumpTexture() != nullptr) ? m_instances[i].getMaterial().getBumpTexture()->getId() : 0;
                    material.layer = m_instances[i].getMaterial().getLayer();
                    materialDatas[p].push_back(material);
                    std::vector<TransformMatrix*> tm = m_instances[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData model;
                        model.worldMat = tm[j]->getMatrix().transpose();
                        modelDatas[p].push_back(model);
                    }
                    unsigned int vertexCount = 0;
                    if (m_instances[i].getVertexArrays().size() > 0) {
                        Entity* entity = m_instances[i].getVertexArrays()[0]->getEntity();
                        for (unsigned int j = 0; j < m_instances[i].getVertexArrays().size(); j++) {
                            if (entity == m_instances[i].getVertexArrays()[j]->getEntity()) {
                                for (unsigned int k = 0; k < m_instances[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                    vertexCount++;
                                    vbBindlessTex[p].append((*m_instances[i].getVertexArrays()[j])[k]);
                                }
                            }
                        }
                    }
                    drawArraysIndirectCommand.count = vertexCount;
                    drawArraysIndirectCommand.firstIndex = firstIndex[p];
                    drawArraysIndirectCommand.baseInstance = baseInstance[p];
                    drawArraysIndirectCommand.instanceCount = tm.size();
                    drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                    firstIndex[p] += vertexCount;
                    baseInstance[p] += tm.size();
                }
            }
            currentStates.blendMode = BlendNone;
            currentStates.shader = &bumpTextureGenerator;
            currentStates.texture = nullptr;
            for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                if (vbBindlessTex[p].getVertexCount() > 0) {
                    vbBindlessTex[p].update();
                    VkDeviceSize bufferSize = sizeof(ModelData) * modelDatas[p].size();
                    if (bufferSize > maxModelDataSize) {
                        if (modelDataStagingBuffer != nullptr) {
                            vkDestroyBuffer(vkDevice.getDevice(), modelDataStagingBuffer, nullptr);
                            vkFreeMemory(vkDevice.getDevice(), modelDataStagingBufferMemory, nullptr);
                        }
                        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, modelDataStagingBuffer, modelDataStagingBufferMemory);
                        for (unsigned int i = 0; i < modelDataShaderStorageBuffers.size(); i++) {
                            vkDestroyBuffer(vkDevice.getDevice(), modelDataShaderStorageBuffers[i], nullptr);
                            vkFreeMemory(vkDevice.getDevice(), modelDataShaderStorageBuffersMemory[i], nullptr);
                        }
                        modelDataShaderStorageBuffers.resize(depthBuffer.getMaxFramesInFlight());
                        modelDataShaderStorageBuffersMemory.resize(depthBuffer.getMaxFramesInFlight());
                        for (unsigned int i = 0; i < depthBuffer.getMaxFramesInFlight(); i++) {
                            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, modelDataShaderStorageBuffers[i], modelDataShaderStorageBuffersMemory[i]);
                        }
                        maxModelDataSize = bufferSize;
                        needToUpdateDS  = true;
                    }


                    void* data;
                    vkMapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, modelDatas[p].data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory);
                    for (unsigned int i = 0; i < depthBuffer.getMaxFramesInFlight(); i++) {
                        copyBuffer(modelDataStagingBuffer, modelDataShaderStorageBuffers[i], bufferSize);
                    }

                    bufferSize = sizeof(MaterialData) * materialDatas[p].size();
                    if (bufferSize > maxMaterialDataSize) {
                        if (materialDataStagingBuffer != nullptr) {
                            vkDestroyBuffer(vkDevice.getDevice(), materialDataStagingBuffer, nullptr);
                            vkFreeMemory(vkDevice.getDevice(), materialDataStagingBufferMemory, nullptr);
                        }
                        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, materialDataStagingBuffer, materialDataStagingBufferMemory);
                        for (unsigned int i = 0; i < materialDataShaderStorageBuffers.size(); i++) {
                            vkDestroyBuffer(vkDevice.getDevice(),materialDataShaderStorageBuffers[i], nullptr);
                            vkFreeMemory(vkDevice.getDevice(), materialDataShaderStorageBuffersMemory[i], nullptr);
                        }
                        materialDataShaderStorageBuffers.resize(depthBuffer.getMaxFramesInFlight());
                        materialDataShaderStorageBuffersMemory.resize(depthBuffer.getMaxFramesInFlight());
                        for (unsigned int i = 0; i < depthBuffer.getMaxFramesInFlight(); i++) {
                            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, materialDataShaderStorageBuffers[i], materialDataShaderStorageBuffersMemory[i]);
                        }
                        maxMaterialDataSize = bufferSize;
                        needToUpdateDS = true;
                    }

                    vkMapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, materialDatas[p].data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory);
                    for (unsigned int i = 0; i < depthBuffer.getMaxFramesInFlight(); i++) {
                        copyBuffer(materialDataStagingBuffer, materialDataShaderStorageBuffers[i], bufferSize);
                    }
                    bufferSize = sizeof(DrawArraysIndirectCommand) * drawArraysIndirectCommands[p].size();

                    if (bufferSize > maxVboIndirectSize) {
                        if (vboIndirectStagingBuffer != nullptr) {
                            vkDestroyBuffer(vkDevice.getDevice(), vboIndirectStagingBuffer, nullptr);
                            vkFreeMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemory, nullptr);
                        }
                        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vboIndirectStagingBuffer, vboIndirectStagingBufferMemory);
                        if (vboIndirect != VK_NULL_HANDLE) {
                            vkDestroyBuffer(vkDevice.getDevice(),vboIndirect, nullptr);
                            vkFreeMemory(vkDevice.getDevice(), vboIndirectMemory, nullptr);
                        }
                        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vboIndirect, vboIndirectMemory);
                        maxVboIndirectSize = bufferSize;
                    }

                    vkMapMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, drawArraysIndirectCommands[p].data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemory);
                    copyBuffer(vboIndirectStagingBuffer, vboIndirect, bufferSize);
                    //createDescriptorSets(p, currentStates);
                    createCommandBuffersIndirect(p, drawArraysIndirectCommands[p].size(), sizeof(DrawArraysIndirectCommand), NODEPTHNOSTENCIL, currentStates);
                }
            }
            for (unsigned int i = 0; i < firstIndex.size(); i++) {
                firstIndex[i] = 0;
            }
            for (unsigned int i = 0; i < baseInstance.size(); i++) {
                baseInstance[i] = 0;
            }
            for (unsigned int i = 0; i < modelDatas.size(); i++) {
                modelDatas[i].clear();
            }
            for (unsigned int i = 0; i < materialDatas.size(); i++) {
                materialDatas[i].clear();
            }
            for (unsigned int i = 0; i < drawArraysIndirectCommands.size(); i++) {
                drawArraysIndirectCommands[i].clear();
            }
            for (unsigned int i = 0; i < m_normals.size(); i++) {
               if (m_normals[i].getAllVertices().getVertexCount() > 0) {
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    unsigned int p = m_normals[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = (m_normals[i].getMaterial().getTexture() != nullptr) ? m_normals[i].getMaterial().getTexture()->getId() : 0;
                    material.layer = m_normals[i].getMaterial().getLayer();
                    material.specularIntensity = m_normals[i].getMaterial().getSpecularIntensity();
                    material.specularPower = m_normals[i].getMaterial().getSpecularPower();
                    materialDatas[p].push_back(material);
                    TransformMatrix tm;
                    ModelData model;
                    model.worldMat = tm.getMatrix().transpose();
                    modelDatas[p].push_back(model);
                    unsigned int vertexCount = 0;
                    for (unsigned int j = 0; j < m_normals[i].getAllVertices().getVertexCount(); j++) {
                        vertexCount++;
                        vbBindlessTex[p].append(m_normals[i].getAllVertices()[j]);
                    }
                    drawArraysIndirectCommand.count = vertexCount;
                    drawArraysIndirectCommand.firstIndex = firstIndex[p];
                    drawArraysIndirectCommand.baseInstance = baseInstance[p];
                    drawArraysIndirectCommand.instanceCount = 1;
                    drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                    firstIndex[p] += vertexCount;
                    baseInstance[p] += 1;
                }
            }
            for (unsigned int i = 0; i < m_instances.size(); i++) {
                DrawArraysIndirectCommand drawArraysIndirectCommand;
                if (m_instances[i].getAllVertices().getVertexCount() > 0) {
                    unsigned int p = m_instances[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = (m_instances[i].getMaterial().getBumpTexture() != nullptr) ? m_instances[i].getMaterial().getBumpTexture()->getId() : 0;
                    material.layer = m_instances[i].getMaterial().getLayer();
                    material.specularIntensity = m_instances[i].getMaterial().getSpecularIntensity();
                    material.specularPower = m_instances[i].getMaterial().getSpecularPower();
                    materialDatas[p].push_back(material);
                    std::vector<TransformMatrix*> tm = m_instances[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData model;
                        model.worldMat = tm[j]->getMatrix().transpose();
                        modelDatas[p].push_back(model);
                    }
                    unsigned int vertexCount = 0;
                    if (m_instances[i].getVertexArrays().size() > 0) {
                        Entity* entity = m_instances[i].getVertexArrays()[0]->getEntity();
                        for (unsigned int j = 0; j < m_instances[i].getVertexArrays().size(); j++) {
                            if (entity == m_instances[i].getVertexArrays()[j]->getEntity()) {
                                for (unsigned int k = 0; k < m_instances[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                    vertexCount++;
                                    vbBindlessTex[p].append((*m_instances[i].getVertexArrays()[j])[k]);
                                }
                            }
                        }
                    }
                    drawArraysIndirectCommand.count = vertexCount;
                    drawArraysIndirectCommand.firstIndex = firstIndex[p];
                    drawArraysIndirectCommand.baseInstance = baseInstance[p];
                    drawArraysIndirectCommand.instanceCount = tm.size();
                    drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                    firstIndex[p] += vertexCount;
                    baseInstance[p] += tm.size();
                }
            }
            currentStates.blendMode = BlendNone;
            currentStates.shader = &specularTextureGenerator;
            currentStates.texture = nullptr;
            for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                if (vbBindlessTex[p].getVertexCount() > 0) {
                    vbBindlessTex[p].update();
                    VkDeviceSize bufferSize = sizeof(ModelData) * modelDatas[p].size();
                    if (bufferSize > maxModelDataSize) {
                        if (modelDataStagingBuffer != nullptr) {
                            vkDestroyBuffer(vkDevice.getDevice(), modelDataStagingBuffer, nullptr);
                            vkFreeMemory(vkDevice.getDevice(), modelDataStagingBufferMemory, nullptr);
                        }
                        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, modelDataStagingBuffer, modelDataStagingBufferMemory);
                        for (unsigned int i = 0; i < modelDataShaderStorageBuffers.size(); i++) {
                            vkDestroyBuffer(vkDevice.getDevice(), modelDataShaderStorageBuffers[i], nullptr);
                            vkFreeMemory(vkDevice.getDevice(), modelDataShaderStorageBuffersMemory[i], nullptr);
                        }
                        modelDataShaderStorageBuffers.resize(depthBuffer.getMaxFramesInFlight());
                        modelDataShaderStorageBuffersMemory.resize(depthBuffer.getMaxFramesInFlight());
                        for (unsigned int i = 0; i < depthBuffer.getMaxFramesInFlight(); i++) {
                            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, modelDataShaderStorageBuffers[i], modelDataShaderStorageBuffersMemory[i]);
                        }
                        maxModelDataSize = bufferSize;
                        needToUpdateDS  = true;
                    }


                    void* data;
                    vkMapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, modelDatas[p].data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory);
                    for (unsigned int i = 0; i < depthBuffer.getMaxFramesInFlight(); i++) {
                        copyBuffer(modelDataStagingBuffer, modelDataShaderStorageBuffers[i], bufferSize);
                    }

                    bufferSize = sizeof(MaterialData) * materialDatas[p].size();
                    if (bufferSize > maxMaterialDataSize) {
                        if (materialDataStagingBuffer != nullptr) {
                            vkDestroyBuffer(vkDevice.getDevice(), materialDataStagingBuffer, nullptr);
                            vkFreeMemory(vkDevice.getDevice(), materialDataStagingBufferMemory, nullptr);
                        }
                        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, materialDataStagingBuffer, materialDataStagingBufferMemory);
                        for (unsigned int i = 0; i < materialDataShaderStorageBuffers.size(); i++) {
                            vkDestroyBuffer(vkDevice.getDevice(),materialDataShaderStorageBuffers[i], nullptr);
                            vkFreeMemory(vkDevice.getDevice(), materialDataShaderStorageBuffersMemory[i], nullptr);
                        }
                        materialDataShaderStorageBuffers.resize(depthBuffer.getMaxFramesInFlight());
                        materialDataShaderStorageBuffersMemory.resize(depthBuffer.getMaxFramesInFlight());
                        for (unsigned int i = 0; i < depthBuffer.getMaxFramesInFlight(); i++) {
                            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, materialDataShaderStorageBuffers[i], materialDataShaderStorageBuffersMemory[i]);
                        }
                        maxMaterialDataSize = bufferSize;
                        needToUpdateDS = true;
                    }

                    vkMapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, materialDatas[p].data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory);
                    for (unsigned int i = 0; i < depthBuffer.getMaxFramesInFlight(); i++) {
                        copyBuffer(materialDataStagingBuffer, materialDataShaderStorageBuffers[i], bufferSize);
                    }
                    bufferSize = sizeof(DrawArraysIndirectCommand) * drawArraysIndirectCommands[p].size();

                    if (bufferSize > maxVboIndirectSize) {
                        if (vboIndirectStagingBuffer != nullptr) {
                            vkDestroyBuffer(vkDevice.getDevice(), vboIndirectStagingBuffer, nullptr);
                            vkFreeMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemory, nullptr);
                        }
                        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vboIndirectStagingBuffer, vboIndirectStagingBufferMemory);
                        if (vboIndirect != VK_NULL_HANDLE) {
                            vkDestroyBuffer(vkDevice.getDevice(),vboIndirect, nullptr);
                            vkFreeMemory(vkDevice.getDevice(), vboIndirectMemory, nullptr);
                        }
                        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vboIndirect, vboIndirectMemory);
                        maxVboIndirectSize = bufferSize;
                    }

                    vkMapMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, drawArraysIndirectCommands[p].data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemory);
                    copyBuffer(vboIndirectStagingBuffer, vboIndirect, bufferSize);
                    //createDescriptorSets(p, currentStates);
                    createCommandBuffersIndirect(p, drawArraysIndirectCommands[p].size(), sizeof(DrawArraysIndirectCommand), NODEPTHNOSTENCIL, currentStates);
                }
            }
        }
        void LightRenderComponent::draw(RenderTarget& target, RenderStates states) {
            if (&target == &window)
                window.setSemaphore(renderFinishedSemaphore);
            const_cast<Texture&>(lightMap.getTexture()).toShaderReadOnlyOptimal(window.getCommandBuffers()[window.getCurrentFrame()]);
            lightMapTile.setCenter(target.getView().getPosition());
            if (&target == &window)
                window.beginRenderPass();
            states.blendMode = BlendMultiply;
            target.draw(lightMapTile, states);
            if (&target == &window)
                window.endRenderPass();
        }
        bool LightRenderComponent::loadEntitiesOnComponent(std::vector<Entity*> vEntities)
        {
            {
                std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                datasReady = false;
                batcher.clear();
                normalBatcher.clear();
                lightBatcher.clear();
            }

            for (unsigned int i = 0; i < vEntities.size(); i++) {

                if (vEntities[i] != nullptr && vEntities[i]->isLeaf()) {
                    std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                    if (vEntities[i]->isLight()) {
                        for (unsigned int j = 0; j <  vEntities[i]->getNbFaces(); j++) {
                            lightBatcher.addFace(vEntities[i]->getFace(j));
                        }
                    } else {
                        for (unsigned int j = 0; j <  vEntities[i]->getNbFaces(); j++) {
                            if (vEntities[i]->getDrawMode() == Entity::INSTANCED)
                                batcher.addFace(vEntities[i]->getFace(j));
                            else
                                normalBatcher.addFace(vEntities[i]->getFace(j));
                        }
                    }
                }
            }

            visibleEntities = vEntities;
            update = true;
            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
            datasReady = true;
            return true;
        }
        void LightRenderComponent::setView(View view){
            this->view = view;
            depthBuffer.setView(view);
            specularTexture.setView(view);
            bumpTexture.setView(view);
            lightMap.setView(view);
            lightDepthBuffer.setView(view);
        }
        void LightRenderComponent::setExpression(std::string expression) {
            update = true;
            this->expression = expression;
        }
        std::string LightRenderComponent::getExpression() {
            return expression;
        }
        bool LightRenderComponent::needToUpdate() {
            return update;
        }
        void LightRenderComponent::onVisibilityChanged (bool visible) {

        }
        std::vector<Entity*> LightRenderComponent::getEntities() {
            return visibleEntities;
        }
        View& LightRenderComponent::getView() {
            return view;
        }
        int LightRenderComponent::getLayer() {
            return getPosition().z();
        }
        RenderTexture* LightRenderComponent::getFrameBuffer() {
            return &lightMap;
        }
        void LightRenderComponent::pushEvent(window::IEvent event, RenderWindow& rw) {

        }
        LightRenderComponent::~LightRenderComponent() {
        }
        #else
        LightRenderComponent::LightRenderComponent (RenderWindow& window, int layer, std::string expression,window::ContextSettings settings) :
                    HeavyComponent(window, math::Vec3f(window.getView().getPosition().x(), window.getView().getPosition().y(), layer),
                                  math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0),
                                  math::Vec3f(window.getView().getSize().x() + window.getView().getSize().x() * 0.5f, window.getView().getPosition().y() + window.getView().getSize().y() * 0.5f, layer)),
                    view(window.getView()),
                    expression(expression),
                    quad(math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), window.getSize().y() * 0.5f)) {
                    update = false;
                    datasReady = false;
                    quad.move(math::Vec3f(-window.getView().getSize().x() * 0.5f, -window.getView().getSize().y() * 0.5f, 0));
                    math::Vec4f resolution ((int) window.getSize().x(), (int) window.getSize().y(), window.getView().getSize().z(), 1);
                    //settings.depthBits = 24;
                    depthBuffer.create(resolution.x(), resolution.y(),settings);
                    glCheck(glGenTextures(1, &depthTex));
                    glCheck(glBindTexture(GL_TEXTURE_2D, depthTex));
                    glCheck(glTexStorage2D(GL_TEXTURE_2D,1,GL_RGBA32F,window.getView().getSize().x(), window.getView().getSize().y()));
                    glCheck(glBindImageTexture(0, depthTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F));
                    glCheck(glBindTexture(GL_TEXTURE_2D, 0));
                    std::vector<GLfloat> depthClearBuf(window.getView().getSize().x()*window.getView().getSize().y()*4, 0);
                    glCheck(glGenBuffers(1, &clearBuf));
                    glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf));
                    glCheck(glBufferData(GL_PIXEL_UNPACK_BUFFER, depthClearBuf.size() * sizeof(GLfloat),
                    &depthClearBuf[0], GL_STATIC_COPY));
                    glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));

                    lightDepthBuffer.create(resolution.x(), resolution.y(),settings);
                    glCheck(glGenTextures(1, &lightDepthTex));
                    glCheck(glBindTexture(GL_TEXTURE_2D, lightDepthTex));
                    glCheck(glTexStorage2D(GL_TEXTURE_2D,1,GL_RGBA32F,window.getView().getSize().x(), window.getView().getSize().y()));
                    glCheck(glBindImageTexture(0, lightDepthTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F));
                    glCheck(glBindTexture(GL_TEXTURE_2D, 0));
                    std::vector<GLfloat> lDepthClearBuf(window.getView().getSize().x()*window.getView().getSize().y()*4, 0);
                    glCheck(glGenBuffers(1, &clearBuf2));
                    glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf2));
                    glCheck(glBufferData(GL_PIXEL_UNPACK_BUFFER, lDepthClearBuf.size() * sizeof(GLfloat),
                    &lDepthClearBuf[0], GL_STATIC_COPY));
                    glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
                    settings.depthBits = 0;
                    alphaBuffer.create(resolution.x(), resolution.y(),settings);
                    glCheck(glGenTextures(1, &alphaTex));
                    glCheck(glBindTexture(GL_TEXTURE_2D, alphaTex));
                    glCheck(glTexStorage2D(GL_TEXTURE_2D,1,GL_RGBA32F,window.getView().getSize().x(), window.getView().getSize().y()));
                    glCheck(glBindImageTexture(0, alphaTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F));
                    glCheck(glBindTexture(GL_TEXTURE_2D, 0));
                    std::vector<GLfloat> lAlphaClearBuf(window.getView().getSize().x()*window.getView().getSize().y()*4, 0);
                    glCheck(glGenBuffers(1, &clearBuf3));
                    glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf3));
                    glCheck(glBufferData(GL_PIXEL_UNPACK_BUFFER, lAlphaClearBuf.size() * sizeof(GLfloat),
                    &lAlphaClearBuf[0], GL_STATIC_COPY));
                    glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));


                    specularTexture.create(resolution.x(), resolution.y(),settings);
                    bumpTexture.create(resolution.x(), resolution.y(),settings);
                    lightMap.create(resolution.x(), resolution.y(),settings);
                    normalMap.create(resolution.x(), resolution.y(),settings);
                    normalMap.setView(window.getView());
                    depthBuffer.setView(window.getView());
                    specularTexture.setView(window.getView());
                    bumpTexture.setView(window.getView());
                    lightMap.setView(window.getView());
                    lightMapTile = Sprite(lightMap.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0), IntRect(0, 0, window.getView().getSize().x(), window.getView().getSize().y()));
                    depthBufferTile = Sprite(depthBuffer.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0), IntRect(0, 0, window.getView().getSize().x(), window.getView().getSize().y()));
                    specularBufferTile = Sprite(specularTexture.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0), IntRect(0, 0, window.getView().getSize().x(), window.getView().getSize().y()));
                    bumpMapTile = Sprite(bumpTexture.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0), IntRect(0, 0, window.getView().getSize().x(), window.getView().getSize().y()));


                    core::FastDelegate<bool> signal (&LightRenderComponent::needToUpdate, this);
                    core::FastDelegate<void> slot (&LightRenderComponent::drawNextFrame, this);
                    core::Command cmd(signal, slot);
                    getListener().connect("UPDATE", cmd);
                    glCheck(glGenBuffers(1, &vboIndirect));
                    glGenBuffers(1, &modelDataBuffer);
                    glGenBuffers(1, &materialDataBuffer);
                    //To debug.
                    lightMap.setActive(true);
                    glCheck(glGenTextures(1, &frameBufferTex));
                    glCheck(glBindTexture(GL_TEXTURE_2D, frameBufferTex));
                    glCheck(glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, window.getView().getSize().x(), window.getView().getSize().y()));
                    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
                    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
                    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
                    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
                    glCheck(glBindImageTexture(0, frameBufferTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F));
                    glCheck(glBindTexture(GL_TEXTURE_2D, 0));
                    std::vector<GLfloat> texClearBuf(window.getView().getSize().x()*window.getView().getSize().y()*4, 0);
                    glCheck(glGenBuffers(1, &clearBuf4));
                    glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf4));
                    glCheck(glBufferData(GL_PIXEL_UNPACK_BUFFER, texClearBuf.size() * sizeof(GLfloat),
                    &texClearBuf[0], GL_STATIC_COPY));
                    glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
                    if (settings.versionMajor >= 3 && settings.versionMinor >= 3) {

                         const std::string  simpleVertexShader = R"(#version 460
                                                        layout (location = 0) in vec3 position;
                                                        layout (location = 1) in vec4 color;
                                                        layout (location = 2) in vec2 texCoords;
                                                        layout (location = 3) in vec3 normals;
                                                        uniform mat4 projectionMatrix;
                                                        uniform mat4 viewMatrix;
                                                        uniform mat4 worldMat;
                                                        void main () {
                                                            gl_Position = projectionMatrix * viewMatrix * worldMat * vec4(position, 1.f);
                                                        })";
                        const std::string simpleFragmentShader = R"(#version 460
                                                                    layout(origin_upper_left) in vec4 gl_FragCoord;
                                                                    layout(rgba32f, binding = 0) uniform image2D img_output;
                                                                    layout(location = 0) out vec4 fcolor;
                                                                    void main() {
                                                                        fcolor = imageLoad(img_output, ivec2(gl_FragCoord.x()y));
                                                                    })";
                        const std::string indirectRenderingVertexShader = R"(#version 460
                                                                             layout (location = 0) in vec3 position;
                                                                             layout (location = 1) in vec4 color;
                                                                             layout (location = 2) in vec2 texCoords;
                                                                             layout (location = 3) in vec3 normals;
                                                                             uniform mat4 projectionMatrix;
                                                                             uniform mat4 viewMatrix;
                                                                             uniform mat4 textureMatrix[)"+core::conversionUIntString(Texture::getAllTextures().size())+R"(];
                                                                             struct ModelData {
                                                                                mat4 modelMatrix;
                                                                             };
                                                                             struct MaterialData {
                                                                                uint textureIndex;
                                                                                uint layer;
                                                                                float specularIntensity;
                                                                                float specularPower;
                                                                                vec4 lightCenter;
                                                                                vec4 lightColor;
                                                                             };
                                                                             layout(binding = 0, std430) buffer modelData {
                                                                                 ModelData modelDatas[];
                                                                             };
                                                                             layout(binding = 1, std430) buffer materialData {
                                                                                 MaterialData materialDatas[];
                                                                             };
                                                                             out vec2 fTexCoords;
                                                                             out vec4 frontColor;
                                                                             out uint texIndex;
                                                                             out uint layer;
                                                                             void main() {
                                                                                 ModelData model = modelDatas[gl_BaseInstance + gl_InstanceID];
                                                                                 MaterialData material = materialDatas[gl_DrawID];
                                                                                 uint textureIndex = material.textureIndex;
                                                                                 uint l = material.layer;
                                                                                 gl_Position = projectionMatrix * viewMatrix * model.modelMatrix * vec4(position, 1.f);
                                                                                 fTexCoords = (textureIndex != 0) ? (textureMatrix[textureIndex-1] * vec4(texCoords, 1.f, 1.f)).x()y : texCoords;
                                                                                 frontColor = color;
                                                                                 texIndex = textureIndex;
                                                                                 layer = l;
                                                                             }
                                                                             )";
                        const std::string specularIndirectRenderingVertexShader = R"(#version 460
                                                                                     layout (location = 0) in vec3 position;
                                                                                     layout (location = 1) in vec4 color;
                                                                                     layout (location = 2) in vec2 texCoords;
                                                                                     layout (location = 3) in vec3 normals;
                                                                                     uniform mat4 projectionMatrix;
                                                                                     uniform mat4 viewMatrix;
                                                                                     uniform mat4 textureMatrix[)"+core::conversionUIntString(Texture::getAllTextures().size())+R"(];
                                                                                     struct ModelData {
                                                                                        mat4 modelMatrix;
                                                                                     };
                                                                                     struct MaterialData {
                                                                                        uint textureIndex;
                                                                                        uint layer;
                                                                                        float specularIntensity;
                                                                                        float specularPower;
                                                                                        vec4 lightCenter;
                                                                                        vec4 lightColor;
                                                                                     };
                                                                                     layout(binding = 0, std430) buffer modelData {
                                                                                         ModelData modelDatas[];
                                                                                     };
                                                                                     layout(binding = 1, std430) buffer materialData {
                                                                                         MaterialData materialDatas[];
                                                                                     };
                                                                                     out vec2 fTexCoords;
                                                                                     out vec4 frontColor;
                                                                                     out uint texIndex;
                                                                                     out uint layer;
                                                                                     out vec2 specular;
                                                                                     void main() {
                                                                                         ModelData model = modelDatas[gl_BaseInstance + gl_InstanceID];
                                                                                         MaterialData material = materialDatas[gl_DrawID];
                                                                                         uint textureIndex = material.textureIndex;
                                                                                         uint l = material.layer;
                                                                                         gl_Position = projectionMatrix * viewMatrix * model.modelMatrix * vec4(position, 1.f);
                                                                                         frontColor = color;
                                                                                         texIndex = textureIndex;
                                                                                         layer = l;
                                                                                         specular = vec2(material.specularIntensity, material.specularPower);
                                                                                     }
                                                                                     )";
                        const std::string perPixLightingIndirectRenderingVertexShader = R"(#version 460
                                                                                      layout (location = 0) in vec3 position;
                                                                                      layout (location = 1) in vec4 color;
                                                                                      layout (location = 2) in vec2 texCoords;
                                                                                      layout (location = 3) in vec3 normals;
                                                                                      uniform mat4 projectionMatrix;
                                                                                      uniform mat4 viewMatrix;
                                                                                      uniform mat4 viewportMatrix;
                                                                                      uniform mat4 textureMatrix;
                                                                                      uniform vec3 resolution;
                                                                                      struct ModelData {
                                                                                         mat4 modelMatrix;
                                                                                      };
                                                                                      struct MaterialData {
                                                                                         uint textureIndex;
                                                                                         uint layer;
                                                                                         float specularIntensity;
                                                                                         float specularPower;
                                                                                         vec4 lightCenter;
                                                                                         vec4 lightColor;
                                                                                      };
                                                                                      layout(binding = 0, std430) buffer modelData {
                                                                                          ModelData modelDatas[];
                                                                                      };
                                                                                      layout(binding = 1, std430) buffer materialData {
                                                                                          MaterialData materialDatas[];
                                                                                      };
                                                                                      out vec2 fTexCoords;
                                                                                      out vec4 frontColor;
                                                                                      out uint layer;
                                                                                      out vec4 lightPos;
                                                                                      out vec4 lightColor;
                                                                                      void main() {
                                                                                         ModelData model = modelDatas[gl_BaseInstance + gl_InstanceID];
                                                                                         MaterialData material = materialDatas[gl_DrawID];
                                                                                         uint l = material.layer;
                                                                                         gl_Position = projectionMatrix * viewMatrix * model.modelMatrix * vec4(position, 1.f);
                                                                                         fTexCoords = (textureMatrix * vec4(texCoords, 1.f, 1.f)).x()y;
                                                                                         frontColor = color;
                                                                                         layer = l;
                                                                                         vec4 coords = vec4(material.lightCenter.x()yz, 1);
                                                                                         coords = projectionMatrix * viewMatrix * model.modelMatrix * coords;
                                                                                         if (coords.w == 0)
                                                                                             coords.w = resolution.z() * 0.5;
                                                                                         coords = coords / coords.w;
                                                                                         coords = viewportMatrix * coords;
                                                                                         coords.w = material.lightCenter.w;
                                                                                         lightPos = coords;
                                                                                         lightColor = material.lightColor;
                                                                                      }
                                                                                      )";

                        const std::string depthGenFragShader = R"(#version 460
                                                                          #extension GL_ARB_bindless_texture : enable
                                                                          #extension GL_ARB_fragment_shader_interlock : require
                                                                          in vec4 frontColor;
                                                                          in vec2 fTexCoords;
                                                                          in flat uint texIndex;
                                                                          in flat uint layer;
                                                                          layout(std140, binding=0) uniform ALL_TEXTURES {
                                                                              sampler2D textures[200];
                                                                          };

                                                                          layout(binding = 0, rgba32f) uniform image2D depthBuffer;
                                                                          layout (location = 0) out vec4 fColor;
                                                                          void main () {
                                                                              vec4 texel = (texIndex != 0) ? frontColor * texture2D(textures[texIndex-1], fTexCoords.x()y) : frontColor;
                                                                              float z = gl_FragCoord.z();
                                                                              float l = layer;
                                                                              beginInvocationInterlockARB();
                                                                              vec4 depth = imageLoad(depthBuffer,ivec2(gl_FragCoord.x()y));
                                                                              if (/*l > depth.y() || l == depth.y() &&*/ z > depth.z()) {
                                                                                imageStore(depthBuffer,ivec2(gl_FragCoord.x()y),vec4(0,l,z,texel.a));
                                                                                memoryBarrier();
                                                                                fColor = vec4(0, l, z, texel.a);
                                                                              } else {
                                                                                fColor = depth;
                                                                              }
                                                                              endInvocationInterlockARB();
                                                                          }
                                                                        )";
                        const std::string buildAlphaBufferFragmentShader = R"(#version 460
                                                                      #extension GL_ARB_bindless_texture : enable
                                                                      #extension GL_ARB_fragment_shader_interlock : require
                                                                      layout(std140, binding=0) uniform ALL_TEXTURES {
                                                                        sampler2D textures[200];
                                                                      };
                                                                      layout(binding = 0, rgba32f) uniform image2D alphaBuffer;
                                                                      layout (location = 0) out vec4 fColor;
                                                                      uniform sampler2D lightDepthBuffer;
                                                                      uniform vec3 resolution;
                                                                      uniform mat4 lviewMatrix;
                                                                      uniform mat4 lprojectionMatrix;
                                                                      in vec4 frontColor;
                                                                      in vec2 fTexCoords;
                                                                      in flat uint texIndex;
                                                                      in flat uint layer;
                                                                      void main() {
                                                                          vec4 texel = (texIndex != 0) ? frontColor * texture2D(textures[texIndex-1], fTexCoords.x()y) : frontColor;
                                                                          float current_alpha = texel.a;
                                                                          vec2 position = (gl_FragCoord.x()y / resolution.x()y);
                                                                          vec4 depth = texture2D (lightDepthBuffer, position);
                                                                          beginInvocationInterlockARB();
                                                                          vec4 alpha = imageLoad(alphaBuffer,ivec2(gl_FragCoord.x()y));
                                                                          float l = layer;
                                                                          float z = gl_FragCoord.z();
                                                                          if (/*l > depth.y() || l == depth.y() &&*/ depth.z() >= z && current_alpha > alpha.a) {
                                                                              imageStore(alphaBuffer,ivec2(gl_FragCoord.x()y),vec4(0, l, z, current_alpha));
                                                                              memoryBarrier();
                                                                              fColor = vec4(0, l, z, current_alpha);
                                                                          } else {
                                                                              fColor = alpha;
                                                                          }
                                                                          endInvocationInterlockARB();
                                                                      }
                                                                      )";
                        const std::string specularGenFragShader = R"(#version 460
                                                                     #extension GL_ARB_bindless_texture : enable
                                                                     layout(std140, binding=0) uniform ALL_TEXTURES {
                                                                        sampler2D textures[200];
                                                                     };
                                                                     in vec4 frontColor;
                                                                     in vec2 fTexCoords;
                                                                     in flat uint texIndex;
                                                                     in flat uint layer;
                                                                     in flat vec2 specular;
                                                                     uniform float maxM;
                                                                     uniform float maxP;
                                                                     uniform sampler2D depthBuffer;
                                                                     uniform sampler2D specularBuffer;
                                                                     uniform vec3 resolution;
                                                                     layout (location = 0) out vec4 fColor;
                                                                     void main() {
                                                                        vec4 texel = (texIndex != 0) ? frontColor * texture2D(textures[texIndex-1], fTexCoords.x()y) : frontColor;
                                                                        vec4 depth = texture2D(depthBuffer, (gl_FragCoord.x()y / resolution.x()y));
                                                                        vec4 specular = texture2D(specularBuffer,(gl_FragCoord.x()y / resolution.x()y));
                                                                        vec4 colors[2];
                                                                        colors[1] = texel * frontColor;
                                                                        colors[0] = frontColor;
                                                                        bool b = (texIndex != 0);
                                                                        vec4 color = colors[int(b)];
                                                                        float z = gl_FragCoord.z();
                                                                        float intensity = (maxM != 0.f) ? specular.x() / maxM : 0.f;
                                                                        float power = (maxP != 0.f) ? specular.y() / maxP : 0.f;
                                                                        if (/*layer > depth.y() || layer == depth.y() &&*/ z > depth.z())
                                                                            fColor = vec4(intensity, power, z, color.a);
                                                                        else
                                                                            fColor = specular;
                                                                     }
                                                                  )";
                        const std::string bumpGenFragShader =    R"(#version 460
                                                                 #extension GL_ARB_bindless_texture : enable
                                                                 layout(std140, binding=0) uniform ALL_TEXTURES {
                                                                    sampler2D textures[200];
                                                                 };
                                                                 in vec4 frontColor;
                                                                 in vec2 fTexCoords;
                                                                 in flat uint texIndex;
                                                                 in flat uint layer;
                                                                 uniform sampler2D depthBuffer;
                                                                 uniform sampler2D bumpMap;
                                                                 uniform vec3 resolution;

                                                                 layout (location = 0) out vec4 fColor;
                                                                 void main() {
                                                                     vec4 color = (texIndex != 0) ? texture2D(textures[texIndex-1], fTexCoords.x()y) : vec4(0, 0, 0, 0);
                                                                     vec2 position = gl_FragCoord.x()y / resolution.x()y;
                                                                     vec4 depth = texture2D(depthBuffer, position);
                                                                     vec4 bump = texture2D(bumpMap, position);
                                                                     if (/*layer > depth.y() || layer == depth.y() &&*/ gl_FragCoord.z() > depth.z()) {
                                                                        fColor = color;
                                                                     } else {
                                                                        fColor = bump;
                                                                     }
                                                                 }
                                                                 )";
                        const std::string perPixLightingFragmentShader =  R"(#version 460
                                                                 in vec4 frontColor;
                                                                 in vec2 fTexCoords;
                                                                 in flat uint layer;
                                                                 in flat vec4 lightColor;
                                                                 in flat vec4 lightPos;
                                                                 const vec2 size = vec2(2.0,0.0);
                                                                 const ivec3 off = ivec3(-1,0,1);
                                                                 uniform sampler2D depthTexture;
                                                                 uniform sampler2D lightMap;
                                                                 uniform sampler2D specularTexture;
                                                                 uniform sampler2D bumpMap;
                                                                 uniform sampler2D alphaMap;
                                                                 uniform vec3 resolution;
                                                                 layout (location = 0) out vec4 fColor;
                                                                 layout(rgba32f, binding = 0) uniform image2D img_output;

                                                                 /*Functions to debug, draw numbers to the image,
                                                      draw a vertical ligne*/
                                                      void drawVLine (ivec2 position, int width, int nbPixels, vec4 color) {
                                                          int startY = position.y();
                                                          int startX = position.x();
                                                          while (position.y() < startY + nbPixels) {
                                                             while (position.x() < startX + width) {
                                                                imageStore(img_output, position, color);
                                                                position.x()++;
                                                             }
                                                             position.y()++;
                                                             position.x() = startX;
                                                          }
                                                      }
                                                      /*Draw an horizontal line*/
                                                      void drawHLine (ivec2 position, int height, int nbPixels, vec4 color) {
                                                          int startY = position.y();
                                                          int startX = position.x();
                                                          while (position.y() > startY - height) {
                                                             while (position.x() < startX + nbPixels) {
                                                                imageStore(img_output, position, color);
                                                                position.x()++;
                                                             }
                                                             position.y()--;
                                                             position.x() = startX;
                                                          }
                                                      }
                                                      /*Draw digits.*/
                                                      void drawDigit (ivec2 position, int nbPixels, vec4 color, uint digit) {
                                                          int digitSize = nbPixels * 10;
                                                          if (digit == 0) {
                                                              drawVLine(position, digitSize / 2, nbPixels, color);
                                                              drawHLine(position, digitSize, nbPixels, color);
                                                              drawHLine(ivec2(position.x() + digitSize / 2 - nbPixels, position.y()), digitSize, nbPixels, color);
                                                              drawVLine(ivec2(position.x(), position.y() - digitSize + nbPixels), digitSize / 2, nbPixels, color);
                                                          } else if (digit == 1) {
                                                              drawHLine(ivec2(position.x() + digitSize / 2 - nbPixels, position.y()), digitSize, nbPixels, color);
                                                          } else if (digit == 2) {
                                                              drawVLine(position, digitSize / 2, nbPixels, color);
                                                              drawHLine(ivec2(position.x(), position.y()), digitSize / 2 + nbPixels / 2, nbPixels, color);
                                                              drawVLine(ivec2(position.x(), position.y() - digitSize / 2 + nbPixels / 2), digitSize / 2, nbPixels, color);
                                                              drawHLine(ivec2(position.x() + digitSize / 2 - nbPixels, position.y() - digitSize / 2 + nbPixels / 2), digitSize / 2 + nbPixels / 2, nbPixels, color);
                                                              drawVLine(ivec2(position.x(), position.y() - digitSize + nbPixels), digitSize / 2, nbPixels, color);
                                                          } else if (digit == 3) {
                                                              drawHLine(ivec2(position.x() + digitSize / 2 - nbPixels, position.y()), digitSize, nbPixels, color);
                                                              drawVLine(position, digitSize / 2, nbPixels, color);
                                                              drawVLine(ivec2(position.x(), position.y() - digitSize / 2 + nbPixels / 2), digitSize / 2, nbPixels, color);
                                                              drawVLine(ivec2(position.x(), position.y() - digitSize + nbPixels), digitSize / 2, nbPixels, color);
                                                          } else if (digit == 4) {
                                                              drawHLine(ivec2(position.x(), position.y() - digitSize / 2), digitSize / 2 + nbPixels / 2, nbPixels, color);
                                                              drawHLine(ivec2(position.x() + digitSize / 2 - nbPixels, position.y()), digitSize, nbPixels, color);
                                                              drawVLine(ivec2(position.x(), position.y() - digitSize / 2 + nbPixels / 2), digitSize / 2, nbPixels, color);
                                                          } else if (digit == 5) {
                                                              drawVLine(position, digitSize / 2, nbPixels, color);
                                                              drawHLine(ivec2(position.x(), position.y() - digitSize / 2 + nbPixels / 2), digitSize / 2 + nbPixels / 2, nbPixels, color);
                                                              drawVLine(ivec2(position.x(), position.y() - digitSize / 2 + nbPixels / 2), digitSize / 2, nbPixels, color);
                                                              drawHLine(ivec2(position.x() + digitSize / 2 - nbPixels, position.y()), digitSize / 2 + nbPixels / 2, nbPixels, color);
                                                              drawVLine(ivec2(position.x(), position.y() - digitSize + nbPixels), digitSize / 2, nbPixels, color);
                                                          } else if (digit == 6) {
                                                              drawVLine(position, digitSize / 2, nbPixels, color);
                                                              drawHLine(ivec2(position.x() + digitSize / 2 - nbPixels, position.y()), digitSize, nbPixels, color);
                                                              drawVLine(ivec2(position.x(), position.y() - digitSize / 2 + nbPixels / 2), digitSize / 2, nbPixels, color);
                                                              drawHLine(position, digitSize / 2 + nbPixels / 2, nbPixels, color);
                                                              drawVLine(ivec2(position.x(), position.y() - digitSize + nbPixels), digitSize / 2, nbPixels, color);
                                                          } else if (digit == 7) {
                                                              drawVLine(ivec2(position.x(), position.y() - digitSize + nbPixels), digitSize / 2, nbPixels, color);
                                                              drawHLine(ivec2(position.x() + digitSize / 2 - nbPixels, position.y()), digitSize, nbPixels, color);
                                                          } else if (digit == 8) {
                                                              drawHLine(position, digitSize, nbPixels, color);
                                                              drawHLine(ivec2(position.x() + digitSize / 2 - nbPixels, position.y()), digitSize, nbPixels, color);
                                                              drawVLine(position, digitSize / 2, nbPixels, color);
                                                              drawVLine(ivec2(position.x(), position.y() - digitSize / 2 + nbPixels / 2), digitSize / 2, nbPixels, color);
                                                              drawVLine(ivec2(position.x(), position.y() - digitSize + nbPixels), digitSize / 2, nbPixels, color);
                                                          } else if (digit == 9) {
                                                              drawVLine(position, digitSize / 2, nbPixels, color);
                                                              drawHLine(ivec2(position.x() + digitSize / 2 - nbPixels, position.y()), digitSize, nbPixels, color);
                                                              drawVLine(ivec2(position.x(), position.y() - digitSize / 2 + nbPixels / 2), digitSize / 2, nbPixels, color);
                                                              drawHLine(ivec2(position.x(), position.y() - digitSize / 2 + nbPixels / 2), digitSize / 2 + nbPixels / 2, nbPixels, color);
                                                              drawVLine(ivec2(position.x(), position.y() - digitSize + nbPixels), digitSize / 2, nbPixels, color);
                                                          }
                                                      }
                                                      void drawSquare(ivec2 position, int size, vec4 color) {
                                                          int startY = position.y();
                                                          int startX = position.x();
                                                          while (position.y() > startY - size) {
                                                             while (position.x() < startX + size) {
                                                                imageStore(img_output, position, color);
                                                                position.x()++;
                                                             }
                                                             position.y()--;
                                                             position.x() = startX;
                                                          }
                                                      }
                                                      void drawPunt(ivec2 position, int nbPixels, vec4 color) {
                                                          int puntSize = nbPixels * 2;
                                                          drawSquare(position, puntSize, color);
                                                      })" \
                                                      R"(ivec2 print (ivec2 position, int nbPixels, vec4 color, double number) {
                                                          int digitSize = nbPixels * 10;
                                                          int digitSpacing = nbPixels * 6;
                                                          if (number < 0) {
                                                             number = -number;
                                                             drawVLine(ivec2(position.x(), position.y() - digitSize / 2 + nbPixels / 2), digitSize / 2, nbPixels, color);
                                                             position.x() += digitSpacing;
                                                          }
                                                          int pe = int(number);
                                                          int n = 0;
                                                          uint rpe[10];
                                                          do {
                                                             uint digit = pe % 10;
                                                             pe /= 10;
                                                             if (n < 10) {
                                                                rpe[n] = digit;
                                                             }
                                                             n++;
                                                          } while (pe != 0);
                                                          if (n >= 10)
                                                            n = 9;
                                                          //drawDigit(position, nbPixels, color,0);
                                                          for (int i = n-1; i >= 0; i--) {
                                                             drawDigit(position, nbPixels, color, rpe[i]);
                                                             //drawDigit(position, nbPixels, color,n-i-1);
                                                             position.x() += digitSpacing;
                                                          }
                                                          double rest = fract(number);
                                                          if (rest > 0) {
                                                              drawPunt(position, nbPixels, color);
                                                              position.x() += digitSpacing;
                                                              do {
                                                                 rest *= 10;
                                                                 int digit = int(rest);
                                                                 rest -= digit;
                                                                 drawDigit(position, nbPixels, color, digit);
                                                                 position.x() += digitSpacing;
                                                              } while (rest != 0);
                                                          }
                                                          return position;
                                                      }
                                                      ivec2 print (ivec2 position, int nbPixels, vec4 color, mat4 matrix) {
                                                          int numberSpacing = 10;
                                                          for (uint i = 0; i < 4; i++) {
                                                             for (uint j = 0; j < 4; j++) {
                                                                position = print(position, nbPixels, color, matrix[i][j]);
                                                                position.x() += numberSpacing;
                                                             }
                                                          }
                                                          return position;
                                                      }
                                                      ivec2 print (ivec2 position, int nbPixels, vec4 color, vec4 vector) {
                                                          int numberSpacing = 10;
                                                          for (uint i = 0; i < 4; i++) {
                                                            position = print(position, nbPixels, color, vector[i]);
                                                            position.x() += numberSpacing;
                                                          }
                                                          return position;
                                                      }

                                                                 void main () {
                                                                     vec2 position = (gl_FragCoord.x()y / resolution.x()y);
                                                                     vec2 invPosition = vec2(position.x(), 1 - position.y());
                                                                     vec4 depth = texture2D(depthTexture, position);
                                                                     vec4 invDepth = texture2D (depthTexture, invPosition);
                                                                     vec4 alpha = texture2D(alphaMap, position);
                                                                     float s01 = textureOffset(depthTexture, position, off.x()y).z();
                                                                     float s21 = textureOffset(depthTexture, position, off.z()y).z();
                                                                     float s10 = textureOffset(depthTexture, position, off.y()x).z();
                                                                     float s12 = textureOffset(depthTexture, position, off.y()z).z();
                                                                     vec3 va = normalize (vec3(size.x()y, s21 - s01));
                                                                     vec3 vb = normalize (vec3(size.y()x, s12 - s10));
                                                                     vec3 normal = vec3(cross(va, vb));
                                                                     vec4 bump = texture2D(bumpMap, position);
                                                                     vec4 specularInfos = texture2D(specularTexture, position);
                                                                     vec3 sLightPos = vec3 (lightPos.x(), lightPos.y(), -lightPos.z() * (gl_DepthRange.far - gl_DepthRange.near));
                                                                     float radius = lightPos.w;
                                                                     vec3 pixPos = vec3 (gl_FragCoord.x(), gl_FragCoord.y(), -depth.z() * (gl_DepthRange.far - gl_DepthRange.near));
                                                                     vec4 lightMapColor = texture2D(lightMap, position);
                                                                     vec3 viewPos = vec3(resolution.x() * 0.5f, resolution.y() * 0.5f, 0);
                                                                     float z = gl_FragCoord.z();
                                                                     vec3 vertexToLight = sLightPos - pixPos;
                                                                     if (bump.x() != 0 || bump.y() != 0 || bump.z() != 0) {
                                                                         vec3 tmpNormal = (normal.x()yz);
                                                                         vec3 tangeant = normalize (vec3(size.x()y, s21 - s01));
                                                                         vec3 binomial = normalize (vec3(size.y()x, s12 - s10));
                                                                         normal.x() = dot(bump.x()yz, tangeant);
                                                                         normal.y() = dot(bump.x()yz, binomial);
                                                                         normal.z() = dot(bump.x()yz, tmpNormal);
                                                                     }
                                                                     if (/*layer > depth.y() || layer == depth.y() &&*/ z > depth.z()) {
                                                                         vec4 specularColor = vec4(0, 0, 0, 0);
                                                                         float attenuation = 1.f - length(vertexToLight) / radius;
                                                                         vec3 pixToView = pixPos - viewPos;
                                                                         float normalLength = dot(normal.x()yz, vertexToLight);
                                                                         vec3 lightReflect = vertexToLight + 2 * (normal.x()yz * normalLength - vertexToLight);
                                                                         float m = specularInfos.r;
                                                                         float p = specularInfos.g;
                                                                         float specularFactor = dot(normalize(pixToView), normalize(lightReflect));
                                                                         specularFactor = pow (specularFactor, p);
                                                                         if (specularFactor > 0) {
                                                                             specularColor = vec4(lightColor.rgb, 1) * m * specularFactor;
                                                                         }
                                                                         if (normal.x() != 0 || normal.y() != 0 || normal.z() != 0 && vertexToLight.z() > 0.f) {
                                                                             vec3 dirToLight = normalize(vertexToLight.x()yz);
                                                                             float nDotl = max(dot (dirToLight, normal.x()yz), 0.0);
                                                                             attenuation *= nDotl;

                                                                         }
                                                                         fColor = vec4(lightColor.rgb, 1) * max(0.0f, attenuation) + specularColor * (1 - alpha.a);
                                                                     } else {
                                                                         fColor = lightMapColor;
                                                                     }
                                                                 }
                                                                 )";
                        if (!debugShader.loadFromMemory(simpleVertexShader, simpleFragmentShader)) {
                            throw core::Erreur(54, "Failed to load debug shader", 0);
                        }
                        if (!depthBufferGenerator.loadFromMemory(indirectRenderingVertexShader, depthGenFragShader))
                            throw core::Erreur(50, "Failed to load depth buffer generator shader", 0);


                        if (!specularTextureGenerator.loadFromMemory(specularIndirectRenderingVertexShader, specularGenFragShader))
                            throw core::Erreur(52, "Failed to load specular texture generator shader", 0);

                        if (!bumpTextureGenerator.loadFromMemory(indirectRenderingVertexShader, bumpGenFragShader))
                            throw core::Erreur(53, "Failed to load bump texture generator shader", 0);
                        if (!buildAlphaBufferGenerator.loadFromMemory(indirectRenderingVertexShader, buildAlphaBufferFragmentShader))
                            throw core::Erreur(53, "Failed to load build alpha buffer generator shader", 0);

                        if (!lightMapGenerator.loadFromMemory(perPixLightingIndirectRenderingVertexShader, perPixLightingFragmentShader))
                            throw core::Erreur(54, "Failed to load light map generator shader", 0);
                        bumpTextureGenerator.setParameter("resolution", resolution.x(), resolution.y(), resolution.z());
                        bumpTextureGenerator.setParameter("depthBuffer", depthBuffer.getTexture());
                        bumpTextureGenerator.setParameter("bumpMap", bumpTexture.getTexture());

                        specularTextureGenerator.setParameter("resolution", resolution.x(), resolution.y(), resolution.z());
                        specularTextureGenerator.setParameter("depthBuffer", depthBuffer.getTexture());
                        specularTextureGenerator.setParameter("specularBuffer", specularTexture.getTexture());

                        buildAlphaBufferGenerator.setParameter("lightDepthBuffer", lightDepthBuffer.getTexture());

                        lightMapGenerator.setParameter("resolution", resolution.x(), resolution.y(), resolution.z());
                        lightMapGenerator.setParameter("depthTexture", depthBuffer.getTexture());
                        lightMapGenerator.setParameter("alphaMap", alphaBuffer.getTexture());
                        lightMapGenerator.setParameter("specularTexture",specularTexture.getTexture());
                        lightMapGenerator.setParameter("bumpMap",bumpTexture.getTexture());
                        lightMapGenerator.setParameter("lightMap",lightMap.getTexture());
                        std::vector<Texture*> allTextures = Texture::getAllTextures();
                        Samplers allSamplers{};
                        std::vector<math::Matrix4f> textureMatrices;
                        for (unsigned int i = 0; i < allTextures.size(); i++) {
                            textureMatrices.push_back(allTextures[i]->getTextureMatrix());
                            GLuint64 handle_texture = allTextures[i]->getTextureHandle();
                            allTextures[i]->makeTextureResident(handle_texture);
                            allSamplers.tex[i].handle = handle_texture;
                            ////std::cout<<"add texture i : "<<i<<" id : "<<allTextures[i]->getNativeHandle()<<std::endl;
                        }
                        depthBufferGenerator.setParameter("textureMatrix", textureMatrices);
                        specularTextureGenerator.setParameter("textureMatrix", textureMatrices);
                        bumpTextureGenerator.setParameter("textureMatrix", textureMatrices);
                        depthBuffer.setActive(true);
                        glCheck(glGenBuffers(1, &ubo));

                        glCheck(glBindBuffer(GL_UNIFORM_BUFFER, ubo));
                        glCheck(glBufferData(GL_UNIFORM_BUFFER, sizeof(Samplers),allSamplers.tex, GL_STATIC_DRAW));
                        ////std::cout<<"size : "<<sizeof(Samplers)<<" "<<alignof (alignas(16) uint64_t[200])<<std::endl;

                        glCheck(glBindBuffer(GL_UNIFORM_BUFFER, 0));
                        glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                        glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, modelDataBuffer));
                        glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, materialDataBuffer));

                        lightDepthBuffer.setActive(true);
                        glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                        glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, modelDataBuffer));
                        glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, materialDataBuffer));
                        alphaBuffer.setActive(true);
                        glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                        glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, modelDataBuffer));
                        glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, materialDataBuffer));
                        specularTexture.setActive(true);
                        glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                        glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, modelDataBuffer));
                        glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, materialDataBuffer));
                        bumpTexture.setActive(true);
                        glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                        glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, modelDataBuffer));
                        glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, materialDataBuffer));
                        lightMap.setActive(true);
                        glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                        glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, modelDataBuffer));
                        glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, materialDataBuffer));

                        for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                            vbBindlessTex[i].setPrimitiveType(static_cast<PrimitiveType>(i));
                        }
                    }

            }
            void LightRenderComponent::pushEvent(window::IEvent event, RenderWindow& rw) {
                if (event.type == window::IEvent::WINDOW_EVENT && event.window.type == window::IEvent::WINDOW_EVENT_RESIZED && &getWindow() == &rw && isAutoResized()) {
                    //std::cout<<"recompute size"<<std::endl;
                    recomputeSize();
                    getListener().pushEvent(event);
                    getView().reset(physic::BoundingBox(getView().getViewport().getPosition().x(), getView().getViewport().getPosition().y(), getView().getViewport().getPosition().z(), event.window.data1, event.window.data2, getView().getViewport().getDepth()));
                }
            }
            bool LightRenderComponent::needToUpdate() {
            return update;
        }
        void LightRenderComponent::loadTextureIndexes () {
            std::vector<Texture*> allTextures = Texture::getAllTextures();
            Samplers allSamplers{};
            std::vector<math::Matrix4f> textureMatrices;
            for (unsigned int i = 0; i < allTextures.size(); i++) {
                textureMatrices.push_back(allTextures[i]->getTextureMatrix());
                GLuint64 handle_texture = allTextures[i]->getTextureHandle();
                allTextures[i]->makeTextureResident(handle_texture);
                allSamplers.tex[i].handle = handle_texture;
                ////std::cout<<"add texture i : "<<i<<" id : "<<allTextures[i]->getNativeHandle()<<std::endl;
            }
            glCheck(glBindBuffer(GL_UNIFORM_BUFFER, ubo));
            glCheck(glBufferData(GL_UNIFORM_BUFFER, sizeof(Samplers),allSamplers.tex, GL_STATIC_DRAW));
            glCheck(glBindBuffer(GL_UNIFORM_BUFFER, 0));
        }
        std::string LightRenderComponent::getExpression() {
            return expression;
        }
        void LightRenderComponent::clear() {
             depthBuffer.clear(Color::Transparent);
             alphaBuffer.clear(Color::Transparent);

             glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf));
             glCheck(glBindTexture(GL_TEXTURE_2D, depthTex));
             glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, view.getSize().x(), view.getSize().y(), GL_RGBA,
             GL_FLOAT, NULL));
             glCheck(glBindTexture(GL_TEXTURE_2D, 0));
             glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
             lightDepthBuffer.clear(Color::Transparent);
             glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf2));
             glCheck(glBindTexture(GL_TEXTURE_2D, lightDepthTex));
             glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, view.getSize().x(), view.getSize().y(), GL_RGBA,
             GL_FLOAT, NULL));
             glCheck(glBindTexture(GL_TEXTURE_2D, 0));
             glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
             alphaBuffer.clear(Color::Transparent);
             glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf3));
             glCheck(glBindTexture(GL_TEXTURE_2D, alphaTex));
             glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, view.getSize().x(), view.getSize().y(), GL_RGBA,
             GL_FLOAT, NULL));
             glCheck(glBindTexture(GL_TEXTURE_2D, 0));
             glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
             glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf4));
             glCheck(glBindTexture(GL_TEXTURE_2D, frameBufferTex));
             glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, view.getSize().x(), view.getSize().y(), GL_RGBA,
             GL_FLOAT, NULL));
             glCheck(glBindTexture(GL_TEXTURE_2D, 0));
             glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
             normalMap.clear(Color::Transparent);
             specularTexture.clear(Color::Transparent);
             bumpTexture.clear(Color::Transparent);
             Color ambientColor = g2d::AmbientLight::getAmbientLight().getColor();
             lightMap.clear(ambientColor);
        }
        Sprite& LightRenderComponent::getDepthBufferTile() {
            return depthBufferTile;
        }
        Sprite& LightRenderComponent::getspecularTile () {
            return specularBufferTile;
        }
        Sprite& LightRenderComponent::getBumpTile() {
            return bumpMapTile;
        }
        Sprite& LightRenderComponent::getLightTile() {
            return lightMapTile;
        }
        const Texture& LightRenderComponent::getDepthBufferTexture() {
            return depthBuffer.getTexture();
        }
        const Texture& LightRenderComponent::getSpecularTexture() {
            return specularTexture.getTexture();
        }
        const Texture& LightRenderComponent::getbumpTexture() {
            return bumpTexture.getTexture();
        }
        const Texture& LightRenderComponent::getLightMapTexture() {
            return lightMap.getTexture();
        }
        bool LightRenderComponent::loadEntitiesOnComponent(std::vector<Entity*> vEntities)
        {
            {
                std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                datasReady = false;
                batcher.clear();
                normalBatcher.clear();
                lightBatcher.clear();
            }

            for (unsigned int i = 0; i < vEntities.size(); i++) {

                if (vEntities[i] != nullptr && vEntities[i]->isLeaf()) {
                    std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                    if (vEntities[i]->isLight()) {
                        for (unsigned int j = 0; j <  vEntities[i]->getNbFaces(); j++) {
                            lightBatcher.addFace(vEntities[i]->getFace(j));
                        }
                    } else {
                        for (unsigned int j = 0; j <  vEntities[i]->getNbFaces(); j++) {
                            if (vEntities[i]->getDrawMode() == Entity::INSTANCED)
                                batcher.addFace(vEntities[i]->getFace(j));
                            else
                                normalBatcher.addFace(vEntities[i]->getFace(j));
                        }
                    }
                }
            }

            visibleEntities = vEntities;
            update = true;
            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
            datasReady = true;
            return true;
        }
        void LightRenderComponent::setView(View view){
            this->view = view;
            depthBuffer.setView(view);
            normalMap.setView(view);
            specularTexture.setView(view);
            bumpTexture.setView(view);
            lightMap.setView(view);
            lightDepthBuffer.setView(view);
        }
        void LightRenderComponent::setExpression(std::string expression) {
            update = true;
            this->expression = expression;
        }
        void LightRenderComponent::onVisibilityChanged (bool visible) {
            if (visible) {
                glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
            } else {
                glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0));
            }
        }
        void LightRenderComponent::drawDepthLightInstances() {
            for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                vbBindlessTex[i].clear();
            }
            std::array<std::vector<DrawArraysIndirectCommand>, Batcher::nbPrimitiveTypes> drawArraysIndirectCommands;
            std::array<std::vector<ModelData>, Batcher::nbPrimitiveTypes> matrices;
            std::array<std::vector<MaterialData>, Batcher::nbPrimitiveTypes> materials;
            std::array<unsigned int, Batcher::nbPrimitiveTypes> firstIndex, baseInstance;
            for (unsigned int i = 0; i < firstIndex.size(); i++) {
                firstIndex[i] = 0;
            }
            for (unsigned int i = 0; i < baseInstance.size(); i++) {
                baseInstance[i] = 0;
            }
            for (unsigned int i = 0; i < m_light_instances.size(); i++) {
                if (m_light_instances[i].getAllVertices().getVertexCount() > 0) {
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    ////std::cout<<"instance : "<<i<<std::endl;
                    unsigned int p = m_light_instances[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = 0;
                    material.layer = m_light_instances[i].getMaterial().getLayer();
                    materials[p].push_back(material);
                    ModelData model;
                    TransformMatrix tm;
                    model.worldMat = tm.getMatrix().transpose();
                    matrices[p].push_back(model);
                    unsigned int vertexCount = 0;
                    for (unsigned int j = 0; j < m_light_instances[i].getAllVertices().getVertexCount(); j++) {
                        vertexCount++;
                        vbBindlessTex[p].append(m_light_instances[i].getAllVertices()[j]);
                    }
                    drawArraysIndirectCommand.count = vertexCount;
                    drawArraysIndirectCommand.firstIndex = firstIndex[p];
                    drawArraysIndirectCommand.baseInstance = baseInstance[p];
                    drawArraysIndirectCommand.instanceCount = 1;
                    drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                    firstIndex[p] += vertexCount;
                    baseInstance[p] += 1;
                }
            }
            RenderStates states;
            states.blendMode = BlendNone;
            states.shader = &depthBufferGenerator;
            for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                if (vbBindlessTex[p].getVertexCount() > 0) {
                    glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelDataBuffer));
                    glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, matrices[p].size() * sizeof(ModelData), &matrices[p][0], GL_DYNAMIC_DRAW));
                    glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialDataBuffer));
                    glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, materials[p].size() * sizeof(MaterialData), &materials[p][0], GL_DYNAMIC_DRAW));
                    glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
                    glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vboIndirect));
                    glCheck(glBufferData(GL_DRAW_INDIRECT_BUFFER, drawArraysIndirectCommands[p].size() * sizeof(DrawArraysIndirectCommand), &drawArraysIndirectCommands[p][0], GL_DYNAMIC_DRAW));
                    glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
                    vbBindlessTex[p].update();
                    lightDepthBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), states, vboIndirect);
                }
            }
        }
        void LightRenderComponent::drawLightInstances() {
            for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                vbBindlessTex[i].clear();
            }
            std::array<std::vector<DrawArraysIndirectCommand>, Batcher::nbPrimitiveTypes> drawArraysIndirectCommands;
            std::array<std::vector<ModelData>, Batcher::nbPrimitiveTypes> matrices;
            std::array<std::vector<MaterialData>, Batcher::nbPrimitiveTypes> materials;
            std::array<unsigned int, Batcher::nbPrimitiveTypes> firstIndex, baseInstance;
            for (unsigned int i = 0; i < firstIndex.size(); i++) {
                firstIndex[i] = 0;
            }
            for (unsigned int i = 0; i < baseInstance.size(); i++) {
                baseInstance[i] = 0;
            }
            for (unsigned int i = 0; i < m_light_instances.size(); i++) {
                if (m_light_instances[i].getAllVertices().getVertexCount() > 0) {
                    ////std::cout<<"instance : "<<i<<std::endl;
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    unsigned int p = m_light_instances[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = 0;
                    material.layer = m_light_instances[i].getMaterial().getLayer();
                    material.lightCenter = m_light_instances[i].getMaterial().getLightCenter();
                    Color c = m_light_instances[i].getMaterial().getLightColor();
                    material.lightColor = math::Vec4f(1.f / 255.f * c.r, 1.f / 255.f * c.g, 1.f / 255.f * c.b, 1.f / 255.f * c.a);
                    materials[p].push_back(material);
                    ModelData model;
                    TransformMatrix tm;
                    model.worldMat = tm.getMatrix().transpose();
                    matrices[p].push_back(model);
                    unsigned int vertexCount = 0;
                    for (unsigned int j = 0; j < m_light_instances[i].getAllVertices().getVertexCount(); j++) {
                        vertexCount++;
                        vbBindlessTex[p].append(m_light_instances[i].getAllVertices()[j]);
                    }
                    drawArraysIndirectCommand.count = vertexCount;
                    drawArraysIndirectCommand.firstIndex = firstIndex[p];
                    drawArraysIndirectCommand.baseInstance = baseInstance[p];
                    drawArraysIndirectCommand.instanceCount = 1;
                    drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                    firstIndex[p] += vertexCount;
                    baseInstance[p] += 1;
                }
            }
            RenderStates states;
            states.blendMode = BlendAdd;
            states.shader = &lightMapGenerator;
            states.texture = nullptr;
            for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                if (vbBindlessTex[p].getVertexCount() > 0) {
                    glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelDataBuffer));
                    glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, matrices[p].size() * sizeof(ModelData), &matrices[p][0], GL_DYNAMIC_DRAW));
                    glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialDataBuffer));
                    glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, materials[p].size() * sizeof(MaterialData), &materials[p][0], GL_DYNAMIC_DRAW));
                    glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
                    glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vboIndirect));
                    glCheck(glBufferData(GL_DRAW_INDIRECT_BUFFER, drawArraysIndirectCommands[p].size() * sizeof(DrawArraysIndirectCommand), &drawArraysIndirectCommands[p][0], GL_DYNAMIC_DRAW));
                    glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
                    vbBindlessTex[p].update();
                    lightMap.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), states, vboIndirect);
                }
            }
            /*math::Matrix4f viewMatrix = view.getViewMatrix().getMatrix().transpose();
            math::Matrix4f projMatrix = view.getProjMatrix().getMatrix().transpose();
            debugShader.setParameter("projectionMatrix", projMatrix);
            debugShader.setParameter("viewMatrix", viewMatrix);
            glCheck(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
            vb.clear();
            vb.setPrimitiveType(Quads);
            Vertex v1 (math::Vec3f(0, 0, quad.getSize().z()));
            Vertex v2 (math::Vec3f(quad.getSize().x(),0, quad.getSize().z()));
            Vertex v3 (math::Vec3f(quad.getSize().x(), quad.getSize().y(), quad.getSize().z()));
            Vertex v4 (math::Vec3f(0, quad.getSize().y(), quad.getSize().z()));
            vb.append(v1);
            vb.append(v2);
            vb.append(v3);
            vb.append(v4);
            vb.update();
            math::Matrix4f matrix = quad.getTransform().getMatrix().transpose();
            debugShader.setParameter("worldMat", matrix);
            states.blendMode = BlendNone;
            states.shader = &debugShader;
            lightMap.drawVertexBuffer(vb, states);
            glCheck(glFinish());*/
            lightMap.display();
        }
        void LightRenderComponent::drawInstances() {
            for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                vbBindlessTex[i].clear();
            }
            std::array<std::vector<DrawArraysIndirectCommand>, Batcher::nbPrimitiveTypes> drawArraysIndirectCommands;
            std::array<std::vector<ModelData>, Batcher::nbPrimitiveTypes> matrices;
            std::array<std::vector<MaterialData>, Batcher::nbPrimitiveTypes> materials;
            std::array<unsigned int, Batcher::nbPrimitiveTypes> firstIndex, baseInstance;
            for (unsigned int i = 0; i < firstIndex.size(); i++) {
                firstIndex[i] = 0;
            }
            for (unsigned int i = 0; i < baseInstance.size(); i++) {
                baseInstance[i] = 0;
            }
            for (unsigned int i = 0; i < m_normals.size(); i++) {
               if (m_normals[i].getAllVertices().getVertexCount() > 0) {
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    unsigned int p = m_normals[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = (m_normals[i].getMaterial().getTexture() != nullptr) ? m_normals[i].getMaterial().getTexture()->getNativeHandle() : 0;
                    material.layer = m_normals[i].getMaterial().getLayer();
                    materials[p].push_back(material);
                    ModelData model;
                    TransformMatrix tm;
                    model.worldMat = tm.getMatrix().transpose();
                    matrices[p].push_back(model);
                    unsigned int vertexCount = 0;
                    for (unsigned int j = 0; j < m_normals[i].getAllVertices().getVertexCount(); j++) {
                        vertexCount++;
                        vbBindlessTex[p].append(m_normals[i].getAllVertices()[j]);
                    }
                    drawArraysIndirectCommand.count = vertexCount;
                    drawArraysIndirectCommand.firstIndex = firstIndex[p];
                    drawArraysIndirectCommand.baseInstance = baseInstance[p];
                    drawArraysIndirectCommand.instanceCount = 1;
                    drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                    firstIndex[p] += vertexCount;
                    baseInstance[p] += 1;
                }
            }
            for (unsigned int i = 0; i < m_instances.size(); i++) {

                if (m_instances[i].getAllVertices().getVertexCount() > 0) {
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    unsigned int p = m_instances[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = (m_instances[i].getMaterial().getTexture() != nullptr) ? m_instances[i].getMaterial().getTexture()->getNativeHandle() : 0;
                    material.layer = m_instances[i].getMaterial().getLayer();
                    materials[p].push_back(material);
                    std::vector<TransformMatrix*> tm = m_instances[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData model;
                        model.worldMat = tm[j]->getMatrix().transpose();
                        matrices[p].push_back(model);
                    }
                    unsigned int vertexCount = 0;
                    if (m_instances[i].getVertexArrays().size() > 0) {
                        Entity* entity = m_instances[i].getVertexArrays()[0]->getEntity();
                        for (unsigned int j = 0; j < m_instances[i].getVertexArrays().size(); j++) {
                            if (entity == m_instances[i].getVertexArrays()[j]->getEntity()) {
                                for (unsigned int k = 0; k < m_instances[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                    vertexCount++;
                                    vbBindlessTex[p].append((*m_instances[i].getVertexArrays()[j])[k], (m_instances[i].getMaterial().getTexture() != nullptr) ? m_instances[i].getMaterial().getTexture()->getId() : 0);
                                    vbBindlessTex[p].addLayer(m_instances[i].getMaterial().getLayer());
                                }
                            }
                        }
                    }
                    drawArraysIndirectCommand.count = vertexCount;
                    drawArraysIndirectCommand.firstIndex = firstIndex[p];
                    drawArraysIndirectCommand.baseInstance = baseInstance[p];
                    drawArraysIndirectCommand.instanceCount = tm.size();
                    drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                    firstIndex[p] += vertexCount;
                    baseInstance[p] += tm.size();
                }
            }
            RenderStates states;
            states.blendMode = BlendNone;
            states.shader = &depthBufferGenerator;
            states.texture = nullptr;
            for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                if (vbBindlessTex[p].getVertexCount() > 0) {
                    glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelDataBuffer));
                    glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, matrices[p].size() * sizeof(ModelData), &matrices[p][0], GL_DYNAMIC_DRAW));
                    glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialDataBuffer));
                    glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, materials[p].size() * sizeof(MaterialData), &materials[p][0], GL_DYNAMIC_DRAW));
                    glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
                    glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vboIndirect));
                    glCheck(glBufferData(GL_DRAW_INDIRECT_BUFFER, drawArraysIndirectCommands[p].size() * sizeof(DrawArraysIndirectCommand), &drawArraysIndirectCommands[p][0], GL_DYNAMIC_DRAW));
                    glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
                    vbBindlessTex[p].update();
                    depthBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), states, vboIndirect);
                }
            }
            states.shader = &buildAlphaBufferGenerator;
            glCheck(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
            for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                if (vbBindlessTex[p].getVertexCount() > 0) {
                    glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelDataBuffer));
                    glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, matrices[p].size() * sizeof(ModelData), &matrices[p][0], GL_DYNAMIC_DRAW));
                    glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialDataBuffer));
                    glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, materials[p].size() * sizeof(MaterialData), &materials[p][0], GL_DYNAMIC_DRAW));
                    glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
                    glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vboIndirect));
                    glCheck(glBufferData(GL_DRAW_INDIRECT_BUFFER, drawArraysIndirectCommands[p].size() * sizeof(DrawArraysIndirectCommand), &drawArraysIndirectCommands[p][0], GL_DYNAMIC_DRAW));
                    glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
                    vbBindlessTex[p].update();
                    alphaBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), states, vboIndirect);
                    vbBindlessTex[p].clear();
                }
            }
            glCheck(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));

            for (unsigned int i = 0; i < firstIndex.size(); i++) {
                firstIndex[i] = 0;
            }
            for (unsigned int i = 0; i < baseInstance.size(); i++) {
                baseInstance[i] = 0;
            }
            for (unsigned int i = 0; i < matrices.size(); i++) {
                matrices[i].clear();
            }
            for (unsigned int i = 0; i < materials.size(); i++) {
                materials[i].clear();
            }
            for (unsigned int i = 0; i < drawArraysIndirectCommands.size(); i++) {
                drawArraysIndirectCommands[i].clear();
            }
            for (unsigned int i = 0; i < m_normals.size(); i++) {
               if (m_normals[i].getAllVertices().getVertexCount() > 0) {
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    unsigned int p = m_normals[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = (m_normals[i].getMaterial().getBumpTexture() != nullptr) ? m_normals[i].getMaterial().getBumpTexture()->getNativeHandle() : 0;
                    material.layer = m_normals[i].getMaterial().getLayer();
                    materials[p].push_back(material);
                    ModelData model;
                    TransformMatrix tm;
                    model.worldMat = tm.getMatrix().transpose();
                    matrices[p].push_back(model);
                    unsigned int vertexCount = 0;
                    for (unsigned int j = 0; j < m_normals[i].getAllVertices().getVertexCount(); j++) {
                        vertexCount++;
                        vbBindlessTex[p].append(m_normals[i].getAllVertices()[j]);
                    }
                    drawArraysIndirectCommand.count = vertexCount;
                    drawArraysIndirectCommand.firstIndex = firstIndex[p];
                    drawArraysIndirectCommand.baseInstance = baseInstance[p];
                    drawArraysIndirectCommand.instanceCount = 1;
                    drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                    firstIndex[p] += vertexCount;
                    baseInstance[p] += 1;
                }
            }
            for (unsigned int i = 0; i < m_instances.size(); i++) {
                if (m_instances[i].getAllVertices().getVertexCount() > 0) {
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    unsigned int p = m_instances[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = (m_instances[i].getMaterial().getBumpTexture() != nullptr) ? m_instances[i].getMaterial().getBumpTexture()->getNativeHandle() : 0;
                    material.layer = m_instances[i].getMaterial().getLayer();
                    materials[p].push_back(material);
                    std::vector<TransformMatrix*> tm = m_instances[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData model;
                        model.worldMat = tm[j]->getMatrix().transpose();
                        matrices[p].push_back(model);
                    }
                    unsigned int vertexCount = 0;
                    if (m_instances[i].getVertexArrays().size() > 0) {
                        Entity* entity = m_instances[i].getVertexArrays()[0]->getEntity();
                        for (unsigned int j = 0; j < m_instances[i].getVertexArrays().size(); j++) {
                            if (entity == m_instances[i].getVertexArrays()[j]->getEntity()) {
                                for (unsigned int k = 0; k < m_instances[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                    vertexCount++;
                                    vbBindlessTex[p].append((*m_instances[i].getVertexArrays()[j])[k], (m_instances[i].getMaterial().getBumpTexture() != nullptr) ? m_instances[i].getMaterial().getBumpTexture()->getId() : 0);
                                    vbBindlessTex[p].addLayer(m_instances[i].getMaterial().getLayer());
                                }
                            }
                        }
                    }
                    drawArraysIndirectCommand.count = vertexCount;
                    drawArraysIndirectCommand.firstIndex = firstIndex[p];
                    drawArraysIndirectCommand.baseInstance = baseInstance[p];
                    drawArraysIndirectCommand.instanceCount = tm.size();
                    drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                    firstIndex[p] += vertexCount;
                    baseInstance[p] += tm.size();
                }
            }
            states.blendMode = BlendNone;
            states.shader = &bumpTextureGenerator;
            states.texture = nullptr;
            for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                if (vbBindlessTex[p].getVertexCount() > 0) {
                    glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelDataBuffer));
                    glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, matrices[p].size() * sizeof(ModelData), &matrices[p][0], GL_DYNAMIC_DRAW));
                    glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialDataBuffer));
                    glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, materials[p].size() * sizeof(MaterialData), &materials[p][0], GL_DYNAMIC_DRAW));
                    glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
                    glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vboIndirect));
                    glCheck(glBufferData(GL_DRAW_INDIRECT_BUFFER, drawArraysIndirectCommands[p].size() * sizeof(DrawArraysIndirectCommand), &drawArraysIndirectCommands[p][0], GL_DYNAMIC_DRAW));
                    glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
                    vbBindlessTex[p].update();
                    bumpTexture.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), states, vboIndirect);
                    vbBindlessTex[p].clear();
                }
            }
            for (unsigned int i = 0; i < firstIndex.size(); i++) {
                firstIndex[i] = 0;
            }
            for (unsigned int i = 0; i < baseInstance.size(); i++) {
                baseInstance[i] = 0;
            }
            for (unsigned int i = 0; i < matrices.size(); i++) {
                matrices[i].clear();
            }
            for (unsigned int i = 0; i < materials.size(); i++) {
                materials[i].clear();
            }
            for (unsigned int i = 0; i < drawArraysIndirectCommands.size(); i++) {
                drawArraysIndirectCommands[i].clear();
            }
            for (unsigned int i = 0; i < m_normals.size(); i++) {
               if (m_normals[i].getAllVertices().getVertexCount() > 0) {
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    unsigned int p = m_normals[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = (m_normals[i].getMaterial().getTexture() != nullptr) ? m_normals[i].getMaterial().getTexture()->getNativeHandle() : 0;
                    material.layer = m_normals[i].getMaterial().getLayer();
                    material.specularIntensity = m_normals[i].getMaterial().getSpecularIntensity();
                    material.specularPower = m_normals[i].getMaterial().getSpecularPower();
                    materials[p].push_back(material);
                    TransformMatrix tm;
                    ModelData model;
                    model.worldMat = tm.getMatrix().transpose();
                    matrices[p].push_back(model);
                    unsigned int vertexCount = 0;
                    for (unsigned int j = 0; j < m_normals[i].getAllVertices().getVertexCount(); j++) {
                        vertexCount++;
                        vbBindlessTex[p].append(m_normals[i].getAllVertices()[j]);
                    }
                    drawArraysIndirectCommand.count = vertexCount;
                    drawArraysIndirectCommand.firstIndex = firstIndex[p];
                    drawArraysIndirectCommand.baseInstance = baseInstance[p];
                    drawArraysIndirectCommand.instanceCount = 1;
                    drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                    firstIndex[p] += vertexCount;
                    baseInstance[p] += 1;
                }
            }
            for (unsigned int i = 0; i < m_instances.size(); i++) {
                DrawArraysIndirectCommand drawArraysIndirectCommand;
                if (m_instances[i].getAllVertices().getVertexCount() > 0) {
                    unsigned int p = m_instances[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = (m_instances[i].getMaterial().getBumpTexture() != nullptr) ? m_instances[i].getMaterial().getBumpTexture()->getNativeHandle() : 0;
                    material.layer = m_instances[i].getMaterial().getLayer();
                    material.specularIntensity = m_instances[i].getMaterial().getSpecularIntensity();
                    material.specularPower = m_instances[i].getMaterial().getSpecularPower();
                    materials[p].push_back(material);
                    std::vector<TransformMatrix*> tm = m_instances[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData model;
                        model.worldMat = tm[j]->getMatrix().transpose();
                        matrices[p].push_back(model);
                    }
                    unsigned int vertexCount = 0;
                    if (m_instances[i].getVertexArrays().size() > 0) {
                        Entity* entity = m_instances[i].getVertexArrays()[0]->getEntity();
                        for (unsigned int j = 0; j < m_instances[i].getVertexArrays().size(); j++) {
                            if (entity == m_instances[i].getVertexArrays()[j]->getEntity()) {
                                for (unsigned int k = 0; k < m_instances[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                    vertexCount++;
                                    vbBindlessTex[p].append((*m_instances[i].getVertexArrays()[j])[k]);
                                }
                            }
                        }
                    }
                    drawArraysIndirectCommand.count = vertexCount;
                    drawArraysIndirectCommand.firstIndex = firstIndex[p];
                    drawArraysIndirectCommand.baseInstance = baseInstance[p];
                    drawArraysIndirectCommand.instanceCount = tm.size();
                    drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                    firstIndex[p] += vertexCount;
                    baseInstance[p] += tm.size();
                }
            }
            states.blendMode = BlendNone;
            states.shader = &specularTextureGenerator;
            states.texture = nullptr;
            for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                if (vbBindlessTex[p].getVertexCount() > 0) {
                    glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelDataBuffer));
                    glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, matrices[p].size() * sizeof(ModelData), &matrices[p][0], GL_DYNAMIC_DRAW));
                    glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialDataBuffer));
                    glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, materials[p].size() * sizeof(MaterialData), &materials[p][0], GL_DYNAMIC_DRAW));
                    glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
                    glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vboIndirect));
                    glCheck(glBufferData(GL_DRAW_INDIRECT_BUFFER, drawArraysIndirectCommands[p].size() * sizeof(DrawArraysIndirectCommand), &drawArraysIndirectCommands[p][0], GL_DYNAMIC_DRAW));
                    glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
                    vbBindlessTex[p].update();
                    specularTexture.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), states, vboIndirect);
                    vbBindlessTex[p].clear();
                }
            }
            specularTexture.display();
            bumpTexture.display();
            depthBuffer.display();
        }
        void LightRenderComponent::drawNextFrame() {
            update = false;
            physic::BoundingBox viewArea = view.getViewVolume();
            math::Vec3f position (viewArea.getPosition().x(),viewArea.getPosition().y(), view.getPosition().z());
            math::Vec3f size (viewArea.getWidth(), viewArea.getHeight(), 0);

            if (lightMap.getSettings().versionMajor >= 3 && lightMap.getSettings().versionMinor >= 3) {

                   {
                       std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                       if (datasReady) {
                           datasReady = false;
                           m_instances = batcher.getInstances();
                           m_normals = normalBatcher.getInstances();
                           m_light_instances = lightBatcher.getInstances();
                       }

                   }
                    //glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                    if (!view.isOrtho())
                        view.setPerspective(80, view.getViewport().getSize().x() / view.getViewport().getSize().y(), 0.1, view.getViewport().getSize().z());
                    math::Matrix4f viewMatrix = view.getViewMatrix().getMatrix().transpose();
                    math::Matrix4f projMatrix = view.getProjMatrix().getMatrix().transpose();
                    depthBufferGenerator.setParameter("projectionMatrix", projMatrix);
                    depthBufferGenerator.setParameter("viewMatrix", viewMatrix);
                    specularTextureGenerator.setParameter("projectionMatrix", projMatrix);
                    specularTextureGenerator.setParameter("viewMatrix", viewMatrix);
                    bumpTextureGenerator.setParameter("projectionMatrix", projMatrix);
                    bumpTextureGenerator.setParameter("viewMatrix", viewMatrix);
                    buildAlphaBufferGenerator.setParameter("projectionMatrix", projMatrix);
                    buildAlphaBufferGenerator.setParameter("viewMatrix", viewMatrix);
                    /*for (unsigned int i = 0; i < m_light_instances.size(); i++) {
                        if (m_light_instances[i].getAllVertices().getVertexCount() > 0) {
                            vb.clear();
                            vb.setPrimitiveType( m_light_instances[i].getAllVertices().getPrimitiveType());
                            for (unsigned int n = 0; n < m_light_instances[i].getAllVertices().getVertexCount(); n++) {
                                vb.append(m_light_instances[i].getAllVertices()[n]);
                                vb.addLayer(m_light_instances[i].getMaterial().getLayer());
                            }
                            vb.update();
                            RenderStates states;
                            states.blendMode = BlendNone;
                            states.shader = &depthBufferNormalGenerator;
                            lightDepthBuffer.drawVertexBuffer(vb, states);
                        }
                    }
                    lightDepthBuffer.display();*/
                    drawDepthLightInstances();
                    //drawNormals();
                    drawInstances();
                    lightMapGenerator.setParameter("projectionMatrix", projMatrix);
                    lightMapGenerator.setParameter("viewMatrix", viewMatrix);
                    lightMapGenerator.setParameter("viewportMatrix", lightMap.getViewportMatrix(&lightMap.getView()).getMatrix().transpose());


                    drawLightInstances();
                    /*states.shader = &normalMapGenerator;
                    VertexArray va = depthBufferTile.getVertexArray();
                    depthBufferTile.setCenter(view.getPosition());
                    vb.clear();
                    vb.setPrimitiveType(va.getPrimitiveType());
                    for (unsigned int n = 0; n < va.getVertexCount(); n++) {
                        vb.append(va[n]);
                    }
                    vb.update();
                    math::Matrix4f worldMatrix = depthBufferTile.getTransform().getMatrix().transpose();
                    math::Matrix4f texMatrix = depthBufferTile.getTexture()->getTextureMatrix();
                    normalMapGenerator.setParameter("projectionMatrix", projMatrix);
                    normalMapGenerator.setParameter("viewMatrix", viewMatrix);
                    normalMapGenerator.setParameter("textureMatrix", texMatrix);
                    normalMapGenerator.setParameter("worldMatrix", worldMatrix);
                    states.texture = depthBufferTile.getTexture();
                    normalMap.drawVertexBuffer(vb, states);
                    normalMap.display();*/
                    /*RenderStates states;
                    states.shader = &lightMapGenerator;
                    states.blendMode = BlendAdd;
                    lightMapGenerator.setParameter("projectionMatrix", projMatrix);
                    lightMapGenerator.setParameter("viewMatrix", viewMatrix);
                    lightMapGenerator.setParameter("viewportMatrix", lightMap.getViewportMatrix(&lightMap.getView()).getMatrix().transpose());
                    for (unsigned int i = 0; i < m_light_instances.size(); i++) {
                        if (m_light_instances[i].getAllVertices().getVertexCount() > 0) {
                            for (unsigned int j = 0; j < m_light_instances[i].getVertexArrays().size(); j++) {
                                vb.clear();
                                vb.setPrimitiveType( m_light_instances[i].getVertexArrays()[j]->getPrimitiveType());
                                for (unsigned int n = 0; n < m_light_instances[i].getVertexArrays()[j]->getVertexCount(); n++) {
                                    vb.append((*m_light_instances[i].getVertexArrays()[j])[n]);
                                    vb.addLayer(m_light_instances[i].getMaterial().getLayer());
                                }
                                vb.update();
                                math::Matrix4f m = m_light_instances[i].getPerVaTransforms()[j]->getMatrix().transpose();
                                lightMapGenerator.setParameter("worldMat", m);
                                Entity* el = m_light_instances[i].getVertexArrays()[j]->getEntity();
                                ////std::cout<<"add light : "<<el<<std::endl;
                                math::Vec3f center = getWindow().mapCoordsToPixel(el->getCenter() - el->getSize()*0.5f, view);
                                //std::cout<<"light center : "<<center<<std::endl;
                                center.w = el->getSize().x() * 0.5f;
                                ////std::cout<<"center : "<<center<<std::endl;
                                /*lightMapGenerator.setParameter("lightPos", center.x(), center.y(), center.z(), center.w);
                                lightMapGenerator.setParameter("lightColor", el->getColor().r, el->getColor().g,el->getColor().b,el->getColor().a);
                                lightMap.drawVertexBuffer(vb, states);
                            }
                        }
                    }
                    lightMap.display();*/
                    //glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0));
            }
        }
        std::vector<Entity*> LightRenderComponent::getEntities() {
            return visibleEntities;
        }
        void LightRenderComponent::draw(RenderTarget& target, RenderStates states) {
            lightMapTile.setCenter(target.getView().getPosition());
            states.blendMode = BlendMultiply;
            target.draw(lightMapTile, states);
        }
        View& LightRenderComponent::getView() {
            return view;
        }
        int LightRenderComponent::getLayer() {
            return getPosition().z();
        }
        RenderTexture* LightRenderComponent::getFrameBuffer() {
            return &lightMap;
        }
        LightRenderComponent::~LightRenderComponent() {
            glDeleteBuffers(1, &modelDataBuffer);
            glDeleteBuffers(1, &materialDataBuffer);
            glDeleteBuffers(1, &ubo);
        }
        #endif

    }
}

