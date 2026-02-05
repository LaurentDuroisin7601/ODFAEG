#include "../../../include/odfaeg/Graphics/shadowRenderComponent.hpp"
//#include "../../../include/odfaeg/Graphics/application.h"
#ifndef VULKAN
#include <ODFAEG/OpenGL.hpp>
#include "glCheck.h"
#endif
#include <memory.h>

using namespace std;
namespace odfaeg {
    namespace graphic {
        #ifdef VULKAN
            ShadowRenderComponent::ShadowRenderComponent (RenderWindow& window, int layer, std::string expression,window::ContextSettings settings, bool useThread) :
            HeavyComponent(window, math::Vec3f(window.getView().getPosition().x(), window.getView().getPosition().y(), layer),
                          math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0),
                          math::Vec3f(window.getView().getSize().x() + window.getView().getSize().x() * 0.5f, window.getView().getPosition().y() + window.getView().getSize().y() * 0.5f, layer)),
            view(window.getView()),
            quad(math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), window.getSize().y() * 0.5f)),
            useThread(useThread),
            expression(expression),
            vb(window.getDevice()), vb2(window.getDevice()),
            vbBindlessTex {VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice())},
             vbBindlessTexIndexed {VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice())},
             depthGenShader(window.getDevice()), sBuildAlphaBufferShader(window.getDevice()), buildShadowMapShader(window.getDevice()), perPixShadowShader(window.getDevice()),
             vkDevice(window.getDevice()),
             depthBuffer(window.getDevice()),
             alphaBuffer(window.getDevice()),
             stencilBuffer(window.getDevice()),
             shadowMap(window.getDevice()),
             window(window)
             {
                vboIndirect = vboIndirectStagingBuffer = vboIndexedIndirectStagingBuffer = modelDataStagingBuffer = materialDataStagingBuffer = nullptr;
                maxVboIndirectSize = maxModelDataSize = maxMaterialDataSize = 0;
                needToUpdateDS = false;
                createCommandPool();
                depthBuffer.create(window.getView().getSize().x(), window.getView().getSize().y());
                depthBuffer.setView(view);
                math::Vec4f resolution(window.getView().getSize().x(), window.getView().getSize().y(),window.getView().getSize().z(), 1);

                depthBufferTile = Sprite(depthBuffer.getTexture(depthBuffer.getImageIndex()), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0), IntRect(0, 0, window.getView().getSize().x(), window.getView().getSize().y()));
                alphaBuffer.create(window.getView().getSize().x(), window.getView().getSize().y());
                alphaBuffer.setView(view);
                //alphaBuffer.m_name = "alphaBuffer";
                alphaBufferSprite = Sprite(alphaBuffer.getTexture(alphaBuffer.getImageIndex()), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0), IntRect(0, 0, window.getView().getSize().x(), window.getView().getSize().y()));

                stencilBuffer.create(1024, 1024);
                //stencilBuffer.setView(view);
                stencilBufferTile = Sprite(stencilBuffer.getTexture(stencilBuffer.getImageIndex()), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0), IntRect(0, 0, window.getView().getSize().x(), window.getView().getSize().y()));

                shadowMap.create(window.getView().getSize().x(), window.getView().getSize().y());
                shadowMap.setView(view);
                shadowTile = Sprite(shadowMap.getTexture(shadowMap.getImageIndex()), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0), IntRect(0, 0, window.getView().getSize().x(), window.getView().getSize().y()));


                core::FastDelegate<bool> signal (&ShadowRenderComponent::needToUpdate, this);
                core::FastDelegate<void> slot (&ShadowRenderComponent::drawNextFrame, this);
                core::Command cmd(signal, slot);
                getListener().connect("UPDATE", cmd);

                compileShaders();
                VkDeviceSize size = sizeof(ShadowUBO);
                shadowUBO.resize(shadowMap.getMaxFramesInFlight());
                shadowUBOMemory.resize(shadowMap.getMaxFramesInFlight());
                for (unsigned int i = 0; i < shadowMap.getMaxFramesInFlight(); i++) {
                    createBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, shadowUBO[i], shadowUBOMemory[i]);
                }
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
                for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    if (vkCreateImage(window.getDevice().getDevice(), &imageInfo, nullptr, &depthTextureImage[i]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de la creation d'une image!");
                    }


                    VkMemoryRequirements memRequirements;
                    vkGetImageMemoryRequirements(window.getDevice().getDevice(), depthTextureImage[i], &memRequirements);

                    VkMemoryAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                    allocInfo.allocationSize = memRequirements.size;
                    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

                    if (vkAllocateMemory(window.getDevice().getDevice(), &allocInfo, nullptr, &depthTextureImageMemory[i]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de l'allocation de la memoire d'une image!");
                    }
                    vkBindImageMemory(window.getDevice().getDevice(), depthTextureImage[i], depthTextureImageMemory[i], 0);
                }
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
                for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    if (vkCreateImage(window.getDevice().getDevice(), &imageInfo2, nullptr, &alphaTextureImage[i]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de la creation d'une image!");
                    }

                    VkMemoryRequirements memRequirements;
                    vkGetImageMemoryRequirements(window.getDevice().getDevice(), alphaTextureImage[i], &memRequirements);

                    VkMemoryAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                    allocInfo.allocationSize = memRequirements.size;
                    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                    if (vkAllocateMemory(window.getDevice().getDevice(), &allocInfo, nullptr, &alphaTextureImageMemory[i]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de l'allocation de la memoire d'une image!");
                    }
                    vkBindImageMemory(window.getDevice().getDevice(), alphaTextureImage[i], alphaTextureImageMemory[i], 0);
                }

                VkImageCreateInfo imageInfo3{};
                imageInfo3.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                imageInfo3.imageType = VK_IMAGE_TYPE_2D;
                imageInfo3.extent.width = static_cast<uint32_t>(window.getView().getSize().x());
                imageInfo3.extent.height = static_cast<uint32_t>(window.getView().getSize().y());
                imageInfo3.extent.depth = 1;
                imageInfo3.mipLevels = 1;
                imageInfo3.arrayLayers = 1;
                imageInfo3.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                imageInfo3.tiling = VK_IMAGE_TILING_OPTIMAL;
                imageInfo3.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                imageInfo3.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
                imageInfo3.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                imageInfo3.samples = VK_SAMPLE_COUNT_1_BIT;
                imageInfo3.flags = 0; // Optionnel
                for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    if (vkCreateImage(window.getDevice().getDevice(), &imageInfo3, nullptr, &stencilTextureImage[i]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de la creation d'une image!");
                    }
                    VkMemoryRequirements memRequirements;
                    vkGetImageMemoryRequirements(window.getDevice().getDevice(), stencilTextureImage[i], &memRequirements);
                    VkMemoryAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                    allocInfo.allocationSize = memRequirements.size;
                    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                    if (vkAllocateMemory(window.getDevice().getDevice(), &allocInfo, nullptr, &stencilTextureImageMemory[i]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de l'allocation de la memoire d'une image!");
                    }
                    vkBindImageMemory(window.getDevice().getDevice(), stencilTextureImage[i], stencilTextureImageMemory[i], 0);
                }
                createImageView();
                createSampler();

                depthBuffer.beginRecordCommandBuffers();
                std::vector<VkCommandBuffer> commandBuffers = depthBuffer.getCommandBuffers();
                unsigned int currentFrame =  depthBuffer.getCurrentFrame();
                for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    VkImageMemoryBarrier barrier = {};
                    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
                    barrier.image = depthTextureImage[i];
                    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    barrier.subresourceRange.levelCount = 1;
                    barrier.subresourceRange.layerCount = 1;
                    vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
                    VkImageMemoryBarrier barrier2 = {};
                    barrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    barrier2.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    barrier2.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    barrier2.newLayout = VK_IMAGE_LAYOUT_GENERAL;
                    barrier2.image = alphaTextureImage[i];
                    barrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    barrier2.subresourceRange.levelCount = 1;
                    barrier2.subresourceRange.layerCount = 1;
                    vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier2);
                    VkImageMemoryBarrier barrier3 = {};
                    barrier3.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    barrier3.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    barrier3.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    barrier3.newLayout = VK_IMAGE_LAYOUT_GENERAL;
                    barrier3.image = stencilTextureImage[i];
                    barrier3.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    barrier3.subresourceRange.levelCount = 1;
                    barrier3.subresourceRange.layerCount = 1;
                    vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier3);


                }
                for (unsigned int i = 0; i < shadowMap.getSwapchainImagesSize(); i++) {
                    const_cast<Texture&>(shadowMap.getTexture(i)).toShaderReadOnlyOptimal(depthBuffer.getCommandBuffers()[depthBuffer.getCurrentFrame()]);
                }


                VkSemaphoreCreateInfo semaphoreInfo{};
                semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

                offscreenAlphaFinishedSemaphore.resize(alphaBuffer.getMaxFramesInFlight());
                for (unsigned int i = 0; i < alphaBuffer.getMaxFramesInFlight(); i++) {
                    if (vkCreateSemaphore(vkDevice.getDevice(), &semaphoreInfo, nullptr, &offscreenAlphaFinishedSemaphore[i]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create compute synchronization objects for a frame!");
                    }
                }
                VkSemaphoreTypeCreateInfo timelineCreateInfo{};
                timelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
                timelineCreateInfo.pNext = nullptr;
                timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
                semaphoreInfo.pNext = &timelineCreateInfo;            ;
                for (unsigned int i = 0; i < values.size(); i++) {
                    values[i] = 0;
                }
                offscreenFinishedSemaphore.resize(shadowMap.getMaxFramesInFlight());
                for (unsigned int i = 0; i < shadowMap.getMaxFramesInFlight(); i++) {
                    timelineCreateInfo.initialValue = values[i];
                    if (vkCreateSemaphore(vkDevice.getDevice(), &semaphoreInfo, nullptr, &offscreenFinishedSemaphore[i]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create compute synchronization objects for a frame!");
                    }
                }
                for (unsigned int i = 0; i < values2.size(); i++) {
                    values2[i] = 0;
                }
                offscreenDepthFinishedSemaphore.resize(depthBuffer.getMaxFramesInFlight());
                for (unsigned int i = 0; i < depthBuffer.getMaxFramesInFlight(); i++) {
                    timelineCreateInfo.initialValue = values2[i];
                    if (vkCreateSemaphore(vkDevice.getDevice(), &semaphoreInfo, nullptr, &offscreenDepthFinishedSemaphore[i]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create compute synchronization objects for a frame!");
                    }
                }
                for (unsigned int i = 0; i < copyValues.size(); i++) {
                    copyValues[i] = 0;
                }
                copyFinishedSemaphore.resize(depthBuffer.getMaxFramesInFlight());
                for (unsigned int i = 0; i < depthBuffer.getMaxFramesInFlight(); i++) {
                    timelineCreateInfo.initialValue = copyValues[i];
                    if (vkCreateSemaphore(vkDevice.getDevice(), &semaphoreInfo, nullptr, &copyFinishedSemaphore[i]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create compute synchronization objects for a frame!");
                    }
                }

                for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                    vbBindlessTex[i].setPrimitiveType(static_cast<PrimitiveType>(i));
                    vbBindlessTexIndexed[i].setPrimitiveType(static_cast<PrimitiveType>(i));
                    for (unsigned int j = 0; j < MAX_FRAMES_IN_FLIGHT; j++) {
                        maxBufferSizeModelData[i][j] = 0;
                        maxBufferSizeMaterialData[i][j] = 0;
                        maxBufferSizeDrawCommand[i][j] = 0;
                        maxBufferSizeIndexedDrawCommand[i][j] = 0;
                        needToUpdateDSs[i][j] = false;
                    }
                }
                VkCommandBufferAllocateInfo bufferAllocInfo{};
                bufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                bufferAllocInfo.commandPool = secondaryBufferCommandPool;
                bufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
                bufferAllocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
                if (vkAllocateCommandBuffers(vkDevice.getDevice(), &bufferAllocInfo, copyMaterialDataBufferCommandBuffer.data()) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to allocate command buffers!", 1);
                }
                if (vkAllocateCommandBuffers(vkDevice.getDevice(), &bufferAllocInfo, copyDrawBufferCommandBuffer.data()) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to allocate command buffers!", 1);
                }
                if (vkAllocateCommandBuffers(vkDevice.getDevice(), &bufferAllocInfo, copyDrawIndexedBufferCommandBuffer.data()) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to allocate command buffers!", 1);
                }
                if (vkAllocateCommandBuffers(vkDevice.getDevice(), &bufferAllocInfo, copyVbBufferCommandBuffer.data()) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to allocate command buffers!", 1);
                }
                if (vkAllocateCommandBuffers(vkDevice.getDevice(), &bufferAllocInfo, copyVbIndexedBufferCommandBuffer.data()) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to allocate command buffers!", 1);
                }
                if (vkAllocateCommandBuffers(vkDevice.getDevice(), &bufferAllocInfo, copyModelDataBufferCommandBuffer.data()) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to allocate command buffers!", 1);
                }
                if (vkAllocateCommandBuffers(vkDevice.getDevice(), &bufferAllocInfo, depthCommandBuffer.data()) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to allocate command buffers!", 1);
                }
                if (vkAllocateCommandBuffers(vkDevice.getDevice(), &bufferAllocInfo, alphaCommandBuffer.data()) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to allocate command buffers!", 1);
                }
                if (vkAllocateCommandBuffers(vkDevice.getDevice(), &bufferAllocInfo, stencilCommandBuffer.data()) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to allocate command buffers!", 1);
                }
                if (vkAllocateCommandBuffers(vkDevice.getDevice(), &bufferAllocInfo, shadowCommandBuffer.data()) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to allocate command buffers!", 1);
                }
                VkPhysicalDeviceProperties deviceProperties;
                vkGetPhysicalDeviceProperties(vkDevice.getPhysicalDevice(), &deviceProperties);
                alignment = deviceProperties.limits.minStorageBufferOffsetAlignment;
                isSomethingDrawn = false;
                resolutionPC.resolution = resolution;
                layerPC.resolution = resolution;
                update = true;
             }
             void ShadowRenderComponent::compileShaders() {
                const std::string indirectRenderingVertexShader = R"(#version 460
                                                                     #extension GL_EXT_debug_printf : enable
                                                                 layout (location = 0) in vec3 position;
                                                                 layout (location = 1) in vec4 color;
                                                                 layout (location = 2) in vec2 texCoords;
                                                                 layout (location = 3) in vec3 normals;
                                                                 layout (location = 4) in int drawableDataID;
                                                                 layout (push_constant)uniform PushConsts {
                                                                     mat4 projectionMatrix;
                                                                     layout (offset = 64) mat4 viewMatrix;
                                                                 } pushConsts;
                                                                 struct ModelData {
                                                                    mat4 modelMatrix;
                                                                    mat4 shadowProjMatrix;
                                                                 };
                                                                 struct MaterialData {
                                                                     vec2 uvScale;
                                                                     vec2 uvOffset;
                                                                     uint textureIndex;
                                                                     uint layer;
                                                                     uint _padding[2];
                                                                 };
                                                                 layout(binding = 0) buffer modelData {
                                                                     ModelData modelDatas[];
                                                                 };
                                                                 layout(binding = 1) buffer materialData {
                                                                     MaterialData materialDatas[];
                                                                 };
                                                                 layout (location = 0) out vec2 fTexCoords;
                                                                 layout (location = 1) out vec4 frontColor;
                                                                 layout (location = 2) out uint texIndex;
                                                                 layout (location = 3) out uint layer;
                                                                 layout (location = 4) out vec3 normal;
                                                                 layout (location = 5) out int drawableID;
                                                                 void main() {
                                                                    gl_PointSize = 2.0f;
                                                                    ModelData model = modelDatas[gl_InstanceIndex];
                                                                    MaterialData material = materialDatas[gl_DrawID];
                                                                    uint textureIndex = material.textureIndex;
                                                                    uint l = material.layer;
                                                                    gl_Position = vec4(position, 1.f) * model.modelMatrix * pushConsts.viewMatrix * pushConsts.projectionMatrix;
                                                                    //debugPrintfEXT("%v4f\n%v4f\n%v4f\n%v4f", pushConsts.viewMatrix[0], pushConsts.viewMatrix[1], pushConsts.viewMatrix[2], pushConsts.viewMatrix[3]);
                                                                    fTexCoords = texCoords * material.uvScale + material.uvOffset;
                                                                    frontColor = color;
                                                                    texIndex = textureIndex;
                                                                    layer = l;
                                                                    normal = normals;
                                                                    drawableID = drawableDataID;
                                                                 }
                                                                 )";
             const std::string buildDepthBufferFragmentShader = R"(#version 460
                                                                  #extension GL_EXT_nonuniform_qualifier : enable
                                                                  #extension GL_ARB_fragment_shader_interlock : require
                                                                  layout (location = 0) in vec2 fTexCoords;
                                                                  layout (location = 1) in vec4 frontColor;
                                                                  layout (location = 2) in flat uint texIndex;
                                                                  layout (location = 3) in flat uint layer;
                                                                  layout (location = 4) in vec3 normal;
                                                                  layout (location = 5) in flat int drawableID;
                                                                  layout (push_constant) uniform PushConsts {
                                                                     layout (offset = 128) vec4 resolution;
                                                                     layout (offset = 144) uint nbLayers;
                                                                  } pushConsts;
                                                                  layout(set = 0, binding = 3) uniform sampler2D textures[];
                                                                  layout(set = 0, binding = 2, rgba32f) uniform coherent image2D depthBuffer;
                                                                  layout(location = 0) out vec4 fColor;

                                                                  void main () {
                                                                      vec4 texel = (texIndex != 0) ? frontColor * texture(textures[texIndex-1], fTexCoords.xy) : frontColor;
                                                                      float z = gl_FragCoord.z;
                                                                      float l = layer;
                                                                      beginInvocationInterlockARB();
                                                                      vec4 depth = imageLoad(depthBuffer,ivec2(gl_FragCoord.xy));
                                                                      if (l > depth.y || l == depth.y && z >= depth.z) {
                                                                        if (texel.a < 0.1) discard;
                                                                        imageStore(depthBuffer,ivec2(gl_FragCoord.xy),vec4(0,l,z,texel.a));
                                                                        memoryBarrier();
                                                                        fColor = vec4(0, l, z, texel.a);
                                                                      } else {
                                                                        discard;
                                                                      }
                                                                      endInvocationInterlockARB();
                                                                  }
                                                                )";
             const std::string buildAlphaBufferFragmentShader = R"(#version 460
                                                              #extension GL_EXT_nonuniform_qualifier : enable
                                                              #extension GL_ARB_fragment_shader_interlock : require
                                                              #extension GL_EXT_debug_printf : enable
                                                              layout(set = 0, binding = 6) uniform sampler2D textures[];
                                                              layout(set = 0, binding = 2, rgba32f) uniform coherent image2D alphaBuffer;
                                                              layout (location = 0) out vec4 fColor;
                                                              layout (set = 0, binding = 3) uniform sampler2D depthBuffer[];
                                                              layout (set = 0, binding = 4) uniform sampler2D stencilBuffer[];
                                                              layout (push_constant) uniform PushConsts {
                                                                  layout (offset = 0) vec4 resolution;
                                                                  layout (offset = 16) uint nbLayers;
                                                                  layout (offset = 20) uint imageIndex;
                                                              } pushConsts;
                                                              layout (location = 0) in vec2 fTexCoords;
                                                              layout (location = 1) in vec4 frontColor;
                                                              layout (location = 2) in flat uint texIndex;
                                                              layout (location = 3) in flat uint layer;
                                                              layout (location = 4) in vec3 normal;
                                                              layout (location = 5) in vec4 shadowCoords;
                                                              layout (location = 6) in flat int drawableID;
                                                              void main() {
                                                                  vec4 texel = (texIndex != 0) ? frontColor * texture(textures[texIndex-1], fTexCoords.xy) : frontColor;
                                                                  float current_alpha = texel.a;
                                                                  vec4 depth = texture(depthBuffer[pushConsts.imageIndex], gl_FragCoord.xy / pushConsts.resolution.xy);
                                                                  beginInvocationInterlockARB();
                                                                  vec4 alpha = imageLoad(alphaBuffer, ivec2(gl_FragCoord.xy));
                                                                  vec3 projCoords = shadowCoords.xyz / shadowCoords.w;
                                                                  projCoords.xy = projCoords.xy * 0.5 + 0.5;
                                                                  //projCoords.y = 1.0 - projCoords.y;
                                                                  vec4 stencil = texture (stencilBuffer[pushConsts.imageIndex], projCoords.xy);
                                                                  float l = layer;
                                                                  float z = gl_FragCoord.z;
                                                                  float bias = 0.025;
                                                                  if (l > stencil.y || l == stencil.y && stencil.z - bias > 1 - projCoords.z && depth.z - bias > z && current_alpha > alpha.a) {
                                                                      imageStore(alphaBuffer,ivec2(gl_FragCoord.xy),vec4(0, l, z, current_alpha));
                                                                      memoryBarrier();

                                                                      fColor = vec4(0, 1, z, current_alpha);
                                                                  } else {
                                                                      discard;
                                                                  }
                                                                  endInvocationInterlockARB();
                                                              }
                                                              )";
            const std::string buildShadowMapFragmentShader = R"(#version 460
                                                                #extension GL_ARB_fragment_shader_interlock : require
                                                                #extension GL_EXT_nonuniform_qualifier : enable
                                                                #extension GL_EXT_debug_printf : enable

                                                                layout(set = 0, binding = 3) uniform sampler2D textures[];
                                                                layout (location = 0) in vec2 fTexCoords;
                                                                layout (location = 1) in vec4 frontColor;
                                                                layout (location = 2) in flat uint texIndex;
                                                                layout (location = 3) in flat uint layer;
                                                                layout (location = 4) in vec3 normal;
                                                                layout (location = 5) in flat int drawableID;
                                                                layout (push_constant) uniform PushConsts {
                                                                     layout (offset = 128) vec4 resolution;
                                                                     layout (offset = 144) uint nbLayers;
                                                                } pushConsts;

                                                                layout(binding = 2, rgba32f) uniform coherent image2D stencilBuffer;
                                                                layout (location = 0) out vec4 fColor;
                                                                void main() {
                                                                    vec4 texel = (texIndex != 0) ? frontColor * texture(textures[texIndex-1], fTexCoords) : frontColor;
                                                                    float current_alpha = texel.a;
                                                                    beginInvocationInterlockARB();
                                                                    vec4 alpha = imageLoad(stencilBuffer,ivec2(gl_FragCoord.xy));
                                                                    float l = layer;
                                                                    float z = gl_FragCoord.z;
                                                                    if (l > alpha.y || l == alpha.y && z > alpha.z) {
                                                                        //debugPrintfEXT("stencil : %f", z);
                                                                        if (current_alpha < 0.01) discard;
                                                                        imageStore(stencilBuffer,ivec2(gl_FragCoord.xy),vec4(0, l, z, current_alpha));
                                                                        memoryBarrier();

                                                                        //fColor = vec4(0, l, z, current_alpha);
                                                                        fColor = vec4(z, z, z, 1);
                                                                    } else {
                                                                        discard;
                                                                    }
                                                                    endInvocationInterlockARB();
                                                                }
                                                            )";
                const std::string perPixShadowIndirectRenderingVertexShader = R"(#version 460
                                                                 #extension GL_EXT_debug_printf : enable
                                                                 layout (location = 0) in vec3 position;
                                                                 layout (location = 1) in vec4 color;
                                                                 layout (location = 2) in vec2 texCoords;
                                                                 layout (location = 3) in vec3 normals;
                                                                 layout (location = 4) in int drawableDataID;
                                                                 layout (binding = 5) uniform UniformBufferObject {
                                                                     mat4 projectionMatrix;
                                                                     mat4 viewMatrix;
                                                                     mat4 lprojectionMatrix;
                                                                     mat4 lviewMatrix;
                                                                     mat4 viewportMatrix;
                                                                     vec4 lightCenter;
                                                                 } ubo;
                                                                 struct ModelData {
                                                                    mat4 modelMatrix;
                                                                    mat4 shadowProjMatrix;
                                                                 };
                                                                 struct MaterialData {
                                                                     vec2 uvScale;
                                                                     vec2 uvOffset;
                                                                     uint textureIndex;
                                                                     uint layer;
                                                                     uint _padding[2];
                                                                 };
                                                                 layout(binding = 0) buffer modelData {
                                                                     ModelData modelDatas[];
                                                                 };
                                                                 layout(binding = 1) buffer materialData {
                                                                     MaterialData materialDatas[];
                                                                 };
                                                                 layout (location = 0) out vec2 fTexCoords;
                                                                 layout (location = 1) out vec4 frontColor;
                                                                 layout (location = 2) out flat uint texIndex;
                                                                 layout (location = 3) out flat uint layer;
                                                                 layout (location = 4) out vec3 normal;
                                                                 layout (location = 5) out vec4 shadowCoords;
                                                                 layout (location = 6) out vec4 lightCenter;
                                                                 layout (location = 7) out float isOrthoProj;
                                                                 layout (location = 8) out int drawableID;
                                                                 void main() {
                                                                    gl_PointSize = 2.0f;
                                                                    ModelData model = modelDatas[gl_InstanceIndex];
                                                                    MaterialData material = materialDatas[gl_DrawID];
                                                                    uint textureIndex = material.textureIndex;
                                                                    uint l = material.layer;

                                                                    gl_Position = vec4(position, 1.f) * model.modelMatrix * model.shadowProjMatrix * ubo.viewMatrix * ubo.projectionMatrix;
                                                                    shadowCoords = vec4(position, 1.f) * model.modelMatrix * model.shadowProjMatrix * ubo.lviewMatrix * ubo.lprojectionMatrix;

                                                                    //debugPrintfEXT("%v4f\n%v4f",shadowCoords,gl_Position);
                                                                    fTexCoords = texCoords * material.uvScale + material.uvOffset;
                                                                    frontColor = color;
                                                                    texIndex = textureIndex;
                                                                    layer = l;
                                                                    normal = normals;
                                                                    drawableID = drawableDataID;
                                                                    vec4 coords = vec4(ubo.lightCenter.xyz, 1);
                                                                    coords = ubo.projectionMatrix * ubo.viewMatrix * model.modelMatrix * coords;

                                                                    coords = coords / coords.w;
                                                                    coords = ubo.viewportMatrix * coords;
                                                                    coords.w = ubo.lightCenter.w;
                                                                    lightCenter = coords;
                                                                    isOrthoProj = ubo.projectionMatrix[3][3];

                                                                 }
                                                                 )";
                const std::string perPixShadowFragmentShader = R"(#version 460
                                                                  #extension GL_EXT_nonuniform_qualifier : enable
                                                                  #extension GL_EXT_debug_printf : enable
                                                                  const vec2 size = vec2(2.0,0.0);
                                                                  const ivec3 off = ivec3(-1,0,1);
                                                                  layout (location = 0) in vec2 fTexCoords;
                                                                  layout (location = 1) in vec4 frontColor;
                                                                  layout (location = 2) in flat uint texIndex;
                                                                  layout (location = 3) in flat uint layer;
                                                                  layout (location = 4) in vec3 normal;
                                                                  layout (location = 5) in vec4 shadowCoords;
                                                                  layout (location = 6) in vec4 lightCenter;
                                                                  layout (location = 7) in float isOrthoProj;
                                                                  layout (location = 8) in flat int drawableID;
                                                                  layout (push_constant) uniform PushConsts {
                                                                       layout (offset = 0) vec4 resolution;
                                                                       layout (offset = 16) float near;
                                                                       layout (offset = 20) float far;
                                                                       layout (offset = 24) uint imageIndex;
                                                                  } pushConsts;
                                                                  layout(set = 0, binding = 6) uniform sampler2D textures[];
                                                                  layout (binding = 2) uniform sampler2D depthBuffer[];
                                                                  layout (binding = 3) uniform sampler2D alphaBuffer[];
                                                                  layout (binding = 4) uniform sampler2D stencilBuffer[];


                                                                  layout (location = 0) out vec4 fColor;

                                                                void main() {
                                                                    vec4 texel = (texIndex != 0) ? frontColor * texture(textures[texIndex-1], fTexCoords) : frontColor;

                                                                    float color = texel.a;
                                                                    if (color < 0.01) discard;
                                                                    vec4 alpha = texture(alphaBuffer[pushConsts.imageIndex], gl_FragCoord.xy / pushConsts.resolution.xy, 0);
                                                                    vec4 depth = texture(depthBuffer[pushConsts.imageIndex], gl_FragCoord.xy / pushConsts.resolution.xy, 0);
                                                                    float s01 = textureOffset(depthBuffer[pushConsts.imageIndex], gl_FragCoord.xy / pushConsts.resolution.xy, off.xy).z;
                                                                    float s21 = textureOffset(depthBuffer[pushConsts.imageIndex], gl_FragCoord.xy / pushConsts.resolution.xy, off.zy).z;
                                                                    float s10 = textureOffset(depthBuffer[pushConsts.imageIndex], gl_FragCoord.xy / pushConsts.resolution.xy, off.yx).z;
                                                                    float s12 = textureOffset(depthBuffer[pushConsts.imageIndex], gl_FragCoord.xy / pushConsts.resolution.xy, off.yz).z;
                                                                    vec3 va = normalize (vec3(size.xy, s21 - s01));
                                                                    vec3 vb = normalize (vec3(size.yx, s12 - s10));
                                                                    vec3 n = vec3(cross(va, vb));

                                                                    vec3 projCoords = shadowCoords.xyz / shadowCoords.w;
                                                                    float pixelViewZ, lightViewZ;
                                                                    bool useOrtho = (isOrthoProj != 0);

                                                                    if (useOrtho) {
                                                                        pixelViewZ = pushConsts.near + gl_FragCoord.z * (pushConsts.far - pushConsts.near);
                                                                        lightViewZ = pushConsts.near + lightCenter.z * (pushConsts.far - pushConsts.near);
                                                                    } else {
                                                                        float ndcZ = gl_FragCoord.z * 2.0 - 1.0; // Convertit de [0,1] à [-1,1]
                                                                        pixelViewZ = (2.0 * pushConsts.near * pushConsts.far) /
                                                                        (ndcZ * (pushConsts.near - pushConsts.far) - (pushConsts.near + pushConsts.far));
                                                                        ndcZ = lightCenter.z * 2.0 - 1.0; // Convertit de [0,1] à [-1,1]
                                                                        lightViewZ = (2.0 * pushConsts.near * pushConsts.far) /
                                                                        (ndcZ * (pushConsts.near - pushConsts.far) - (pushConsts.near + pushConsts.far));
                                                                    }
                                                                    projCoords.xy = projCoords.xy * 0.5 + 0.5;
                                                                    //projCoords.y = 1.0 - projCoords.y;

                                                                    vec4 stencil = texture(stencilBuffer[pushConsts.imageIndex], projCoords.xy);
                                                                    /*if (stencil.z != 0)
                                                                        debugPrintfEXT("shadow coords : %v3f\n position : %v3f", projCoords, gl_FragCoord.xyz / pushConsts.resolution.xyz);*/
                                                                    float z = gl_FragCoord.z;
                                                                    uint l = layer;
                                                                    float shadowFactor;
                                                                    vec3 lightDir = vec3(lightCenter.x, lightCenter.y, lightViewZ) - vec3(gl_FragCoord.x, gl_FragCoord.y, pixelViewZ);
                                                                    float bias = 0.002;
                                                                    if (stencil.z - bias > projCoords.z) {
                                                                        if (depth.z - bias >= z) {
                                                                            shadowFactor = 1.0;
                                                                        } else {
                                                                            //debugPrintfEXT("draw shadow");
                                                                            shadowFactor = clamp(dot(normalize(n), normalize(lightDir)), 0.0, 1.0);
                                                                        }

                                                                        vec4 litColor = vec4(1.0, 1.0, 1.0, alpha.a);
                                                                        vec4 shadowColor = vec4(0.5, 0.5, 0.5, color);

                                                                        fColor = mix(shadowColor, litColor, shadowFactor);
                                                                    } else {
                                                                        discard;
                                                                    }
                                                                  }
                                                                  )";
                if (!depthGenShader.loadFromMemory(indirectRenderingVertexShader, buildDepthBufferFragmentShader))  {
                    throw core::Erreur(52, "Error, failed to load build depth buffer shader", 3);
                }
                if (!buildShadowMapShader.loadFromMemory(indirectRenderingVertexShader, buildShadowMapFragmentShader)) {
                    throw core::Erreur(53, "Error, failed to load build shadow map shader", 3);
                }
                if (!perPixShadowShader.loadFromMemory(perPixShadowIndirectRenderingVertexShader, perPixShadowFragmentShader)) {
                    throw core::Erreur(54, "Error, failed to load per pix shadow map shader", 3);
                }
                if (!sBuildAlphaBufferShader.loadFromMemory(perPixShadowIndirectRenderingVertexShader,buildAlphaBufferFragmentShader)) {
                    throw core::Erreur(60, "Error, failed to load build alpha buffer shader", 3);
                }
             }
             void ShadowRenderComponent::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
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
             void ShadowRenderComponent::launchRenderer() {
                 if (useThread) {
                    stop = false;
                    getListener().launch();
                 }
             }
             void ShadowRenderComponent::stopRenderer() {
                stop = true;
                cv.notify_all();
                getListener().stop();
                unsigned int currentFrame = depthBuffer.getCurrentFrame();
                VkCommandBufferInheritanceInfo inheritanceInfo{};

                VkCommandBufferBeginInfo beginInfo{};

                inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                inheritanceInfo.renderPass = depthBuffer.getRenderPass(1);
                inheritanceInfo.framebuffer = VK_NULL_HANDLE;
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.pInheritanceInfo = &inheritanceInfo;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
                vkResetCommandBuffer(depthCommandBuffer[currentFrame], 0);
                if (vkBeginCommandBuffer(depthCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }


                if (vkEndCommandBuffer(depthCommandBuffer[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }

                inheritanceInfo.renderPass = stencilBuffer.getRenderPass(1);
                inheritanceInfo.framebuffer = VK_NULL_HANDLE;
                beginInfo.pInheritanceInfo = &inheritanceInfo;
                vkResetCommandBuffer(stencilCommandBuffer[currentFrame], 0);
                if (vkBeginCommandBuffer(stencilCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }

                if (vkEndCommandBuffer(stencilCommandBuffer[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }

                inheritanceInfo.renderPass = alphaBuffer.getRenderPass(1);
                inheritanceInfo.framebuffer = VK_NULL_HANDLE;
                beginInfo.pInheritanceInfo = &inheritanceInfo;
                vkResetCommandBuffer(alphaCommandBuffer[currentFrame], 0);
                if (vkBeginCommandBuffer(alphaCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }
                if (vkEndCommandBuffer(alphaCommandBuffer[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }

                inheritanceInfo.renderPass = shadowMap.getRenderPass(1);
                inheritanceInfo.framebuffer = VK_NULL_HANDLE;
                beginInfo.pInheritanceInfo = &inheritanceInfo;
                vkResetCommandBuffer(shadowCommandBuffer[currentFrame], 0);
                if (vkBeginCommandBuffer(shadowCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }
                if (vkEndCommandBuffer(shadowCommandBuffer[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }
             }
             unsigned int ShadowRenderComponent::align(unsigned int offset) {
            ////std::cout << "alignment = " << alignment << std::endl;
                return (offset + alignment - 1) & ~(alignment - 1);
            }
             void ShadowRenderComponent::createDescriptorsAndPipelines() {
                RenderStates states;
                if (useThread) {
                    states.shader = &depthGenShader;
                    createDescriptorSetLayout(states);
                    for (unsigned int p = 0; p < (Batcher::nbPrimitiveTypes -1); p++) {
                        createDescriptorPool(p, states);
                        allocateDescriptorSets(p, states);
                    }
                    states.shader = &sBuildAlphaBufferShader;
                    createDescriptorSetLayout(states);
                    for (unsigned int p = 0; p < (Batcher::nbPrimitiveTypes -1); p++) {
                        createDescriptorPool(p, states);
                        allocateDescriptorSets(p, states);
                    }
                    states.shader = &buildShadowMapShader;
                    createDescriptorSetLayout(states);
                    for (unsigned int p = 0; p < (Batcher::nbPrimitiveTypes -1); p++) {
                        createDescriptorPool(p, states);
                        allocateDescriptorSets(p, states);
                    }
                    states.shader = &perPixShadowShader;
                    createDescriptorSetLayout(states);
                    for (unsigned int p = 0; p < (Batcher::nbPrimitiveTypes -1); p++) {
                        createDescriptorPool(p, states);
                        allocateDescriptorSets(p, states);
                    }
                } else {
                    states.shader = &depthGenShader;
                    createDescriptorPool(states);
                    createDescriptorSetLayout(states);
                    allocateDescriptorSets(states);
                    states.shader = &sBuildAlphaBufferShader;
                    createDescriptorPool(states);
                    createDescriptorSetLayout(states);
                    allocateDescriptorSets(states);
                    states.shader = &buildShadowMapShader;
                    createDescriptorPool(states);
                    createDescriptorSetLayout(states);
                    allocateDescriptorSets(states);
                    states.shader = &perPixShadowShader;
                    createDescriptorPool(states);
                    createDescriptorSetLayout(states);
                    allocateDescriptorSets(states);
                }
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
                {
                    std::vector<std::vector<std::vector<VkPipelineLayoutCreateInfo>>>& pipelineLayoutInfo = depthBuffer.getPipelineLayoutCreateInfo();
                    std::vector<std::vector<std::vector<VkPipelineDepthStencilStateCreateInfo>>>& depthStencilCreateInfo = depthBuffer.getDepthStencilCreateInfo();
                    if ((Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders() > pipelineLayoutInfo.size()) {
                        pipelineLayoutInfo.resize((Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders());
                        depthStencilCreateInfo.resize((Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders());
                    }
                    for (unsigned int i = 0; i < (Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders(); i++) {
                        pipelineLayoutInfo[i].resize(1);
                        depthStencilCreateInfo[i].resize(1);
                    }
                    for (unsigned int i = 0; i < (Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders(); i++) {
                        for (unsigned int j = 0; j < 1; j++) {
                            if (SHADOWNBDEPTHSTENCIL * none.nbBlendModes > pipelineLayoutInfo[i][j].size()) {
                                pipelineLayoutInfo[i][j].resize(SHADOWNBDEPTHSTENCIL * none.nbBlendModes);
                                depthStencilCreateInfo[i][j].resize(SHADOWNBDEPTHSTENCIL* none.nbBlendModes);
                            }
                        }
                    }

                    stencilBuffer.enableDepthTest(true);
                    depthBuffer.enableDepthTest(true);
                    alphaBuffer.enableDepthTest(true);
                    shadowMap.enableDepthTest(true);
                    states.shader = &depthGenShader;

                    for (unsigned int b = 0; b < blendModes.size(); b++) {
                        states.blendMode = blendModes[b];
                        states.blendMode.updateIds();
                        for (unsigned int j = 0; j < SHADOWNBDEPTHSTENCIL; j++) {
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
                                    pipelineLayoutInfo[depthGenShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][SHADOWNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = push_constants.data();
                                    pipelineLayoutInfo[depthGenShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][SHADOWNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 2;
                                    depthStencilCreateInfo[depthGenShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][SHADOWNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                                    depthStencilCreateInfo[depthGenShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][SHADOWNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = {};
                                    depthStencilCreateInfo[depthGenShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][SHADOWNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back = {};
                                    depthBuffer.createGraphicPipeline(static_cast<PrimitiveType>(i), states, SHADOWNODEPTHNOSTENCIL, SHADOWNBDEPTHSTENCIL);
                                }
                            }
                        }
                    }
                }
                {
                    std::vector<std::vector<std::vector<VkPipelineLayoutCreateInfo>>>& pipelineLayoutInfo = alphaBuffer.getPipelineLayoutCreateInfo();
                    std::vector<std::vector<std::vector<VkPipelineDepthStencilStateCreateInfo>>>& depthStencilCreateInfo = alphaBuffer.getDepthStencilCreateInfo();
                    if ((Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders() > pipelineLayoutInfo.size()) {
                        pipelineLayoutInfo.resize((Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders());
                        depthStencilCreateInfo.resize((Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders());
                    }
                    for (unsigned int i = 0; i < (Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders(); i++) {
                        pipelineLayoutInfo[i].resize(1);
                        depthStencilCreateInfo[i].resize(1);
                    }
                    for (unsigned int i = 0; i < (Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders(); i++) {
                        for (unsigned int j = 0; j < 1; j++) {
                            if (SHADOWNBDEPTHSTENCIL * none.nbBlendModes > pipelineLayoutInfo[i][j].size()) {
                                pipelineLayoutInfo[i][j].resize(SHADOWNBDEPTHSTENCIL * none.nbBlendModes);
                                depthStencilCreateInfo[i][j].resize(SHADOWNBDEPTHSTENCIL* none.nbBlendModes);
                            }
                        }
                    }
                    states.shader = &sBuildAlphaBufferShader;
                    for (unsigned int b = 0; b < blendModes.size(); b++) {
                        states.blendMode = blendModes[b];
                        states.blendMode.updateIds();

                        for (unsigned int j = 0; j < SHADOWNBDEPTHSTENCIL; j++) {
                            for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes - 1; i++) {
                                if (j == 0) {
                                    VkPushConstantRange push_constant;
                                    //this push constant range takes up the size of a MeshPushConstants struct
                                    push_constant.offset = 0;
                                    push_constant.size = sizeof(LayerPC) + sizeof(unsigned int);
                                    //this push constant range is accessible only in the vertex shader
                                    push_constant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                                    pipelineLayoutInfo[sBuildAlphaBufferShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][SHADOWNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = &push_constant;
                                    pipelineLayoutInfo[sBuildAlphaBufferShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][SHADOWNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 1;
                                    depthStencilCreateInfo[sBuildAlphaBufferShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][SHADOWNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                                    depthStencilCreateInfo[sBuildAlphaBufferShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][SHADOWNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = {};
                                    depthStencilCreateInfo[sBuildAlphaBufferShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][SHADOWNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back = {};
                                    alphaBuffer.createGraphicPipeline(static_cast<PrimitiveType>(i), states, SHADOWNODEPTHNOSTENCIL, SHADOWNBDEPTHSTENCIL);
                                }
                            }
                        }
                    }
                }
                {
                    std::vector<std::vector<std::vector<VkPipelineLayoutCreateInfo>>>& pipelineLayoutInfo = stencilBuffer.getPipelineLayoutCreateInfo();
                    std::vector<std::vector<std::vector<VkPipelineDepthStencilStateCreateInfo>>>& depthStencilCreateInfo = stencilBuffer.getDepthStencilCreateInfo();
                    if ((Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders() > pipelineLayoutInfo.size()) {
                        pipelineLayoutInfo.resize((Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders());
                        depthStencilCreateInfo.resize((Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders());
                    }
                    for (unsigned int i = 0; i < (Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders(); i++) {
                        pipelineLayoutInfo[i].resize(1);
                        depthStencilCreateInfo[i].resize(1);
                    }
                    for (unsigned int i = 0; i < (Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders(); i++) {
                        for (unsigned int j = 0; j < 1; j++) {
                            if (SHADOWNBDEPTHSTENCIL * none.nbBlendModes > pipelineLayoutInfo[i][j].size()) {
                                pipelineLayoutInfo[i][j].resize(SHADOWNBDEPTHSTENCIL * none.nbBlendModes);
                                depthStencilCreateInfo[i][j].resize(SHADOWNBDEPTHSTENCIL* none.nbBlendModes);
                            }
                        }
                    }
                    states.shader = &buildShadowMapShader;
                    stencilBuffer.setCullMode(RenderTarget::FRONT);
                    for (unsigned int b = 0; b < blendModes.size(); b++) {
                        states.blendMode = blendModes[b];
                        states.blendMode.updateIds();

                        for (unsigned int j = 0; j < SHADOWNBDEPTHSTENCIL; j++) {
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
                                    push_constant2.size = sizeof(LayerPC);
                                    //this push constant range is accessible only in the vertex shader
                                    push_constant2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                                    push_constants[1] = push_constant2;
                                    pipelineLayoutInfo[buildShadowMapShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][SHADOWNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = push_constants.data();
                                    pipelineLayoutInfo[buildShadowMapShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][SHADOWNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 2;
                                   depthStencilCreateInfo[buildShadowMapShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][SHADOWNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                                   depthStencilCreateInfo[buildShadowMapShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][SHADOWNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = {};
                                   depthStencilCreateInfo[buildShadowMapShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][SHADOWNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back = {};
                                   stencilBuffer.createGraphicPipeline(static_cast<PrimitiveType>(i), states, SHADOWNODEPTHNOSTENCIL, SHADOWNBDEPTHSTENCIL);
                                   //////std::cout<<"pipeline layout : "<<stencilBuffer.getPipelineLayout()[buildShadowMapShader.getId() * (Batcher::nbPrimitiveTypes - 1) + i][stencilBuffer.getId()][j]<<std::endl;
                                }
                            }
                        }
                    }
                }
                {
                    std::vector<std::vector<std::vector<VkPipelineLayoutCreateInfo>>>& pipelineLayoutInfo = shadowMap.getPipelineLayoutCreateInfo();
                    std::vector<std::vector<std::vector<VkPipelineDepthStencilStateCreateInfo>>>& depthStencilCreateInfo = shadowMap.getDepthStencilCreateInfo();

                    if ((Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders() > pipelineLayoutInfo.size()) {
                        pipelineLayoutInfo.resize((Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders());
                        depthStencilCreateInfo.resize((Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders());
                    }
                    for (unsigned int i = 0; i < (Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders(); i++) {
                        pipelineLayoutInfo[i].resize(1);
                        depthStencilCreateInfo[i].resize(1);
                    }
                    for (unsigned int i = 0; i < (Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders(); i++) {
                        for (unsigned int j = 0; j < 1; j++) {
                            if (SHADOWNBDEPTHSTENCIL * none.nbBlendModes > pipelineLayoutInfo[i][j].size()) {
                                pipelineLayoutInfo[i][j].resize(SHADOWNBDEPTHSTENCIL * none.nbBlendModes);
                                depthStencilCreateInfo[i][j].resize(SHADOWNBDEPTHSTENCIL* none.nbBlendModes);
                            }
                        }
                    }
                    states.shader = &perPixShadowShader;
                    for (unsigned int b = 0; b < blendModes.size(); b++) {
                        states.blendMode = blendModes[b];
                        states.blendMode.updateIds();

                        for (unsigned int j = 0; j < SHADOWNBDEPTHSTENCIL; j++) {
                            for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes - 1; i++) {
                                if (j == 0) {
                                   VkPushConstantRange push_constant;
                                    //this push constant range starts at the beginning
                                   push_constant.offset = 0;
                                    //this push constant range takes up the size of a MeshPushConstants struct
                                   push_constant.size = sizeof(ResolutionPC) + sizeof(unsigned int);
                                    //this push constant range is accessible only in the vertex shader
                                   push_constant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                                   pipelineLayoutInfo[perPixShadowShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][SHADOWNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = &push_constant;
                                   pipelineLayoutInfo[perPixShadowShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][SHADOWNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 1;
                                   depthStencilCreateInfo[perPixShadowShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][SHADOWNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                                   depthStencilCreateInfo[perPixShadowShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][SHADOWNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = {};
                                   depthStencilCreateInfo[perPixShadowShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][SHADOWNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back = {};
                                   shadowMap.createGraphicPipeline(static_cast<PrimitiveType>(i), states, SHADOWNODEPTHNOSTENCIL, SHADOWNBDEPTHSTENCIL);
                                }
                            }
                        }
                    }
                }
             }
             void ShadowRenderComponent::createDescriptorPool(unsigned int p, RenderStates states) {
                Shader* shader = const_cast<Shader*>(states.shader);
                 if (shader == &depthGenShader) {
                    std::vector<VkDescriptorPool>& descriptorPool = depthBuffer.getDescriptorPool();
                    if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                    unsigned int descriptorId = p *  shader->getNbShaders() + shader->getId();
                    std::array<VkDescriptorPoolSize, 4> poolSizes;
                    std::vector<Texture*> allTextures = Texture::getAllTextures();

                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());
                    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    poolSizes[2].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());
                    poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[3].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight() * MAX_TEXTURES);

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
                 } else if (shader == &sBuildAlphaBufferShader) {
                    std::vector<VkDescriptorPool>& descriptorPool = alphaBuffer.getDescriptorPool();
                    if (shader->getNbShaders()* (Batcher::nbPrimitiveTypes - 1) > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders()* (Batcher::nbPrimitiveTypes - 1));
                    unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                    std::array<VkDescriptorPoolSize, 7> poolSizes;
                    std::vector<Texture*> allTextures = Texture::getAllTextures();

                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight());
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight());
                    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    poolSizes[2].descriptorCount = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight());
                    poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[3].descriptorCount = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight()) * depthBuffer.getSwapchainImagesSize();
                    poolSizes[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[4].descriptorCount = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight()) * stencilBuffer.getSwapchainImagesSize();
                    poolSizes[5].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    poolSizes[5].descriptorCount = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight());
                    poolSizes[6].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[6].descriptorCount = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight() * MAX_TEXTURES);

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
                } else if (shader == &buildShadowMapShader) {
                    std::vector<VkDescriptorPool>& descriptorPool = stencilBuffer.getDescriptorPool();
                    if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                    unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                    std::array<VkDescriptorPoolSize, 4> poolSizes;
                    std::vector<Texture*> allTextures = Texture::getAllTextures();

                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(stencilBuffer.getMaxFramesInFlight());
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(stencilBuffer.getMaxFramesInFlight());
                    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    poolSizes[2].descriptorCount = static_cast<uint32_t>(stencilBuffer.getMaxFramesInFlight());
                    poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[3].descriptorCount = static_cast<uint32_t>(stencilBuffer.getMaxFramesInFlight() * MAX_TEXTURES);

                    if (descriptorPool[descriptorId] != nullptr) {
                        vkDestroyDescriptorPool(vkDevice.getDevice(), descriptorPool[descriptorId], nullptr);
                    }

                    VkDescriptorPoolCreateInfo poolInfo{};
                    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
                    poolInfo.pPoolSizes = poolSizes.data();
                    poolInfo.maxSets = static_cast<uint32_t>(stencilBuffer.getMaxFramesInFlight());
                    if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de la creation de la pool de descripteurs!");
                    }
                } else {
                    std::vector<VkDescriptorPool>& descriptorPool = shadowMap.getDescriptorPool();
                    if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                    unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                    std::array<VkDescriptorPoolSize, 7> poolSizes;
                    std::vector<Texture*> allTextures = Texture::getAllTextures();

                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(shadowMap.getMaxFramesInFlight());
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(shadowMap.getMaxFramesInFlight());
                    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[2].descriptorCount = static_cast<uint32_t>(shadowMap.getMaxFramesInFlight()) * depthBuffer.getSwapchainImagesSize();
                    poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[3].descriptorCount = static_cast<uint32_t>(shadowMap.getMaxFramesInFlight()) * alphaBuffer.getSwapchainImagesSize();
                    poolSizes[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[4].descriptorCount = static_cast<uint32_t>(shadowMap.getMaxFramesInFlight()) * stencilBuffer.getSwapchainImagesSize();
                    poolSizes[5].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    poolSizes[5].descriptorCount = static_cast<uint32_t>(shadowMap.getMaxFramesInFlight());
                    poolSizes[6].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[6].descriptorCount = static_cast<uint32_t>(shadowMap.getMaxFramesInFlight() * MAX_TEXTURES);

                    if (descriptorPool[descriptorId] != nullptr) {
                        vkDestroyDescriptorPool(vkDevice.getDevice(), descriptorPool[descriptorId], nullptr);
                    }

                    VkDescriptorPoolCreateInfo poolInfo{};
                    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
                    poolInfo.pPoolSizes = poolSizes.data();
                    poolInfo.maxSets = static_cast<uint32_t>(shadowMap.getMaxFramesInFlight());
                    if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de la creation de la pool de descripteurs!");
                    }
                }
             }
             void ShadowRenderComponent::createDescriptorPool(RenderStates states) {
                 Shader* shader = const_cast<Shader*>(states.shader);
                 if (shader == &depthGenShader) {
                    std::vector<VkDescriptorPool>& descriptorPool = depthBuffer.getDescriptorPool();
                    if (shader->getNbShaders() > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders());
                    unsigned int descriptorId = shader->getId();
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
                 } else if (shader == &sBuildAlphaBufferShader) {
                    std::vector<VkDescriptorPool>& descriptorPool = alphaBuffer.getDescriptorPool();
                    if (shader->getNbShaders() > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders());
                    unsigned int descriptorId = shader->getId();
                    std::array<VkDescriptorPoolSize, 7> poolSizes;
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight() * allTextures.size());
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight());
                    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[2].descriptorCount = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight());
                    poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[3].descriptorCount = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight());
                    poolSizes[4].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[4].descriptorCount = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight());
                    poolSizes[5].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[5].descriptorCount = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight());
                    poolSizes[6].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    poolSizes[6].descriptorCount = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight());

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
                } else if (shader == &buildShadowMapShader) {
                    std::vector<VkDescriptorPool>& descriptorPool = stencilBuffer.getDescriptorPool();
                    if (shader->getNbShaders() > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders());
                    unsigned int descriptorId = shader->getId();
                    std::array<VkDescriptorPoolSize, 4> poolSizes;
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(stencilBuffer.getMaxFramesInFlight() * allTextures.size());
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(stencilBuffer.getMaxFramesInFlight());
                    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[2].descriptorCount = static_cast<uint32_t>(stencilBuffer.getMaxFramesInFlight());
                    poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[3].descriptorCount = static_cast<uint32_t>(stencilBuffer.getMaxFramesInFlight());

                    if (descriptorPool[descriptorId] != nullptr) {
                        vkDestroyDescriptorPool(vkDevice.getDevice(), descriptorPool[descriptorId], nullptr);
                    }

                    VkDescriptorPoolCreateInfo poolInfo{};
                    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
                    poolInfo.pPoolSizes = poolSizes.data();
                    poolInfo.maxSets = static_cast<uint32_t>(stencilBuffer.getMaxFramesInFlight());
                    if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de la creation de la pool de descripteurs!");
                    }
                } else {
                    std::vector<VkDescriptorPool>& descriptorPool = shadowMap.getDescriptorPool();
                    if (shader->getNbShaders() > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders());
                    unsigned int descriptorId = shader->getId();
                    std::array<VkDescriptorPoolSize, 7> poolSizes;
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(shadowMap.getMaxFramesInFlight() * allTextures.size());
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(shadowMap.getMaxFramesInFlight());
                    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[2].descriptorCount = static_cast<uint32_t>(shadowMap.getMaxFramesInFlight());
                    poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[3].descriptorCount = static_cast<uint32_t>(shadowMap.getMaxFramesInFlight());
                    poolSizes[4].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[4].descriptorCount = static_cast<uint32_t>(shadowMap.getMaxFramesInFlight());
                    poolSizes[5].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[5].descriptorCount = static_cast<uint32_t>(shadowMap.getMaxFramesInFlight());
                    poolSizes[6].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    poolSizes[6].descriptorCount = static_cast<uint32_t>(shadowMap.getMaxFramesInFlight());

                    if (descriptorPool[descriptorId] != nullptr) {
                        vkDestroyDescriptorPool(vkDevice.getDevice(), descriptorPool[descriptorId], nullptr);
                    }

                    VkDescriptorPoolCreateInfo poolInfo{};
                    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
                    poolInfo.pPoolSizes = poolSizes.data();
                    poolInfo.maxSets = static_cast<uint32_t>(shadowMap.getMaxFramesInFlight());
                    if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de la creation de la pool de descripteurs!");
                    }
                }
             }
             void ShadowRenderComponent::createDescriptorSetLayout(RenderStates states) {
                 Shader* shader = const_cast<Shader*>(states.shader);
                 if (shader == &depthGenShader) {
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = depthBuffer.getDescriptorSetLayout();
                    if (shader->getNbShaders() > descriptorSetLayout.size())
                        descriptorSetLayout.resize(shader->getNbShaders());
                    unsigned int descriptorId = shader->getId();
                    ////////std::cout<<"ppll descriptor id : "<<descriptorId<<std::endl;
                    VkDescriptorSetLayoutBinding modelDataLayoutBinding{};
                    modelDataLayoutBinding.binding = 0;
                    modelDataLayoutBinding.descriptorCount = 1;
                    modelDataLayoutBinding.descriptorType = (useThread) ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    modelDataLayoutBinding.pImmutableSamplers = nullptr;
                    modelDataLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                    VkDescriptorSetLayoutBinding materialDataLayoutBinding{};
                    materialDataLayoutBinding.binding = 1;
                    materialDataLayoutBinding.descriptorCount = 1;
                    materialDataLayoutBinding.descriptorType = (useThread) ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    materialDataLayoutBinding.pImmutableSamplers = nullptr;
                    materialDataLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                    VkDescriptorSetLayoutBinding headPtrImageLayoutBinding;
                    headPtrImageLayoutBinding.binding = 2;
                    headPtrImageLayoutBinding.descriptorCount = 1;
                    headPtrImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    headPtrImageLayoutBinding.pImmutableSamplers = nullptr;
                    headPtrImageLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    std::vector<VkDescriptorBindingFlags> bindingFlags(4, 0);
                    bindingFlags[3] = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                                       VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT; // seulement pour sampler[]
                    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
                    bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
                    bindingFlagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
                    bindingFlagsInfo.pBindingFlags = bindingFlags.data();

                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
                    samplerLayoutBinding.binding = 3;
                    samplerLayoutBinding.descriptorCount = MAX_TEXTURES;
                    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding.pImmutableSamplers = nullptr;
                    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    std::array<VkDescriptorSetLayoutBinding, 4> bindings = {modelDataLayoutBinding, materialDataLayoutBinding, headPtrImageLayoutBinding, samplerLayoutBinding};

                    if (descriptorSetLayout[descriptorId] != nullptr) {
                        vkDestroyDescriptorSetLayout(vkDevice.getDevice(), descriptorSetLayout[descriptorId], nullptr);
                    }

                    VkDescriptorSetLayoutCreateInfo layoutInfo{};
                    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                    layoutInfo.pNext = &bindingFlagsInfo;
                    //layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
                    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());;
                    layoutInfo.pBindings = bindings.data();

                    if (vkCreateDescriptorSetLayout(vkDevice.getDevice(), &layoutInfo, nullptr, &descriptorSetLayout[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create descriptor set layout!");
                    }
                } else if (shader == &sBuildAlphaBufferShader) {
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = alphaBuffer.getDescriptorSetLayout();
                    if (shader->getNbShaders() > descriptorSetLayout.size())
                        descriptorSetLayout.resize(shader->getNbShaders());
                    unsigned int descriptorId = shader->getId();
                    std::vector<Texture*> allTextures = Texture::getAllTextures();

                    VkDescriptorSetLayoutBinding modelDataLayoutBinding{};
                    modelDataLayoutBinding.binding = 0;
                    modelDataLayoutBinding.descriptorCount = 1;
                    modelDataLayoutBinding.descriptorType = (useThread) ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    modelDataLayoutBinding.pImmutableSamplers = nullptr;
                    modelDataLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                    VkDescriptorSetLayoutBinding materialDataLayoutBinding{};
                    materialDataLayoutBinding.binding = 1;
                    materialDataLayoutBinding.descriptorCount = 1;
                    materialDataLayoutBinding.descriptorType = (useThread) ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    materialDataLayoutBinding.pImmutableSamplers = nullptr;
                    materialDataLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                    VkDescriptorSetLayoutBinding headPtrImageLayoutBinding;
                    headPtrImageLayoutBinding.binding = 2;
                    headPtrImageLayoutBinding.descriptorCount = 1;
                    headPtrImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    headPtrImageLayoutBinding.pImmutableSamplers = nullptr;
                    headPtrImageLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding sampler2LayoutBinding{};
                    sampler2LayoutBinding.binding = 3;
                    sampler2LayoutBinding.descriptorCount = depthBuffer.getSwapchainImagesSize();
                    sampler2LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    sampler2LayoutBinding.pImmutableSamplers = nullptr;
                    sampler2LayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding sampler3LayoutBinding{};
                    sampler3LayoutBinding.binding = 4;
                    sampler3LayoutBinding.descriptorCount = stencilBuffer.getSwapchainImagesSize();
                    sampler3LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    sampler3LayoutBinding.pImmutableSamplers = nullptr;
                    sampler3LayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding uboLayoutBinding{};
                    uboLayoutBinding.binding = 5;
                    uboLayoutBinding.descriptorCount = 1;
                    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    uboLayoutBinding.pImmutableSamplers = nullptr;
                    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                    std::vector<VkDescriptorBindingFlags> bindingFlags(7, 0); // 6 bindings, flags par défaut à 0
                    bindingFlags[6] = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                          VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT; // seulement pour sampler[]

                    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
                    bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
                    bindingFlagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
                    bindingFlagsInfo.pBindingFlags = bindingFlags.data();

                    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
                    samplerLayoutBinding.binding = 6;
                    samplerLayoutBinding.descriptorCount = MAX_TEXTURES;
                    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding.pImmutableSamplers = nullptr;
                    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    std::array<VkDescriptorSetLayoutBinding, 7> bindings = {modelDataLayoutBinding, materialDataLayoutBinding, headPtrImageLayoutBinding, sampler2LayoutBinding, sampler3LayoutBinding, uboLayoutBinding, samplerLayoutBinding};

                    if (descriptorSetLayout[descriptorId] != nullptr) {
                        vkDestroyDescriptorSetLayout(vkDevice.getDevice(), descriptorSetLayout[descriptorId], nullptr);
                    }

                    VkDescriptorSetLayoutCreateInfo layoutInfo{};
                    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                    layoutInfo.pNext = &bindingFlagsInfo;
                    //layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
                    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());;
                    layoutInfo.pBindings = bindings.data();

                    if (vkCreateDescriptorSetLayout(vkDevice.getDevice(), &layoutInfo, nullptr, &descriptorSetLayout[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create descriptor set layout!");
                    }
                } else if (shader == &buildShadowMapShader) {
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = stencilBuffer.getDescriptorSetLayout();
                    if (shader->getNbShaders() > descriptorSetLayout.size())
                        descriptorSetLayout.resize(shader->getNbShaders());
                    unsigned int descriptorId = shader->getId();
                    std::vector<Texture*> allTextures = Texture::getAllTextures();

                    VkDescriptorSetLayoutBinding modelDataLayoutBinding{};
                    modelDataLayoutBinding.binding = 0;
                    modelDataLayoutBinding.descriptorCount = 1;
                    modelDataLayoutBinding.descriptorType = (useThread) ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    modelDataLayoutBinding.pImmutableSamplers = nullptr;
                    modelDataLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                    VkDescriptorSetLayoutBinding materialDataLayoutBinding{};
                    materialDataLayoutBinding.binding = 1;
                    materialDataLayoutBinding.descriptorCount = 1;
                    materialDataLayoutBinding.descriptorType = (useThread) ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    materialDataLayoutBinding.pImmutableSamplers = nullptr;
                    materialDataLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;


                    VkDescriptorSetLayoutBinding headPtrImageLayoutBinding;
                    headPtrImageLayoutBinding.binding = 2;
                    headPtrImageLayoutBinding.descriptorCount = 1;
                    headPtrImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    headPtrImageLayoutBinding.pImmutableSamplers = nullptr;
                    headPtrImageLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;



                    std::vector<VkDescriptorBindingFlags> bindingFlags(4, 0); // 6 bindings, flags par défaut à 0
                    bindingFlags[3] = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                          VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT; // seulement pour sampler[]

                    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
                    bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
                    bindingFlagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
                    bindingFlagsInfo.pBindingFlags = bindingFlags.data();

                    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
                    samplerLayoutBinding.binding = 3;
                    samplerLayoutBinding.descriptorCount = MAX_TEXTURES;
                    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding.pImmutableSamplers = nullptr;
                    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    std::array<VkDescriptorSetLayoutBinding, 4> bindings = {modelDataLayoutBinding, materialDataLayoutBinding, headPtrImageLayoutBinding, samplerLayoutBinding};

                    if (descriptorSetLayout[descriptorId] != nullptr) {
                        vkDestroyDescriptorSetLayout(vkDevice.getDevice(), descriptorSetLayout[descriptorId], nullptr);
                    }

                    VkDescriptorSetLayoutCreateInfo layoutInfo{};
                    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                    layoutInfo.pNext = &bindingFlagsInfo;
                    //layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
                    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());;
                    layoutInfo.pBindings = bindings.data();
                    if (vkCreateDescriptorSetLayout(vkDevice.getDevice(), &layoutInfo, nullptr, &descriptorSetLayout[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create descriptor set layout!");
                    }
                } else {
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = shadowMap.getDescriptorSetLayout();
                    if (shader->getNbShaders() > descriptorSetLayout.size())
                        descriptorSetLayout.resize(shader->getNbShaders());
                    unsigned int descriptorId = shader->getId();
                    std::vector<Texture*> allTextures = Texture::getAllTextures();

                    VkDescriptorSetLayoutBinding modelDataLayoutBinding{};
                    modelDataLayoutBinding.binding = 0;
                    modelDataLayoutBinding.descriptorCount = 1;
                    modelDataLayoutBinding.descriptorType = (useThread) ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    modelDataLayoutBinding.pImmutableSamplers = nullptr;
                    modelDataLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                    VkDescriptorSetLayoutBinding materialDataLayoutBinding{};
                    materialDataLayoutBinding.binding = 1;
                    materialDataLayoutBinding.descriptorCount = 1;
                    materialDataLayoutBinding.descriptorType = (useThread) ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    materialDataLayoutBinding.pImmutableSamplers = nullptr;
                    materialDataLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                    VkDescriptorSetLayoutBinding sampler2LayoutBinding{};
                    sampler2LayoutBinding.binding = 2;
                    sampler2LayoutBinding.descriptorCount = depthBuffer.getSwapchainImagesSize();
                    sampler2LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    sampler2LayoutBinding.pImmutableSamplers = nullptr;
                    sampler2LayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding sampler3LayoutBinding{};
                    sampler3LayoutBinding.binding = 3;
                    sampler3LayoutBinding.descriptorCount = alphaBuffer.getSwapchainImagesSize();
                    sampler3LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    sampler3LayoutBinding.pImmutableSamplers = nullptr;
                    sampler3LayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding sampler4LayoutBinding{};
                    sampler4LayoutBinding.binding = 4;
                    sampler4LayoutBinding.descriptorCount = stencilBuffer.getSwapchainImagesSize();
                    sampler4LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    sampler4LayoutBinding.pImmutableSamplers = nullptr;
                    sampler4LayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding uboLayoutBinding{};
                    uboLayoutBinding.binding = 5;
                    uboLayoutBinding.descriptorCount = 1;
                    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    uboLayoutBinding.pImmutableSamplers = nullptr;
                    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                    std::vector<VkDescriptorBindingFlags> bindingFlags(7, 0); // 6 bindings, flags par défaut à 0
                    bindingFlags[6] = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                          VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT; // seulement pour sampler[]

                    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
                    bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
                    bindingFlagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
                    bindingFlagsInfo.pBindingFlags = bindingFlags.data();

                    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
                    samplerLayoutBinding.binding = 6;
                    samplerLayoutBinding.descriptorCount = MAX_TEXTURES;
                    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding.pImmutableSamplers = nullptr;
                    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    std::array<VkDescriptorSetLayoutBinding, 7> bindings = {modelDataLayoutBinding, materialDataLayoutBinding, sampler2LayoutBinding, sampler3LayoutBinding, sampler4LayoutBinding, uboLayoutBinding,  samplerLayoutBinding};

                    if (descriptorSetLayout[descriptorId] != nullptr) {
                        vkDestroyDescriptorSetLayout(vkDevice.getDevice(), descriptorSetLayout[descriptorId], nullptr);
                    }

                    VkDescriptorSetLayoutCreateInfo layoutInfo{};
                    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                    layoutInfo.pNext = &bindingFlagsInfo;
                    //layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
                    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());;
                    layoutInfo.pBindings = bindings.data();
                    if (vkCreateDescriptorSetLayout(vkDevice.getDevice(), &layoutInfo, nullptr, &descriptorSetLayout[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create descriptor set layout!");
                    }
                }
             }
             void ShadowRenderComponent::allocateDescriptorSets(RenderStates states) {
                Shader* shader = const_cast<Shader*>(states.shader);
                if (shader == &depthGenShader) {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = depthBuffer.getDescriptorSet();
                    std::vector<VkDescriptorPool>& descriptorPool = depthBuffer.getDescriptorPool();
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = depthBuffer.getDescriptorSetLayout();
                    if (shader->getNbShaders() > descriptorSets.size())
                        descriptorSets.resize(shader->getNbShaders());
                    unsigned int descriptorId = shader->getId();
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
                } else if (shader == &sBuildAlphaBufferShader) {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = alphaBuffer.getDescriptorSet();
                    std::vector<VkDescriptorPool>& descriptorPool = alphaBuffer.getDescriptorPool();
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = alphaBuffer.getDescriptorSetLayout();
                    if (shader->getNbShaders() > descriptorSets.size())
                        descriptorSets.resize(shader->getNbShaders());
                    unsigned int descriptorId = shader->getId();
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
                } else if (shader == &buildShadowMapShader) {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = stencilBuffer.getDescriptorSet();
                    std::vector<VkDescriptorPool>& descriptorPool = stencilBuffer.getDescriptorPool();
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = stencilBuffer.getDescriptorSetLayout();
                    if (shader->getNbShaders() > descriptorSets.size())
                        descriptorSets.resize(shader->getNbShaders());
                    unsigned int descriptorId = shader->getId();
                    for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                        descriptorSets[i].resize(stencilBuffer.getMaxFramesInFlight());
                    }
                    std::vector<VkDescriptorSetLayout> layouts(stencilBuffer.getMaxFramesInFlight(), descriptorSetLayout[descriptorId]);
                    VkDescriptorSetAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    allocInfo.descriptorPool = descriptorPool[descriptorId];
                    allocInfo.descriptorSetCount = static_cast<uint32_t>(stencilBuffer.getMaxFramesInFlight());
                    allocInfo.pSetLayouts = layouts.data();
                    if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, descriptorSets[descriptorId].data()) != VK_SUCCESS) {
                        throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
                    }
                } else {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = shadowMap.getDescriptorSet();
                    std::vector<VkDescriptorPool>& descriptorPool = shadowMap.getDescriptorPool();
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = shadowMap.getDescriptorSetLayout();
                    if (shader->getNbShaders() > descriptorSets.size())
                        descriptorSets.resize(shader->getNbShaders());
                    unsigned int descriptorId = shader->getId();
                    for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                        descriptorSets[i].resize(shadowMap.getMaxFramesInFlight());
                    }
                    std::vector<VkDescriptorSetLayout> layouts(shadowMap.getMaxFramesInFlight(), descriptorSetLayout[descriptorId]);
                    VkDescriptorSetAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    allocInfo.descriptorPool = descriptorPool[descriptorId];
                    allocInfo.descriptorSetCount = static_cast<uint32_t>(shadowMap.getMaxFramesInFlight());
                    allocInfo.pSetLayouts = layouts.data();
                    if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, descriptorSets[descriptorId].data()) != VK_SUCCESS) {
                        throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
                    }
                }
             }
             void ShadowRenderComponent::allocateDescriptorSets (unsigned int p, RenderStates states) {
                 Shader* shader = const_cast<Shader*>(states.shader);
                if (shader == &depthGenShader) {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = depthBuffer.getDescriptorSet();
                    std::vector<VkDescriptorPool>& descriptorPool = depthBuffer.getDescriptorPool();
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = depthBuffer.getDescriptorSetLayout();
                    if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorSets.size())
                        descriptorSets.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                    unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                    for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                        descriptorSets[i].resize(depthBuffer.getMaxFramesInFlight());
                    }
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    std::vector<uint32_t> variableCounts(depthBuffer.getMaxFramesInFlight(), static_cast<uint32_t>(MAX_TEXTURES));

                    VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
                    variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
                    variableCountInfo.descriptorSetCount = static_cast<uint32_t>(variableCounts.size());;
                    variableCountInfo.pDescriptorCounts = variableCounts.data();
                    std::vector<VkDescriptorSetLayout> layouts(depthBuffer.getMaxFramesInFlight(), descriptorSetLayout[shader->getId()]);
                    VkDescriptorSetAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    allocInfo.pNext = &variableCountInfo;
                    allocInfo.descriptorPool = descriptorPool[descriptorId];
                    allocInfo.descriptorSetCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());
                    allocInfo.pSetLayouts = layouts.data();
                    if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, descriptorSets[descriptorId].data()) != VK_SUCCESS) {
                        throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
                    }
                } else if (shader == &sBuildAlphaBufferShader) {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = alphaBuffer.getDescriptorSet();
                    std::vector<VkDescriptorPool>& descriptorPool = alphaBuffer.getDescriptorPool();
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = alphaBuffer.getDescriptorSetLayout();
                    if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorSets.size())
                        descriptorSets.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                    unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                    for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                        descriptorSets[i].resize(alphaBuffer.getMaxFramesInFlight());
                    }
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    std::vector<uint32_t> variableCounts(alphaBuffer.getMaxFramesInFlight(), static_cast<uint32_t>(MAX_TEXTURES));

                    VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
                    variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
                    variableCountInfo.descriptorSetCount = static_cast<uint32_t>(variableCounts.size());;
                    variableCountInfo.pDescriptorCounts = variableCounts.data();
                    std::vector<VkDescriptorSetLayout> layouts(alphaBuffer.getMaxFramesInFlight(), descriptorSetLayout[shader->getId()]);
                    VkDescriptorSetAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    allocInfo.pNext = &variableCountInfo;
                    allocInfo.descriptorPool = descriptorPool[descriptorId];
                    allocInfo.descriptorSetCount = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight());
                    allocInfo.pSetLayouts = layouts.data();
                    if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, descriptorSets[descriptorId].data()) != VK_SUCCESS) {
                        throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
                    }
                } else if (shader == &buildShadowMapShader) {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = stencilBuffer.getDescriptorSet();
                    std::vector<VkDescriptorPool>& descriptorPool = stencilBuffer.getDescriptorPool();
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = stencilBuffer.getDescriptorSetLayout();
                    if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorSets.size())
                        descriptorSets.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                    unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                    for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                        descriptorSets[i].resize(stencilBuffer.getMaxFramesInFlight());
                    }
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    std::vector<uint32_t> variableCounts(stencilBuffer.getMaxFramesInFlight(), static_cast<uint32_t>(MAX_TEXTURES));

                    VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
                    variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
                    variableCountInfo.descriptorSetCount = static_cast<uint32_t>(variableCounts.size());;
                    variableCountInfo.pDescriptorCounts = variableCounts.data();
                    std::vector<VkDescriptorSetLayout> layouts(stencilBuffer.getMaxFramesInFlight(), descriptorSetLayout[shader->getId()]);
                    VkDescriptorSetAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    allocInfo.pNext = &variableCountInfo;
                    allocInfo.descriptorPool = descriptorPool[descriptorId];
                    allocInfo.descriptorSetCount = static_cast<uint32_t>(stencilBuffer.getMaxFramesInFlight());
                    allocInfo.pSetLayouts = layouts.data();
                    if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, descriptorSets[descriptorId].data()) != VK_SUCCESS) {
                        throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
                    }
                } else {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = shadowMap.getDescriptorSet();
                    std::vector<VkDescriptorPool>& descriptorPool = shadowMap.getDescriptorPool();
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = shadowMap.getDescriptorSetLayout();
                    if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorSets.size())
                        descriptorSets.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                    unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                    for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                        descriptorSets[i].resize(shadowMap.getMaxFramesInFlight());
                    }
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    std::vector<uint32_t> variableCounts(shadowMap.getMaxFramesInFlight(), static_cast<uint32_t>(MAX_TEXTURES));

                    VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
                    variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
                    variableCountInfo.descriptorSetCount = static_cast<uint32_t>(variableCounts.size());;
                    variableCountInfo.pDescriptorCounts = variableCounts.data();
                    std::vector<VkDescriptorSetLayout> layouts(shadowMap.getMaxFramesInFlight(), descriptorSetLayout[shader->getId()]);
                    VkDescriptorSetAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    allocInfo.pNext = &variableCountInfo;
                    allocInfo.descriptorPool = descriptorPool[descriptorId];
                    allocInfo.descriptorSetCount = static_cast<uint32_t>(shadowMap.getMaxFramesInFlight());
                    allocInfo.pSetLayouts = layouts.data();
                    if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, descriptorSets[descriptorId].data()) != VK_SUCCESS) {
                        throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
                    }
                }

             }
             void ShadowRenderComponent::updateDescriptorSets(unsigned int currentFrame, unsigned int p, RenderStates states) {
                Shader* shader = const_cast<Shader*>(states.shader);
                 if (shader == &depthGenShader) {
                        std::vector<std::vector<VkDescriptorSet>>& descriptorSets = depthBuffer.getDescriptorSet();
                        std::vector<Texture*> allTextures = Texture::getAllTextures();
                        unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();

                        std::array<VkWriteDescriptorSet, 4> descriptorWrites{};

                        VkDescriptorBufferInfo modelDataStorageBufferInfoLastFrame{};
                        modelDataStorageBufferInfoLastFrame.buffer = modelDataBufferMT[p][currentFrame];
                        modelDataStorageBufferInfoLastFrame.offset = 0;
                        modelDataStorageBufferInfoLastFrame.range = maxAlignedSizeModelData[p];

                        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[0].dstSet = descriptorSets[descriptorId][currentFrame];
                        descriptorWrites[0].dstBinding = 0;
                        descriptorWrites[0].dstArrayElement = 0;
                        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                        descriptorWrites[0].descriptorCount = 1;
                        descriptorWrites[0].pBufferInfo = &modelDataStorageBufferInfoLastFrame;

                        VkDescriptorBufferInfo materialDataStorageBufferInfoLastFrame{};
                        materialDataStorageBufferInfoLastFrame.buffer = materialDataBufferMT[p][currentFrame];
                        materialDataStorageBufferInfoLastFrame.offset = 0;
                        materialDataStorageBufferInfoLastFrame.range = maxAlignedSizeMaterialData[p];

                        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[1].dstSet = descriptorSets[descriptorId][currentFrame];
                        descriptorWrites[1].dstBinding = 1;
                        descriptorWrites[1].dstArrayElement = 0;
                        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                        descriptorWrites[1].descriptorCount = 1;
                        descriptorWrites[1].pBufferInfo = &materialDataStorageBufferInfoLastFrame;

                        VkDescriptorImageInfo headPtrDescriptorImageInfo;
                        headPtrDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                        headPtrDescriptorImageInfo.imageView = depthTextureImageView[currentFrame];
                        headPtrDescriptorImageInfo.sampler = depthTextureSampler[currentFrame];

                        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[2].dstSet = descriptorSets[descriptorId][currentFrame];
                        descriptorWrites[2].dstBinding = 2;
                        descriptorWrites[2].dstArrayElement = 0;
                        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                        descriptorWrites[2].descriptorCount = 1;
                        descriptorWrites[2].pImageInfo = &headPtrDescriptorImageInfo;



                        std::vector<VkDescriptorImageInfo>	descriptorImageInfos;
                        descriptorImageInfos.resize(allTextures.size());
                        for (unsigned int j = 0; j < allTextures.size(); j++) {
                            descriptorImageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                            descriptorImageInfos[j].imageView = allTextures[j]->getImageView();
                            descriptorImageInfos[j].sampler = allTextures[j]->getSampler();
                        }

                        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[3].dstSet = descriptorSets[descriptorId][currentFrame];
                        descriptorWrites[3].dstBinding = 3;
                        descriptorWrites[3].dstArrayElement = 0;
                        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[3].descriptorCount = allTextures.size();
                        descriptorWrites[3].pImageInfo = descriptorImageInfos.data();

                        vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

                } else if (shader == &sBuildAlphaBufferShader) {
                        std::vector<std::vector<VkDescriptorSet>>& descriptorSets = alphaBuffer.getDescriptorSet();
                        std::vector<Texture*> allTextures = Texture::getAllTextures();
                        unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                        std::vector<VkDescriptorImageInfo>	descriptorImageInfos;

                        std::array<VkWriteDescriptorSet, 7> descriptorWrites{};
                        VkDescriptorBufferInfo modelDataStorageBufferInfoLastFrame{};
                        modelDataStorageBufferInfoLastFrame.buffer = modelDataBufferMT[p][currentFrame];
                        modelDataStorageBufferInfoLastFrame.offset = 0;
                        modelDataStorageBufferInfoLastFrame.range = maxAlignedSizeModelData[p];

                        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[0].dstSet = descriptorSets[descriptorId][currentFrame];
                        descriptorWrites[0].dstBinding = 0;
                        descriptorWrites[0].dstArrayElement = 0;
                        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                        descriptorWrites[0].descriptorCount = 1;
                        descriptorWrites[0].pBufferInfo = &modelDataStorageBufferInfoLastFrame;

                        VkDescriptorBufferInfo materialDataStorageBufferInfoLastFrame{};
                        materialDataStorageBufferInfoLastFrame.buffer = materialDataBufferMT[p][currentFrame];
                        materialDataStorageBufferInfoLastFrame.offset = 0;
                        materialDataStorageBufferInfoLastFrame.range = maxAlignedSizeMaterialData[p];

                        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[1].dstSet = descriptorSets[descriptorId][currentFrame];
                        descriptorWrites[1].dstBinding = 1;
                        descriptorWrites[1].dstArrayElement = 0;
                        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                        descriptorWrites[1].descriptorCount = 1;
                        descriptorWrites[1].pBufferInfo = &materialDataStorageBufferInfoLastFrame;

                        VkDescriptorImageInfo headPtrDescriptorImageInfo;
                        headPtrDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                        headPtrDescriptorImageInfo.imageView = alphaTextureImageView[currentFrame];
                        headPtrDescriptorImageInfo.sampler = alphaTextureSampler[currentFrame];

                        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[2].dstSet = descriptorSets[descriptorId][currentFrame];
                        descriptorWrites[2].dstBinding = 2;
                        descriptorWrites[2].dstArrayElement = 0;
                        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                        descriptorWrites[2].descriptorCount = 1;
                        descriptorWrites[2].pImageInfo = &headPtrDescriptorImageInfo;

                        std::array<VkDescriptorImageInfo, RenderTexture::NB_SWAPCHAIN_IMAGES>	descriptorImageInfos2;
                        for (unsigned int j = 0; j < depthBuffer.getSwapchainImagesSize(); j++) {
                            descriptorImageInfos2[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                            descriptorImageInfos2[j].imageView = depthBuffer.getTexture(j).getImageView();
                            descriptorImageInfos2[j].sampler = depthBuffer.getTexture(j).getSampler();
                        }

                        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[3].dstSet = descriptorSets[descriptorId][currentFrame];
                        descriptorWrites[3].dstBinding = 3;
                        descriptorWrites[3].dstArrayElement = 0;
                        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[3].descriptorCount = descriptorImageInfos2.size();
                        descriptorWrites[3].pImageInfo = descriptorImageInfos2.data();

                        std::array<VkDescriptorImageInfo, RenderTexture::NB_SWAPCHAIN_IMAGES>	descriptorImageInfos3;
                        for (unsigned int j = 0; j < stencilBuffer.getSwapchainImagesSize(); j++) {
                            descriptorImageInfos3[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                            descriptorImageInfos3[j].imageView = stencilBuffer.getTexture(j).getImageView();
                            descriptorImageInfos3[j].sampler = stencilBuffer.getTexture(j).getSampler();
                        }

                        descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[4].dstSet = descriptorSets[descriptorId][currentFrame];
                        descriptorWrites[4].dstBinding = 4;
                        descriptorWrites[4].dstArrayElement = 0;
                        descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[4].descriptorCount = descriptorImageInfos3.size();
                        descriptorWrites[4].pImageInfo = descriptorImageInfos3.data();

                        VkDescriptorBufferInfo uboInfoLastFrame{};
                        uboInfoLastFrame.buffer = shadowUBO[currentFrame];
                        uboInfoLastFrame.offset = 0;
                        uboInfoLastFrame.range = sizeof(ShadowUBO);

                        descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[5].dstSet = descriptorSets[descriptorId][currentFrame];
                        descriptorWrites[5].dstBinding = 5;
                        descriptorWrites[5].dstArrayElement = 0;
                        descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                        descriptorWrites[5].descriptorCount = 1;
                        descriptorWrites[5].pBufferInfo = &uboInfoLastFrame;

                        descriptorImageInfos.resize(allTextures.size());
                        for (unsigned int j = 0; j < allTextures.size(); j++) {
                            descriptorImageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                            descriptorImageInfos[j].imageView = allTextures[j]->getImageView();
                            descriptorImageInfos[j].sampler = allTextures[j]->getSampler();
                        }

                        descriptorWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[6].dstSet = descriptorSets[descriptorId][currentFrame];
                        descriptorWrites[6].dstBinding = 6;
                        descriptorWrites[6].dstArrayElement = 0;
                        descriptorWrites[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[6].descriptorCount = allTextures.size();
                        descriptorWrites[6].pImageInfo = descriptorImageInfos.data();


                        vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

                } else if (shader == &buildShadowMapShader) {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = stencilBuffer.getDescriptorSet();
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                    std::array<VkWriteDescriptorSet, 4> descriptorWrites{};

                    VkDescriptorBufferInfo modelDataStorageBufferInfoLastFrame{};
                    modelDataStorageBufferInfoLastFrame.buffer = modelDataBufferMT[p][currentFrame];
                    modelDataStorageBufferInfoLastFrame.offset = 0;
                    modelDataStorageBufferInfoLastFrame.range = maxAlignedSizeModelData[p];

                    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[0].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[0].dstBinding = 0;
                    descriptorWrites[0].dstArrayElement = 0;
                    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    descriptorWrites[0].descriptorCount = 1;
                    descriptorWrites[0].pBufferInfo = &modelDataStorageBufferInfoLastFrame;

                    VkDescriptorBufferInfo materialDataStorageBufferInfoLastFrame{};
                    materialDataStorageBufferInfoLastFrame.buffer = materialDataBufferMT[p][currentFrame];
                    materialDataStorageBufferInfoLastFrame.offset = 0;
                    materialDataStorageBufferInfoLastFrame.range = maxAlignedSizeMaterialData[p];

                    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[1].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[1].dstBinding = 1;
                    descriptorWrites[1].dstArrayElement = 0;
                    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    descriptorWrites[1].descriptorCount = 1;
                    descriptorWrites[1].pBufferInfo = &materialDataStorageBufferInfoLastFrame;

                    VkDescriptorImageInfo headPtrDescriptorImageInfo;
                    headPtrDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                    headPtrDescriptorImageInfo.imageView = stencilTextureImageView[currentFrame];
                    headPtrDescriptorImageInfo.sampler = stencilTextureSampler[currentFrame];

                    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[2].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[2].dstBinding = 2;
                    descriptorWrites[2].dstArrayElement = 0;
                    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    descriptorWrites[2].descriptorCount = 1;
                    descriptorWrites[2].pImageInfo = &headPtrDescriptorImageInfo;



                    std::vector<VkDescriptorImageInfo>	descriptorImageInfos;
                    descriptorImageInfos.resize(allTextures.size());
                    for (unsigned int j = 0; j < allTextures.size(); j++) {
                        descriptorImageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        descriptorImageInfos[j].imageView = allTextures[j]->getImageView();
                        descriptorImageInfos[j].sampler = allTextures[j]->getSampler();
                    }

                    descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[3].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[3].dstBinding = 3;
                    descriptorWrites[3].dstArrayElement = 0;
                    descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrites[3].descriptorCount = allTextures.size();
                    descriptorWrites[3].pImageInfo = descriptorImageInfos.data();

                    vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

                } else {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = shadowMap.getDescriptorSet();
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();

                    std::vector<VkDescriptorImageInfo>	descriptorImageInfos;
                    std::array<VkWriteDescriptorSet, 7> descriptorWrites{};
                    VkDescriptorBufferInfo modelDataStorageBufferInfoLastFrame{};
                    modelDataStorageBufferInfoLastFrame.buffer = modelDataBufferMT[p][currentFrame];
                    modelDataStorageBufferInfoLastFrame.offset = 0;
                    modelDataStorageBufferInfoLastFrame.range = maxAlignedSizeModelData[p];

                    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[0].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[0].dstBinding = 0;
                    descriptorWrites[0].dstArrayElement = 0;
                    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    descriptorWrites[0].descriptorCount = 1;
                    descriptorWrites[0].pBufferInfo = &modelDataStorageBufferInfoLastFrame;

                    VkDescriptorBufferInfo materialDataStorageBufferInfoLastFrame{};
                    materialDataStorageBufferInfoLastFrame.buffer = materialDataBufferMT[p][currentFrame];
                    materialDataStorageBufferInfoLastFrame.offset = 0;
                    materialDataStorageBufferInfoLastFrame.range = maxAlignedSizeMaterialData[p];

                    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[1].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[1].dstBinding = 1;
                    descriptorWrites[1].dstArrayElement = 0;
                    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    descriptorWrites[1].descriptorCount = 1;
                    descriptorWrites[1].pBufferInfo = &materialDataStorageBufferInfoLastFrame;

                    std::array<VkDescriptorImageInfo, RenderTexture::NB_SWAPCHAIN_IMAGES>	descriptorImageInfos2;
                    for (unsigned int j = 0; j < depthBuffer.getSwapchainImagesSize(); j++) {
                        descriptorImageInfos2[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        descriptorImageInfos2[j].imageView = depthBuffer.getTexture(j).getImageView();
                        descriptorImageInfos2[j].sampler = depthBuffer.getTexture(j).getSampler();
                    }

                    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[2].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[2].dstBinding = 2;
                    descriptorWrites[2].dstArrayElement = 0;
                    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrites[2].descriptorCount = descriptorImageInfos2.size();
                    descriptorWrites[2].pImageInfo = descriptorImageInfos2.data();

                    std::array<VkDescriptorImageInfo, RenderTexture::NB_SWAPCHAIN_IMAGES>	descriptorImageInfos3;
                    for (unsigned int j = 0; j < alphaBuffer.getSwapchainImagesSize(); j++) {
                        descriptorImageInfos3[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        descriptorImageInfos3[j].imageView = alphaBuffer.getTexture(j).getImageView();
                        descriptorImageInfos3[j].sampler = alphaBuffer.getTexture(j).getSampler();
                    }

                    descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[3].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[3].dstBinding = 3;
                    descriptorWrites[3].dstArrayElement = 0;
                    descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrites[3].descriptorCount = descriptorImageInfos3.size();
                    descriptorWrites[3].pImageInfo = descriptorImageInfos3.data();

                    std::array<VkDescriptorImageInfo, RenderTexture::NB_SWAPCHAIN_IMAGES>	descriptorImageInfos4;
                    for (unsigned int j = 0; j < stencilBuffer.getSwapchainImagesSize(); j++) {
                        descriptorImageInfos4[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        descriptorImageInfos4[j].imageView = stencilBuffer.getTexture(j).getImageView();
                        descriptorImageInfos4[j].sampler = stencilBuffer.getTexture(j).getSampler();
                    }

                    descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[4].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[4].dstBinding = 4;
                    descriptorWrites[4].dstArrayElement = 0;
                    descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrites[4].descriptorCount = descriptorImageInfos4.size();
                    descriptorWrites[4].pImageInfo = descriptorImageInfos4.data();



                    VkDescriptorBufferInfo uboInfoLastFrame{};
                    uboInfoLastFrame.buffer = shadowUBO[currentFrame];
                    uboInfoLastFrame.offset = 0;
                    uboInfoLastFrame.range = sizeof(ShadowUBO);

                    descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[5].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[5].dstBinding = 5;
                    descriptorWrites[5].dstArrayElement = 0;
                    descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    descriptorWrites[5].descriptorCount = 1;
                    descriptorWrites[5].pBufferInfo = &uboInfoLastFrame;

                    descriptorImageInfos.resize(allTextures.size());
                    for (unsigned int j = 0; j < allTextures.size(); j++) {
                        descriptorImageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        descriptorImageInfos[j].imageView = allTextures[j]->getImageView();
                        descriptorImageInfos[j].sampler = allTextures[j]->getSampler();
                    }

                    descriptorWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[6].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[6].dstBinding = 6;
                    descriptorWrites[6].dstArrayElement = 0;
                    descriptorWrites[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrites[6].descriptorCount = allTextures.size();
                    descriptorWrites[6].pImageInfo = descriptorImageInfos.data();

                    vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
                }

             }
             void ShadowRenderComponent::createDescriptorSets(RenderStates states) {
                 Shader* shader = const_cast<Shader*>(states.shader);
                 if (shader == &depthGenShader) {
                       std::vector<std::vector<VkDescriptorSet>>& descriptorSets = depthBuffer.getDescriptorSet();
                       std::vector<Texture*> allTextures = Texture::getAllTextures();
                       unsigned int descriptorId =shader->getId();
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
                            headPtrDescriptorImageInfo.imageView = depthTextureImageView[i];
                            headPtrDescriptorImageInfo.sampler = depthTextureSampler[i];

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
                } else if (shader == &sBuildAlphaBufferShader) {
                        std::vector<std::vector<VkDescriptorSet>>& descriptorSets = alphaBuffer.getDescriptorSet();
                        std::vector<Texture*> allTextures = Texture::getAllTextures();
                        unsigned int descriptorId = shader->getId();
                        for (size_t i = 0; i < alphaBuffer.getMaxFramesInFlight(); i++) {
                            std::vector<VkDescriptorImageInfo>	descriptorImageInfos;
                            descriptorImageInfos.resize(allTextures.size());
                            for (unsigned int j = 0; j < allTextures.size(); j++) {
                                descriptorImageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                                descriptorImageInfos[j].imageView = allTextures[j]->getImageView();
                                descriptorImageInfos[j].sampler = allTextures[j]->getSampler();
                            }
                            std::array<VkWriteDescriptorSet, 7> descriptorWrites{};
                            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                            descriptorWrites[0].dstSet = descriptorSets[descriptorId][i];
                            descriptorWrites[0].dstBinding = 0;
                            descriptorWrites[0].dstArrayElement = 0;
                            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                            descriptorWrites[0].descriptorCount = allTextures.size();
                            descriptorWrites[0].pImageInfo = descriptorImageInfos.data();

                            VkDescriptorImageInfo headPtrDescriptorImageInfo;
                            headPtrDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                            headPtrDescriptorImageInfo.imageView = alphaTextureImageView[i];
                            headPtrDescriptorImageInfo.sampler = alphaTextureSampler[i];

                            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                            descriptorWrites[1].dstSet = descriptorSets[descriptorId][i];
                            descriptorWrites[1].dstBinding = 1;
                            descriptorWrites[1].dstArrayElement = 0;
                            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                            descriptorWrites[1].descriptorCount = 1;
                            descriptorWrites[1].pImageInfo = &headPtrDescriptorImageInfo;

                            VkDescriptorImageInfo	descriptorImageInfo;
                            descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                            descriptorImageInfo.imageView = depthBuffer.getTexture((depthBuffer.getImageIndex() + i)%depthBuffer.getSwapchainImagesSize()).getImageView();
                            descriptorImageInfo.sampler = depthBuffer.getTexture((depthBuffer.getImageIndex() + i)%depthBuffer.getSwapchainImagesSize()).getSampler();

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

                            VkDescriptorBufferInfo uboInfoLastFrame{};
                            uboInfoLastFrame.buffer = shadowUBO[i];
                            uboInfoLastFrame.offset = 0;
                            uboInfoLastFrame.range = sizeof(ShadowUBO);

                            descriptorWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                            descriptorWrites[6].dstSet = descriptorSets[descriptorId][i];
                            descriptorWrites[6].dstBinding = 6;
                            descriptorWrites[6].dstArrayElement = 0;
                            descriptorWrites[6].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                            descriptorWrites[6].descriptorCount = 1;
                            descriptorWrites[6].pBufferInfo = &uboInfoLastFrame;


                            vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
                    }
                } else if (shader == &buildShadowMapShader) {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = stencilBuffer.getDescriptorSet();
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    unsigned int descriptorId = shader->getId();
                    for (size_t i = 0; i < stencilBuffer.getMaxFramesInFlight(); i++) {
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
                        headPtrDescriptorImageInfo.imageView = stencilTextureImageView[i];
                        headPtrDescriptorImageInfo.sampler = stencilTextureSampler[i];

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
                } else {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = shadowMap.getDescriptorSet();
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    unsigned int descriptorId = shader->getId();
                    for (size_t i = 0; i < shadowMap.getMaxFramesInFlight(); i++) {
                        std::vector<VkDescriptorImageInfo>	descriptorImageInfos;
                        descriptorImageInfos.resize(allTextures.size());
                        for (unsigned int j = 0; j < allTextures.size(); j++) {
                            descriptorImageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                            descriptorImageInfos[j].imageView = allTextures[j]->getImageView();
                            descriptorImageInfos[j].sampler = allTextures[j]->getSampler();
                        }
                        std::array<VkWriteDescriptorSet, 7> descriptorWrites{};
                        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[0].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[0].dstBinding = 0;
                        descriptorWrites[0].dstArrayElement = 0;
                        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[0].descriptorCount = allTextures.size();
                        descriptorWrites[0].pImageInfo = descriptorImageInfos.data();

                        VkDescriptorImageInfo	descriptorImageInfo2;
                        descriptorImageInfo2.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        descriptorImageInfo2.imageView = depthBuffer.getTexture((depthBuffer.getImageIndex() + i)%depthBuffer.getSwapchainImagesSize()).getImageView();
                        descriptorImageInfo2.sampler = depthBuffer.getTexture((depthBuffer.getImageIndex() + i)%depthBuffer.getSwapchainImagesSize()).getSampler();

                        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[1].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[1].dstBinding = 1;
                        descriptorWrites[1].dstArrayElement = 0;
                        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[1].descriptorCount = 1;
                        descriptorWrites[1].pImageInfo = &descriptorImageInfo2;

                        VkDescriptorImageInfo	descriptorImageInfo;
                        descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        descriptorImageInfo.imageView = alphaBuffer.getTexture((alphaBuffer.getImageIndex() + i)%alphaBuffer.getSwapchainImagesSize()).getImageView();
                        descriptorImageInfo.sampler = alphaBuffer.getTexture((alphaBuffer.getImageIndex() + i)%alphaBuffer.getSwapchainImagesSize()).getSampler();

                        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[2].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[2].dstBinding = 2;
                        descriptorWrites[2].dstArrayElement = 0;
                        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[2].descriptorCount = 1;
                        descriptorWrites[2].pImageInfo = &descriptorImageInfo;

                        VkDescriptorImageInfo	descriptorImageInfo3;
                        descriptorImageInfo3.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        descriptorImageInfo3.imageView = stencilBuffer.getTexture((stencilBuffer.getImageIndex() + i)%stencilBuffer.getSwapchainImagesSize()).getImageView();
                        descriptorImageInfo3.sampler = stencilBuffer.getTexture((stencilBuffer.getImageIndex() + i)%stencilBuffer.getSwapchainImagesSize()).getSampler();

                        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[3].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[3].dstBinding = 3;
                        descriptorWrites[3].dstArrayElement = 0;
                        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[3].descriptorCount = 1;
                        descriptorWrites[3].pImageInfo = &descriptorImageInfo3;

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

                        VkDescriptorBufferInfo uboInfoLastFrame{};
                        uboInfoLastFrame.buffer = shadowUBO[i];
                        uboInfoLastFrame.offset = 0;
                        uboInfoLastFrame.range = sizeof(ShadowUBO);

                        descriptorWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[6].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[6].dstBinding = 6;
                        descriptorWrites[6].dstArrayElement = 0;
                        descriptorWrites[6].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                        descriptorWrites[6].descriptorCount = 1;
                        descriptorWrites[6].pBufferInfo = &uboInfoLastFrame;

                        vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
                    }
                }
             }
             void ShadowRenderComponent::createImageView() {
                for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    VkImageViewCreateInfo viewInfo{};
                    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                    viewInfo.image = depthTextureImage[i];
                    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                    viewInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    viewInfo.subresourceRange.baseMipLevel = 0;
                    viewInfo.subresourceRange.levelCount = 1;
                    viewInfo.subresourceRange.baseArrayLayer = 0;
                    viewInfo.subresourceRange.layerCount = 1;

                    if (vkCreateImageView(vkDevice.getDevice(), &viewInfo, nullptr, &depthTextureImageView[i]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create head ptr texture image view!");
                    }
                }
                for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    VkImageViewCreateInfo viewInfo2{};
                    viewInfo2.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                    viewInfo2.image = alphaTextureImage[i];
                    viewInfo2.viewType = VK_IMAGE_VIEW_TYPE_2D;
                    viewInfo2.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                    viewInfo2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    viewInfo2.subresourceRange.baseMipLevel = 0;
                    viewInfo2.subresourceRange.levelCount = 1;
                    viewInfo2.subresourceRange.baseArrayLayer = 0;
                    viewInfo2.subresourceRange.layerCount = 1;

                    if (vkCreateImageView(vkDevice.getDevice(), &viewInfo2, nullptr, &alphaTextureImageView[i]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create head ptr texture image view!");
                    }
                }
                for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    VkImageViewCreateInfo viewInfo3{};
                    viewInfo3.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                    viewInfo3.image = stencilTextureImage[i];
                    viewInfo3.viewType = VK_IMAGE_VIEW_TYPE_2D;
                    viewInfo3.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                    viewInfo3.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    viewInfo3.subresourceRange.baseMipLevel = 0;
                    viewInfo3.subresourceRange.levelCount = 1;
                    viewInfo3.subresourceRange.baseArrayLayer = 0;
                    viewInfo3.subresourceRange.layerCount = 1;

                    if (vkCreateImageView(vkDevice.getDevice(), &viewInfo3, nullptr, &stencilTextureImageView[i]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create head ptr texture image view!");
                    }
                }
            }
            void ShadowRenderComponent::createSampler() {
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
                for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    if (vkCreateSampler(vkDevice.getDevice(), &samplerInfo, nullptr, &depthTextureSampler[i]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create texture sampler!");
                    }
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
                for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    if (vkCreateSampler(vkDevice.getDevice(), &samplerInfo2, nullptr, &alphaTextureSampler[i]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create texture sampler!");
                    }
                }
                VkSamplerCreateInfo samplerInfo3{};
                samplerInfo3.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                samplerInfo3.magFilter = VK_FILTER_LINEAR;
                samplerInfo3.minFilter = VK_FILTER_LINEAR;
                samplerInfo3.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo3.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo3.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo3.anisotropyEnable = VK_TRUE;
                vkGetPhysicalDeviceProperties(vkDevice.getPhysicalDevice(), &properties);
                samplerInfo3.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
                samplerInfo3.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
                samplerInfo3.unnormalizedCoordinates = VK_FALSE;
                samplerInfo3.compareEnable = VK_FALSE;
                samplerInfo3.compareOp = VK_COMPARE_OP_ALWAYS;
                samplerInfo3.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
                samplerInfo3.mipLodBias = 0.0f;
                samplerInfo3.minLod = 0.0f;
                samplerInfo3.maxLod = 0.0f;
                for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    if (vkCreateSampler(vkDevice.getDevice(), &samplerInfo3, nullptr, &stencilTextureSampler[i]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create texture sampler!");
                    }
                }
            }
            uint32_t ShadowRenderComponent::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
                VkPhysicalDeviceMemoryProperties memProperties;
                vkGetPhysicalDeviceMemoryProperties(vkDevice.getPhysicalDevice(), &memProperties);
                for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
                    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                        return i;
                    }
                }
                throw std::runtime_error("aucun type de memoire ne satisfait le buffer!");
            }
            void ShadowRenderComponent::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandBuffer cmd) {
                //std::cout<<"opy buffers"<<std::endl;
                if (srcBuffer != nullptr && dstBuffer != nullptr) {
                    VkBufferCopy copyRegion{};
                    copyRegion.size = size;
                    vkCmdCopyBuffer(cmd, srcBuffer, dstBuffer, 1, &copyRegion);
                }
            }
            void ShadowRenderComponent::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
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
            void ShadowRenderComponent::drawInstanced() {
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
                        {
                            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                            material.textureIndex = (m_normals[i].getMaterial().getTexture() != nullptr) ? m_normals[i].getMaterial().getTexture()->getId() : 0;
                            material.layer = m_normals[i].getMaterial().getLayer();
                            material.uvScale = (m_normals[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_normals[i].getMaterial().getTexture()->getSize().x(), 1.f / m_normals[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                            material.uvOffset = math::Vec2f(0, 0);
                        }

                        materialDatas[p].push_back(material);
                        TransformMatrix tm;
                        ModelData model;
                        model.worldMat = tm.getMatrix()/*.transpose()*/;
                        model.shadowProjMat = tm.getMatrix()/*.transpose()*/;
                        modelDatas[p].push_back(model);
                        unsigned int vertexCount = 0;
                        for (unsigned int j = 0; j < m_normals[i].getAllVertices().getVertexCount(); j++) {
                            vbBindlessTex[p].append(m_normals[i].getAllVertices()[j]);
                            vertexCount++;
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
                        material.uvScale = (m_instances[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_instances[i].getMaterial().getTexture()->getSize().x(), 1.f / m_instances[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        materialDatas[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_instances[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            ModelData model;
                            model.worldMat = tm[j]->getMatrix()/*.transpose()*/;
                            TransformMatrix tm;
                            model.shadowProjMat = tm.getMatrix()/*.transpose()*/;
                            modelDatas[p].push_back(model);
                        }
                        unsigned int vertexCount = 0;
                        if (m_instances[i].getVertexArrays().size() > 0) {
                            Entity* entity = m_instances[i].getVertexArrays()[0]->getEntity();
                            for (unsigned int j = 0; j < m_instances[i].getVertexArrays().size(); j++) {
                                if (entity == m_instances[i].getVertexArrays()[j]->getEntity()) {
                                    unsigned int p = m_instances[i].getVertexArrays()[j]->getPrimitiveType();
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
                currentStates.shader = &buildShadowMapShader;
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
                        createCommandBuffersIndirect(p, drawArraysIndirectCommands[p].size(), sizeof(DrawArraysIndirectCommand), SHADOWNODEPTHNOSTENCIL, currentStates);
                    }
                }
                currentStates.shader = &depthGenShader;
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
                        createCommandBuffersIndirect(p, drawArraysIndirectCommands[p].size(), sizeof(DrawArraysIndirectCommand), SHADOWNODEPTHNOSTENCIL, currentStates);
                    }
                }

                for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                    vbBindlessTex[i].clear();
                    materialDatas[i].clear();
                    modelDatas[i].clear();
                }
                for (unsigned int i = 0; i < firstIndex.size(); i++) {
                    firstIndex[i] = 0;
                }
                for (unsigned int i = 0; i < baseInstance.size(); i++) {
                    baseInstance[i] = 0;
                }

                for (unsigned int i = 0; i < drawArraysIndirectCommands.size(); i++) {
                    drawArraysIndirectCommands[i].clear();
                }
                for (unsigned int i = 0; i < m_shadow_normals.size(); i++) {
                    if (m_shadow_normals[i].getAllVertices().getVertexCount() > 0) {

                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        unsigned int p = m_shadow_normals[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_shadow_normals[i].getMaterial().getTexture() != nullptr) ? m_shadow_normals[i].getMaterial().getTexture()->getId() : 0;
                        material.layer = m_shadow_normals[i].getMaterial().getLayer();
                        material.uvScale = (m_shadow_normals[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_shadow_normals[i].getMaterial().getTexture()->getSize().x(), 1.f / m_shadow_normals[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        materialDatas[p].push_back(material);
                        TransformMatrix tm;
                        ModelData model;
                        model.worldMat = tm.getMatrix()/*.transpose()*/;
                        model.shadowProjMat = tm.getMatrix()/*.transpose()*/;
                        modelDatas[p].push_back(model);
                        unsigned int vertexCount = 0;
                        for (unsigned int j = 0; j < m_shadow_normals[i].getAllVertices().getVertexCount(); j++) {
                            vertexCount++;
                            vbBindlessTex[p].append(m_shadow_normals[i].getAllVertices()[j]);
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
                for (unsigned int i = 0; i < m_shadow_instances.size(); i++) {
                    if (m_shadow_instances[i].getAllVertices().getVertexCount() > 0) {
                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        unsigned int p = m_shadow_instances[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_shadow_instances[i].getMaterial().getTexture() != nullptr) ? m_shadow_instances[i].getMaterial().getTexture()->getId() : 0;
                        material.layer = m_shadow_instances[i].getMaterial().getLayer();
                        material.uvScale = (m_shadow_instances[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_shadow_instances[i].getMaterial().getTexture()->getSize().x(), 1.f / m_shadow_instances[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        materialDatas[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_shadow_instances[i].getTransforms();
                        std::vector<TransformMatrix> tm2 = m_shadow_instances[i].getShadowProjMatrix();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            tm2[j].update();
                            ModelData model;
                            model.worldMat = tm[j]->getMatrix()/*.transpose()*/;
                            model.shadowProjMat = tm2[j].getMatrix()/*.transpose()*/;
                            modelDatas[p].push_back(model);
                        }
                        unsigned int vertexCount=0;
                        if (m_shadow_instances[i].getVertexArrays().size() > 0) {
                            Entity* entity = m_shadow_instances[i].getVertexArrays()[0]->getEntity();
                            for (unsigned int j = 0; j < m_shadow_instances[i].getVertexArrays().size(); j++) {
                                if (entity == m_shadow_instances[i].getVertexArrays()[j]->getEntity()) {
                                    unsigned int p = m_shadow_instances[i].getVertexArrays()[j]->getPrimitiveType();
                                    for (unsigned int k = 0; k < m_shadow_instances[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                        vertexCount++;
                                        vbBindlessTex[p].append((*m_shadow_instances[i].getVertexArrays()[j])[k]);
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
                currentStates.shader = &sBuildAlphaBufferShader;
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
                        createCommandBuffersIndirect(p, drawArraysIndirectCommands[p].size(), sizeof(DrawArraysIndirectCommand), SHADOWNODEPTHNOSTENCIL, currentStates);
                    }
                }
                currentStates.shader = &perPixShadowShader;
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
                        createCommandBuffersIndirect(p, drawArraysIndirectCommands[p].size(), sizeof(DrawArraysIndirectCommand), SHADOWNODEPTHNOSTENCIL, currentStates);
                    }
                }
            }
            void ShadowRenderComponent::drawInstancedIndexed() {
                for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                    modelDatas[i].clear();
                    materialDatas[i].clear();
                    vbBindlessTex[i].clear();
                }
                std::array<std::vector<DrawElementsIndirectCommand>, Batcher::nbPrimitiveTypes> drawElementsIndirectCommands;
                std::array<unsigned int, Batcher::nbPrimitiveTypes> firstIndex, baseInstance, baseVertex;
                for (unsigned int i = 0; i < firstIndex.size(); i++) {
                    firstIndex[i] = 0;
                }
                for (unsigned int i = 0; i < baseVertex.size(); i++) {
                    baseVertex[i] = 0;
                }
                for (unsigned int i = 0; i < baseInstance.size(); i++) {
                    baseInstance[i] = 0;
                }
                for (unsigned int i = 0; i < m_normalsIndexed.size(); i++) {
                   if (m_normalsIndexed[i].getAllVertices().getVertexCount() > 0) {

                        DrawElementsIndirectCommand drawElementsIndirectCommand;
                        unsigned int p = m_normalsIndexed[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_normalsIndexed[i].getMaterial().getTexture() != nullptr) ? m_normalsIndexed[i].getMaterial().getTexture()->getId() : 0;
                        material.layer = m_normalsIndexed[i].getMaterial().getLayer();
                        material.uvScale = (m_normalsIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_normalsIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_normalsIndexed[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        materialDatas[p].push_back(material);
                        TransformMatrix tm;
                        ModelData model;
                        model.worldMat = tm.getMatrix().transpose();
                        model.shadowProjMat = tm.getMatrix().transpose();
                        modelDatas[p].push_back(model);
                        unsigned int indexCount = 0, vertexCount = 0;
                        for (unsigned int j = 0; j < m_normalsIndexed[i].getAllVertices().getVertexCount(); j++) {
                            vertexCount++;
                            vbBindlessTex[p].append(m_normalsIndexed[i].getAllVertices()[j]);
                        }
                        for (unsigned int j = 0; j < m_normalsIndexed[i].getAllVertices().getIndexes().size(); j++) {
                            ////std::cout<<"add  norm index"<<std::endl;
                            indexCount++;
                            vbBindlessTex[p].addIndex(m_normalsIndexed[i].getAllVertices().getIndexes()[j]);
                        }
                        drawElementsIndirectCommand.index_count = indexCount;
                        drawElementsIndirectCommand.first_index = firstIndex[p];
                        drawElementsIndirectCommand.instance_base = baseInstance[p];
                        drawElementsIndirectCommand.vertex_base = baseVertex[p];
                        drawElementsIndirectCommand.instance_count = 1;
                        drawElementsIndirectCommands[p].push_back(drawElementsIndirectCommand);
                        firstIndex[p] += indexCount;
                        baseVertex[p] += vertexCount;
                        baseInstance[p] += 1;
                    }
                }
                for (unsigned int i = 0; i < m_instancesIndexed.size(); i++) {
                    if (m_instancesIndexed[i].getAllVertices().getVertexCount() > 0) {
                        DrawElementsIndirectCommand drawElementsIndirectCommand;
                        unsigned int p = m_instancesIndexed[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_instancesIndexed[i].getMaterial().getTexture() != nullptr) ? m_instancesIndexed[i].getMaterial().getTexture()->getId() : 0;
                        material.uvScale = (m_instancesIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_instancesIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_instancesIndexed[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        material.layer = m_instancesIndexed[i].getMaterial().getLayer();
                        materialDatas[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_instancesIndexed[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            ModelData model;
                            model.worldMat = tm[j]->getMatrix().transpose();
                            TransformMatrix tm;
                            model.shadowProjMat = tm.getMatrix().transpose();
                            modelDatas[p].push_back(model);
                        }
                        unsigned int vertexCount = 0, indexCount = 0;
                        if (m_instancesIndexed[i].getVertexArrays().size() > 0) {
                            Entity* entity = m_instancesIndexed[i].getVertexArrays()[0]->getEntity();
                            for (unsigned int j = 0; j < m_instancesIndexed[i].getVertexArrays().size(); j++) {
                                if (entity == m_instancesIndexed[i].getVertexArrays()[j]->getEntity()) {
                                    unsigned int p = m_instancesIndexed[i].getVertexArrays()[j]->getPrimitiveType();
                                    for (unsigned int k = 0; k < m_instancesIndexed[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                        vertexCount++;
                                        vbBindlessTex[p].append((*m_instancesIndexed[i].getVertexArrays()[j])[k]);
                                    }
                                    for (unsigned int k = 0; k < m_instancesIndexed[i].getVertexArrays()[j]->getIndexes().size(); k++) {
                                        indexCount++;
                                        vbBindlessTex[p].addIndex(m_instancesIndexed[i].getVertexArrays()[j]->getIndexes()[k]);
                                    }
                                }
                            }
                        }
                        drawElementsIndirectCommand.index_count = indexCount;
                        drawElementsIndirectCommand.first_index = firstIndex[p];
                        drawElementsIndirectCommand.instance_base = baseInstance[p];
                        drawElementsIndirectCommand.vertex_base = baseVertex[p];
                        drawElementsIndirectCommand.instance_count = tm.size();
                        drawElementsIndirectCommands[p].push_back(drawElementsIndirectCommand);
                        firstIndex[p] += indexCount;
                        baseVertex[p] += vertexCount;
                        baseInstance[p] += tm.size();
                    }
                }
                RenderStates currentStates;
                currentStates.blendMode = BlendNone;
                currentStates.shader = &buildShadowMapShader;
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
                        bufferSize = sizeof(DrawElementsIndirectCommand) * drawElementsIndirectCommands[p].size();

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
                        memcpy(data, drawElementsIndirectCommands[p].data(), (size_t)bufferSize);
                        vkUnmapMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemory);
                        copyBuffer(vboIndirectStagingBuffer, vboIndirect, bufferSize);
                        //createDescriptorSets(p, currentStates);
                        createCommandBuffersIndirect(p, drawElementsIndirectCommands[p].size(), sizeof(DrawElementsIndirectCommand), SHADOWNODEPTHNOSTENCIL, currentStates);
                    }
                }
                currentStates.shader = &depthGenShader;
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
                        bufferSize = sizeof(DrawElementsIndirectCommand) * drawElementsIndirectCommands[p].size();

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
                        memcpy(data, drawElementsIndirectCommands[p].data(), (size_t)bufferSize);
                        vkUnmapMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemory);
                        copyBuffer(vboIndirectStagingBuffer, vboIndirect, bufferSize);
                        //createDescriptorSets(p, currentStates);
                        createCommandBuffersIndirect(p, drawElementsIndirectCommands[p].size(), sizeof(DrawElementsIndirectCommand), SHADOWNODEPTHNOSTENCIL, currentStates);
                    }
                }
                for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                    vbBindlessTex[i].clear();
                    materialDatas[i].clear();
                    modelDatas[i].clear();
                }
                for (unsigned int i = 0; i < firstIndex.size(); i++) {
                    firstIndex[i] = 0;
                }
                for (unsigned int i = 0; i < baseVertex.size(); i++) {
                    baseVertex[i] = 0;
                }
                for (unsigned int i = 0; i < baseInstance.size(); i++) {
                    baseInstance[i] = 0;
                }

                for (unsigned int i = 0; i < drawElementsIndirectCommands.size(); i++) {
                    drawElementsIndirectCommands[i].clear();
                }
                for (unsigned int i = 0; i < m_shadow_normalsIndexed.size(); i++) {
                    if (m_shadow_normalsIndexed[i].getAllVertices().getVertexCount() > 0) {
                        DrawElementsIndirectCommand drawElementsIndirectCommand;
                        unsigned int p = m_shadow_normalsIndexed[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_shadow_normalsIndexed[i].getMaterial().getTexture() != nullptr) ? m_shadow_normalsIndexed[i].getMaterial().getTexture()->getId() : 0;
                        material.layer = m_shadow_normalsIndexed[i].getMaterial().getLayer();
                        material.uvScale = (m_shadow_normalsIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_shadow_normalsIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_shadow_normalsIndexed[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        materialDatas[p].push_back(material);
                        TransformMatrix tm;
                        ModelData model;
                        model.worldMat = tm.getMatrix().transpose();
                        model.shadowProjMat = tm.getMatrix().transpose();
                        modelDatas[p].push_back(model);
                        unsigned int indexCount = 0, vertexCount = 0;
                        for (unsigned int j = 0; j < m_shadow_normalsIndexed[i].getAllVertices().getVertexCount(); j++) {
                            ////std::cout<<"add shadow norm vert"<<std::endl;
                            vertexCount++;
                            vbBindlessTex[p].append(m_shadow_normalsIndexed[i].getAllVertices()[j]);
                        }
                        for (unsigned int j = 0; j < m_shadow_normalsIndexed[i].getAllVertices().getIndexes().size(); j++) {
                            ////std::cout<<"add shadow norm index"<<std::endl;
                            indexCount++;
                            vbBindlessTex[p].addIndex(m_shadow_normalsIndexed[i].getAllVertices().getIndexes()[j]);
                        }
                        drawElementsIndirectCommand.index_count = indexCount;
                        drawElementsIndirectCommand.first_index = firstIndex[p];
                        drawElementsIndirectCommand.instance_base = baseInstance[p];
                        drawElementsIndirectCommand.vertex_base = baseVertex[p];
                        drawElementsIndirectCommand.instance_count = 1;
                        drawElementsIndirectCommands[p].push_back(drawElementsIndirectCommand);
                        firstIndex[p] += indexCount;
                        baseVertex[p] += vertexCount;
                        baseInstance[p] += 1;
                    }
                }
                for (unsigned int i = 0; i < m_shadow_instances_indexed.size(); i++) {
                    DrawElementsIndirectCommand drawElementsIndirectCommand;
                    if (m_shadow_instances_indexed[i].getAllVertices().getVertexCount() > 0) {
                        unsigned int p = m_shadow_instances_indexed[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_shadow_instances_indexed[i].getMaterial().getTexture() != nullptr) ? m_shadow_instances_indexed[i].getMaterial().getTexture()->getId() : 0;
                        material.layer = m_shadow_instances_indexed[i].getMaterial().getLayer();
                        material.uvScale = (m_shadow_instances_indexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_shadow_instances[i].getMaterial().getTexture()->getSize().x(), 1.f / m_shadow_instances_indexed[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        materialDatas[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_shadow_instances_indexed[i].getTransforms();
                        std::vector<TransformMatrix> tm2 = m_shadow_instances_indexed[i].getShadowProjMatrix();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            tm2[j].update();
                            ModelData model;
                            model.worldMat = tm[j]->getMatrix().transpose();
                            model.shadowProjMat = tm2[j].getMatrix().transpose();
                            modelDatas[p].push_back(model);
                        }
                        unsigned int vertexCount = 0, indexCount = 0;
                        if (m_shadow_instances_indexed[i].getVertexArrays().size() > 0) {
                            Entity* entity = m_shadow_instances_indexed[i].getVertexArrays()[0]->getEntity();
                            for (unsigned int j = 0; j < m_shadow_instances_indexed[i].getVertexArrays().size(); j++) {
                                if (entity == m_shadow_instances_indexed[i].getVertexArrays()[j]->getEntity()) {
                                    unsigned int p = m_shadow_instances_indexed[i].getVertexArrays()[j]->getPrimitiveType();
                                    for (unsigned int k = 0; k < m_shadow_instances_indexed[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                        vertexCount++;
                                        vbBindlessTex[p].append((*m_shadow_instances_indexed[i].getVertexArrays()[j])[k]);
                                    }
                                    for (unsigned int k = 0; k < m_shadow_instances_indexed[i].getVertexArrays()[j]->getIndexes().size(); k++) {
                                        //std::cout<<"add shadow instance index"<<std::endl;
                                        indexCount++;
                                        vbBindlessTex[p].addIndex(m_shadow_instances_indexed[i].getVertexArrays()[j]->getIndexes()[k]);
                                    }
                                }
                            }
                        }
                        drawElementsIndirectCommand.index_count = indexCount;
                        drawElementsIndirectCommand.first_index = firstIndex[p];
                        drawElementsIndirectCommand.instance_base = baseInstance[p];
                        drawElementsIndirectCommand.vertex_base = baseVertex[p];
                        drawElementsIndirectCommand.instance_count = tm.size();
                        drawElementsIndirectCommands[p].push_back(drawElementsIndirectCommand);
                        firstIndex[p] += indexCount;
                        baseVertex[p] += vertexCount;
                        baseInstance[p] += tm.size();
                    }
                }
                currentStates.shader = &sBuildAlphaBufferShader;
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
                        bufferSize = sizeof(DrawElementsIndirectCommand) * drawElementsIndirectCommands[p].size();

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
                        memcpy(data, drawElementsIndirectCommands[p].data(), (size_t)bufferSize);
                        vkUnmapMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemory);
                        copyBuffer(vboIndirectStagingBuffer, vboIndirect, bufferSize);
                        //createDescriptorSets(p, currentStates);
                        createCommandBuffersIndirect(p, drawElementsIndirectCommands[p].size(), sizeof(DrawElementsIndirectCommand), SHADOWNODEPTHNOSTENCIL, currentStates);
                    }
                }
                currentStates.shader = &perPixShadowShader;
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
                        bufferSize = sizeof(DrawElementsIndirectCommand) * drawElementsIndirectCommands[p].size();

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
                        memcpy(data, drawElementsIndirectCommands[p].data(), (size_t)bufferSize);
                        vkUnmapMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemory);
                        copyBuffer(vboIndirectStagingBuffer, vboIndirect, bufferSize);
                        //createDescriptorSets(p, currentStates);
                        createCommandBuffersIndirect(p, drawElementsIndirectCommands[p].size(), sizeof(DrawElementsIndirectCommand), SHADOWNODEPTHNOSTENCIL, currentStates);
                    }
                }
            }
            void ShadowRenderComponent::recordCommandBufferIndirect(unsigned int currentFrame, unsigned int p, unsigned int nbIndirectCommands, unsigned int stride, ShadowDepthStencilID depthStencilID, unsigned int vertexOffset, unsigned int indexOffset, unsigned int uboOffset, unsigned int modelDataOffset, unsigned int materialDataOffset, unsigned int drawCommandOffset, RenderStates currentStates, VkCommandBuffer commandBuffer) {
                currentStates.blendMode.updateIds();
                Shader* shader = const_cast<Shader*>(currentStates.shader);
                if (shader == &depthGenShader) {
                    layerPC.nbLayers = GameObject::getNbLayers();
                    vkCmdPushConstants(commandBuffer, depthBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(IndirectRenderingPC), &indirectRenderingPC);
                    vkCmdPushConstants(commandBuffer, depthBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_FRAGMENT_BIT, 128, sizeof(LayerPC), &layerPC);
                    std::vector<unsigned int> dynamicBufferOffsets;
                    dynamicBufferOffsets.push_back(modelDataOffset);
                    dynamicBufferOffsets.push_back(materialDataOffset);
                    if (indexOffset == -1)
                        depthBuffer.drawIndirect(commandBuffer, currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], drawCommandBufferMT[p][currentFrame], depthStencilID,currentStates, p, vertexOffset, drawCommandOffset, dynamicBufferOffsets);
                    else
                        depthBuffer.drawIndirect(commandBuffer, currentFrame, nbIndirectCommands, stride, vbBindlessTexIndexed[p], drawCommandBufferIndexedMT[p][currentFrame], depthStencilID,currentStates, p, vertexOffset, drawCommandOffset, dynamicBufferOffsets, indexOffset);
                } else if (shader == &sBuildAlphaBufferShader) {
                    layerPC.nbLayers = GameObject::getNbLayers();
                    /*vkCmdResetEvent(commandBuffers[currentFrame], events[currentFrame],  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                    vkCmdSetEvent(commandBuffers[currentFrame], events[currentFrame],  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);*/
                    //vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);

                    //std::cout<<"record ids : "<<(sBuildAlphaBufferShader.getId() * (Batcher::nbPrimitiveTypes - 1) + p)<<","<<(SHADOWNODEPTHNOSTENCIL*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id)<<std::endl;

                    vkCmdPushConstants(commandBuffer, alphaBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(LayerPC), &layerPC);



                    std::vector<unsigned int> dynamicBufferOffsets;
                    dynamicBufferOffsets.push_back(modelDataOffset);
                    dynamicBufferOffsets.push_back(materialDataOffset);
                    if (indexOffset == -1)
                        alphaBuffer.drawIndirect(commandBuffer, currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], drawCommandBufferMT[p][currentFrame], depthStencilID,currentStates, p, vertexOffset, drawCommandOffset, dynamicBufferOffsets);
                    else
                        alphaBuffer.drawIndirect(commandBuffer, currentFrame, nbIndirectCommands, stride, vbBindlessTexIndexed[p], drawCommandBufferIndexedMT[p][currentFrame], depthStencilID,currentStates, p, vertexOffset, drawCommandOffset, dynamicBufferOffsets, indexOffset);

                } else if (shader == &buildShadowMapShader) {


                    //vkCmdWaitEvents(commandBuffers[currentFrame], 1, &events[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                    vkCmdPushConstants(commandBuffer, stencilBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(LightIndirectRenderingPC), &lightIndirectRenderingPC);
                    vkCmdPushConstants(commandBuffer, stencilBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_FRAGMENT_BIT, 128, sizeof(LayerPC), &layerPC);
                    std::vector<unsigned int> dynamicBufferOffsets;
                    dynamicBufferOffsets.push_back(modelDataOffset);
                    dynamicBufferOffsets.push_back(materialDataOffset);
                    if (indexOffset == -1)
                        stencilBuffer.drawIndirect(commandBuffer, currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], drawCommandBufferMT[p][currentFrame], depthStencilID,currentStates, p, vertexOffset, drawCommandOffset, dynamicBufferOffsets);
                    else
                        stencilBuffer.drawIndirect(commandBuffer, currentFrame, nbIndirectCommands, stride, vbBindlessTexIndexed[p], drawCommandBufferIndexedMT[p][currentFrame], depthStencilID,currentStates, p, vertexOffset, drawCommandOffset, dynamicBufferOffsets, indexOffset);

                } else {
                    vkCmdPushConstants(commandBuffer, shadowMap.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ResolutionPC), &resolutionPC);
                    std::vector<unsigned int> dynamicBufferOffsets;
                    dynamicBufferOffsets.push_back(modelDataOffset);
                    dynamicBufferOffsets.push_back(materialDataOffset);
                    if (indexOffset == -1)
                        shadowMap.drawIndirect(commandBuffer, currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], drawCommandBufferMT[p][currentFrame], depthStencilID,currentStates, p, vertexOffset, drawCommandOffset, dynamicBufferOffsets);
                    else
                        shadowMap.drawIndirect(commandBuffer, currentFrame, nbIndirectCommands, stride, vbBindlessTexIndexed[p], drawCommandBufferIndexedMT[p][currentFrame], depthStencilID,currentStates, p, vertexOffset, drawCommandOffset, dynamicBufferOffsets, indexOffset);

                }
                isSomethingDrawn = true;
            }
            void ShadowRenderComponent::createCommandBuffersIndirect(unsigned int p, unsigned int nbIndirectCommands, unsigned int stride, ShadowDepthStencilID depthStencilID, RenderStates currentStates) {
                if (needToUpdateDS) {
                    Shader* shader = const_cast<Shader*>(currentStates.shader);
                    currentStates.shader = &buildShadowMapShader;
                    createDescriptorSets(currentStates);
                    currentStates.shader = &depthGenShader;
                    createDescriptorSets(currentStates);
                    currentStates.shader = &sBuildAlphaBufferShader;
                    createDescriptorSets(currentStates);
                    currentStates.shader = &perPixShadowShader;
                    createDescriptorSets(currentStates);
                    currentStates.shader = shader;
                    needToUpdateDS = false;
                }
                currentStates.blendMode.updateIds();
                Shader* shader = const_cast<Shader*>(currentStates.shader);
                if (shader == &depthGenShader) {
                    depthBuffer.beginRecordCommandBuffers();
                    ////////std::cout<<"draw on db"<<std::endl;
                    std::vector<VkCommandBuffer> commandBuffers = depthBuffer.getCommandBuffers();
                    unsigned int currentFrame = depthBuffer.getCurrentFrame();
                    layerPC.nbLayers = GameObject::getNbLayers();
                    vkCmdPushConstants(commandBuffers[currentFrame], depthBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(IndirectRenderingPC), &indirectRenderingPC);
                    vkCmdPushConstants(commandBuffers[currentFrame], depthBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_FRAGMENT_BIT, 128, sizeof(LayerPC), &layerPC);
                    depthBuffer.beginRenderPass();
                    depthBuffer.drawIndirect(commandBuffers[currentFrame], currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], vboIndirect, depthStencilID,currentStates);
                    depthBuffer.endRenderPass();
                    /*std::vector<VkSemaphore> signalSemaphores, waitSemaphores;
                    std::vector<VkPipelineStageFlags> waitStages;
                    std::vector<uint64_t> signalValues, waitValues;
                    waitSemaphores.push_back(offscreenFinishedSemaphore[shadowMap.getCurrentFrame()]);
                    waitStages.push_back(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                    waitValues.push_back(values[shadowMap.getCurrentFrame()]);
                    signalSemaphores.push_back(offscreenDepthFinishedSemaphore[shadowMap.getCurrentFrame()]);
                    values2[stencilBuffer.getCurrentFrame()]++;
                    signalValues.push_back(values2[stencilBuffer.getCurrentFrame()]);*/
                    depthBuffer.submit(/*true, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues*/);
                } else if (shader == &sBuildAlphaBufferShader) {
                    alphaBuffer.beginRecordCommandBuffers();
                    const_cast<Texture&>(depthBuffer.getTexture(depthBuffer.getImageIndex())).toShaderReadOnlyOptimal(alphaBuffer.getCommandBuffers()[alphaBuffer.getCurrentFrame()]);


                    std::vector<VkCommandBuffer> commandBuffers = alphaBuffer.getCommandBuffers();
                    unsigned int currentFrame = alphaBuffer.getCurrentFrame();
                    layerPC.nbLayers = GameObject::getNbLayers();
                    /*vkCmdResetEvent(commandBuffers[currentFrame], events[currentFrame],  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                    vkCmdSetEvent(commandBuffers[currentFrame], events[currentFrame],  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);*/
                    //vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);



                    vkCmdPushConstants(commandBuffers[currentFrame], alphaBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(LayerPC), &layerPC);


                    VkMemoryBarrier memoryBarrier={};
                    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                    memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                    memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                    //vkCmdWaitEvents(commandBuffers[currentFrame], 1, &events[currentFrame], VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                    vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                    alphaBuffer.beginRenderPass();
                    alphaBuffer.drawIndirect(commandBuffers[currentFrame], currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], vboIndirect, depthStencilID,currentStates);
                    alphaBuffer.endRenderPass();
                    const_cast<Texture&>(depthBuffer.getTexture(depthBuffer.getImageIndex())).toColorAttachmentOptimal(alphaBuffer.getCommandBuffers()[alphaBuffer.getCurrentFrame()]);
                    /*waitSemaphores.push_back(offscreenDepthFinishedSemaphore[shadowMap.getCurrentFrame()]);
                    waitStages.push_back(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                    waitValues.clear();
                    waitValues.push_back(values[stencilBuffer.getCurrentFrame()]);
                    waitValues.push_back(values2[depthBuffer.getCurrentFrame()]);
                    signalSemaphores.clear();
                    signalSemaphores.push_back(offscreenAlphaFinishedSemaphore[alphaBuffer.getCurrentFrame()]);
                    signalValues.clear();*/
                    alphaBuffer.submit(/*true, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues*/);

                } else if (shader == &buildShadowMapShader) {

                    stencilBuffer.beginRecordCommandBuffers();
                    std::vector<VkCommandBuffer> commandBuffers = stencilBuffer.getCommandBuffers();
                    unsigned int currentFrame = stencilBuffer.getCurrentFrame();


                    //vkCmdWaitEvents(commandBuffers[currentFrame], 1, &events[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                    vkCmdPushConstants(commandBuffers[currentFrame], stencilBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(LightIndirectRenderingPC), &lightIndirectRenderingPC);
                    vkCmdPushConstants(commandBuffers[currentFrame], stencilBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_FRAGMENT_BIT, 128, sizeof(LayerPC), &layerPC);
                    /*////std::cout<<"pipeline layout : "<<stencilBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][stencilBuffer.getId()][depthStencilID]<<std::endl;
                    system("PAUSE");*/

                    stencilBuffer.beginRenderPass();
                    stencilBuffer.drawIndirect(commandBuffers[currentFrame], currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], vboIndirect, depthStencilID,currentStates);
                    stencilBuffer.endRenderPass();
                    /*
                    values[stencilBuffer.getCurrentFrame()]++;
                    signalValues.clear();
                    signalValues.push_back(values[stencilBuffer.getCurrentFrame()]);
                    signalSemaphores.clear();
                    signalSemaphores.push_back(offscreenFinishedSemaphore[shadowMap.getCurrentFrame()]);*/
                    stencilBuffer.submit(/*true, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues*/);

                } else {

                    const_cast<Texture&>(stencilBuffer.getTexture(stencilBuffer.getImageIndex())).toShaderReadOnlyOptimal(shadowMap.getCommandBuffers()[shadowMap.getCurrentFrame()]);
                    const_cast<Texture&>(alphaBuffer.getTexture(alphaBuffer.getImageIndex())).toShaderReadOnlyOptimal(shadowMap.getCommandBuffers()[shadowMap.getCurrentFrame()]);
                    const_cast<Texture&>(depthBuffer.getTexture(depthBuffer.getImageIndex())).toShaderReadOnlyOptimal(shadowMap.getCommandBuffers()[shadowMap.getCurrentFrame()]);


                    std::vector<VkCommandBuffer> commandBuffers = shadowMap.getCommandBuffers();
                    unsigned int currentFrame = shadowMap.getCurrentFrame();



                    vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);
                    VkMemoryBarrier memoryBarrier;
                    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                    memoryBarrier.pNext = VK_NULL_HANDLE;
                    memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                    memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                    //vkCmdWaitEvents(commandBuffers[currentFrame], 1, &events[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

                    vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                    vkCmdPushConstants(commandBuffers[currentFrame], shadowMap.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ResolutionPC), &resolutionPC);
                    shadowMap.beginRenderPass();
                    shadowMap.drawIndirect(commandBuffers[currentFrame], currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], vboIndirect, depthStencilID,currentStates);
                    shadowMap.endRenderPass();

                    const_cast<Texture&>(alphaBuffer.getTexture(alphaBuffer.getImageIndex())).toColorAttachmentOptimal(shadowMap.getCommandBuffers()[shadowMap.getCurrentFrame()]);
                    const_cast<Texture&>(depthBuffer.getTexture(depthBuffer.getImageIndex())).toColorAttachmentOptimal(shadowMap.getCommandBuffers()[shadowMap.getCurrentFrame()]);
                    const_cast<Texture&>(stencilBuffer.getTexture(stencilBuffer.getImageIndex())).toColorAttachmentOptimal(shadowMap.getCommandBuffers()[shadowMap.getCurrentFrame()]);

                    /*waitSemaphores.push_back(offscreenAlphaFinishedSemaphore[alphaBuffer.getCurrentFrame()]);
                    waitStages.push_back(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                    waitValues.push_back(0);
                    signalSemaphores.clear();
                    signalSemaphores.push_back(offscreenFinishedSemaphore[shadowMap.getCurrentFrame()]);
                    values[shadowMap.getCurrentFrame()]++;
                    signalValues.push_back(values[shadowMap.getCurrentFrame()]);*/
                    shadowMap.submit(/*true, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues*/);

                }
                isSomethingDrawn = true;
            }
            void ShadowRenderComponent::resetBuffers() {
                for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                    totalBufferSizeModelData[i] = 0;
                    totalBufferSizeMaterialData[i] = 0;
                    totalVertexCount[i] = 0;
                    totalVertexIndexCount[i] = 0;
                    totalIndexCount[i] = 0;
                    totalBufferSizeDrawCommand[i] = 0;
                    totalBufferSizeIndexedDrawCommand[i] = 0;
                    modelDataOffsets[i].clear();
                    materialDataOffsets[i].clear();
                    drawCommandBufferOffsets[i].clear();
                    drawIndexedCommandBufferOffsets[i].clear();
                    nbDrawCommandBuffer[i].clear();
                    nbIndexedDrawCommandBuffer[i].clear();
                    vbBindlessTexIndexed[i].clear();
                    vbBindlessTex[i].clear();
                    materialDatas[i].clear();
                    modelDatas[i].clear();
                    drawArraysIndirectCommands[i].clear();
                    drawElementsIndirectCommands[i].clear();
                    currentModelOffset[i] = 0;
                    currentMaterialOffset[i] = 0;
                    maxAlignedSizeModelData[i] = 0;
                    maxAlignedSizeMaterialData[i] = 0;
                    oldTotalBufferSizeMaterialData[i] = 0;
                    oldTotalBufferSizeModelData[i] = 0;
                }
            }
            void ShadowRenderComponent::fillBuffersMT() {
                std::array<unsigned int, Batcher::nbPrimitiveTypes> firstIndex, baseInstance;
                for (unsigned int i = 0; i < firstIndex.size(); i++) {
                    firstIndex[i] = 0;
                }
                for (unsigned int i = 0; i < baseInstance.size(); i++) {
                    baseInstance[i] = 0;
                }
                std::array<unsigned int, Batcher::nbPrimitiveTypes> drawCommandCount, oldTotalVertexCount;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    drawCommandBufferOffsets[p].push_back(totalBufferSizeDrawCommand[p]);
                    drawCommandCount[p] = 0;
                    oldTotalVertexCount[p] = totalVertexCount[p];
                }
                for (unsigned int i = 0; i < m_normals.size(); i++) {
                   if (m_normals[i].getAllVertices().getVertexCount() > 0) {
                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        unsigned int p = m_normals[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        {
                            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                            material.textureIndex = (m_normals[i].getMaterial().getTexture() != nullptr) ? m_normals[i].getMaterial().getTexture()->getId() : 0;
                            material.layer = m_normals[i].getMaterial().getLayer();
                            material.uvScale = (m_normals[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_normals[i].getMaterial().getTexture()->getSize().x(), 1.f / m_normals[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                            material.uvOffset = math::Vec2f(0, 0);
                        }

                        materialDatas[p].push_back(material);
                        TransformMatrix tm;
                        ModelData model;
                        model.worldMat = tm.getMatrix()/*.transpose()*/;
                        model.shadowProjMat = tm.getMatrix()/*.transpose()*/;

                        modelDatas[p].push_back(model);
                        unsigned int vertexCount = 0;
                        for (unsigned int j = 0; j < m_normals[i].getAllVertices().getVertexCount(); j++) {
                            vbBindlessTex[p].append(m_normals[i].getAllVertices()[j]);
                            vertexCount++;
                        }
                        drawArraysIndirectCommand.count = vertexCount;
                        drawArraysIndirectCommand.firstIndex = firstIndex[p] + oldTotalVertexCount[p];
                        drawArraysIndirectCommand.baseInstance = baseInstance[p];
                        drawArraysIndirectCommand.instanceCount = 1;
                        drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                        firstIndex[p] += vertexCount;
                        baseInstance[p] += 1;
                        totalVertexCount[p] += vertexCount;
                        drawCommandCount[p]++;
                    }
                }
                for (unsigned int i = 0; i < m_instances.size(); i++) {
                    if (m_instances[i].getAllVertices().getVertexCount() > 0) {
                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        unsigned int p = m_instances[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        {
                            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                            material.textureIndex = (m_instances[i].getMaterial().getTexture() != nullptr) ? m_instances[i].getMaterial().getTexture()->getId() : 0;
                            material.layer = m_instances[i].getMaterial().getLayer();
                            material.uvScale = (m_instances[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_instances[i].getMaterial().getTexture()->getSize().x(), 1.f / m_instances[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                            material.uvOffset = math::Vec2f(0, 0);
                        }
                        materialDatas[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_instances[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            ModelData model;
                            model.worldMat = tm[j]->getMatrix()/*.transpose()*/;
                            TransformMatrix tm;
                            model.shadowProjMat = tm.getMatrix()/*.transpose()*/;

                            modelDatas[p].push_back(model);
                        }
                        unsigned int vertexCount = 0;
                        if (m_instances[i].getEntities().size() > 0) {
                            Entity* firstInstance = m_instances[i].getEntities()[0];
                            for (unsigned int j = 0; j < firstInstance->getFaces().size(); j++) {
                                for (unsigned int k = 0; k < firstInstance->getFace(j)->getVertexArray().getVertexCount(); k++) {
                                    vertexCount++;
                                    vbBindlessTex[p].append(firstInstance->getFace(j)->getVertexArray()[k]);
                                }
                            }
                        }
                        drawArraysIndirectCommand.count = vertexCount;
                        drawArraysIndirectCommand.firstIndex = firstIndex[p] + oldTotalVertexCount[p];
                        drawArraysIndirectCommand.baseInstance = baseInstance[p];
                        drawArraysIndirectCommand.instanceCount = tm.size();
                        drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                        firstIndex[p] += vertexCount;
                        baseInstance[p] += tm.size();
                        totalVertexCount[p] += vertexCount;
                        drawCommandCount[p]++;
                    }
                }
                std::array<unsigned int, Batcher::nbPrimitiveTypes> alignedOffsetModelData, alignedOffsetMaterialData;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    nbDrawCommandBuffer[p].push_back(drawCommandCount[p]);
                    alignedOffsetModelData[p] = align(currentModelOffset[p]);
                    modelDataOffsets[p].push_back(alignedOffsetModelData[p]);
                    alignedOffsetMaterialData[p] = align(currentMaterialOffset[p]);
                    materialDataOffsets[p].push_back(alignedOffsetMaterialData[p]);
                }
                VkCommandBufferInheritanceInfo inheritanceInfo{};
                inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                inheritanceInfo.renderPass = VK_NULL_HANDLE; // pas de render pass
                inheritanceInfo.subpass = 0;
                inheritanceInfo.framebuffer = VK_NULL_HANDLE;
                inheritanceInfo.occlusionQueryEnable = VK_FALSE;
                inheritanceInfo.queryFlags = 0;
                inheritanceInfo.pipelineStatistics = 0;
                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                beginInfo.pInheritanceInfo = &inheritanceInfo; // obligatoire pour secondaire
                unsigned int currentFrame = depthBuffer.getCurrentFrame();
                vkResetCommandBuffer(copyModelDataBufferCommandBuffer[currentFrame], 0);
                vkResetCommandBuffer(copyMaterialDataBufferCommandBuffer[currentFrame], 0);
                vkResetCommandBuffer(copyDrawBufferCommandBuffer[currentFrame], 0);
                vkResetCommandBuffer(copyVbBufferCommandBuffer[currentFrame], 0);
                if (vkBeginCommandBuffer(copyModelDataBufferCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }
                if (vkBeginCommandBuffer(copyMaterialDataBufferCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }
                if (vkBeginCommandBuffer(copyDrawBufferCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }
                if (vkBeginCommandBuffer(copyVbBufferCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (nbDrawCommandBuffer[p][0] > 0) {

                        VkDeviceSize bufferSize = sizeof(ModelData) * modelDatas[p].size();

                        currentModelOffset[p] = alignedOffsetModelData[p] + ((bufferSize - oldTotalBufferSizeModelData[p] > 0) ? bufferSize - oldTotalBufferSizeModelData[p] : 0);

                        maxAlignedSizeModelData[p] = (bufferSize - oldTotalBufferSizeModelData[p] > maxAlignedSizeModelData[p]) ? bufferSize - oldTotalBufferSizeModelData[p] : maxAlignedSizeModelData[p];
                        totalBufferSizeModelData[p] = (alignedOffsetModelData[p] + maxAlignedSizeModelData[p] > bufferSize) ? alignedOffsetModelData[p] + maxAlignedSizeModelData[p] : bufferSize;
                        oldTotalBufferSizeModelData[p] = bufferSize;
                        if (totalBufferSizeModelData[p] > maxBufferSizeModelData[p][currentFrame]) {

                            if (modelDataStagingBufferMT[p][currentFrame] != nullptr) {
                                vkDestroyBuffer(vkDevice.getDevice(), modelDataStagingBufferMT[p][currentFrame], nullptr);
                                vkFreeMemory(vkDevice.getDevice(), modelDataStagingBufferMemoryMT[p][currentFrame], nullptr);
                            }

                            createBuffer(totalBufferSizeModelData[p], VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, modelDataStagingBufferMT[p][currentFrame], modelDataStagingBufferMemoryMT[p][currentFrame]);

                            if (modelDataBufferMT[p][currentFrame] != nullptr) {
                                vkDestroyBuffer(vkDevice.getDevice(), modelDataBufferMT[p][currentFrame], nullptr);
                                vkFreeMemory(vkDevice.getDevice(), modelDataBufferMemoryMT[p][currentFrame], nullptr);
                            }

                            createBuffer(totalBufferSizeModelData[p], VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, modelDataBufferMT[p][currentFrame], modelDataBufferMemoryMT[p][currentFrame]);


                            maxBufferSizeModelData[p][currentFrame] = totalBufferSizeModelData[p];
                            needToUpdateDSs[p][currentFrame]  = true;
                        }


                        /*void* data;
                        vkMapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory, 0, bufferSize, 0, &data);
                        memcpy(data, modelDatas[p].data(), (size_t)bufferSize);
                        vkUnmapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory);*/



                        bufferSize = sizeof(MaterialData) * materialDatas[p].size();

                        currentMaterialOffset[p] = alignedOffsetMaterialData[p] + ((bufferSize - oldTotalBufferSizeMaterialData[p] > 0) ? bufferSize - oldTotalBufferSizeMaterialData[p] : 0);

                        maxAlignedSizeMaterialData[p] = (currentMaterialOffset[p] - oldTotalBufferSizeMaterialData[p] > maxAlignedSizeMaterialData[p]) ? currentMaterialOffset[p] - oldTotalBufferSizeMaterialData[p] : maxAlignedSizeMaterialData[p];
                        totalBufferSizeMaterialData[p] = (alignedOffsetMaterialData[p] + maxAlignedSizeMaterialData[p] > bufferSize) ? alignedOffsetMaterialData[p] + maxAlignedSizeMaterialData[p] : bufferSize;
                        oldTotalBufferSizeMaterialData[p] = bufferSize;
                        if (totalBufferSizeMaterialData[p] > maxBufferSizeMaterialData[p][currentFrame]) {
                            if (materialDataStagingBufferMT[p][currentFrame] != nullptr) {
                                vkDestroyBuffer(vkDevice.getDevice(), materialDataStagingBufferMT[p][currentFrame], nullptr);
                                vkFreeMemory(vkDevice.getDevice(), materialDataStagingBufferMemoryMT[p][currentFrame], nullptr);
                            }
                            createBuffer(totalBufferSizeMaterialData[p], VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, materialDataStagingBufferMT[p][currentFrame], materialDataStagingBufferMemoryMT[p][currentFrame]);

                            if (materialDataBufferMT[p][currentFrame] != nullptr) {
                                vkDestroyBuffer(vkDevice.getDevice(),materialDataBufferMT[p][currentFrame], nullptr);
                                vkFreeMemory(vkDevice.getDevice(), materialDataBufferMemoryMT[p][currentFrame], nullptr);
                            }
                            createBuffer(totalBufferSizeMaterialData[p], VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, materialDataBufferMT[p][currentFrame], materialDataBufferMemoryMT[p][currentFrame]);


                            maxBufferSizeMaterialData[p][currentFrame] = totalBufferSizeMaterialData[p];
                            needToUpdateDSs[p][currentFrame]  = true;
                        }

                        /*vkMapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory, 0, bufferSize, 0, &data);
                        memcpy(data, materialDatas[p].data(), (size_t)bufferSize);
                        vkUnmapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory);*/

                        bufferSize = sizeof(DrawArraysIndirectCommand) * drawArraysIndirectCommands[p].size();

                        totalBufferSizeDrawCommand[p] = bufferSize;

                        if (totalBufferSizeDrawCommand[p] > maxBufferSizeDrawCommand[p][currentFrame]) {
                            if (vboIndirectStagingBufferMT[p][currentFrame] != nullptr) {
                                vkDestroyBuffer(vkDevice.getDevice(), vboIndirectStagingBufferMT[p][currentFrame], nullptr);
                                vkFreeMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemoryMT[p][currentFrame], nullptr);
                            }
                            createBuffer(totalBufferSizeDrawCommand[p], VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vboIndirectStagingBufferMT[p][currentFrame], vboIndirectStagingBufferMemoryMT[p][currentFrame]);

                            if (drawCommandBufferMT[p][currentFrame] != nullptr) {
                                vkDestroyBuffer(vkDevice.getDevice(),drawCommandBufferMT[p][currentFrame], nullptr);
                                vkFreeMemory(vkDevice.getDevice(), drawCommandBufferMemoryMT[p][currentFrame], nullptr);
                            }
                            createBuffer(totalBufferSizeDrawCommand[p], VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, drawCommandBufferMT[p][currentFrame], drawCommandBufferMemoryMT[p][currentFrame]);

                            maxBufferSizeDrawCommand[p][currentFrame] = totalBufferSizeDrawCommand[p];
                        }



                    }
                    VkDeviceSize bufferSize = sizeof(ModelData) * modelDatas[p].size();
                    if (bufferSize > 0)
                        copyBuffer(modelDataStagingBufferMT[p][currentFrame], modelDataBufferMT[p][currentFrame], bufferSize, copyModelDataBufferCommandBuffer[currentFrame]);
                    bufferSize = sizeof(MaterialData) * materialDatas[p].size();
                    if (bufferSize > 0)
                        copyBuffer(materialDataStagingBufferMT[p][currentFrame],materialDataBufferMT[p][currentFrame], bufferSize, copyMaterialDataBufferCommandBuffer[currentFrame]);
                    bufferSize = sizeof(DrawArraysIndirectCommand) * drawArraysIndirectCommands[p].size();
                    if (bufferSize > 0)
                        copyBuffer(vboIndirectStagingBufferMT[p][currentFrame], drawCommandBufferMT[p][currentFrame], totalBufferSizeDrawCommand[p], copyDrawBufferCommandBuffer[currentFrame]);
                    if (vbBindlessTex[p].getVertexCount() > 0)
                        vbBindlessTex[p].update(currentFrame, copyVbBufferCommandBuffer[currentFrame]);

                }
                if (vkEndCommandBuffer(copyModelDataBufferCommandBuffer[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }
                if (vkEndCommandBuffer(copyMaterialDataBufferCommandBuffer[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }
                if (vkEndCommandBuffer(copyDrawBufferCommandBuffer[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }
                if (vkEndCommandBuffer(copyVbBufferCommandBuffer[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }
            }
            void ShadowRenderComponent::fillIndexedBuffersMT() {
                std::array<unsigned int, Batcher::nbPrimitiveTypes> firstIndex, baseInstance, baseVertex;
                for (unsigned int i = 0; i < firstIndex.size(); i++) {
                    firstIndex[i] = 0;
                }
                for (unsigned int i = 0; i < baseVertex.size(); i++) {
                    baseVertex[i] = 0;
                }
                for (unsigned int i = 0; i < baseInstance.size(); i++) {
                    baseInstance[i] = 0;
                }
                std::array<unsigned int, Batcher::nbPrimitiveTypes> drawCommandCount, oldTotalVertexIndexCount, oldTotalIndexCount;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    drawIndexedCommandBufferOffsets[p].push_back(totalBufferSizeIndexedDrawCommand[p]);
                    drawCommandCount[p] = 0;
                    oldTotalVertexIndexCount[p] = totalVertexIndexCount[p];
                    oldTotalIndexCount[p] = totalIndexCount[p];
                }
                for (unsigned int i = 0; i < m_normalsIndexed.size(); i++) {
                   if (m_normalsIndexed[i].getAllVertices().getVertexCount() > 0) {

                        DrawElementsIndirectCommand drawElementsIndirectCommand;
                        unsigned int p = m_normalsIndexed[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        {
                            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                            material.textureIndex = (m_normalsIndexed[i].getMaterial().getTexture() != nullptr) ? m_normalsIndexed[i].getMaterial().getTexture()->getId() : 0;
                            material.layer = m_normalsIndexed[i].getMaterial().getLayer();
                            material.uvScale = (m_normalsIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_normalsIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_normalsIndexed[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                            material.uvOffset = math::Vec2f(0, 0);
                        }
                        materialDatas[p].push_back(material);
                        TransformMatrix tm;
                        ModelData model;
                        model.worldMat = tm.getMatrix().transpose();
                        model.shadowProjMat = tm.getMatrix().transpose();

                        modelDatas[p].push_back(model);
                        unsigned int indexCount = 0, vertexCount = 0;
                        for (unsigned int j = 0; j < m_normalsIndexed[i].getAllVertices().getVertexCount(); j++) {
                            vertexCount++;
                            vbBindlessTexIndexed[p].append(m_normalsIndexed[i].getAllVertices()[j]);
                        }
                        for (unsigned int j = 0; j < m_normalsIndexed[i].getAllVertices().getIndexes().size(); j++) {
                            ////std::cout<<"add  norm index"<<std::endl;
                            indexCount++;
                            vbBindlessTexIndexed[p].addIndex(m_normalsIndexed[i].getAllVertices().getIndexes()[j]);
                        }
                        drawElementsIndirectCommand.index_count = indexCount;
                        drawElementsIndirectCommand.first_index = firstIndex[p] + oldTotalIndexCount[p];
                        drawElementsIndirectCommand.instance_base = baseInstance[p];
                        drawElementsIndirectCommand.vertex_base = baseVertex[p] + oldTotalVertexIndexCount[p];
                        drawElementsIndirectCommand.instance_count = 1;
                        drawElementsIndirectCommands[p].push_back(drawElementsIndirectCommand);
                        firstIndex[p] += indexCount;
                        baseVertex[p] += vertexCount;
                        baseInstance[p] += 1;
                        totalIndexCount[p] += indexCount;
                        totalVertexIndexCount[p] += vertexCount;
                        drawCommandCount[p]++;
                    }
                }
                for (unsigned int i = 0; i < m_instancesIndexed.size(); i++) {
                    if (m_instancesIndexed[i].getAllVertices().getVertexCount() > 0) {
                        DrawElementsIndirectCommand drawElementsIndirectCommand;
                        unsigned int p = m_instancesIndexed[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        {
                            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                            material.textureIndex = (m_instancesIndexed[i].getMaterial().getTexture() != nullptr) ? m_instancesIndexed[i].getMaterial().getTexture()->getId() : 0;
                            material.uvScale = (m_instancesIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_instancesIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_instancesIndexed[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                            material.uvOffset = math::Vec2f(0, 0);
                            material.layer = m_instancesIndexed[i].getMaterial().getLayer();
                        }
                        materialDatas[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_instancesIndexed[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            ModelData model;
                            model.worldMat = tm[j]->getMatrix().transpose();
                            TransformMatrix tm;
                            model.shadowProjMat = tm.getMatrix().transpose();

                            modelDatas[p].push_back(model);
                        }
                        unsigned int vertexCount = 0, indexCount = 0;
                        if (m_instancesIndexed[i].getEntities().size() > 0) {
                            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                            Entity* firstInstance = m_instancesIndexed[i].getEntities()[0];
                            for (unsigned int j = 0; j < firstInstance->getFaces().size(); j++) {
                                for (unsigned int k = 0; k < firstInstance->getFace(j)->getVertexArray().getVertexCount(); k++) {
                                    vertexCount++;
                                    vbBindlessTexIndexed[p].append(firstInstance->getFace(j)->getVertexArray()[k]);
                                }
                                for (unsigned int k = 0; k < firstInstance->getFace(j)->getVertexArray().getIndexes().size(); k++) {
                                    indexCount++;
                                    vbBindlessTexIndexed[p].addIndex(firstInstance->getFace(j)->getVertexArray().getIndexes()[k]);

                                }
                            }
                        }
                        drawElementsIndirectCommand.index_count = indexCount;
                        drawElementsIndirectCommand.first_index = firstIndex[p] + oldTotalIndexCount[p];
                        drawElementsIndirectCommand.instance_base = baseInstance[p];
                        drawElementsIndirectCommand.vertex_base = baseVertex[p] + oldTotalVertexIndexCount[p];
                        drawElementsIndirectCommand.instance_count = tm.size();
                        drawElementsIndirectCommands[p].push_back(drawElementsIndirectCommand);
                        firstIndex[p] += indexCount;
                        baseVertex[p] += vertexCount;
                        baseInstance[p] += tm.size();
                        totalIndexCount[p] += indexCount;
                        totalVertexIndexCount[p] += vertexCount;
                        drawCommandCount[p]++;
                    }
                }
                std::array<unsigned int, Batcher::nbPrimitiveTypes> alignedOffsetModelData, alignedOffsetMaterialData;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    nbIndexedDrawCommandBuffer[p].push_back(drawCommandCount[p]);
                    alignedOffsetModelData[p] = align(currentModelOffset[p]);
                    modelDataOffsets[p].push_back(alignedOffsetModelData[p]);
                    alignedOffsetMaterialData[p] = align(currentMaterialOffset[p]);
                    materialDataOffsets[p].push_back(alignedOffsetMaterialData[p]);

                }
                VkCommandBufferInheritanceInfo inheritanceInfo{};
                inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                inheritanceInfo.renderPass = VK_NULL_HANDLE; // pas de render pass
                inheritanceInfo.subpass = 0;
                inheritanceInfo.framebuffer = VK_NULL_HANDLE;
                inheritanceInfo.occlusionQueryEnable = VK_FALSE;
                inheritanceInfo.queryFlags = 0;
                inheritanceInfo.pipelineStatistics = 0;
                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                beginInfo.pInheritanceInfo = &inheritanceInfo; // obligatoire pour secondaire
                unsigned int currentFrame = depthBuffer.getCurrentFrame();
                vkResetCommandBuffer(copyModelDataBufferCommandBuffer[currentFrame], 0);
                vkResetCommandBuffer(copyMaterialDataBufferCommandBuffer[currentFrame], 0);
                vkResetCommandBuffer(copyDrawIndexedBufferCommandBuffer[currentFrame], 0);
                vkResetCommandBuffer(copyVbIndexedBufferCommandBuffer[currentFrame], 0);
                if (vkBeginCommandBuffer(copyModelDataBufferCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }
                if (vkBeginCommandBuffer(copyMaterialDataBufferCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }
                if (vkBeginCommandBuffer(copyDrawIndexedBufferCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }
                if (vkBeginCommandBuffer(copyVbIndexedBufferCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }

                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (nbIndexedDrawCommandBuffer[p][0] > 0) {

                        VkDeviceSize bufferSize = sizeof(ModelData) * modelDatas[p].size();

                        currentModelOffset[p] = alignedOffsetModelData[p] + ((bufferSize - oldTotalBufferSizeModelData[p] > 0) ? bufferSize - oldTotalBufferSizeModelData[p] : 0);

                        maxAlignedSizeModelData[p] = (bufferSize - oldTotalBufferSizeModelData[p] > maxAlignedSizeModelData[p]) ? bufferSize - oldTotalBufferSizeModelData[p] : maxAlignedSizeModelData[p];
                        totalBufferSizeModelData[p] = (alignedOffsetModelData[p] + maxAlignedSizeModelData[p] > bufferSize) ? alignedOffsetModelData[p] + maxAlignedSizeModelData[p] : bufferSize;
                        oldTotalBufferSizeModelData[p] = bufferSize;
                        if (totalBufferSizeModelData[p] > maxBufferSizeModelData[p][currentFrame]) {

                            if (modelDataStagingBufferMT[p][currentFrame] != nullptr) {
                                vkDestroyBuffer(vkDevice.getDevice(), modelDataStagingBufferMT[p][currentFrame], nullptr);
                                vkFreeMemory(vkDevice.getDevice(), modelDataStagingBufferMemoryMT[p][currentFrame], nullptr);
                            }

                            createBuffer(totalBufferSizeModelData[p], VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, modelDataStagingBufferMT[p][currentFrame], modelDataStagingBufferMemoryMT[p][currentFrame]);

                            if (modelDataBufferMT[p][currentFrame] != nullptr) {
                                vkDestroyBuffer(vkDevice.getDevice(), modelDataBufferMT[p][currentFrame], nullptr);
                                vkFreeMemory(vkDevice.getDevice(), modelDataBufferMemoryMT[p][currentFrame], nullptr);
                            }

                            createBuffer(totalBufferSizeModelData[p], VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, modelDataBufferMT[p][currentFrame], modelDataBufferMemoryMT[p][currentFrame]);


                            maxBufferSizeModelData[p][currentFrame] = totalBufferSizeModelData[p];
                            needToUpdateDSs[p][currentFrame]  = true;
                        }


                        /*void* data;
                        vkMapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory, 0, bufferSize, 0, &data);
                        memcpy(data, modelDatas[p].data(), (size_t)bufferSize);
                        vkUnmapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory);*/



                        bufferSize = sizeof(MaterialData) * materialDatas[p].size();

                        currentMaterialOffset[p] = alignedOffsetMaterialData[p] + ((bufferSize - oldTotalBufferSizeMaterialData[p] > 0) ? bufferSize - oldTotalBufferSizeMaterialData[p] : 0);

                        maxAlignedSizeMaterialData[p] = (currentMaterialOffset[p] - oldTotalBufferSizeMaterialData[p] > maxAlignedSizeMaterialData[p]) ? currentMaterialOffset[p] - oldTotalBufferSizeMaterialData[p] : maxAlignedSizeMaterialData[p];
                        totalBufferSizeMaterialData[p] = (alignedOffsetMaterialData[p] + maxAlignedSizeMaterialData[p] > bufferSize) ? alignedOffsetMaterialData[p] + maxAlignedSizeMaterialData[p] : bufferSize;
                        oldTotalBufferSizeMaterialData[p] = bufferSize;
                        if (totalBufferSizeMaterialData[p] > maxBufferSizeMaterialData[p][currentFrame]) {
                            if (materialDataStagingBufferMT[p][currentFrame] != nullptr) {
                                vkDestroyBuffer(vkDevice.getDevice(), materialDataStagingBufferMT[p][currentFrame], nullptr);
                                vkFreeMemory(vkDevice.getDevice(), materialDataStagingBufferMemoryMT[p][currentFrame], nullptr);
                            }
                            createBuffer(totalBufferSizeMaterialData[p], VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, materialDataStagingBufferMT[p][currentFrame], materialDataStagingBufferMemoryMT[p][currentFrame]);

                            if (materialDataBufferMT[p][currentFrame] != nullptr) {
                                vkDestroyBuffer(vkDevice.getDevice(),materialDataBufferMT[p][currentFrame], nullptr);
                                vkFreeMemory(vkDevice.getDevice(), materialDataBufferMemoryMT[p][currentFrame], nullptr);
                            }
                            createBuffer(totalBufferSizeMaterialData[p], VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, materialDataBufferMT[p][currentFrame], materialDataBufferMemoryMT[p][currentFrame]);


                            maxBufferSizeMaterialData[p][currentFrame] = totalBufferSizeMaterialData[p];
                            needToUpdateDSs[p][currentFrame]  = true;
                        }


                        /*vkMapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory, 0, bufferSize, 0, &data);
                        memcpy(data, materialDatas[p].data(), (size_t)bufferSize);
                        vkUnmapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory);*/
                        //copyBuffer(materialDataStagingBuffer,materialDataBufferMT[p], bufferSize, copyMaterialDataBufferCommandBuffer);

                        bufferSize = sizeof(DrawElementsIndirectCommand) * drawElementsIndirectCommands[p].size();
                        totalBufferSizeIndexedDrawCommand[p] = bufferSize;

                        ////std::cout<<"buffer size : "<<bufferSize<<std::endl<<"max : "<<maxBufferSizeIndexedDrawCommand[p]<<std::endl;
                        if (totalBufferSizeIndexedDrawCommand[p] > maxBufferSizeIndexedDrawCommand[p][currentFrame]) {
                            if (vboIndexedIndirectStagingBufferMT[p][currentFrame] != nullptr) {
                                    vkDestroyBuffer(vkDevice.getDevice(), vboIndexedIndirectStagingBufferMT[p][currentFrame], nullptr);
                                    vkFreeMemory(vkDevice.getDevice(), vboIndexedIndirectStagingBufferMemoryMT[p][currentFrame], nullptr);
                                }
                                createBuffer(totalBufferSizeIndexedDrawCommand[p], VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vboIndexedIndirectStagingBufferMT[p][currentFrame], vboIndexedIndirectStagingBufferMemoryMT[p][currentFrame]);

                            if (drawCommandBufferIndexedMT[p][currentFrame] != nullptr) {
                                vkDestroyBuffer(vkDevice.getDevice(),drawCommandBufferIndexedMT[p][currentFrame], nullptr);
                                vkFreeMemory(vkDevice.getDevice(), drawCommandBufferIndexedMemoryMT[p][currentFrame], nullptr);
                            }
                            createBuffer(totalBufferSizeIndexedDrawCommand[p], VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, drawCommandBufferIndexedMT[p][currentFrame], drawCommandBufferIndexedMemoryMT[p][currentFrame]);

                            maxBufferSizeIndexedDrawCommand[p][currentFrame] = totalBufferSizeIndexedDrawCommand[p];
                        }

                    }
                    VkDeviceSize bufferSize = sizeof(ModelData) * modelDatas[p].size();
                    if (bufferSize > 0)
                        copyBuffer(modelDataStagingBufferMT[p][currentFrame], modelDataBufferMT[p][currentFrame], bufferSize, copyModelDataBufferCommandBuffer[currentFrame]);
                    bufferSize = sizeof(MaterialData) * materialDatas[p].size();
                    if (bufferSize > 0)
                        copyBuffer(materialDataStagingBufferMT[p][currentFrame],materialDataBufferMT[p][currentFrame], bufferSize, copyMaterialDataBufferCommandBuffer[currentFrame]);
                    bufferSize = sizeof(DrawElementsIndirectCommand) * drawElementsIndirectCommands[p].size();
                    if (bufferSize > 0)
                        copyBuffer(vboIndexedIndirectStagingBufferMT[p][currentFrame], drawCommandBufferIndexedMT[p][currentFrame], bufferSize, copyDrawIndexedBufferCommandBuffer[currentFrame]);
                    if (vbBindlessTexIndexed[p].getVertexCount() > 0)
                        vbBindlessTexIndexed[p].update(currentFrame, copyVbIndexedBufferCommandBuffer[currentFrame]);

                }
                if (vkEndCommandBuffer(copyModelDataBufferCommandBuffer[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }
                if (vkEndCommandBuffer(copyMaterialDataBufferCommandBuffer[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }
                if (vkEndCommandBuffer(copyDrawIndexedBufferCommandBuffer[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }
                if (vkEndCommandBuffer(copyVbIndexedBufferCommandBuffer[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }
            }
            void ShadowRenderComponent::fillShadowBuffersMT() {
                std::array<unsigned int, Batcher::nbPrimitiveTypes> firstIndex, baseInstance;
                for (unsigned int i = 0; i < firstIndex.size(); i++) {
                    firstIndex[i] = 0;
                }
                for (unsigned int i = 0; i < baseInstance.size(); i++) {
                    baseInstance[i] = 0;
                }
                std::array<unsigned int, Batcher::nbPrimitiveTypes> drawCommandCount, oldTotalVertexCount;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    drawCommandBufferOffsets[p].push_back(totalBufferSizeDrawCommand[p]);
                    drawCommandCount[p] = 0;
                    oldTotalVertexCount[p] = totalVertexCount[p];
                }
                for (unsigned int i = 0; i < m_shadow_normals.size(); i++) {
                    if (m_shadow_normals[i].getAllVertices().getVertexCount() > 0) {

                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        unsigned int p = m_shadow_normals[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        {
                            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                            material.textureIndex = (m_shadow_normals[i].getMaterial().getTexture() != nullptr) ? m_shadow_normals[i].getMaterial().getTexture()->getId() : 0;
                            material.layer = m_shadow_normals[i].getMaterial().getLayer();
                            material.uvScale = (m_shadow_normals[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_shadow_normals[i].getMaterial().getTexture()->getSize().x(), 1.f / m_shadow_normals[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                            material.uvOffset = math::Vec2f(0, 0);
                        }
                        materialDatas[p].push_back(material);
                        TransformMatrix tm;
                        ModelData model;
                        model.worldMat = tm.getMatrix()/*.transpose()*/;
                        model.shadowProjMat = tm.getMatrix()/*.transpose()*/;

                        modelDatas[p].push_back(model);
                        unsigned int vertexCount = 0;
                        for (unsigned int j = 0; j < m_shadow_normals[i].getAllVertices().getVertexCount(); j++) {
                            vertexCount++;
                            vbBindlessTex[p].append(m_shadow_normals[i].getAllVertices()[j]);
                        }
                        drawArraysIndirectCommand.count = vertexCount;
                        drawArraysIndirectCommand.firstIndex = firstIndex[p] + oldTotalVertexCount[p];
                        drawArraysIndirectCommand.baseInstance = baseInstance[p];
                        drawArraysIndirectCommand.instanceCount = 1;
                        drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                        firstIndex[p] += vertexCount;
                        baseInstance[p] += 1;
                        totalVertexCount[p] += vertexCount;
                        drawCommandCount[p]++;
                    }
                }
                for (unsigned int i = 0; i < m_shadow_instances.size(); i++) {
                    if (m_shadow_instances[i].getAllVertices().getVertexCount() > 0) {
                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        unsigned int p = m_shadow_instances[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        {
                            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                            material.textureIndex = (m_shadow_instances[i].getMaterial().getTexture() != nullptr) ? m_shadow_instances[i].getMaterial().getTexture()->getId() : 0;
                            material.layer = m_shadow_instances[i].getMaterial().getLayer();
                            material.uvScale = (m_shadow_instances[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_shadow_instances[i].getMaterial().getTexture()->getSize().x(), 1.f / m_shadow_instances[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                            material.uvOffset = math::Vec2f(0, 0);
                        }
                        materialDatas[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_shadow_instances[i].getTransforms();
                        std::vector<TransformMatrix> tm2 = m_shadow_instances[i].getShadowProjMatrix();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            tm2[j].update();
                            ModelData model;
                            model.worldMat = tm[j]->getMatrix()/*.transpose()*/;
                            model.shadowProjMat = tm2[j].getMatrix()/*.transpose()*/;

                            modelDatas[p].push_back(model);
                        }
                        unsigned int vertexCount=0;
                        if (m_shadow_instances[i].getEntities().size() > 0) {
                            Entity* firstInstance = m_shadow_instances[i].getEntities()[0];
                            for (unsigned int j = 0; j < firstInstance->getFaces().size(); j++) {
                                for (unsigned int k = 0; k < firstInstance->getFace(j)->getVertexArray().getVertexCount(); k++) {
                                    vertexCount++;
                                    vbBindlessTex[p].append(firstInstance->getFace(j)->getVertexArray()[k]);
                                }
                            }
                        }
                        drawArraysIndirectCommand.count = vertexCount;
                        drawArraysIndirectCommand.firstIndex = firstIndex[p] + oldTotalVertexCount[p];
                        drawArraysIndirectCommand.baseInstance = baseInstance[p];
                        drawArraysIndirectCommand.instanceCount = tm.size();
                        drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                        firstIndex[p] += vertexCount;
                        baseInstance[p] += tm.size();
                        totalVertexCount[p] += vertexCount;
                        drawCommandCount[p]++;
                    }
                }
                std::array<unsigned int, Batcher::nbPrimitiveTypes> alignedOffsetModelData, alignedOffsetMaterialData;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    nbDrawCommandBuffer[p].push_back(drawCommandCount[p]);
                    alignedOffsetModelData[p] = align(currentModelOffset[p]);
                    modelDataOffsets[p].push_back(alignedOffsetModelData[p]);
                    alignedOffsetMaterialData[p] = align(currentMaterialOffset[p]);
                    materialDataOffsets[p].push_back(alignedOffsetMaterialData[p]);
                }
                VkCommandBufferInheritanceInfo inheritanceInfo{};
                inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                inheritanceInfo.renderPass = VK_NULL_HANDLE; // pas de render pass
                inheritanceInfo.subpass = 0;
                inheritanceInfo.framebuffer = VK_NULL_HANDLE;
                inheritanceInfo.occlusionQueryEnable = VK_FALSE;
                inheritanceInfo.queryFlags = 0;
                inheritanceInfo.pipelineStatistics = 0;
                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                beginInfo.pInheritanceInfo = &inheritanceInfo; // obligatoire pour secondaire
                unsigned int currentFrame = depthBuffer.getCurrentFrame();
                vkResetCommandBuffer(copyModelDataBufferCommandBuffer[currentFrame], 0);
                vkResetCommandBuffer(copyMaterialDataBufferCommandBuffer[currentFrame], 0);
                vkResetCommandBuffer(copyDrawBufferCommandBuffer[currentFrame], 0);
                vkResetCommandBuffer(copyVbBufferCommandBuffer[currentFrame], 0);
                if (vkBeginCommandBuffer(copyModelDataBufferCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }
                if (vkBeginCommandBuffer(copyMaterialDataBufferCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }
                if (vkBeginCommandBuffer(copyDrawBufferCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }
                if (vkBeginCommandBuffer(copyVbBufferCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (nbDrawCommandBuffer[p][1] > 0) {

                        VkDeviceSize bufferSize = sizeof(ModelData) * modelDatas[p].size();

                        currentModelOffset[p] = alignedOffsetModelData[p] + ((bufferSize - oldTotalBufferSizeModelData[p] > 0) ? bufferSize - oldTotalBufferSizeModelData[p] : 0);

                        maxAlignedSizeModelData[p] = (bufferSize - oldTotalBufferSizeModelData[p] > maxAlignedSizeModelData[p]) ? bufferSize - oldTotalBufferSizeModelData[p] : maxAlignedSizeModelData[p];
                        totalBufferSizeModelData[p] = (alignedOffsetModelData[p] + maxAlignedSizeModelData[p] > bufferSize) ? alignedOffsetModelData[p] + maxAlignedSizeModelData[p] : bufferSize;
                        oldTotalBufferSizeModelData[p] = bufferSize;
                        if (totalBufferSizeModelData[p] > maxBufferSizeModelData[p][currentFrame]) {

                            if (modelDataStagingBufferMT[p][currentFrame] != nullptr) {
                                vkDestroyBuffer(vkDevice.getDevice(), modelDataStagingBufferMT[p][currentFrame], nullptr);
                                vkFreeMemory(vkDevice.getDevice(), modelDataStagingBufferMemoryMT[p][currentFrame], nullptr);
                            }

                            createBuffer(totalBufferSizeModelData[p], VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, modelDataStagingBufferMT[p][currentFrame], modelDataStagingBufferMemoryMT[p][currentFrame]);

                            if (modelDataBufferMT[p][currentFrame] != nullptr) {
                                vkDestroyBuffer(vkDevice.getDevice(), modelDataBufferMT[p][currentFrame], nullptr);
                                vkFreeMemory(vkDevice.getDevice(), modelDataBufferMemoryMT[p][currentFrame], nullptr);
                            }

                            createBuffer(totalBufferSizeModelData[p], VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, modelDataBufferMT[p][currentFrame], modelDataBufferMemoryMT[p][currentFrame]);


                            maxBufferSizeModelData[p][currentFrame] = totalBufferSizeModelData[p];
                            needToUpdateDSs[p][currentFrame]  = true;
                        }


                        /*void* data;
                        vkMapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory, 0, bufferSize, 0, &data);
                        memcpy(data, modelDatas[p].data(), (size_t)bufferSize);
                        vkUnmapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory);*/



                        bufferSize = sizeof(MaterialData) * materialDatas[p].size();

                        currentMaterialOffset[p] = alignedOffsetMaterialData[p] + ((bufferSize - oldTotalBufferSizeMaterialData[p] > 0) ? bufferSize - oldTotalBufferSizeMaterialData[p] : 0);

                        maxAlignedSizeMaterialData[p] = (currentMaterialOffset[p] - oldTotalBufferSizeMaterialData[p] > maxAlignedSizeMaterialData[p]) ? currentMaterialOffset[p] - oldTotalBufferSizeMaterialData[p] : maxAlignedSizeMaterialData[p];
                        totalBufferSizeMaterialData[p] = (alignedOffsetMaterialData[p] + maxAlignedSizeMaterialData[p] > bufferSize) ? alignedOffsetMaterialData[p] + maxAlignedSizeMaterialData[p] : bufferSize;
                        oldTotalBufferSizeMaterialData[p] = bufferSize;
                        if (totalBufferSizeMaterialData[p] > maxBufferSizeMaterialData[p][currentFrame]) {
                            if (materialDataStagingBufferMT[p][currentFrame] != nullptr) {
                                vkDestroyBuffer(vkDevice.getDevice(), materialDataStagingBufferMT[p][currentFrame], nullptr);
                                vkFreeMemory(vkDevice.getDevice(), materialDataStagingBufferMemoryMT[p][currentFrame], nullptr);
                            }
                            createBuffer(totalBufferSizeMaterialData[p], VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, materialDataStagingBufferMT[p][currentFrame], materialDataStagingBufferMemoryMT[p][currentFrame]);

                            if (materialDataBufferMT[p][currentFrame] != nullptr) {
                                vkDestroyBuffer(vkDevice.getDevice(),materialDataBufferMT[p][currentFrame], nullptr);
                                vkFreeMemory(vkDevice.getDevice(), materialDataBufferMemoryMT[p][currentFrame], nullptr);
                            }
                            createBuffer(totalBufferSizeMaterialData[p], VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, materialDataBufferMT[p][currentFrame], materialDataBufferMemoryMT[p][currentFrame]);


                            maxBufferSizeMaterialData[p][currentFrame] = totalBufferSizeMaterialData[p];
                            needToUpdateDSs[p][currentFrame]  = true;
                        }

                        /*vkMapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory, 0, bufferSize, 0, &data);
                        memcpy(data, materialDatas[p].data(), (size_t)bufferSize);
                        vkUnmapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory);*/

                        bufferSize = sizeof(DrawArraysIndirectCommand) * drawArraysIndirectCommands[p].size();

                        totalBufferSizeDrawCommand[p] = bufferSize;

                        if (totalBufferSizeDrawCommand[p] > maxBufferSizeDrawCommand[p][currentFrame]) {

                            if (vboIndirectStagingBufferMT[p][currentFrame] != nullptr) {
                                vkDestroyBuffer(vkDevice.getDevice(), vboIndirectStagingBufferMT[p][currentFrame], nullptr);
                                vkFreeMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemoryMT[p][currentFrame], nullptr);
                            }
                            createBuffer(totalBufferSizeDrawCommand[p], VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vboIndirectStagingBufferMT[p][currentFrame], vboIndirectStagingBufferMemoryMT[p][currentFrame]);

                            if (drawCommandBufferMT[p][currentFrame] != nullptr) {
                                vkDestroyBuffer(vkDevice.getDevice(),drawCommandBufferMT[p][currentFrame], nullptr);
                                vkFreeMemory(vkDevice.getDevice(), drawCommandBufferMemoryMT[p][currentFrame], nullptr);
                            }
                            createBuffer(totalBufferSizeDrawCommand[p], VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, drawCommandBufferMT[p][currentFrame], drawCommandBufferMemoryMT[p][currentFrame]);

                            maxBufferSizeDrawCommand[p][currentFrame] = totalBufferSizeDrawCommand[p];
                        }



                    }
                    VkDeviceSize bufferSize = sizeof(ModelData) * modelDatas[p].size();
                    if (bufferSize > 0)
                        copyBuffer(modelDataStagingBufferMT[p][currentFrame], modelDataBufferMT[p][currentFrame], bufferSize, copyModelDataBufferCommandBuffer[currentFrame]);
                    bufferSize = sizeof(MaterialData) * materialDatas[p].size();
                    if (bufferSize > 0)
                        copyBuffer(materialDataStagingBufferMT[p][currentFrame],materialDataBufferMT[p][currentFrame], bufferSize, copyMaterialDataBufferCommandBuffer[currentFrame]);
                    bufferSize = sizeof(DrawArraysIndirectCommand) * drawArraysIndirectCommands[p].size();
                    if (bufferSize > 0)
                        copyBuffer(vboIndirectStagingBufferMT[p][currentFrame], drawCommandBufferMT[p][currentFrame], totalBufferSizeDrawCommand[p], copyDrawBufferCommandBuffer[currentFrame]);
                    if (vbBindlessTex[p].getVertexCount() > 0)
                        vbBindlessTex[p].update(currentFrame, copyVbBufferCommandBuffer[currentFrame]);

                }
                if (vkEndCommandBuffer(copyModelDataBufferCommandBuffer[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }
                if (vkEndCommandBuffer(copyMaterialDataBufferCommandBuffer[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }
                if (vkEndCommandBuffer(copyDrawBufferCommandBuffer[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }
                if (vkEndCommandBuffer(copyVbBufferCommandBuffer[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }
            }
            void ShadowRenderComponent::fillShadowIndexedBuffersMT() {
                std::array<unsigned int, Batcher::nbPrimitiveTypes> firstIndex, baseInstance, baseVertex;
                for (unsigned int i = 0; i < firstIndex.size(); i++) {
                    firstIndex[i] = 0;
                }
                for (unsigned int i = 0; i < baseVertex.size(); i++) {
                    baseVertex[i] = 0;
                }
                for (unsigned int i = 0; i < baseInstance.size(); i++) {
                    baseInstance[i] = 0;
                }
                std::array<unsigned int, Batcher::nbPrimitiveTypes> drawCommandCount, oldTotalVertexIndexCount, oldTotalIndexCount;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    drawIndexedCommandBufferOffsets[p].push_back(totalBufferSizeIndexedDrawCommand[p]);
                    drawCommandCount[p] = 0;
                    oldTotalVertexIndexCount[p] = totalVertexIndexCount[p];
                    oldTotalIndexCount[p] = totalIndexCount[p];
                }
                for (unsigned int i = 0; i < m_shadow_normalsIndexed.size(); i++) {
                    if (m_shadow_normalsIndexed[i].getAllVertices().getVertexCount() > 0) {
                        DrawElementsIndirectCommand drawElementsIndirectCommand;
                        unsigned int p = m_shadow_normalsIndexed[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        {
                            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                            material.textureIndex = (m_shadow_normalsIndexed[i].getMaterial().getTexture() != nullptr) ? m_shadow_normalsIndexed[i].getMaterial().getTexture()->getId() : 0;
                            material.layer = m_shadow_normalsIndexed[i].getMaterial().getLayer();
                            material.uvScale = (m_shadow_normalsIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_shadow_normalsIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_shadow_normalsIndexed[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                            material.uvOffset = math::Vec2f(0, 0);
                        }

                        materialDatas[p].push_back(material);
                        TransformMatrix tm;
                        ModelData model;
                        model.worldMat = tm.getMatrix().transpose();
                        model.shadowProjMat = tm.getMatrix().transpose();

                        modelDatas[p].push_back(model);
                        unsigned int indexCount = 0, vertexCount = 0;
                        for (unsigned int j = 0; j < m_shadow_normalsIndexed[i].getAllVertices().getVertexCount(); j++) {
                            ////std::cout<<"add shadow norm vert"<<std::endl;
                            vertexCount++;
                            vbBindlessTexIndexed[p].append(m_shadow_normalsIndexed[i].getAllVertices()[j]);
                        }
                        for (unsigned int j = 0; j < m_shadow_normalsIndexed[i].getAllVertices().getIndexes().size(); j++) {
                            ////std::cout<<"add shadow norm index"<<std::endl;
                            indexCount++;
                            vbBindlessTexIndexed[p].addIndex(m_shadow_normalsIndexed[i].getAllVertices().getIndexes()[j]);
                        }
                        drawElementsIndirectCommand.index_count = indexCount;
                        drawElementsIndirectCommand.first_index = firstIndex[p] + oldTotalIndexCount[p];
                        drawElementsIndirectCommand.instance_base = baseInstance[p];
                        drawElementsIndirectCommand.vertex_base = baseVertex[p] + oldTotalVertexIndexCount[p];
                        drawElementsIndirectCommand.instance_count = 1;
                        drawElementsIndirectCommands[p].push_back(drawElementsIndirectCommand);
                        firstIndex[p] += indexCount;
                        baseVertex[p] += vertexCount;
                        baseInstance[p] += 1;
                        totalIndexCount[p] += indexCount;
                        totalVertexIndexCount[p] += vertexCount;
                        drawCommandCount[p]++;
                    }
                }
                for (unsigned int i = 0; i < m_shadow_instances_indexed.size(); i++) {
                    DrawElementsIndirectCommand drawElementsIndirectCommand;
                    if (m_shadow_instances_indexed[i].getAllVertices().getVertexCount() > 0) {
                        unsigned int p = m_shadow_instances_indexed[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        {
                            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                            material.textureIndex = (m_shadow_instances_indexed[i].getMaterial().getTexture() != nullptr) ? m_shadow_instances_indexed[i].getMaterial().getTexture()->getId() : 0;
                            material.layer = m_shadow_instances_indexed[i].getMaterial().getLayer();
                            material.uvScale = (m_shadow_instances_indexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_shadow_instances_indexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_shadow_instances_indexed[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                            material.uvOffset = math::Vec2f(0, 0);
                        }
                        materialDatas[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_shadow_instances_indexed[i].getTransforms();
                        std::vector<TransformMatrix> tm2 = m_shadow_instances_indexed[i].getShadowProjMatrix();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            tm2[j].update();
                            ModelData model;
                            model.worldMat = tm[j]->getMatrix().transpose();
                            model.shadowProjMat = tm2[j].getMatrix().transpose();

                            modelDatas[p].push_back(model);
                        }
                        unsigned int vertexCount = 0, indexCount = 0;
                        if (m_shadow_instances_indexed[i].getEntities().size() > 0) {
                            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                            Entity* firstInstance = m_shadow_instances_indexed[i].getEntities()[0];
                            for (unsigned int j = 0; j < firstInstance->getFaces().size(); j++) {
                                for (unsigned int k = 0; k < firstInstance->getFace(j)->getVertexArray().getVertexCount(); k++) {
                                    vertexCount++;
                                    vbBindlessTexIndexed[p].append(firstInstance->getFace(j)->getVertexArray()[k]);
                                }
                                for (unsigned int k = 0; k < firstInstance->getFace(j)->getVertexArray().getIndexes().size(); k++) {
                                    indexCount++;
                                    vbBindlessTexIndexed[p].addIndex(firstInstance->getFace(j)->getVertexArray().getIndexes()[k]);

                                }
                            }
                        }
                        drawElementsIndirectCommand.index_count = indexCount;
                        drawElementsIndirectCommand.first_index = firstIndex[p];
                        drawElementsIndirectCommand.instance_base = baseInstance[p];
                        drawElementsIndirectCommand.vertex_base = baseVertex[p];
                        drawElementsIndirectCommand.instance_count = tm.size();
                        drawElementsIndirectCommands[p].push_back(drawElementsIndirectCommand);
                        firstIndex[p] += indexCount;
                        baseVertex[p] += vertexCount;
                        baseInstance[p] += tm.size();
                        totalIndexCount[p] += indexCount;
                        totalVertexIndexCount[p] += vertexCount;
                        drawCommandCount[p]++;
                    }
                }
                std::array<unsigned int, Batcher::nbPrimitiveTypes> alignedOffsetModelData, alignedOffsetMaterialData;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    nbIndexedDrawCommandBuffer[p].push_back(drawCommandCount[p]);
                    alignedOffsetModelData[p] = align(currentModelOffset[p]);
                    modelDataOffsets[p].push_back(alignedOffsetModelData[p]);
                    alignedOffsetMaterialData[p] = align(currentMaterialOffset[p]);
                    materialDataOffsets[p].push_back(alignedOffsetMaterialData[p]);

                }
                VkCommandBufferInheritanceInfo inheritanceInfo{};
                inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                inheritanceInfo.renderPass = VK_NULL_HANDLE; // pas de render pass
                inheritanceInfo.subpass = 0;
                inheritanceInfo.framebuffer = VK_NULL_HANDLE;
                inheritanceInfo.occlusionQueryEnable = VK_FALSE;
                inheritanceInfo.queryFlags = 0;
                inheritanceInfo.pipelineStatistics = 0;
                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                beginInfo.pInheritanceInfo = &inheritanceInfo; // obligatoire pour secondaire
                unsigned int currentFrame = depthBuffer.getCurrentFrame();
                vkResetCommandBuffer(copyModelDataBufferCommandBuffer[currentFrame], 0);
                vkResetCommandBuffer(copyMaterialDataBufferCommandBuffer[currentFrame], 0);
                vkResetCommandBuffer(copyDrawIndexedBufferCommandBuffer[currentFrame], 0);
                vkResetCommandBuffer(copyVbIndexedBufferCommandBuffer[currentFrame], 0);
                if (vkBeginCommandBuffer(copyModelDataBufferCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }
                if (vkBeginCommandBuffer(copyMaterialDataBufferCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }
                if (vkBeginCommandBuffer(copyDrawIndexedBufferCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }
                if (vkBeginCommandBuffer(copyVbIndexedBufferCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }

                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (nbIndexedDrawCommandBuffer[p][0] > 0) {

                        VkDeviceSize bufferSize = sizeof(ModelData) * modelDatas[p].size();

                        currentModelOffset[p] = alignedOffsetModelData[p] + ((bufferSize - oldTotalBufferSizeModelData[p] > 0) ? bufferSize - oldTotalBufferSizeModelData[p] : 0);

                        maxAlignedSizeModelData[p] = (bufferSize - oldTotalBufferSizeModelData[p] > maxAlignedSizeModelData[p]) ? bufferSize - oldTotalBufferSizeModelData[p] : maxAlignedSizeModelData[p];
                        totalBufferSizeModelData[p] = (alignedOffsetModelData[p] + maxAlignedSizeModelData[p] > bufferSize) ? alignedOffsetModelData[p] + maxAlignedSizeModelData[p] : bufferSize;
                        oldTotalBufferSizeModelData[p] = bufferSize;
                        if (totalBufferSizeModelData[p] > maxBufferSizeModelData[p][currentFrame]) {

                            if (modelDataStagingBufferMT[p][currentFrame] != nullptr) {
                                vkDestroyBuffer(vkDevice.getDevice(), modelDataStagingBufferMT[p][currentFrame], nullptr);
                                vkFreeMemory(vkDevice.getDevice(), modelDataStagingBufferMemoryMT[p][currentFrame], nullptr);
                            }

                            createBuffer(totalBufferSizeModelData[p], VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, modelDataStagingBufferMT[p][currentFrame], modelDataStagingBufferMemoryMT[p][currentFrame]);

                            if (modelDataBufferMT[p][currentFrame] != nullptr) {
                                vkDestroyBuffer(vkDevice.getDevice(), modelDataBufferMT[p][currentFrame], nullptr);
                                vkFreeMemory(vkDevice.getDevice(), modelDataBufferMemoryMT[p][currentFrame], nullptr);
                            }

                            createBuffer(totalBufferSizeModelData[p], VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, modelDataBufferMT[p][currentFrame], modelDataBufferMemoryMT[p][currentFrame]);


                            maxBufferSizeModelData[p][currentFrame] = totalBufferSizeModelData[p];
                            needToUpdateDSs[p][currentFrame]  = true;
                        }


                        /*void* data;
                        vkMapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory, 0, bufferSize, 0, &data);
                        memcpy(data, modelDatas[p].data(), (size_t)bufferSize);
                        vkUnmapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory);*/



                        bufferSize = sizeof(MaterialData) * materialDatas[p].size();

                        currentMaterialOffset[p] = alignedOffsetMaterialData[p] + ((bufferSize - oldTotalBufferSizeMaterialData[p] > 0) ? bufferSize - oldTotalBufferSizeMaterialData[p] : 0);

                        maxAlignedSizeMaterialData[p] = (currentMaterialOffset[p] - oldTotalBufferSizeMaterialData[p] > maxAlignedSizeMaterialData[p]) ? currentMaterialOffset[p] - oldTotalBufferSizeMaterialData[p] : maxAlignedSizeMaterialData[p];
                        totalBufferSizeMaterialData[p] = (alignedOffsetMaterialData[p] + maxAlignedSizeMaterialData[p] > bufferSize) ? alignedOffsetMaterialData[p] + maxAlignedSizeMaterialData[p] : bufferSize;
                        oldTotalBufferSizeMaterialData[p] = bufferSize;
                        if (totalBufferSizeMaterialData[p] > maxBufferSizeMaterialData[p][currentFrame]) {
                            if (materialDataStagingBufferMT[p][currentFrame] != nullptr) {
                                vkDestroyBuffer(vkDevice.getDevice(), materialDataStagingBufferMT[p][currentFrame], nullptr);
                                vkFreeMemory(vkDevice.getDevice(), materialDataStagingBufferMemoryMT[p][currentFrame], nullptr);
                            }
                            createBuffer(totalBufferSizeMaterialData[p], VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, materialDataStagingBufferMT[p][currentFrame], materialDataStagingBufferMemoryMT[p][currentFrame]);

                            if (materialDataBufferMT[p][currentFrame] != nullptr) {
                                vkDestroyBuffer(vkDevice.getDevice(),materialDataBufferMT[p][currentFrame], nullptr);
                                vkFreeMemory(vkDevice.getDevice(), materialDataBufferMemoryMT[p][currentFrame], nullptr);
                            }
                            createBuffer(totalBufferSizeMaterialData[p], VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, materialDataBufferMT[p][currentFrame], materialDataBufferMemoryMT[p][currentFrame]);


                            maxBufferSizeMaterialData[p][currentFrame] = totalBufferSizeMaterialData[p];
                            needToUpdateDSs[p][currentFrame]  = true;
                        }


                        /*vkMapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory, 0, bufferSize, 0, &data);
                        memcpy(data, materialDatas[p].data(), (size_t)bufferSize);
                        vkUnmapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory);*/
                        //copyBuffer(materialDataStagingBuffer,materialDataBufferMT[p], bufferSize, copyMaterialDataBufferCommandBuffer);

                        bufferSize = sizeof(DrawElementsIndirectCommand) * drawElementsIndirectCommands[p].size();
                        totalBufferSizeIndexedDrawCommand[p] = bufferSize;

                        ////std::cout<<"buffer size : "<<bufferSize<<std::endl<<"max : "<<maxBufferSizeIndexedDrawCommand[p]<<std::endl;
                        if (totalBufferSizeIndexedDrawCommand[p] > maxBufferSizeIndexedDrawCommand[p][currentFrame]) {

                            if (vboIndexedIndirectStagingBufferMT[p][currentFrame] != nullptr) {
                                    vkDestroyBuffer(vkDevice.getDevice(), vboIndexedIndirectStagingBufferMT[p][currentFrame], nullptr);
                                    vkFreeMemory(vkDevice.getDevice(), vboIndexedIndirectStagingBufferMemoryMT[p][currentFrame], nullptr);
                                }
                                createBuffer(totalBufferSizeIndexedDrawCommand[p], VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vboIndexedIndirectStagingBufferMT[p][currentFrame], vboIndexedIndirectStagingBufferMemoryMT[p][currentFrame]);

                            if (drawCommandBufferIndexedMT[p][currentFrame] != nullptr) {
                                vkDestroyBuffer(vkDevice.getDevice(),drawCommandBufferIndexedMT[p][currentFrame], nullptr);
                                vkFreeMemory(vkDevice.getDevice(), drawCommandBufferIndexedMemoryMT[p][currentFrame], nullptr);
                            }
                            createBuffer(totalBufferSizeIndexedDrawCommand[p], VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, drawCommandBufferIndexedMT[p][currentFrame], drawCommandBufferIndexedMemoryMT[p][currentFrame]);

                            maxBufferSizeIndexedDrawCommand[p][currentFrame] = totalBufferSizeIndexedDrawCommand[p];
                        }

                    }
                    VkDeviceSize bufferSize = sizeof(ModelData) * modelDatas[p].size();
                    if (bufferSize > 0)
                        copyBuffer(modelDataStagingBufferMT[p][currentFrame], modelDataBufferMT[p][currentFrame], bufferSize, copyModelDataBufferCommandBuffer[currentFrame]);
                    bufferSize = sizeof(MaterialData) * materialDatas[p].size();
                    if (bufferSize > 0)
                        copyBuffer(materialDataStagingBufferMT[p][currentFrame],materialDataBufferMT[p][currentFrame], bufferSize, copyMaterialDataBufferCommandBuffer[currentFrame]);
                    bufferSize = sizeof(DrawElementsIndirectCommand) * drawElementsIndirectCommands[p].size();
                    if (bufferSize > 0)
                        copyBuffer(vboIndexedIndirectStagingBufferMT[p][currentFrame], drawCommandBufferIndexedMT[p][currentFrame], bufferSize, copyDrawIndexedBufferCommandBuffer[currentFrame]);
                    if (vbBindlessTexIndexed[p].getVertexCount() > 0)
                        vbBindlessTexIndexed[p].update(currentFrame, copyVbIndexedBufferCommandBuffer[currentFrame]);

                }
                if (vkEndCommandBuffer(copyModelDataBufferCommandBuffer[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }
                if (vkEndCommandBuffer(copyMaterialDataBufferCommandBuffer[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }
                if (vkEndCommandBuffer(copyDrawIndexedBufferCommandBuffer[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }
                if (vkEndCommandBuffer(copyVbIndexedBufferCommandBuffer[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }
            }
            void ShadowRenderComponent::createCommandPool() {
                window::Device::QueueFamilyIndices queueFamilyIndices = vkDevice.findQueueFamilies(vkDevice.getPhysicalDevice(), VK_NULL_HANDLE);

                VkCommandPoolCreateInfo poolInfo{};
                poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
                poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optionel
                if (vkCreateCommandPool(vkDevice.getDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
                    throw core::Erreur(0, "échec de la création d'une command pool!", 1);
                }
                if (vkCreateCommandPool(vkDevice.getDevice(), &poolInfo, nullptr, &secondaryBufferCommandPool) != VK_SUCCESS) {
                    throw core::Erreur(0, "échec de la création d'une command pool!", 1);
                }
            }
            void ShadowRenderComponent::clear() {
                depthBuffer.clear(Color::Transparent);
                std::vector<VkCommandBuffer> commandBuffers = depthBuffer.getCommandBuffers();
                unsigned int currentFrame = depthBuffer.getCurrentFrame();
                VkClearColorValue clearColor = {0.f, 0.f, 0.f, 0.f};
                VkImageSubresourceRange subresRange = {};
                subresRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                subresRange.levelCount = 1;
                subresRange.layerCount = 1;

                vkCmdClearColorImage(commandBuffers[currentFrame], depthTextureImage[currentFrame], VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &subresRange);
                VkMemoryBarrier memoryBarrier{};
                memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                memoryBarrier.pNext = VK_NULL_HANDLE;
                memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

                alphaBuffer.clear(Color::Transparent);
                commandBuffers = alphaBuffer.getCommandBuffers();
                currentFrame = alphaBuffer.getCurrentFrame();
                vkCmdClearColorImage(commandBuffers[currentFrame], alphaTextureImage[currentFrame], VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &subresRange);

                memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                memoryBarrier.pNext = VK_NULL_HANDLE;
                memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

                stencilBuffer.clear(Color::Transparent);
                commandBuffers = stencilBuffer.getCommandBuffers();
                currentFrame = stencilBuffer.getCurrentFrame();
                vkCmdClearColorImage(commandBuffers[currentFrame], stencilTextureImage[currentFrame], VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &subresRange);

                memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                memoryBarrier.pNext = VK_NULL_HANDLE;
                memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

                shadowMap.beginRecordCommandBuffers();
                const_cast<Texture&>(shadowMap.getTexture(shadowMap.getImageIndex())).toColorAttachmentOptimal(shadowMap.getCommandBuffers()[shadowMap.getCurrentFrame()]);
                shadowMap.clear(Color::White);

            }
            void ShadowRenderComponent::drawBuffers() {
               unsigned int currentFrame = depthBuffer.getCurrentFrame();
               unsigned int texturesInUse = Texture::getAllTextures().size();
               for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {

                    if (modelDatas[p].size() > 0 && texturesInUse > maxTexturesInUse[currentFrame]) {
                        needToUpdateDSs[p][currentFrame];

                    }

                    unsigned int bufferSize = sizeof(ModelData) * modelDatas[p].size();

                    if (bufferSize > 0) {
                        //std::cout<<"size models : "<<bufferSize<<std::endl;
                        void* data;
                        vkMapMemory(vkDevice.getDevice(), modelDataStagingBufferMemoryMT[p][currentFrame], 0, bufferSize, 0, &data);
                        memcpy(data, modelDatas[p].data(), (size_t)bufferSize);
                        vkUnmapMemory(vkDevice.getDevice(), modelDataStagingBufferMemoryMT[p][currentFrame]);
                    }
                    bufferSize = sizeof(MaterialData) * materialDatas[p].size();

                    if (bufferSize > 0) {
                        //std::cout<<"size materials : "<<bufferSize<<std::endl;
                        void* data;
                        vkMapMemory(vkDevice.getDevice(), materialDataStagingBufferMemoryMT[p][currentFrame], 0, bufferSize, 0, &data);
                        memcpy(data, materialDatas[p].data(), (size_t)bufferSize);
                        vkUnmapMemory(vkDevice.getDevice(), materialDataStagingBufferMemoryMT[p][currentFrame]);
                    }
                    bufferSize = sizeof(DrawArraysIndirectCommand) * drawArraysIndirectCommands[p].size();
                    if (bufferSize > 0) {
                        ////std::cout<<"size draw arrays : "<<bufferSize<<std::endl;
                        void* data;
                        vkMapMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemoryMT[p][currentFrame], 0, bufferSize, 0, &data);
                        memcpy(data, drawArraysIndirectCommands[p].data(), (size_t)bufferSize);
                        vkUnmapMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemoryMT[p][currentFrame]);
                    }
                    bufferSize = sizeof(DrawElementsIndirectCommand) * drawElementsIndirectCommands[p].size();
                    if (bufferSize > 0) {
                        //std::cout<<"size draw elements : "<<bufferSize<<std::endl;
                        void* data;
                        vkMapMemory(vkDevice.getDevice(), vboIndexedIndirectStagingBufferMemoryMT[p][currentFrame], 0, bufferSize, 0, &data);
                        memcpy(data, drawElementsIndirectCommands[p].data(), (size_t)bufferSize);
                        vkUnmapMemory(vkDevice.getDevice(), vboIndexedIndirectStagingBufferMemoryMT[p][currentFrame]);
                    }
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        ////std::cout<<"size vb : "<<vbBindlessTex[p].getVertexCount()<<std::endl;
                        vbBindlessTex[p].updateStagingBuffers(currentFrame);
                    }
                    if (vbBindlessTexIndexed[p].getVertexCount() > 0) {
                        //std::cout<<"size vb indexed : "<<vbBindlessTexIndexed[p].getIndicesSize()<<std::endl;
                        vbBindlessTexIndexed[p].updateStagingBuffers(currentFrame);
                    }
                }
                maxTexturesInUse[currentFrame] = texturesInUse;
                VkCommandBufferInheritanceInfo inheritanceInfo{};

                VkCommandBufferBeginInfo beginInfo{};

                inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                inheritanceInfo.renderPass = depthBuffer.getRenderPass(1);
                inheritanceInfo.framebuffer = VK_NULL_HANDLE;
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.pInheritanceInfo = &inheritanceInfo;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
                vkResetCommandBuffer(depthCommandBuffer[currentFrame], 0);
                if (vkBeginCommandBuffer(depthCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }

                RenderStates currentStates;
                currentStates.blendMode = BlendNone;
                currentStates.shader = &depthGenShader;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes-1; p++) {
                    if (needToUpdateDSs[p][currentFrame])
                        updateDescriptorSets(currentFrame, p, currentStates);
                    if (nbDrawCommandBuffer[p][0] > 0) {
                        recordCommandBufferIndirect(currentFrame, p, nbDrawCommandBuffer[p][0], sizeof(DrawArraysIndirectCommand), SHADOWNODEPTHNOSTENCIL, 0, -1, -1, modelDataOffsets[p][0], materialDataOffsets[p][0],drawCommandBufferOffsets[p][0], currentStates, depthCommandBuffer[currentFrame]);
                    }
                    if (nbIndexedDrawCommandBuffer[p][0] > 0) {
                        recordCommandBufferIndirect(currentFrame, p, nbIndexedDrawCommandBuffer[p][0], sizeof(DrawElementsIndirectCommand), SHADOWNODEPTHNOSTENCIL, 0, 0, -1, modelDataOffsets[p][1], materialDataOffsets[p][1],drawIndexedCommandBufferOffsets[p][0], currentStates, depthCommandBuffer[currentFrame]);
                    }

                }

                if (vkEndCommandBuffer(depthCommandBuffer[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }

                inheritanceInfo.renderPass = stencilBuffer.getRenderPass(1);
                inheritanceInfo.framebuffer = VK_NULL_HANDLE;
                beginInfo.pInheritanceInfo = &inheritanceInfo;
                vkResetCommandBuffer(stencilCommandBuffer[currentFrame], 0);
                if (vkBeginCommandBuffer(stencilCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }
                currentStates.blendMode = BlendNone;
                currentStates.shader = &buildShadowMapShader;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes-1; p++) {
                    if (needToUpdateDSs[p][currentFrame])
                        updateDescriptorSets(currentFrame, p, currentStates);
                    if (nbDrawCommandBuffer[p][0] > 0) {
                        recordCommandBufferIndirect(currentFrame, p, nbDrawCommandBuffer[p][0], sizeof(DrawArraysIndirectCommand), SHADOWNODEPTHNOSTENCIL, 0, -1, -1, modelDataOffsets[p][0], materialDataOffsets[p][0],drawCommandBufferOffsets[p][0], currentStates, stencilCommandBuffer[currentFrame]);
                    }
                    if (nbIndexedDrawCommandBuffer[p][0] > 0) {
                        recordCommandBufferIndirect(currentFrame, p, nbIndexedDrawCommandBuffer[p][0], sizeof(DrawElementsIndirectCommand), SHADOWNODEPTHNOSTENCIL, 0, 0, -1, modelDataOffsets[p][1], materialDataOffsets[p][1],drawIndexedCommandBufferOffsets[p][0], currentStates, stencilCommandBuffer[currentFrame]);
                    }

                }
                if (vkEndCommandBuffer(stencilCommandBuffer[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }

                inheritanceInfo.renderPass = alphaBuffer.getRenderPass(1);
                inheritanceInfo.framebuffer = VK_NULL_HANDLE;
                beginInfo.pInheritanceInfo = &inheritanceInfo;
                vkResetCommandBuffer(alphaCommandBuffer[currentFrame], 0);
                if (vkBeginCommandBuffer(alphaCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }

                currentStates.shader = &sBuildAlphaBufferShader;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes-1; p++) {
                    if (needToUpdateDSs[p][currentFrame])
                        updateDescriptorSets(currentFrame, p, currentStates);
                    if (nbDrawCommandBuffer[p][1] > 0) {
                        recordCommandBufferIndirect(currentFrame, p, nbDrawCommandBuffer[p][1], sizeof(DrawArraysIndirectCommand), SHADOWNODEPTHNOSTENCIL, 0, -1, -1, modelDataOffsets[p][2], materialDataOffsets[p][2],drawCommandBufferOffsets[p][1], currentStates, alphaCommandBuffer[currentFrame]);
                    }
                    if (nbIndexedDrawCommandBuffer[p][1] > 0) {
                        recordCommandBufferIndirect(currentFrame, p, nbIndexedDrawCommandBuffer[p][1], sizeof(DrawElementsIndirectCommand), SHADOWNODEPTHNOSTENCIL, 0, 0, -1, modelDataOffsets[p][3], materialDataOffsets[p][3],drawIndexedCommandBufferOffsets[p][1], currentStates, alphaCommandBuffer[currentFrame]);
                    }
                }
                if (vkEndCommandBuffer(alphaCommandBuffer[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }

                inheritanceInfo.renderPass = shadowMap.getRenderPass(1);
                inheritanceInfo.framebuffer = VK_NULL_HANDLE;
                beginInfo.pInheritanceInfo = &inheritanceInfo;
                vkResetCommandBuffer(shadowCommandBuffer[currentFrame], 0);
                if (vkBeginCommandBuffer(shadowCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }
                currentStates.blendMode = BlendMultiply;
                currentStates.shader = &perPixShadowShader;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes-1; p++) {
                    if (needToUpdateDSs[p][currentFrame])
                        updateDescriptorSets(currentFrame, p, currentStates);
                    if (nbDrawCommandBuffer[p][1] > 0) {
                        recordCommandBufferIndirect(currentFrame, p, nbDrawCommandBuffer[p][1], sizeof(DrawArraysIndirectCommand), SHADOWNODEPTHNOSTENCIL, 0, -1, -1, modelDataOffsets[p][2], materialDataOffsets[p][2],drawCommandBufferOffsets[p][1], currentStates, shadowCommandBuffer[currentFrame]);
                    }
                    if (nbIndexedDrawCommandBuffer[p][1] > 0) {
                        recordCommandBufferIndirect(currentFrame, p, nbIndexedDrawCommandBuffer[p][1], sizeof(DrawElementsIndirectCommand), SHADOWNODEPTHNOSTENCIL, 0, 0, -1, modelDataOffsets[p][3], materialDataOffsets[p][3],drawIndexedCommandBufferOffsets[p][1], currentStates, shadowCommandBuffer[currentFrame]);
                    }

                }
                if (vkEndCommandBuffer(shadowCommandBuffer[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }

                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes-1; p++)
                    needToUpdateDSs[p][currentFrame] = false;
            }
            void ShadowRenderComponent::drawNextFrame() {
                //glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));

                {
                    std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                    if (datasReady) {
                        datasReady = false;
                        m_instances = batcher.getInstances();
                        m_normals = normalBatcher.getInstances();
                        m_shadow_instances = shadowBatcher.getInstances();
                        m_shadow_normals = normalShadowBatcher.getInstances();
                        m_instancesIndexed = batcherIndexed.getInstances();
                        m_shadow_instances_indexed = shadowBatcherIndexed.getInstances();
                        m_normalsIndexed = normalBatcherIndexed.getInstances();
                        m_shadow_normalsIndexed = normalShadowBatcherIndexed.getInstances();
                        /*m_stencil_buffer = normalStencilBuffer.getInstances();*/
                    }
                }

                math::Vec3f centerLight = g2d::AmbientLight::getAmbientLight().getLightCenter();
                lightView = View(g2d::AmbientLight::getAmbientLight().getRadius(), g2d::AmbientLight::getAmbientLight().getRadius(), 1.0f, g2d::AmbientLight::getAmbientLight().getHeight());
                lightView.reset(physic::BoundingBox(0, 0, 1.f, 1024, 1024, g2d::AmbientLight::getAmbientLight().getHeight()));

                lightView.setCenter(centerLight);
                resolutionPC.near = view.getViewport().getPosition().z();
                resolutionPC.far = view.getViewport().getSize().z();
                lightView.lookAt(0, 0, 0, math::Vec3f(0, 1, 0));

                stencilBuffer.setView(lightView);
                /*depthBuffer.setView(view);*/
                math::Matrix4f lviewMatrix = lightView.getViewMatrix().getMatrix()/*.transpose()*/;
                //std::cout<<lviewMatrix<<std::endl;

                math::Matrix4f lprojMatrix = lightView.getProjMatrix().getMatrix()/*.transpose()*/;
                /*std::cout<<"light proj : "<<lprojMatrix<<std::endl;
                std::cout<<"light view : "<<lviewMatrix<<std::endl;*/

                lightIndirectRenderingPC.projectionMatrix = toVulkanMatrix(lprojMatrix);
                lightIndirectRenderingPC.viewMatrix = toVulkanMatrix(lviewMatrix);

                math::Matrix4f viewMatrix = view.getViewMatrix().getMatrix()/*.transpose()*/;
                math::Matrix4f projMatrix = view.getProjMatrix().getMatrix()/*.transpose()*/;
                math::Matrix4f viewportMatrix = view.getViewMatrix().getMatrix();
                indirectRenderingPC.projectionMatrix = toVulkanMatrix(projMatrix);
                indirectRenderingPC.viewMatrix = toVulkanMatrix(viewMatrix);


                shadowUBODatas.projectionMatrix = toVulkanMatrix(projMatrix);
                shadowUBODatas.viewMatrix = toVulkanMatrix(viewMatrix);
                shadowUBODatas.lprojectionMatrix = toVulkanMatrix(lprojMatrix);
                shadowUBODatas.lviewMatrix = toVulkanMatrix(lviewMatrix);
                shadowUBODatas.viewportMatrix = toVulkanMatrix(viewportMatrix);
                shadowUBODatas.lightCenter = math::Vec4f(g2d::AmbientLight::getAmbientLight().getLightCenter().x(),
                                                         g2d::AmbientLight::getAmbientLight().getLightCenter().y(),
                                                         g2d::AmbientLight::getAmbientLight().getLightCenter().z(),
                                                         g2d::AmbientLight::getAmbientLight().getRadius());
                VkDeviceSize bufferSize = sizeof(shadowUBODatas);
                void* data;
                unsigned int currentFrame = depthBuffer.getCurrentFrame();
                vkMapMemory(vkDevice.getDevice(), shadowUBOMemory[currentFrame], 0, bufferSize, 0, &data);
                memcpy(data, &shadowUBODatas, (size_t)bufferSize);
                vkUnmapMemory(vkDevice.getDevice(), shadowUBOMemory[currentFrame]);

                if (useThread) {
                    std::unique_lock<std::mutex> lock(mtx);
                    cv.wait(lock, [this](){return registerFrameJob[depthBuffer.getCurrentFrame()].load() || stop.load();});
                    registerFrameJob[currentFrame] = false;
                    resetBuffers();
                    fillBuffersMT();
                    fillIndexedBuffersMT();
                    fillShadowBuffersMT();
                    fillShadowIndexedBuffersMT();
                    drawBuffers();
                    commandBufferReady[currentFrame] = true;
                    cv.notify_one();
                } else {
                    drawInstanced();
                    drawInstancedIndexed();
                    if (!isSomethingDrawn) {
                        depthBuffer.beginRecordCommandBuffers();
                        std::vector<VkSemaphore> waitSemaphores;
                        waitSemaphores.push_back(offscreenFinishedSemaphore[depthBuffer.getCurrentFrame()]);
                        std::vector<VkPipelineStageFlags> waitStages;
                        waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                        std::vector<uint64_t> waitValues;
                        waitValues.push_back(values[depthBuffer.getCurrentFrame()]);
                        std::vector<VkSemaphore> signalSemaphores;
                        signalSemaphores.push_back(offscreenDepthFinishedSemaphore[depthBuffer.getCurrentFrame()]);
                        std::vector<uint64_t> signalValues;
                        values2[depthBuffer.getCurrentFrame()]++;
                        signalValues.push_back(values2[depthBuffer.getCurrentFrame()]);
                        depthBuffer.submit(true, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);
                        shadowMap.beginRecordCommandBuffers();
                        waitSemaphores.clear();
                        waitStages.clear();
                        signalSemaphores.clear();
                        waitValues.clear();
                        signalValues.clear();
                        waitSemaphores.push_back(offscreenDepthFinishedSemaphore[shadowMap.getCurrentFrame()]);
                        waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                        waitValues.push_back(values2[depthBuffer.getCurrentFrame()]);
                        values[shadowMap.getCurrentFrame()]++;
                        signalSemaphores.push_back(offscreenFinishedSemaphore[shadowMap.getCurrentFrame()]);
                        signalValues.push_back(values[shadowMap.getCurrentFrame()]);
                        shadowMap.submit(true, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);
                    }
                    isSomethingDrawn = false;
                }
            }
            void ShadowRenderComponent::pushEvent(window::IEvent event, RenderWindow& rw) {
                if (&rw == &window && event.type == window::IEvent::WINDOW_EVENT && event.window.type == window::IEvent::WINDOW_EVENT_CLOSED) {
                    stop = true;
                    cv.notify_all();
                    getListener().stop();
                }
            }
            bool ShadowRenderComponent::needToUpdate() {
                return update;
            }
            std::string ShadowRenderComponent::getExpression() {
                return expression;
            }
            void ShadowRenderComponent::setExpression (std::string expression) {
                this->expression = expression;
            }
            void ShadowRenderComponent::setView(View view) {
                depthBuffer.setView(view);
                alphaBuffer.setView(view);
                shadowMap.setView(view);
                this->view = view;
            }
            View& ShadowRenderComponent::getView() {
                return view;
            }
            RenderTexture* ShadowRenderComponent::getFrameBuffer() {
                return &shadowMap;
            }
            int ShadowRenderComponent::getLayer() {
                return getPosition().z();
            }
            void ShadowRenderComponent::draw(RenderTarget& target, RenderStates states) {
                if (useThread) {
                    std::unique_lock<std::mutex> lock(mtx);
                    cv.wait(lock, [this] { return commandBufferReady[depthBuffer.getCurrentFrame()].load() || stop.load(); });

                    commandBufferReady[depthBuffer.getCurrentFrame()] = false;
                    depthBuffer.beginRecordCommandBuffers();
                    std::vector<VkCommandBuffer> commandBuffers = depthBuffer.getCommandBuffers();
                    unsigned int currentFrame = depthBuffer.getCurrentFrame();
                    vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &copyModelDataBufferCommandBuffer[currentFrame]);

                    vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &copyMaterialDataBufferCommandBuffer[currentFrame]);

                    vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &copyDrawBufferCommandBuffer[currentFrame]);
                    vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &copyVbBufferCommandBuffer[currentFrame]);

                    vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &copyDrawIndexedBufferCommandBuffer[currentFrame]);
                    vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &copyVbIndexedBufferCommandBuffer[currentFrame]);
                    for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                        VkBufferMemoryBarrier buffersMemoryBarrier{};
                        buffersMemoryBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
                        buffersMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                        buffersMemoryBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
                        buffersMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                        buffersMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                        buffersMemoryBarrier.offset = 0;
                        buffersMemoryBarrier.size = VK_WHOLE_SIZE;
                        if (vbBindlessTex[p].getVertexBuffer(currentFrame) != nullptr) {
                            buffersMemoryBarrier.buffer = vbBindlessTex[p].getVertexBuffer(currentFrame);
                            vkCmdPipelineBarrier(
                            commandBuffers[currentFrame],
                            VK_PIPELINE_STAGE_TRANSFER_BIT,
                            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                            0,
                            0, nullptr,
                            1, &buffersMemoryBarrier,
                            0, nullptr
                            );
                        }
                        if (vbBindlessTexIndexed[p].getVertexBuffer(currentFrame) != nullptr && vbBindlessTexIndexed[p].getIndexBuffer(currentFrame) != nullptr) {

                            buffersMemoryBarrier.buffer = vbBindlessTexIndexed[p].getVertexBuffer(currentFrame);
                            vkCmdPipelineBarrier(
                            commandBuffers[currentFrame],
                            VK_PIPELINE_STAGE_TRANSFER_BIT,
                            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                            0,
                            0, nullptr,
                            1, &buffersMemoryBarrier,
                            0, nullptr
                            );
                            buffersMemoryBarrier.buffer = vbBindlessTexIndexed[p].getIndexBuffer(currentFrame);
                            vkCmdPipelineBarrier(
                            commandBuffers[currentFrame],
                            VK_PIPELINE_STAGE_TRANSFER_BIT,
                            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                            0,
                            0, nullptr,
                            1, &buffersMemoryBarrier,
                            0, nullptr
                            );
                        }
                        buffersMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                        if (modelDataBufferMT[p][currentFrame] != nullptr) {
                            buffersMemoryBarrier.buffer = modelDataBufferMT[p][currentFrame];
                            vkCmdPipelineBarrier(
                            commandBuffers[currentFrame],
                            VK_PIPELINE_STAGE_TRANSFER_BIT,
                            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                            0,
                            0, nullptr,
                            1, &buffersMemoryBarrier,
                            0, nullptr
                            );
                        }
                        if (materialDataBufferMT[p][currentFrame] != nullptr) {
                            buffersMemoryBarrier.buffer = materialDataBufferMT[p][currentFrame];
                            vkCmdPipelineBarrier(
                            commandBuffers[currentFrame],
                            VK_PIPELINE_STAGE_TRANSFER_BIT,
                            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                            0,
                            0, nullptr,
                            1, &buffersMemoryBarrier,
                            0, nullptr
                            );
                        }
                        buffersMemoryBarrier.dstAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
                        if (drawCommandBufferMT[p][currentFrame] != nullptr) {
                            buffersMemoryBarrier.buffer = drawCommandBufferMT[p][currentFrame];
                            vkCmdPipelineBarrier(
                            commandBuffers[currentFrame],
                            VK_PIPELINE_STAGE_TRANSFER_BIT,
                            VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
                            0,
                            0, nullptr,
                            1, &buffersMemoryBarrier,
                            0, nullptr
                            );
                        }
                        if (drawCommandBufferIndexedMT[p][currentFrame] != nullptr) {
                            buffersMemoryBarrier.buffer = drawCommandBufferIndexedMT[p][currentFrame];
                            vkCmdPipelineBarrier(
                            commandBuffers[currentFrame],
                            VK_PIPELINE_STAGE_TRANSFER_BIT,
                            VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
                            0,
                            0, nullptr,
                            1, &buffersMemoryBarrier,
                            0, nullptr
                            );
                        }
                    }
                    std::vector<VkSemaphore> signalSemaphores;
                    signalSemaphores.push_back(copyFinishedSemaphore[currentFrame]);
                    std::vector<VkSemaphore> waitSemaphores;
                    waitSemaphores.push_back(offscreenFinishedSemaphore[shadowMap.getCurrentFrame()]);
                    std::vector<VkPipelineStageFlags> waitStages;
                    waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                    std::vector<uint64_t> signalValues;
                    std::vector<uint64_t> waitValues;
                    waitValues.push_back(values[shadowMap.getCurrentFrame()]);
                    copyValues[currentFrame]++;
                    signalValues.push_back(copyValues[currentFrame]);
                    depthBuffer.submit(false, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);


                    depthBuffer.beginRecordCommandBuffers();
                    depthBuffer.beginRenderPass();
                    vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &depthCommandBuffer[currentFrame]);
                    depthBuffer.endRenderPass();

                    waitSemaphores.clear();
                    waitSemaphores.push_back(copyFinishedSemaphore[currentFrame]);
                    waitStages.clear();
                    waitStages.push_back(VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
                    waitValues.clear();
                    waitValues.push_back(copyValues[depthBuffer.getCurrentFrame()]);
                    signalSemaphores.clear();
                    signalSemaphores.push_back(offscreenDepthFinishedSemaphore[shadowMap.getCurrentFrame()]);
                    values2[stencilBuffer.getCurrentFrame()]++;
                    signalValues.clear();
                    signalValues.push_back(values2[depthBuffer.getCurrentFrame()]);
                    depthBuffer.submit(true, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);



                    stencilBuffer.beginRecordCommandBuffers();
                    commandBuffers = stencilBuffer.getCommandBuffers();
                    currentFrame = stencilBuffer.getCurrentFrame();
                    stencilBuffer.beginRenderPass();
                    vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &stencilCommandBuffer[currentFrame]);
                    stencilBuffer.endRenderPass();

                    values[stencilBuffer.getCurrentFrame()]++;
                    signalValues.clear();
                    signalValues.push_back(values[stencilBuffer.getCurrentFrame()]);
                    signalSemaphores.clear();
                    signalSemaphores.push_back(offscreenFinishedSemaphore[shadowMap.getCurrentFrame()]);
                    waitSemaphores.clear();
                    waitSemaphores.push_back(copyFinishedSemaphore[depthBuffer.getCurrentFrame()]);
                    waitStages.clear();
                    waitStages.push_back(VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
                    waitValues.clear();
                    waitValues.push_back(copyValues[depthBuffer.getCurrentFrame()]);
                    stencilBuffer.submit(true, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);



                    alphaBuffer.beginRecordCommandBuffers();

                    const_cast<Texture&>(depthBuffer.getTexture(depthBuffer.getImageIndex())).toShaderReadOnlyOptimal(alphaBuffer.getCommandBuffers()[alphaBuffer.getCurrentFrame()]);


                    commandBuffers = alphaBuffer.getCommandBuffers();
                    currentFrame = alphaBuffer.getCurrentFrame();
                    BlendMode blendNone = BlendNone;
                    blendNone.updateIds();
                    for (unsigned int p = 0; p < (Batcher::nbPrimitiveTypes -1); p++) {
                        if (nbDrawCommandBuffer[p][0] > 0 || nbIndexedDrawCommandBuffer[p][0] > 0) {
                            //std::cout<<"draw ids : "<<(sBuildAlphaBufferShader.getId() * (Batcher::nbPrimitiveTypes - 1) + p)<<","<<(SHADOWNODEPTHNOSTENCIL*blendNone.nbBlendModes+blendNone.id)<<std::endl;
                            vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS,alphaBuffer.getGraphicPipeline()[sBuildAlphaBufferShader.getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][SHADOWNODEPTHNOSTENCIL*blendNone.nbBlendModes+blendNone.id]);
                            vkCmdPushConstants(commandBuffers[currentFrame], alphaBuffer.getPipelineLayout()[sBuildAlphaBufferShader.getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][SHADOWNODEPTHNOSTENCIL*blendNone.nbBlendModes+blendNone.id], VK_SHADER_STAGE_FRAGMENT_BIT, 20, sizeof(unsigned int), &alphaBuffer.getImageIndex());
                        }
                    }
                    VkMemoryBarrier memoryBarrier={};
                    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                    memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                    memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                    //vkCmdWaitEvents(commandBuffers[currentFrame], 1, &events[currentFrame], VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                    vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

                    alphaBuffer.beginRenderPass();
                    vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &alphaCommandBuffer[currentFrame]);
                    alphaBuffer.endRenderPass();
                    const_cast<Texture&>(depthBuffer.getTexture(depthBuffer.getImageIndex())).toColorAttachmentOptimal(alphaBuffer.getCommandBuffers()[alphaBuffer.getCurrentFrame()]);
                    waitSemaphores.clear();
                    waitSemaphores.push_back(offscreenDepthFinishedSemaphore[shadowMap.getCurrentFrame()]);
                    waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                    waitValues.clear();
                    waitValues.push_back(values2[depthBuffer.getCurrentFrame()]);
                    signalSemaphores.clear();
                    signalSemaphores.push_back(offscreenAlphaFinishedSemaphore[alphaBuffer.getCurrentFrame()]);
                    signalValues.clear();
                    alphaBuffer.submit(true, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);

                    shadowMap.beginRecordCommandBuffers();

                    const_cast<Texture&>(stencilBuffer.getTexture(stencilBuffer.getImageIndex())).toShaderReadOnlyOptimal(shadowMap.getCommandBuffers()[shadowMap.getCurrentFrame()]);
                    const_cast<Texture&>(alphaBuffer.getTexture(alphaBuffer.getImageIndex())).toShaderReadOnlyOptimal(shadowMap.getCommandBuffers()[shadowMap.getCurrentFrame()]);
                    const_cast<Texture&>(depthBuffer.getTexture(depthBuffer.getImageIndex())).toShaderReadOnlyOptimal(shadowMap.getCommandBuffers()[shadowMap.getCurrentFrame()]);


                    commandBuffers = shadowMap.getCommandBuffers();
                    currentFrame = shadowMap.getCurrentFrame();

                    for (unsigned int p = 0; p < (Batcher::nbPrimitiveTypes -1); p++) {

                        if (nbDrawCommandBuffer[p][1] > 0 || nbIndexedDrawCommandBuffer[p][1] > 0) {
                            vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS,shadowMap.getGraphicPipeline()[perPixShadowShader.getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][SHADOWNODEPTHNOSTENCIL*blendNone.nbBlendModes+blendNone.id]);
                            vkCmdPushConstants(commandBuffers[currentFrame], shadowMap.getPipelineLayout()[perPixShadowShader.getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][SHADOWNODEPTHNOSTENCIL*blendNone.nbBlendModes+blendNone.id], VK_SHADER_STAGE_FRAGMENT_BIT, 24, sizeof(unsigned int), &shadowMap.getImageIndex());
                        }

                    }






                    vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);

                    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                    memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                    memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                    //vkCmdWaitEvents(commandBuffers[currentFrame], 1, &events[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

                    vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

                    shadowMap.beginRenderPass();
                    vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &shadowCommandBuffer[currentFrame]);
                    shadowMap.endRenderPass();

                    const_cast<Texture&>(alphaBuffer.getTexture(alphaBuffer.getImageIndex())).toColorAttachmentOptimal(shadowMap.getCommandBuffers()[shadowMap.getCurrentFrame()]);
                    const_cast<Texture&>(depthBuffer.getTexture(depthBuffer.getImageIndex())).toColorAttachmentOptimal(shadowMap.getCommandBuffers()[shadowMap.getCurrentFrame()]);
                    const_cast<Texture&>(stencilBuffer.getTexture(stencilBuffer.getImageIndex())).toColorAttachmentOptimal(shadowMap.getCommandBuffers()[shadowMap.getCurrentFrame()]);

                    waitSemaphores.push_back(offscreenAlphaFinishedSemaphore[alphaBuffer.getCurrentFrame()]);
                    waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                    waitValues.push_back(0);
                    signalSemaphores.clear();
                    signalSemaphores.push_back(offscreenFinishedSemaphore[shadowMap.getCurrentFrame()]);
                    values[shadowMap.getCurrentFrame()]++;
                    signalValues.push_back(values[shadowMap.getCurrentFrame()]);
                    shadowMap.submit(true, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);
                }
                target.beginRecordCommandBuffers();
                const_cast<Texture&>(shadowMap.getTexture(shadowMap.getImageIndex())).toShaderReadOnlyOptimal(window.getCommandBuffers()[window.getCurrentFrame()]);

                shadowTile.setCenter(target.getView().getPosition());


                stencilBufferTile.setTexture(shadowMap.getTexture(shadowMap.getImageIndex()));

                states.blendMode = BlendMultiply;
                target.draw(shadowTile, states);
                /*if (&target == &window)
                    window.endRenderPass();*/
                std::vector<VkSemaphore> waitSemaphores, signalSemaphores;
                std::vector<VkPipelineStageFlags> waitStages;
                std::vector<uint64_t> waitValues, signalValues;
                waitSemaphores.push_back(offscreenFinishedSemaphore[shadowMap.getCurrentFrame()]);
                waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                signalSemaphores.push_back(offscreenFinishedSemaphore[shadowMap.getCurrentFrame()]);
                waitValues.push_back(values[shadowMap.getCurrentFrame()]);
                values[shadowMap.getCurrentFrame()]++;
                signalValues.push_back(values[shadowMap.getCurrentFrame()]);
                window.submit(false, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);
                depthBuffer.display();
                alphaBuffer.display();
                stencilBuffer.display();
                shadowMap.display();
                registerFrameJob[depthBuffer.getCurrentFrame()] = true;
                cv.notify_one();

            }
            void ShadowRenderComponent::loadTextureIndexes() {
            }
            void ShadowRenderComponent::onVisibilityChanged(bool visible) {
            }
            Entity* ShadowRenderComponent::getShadow(Entity* entity) {
                //std::cout<<"get shadow"<<std::endl;
                math::Vec3f shadowOrigin, shadowCenter, shadowScale(1.f, 1.f, 1.f), shadowRotationAxis, shadowTranslation;
                float shadowRotationAngle = 0;
                shadowCenter = entity->getRootEntity()->getShadowCenter();
                shadowScale = entity->getRootEntity()->getShadowScale();
                shadowRotationAxis = entity->getRootEntity()->getShadowRotationAxis();
                shadowRotationAngle = entity->getRootEntity()->getShadowRotationAngle();
                shadowOrigin = entity->getRootEntity()->getPosition();
                shadowTranslation = entity->getRootEntity()->getPosition() + shadowCenter;
                //std::cout<<"shadow infos get"<<std::endl;
                TransformMatrix tm;
                tm.setOrigin(shadowOrigin);
                tm.setScale(shadowScale);
                tm.setRotation(shadowRotationAxis, shadowRotationAngle);
                tm.setTranslation(shadowTranslation);
                tm.combine(entity->getTransform().getMatrix());
                for (unsigned int i = 0; i < shadows.size(); i++) {
                    if (shadows[i]->getShadowId() == entity->getId()) {

                        shadows[i]->getRootEntity()->setTransform(tm.getMatrix());
                        return shadows[i].get();
                    }
                }

                //std::cout<<"clone"<<std::endl;
                Entity* shadow = entity->clone();
                //std::cout<<"cloned"<<std::endl;
                shadow->setShadowId(entity->getId());
                shadow->getRootEntity()->setTransform(tm.getMatrix());
                std::unique_ptr<Entity> ptr;
                ptr.reset(shadow);
                shadows.push_back(std::move(ptr));
                //std::cout<<"shadow pushed"<<std::endl;
                return shadow;
            }
            bool ShadowRenderComponent::loadEntitiesOnComponent(std::vector<Entity*> vEntities)
            {
                {
                    std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                    datasReady = false;
                    batcher.clear();
                    normalBatcher.clear();
                    shadowBatcher.clear();
                    normalShadowBatcher.clear();
                    batcherIndexed.clear();
                    shadowBatcherIndexed.clear();
                    normalBatcherIndexed.clear();
                    normalShadowBatcherIndexed.clear();
                    //normalStencilBuffer.clear();

                }



                for (unsigned int i = 0; i < vEntities.size(); i++) {
                    std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                    if ( vEntities[i] != nullptr && vEntities[i]->isLeaf()) {

                        //Entity* shadow = getShadow(vEntities[i]);
                        //std::cout<<"shadow get"<<std::endl;
                        Entity* entity = vEntities[i]->getRootEntity();
                        math::Vec3f shadowOrigin, shadowCenter, shadowScale(1.f, 1.f, 1.f), shadowRotationAxis, shadowTranslation;
                        float shadowRotationAngle = 0;
                        //if (entity != nullptr && entity->isModel()) {
                            shadowCenter = entity->getShadowCenter();
                            shadowScale = entity->getShadowScale();
                            shadowRotationAxis = entity->getShadowRotationAxis();
                            shadowRotationAngle = entity->getShadowRotationAngle();
                            shadowOrigin = entity->getPosition();
                            shadowTranslation = entity->getPosition() + shadowCenter;
                            //if (entity->getType() == "E_WALL") {
                                //////std::cout<<"shadow center : "<<shadowCenter<<std::endl;
                                //////std::cout<<"shadow scale : "<<shadowScale<<std::endl;
                                //////std::cout<<"shadow rotation axis : "<<shadowRotationAxis<<std::endl;
                                //////std::cout<<"shadow rotation angle : "<<shadowRotationAngle<<std::endl;
                                //////std::cout<<"shadow origin : "<<shadowOrigin<<std::endl;
                                //////std::cout<<"shadow translation : "<<shadowTranslation<<std::endl;
                            //}
                        //}
                        TransformMatrix tm;
                        tm.setOrigin(shadowOrigin);
                        tm.setScale(shadowScale);
                        tm.setRotation(shadowRotationAxis, shadowRotationAngle);
                        tm.setTranslation(shadowTranslation);

                        for (unsigned int j = 0; j <  vEntities[i]->getNbFaces(); j++) {

                             if(vEntities[i]->getDrawMode() == Entity::INSTANCED) {
                                if (vEntities[i]->getFace(j)->getVertexArray().getIndexes().size() == 0) {

                                    batcher.addFace( vEntities[i]->getFace(j));
                                    shadowBatcher.addShadowFace(vEntities[i]->getFace(j), view.getViewMatrix(), tm);
                                } else {

                                    batcherIndexed.addFace( vEntities[i]->getFace(j));
                                    shadowBatcherIndexed.addShadowFace(vEntities[i]->getFace(j), view.getViewMatrix(), tm);
                                }
                             } else {
                                 if (vEntities[i]->getFace(j)->getVertexArray().getIndexes().size() == 0) {

                                    normalBatcher.addFace( vEntities[i]->getFace(j));
                                    normalShadowBatcher.addShadowFace(vEntities[i]->getFace(j), view.getViewMatrix(), tm);
                                    /*if (vEntities[i]->getRootEntity()->getType() != "E_BIGTILE") {
                                        std::lock_guard<std::recursive_mutex> lock(rec_mutex);

                                        normalStencilBuffer.addFace(vEntities[i]->getFace(j));
                                    }*/
                                 } else {
                                    ////std::cout<<"add shadow indexes"<<std::endl;

                                    normalBatcherIndexed.addFace( vEntities[i]->getFace(j));
                                    normalShadowBatcherIndexed.addShadowFace(vEntities[i]->getFace(j), view.getViewMatrix(), tm);
                                 }
                             }
                        }
                    }
                }
                std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                visibleEntities = vEntities;
                datasReady = true;
                return true;
            }
            std::vector<Entity*> ShadowRenderComponent::getEntities() {
                return visibleEntities;
            }
            ShadowRenderComponent::~ShadowRenderComponent() {
                vkDestroyCommandPool(vkDevice.getDevice(), commandPool, nullptr);
                for (unsigned int i = 0; i < offscreenDepthFinishedSemaphore.size(); i++) {
                    vkDestroySemaphore(vkDevice.getDevice(), offscreenDepthFinishedSemaphore[i], nullptr);
                }
                for (unsigned int i = 0; i < offscreenAlphaFinishedSemaphore.size(); i++) {
                    vkDestroySemaphore(vkDevice.getDevice(), offscreenAlphaFinishedSemaphore[i], nullptr);
                }
                for (unsigned int i = 0; i < offscreenFinishedSemaphore.size(); i++) {
                    vkDestroySemaphore(vkDevice.getDevice(), offscreenFinishedSemaphore[i], nullptr);
                }
                for (unsigned int i = 0; i < copyFinishedSemaphore.size(); i++) {
                    vkDestroySemaphore(vkDevice.getDevice(), copyFinishedSemaphore[i], nullptr);
                }
                for (size_t i = 0; i < modelDataShaderStorageBuffers.size(); i++) {
                    vkDestroyBuffer(vkDevice.getDevice(), modelDataShaderStorageBuffers[i], nullptr);
                    vkFreeMemory(vkDevice.getDevice(), modelDataShaderStorageBuffersMemory[i], nullptr);
                }
                if (modelDataStagingBuffer != nullptr) {
                    vkDestroyBuffer(vkDevice.getDevice(), modelDataStagingBuffer, nullptr);
                    vkFreeMemory(vkDevice.getDevice(), modelDataStagingBufferMemory, nullptr);
                }
                //////std::cout<<"model data ssbo destroyed"<<std::endl;
                for (size_t i = 0; i < materialDataShaderStorageBuffers.size(); i++) {
                    vkDestroyBuffer(vkDevice.getDevice(), materialDataShaderStorageBuffers[i], nullptr);
                    vkFreeMemory(vkDevice.getDevice(), materialDataShaderStorageBuffersMemory[i], nullptr);
                }
                if (materialDataStagingBuffer != nullptr) {
                    vkDestroyBuffer(vkDevice.getDevice(), materialDataStagingBuffer, nullptr);
                    vkFreeMemory(vkDevice.getDevice(), materialDataStagingBufferMemory, nullptr);
                }
                if (vboIndirectStagingBuffer != nullptr) {
                    vkDestroyBuffer(vkDevice.getDevice(), vboIndirectStagingBuffer, nullptr);
                    vkFreeMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemory, nullptr);
                }
                //////std::cout<<"material data ssbo destroyed"<<std::endl;
                if (vboIndirect != VK_NULL_HANDLE) {
                    vkDestroyBuffer(vkDevice.getDevice(),vboIndirect, nullptr);
                    vkFreeMemory(vkDevice.getDevice(), vboIndirectMemory, nullptr);
                }

                for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    vkDestroySampler(vkDevice.getDevice(), depthTextureSampler[i], nullptr);
                    vkDestroyImageView(vkDevice.getDevice(), depthTextureImageView[i], nullptr);
                    vkDestroyImage(vkDevice.getDevice(), depthTextureImage[i], nullptr);
                    vkFreeMemory(vkDevice.getDevice(), depthTextureImageMemory[i], nullptr);

                    vkDestroySampler(vkDevice.getDevice(), alphaTextureSampler[i], nullptr);
                    vkDestroyImageView(vkDevice.getDevice(), alphaTextureImageView[i], nullptr);
                    vkDestroyImage(vkDevice.getDevice(), alphaTextureImage[i], nullptr);
                    vkFreeMemory(vkDevice.getDevice(), alphaTextureImageMemory[i], nullptr);

                    vkDestroySampler(vkDevice.getDevice(), stencilTextureSampler[i], nullptr);
                    vkDestroyImageView(vkDevice.getDevice(), stencilTextureImageView[i], nullptr);
                    vkDestroyImage(vkDevice.getDevice(), stencilTextureImage[i], nullptr);
                    vkFreeMemory(vkDevice.getDevice(), stencilTextureImageMemory[i], nullptr);
                }

                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, copyModelDataBufferCommandBuffer.size(), copyModelDataBufferCommandBuffer.data());

                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, copyMaterialDataBufferCommandBuffer.size(), copyMaterialDataBufferCommandBuffer.data());

                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, copyDrawBufferCommandBuffer.size(), copyDrawBufferCommandBuffer.data());

                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, copyDrawIndexedBufferCommandBuffer.size(), copyDrawIndexedBufferCommandBuffer.data());
                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, copyVbBufferCommandBuffer.size(), copyVbBufferCommandBuffer.data());
                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, copyVbIndexedBufferCommandBuffer.size(), copyVbIndexedBufferCommandBuffer.data());
                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, depthCommandBuffer.size(), depthCommandBuffer.data());
                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, alphaCommandBuffer.size(), alphaCommandBuffer.data());
                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, stencilCommandBuffer.size(), stencilCommandBuffer.data());
                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, shadowCommandBuffer.size(), shadowCommandBuffer.data());
                vkDestroyCommandPool(vkDevice.getDevice(), secondaryBufferCommandPool, nullptr);
            }
        #else
            ShadowRenderComponent::ShadowRenderComponent (RenderWindow& window, int layer, std::string expression,window::ContextSettings settings) :
            HeavyComponent(window, math::Vec3f(window.getView().getPosition().x(), window.getView().getPosition().y(), layer),
                          math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0),
                          math::Vec3f(window.getView().getSize().x() + window.getView().getSize().x() * 0.5f, window.getView().getPosition().y() + window.getView().getSize().y() * 0.5f, layer)),
            view(window.getView()),
            quad(math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), window.getSize().y() * 0.5f)),
            expression(expression) {
            update = false;
            datasReady = false;
            quad.move(math::Vec3f(-window.getView().getSize().x() * 0.5f, -window.getView().getSize().y() * 0.5f, 0));
            math::Vec3f resolution ((int) window.getSize().x(), (int) window.getSize().y(), window.getView().getSize().z());
            //settings.depthBits = 24;
            depthBuffer.create(resolution.x(), resolution.y(), settings);
            //settings.depthBits = 0;

            depthBufferTile = Sprite(depthBuffer.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0), IntRect(0, 0, window.getView().getSize().x(), window.getView().getSize().y()));

            glCheck(glGenTextures(1, &depthTex));
            glCheck(glBindTexture(GL_TEXTURE_2D, depthTex));
            glCheck(glTexStorage2D(GL_TEXTURE_2D,1,GL_RGBA32F,window.getView().getSize().x(), window.getView().getSize().y()));
            glCheck(glBindImageTexture(0, depthTex,0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F));
            glCheck(glBindTexture(GL_TEXTURE_2D, 0));
            std::vector<GLfloat> depthClearBuf(window.getView().getSize().x()*window.getView().getSize().y()*4, 0);
            glCheck(glGenBuffers(1, &clearBuf));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf));
            glCheck(glBufferData(GL_PIXEL_UNPACK_BUFFER, depthClearBuf.size() * sizeof(GLfloat),
            &depthClearBuf[0], GL_STATIC_COPY));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));



            stencilBuffer.create(resolution.x(), resolution.y(),settings);
            stencilBufferTile = Sprite(stencilBuffer.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0), IntRect(0, 0, window.getView().getSize().x(), window.getView().getSize().y()));
            glCheck(glGenTextures(1, &stencilTex));
            glCheck(glBindTexture(GL_TEXTURE_2D, stencilTex));
            glCheck(glTexStorage2D(GL_TEXTURE_2D,1,GL_RGBA32F,window.getView().getSize().x(), window.getView().getSize().y()));
            glCheck(glBindImageTexture(0, stencilTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F));
            glCheck(glBindTexture(GL_TEXTURE_2D, 0));
            std::vector<GLfloat> stencilClearBuf(window.getView().getSize().x()*window.getView().getSize().y()*4, 0);
            glCheck(glGenBuffers(1, &clearBuf2));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf2));
            glCheck(glBufferData(GL_PIXEL_UNPACK_BUFFER, stencilClearBuf.size() * sizeof(GLfloat),
            &stencilClearBuf[0], GL_STATIC_COPY));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));

            alphaBuffer.create(window.getView().getSize().x(), window.getView().getSize().y(), settings);
            glCheck(glGenTextures(1, &alphaTex));
            glCheck(glBindTexture(GL_TEXTURE_2D, alphaTex));
            glCheck(glTexStorage2D(GL_TEXTURE_2D,1,GL_RGBA32F,window.getView().getSize().x(), window.getView().getSize().y()));
            glCheck(glBindImageTexture(0, alphaTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F));
            glCheck(glBindTexture(GL_TEXTURE_2D, 0));
            std::vector<GLfloat> alphaClearBuf(window.getView().getSize().x()*window.getView().getSize().y()*4, 0);
            glCheck(glGenBuffers(1, &clearBuf3));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf3));
            glCheck(glBufferData(GL_PIXEL_UNPACK_BUFFER, alphaClearBuf.size() * sizeof(GLfloat),
            &alphaClearBuf[0], GL_STATIC_COPY));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
            alphaBufferSprite = Sprite(alphaBuffer.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0), IntRect(0, 0, window.getView().getSize().x(), window.getView().getSize().y()));
            //Debug image.
            shadowMap.create(resolution.x(), resolution.y(),settings);
            shadowTile = Sprite(shadowMap.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0), IntRect(0, 0, window.getView().getSize().x(), window.getView().getSize().y()));
            glCheck(glGenBuffers(1, &atomicBuffer));
            glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicBuffer));
            glCheck(glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW));
            glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0));
            glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicBuffer));
            glCheck(glGenTextures(1, &frameBufferTex));
            glCheck(glBindTexture(GL_TEXTURE_2D, frameBufferTex));
            glCheck(glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, window.getView().getSize().x(), window.getView().getSize().y()));
            glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
            glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            glCheck(glBindImageTexture(0, frameBufferTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F));
            glCheck(glBindTexture(GL_TEXTURE_2D, 0));
            std::vector<GLfloat> texClearBuf(window.getView().getSize().x()*window.getView().getSize().y()*4, 0);
            glCheck(glGenBuffers(1, &clearBuf4));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf4));
            glCheck(glBufferData(GL_PIXEL_UNPACK_BUFFER, texClearBuf.size() * sizeof(GLfloat),
            &texClearBuf[0], GL_STATIC_COPY));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
            core::FastDelegate<bool> signal (&ShadowRenderComponent::needToUpdate, this);
            core::FastDelegate<void> slot (&ShadowRenderComponent::drawNextFrame, this);
            core::Command cmd(signal, slot);
            getListener().connect("UPDATE", cmd);
            glCheck(glGenBuffers(1, &vboIndirect));
            glGenBuffers(1, &modelDataBuffer);
            glGenBuffers(1, &materialDataBuffer);
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
                                                                    mat4 shadowProjMatrix;
                                                                 };
                                                                 struct MaterialData {
                                                                     uint textureIndex;
                                                                     uint layer;
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
             const std::string buildDepthBufferFragmentShader = R"(#version 460
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
                                                                        fColor = vec4(0, l, z, texel.a);
                                                                        imageStore(depthBuffer,ivec2(gl_FragCoord.x()y),vec4(0,l,z,texel.a));
                                                                        memoryBarrier();
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
                                                              layout(binding = 0, rgba32f) coherent uniform image2D alphaBuffer;
                                                              layout (location = 0) out vec4 fColor;
                                                              uniform sampler2D depthBuffer;
                                                              uniform sampler2D stencilBuffer;
                                                              uniform vec3 resolution;
                                                              uniform mat4 lviewMatrix;
                                                              uniform mat4 lprojectionMatrix;
                                                              in vec4 frontColor;
                                                              in vec2 fTexCoords;
                                                              in flat uint texIndex;
                                                              in flat uint layer;
                                                              in vec4 shadowCoords;
                                                              void main() {
                                                                  vec4 texel = (texIndex != 0) ? frontColor * texture2D(textures[texIndex-1], fTexCoords.x()y) : frontColor;
                                                                  float current_alpha = texel.a;
                                                                  vec2 position = (gl_FragCoord.x()y / resolution.x()y);
                                                                  vec4 depth = texture2D (depthBuffer, position);
                                                                  beginInvocationInterlockARB();
                                                                  vec4 alpha = imageLoad(alphaBuffer,ivec2(gl_FragCoord.x()y));
                                                                  vec3 projCoords = shadowCoords.x()yz / shadowCoords.w;
                                                                  projCoords = projCoords * 0.5 + 0.5;
                                                                  vec4 stencil = texture2D (stencilBuffer, projCoords.x()y);
                                                                  float l = layer;
                                                                  float z = gl_FragCoord.z();
                                                                  if (/*l > stencil.y() || l == stencil.y() &&*/ stencil.z() > projCoords.z() && depth.z() > z && current_alpha > alpha.a) {
                                                                      imageStore(alphaBuffer,ivec2(gl_FragCoord.x()y),vec4(0, l, z, current_alpha));
                                                                      memoryBarrier();
                                                                      fColor = vec4(0, 1, z, current_alpha);
                                                                  } else {
                                                                      fColor = alpha;
                                                                  }
                                                                  endInvocationInterlockARB();
                                                              }
                                                              )";
            const std::string buildShadowMapFragmentShader = R"(#version 460
                                                                #extension GL_ARB_bindless_texture : enable
                                                                #extension GL_ARB_fragment_shader_interlock : require
                                                                in vec4 frontColor;
                                                                in vec2 fTexCoords;

                                                                layout (std140, binding = 0) uniform ALL_TEXTURES {
                                                                    sampler2D textures[200];
                                                                };
                                                                in flat uint texIndex;
                                                                in flat uint layer;
                                                                layout(binding = 0, rgba32f) coherent uniform image2D stencilBuffer;
                                                                layout (location = 0) out vec4 fColor;
                                                                void main() {
                                                                    vec4 texel = (texIndex != 0) ? frontColor * texture2D(textures[texIndex-1], fTexCoords) : frontColor;
                                                                    float current_alpha = texel.a;
                                                                    beginInvocationInterlockARB();
                                                                    vec4 alpha = imageLoad(stencilBuffer,ivec2(gl_FragCoord.x()y));
                                                                    float l = layer;
                                                                    float z = gl_FragCoord.z();
                                                                    if (/*l > alpha.y() || l == alpha.y() &&*/ z > alpha.z()) {
                                                                        imageStore(stencilBuffer,ivec2(gl_FragCoord.x()y),vec4(0, l, z, current_alpha));
                                                                        memoryBarrier();
                                                                        fColor = vec4(0, l, z, current_alpha);
                                                                    } else {
                                                                        fColor = alpha;
                                                                    }
                                                                    endInvocationInterlockARB();
                                                                }
                                                            )";
                const std::string perPixShadowIndirectRenderingVertexShader = R"(#version 460
                                                                 layout (location = 0) in vec3 position;
                                                                 layout (location = 1) in vec4 color;
                                                                 layout (location = 2) in vec2 texCoords;
                                                                 layout (location = 3) in vec3 normals;
                                                                 uniform mat4 projectionMatrix;
                                                                 uniform mat4 viewMatrix;
                                                                 uniform mat4 lviewMatrix;
                                                                 uniform mat4 lprojectionMatrix;
                                                                 uniform mat4 textureMatrix[)"+core::conversionUIntString(Texture::getAllTextures().size())+R"(];
                                                                 struct ModelData {
                                                                    mat4 modelMatrix;
                                                                    mat4 shadowProjMatrix;
                                                                 };
                                                                 struct MaterialData {
                                                                     uint textureIndex;
                                                                     uint layer;
                                                                 };
                                                                 layout(binding = 0, std430) buffer modelData {
                                                                     ModelData modelDatas[];
                                                                 };
                                                                 layout(binding = 1, std430) buffer materialData {
                                                                     MaterialData materialDatas[];
                                                                 };
                                                                 out vec4 shadowCoords;
                                                                 out vec2 fTexCoords;
                                                                 out vec4 frontColor;
                                                                 out uint texIndex;
                                                                 out uint layer;
                                                                 void main() {
                                                                    ModelData model = modelDatas[gl_BaseInstance + gl_InstanceID];
                                                                    MaterialData material = materialDatas[gl_DrawID];
                                                                    uint textureIndex = material.textureIndex;
                                                                    uint l = material.layer;
                                                                    gl_Position = projectionMatrix * viewMatrix * model.shadowProjMatrix * model.modelMatrix * vec4(position, 1.f);
                                                                    shadowCoords = lprojectionMatrix * lviewMatrix * model.shadowProjMatrix * model.modelMatrix * vec4(position, 1);
                                                                    fTexCoords = (textureIndex != 0) ? (textureMatrix[textureIndex-1] * vec4(texCoords, 1.f, 1.f)).x()y : texCoords;
                                                                    frontColor = color;
                                                                    texIndex = textureIndex;
                                                                    layer = l;
                                                                 }
                                                                 )";
                const std::string perPixShadowFragmentShader = R"(#version 460
                                                                  #extension GL_ARB_bindless_texture : enable
                                                                  in vec4 shadowCoords;
                                                                  in vec4 frontColor;
                                                                  in vec2 fTexCoords;
                                                                  in flat uint texIndex;
                                                                  in flat uint layer;
                                                                  uniform sampler2D stencilBuffer;
                                                                  uniform sampler2D depthBuffer;
                                                                  uniform sampler2D alphaBuffer;
                                                                  uniform float haveTexture;
                                                                  uniform vec3 resolution;
                                                                  layout (std140, binding = 0) uniform ALL_TEXTURES {
                                                                      sampler2D textures[200];
                                                                  };
                                                                  layout (location = 0) out vec4 fColor;
                                                                  layout(rgba32f, binding = 0) uniform image2D img_output;
                                                                  layout(binding = 0, offset = 0) uniform atomic_uint nextNodeCounter;

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
                                                                void main() {
                                                                    uint fragmentIdx = atomicCounterIncrement(nextNodeCounter);
                                                                    vec2 position = (gl_FragCoord.x()y / resolution.x()y);
                                                                    vec4 depth = texture(depthBuffer, position);
                                                                    vec4 alpha = texture(alphaBuffer, position);
                                                                    vec4 texel = (texIndex != 0) ? frontColor * texture2D(textures[texIndex-1], fTexCoords) : frontColor;

                                                                    float color = texel.a;
                                                                    vec3 projCoords = shadowCoords.x()yz / shadowCoords.w;
                                                                    projCoords = projCoords * 0.5 + 0.5;
                                                                    vec4 stencil = texture (stencilBuffer, projCoords.x()y);
                                                                    float z = gl_FragCoord.z();
                                                                    vec4 visibility;
                                                                    uint l = layer;
                                                                    if (/*l > stencil.y() || l == stencil.y() &&*/ stencil.z() > projCoords.z()) {
                                                                        if (depth.z() > z) {
                                                                            visibility = vec4 (1, 1, 1, alpha.a);
                                                                        } else {
                                                                            visibility = vec4 (0.5, 0.5, 0.5, color);
                                                                        }
                                                                    } else {
                                                                        visibility = vec4 (1, 1, 1, 1);
                                                                    }
                                                                    /*if (fragmentIdx == 0)
                                                                        print(ivec2(200, 100), 1, vec4(1, 0, 0, 1), vec4(0, 0, depth.z(), z));*/
                                                                    fColor = visibility /*vec4(0, 0, z*100, 1)*/;
                                                                  }
                                                                  )";
                if (!debugShader.loadFromMemory(simpleVertexShader, simpleFragmentShader)) {
                    throw core::Erreur(51, "Failed to load debug shader", 0);
                }
                if (!depthGenShader.loadFromMemory(indirectRenderingVertexShader, buildDepthBufferFragmentShader))  {
                    throw core::Erreur(52, "Error, failed to load build depth buffer shader", 3);
                }
                if (!buildShadowMapShader.loadFromMemory(indirectRenderingVertexShader, buildShadowMapFragmentShader)) {
                    throw core::Erreur(53, "Error, failed to load build shadow map shader", 3);
                }
                if (!perPixShadowShader.loadFromMemory(perPixShadowIndirectRenderingVertexShader, perPixShadowFragmentShader)) {
                    throw core::Erreur(54, "Error, failed to load per pix shadow map shader", 3);
                }
                if (!sBuildAlphaBufferShader.loadFromMemory(perPixShadowIndirectRenderingVertexShader,buildAlphaBufferFragmentShader)) {
                    throw core::Erreur(60, "Error, failed to load build alpha buffer shader", 3);
                }
                math::Matrix4f viewMatrix = window.getDefaultView().getViewMatrix().getMatrix().transpose();
                math::Matrix4f projMatrix = window.getDefaultView().getProjMatrix().getMatrix().transpose();
                debugShader.setParameter("projectionMatrix", projMatrix);
                debugShader.setParameter("viewMatrix", viewMatrix);
                depthGenShader.setParameter("texture", Shader::CurrentTexture);
                buildShadowMapShader.setParameter("texture", Shader::CurrentTexture);
                perPixShadowShader.setParameter("stencilBuffer", stencilBuffer.getTexture());
                perPixShadowShader.setParameter("depthBuffer", depthBuffer.getTexture());
                perPixShadowShader.setParameter("texture", Shader::CurrentTexture);
                perPixShadowShader.setParameter("resolution", resolution.x(), resolution.y(), resolution.z());
                perPixShadowShader.setParameter("alphaBuffer", alphaBuffer.getTexture());
                sBuildAlphaBufferShader.setParameter("depthBuffer", depthBuffer.getTexture());
                sBuildAlphaBufferShader.setParameter("stencilBuffer", stencilBuffer.getTexture());
                sBuildAlphaBufferShader.setParameter("texture", Shader::CurrentTexture);
                sBuildAlphaBufferShader.setParameter("resolution", resolution.x(), resolution.y(), resolution.z());

                std::vector<Texture*> allTextures = Texture::getAllTextures();
                Samplers allSamplers{};
                std::vector<math::Matrix4f> textureMatrices;
                for (unsigned int i = 0; i < allTextures.size(); i++) {
                    textureMatrices.push_back(allTextures[i]->getTextureMatrix());
                    GLuint64 handle_texture = allTextures[i]->getTextureHandle();
                    allTextures[i]->makeTextureResident(handle_texture);
                    allSamplers.tex[i].handle = handle_texture;
                    ////////std::cout<<"add texture i : "<<i<<" id : "<<allTextures[i]->getNativeHandle()<<std::endl;
                }
                buildShadowMapShader.setParameter("textureMatrix", textureMatrices);
                depthGenShader.setParameter("textureMatrix", textureMatrices);
                perPixShadowShader.setParameter("textureMatrix", textureMatrices);
                sBuildAlphaBufferShader.setParameter("textureMatrix", textureMatrices);

                glCheck(glGenBuffers(1, &ubo));
                glCheck(glBindBuffer(GL_UNIFORM_BUFFER, ubo));
                glCheck(glBufferData(GL_UNIFORM_BUFFER, sizeof(Samplers),allSamplers.tex, GL_STATIC_DRAW));
                glCheck(glBindBuffer(GL_UNIFORM_BUFFER, 0));
                glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, modelDataBuffer));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, materialDataBuffer));
                stencilBuffer.setActive();
                glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, modelDataBuffer));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, materialDataBuffer));
                depthBuffer.setActive();
                glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, modelDataBuffer));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, materialDataBuffer));
                alphaBuffer.setActive();
                glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, modelDataBuffer));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, materialDataBuffer));
                alphaBuffer.setActive(false);
                ////////std::cout<<"size : "<<sizeof(Samplers)<<" "<<alignof (alignas(16) uint64_t[200])<<std::endl;

                /*glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo2));
                glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo3));*/


                for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                    vbBindlessTex[i].setPrimitiveType(static_cast<PrimitiveType>(i));
                }
                //getListener().launchThread();
            }
            void ShadowRenderComponent::loadTextureIndexes() {
                std::vector<Texture*> allTextures = Texture::getAllTextures();
                Samplers allSamplers{};
                std::vector<math::Matrix4f> textureMatrices;
                for (unsigned int i = 0; i < allTextures.size(); i++) {
                    textureMatrices.push_back(allTextures[i]->getTextureMatrix());
                    GLuint64 handle_texture = allTextures[i]->getTextureHandle();
                    allTextures[i]->makeTextureResident(handle_texture);
                    allSamplers.tex[i].handle = handle_texture;
                    ////////std::cout<<"add texture i : "<<i<<" id : "<<allTextures[i]->getNativeHandle()<<std::endl;
                }
                glCheck(glBindBuffer(GL_UNIFORM_BUFFER, ubo));
                glCheck(glBufferData(GL_UNIFORM_BUFFER, sizeof(Samplers),allSamplers.tex, GL_STATIC_DRAW));
                glCheck(glBindBuffer(GL_UNIFORM_BUFFER, 0));
            }
            void ShadowRenderComponent::onVisibilityChanged(bool visible) {
                if (visible) {
                    glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                } else {
                    glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0));
                }
            }
            void ShadowRenderComponent::drawInstanced() {
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
                        {
                            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                            material.textureIndex = (m_normals[i].getMaterial().getTexture() != nullptr) ? m_normals[i].getMaterial().getTexture()->getNativeHandle() : 0;
                            material.layer = m_normals[i].getMaterial().getLayer();
                        }

                        materials[p].push_back(material);
                        TransformMatrix tm;
                        ModelData model;
                        model.worldMat = tm.getMatrix().transpose();
                        model.shadowProjMat = tm.getMatrix().transpose();
                        matrices[p].push_back(model);
                        unsigned int vertexCount = 0;
                        for (unsigned int j = 0; j < m_normals[i].getAllVertices().getVertexCount(); j++) {
                            vbBindlessTex[p].append(m_normals[i].getAllVertices()[j]);
                            vertexCount++;
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
                            TransformMatrix tm;
                            model.shadowProjMat = tm.getMatrix().transpose();
                            matrices[p].push_back(model);
                        }
                        unsigned int vertexCount = 0;
                        if (m_instances[i].getVertexArrays().size() > 0) {
                            Entity* entity = m_instances[i].getVertexArrays()[0]->getEntity();
                            for (unsigned int j = 0; j < m_instances[i].getVertexArrays().size(); j++) {
                                if (entity == m_instances[i].getVertexArrays()[j]->getEntity()) {
                                    unsigned int p = m_instances[i].getVertexArrays()[j]->getPrimitiveType();
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
                        currentStates.shader = &buildShadowMapShader;
                        stencilBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), currentStates, vboIndirect);
                        currentStates.shader = &depthGenShader;
                        depthBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), currentStates, vboIndirect);
                        vbBindlessTex[p].clear();
                    }
                }
                glCheck(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
                physic::BoundingBox viewArea = view.getViewVolume();
                math::Vec3f position (viewArea.getPosition().x(),viewArea.getPosition().y(), view.getPosition().z());
                math::Vec3f size (viewArea.getWidth(), viewArea.getHeight(), 0);
                stencilBuffer.display();
                stencilBufferTile.setPosition(position);
                depthBuffer.display();
                depthBufferTile.setPosition(position);
                shadowMap.setView(view);
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
                for (unsigned int i = 0; i < m_shadow_normals.size(); i++) {
                    if (m_shadow_normals[i].getAllVertices().getVertexCount() > 0) {
                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        unsigned int p = m_shadow_normals[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_shadow_normals[i].getMaterial().getTexture() != nullptr) ? m_shadow_normals[i].getMaterial().getTexture()->getNativeHandle() : 0;
                        material.layer = m_shadow_normals[i].getMaterial().getLayer();
                        materials[p].push_back(material);
                        TransformMatrix tm;
                        ModelData model;
                        model.worldMat = tm.getMatrix().transpose();
                        model.shadowProjMat = tm.getMatrix().transpose();
                        matrices[p].push_back(model);
                        unsigned int vertexCount = 0;
                        for (unsigned int j = 0; j < m_shadow_normals[i].getAllVertices().getVertexCount(); j++) {
                            vertexCount++;
                            vbBindlessTex[p].append(m_shadow_normals[i].getAllVertices()[j]);
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
                for (unsigned int i = 0; i < m_shadow_instances.size(); i++) {
                    if (m_shadow_instances[i].getAllVertices().getVertexCount() > 0) {
                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        unsigned int p = m_shadow_instances[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_shadow_normals[i].getMaterial().getTexture() != nullptr) ? m_shadow_normals[i].getMaterial().getTexture()->getNativeHandle() : 0;
                        material.layer = m_shadow_normals[i].getMaterial().getLayer();
                        materials[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_shadow_instances[i].getTransforms();
                        std::vector<TransformMatrix> tm2 = m_shadow_instances[i].getShadowProjMatrix();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            tm2[j].update();
                            ModelData model;
                            model.worldMat = tm[j]->getMatrix().transpose();
                            model.shadowProjMat = tm2[j].getMatrix().transpose();
                            matrices[p].push_back(model);
                        }
                        unsigned int vertexCount=0;
                        if (m_shadow_instances[i].getVertexArrays().size() > 0) {
                            Entity* entity = m_shadow_instances[i].getVertexArrays()[0]->getEntity();
                            for (unsigned int j = 0; j < m_shadow_instances[i].getVertexArrays().size(); j++) {
                                if (entity == m_shadow_instances[i].getVertexArrays()[j]->getEntity()) {
                                    unsigned int p = m_shadow_instances[i].getVertexArrays()[j]->getPrimitiveType();
                                    for (unsigned int k = 0; k < m_shadow_instances[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                        vertexCount++;
                                        vbBindlessTex[p].append((*m_shadow_instances[i].getVertexArrays()[j])[k], (m_shadow_instances[i].getMaterial().getTexture() != nullptr) ? m_shadow_instances[i].getMaterial().getTexture()->getId() : 0);
                                        vbBindlessTex[p].addLayer(m_shadow_instances[i].getMaterial().getLayer());
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
                currentStates.shader = &sBuildAlphaBufferShader;
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
                        alphaBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), currentStates, vboIndirect);
                    }
                }
                glCheck(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
                currentStates.shader = &perPixShadowShader;
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
                        shadowMap.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), currentStates, vboIndirect);
                        vbBindlessTex[p].clear();
                    }
                }
                shadowMap.display();
            }
            void ShadowRenderComponent::drawInstancedIndexed() {
                for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                    vbBindlessTex[i].clear();
                }
                std::array<std::vector<DrawElementsIndirectCommand>, Batcher::nbPrimitiveTypes> drawElementsIndirectCommands;
                std::array<std::vector<ModelData>, Batcher::nbPrimitiveTypes> matrices;
                std::array<std::vector<MaterialData>, Batcher::nbPrimitiveTypes> materials;
                std::array<unsigned int, Batcher::nbPrimitiveTypes> firstIndex, baseInstance, baseVertex;
                for (unsigned int i = 0; i < firstIndex.size(); i++) {
                firstIndex[i] = 0;
                }
                for (unsigned int i = 0; i < baseVertex.size(); i++) {
                    baseVertex[i] = 0;
                }
                for (unsigned int i = 0; i < baseInstance.size(); i++) {
                    baseInstance[i] = 0;
                }
                for (unsigned int i = 0; i < m_normalsIndexed.size(); i++) {
                   if (m_normalsIndexed[i].getAllVertices().getVertexCount() > 0) {
                        DrawElementsIndirectCommand drawElementsIndirectCommand;
                        unsigned int p = m_normalsIndexed[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_normalsIndexed[i].getMaterial().getTexture() != nullptr) ? m_normalsIndexed[i].getMaterial().getTexture()->getNativeHandle() : 0;
                        material.layer = m_normalsIndexed[i].getMaterial().getLayer();
                        materials[p].push_back(material);
                        TransformMatrix tm;
                        ModelData model;
                        model.worldMat = tm.getMatrix().transpose();
                        model.shadowProjMat = tm.getMatrix().transpose();
                        matrices[p].push_back(model);
                        unsigned int indexCount = 0, vertexCount = 0;
                        for (unsigned int j = 0; j < m_normalsIndexed[i].getAllVertices().getVertexCount(); j++) {
                            vertexCount++;
                            vbBindlessTex[p].append(m_normalsIndexed[i].getAllVertices()[j]);
                        }
                        for (unsigned int j = 0; j < m_normalsIndexed[i].getAllVertices().getIndexes().size(); j++) {
                            indexCount++;
                            vbBindlessTex[p].addIndex(m_normalsIndexed[i].getAllVertices().getIndexes()[j]);
                        }
                        drawElementsIndirectCommand.index_count = indexCount;
                        drawElementsIndirectCommand.first_index = firstIndex[p];
                        drawElementsIndirectCommand.instance_base = baseInstance[p];
                        drawElementsIndirectCommand.vertex_base = baseVertex[p];
                        drawElementsIndirectCommand.instance_count = 1;
                        drawElementsIndirectCommands[p].push_back(drawElementsIndirectCommand);
                        firstIndex[p] += indexCount;
                        baseVertex[p] += vertexCount;
                        baseInstance[p] += 1;
                    }
                }
                for (unsigned int i = 0; i < m_instancesIndexed.size(); i++) {
                    if (m_instancesIndexed[i].getAllVertices().getVertexCount() > 0) {
                        DrawElementsIndirectCommand drawElementsIndirectCommand;
                        unsigned int p = m_instancesIndexed[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_normalsIndexed[i].getMaterial().getTexture() != nullptr) ? m_normalsIndexed[i].getMaterial().getTexture()->getNativeHandle() : 0;
                        material.layer = m_normalsIndexed[i].getMaterial().getLayer();
                        materials[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_instancesIndexed[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            ModelData model;
                            model.worldMat = tm[j]->getMatrix().transpose();
                            TransformMatrix tm;
                            model.shadowProjMat = tm.getMatrix().transpose();
                            matrices[p].push_back(model);
                        }
                        unsigned int vertexCount = 0, indexCount = 0;
                        if (m_instancesIndexed[i].getVertexArrays().size() > 0) {
                            Entity* entity = m_instancesIndexed[i].getVertexArrays()[0]->getEntity();
                            for (unsigned int j = 0; j < m_instancesIndexed[i].getVertexArrays().size(); j++) {
                                if (entity == m_instancesIndexed[i].getVertexArrays()[j]->getEntity()) {
                                    unsigned int p = m_instancesIndexed[i].getVertexArrays()[j]->getPrimitiveType();
                                    for (unsigned int k = 0; k < m_instancesIndexed[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                        vertexCount++;
                                        vbBindlessTex[p].append((*m_instancesIndexed[i].getVertexArrays()[j])[k]);
                                    }
                                    for (unsigned int k = 0; k < m_instancesIndexed[i].getVertexArrays()[j]->getIndexes().size(); k++) {
                                        indexCount++;
                                        vbBindlessTex[p].addIndex(m_instancesIndexed[i].getVertexArrays()[j]->getIndexes()[k]);
                                    }
                                }
                            }
                        }
                        drawElementsIndirectCommand.index_count = indexCount;
                        drawElementsIndirectCommand.first_index = firstIndex[p];
                        drawElementsIndirectCommand.instance_base = baseInstance[p];
                        drawElementsIndirectCommand.vertex_base = baseVertex[p];
                        drawElementsIndirectCommand.instance_count = tm.size();
                        drawElementsIndirectCommands[p].push_back(drawElementsIndirectCommand);
                        firstIndex[p] += indexCount;
                        baseVertex[p] += vertexCount;
                        baseInstance[p] += tm.size();
                    }
                }
                RenderStates currentStates;
                currentStates.blendMode = BlendNone;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelDataBuffer));
                        glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, matrices[p].size() * sizeof(ModelData), &matrices[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialDataBuffer));
                        glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, materials[p].size() * sizeof(MaterialData), &materials[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vboIndirect));
                        glCheck(glBufferData(GL_DRAW_INDIRECT_BUFFER, drawElementsIndirectCommands[p].size() * sizeof(DrawElementsIndirectCommand), &drawElementsIndirectCommands[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
                        vbBindlessTex[p].update();
                        currentStates.shader = &buildShadowMapShader;
                        stencilBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawElementsIndirectCommands[p].size(), currentStates, vboIndirect);
                        currentStates.shader = &depthGenShader;
                        depthBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawElementsIndirectCommands[p].size(), currentStates, vboIndirect);
                        vbBindlessTex[p].clear();
                    }
                }
                physic::BoundingBox viewArea = view.getViewVolume();
                math::Vec3f position (viewArea.getPosition().x(),viewArea.getPosition().y(), view.getPosition().z());
                math::Vec3f size (viewArea.getWidth(), viewArea.getHeight(), 0);
                stencilBuffer.display();
                stencilBufferTile.setPosition(position);
                depthBuffer.display();
                depthBufferTile.setPosition(position);
                shadowMap.setView(view);
                for (unsigned int i = 0; i < firstIndex.size(); i++) {
                firstIndex[i] = 0;
                }
                for (unsigned int i = 0; i < baseVertex.size(); i++) {
                    baseVertex[i] = 0;
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
                for (unsigned int i = 0; i < drawElementsIndirectCommands.size(); i++) {
                    drawElementsIndirectCommands[i].clear();
                }
                for (unsigned int i = 0; i < m_shadow_normalsIndexed.size(); i++) {
                    if (m_shadow_normalsIndexed[i].getAllVertices().getVertexCount() > 0) {
                        DrawElementsIndirectCommand drawElementsIndirectCommand;
                        unsigned int p = m_shadow_normalsIndexed[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_shadow_normalsIndexed[i].getMaterial().getTexture() != nullptr) ? m_shadow_normalsIndexed[i].getMaterial().getTexture()->getNativeHandle() : 0;
                        material.layer = m_shadow_normalsIndexed[i].getMaterial().getLayer();
                        materials[p].push_back(material);
                        TransformMatrix tm;
                        ModelData model;
                        model.worldMat = tm.getMatrix().transpose();
                        model.shadowProjMat = tm.getMatrix().transpose();
                        matrices[p].push_back(model);
                        unsigned int indexCount = 0, vertexCount = 0;
                        for (unsigned int j = 0; j < m_shadow_normalsIndexed[i].getAllVertices().getVertexCount(); j++) {
                            vertexCount++;
                            vbBindlessTex[p].append(m_shadow_normalsIndexed[i].getAllVertices()[j]);
                        }
                        for (unsigned int j = 0; j < m_shadow_normalsIndexed[i].getAllVertices().getIndexes().size(); j++) {
                            indexCount++;
                            vbBindlessTex[p].addIndex(m_shadow_normalsIndexed[i].getAllVertices().getIndexes()[j]);
                        }
                        drawElementsIndirectCommand.index_count = indexCount;
                        drawElementsIndirectCommand.first_index = firstIndex[p];
                        drawElementsIndirectCommand.instance_base = baseInstance[p];
                        drawElementsIndirectCommand.vertex_base = baseVertex[p];
                        drawElementsIndirectCommand.instance_count = 1;
                        drawElementsIndirectCommands[p].push_back(drawElementsIndirectCommand);
                        firstIndex[p] += indexCount;
                        baseVertex[p] += vertexCount;
                        baseInstance[p] += 1;
                    }
                }
                for (unsigned int i = 0; i < m_shadow_instances_indexed.size(); i++) {
                    DrawElementsIndirectCommand drawElementsIndirectCommand;
                    if (m_shadow_instances_indexed[i].getAllVertices().getVertexCount() > 0) {
                        unsigned int p = m_shadow_instances_indexed[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_shadow_instances_indexed[i].getMaterial().getTexture() != nullptr) ? m_shadow_instances_indexed[i].getMaterial().getTexture()->getNativeHandle() : 0;
                        material.layer = m_shadow_instances_indexed[i].getMaterial().getLayer();
                        materials[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_shadow_instances_indexed[i].getTransforms();
                        std::vector<TransformMatrix> tm2 = m_shadow_instances_indexed[i].getShadowProjMatrix();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            tm2[j].update();
                            ModelData model;
                            model.worldMat = tm[j]->getMatrix().transpose();
                            model.shadowProjMat = tm2[j].getMatrix().transpose();
                            matrices[p].push_back(model);
                        }
                        unsigned int vertexCount = 0, indexCount = 0;
                        if (m_shadow_instances_indexed[i].getVertexArrays().size() > 0) {
                            Entity* entity = m_shadow_instances_indexed[i].getVertexArrays()[0]->getEntity();
                            for (unsigned int j = 0; j < m_shadow_instances_indexed[i].getVertexArrays().size(); j++) {
                                if (entity == m_shadow_instances_indexed[i].getVertexArrays()[j]->getEntity()) {
                                    unsigned int p = m_shadow_instances_indexed[i].getVertexArrays()[j]->getPrimitiveType();
                                    for (unsigned int k = 0; k < m_shadow_instances_indexed[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                        vertexCount++;
                                        vbBindlessTex[p].append((*m_shadow_instances_indexed[i].getVertexArrays()[j])[k]);
                                    }
                                    for (unsigned int k = 0; k < m_shadow_instances_indexed[i].getVertexArrays()[j]->getIndexes().size(); k++) {
                                        indexCount++;
                                        vbBindlessTex[p].addIndex(m_shadow_instances_indexed[i].getVertexArrays()[j]->getIndexes()[k]);
                                    }
                                }
                            }
                        }
                        drawElementsIndirectCommand.index_count = indexCount;
                        drawElementsIndirectCommand.first_index = firstIndex[p];
                        drawElementsIndirectCommand.instance_base = baseInstance[p];
                        drawElementsIndirectCommand.vertex_base = baseVertex[p];
                        drawElementsIndirectCommand.instance_count = tm.size();
                        drawElementsIndirectCommands[p].push_back(drawElementsIndirectCommand);
                        firstIndex[p] += indexCount;
                        baseVertex[p] += vertexCount;
                        baseInstance[p] += tm.size();
                    }
                }
                currentStates.shader = &sBuildAlphaBufferShader;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelDataBuffer));
                        glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, matrices[p].size() * sizeof(ModelData), &matrices[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialDataBuffer));
                        glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, materials[p].size() * sizeof(MaterialData), &materials[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vboIndirect));
                        glCheck(glBufferData(GL_DRAW_INDIRECT_BUFFER, drawElementsIndirectCommands[p].size() * sizeof(DrawElementsIndirectCommand), &drawElementsIndirectCommands[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
                        vbBindlessTex[p].update();
                        alphaBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawElementsIndirectCommands[p].size(), currentStates, vboIndirect);
                    }
                }
                currentStates.shader = &perPixShadowShader;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelDataBuffer));
                        glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, matrices[p].size() * sizeof(ModelData), &matrices[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialDataBuffer));
                        glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, materials[p].size() * sizeof(MaterialData), &materials[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vboIndirect));
                        glCheck(glBufferData(GL_DRAW_INDIRECT_BUFFER, drawElementsIndirectCommands[p].size() * sizeof(DrawElementsIndirectCommand), &drawElementsIndirectCommands[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
                        vbBindlessTex[p].update();
                        shadowMap.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawElementsIndirectCommands[p].size(), currentStates, vboIndirect);
                        vbBindlessTex[p].clear();
                    }
                }
                shadowMap.display();
            }
            void ShadowRenderComponent::drawNextFrame() {
                //glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));

                {
                    std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                    if (datasReady) {
                        datasReady = false;
                        m_instances = batcher.getInstances();
                        m_normals = normalBatcher.getInstances();
                        m_shadow_instances = shadowBatcher.getInstances();
                        m_shadow_normals = normalShadowBatcher.getInstances();
                        m_instancesIndexed = batcherIndexed.getInstances();
                        m_shadow_instances_indexed = shadowBatcherIndexed.getInstances();
                        m_normalsIndexed = normalBatcherIndexed.getInstances();
                        m_shadow_normalsIndexed = normalShadowBatcherIndexed.getInstances();
                        m_stencil_buffer = normalStencilBuffer.getInstances();
                    }
                }

                math::Vec3f centerLight = g2d::AmbientLight::getAmbientLight().getLightCenter();

                View lightView = View(view.getSize().x(), view.getSize().y(), 0, g2d::AmbientLight::getAmbientLight().getHeight());
                lightView.setCenter(centerLight);
                math::Vec3f forward = (view.getPosition() - lightView.getPosition()).normalize();
                math::Vec3f target = lightView.getPosition() + forward;
                lightView.lookAt(target.x(), target.y(), target.z());
                stencilBuffer.setView(lightView);
                depthBuffer.setView(view);
                math::Matrix4f lviewMatrix = lightView.getViewMatrix().getMatrix().transpose();
                math::Matrix4f lprojMatrix = lightView.getProjMatrix().getMatrix().transpose();
                buildShadowMapShader.setParameter("projectionMatrix", lprojMatrix);
                buildShadowMapShader.setParameter("viewMatrix", lviewMatrix);
                float zNear = view.getViewport().getPosition().z();
                if (!view.isOrtho())
                    view.setPerspective(80, view.getViewport().getSize().x() / view.getViewport().getSize().y(), zNear * 0.5f, view.getViewport().getSize().z());
                math::Matrix4f viewMatrix = view.getViewMatrix().getMatrix().transpose();
                math::Matrix4f projMatrix = view.getProjMatrix().getMatrix().transpose();
                depthGenShader.setParameter("projectionMatrix", projMatrix);
                depthGenShader.setParameter("viewMatrix", viewMatrix);
                if (!view.isOrtho())
                    view.setPerspective(80, view.getViewport().getSize().x() / view.getViewport().getSize().y(), zNear, view.getViewport().getSize().z());
                viewMatrix = view.getViewMatrix().getMatrix().transpose();
                projMatrix = view.getProjMatrix().getMatrix().transpose();
                perPixShadowShader.setParameter("projectionMatrix", projMatrix);
                perPixShadowShader.setParameter("viewMatrix", viewMatrix);
                perPixShadowShader.setParameter("lviewMatrix", lviewMatrix);
                perPixShadowShader.setParameter("lprojectionMatrix", lprojMatrix);
                sBuildAlphaBufferShader.setParameter("projectionMatrix", projMatrix);
                sBuildAlphaBufferShader.setParameter("viewMatrix", viewMatrix);
                sBuildAlphaBufferShader.setParameter("lviewMatrix", lviewMatrix);
                sBuildAlphaBufferShader.setParameter("lprojectionMatrix", lprojMatrix);
                drawInstanced();
                drawInstancedIndexed();


                /*glCheck(glFinish());
                vb.clear();
            //vb.name = "";
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
                RenderStates currentStates;
                currentStates.blendMode = BlendAlpha;
                currentStates.shader = &debugShader;
                currentStates.texture = nullptr;
                shadowMap.drawVertexBuffer(vb, currentStates);
                glCheck(glFinish());
                shadowMap.display();*/
                    //glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0));

            }
            std::vector<Entity*> ShadowRenderComponent::getEntities() {
                return visibleEntities;
            }
            void ShadowRenderComponent::draw(RenderTarget& target, RenderStates states) {
                shadowTile.setCenter(target.getView().getPosition());
                states.blendMode = BlendMultiply;
                target.draw(shadowTile, states);
            }
            void ShadowRenderComponent::pushEvent(window::IEvent event, RenderWindow& rw) {
                if (event.type == window::IEvent::WINDOW_EVENT && event.window.type == window::IEvent::WINDOW_EVENT_RESIZED && &getWindow() == &rw && isAutoResized()) {
                    recomputeSize();
                    getListener().pushEvent(event);
                    getView().reset(physic::BoundingBox(getView().getViewport().getPosition().x(), getView().getViewport().getPosition().y(), getView().getViewport().getPosition().z(), event.window.data1, event.window.data2, getView().getViewport().getDepth()));
                }
            }
            bool ShadowRenderComponent::needToUpdate() {
                return update;
            }
            View& ShadowRenderComponent::getView() {
                return view;
            }
            int ShadowRenderComponent::getLayer() {
                return getPosition().z();
            }
            const Texture& ShadowRenderComponent::getStencilBufferTexture() {
                return stencilBuffer.getTexture();
            }
            const Texture& ShadowRenderComponent::getShadowMapTexture() {
                return shadowMap.getTexture();
            }
            Sprite& ShadowRenderComponent::getFrameBufferTile () {
                return stencilBufferTile;
            }
            Sprite& ShadowRenderComponent::getDepthBufferTile() {
                return shadowTile;
            }
            void ShadowRenderComponent::setExpression(std::string expression) {
                update = true;
                this->expression = expression;
            }
            std::string ShadowRenderComponent::getExpression() {
                return expression;
            }
            void ShadowRenderComponent::setView(View view) {
                this->view = view;/*View(view.getSize().x(), view.getSize().y(), view.getPosition().z(), view.getDepth());
                this->view.setCenter(view.getPosition());*/
                shadowMap.setView(this->view);
                depthBuffer.setView(this->view);
                alphaBuffer.setView(this->view);
            }
            bool ShadowRenderComponent::loadEntitiesOnComponent(std::vector<Entity*> vEntities)
            {
                {
                    std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                    datasReady = false;
                    batcher.clear();
                    normalBatcher.clear();
                    shadowBatcher.clear();
                    normalShadowBatcher.clear();
                    batcherIndexed.clear();
                    shadowBatcherIndexed.clear();
                    normalBatcherIndexed.clear();
                    normalShadowBatcherIndexed.clear();
                    normalStencilBuffer.clear();

                }



                for (unsigned int i = 0; i < vEntities.size(); i++) {
                    if ( vEntities[i] != nullptr && vEntities[i]->isLeaf()) {

                        Entity* entity = vEntities[i]->getRootEntity();
                        math::Vec3f shadowOrigin, shadowCenter, shadowScale(1.f, 1.f, 1.f), shadowRotationAxis, shadowTranslation;
                        float shadowRotationAngle = 0;
                        //if (entity != nullptr && entity->isModel()) {
                            shadowCenter = entity->getShadowCenter();
                            shadowScale = entity->getShadowScale();
                            shadowRotationAxis = entity->getShadowRotationAxis();
                            shadowRotationAngle = entity->getShadowRotationAngle();
                            shadowOrigin = entity->getPosition();
                            shadowTranslation = entity->getPosition() + shadowCenter;
                            /*if (entity->getType() == "E_WALL") {
                                //////std::cout<<"shadow center : "<<shadowCenter<<std::endl;
                                //////std::cout<<"shadow scale : "<<shadowScale<<std::endl;
                                //////std::cout<<"shadow rotation axis : "<<shadowRotationAxis<<std::endl;
                                //////std::cout<<"shadow rotation angle : "<<shadowRotationAngle<<std::endl;
                                //////std::cout<<"shadow origin : "<<shadowOrigin<<std::endl;
                                //////std::cout<<"shadow translation : "<<shadowTranslation<<std::endl;
                            }*/
                        //}
                        TransformMatrix tm;
                        tm.setOrigin(shadowOrigin);
                        tm.setScale(shadowScale);
                        tm.setRotation(shadowRotationAxis, shadowRotationAngle);
                        tm.setTranslation(shadowTranslation);

                        for (unsigned int j = 0; j <  vEntities[i]->getNbFaces(); j++) {

                             if(vEntities[i]->getDrawMode() == Entity::INSTANCED) {
                                if (vEntities[i]->getFace(j)->getVertexArray().getIndexes().size() == 0) {
                                    std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                                    batcher.addFace( vEntities[i]->getFace(j));
                                    shadowBatcher.addShadowFace(vEntities[i]->getFace(j),  view.getViewMatrix(), tm);
                                } else {
                                    std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                                    batcherIndexed.addFace( vEntities[i]->getFace(j));
                                    shadowBatcherIndexed.addShadowFace(vEntities[i]->getFace(j),  view.getViewMatrix(), tm);
                                }
                             } else {
                                 if (vEntities[i]->getFace(j)->getVertexArray().getIndexes().size() == 0) {
                                    std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                                    normalBatcher.addFace( vEntities[i]->getFace(j));
                                    if (vEntities[i]->getRootEntity()->getType() != "E_BIGTILE") {
                                        std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                                        normalShadowBatcher.addShadowFace(vEntities[i]->getFace(j), view.getViewMatrix(), tm);
                                        normalStencilBuffer.addFace(vEntities[i]->getFace(j));
                                    }
                                 } else {
                                    std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                                    normalBatcherIndexed.addFace( vEntities[i]->getFace(j));
                                    normalShadowBatcherIndexed.addShadowFace(vEntities[i]->getFace(j), view.getViewMatrix(), tm);
                                 }
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
            void ShadowRenderComponent::clear() {
                 shadowMap.clear(Color::White);
                 depthBuffer.clear(Color::Transparent);
                 stencilBuffer.clear(Color::Transparent);
                 alphaBuffer.clear(Color::Transparent);
                 glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf));
                 glCheck(glBindTexture(GL_TEXTURE_2D, depthTex));
                 glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, view.getSize().x(), view.getSize().y(), GL_RGBA,
                 GL_FLOAT, NULL));
                 glCheck(glBindTexture(GL_TEXTURE_2D, 0));
                 glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
                 glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf2));
                 glCheck(glBindTexture(GL_TEXTURE_2D, stencilTex));
                 glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, view.getSize().x(), view.getSize().y(), GL_RGBA,
                 GL_FLOAT, NULL));
                 glCheck(glBindTexture(GL_TEXTURE_2D, 0));
                 glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
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
                 GLuint zero = 0;
                 glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicBuffer));
                 glCheck(glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero));
                 glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0));

            }
            RenderTexture* ShadowRenderComponent::getFrameBuffer() {
                return &shadowMap;
            }
            ShadowRenderComponent::~ShadowRenderComponent() {
                glDeleteBuffers(1, &modelDataBuffer);
                glDeleteBuffers(1, &materialDataBuffer);
                glDeleteBuffers(1, &ubo);
            }
            #endif // VULKAN
        }
    }
