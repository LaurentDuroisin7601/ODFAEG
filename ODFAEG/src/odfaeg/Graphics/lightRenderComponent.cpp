#include "../../../include/odfaeg/Graphics/lightRenderComponent.hpp"


#include <memory.h>

using namespace std;
namespace odfaeg {
    namespace graphic {
        #ifdef VULKAN
        LightRenderComponent::LightRenderComponent (RenderWindow& window, int layer, std::string expression,window::ContextSettings settings, bool useThread) :
                HeavyComponent(window, math::Vec3f(window.getView().getPosition().x(), window.getView().getPosition().y(), layer),
                              math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0),
                              math::Vec3f(window.getView().getSize().x() + window.getView().getSize().x() * 0.5f, window.getView().getPosition().y() + window.getView().getSize().y() * 0.5f, layer)),
                view(window.getView()),
                useThread(useThread),
                expression(expression),
                vb(window.getDevice()),
                vbBindlessTex {VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()),
                VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice())},
                vbBindlessTexIndexed {VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()),
                VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice())},
                depthBufferGenerator(window.getDevice()), buildAlphaBufferGenerator(window.getDevice()), specularTextureGenerator(window.getDevice()),bumpTextureGenerator(window.getDevice()), lightMapGenerator(window.getDevice()),
                vkDevice(window.getDevice()),
                depthBuffer(window.getDevice()),
                alphaBuffer(window.getDevice()),
                bumpTexture(window.getDevice()),
                specularTexture(window.getDevice()),
                lightMap(window.getDevice()),
                lightDepthBuffer(window.getDevice()),
                window(window) {
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

                bumpTexture.create(window.getView().getSize().x(), window.getView().getSize().y());
                bumpTexture.setView(view);
                specularTexture.create(window.getView().getSize().x(), window.getView().getSize().y());
                specularTexture.setView(view);
                lightMap.create(window.getView().getSize().x(), window.getView().getSize().y());
                lightMap.setView(view);
                lightMapTile = Sprite(lightMap.getTexture(lightMap.getImageIndex()), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0), IntRect(0, 0, window.getView().getSize().x(), window.getView().getSize().y()));
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
                    if (vkCreateImage(window.getDevice().getDevice(), &imageInfo3, nullptr, &lightDepthTextureImage[i]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de la creation d'une image!");
                    }

                    VkMemoryRequirements memRequirements;
                    vkGetImageMemoryRequirements(window.getDevice().getDevice(), lightDepthTextureImage[i], &memRequirements);

                    VkMemoryAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                    allocInfo.allocationSize = memRequirements.size;
                    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                    if (vkAllocateMemory(window.getDevice().getDevice(), &allocInfo, nullptr, &lightDepthTextureImageMemory[i]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de l'allocation de la memoire d'une image!");
                    }
                    vkBindImageMemory(window.getDevice().getDevice(), lightDepthTextureImage[i], lightDepthTextureImageMemory[i], 0);
                }
                createImageView();
                createSampler();
                lightDepthBuffer.beginRecordCommandBuffers();
                std::vector<VkCommandBuffer> commandBuffers = lightDepthBuffer.getCommandBuffers();
                unsigned int currentFrame =  lightDepthBuffer.getCurrentFrame();
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
                    barrier3.image = lightDepthTextureImage[i];
                    barrier3.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    barrier3.subresourceRange.levelCount = 1;
                    barrier3.subresourceRange.layerCount = 1;
                    vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier3);


                }
                for (unsigned int i = 0; i < lightMap.getSwapchainImagesSize(); i++) {
                    const_cast<Texture&>(lightMap.getTexture(i)).toShaderReadOnlyOptimal(lightDepthBuffer.getCommandBuffers()[lightDepthBuffer.getCurrentFrame()]);
                }


                VkSemaphoreCreateInfo semaphoreInfo{};
                semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                VkSemaphoreTypeCreateInfo timelineCreateInfo{};
                timelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
                timelineCreateInfo.pNext = nullptr;
                timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
                semaphoreInfo.pNext = &timelineCreateInfo;
                offscreenFinishedSemaphore.resize(lightMap.getMaxFramesInFlight());
                for (unsigned int i = 0; i < valuesFinished.size(); i++) {
                    valuesFinished[i] = 0;
                }
                for (unsigned int i = 0; i < lightMap.getMaxFramesInFlight(); i++) {
                    timelineCreateInfo.initialValue = valuesFinished[i];
                    if (vkCreateSemaphore(vkDevice.getDevice(), &semaphoreInfo, nullptr, &offscreenFinishedSemaphore[i]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create compute synchronization objects for a frame!");
                    }
                }
                offscreenLightDepthAlphaFinishedSemaphore.resize(lightMap.getMaxFramesInFlight());
                for (unsigned int i = 0; i < valuesLightDepthAlpha.size(); i++) {
                    valuesLightDepthAlpha[i] = 0;
                }
                for (unsigned int i = 0; i < lightMap.getMaxFramesInFlight(); i++) {
                    timelineCreateInfo.initialValue = valuesLightDepthAlpha[i];
                    if (vkCreateSemaphore(vkDevice.getDevice(), &semaphoreInfo, nullptr, &offscreenLightDepthAlphaFinishedSemaphore[i]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create compute synchronization objects for a frame!");
                    }
                }
                copyFinishedSemaphore.resize(depthBuffer.getMaxFramesInFlight());
                for (unsigned int i = 0; i < copyValues.size(); i++) {
                    copyValues[i] = 0;
                }
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
                if (vkAllocateCommandBuffers(vkDevice.getDevice(), &bufferAllocInfo, lightDepthCommandBuffer.data()) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to allocate command buffers!", 1);
                }
                if (vkAllocateCommandBuffers(vkDevice.getDevice(), &bufferAllocInfo, depthCommandBuffer.data()) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to allocate command buffers!", 1);
                }
                if (vkAllocateCommandBuffers(vkDevice.getDevice(), &bufferAllocInfo, alphaCommandBuffer.data()) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to allocate command buffers!", 1);
                }
                if (vkAllocateCommandBuffers(vkDevice.getDevice(), &bufferAllocInfo, specularCommandBuffer.data()) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to allocate command buffers!", 1);
                }
                if (vkAllocateCommandBuffers(vkDevice.getDevice(), &bufferAllocInfo, bumpCommandBuffer.data()) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to allocate command buffers!", 1);
                }
                if (vkAllocateCommandBuffers(vkDevice.getDevice(), &bufferAllocInfo, lightCommandBuffer.data()) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to allocate command buffers!", 1);
                }
                VkPhysicalDeviceProperties deviceProperties;
                vkGetPhysicalDeviceProperties(vkDevice.getPhysicalDevice(), &deviceProperties);
                alignment = deviceProperties.limits.minStorageBufferOffsetAlignment;
                resolutionPC.resolution = resolution;
                layerPC.resolution = resolution;
                needToUpdateDS = false;
                isSomethingDrawn = false;
                datasReady = false;
                update = true;
            }
            void LightRenderComponent::compileShaders() {
                const std::string indirectRenderingVertexShader = R"(#version 460
                                                                             layout (location = 0) in vec3 position;
                                                                             layout (location = 1) in vec4 color;
                                                                             layout (location = 2) in vec2 texCoords;
                                                                             layout (location = 3) in vec3 normals;
                                                                             layout (location = 4) in int drawableDataID;
                                                                             struct ModelData {
                                                                                mat4 modelMatrix;
                                                                             };
                                                                             struct MaterialData {
                                                                                vec2 uvScale;
                                                                                vec2 uvOffset;
                                                                                uint textureIndex;
                                                                                uint bumpTextureIndex;
                                                                                uint layer;
                                                                                float specularIntensity;
                                                                                float specularPower;
                                                                                uint padding1;
                                                                                uint padding2;
                                                                                uint padding3;
                                                                                vec4 lightCenter;
                                                                                vec4 lightColor;
                                                                             };
                                                                             layout (push_constant)uniform PushConsts {
                                                                                 mat4 projectionMatrix;
                                                                                 layout (offset = 64) mat4 viewMatrix;
                                                                             } pushConsts;
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

                                                                                 gl_Position = pushConsts.projectionMatrix * pushConsts.viewMatrix * model.modelMatrix * vec4(position, 1.f);
                                                                                 fTexCoords = texCoords * material.uvScale + material.uvOffset;
                                                                                 frontColor = color;
                                                                                 texIndex = textureIndex;
                                                                                 layer = l;
                                                                                 normal = normals;
                                                                                 drawableID = drawableDataID;
                                                                             }
                                                                             )";
                        const std::string bumpIndirectRenderingVertexShader = R"(#version 460
                                                                             layout (location = 0) in vec3 position;
                                                                             layout (location = 1) in vec4 color;
                                                                             layout (location = 2) in vec2 texCoords;
                                                                             layout (location = 3) in vec3 normals;
                                                                             layout (location = 4) in int drawableDataID;

                                                                             struct ModelData {
                                                                                mat4 modelMatrix;
                                                                             };
                                                                             struct MaterialData {
                                                                                vec2 uvScale;
                                                                                vec2 uvOffset;
                                                                                uint textureIndex;
                                                                                uint bumpTextureIndex;
                                                                                uint layer;
                                                                                float specularIntensity;
                                                                                float specularPower;
                                                                                uint padding1;
                                                                                uint padding2;
                                                                                uint padding3;
                                                                                vec4 lightCenter;
                                                                                vec4 lightColor;

                                                                             };
                                                                             layout (push_constant)uniform PushConsts {
                                                                                 mat4 projectionMatrix;
                                                                                 layout (offset = 64) mat4 viewMatrix;
                                                                             } pushConsts;
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
                                                                                 uint textureIndex = material.bumpTextureIndex;
                                                                                 uint l = material.layer;

                                                                                 gl_Position = pushConsts.projectionMatrix * pushConsts.viewMatrix * model.modelMatrix * vec4(position, 1.f);
                                                                                 fTexCoords = texCoords * material.uvScale + material.uvOffset;
                                                                                 frontColor = color;
                                                                                 texIndex = textureIndex;
                                                                                 layer = l;
                                                                                 normal = normals;
                                                                                 drawableID = drawableDataID;
                                                                             }
                                                                             )";
                        const std::string specularIndirectRenderingVertexShader = R"(#version 460
                                                                                     layout (location = 0) in vec3 position;
                                                                                     layout (location = 1) in vec4 color;
                                                                                     layout (location = 2) in vec2 texCoords;
                                                                                     layout (location = 3) in vec3 normals;
                                                                                     layout (location = 4) in int drawableDataID;
                                                                                     struct ModelData {
                                                                                        mat4 modelMatrix;
                                                                                     };
                                                                                     struct MaterialData {
                                                                                        vec2 uvScale;
                                                                                        vec2 uvOffset;
                                                                                        uint textureIndex;
                                                                                        uint bumpTextureIndex;
                                                                                        uint layer;
                                                                                        float specularIntensity;
                                                                                        float specularPower;
                                                                                        uint padding1;
                                                                                        uint padding2;
                                                                                        uint padding3;
                                                                                        vec4 lightCenter;
                                                                                        vec4 lightColor;

                                                                                     };
                                                                                     layout (push_constant)uniform PushConsts {
                                                                                         mat4 projectionMatrix;
                                                                                         layout (offset = 64) mat4 viewMatrix;
                                                                                     } pushConsts;
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
                                                                                     layout (location = 5) out vec2 specular;
                                                                                     layout (location = 6) out int drawableID;
                                                                                     void main() {
                                                                                         gl_PointSize = 2.0f;
                                                                                         ModelData model = modelDatas[gl_InstanceIndex];
                                                                                         MaterialData material = materialDatas[gl_DrawID];
                                                                                         uint textureIndex = material.textureIndex;
                                                                                         uint l = material.layer;

                                                                                         gl_Position = pushConsts.projectionMatrix * pushConsts.viewMatrix * model.modelMatrix * vec4(position, 1.f);
                                                                                         fTexCoords = texCoords * material.uvScale + material.uvOffset;
                                                                                         frontColor = color;
                                                                                         texIndex = textureIndex;
                                                                                         layer = l;
                                                                                         specular = vec2(material.specularIntensity, material.specularPower);
                                                                                         normal = normals;
                                                                                         drawableID = drawableDataID;
                                                                                     }
                                                                                     )";
                        const std::string perPixLightingIndirectRenderingVertexShader = R"(#version 460
                                                                                           #extension GL_EXT_debug_printf : enable
                                                                                      layout (location = 0) in vec3 position;
                                                                                      layout (location = 1) in vec4 color;
                                                                                      layout (location = 2) in vec2 texCoords;
                                                                                      layout (location = 3) in vec3 normals;
                                                                                      layout (location = 4) in int drawableDataID;
                                                                                      layout (push_constant)uniform PushConsts {
                                                                                         mat4 projectionMatrix;
                                                                                         layout (offset = 64) mat4 viewMatrix;
                                                                                         layout (offset = 128) mat4 viewportMatrix;
                                                                                      } pushConsts;
                                                                                      struct ModelData {
                                                                                         mat4 modelMatrix;
                                                                                      };
                                                                                      struct MaterialData {
                                                                                         vec2 uvScale;
                                                                                         vec2 uvOffset;
                                                                                         uint textureIndex;
                                                                                         uint bumpTextureIndex;
                                                                                         uint layer;
                                                                                         float specularIntensity;
                                                                                         float specularPower;
                                                                                         uint padding1;
                                                                                         uint padding2;
                                                                                         uint padding3;
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
                                                                                      layout (location = 7) out float isOrthoProj;
                                                                                      layout (location = 8) out int drawableID;
                                                                                      void main() {
                                                                                         gl_PointSize = 2.0f;
                                                                                         ModelData model = modelDatas[gl_InstanceIndex];
                                                                                         MaterialData material = materialDatas[gl_DrawID];
                                                                                         uint l = material.layer;

                                                                                         gl_Position = pushConsts.projectionMatrix * pushConsts.viewMatrix * model.modelMatrix * vec4(position, 1.f);
                                                                                         //debugPrintfEXT("vertex position %v4f", gl_Position);

                                                                                         fTexCoords = texCoords * material.uvScale + material.uvOffset;
                                                                                         frontColor = color;
                                                                                         layer = l;
                                                                                         vec4 coords = vec4(material.lightCenter.xyz, 1);
                                                                                         coords = pushConsts.projectionMatrix * pushConsts.viewMatrix * model.modelMatrix * coords;

                                                                                         coords = coords / coords.w;
                                                                                         coords = pushConsts.viewportMatrix * coords;
                                                                                         coords.w = material.lightCenter.w;

                                                                                         lightPos = coords;
                                                                                         lightColor = material.lightColor;

                                                                                         normal = normals;
                                                                                         drawableID = drawableDataID;
                                                                                         isOrthoProj = pushConsts.projectionMatrix[3][3];
                                                                                      }
                                                                                      )";

                        const std::string depthGenFragShader = R"(#version 460
                                                                          #extension GL_ARB_fragment_shader_interlock : require
                                                                          #extension GL_EXT_nonuniform_qualifier : enable
                                                                          #extension GL_EXT_debug_printf : enable

                                                                          layout (location = 0) in vec2 fTexCoords;
                                                                          layout (location = 1) in vec4 frontColor;
                                                                          layout (location = 2) in flat uint texIndex;
                                                                          layout (location = 3) in flat uint layer;
                                                                          layout (location = 4) in vec3 normal;
                                                                          layout (location = 5) in flat int drawableID;
                                                                          layout(set = 0, binding = 3) uniform sampler2D textures[];
                                                                          layout(set = 0, binding = 2, rgba32f) uniform coherent image2D depthBuffer;
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
                                                                              //debugPrintfEXT("depth frag shader");
                                                                              if (/*l > depth.y() || l == depth.y() &&*/ z > depth.z) {
                                                                                if (texel.a < 0.1) discard;
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
                                                                      #extension GL_EXT_debug_printf : enable
                                                                      layout (location = 0) in vec2 fTexCoords;
                                                                      layout (location = 1) in vec4 frontColor;
                                                                      layout (location = 2) in flat uint texIndex;
                                                                      layout (location = 3) in flat uint layer;
                                                                      layout (location = 4) in vec3 normal;
                                                                      layout (location = 5) in flat int drawableID;
                                                                      layout(set = 0, binding = 4) uniform sampler2D textures[];
                                                                      layout(binding = 2, rgba32f) uniform coherent image2D alphaBuffer;
                                                                      layout(set = 0, binding =3) uniform sampler2D depthBuffer[];

                                                                      layout (location = 0) out vec4 fColor;
                                                                      layout (push_constant) uniform PushConsts {
                                                                          layout (offset = 128) vec4 resolution;
                                                                          layout (offset = 144) uint nbLayers;
                                                                          layout (offset = 148) uint imageIndex;
                                                                      } pushConsts;
                                                                      void main() {
                                                                          vec4 texel = (texIndex != 0) ? frontColor * texture(textures[texIndex-1], fTexCoords.xy) : frontColor;
                                                                          float current_alpha = texel.a;
                                                                          vec4 depth = texture(depthBuffer[pushConsts.imageIndex], gl_FragCoord.xy / pushConsts.resolution.xy);
                                                                          beginInvocationInterlockARB();
                                                                          vec4 alpha = imageLoad(alphaBuffer,ivec2(gl_FragCoord.xy));
                                                                          float l = layer;
                                                                          float z = gl_FragCoord.z;
                                                                          //debugPrintfEXT("alpha frag shader");
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
                                                                     #extension GL_EXT_debug_printf : enable
                                                                     layout (location = 0) in vec2 fTexCoords;
                                                                     layout (location = 1) in vec4 frontColor;
                                                                     layout (location = 2) in flat uint texIndex;
                                                                     layout (location = 3) in flat uint layer;
                                                                     layout (location = 4) in vec3 normal;
                                                                     layout (location = 5) in vec2 specular;
                                                                     layout (location = 6) in flat int drawableID;
                                                                     layout (push_constant) uniform PushConsts {
                                                                         layout (offset = 128) vec4 resolution;
                                                                         layout (offset = 144) float maxM;
                                                                         layout (offset = 148) float maxP;
                                                                         layout (offset = 152) uint imageIndex;
                                                                     } pushConsts;
                                                                     layout(set = 0, binding = 3) uniform sampler2D textures[];
                                                                     layout(set = 0, binding = 2) uniform sampler2D depthBuffer[];
                                                                     layout (location = 0) out vec4 fColor;
                                                                     void main() {
                                                                        vec4 texel = (texIndex != 0) ? frontColor * texture(textures[texIndex-1], fTexCoords.xy) : frontColor;
                                                                        vec4 depth = texture(depthBuffer[pushConsts.imageIndex], gl_FragCoord.xy / pushConsts.resolution.xy);
                                                                        float z = gl_FragCoord.z;
                                                                        float intensity = (pushConsts.maxM != 0.f) ? specular.x / pushConsts.maxM : 0.f;
                                                                        float power = (pushConsts.maxP != 0.f) ? specular.y / pushConsts.maxP : 0.f;
                                                                        //debugPrintfEXT("stencil frag shader");
                                                                        if (/*layer > depth.y || layer == depth.y &&*/ z > depth.z)
                                                                            fColor = vec4(intensity, power, z, texel.a);
                                                                        else
                                                                            fColor = vec4(0, 0, 0, 0);
                                                                     }
                                                                  )";
                        const std::string bumpGenFragShader =    R"(#version 460
                                                                    #extension GL_EXT_nonuniform_qualifier : enable
                                                                 layout(set = 0, binding = 3) uniform sampler2D textures[];
                                                                 layout (location = 0) in vec2 fTexCoords;
                                                                 layout (location = 1) in vec4 frontColor;
                                                                 layout (location = 2) in flat uint texIndex;
                                                                 layout (location = 3) in flat uint layer;
                                                                 layout (location = 4) in vec3 normal;
                                                                 layout (location = 5) in flat int drawableID;
                                                                 layout(set = 0, binding = 2) uniform sampler2D depthBuffer[];
                                                                 layout (push_constant) uniform PushConsts {
                                                                    layout (offset = 128) vec4 resolution;
                                                                    layout (offset = 144) uint nbLayers;
                                                                    layout (offset = 148) uint imageIndex;
                                                                 } pushConsts;
                                                                 layout (location = 0) out vec4 fColor;
                                                                 void main() {
                                                                     vec4 color = (texIndex != 0) ? texture(textures[texIndex-1], fTexCoords.xy) : vec4(0, 0, 0, 0);
                                                                     vec4 depth = texture(depthBuffer[pushConsts.imageIndex], gl_FragCoord.xy / pushConsts.resolution.xy);
                                                                     if (/*layer > depth.y() || layer == depth.y() &&*/ gl_FragCoord.z > depth.z) {
                                                                        fColor = color;
                                                                     } else {
                                                                        fColor = vec4(0, 0, 0, 0);
                                                                     }
                                                                 }
                                                                 )";
                        const std::string perPixLightingFragmentShader =  R"(#version 460
                                                                             #extension GL_EXT_nonuniform_qualifier : enable
                                                                             #extension GL_EXT_debug_printf : enable
                                                                 layout (location = 0) in vec2 fTexCoords;
                                                                 layout (location = 1) in vec4 frontColor;
                                                                 layout (location = 2) in flat uint texIndex;
                                                                 layout (location = 3) in flat uint layer;
                                                                 layout (location = 4) in vec3 normal;
                                                                 layout (location = 5) in flat vec4 lightPos;
                                                                 layout (location = 6) in flat vec4 lightColor;
                                                                 layout (location = 7) in float isOrthoProj;
                                                                 layout (location = 8) in flat int drawableID;
                                                                 const vec2 size = vec2(2.0,0.0);
                                                                 const ivec3 off = ivec3(-1,0,1);
                                                                 layout(set = 0, binding = 0) uniform sampler2D depthTexture[];
                                                                 layout(set = 0, binding = 1) uniform sampler2D bumpMap[];
                                                                 layout(set = 0, binding = 2) uniform sampler2D specularTexture[];
                                                                 layout(set = 0, binding = 3) uniform sampler2D alphaMap[];
                                                                 layout (location = 0) out vec4 fColor;
                                                                 layout (push_constant) uniform PushConsts {
                                                                     layout (offset = 192) vec4 resolution;
                                                                     layout (offset = 208) float near;
                                                                     layout (offset = 212) float far;
                                                                     layout (offset = 216) uint imageIndex;
                                                                 } pushConsts;
                                                         void main () {
                                                             vec4 depth = texture(depthTexture[pushConsts.imageIndex], gl_FragCoord.xy / pushConsts.resolution.xy);
                                                             vec4 alpha = texture(alphaMap[pushConsts.imageIndex], gl_FragCoord.xy / pushConsts.resolution.xy);
                                                             float s01 = textureOffset(depthTexture[pushConsts.imageIndex], gl_FragCoord.xy / pushConsts.resolution.xy, off.xy).z;
                                                             float s21 = textureOffset(depthTexture[pushConsts.imageIndex], gl_FragCoord.xy / pushConsts.resolution.xy, off.zy).z;
                                                             float s10 = textureOffset(depthTexture[pushConsts.imageIndex], gl_FragCoord.xy / pushConsts.resolution.xy, off.yx).z;
                                                             float s12 = textureOffset(depthTexture[pushConsts.imageIndex], gl_FragCoord.xy / pushConsts.resolution.xy, off.yz).z;
                                                             vec3 va = normalize (vec3(size.xy, s21 - s01));
                                                             vec3 vb = normalize (vec3(size.yx, s12 - s10));
                                                             vec3 n = vec3(cross(va, vb));
                                                             vec4 bump = texture(bumpMap[pushConsts.imageIndex], gl_FragCoord.xy / pushConsts.resolution.xy);
                                                             vec4 specularInfos = texture(specularTexture[pushConsts.imageIndex], gl_FragCoord.xy / pushConsts.resolution.xy);
                                                             bool useOrthoProj = (isOrthoProj != 0);

                                                             float pixelViewZ;
                                                             float lightViewZ;
                                                             if (useOrthoProj) {
                                                                pixelViewZ = pushConsts.near + gl_FragCoord.z * (pushConsts.far - pushConsts.near);
                                                                lightViewZ = pushConsts.near + lightPos.z * (pushConsts.far - pushConsts.near);
                                                             } else {
                                                                float ndcZ = gl_FragCoord.z * 2.0 - 1.0; // Convertit de [0,1] à [-1,1]
                                                                pixelViewZ = (2.0 * pushConsts.near * pushConsts.far) /
                                                               (ndcZ * (pushConsts.near - pushConsts.far) - (pushConsts.near + pushConsts.far));
                                                                ndcZ = lightPos.z * 2.0 - 1.0; // Convertit de [0,1] à [-1,1]
                                                                lightViewZ = (2.0 * pushConsts.near * pushConsts.far) /
                                                                (ndcZ * (pushConsts.near - pushConsts.far) - (pushConsts.near + pushConsts.far));
                                                             }
                                                             vec3 sLightPos = vec3 (lightPos.x, lightPos.y,lightViewZ);
                                                             float radius = lightPos.w;
                                                             vec3 pixPos = vec3 (gl_FragCoord.x, gl_FragCoord.y, pixelViewZ);
                                                             vec3 viewPos = vec3(pushConsts.resolution.x * 0.5f, pushConsts.resolution.y * 0.5f, 0);
                                                             vec3 vertexToLight = sLightPos - pixPos;
                                                             //debugPrintfEXT("light pos : %v3f pix pos : %v3f", sLightPos, pixPos);

                                                             if (bump.x != 0 || bump.y != 0 || bump.z != 0) {
                                                                 vec3 tmpNormal = (n.xyz);
                                                                 vec3 tangeant = normalize (vec3(size.xy, s21 - s01));
                                                                 vec3 binomial = normalize (vec3(size.yx, s12 - s10));
                                                                 n.x = dot(bump.xyz, tangeant);
                                                                 n.y = dot(bump.xyz, binomial);
                                                                 n.z = dot(bump.xyz, tmpNormal);
                                                             }
                                                             //debugPrintfEXT("depth : %f", depth.z);
                                                             if (/*layer > depth.y || layer == depth.y &&*/ gl_FragCoord.z > depth.z) {
                                                                 vec4 specularColor = vec4(0, 0, 0, 0);

                                                                 float attenuation = 1.f - length(vertexToLight) / radius;
                                                                 //debugPrintfEXT("vertex to light, radius, attenuation : %v3f, %f, %f", vertexToLight, radius, attenuation);

                                                                 vec3 pixToView = pixPos - viewPos;
                                                                 float normalLength = dot(n.xyz, vertexToLight);
                                                                 vec3 lightReflect = vertexToLight + 2 * (n.xyz * normalLength - vertexToLight);
                                                                 float m = specularInfos.r;
                                                                 float p = specularInfos.g;
                                                                 float specularFactor = dot(normalize(pixToView), normalize(lightReflect));
                                                                 specularFactor = pow (specularFactor, p);
                                                                 if (specularFactor > 0) {
                                                                     specularColor = vec4(lightColor.rgb, 1) * m * specularFactor;
                                                                 }
                                                                 if (n.x != 0 || n.y != 0 || n.z != 0 && vertexToLight.z > 0.f) {
                                                                     vec3 dirToLight = normalize(vertexToLight.xyz);
                                                                     float nDotl = max(dot (dirToLight, n.xyz), 0.0);

                                                                     attenuation *= nDotl;


                                                                 }

                                                                 fColor = vec4(lightColor.rgb, 1) * max(0.0f, attenuation) + specularColor * (1 - alpha.a);
                                                             } else {
                                                                 discard;
                                                             }
                                                         }
                                                     )";
                        if (!depthBufferGenerator.loadFromMemory(indirectRenderingVertexShader, depthGenFragShader))
                            throw core::Erreur(50, "Failed to load depth buffer generator shader", 0);
                        if (!specularTextureGenerator.loadFromMemory(specularIndirectRenderingVertexShader, specularGenFragShader))
                            throw core::Erreur(52, "Failed to load specular texture generator shader", 0);

                        if (!bumpTextureGenerator.loadFromMemory(bumpIndirectRenderingVertexShader, bumpGenFragShader))
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
                    throw core::Erreur(0, "échec de la création d'une command pool!", 1);
                }
                if (vkCreateCommandPool(vkDevice.getDevice(), &poolInfo, nullptr, &secondaryBufferCommandPool) != VK_SUCCESS) {
                    throw core::Erreur(0, "échec de la création d'une command pool!", 1);
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

            void LightRenderComponent::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandBuffer cmd) {
                //std::cout<<"opy buffers"<<std::endl;
                if (srcBuffer != nullptr && dstBuffer != nullptr) {
                    VkBufferCopy copyRegion{};
                    copyRegion.size = size;
                    vkCmdCopyBuffer(cmd, srcBuffer, dstBuffer, 1, &copyRegion);
                }
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
                lightDepthBuffer.clear(Color::Transparent);
                std::vector<VkCommandBuffer> commandBuffers = lightDepthBuffer.getCommandBuffers();
                unsigned int currentFrame = lightDepthBuffer.getCurrentFrame();
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

                depthBuffer.clear(Color::Transparent);
                commandBuffers = depthBuffer.getCommandBuffers();
                currentFrame = depthBuffer.getCurrentFrame();

                vkCmdClearColorImage(commandBuffers[currentFrame], depthTextureImage[currentFrame], VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &subresRange);

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

                specularTexture.clear(Color::Transparent);
                bumpTexture.clear(Color::Transparent);
                Color ambientColor = g2d::AmbientLight::getAmbientLight().getColor();
                lightMap.beginRecordCommandBuffers();
                const_cast<Texture&>(lightMap.getTexture(lightMap.getImageIndex())).toColorAttachmentOptimal(lightMap.getCommandBuffers()[lightMap.getCurrentFrame()]);
                lightMap.clear(ambientColor);

            }
            void LightRenderComponent::resetBuffers() {
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
            void LightRenderComponent::createImageView() {
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
                    viewInfo3.image = lightDepthTextureImage[i];
                    viewInfo3.viewType = VK_IMAGE_VIEW_TYPE_2D;
                    viewInfo3.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                    viewInfo3.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    viewInfo3.subresourceRange.baseMipLevel = 0;
                    viewInfo3.subresourceRange.levelCount = 1;
                    viewInfo3.subresourceRange.baseArrayLayer = 0;
                    viewInfo3.subresourceRange.layerCount = 1;

                    if (vkCreateImageView(vkDevice.getDevice(), &viewInfo3, nullptr, &lightDepthTextureImageView[i]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create head ptr texture image view!");
                    }
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
                    if (vkCreateSampler(vkDevice.getDevice(), &samplerInfo3, nullptr, &lightDepthTextureSampler[i]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create texture sampler!");
                    }
                }
            }
            void LightRenderComponent::createDescriptorsAndPipelines () {
                RenderStates states;
                if (useThread) {
                    states.shader = &depthBufferGenerator;
                    createDescriptorSetLayout(states);
                    for (unsigned int p = 0; p < (Batcher::nbPrimitiveTypes - 1); p++) {
                        createDescriptorPool(p, states);
                        allocateDescriptorSets(p, states);
                    }
                    states.shader = &buildAlphaBufferGenerator;
                    createDescriptorSetLayout(states);
                    for (unsigned int p = 0; p < (Batcher::nbPrimitiveTypes - 1); p++) {
                        createDescriptorPool(p, states);
                        allocateDescriptorSets(p, states);
                    }
                    states.shader = &specularTextureGenerator;
                    createDescriptorSetLayout(states);
                    for (unsigned int p = 0; p < (Batcher::nbPrimitiveTypes - 1); p++) {
                        createDescriptorPool(p, states);
                        allocateDescriptorSets(p, states);
                    }
                    states.shader = &bumpTextureGenerator;
                    createDescriptorSetLayout(states);
                    for (unsigned int p = 0; p < (Batcher::nbPrimitiveTypes - 1); p++) {
                        createDescriptorPool(p, states);
                        allocateDescriptorSets(p, states);
                    }
                    states.shader = &lightMapGenerator;
                    createDescriptorSetLayout(states);
                    for (unsigned int p = 0; p < (Batcher::nbPrimitiveTypes - 1); p++) {
                        createDescriptorPool(p, states);
                        allocateDescriptorSets(p, states);
                    }
                } else {
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
                            if (LIGHTNBDEPTHSTENCIL * none.nbBlendModes > pipelineLayoutInfo[i][j].size()) {
                                pipelineLayoutInfo[i][j].resize(LIGHTNBDEPTHSTENCIL * none.nbBlendModes);
                                depthStencilCreateInfo[i][j].resize(LIGHTNBDEPTHSTENCIL* none.nbBlendModes);
                            }
                        }
                    }
                    specularTexture.enableDepthTest(true);
                    depthBuffer.enableDepthTest(true);
                    lightDepthBuffer.enableDepthTest(true);
                    alphaBuffer.enableDepthTest(true);
                    bumpTexture.enableDepthTest(true);
                    lightMap.enableDepthTest(true);
                    states.shader = &depthBufferGenerator;
                    for (unsigned int b = 0; b < blendModes.size(); b++) {
                        states.blendMode = blendModes[b];
                        states.blendMode.updateIds();
                        for (unsigned int j = 0; j < LIGHTNBDEPTHSTENCIL; j++) {
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
                                    pipelineLayoutInfo[depthBufferGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][LIGHTNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = push_constants.data();
                                    pipelineLayoutInfo[depthBufferGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][LIGHTNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 2;
                                   depthStencilCreateInfo[depthBufferGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][LIGHTNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                                   depthStencilCreateInfo[depthBufferGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][LIGHTNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = {};
                                   depthStencilCreateInfo[depthBufferGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][LIGHTNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back = {};
                                   depthBuffer.createGraphicPipeline(static_cast<PrimitiveType>(i), states, LIGHTNODEPTHNOSTENCIL, LIGHTNBDEPTHSTENCIL);
                                }
                            }
                        }
                    }
                }
                {
                    std::vector<std::vector<std::vector<VkPipelineLayoutCreateInfo>>>& pipelineLayoutInfo = lightDepthBuffer.getPipelineLayoutCreateInfo();
                    std::vector<std::vector<std::vector<VkPipelineDepthStencilStateCreateInfo>>>& depthStencilCreateInfo = lightDepthBuffer.getDepthStencilCreateInfo();
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
                            if (LIGHTNBDEPTHSTENCIL * none.nbBlendModes > pipelineLayoutInfo[i][j].size()) {
                                pipelineLayoutInfo[i][j].resize(LIGHTNBDEPTHSTENCIL * none.nbBlendModes);
                                depthStencilCreateInfo[i][j].resize(LIGHTNBDEPTHSTENCIL* none.nbBlendModes);
                            }
                        }
                    }
                    for (unsigned int b = 0; b < blendModes.size(); b++) {
                        states.blendMode = blendModes[b];
                        states.blendMode.updateIds();
                        for (unsigned int j = 0; j < LIGHTNBDEPTHSTENCIL; j++) {
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
                                    pipelineLayoutInfo[depthBufferGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][LIGHTNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = push_constants.data();
                                    pipelineLayoutInfo[depthBufferGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][LIGHTNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 2;
                                   depthStencilCreateInfo[depthBufferGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][LIGHTNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                                   depthStencilCreateInfo[depthBufferGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][LIGHTNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = {};
                                   depthStencilCreateInfo[depthBufferGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][LIGHTNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back = {};
                                   lightDepthBuffer.createGraphicPipeline(static_cast<PrimitiveType>(i), states, LIGHTNODEPTHNOSTENCIL, LIGHTNBDEPTHSTENCIL);
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
                            if (LIGHTNBDEPTHSTENCIL * none.nbBlendModes > pipelineLayoutInfo[i][j].size()) {
                                pipelineLayoutInfo[i][j].resize(LIGHTNBDEPTHSTENCIL * none.nbBlendModes);
                                depthStencilCreateInfo[i][j].resize(LIGHTNBDEPTHSTENCIL* none.nbBlendModes);
                            }
                        }
                    }
                    states.shader = &buildAlphaBufferGenerator;
                    for (unsigned int b = 0; b < blendModes.size(); b++) {
                        states.blendMode = blendModes[b];
                        states.blendMode.updateIds();

                        for (unsigned int j = 0; j < LIGHTNBDEPTHSTENCIL; j++) {
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
                                    push_constant2.size = sizeof(LayerPC) + sizeof(unsigned int);
                                    //this push constant range is accessible only in the vertex shader
                                    push_constant2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                                    push_constants[1] = push_constant2;

                                    pipelineLayoutInfo[buildAlphaBufferGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][LIGHTNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = push_constants.data();
                                    pipelineLayoutInfo[buildAlphaBufferGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][LIGHTNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 2;
                                    depthStencilCreateInfo[buildAlphaBufferGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][LIGHTNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                                    depthStencilCreateInfo[buildAlphaBufferGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][LIGHTNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = {};
                                    depthStencilCreateInfo[buildAlphaBufferGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][LIGHTNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back = {};
                                    alphaBuffer.createGraphicPipeline(static_cast<PrimitiveType>(i), states, LIGHTNODEPTHNOSTENCIL, LIGHTNBDEPTHSTENCIL);
                                }
                            }
                        }
                    }
                }
                {
                    std::vector<std::vector<std::vector<VkPipelineLayoutCreateInfo>>>& pipelineLayoutInfo = specularTexture.getPipelineLayoutCreateInfo();
                    std::vector<std::vector<std::vector<VkPipelineDepthStencilStateCreateInfo>>>& depthStencilCreateInfo = specularTexture.getDepthStencilCreateInfo();
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
                            if (LIGHTNBDEPTHSTENCIL * none.nbBlendModes > pipelineLayoutInfo[i][j].size()) {
                                pipelineLayoutInfo[i][j].resize(LIGHTNBDEPTHSTENCIL * none.nbBlendModes);
                                depthStencilCreateInfo[i][j].resize(LIGHTNBDEPTHSTENCIL* none.nbBlendModes);
                            }
                        }
                    }
                    for (unsigned int b = 0; b < blendModes.size(); b++) {
                        states.blendMode = blendModes[b];
                        states.blendMode.updateIds();
                        states.shader = &specularTextureGenerator;
                        for (unsigned int j = 0; j < LIGHTNBDEPTHSTENCIL; j++) {
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
                                    push_constant2.size = sizeof(MaxSpecPC) + sizeof(unsigned int);
                                    //this push constant range is accessible only in the vertex shader
                                    push_constant2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                                    push_constants[1] = push_constant2;

                                    pipelineLayoutInfo[specularTextureGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][LIGHTNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = push_constants.data();
                                    pipelineLayoutInfo[specularTextureGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][LIGHTNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 2;
                                    depthStencilCreateInfo[specularTextureGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][LIGHTNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                                    depthStencilCreateInfo[specularTextureGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][LIGHTNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = {};
                                    depthStencilCreateInfo[specularTextureGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][LIGHTNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back = {};
                                    specularTexture.createGraphicPipeline(static_cast<PrimitiveType>(i), states, LIGHTNODEPTHNOSTENCIL, LIGHTNBDEPTHSTENCIL);
                                }
                            }
                        }
                    }
                }
                {
                    std::vector<std::vector<std::vector<VkPipelineLayoutCreateInfo>>>& pipelineLayoutInfo = bumpTexture.getPipelineLayoutCreateInfo();
                    std::vector<std::vector<std::vector<VkPipelineDepthStencilStateCreateInfo>>>& depthStencilCreateInfo = bumpTexture.getDepthStencilCreateInfo();
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
                            if (LIGHTNBDEPTHSTENCIL * none.nbBlendModes > pipelineLayoutInfo[i][j].size()) {
                                pipelineLayoutInfo[i][j].resize(LIGHTNBDEPTHSTENCIL * none.nbBlendModes);
                                depthStencilCreateInfo[i][j].resize(LIGHTNBDEPTHSTENCIL* none.nbBlendModes);
                            }
                        }
                    }
                    for (unsigned int b = 0; b < blendModes.size(); b++) {
                        states.blendMode = blendModes[b];
                        states.blendMode.updateIds();
                        states.shader = &bumpTextureGenerator;
                        for (unsigned int j = 0; j < LIGHTNBDEPTHSTENCIL; j++) {
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
                                    push_constant2.size = sizeof(LayerPC) + sizeof(unsigned int);
                                    //this push constant range is accessible only in the vertex shader
                                    push_constant2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                                    push_constants[1] = push_constant2;


                                    pipelineLayoutInfo[bumpTextureGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][LIGHTNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = push_constants.data();
                                    pipelineLayoutInfo[bumpTextureGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][LIGHTNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 2;
                                    depthStencilCreateInfo[bumpTextureGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][LIGHTNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                                    depthStencilCreateInfo[bumpTextureGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][LIGHTNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = {};
                                    depthStencilCreateInfo[bumpTextureGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][LIGHTNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back = {};
                                    bumpTexture.createGraphicPipeline(static_cast<PrimitiveType>(i), states, LIGHTNODEPTHNOSTENCIL, LIGHTNBDEPTHSTENCIL);
                                }
                            }
                        }
                    }
                }
                {
                    std::vector<std::vector<std::vector<VkPipelineLayoutCreateInfo>>>& pipelineLayoutInfo = lightMap.getPipelineLayoutCreateInfo();
                    std::vector<std::vector<std::vector<VkPipelineDepthStencilStateCreateInfo>>>& depthStencilCreateInfo = lightMap.getDepthStencilCreateInfo();
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
                            if (LIGHTNBDEPTHSTENCIL * none.nbBlendModes > pipelineLayoutInfo[i][j].size()) {
                                pipelineLayoutInfo[i][j].resize(LIGHTNBDEPTHSTENCIL * none.nbBlendModes);
                                depthStencilCreateInfo[i][j].resize(LIGHTNBDEPTHSTENCIL* none.nbBlendModes);
                            }
                        }
                    }
                    for (unsigned int b = 0; b < blendModes.size(); b++) {
                        states.blendMode = blendModes[b];
                        states.blendMode.updateIds();
                        states.shader = &lightMapGenerator;
                        for (unsigned int j = 0; j < LIGHTNBDEPTHSTENCIL; j++) {
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
                                    push_constant2.offset = 192;
                                    //this push constant range takes up the size of a MeshPushConstants struct
                                    push_constant2.size = sizeof(ResolutionPC) + sizeof(unsigned int);
                                    //this push constant range is accessible only in the vertex shader
                                    push_constant2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                                    push_constants[1] = push_constant2;

                                    pipelineLayoutInfo[lightMapGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][LIGHTNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = push_constants.data();
                                    pipelineLayoutInfo[lightMapGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][LIGHTNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 2;
                                    depthStencilCreateInfo[lightMapGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][LIGHTNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                                    depthStencilCreateInfo[lightMapGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][LIGHTNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = {};
                                    depthStencilCreateInfo[lightMapGenerator.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][LIGHTNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back = {};
                                    lightMap.createGraphicPipeline(static_cast<PrimitiveType>(i), states, LIGHTNODEPTHNOSTENCIL, LIGHTNBDEPTHSTENCIL);
                                }
                            }
                        }
                    }
                }
            }
            void LightRenderComponent::launchRenderer() {
                if (useThread) {
                    stop = false;
                    getListener().launch();
                }
            }
            void LightRenderComponent::stopRenderer() {
                stop = true;
                cv.notify_all();
                getListener().stop();
            }
            unsigned int LightRenderComponent::align(unsigned int offset) {
            ////std::cout << "alignment = " << alignment << std::endl;
                return (offset + alignment - 1) & ~(alignment - 1);
            }
            void LightRenderComponent::createDescriptorPool(unsigned int p, RenderStates states) {
                Shader* shader = const_cast<Shader*>(states.shader);
                if (shader == &depthBufferGenerator) {
                    {
                        std::vector<VkDescriptorPool>& descriptorPool = depthBuffer.getDescriptorPool();
                        if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorPool.size())
                            descriptorPool.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                        unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                        std::array<VkDescriptorPoolSize, 4> poolSizes;
                        std::vector<Texture*> allTextures = Texture::getAllTextures();
                        poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                        poolSizes[0].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());
                        poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                        poolSizes[1].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());
                        poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        poolSizes[2].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight() * MAX_TEXTURES);
                        poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
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
                    }
                    {
                        std::vector<VkDescriptorPool>& descriptorPool = lightDepthBuffer.getDescriptorPool();
                        if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorPool.size())
                            descriptorPool.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                        unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                        std::array<VkDescriptorPoolSize, 4> poolSizes;
                        std::vector<Texture*> allTextures = Texture::getAllTextures();

                        poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                        poolSizes[0].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());
                        poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                        poolSizes[1].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());
                        poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        poolSizes[2].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight() * MAX_TEXTURES);
                        poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
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

                    }
                } else if (shader == &buildAlphaBufferGenerator) {
                    std::vector<VkDescriptorPool>& descriptorPool = alphaBuffer.getDescriptorPool();
                    if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                    unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                    std::array<VkDescriptorPoolSize, 5> poolSizes;
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight());
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight());
                    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[2].descriptorCount = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight() * MAX_TEXTURES);
                    poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    poolSizes[3].descriptorCount = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight());
                    poolSizes[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[4].descriptorCount = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight()) * lightDepthBuffer.getSwapchainImagesSize();


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
                    std::vector<VkDescriptorPool>& descriptorPool = specularTexture.getDescriptorPool();
                    if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                    unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                    std::array<VkDescriptorPoolSize, 4> poolSizes;
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(specularTexture.getMaxFramesInFlight());
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(specularTexture.getMaxFramesInFlight());
                    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[2].descriptorCount = static_cast<uint32_t>(specularTexture.getMaxFramesInFlight() * MAX_TEXTURES);
                    poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[3].descriptorCount = static_cast<uint32_t>(specularTexture.getMaxFramesInFlight()) * depthBuffer.getSwapchainImagesSize();

                    VkDescriptorPoolCreateInfo poolInfo{};
                    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
                    poolInfo.pPoolSizes = poolSizes.data();
                    poolInfo.maxSets = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight());
                    if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de la creation de la pool de descripteurs!");
                    }
                } else if (shader == &bumpTextureGenerator) {
                    std::vector<VkDescriptorPool>& descriptorPool = bumpTexture.getDescriptorPool();
                    if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                    unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                    std::array<VkDescriptorPoolSize, 4> poolSizes;
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(bumpTexture.getMaxFramesInFlight());
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(bumpTexture.getMaxFramesInFlight());
                    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[2].descriptorCount = static_cast<uint32_t>(bumpTexture.getMaxFramesInFlight() * MAX_TEXTURES);
                    poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[3].descriptorCount = static_cast<uint32_t>(bumpTexture.getMaxFramesInFlight()) * depthBuffer.getSwapchainImagesSize();

                    VkDescriptorPoolCreateInfo poolInfo{};
                    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
                    poolInfo.pPoolSizes = poolSizes.data();
                    poolInfo.maxSets = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight());
                    if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de la creation de la pool de descripteurs!");
                    }

                } else {
                    std::vector<VkDescriptorPool>& descriptorPool = lightMap.getDescriptorPool();
                    if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                    unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                    std::array<VkDescriptorPoolSize, 6> poolSizes;
                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(lightMap.getMaxFramesInFlight()) * depthBuffer.getSwapchainImagesSize();
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(lightMap.getMaxFramesInFlight()) * alphaBuffer.getSwapchainImagesSize();
                    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[2].descriptorCount = static_cast<uint32_t>(lightMap.getMaxFramesInFlight()) * specularTexture.getSwapchainImagesSize();
                    poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[3].descriptorCount = static_cast<uint32_t>(lightMap.getMaxFramesInFlight()) * bumpTexture.getSwapchainImagesSize();
                    poolSizes[4].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    poolSizes[4].descriptorCount = static_cast<uint32_t>(lightMap.getMaxFramesInFlight());
                    poolSizes[5].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    poolSizes[5].descriptorCount = static_cast<uint32_t>(lightMap.getMaxFramesInFlight());
                    VkDescriptorPoolCreateInfo poolInfo{};
                    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
                    poolInfo.pPoolSizes = poolSizes.data();
                    poolInfo.maxSets = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight());
                    if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de la creation de la pool de descripteurs!");
                    }
                }
            }
            void LightRenderComponent::createDescriptorPool(RenderStates states) {
                Shader* shader = const_cast<Shader*>(states.shader);
                if (shader == &depthBufferGenerator) {
                    {
                        std::vector<VkDescriptorPool>& descriptorPool = depthBuffer.getDescriptorPool();
                        if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorPool.size())
                            descriptorPool.resize(shader->getNbShaders()*depthBuffer.getNbRenderTargets());
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
                    }
                    {
                        std::vector<VkDescriptorPool>& descriptorPool = lightDepthBuffer.getDescriptorPool();
                        if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorPool.size())
                            descriptorPool.resize(shader->getNbShaders()*depthBuffer.getNbRenderTargets());
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

                    }
                } else if (shader == &buildAlphaBufferGenerator) {
                    std::vector<VkDescriptorPool>& descriptorPool = alphaBuffer.getDescriptorPool();
                    if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders()*alphaBuffer.getNbRenderTargets());
                    unsigned int descriptorId = shader->getId();
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
                    std::vector<VkDescriptorPool>& descriptorPool = specularTexture.getDescriptorPool();
                    if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders()*specularTexture.getNbRenderTargets());
                    unsigned int descriptorId = shader->getId();
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
                    VkDescriptorPoolCreateInfo poolInfo{};
                    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
                    poolInfo.pPoolSizes = poolSizes.data();
                    poolInfo.maxSets = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight());
                    if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de la creation de la pool de descripteurs!");
                    }
                } else if (shader == &bumpTextureGenerator) {
                    std::vector<VkDescriptorPool>& descriptorPool = bumpTexture.getDescriptorPool();
                    if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders()*bumpTexture.getNbRenderTargets());
                    unsigned int descriptorId = shader->getId();
                    std::array<VkDescriptorPoolSize, 4> poolSizes;
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(bumpTexture.getMaxFramesInFlight() * allTextures.size());
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(bumpTexture.getMaxFramesInFlight());
                    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[2].descriptorCount = static_cast<uint32_t>(bumpTexture.getMaxFramesInFlight());
                    poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[3].descriptorCount = static_cast<uint32_t>(bumpTexture.getMaxFramesInFlight());
                    VkDescriptorPoolCreateInfo poolInfo{};
                    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
                    poolInfo.pPoolSizes = poolSizes.data();
                    poolInfo.maxSets = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight());
                    if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de la creation de la pool de descripteurs!");
                    }

                } else {
                    std::vector<VkDescriptorPool>& descriptorPool = lightMap.getDescriptorPool();
                    if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders()*lightMap.getNbRenderTargets());
                    unsigned int descriptorId = shader->getId();
                    std::array<VkDescriptorPoolSize, 6> poolSizes;
                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(lightMap.getMaxFramesInFlight());
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(lightMap.getMaxFramesInFlight());
                    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[2].descriptorCount = static_cast<uint32_t>(lightMap.getMaxFramesInFlight());
                    poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[3].descriptorCount = static_cast<uint32_t>(lightMap.getMaxFramesInFlight());
                    poolSizes[4].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[4].descriptorCount = static_cast<uint32_t>(lightMap.getMaxFramesInFlight());
                    poolSizes[5].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[5].descriptorCount = static_cast<uint32_t>(lightMap.getMaxFramesInFlight());
                    VkDescriptorPoolCreateInfo poolInfo{};
                    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
                    poolInfo.pPoolSizes = poolSizes.data();
                    poolInfo.maxSets = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight());
                    if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de la creation de la pool de descripteurs!");
                    }
                }
            }
            void LightRenderComponent::createDescriptorSetLayout(RenderStates states) {
                Shader* shader = const_cast<Shader*>(states.shader);
                if (shader == &depthBufferGenerator) {
                    {
                        std::vector<VkDescriptorSetLayout>& descriptorSetLayout = depthBuffer.getDescriptorSetLayout();
                        if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorSetLayout.size())
                            descriptorSetLayout.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
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


                        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
                        samplerLayoutBinding.binding = 3;
                        samplerLayoutBinding.descriptorCount = MAX_TEXTURES;
                        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        samplerLayoutBinding.pImmutableSamplers = nullptr;
                        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                        std::vector<VkDescriptorBindingFlags> bindingFlags(4, 0); // 6 bindings, flags par défaut à 0
                        bindingFlags[3] = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
                        VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
                        bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
                        bindingFlagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
                        bindingFlagsInfo.pBindingFlags = bindingFlags.data();

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

                    }
                    {
                        std::vector<VkDescriptorSetLayout>& descriptorSetLayout = lightDepthBuffer.getDescriptorSetLayout();
                        if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorSetLayout.size())
                            descriptorSetLayout.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
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


                        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
                        samplerLayoutBinding.binding = 3;
                        samplerLayoutBinding.descriptorCount = MAX_TEXTURES;
                        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        samplerLayoutBinding.pImmutableSamplers = nullptr;
                        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                        std::vector<VkDescriptorBindingFlags> bindingFlags(4, 0); // 6 bindings, flags par défaut à 0
                        bindingFlags[3] = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
                        VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
                        bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
                        bindingFlagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
                        bindingFlagsInfo.pBindingFlags = bindingFlags.data();

                        std::array<VkDescriptorSetLayoutBinding, 4> bindings = { modelDataLayoutBinding, materialDataLayoutBinding, headPtrImageLayoutBinding, samplerLayoutBinding};

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

                } else if (shader == &buildAlphaBufferGenerator) {
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = alphaBuffer.getDescriptorSetLayout();
                    if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorSetLayout.size())
                        descriptorSetLayout.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
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

                    VkDescriptorSetLayoutBinding samplerLayoutBinding2{};
                    samplerLayoutBinding2.binding = 3;
                    samplerLayoutBinding2.descriptorCount = lightDepthBuffer.getSwapchainImagesSize();
                    samplerLayoutBinding2.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding2.pImmutableSamplers = nullptr;
                    samplerLayoutBinding2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
                    samplerLayoutBinding.binding = 4;
                    samplerLayoutBinding.descriptorCount = MAX_TEXTURES;
                    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding.pImmutableSamplers = nullptr;
                    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    std::vector<VkDescriptorBindingFlags> bindingFlags(5, 0); // 6 bindings, flags par défaut à 0
                    bindingFlags[4] = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
                    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
                    bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
                    bindingFlagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
                    bindingFlagsInfo.pBindingFlags = bindingFlags.data();

                    std::array<VkDescriptorSetLayoutBinding, 5> bindings = {modelDataLayoutBinding, materialDataLayoutBinding, headPtrImageLayoutBinding, samplerLayoutBinding2, samplerLayoutBinding};

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
                } else if (shader == &specularTextureGenerator) {
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = specularTexture.getDescriptorSetLayout();
                    if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorSetLayout.size())
                        descriptorSetLayout.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
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

                    VkDescriptorSetLayoutBinding samplerLayoutBinding2{};
                    samplerLayoutBinding2.binding = 2;
                    samplerLayoutBinding2.descriptorCount = depthBuffer.getSwapchainImagesSize();
                    samplerLayoutBinding2.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding2.pImmutableSamplers = nullptr;
                    samplerLayoutBinding2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;



                    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
                    samplerLayoutBinding.binding = 3;
                    samplerLayoutBinding.descriptorCount = MAX_TEXTURES;
                    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding.pImmutableSamplers = nullptr;
                    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    std::vector<VkDescriptorBindingFlags> bindingFlags(4, 0); // 6 bindings, flags par défaut à 0
                    bindingFlags[3] = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
                    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
                    bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
                    bindingFlagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
                    bindingFlagsInfo.pBindingFlags = bindingFlags.data();

                    std::array<VkDescriptorSetLayoutBinding, 4> bindings = {modelDataLayoutBinding, materialDataLayoutBinding, samplerLayoutBinding2, samplerLayoutBinding};

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
                } else if (shader == &bumpTextureGenerator) {
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = bumpTexture.getDescriptorSetLayout();
                    if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorSetLayout.size())
                        descriptorSetLayout.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
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

                    VkDescriptorSetLayoutBinding samplerLayoutBinding2{};
                    samplerLayoutBinding2.binding = 2;
                    samplerLayoutBinding2.descriptorCount = depthBuffer.getSwapchainImagesSize();
                    samplerLayoutBinding2.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding2.pImmutableSamplers = nullptr;
                    samplerLayoutBinding2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
                    samplerLayoutBinding.binding = 3;
                    samplerLayoutBinding.descriptorCount = MAX_TEXTURES;
                    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding.pImmutableSamplers = nullptr;
                    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    std::vector<VkDescriptorBindingFlags> bindingFlags(4, 0); // 6 bindings, flags par défaut à 0
                    bindingFlags[3] = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
                    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
                    bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
                    bindingFlagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
                    bindingFlagsInfo.pBindingFlags = bindingFlags.data();

                    std::array<VkDescriptorSetLayoutBinding, 4> bindings = {modelDataLayoutBinding, materialDataLayoutBinding, samplerLayoutBinding2, samplerLayoutBinding};

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
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = lightMap.getDescriptorSetLayout();
                    if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorSetLayout.size())
                        descriptorSetLayout.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                    unsigned int descriptorId = shader->getId();
                    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
                    samplerLayoutBinding.binding = 0;
                    samplerLayoutBinding.descriptorCount = depthBuffer.getSwapchainImagesSize();
                    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding.pImmutableSamplers = nullptr;
                    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding samplerLayoutBinding2{};
                    samplerLayoutBinding2.binding = 1;
                    samplerLayoutBinding2.descriptorCount = alphaBuffer.getSwapchainImagesSize();
                    samplerLayoutBinding2.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding2.pImmutableSamplers = nullptr;
                    samplerLayoutBinding2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding samplerLayoutBinding3{};
                    samplerLayoutBinding3.binding = 2;
                    samplerLayoutBinding3.descriptorCount = specularTexture.getSwapchainImagesSize();
                    samplerLayoutBinding3.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding3.pImmutableSamplers = nullptr;
                    samplerLayoutBinding3.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding samplerLayoutBinding4{};
                    samplerLayoutBinding4.binding = 3;
                    samplerLayoutBinding4.descriptorCount = bumpTexture.getSwapchainImagesSize();
                    samplerLayoutBinding4.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding4.pImmutableSamplers = nullptr;
                    samplerLayoutBinding4.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding modelDataLayoutBinding{};
                    modelDataLayoutBinding.binding = 4;
                    modelDataLayoutBinding.descriptorCount = 1;
                    modelDataLayoutBinding.descriptorType = (useThread) ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    modelDataLayoutBinding.pImmutableSamplers = nullptr;
                    modelDataLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                    VkDescriptorSetLayoutBinding materialDataLayoutBinding{};
                    materialDataLayoutBinding.binding = 5;
                    materialDataLayoutBinding.descriptorCount = 1;
                    materialDataLayoutBinding.descriptorType = (useThread) ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    materialDataLayoutBinding.pImmutableSamplers = nullptr;
                    materialDataLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;


                    std::array<VkDescriptorSetLayoutBinding, 6> bindings = {samplerLayoutBinding, samplerLayoutBinding2, samplerLayoutBinding3, samplerLayoutBinding4, modelDataLayoutBinding, materialDataLayoutBinding};

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
            void LightRenderComponent::updateDescriptorSets(unsigned int currentFrame, unsigned int p, RenderStates states, bool lightDepth) {
                Shader* shader = const_cast<Shader*>(states.shader);
                if (shader == &depthBufferGenerator) {
                    std::vector<std::vector<VkDescriptorSet>> descriptorSets = (lightDepth) ? lightDepthBuffer.getDescriptorSet() : depthBuffer.getDescriptorSet();
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


                    if (!lightDepth) {
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
                    } else {
                        VkDescriptorImageInfo headPtrDescriptorImageInfo;
                        headPtrDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                        headPtrDescriptorImageInfo.imageView = lightDepthTextureImageView[currentFrame];
                        headPtrDescriptorImageInfo.sampler = lightDepthTextureSampler[currentFrame];

                        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[2].dstSet = descriptorSets[descriptorId][currentFrame];
                        descriptorWrites[2].dstBinding = 2;
                        descriptorWrites[2].dstArrayElement = 0;
                        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                        descriptorWrites[2].descriptorCount = 1;
                        descriptorWrites[2].pImageInfo = &headPtrDescriptorImageInfo;
                    }


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

                } else if (shader == &buildAlphaBufferGenerator) {
                        std::vector<std::vector<VkDescriptorSet>>& descriptorSets = alphaBuffer.getDescriptorSet();
                        std::vector<Texture*> allTextures = Texture::getAllTextures();
                        unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                        std::vector<VkDescriptorImageInfo>	descriptorImageInfos;
                        descriptorImageInfos.resize(allTextures.size());
                        std::array<VkWriteDescriptorSet, 5> descriptorWrites{};

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
                            descriptorImageInfos2[j].imageView = lightDepthBuffer.getTexture(j).getImageView();
                            descriptorImageInfos2[j].sampler = lightDepthBuffer.getTexture(j).getSampler();
                        }

                        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[3].dstSet = descriptorSets[descriptorId][currentFrame];
                        descriptorWrites[3].dstBinding = 3;
                        descriptorWrites[3].dstArrayElement = 0;
                        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[3].descriptorCount = descriptorImageInfos2.size();
                        descriptorWrites[3].pImageInfo = descriptorImageInfos2.data();

                        for (unsigned int j = 0; j < allTextures.size(); j++) {
                            descriptorImageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                            descriptorImageInfos[j].imageView = allTextures[j]->getImageView();
                            descriptorImageInfos[j].sampler = allTextures[j]->getSampler();
                        }

                        descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[4].dstSet = descriptorSets[descriptorId][currentFrame];
                        descriptorWrites[4].dstBinding = 4;
                        descriptorWrites[4].dstArrayElement = 0;
                        descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[4].descriptorCount = allTextures.size();
                        descriptorWrites[4].pImageInfo = descriptorImageInfos.data();



                        vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

                } else if (shader == &specularTextureGenerator) {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = specularTexture.getDescriptorSet();
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                    std::vector<VkDescriptorImageInfo>	descriptorImageInfos;
                    descriptorImageInfos.resize(allTextures.size());
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

                } else if (shader == &bumpTextureGenerator) {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = bumpTexture.getDescriptorSet();
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
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = lightMap.getDescriptorSet();
                    unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                    std::array<VkWriteDescriptorSet, 6> descriptorWrites{};
                    std::array<VkDescriptorImageInfo, RenderTexture::NB_SWAPCHAIN_IMAGES>	descriptorImageInfos;
                    for (unsigned int j = 0; j < depthBuffer.getSwapchainImagesSize(); j++) {
                        descriptorImageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        descriptorImageInfos[j].imageView = depthBuffer.getTexture(j).getImageView();
                        descriptorImageInfos[j].sampler = depthBuffer.getTexture(j).getSampler();
                    }

                    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[0].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[0].dstBinding = 0;
                    descriptorWrites[0].dstArrayElement = 0;
                    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrites[0].descriptorCount = descriptorImageInfos.size();
                    descriptorWrites[0].pImageInfo = descriptorImageInfos.data();

                    std::array<VkDescriptorImageInfo, RenderTexture::NB_SWAPCHAIN_IMAGES>	descriptorImageInfos2;
                    for (unsigned int j = 0; j < bumpTexture.getSwapchainImagesSize(); j++) {
                        descriptorImageInfos2[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        descriptorImageInfos2[j].imageView = bumpTexture.getTexture(j).getImageView();
                        descriptorImageInfos2[j].sampler = bumpTexture.getTexture(j).getSampler();
                    }

                    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[1].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[1].dstBinding = 1;
                    descriptorWrites[1].dstArrayElement = 0;
                    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrites[1].descriptorCount = descriptorImageInfos2.size();
                    descriptorWrites[1].pImageInfo = descriptorImageInfos2.data();

                    std::array<VkDescriptorImageInfo, RenderTexture::NB_SWAPCHAIN_IMAGES>	descriptorImageInfos3;
                    for (unsigned int j = 0; j < bumpTexture.getSwapchainImagesSize(); j++) {
                        descriptorImageInfos3[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        descriptorImageInfos3[j].imageView = specularTexture.getTexture(j).getImageView();
                        descriptorImageInfos3[j].sampler = specularTexture.getTexture(j).getSampler();
                    }

                    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[2].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[2].dstBinding = 2;
                    descriptorWrites[2].dstArrayElement = 0;
                    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrites[2].descriptorCount = descriptorImageInfos3.size();
                    descriptorWrites[2].pImageInfo = descriptorImageInfos3.data();

                    std::array<VkDescriptorImageInfo, RenderTexture::NB_SWAPCHAIN_IMAGES>	descriptorImageInfos4;
                    for (unsigned int j = 0; j < depthBuffer.getSwapchainImagesSize(); j++) {
                        descriptorImageInfos4[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        descriptorImageInfos4[j].imageView = alphaBuffer.getTexture(j).getImageView();
                        descriptorImageInfos4[j].sampler = alphaBuffer.getTexture(j).getSampler();
                    }

                    descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[3].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[3].dstBinding = 3;
                    descriptorWrites[3].dstArrayElement = 0;
                    descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrites[3].descriptorCount = descriptorImageInfos4.size();
                    descriptorWrites[3].pImageInfo = descriptorImageInfos4.data();

                    VkDescriptorBufferInfo modelDataStorageBufferInfoLastFrame{};
                    modelDataStorageBufferInfoLastFrame.buffer = modelDataBufferMT[p][currentFrame];
                    modelDataStorageBufferInfoLastFrame.offset = 0;
                    modelDataStorageBufferInfoLastFrame.range = maxAlignedSizeModelData[p];

                    descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[4].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[4].dstBinding = 4;
                    descriptorWrites[4].dstArrayElement = 0;
                    descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    descriptorWrites[4].descriptorCount = 1;
                    descriptorWrites[4].pBufferInfo = &modelDataStorageBufferInfoLastFrame;

                    VkDescriptorBufferInfo materialDataStorageBufferInfoLastFrame{};
                    materialDataStorageBufferInfoLastFrame.buffer = materialDataBufferMT[p][currentFrame];
                    materialDataStorageBufferInfoLastFrame.offset = 0;
                    materialDataStorageBufferInfoLastFrame.range = maxAlignedSizeMaterialData[p];

                    descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[5].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[5].dstBinding = 5;
                    descriptorWrites[5].dstArrayElement = 0;
                    descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    descriptorWrites[5].descriptorCount = 1;
                    descriptorWrites[5].pBufferInfo = &materialDataStorageBufferInfoLastFrame;

                    vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

                }
            }
            void LightRenderComponent::createDescriptorSets(RenderStates states, bool lightDepth) {

                Shader* shader = const_cast<Shader*>(states.shader);
                if (shader == &depthBufferGenerator) {
                   std::vector<std::vector<VkDescriptorSet>> descriptorSets = (lightDepth) ? lightDepthBuffer.getDescriptorSet() : depthBuffer.getDescriptorSet();
                   std::vector<Texture*> allTextures = Texture::getAllTextures();
                   unsigned int descriptorId = shader->getId();
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

                        if (!lightDepth) {
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
                        } else {
                            VkDescriptorImageInfo headPtrDescriptorImageInfo;
                            headPtrDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                            headPtrDescriptorImageInfo.imageView = lightDepthTextureImageView[i];
                            headPtrDescriptorImageInfo.sampler = lightDepthTextureSampler[i];

                            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                            descriptorWrites[1].dstSet = descriptorSets[descriptorId][i];
                            descriptorWrites[1].dstBinding = 1;
                            descriptorWrites[1].dstArrayElement = 0;
                            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                            descriptorWrites[1].descriptorCount = 1;
                            descriptorWrites[1].pImageInfo = &headPtrDescriptorImageInfo;
                        }
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
                            std::array<VkWriteDescriptorSet, 5> descriptorWrites{};
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
                            descriptorImageInfo.imageView = lightDepthBuffer.getTexture((lightDepthBuffer.getImageIndex()+i)%lightDepthBuffer.getSwapchainImagesSize()).getImageView();
                            descriptorImageInfo.sampler = lightDepthBuffer.getTexture((lightDepthBuffer.getImageIndex()+i)%lightDepthBuffer.getSwapchainImagesSize()).getSampler();

                            descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                            descriptorWrites[2].dstSet = descriptorSets[descriptorId][i];
                            descriptorWrites[2].dstBinding = 2;
                            descriptorWrites[2].dstArrayElement = 0;
                            descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                            descriptorWrites[2].descriptorCount = 1;
                            descriptorWrites[2].pImageInfo = &descriptorImageInfo;


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
                } else if (shader == &specularTextureGenerator) {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = specularTexture.getDescriptorSet();
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    unsigned int descriptorId = shader->getId();
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
                        headPtrDescriptorImageInfo.imageView = depthBuffer.getTexture((depthBuffer.getImageIndex()+i)%depthBuffer.getSwapchainImagesSize()).getImageView();
                        headPtrDescriptorImageInfo.sampler = depthBuffer.getTexture((depthBuffer.getImageIndex()+i)%depthBuffer.getSwapchainImagesSize()).getSampler();

                        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[1].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[1].dstBinding = 1;
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
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = bumpTexture.getDescriptorSet();
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    unsigned int descriptorId = shader->getId();
                    for (size_t i = 0; i < bumpTexture.getMaxFramesInFlight(); i++) {
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
                        headPtrDescriptorImageInfo.imageView = depthBuffer.getTexture((depthBuffer.getImageIndex()+i)%depthBuffer.getSwapchainImagesSize()).getImageView();
                        headPtrDescriptorImageInfo.sampler = depthBuffer.getTexture((depthBuffer.getImageIndex()+i)%depthBuffer.getSwapchainImagesSize()).getSampler();

                        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[1].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[1].dstBinding = 1;
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
                } else {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = lightMap.getDescriptorSet();
                    unsigned int descriptorId = shader->getId();
                    for (size_t i = 0; i < lightMap.getMaxFramesInFlight(); i++) {
                        std::array<VkWriteDescriptorSet, 6> descriptorWrites{};
                        VkDescriptorImageInfo headPtrDescriptorImageInfo;
                        headPtrDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        headPtrDescriptorImageInfo.imageView = depthBuffer.getTexture((depthBuffer.getImageIndex()+i)%depthBuffer.getSwapchainImagesSize()).getImageView();
                        headPtrDescriptorImageInfo.sampler = depthBuffer.getTexture((depthBuffer.getImageIndex()+i)%depthBuffer.getSwapchainImagesSize()).getSampler();

                        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[0].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[0].dstBinding = 0;
                        descriptorWrites[0].dstArrayElement = 0;
                        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[0].descriptorCount = 1;
                        descriptorWrites[0].pImageInfo = &headPtrDescriptorImageInfo;

                        VkDescriptorImageInfo headPtrDescriptorImageInfo2;
                        headPtrDescriptorImageInfo2.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        headPtrDescriptorImageInfo2.imageView = bumpTexture.getTexture((bumpTexture.getImageIndex()+i)%bumpTexture.getSwapchainImagesSize()).getImageView();
                        headPtrDescriptorImageInfo2.sampler =  bumpTexture.getTexture((bumpTexture.getImageIndex()+i)%bumpTexture.getSwapchainImagesSize()).getSampler();

                        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[1].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[1].dstBinding = 1;
                        descriptorWrites[1].dstArrayElement = 0;
                        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[1].descriptorCount = 1;
                        descriptorWrites[1].pImageInfo = &headPtrDescriptorImageInfo2;

                        VkDescriptorImageInfo headPtrDescriptorImageInfo3;
                        headPtrDescriptorImageInfo3.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        headPtrDescriptorImageInfo3.imageView = specularTexture.getTexture((specularTexture.getImageIndex()+i)%specularTexture.getSwapchainImagesSize()).getImageView();
                        headPtrDescriptorImageInfo3.sampler = specularTexture.getTexture((specularTexture.getImageIndex()+i)%specularTexture.getSwapchainImagesSize()).getSampler();

                        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[2].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[2].dstBinding = 2;
                        descriptorWrites[2].dstArrayElement = 0;
                        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[2].descriptorCount = 1;
                        descriptorWrites[2].pImageInfo = &headPtrDescriptorImageInfo3;

                        VkDescriptorImageInfo headPtrDescriptorImageInfo4;
                        headPtrDescriptorImageInfo4.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        headPtrDescriptorImageInfo4.imageView = alphaBuffer.getTexture((alphaBuffer.getImageIndex()+i)%alphaBuffer.getSwapchainImagesSize()).getImageView();
                        headPtrDescriptorImageInfo4.sampler =  alphaBuffer.getTexture((alphaBuffer.getImageIndex()+i)%alphaBuffer.getSwapchainImagesSize()).getSampler();

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

                        vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
                    }
                }
            }
            void LightRenderComponent::allocateDescriptorSets(unsigned int p, RenderStates states) {
                Shader* shader = const_cast<Shader*>(states.shader);
                if (shader == &depthBufferGenerator) {
                    {
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
                        variableCountInfo.descriptorSetCount = static_cast<uint32_t>(variableCounts.size());
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
                    }
                    {
                        std::vector<std::vector<VkDescriptorSet>>& descriptorSets =     lightDepthBuffer.getDescriptorSet();
                        std::vector<VkDescriptorPool>& descriptorPool = lightDepthBuffer.getDescriptorPool();
                        std::vector<VkDescriptorSetLayout>& descriptorSetLayout = lightDepthBuffer.getDescriptorSetLayout();
                        if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorSets.size())
                            descriptorSets.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                        unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                        for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                            descriptorSets[i].resize(lightDepthBuffer.getMaxFramesInFlight());
                        }
                        std::vector<Texture*> allTextures = Texture::getAllTextures();
                        std::vector<uint32_t> variableCounts(depthBuffer.getMaxFramesInFlight(), static_cast<uint32_t>(MAX_TEXTURES));

                        VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
                        variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
                        variableCountInfo.descriptorSetCount = static_cast<uint32_t>(variableCounts.size());
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
                    }
                } else if (shader == &buildAlphaBufferGenerator) {
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
                    std::vector<uint32_t> variableCounts(depthBuffer.getMaxFramesInFlight(), static_cast<uint32_t>(MAX_TEXTURES));

                    VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
                    variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
                    variableCountInfo.descriptorSetCount = static_cast<uint32_t>(variableCounts.size());
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

                } else if (shader == &specularTextureGenerator) {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = specularTexture.getDescriptorSet();
                    std::vector<VkDescriptorPool>& descriptorPool = specularTexture.getDescriptorPool();
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = specularTexture.getDescriptorSetLayout();
                    if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorSets.size())
                        descriptorSets.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                    unsigned int descriptorId =  p * shader->getNbShaders() + shader->getId();
                    for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                        descriptorSets[i].resize(specularTexture.getMaxFramesInFlight());
                    }
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    std::vector<uint32_t> variableCounts(depthBuffer.getMaxFramesInFlight(), static_cast<uint32_t>(MAX_TEXTURES));

                    VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
                    variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
                    variableCountInfo.descriptorSetCount = static_cast<uint32_t>(variableCounts.size());
                    variableCountInfo.pDescriptorCounts = variableCounts.data();
                    std::vector<VkDescriptorSetLayout> layouts(specularTexture.getMaxFramesInFlight(), descriptorSetLayout[shader->getId()]);
                    VkDescriptorSetAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    allocInfo.pNext = &variableCountInfo;
                    allocInfo.descriptorPool = descriptorPool[descriptorId];
                    allocInfo.descriptorSetCount = static_cast<uint32_t>(specularTexture.getMaxFramesInFlight());
                    allocInfo.pSetLayouts = layouts.data();
                    if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, descriptorSets[descriptorId].data()) != VK_SUCCESS) {
                        throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
                    }
                } else if (shader == &bumpTextureGenerator) {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = bumpTexture.getDescriptorSet();
                    std::vector<VkDescriptorPool>& descriptorPool = bumpTexture.getDescriptorPool();
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = bumpTexture.getDescriptorSetLayout();
                    if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorSets.size())
                        descriptorSets.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                    unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                    for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                        descriptorSets[i].resize(bumpTexture.getMaxFramesInFlight());
                    }
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    std::vector<uint32_t> variableCounts(depthBuffer.getMaxFramesInFlight(), static_cast<uint32_t>(MAX_TEXTURES));

                    VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
                    variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
                    variableCountInfo.descriptorSetCount = static_cast<uint32_t>(variableCounts.size());
                    variableCountInfo.pDescriptorCounts = variableCounts.data();
                    std::vector<VkDescriptorSetLayout> layouts(bumpTexture.getMaxFramesInFlight(), descriptorSetLayout[shader->getId()]);
                    VkDescriptorSetAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    allocInfo.pNext = &variableCountInfo;
                    allocInfo.descriptorPool = descriptorPool[descriptorId];
                    allocInfo.descriptorSetCount = static_cast<uint32_t>(bumpTexture.getMaxFramesInFlight());
                    allocInfo.pSetLayouts = layouts.data();
                    if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, descriptorSets[descriptorId].data()) != VK_SUCCESS) {
                        throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
                    }
                } else {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = lightMap.getDescriptorSet();
                    std::vector<VkDescriptorPool>& descriptorPool = lightMap.getDescriptorPool();
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = lightMap.getDescriptorSetLayout();
                    if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorSets.size())
                        descriptorSets.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                    unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                    for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                        descriptorSets[i].resize(lightMap.getMaxFramesInFlight());
                    }
                    std::vector<VkDescriptorSetLayout> layouts(lightMap.getMaxFramesInFlight(), descriptorSetLayout[shader->getId()]);
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
            void LightRenderComponent::allocateDescriptorSets(RenderStates states) {
                Shader* shader = const_cast<Shader*>(states.shader);
                if (shader == &depthBufferGenerator) {
                    {
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
                    }
                    {
                        std::vector<std::vector<VkDescriptorSet>>& descriptorSets =     lightDepthBuffer.getDescriptorSet();
                        std::vector<VkDescriptorPool>& descriptorPool = lightDepthBuffer.getDescriptorPool();
                        std::vector<VkDescriptorSetLayout>& descriptorSetLayout = lightDepthBuffer.getDescriptorSetLayout();
                        if (shader->getNbShaders() > descriptorSets.size())
                            descriptorSets.resize(shader->getNbShaders());
                        unsigned int descriptorId = shader->getId();
                        for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                            descriptorSets[i].resize(lightDepthBuffer.getMaxFramesInFlight());
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
                    }
                } else if (shader == &buildAlphaBufferGenerator) {
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

                } else if (shader == &specularTextureGenerator) {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = specularTexture.getDescriptorSet();
                    std::vector<VkDescriptorPool>& descriptorPool = specularTexture.getDescriptorPool();
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = specularTexture.getDescriptorSetLayout();
                    if (shader->getNbShaders() > descriptorSets.size())
                        descriptorSets.resize(shader->getNbShaders());
                    unsigned int descriptorId =  shader->getId();
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
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = bumpTexture.getDescriptorSet();
                    std::vector<VkDescriptorPool>& descriptorPool = bumpTexture.getDescriptorPool();
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = bumpTexture.getDescriptorSetLayout();
                    if (shader->getNbShaders() > descriptorSets.size())
                        descriptorSets.resize(shader->getNbShaders());
                    unsigned int descriptorId = shader->getId();
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
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = lightMap.getDescriptorSet();
                    std::vector<VkDescriptorPool>& descriptorPool = lightMap.getDescriptorPool();
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = lightMap.getDescriptorSetLayout();
                    if (shader->getNbShaders() > descriptorSets.size())
                        descriptorSets.resize(shader->getNbShaders());
                    unsigned int descriptorId = shader->getId();
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
                    ////////std::cout<<"instance : "<<i<<std::endl;
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    unsigned int p = m_light_instances[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = 0;
                    material.layer = m_light_instances[i].getMaterial().getLayer();
                    material.lightCenter = m_light_instances[i].getMaterial().getLightCenter();
                    material.uvScale = math::Vec2f(0, 0);
                    material.uvOffset = math::Vec2f(0, 0);
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
                    createCommandBuffersIndirect(p, drawArraysIndirectCommands[p].size(), sizeof(DrawArraysIndirectCommand), LIGHTNODEPTHNOSTENCIL, currentStates, true);
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
                    ////////std::cout<<"instance : "<<i<<std::endl;
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    unsigned int p = m_light_instances[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = 0;
                    material.layer = m_light_instances[i].getMaterial().getLayer();
                    material.lightCenter = m_light_instances[i].getMaterial().getLightCenter();
                    Color c = m_light_instances[i].getMaterial().getLightColor();
                    material.lightColor = math::Vec4f(1.f / 255.f * c.r, 1.f / 255.f * c.g, 1.f / 255.f * c.b, 1.f / 255.f * c.a);
                    material.uvScale = math::Vec2f(0, 0);
                    material.uvOffset = math::Vec2f(0, 0);
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
                    ////////std::cout<<"instance : "<<i<<std::endl;
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    unsigned int p = m_light_instances[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = 0;
                    material.layer = m_light_instances[i].getMaterial().getLayer();
                    material.lightCenter = m_light_instances[i].getMaterial().getLightCenter();
                    TransformMatrix modelMatrix;
                    /*////std::cout<<"light center : "<<material.lightCenter<<std::endl;
                    ////std::cout<<"light center screen : "<<lightMap.mapCoordsToPixel(modelMatrix.transform(material.lightCenter))<<std::endl;
                    system("PAUSE");*/

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
                    createCommandBuffersIndirect(p, drawArraysIndirectCommands[p].size(), sizeof(DrawArraysIndirectCommand), LIGHTNODEPTHNOSTENCIL, currentStates);
                }
            }
        }
        void LightRenderComponent::recordCommandBufferIndirect(unsigned int currentFrame, unsigned int p, unsigned int nbIndirectCommands, unsigned int stride, LightDepthStencilID depthStencilID, unsigned int vertexOffset, unsigned int indexOffset, unsigned int uboOffset, unsigned int modelDataOffset, unsigned int materialDataOffset, unsigned int drawCommandOffset, RenderStates currentStates, VkCommandBuffer commandBuffer, bool lightDepth) {
            currentStates.blendMode.updateIds();
            Shader* shader = const_cast<Shader*>(currentStates.shader);
            if (shader == &depthBufferGenerator) {

                ////////std::cout<<"draw on db"<<std::endl;
                if (!lightDepth) {
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

                } else {
                    layerPC.nbLayers = GameObject::getNbLayers();
                    vkCmdPushConstants(commandBuffer, lightDepthBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(IndirectRenderingPC), &indirectRenderingPC);
                    vkCmdPushConstants(commandBuffer, lightDepthBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_FRAGMENT_BIT, 128, sizeof(LayerPC), &layerPC);
                    std::vector<unsigned int> dynamicBufferOffsets;
                    dynamicBufferOffsets.push_back(modelDataOffset);
                    dynamicBufferOffsets.push_back(materialDataOffset);
                    if (indexOffset == -1)
                        lightDepthBuffer.drawIndirect(commandBuffer, currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], drawCommandBufferMT[p][currentFrame], depthStencilID,currentStates, p, vertexOffset, drawCommandOffset, dynamicBufferOffsets);
                    else
                        lightDepthBuffer.drawIndirect(commandBuffer, currentFrame, nbIndirectCommands, stride, vbBindlessTexIndexed[p], drawCommandBufferIndexedMT[p][currentFrame], depthStencilID,currentStates, p, vertexOffset, drawCommandOffset, dynamicBufferOffsets, indexOffset);
                }
            } else if (shader == &buildAlphaBufferGenerator) {
                layerPC.nbLayers = GameObject::getNbLayers();
                /*vkCmdResetEvent(commandBuffers[currentFrame], events[currentFrame],  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                vkCmdSetEvent(commandBuffers[currentFrame], events[currentFrame],  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);*/
                //vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);


                vkCmdPushConstants(commandBuffer, alphaBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(IndirectRenderingPC), &indirectRenderingPC);
                vkCmdPushConstants(commandBuffer, alphaBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_FRAGMENT_BIT, 128, sizeof(LayerPC), &layerPC);

                std::vector<unsigned int> dynamicBufferOffsets;
                dynamicBufferOffsets.push_back(modelDataOffset);
                dynamicBufferOffsets.push_back(materialDataOffset);
                if (indexOffset == -1)
                    alphaBuffer.drawIndirect(commandBuffer, currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], drawCommandBufferMT[p][currentFrame], depthStencilID,currentStates, p, vertexOffset, drawCommandOffset, dynamicBufferOffsets);
                else
                    alphaBuffer.drawIndirect(commandBuffer, currentFrame, nbIndirectCommands, stride, vbBindlessTexIndexed[p], drawCommandBufferIndexedMT[p][currentFrame], depthStencilID,currentStates, p, vertexOffset, drawCommandOffset, dynamicBufferOffsets, indexOffset);


            } else if (shader == &specularTextureGenerator) {


                vkCmdPushConstants(commandBuffer, specularTexture.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(IndirectRenderingPC), &indirectRenderingPC);
                vkCmdPushConstants(commandBuffer, specularTexture.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_FRAGMENT_BIT, 128, sizeof(MaxSpecPC), &maxSpecPC);

                std::vector<unsigned int> dynamicBufferOffsets;
                dynamicBufferOffsets.push_back(modelDataOffset);
                dynamicBufferOffsets.push_back(materialDataOffset);
                if (indexOffset == -1)
                    specularTexture.drawIndirect(commandBuffer, currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], drawCommandBufferMT[p][currentFrame], depthStencilID,currentStates, p, vertexOffset, drawCommandOffset, dynamicBufferOffsets);
                else
                    specularTexture.drawIndirect(commandBuffer, currentFrame, nbIndirectCommands, stride, vbBindlessTexIndexed[p], drawCommandBufferIndexedMT[p][currentFrame], depthStencilID,currentStates, p, vertexOffset, drawCommandOffset, dynamicBufferOffsets, indexOffset);

            } else if (shader == &bumpTextureGenerator) {
                //vkCmdWaitEvents(commandBuffers[currentFrame], 1, &events[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                vkCmdPushConstants(commandBuffer, bumpTexture.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(IndirectRenderingPC), &indirectRenderingPC);
                vkCmdPushConstants(commandBuffer, bumpTexture.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_FRAGMENT_BIT, 128, sizeof(LayerPC), &layerPC);

                std::vector<unsigned int> dynamicBufferOffsets;
                dynamicBufferOffsets.push_back(modelDataOffset);
                dynamicBufferOffsets.push_back(materialDataOffset);
                if (indexOffset == -1)
                    bumpTexture.drawIndirect(commandBuffer, currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], drawCommandBufferMT[p][currentFrame], depthStencilID,currentStates, p, vertexOffset, drawCommandOffset, dynamicBufferOffsets);
                else
                    bumpTexture.drawIndirect(commandBuffer, currentFrame, nbIndirectCommands, stride, vbBindlessTexIndexed[p], drawCommandBufferIndexedMT[p][currentFrame], depthStencilID,currentStates, p, vertexOffset, drawCommandOffset, dynamicBufferOffsets, indexOffset);


            } else {
                vkCmdPushConstants(commandBuffer, lightMap.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(LightIndirectRenderingPC), &lightIndirectRenderingPC);
                vkCmdPushConstants(commandBuffer, lightMap.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_FRAGMENT_BIT, 192, sizeof(ResolutionPC), &resolutionPC);
                std::vector<unsigned int> dynamicBufferOffsets;
                dynamicBufferOffsets.push_back(modelDataOffset);
                dynamicBufferOffsets.push_back(materialDataOffset);
                /*if (p == TriangleStrip)
                    std::cout<<"record lights"<<std::endl;*/
                if (indexOffset == -1)
                    lightMap.drawIndirect(commandBuffer, currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], drawCommandBufferMT[p][currentFrame], depthStencilID,currentStates, p, vertexOffset, drawCommandOffset, dynamicBufferOffsets);
                else
                    lightMap.drawIndirect(commandBuffer, currentFrame, nbIndirectCommands, stride, vbBindlessTexIndexed[p], drawCommandBufferIndexedMT[p][currentFrame], depthStencilID,currentStates, p, vertexOffset, drawCommandOffset, dynamicBufferOffsets, indexOffset);

            }
            isSomethingDrawn = true;
        }
        void LightRenderComponent::createCommandBuffersIndirect(unsigned int p, unsigned int nbIndirectCommands, unsigned int stride, LightDepthStencilID depthStencilID, RenderStates currentStates, bool lightDepth) {

            if (needToUpdateDS) {
                Shader* shader = const_cast<Shader*>(currentStates.shader);
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

                ////////std::cout<<"draw on db"<<std::endl;
                if (!lightDepth) {
                    createDescriptorSets(currentStates, false);
                    depthBuffer.beginRecordCommandBuffers();
                    std::vector<VkCommandBuffer> commandBuffers = depthBuffer.getCommandBuffers();
                    unsigned int currentFrame = depthBuffer.getCurrentFrame();
                    layerPC.nbLayers = GameObject::getNbLayers();
                    vkCmdPushConstants(commandBuffers[currentFrame], depthBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(IndirectRenderingPC), &indirectRenderingPC);
                    vkCmdPushConstants(commandBuffers[currentFrame], depthBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_FRAGMENT_BIT, 128, sizeof(LayerPC), &layerPC);
                    depthBuffer.beginRenderPass();
                    depthBuffer.drawIndirect(commandBuffers[currentFrame], currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], vboIndirect, depthStencilID,currentStates);
                    depthBuffer.endRenderPass();
                    /*signalSemaphores.clear();
                    waitSemaphores.clear();
                    signalSemaphores.push_back(offscreenFinishedSemaphore[depthBuffer.getCurrentFrame()]);
                    waitSemaphores.push_back(offscreenFinishedSemaphore[depthBuffer.getCurrentFrame()]);
                    waitValues.clear();
                    signalValues.clear();
                    waitValues.push_back(valuesFinished[depthBuffer.getCurrentFrame()]);
                    valuesFinished[depthBuffer.getCurrentFrame()]++;
                    signalValues.push_back(valuesFinished[depthBuffer.getCurrentFrame()]);*/
                    depthBuffer.submit(/*true, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues*/);
                } else {
                    createDescriptorSets(currentStates, true);
                    lightDepthBuffer.beginRecordCommandBuffers();
                    std::vector<VkCommandBuffer> commandBuffers = lightDepthBuffer.getCommandBuffers();
                    unsigned int currentFrame = lightDepthBuffer.getCurrentFrame();
                    layerPC.nbLayers = GameObject::getNbLayers();
                    vkCmdPushConstants(commandBuffers[currentFrame], lightDepthBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(IndirectRenderingPC), &indirectRenderingPC);
                    vkCmdPushConstants(commandBuffers[currentFrame], lightDepthBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_FRAGMENT_BIT, 128, sizeof(LayerPC), &layerPC);
                    lightDepthBuffer.beginRenderPass();
                    lightDepthBuffer.drawIndirect(commandBuffers[currentFrame], currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], vboIndirect, depthStencilID,currentStates);
                    lightDepthBuffer.endRenderPass();
                    /*std::vector<VkSemaphore> signalSemaphores, waitSemaphores;
                    std::vector<VkPipelineStageFlags> waitStages;
                    std::vector<uint64_t> signalValues, waitValues;
                    waitSemaphores.push_back(offscreenLightDepthAlphaFinishedSemaphore[lightDepthBuffer.getCurrentFrame()]);
                    waitStages.push_back(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                    waitValues.push_back(valuesLightDepthAlpha[lightDepthBuffer.getCurrentFrame()]);
                    signalSemaphores.push_back(offscreenLightDepthAlphaFinishedSemaphore[lightDepthBuffer.getCurrentFrame()]);
                    valuesLightDepthAlpha[lightDepthBuffer.getCurrentFrame()]++;
                    signalValues.push_back(valuesLightDepthAlpha[lightDepthBuffer.getCurrentFrame()]);*/
                    lightDepthBuffer.submit(/*true, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues*/);
                }

            } else if (shader == &buildAlphaBufferGenerator) {
                alphaBuffer.beginRecordCommandBuffers();
                const_cast<Texture&>(lightDepthBuffer.getTexture(lightDepthBuffer.getImageIndex())).toShaderReadOnlyOptimal(alphaBuffer.getCommandBuffers()[alphaBuffer.getCurrentFrame()]);


                std::vector<VkCommandBuffer> commandBuffers = alphaBuffer.getCommandBuffers();
                unsigned int currentFrame = alphaBuffer.getCurrentFrame();
                layerPC.nbLayers = GameObject::getNbLayers();
                /*vkCmdResetEvent(commandBuffers[currentFrame], events[currentFrame],  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                vkCmdSetEvent(commandBuffers[currentFrame], events[currentFrame],  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);*/
                //vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);


                vkCmdPushConstants(commandBuffers[currentFrame], alphaBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(IndirectRenderingPC), &indirectRenderingPC);
                vkCmdPushConstants(commandBuffers[currentFrame], alphaBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_FRAGMENT_BIT, 128, sizeof(LayerPC), &layerPC);


                VkMemoryBarrier memoryBarrier={};
                memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                //vkCmdWaitEvents(commandBuffers[currentFrame], 1, &events[currentFrame], VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                alphaBuffer.beginRenderPass();
                alphaBuffer.drawIndirect(commandBuffers[currentFrame], currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], vboIndirect, depthStencilID,currentStates);
                alphaBuffer.endRenderPass();
                const_cast<Texture&>(lightDepthBuffer.getTexture(lightDepthBuffer.getImageIndex())).toColorAttachmentOptimal(alphaBuffer.getCommandBuffers()[alphaBuffer.getCurrentFrame()]);
                /*waitValues.clear();
                signalValues.clear();
                waitValues.push_back(valuesLightDepthAlpha[alphaBuffer.getCurrentFrame()]);
                valuesLightDepthAlpha[alphaBuffer.getCurrentFrame()]++;
                signalValues.push_back(valuesLightDepthAlpha[alphaBuffer.getCurrentFrame()]);*/
                alphaBuffer.submit(/*true, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues*/);


            } else if (shader == &specularTextureGenerator) {


                const_cast<Texture&>(depthBuffer.getTexture(depthBuffer.getImageIndex())).toShaderReadOnlyOptimal(specularTexture.getCommandBuffers()[specularTexture.getCurrentFrame()]);
                std::vector<VkCommandBuffer> commandBuffers = specularTexture.getCommandBuffers();
                unsigned int currentFrame = specularTexture.getCurrentFrame();


                //vkCmdWaitEvents(commandBuffers[currentFrame], 1, &events[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                vkCmdPushConstants(commandBuffers[currentFrame], specularTexture.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(IndirectRenderingPC), &indirectRenderingPC);
                vkCmdPushConstants(commandBuffers[currentFrame], specularTexture.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_FRAGMENT_BIT, 128, sizeof(MaxSpecPC), &maxSpecPC);
                /*////std::cout<<"pipeline layout : "<<stencilBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][stencilBuffer.getId()][depthStencilID]<<std::endl;
                system("PAUSE");*/
                vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);
                VkMemoryBarrier memoryBarrier;
                memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                memoryBarrier.pNext = VK_NULL_HANDLE;
                memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                //vkCmdWaitEvents(commandBuffers[currentFrame], 1, &events[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

                vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                specularTexture.beginRenderPass();
                specularTexture.drawIndirect(commandBuffers[currentFrame], currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], vboIndirect, depthStencilID,currentStates);
                specularTexture.endRenderPass();
                const_cast<Texture&>(depthBuffer.getTexture(depthBuffer.getImageIndex())).toColorAttachmentOptimal(specularTexture.getCommandBuffers()[specularTexture.getCurrentFrame()]);
                /*valuesFinished[bumpTexture.getCurrentFrame()]++;
                signalValues.clear();
                signalValues.push_back(valuesFinished[bumpTexture.getCurrentFrame()]);*/
                specularTexture.submit(/*true, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues*/);


            } else if (shader == &bumpTextureGenerator) {

                bumpTexture.beginRecordCommandBuffers();
                const_cast<Texture&>(depthBuffer.getTexture(depthBuffer.getImageIndex())).toShaderReadOnlyOptimal(bumpTexture.getCommandBuffers()[bumpTexture.getCurrentFrame()]);
                std::vector<VkCommandBuffer> commandBuffers = bumpTexture.getCommandBuffers();
                unsigned int currentFrame = bumpTexture.getCurrentFrame();


                //vkCmdWaitEvents(commandBuffers[currentFrame], 1, &events[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                vkCmdPushConstants(commandBuffers[currentFrame], bumpTexture.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(IndirectRenderingPC), &indirectRenderingPC);
                vkCmdPushConstants(commandBuffers[currentFrame], bumpTexture.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_FRAGMENT_BIT, 128, sizeof(LayerPC), &layerPC);
                /*////std::cout<<"pipeline layout : "<<stencilBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][stencilBuffer.getId()][depthStencilID]<<std::endl;
                system("PAUSE");*/
                 vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);
                VkMemoryBarrier memoryBarrier;
                memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                memoryBarrier.pNext = VK_NULL_HANDLE;
                memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                //vkCmdWaitEvents(commandBuffers[currentFrame], 1, &events[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

                vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                bumpTexture.beginRenderPass();
                bumpTexture.drawIndirect(commandBuffers[currentFrame], currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], vboIndirect, depthStencilID,currentStates);
                bumpTexture.endRenderPass();

                const_cast<Texture&>(depthBuffer.getTexture(depthBuffer.getImageIndex())).toColorAttachmentOptimal(bumpTexture.getCommandBuffers()[bumpTexture.getCurrentFrame()]);
                /*waitValues.clear();
                signalValues.clear();
                waitValues.push_back(valuesFinished[bumpTexture.getCurrentFrame()]);
                valuesFinished[bumpTexture.getCurrentFrame()]++;
                signalValues.push_back(valuesFinished[bumpTexture.getCurrentFrame()]);*/
                bumpTexture.submit(/*true, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues*/);


            } else {
                lightMap.beginRecordCommandBuffers();
                const_cast<Texture&>(specularTexture.getTexture(specularTexture.getImageIndex())).toShaderReadOnlyOptimal(lightMap.getCommandBuffers()[lightMap.getCurrentFrame()]);
                const_cast<Texture&>(bumpTexture.getTexture(bumpTexture.getImageIndex())).toShaderReadOnlyOptimal(lightMap.getCommandBuffers()[lightMap.getCurrentFrame()]);
                const_cast<Texture&>(alphaBuffer.getTexture(alphaBuffer.getImageIndex())).toShaderReadOnlyOptimal(lightMap.getCommandBuffers()[lightMap.getCurrentFrame()]);
                const_cast<Texture&>(depthBuffer.getTexture(depthBuffer.getImageIndex())).toShaderReadOnlyOptimal(lightMap.getCommandBuffers()[lightMap.getCurrentFrame()]);


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
                vkCmdPushConstants(commandBuffers[currentFrame], lightMap.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(LightIndirectRenderingPC), &lightIndirectRenderingPC);
                vkCmdPushConstants(commandBuffers[currentFrame], lightMap.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_FRAGMENT_BIT, 192, sizeof(ResolutionPC), &resolutionPC);
                lightMap.beginRenderPass();
                lightMap.drawIndirect(commandBuffers[currentFrame], currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], vboIndirect, depthStencilID,currentStates);
                lightMap.endRenderPass();


                const_cast<Texture&>(alphaBuffer.getTexture(alphaBuffer.getImageIndex())).toColorAttachmentOptimal(lightMap.getCommandBuffers()[lightMap.getCurrentFrame()]);
                const_cast<Texture&>(bumpTexture.getTexture(bumpTexture.getImageIndex())).toColorAttachmentOptimal(lightMap.getCommandBuffers()[lightMap.getCurrentFrame()]);
                const_cast<Texture&>(depthBuffer.getTexture(depthBuffer.getImageIndex())).toColorAttachmentOptimal(lightMap.getCommandBuffers()[lightMap.getCurrentFrame()]);
                const_cast<Texture&>(specularTexture.getTexture(specularTexture.getImageIndex())).toColorAttachmentOptimal(lightMap.getCommandBuffers()[lightMap.getCurrentFrame()]);
                /*waitSemaphores.push_back(offscreenLightDepthAlphaFinishedSemaphore[lightMap.getCurrentFrame()]);
                waitStages.push_back(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                signalSemaphores.push_back(offscreenLightDepthAlphaFinishedSemaphore[lightMap.getCurrentFrame()]);
                waitValues.clear();
                signalValues.clear();
                waitValues.push_back(valuesFinished[lightMap.getCurrentFrame()]);
                waitValues.push_back(valuesLightDepthAlpha[lightMap.getCurrentFrame()]);
                valuesFinished[lightMap.getCurrentFrame()]++;
                valuesLightDepthAlpha[lightMap.getCurrentFrame()]++;
                signalValues.push_back(valuesFinished[lightMap.getCurrentFrame()]);
                signalValues.push_back(valuesLightDepthAlpha[lightMap.getCurrentFrame()]);*/
                lightMap.submit(/*true, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues*/);

            }
            isSomethingDrawn = true;
        }
        void LightRenderComponent::fillBuffersMT() {
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
                        material.bumpTextureIndex = (m_normals[i].getMaterial().getBumpTexture() != nullptr) ? m_normals[i].getMaterial().getBumpTexture()->getId() : 0;
                        material.layer = m_normals[i].getMaterial().getLayer();
                        material.uvScale = (m_normals[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_normals[i].getMaterial().getTexture()->getSize().x(), 1.f / m_normals[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                    }
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
                    material.textureIndex = (m_instances[i].getMaterial().getTexture() != nullptr) ? m_instances[i].getMaterial().getTexture()->getId() : 0;
                    material.bumpTextureIndex = (m_instances[i].getMaterial().getBumpTexture() != nullptr) ? m_instances[i].getMaterial().getBumpTexture()->getId() : 0;
                    material.layer = m_instances[i].getMaterial().getLayer();
                    material.uvScale = (m_instances[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_instances[i].getMaterial().getTexture()->getSize().x(), 1.f / m_instances[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                    material.uvOffset = math::Vec2f(0, 0);
                    materialDatas[p].push_back(material);
                    std::vector<TransformMatrix*> tm = m_instances[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData model;
                        model.worldMat = tm[j]->getMatrix().transpose();

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
                        for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
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
                        }
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
        void LightRenderComponent::fillIndexedBuffersMT() {
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
                        material.bumpTextureIndex = (m_normalsIndexed[i].getMaterial().getBumpTexture() != nullptr) ? m_normalsIndexed[i].getMaterial().getBumpTexture()->getId() : 0;
                        material.layer = m_normalsIndexed[i].getMaterial().getLayer();
                        material.uvScale = (m_normalsIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_normalsIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_normalsIndexed[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                    }
                    materialDatas[p].push_back(material);
                    ModelData model;
                    TransformMatrix tm;
                    model.worldMat = tm.getMatrix().transpose();

                    modelDatas[p].push_back(model);
                    unsigned int vertexCount = 0, indexCount = 0;
                    for (unsigned int j = 0; j < m_normalsIndexed[i].getAllVertices().getVertexCount(); j++) {
                        vertexCount++;
                        vbBindlessTexIndexed[p].append(m_normalsIndexed[i].getAllVertices()[j]);
                    }
                    for (unsigned int j = 0; j < m_normalsIndexed[i].getAllVertices().getIndexes().size(); j++) {
                        vbBindlessTexIndexed[p].addIndex(m_normalsIndexed[i].getAllVertices().getIndexes()[j]);
                        indexCount++;
                    }
                    drawElementsIndirectCommand.indexCount = indexCount;
                    drawElementsIndirectCommand.firstIndex = firstIndex[p] + oldTotalIndexCount[p];
                    drawElementsIndirectCommand.baseInstance = baseInstance[p];
                    drawElementsIndirectCommand.baseVertex = baseVertex[p] + oldTotalVertexIndexCount[p];
                    drawElementsIndirectCommand.instanceCount = 1;
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
                        material.bumpTextureIndex = (m_instancesIndexed[i].getMaterial().getBumpTexture() != nullptr) ? m_instancesIndexed[i].getMaterial().getBumpTexture()->getId() : 0;
                        material.layer = m_instancesIndexed[i].getMaterial().getLayer();
                        material.uvScale = (m_instancesIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_instancesIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_instancesIndexed[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                    }
                    materialDatas[p].push_back(material);
                    std::vector<TransformMatrix*> tm = m_instancesIndexed[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData model;
                        model.worldMat = tm[j]->getMatrix().transpose();

                        modelDatas[p].push_back(model);
                    }
                    unsigned int vertexCount = 0, indexCount = 0;
                    if (m_instancesIndexed[i].getEntities().size() > 0) {
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
                    drawElementsIndirectCommand.indexCount = indexCount;
                    drawElementsIndirectCommand.firstIndex = firstIndex[p] + oldTotalIndexCount[p];
                    drawElementsIndirectCommand.baseInstance = baseInstance[p];
                    drawElementsIndirectCommand.baseVertex = baseVertex[p] + oldTotalVertexIndexCount[p];
                    drawElementsIndirectCommand.instanceCount = tm.size();
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
                        for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                            if (vboIndexedIndirectStagingBufferMT[p][currentFrame] != nullptr) {
                                    vkDestroyBuffer(vkDevice.getDevice(), vboIndexedIndirectStagingBufferMT[p][currentFrame], nullptr);
                                    vkFreeMemory(vkDevice.getDevice(), vboIndexedIndirectStagingBufferMemoryMT[p][currentFrame], nullptr);
                                }
                                createBuffer(totalBufferSizeIndexedDrawCommand[p], VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vboIndexedIndirectStagingBufferMT[p][currentFrame], vboIndexedIndirectStagingBufferMemoryMT[p][currentFrame]);

                            if (drawCommandBufferIndexedMT[p][i] != nullptr) {
                                vkDestroyBuffer(vkDevice.getDevice(),drawCommandBufferIndexedMT[p][currentFrame], nullptr);
                                vkFreeMemory(vkDevice.getDevice(), drawCommandBufferIndexedMemoryMT[p][currentFrame], nullptr);
                            }
                            createBuffer(totalBufferSizeIndexedDrawCommand[p], VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, drawCommandBufferIndexedMT[p][currentFrame], drawCommandBufferIndexedMemoryMT[p][currentFrame]);
                        }
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
        void LightRenderComponent::fillLightBuffersMT() {
            //std::cout<<"fill light buffers"<<std::endl;
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
            for (unsigned int i = 0; i < m_light_instances.size(); i++) {
                if (m_light_instances[i].getAllVertices().getVertexCount() > 0) {
                    //std::cout<<"draw light instances : "<<i<<std::endl;

                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    unsigned int p = m_light_instances[i].getAllVertices().getPrimitiveType();



                    MaterialData material;
                    {
                        std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                        material.textureIndex = 0;
                        material.layer = m_light_instances[i].getMaterial().getLayer();
                        material.lightCenter = m_light_instances[i].getMaterial().getLightCenter();
                        material.uvScale = math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        Color c = m_light_instances[i].getMaterial().getLightColor();
                        material.lightColor = math::Vec4f(1.f / 255.f * c.r, 1.f / 255.f * c.g, 1.f / 255.f * c.b, 1.f / 255.f * c.a);
                        materialDatas[p].push_back(material);

                    }
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
                        for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
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
                        }
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
        void LightRenderComponent::fillLightBuffersIndexedMT() {
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
            for (unsigned int i = 0; i < m_light_instances_indexed.size(); i++) {

               if ( m_light_instances_indexed[i].getAllVertices().getVertexCount() > 0) {


                    DrawElementsIndirectCommand drawElementsIndirectCommand;
                    unsigned int p =  m_light_instances_indexed[i].getAllVertices().getPrimitiveType();

                    MaterialData material;
                    {
                        std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                        material.textureIndex = 0;
                        material.layer =  m_light_instances_indexed[i].getMaterial().getLayer();
                        material.lightCenter = m_light_instances_indexed[i].getMaterial().getLightCenter();
                        //std::cout<<"light center : "<<m_light_instances_indexed[i].getMaterial().getLightCenter()<<std::endl;
                        material.uvScale = math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        Color c =  m_light_instances_indexed[i].getMaterial().getLightColor();
                        material.lightColor = math::Vec4f(1.f / 255.f * c.r, 1.f / 255.f * c.g, 1.f / 255.f * c.b, 1.f / 255.f * c.a);

                    }

                    materialDatas[p].push_back(material);

                    TransformMatrix tm;
                    ModelData modelData;
                    modelData.worldMat = tm.getMatrix();

                    modelDatas[p].push_back(modelData);
                    unsigned int indexCount = 0, vertexCount = 0;
                    for (unsigned int j = 0; j < m_light_instances_indexed[i].getAllVertices().getVertexCount(); j++) {
                        //std::cout<<"add vertex"<<std::endl;
                        vbBindlessTexIndexed[p].append(m_light_instances_indexed[i].getAllVertices()[j]);
                        vertexCount++;
                    }
                    for (unsigned int j = 0; j < m_light_instances_indexed[i].getAllVertices().getIndexes().size(); j++) {
                        // std::cout<<"add index"<<std::endl;
                        vbBindlessTexIndexed[p].addIndex(m_light_instances_indexed[i].getAllVertices().getIndexes()[j]);
                        indexCount++;
                    }

                    drawElementsIndirectCommand.indexCount = indexCount;
                    drawElementsIndirectCommand.firstIndex = firstIndex[p] + oldTotalIndexCount[p];
                    drawElementsIndirectCommand.baseInstance = baseInstance[p];
                    drawElementsIndirectCommand.baseVertex = baseVertex[p] + oldTotalVertexIndexCount[p];
                    drawElementsIndirectCommand.instanceCount = 1;
                    drawElementsIndirectCommands[p].push_back(drawElementsIndirectCommand);
                    firstIndex[p] += indexCount;
                    baseVertex[p] += vertexCount;
                    baseInstance[p] += 1;
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
            unsigned int currentFrame = lightDepthBuffer.getCurrentFrame();
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
                if (nbIndexedDrawCommandBuffer[p][1] > 0) {
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
                        for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                            if (vboIndexedIndirectStagingBufferMT[p][currentFrame] != nullptr) {
                                    vkDestroyBuffer(vkDevice.getDevice(), vboIndexedIndirectStagingBufferMT[p][currentFrame], nullptr);
                                    vkFreeMemory(vkDevice.getDevice(), vboIndexedIndirectStagingBufferMemoryMT[p][currentFrame], nullptr);
                                }
                                createBuffer(totalBufferSizeIndexedDrawCommand[p], VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vboIndexedIndirectStagingBufferMT[p][currentFrame], vboIndexedIndirectStagingBufferMemoryMT[p][currentFrame]);

                            if (drawCommandBufferIndexedMT[p][i] != nullptr) {
                                vkDestroyBuffer(vkDevice.getDevice(),drawCommandBufferIndexedMT[p][currentFrame], nullptr);
                                vkFreeMemory(vkDevice.getDevice(), drawCommandBufferIndexedMemoryMT[p][currentFrame], nullptr);
                            }
                            createBuffer(totalBufferSizeIndexedDrawCommand[p], VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, drawCommandBufferIndexedMT[p][currentFrame], drawCommandBufferIndexedMemoryMT[p][currentFrame]);
                        }
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
        void LightRenderComponent::drawBuffers() {
           unsigned int currentFrame = depthBuffer.getCurrentFrame();
           unsigned int texturesInUse = Texture::getAllTextures().size();

           for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {

                if (modelDatas[p].size() > 0 && texturesInUse > maxTexturesInUse[currentFrame]) {
                    needToUpdateDSs[p][currentFrame] = true;
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
            currentStates.shader = &depthBufferGenerator;
            for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes-1; p++) {

                if (needToUpdateDSs[p][currentFrame])
                    updateDescriptorSets(currentFrame, p, currentStates);

                if (nbDrawCommandBuffer[p][0] > 0) {
                    recordCommandBufferIndirect(currentFrame, p, nbDrawCommandBuffer[p][0], sizeof(DrawArraysIndirectCommand), LIGHTNBDEPTHSTENCIL, 0, -1, -1, modelDataOffsets[p][0], materialDataOffsets[p][0],drawCommandBufferOffsets[p][0], currentStates, depthCommandBuffer[currentFrame]);
                }
                if (nbIndexedDrawCommandBuffer[p][0] > 0) {
                    recordCommandBufferIndirect(currentFrame, p, nbIndexedDrawCommandBuffer[p][0], sizeof(DrawElementsIndirectCommand), LIGHTNODEPTHNOSTENCIL, 0, 0, -1, modelDataOffsets[p][1], materialDataOffsets[p][1],drawIndexedCommandBufferOffsets[p][0], currentStates, depthCommandBuffer[currentFrame]);
                }

            }
            if (vkEndCommandBuffer(depthCommandBuffer[currentFrame]) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to record command buffer!", 1);
            }



            inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
            inheritanceInfo.renderPass = lightDepthBuffer.getRenderPass(1);
            inheritanceInfo.framebuffer = VK_NULL_HANDLE;

            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.pInheritanceInfo = &inheritanceInfo;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
            vkResetCommandBuffer(lightDepthCommandBuffer[currentFrame], 0);
            if (vkBeginCommandBuffer(lightDepthCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                throw core::Erreur(0, "failed to begin recording command buffer!", 1);
            }

            currentStates.blendMode = BlendNone;
            for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes-1; p++) {

                if (needToUpdateDSs[p][currentFrame])
                    updateDescriptorSets(currentFrame, p, currentStates, true);

                if (nbDrawCommandBuffer[p][1] > 0) {
                    recordCommandBufferIndirect(currentFrame, p, nbDrawCommandBuffer[p][1], sizeof(DrawArraysIndirectCommand), LIGHTNODEPTHNOSTENCIL, 0, -1, -1, modelDataOffsets[p][2], materialDataOffsets[p][2],drawCommandBufferOffsets[p][1], currentStates, lightDepthCommandBuffer[currentFrame]);
                }

            }
            if (vkEndCommandBuffer(lightDepthCommandBuffer[currentFrame]) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to record command buffer!", 1);
            }


            inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
            inheritanceInfo.renderPass = alphaBuffer.getRenderPass(1);
            inheritanceInfo.framebuffer = VK_NULL_HANDLE;

            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.pInheritanceInfo = &inheritanceInfo;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
            vkResetCommandBuffer(alphaCommandBuffer[currentFrame], 0);
            if (vkBeginCommandBuffer(alphaCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                throw core::Erreur(0, "failed to begin recording command buffer!", 1);
            }


            currentStates.blendMode = BlendNone;
            currentStates.shader = &buildAlphaBufferGenerator;
            for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes-1; p++) {

                if (needToUpdateDSs[p][currentFrame])
                    updateDescriptorSets(currentFrame, p, currentStates);

                if (nbDrawCommandBuffer[p][0] > 0) {
                    recordCommandBufferIndirect(currentFrame, p, nbDrawCommandBuffer[p][0], sizeof(DrawArraysIndirectCommand), LIGHTNODEPTHNOSTENCIL, 0, -1, -1, modelDataOffsets[p][0], materialDataOffsets[p][0],drawCommandBufferOffsets[p][0], currentStates, alphaCommandBuffer[currentFrame]);
                }
                if (nbIndexedDrawCommandBuffer[p][0] > 0) {
                    recordCommandBufferIndirect(currentFrame, p, nbIndexedDrawCommandBuffer[p][0], sizeof(DrawElementsIndirectCommand), LIGHTNODEPTHNOSTENCIL, 0, 0, -1, modelDataOffsets[p][1], materialDataOffsets[p][1],drawIndexedCommandBufferOffsets[p][0], currentStates, alphaCommandBuffer[currentFrame]);
                }


            }
            if (vkEndCommandBuffer(alphaCommandBuffer[currentFrame]) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to record command buffer!", 1);
            }


            inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
            inheritanceInfo.renderPass = specularTexture.getRenderPass(1);
            inheritanceInfo.framebuffer = VK_NULL_HANDLE;

            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.pInheritanceInfo = &inheritanceInfo;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
            vkResetCommandBuffer(specularCommandBuffer[currentFrame], 0);
            if (vkBeginCommandBuffer(specularCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                throw core::Erreur(0, "failed to begin recording command buffer!", 1);
            }


            currentStates.blendMode = BlendNone;
            currentStates.shader = &specularTextureGenerator;
            for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes-1; p++) {

                if (needToUpdateDSs[p][currentFrame])
                    updateDescriptorSets(currentFrame, p, currentStates);

                if (nbDrawCommandBuffer[p][0] > 0) {
                    recordCommandBufferIndirect(currentFrame, p, nbDrawCommandBuffer[p][0], sizeof(DrawArraysIndirectCommand), LIGHTNODEPTHNOSTENCIL, 0, -1, -1, modelDataOffsets[p][0], materialDataOffsets[p][0],drawCommandBufferOffsets[p][0], currentStates, specularCommandBuffer[currentFrame]);
                }
                if (nbIndexedDrawCommandBuffer[p][0] > 0) {
                    recordCommandBufferIndirect(currentFrame, p, nbIndexedDrawCommandBuffer[p][0], sizeof(DrawElementsIndirectCommand), LIGHTNODEPTHNOSTENCIL, 0, 0, -1, modelDataOffsets[p][1], materialDataOffsets[p][1],drawIndexedCommandBufferOffsets[p][0], currentStates, specularCommandBuffer[currentFrame]);
                }


            }
            if (vkEndCommandBuffer(specularCommandBuffer[currentFrame]) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to record command buffer!", 1);
            }

            inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
            inheritanceInfo.renderPass = bumpTexture.getRenderPass(1);
            inheritanceInfo.framebuffer = VK_NULL_HANDLE;

            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.pInheritanceInfo = &inheritanceInfo;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
            vkResetCommandBuffer(bumpCommandBuffer[currentFrame], 0);
            if (vkBeginCommandBuffer(bumpCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                throw core::Erreur(0, "failed to begin recording command buffer!", 1);
            }


            currentStates.blendMode = BlendNone;
            currentStates.shader = &bumpTextureGenerator;
            for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes-1; p++) {

                if (needToUpdateDSs[p][currentFrame])
                    updateDescriptorSets(currentFrame, p, currentStates);

                if (nbDrawCommandBuffer[p][0] > 0) {
                    recordCommandBufferIndirect(currentFrame, p, nbDrawCommandBuffer[p][0], sizeof(DrawArraysIndirectCommand), LIGHTNODEPTHNOSTENCIL, 0, -1, -1, modelDataOffsets[p][0], materialDataOffsets[p][0],drawCommandBufferOffsets[p][0], currentStates, bumpCommandBuffer[currentFrame]);
                }
                if (nbIndexedDrawCommandBuffer[p][0] > 0) {
                    recordCommandBufferIndirect(currentFrame, p, nbIndexedDrawCommandBuffer[p][0], sizeof(DrawElementsIndirectCommand), LIGHTNODEPTHNOSTENCIL, 0, 0, -1, modelDataOffsets[p][1], materialDataOffsets[p][1],drawIndexedCommandBufferOffsets[p][0], currentStates, bumpCommandBuffer[currentFrame]);
                }

            }
            if (vkEndCommandBuffer(bumpCommandBuffer[currentFrame]) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to record command buffer!", 1);
            }

            inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
            inheritanceInfo.renderPass = lightMap.getRenderPass(1);
            inheritanceInfo.framebuffer = VK_NULL_HANDLE;

            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.pInheritanceInfo = &inheritanceInfo;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
            vkResetCommandBuffer(lightCommandBuffer[currentFrame], 0);
            if (vkBeginCommandBuffer(lightCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                throw core::Erreur(0, "failed to begin recording command buffer!", 1);
            }


            currentStates.blendMode = BlendAdd;
            currentStates.shader = &lightMapGenerator;
            for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes-1; p++) {

                if (needToUpdateDSs[p][currentFrame])
                    updateDescriptorSets(currentFrame, p, currentStates);

                if (nbDrawCommandBuffer[p][1] > 0) {

                    recordCommandBufferIndirect(currentFrame, p, nbDrawCommandBuffer[p][1], sizeof(DrawArraysIndirectCommand), LIGHTNODEPTHNOSTENCIL, 0, -1, -1, modelDataOffsets[p][2], materialDataOffsets[p][2],drawCommandBufferOffsets[p][1], currentStates, lightCommandBuffer[currentFrame]);
                }
                if (nbIndexedDrawCommandBuffer[p][1] > 0) {
                    //std::cout<<"record indexed : "<<nbIndexedDrawCommandBuffer[p][1]<<std::endl;
                    recordCommandBufferIndirect(currentFrame, p, nbIndexedDrawCommandBuffer[p][1], sizeof(DrawElementsIndirectCommand), LIGHTNODEPTHNOSTENCIL, 0, 0, -1, modelDataOffsets[p][3], materialDataOffsets[p][3],drawIndexedCommandBufferOffsets[p][1], currentStates, lightCommandBuffer[currentFrame]);
                }

            }
            if (vkEndCommandBuffer(lightCommandBuffer[currentFrame]) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to record command buffer!", 1);
            }

            for (unsigned int p = 0; p < (Batcher::nbPrimitiveTypes -1); p++)
                needToUpdateDSs[p][currentFrame] = false;
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
                   m_instancesIndexed = indexedBatcher.getInstances();
                   m_normalsIndexed = normalIndexedBatcher.getInstances();
                   m_light_instances = lightBatcher.getInstances();
                   m_light_instances_indexed = lightIndexedBatcher.getInstances();
               }

           }
            //glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
            /*if (!view.isOrtho())
                view.setPerspective(80, view.getViewport().getSize().x() / view.getViewport().getSize().y(), 0.1, view.getViewport().getSize().z());*/
            math::Matrix4f viewMatrix = view.getViewMatrix().getMatrix().transpose();
            math::Matrix4f projMatrix = view.getProjMatrix().getMatrix().transpose();
            indirectRenderingPC.projMatrix = projMatrix;
            indirectRenderingPC.viewMatrix = viewMatrix;
            layerPC.nbLayers = GameObject::getNbLayers();
            maxSpecPC.maxM = 1;
            maxSpecPC.maxP = 1;
            if (useThread) {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [this](){return registerFrameJob[lightDepthBuffer.getCurrentFrame()].load() || stop.load();});
                registerFrameJob[lightDepthBuffer.getCurrentFrame()] = false;
                //std::cout<<"draw buffers"<<std::endl;
                resetBuffers();
                fillBuffersMT();
                fillIndexedBuffersMT();
                fillLightBuffersMT();
                fillLightBuffersIndexedMT();
                //std::cout<<"buffer filled"<<std::endl;
                lightIndirectRenderingPC.projMatrix = projMatrix;
                lightIndirectRenderingPC.viewMatrix = viewMatrix;
                lightIndirectRenderingPC.viewportMatrix = lightMap.getViewportMatrix(&lightMap.getView()).getMatrix().transpose();

                resolutionPC.near = view.getViewport().getPosition().z();
                resolutionPC.far = view.getViewport().getSize().z();

                drawBuffers();
                //std::cout<<"current frame : "<<lightDepthBuffer.getCurrentFrame()<<std::endl;
                commandBufferReady[lightDepthBuffer.getCurrentFrame()] = true;
                cv.notify_one();
                //std::cout<<"buffer drawn"<<std::endl;
            } else {
                drawDepthLightInstances();


                /*if (!isSomethingDrawn)
                    lightDepthBuffer.display();*/
                drawInstances();
                drawIndexedInstances();


                //drawNormals();

                lightIndirectRenderingPC.projMatrix = projMatrix;
                lightIndirectRenderingPC.viewMatrix = viewMatrix;
                lightIndirectRenderingPC.viewportMatrix = lightMap.getViewportMatrix(&lightMap.getView()).getMatrix().transpose();

                resolutionPC.near = view.getViewport().getPosition().z();
                resolutionPC.far = view.getViewport().getSize().z();

                drawLightInstances();

                if (!isSomethingDrawn) {
                    lightDepthBuffer.beginRecordCommandBuffers();
                    lightDepthBuffer.display();
                    lightMap.beginRecordCommandBuffers();
                    lightMap.display();
                }
                isSomethingDrawn = false;
            }

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
                    {
                        std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                        material.textureIndex = (m_normals[i].getMaterial().getTexture() != nullptr) ? m_normals[i].getMaterial().getTexture()->getId() : 0;
                        material.layer = m_normals[i].getMaterial().getLayer();
                        material.uvScale = (m_normals[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_normals[i].getMaterial().getTexture()->getSize().x(), 1.f / m_normals[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                    }
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
                    material.uvScale = (m_instances[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_instances[i].getMaterial().getTexture()->getSize().x(), 1.f / m_instances[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                    material.uvOffset = math::Vec2f(0, 0);
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
                    createCommandBuffersIndirect(p, drawArraysIndirectCommands[p].size(), sizeof(DrawArraysIndirectCommand), LIGHTNODEPTHNOSTENCIL, currentStates);
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
                    createCommandBuffersIndirect(p, drawArraysIndirectCommands[p].size(), sizeof(DrawArraysIndirectCommand), LIGHTNODEPTHNOSTENCIL, currentStates);
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
                    material.bumpTextureIndex = (m_normals[i].getMaterial().getBumpTexture() != nullptr) ? m_normals[i].getMaterial().getBumpTexture()->getId() : 0;
                    material.layer = m_normals[i].getMaterial().getLayer();
                    material.uvScale = (m_normals[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_normals[i].getMaterial().getTexture()->getSize().x(), 1.f / m_normals[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                    material.uvOffset = math::Vec2f(0, 0);
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
                    material.bumpTextureIndex = (m_instances[i].getMaterial().getBumpTexture() != nullptr) ? m_instances[i].getMaterial().getBumpTexture()->getId() : 0;
                    material.layer = m_instances[i].getMaterial().getLayer();
                    material.uvScale = (m_instances[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_instances[i].getMaterial().getTexture()->getSize().x(), 1.f / m_instances[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                    material.uvOffset = math::Vec2f(0, 0);
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
                    createCommandBuffersIndirect(p, drawArraysIndirectCommands[p].size(), sizeof(DrawArraysIndirectCommand), LIGHTNODEPTHNOSTENCIL, currentStates);
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
                    material.uvScale = (m_normals[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_normals[i].getMaterial().getTexture()->getSize().x(), 1.f / m_normals[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                    material.uvOffset = math::Vec2f(0, 0);
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
                    material.uvScale = (m_instances[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_instances[i].getMaterial().getTexture()->getSize().x(), 1.f / m_instances[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                    material.uvOffset = math::Vec2f(0, 0);
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
                    createCommandBuffersIndirect(p, drawArraysIndirectCommands[p].size(), sizeof(DrawArraysIndirectCommand), LIGHTNODEPTHNOSTENCIL, currentStates);
                }
            }
        }
        void LightRenderComponent::drawIndexedInstances() {
            for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                vbBindlessTex[i].clear();
                modelDatas[i].clear();
                materialDatas[i].clear();
            }
            std::array<std::vector<DrawElementsIndirectCommand>, Batcher::nbPrimitiveTypes> drawElementsIndirectCommands;
            std::array<unsigned int, Batcher::nbPrimitiveTypes> firstIndex, baseInstance, baseVertex;
            for (unsigned int i = 0; i < firstIndex.size(); i++) {
                firstIndex[i] = 0;
            }
            for (unsigned int i = 0; i < baseInstance.size(); i++) {
                baseInstance[i] = 0;
            }
            for (unsigned int i = 0; i < baseVertex.size(); i++) {
                baseVertex[i] = 0;
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
                    ModelData model;
                    TransformMatrix tm;
                    model.worldMat = tm.getMatrix().transpose();
                    modelDatas[p].push_back(model);
                    unsigned int vertexCount = 0, indexCount = 0;
                    for (unsigned int j = 0; j < m_normalsIndexed[i].getAllVertices().getVertexCount(); j++) {
                        vertexCount++;
                        vbBindlessTex[p].append(m_normalsIndexed[i].getAllVertices()[j]);
                    }
                    for (unsigned int j = 0; j < m_normalsIndexed[i].getAllVertices().getIndexes().size(); j++) {
                        vbBindlessTex[p].addIndex(m_normalsIndexed[i].getAllVertices().getIndexes()[j]);
                        indexCount++;
                    }
                    drawElementsIndirectCommand.indexCount = indexCount;
                    drawElementsIndirectCommand.firstIndex = firstIndex[p];
                    drawElementsIndirectCommand.baseInstance = baseInstance[p];
                    drawElementsIndirectCommand.baseVertex = baseVertex[p];
                    drawElementsIndirectCommand.instanceCount = 1;
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
                    material.layer = m_instancesIndexed[i].getMaterial().getLayer();
                    material.uvScale = (m_instancesIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_instancesIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_instancesIndexed[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                    material.uvOffset = math::Vec2f(0, 0);
                    materialDatas[p].push_back(material);
                    std::vector<TransformMatrix*> tm = m_instancesIndexed[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData model;
                        model.worldMat = tm[j]->getMatrix().transpose();
                        modelDatas[p].push_back(model);
                    }
                    unsigned int vertexCount = 0, indexCount = 0;
                    if (m_instancesIndexed[i].getVertexArrays().size() > 0) {
                        std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                        Entity* entity = m_instancesIndexed[i].getVertexArrays()[0]->getEntity();
                        for (unsigned int j = 0; j < m_instancesIndexed[i].getVertexArrays().size(); j++) {
                            if (entity == m_instancesIndexed[i].getVertexArrays()[j]->getEntity()) {
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
                    drawElementsIndirectCommand.indexCount = indexCount;
                    drawElementsIndirectCommand.firstIndex = firstIndex[p];
                    drawElementsIndirectCommand.baseInstance = baseInstance[p];
                    drawElementsIndirectCommand.baseVertex = baseVertex[p];
                    drawElementsIndirectCommand.instanceCount = tm.size();
                    drawElementsIndirectCommands[p].push_back(drawElementsIndirectCommand);
                    firstIndex[p] += indexCount;
                    baseVertex[p] += vertexCount;
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
                    createCommandBuffersIndirect(p, drawElementsIndirectCommands[p].size(), sizeof(DrawElementsIndirectCommand), LIGHTNODEPTHNOSTENCIL, currentStates);
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
                    createCommandBuffersIndirect(p, drawElementsIndirectCommands[p].size(), sizeof(DrawElementsIndirectCommand), LIGHTNODEPTHNOSTENCIL, currentStates);
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
            for (unsigned int i = 0; i < baseVertex.size(); i++) {
                baseVertex[i] = 0;
            }
            for (unsigned int i = 0; i < drawElementsIndirectCommands.size(); i++) {
                drawElementsIndirectCommands[i].clear();
            }
            for (unsigned int i = 0; i < m_normalsIndexed.size(); i++) {
               if (m_normalsIndexed[i].getAllVertices().getVertexCount() > 0) {
                    DrawElementsIndirectCommand drawElementsIndirectCommand;
                    unsigned int p = m_normalsIndexed[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    {
                        std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                        material.bumpTextureIndex = (m_normalsIndexed[i].getMaterial().getBumpTexture() != nullptr) ? m_normalsIndexed[i].getMaterial().getBumpTexture()->getId() : 0;
                        material.layer = m_normalsIndexed[i].getMaterial().getLayer();
                        material.uvScale = (m_normalsIndexed[i].getMaterial().getBumpTexture() != nullptr) ? math::Vec2f(1.f / m_normalsIndexed[i].getMaterial().getBumpTexture()->getSize().x(), 1.f / m_normalsIndexed[i].getMaterial().getBumpTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                    }
                    materialDatas[p].push_back(material);
                    ModelData model;
                    TransformMatrix tm;
                    model.worldMat = tm.getMatrix().transpose();
                    modelDatas[p].push_back(model);
                    unsigned int vertexCount = 0, indexCount = 0;
                    for (unsigned int j = 0; j < m_normalsIndexed[i].getAllVertices().getVertexCount(); j++) {
                        vertexCount++;
                        vbBindlessTex[p].append(m_normalsIndexed[i].getAllVertices()[j]);
                    }
                    for (unsigned int j = 0; j < m_normalsIndexed[i].getAllVertices().getIndexes().size(); j++) {
                        vbBindlessTex[p].addIndex(m_normalsIndexed[i].getAllVertices().getIndexes()[j]);
                        indexCount++;
                    }
                    drawElementsIndirectCommand.indexCount = indexCount;
                    drawElementsIndirectCommand.firstIndex = firstIndex[p];
                    drawElementsIndirectCommand.baseInstance = baseInstance[p];
                    drawElementsIndirectCommand.baseVertex = baseVertex[p];
                    drawElementsIndirectCommand.instanceCount = 1;
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
                    material.bumpTextureIndex = (m_instancesIndexed[i].getMaterial().getBumpTexture() != nullptr) ? m_instancesIndexed[i].getMaterial().getBumpTexture()->getId() : 0;
                    material.layer = m_instancesIndexed[i].getMaterial().getLayer();
                    material.uvScale = (m_instancesIndexed[i].getMaterial().getBumpTexture() != nullptr) ? math::Vec2f(1.f / m_instancesIndexed[i].getMaterial().getBumpTexture()->getSize().x(), 1.f / m_instancesIndexed[i].getMaterial().getBumpTexture()->getSize().y()) : math::Vec2f(0, 0);
                    material.uvOffset = math::Vec2f(0, 0);
                    materialDatas[p].push_back(material);
                    std::vector<TransformMatrix*> tm = m_instancesIndexed[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData model;
                        model.worldMat = tm[j]->getMatrix().transpose();
                        modelDatas[p].push_back(model);
                    }
                    unsigned int vertexCount = 0, indexCount = 0;
                    if (m_instancesIndexed[i].getVertexArrays().size() > 0) {
                        Entity* entity = m_instancesIndexed[i].getVertexArrays()[0]->getEntity();
                        for (unsigned int j = 0; j < m_instancesIndexed[i].getVertexArrays().size(); j++) {
                            if (entity == m_instancesIndexed[i].getVertexArrays()[j]->getEntity()) {
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
                    drawElementsIndirectCommand.indexCount = indexCount;
                    drawElementsIndirectCommand.firstIndex = firstIndex[p];
                    drawElementsIndirectCommand.baseInstance = baseInstance[p];
                    drawElementsIndirectCommand.baseVertex = baseVertex[p];
                    drawElementsIndirectCommand.instanceCount = tm.size();
                    drawElementsIndirectCommands[p].push_back(drawElementsIndirectCommand);
                    firstIndex[p] += indexCount;
                    baseVertex[p] += vertexCount;
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
                    createCommandBuffersIndirect(p, drawElementsIndirectCommands[p].size(), sizeof(DrawElementsIndirectCommand), LIGHTNODEPTHNOSTENCIL, currentStates);
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
            for (unsigned int i = 0; i < baseVertex.size(); i++) {
                baseVertex[i] = 0;
            }
            for (unsigned int i = 0; i < drawElementsIndirectCommands.size(); i++) {
                drawElementsIndirectCommands[i].clear();
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
                    ModelData model;
                    TransformMatrix tm;
                    model.worldMat = tm.getMatrix().transpose();
                    modelDatas[p].push_back(model);
                    unsigned int vertexCount = 0, indexCount = 0;
                    for (unsigned int j = 0; j < m_normalsIndexed[i].getAllVertices().getVertexCount(); j++) {
                        vertexCount++;
                        vbBindlessTex[p].append(m_normalsIndexed[i].getAllVertices()[j]);
                    }
                    for (unsigned int j = 0; j < m_normalsIndexed[i].getAllVertices().getIndexes().size(); j++) {
                        vbBindlessTex[p].addIndex(m_normalsIndexed[i].getAllVertices().getIndexes()[j]);
                        indexCount++;
                    }
                    drawElementsIndirectCommand.indexCount = indexCount;
                    drawElementsIndirectCommand.firstIndex = firstIndex[p];
                    drawElementsIndirectCommand.baseInstance = baseInstance[p];
                    drawElementsIndirectCommand.baseVertex = baseVertex[p];
                    drawElementsIndirectCommand.instanceCount = 1;
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
                    material.layer = m_instancesIndexed[i].getMaterial().getLayer();
                    material.uvScale = (m_instancesIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_instancesIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_instancesIndexed[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                    material.uvOffset = math::Vec2f(0, 0);
                    materialDatas[p].push_back(material);
                    std::vector<TransformMatrix*> tm = m_instancesIndexed[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData model;
                        model.worldMat = tm[j]->getMatrix().transpose();
                        modelDatas[p].push_back(model);
                    }
                    unsigned int vertexCount = 0, indexCount = 0;
                    if (m_instancesIndexed[i].getVertexArrays().size() > 0) {
                        Entity* entity = m_instancesIndexed[i].getVertexArrays()[0]->getEntity();
                        for (unsigned int j = 0; j < m_instancesIndexed[i].getVertexArrays().size(); j++) {
                            if (entity == m_instancesIndexed[i].getVertexArrays()[j]->getEntity()) {
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
                    drawElementsIndirectCommand.indexCount = indexCount;
                    drawElementsIndirectCommand.firstIndex = firstIndex[p];
                    drawElementsIndirectCommand.baseInstance = baseInstance[p];
                    drawElementsIndirectCommand.baseVertex = baseVertex[p];
                    drawElementsIndirectCommand.instanceCount = tm.size();
                    drawElementsIndirectCommands[p].push_back(drawElementsIndirectCommand);
                    firstIndex[p] += indexCount;
                    baseVertex[p] += vertexCount;
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
                    createCommandBuffersIndirect(p, drawElementsIndirectCommands[p].size(), sizeof(DrawElementsIndirectCommand), LIGHTNODEPTHNOSTENCIL, currentStates);
                }
            }
        }
        void LightRenderComponent::draw(RenderTarget& target, RenderStates states) {

            if (useThread) {
                //std::cout<<"draw current frame : "<<lightDepthBuffer.getCurrentFrame()<<std::endl;
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [this] { return commandBufferReady[lightDepthBuffer.getCurrentFrame()].load() || stop.load(); });
                //std::cout<<"copy"<<std::endl;
                commandBufferReady[lightDepthBuffer.getCurrentFrame()] = false;
                lightDepthBuffer.beginRecordCommandBuffers();
                std::vector<VkCommandBuffer> commandBuffers = lightDepthBuffer.getCommandBuffers();
                unsigned int currentFrame = lightDepthBuffer.getCurrentFrame();
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
                waitSemaphores.push_back(offscreenLightDepthAlphaFinishedSemaphore[lightMap.getCurrentFrame()]);
                std::vector<VkPipelineStageFlags> waitStages;
                waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                std::vector<uint64_t> signalValues;
                std::vector<uint64_t> waitValues;
                waitValues.push_back(valuesLightDepthAlpha[lightMap.getCurrentFrame()]);
                copyValues[currentFrame]++;
                signalValues.push_back(copyValues[currentFrame]);
                lightDepthBuffer.submit(false, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);
                //std::cout<<"draw"<<std::endl;

                lightDepthBuffer.beginRecordCommandBuffers();
                lightDepthBuffer.beginRenderPass();
                vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &lightDepthCommandBuffer[currentFrame]);
                lightDepthBuffer.endRenderPass();
                signalSemaphores.clear();
                waitSemaphores.clear();
                signalSemaphores.push_back(offscreenLightDepthAlphaFinishedSemaphore[depthBuffer.getCurrentFrame()]);
                waitSemaphores.push_back(copyFinishedSemaphore[depthBuffer.getCurrentFrame()]);
                waitStages.clear();
                waitStages.push_back(VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
                waitValues.clear();
                signalValues.clear();
                waitValues.push_back(copyValues[lightDepthBuffer.getCurrentFrame()]);
                valuesLightDepthAlpha[lightDepthBuffer.getCurrentFrame()]++;
                signalValues.push_back(valuesLightDepthAlpha[lightDepthBuffer.getCurrentFrame()]);
                lightDepthBuffer.submit(true, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);


                depthBuffer.beginRecordCommandBuffers();
                commandBuffers = depthBuffer.getCommandBuffers();
                currentFrame = depthBuffer.getCurrentFrame();
                depthBuffer.beginRenderPass();
                vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &depthCommandBuffer[currentFrame]);
                depthBuffer.endRenderPass();
                waitSemaphores.clear();
                signalSemaphores.clear();
                waitStages.clear();
                signalValues.clear();
                waitValues.clear();
                waitSemaphores.push_back(offscreenFinishedSemaphore[depthBuffer.getCurrentFrame()]);
                waitSemaphores.push_back(copyFinishedSemaphore[lightDepthBuffer.getCurrentFrame()]);
                waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                waitStages.push_back(VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
                waitValues.push_back(valuesFinished[lightDepthBuffer.getCurrentFrame()]);
                waitValues.push_back(copyValues[lightDepthBuffer.getCurrentFrame()]);
                signalSemaphores.push_back(offscreenFinishedSemaphore[lightDepthBuffer.getCurrentFrame()]);
                valuesFinished[lightDepthBuffer.getCurrentFrame()]++;
                signalValues.push_back(valuesFinished[depthBuffer.getCurrentFrame()]);
                depthBuffer.submit(true, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);


                alphaBuffer.beginRecordCommandBuffers();

                const_cast<Texture&>(lightDepthBuffer.getTexture(lightDepthBuffer.getImageIndex())).toShaderReadOnlyOptimal(alphaBuffer.getCommandBuffers()[alphaBuffer.getCurrentFrame()]);


                commandBuffers = alphaBuffer.getCommandBuffers();
                currentFrame = alphaBuffer.getCurrentFrame();
                BlendMode blendNone = BlendNone;
                blendNone.updateIds();
                for (unsigned int p = 0; p < (Batcher::nbPrimitiveTypes -1); p++) {
                    if (nbDrawCommandBuffer[p][0] > 0 || nbIndexedDrawCommandBuffer[p][0] > 0) {
                        vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS,alphaBuffer.getGraphicPipeline()[buildAlphaBufferGenerator.getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][LIGHTNODEPTHNOSTENCIL*blendNone.nbBlendModes+blendNone.id]);
                        vkCmdPushConstants(commandBuffers[currentFrame], alphaBuffer.getPipelineLayout()[buildAlphaBufferGenerator.getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][LIGHTNODEPTHNOSTENCIL*blendNone.nbBlendModes+blendNone.id], VK_SHADER_STAGE_FRAGMENT_BIT, 148, sizeof(unsigned int), &alphaBuffer.getImageIndex());
                    }


                }


                VkMemoryBarrier memoryBarrier{};
                memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                //vkCmdWaitEvents(commandBuffers[currentFrame], 1, &events[currentFrame], VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                alphaBuffer.beginRenderPass();
                vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &alphaCommandBuffer[currentFrame]);
                alphaBuffer.endRenderPass();
                const_cast<Texture&>(lightDepthBuffer.getTexture(lightDepthBuffer.getImageIndex())).toColorAttachmentOptimal(alphaBuffer.getCommandBuffers()[alphaBuffer.getCurrentFrame()]);
                waitValues.clear();
                signalValues.clear();
                waitSemaphores.clear();
                signalSemaphores.clear();
                waitStages.clear();
                waitSemaphores.push_back(offscreenLightDepthAlphaFinishedSemaphore[lightDepthBuffer.getCurrentFrame()]);
                waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                signalSemaphores.push_back(offscreenLightDepthAlphaFinishedSemaphore[lightDepthBuffer.getCurrentFrame()]);
                waitValues.push_back(valuesLightDepthAlpha[alphaBuffer.getCurrentFrame()]);
                valuesLightDepthAlpha[alphaBuffer.getCurrentFrame()]++;
                signalValues.push_back(valuesLightDepthAlpha[alphaBuffer.getCurrentFrame()]);
                alphaBuffer.submit(true, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);


                specularTexture.beginRecordCommandBuffers();

                const_cast<Texture&>(depthBuffer.getTexture(depthBuffer.getImageIndex())).toShaderReadOnlyOptimal(specularTexture.getCommandBuffers()[specularTexture.getCurrentFrame()]);
                commandBuffers = specularTexture.getCommandBuffers();
                currentFrame = specularTexture.getCurrentFrame();
                for (unsigned int p = 0; p < (Batcher::nbPrimitiveTypes -1); p++) {

                    if (nbDrawCommandBuffer[p][0] > 0 || nbIndexedDrawCommandBuffer[p][0] > 0) {
                        vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS,specularTexture.getGraphicPipeline()[specularTextureGenerator.getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][LIGHTNODEPTHNOSTENCIL*blendNone.nbBlendModes+blendNone.id]);
                        vkCmdPushConstants(commandBuffers[currentFrame], specularTexture.getPipelineLayout()[specularTextureGenerator.getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][LIGHTNODEPTHNOSTENCIL*blendNone.nbBlendModes+blendNone.id], VK_SHADER_STAGE_FRAGMENT_BIT, 152, sizeof(unsigned int), &specularTexture.getImageIndex());
                    }


                }

                memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                specularTexture.beginRenderPass();
                vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &specularCommandBuffer[currentFrame]);
                specularTexture.endRenderPass();
                const_cast<Texture&>(depthBuffer.getTexture(depthBuffer.getImageIndex())).toColorAttachmentOptimal(specularTexture.getCommandBuffers()[specularTexture.getCurrentFrame()]);
                waitSemaphores.clear();
                waitSemaphores.push_back(offscreenFinishedSemaphore[specularTexture.getCurrentFrame()]);
                waitValues.clear();
                waitValues.push_back(valuesFinished[bumpTexture.getCurrentFrame()]);
                valuesFinished[bumpTexture.getCurrentFrame()]++;
                signalSemaphores.clear();
                signalSemaphores.push_back(offscreenFinishedSemaphore[specularTexture.getCurrentFrame()]);
                signalValues.clear();
                signalValues.push_back(valuesFinished[bumpTexture.getCurrentFrame()]);
                specularTexture.submit(true, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);


                bumpTexture.beginRecordCommandBuffers();

                const_cast<Texture&>(depthBuffer.getTexture(depthBuffer.getImageIndex())).toShaderReadOnlyOptimal(bumpTexture.getCommandBuffers()[bumpTexture.getCurrentFrame()]);
                commandBuffers = bumpTexture.getCommandBuffers();
                currentFrame = bumpTexture.getCurrentFrame();

                for (unsigned int p = 0; p < (Batcher::nbPrimitiveTypes -1); p++) {

                    if (nbDrawCommandBuffer[p][0] > 0 || nbIndexedDrawCommandBuffer[p][0] > 0) {
                        vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS,bumpTexture.getGraphicPipeline()[bumpTextureGenerator.getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][LIGHTNODEPTHNOSTENCIL*blendNone.nbBlendModes+blendNone.id]);
                        vkCmdPushConstants(commandBuffers[currentFrame], bumpTexture.getPipelineLayout()[bumpTextureGenerator.getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][LIGHTNODEPTHNOSTENCIL*blendNone.nbBlendModes+blendNone.id], VK_SHADER_STAGE_FRAGMENT_BIT, 148, sizeof(unsigned int), &bumpTexture.getImageIndex());
                    }


                }



                vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);

                memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;


                vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                bumpTexture.beginRenderPass();
                vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &bumpCommandBuffer[currentFrame]);
                bumpTexture.endRenderPass();

                const_cast<Texture&>(depthBuffer.getTexture(depthBuffer.getImageIndex())).toColorAttachmentOptimal(bumpTexture.getCommandBuffers()[bumpTexture.getCurrentFrame()]);

                signalValues.clear();
                valuesFinished[bumpTexture.getCurrentFrame()]++;
                signalValues.push_back(valuesFinished[bumpTexture.getCurrentFrame()]);
                bumpTexture.submit(true, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);


                lightMap.beginRecordCommandBuffers();

                const_cast<Texture&>(specularTexture.getTexture(specularTexture.getImageIndex())).toShaderReadOnlyOptimal(lightMap.getCommandBuffers()[lightMap.getCurrentFrame()]);
                const_cast<Texture&>(bumpTexture.getTexture(bumpTexture.getImageIndex())).toShaderReadOnlyOptimal(lightMap.getCommandBuffers()[lightMap.getCurrentFrame()]);
                const_cast<Texture&>(alphaBuffer.getTexture(alphaBuffer.getImageIndex())).toShaderReadOnlyOptimal(lightMap.getCommandBuffers()[lightMap.getCurrentFrame()]);
                const_cast<Texture&>(depthBuffer.getTexture(depthBuffer.getImageIndex())).toShaderReadOnlyOptimal(lightMap.getCommandBuffers()[lightMap.getCurrentFrame()]);


                commandBuffers = lightMap.getCommandBuffers();
                currentFrame = lightMap.getCurrentFrame();
                BlendMode blendAdd = BlendAdd;
                blendAdd.updateIds();

                for (unsigned int p = 0; p < (Batcher::nbPrimitiveTypes -1); p++) {

                    if (nbDrawCommandBuffer[p][1] > 0) {
                        vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS,lightMap.getGraphicPipeline()[lightMapGenerator.getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][LIGHTNODEPTHNOSTENCIL*blendAdd.nbBlendModes+blendAdd.id]);
                        vkCmdPushConstants(commandBuffers[currentFrame], lightMap.getPipelineLayout()[lightMapGenerator.getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][LIGHTNODEPTHNOSTENCIL*blendAdd.nbBlendModes+blendAdd.id], VK_SHADER_STAGE_FRAGMENT_BIT, 216, sizeof(unsigned int), &lightMap.getImageIndex());
                    }


                }


                vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);

                memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;


                vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

                lightMap.beginRenderPass();
                vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &lightCommandBuffer[currentFrame]);
                lightMap.endRenderPass();


                const_cast<Texture&>(alphaBuffer.getTexture(alphaBuffer.getImageIndex())).toColorAttachmentOptimal(lightMap.getCommandBuffers()[lightMap.getCurrentFrame()]);
                const_cast<Texture&>(bumpTexture.getTexture(bumpTexture.getImageIndex())).toColorAttachmentOptimal(lightMap.getCommandBuffers()[lightMap.getCurrentFrame()]);
                const_cast<Texture&>(depthBuffer.getTexture(depthBuffer.getImageIndex())).toColorAttachmentOptimal(lightMap.getCommandBuffers()[lightMap.getCurrentFrame()]);
                const_cast<Texture&>(specularTexture.getTexture(specularTexture.getImageIndex())).toColorAttachmentOptimal(lightMap.getCommandBuffers()[lightMap.getCurrentFrame()]);
                waitSemaphores.clear();
                waitSemaphores.push_back(offscreenLightDepthAlphaFinishedSemaphore[lightMap.getCurrentFrame()]);
                waitSemaphores.push_back(offscreenFinishedSemaphore[lightMap.getCurrentFrame()]);
                waitStages.clear();
                waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                signalSemaphores.clear();
                signalSemaphores.push_back(offscreenLightDepthAlphaFinishedSemaphore[lightMap.getCurrentFrame()]);
                signalSemaphores.push_back(offscreenFinishedSemaphore[lightMap.getCurrentFrame()]);
                waitValues.clear();
                signalValues.clear();
                waitValues.push_back(valuesLightDepthAlpha[lightMap.getCurrentFrame()]);
                waitValues.push_back(valuesFinished[lightMap.getCurrentFrame()]);
                valuesFinished[lightMap.getCurrentFrame()]++;
                valuesLightDepthAlpha[lightMap.getCurrentFrame()]++;
                signalValues.push_back(valuesLightDepthAlpha[lightMap.getCurrentFrame()]);
                signalValues.push_back(valuesFinished[lightMap.getCurrentFrame()]);
                lightMap.submit(true, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);
            }
            target.beginRecordCommandBuffers();
            const_cast<Texture&>(lightMap.getTexture(lightMap.getImageIndex())).toShaderReadOnlyOptimal(window.getCommandBuffers()[window.getCurrentFrame()]);
            lightMapTile.setCenter(target.getView().getPosition());
            lightMapTile.setTexture(lightMap.getTexture(lightMap.getImageIndex()));

            states.blendMode = BlendMultiply;
            target.draw(lightMapTile, states);

            std::vector<VkSemaphore> signalSemaphores, waitSemaphores;
            std::vector<VkPipelineStageFlags> waitStages;
            std::vector<uint64_t> signalValues, waitValues;
            signalSemaphores.push_back(offscreenFinishedSemaphore[lightMap.getCurrentFrame()]);
            signalSemaphores.push_back(offscreenLightDepthAlphaFinishedSemaphore[lightMap.getCurrentFrame()]);
            waitSemaphores.push_back(offscreenFinishedSemaphore[lightMap.getCurrentFrame()]);
            waitSemaphores.push_back(offscreenLightDepthAlphaFinishedSemaphore[lightMap.getCurrentFrame()]);
            waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
            waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
            waitValues.push_back(valuesFinished[lightMap.getCurrentFrame()]);
            waitValues.push_back(valuesLightDepthAlpha[lightMap.getCurrentFrame()]);
            valuesFinished[lightMap.getCurrentFrame()]++;
            valuesLightDepthAlpha[lightMap.getCurrentFrame()]++;
            signalValues.push_back(valuesFinished[lightMap.getCurrentFrame()]);
            signalValues.push_back(valuesLightDepthAlpha[lightMap.getCurrentFrame()]);
            window.submit(false, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);
            lightDepthBuffer.display();
            depthBuffer.display();
            alphaBuffer.display();
            bumpTexture.display();
            specularTexture.display();
            lightMap.display();
            //std::cout<<"next frame"<<std::endl;
            registerFrameJob[lightDepthBuffer.getCurrentFrame()] = true;
            cv.notify_one();
        }
        bool LightRenderComponent::loadEntitiesOnComponent(std::vector<Entity*> vEntities)
        {
            {
                std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                datasReady = false;
                batcher.clear();
                normalBatcher.clear();
                lightBatcher.clear();
                indexedBatcher.clear();
                normalIndexedBatcher.clear();
                lightIndexedBatcher.clear();
            }

            for (unsigned int i = 0; i < vEntities.size(); i++) {

                if (vEntities[i] != nullptr && vEntities[i]->isLeaf()) {
                    std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                    if (vEntities[i]->isLight()) {
                        for (unsigned int j = 0; j <  vEntities[i]->getNbFaces(); j++) {
                            if (vEntities[i]->getFace(j)->getVertexArray().getIndexes().size() == 0)
                                lightBatcher.addFace(vEntities[i]->getFace(j));
                            else
                                lightIndexedBatcher.addFace(vEntities[i]->getFace(j));
                        }
                    } else {
                        for (unsigned int j = 0; j <  vEntities[i]->getNbFaces(); j++) {
                            if (vEntities[i]->getDrawMode() == Entity::INSTANCED) {
                                if (vEntities[i]->getFace(j)->getVertexArray().getIndexes().size() == 0)
                                    batcher.addFace(vEntities[i]->getFace(j));
                                else
                                    indexedBatcher.addFace(vEntities[i]->getFace(j));
                            } else {
                                if (vEntities[i]->getFace(j)->getVertexArray().getIndexes().size() == 0)
                                    normalBatcher.addFace(vEntities[i]->getFace(j));
                                else
                                    normalIndexedBatcher.addFace(vEntities[i]->getFace(j));
                            }
                        }
                    }
                }
            }

            visibleEntities = vEntities;

            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
            datasReady = true;
            return true;
        }
        void LightRenderComponent::setView(View view){
            this->view = view;
            depthBuffer.setView(view);
            alphaBuffer.setView(view);
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
            if (&rw == &window && event.type == window::IEvent::WINDOW_EVENT && event.window.type == window::IEvent::WINDOW_EVENT_CLOSED) {
                stop = true;
                cv.notify_all();
                getListener().stop();
            }
        }
        LightRenderComponent::~LightRenderComponent() {
            vkDestroyCommandPool(vkDevice.getDevice(), commandPool, nullptr);
            for (unsigned int i = 0; i < offscreenLightDepthAlphaFinishedSemaphore.size(); i++) {
                vkDestroySemaphore(vkDevice.getDevice(), offscreenLightDepthAlphaFinishedSemaphore[i], nullptr);
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

                    vkDestroySampler(vkDevice.getDevice(), lightDepthTextureSampler[i], nullptr);
                    vkDestroyImageView(vkDevice.getDevice(), lightDepthTextureImageView[i], nullptr);
                    vkDestroyImage(vkDevice.getDevice(), lightDepthTextureImage[i], nullptr);
                    vkFreeMemory(vkDevice.getDevice(), lightDepthTextureImageMemory[i], nullptr);
                }

                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, copyModelDataBufferCommandBuffer.size(), copyModelDataBufferCommandBuffer.data());

                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, copyMaterialDataBufferCommandBuffer.size(), copyMaterialDataBufferCommandBuffer.data());

                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, copyDrawBufferCommandBuffer.size(), copyDrawBufferCommandBuffer.data());

                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, copyDrawIndexedBufferCommandBuffer.size(), copyDrawIndexedBufferCommandBuffer.data());
                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, copyVbBufferCommandBuffer.size(), copyVbBufferCommandBuffer.data());
                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, copyVbIndexedBufferCommandBuffer.size(), copyVbIndexedBufferCommandBuffer.data());
                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, lightDepthCommandBuffer.size(), lightDepthCommandBuffer.data());
                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, depthCommandBuffer.size(), depthCommandBuffer.data());
                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, alphaCommandBuffer.size(), alphaCommandBuffer.data());
                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, specularCommandBuffer.size(), specularCommandBuffer.data());
                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, bumpCommandBuffer.size(), bumpCommandBuffer.data());
                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, lightCommandBuffer.size(), lightCommandBuffer.data());
                vkDestroyCommandPool(vkDevice.getDevice(), secondaryBufferCommandPool, nullptr);
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
                                                                                         vec4 coords = vec4(material.lightCenter.xyz, 1);
                                                                                         coords = projectionMatrix * viewMatrix * model.modelMatrix * coords;

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
                            ////////std::cout<<"add texture i : "<<i<<" id : "<<allTextures[i]->getNativeHandle()<<std::endl;
                        }
                        depthBufferGenerator.setParameter("textureMatrix", textureMatrices);
                        specularTextureGenerator.setParameter("textureMatrix", textureMatrices);
                        bumpTextureGenerator.setParameter("textureMatrix", textureMatrices);
                        depthBuffer.setActive(true);
                        glCheck(glGenBuffers(1, &ubo));

                        glCheck(glBindBuffer(GL_UNIFORM_BUFFER, ubo));
                        glCheck(glBufferData(GL_UNIFORM_BUFFER, sizeof(Samplers),allSamplers.tex, GL_STATIC_DRAW));
                        ////////std::cout<<"size : "<<sizeof(Samplers)<<" "<<alignof (alignas(16) uint64_t[200])<<std::endl;

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
                    //////std::cout<<"recompute size"<<std::endl;
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
                ////////std::cout<<"add texture i : "<<i<<" id : "<<allTextures[i]->getNativeHandle()<<std::endl;
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
                    ////////std::cout<<"instance : "<<i<<std::endl;
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
                    ////////std::cout<<"instance : "<<i<<std::endl;
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
                                ////////std::cout<<"add light : "<<el<<std::endl;
                                math::Vec3f center = getWindow().mapCoordsToPixel(el->getCenter() - el->getSize()*0.5f, view);
                                //////std::cout<<"light center : "<<center<<std::endl;
                                center.w = el->getSize().x() * 0.5f;
                                ////////std::cout<<"center : "<<center<<std::endl;
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

