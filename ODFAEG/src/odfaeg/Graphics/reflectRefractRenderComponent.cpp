#include "../../../include/odfaeg/Graphics/reflectRefractRenderComponent.hpp"
#ifndef VULKAN
#include "glCheck.h"
#endif // VULKAN
namespace odfaeg {
    namespace graphic {
        #ifdef VULKAN
            ReflectRefractRenderComponent::ReflectRefractRenderComponent (RenderWindow& window, int layer, std::string expression, window::ContextSettings settings, bool useThread) :
            HeavyComponent(window, math::Vec3f(window.getView().getPosition().x(), window.getView().getPosition().y(), layer),
                          math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0),
                          math::Vec3f(window.getView().getSize().x() + window.getView().getSize().x() * 0.5f, window.getView().getPosition().y() + window.getView().getSize().y() * 0.5f, layer)),
            window(window),
            view(window.getView()),
            expression(expression),
            depthBuffer(window.getDevice()), alphaBuffer(window.getDevice()), environmentMap(window.getDevice()), reflectRefractTex(window.getDevice()),
            useThread(useThread),
            vb(window.getDevice()), vb2(window.getDevice()),
            vbBindlessTex {VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()),
            VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice())},
            vbBindlessTexIndexed {VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()),
            VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice())},
             sBuildDepthBuffer(window.getDevice()), sBuildAlphaBuffer(window.getDevice()), sReflectRefract(window.getDevice()), sLinkedList(window.getDevice()),
             sLinkedList2(window.getDevice()), skyboxShader(window.getDevice()),
             clearHeadptrComputeShader(window.getDevice()),
             vkDevice(window.getDevice())
             {
                vboIndirect = vboIndirectStagingBuffer = vboIndexedIndirectStagingBuffer = modelDataStagingBuffer = materialDataStagingBuffer = nullptr;
                maxVboIndirectSize = maxModelDataSize = maxMaterialDataSize = 0;
                createCommandPool();
                if (window.getView().getSize().x() > window.getView().getSize().y()) {
                    squareSize = window.getView().getSize().x();
                } else {
                    squareSize = window.getView().getSize().y();
                }
                needToUpdateDS = false;
                quad = RectangleShape(math::Vec3f(squareSize, squareSize, squareSize * 0.5f));
                quad.move(math::Vec3f(-squareSize * 0.5f, -squareSize * 0.5f, 0));
                resolution = math::Vec4f((int) window.getSize().x(), (int) window.getSize().y(), window.getView().getSize().z(), 1);
                dirs[0] = math::Vec3f(1, 0, 0);
                dirs[1] = math::Vec3f(-1, 0, 0);
                dirs[2] = math::Vec3f(0, 1, 0);
                dirs[3] = math::Vec3f(0, -1, 0);
                dirs[4] = math::Vec3f(0, 0, 1);
                dirs[5] = math::Vec3f(0, 0, -1);
                ups[0] = math::Vec3f(0, -1, 0);
                ups[1] = math::Vec3f(0, -1, 0);
                ups[2] = math::Vec3f(0, 0, 1);
                ups[3] = math::Vec3f(0, 0, -1);
                ups[4] = math::Vec3f(0, -1, 0);
                ups[5] = math::Vec3f(0, -1, 0);
                //depthBuffer.m_name = "depthBuffer";
                depthBuffer.create(window.getView().getSize().x(), window.getView().getSize().y());
                ////std::cout<<"depth buffer created"<<std::endl;
                depthBuffer.setView(view);

                depthBufferSprite = Sprite(depthBuffer.getTexture(depthBuffer.getImageIndex()), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0), IntRect(0, 0, window.getView().getSize().x(), window.getView().getSize().y()));
                alphaBuffer.create(window.getView().getSize().x(), window.getView().getSize().y());
                ////std::cout<<"alpha buffer created"<<std::endl;
                alphaBuffer.setView(view);
                //alphaBuffer.m_name = "alphaBuffer";
                alphaBufferSprite = Sprite(alphaBuffer.getTexture(alphaBuffer.getImageIndex()), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0), IntRect(0, 0, window.getView().getSize().x(), window.getView().getSize().y()));
                environmentMap.createCubeMap(squareSize, squareSize);
                ////std::cout<<"environment map created"<<std::endl;
                //environmentMap.m_name = "environmentMap";
                reflectRefractTex.create(window.getView().getSize().x(), window.getView().getSize().y());
                reflectRefractTex.setView(view);
                //reflectRefractTex.m_name = "relfectRefractTex";

                reflectRefractTexSprite = Sprite(reflectRefractTex.getTexture(reflectRefractTex.getImageIndex()), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0), IntRect(0, 0, window.getView().getSize().x(), window.getView().getSize().y()));
                linkedListShaderStorageBuffers.resize(reflectRefractTex.getMaxFramesInFlight());
                linkedListShaderStorageBuffersMemory.resize(reflectRefractTex.getMaxFramesInFlight());
                maxNodes = 20 * squareSize * squareSize;;
                unsigned int nodeSize = 5  * sizeof(float) + sizeof(unsigned int);
                VkDeviceSize bufferSize = maxNodes * nodeSize * 6;
                for (unsigned int i = 0; i < environmentMap.getMaxFramesInFlight(); i++) {
                    createBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, linkedListShaderStorageBuffers[i], linkedListShaderStorageBuffersMemory[i]);
                }

                core::FastDelegate<bool> signal (&ReflectRefractRenderComponent::needToUpdate, this);
                core::FastDelegate<void> slot (&ReflectRefractRenderComponent::drawNextFrame, this);
                core::Command cmd(signal, slot);
                getListener().connect("UPDATE", cmd);

                AtomicCounterSSBO counter;
                for (unsigned int i = 0; i < 6; i++) {
                    counter.count[i] = 0;
                }
                VkBuffer stagingBuffer;
                VkDeviceMemory stagingBufferMemory;
                counter.maxNodeCount = maxNodes;
                bufferSize = sizeof(AtomicCounterSSBO);


                createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
                void* data;
                vkMapMemory(vkDevice.getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
                memcpy(data, &counter, (size_t)bufferSize);
                vkUnmapMemory(vkDevice.getDevice(), stagingBufferMemory);
                for (unsigned int i = 0; i < counterShaderStorageBuffers.size(); i++) {
                    vkDestroyBuffer(vkDevice.getDevice(), counterShaderStorageBuffers[i], nullptr);
                    vkFreeMemory(vkDevice.getDevice(), counterShaderStorageBuffersMemory[i], nullptr);
                }
                counterShaderStorageBuffers.resize(reflectRefractTex.getMaxFramesInFlight());
                counterShaderStorageBuffersMemory.resize(reflectRefractTex.getMaxFramesInFlight());
                for (size_t i = 0; i < reflectRefractTex.getMaxFramesInFlight(); i++) {
                    createBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, counterShaderStorageBuffers[i], counterShaderStorageBuffersMemory[i]);
                    copyBuffer(stagingBuffer, counterShaderStorageBuffers[i], bufferSize);
                }
                vkDestroyBuffer(vkDevice.getDevice(), stagingBuffer, nullptr);
                vkFreeMemory(vkDevice.getDevice(), stagingBufferMemory, nullptr);
                createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
                uint32_t count = 6;
                bufferSize = sizeof(uint32_t);
                vkMapMemory(vkDevice.getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
                memcpy(data, &count, (size_t)bufferSize);
                vkUnmapMemory(vkDevice.getDevice(), stagingBufferMemory);
                createBuffer(bufferSize, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vboCount, vboCountMemory);
                copyBuffer(stagingBuffer, vboCount, bufferSize);
                vkDestroyBuffer(vkDevice.getDevice(), stagingBuffer, nullptr);
                vkFreeMemory(vkDevice.getDevice(), stagingBufferMemory, nullptr);
                VkImageCreateInfo imageInfo{};
                imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                imageInfo.imageType = VK_IMAGE_TYPE_2D;
                imageInfo.extent.width = static_cast<uint32_t>(squareSize);
                imageInfo.extent.height = static_cast<uint32_t>(squareSize);
                imageInfo.extent.depth = 1;
                imageInfo.mipLevels = 1;
                imageInfo.arrayLayers = 6;
                imageInfo.format = VK_FORMAT_R32_UINT;
                imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
                imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
                imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                imageInfo.flags = 0; // Optionnel
                for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    if (vkCreateImage(window.getDevice().getDevice(), &imageInfo, nullptr, &headPtrTextureImage[i]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de la creation d'une image!");
                    }
                    VkMemoryRequirements memRequirements;
                    vkGetImageMemoryRequirements(window.getDevice().getDevice(), headPtrTextureImage[i], &memRequirements);

                    VkMemoryAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                    allocInfo.allocationSize = memRequirements.size;
                    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);


                    if (vkAllocateMemory(window.getDevice().getDevice(), &allocInfo, nullptr, &headPtrTextureImageMemory[i]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de l'allocation de la memoire d'une image!");
                    }
                    vkBindImageMemory(window.getDevice().getDevice(), headPtrTextureImage[i], headPtrTextureImageMemory[i], 0);
                }
                //transitionImageLayout(headPtrTextureImage, VK_FORMAT_R32_UINT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

                VkImageCreateInfo imageInfo2{};
                imageInfo2.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                imageInfo2.imageType = VK_IMAGE_TYPE_2D;
                imageInfo2.extent.width = static_cast<uint32_t>(window.getView().getSize().x());
                imageInfo2.extent.height = static_cast<uint32_t>(window.getView().getSize().y());
                imageInfo2.extent.depth = 1;
                imageInfo2.mipLevels = 1;
                imageInfo2.arrayLayers = 1;
                imageInfo2.format =  VK_FORMAT_R32G32B32A32_SFLOAT;
                imageInfo2.tiling = VK_IMAGE_TILING_OPTIMAL;
                imageInfo2.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                imageInfo2.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
                imageInfo2.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                imageInfo2.samples = VK_SAMPLE_COUNT_1_BIT;
                imageInfo2.flags = 0; // Optionnel
                for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    if (vkCreateImage(window.getDevice().getDevice(), &imageInfo2, nullptr, &depthTextureImage[i]) != VK_SUCCESS) {
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
                    if (vkCreateImage(window.getDevice().getDevice(), &imageInfo3, nullptr, &alphaTextureImage[i]) != VK_SUCCESS) {
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
                //transitionImageLayout(alphaTextureImage, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
                createImageView();
                createSampler();
                if (!useThread)
                    createUniformBuffers();
                compileShaders();
                depthBuffer.beginRecordCommandBuffers();
                std::vector<VkCommandBuffer> commandBuffers = depthBuffer.getCommandBuffers();
                unsigned int currentFrame =  depthBuffer.getCurrentFrame();
                for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    VkImageMemoryBarrier barrier = {};
                    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
                    barrier.image = headPtrTextureImage[i];
                    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    barrier.subresourceRange.levelCount = 1;
                    barrier.subresourceRange.layerCount = 6;
                    vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
                    VkImageMemoryBarrier barrier2 = {};
                    barrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    barrier2.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    barrier2.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    barrier2.newLayout = VK_IMAGE_LAYOUT_GENERAL;
                    barrier2.image = depthTextureImage[i];
                    barrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    barrier2.subresourceRange.levelCount = 1;
                    barrier2.subresourceRange.layerCount = 1;
                    vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier2);
                    VkImageMemoryBarrier barrier3 = {};
                    barrier3.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    barrier3.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    barrier3.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    barrier3.newLayout = VK_IMAGE_LAYOUT_GENERAL;
                    barrier3.image = alphaTextureImage[i];
                    barrier3.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    barrier3.subresourceRange.levelCount = 1;
                    barrier3.subresourceRange.layerCount = 1;
                    vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier3);



                }
                for (unsigned int i = 0; i < depthBuffer.getSwapchainImagesSize(); i++) {
                    const_cast<Texture&>(reflectRefractTex.getTexture(i)).toShaderReadOnlyOptimal(depthBuffer.getCommandBuffers()[depthBuffer.getCurrentFrame()]);
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
                for (unsigned int i = 0; i < reflectRefractTex.getMaxFramesInFlight(); i++) {
                    VkEventCreateInfo eventInfo = {};
                    eventInfo.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
                    eventInfo.pNext = NULL;
                    eventInfo.flags = 0;
                    VkEvent event;
                    vkCreateEvent(vkDevice.getDevice(), &eventInfo, NULL, &event);
                    events.push_back(event);
                }
                vkCmdPushDescriptorSetKHR = (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(vkDevice.getDevice(), "vkCmdPushDescriptorSetKHR");
                skybox = nullptr;
                datasReady = false;
                VkSemaphoreCreateInfo semaphoreInfo{};
                semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;




                offscreenAlphaPassFinishedSemaphore.resize(alphaBuffer.getMaxFramesInFlight());
                for (unsigned int i = 0; i < alphaBuffer.getMaxFramesInFlight(); i++) {
                    if (vkCreateSemaphore(vkDevice.getDevice(), &semaphoreInfo, nullptr, &offscreenAlphaPassFinishedSemaphore[i]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create compute synchronization objects for a frame!");
                    }
                }
                offscreenDepthPassFinishedSemaphore.resize(depthBuffer.getMaxFramesInFlight());
                for (unsigned int i = 0; i < depthBuffer.getMaxFramesInFlight(); i++) {
                    if (vkCreateSemaphore(vkDevice.getDevice(), &semaphoreInfo, nullptr, &offscreenDepthPassFinishedSemaphore[i]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create compute synchronization objects for a frame!");
                    }
                }
                VkSemaphoreTypeCreateInfo timelineCreateInfo{};
                timelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
                timelineCreateInfo.pNext = nullptr;
                timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
                semaphoreInfo.pNext = &timelineCreateInfo;
                for (unsigned int i = 0; i < valuesCopy.size(); i++) {
                    valuesCopy[i] = 0;
                }
                copyFinishedSemaphore.resize(depthBuffer.getMaxFramesInFlight());
                for (unsigned int i = 0; i < depthBuffer.getMaxFramesInFlight(); i++) {
                    timelineCreateInfo.initialValue = valuesCopy[i];
                    if (vkCreateSemaphore(vkDevice.getDevice(), &semaphoreInfo, nullptr, &copyFinishedSemaphore[i]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create compute synchronization objects for a frame!");
                    }
                }
                for (unsigned int i = 0; i < values.size(); i++) {
                    values[i] = 0;
                }
                offscreenRenderingFinishedSemaphore.resize(reflectRefractTex.getMaxFramesInFlight());
                for (unsigned int i = 0; i < reflectRefractTex.getMaxFramesInFlight(); i++) {
                    timelineCreateInfo.initialValue = values[i];
                    if (vkCreateSemaphore(vkDevice.getDevice(), &semaphoreInfo, nullptr, &offscreenRenderingFinishedSemaphore[i]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create compute synchronization objects for a frame!");
                    }
                }


                isSomethingDrawn = false;
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
                if (vkAllocateCommandBuffers(vkDevice.getDevice(), &bufferAllocInfo, copyVbEnvPass2BufferCommandBuffer.data()) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to allocate command buffers!", 1);
                }
                if (vkAllocateCommandBuffers(vkDevice.getDevice(), &bufferAllocInfo, depthBufferCommandBuffer.data()) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to allocate command buffers!", 1);
                }
                if (vkAllocateCommandBuffers(vkDevice.getDevice(), &bufferAllocInfo, alphaBufferCommandBuffer.data()) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to allocate command buffers!", 1);
                }
                if (vkAllocateCommandBuffers(vkDevice.getDevice(), &bufferAllocInfo, environmentMapPass2CommandBuffer.data()) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to allocate command buffers!", 1);
                }
                if (vkAllocateCommandBuffers(vkDevice.getDevice(), &bufferAllocInfo, copySkyboxCommandBuffer.data()) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to allocate command buffers!", 1);
                }
                VkPhysicalDeviceProperties deviceProperties;
                vkGetPhysicalDeviceProperties(vkDevice.getPhysicalDevice(), &deviceProperties);

                uboAlignment = deviceProperties.limits.minUniformBufferOffsetAlignment;
                alignment = deviceProperties.limits.minStorageBufferOffsetAlignment;
                ////std::cout<<"align : "<<uboAlignment<<std::endl;
                update = true;
                needToUpdateDS = false;
                vb2.append(Vertex(math::Vec3f(0, 0, 0)));
                vb2.update();

            }
            void ReflectRefractRenderComponent::createDescriptorsAndPipelines() {

                RenderStates states;
                if (useThread) {
                    states.shader = &skyboxShader;
                    createDescriptorSetLayout(states);
                    for (unsigned int p = 0; p < (Batcher::nbPrimitiveTypes - 1); p++) {
                        createDescriptorPool(p, states);
                        allocateDescriptorSets(p, states);
                    }
                    states.shader = &sLinkedList;
                    createDescriptorSetLayout(states);
                    for (unsigned int p = 0; p < (Batcher::nbPrimitiveTypes - 1); p++) {
                        createDescriptorPool(p, states);
                        allocateDescriptorSets(p, states);
                    }
                    states.shader = &sLinkedList2;
                    createDescriptorSetLayout(states);
                    for (unsigned int p = 0; p < (Batcher::nbPrimitiveTypes - 1); p++) {
                        createDescriptorPool(p, states);
                        allocateDescriptorSets(p, states);
                    }
                    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
                        updateDescriptorSets(i, 0, states);
                    states.shader = &sBuildDepthBuffer;
                    createDescriptorSetLayout(states);
                    for (unsigned int p = 0; p < (Batcher::nbPrimitiveTypes - 1); p++) {
                        createDescriptorPool(p, states);
                        allocateDescriptorSets(p, states);
                    }
                    states.shader = &sBuildAlphaBuffer;
                    createDescriptorSetLayout(states);
                    for (unsigned int p = 0; p < (Batcher::nbPrimitiveTypes - 1); p++) {
                        createDescriptorPool(p, states);
                        allocateDescriptorSets(p, states);
                    }
                    states.shader = &sReflectRefract;
                    createDescriptorSetLayout(states);
                    for (unsigned int p = 0; p < (Batcher::nbPrimitiveTypes - 1); p++) {
                        createDescriptorPool(p, states);
                        allocateDescriptorSets(p, states);
                    }
                } else {
                    states.shader = &sLinkedList;
                    createDescriptorPool(states);
                    createDescriptorSetLayout(states);
                    allocateDescriptorSets(states);
                    states.shader = &sLinkedList2;
                    createDescriptorPool(states);
                    createDescriptorSetLayout(states);
                    allocateDescriptorSets(states);
                    createDescriptorSets(states);
                    states.shader = &sBuildDepthBuffer;
                    createDescriptorPool(states);
                    createDescriptorSetLayout(states);
                    allocateDescriptorSets(states);
                    states.shader = &sBuildAlphaBuffer;
                    createDescriptorPool(states);
                    createDescriptorSetLayout(states);
                    allocateDescriptorSets(states);
                    states.shader = &sReflectRefract;
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

                environmentMap.enableDepthTest(true);
                depthBuffer.enableDepthTest(true);
                alphaBuffer.enableDepthTest(true);
                reflectRefractTex.enableDepthTest(true);
                states.shader = &skyboxShader;
                {
                    std::vector<std::vector<std::vector<VkPipelineLayoutCreateInfo>>>& pipelineLayoutInfo = environmentMap.getPipelineLayoutCreateInfo();
                    std::vector<std::vector<std::vector<VkPipelineDepthStencilStateCreateInfo>>>& depthStencilCreateInfo = environmentMap.getDepthStencilCreateInfo();
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
                            if (RRRCNBDEPTHSTENCIL*none.nbBlendModes > pipelineLayoutInfo[i][j].size()) {
                                pipelineLayoutInfo[i][j].resize(RRRCNBDEPTHSTENCIL*none.nbBlendModes);
                                depthStencilCreateInfo[i][j].resize(RRRCNBDEPTHSTENCIL*none.nbBlendModes);
                            }
                        }
                    }
                    for (unsigned int b = 0; b < blendModes.size(); b++) {
                        states.blendMode = blendModes[b];
                        states.blendMode.updateIds();
                        for (unsigned int j = 0; j < RRRCNBDEPTHSTENCIL; j++) {
                            for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes - 1; i++) {
                                if (j == 0) {
                                   depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                                   depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = {};
                                   depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back = {};
                                   environmentMap.createGraphicPipeline(static_cast<PrimitiveType>(i), states, RRRCNODEPTHNOSTENCIL, RRRCNBDEPTHSTENCIL);
                                } else {
                                   depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCDEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_GREATER;
                                   depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCDEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = {};
                                   depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCDEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back = {};
                                   environmentMap.createGraphicPipeline(static_cast<PrimitiveType>(i), states, RRRCDEPTHNOSTENCIL, RRRCNBDEPTHSTENCIL);
                                }
                            }
                        }
                    }
                }
                states.shader = &sLinkedList;
                {
                    std::vector<std::vector<std::vector<VkPipelineLayoutCreateInfo>>>& pipelineLayoutInfo = environmentMap.getPipelineLayoutCreateInfo();
                    std::vector<std::vector<std::vector<VkPipelineDepthStencilStateCreateInfo>>>& depthStencilCreateInfo = environmentMap.getDepthStencilCreateInfo();
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
                            if (RRRCNBDEPTHSTENCIL*none.nbBlendModes > pipelineLayoutInfo[i][j].size()) {
                                pipelineLayoutInfo[i][j].resize(RRRCNBDEPTHSTENCIL*none.nbBlendModes);
                                depthStencilCreateInfo[i][j].resize(RRRCNBDEPTHSTENCIL*none.nbBlendModes);
                            }
                        }
                    }
                    for (unsigned int b = 0; b < blendModes.size(); b++) {
                        states.blendMode = blendModes[b];
                        states.blendMode.updateIds();
                        for (unsigned int j = 0; j < RRRCNBDEPTHSTENCIL; j++) {
                            for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes - 1; i++) {
                                if (j == 0) {
                                   depthStencilCreateInfo[sLinkedList.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                                   depthStencilCreateInfo[sLinkedList.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = {};
                                   depthStencilCreateInfo[sLinkedList.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back = {};
                                   environmentMap.createGraphicPipeline(static_cast<PrimitiveType>(i), states, RRRCNODEPTHNOSTENCIL, RRRCNBDEPTHSTENCIL);
                                } else {
                                   depthStencilCreateInfo[sLinkedList.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCDEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_GREATER;
                                   depthStencilCreateInfo[sLinkedList.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCDEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = {};
                                   depthStencilCreateInfo[sLinkedList.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCDEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back = {};
                                   environmentMap.createGraphicPipeline(static_cast<PrimitiveType>(i), states, RRRCDEPTHNOSTENCIL, RRRCNBDEPTHSTENCIL);
                                }
                            }
                        }
                    }
                    states.shader = &sLinkedList2;
                    for (unsigned int b = 0; b < blendModes.size(); b++) {
                        states.blendMode = blendModes[b];
                        states.blendMode.updateIds();
                        for (unsigned int j = 0; j < RRRCNBDEPTHSTENCIL; j++) {
                            for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes - 1; i++) {
                                if (j == 0) {
                                   VkPushConstantRange push_constant;
                                   //this push constant range starts at the beginning
                                   push_constant.offset = 0;
                                   //this push constant range takes up the size of a MeshPushConstants struct
                                   push_constant.size = sizeof(LinkedList2PC);
                                   //this push constant range is accessible only in the vertex shader
                                   push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                                   pipelineLayoutInfo[sLinkedList2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = &push_constant;
                                   pipelineLayoutInfo[sLinkedList2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 1;
                                   depthStencilCreateInfo[sLinkedList2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                                   depthStencilCreateInfo[sLinkedList2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = {};
                                   depthStencilCreateInfo[sLinkedList2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back = {};
                                   environmentMap.createGraphicPipeline(static_cast<PrimitiveType>(i), states, RRRCNODEPTHNOSTENCIL, RRRCNBDEPTHSTENCIL);
                                   ////////std::cout<<"pipeline ids : "<<sLinkedList2.getId() * (Batcher::nbPrimitiveTypes - 1) + i<<","<<environmentMap.getId()<<","<<RRRCNODEPTHNOSTENCIL<<std::endl;
                                } else {
                                   VkPushConstantRange push_constant;
                                   //this push constant range starts at the beginning
                                   push_constant.offset = 0;
                                   //this push constant range takes up the size of a MeshPushConstants struct
                                   push_constant.size = sizeof(LinkedList2PC);
                                   //this push constant range is accessible only in the vertex shader
                                   push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                                   pipelineLayoutInfo[sLinkedList2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCDEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = &push_constant;
                                   pipelineLayoutInfo[sLinkedList2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCDEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 1;
                                   depthStencilCreateInfo[sLinkedList2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCDEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_GREATER;
                                   depthStencilCreateInfo[sLinkedList2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCDEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = {};
                                   depthStencilCreateInfo[sLinkedList2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCDEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back = {};
                                   environmentMap.createGraphicPipeline(static_cast<PrimitiveType>(i), states, RRRCDEPTHNOSTENCIL, RRRCNBDEPTHSTENCIL);
                                }
                            }
                        }
                    }
                }
                {
                    std::vector<std::vector<std::vector<VkPipelineLayoutCreateInfo>>>& pipelineLayoutInfo = depthBuffer.getPipelineLayoutCreateInfo();
                    std::vector<std::vector<std::vector<VkPipelineDepthStencilStateCreateInfo>>>& depthStencilCreateInfo = depthBuffer.getDepthStencilCreateInfo();
                    //std::cout<<"size : "<<pipelineLayoutInfo.size()<<std::endl;
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
                            if (RRRCNBDEPTHSTENCIL*none.nbBlendModes > pipelineLayoutInfo[i][j].size()) {
                                pipelineLayoutInfo[i][j].resize(RRRCNBDEPTHSTENCIL*none.nbBlendModes);
                                depthStencilCreateInfo[i][j].resize(RRRCNBDEPTHSTENCIL*none.nbBlendModes);
                            }
                        }
                    }
                    states.shader = &sBuildDepthBuffer;
                    for (unsigned int b = 0; b < blendModes.size(); b++) {
                        states.blendMode = blendModes[b];
                        states.blendMode.updateIds();
                        for (unsigned int j = 0; j < RRRCNBDEPTHSTENCIL; j++) {
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
                                    push_constant2.size = sizeof(BuildDepthPC);
                                    //this push constant range is accessible only in the vertex shader
                                    push_constant2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                                    push_constants[1] = push_constant2;
                                    pipelineLayoutInfo[sBuildDepthBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = push_constants.data();
                                    pipelineLayoutInfo[sBuildDepthBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 2;
                                    depthStencilCreateInfo[sBuildDepthBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                                    depthStencilCreateInfo[sBuildDepthBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = {};
                                    depthStencilCreateInfo[sBuildDepthBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back = {};
                                    //std::cout<<"depth buffer ids  : "<<sBuildDepthBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i<<","<<0<<","<<RRRCNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id<<std::endl;
                                    depthBuffer.createGraphicPipeline(static_cast<PrimitiveType>(i), states, RRRCNODEPTHNOSTENCIL, RRRCNBDEPTHSTENCIL);
                                } else {
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
                                    push_constant2.size = sizeof(BuildDepthPC);
                                    //this push constant range is accessible only in the vertex shader
                                    push_constant2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                                    push_constants[1] = push_constant2;
                                    pipelineLayoutInfo[sBuildDepthBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCDEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = push_constants.data();
                                    pipelineLayoutInfo[sBuildDepthBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCDEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 2;
                                    depthStencilCreateInfo[sBuildDepthBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCDEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_GREATER;
                                    depthStencilCreateInfo[sBuildDepthBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCDEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = {};
                                    depthStencilCreateInfo[sBuildDepthBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCDEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back = {};
                                    depthBuffer.createGraphicPipeline(static_cast<PrimitiveType>(i), states, RRRCDEPTHNOSTENCIL, RRRCNBDEPTHSTENCIL);
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
                            if (RRRCNBDEPTHSTENCIL*none.nbBlendModes > pipelineLayoutInfo[i][j].size()) {
                                pipelineLayoutInfo[i][j].resize(RRRCNBDEPTHSTENCIL*none.nbBlendModes);
                                depthStencilCreateInfo[i][j].resize(RRRCNBDEPTHSTENCIL*none.nbBlendModes);
                            }
                        }
                    }
                    states.shader = &sBuildAlphaBuffer;
                    for (unsigned int b = 0; b < blendModes.size(); b++) {
                        states.blendMode = blendModes[b];
                        states.blendMode.updateIds();

                        for (unsigned int j = 0; j < RRRCNBDEPTHSTENCIL; j++) {
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
                                    push_constant2.size = sizeof(BuildAlphaPC) + sizeof(unsigned int);
                                    //this push constant range is accessible only in the vertex shader
                                    push_constant2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                                    push_constants[1] = push_constant2;
                                    pipelineLayoutInfo[sBuildAlphaBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = push_constants.data();
                                    pipelineLayoutInfo[sBuildAlphaBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 2;
                                    depthStencilCreateInfo[sBuildAlphaBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                                    depthStencilCreateInfo[sBuildAlphaBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = {};
                                    depthStencilCreateInfo[sBuildAlphaBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back = {};
                                    //std::cout<<"alpha ids : "<<sBuildAlphaBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i<<","<<RRRCNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id<<std::endl;
                                    alphaBuffer.createGraphicPipeline(static_cast<PrimitiveType>(i), states, RRRCNODEPTHNOSTENCIL, RRRCNBDEPTHSTENCIL);
                                } else {
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
                                    push_constant2.size = sizeof(BuildAlphaPC) + sizeof(unsigned int);;
                                    //this push constant range is accessible only in the vertex shader
                                    push_constant2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                                    push_constants[1] = push_constant2;
                                    pipelineLayoutInfo[sBuildAlphaBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCDEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = push_constants.data();
                                    pipelineLayoutInfo[sBuildAlphaBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCDEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 2;
                                    depthStencilCreateInfo[sBuildAlphaBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCDEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_GREATER;
                                    depthStencilCreateInfo[sBuildAlphaBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCDEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = {};
                                    depthStencilCreateInfo[sBuildAlphaBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCDEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back = {};
                                    alphaBuffer.createGraphicPipeline(static_cast<PrimitiveType>(i), states, RRRCDEPTHNOSTENCIL, RRRCNBDEPTHSTENCIL);
                                }
                            }
                        }
                    }
                }
                {
                    std::vector<std::vector<std::vector<VkPipelineLayoutCreateInfo>>>& pipelineLayoutInfo = reflectRefractTex.getPipelineLayoutCreateInfo();
                    std::vector<std::vector<std::vector<VkPipelineDepthStencilStateCreateInfo>>>& depthStencilCreateInfo = reflectRefractTex.getDepthStencilCreateInfo();
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
                            if (RRRCNBDEPTHSTENCIL*none.nbBlendModes > pipelineLayoutInfo[i][j].size()) {
                                pipelineLayoutInfo[i][j].resize(RRRCNBDEPTHSTENCIL*none.nbBlendModes);
                                depthStencilCreateInfo[i][j].resize(RRRCNBDEPTHSTENCIL*none.nbBlendModes);
                            }
                        }
                    }
                    states.shader = &sReflectRefract;
                    for (unsigned int b = 0; b < blendModes.size(); b++) {
                        states.blendMode = blendModes[b];
                        states.blendMode.updateIds();

                        for (unsigned int j = 0; j < RRRCNBDEPTHSTENCIL; j++) {
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
                                    push_constant2.size = sizeof(BuildFrameBufferPC) + sizeof(unsigned int);
                                    //this push constant range is accessible only in the vertex shader
                                    push_constant2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                                    push_constants[1] = push_constant2;
                                    pipelineLayoutInfo[sReflectRefract.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = push_constants.data();
                                    pipelineLayoutInfo[sReflectRefract.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 2;
                                    depthStencilCreateInfo[sReflectRefract.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                                    depthStencilCreateInfo[sReflectRefract.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = {};
                                    depthStencilCreateInfo[sReflectRefract.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back = {};
                                    reflectRefractTex.createGraphicPipeline(static_cast<PrimitiveType>(i), states, RRRCNODEPTHNOSTENCIL, RRRCNBDEPTHSTENCIL);
                                } else {
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
                                    push_constant2.size = sizeof(BuildFrameBufferPC) + sizeof(unsigned int);;
                                    //this push constant range is accessible only in the vertex shader
                                    push_constant2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                                    push_constants[1] = push_constant2;
                                    pipelineLayoutInfo[sReflectRefract.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCDEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = push_constants.data();
                                    pipelineLayoutInfo[sReflectRefract.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCDEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 2;
                                    depthStencilCreateInfo[sReflectRefract.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCDEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_GREATER;
                                    depthStencilCreateInfo[sReflectRefract.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCDEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = {};
                                    depthStencilCreateInfo[sReflectRefract.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][RRRCDEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back = {};
                                    reflectRefractTex.createGraphicPipeline(static_cast<PrimitiveType>(i), states, RRRCDEPTHNOSTENCIL, RRRCNBDEPTHSTENCIL);
                                }
                            }
                        }
                    }
                }
            }
            void ReflectRefractRenderComponent::launchRenderer() {
                if (useThread) {
                    getListener().launch();
                }
            }

            VkCommandBuffer ReflectRefractRenderComponent::beginSingleTimeCommands() {
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

                return commandBuffer;
            }

            void ReflectRefractRenderComponent::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
                vkEndCommandBuffer(commandBuffer);

                VkSubmitInfo submitInfo{};
                submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &commandBuffer;

                vkQueueSubmit(vkDevice.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
                vkQueueWaitIdle(vkDevice.getGraphicsQueue());

                vkFreeCommandBuffers(vkDevice.getDevice(), commandPool, 1, &commandBuffer);
            }
            void ReflectRefractRenderComponent::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
                VkCommandBuffer commandBuffer = beginSingleTimeCommands();

                VkImageMemoryBarrier barrier{};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.oldLayout = oldLayout;
                barrier.newLayout = newLayout;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.image = image;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = 1;

                VkPipelineStageFlags sourceStage;
                VkPipelineStageFlags destinationStage;

                if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                    barrier.srcAccessMask = 0;
                    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                } else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
                    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                    sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
                    barrier.srcAccessMask = 0;
                    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                    destinationStage =  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                } else {
                   throw std::invalid_argument("unsupported layout transition!");
                }
                vkCmdPipelineBarrier(
                commandBuffer,
                sourceStage, destinationStage,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
                );

                endSingleTimeCommands(commandBuffer);
            }
            void ReflectRefractRenderComponent::loadTextureIndexes() {
            }
            std::vector<Entity*> ReflectRefractRenderComponent::getEntities() {
                return visibleEntities;
            }
            uint32_t ReflectRefractRenderComponent::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
                VkPhysicalDeviceMemoryProperties memProperties;
                vkGetPhysicalDeviceMemoryProperties(vkDevice.getPhysicalDevice(), &memProperties);
                for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
                    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                        return i;
                    }
                }
                throw std::runtime_error("aucun type de memoire ne satisfait le buffer!");
            }
            void ReflectRefractRenderComponent::createUniformBuffers() {
                VkDeviceSize bufferSize = sizeof(UniformBufferObject);
                uniformBuffer.resize(environmentMap.getMaxFramesInFlight());
                uniformBufferMemory.resize(environmentMap.getMaxFramesInFlight());
                for (size_t i = 0; i < environmentMap.getMaxFramesInFlight(); i++) {
                    createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffer[i], uniformBufferMemory[i]);
                    ////////std::cout<<"uniform buffer : "<<ubos[i]<<std::endl;
                }
            }
            void ReflectRefractRenderComponent::createUniformBuffersMT() {
                VkDeviceSize bufferSize = nbReflRefrEntities * alignUBO(sizeof(UniformBufferObject));
                uniformBuffer.resize(environmentMap.getMaxFramesInFlight());
                uniformBufferMemory.resize(environmentMap.getMaxFramesInFlight());
                for (size_t i = 0; i < environmentMap.getMaxFramesInFlight(); i++) {
                    if (uniformBuffer[i] != nullptr) {
                        vkDestroyBuffer(vkDevice.getDevice(), uniformBuffer[i], nullptr);
                        vkFreeMemory(vkDevice.getDevice(), uniformBufferMemory[i], nullptr);
                    }
                    createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffer[i], uniformBufferMemory[i]);
                    ////////std::cout<<"uniform buffer : "<<ubos[i]<<std::endl;
                }
            }
            void ReflectRefractRenderComponent::updateUniformBuffer(uint32_t currentImage, std::vector<UniformBufferObject> ubo) {
                VkDeviceSize alignedSize = ((sizeof(UniformBufferObject) + uboAlignment - 1) / uboAlignment) * uboAlignment;

                void* data;
                vkMapMemory(vkDevice.getDevice(), uniformBufferMemory[currentImage], 0, alignedSize * ubos.size(), 0, &data);

                for (size_t i = 0; i < ubos.size(); ++i) {
                    memcpy(static_cast<char*>(data) + i * alignedSize, &ubos[i], sizeof(UniformBufferObject));
                }

                vkUnmapMemory(vkDevice.getDevice(), uniformBufferMemory[currentImage]);

            }
            void ReflectRefractRenderComponent::updateUniformBuffer(uint32_t currentImage, UniformBufferObject ubo) {
                void* data;
                vkMapMemory(vkDevice.getDevice(), uniformBufferMemory[currentImage], 0, sizeof(ubo), 0, &data);
                    memcpy(data, &ubo, sizeof(ubo));
                vkUnmapMemory(vkDevice.getDevice(), uniformBufferMemory[currentImage]);

            }
            void ReflectRefractRenderComponent::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
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
            void ReflectRefractRenderComponent::createCommandPool() {
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
            void ReflectRefractRenderComponent::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandBuffer cmd) {

                if (srcBuffer != nullptr && dstBuffer != nullptr) {
                    VkBufferCopy copyRegion{};
                    copyRegion.size = size;
                    vkCmdCopyBuffer(cmd, srcBuffer, dstBuffer, 1, &copyRegion);
                }

            }
            void ReflectRefractRenderComponent::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
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
            void ReflectRefractRenderComponent::compileShaders() {
                const std::string clearHeadptrComputeShaderCode = R"(#version 460
                                                                     layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
                                                                     layout(binding = 0, r32ui) uniform uimage2DArray headPtr;
                                                                     layout(binding = 1) buffer CounterSSBO {
                                                                        uint count[6];
                                                                        uint maxNodes;
                                                                     };
                                                                     void main() {
                                                                        ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
                                                                        int layer = int(gl_GlobalInvocationID.z);
                                                                        imageStore(headPtr, ivec3(texelCoord, layer), uvec4(0xFFFFFFFFu));
                                                                        count[layer] = 0;
                                                                     }
                                                                     )";
                const std::string skyboxVertexShader = R"(#version 460
                                                          #extension GL_EXT_multiview : enable
                                                          #extension GL_EXT_debug_printf : enable
                                                          layout (location = 0) in vec3 aPos;
                                                          layout (location = 1) in vec4 color;
                                                          layout (location = 2) in vec2 texCoords;
                                                          layout (location = 3) in vec3 normals;

                                                          layout(location = 0) out vec4 frontColor;
                                                          layout(location = 1) out vec3 fTexCoords;
                                                          layout(location = 2) out vec3 normal;
                                                          struct MatricesDatas {
                                                              mat4 projectionMatrix;
                                                              mat4 viewMatrix;
                                                          };
                                                          layout (set = 0, binding = 0) uniform UniformBufferObject {
                                                             MatricesDatas datas[6];
                                                          };
                                                          void main()
                                                          {
                                                              gl_PointSize = 2.0f;
                                                              fTexCoords = aPos;
                                                              frontColor = color;
                                                              normal = normals;
                                                              gl_Position = vec4(aPos, 1.0) * mat4(mat3(datas[gl_ViewIndex].viewMatrix)) * datas[gl_ViewIndex].projectionMatrix;
                                                          }
                                                          )";
                const std::string indirectRenderingVertexShader = R"(#version 460
                                                                     #extension GL_EXT_debug_printf : enable
                                                                     layout (location = 0) in vec3 position;
                                                                     layout (location = 1) in vec4 color;
                                                                     layout (location = 2) in vec2 texCoords;
                                                                     layout (location = 3) in vec3 normals;
                                                                     layout (push_constant)uniform PushConsts {
                                                                         mat4 projectionMatrix;
                                                                         layout (offset = 64) mat4 viewMatrix;
                                                                     } pushConsts;
                                                                     struct ModelData {
                                                                         mat4 modelMatrix;
                                                                     };
                                                                     struct MaterialData {
                                                                         vec2 uvScale;
                                                                         vec2 uvOffset;
                                                                         uint textureIndex;
                                                                         uint layer;
                                                                         uint materialType;
                                                                         uint padding;
                                                                     };
                                                                     layout(set = 0, binding = 3) buffer modelData {
                                                                         ModelData modelDatas[];
                                                                     };
                                                                     layout(set = 0, binding = 4) buffer materialData {
                                                                         MaterialData materialDatas[];
                                                                     };
                                                                     layout (location = 0) out vec2 fTexCoords;
                                                                     layout (location = 1) out vec4 frontColor;
                                                                     layout (location = 2) out uint texIndex;
                                                                     layout (location = 3) out uint layer;
                                                                     layout (location = 4) out vec3 normal;
                                                                     void main() {
                                                                         gl_PointSize = 2.0f;
                                                                         MaterialData material = materialDatas[gl_DrawID];
                                                                         ModelData modelData = modelDatas[gl_InstanceIndex];
                                                                         uint textureIndex = material.textureIndex;
                                                                         uint l = material.layer;

                                                                         gl_Position =  vec4(position, 1.f) * modelData.modelMatrix * pushConsts.viewMatrix * pushConsts.projectionMatrix;
                                                                         /*debugPrintfEXT("view matrix r1 : %v4f", pushConsts.viewMatrix[0]);
                                                                         debugPrintfEXT("view matrix r2 : %v4f", pushConsts.viewMatrix[1]);
                                                                         debugPrintfEXT("view matrix r3 : %v4f", pushConsts.viewMatrix[2]);
                                                                         debugPrintfEXT("view matrix r4 : %v4f", pushConsts.viewMatrix[3]);
                                                                         debugPrintfEXT("vertex position : %v4f", gl_Position);*/

                                                                         fTexCoords = texCoords * material.uvScale + material.uvOffset;
                                                                         frontColor = color;
                                                                         texIndex = textureIndex;
                                                                         normal = normals;
                                                                         layer = l;
                                                                         gl_PointSize = 2.0f;
                                                                     }
                                                                     )";
                const std::string linkedListIndirectRenderingVertexShader = R"(#version 460
                                                               #extension GL_EXT_multiview : enable
                                                               #extension GL_EXT_debug_printf : enable
                                                               layout (location = 0) in vec3 position;
                                                               layout (location = 1) in vec4 color;
                                                               layout (location = 2) in vec2 texCoords;
                                                               layout (location = 3) in vec3 normals;
                                                               struct ModelData {
                                                                   mat4 modelMatrix;
                                                               };
                                                               struct MaterialData {
                                                                   vec2 uvScale;
                                                                   vec2 uvOffset;
                                                                   uint textureIndex;
                                                                   uint layer;
                                                                   uint materialType;
                                                                   uint padding;
                                                               };
                                                               layout(set = 0, binding = 3) buffer modelData {
                                                                   ModelData modelDatas[];
                                                               };
                                                               layout(set = 0, binding = 4) buffer materialData {
                                                                   MaterialData materialDatas[];
                                                               };
                                                               struct MatricesDatas {
                                                                  mat4 projectionMatrix;
                                                                  mat4 viewMatrix;
                                                               };
                                                               layout (set = 0, binding = 5) uniform UniformBufferObject {
                                                                  MatricesDatas datas[6];
                                                               };
                                                               layout (location = 0) out vec4 frontColor;
                                                               layout (location = 1) out vec2 fTexCoords;
                                                               layout (location = 2) out uint texIndex;
                                                               layout (location = 3) out vec3 normal;
                                                               void main() {
                                                                    gl_PointSize = 2.0f;
                                                                    MaterialData material = materialDatas[gl_DrawID];
                                                                    ModelData model = modelDatas[gl_InstanceIndex];

                                                                    uint textureIndex = material.textureIndex;

                                                                    gl_Position = vec4(position, 1.f) * model.modelMatrix * datas[gl_ViewIndex].viewMatrix * datas[gl_ViewIndex].projectionMatrix;
                                                                    fTexCoords = texCoords * material.uvScale + material.uvOffset;
                                                                    frontColor = color;
                                                                    texIndex = textureIndex;
                                                                    normal = normals;
                                                               }
                                                               )";
                const std::string  linkedListVertexShader2 = R"(#version 460
                                                                #extension GL_EXT_multiview : enable
                                                                #extension GL_EXT_debug_printf : enable
                                                                layout (location = 0) in vec3 position;
                                                                layout (location = 1) in vec4 color;
                                                                layout (location = 2) in vec2 texCoords;
                                                                layout (location = 3) in vec3 normals;
                                                                layout (push_constant)uniform PushConsts {
                                                                     mat4 projectionMatrix;
                                                                     mat4 viewMatrix;
                                                                     mat4 worldMat;
                                                                } pushConsts;
                                                                layout (location = 0) out vec4 frontColor;
                                                                layout (location = 1) out vec2 fTexCoords;
                                                                layout (location = 2) out vec3 normal;
                                                                void main () {
                                                                     gl_Position = vec4(position, 1.f) * pushConsts.worldMat * pushConsts.viewMatrix * pushConsts.projectionMatrix;
                                                                     gl_PointSize = 2.0f;
                                                                     frontColor = color;
                                                                     fTexCoords = texCoords;
                                                                     normal = normals;
                                                                     //debugPrintfEXT("view index : %i\n", gl_ViewIndex);
                                                                })";
                const std::string perPixReflectRefractIndirectRenderingVertexShader = R"(#version 460
                                                                                         #extension GL_EXT_debug_printf : enable
                                                                                         layout (location = 0) in vec3 position;
                                                                                         layout (location = 1) in vec4 color;
                                                                                         layout (location = 2) in vec2 texCoords;
                                                                                         layout (location = 3) in vec3 normals;
                                                                                         layout (push_constant)uniform PushConsts {
                                                                                             mat4 projectionMatrix;
                                                                                             layout (offset = 64) mat4 viewMatrix;
                                                                                         } pushConsts;
                                                                                         struct ModelData {
                                                                                             mat4 modelMatrix;
                                                                                         };
                                                                                         struct MaterialData {
                                                                                             vec2 uvScale;
                                                                                             vec2 uvOffset;
                                                                                             uint textureIndex;
                                                                                             uint layer;
                                                                                             uint materialType;
                                                                                             uint padding;
                                                                                         };
                                                                                         layout(set = 0, binding = 3) buffer modelData {
                                                                                             ModelData modelDatas[];
                                                                                         };
                                                                                         layout(set = 0, binding = 4) buffer materialData {
                                                                                             MaterialData materialDatas[];
                                                                                         };
                                                                                         layout (location = 0) out vec3 pos;
                                                                                         layout (location = 1) out vec4 frontColor;
                                                                                         layout (location = 2) out vec2 texCoord;
                                                                                         layout (location = 3) out uint materialType;
                                                                                         layout (location = 4) out vec3 normal;
                                                                                         void main () {
                                                                                             gl_PointSize = 2.0f;
                                                                                             MaterialData material = materialDatas[gl_DrawID];
                                                                                             ModelData model = modelDatas[gl_InstanceIndex];
                                                                                             uint materialT = material.materialType;
                                                                                             pos = vec3(vec4(position, 1.0) * model.modelMatrix);

                                                                                             gl_Position = vec4(position, 1.f) * model.modelMatrix * pushConsts.viewMatrix * pushConsts.projectionMatrix;
                                                                                             frontColor = color;
                                                                                             texCoord = texCoords * material.uvScale + material.uvOffset;
                                                                                             normal = mat3(transpose(inverse(model.modelMatrix))) * normals;
                                                                                             materialType = materialT;
                                                                                             //debugPrintfEXT("vertex position : %v4f", gl_Position);
                                                                                         }
                                                                                         )";
                    const std::string skyboxFragmentShader = R"(#version 460
                                                                #extension GL_EXT_debug_printf : enable
                                                            layout (location = 0) out vec4 fcolor;
                                                            layout(location = 0) in vec4 frontColor;
                                                            layout(location = 1) in vec3 fTexCoords;
                                                            layout(location = 2) in vec3 normal;
                                                            layout (binding = 1) uniform samplerCube skybox;
                                                            void main() {
                                                                fcolor = texture(skybox, fTexCoords);
                                                            }
                                                            )";
                    const std::string buildDepthBufferFragmentShader = R"(#version 460
                                                                          #extension GL_ARB_fragment_shader_interlock : require
                                                                          #extension GL_EXT_nonuniform_qualifier : enable
                                                                          #extension GL_EXT_debug_printf : enable
                                                                          layout (location = 0) in vec2 fTexCoords;
                                                                          layout (location = 1) in vec4 frontColor;
                                                                          layout (location = 2) in flat uint texIndex;
                                                                          layout (location = 3) in flat uint layer;
                                                                          layout (location = 4) in vec3 normal;
                                                                          layout (push_constant) uniform PushConsts {
                                                                             layout (offset = 128) uint nbLayers;
                                                                          } pushConsts;
                                                                          layout(set = 0, binding = 5) uniform sampler2D textures[];
                                                                          layout(set = 0, binding = 1, rgba32f) uniform coherent image2D depthBuffer;
                                                                          layout(location = 0) out vec4 fColor;

                                                                          void main () {
                                                                              vec4 texel = (texIndex != 0) ? frontColor * texture(textures[texIndex-1], fTexCoords.xy) : frontColor;
                                                                              float z = gl_FragCoord.z;
                                                                              float l = float(layer) / float(pushConsts.nbLayers);
                                                                              beginInvocationInterlockARB();
                                                                              memoryBarrier();
                                                                              //debugPrintfEXT("indirect depth fragment shader");
                                                                              vec4 depth = imageLoad(depthBuffer,ivec2(gl_FragCoord.xy));
                                                                              if (l > depth.y || l == depth.y && z > depth.z) {
                                                                                imageStore(depthBuffer,ivec2(gl_FragCoord.xy),vec4(0,l,z,texel.a));
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
                                                                      layout(set = 0, binding = 5) uniform sampler2D textures[];
                                                                      layout(set = 0, binding = 1, rgba32f) uniform coherent image2D alphaBuffer;
                                                                      layout (location = 0) out vec4 fColor;
                                                                      layout (set = 0, binding = 2) uniform sampler2D depthBuffer[];
                                                                      layout (push_constant) uniform PushConsts {
                                                                            layout (offset = 128) vec4 resolution;
                                                                            layout (offset = 144) uint nbLayers;
                                                                            layout (offset = 148) uint imageIndex;
                                                                      } pushConsts;
                                                                      layout (location = 0) in vec2 fTexCoords;
                                                                      layout (location = 1) in vec4 frontColor;
                                                                      layout (location = 2) in flat uint texIndex;
                                                                      layout (location = 3) in flat uint layer;
                                                                      layout (location = 4) in vec3 normal;
                                                                      void main() {
                                                                          vec4 texel = (texIndex != 0) ? frontColor * texture(textures[texIndex-1], fTexCoords.xy) : frontColor;
                                                                          float current_alpha = texel.a;
                                                                          vec2 position = (gl_FragCoord.xy / pushConsts.resolution.xy);
                                                                          vec4 depth = texture (depthBuffer[pushConsts.imageIndex], position);
                                                                          beginInvocationInterlockARB();
                                                                          memoryBarrier();
                                                                          //debugPrintfEXT("indirect alpha fragment shader");
                                                                          vec4 alpha = imageLoad(alphaBuffer,ivec2(gl_FragCoord.xy));
                                                                          float l = float(layer) / float(pushConsts.nbLayers);
                                                                          float z = gl_FragCoord.z;
                                                                          if ((l > depth.y || l == depth.y && z > depth.z) && current_alpha > alpha.a) {
                                                                              imageStore(alphaBuffer,ivec2(gl_FragCoord.xy),vec4(0, l, z, current_alpha));
                                                                              fColor = vec4(0, l, z, current_alpha);
                                                                          } else {
                                                                              fColor = alpha;
                                                                          }
                                                                          endInvocationInterlockARB();
                                                                      }
                                                                      )";
                const std::string buildFramebufferShader = R"(#version 460
                                                              #extension GL_ARB_fragment_shader_interlock : require
                                                              #extension GL_EXT_nonuniform_qualifier : enable
                                                              #extension GL_EXT_debug_printf : enable
                                                                layout (location = 0) in vec3 pos;
                                                                layout (location = 1) in vec4 frontColor;
                                                                layout (location = 2) in vec2 texCoord;
                                                                layout (location = 3) in flat uint materialType;
                                                                layout (location = 4) in vec3 normal;
                                                                layout (push_constant) uniform PushConsts {
                                                                    layout(offset = 128) vec4 cameraPos;
                                                                    layout(offset = 144) vec4 resolution;
                                                                    layout(offset = 160) uint imageIndex;
                                                                } pushConsts;
                                                                layout (set = 0, binding = 0) uniform samplerCube sceneBox[];
                                                                layout (set = 0, binding = 1) uniform sampler2D alphaBuffer[];
                                                                layout (location = 0) out vec4 fColor;
                                                                void main () {

                                                                    vec2 position = (gl_FragCoord.xy / pushConsts.resolution.xy);
                                                                    vec4 alpha = texture(alphaBuffer[pushConsts.imageIndex], position);

                                                                    bool refr = false;
                                                                    float ratio = 1;
                                                                    if (materialType == 1) {
                                                                        ratio = 1.00 / 1.33;
                                                                        refr = true;
                                                                    } else if (materialType == 2) {
                                                                        ratio = 1.00 / 1.309;
                                                                        refr = true;
                                                                    } else if (materialType == 3) {
                                                                        ratio = 1.00 / 1.52;
                                                                        refr = true;
                                                                    } else if (materialType == 4) {
                                                                        ratio = 1.00 / 2.42;
                                                                        refr = true;
                                                                    }
                                                                    vec3 i = (vec4(pos.xyz, 1) - pushConsts.cameraPos).xyz;
                                                                    if (refr) {
                                                                        //debugPrintfEXT("indirect alpha fragment shader");
                                                                        vec3 r = refract (i, normalize(normal), ratio);
                                                                        fColor = texture(sceneBox[pushConsts.imageIndex], r) * (1 - alpha.a);
                                                                    } else {
                                                                        vec3 r = reflect (i, normalize(normal));
                                                                        fColor = texture(sceneBox[pushConsts.imageIndex], r) * (1 - alpha.a);
                                                                    }
                                                                }
                                                              )";
                const std::string fragmentShader = R"(#version 460
                                                      #extension GL_EXT_nonuniform_qualifier : enable
                                                      #extension GL_EXT_multiview : enable
                                                      #extension GL_EXT_debug_printf : enable
                                                      struct NodeType {
                                                          vec4 color;
                                                          float depth;
                                                          uint next;
                                                      };
                                                      layout(set = 0, binding = 0) buffer CounterSSBO {
                                                          uint count[6];
                                                          uint maxNodes;
                                                      };
                                                      layout(set = 0, binding = 1, r32ui) uniform coherent uimage2DArray headPointers;
                                                      layout(set = 0, binding = 2) buffer linkedLists {
                                                          NodeType nodes[];
                                                      };
                                                      layout(set = 0, binding = 6) uniform sampler2D textures[];
                                                      layout (location = 0) in vec4 frontColor;
                                                      layout (location = 1) in vec2 fTexCoords;
                                                      layout (location = 2) in flat uint texIndex;
                                                      layout (location = 3) in vec3 normal;
                                                      layout(location = 0) out vec4 fcolor;
                                                      void main() {
                                                           uint nodeIdx = atomicAdd(count[gl_ViewIndex], 1);
                                                           vec4 texel = (texIndex != 0) ? frontColor * texture(textures[texIndex-1], fTexCoords.xy) : frontColor;
                                                           if (nodeIdx < maxNodes) {
                                                                uint prevHead = imageAtomicExchange(headPointers, ivec3(gl_FragCoord.xy, gl_ViewIndex), nodeIdx);
                                                                nodes[nodeIdx+gl_ViewIndex*maxNodes].color = texel;
                                                                nodes[nodeIdx+gl_ViewIndex*maxNodes].depth = gl_FragCoord.z;
                                                                nodes[nodeIdx+gl_ViewIndex*maxNodes].next = prevHead;
                                                           }
                                                           fcolor = vec4(0, 0, 0, 0);
                                                      })";
                 const std::string fragmentShader2 =
                   R"(
                   #version 460
                   #define MAX_FRAGMENTS 20
                   #extension GL_EXT_debug_printf : enable
                   #extension GL_EXT_multiview : enable
                   struct NodeType {
                      vec4 color;
                      float depth;
                      uint next;
                   };
                   layout(set = 0, binding = 0) buffer CounterSSBO {
                      uint count[6];
                      uint maxNodes;
                   };
                   layout(set = 0, binding = 1, r32ui) uniform uimage2DArray headPointers;
                   layout(set = 0, binding = 2) buffer linkedLists {
                       NodeType nodes[];
                   };
                   layout (location = 0) out vec4 fcolor;
                   layout (location = 0) in vec4 frontColor;
                   layout (location = 1) in vec2 fTexCoords;
                   layout (location = 2) in vec3 normal;
                   void main() {
                      NodeType frags[MAX_FRAGMENTS];
                      int count = 0;
                      uint n = imageLoad(headPointers, ivec3(gl_FragCoord.xy, gl_ViewIndex)).r;
                      while(n != 0xffffffffu && count < MAX_FRAGMENTS) {
                           frags[count] = nodes[n+maxNodes*gl_ViewIndex];
                           n = frags[count].next/*+maxNodes*viewIndex*/;
                           count++;
                      }
                      /*if (count > 0)
                        debugPrintfEXT("count : %i", count);*/
                       // Do the insertion sort
                      for (uint i = 1; i < count; ++i)
                      {
                          NodeType insert = frags[i];
                          uint j = i;
                          while (j > 0 && insert.depth < frags[j - 1].depth)
                          {
                              frags[j] = frags[j-1];
                              --j;
                          }
                          frags[j] = insert;
                      }
                      vec4 color = vec4(0, 0, 0, 0);
                      for( int i = 0; i < count; i++)
                      {
                        color.rgb = frags[i].color.rgb * frags[i].color.a + color.rgb * (1 - frags[i].color.a);
                        color.a = frags[i].color.a + color.a * (1 - frags[i].color.a);
                      }
                      /*if (color.r != 0 || color.g != 0 || color.b != 0 || color.a != 0)
                        debugPrintfEXT("count : %v4f\n", color);*/
                      //debugPrintfEXT("linked list p2");
                      fcolor = color;
                   })";
                    if (!skyboxShader.loadFromMemory(skyboxVertexShader, skyboxFragmentShader)) {
                        throw core::Erreur(51, "Error, failed to load build skybox shader", 3);
                    }
                    if (!sBuildDepthBuffer.loadFromMemory(indirectRenderingVertexShader, buildDepthBufferFragmentShader)) {
                        throw core::Erreur(50, "Error, failed to load build depth buffer shader", 3);
                    }
                    if (!sReflectRefract.loadFromMemory(perPixReflectRefractIndirectRenderingVertexShader, buildFramebufferShader)) {
                        throw core::Erreur(57, "Error, failed to load reflect refract shader", 3);
                    }
                    if (!sLinkedList.loadFromMemory(linkedListIndirectRenderingVertexShader, fragmentShader)) {
                        throw core::Erreur(58, "Error, failed to load per pixel linked list shader", 3);
                    }
                    if (!sLinkedList2.loadFromMemory(linkedListVertexShader2, fragmentShader2)) {
                        throw core::Erreur(59, "Error, failed to load per pixel linked list 2 shader", 3);
                    }
                    if (!sBuildAlphaBuffer.loadFromMemory(indirectRenderingVertexShader,buildAlphaBufferFragmentShader)) {
                        throw core::Erreur(60, "Error, failed to load build alpha buffer shader", 3);
                    }
                    if (!clearHeadptrComputeShader.loadFromMemory(clearHeadptrComputeShaderCode)) {
                        throw core::Erreur(50, "Error, failed to load clear headptr shader", 3);
                    }
                    math::Matrix4f viewMatrix = getWindow().getDefaultView().getViewMatrix().getMatrix()/*.transpose()*/;
                    math::Matrix4f projMatrix = getWindow().getDefaultView().getProjMatrix().getMatrix()/*.transpose()*/;
                    linkedList2PC.viewMatrix = toVulkanMatrix(viewMatrix);
                    linkedList2PC.projMatrix = toVulkanMatrix(projMatrix);
            }
            void ReflectRefractRenderComponent::createImageView() {
                for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    VkImageViewCreateInfo viewInfo{};
                    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                    viewInfo.image = headPtrTextureImage[i];
                    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                    viewInfo.format = VK_FORMAT_R32_UINT;
                    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    viewInfo.subresourceRange.baseMipLevel = 0;
                    viewInfo.subresourceRange.levelCount = 1;
                    viewInfo.subresourceRange.baseArrayLayer = 0;
                    viewInfo.subresourceRange.layerCount = 6;

                    if (vkCreateImageView(vkDevice.getDevice(), &viewInfo, nullptr, &headPtrTextureImageView[i]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create head ptr texture image view!");
                    }
                }
                for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    VkImageViewCreateInfo viewInfo2{};
                    viewInfo2.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                    viewInfo2.image = depthTextureImage[i];
                    viewInfo2.viewType = VK_IMAGE_VIEW_TYPE_2D;
                    viewInfo2.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                    viewInfo2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    viewInfo2.subresourceRange.baseMipLevel = 0;
                    viewInfo2.subresourceRange.levelCount = 1;
                    viewInfo2.subresourceRange.baseArrayLayer = 0;
                    viewInfo2.subresourceRange.layerCount = 1;

                    if (vkCreateImageView(vkDevice.getDevice(), &viewInfo2, nullptr, &depthTextureImageView[i]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create head ptr texture image view!");
                    }
                }
                for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    VkImageViewCreateInfo viewInfo3{};
                    viewInfo3.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                    viewInfo3.image = alphaTextureImage[i];
                    viewInfo3.viewType = VK_IMAGE_VIEW_TYPE_2D;
                    viewInfo3.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                    viewInfo3.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    viewInfo3.subresourceRange.baseMipLevel = 0;
                    viewInfo3.subresourceRange.levelCount = 1;
                    viewInfo3.subresourceRange.baseArrayLayer = 0;
                    viewInfo3.subresourceRange.layerCount = 1;

                    if (vkCreateImageView(vkDevice.getDevice(), &viewInfo3, nullptr, &alphaTextureImageView[i]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create head ptr texture image view!");
                    }
                }
            }
            void ReflectRefractRenderComponent::createSampler() {
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
                    if (vkCreateSampler(vkDevice.getDevice(), &samplerInfo, nullptr, &headPtrTextureSampler[i]) != VK_SUCCESS) {
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
                    if (vkCreateSampler(vkDevice.getDevice(), &samplerInfo2, nullptr, &depthTextureSampler[i]) != VK_SUCCESS) {
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
                    if (vkCreateSampler(vkDevice.getDevice(), &samplerInfo3, nullptr, &alphaTextureSampler[i]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create texture sampler!");
                    }
                }
            }
            void ReflectRefractRenderComponent::createDescriptorPool(unsigned int p, RenderStates states) {
                Shader* shader = const_cast<Shader*>(states.shader);
                if (shader == &skyboxShader) {
                    std::vector<VkDescriptorPool>& descriptorPool = environmentMap.getDescriptorPool();
                    unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                    if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                    std::array<VkDescriptorPoolSize, 2> poolSizes;
                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());

                    if (descriptorPool[descriptorId] != nullptr) {
                        vkDestroyDescriptorPool(vkDevice.getDevice(), descriptorPool[descriptorId], nullptr);
                    }

                    VkDescriptorPoolCreateInfo poolInfo{};
                    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
                    poolInfo.pPoolSizes = poolSizes.data();
                    poolInfo.maxSets = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    ////std::cout<<"descriptor id : "<<descriptorId<<std::endl;
                    if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de la creation de la pool de descripteurs!");
                    }

                } else if (shader == &sLinkedList) {
                    std::vector<VkDescriptorPool>& descriptorPool = environmentMap.getDescriptorPool();
                    unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                    if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                    ////////std::cout<<"ppll descriptor id : "<<environmentMap.getId()<<","<<shader->getId()<<","<<environmentMap.getId() * shader->getNbShaders() + shader->getId()<<std::endl;
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    std::array<VkDescriptorPoolSize, 7> poolSizes;
                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[2].descriptorCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    poolSizes[3].descriptorCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    poolSizes[4].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    poolSizes[4].descriptorCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    poolSizes[5].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                    poolSizes[5].descriptorCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    poolSizes[6].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[6].descriptorCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight() * allTextures.size());

                    if (descriptorPool[descriptorId] != nullptr) {
                        vkDestroyDescriptorPool(vkDevice.getDevice(), descriptorPool[descriptorId], nullptr);
                    }

                    VkDescriptorPoolCreateInfo poolInfo{};
                    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
                    poolInfo.pPoolSizes = poolSizes.data();
                    poolInfo.maxSets = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    ////std::cout<<"descriptor id : "<<descriptorId<<std::endl;
                    if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de la creation de la pool de descripteurs!");
                    }
                } else if (shader == &sLinkedList2) {
                    std::vector<VkDescriptorPool>& descriptorPool = environmentMap.getDescriptorPool();
                    unsigned int descriptorId = shader->getId();
                    if (shader->getNbShaders() > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders());
                    std::array<VkDescriptorPoolSize, 3> poolSizes;
                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[2].descriptorCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());

                    if (descriptorPool[descriptorId] != nullptr) {
                        vkDestroyDescriptorPool(vkDevice.getDevice(), descriptorPool[descriptorId], nullptr);
                    }

                    VkDescriptorPoolCreateInfo poolInfo{};
                    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
                    poolInfo.pPoolSizes = poolSizes.data();
                    poolInfo.maxSets = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de la creation de la pool de descripteurs!");
                    }
                } else if (shader == &sBuildDepthBuffer) {
                    std::vector<VkDescriptorPool>& descriptorPool = depthBuffer.getDescriptorPool();
                    unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                    if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                    std::array<VkDescriptorPoolSize, 4> poolSizes;
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());
                    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    poolSizes[2].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());
                    poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[3].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight() * allTextures.size());

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
                } else if (shader == &sBuildAlphaBuffer) {
                    std::vector<VkDescriptorPool>& descriptorPool = alphaBuffer.getDescriptorPool();
                    unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                    if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                    std::array<VkDescriptorPoolSize, 5> poolSizes;
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight()) * depthBuffer.getSwapchainImagesSize();
                    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    poolSizes[2].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());
                    poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    poolSizes[3].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());
                    poolSizes[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[4].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight() * allTextures.size());

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
                } else {
                    std::vector<VkDescriptorPool>& descriptorPool = reflectRefractTex.getDescriptorPool();
                    unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                    if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                    std::array<VkDescriptorPoolSize, 4> poolSizes;
                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(reflectRefractTex.getMaxFramesInFlight()) * reflectRefractTex.getSwapchainImagesSize();
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(reflectRefractTex.getMaxFramesInFlight()) * reflectRefractTex.getSwapchainImagesSize();
                    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    poolSizes[2].descriptorCount = static_cast<uint32_t>(reflectRefractTex.getMaxFramesInFlight());
                    poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    poolSizes[3].descriptorCount = static_cast<uint32_t>(reflectRefractTex.getMaxFramesInFlight());

                    if (descriptorPool[descriptorId] != nullptr) {
                        vkDestroyDescriptorPool(vkDevice.getDevice(), descriptorPool[descriptorId], nullptr);
                    }

                    VkDescriptorPoolCreateInfo poolInfo{};
                    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
                    poolInfo.pPoolSizes = poolSizes.data();
                    poolInfo.maxSets = static_cast<uint32_t>(reflectRefractTex.getMaxFramesInFlight());
                    if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de la creation de la pool de descripteurs!");
                    }
                }
            }
            void ReflectRefractRenderComponent::createDescriptorPool(RenderStates states) {
                Shader* shader = const_cast<Shader*>(states.shader);
                if (shader == &sLinkedList) {
                    std::vector<VkDescriptorPool>& descriptorPool = environmentMap.getDescriptorPool();
                    unsigned int descriptorId = shader->getId();
                    if (shader->getNbShaders() > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders());
                    ////////std::cout<<"ppll descriptor id : "<<environmentMap.getId()<<","<<shader->getId()<<","<<environmentMap.getId() * shader->getNbShaders() + shader->getId()<<std::endl;
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    std::array<VkDescriptorPoolSize, 7> poolSizes;
                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[2].descriptorCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[3].descriptorCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    poolSizes[4].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[4].descriptorCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    poolSizes[5].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    poolSizes[5].descriptorCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    poolSizes[6].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[6].descriptorCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight() * allTextures.size());

                    if (descriptorPool[descriptorId] != nullptr) {
                        vkDestroyDescriptorPool(vkDevice.getDevice(), descriptorPool[descriptorId], nullptr);
                    }

                    VkDescriptorPoolCreateInfo poolInfo{};
                    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
                    poolInfo.pPoolSizes = poolSizes.data();
                    poolInfo.maxSets = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de la creation de la pool de descripteurs!");
                    }
                } else if (shader == &sLinkedList2) {
                    std::vector<VkDescriptorPool>& descriptorPool = environmentMap.getDescriptorPool();
                    if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders());
                    unsigned int descriptorId = shader->getId();
                    std::array<VkDescriptorPoolSize, 3> poolSizes;
                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[2].descriptorCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());

                    if (descriptorPool[descriptorId] != nullptr) {
                        vkDestroyDescriptorPool(vkDevice.getDevice(), descriptorPool[descriptorId], nullptr);
                    }

                    VkDescriptorPoolCreateInfo poolInfo{};
                    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
                    poolInfo.pPoolSizes = poolSizes.data();
                    poolInfo.maxSets = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de la creation de la pool de descripteurs!");
                    }
                } else if (shader == &sBuildDepthBuffer) {
                    std::vector<VkDescriptorPool>& descriptorPool = depthBuffer.getDescriptorPool();
                    if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders()*depthBuffer.getNbRenderTargets());
                    unsigned int descriptorId = shader->getId();
                    std::array<VkDescriptorPoolSize, 4> poolSizes;
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());
                    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[2].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());
                    poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[3].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight() * allTextures.size());

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
                } else if (shader == &sBuildAlphaBuffer) {
                    std::vector<VkDescriptorPool>& descriptorPool = alphaBuffer.getDescriptorPool();
                    if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders()*alphaBuffer.getNbRenderTargets());
                    unsigned int descriptorId = shader->getId();
                    std::array<VkDescriptorPoolSize, 5> poolSizes;
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());
                    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[2].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());
                    poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[3].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());
                    poolSizes[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[4].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight() * allTextures.size());

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
                } else {
                    std::vector<VkDescriptorPool>& descriptorPool = reflectRefractTex.getDescriptorPool();
                    if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders()*reflectRefractTex.getNbRenderTargets());
                    unsigned int descriptorId = shader->getId();
                    std::array<VkDescriptorPoolSize, 4> poolSizes;
                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(reflectRefractTex.getMaxFramesInFlight());
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(reflectRefractTex.getMaxFramesInFlight());
                    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[2].descriptorCount = static_cast<uint32_t>(reflectRefractTex.getMaxFramesInFlight());
                    poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[3].descriptorCount = static_cast<uint32_t>(reflectRefractTex.getMaxFramesInFlight());

                    if (descriptorPool[descriptorId] != nullptr) {
                        vkDestroyDescriptorPool(vkDevice.getDevice(), descriptorPool[descriptorId], nullptr);
                    }

                    VkDescriptorPoolCreateInfo poolInfo{};
                    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
                    poolInfo.pPoolSizes = poolSizes.data();
                    poolInfo.maxSets = static_cast<uint32_t>(reflectRefractTex.getMaxFramesInFlight());
                    if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de la creation de la pool de descripteurs!");
                    }
                }
            }

            void ReflectRefractRenderComponent::createDescriptorSetLayout(RenderStates states) {
                Shader* shader = const_cast<Shader*>(states.shader);
                if (shader == &skyboxShader) {
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = environmentMap.getDescriptorSetLayout();
                    if (shader->getNbShaders() > descriptorSetLayout.size())
                        descriptorSetLayout.resize(shader->getNbShaders());
                    unsigned int descriptorId = shader->getId();
                    VkDescriptorSetLayoutBinding uniformBufferLayoutBinding;
                    uniformBufferLayoutBinding.binding = 0;
                    uniformBufferLayoutBinding.descriptorCount = 1;
                    uniformBufferLayoutBinding.descriptorType = (useThread) ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    uniformBufferLayoutBinding.pImmutableSamplers = nullptr;
                    uniformBufferLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
                    samplerLayoutBinding.binding = 1;
                    samplerLayoutBinding.descriptorCount = 1;
                    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding.pImmutableSamplers = nullptr;
                    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    if (descriptorSetLayout[descriptorId] != nullptr) {
                        vkDestroyDescriptorSetLayout(vkDevice.getDevice(), descriptorSetLayout[descriptorId], nullptr);
                    }

                    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uniformBufferLayoutBinding, samplerLayoutBinding};
                    VkDescriptorSetLayoutCreateInfo layoutInfo{};
                    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                    //layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
                    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());;
                    layoutInfo.pBindings = bindings.data();

                    if (vkCreateDescriptorSetLayout(vkDevice.getDevice(), &layoutInfo, nullptr, &descriptorSetLayout[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create descriptor set layout!");
                    }
                } else if (shader == &sLinkedList) {
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = environmentMap.getDescriptorSetLayout();
                    if (shader->getNbShaders() > descriptorSetLayout.size())
                        descriptorSetLayout.resize(shader->getNbShaders());
                    unsigned int descriptorId = shader->getId();
                    ////////std::cout<<"ppll descriptor id : "<<descriptorId<<std::endl;
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    std::vector<VkDescriptorBindingFlags> bindingFlags(7, 0); // 6 bindings, flags par défaut à 0
                    bindingFlags[6] = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
                    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
                    bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
                    bindingFlagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
                    bindingFlagsInfo.pBindingFlags = bindingFlags.data();

                    VkDescriptorSetLayoutBinding counterLayoutBinding{};
                    counterLayoutBinding.binding = 0;
                    counterLayoutBinding.descriptorCount = 1;
                    counterLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    counterLayoutBinding.pImmutableSamplers = nullptr;
                    counterLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding headPtrImageLayoutBinding;
                    headPtrImageLayoutBinding.binding = 1;
                    headPtrImageLayoutBinding.descriptorCount = 1;
                    headPtrImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    headPtrImageLayoutBinding.pImmutableSamplers = nullptr;
                    headPtrImageLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding linkedListLayoutBinding{};
                    linkedListLayoutBinding.binding = 2;
                    linkedListLayoutBinding.descriptorCount = 1;
                    linkedListLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    linkedListLayoutBinding.pImmutableSamplers = nullptr;
                    linkedListLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


                    VkDescriptorSetLayoutBinding modelDataLayoutBinding{};
                    modelDataLayoutBinding.binding = 3;
                    modelDataLayoutBinding.descriptorCount = 1;
                    modelDataLayoutBinding.descriptorType = (useThread) ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    modelDataLayoutBinding.pImmutableSamplers = nullptr;
                    modelDataLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                    VkDescriptorSetLayoutBinding materialDataLayoutBinding{};
                    materialDataLayoutBinding.binding = 4;
                    materialDataLayoutBinding.descriptorCount = 1;
                    materialDataLayoutBinding.descriptorType = (useThread) ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    materialDataLayoutBinding.pImmutableSamplers = nullptr;
                    materialDataLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                    VkDescriptorSetLayoutBinding uniformBufferLayoutBinding;
                    uniformBufferLayoutBinding.binding = 5;
                    uniformBufferLayoutBinding.descriptorCount = 1;
                    uniformBufferLayoutBinding.descriptorType = (useThread) ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    uniformBufferLayoutBinding.pImmutableSamplers = nullptr;
                    uniformBufferLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
                    samplerLayoutBinding.binding = 6;
                    samplerLayoutBinding.descriptorCount = allTextures.size();
                    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding.pImmutableSamplers = nullptr;
                    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    if (descriptorSetLayout[descriptorId] != nullptr) {
                        vkDestroyDescriptorSetLayout(vkDevice.getDevice(), descriptorSetLayout[descriptorId], nullptr);
                    }

                    std::array<VkDescriptorSetLayoutBinding, 7> bindings = {counterLayoutBinding, headPtrImageLayoutBinding, linkedListLayoutBinding, modelDataLayoutBinding, materialDataLayoutBinding, uniformBufferLayoutBinding, samplerLayoutBinding};
                    VkDescriptorSetLayoutCreateInfo layoutInfo{};
                    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                    layoutInfo.pNext = &bindingFlagsInfo;
                    //layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
                    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());;
                    layoutInfo.pBindings = bindings.data();

                    if (vkCreateDescriptorSetLayout(vkDevice.getDevice(), &layoutInfo, nullptr, &descriptorSetLayout[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create descriptor set layout!");
                    }
                } else if (shader == &sLinkedList2) {
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = environmentMap.getDescriptorSetLayout();
                    if (shader->getNbShaders() > descriptorSetLayout.size())
                        descriptorSetLayout.resize(shader->getNbShaders());
                    unsigned int descriptorId = shader->getId();
                    ////////std::cout<<"ppll descriptor id : "<<descriptorId<<std::endl;
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    VkDescriptorSetLayoutBinding counterLayoutBinding{};
                    counterLayoutBinding.binding = 0;
                    counterLayoutBinding.descriptorCount = 1;
                    counterLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    counterLayoutBinding.pImmutableSamplers = nullptr;
                    counterLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding headPtrImageLayoutBinding;
                    headPtrImageLayoutBinding.binding = 1;
                    headPtrImageLayoutBinding.descriptorCount = 1;
                    headPtrImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    headPtrImageLayoutBinding.pImmutableSamplers = nullptr;
                    headPtrImageLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding linkedListLayoutBinding{};
                    linkedListLayoutBinding.binding = 2;
                    linkedListLayoutBinding.descriptorCount = 1;
                    linkedListLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    linkedListLayoutBinding.pImmutableSamplers = nullptr;
                    linkedListLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                    std::array<VkDescriptorSetLayoutBinding, 3> bindings = {counterLayoutBinding, headPtrImageLayoutBinding, linkedListLayoutBinding};

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
                } else if (shader == &sBuildDepthBuffer) {
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = depthBuffer.getDescriptorSetLayout();
                    if (shader->getNbShaders() > descriptorSetLayout.size())
                        descriptorSetLayout.resize(shader->getNbShaders());
                    unsigned int descriptorId = shader->getId();
                    ////////std::cout<<"ppll descriptor id : "<<descriptorId<<std::endl;
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    std::vector<VkDescriptorBindingFlags> bindingFlags(4, 0); // 6 bindings, flags par défaut à 0
                    bindingFlags[3] = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
                    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
                    bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
                    bindingFlagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
                    bindingFlagsInfo.pBindingFlags = bindingFlags.data();


                    VkDescriptorSetLayoutBinding headPtrImageLayoutBinding;
                    headPtrImageLayoutBinding.binding = 1;
                    headPtrImageLayoutBinding.descriptorCount = 1;
                    headPtrImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    headPtrImageLayoutBinding.pImmutableSamplers = nullptr;
                    headPtrImageLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding modelDataLayoutBinding{};
                    modelDataLayoutBinding.binding = 3;
                    modelDataLayoutBinding.descriptorCount = 1;
                    modelDataLayoutBinding.descriptorType = (useThread) ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    modelDataLayoutBinding.pImmutableSamplers = nullptr;
                    modelDataLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                    VkDescriptorSetLayoutBinding materialDataLayoutBinding{};
                    materialDataLayoutBinding.binding = 4;
                    materialDataLayoutBinding.descriptorCount = 1;
                    materialDataLayoutBinding.descriptorType = (useThread) ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    materialDataLayoutBinding.pImmutableSamplers = nullptr;
                    materialDataLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
                    samplerLayoutBinding.binding = 5;
                    samplerLayoutBinding.descriptorCount = allTextures.size();
                    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding.pImmutableSamplers = nullptr;
                    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    std::array<VkDescriptorSetLayoutBinding, 4> bindings = {headPtrImageLayoutBinding, modelDataLayoutBinding, materialDataLayoutBinding, samplerLayoutBinding};

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
                } else if (shader == &sBuildAlphaBuffer) {
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = alphaBuffer.getDescriptorSetLayout();
                    if (shader->getNbShaders() > descriptorSetLayout.size())
                        descriptorSetLayout.resize(shader->getNbShaders());
                    unsigned int descriptorId = shader->getId();
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    std::vector<VkDescriptorBindingFlags> bindingFlags(5, 0); // 6 bindings, flags par défaut à 0
                    bindingFlags[4] = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT; // seulement pour sampler[]
                    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
                    bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
                    bindingFlagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
                    bindingFlagsInfo.pBindingFlags = bindingFlags.data();


                    VkDescriptorSetLayoutBinding headPtrImageLayoutBinding;
                    headPtrImageLayoutBinding.binding = 1;
                    headPtrImageLayoutBinding.descriptorCount = 1;
                    headPtrImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    headPtrImageLayoutBinding.pImmutableSamplers = nullptr;
                    headPtrImageLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding sampler2LayoutBinding{};
                    sampler2LayoutBinding.binding = 2;
                    sampler2LayoutBinding.descriptorCount = depthBuffer.getSwapchainImagesSize();
                    sampler2LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    sampler2LayoutBinding.pImmutableSamplers = nullptr;
                    sampler2LayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding modelDataLayoutBinding{};
                    modelDataLayoutBinding.binding = 3;
                    modelDataLayoutBinding.descriptorCount = 1;
                    modelDataLayoutBinding.descriptorType = (useThread) ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    modelDataLayoutBinding.pImmutableSamplers = nullptr;
                    modelDataLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                    VkDescriptorSetLayoutBinding materialDataLayoutBinding{};
                    materialDataLayoutBinding.binding = 4;
                    materialDataLayoutBinding.descriptorCount = 1;
                    materialDataLayoutBinding.descriptorType = (useThread) ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    materialDataLayoutBinding.pImmutableSamplers = nullptr;
                    materialDataLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
                    samplerLayoutBinding.binding = 5;
                    samplerLayoutBinding.descriptorCount = allTextures.size();
                    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding.pImmutableSamplers = nullptr;
                    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    std::array<VkDescriptorSetLayoutBinding, 5> bindings = {headPtrImageLayoutBinding, sampler2LayoutBinding, modelDataLayoutBinding, materialDataLayoutBinding, samplerLayoutBinding};

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
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = reflectRefractTex.getDescriptorSetLayout();
                    if (shader->getNbShaders() > descriptorSetLayout.size())
                        descriptorSetLayout.resize(shader->getNbShaders());
                    unsigned int descriptorId = shader->getId();
                    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
                    samplerLayoutBinding.binding = 0;
                    samplerLayoutBinding.descriptorCount = alphaBuffer.getSwapchainImagesSize();
                    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding.pImmutableSamplers = nullptr;
                    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding sampler2LayoutBinding{};
                    sampler2LayoutBinding.binding = 1;
                    sampler2LayoutBinding.descriptorCount = environmentMap.getSwapchainImagesSize();
                    sampler2LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    sampler2LayoutBinding.pImmutableSamplers = nullptr;
                    sampler2LayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding modelDataLayoutBinding{};
                    modelDataLayoutBinding.binding = 3;
                    modelDataLayoutBinding.descriptorCount = 1;
                    modelDataLayoutBinding.descriptorType = (useThread) ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    modelDataLayoutBinding.pImmutableSamplers = nullptr;
                    modelDataLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                    VkDescriptorSetLayoutBinding materialDataLayoutBinding{};
                    materialDataLayoutBinding.binding = 4;
                    materialDataLayoutBinding.descriptorCount = 1;
                    materialDataLayoutBinding.descriptorType = (useThread) ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    materialDataLayoutBinding.pImmutableSamplers = nullptr;
                    materialDataLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                    std::array<VkDescriptorSetLayoutBinding, 4> bindings = {samplerLayoutBinding, sampler2LayoutBinding, modelDataLayoutBinding, materialDataLayoutBinding};

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
            void ReflectRefractRenderComponent::allocateDescriptorSets(unsigned int p, RenderStates states) {
                Shader* shader = const_cast<Shader*>(states.shader);
                if (shader == &skyboxShader) {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = environmentMap.getDescriptorSet();
                    std::vector<VkDescriptorPool>& descriptorPool = environmentMap.getDescriptorPool();
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = environmentMap.getDescriptorSetLayout();
                    if (shader->getNbShaders()  * (Batcher::nbPrimitiveTypes - 1) > descriptorSets.size())
                        descriptorSets.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                    unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                    for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                        descriptorSets[i].resize(environmentMap.getMaxFramesInFlight());
                    }
                    std::vector<VkDescriptorSetLayout> layouts(environmentMap.getMaxFramesInFlight(), descriptorSetLayout[shader->getId()]);
                    VkDescriptorSetAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    allocInfo.descriptorPool = descriptorPool[descriptorId];
                    allocInfo.descriptorSetCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    allocInfo.pSetLayouts = layouts.data();
                    ////std::cout<<"descriptor id : "<<descriptorId<<std::endl;
                    if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, descriptorSets[descriptorId].data()) != VK_SUCCESS) {
                        throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
                    }
                } else if (shader == &sLinkedList) {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = environmentMap.getDescriptorSet();
                    std::vector<VkDescriptorPool>& descriptorPool = environmentMap.getDescriptorPool();
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = environmentMap.getDescriptorSetLayout();
                    if (shader->getNbShaders()  * (Batcher::nbPrimitiveTypes - 1) > descriptorSets.size())
                        descriptorSets.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                    unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                    for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                        descriptorSets[i].resize(environmentMap.getMaxFramesInFlight());
                    }


                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    std::vector<uint32_t> variableCounts(environmentMap.getMaxFramesInFlight(), static_cast<uint32_t>(allTextures.size()));

                    VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
                    variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
                    variableCountInfo.descriptorSetCount = static_cast<uint32_t>(variableCounts.size());
                    variableCountInfo.pDescriptorCounts = variableCounts.data();
                    std::vector<VkDescriptorSetLayout> layouts(environmentMap.getMaxFramesInFlight(), descriptorSetLayout[shader->getId()]);
                    VkDescriptorSetAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    allocInfo.pNext = &variableCountInfo;
                    allocInfo.descriptorPool = descriptorPool[descriptorId];
                    allocInfo.descriptorSetCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    allocInfo.pSetLayouts = layouts.data();
                    ////std::cout<<"descriptor id : "<<descriptorId<<std::endl;
                    if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, descriptorSets[descriptorId].data()) != VK_SUCCESS) {
                        throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
                    }
                } else if (shader == &sLinkedList2) {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = environmentMap.getDescriptorSet();
                    std::vector<VkDescriptorPool>& descriptorPool = environmentMap.getDescriptorPool();
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = environmentMap.getDescriptorSetLayout();
                    if (shader->getNbShaders() > descriptorSets.size())
                        descriptorSets.resize(shader->getNbShaders());
                    unsigned int descriptorId = shader->getId();
                    for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                        descriptorSets[i].resize(environmentMap.getMaxFramesInFlight());
                    }
                    std::vector<VkDescriptorSetLayout> layouts(environmentMap.getMaxFramesInFlight(), descriptorSetLayout[shader->getId()]);
                    VkDescriptorSetAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    allocInfo.descriptorPool = descriptorPool[descriptorId];
                    allocInfo.descriptorSetCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    allocInfo.pSetLayouts = layouts.data();
                    if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, descriptorSets[descriptorId].data()) != VK_SUCCESS) {
                        throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
                    }

                } else if (shader == &sBuildDepthBuffer) {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = depthBuffer.getDescriptorSet();
                    std::vector<VkDescriptorPool>& descriptorPool = depthBuffer.getDescriptorPool();
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = depthBuffer.getDescriptorSetLayout();
                    if (shader->getNbShaders()  * (Batcher::nbPrimitiveTypes - 1) > descriptorSets.size())
                        descriptorSets.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                    unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                    for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                        descriptorSets[i].resize(depthBuffer.getMaxFramesInFlight());
                    }
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    std::vector<uint32_t> variableCounts(depthBuffer.getMaxFramesInFlight(), static_cast<uint32_t>(allTextures.size()));

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
                } else if (shader == &sBuildAlphaBuffer) {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = alphaBuffer.getDescriptorSet();
                    std::vector<VkDescriptorPool>& descriptorPool = alphaBuffer.getDescriptorPool();
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = alphaBuffer.getDescriptorSetLayout();
                    if (shader->getNbShaders()  * (Batcher::nbPrimitiveTypes - 1) > descriptorSets.size())
                        descriptorSets.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                    unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                    for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                        descriptorSets[i].resize(alphaBuffer.getMaxFramesInFlight());
                    }
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    std::vector<uint32_t> variableCounts(alphaBuffer.getMaxFramesInFlight(), static_cast<uint32_t>(allTextures.size()));

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
                } else {
                    std::vector<VkDescriptorPool>& descriptorPool = reflectRefractTex.getDescriptorPool();
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = reflectRefractTex.getDescriptorSetLayout();
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = reflectRefractTex.getDescriptorSet();
                    if (shader->getNbShaders()  * (Batcher::nbPrimitiveTypes - 1) > descriptorSets.size())
                        descriptorSets.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                    unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                    for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                        descriptorSets[i].resize(reflectRefractTex.getMaxFramesInFlight());
                    }
                    std::vector<VkDescriptorSetLayout> layouts(reflectRefractTex.getMaxFramesInFlight(), descriptorSetLayout[shader->getId()]);
                    VkDescriptorSetAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    allocInfo.descriptorPool = descriptorPool[descriptorId];
                    allocInfo.descriptorSetCount = static_cast<uint32_t>(reflectRefractTex.getMaxFramesInFlight());
                    allocInfo.pSetLayouts = layouts.data();
                    if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, descriptorSets[descriptorId].data()) != VK_SUCCESS) {
                        throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
                    }
                }
            }
            void ReflectRefractRenderComponent::allocateDescriptorSets(RenderStates states) {
                Shader* shader = const_cast<Shader*>(states.shader);
                if (shader == &sLinkedList) {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = environmentMap.getDescriptorSet();
                    std::vector<VkDescriptorPool>& descriptorPool = environmentMap.getDescriptorPool();
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = environmentMap.getDescriptorSetLayout();
                    if (shader->getNbShaders() > descriptorSets.size())
                        descriptorSets.resize(shader->getNbShaders());
                    unsigned int descriptorId = shader->getId();
                    for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                        descriptorSets[i].resize(environmentMap.getMaxFramesInFlight());
                    }
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    std::vector<uint32_t> variableCounts(environmentMap.getMaxFramesInFlight(), static_cast<uint32_t>(allTextures.size()));

                    VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
                    variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
                    variableCountInfo.descriptorSetCount = static_cast<uint32_t>(variableCounts.size());;
                    variableCountInfo.pDescriptorCounts = variableCounts.data();
                    std::vector<VkDescriptorSetLayout> layouts(environmentMap.getMaxFramesInFlight(), descriptorSetLayout[descriptorId]);
                    VkDescriptorSetAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    allocInfo.pNext = &variableCountInfo;
                    allocInfo.descriptorPool = descriptorPool[descriptorId];
                    allocInfo.descriptorSetCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    allocInfo.pSetLayouts = layouts.data();
                    if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, descriptorSets[descriptorId].data()) != VK_SUCCESS) {
                        throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
                    }
                } else if (shader == &sLinkedList2) {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = environmentMap.getDescriptorSet();
                    std::vector<VkDescriptorPool>& descriptorPool = environmentMap.getDescriptorPool();
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = environmentMap.getDescriptorSetLayout();
                    if (shader->getNbShaders() > descriptorSets.size())
                        descriptorSets.resize(shader->getNbShaders());
                    unsigned int descriptorId = shader->getId();
                    for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                        descriptorSets[i].resize(environmentMap.getMaxFramesInFlight());
                    }
                    std::vector<VkDescriptorSetLayout> layouts(environmentMap.getMaxFramesInFlight(), descriptorSetLayout[descriptorId]);
                    VkDescriptorSetAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    allocInfo.descriptorPool = descriptorPool[descriptorId];
                    allocInfo.descriptorSetCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    allocInfo.pSetLayouts = layouts.data();
                    if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, descriptorSets[descriptorId].data()) != VK_SUCCESS) {
                        throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
                    }

                } else if (shader == &sBuildDepthBuffer) {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = depthBuffer.getDescriptorSet();
                    std::vector<VkDescriptorPool>& descriptorPool = depthBuffer.getDescriptorPool();
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = depthBuffer.getDescriptorSetLayout();
                    if (shader->getNbShaders() > descriptorSets.size())
                        descriptorSets.resize(shader->getNbShaders());
                    unsigned int descriptorId = shader->getId();
                    for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                        descriptorSets[i].resize(depthBuffer.getMaxFramesInFlight());
                    }
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    std::vector<uint32_t> variableCounts(depthBuffer.getMaxFramesInFlight(), static_cast<uint32_t>(allTextures.size()));

                    VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
                    variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
                    variableCountInfo.descriptorSetCount = static_cast<uint32_t>(variableCounts.size());;
                    variableCountInfo.pDescriptorCounts = variableCounts.data();
                    std::vector<VkDescriptorSetLayout> layouts(depthBuffer.getMaxFramesInFlight(), descriptorSetLayout[descriptorId]);
                    VkDescriptorSetAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    allocInfo.pNext = &variableCountInfo;
                    allocInfo.descriptorPool = descriptorPool[descriptorId];
                    allocInfo.descriptorSetCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());
                    allocInfo.pSetLayouts = layouts.data();
                    if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, descriptorSets[descriptorId].data()) != VK_SUCCESS) {
                        throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
                    }
                } else if (shader == &sBuildAlphaBuffer) {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = alphaBuffer.getDescriptorSet();
                    std::vector<VkDescriptorPool>& descriptorPool = alphaBuffer.getDescriptorPool();
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = alphaBuffer.getDescriptorSetLayout();
                    if (shader->getNbShaders() > descriptorSets.size())
                        descriptorSets.resize(shader->getNbShaders());
                    unsigned int descriptorId = shader->getId();
                    for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                        descriptorSets[i].resize(alphaBuffer.getMaxFramesInFlight());
                    }
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    std::vector<uint32_t> variableCounts(alphaBuffer.getMaxFramesInFlight(), static_cast<uint32_t>(allTextures.size()));

                    VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
                    variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
                    variableCountInfo.descriptorSetCount = static_cast<uint32_t>(variableCounts.size());;
                    variableCountInfo.pDescriptorCounts = variableCounts.data();
                    std::vector<VkDescriptorSetLayout> layouts(alphaBuffer.getMaxFramesInFlight(), descriptorSetLayout[descriptorId]);
                    VkDescriptorSetAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    allocInfo.pNext = &variableCountInfo;
                    allocInfo.descriptorPool = descriptorPool[descriptorId];
                    allocInfo.descriptorSetCount = static_cast<uint32_t>(alphaBuffer.getMaxFramesInFlight());
                    allocInfo.pSetLayouts = layouts.data();
                    if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, descriptorSets[descriptorId].data()) != VK_SUCCESS) {
                        throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
                    }
                } else {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = reflectRefractTex.getDescriptorSet();
                    std::vector<VkDescriptorPool>& descriptorPool = reflectRefractTex.getDescriptorPool();
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = reflectRefractTex.getDescriptorSetLayout();
                    if (shader->getNbShaders() > descriptorSets.size())
                        descriptorSets.resize(shader->getNbShaders());
                    unsigned int descriptorId = shader->getId();
                    for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                        descriptorSets[i].resize(reflectRefractTex.getMaxFramesInFlight());
                    }
                    std::vector<VkDescriptorSetLayout> layouts(reflectRefractTex.getMaxFramesInFlight(), descriptorSetLayout[descriptorId]);
                    VkDescriptorSetAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    allocInfo.descriptorPool = descriptorPool[descriptorId];
                    allocInfo.descriptorSetCount = static_cast<uint32_t>(reflectRefractTex.getMaxFramesInFlight());
                    allocInfo.pSetLayouts = layouts.data();
                    if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, descriptorSets[descriptorId].data()) != VK_SUCCESS) {
                        throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
                    }
                }
            }
            void ReflectRefractRenderComponent::updateDescriptorSets(unsigned int currentFrame, unsigned int p, RenderStates states) {
                Shader* shader = const_cast<Shader*>(states.shader);
                if (shader == &skyboxShader) {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = environmentMap.getDescriptorSet();
                    unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
                    VkDescriptorBufferInfo bufferInfo{};
                    bufferInfo.buffer = uniformBuffer[currentFrame];
                    bufferInfo.offset = 0;
                    bufferInfo.range = sizeof(UniformBufferObject);

                    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[0].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[0].dstBinding = 0;
                    descriptorWrites[0].dstArrayElement = 0;
                    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                    descriptorWrites[0].descriptorCount = 1;
                    descriptorWrites[0].pBufferInfo = &bufferInfo;

                    VkDescriptorImageInfo descriptorImageInfo;
                    descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    descriptorImageInfo.imageView = skybox->getTexture().getImageView();
                    descriptorImageInfo.sampler = skybox->getTexture().getSampler();

                    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[1].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[1].dstBinding = 1;
                    descriptorWrites[1].dstArrayElement = 0;
                    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrites[1].descriptorCount = 1;
                    descriptorWrites[1].pImageInfo = &descriptorImageInfo;

                    vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);


                } else if (shader == &sLinkedList) {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = environmentMap.getDescriptorSet();
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();

                    std::vector<VkDescriptorImageInfo>	descriptorImageInfos;
                    descriptorImageInfos.resize(allTextures.size());
                    for (unsigned int j = 0; j < allTextures.size(); j++) {
                        descriptorImageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        descriptorImageInfos[j].imageView = allTextures[j]->getImageView();
                        descriptorImageInfos[j].sampler = allTextures[j]->getSampler();
                    }
                    std::array<VkWriteDescriptorSet, 7> descriptorWrites{};

                    VkDescriptorBufferInfo counterStorageBufferInfoLastFrame{};
                    counterStorageBufferInfoLastFrame.buffer = counterShaderStorageBuffers[currentFrame];
                    counterStorageBufferInfoLastFrame.offset = 0;
                    counterStorageBufferInfoLastFrame.range = sizeof(AtomicCounterSSBO);

                    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[0].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[0].dstBinding = 0;
                    descriptorWrites[0].dstArrayElement = 0;
                    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descriptorWrites[0].descriptorCount = 1;
                    descriptorWrites[0].pBufferInfo = &counterStorageBufferInfoLastFrame;

                    VkDescriptorImageInfo headPtrDescriptorImageInfo;
                    headPtrDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                    headPtrDescriptorImageInfo.imageView = headPtrTextureImageView[currentFrame];
                    headPtrDescriptorImageInfo.sampler = headPtrTextureSampler[currentFrame];

                    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[1].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[1].dstBinding = 1;
                    descriptorWrites[1].dstArrayElement = 0;
                    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    descriptorWrites[1].descriptorCount = 1;
                    descriptorWrites[1].pImageInfo = &headPtrDescriptorImageInfo;

                    VkDescriptorBufferInfo linkedListStorageBufferInfoLastFrame{};
                    linkedListStorageBufferInfoLastFrame.buffer = linkedListShaderStorageBuffers[currentFrame];
                    linkedListStorageBufferInfoLastFrame.offset = 0;
                    unsigned int nodeSize = 5 * sizeof(float) + sizeof(unsigned int);
                    linkedListStorageBufferInfoLastFrame.range = maxNodes * nodeSize * 6;

                    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[2].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[2].dstBinding = 2;
                    descriptorWrites[2].dstArrayElement = 0;
                    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descriptorWrites[2].descriptorCount = 1;
                    descriptorWrites[2].pBufferInfo = &linkedListStorageBufferInfoLastFrame;



                    VkDescriptorBufferInfo modelDataStorageBufferInfoLastFrame{};
                    modelDataStorageBufferInfoLastFrame.buffer = modelDataBufferMT[p][currentFrame];
                    modelDataStorageBufferInfoLastFrame.offset = 0;
                    modelDataStorageBufferInfoLastFrame.range = maxAlignedSizeModelData[p];

                    descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[3].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[3].dstBinding = 3;
                    descriptorWrites[3].dstArrayElement = 0;
                    descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    descriptorWrites[3].descriptorCount = 1;
                    descriptorWrites[3].pBufferInfo = &modelDataStorageBufferInfoLastFrame;

                    VkDescriptorBufferInfo materialDataStorageBufferInfoLastFrame{};
                    materialDataStorageBufferInfoLastFrame.buffer = materialDataBufferMT[p][currentFrame];
                    materialDataStorageBufferInfoLastFrame.offset = 0;
                    materialDataStorageBufferInfoLastFrame.range = maxAlignedSizeMaterialData[p];

                    descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[4].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[4].dstBinding = 4;
                    descriptorWrites[4].dstArrayElement = 0;
                    descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    descriptorWrites[4].descriptorCount = 1;
                    descriptorWrites[4].pBufferInfo = &materialDataStorageBufferInfoLastFrame;

                    VkDescriptorBufferInfo bufferInfo{};
                    bufferInfo.buffer = uniformBuffer[currentFrame];
                    bufferInfo.offset = 0;
                    bufferInfo.range = sizeof(UniformBufferObject);

                    descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[5].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[5].dstBinding = 5;
                    descriptorWrites[5].dstArrayElement = 0;
                    descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                    descriptorWrites[5].descriptorCount = 1;
                    descriptorWrites[5].pBufferInfo = &bufferInfo;

                    descriptorWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[6].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[6].dstBinding = 6;
                    descriptorWrites[6].dstArrayElement = 0;
                    descriptorWrites[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrites[6].descriptorCount = allTextures.size();
                    descriptorWrites[6].pImageInfo = descriptorImageInfos.data();

                    vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

                } else if (shader == &sLinkedList2) {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = environmentMap.getDescriptorSet();
                    unsigned int descriptorId = shader->getId();
                    std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

                    VkDescriptorBufferInfo counterStorageBufferInfoLastFrame{};
                    counterStorageBufferInfoLastFrame.buffer = counterShaderStorageBuffers[currentFrame];
                    counterStorageBufferInfoLastFrame.offset = 0;
                    counterStorageBufferInfoLastFrame.range = sizeof(AtomicCounterSSBO);

                    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[0].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[0].dstBinding = 0;
                    descriptorWrites[0].dstArrayElement = 0;
                    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descriptorWrites[0].descriptorCount = 1;
                    descriptorWrites[0].pBufferInfo = &counterStorageBufferInfoLastFrame;

                    VkDescriptorImageInfo headPtrDescriptorImageInfo;
                    headPtrDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                    headPtrDescriptorImageInfo.imageView = headPtrTextureImageView[currentFrame];
                    headPtrDescriptorImageInfo.sampler = headPtrTextureSampler[currentFrame];

                    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[1].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[1].dstBinding = 1;
                    descriptorWrites[1].dstArrayElement = 0;
                    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    descriptorWrites[1].descriptorCount = 1;
                    descriptorWrites[1].pImageInfo = &headPtrDescriptorImageInfo;

                    VkDescriptorBufferInfo linkedListStorageBufferInfoLastFrame{};
                    linkedListStorageBufferInfoLastFrame.buffer = linkedListShaderStorageBuffers[currentFrame];
                    linkedListStorageBufferInfoLastFrame.offset = 0;
                    unsigned int nodeSize = 5 * sizeof(float) + sizeof(unsigned int);
                    linkedListStorageBufferInfoLastFrame.range = maxNodes * nodeSize * 6;

                    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[2].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[2].dstBinding = 2;
                    descriptorWrites[2].dstArrayElement = 0;
                    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descriptorWrites[2].descriptorCount = 1;
                    descriptorWrites[2].pBufferInfo = &linkedListStorageBufferInfoLastFrame;
                    vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

                } else if (shader == &sBuildDepthBuffer) {
                       std::vector<std::vector<VkDescriptorSet>>& descriptorSets = depthBuffer.getDescriptorSet();
                       std::vector<Texture*> allTextures = Texture::getAllTextures();
                       unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                        std::vector<VkDescriptorImageInfo>	descriptorImageInfos;
                        descriptorImageInfos.resize(allTextures.size());
                        for (unsigned int j = 0; j < allTextures.size(); j++) {
                            descriptorImageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                            descriptorImageInfos[j].imageView = allTextures[j]->getImageView();
                            descriptorImageInfos[j].sampler = allTextures[j]->getSampler();
                        }
                        std::array<VkWriteDescriptorSet, 4> descriptorWrites{};

                        VkDescriptorImageInfo headPtrDescriptorImageInfo;
                        headPtrDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                        headPtrDescriptorImageInfo.imageView = depthTextureImageView[currentFrame];
                        headPtrDescriptorImageInfo.sampler = depthTextureSampler[currentFrame];

                        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[0].dstSet = descriptorSets[descriptorId][currentFrame];
                        descriptorWrites[0].dstBinding = 1;
                        descriptorWrites[0].dstArrayElement = 0;
                        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                        descriptorWrites[0].descriptorCount = 1;
                        descriptorWrites[0].pImageInfo = &headPtrDescriptorImageInfo;

                        VkDescriptorBufferInfo modelDataStorageBufferInfoLastFrame{};
                        modelDataStorageBufferInfoLastFrame.buffer = modelDataBufferMT[p][currentFrame];
                        modelDataStorageBufferInfoLastFrame.offset = 0;
                        modelDataStorageBufferInfoLastFrame.range = maxAlignedSizeModelData[p];


                        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[1].dstSet = descriptorSets[descriptorId][currentFrame];
                        descriptorWrites[1].dstBinding = 3;
                        descriptorWrites[1].dstArrayElement = 0;
                        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                        descriptorWrites[1].descriptorCount = 1;
                        descriptorWrites[1].pBufferInfo = &modelDataStorageBufferInfoLastFrame;

                        VkDescriptorBufferInfo materialDataStorageBufferInfoLastFrame{};
                        materialDataStorageBufferInfoLastFrame.buffer = materialDataBufferMT[p][currentFrame];
                        materialDataStorageBufferInfoLastFrame.offset = 0;
                        materialDataStorageBufferInfoLastFrame.range = maxAlignedSizeMaterialData[p];

                        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[2].dstSet = descriptorSets[descriptorId][currentFrame];
                        descriptorWrites[2].dstBinding = 4;
                        descriptorWrites[2].dstArrayElement = 0;
                        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                        descriptorWrites[2].descriptorCount = 1;
                        descriptorWrites[2].pBufferInfo = &materialDataStorageBufferInfoLastFrame;

                        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[3].dstSet = descriptorSets[descriptorId][currentFrame];
                        descriptorWrites[3].dstBinding = 5;
                        descriptorWrites[3].dstArrayElement = 0;
                        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[3].descriptorCount = allTextures.size();
                        descriptorWrites[3].pImageInfo = descriptorImageInfos.data();

                        vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

                } else if (shader == &sBuildAlphaBuffer) {
                        std::vector<std::vector<VkDescriptorSet>>& descriptorSets = alphaBuffer.getDescriptorSet();
                        std::vector<Texture*> allTextures = Texture::getAllTextures();
                        unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                        std::vector<VkDescriptorImageInfo>	descriptorImageInfos;
                        descriptorImageInfos.resize(allTextures.size());
                        for (unsigned int j = 0; j < allTextures.size(); j++) {
                            descriptorImageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                            descriptorImageInfos[j].imageView = allTextures[j]->getImageView();
                            descriptorImageInfos[j].sampler = allTextures[j]->getSampler();
                        }
                        std::array<VkWriteDescriptorSet, 5> descriptorWrites{};

                        VkDescriptorImageInfo headPtrDescriptorImageInfo;
                        headPtrDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                        headPtrDescriptorImageInfo.imageView = alphaTextureImageView[currentFrame];
                        headPtrDescriptorImageInfo.sampler = alphaTextureSampler[currentFrame];

                        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[0].dstSet = descriptorSets[descriptorId][currentFrame];
                        descriptorWrites[0].dstBinding = 1;
                        descriptorWrites[0].dstArrayElement = 0;
                        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                        descriptorWrites[0].descriptorCount = 1;
                        descriptorWrites[0].pImageInfo = &headPtrDescriptorImageInfo;

                        std::array<VkDescriptorImageInfo, RenderTexture::NB_SWAPCHAIN_IMAGES>	descriptorImageInfos2;
                        for (unsigned int j = 0; j < depthBuffer.getSwapchainImagesSize(); j++) {
                            descriptorImageInfos2[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                            descriptorImageInfos2[j].imageView = depthBuffer.getTexture(j).getImageView();
                            descriptorImageInfos2[j].sampler = depthBuffer.getTexture(j).getSampler();
                        }

                        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[1].dstSet = descriptorSets[descriptorId][currentFrame];
                        descriptorWrites[1].dstBinding = 2;
                        descriptorWrites[1].dstArrayElement = 0;
                        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[1].descriptorCount = descriptorImageInfos2.size();
                        descriptorWrites[1].pImageInfo = descriptorImageInfos2.data();

                        VkDescriptorBufferInfo modelDataStorageBufferInfoLastFrame{};
                        modelDataStorageBufferInfoLastFrame.buffer = modelDataBufferMT[p][currentFrame];
                        modelDataStorageBufferInfoLastFrame.offset = 0;
                        modelDataStorageBufferInfoLastFrame.range = maxAlignedSizeModelData[p];
                        ////std::cout<<"max model data : "<<maxAlignedSizeModelData[p]<<std::endl;

                        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[2].dstSet = descriptorSets[descriptorId][currentFrame];
                        descriptorWrites[2].dstBinding = 3;
                        descriptorWrites[2].dstArrayElement = 0;
                        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                        descriptorWrites[2].descriptorCount = 1;
                        descriptorWrites[2].pBufferInfo = &modelDataStorageBufferInfoLastFrame;

                        VkDescriptorBufferInfo materialDataStorageBufferInfoLastFrame{};
                        materialDataStorageBufferInfoLastFrame.buffer = materialDataBufferMT[p][currentFrame];
                        materialDataStorageBufferInfoLastFrame.offset = 0;
                        materialDataStorageBufferInfoLastFrame.range = maxAlignedSizeMaterialData[p];

                        ////std::cout<<"max model data : "<<maxAlignedSizeMaterialData[p]<<std::endl;

                        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[3].dstSet = descriptorSets[descriptorId][currentFrame];
                        descriptorWrites[3].dstBinding = 4;
                        descriptorWrites[3].dstArrayElement = 0;
                        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                        descriptorWrites[3].descriptorCount = 1;
                        descriptorWrites[3].pBufferInfo = &materialDataStorageBufferInfoLastFrame;

                        descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[4].dstSet = descriptorSets[descriptorId][currentFrame];
                        descriptorWrites[4].dstBinding = 5;
                        descriptorWrites[4].dstArrayElement = 0;
                        descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[4].descriptorCount = allTextures.size();
                        descriptorWrites[4].pImageInfo = descriptorImageInfos.data();

                        vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
                } else {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = reflectRefractTex.getDescriptorSet();
                    unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                    std::array<VkDescriptorImageInfo, RenderTexture::NB_SWAPCHAIN_IMAGES>   descriptorImageInfos;
                    for (unsigned int j = 0; j < environmentMap.getSwapchainImagesSize(); j++) {
                        descriptorImageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        descriptorImageInfos[j].imageView = environmentMap.getTexture(j).getImageView();
                        descriptorImageInfos[j].sampler = environmentMap.getTexture(j).getSampler();
                    }

                    std::array<VkWriteDescriptorSet, 4> descriptorWrites{};
                    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[0].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[0].dstBinding = 0;
                    descriptorWrites[0].dstArrayElement = 0;
                    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrites[0].descriptorCount = descriptorImageInfos.size();
                    descriptorWrites[0].pImageInfo = descriptorImageInfos.data();

                    std::array<VkDescriptorImageInfo, RenderTexture::NB_SWAPCHAIN_IMAGES>   descriptorImageInfos2;
                    for (unsigned int j = 0; j < alphaBuffer.getSwapchainImagesSize(); j++) {
                        descriptorImageInfos2[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        descriptorImageInfos2[j].imageView = alphaBuffer.getTexture(j).getImageView();
                        descriptorImageInfos2[j].sampler = alphaBuffer.getTexture(j).getSampler();
                    }

                    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[1].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[1].dstBinding = 1;
                    descriptorWrites[1].dstArrayElement = 0;
                    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrites[1].descriptorCount = descriptorImageInfos2.size();
                    descriptorWrites[1].pImageInfo = descriptorImageInfos2.data();

                    VkDescriptorBufferInfo modelDataStorageBufferInfoLastFrame{};
                    modelDataStorageBufferInfoLastFrame.buffer = modelDataBufferMT[p][currentFrame];
                    modelDataStorageBufferInfoLastFrame.offset = 0;
                    modelDataStorageBufferInfoLastFrame.range = maxAlignedSizeModelData[p];

                    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[2].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[2].dstBinding = 3;
                    descriptorWrites[2].dstArrayElement = 0;
                    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    descriptorWrites[2].descriptorCount = 1;
                    descriptorWrites[2].pBufferInfo = &modelDataStorageBufferInfoLastFrame;

                    VkDescriptorBufferInfo materialDataStorageBufferInfoLastFrame{};
                    materialDataStorageBufferInfoLastFrame.buffer = materialDataBufferMT[p][currentFrame];
                    materialDataStorageBufferInfoLastFrame.offset = 0;
                    materialDataStorageBufferInfoLastFrame.range = maxAlignedSizeMaterialData[p];

                    descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[3].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[3].dstBinding = 4;
                    descriptorWrites[3].dstArrayElement = 0;
                    descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    descriptorWrites[3].descriptorCount = 1;
                    descriptorWrites[3].pBufferInfo = &materialDataStorageBufferInfoLastFrame;

                    vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

                }
            }
            void ReflectRefractRenderComponent::createDescriptorSets(RenderStates states) {
                Shader* shader = const_cast<Shader*>(states.shader);
                if (shader == &sLinkedList) {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = environmentMap.getDescriptorSet();
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    unsigned int descriptorId = shader->getId();
                    for (size_t i = 0; i < environmentMap.getMaxFramesInFlight(); i++) {
                        std::vector<VkDescriptorImageInfo>	descriptorImageInfos;
                        descriptorImageInfos.resize(allTextures.size());
                        for (unsigned int j = 0; j < allTextures.size(); j++) {
                            descriptorImageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                            descriptorImageInfos[j].imageView = allTextures[j]->getImageView();
                            descriptorImageInfos[j].sampler = allTextures[j]->getSampler();
                        }
                        std::array<VkWriteDescriptorSet, 7> descriptorWrites{};

                        VkDescriptorBufferInfo counterStorageBufferInfoLastFrame{};
                        counterStorageBufferInfoLastFrame.buffer = counterShaderStorageBuffers[i];
                        counterStorageBufferInfoLastFrame.offset = 0;
                        counterStorageBufferInfoLastFrame.range = sizeof(AtomicCounterSSBO);

                        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[0].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[0].dstBinding = 0;
                        descriptorWrites[0].dstArrayElement = 0;
                        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                        descriptorWrites[0].descriptorCount = 1;
                        descriptorWrites[0].pBufferInfo = &counterStorageBufferInfoLastFrame;

                        VkDescriptorImageInfo headPtrDescriptorImageInfo;
                        headPtrDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                        headPtrDescriptorImageInfo.imageView = headPtrTextureImageView[i];
                        headPtrDescriptorImageInfo.sampler = headPtrTextureSampler[i];

                        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[1].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[1].dstBinding = 1;
                        descriptorWrites[1].dstArrayElement = 0;
                        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                        descriptorWrites[1].descriptorCount = 1;
                        descriptorWrites[1].pImageInfo = &headPtrDescriptorImageInfo;

                        VkDescriptorBufferInfo linkedListStorageBufferInfoLastFrame{};
                        linkedListStorageBufferInfoLastFrame.buffer = linkedListShaderStorageBuffers[i];
                        linkedListStorageBufferInfoLastFrame.offset = 0;
                        unsigned int nodeSize = 5 * sizeof(float) + sizeof(unsigned int);
                        linkedListStorageBufferInfoLastFrame.range = maxNodes * nodeSize * 6;

                        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[2].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[2].dstBinding = 2;
                        descriptorWrites[2].dstArrayElement = 0;
                        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                        descriptorWrites[2].descriptorCount = 1;
                        descriptorWrites[2].pBufferInfo = &linkedListStorageBufferInfoLastFrame;



                        VkDescriptorBufferInfo modelDataStorageBufferInfoLastFrame{};
                        modelDataStorageBufferInfoLastFrame.buffer = modelDataShaderStorageBuffers[i];
                        modelDataStorageBufferInfoLastFrame.offset = 0;
                        modelDataStorageBufferInfoLastFrame.range = maxModelDataSize;

                        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[3].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[3].dstBinding = 3;
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
                        descriptorWrites[4].dstBinding = 4;
                        descriptorWrites[4].dstArrayElement = 0;
                        descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                        descriptorWrites[4].descriptorCount = 1;
                        descriptorWrites[4].pBufferInfo = &materialDataStorageBufferInfoLastFrame;

                        VkDescriptorBufferInfo bufferInfo{};
                        bufferInfo.buffer = uniformBuffer[i];
                        bufferInfo.offset = 0;
                        bufferInfo.range = sizeof(UniformBufferObject);

                        descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[5].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[5].dstBinding = 5;
                        descriptorWrites[5].dstArrayElement = 0;
                        descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                        descriptorWrites[5].descriptorCount = 1;
                        descriptorWrites[5].pBufferInfo = &bufferInfo;

                        descriptorWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[6].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[6].dstBinding = 6;
                        descriptorWrites[6].dstArrayElement = 0;
                        descriptorWrites[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[6].descriptorCount = allTextures.size();
                        descriptorWrites[6].pImageInfo = descriptorImageInfos.data();

                        vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
                    }
                } else if (shader == &sLinkedList2) {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = environmentMap.getDescriptorSet();
                    unsigned int descriptorId = shader->getId();
                    for (size_t i = 0; i < environmentMap.getMaxFramesInFlight(); i++) {
                        std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

                        VkDescriptorBufferInfo counterStorageBufferInfoLastFrame{};
                        counterStorageBufferInfoLastFrame.buffer = counterShaderStorageBuffers[i];
                        counterStorageBufferInfoLastFrame.offset = 0;
                        counterStorageBufferInfoLastFrame.range = sizeof(AtomicCounterSSBO);

                        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[0].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[0].dstBinding = 0;
                        descriptorWrites[0].dstArrayElement = 0;
                        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                        descriptorWrites[0].descriptorCount = 1;
                        descriptorWrites[0].pBufferInfo = &counterStorageBufferInfoLastFrame;

                        VkDescriptorImageInfo headPtrDescriptorImageInfo;
                        headPtrDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                        headPtrDescriptorImageInfo.imageView = headPtrTextureImageView[i];
                        headPtrDescriptorImageInfo.sampler = headPtrTextureSampler[i];

                        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[1].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[1].dstBinding = 1;
                        descriptorWrites[1].dstArrayElement = 0;
                        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                        descriptorWrites[1].descriptorCount = 1;
                        descriptorWrites[1].pImageInfo = &headPtrDescriptorImageInfo;

                        VkDescriptorBufferInfo linkedListStorageBufferInfoLastFrame{};
                        linkedListStorageBufferInfoLastFrame.buffer = linkedListShaderStorageBuffers[i];
                        linkedListStorageBufferInfoLastFrame.offset = 0;
                        unsigned int nodeSize = 5 * sizeof(float) + sizeof(unsigned int);
                        linkedListStorageBufferInfoLastFrame.range = maxNodes * nodeSize * 6;

                        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[2].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[2].dstBinding = 2;
                        descriptorWrites[2].dstArrayElement = 0;
                        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                        descriptorWrites[2].descriptorCount = 1;
                        descriptorWrites[2].pBufferInfo = &linkedListStorageBufferInfoLastFrame;
                        vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
                    }
                } else if (shader == &sBuildDepthBuffer) {
                       std::vector<std::vector<VkDescriptorSet>>& descriptorSets = depthBuffer.getDescriptorSet();
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

                            VkDescriptorImageInfo headPtrDescriptorImageInfo;
                            headPtrDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                            headPtrDescriptorImageInfo.imageView = depthTextureImageView[i];
                            headPtrDescriptorImageInfo.sampler = depthTextureSampler[i];

                            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                            descriptorWrites[0].dstSet = descriptorSets[descriptorId][i];
                            descriptorWrites[0].dstBinding = 1;
                            descriptorWrites[0].dstArrayElement = 0;
                            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                            descriptorWrites[0].descriptorCount = 1;
                            descriptorWrites[0].pImageInfo = &headPtrDescriptorImageInfo;

                            VkDescriptorBufferInfo modelDataStorageBufferInfoLastFrame{};
                            modelDataStorageBufferInfoLastFrame.buffer = modelDataShaderStorageBuffers[i];
                            modelDataStorageBufferInfoLastFrame.offset = 0;
                            modelDataStorageBufferInfoLastFrame.range = maxModelDataSize;

                            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                            descriptorWrites[1].dstSet = descriptorSets[descriptorId][i];
                            descriptorWrites[1].dstBinding = 3;
                            descriptorWrites[1].dstArrayElement = 0;
                            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                            descriptorWrites[1].descriptorCount = 1;
                            descriptorWrites[1].pBufferInfo = &modelDataStorageBufferInfoLastFrame;

                            VkDescriptorBufferInfo materialDataStorageBufferInfoLastFrame{};
                            materialDataStorageBufferInfoLastFrame.buffer = materialDataShaderStorageBuffers[i];
                            materialDataStorageBufferInfoLastFrame.offset = 0;
                            materialDataStorageBufferInfoLastFrame.range = maxMaterialDataSize;

                            descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                            descriptorWrites[2].dstSet = descriptorSets[descriptorId][i];
                            descriptorWrites[2].dstBinding = 4;
                            descriptorWrites[2].dstArrayElement = 0;
                            descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                            descriptorWrites[2].descriptorCount = 1;
                            descriptorWrites[2].pBufferInfo = &materialDataStorageBufferInfoLastFrame;

                            descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                            descriptorWrites[3].dstSet = descriptorSets[descriptorId][i];
                            descriptorWrites[3].dstBinding = 5;
                            descriptorWrites[3].dstArrayElement = 0;
                            descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                            descriptorWrites[3].descriptorCount = allTextures.size();
                            descriptorWrites[3].pImageInfo = descriptorImageInfos.data();

                            vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
                       }
                } else if (shader == &sBuildAlphaBuffer) {
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

                            VkDescriptorImageInfo headPtrDescriptorImageInfo;
                            headPtrDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                            headPtrDescriptorImageInfo.imageView = alphaTextureImageView[i];
                            headPtrDescriptorImageInfo.sampler = alphaTextureSampler[i];

                            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                            descriptorWrites[0].dstSet = descriptorSets[descriptorId][i];
                            descriptorWrites[0].dstBinding = 1;
                            descriptorWrites[0].dstArrayElement = 0;
                            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                            descriptorWrites[0].descriptorCount = 1;
                            descriptorWrites[0].pImageInfo = &headPtrDescriptorImageInfo;

                            VkDescriptorImageInfo	descriptorImageInfo;
                            descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                            descriptorImageInfo.imageView = depthBuffer.getTexture((depthBuffer.getImageIndex()+i)%depthBuffer.getSwapchainImagesSize()).getImageView();
                            descriptorImageInfo.sampler = depthBuffer.getTexture((depthBuffer.getImageIndex()+i)%depthBuffer.getSwapchainImagesSize()).getSampler();

                            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                            descriptorWrites[1].dstSet = descriptorSets[descriptorId][i];
                            descriptorWrites[1].dstBinding = 2;
                            descriptorWrites[1].dstArrayElement = 0;
                            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                            descriptorWrites[1].descriptorCount = 1;
                            descriptorWrites[1].pImageInfo = descriptorImageInfos.data();

                            VkDescriptorBufferInfo modelDataStorageBufferInfoLastFrame{};
                            modelDataStorageBufferInfoLastFrame.buffer = modelDataShaderStorageBuffers[i];
                            modelDataStorageBufferInfoLastFrame.offset = 0;
                            modelDataStorageBufferInfoLastFrame.range = maxModelDataSize;

                            descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                            descriptorWrites[2].dstSet = descriptorSets[descriptorId][i];
                            descriptorWrites[2].dstBinding = 3;
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
                            descriptorWrites[3].dstBinding = 4;
                            descriptorWrites[3].dstArrayElement = 0;
                            descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                            descriptorWrites[3].descriptorCount = 1;
                            descriptorWrites[3].pBufferInfo = &materialDataStorageBufferInfoLastFrame;

                            descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                            descriptorWrites[4].dstSet = descriptorSets[descriptorId][i];
                            descriptorWrites[4].dstBinding = 5;
                            descriptorWrites[4].dstArrayElement = 0;
                            descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                            descriptorWrites[4].descriptorCount = allTextures.size();
                            descriptorWrites[4].pImageInfo = descriptorImageInfos.data();

                            vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
                    }
                } else {
                    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = reflectRefractTex.getDescriptorSet();
                    unsigned int descriptorId = shader->getId();
                    for (size_t i = 0; i < reflectRefractTex.getMaxFramesInFlight(); i++) {
                        VkDescriptorImageInfo   descriptorImageInfo;
                        descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        descriptorImageInfo.imageView = environmentMap.getTexture((environmentMap.getImageIndex()+i)%environmentMap.getSwapchainImagesSize()).getImageView();
                        descriptorImageInfo.sampler = environmentMap.getTexture((environmentMap.getImageIndex()+i)%environmentMap.getSwapchainImagesSize()).getSampler();

                        std::array<VkWriteDescriptorSet, 4> descriptorWrites{};
                        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[0].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[0].dstBinding = 0;
                        descriptorWrites[0].dstArrayElement = 0;
                        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[0].descriptorCount = 1;
                        descriptorWrites[0].pImageInfo = &descriptorImageInfo;

                        VkDescriptorImageInfo   descriptorImageInfo2;
                        descriptorImageInfo2.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        descriptorImageInfo2.imageView = alphaBuffer.getTexture((alphaBuffer.getImageIndex()+i)%alphaBuffer.getSwapchainImagesSize()).getImageView();
                        descriptorImageInfo2.sampler = alphaBuffer.getTexture((alphaBuffer.getImageIndex()+i)%alphaBuffer.getSwapchainImagesSize()).getSampler();

                        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[1].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[1].dstBinding = 1;
                        descriptorWrites[1].dstArrayElement = 0;
                        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[1].descriptorCount = 1;
                        descriptorWrites[1].pImageInfo = &descriptorImageInfo2;

                        VkDescriptorBufferInfo modelDataStorageBufferInfoLastFrame{};
                        modelDataStorageBufferInfoLastFrame.buffer = modelDataShaderStorageBuffers[i];
                        modelDataStorageBufferInfoLastFrame.offset = 0;
                        modelDataStorageBufferInfoLastFrame.range = maxModelDataSize;

                        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[2].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[2].dstBinding = 3;
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
                        descriptorWrites[3].dstBinding = 4;
                        descriptorWrites[3].dstArrayElement = 0;
                        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                        descriptorWrites[3].descriptorCount = 1;
                        descriptorWrites[3].pBufferInfo = &materialDataStorageBufferInfoLastFrame;

                        vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
                    }
                }
            }
            void ReflectRefractRenderComponent::clear() {
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

                reflectRefractTex.beginRecordCommandBuffers();
                const_cast<Texture&>(reflectRefractTex.getTexture(reflectRefractTex.getImageIndex())).toColorAttachmentOptimal(reflectRefractTex.getCommandBuffers()[reflectRefractTex.getCurrentFrame()]);
                reflectRefractTex.clear(Color::Transparent);
            }
            void ReflectRefractRenderComponent::resetBuffers() {
                for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                        totalBufferSizeModelData[i] = 0;
                        totalBufferSizeMaterialData[i] = 0;
                        totalVertexCount[i] = 0;
                        totalVertexIndexCount[i] = 0;
                        totalIndexCount[i] = 0;
                        vertexOffsets[i].clear();
                        vertexIndexOffsets[i].clear();
                        indexOffsets[i].clear();
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
                        previousModelOffset[i] = 0;
                        currentMaterialOffset[i] = 0;
                        previousMaterialOffset[i] = 0;
                        maxAlignedSizeModelData[i] = 0;
                        maxAlignedSizeMaterialData[i] = 0;
                        oldTotalBufferSizeMaterialData[i] = 0;
                        oldTotalBufferSizeModelData[i] = 0;
                    }
                    nbReflRefrEntities = 0;
            }
            void ReflectRefractRenderComponent::recordCommandBufferIndirect(unsigned int currentFrame, unsigned int p, unsigned int nbIndirectCommands, unsigned int stride, RRRCDepthStencilID depthStencilID, unsigned int vertexOffset, unsigned int indexOffset, unsigned int uboOffset, unsigned int modelDataOffset, unsigned int materialDataOffset, unsigned int drawCommandOffset, RenderStates currentStates, VkCommandBuffer commandBuffer) {
                //std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                Shader* shader = const_cast<Shader*>(currentStates.shader);
                currentStates.blendMode.updateIds();
                if (shader == &sBuildDepthBuffer) {
                    buildDepthPC.nbLayers = GameObject::getNbLayers();
                    vkCmdPushConstants(commandBuffer, depthBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(IndirectRenderingPC), &indirectRenderingPC);
                    vkCmdPushConstants(commandBuffer, depthBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_FRAGMENT_BIT, 128, sizeof(BuildDepthPC), &buildDepthPC);
                    std::vector<unsigned int> dynamicBufferOffsets;
                    dynamicBufferOffsets.push_back(modelDataOffset);
                    dynamicBufferOffsets.push_back(materialDataOffset);
                    if (indexOffset == -1)
                        depthBuffer.drawIndirect(commandBuffer, currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], drawCommandBufferMT[p][currentFrame], depthStencilID,currentStates, p, vertexOffset, drawCommandOffset, dynamicBufferOffsets);
                    else
                        depthBuffer.drawIndirect(commandBuffer, currentFrame, nbIndirectCommands, stride, vbBindlessTexIndexed[p], drawCommandBufferIndexedMT[p][currentFrame], depthStencilID,currentStates, p, vertexOffset, drawCommandOffset, dynamicBufferOffsets, indexOffset);
                } else if (shader == &sBuildAlphaBuffer) {
                    buildAlphaPC.nbLayers = GameObject::getNbLayers();
                    buildAlphaPC.resolution = resolution;
                    //std::cout<<"record alpha ids : "<<depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id<<std::endl;
                    //std::cout<<"record ids : "<<(sBuildAlphaBuffer.getId() * (Batcher::nbPrimitiveTypes - 1) + p)<<","<<(RRRCNODEPTHNOSTENCIL*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id)<<std::endl;
                    vkCmdPushConstants(commandBuffer, alphaBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(IndirectRenderingPC), &indirectRenderingPC);
                    vkCmdPushConstants(commandBuffer, alphaBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_FRAGMENT_BIT, 128, sizeof(BuildAlphaPC), &buildAlphaPC);

                    std::vector<unsigned int> dynamicBufferOffsets;
                    dynamicBufferOffsets.push_back(modelDataOffset);
                    dynamicBufferOffsets.push_back(materialDataOffset);
                    if (indexOffset == -1)
                        alphaBuffer.drawIndirect(commandBuffer, currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], drawCommandBufferMT[p][currentFrame], depthStencilID,currentStates, p,  vertexOffset, drawCommandOffset, dynamicBufferOffsets);
                    else
                        alphaBuffer.drawIndirect(commandBuffer, currentFrame, nbIndirectCommands, stride, vbBindlessTexIndexed[p], drawCommandBufferIndexedMT[p][currentFrame], depthStencilID,currentStates,p, vertexOffset, drawCommandOffset, dynamicBufferOffsets, indexOffset);
                } else if (shader == &sLinkedList) {
                    std::vector<unsigned int> dynamicBufferOffsets;
                    dynamicBufferOffsets.push_back(modelDataOffset);
                    dynamicBufferOffsets.push_back(materialDataOffset);
                    dynamicBufferOffsets.push_back(uboOffset);
                    if (indexOffset == -1)
                        environmentMap.drawIndirect(commandBuffer, currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], drawCommandBufferMT[p][currentFrame], depthStencilID,currentStates, p, vertexOffset, drawCommandOffset, dynamicBufferOffsets);
                    else
                        environmentMap.drawIndirect(commandBuffer, currentFrame, nbIndirectCommands, stride, vbBindlessTexIndexed[p], drawCommandBufferIndexedMT[p][currentFrame], depthStencilID,currentStates,p,  vertexOffset, drawCommandOffset, dynamicBufferOffsets, indexOffset);
                } else {
                   buildFrameBufferPC.resolution = resolution;
                   vkCmdPushConstants(commandBuffer, reflectRefractTex.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(IndirectRenderingPC), &indirectRenderingPC);
                   vkCmdPushConstants(commandBuffer, reflectRefractTex.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_FRAGMENT_BIT, 128, sizeof(BuildFrameBufferPC), &buildFrameBufferPC);
                   std::vector<unsigned int> dynamicBufferOffsets;
                   dynamicBufferOffsets.push_back(modelDataOffset);
                   dynamicBufferOffsets.push_back(materialDataOffset);
                   if (indexOffset == -1)
                        reflectRefractTex.drawIndirect(commandBuffer, currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], drawCommandBufferMT[p][currentFrame], depthStencilID,currentStates,p,  vertexOffset, drawCommandOffset, dynamicBufferOffsets);
                   else
                        reflectRefractTex.drawIndirect(commandBuffer, currentFrame, nbIndirectCommands, stride, vbBindlessTexIndexed[p], drawCommandBufferIndexedMT[p][currentFrame], depthStencilID,currentStates,p,  vertexOffset, drawCommandOffset, dynamicBufferOffsets, indexOffset);

                }
            }
            void ReflectRefractRenderComponent::createCommandBuffersIndirect(unsigned int p, unsigned int nbIndirectCommands, unsigned int stride, RRRCDepthStencilID depthStencilID, RenderStates currentStates) {
                if (needToUpdateDS) {
                    Shader* shader = const_cast<Shader*>(currentStates.shader);
                    currentStates.shader = &sLinkedList;
                    createDescriptorSets(currentStates);
                    currentStates.shader = &sBuildDepthBuffer;
                    createDescriptorSets(currentStates);
                    currentStates.shader = &sBuildAlphaBuffer;
                    createDescriptorSets(currentStates);
                    currentStates.shader = &sReflectRefract;
                    createDescriptorSets(currentStates);
                    currentStates.shader = shader;
                    needToUpdateDS = false;
                }
                Shader* shader = const_cast<Shader*>(currentStates.shader);
                currentStates.blendMode.updateIds();
                if (shader == &sBuildDepthBuffer) {
                    ////////std::cout<<"draw on db"<<std::endl;
                    depthBuffer.beginRecordCommandBuffers();
                    std::vector<VkCommandBuffer> commandBuffers = depthBuffer.getCommandBuffers();
                    unsigned int currentFrame = depthBuffer.getCurrentFrame();
                    buildDepthPC.nbLayers = GameObject::getNbLayers();
                    vkCmdPushConstants(commandBuffers[currentFrame], depthBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(IndirectRenderingPC), &indirectRenderingPC);
                    vkCmdPushConstants(commandBuffers[currentFrame], depthBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_FRAGMENT_BIT, 128, sizeof(BuildDepthPC), &buildDepthPC);
                    depthBuffer.beginRenderPass();
                    depthBuffer.drawIndirect(commandBuffers[currentFrame], currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], vboIndirect, depthStencilID,currentStates);
                    depthBuffer.endRenderPass();
                    std::vector<VkSemaphore> waitSemaphores;
                    waitSemaphores.push_back(offscreenRenderingFinishedSemaphore[depthBuffer.getCurrentFrame()]);
                    std::vector<VkPipelineStageFlags> waitStages;
                    waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                    std::vector<uint64_t> waitValues;
                    waitValues.push_back(values[depthBuffer.getCurrentFrame()]);
                    std::vector<VkSemaphore> signalSemaphores;
                    signalSemaphores.push_back(offscreenDepthPassFinishedSemaphore[depthBuffer.getCurrentFrame()]);
                    std::vector<uint64_t> signalValues;
                    depthBuffer.submit(true, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);

                } else if (shader == &sBuildAlphaBuffer) {
                    alphaBuffer.beginRecordCommandBuffers();
                    const_cast<Texture&>(depthBuffer.getTexture(depthBuffer.getImageIndex())).toShaderReadOnlyOptimal(alphaBuffer.getCommandBuffers()[alphaBuffer.getCurrentFrame()]);


                    std::vector<VkCommandBuffer> commandBuffers = alphaBuffer.getCommandBuffers();
                    unsigned int currentFrame = alphaBuffer.getCurrentFrame();
                    buildAlphaPC.nbLayers = GameObject::getNbLayers();
                    buildAlphaPC.resolution = resolution;

                    vkCmdPushConstants(commandBuffers[currentFrame], alphaBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(IndirectRenderingPC), &indirectRenderingPC);
                    vkCmdPushConstants(commandBuffers[currentFrame], alphaBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_FRAGMENT_BIT, 128, sizeof(BuildAlphaPC), &buildAlphaPC);
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
                    std::vector<VkSemaphore> signalSemaphores, waitSemaphores;
                    std::vector<VkPipelineStageFlags> waitStages;
                    signalSemaphores.push_back(offscreenAlphaPassFinishedSemaphore[alphaBuffer.getCurrentFrame()]);

                    waitSemaphores.push_back(offscreenDepthPassFinishedSemaphore[alphaBuffer.getCurrentFrame()]);
                    waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                    alphaBuffer.submit(true, signalSemaphores, waitSemaphores, waitStages);

                } else if (shader == &sLinkedList) {

                    environmentMap.beginRecordCommandBuffers();
                    std::vector<VkCommandBuffer> commandBuffers = environmentMap.getCommandBuffers();
                    unsigned int currentFrame = environmentMap.getCurrentFrame();



                    //vkCmdWaitEvents(commandBuffers[currentFrame], 1, &events[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                    VkMemoryBarrier memoryBarrier={};
                    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                    memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                    memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                    //vkCmdWaitEvents(commandBuffers[currentFrame], 1, &events[currentFrame], VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                    vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                    environmentMap.beginRenderPass();
                    environmentMap.drawIndirect(commandBuffers[currentFrame], currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], vboIndirect, depthStencilID,currentStates);
                    environmentMap.endRenderPass();
                    std::vector<VkSemaphore> signalSemaphores, waitSemaphores;
                    std::vector<VkPipelineStageFlags> waitStages;
                    std::vector<uint64_t> signalValues, waitValues;
                    waitSemaphores.clear();
                    waitSemaphores.push_back(offscreenRenderingFinishedSemaphore[reflectRefractTex.getCurrentFrame()]);
                    waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                    signalSemaphores.push_back(offscreenRenderingFinishedSemaphore[reflectRefractTex.getCurrentFrame()]);
                    waitValues.push_back(values[reflectRefractTex.getCurrentFrame()]);
                    values[reflectRefractTex.getCurrentFrame()]++;
                    signalValues.push_back(values[reflectRefractTex.getCurrentFrame()]);
                    environmentMap.submit(false, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);


                } else {
                    reflectRefractTex.beginRecordCommandBuffers();
                    const_cast<Texture&>(environmentMap.getTexture(environmentMap.getImageIndex())).toShaderReadOnlyOptimal(reflectRefractTex.getCommandBuffers()[reflectRefractTex.getCurrentFrame()]);
                    const_cast<Texture&>(alphaBuffer.getTexture(alphaBuffer.getImageIndex())).toShaderReadOnlyOptimal(reflectRefractTex.getCommandBuffers()[reflectRefractTex.getCurrentFrame()]);


                    std::vector<VkCommandBuffer> commandBuffers = reflectRefractTex.getCommandBuffers();
                    unsigned int currentFrame = reflectRefractTex.getCurrentFrame();
                    buildFrameBufferPC.resolution = resolution;

                    vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);
                    VkMemoryBarrier memoryBarrier;
                    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                    memoryBarrier.pNext = VK_NULL_HANDLE;
                    memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                    memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                    //vkCmdWaitEvents(commandBuffers[currentFrame], 1, &events[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

                    vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                    vkCmdPushConstants(commandBuffers[currentFrame], reflectRefractTex.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(IndirectRenderingPC), &indirectRenderingPC);
                    vkCmdPushConstants(commandBuffers[currentFrame], reflectRefractTex.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_FRAGMENT_BIT, 128, sizeof(BuildFrameBufferPC), &buildFrameBufferPC);
                    reflectRefractTex.beginRenderPass();
                    reflectRefractTex.drawIndirect(commandBuffers[currentFrame], currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], vboIndirect, depthStencilID,currentStates);
                    reflectRefractTex.endRenderPass();

                    const_cast<Texture&>(alphaBuffer.getTexture(alphaBuffer.getImageIndex())).toColorAttachmentOptimal(reflectRefractTex.getCommandBuffers()[reflectRefractTex.getCurrentFrame()]);
                    const_cast<Texture&>(environmentMap.getTexture(environmentMap.getImageIndex())).toColorAttachmentOptimal(reflectRefractTex.getCommandBuffers()[reflectRefractTex.getCurrentFrame()]);
                    std::vector<VkSemaphore> signalSemaphores, waitSemaphores;
                    std::vector<VkPipelineStageFlags> waitStages;
                    std::vector<uint64_t> signalValues, waitValues;
                    signalSemaphores.push_back(offscreenRenderingFinishedSemaphore[reflectRefractTex.getCurrentFrame()]);
                    waitSemaphores.push_back(offscreenRenderingFinishedSemaphore[reflectRefractTex.getCurrentFrame()]);
                    waitSemaphores.push_back(offscreenAlphaPassFinishedSemaphore[reflectRefractTex.getCurrentFrame()]);
                    waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                    waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                    waitValues.clear();
                    waitValues.push_back(values[reflectRefractTex.getCurrentFrame()]);
                    waitValues.push_back(0);
                    values[reflectRefractTex.getCurrentFrame()]++;
                    signalValues.clear();
                    signalValues.push_back(values[reflectRefractTex.getCurrentFrame()]);
                    reflectRefractTex.submit(true, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);
                    isSomethingDrawn = true;

                }
            }
            void ReflectRefractRenderComponent::recordCommandBufferVertexBuffer(unsigned int currentFrame, RenderStates currentStates, VkCommandBuffer commandBuffer, unsigned int uboOffset) {
                //std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                Shader* shader = const_cast<Shader*>(currentStates.shader);
                currentStates.blendMode.updateIds();
                if (currentStates.shader == &skyboxShader) {
                    std::vector<unsigned int> dynamicOffsets;
                    dynamicOffsets.push_back(uboOffset);
                    environmentMap.drawVertexBuffer(commandBuffer, currentFrame, vb2, RRRCNODEPTHNOSTENCIL, currentStates, dynamicOffsets);
                } else {
                    vkCmdPushConstants(commandBuffer, environmentMap.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + vb.getPrimitiveType()][0][RRRCNODEPTHNOSTENCIL*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(LinkedList2PC), &linkedList2PC);
                    environmentMap.drawVertexBuffer(commandBuffer, currentFrame, vb, RRRCNODEPTHNOSTENCIL, currentStates);
                }
            }
            void ReflectRefractRenderComponent::drawBuffers() {

               unsigned int currentFrame = depthBuffer.getCurrentFrame();
               for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {

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
                if (skybox != nullptr)
                    vb2.updateStagingBuffers(currentFrame);
                vb.updateStagingBuffers(currentFrame);


                VkCommandBufferInheritanceInfo inheritanceInfo{};

                VkCommandBufferBeginInfo beginInfo{};


                inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                inheritanceInfo.renderPass = depthBuffer.getRenderPass(1);
                inheritanceInfo.framebuffer = VK_NULL_HANDLE;
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.pInheritanceInfo = &inheritanceInfo;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
                if (vkBeginCommandBuffer(depthBufferCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }

                RenderStates currentStates;
                currentStates.blendMode = BlendNone;
                currentStates.shader = &sBuildDepthBuffer;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes-1; p++) {
                    if (needToUpdateDSs[p][currentFrame])
                        updateDescriptorSets(currentFrame, p, currentStates);
                    if (nbDrawCommandBuffer[p][0] > 0) {
                        recordCommandBufferIndirect(currentFrame, p, nbDrawCommandBuffer[p][0], sizeof(DrawArraysIndirectCommand), RRRCNODEPTHNOSTENCIL, 0, -1, -1, modelDataOffsets[p][0], materialDataOffsets[p][0],drawCommandBufferOffsets[p][0], currentStates, depthBufferCommandBuffer[currentFrame]);
                    }
                    if (nbIndexedDrawCommandBuffer[p][0] > 0) {
                        recordCommandBufferIndirect(currentFrame, p, nbIndexedDrawCommandBuffer[p][0], sizeof(DrawElementsIndirectCommand), RRRCNODEPTHNOSTENCIL, 0, 0, -1, modelDataOffsets[p][1], materialDataOffsets[p][1],drawIndexedCommandBufferOffsets[p][0], currentStates, depthBufferCommandBuffer[currentFrame]);
                    }
                }
                if (vkEndCommandBuffer(depthBufferCommandBuffer[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }




                inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                inheritanceInfo.renderPass = alphaBuffer.getRenderPass(1);
                inheritanceInfo.framebuffer = VK_NULL_HANDLE;

                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.pInheritanceInfo = &inheritanceInfo;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
                if (vkBeginCommandBuffer(alphaBufferCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }

                currentStates.shader = &sBuildAlphaBuffer;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes-1; p++) {
                    if (needToUpdateDSs[p][currentFrame])
                        updateDescriptorSets(currentFrame, p, currentStates);
                    if (nbDrawCommandBuffer[p][1] > 0) {
                        recordCommandBufferIndirect(currentFrame, p, nbDrawCommandBuffer[p][1], sizeof(DrawArraysIndirectCommand), RRRCNODEPTHNOSTENCIL, 0, -1, -1, modelDataOffsets[p][2], materialDataOffsets[p][2],drawCommandBufferOffsets[p][1], currentStates, alphaBufferCommandBuffer[currentFrame]);
                    }
                    if (nbIndexedDrawCommandBuffer[p][1] > 0) {
                        ////std::cout<<"offsets : "<<modelDataOffsets[p][3]<<","<<materialDataOffsets[p][3]<<std::endl;
                        recordCommandBufferIndirect(currentFrame, p, nbIndexedDrawCommandBuffer[p][1], sizeof(DrawElementsIndirectCommand), RRRCNODEPTHNOSTENCIL, 0, 0, -1, modelDataOffsets[p][3], materialDataOffsets[p][3],drawIndexedCommandBufferOffsets[p][1], currentStates, alphaBufferCommandBuffer[currentFrame]);
                    }

                }
                if (vkEndCommandBuffer(alphaBufferCommandBuffer[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }
                if (skybox != nullptr) {
                    currentStates.shader = &skyboxShader;
                    for (unsigned int i = 0; i < nbReflRefrEntities; i++) {

                        inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                        inheritanceInfo.renderPass = environmentMap.getRenderPass(1);
                        inheritanceInfo.framebuffer = VK_NULL_HANDLE;
                        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                        beginInfo.pInheritanceInfo = &inheritanceInfo;
                        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
                        vkResetCommandBuffer(skyboxCommandBuffer[i][currentFrame], 0);
                        if (vkBeginCommandBuffer(skyboxCommandBuffer[i][currentFrame], &beginInfo) != VK_SUCCESS) {
                            throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                        }
                        recordCommandBufferVertexBuffer(currentFrame, currentStates, skyboxCommandBuffer[i][currentFrame], i * alignUBO(sizeof(UniformBufferObject)));
                        if (vkEndCommandBuffer(skyboxCommandBuffer[i][currentFrame]) != VK_SUCCESS) {
                            throw core::Erreur(0, "failed to record command buffer!", 1);
                        }

                    }
                }
                currentStates.shader = &sLinkedList;
                for (unsigned int i = 0; i < nbReflRefrEntities; i++) {

                    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                    inheritanceInfo.renderPass = environmentMap.getRenderPass(1);
                    inheritanceInfo.framebuffer = VK_NULL_HANDLE;

                    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                    beginInfo.pInheritanceInfo = &inheritanceInfo;
                    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
                    if (vkBeginCommandBuffer(environmentMapCommandBuffer[i][currentFrame], &beginInfo) != VK_SUCCESS) {
                        throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                    }

                    for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes-1; p++) {
                        if (needToUpdateDSs[p][currentFrame] && i == 0)
                            updateDescriptorSets(currentFrame, p, currentStates);
                        if (nbDrawCommandBuffer[p][1] > 0) {
                            recordCommandBufferIndirect(currentFrame, p, nbDrawCommandBuffer[p][1], sizeof(DrawArraysIndirectCommand), RRRCNODEPTHNOSTENCIL, 0, -1, i * alignUBO(sizeof(UniformBufferObject)), modelDataOffsets[p][2], materialDataOffsets[p][2],drawCommandBufferOffsets[p][1], currentStates, environmentMapCommandBuffer[i][currentFrame]);
                        }
                        if (nbIndexedDrawCommandBuffer[p][1] > 0) {
                            recordCommandBufferIndirect(currentFrame, p, nbIndexedDrawCommandBuffer[p][1], sizeof(DrawElementsIndirectCommand), RRRCNODEPTHNOSTENCIL, 0, 0,  i * alignUBO(sizeof(UniformBufferObject)), modelDataOffsets[p][3], materialDataOffsets[p][3],drawIndexedCommandBufferOffsets[p][1], currentStates, environmentMapCommandBuffer[i][currentFrame]);
                        }

                    }
                    if (vkEndCommandBuffer(environmentMapCommandBuffer[i][currentFrame]) != VK_SUCCESS) {
                        throw core::Erreur(0, "failed to record command buffer!", 1);
                    }
                }


                currentStates.shader = &sLinkedList2;
                inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                inheritanceInfo.renderPass = environmentMap.getRenderPass(1);
                inheritanceInfo.framebuffer = VK_NULL_HANDLE;

                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.pInheritanceInfo = &inheritanceInfo;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
                if (vkBeginCommandBuffer(environmentMapPass2CommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }
                currentStates.blendMode=BlendAlpha;
                recordCommandBufferVertexBuffer(currentFrame, currentStates, environmentMapPass2CommandBuffer[currentFrame]);
                if (vkEndCommandBuffer(environmentMapPass2CommandBuffer[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }
                currentStates.blendMode=BlendNone;
                currentStates.shader = &sReflectRefract;
                for (unsigned int i = 0; i < nbReflRefrEntities; i++) {


                    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                    inheritanceInfo.renderPass = reflectRefractTex.getRenderPass(1);
                    inheritanceInfo.framebuffer = VK_NULL_HANDLE;

                    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                    beginInfo.pInheritanceInfo = &inheritanceInfo;
                    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
                    if (vkBeginCommandBuffer(reflectRefractCommandBuffer[i][currentFrame], &beginInfo) != VK_SUCCESS) {
                        throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                    }

                    for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes-1; p++) {
                        if (needToUpdateDSs[p][currentFrame] && i == 0)
                            updateDescriptorSets(currentFrame, p, currentStates);
                        if (nbDrawCommandBuffer[p][i+2] > 0) {
                            recordCommandBufferIndirect(currentFrame, p, nbDrawCommandBuffer[p][i+2], sizeof(DrawArraysIndirectCommand), RRRCDEPTHNOSTENCIL, 0, -1, -1, modelDataOffsets[p][i*2+4], materialDataOffsets[p][i*2+4],drawCommandBufferOffsets[p][i+2], currentStates, reflectRefractCommandBuffer[i][currentFrame]);
                        }
                        if (nbIndexedDrawCommandBuffer[p][i+2] > 0) {
                            recordCommandBufferIndirect(currentFrame, p, nbIndexedDrawCommandBuffer[p][i+2], sizeof(DrawElementsIndirectCommand), RRRCDEPTHNOSTENCIL, 0, 0, -1, modelDataOffsets[p][i*2+5], materialDataOffsets[p][i*2+5],drawIndexedCommandBufferOffsets[p][i+2], currentStates, reflectRefractCommandBuffer[i][currentFrame]);
                        }
                    }
                    if (vkEndCommandBuffer(reflectRefractCommandBuffer[i][currentFrame]) != VK_SUCCESS) {
                        throw core::Erreur(0, "failed to record command buffer!", 1);
                    }
                }
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes-1; p++)
                    needToUpdateDSs[p][currentFrame] = false;
            }
            void ReflectRefractRenderComponent::createCommandBufferVertexBuffer(RenderStates currentStates) {
                environmentMap.beginRecordCommandBuffers();

                Shader* shader = const_cast<Shader*>(currentStates.shader);
                unsigned int currentFrame = environmentMap.getCurrentFrame();
                currentStates.blendMode.updateIds();
                std::vector<VkCommandBuffer> commandBuffers = environmentMap.getCommandBuffers();
                /*vkCmdResetEvent(commandBuffers[currentFrame], events[currentFrame],  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                vkCmdSetEvent(commandBuffers[currentFrame], events[currentFrame],  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);*/
                vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);


                VkMemoryBarrier memoryBarrier;
                memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                memoryBarrier.pNext = VK_NULL_HANDLE;
                memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;



                vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                vkCmdPushConstants(commandBuffers[currentFrame], environmentMap.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + vb.getPrimitiveType()][0][RRRCNODEPTHNOSTENCIL*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(LinkedList2PC), &linkedList2PC);


                ////////std::cout<<"ids : "<<shader->getId() * (Batcher::nbPrimitiveTypes - 1) + vb.getPrimitiveType()<<","<<environmentMap.getId()<<","<<RRRCNODEPTHNOSTENCIL<<std::endl;
                environmentMap.beginRenderPass();
                environmentMap.drawVertexBuffer(commandBuffers[currentFrame], currentFrame, vb, RRRCNODEPTHNOSTENCIL, currentStates);
                environmentMap.endRenderPass();
                std::vector<VkSemaphore> signalSemaphores, waitSemaphores;
                std::vector<VkPipelineStageFlags> waitStages;
                std::vector<uint64_t> signalValues, waitValues;
                waitSemaphores.push_back(offscreenRenderingFinishedSemaphore[reflectRefractTex.getCurrentFrame()]);
                waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                signalSemaphores.push_back(offscreenRenderingFinishedSemaphore[reflectRefractTex.getCurrentFrame()]);
                waitValues.push_back(values[reflectRefractTex.getCurrentFrame()]);
                values[reflectRefractTex.getCurrentFrame()]++;
                signalValues.push_back(values[reflectRefractTex.getCurrentFrame()]);
                environmentMap.submit(true, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);


            }
            unsigned int ReflectRefractRenderComponent::align(unsigned int offset) {
                ////std::cout << "alignment = " << alignment << std::endl;
                return (offset + alignment - 1) & ~(alignment - 1);
            }
            unsigned int ReflectRefractRenderComponent::alignUBO(unsigned int offset) {
                return (offset + uboAlignment - 1) & ~(uboAlignment - 1);
            }
            void ReflectRefractRenderComponent::loadSkybox(Entity* skybox) {
                this->skybox = skybox;
            }
            void ReflectRefractRenderComponent::fillBufferReflMT() {


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
                    vertexOffsets[p].push_back(totalVertexCount[p] * sizeof(Vertex));
                    drawCommandCount[p] = 0;
                    oldTotalVertexCount[p] = totalVertexCount[p];
                }

                for (unsigned int i = 0; i < m_reflNormals.size(); i++) {
                    if (m_reflNormals[i].getAllVertices().getVertexCount() > 0) {
                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        unsigned int p = m_reflNormals[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_reflNormals[i].getMaterial().getTexture() != nullptr) ? m_reflNormals[i].getMaterial().getTexture()->getId() : 0;
                        material.layer = m_reflNormals[i].getMaterial().getLayer();
                        material.uvScale = (m_reflNormals[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_reflNormals[i].getMaterial().getTexture()->getSize().x(), 1.f / m_reflNormals[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        materialDatas[p].push_back(material);
                        ModelData model;
                        TransformMatrix tm;
                        model.worldMat = toVulkanMatrix(tm.getMatrix())/*.transpose()*/;
                        modelDatas[p].push_back(model);
                        unsigned int vertexCount = 0;
                        for (unsigned int j = 0; j < m_reflNormals[i].getAllVertices().getVertexCount(); j++) {
                            vertexCount++;
                            vbBindlessTex[p].append(m_reflNormals[i].getAllVertices()[j]);
                        }
                        drawArraysIndirectCommand.count = vertexCount;
                        drawArraysIndirectCommand.firstIndex = firstIndex[p] + oldTotalVertexCount[p];
                        drawArraysIndirectCommand.baseInstance = baseInstance[p];
                        drawArraysIndirectCommand.instanceCount = 1;
                        drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                        firstIndex[p] += vertexCount;
                        totalVertexCount[p] += vertexCount;
                        baseInstance[p] += 1;
                        drawCommandCount[p]++;
                    }
                }
                for (unsigned int i = 0; i < m_reflInstances.size(); i++) {
                    if (m_reflInstances[i].getAllVertices().getVertexCount() > 0) {
                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        unsigned int p = m_reflInstances[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_reflInstances[i].getMaterial().getTexture() != nullptr) ? m_reflInstances[i].getMaterial().getTexture()->getId() : 0;
                        material.layer = m_reflInstances[i].getMaterial().getLayer();
                        material.uvScale = (m_reflInstances[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_reflInstances[i].getMaterial().getTexture()->getSize().x(), 1.f / m_reflInstances[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        materialDatas[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_reflInstances[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            ModelData model;
                            model.worldMat = toVulkanMatrix(tm[j]->getMatrix())/*.transpose()*/;

                            modelDatas[p].push_back(model);
                        }
                        ////////std::cout<<"prim type : "<<p<<std::endl<<"model datas size : "<<modelDatas[p].size()<<std::endl;
                        unsigned int vertexCount = 0;

                        if (m_reflInstances[i].getEntities().size() > 0) {
                            Entity* firstInstance = m_reflInstances[i].getEntities()[0];
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
                        totalVertexCount[p] += vertexCount;
                        baseInstance[p] += tm.size();
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

            void ReflectRefractRenderComponent::fillBufferReflIndexedMT() {

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
                    vertexIndexOffsets[p].push_back(totalVertexIndexCount[p] * sizeof(Vertex));
                    indexOffsets[p].push_back(totalIndexCount[p] * sizeof(unsigned int));
                    drawCommandCount[p] = 0;
                    oldTotalVertexIndexCount[p] = totalVertexIndexCount[p];
                    oldTotalIndexCount[p] = totalIndexCount[p];
                }


                for (unsigned int i = 0; i < m_reflNormalIndexed.size(); i++) {
                    if (m_reflNormalIndexed[i].getAllVertices().getVertexCount() > 0) {
                        DrawElementsIndirectCommand drawElementsIndirectCommand;
                        unsigned int p = m_reflNormalIndexed[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_reflNormalIndexed[i].getMaterial().getTexture() != nullptr) ? m_reflNormalIndexed[i].getMaterial().getTexture()->getId() : 0;
                        material.layer = m_reflNormalIndexed[i].getMaterial().getLayer();
                        material.uvScale = (m_reflNormalIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_reflNormalIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_reflNormalIndexed[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        materialDatas[p].push_back(material);
                        ModelData model;
                        TransformMatrix tm;
                        model.worldMat = toVulkanMatrix(tm.getMatrix())/*.transpose()*/;

                        modelDatas[p].push_back(model);
                        unsigned int vertexCount = 0, indexCount = 0;
                        for (unsigned int j = 0; j < m_reflNormalIndexed[i].getAllVertices().getVertexCount(); j++) {
                            vertexCount++;
                            vbBindlessTexIndexed[p].append(m_reflNormalIndexed[i].getAllVertices()[j]);
                        }
                        for (unsigned int j = 0; j < m_reflNormalIndexed[i].getAllVertices().getIndexes().size(); j++) {
                            indexCount++;
                            vbBindlessTexIndexed[p].addIndex(m_reflNormalIndexed[i].getAllVertices().getIndexes()[j]);
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
                for (unsigned int i = 0; i < m_reflIndexed.size(); i++) {
                    if (m_reflIndexed[i].getAllVertices().getVertexCount() > 0) {
                        DrawElementsIndirectCommand drawElementsIndirectCommand;
                        unsigned int p = m_reflIndexed[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_reflIndexed[i].getMaterial().getTexture() != nullptr) ? m_reflIndexed[i].getMaterial().getTexture()->getId() : 0;
                        material.layer = m_reflIndexed[i].getMaterial().getLayer();
                        material.uvScale = (m_reflIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_reflIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_reflIndexed[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        materialDatas[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_reflIndexed[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            ModelData model;
                            model.worldMat = toVulkanMatrix(tm[j]->getMatrix())/*.transpose()*/;

                            modelDatas[p].push_back(model);
                        }
                        ////////std::cout<<"prim type : "<<p<<std::endl<<"model datas size : "<<modelDatas[p].size()<<std::endl;
                        unsigned int vertexCount = 0, indexCount = 0;

                        if (m_reflIndexed[i].getEntities().size() > 0) {
                            Entity* firstInstance = m_reflIndexed[i].getEntities()[0];
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
            void ReflectRefractRenderComponent::fillNonReflBufferMT() {
                std::array<unsigned int, Batcher::nbPrimitiveTypes> firstIndex, baseInstance;
                for (unsigned int i = 0; i < firstIndex.size(); i++) {
                    firstIndex[i] = 0;
                }
                for (unsigned int i = 0; i < baseInstance.size(); i++) {
                    baseInstance[i] = 0;
                }
                /*for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    currentModelOffset[p] = 0;
                    previousModelOffset[p] = 0;
                    currentMaterialOffset[p] = 0;
                    previousMaterialOffset[p] = 0;
                }*/
                std::array<unsigned int, Batcher::nbPrimitiveTypes> drawCommandCount, oldTotalVertexCount;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    drawCommandBufferOffsets[p].push_back(totalBufferSizeDrawCommand[p]);
                    vertexOffsets[p].push_back(totalVertexCount[p] * sizeof(Vertex));
                    drawCommandCount[p] = 0;
                    oldTotalVertexCount[p] = totalVertexCount[p];
                }
                for (unsigned int i = 0; i < m_normals.size(); i++) {
                    if (m_normals[i].getAllVertices().getVertexCount() > 0) {
                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        ////////std::cout<<"layer : "<<layer<<" nb layers : "<<Entity::getNbLayers()<<std::endl;
                        unsigned int p = m_normals[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_normals[i].getMaterial().getTexture() != nullptr) ? m_normals[i].getMaterial().getTexture()->getId() : 0;
                        material.layer = m_normals[i].getMaterial().getLayer();
                        material.uvScale = (m_normals[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_normals[i].getMaterial().getTexture()->getSize().x(), 1.f / m_normals[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        materialDatas[p].push_back(material);
                        ModelData model;
                        TransformMatrix tm;
                        model.worldMat = toVulkanMatrix(tm.getMatrix())/*.transpose()*/;

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
                        drawCommandCount[p]++;
                        totalVertexCount[p] += vertexCount;
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
                            model.worldMat = toVulkanMatrix(tm[j]->getMatrix())/*.transpose()*/;

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
                        drawCommandCount[p]++;
                        totalVertexCount[p] += vertexCount;
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
            void ReflectRefractRenderComponent::fillNonReflIndexedBufferMT() {
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
                /*for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    currentModelOffset[p] = 0;
                    previousModelOffset[p] = 0;
                    currentMaterialOffset[p] = 0;
                    previousMaterialOffset[p] = 0;
                }*/
                std::array<unsigned int, Batcher::nbPrimitiveTypes> drawCommandCount, oldTotalVertexIndexCount, oldTotalIndexCount;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    drawIndexedCommandBufferOffsets[p].push_back(totalBufferSizeIndexedDrawCommand[p]);
                    vertexIndexOffsets[p].push_back(totalVertexIndexCount[p] * sizeof(Vertex));
                    indexOffsets[p].push_back(totalIndexCount[p] * sizeof(unsigned int));
                    drawCommandCount[p] = 0;
                    oldTotalVertexIndexCount[p] = totalVertexIndexCount[p];
                    oldTotalIndexCount[p] = totalIndexCount[p];
                }

                for (unsigned int i = 0; i < m_normalIndexed.size(); i++) {
                    if (m_normalIndexed[i].getAllVertices().getVertexCount() > 0) {
                        DrawElementsIndirectCommand drawElementsIndirectCommand;
                        unsigned int p = m_normalIndexed[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        {
                            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                            material.textureIndex = (m_normalIndexed[i].getMaterial().getTexture() != nullptr) ? m_normalIndexed[i].getMaterial().getTexture()->getId() : 0;
                            material.layer = m_normalIndexed[i].getMaterial().getLayer();
                            material.uvScale = (m_normalIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_normalIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_normalIndexed[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                            material.uvOffset = math::Vec2f(0, 0);
                        }
                        materialDatas[p].push_back(material);
                        ModelData model;
                        TransformMatrix tm;
                        model.worldMat = toVulkanMatrix(tm.getMatrix())/*.transpose()*/;

                        modelDatas[p].push_back(model);
                        unsigned int vertexCount = 0, indexCount = 0;
                        for (unsigned int j = 0; j < m_normalIndexed[i].getAllVertices().getVertexCount(); j++) {
                            vertexCount++;
                            vbBindlessTexIndexed[p].append(m_normalIndexed[i].getAllVertices()[j]);
                        }
                        for (unsigned int j = 0; j < m_normalIndexed[i].getAllVertices().getIndexes().size(); j++) {
                            ////std::cout<<"add norm indexed"<<std::endl;
                            indexCount++;
                            vbBindlessTexIndexed[p].addIndex(m_normalIndexed[i].getAllVertices().getIndexes()[j]);
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
                        totalVertexIndexCount[p] += vertexCount;
                        totalIndexCount[p] += indexCount;
                        drawCommandCount[p]++;
                    }
                }
                for (unsigned int i = 0; i < m_indexed.size(); i++) {
                    if (m_indexed[i].getAllVertices().getVertexCount() > 0) {
                        DrawElementsIndirectCommand drawElementsIndirectCommand;
                        unsigned int p = m_indexed[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_indexed[i].getMaterial().getTexture() != nullptr) ? m_indexed[i].getMaterial().getTexture()->getId() : 0;
                        material.layer = m_indexed[i].getMaterial().getLayer();
                        material.uvScale = (m_indexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_indexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_indexed[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        materialDatas[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_indexed[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            ModelData model;
                            model.worldMat = toVulkanMatrix(tm[j]->getMatrix())/*.transpose()*/;

                            modelDatas[p].push_back(model);
                        }
                        ////////std::cout<<"prim type : "<<p<<std::endl<<"model datas size : "<<modelDatas[p].size()<<std::endl;
                        unsigned int vertexCount = 0, indexCount = 0;

                        if (m_indexed[i].getEntities().size() > 0) {
                            Entity* firstInstance = m_indexed[i].getEntities()[0];
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
                        totalVertexIndexCount[p] += vertexCount;
                        totalIndexCount[p] += indexCount;
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

                        /*vkMapMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemory, 0, totalBufferSizeIndexedDrawCommand[p], 0, &data);
                        memcpy(data, drawElementsIndirectCommands[p].data(), (size_t)totalBufferSizeIndexedDrawCommand[p]);
                        vkUnmapMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemory);*/
                        //copyBuffer(vboIndexedIndirectStagingBuffer, drawCommandBufferIndexedMT[p], totalBufferSizeIndexedDrawCommand[p], copyDrawIndexedBufferCommandBuffer);

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
            void ReflectRefractRenderComponent::fillReflEntityBufferMT(Entity* reflectEntity) {
                std::array<unsigned int, Batcher::nbPrimitiveTypes> firstIndex, baseInstance;
                for (unsigned int i = 0; i < firstIndex.size(); i++) {
                    firstIndex[i] = 0;
                }
                for (unsigned int i = 0; i < baseInstance.size(); i++) {
                    baseInstance[i] = 0;
                }
                /*for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    currentModelOffset[p] = 0;
                    previousModelOffset[p] = 0;
                    currentMaterialOffset[p] = 0;
                    previousMaterialOffset[p] = 0;
                }*/
                std::array<unsigned int, Batcher::nbPrimitiveTypes> drawCommandCount, oldTotalVertexCount;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    drawCommandBufferOffsets[p].push_back(totalBufferSizeDrawCommand[p]);
                    vertexOffsets[p].push_back(totalVertexCount[p] * sizeof(Vertex));
                    drawCommandCount[p] = 0;
                    oldTotalVertexCount[p] = totalVertexCount[p];
                }
                for (unsigned int i = 0; i < m_reflNormals.size(); i++) {
                    if (m_reflNormals[i].getVertexArrays().size() > 0) {
                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        unsigned int p = m_reflNormals[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_reflNormals[i].getMaterial().getTexture() != nullptr) ? m_reflNormals[i].getMaterial().getTexture()->getId() : 0;
                        material.materialType = m_reflNormals[i].getMaterial().getType();
                        material.uvScale = (m_reflNormals[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_reflNormals[i].getMaterial().getTexture()->getSize().x(), 1.f / m_reflNormals[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        materialDatas[p].push_back(material);
                        ModelData model;
                        TransformMatrix tm;
                        model.worldMat = toVulkanMatrix(tm.getMatrix())/*.transpose()*/;

                        modelDatas[p].push_back(model);
                        unsigned int vertexCount = 0;
                        for (unsigned int j = 0; j < m_reflNormals[i].getVertexArrays().size(); j++) {
                            Entity* entity = m_reflNormals[i].getVertexArrays()[j]->getEntity()->getRootEntity();
                            if (entity == reflectEntity) {
                                for (unsigned int k = 0; k < m_reflNormals[i].getVertexArrays()[j]->getVertexCount(); k++) {

                                    vertexCount++;
                                    math::Vec3f t = m_reflNormals[i].getVertexArrays()[j]->getEntity()->getTransform().transform(math::Vec4f((*m_reflNormals[i].getVertexArrays()[j])[k].position));
                                    Vertex v (t, (*m_reflNormals[i].getVertexArrays()[j])[k].color, (*m_reflNormals[i].getVertexArrays()[j])[k].texCoords);
                                    v.normal = (*m_reflNormals[i].getVertexArrays()[j])[k].normal;
                                    vbBindlessTex[p].append(v);
                                }
                            }
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
                for (unsigned int i = 0; i < m_reflInstances.size(); i++) {
                    if (m_reflInstances[i].getAllVertices().getVertexCount() > 0) {
                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        unsigned int p = m_reflInstances[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_reflInstances[i].getMaterial().getTexture() != nullptr) ? m_reflInstances[i].getMaterial().getTexture()->getId() : 0;
                        material.materialType = m_reflInstances[i].getMaterial().getType();
                        material.uvScale = (m_reflInstances[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_reflInstances[i].getMaterial().getTexture()->getSize().x(), 1.f / m_reflInstances[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        materialDatas[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_reflInstances[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            ModelData model;
                            model.worldMat = toVulkanMatrix(tm[j]->getMatrix())/*.transpose()*/;

                            modelDatas[p].push_back(model);
                        }
                        unsigned int vertexCount = 0;
                        if (m_reflInstances[i].getVertexArrays().size() > 0) {
                            Entity* entity = m_reflInstances[i].getVertexArrays()[0]->getEntity();
                            for (unsigned int j = 0; j < m_reflInstances[i].getVertexArrays().size(); j++) {
                                if (entity == m_reflInstances[i].getVertexArrays()[j]->getEntity() && entity->getRootEntity() == reflectEntity) {
                                    for (unsigned int k = 0; k < m_reflInstances[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                        vertexCount++;
                                        vbBindlessTex[p].append((*m_reflInstances[i].getVertexArrays()[j])[k]);
                                    }
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

                    if (nbDrawCommandBuffer[p][nbDrawCommandBuffer[p].size()-1] > 0) {
                        //std::cout<<"create buffers"<<std::endl;

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
            void ReflectRefractRenderComponent::fillIndexedReflEntityBufferMT(Entity* reflectEntity) {
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
                /*for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    currentModelOffset[p] = 0;
                    previousModelOffset[p] = 0;
                    currentMaterialOffset[p] = 0;
                    previousMaterialOffset[p] = 0;
                }*/
                std::array<unsigned int, Batcher::nbPrimitiveTypes> drawCommandCount, oldTotalVertexIndexCount, oldTotalIndexCount;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    drawIndexedCommandBufferOffsets[p].push_back(totalBufferSizeIndexedDrawCommand[p]);
                    vertexIndexOffsets[p].push_back(totalVertexIndexCount[p] * sizeof(Vertex));
                    indexOffsets[p].push_back(totalIndexCount[p] * sizeof(unsigned int));
                    drawCommandCount[p] = 0;
                    oldTotalVertexIndexCount[p] = totalVertexCount[p];
                    oldTotalIndexCount[p] = totalIndexCount[p];
                }
                for (unsigned int i = 0; i < m_reflNormalIndexed.size(); i++) {
                    if (m_reflNormalIndexed[i].getAllVertices().getVertexCount() > 0) {
                        DrawElementsIndirectCommand drawElementsIndirectCommand;
                        unsigned int p = m_reflNormalIndexed[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_reflNormalIndexed[i].getMaterial().getTexture() != nullptr) ? m_reflNormalIndexed[i].getMaterial().getTexture()->getId() : 0;
                        material.materialType = m_reflNormalIndexed[i].getMaterial().getType();
                        material.uvScale = (m_reflNormalIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_reflNormalIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_reflNormalIndexed[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        materialDatas[p].push_back(material);
                        ModelData model;
                        TransformMatrix tm;
                        model.worldMat = toVulkanMatrix(tm.getMatrix())/*.transpose()*/;

                        modelDatas[p].push_back(model);
                        unsigned int vertexCount = 0, indexCount = 0;
                        for (unsigned int j = 0; j < m_reflNormalIndexed[i].getVertexArrays().size(); j++) {
                            Entity* entity = m_reflNormalIndexed[i].getVertexArrays()[j]->getEntity()->getRootEntity();
                            if (entity == reflectEntity) {
                                for (unsigned int k = 0; k < m_reflNormalIndexed[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                    vertexCount++;
                                    math::Vec3f t = m_reflNormalIndexed[i].getVertexArrays()[j]->getEntity()->getTransform().transform(math::Vec4f((*m_reflNormalIndexed[i].getVertexArrays()[j])[k].position));
                                    Vertex v (t, (*m_reflNormalIndexed[i].getVertexArrays()[j])[k].color, (*m_reflNormalIndexed[i].getVertexArrays()[j])[k].texCoords);
                                    v.normal = (*m_reflNormalIndexed[i].getVertexArrays()[j])[k].normal;
                                    vbBindlessTexIndexed[p].append(v);
                                }
                                for (unsigned int k = 0; k < m_reflNormalIndexed[i].getVertexArrays()[j]->getIndexes().size(); k++) {
                                    ////std::cout<<"add refl norm indexed"<<std::endl;
                                    indexCount++;
                                    vbBindlessTexIndexed[p].addIndex(m_reflNormalIndexed[i].getVertexArrays()[j]->getIndexes()[j]);
                                }
                            }
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
                        totalVertexIndexCount[p] += vertexCount;
                        totalIndexCount[p] += indexCount;
                        drawCommandCount[p]++;
                    }
                }
                for (unsigned int i = 0; i < m_reflIndexed.size(); i++) {
                    if (m_reflIndexed[i].getAllVertices().getVertexCount() > 0) {
                        DrawElementsIndirectCommand drawElementsIndirectCommand;
                        unsigned int p = m_reflIndexed[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_reflIndexed[i].getMaterial().getTexture() != nullptr) ? m_reflIndexed[i].getMaterial().getTexture()->getId() : 0;
                        material.materialType = m_reflIndexed[i].getMaterial().getType();
                        material.uvScale = (m_reflIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_reflIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_reflIndexed[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        materialDatas[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_reflIndexed[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            ModelData model;
                            model.worldMat = toVulkanMatrix(tm[j]->getMatrix())/*.transpose()*/;

                            modelDatas[p].push_back(model);
                        }
                        ////////std::cout<<"prim type : "<<p<<std::endl<<"model datas size : "<<modelDatas[p].size()<<std::endl;
                        unsigned int vertexCount = 0, indexCount = 0;

                        if (m_reflIndexed[i].getVertexArrays().size() > 0) {
                            Entity* entity = m_reflIndexed[i].getVertexArrays()[0]->getEntity();

                            for (unsigned int j = 0; j < m_reflIndexed[i].getVertexArrays().size(); j++) {

                                if (entity == m_reflIndexed[i].getVertexArrays()[j]->getEntity() && entity->getRootEntity() == reflectEntity) {

                                    unsigned int p = m_reflIndexed[i].getVertexArrays()[j]->getPrimitiveType();
                                    for (unsigned int k = 0; k < m_reflIndexed[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                        ////std::cout<<"add refl inst vert"<<std::endl;
                                        vertexCount++;
                                        vbBindlessTexIndexed[p].append((*m_reflIndexed[i].getVertexArrays()[j])[k]);
                                    }
                                    for (unsigned int k = 0; k < m_reflIndexed[i].getVertexArrays()[j]->getIndexes().size(); k++) {
                                        ////std::cout<<"add refl inst indexed"<<std::endl;
                                        indexCount++;
                                        vbBindlessTexIndexed[p].addIndex(m_reflIndexed[i].getVertexArrays()[j]->getIndexes()[k]);
                                    }
                                }
                            }
                        }
                        drawElementsIndirectCommand.indexCount = indexCount;
                        drawElementsIndirectCommand.firstIndex = firstIndex[p] + oldTotalIndexCount[p];
                        drawElementsIndirectCommand.baseInstance = baseInstance[p] + oldTotalVertexIndexCount[p];
                        drawElementsIndirectCommand.baseVertex = baseVertex[p];
                        drawElementsIndirectCommand.instanceCount = tm.size();
                        drawElementsIndirectCommands[p].push_back(drawElementsIndirectCommand);
                        firstIndex[p] += indexCount;
                        baseVertex[p] += vertexCount;
                        baseInstance[p] += tm.size();
                        totalVertexIndexCount[p] += vertexCount;
                        totalIndexCount[p] += indexCount;
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
                    if (nbIndexedDrawCommandBuffer[p][nbIndexedDrawCommandBuffer[p].size()-1] > 0) {

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
            void ReflectRefractRenderComponent::drawDepthReflInst() {

                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    vbBindlessTex[p].clear();
                    materialDatas[p].clear();
                    modelDatas[p].clear();
                }
                std::array<std::vector<DrawArraysIndirectCommand>, Batcher::nbPrimitiveTypes> drawArraysIndirectCommands;
                std::array<unsigned int, Batcher::nbPrimitiveTypes> firstIndex, baseInstance;
                for (unsigned int i = 0; i < firstIndex.size(); i++) {
                    firstIndex[i] = 0;
                }
                for (unsigned int i = 0; i < baseInstance.size(); i++) {
                    baseInstance[i] = 0;
                }
                for (unsigned int i = 0; i < m_reflNormals.size(); i++) {
                    if (m_reflNormals[i].getAllVertices().getVertexCount() > 0) {
                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        unsigned int p = m_reflNormals[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_reflNormals[i].getMaterial().getTexture() != nullptr) ? m_reflNormals[i].getMaterial().getTexture()->getId() : 0;
                        material.layer = m_reflNormals[i].getMaterial().getLayer();
                        material.uvScale = (m_reflNormals[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_reflNormals[i].getMaterial().getTexture()->getSize().x(), 1.f / m_reflNormals[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        materialDatas[p].push_back(material);
                        ModelData model;
                        TransformMatrix tm;
                        model.worldMat = toVulkanMatrix(tm.getMatrix())/*.transpose()*/;
                        modelDatas[p].push_back(model);
                        unsigned int vertexCount = 0;
                        for (unsigned int j = 0; j < m_reflNormals[i].getAllVertices().getVertexCount(); j++) {
                            vertexCount++;
                            vbBindlessTex[p].append(m_reflNormals[i].getAllVertices()[j]);
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
                for (unsigned int i = 0; i < m_reflInstances.size(); i++) {
                    if (m_reflInstances[i].getAllVertices().getVertexCount() > 0) {
                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        unsigned int p = m_reflInstances[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_reflInstances[i].getMaterial().getTexture() != nullptr) ? m_reflInstances[i].getMaterial().getTexture()->getId() : 0;
                        material.layer = m_reflInstances[i].getMaterial().getLayer();
                        material.uvScale = (m_reflInstances[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_reflInstances[i].getMaterial().getTexture()->getSize().x(), 1.f / m_reflInstances[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        materialDatas[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_reflInstances[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            ModelData model;
                            model.worldMat = toVulkanMatrix(tm[j]->getMatrix())/*.transpose()*/;
                            modelDatas[p].push_back(model);
                        }
                        ////////std::cout<<"prim type : "<<p<<std::endl<<"model datas size : "<<modelDatas[p].size()<<std::endl;
                        unsigned int vertexCount = 0;

                        if (m_reflInstances[i].getVertexArrays().size() > 0) {
                            Entity* entity = m_reflInstances[i].getVertexArrays()[0]->getEntity();

                            for (unsigned int j = 0; j < m_reflInstances[i].getVertexArrays().size(); j++) {

                                if (entity == m_reflInstances[i].getVertexArrays()[j]->getEntity()) {

                                    unsigned int p = m_reflInstances[i].getVertexArrays()[j]->getPrimitiveType();
                                    for (unsigned int k = 0; k < m_reflInstances[i].getVertexArrays()[j]->getVertexCount(); k++) {

                                        vertexCount++;
                                        vbBindlessTex[p].append((*m_reflInstances[i].getVertexArrays()[j])[k]);
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
                currentStates.shader = &sBuildDepthBuffer;
                currentStates.texture = nullptr;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        vbBindlessTex[p].update();
                        VkDeviceSize bufferSize = sizeof(ModelData) * modelDatas[p].size();
                        ////////std::cout<<"prim type : "<<p<<std::endl<<"model datas size : "<<modelDatas[p].size()<<std::endl;
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

                        createCommandBuffersIndirect(p, drawArraysIndirectCommands[p].size(), sizeof(DrawArraysIndirectCommand), RRRCNODEPTHNOSTENCIL, currentStates);
                    }
                }
            }
            void ReflectRefractRenderComponent::drawDepthReflIndexedInst() {

                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    vbBindlessTex[p].clear();
                    materialDatas[p].clear();
                    modelDatas[p].clear();
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
                for (unsigned int i = 0; i < m_reflNormalIndexed.size(); i++) {
                    if (m_reflNormalIndexed[i].getAllVertices().getVertexCount() > 0) {
                        DrawElementsIndirectCommand drawElementsIndirectCommand;
                        unsigned int p = m_reflNormalIndexed[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_reflNormalIndexed[i].getMaterial().getTexture() != nullptr) ? m_reflNormalIndexed[i].getMaterial().getTexture()->getId() : 0;
                        material.layer = m_reflNormalIndexed[i].getMaterial().getLayer();
                        material.uvScale = (m_reflNormalIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_reflNormalIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_reflNormalIndexed[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        materialDatas[p].push_back(material);
                        ModelData model;
                        TransformMatrix tm;
                        model.worldMat = toVulkanMatrix(tm.getMatrix())/*.transpose()*/;
                        modelDatas[p].push_back(model);
                        unsigned int vertexCount = 0, indexCount = 0;
                        for (unsigned int j = 0; j < m_reflNormalIndexed[i].getAllVertices().getVertexCount(); j++) {
                            vertexCount++;
                            vbBindlessTex[p].append(m_reflNormalIndexed[i].getAllVertices()[j]);
                        }
                        for (unsigned int j = 0; j < m_reflNormalIndexed[i].getAllVertices().getIndexes().size(); j++) {
                            indexCount++;
                            vbBindlessTex[p].addIndex(m_reflNormalIndexed[i].getAllVertices().getIndexes()[j]);
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
                for (unsigned int i = 0; i < m_reflIndexed.size(); i++) {
                    if (m_reflIndexed[i].getAllVertices().getVertexCount() > 0) {
                        DrawElementsIndirectCommand drawElementsIndirectCommand;
                        unsigned int p = m_reflIndexed[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_reflIndexed[i].getMaterial().getTexture() != nullptr) ? m_reflIndexed[i].getMaterial().getTexture()->getId() : 0;
                        material.layer = m_reflIndexed[i].getMaterial().getLayer();
                        material.uvScale = (m_reflIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_reflIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_reflIndexed[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        materialDatas[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_reflIndexed[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            ModelData model;
                            model.worldMat = toVulkanMatrix(tm[j]->getMatrix())/*.transpose()*/;
                            modelDatas[p].push_back(model);
                        }
                        ////////std::cout<<"prim type : "<<p<<std::endl<<"model datas size : "<<modelDatas[p].size()<<std::endl;
                        unsigned int vertexCount = 0, indexCount = 0;

                        if (m_reflIndexed[i].getVertexArrays().size() > 0) {
                            Entity* entity = m_reflIndexed[i].getVertexArrays()[0]->getEntity();

                            for (unsigned int j = 0; j < m_reflIndexed[i].getVertexArrays().size(); j++) {

                                if (entity == m_reflIndexed[i].getVertexArrays()[j]->getEntity()) {

                                    unsigned int p = m_reflIndexed[i].getVertexArrays()[j]->getPrimitiveType();
                                    for (unsigned int k = 0; k < m_reflIndexed[i].getVertexArrays()[j]->getVertexCount(); k++) {

                                        vertexCount++;
                                        vbBindlessTex[p].append((*m_reflIndexed[i].getVertexArrays()[j])[k]);
                                    }
                                    for (unsigned int k = 0; k < m_reflIndexed[i].getVertexArrays()[j]->getIndexes().size(); k++) {
                                        ////std::cout<<"add depth refl inst indexed"<<std::endl;
                                        indexCount++;
                                        vbBindlessTex[p].addIndex(m_reflIndexed[i].getVertexArrays()[j]->getIndexes()[k]);
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
                currentStates.shader = &sBuildDepthBuffer;
                currentStates.texture = nullptr;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        vbBindlessTex[p].update();
                        VkDeviceSize bufferSize = sizeof(ModelData) * modelDatas[p].size();
                        ////std::cout<<"model datas size : "<<bufferSize<<std::endl;
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

                        createCommandBuffersIndirect(p, drawElementsIndirectCommands[p].size(), sizeof(DrawElementsIndirectCommand), RRRCNODEPTHNOSTENCIL, currentStates);
                    }
                }
            }
            void ReflectRefractRenderComponent::drawAlphaInst() {
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    vbBindlessTex[p].clear();
                    materialDatas[p].clear();
                    modelDatas[p].clear();
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
                        ////////std::cout<<"layer : "<<layer<<" nb layers : "<<Entity::getNbLayers()<<std::endl;
                        unsigned int p = m_normals[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_normals[i].getMaterial().getTexture() != nullptr) ? m_normals[i].getMaterial().getTexture()->getId() : 0;
                        material.layer = m_normals[i].getMaterial().getLayer();
                        material.uvScale = (m_normals[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_normals[i].getMaterial().getTexture()->getSize().x(), 1.f / m_normals[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        materialDatas[p].push_back(material);
                        ModelData model;
                        TransformMatrix tm;
                        model.worldMat = toVulkanMatrix(tm.getMatrix())/*.transpose()*/;
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
                            model.worldMat = toVulkanMatrix(tm[j]->getMatrix())/*.transpose()*/;
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
                currentStates.shader = &sBuildAlphaBuffer;
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
                        createCommandBuffersIndirect(p, drawArraysIndirectCommands[p].size(), sizeof(DrawArraysIndirectCommand), RRRCNODEPTHNOSTENCIL, currentStates);
                    }
                }
            }
            void ReflectRefractRenderComponent::drawAlphaIndexedInst() {
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    vbBindlessTex[p].clear();
                    materialDatas[p].clear();
                    modelDatas[p].clear();
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
                for (unsigned int i = 0; i < m_normalIndexed.size(); i++) {
                    if (m_normalIndexed[i].getAllVertices().getVertexCount() > 0) {
                        DrawElementsIndirectCommand drawElementsIndirectCommand;
                        unsigned int p = m_normalIndexed[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_normalIndexed[i].getMaterial().getTexture() != nullptr) ? m_normalIndexed[i].getMaterial().getTexture()->getId() : 0;
                        material.layer = m_normalIndexed[i].getMaterial().getLayer();
                        material.uvScale = (m_normalIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_normalIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_normalIndexed[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        materialDatas[p].push_back(material);
                        ModelData model;
                        TransformMatrix tm;
                        model.worldMat = toVulkanMatrix(tm.getMatrix())/*.transpose()*/;
                        modelDatas[p].push_back(model);
                        unsigned int vertexCount = 0, indexCount = 0;
                        for (unsigned int j = 0; j < m_normalIndexed[i].getAllVertices().getVertexCount(); j++) {
                            vertexCount++;
                            vbBindlessTex[p].append(m_normalIndexed[i].getAllVertices()[j]);
                        }
                        for (unsigned int j = 0; j < m_normalIndexed[i].getAllVertices().getIndexes().size(); j++) {
                            ////std::cout<<"add norm indexed"<<std::endl;
                            indexCount++;
                            vbBindlessTex[p].addIndex(m_normalIndexed[i].getAllVertices().getIndexes()[j]);
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
                for (unsigned int i = 0; i < m_indexed.size(); i++) {
                    if (m_indexed[i].getAllVertices().getVertexCount() > 0) {
                        DrawElementsIndirectCommand drawElementsIndirectCommand;
                        unsigned int p = m_indexed[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_indexed[i].getMaterial().getTexture() != nullptr) ? m_indexed[i].getMaterial().getTexture()->getId() : 0;
                        material.layer = m_indexed[i].getMaterial().getLayer();
                        material.uvScale = (m_indexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_indexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_indexed[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        materialDatas[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_indexed[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            ModelData model;
                            model.worldMat = toVulkanMatrix(tm[j]->getMatrix())/*.transpose()*/;
                            modelDatas[p].push_back(model);
                        }
                        ////////std::cout<<"prim type : "<<p<<std::endl<<"model datas size : "<<modelDatas[p].size()<<std::endl;
                        unsigned int vertexCount = 0, indexCount = 0;

                        if (m_indexed[i].getVertexArrays().size() > 0) {
                            Entity* entity = m_indexed[i].getVertexArrays()[0]->getEntity();

                            for (unsigned int j = 0; j < m_indexed[i].getVertexArrays().size(); j++) {

                                if (entity == m_indexed[i].getVertexArrays()[j]->getEntity()) {

                                    unsigned int p = m_indexed[i].getVertexArrays()[j]->getPrimitiveType();
                                    for (unsigned int k = 0; k < m_indexed[i].getVertexArrays()[j]->getVertexCount(); k++) {

                                        vertexCount++;
                                        vbBindlessTex[p].append((*m_indexed[i].getVertexArrays()[j])[k]);
                                    }
                                    for (unsigned int k = 0; k < m_indexed[i].getVertexArrays()[j]->getIndexes().size(); k++) {

                                        indexCount++;
                                        vbBindlessTex[p].addIndex(m_indexed[i].getVertexArrays()[j]->getIndexes()[k]);
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
                currentStates.shader = &sBuildAlphaBuffer;
                currentStates.texture = nullptr;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        vbBindlessTex[p].update();
                        VkDeviceSize bufferSize = sizeof(ModelData) * modelDatas[p].size();
                        ////////std::cout<<"prim type : "<<p<<std::endl<<"model datas size : "<<modelDatas[p].size()<<std::endl;
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

                        createCommandBuffersIndirect(p, drawElementsIndirectCommands[p].size(), sizeof(DrawElementsIndirectCommand), RRRCNODEPTHNOSTENCIL, currentStates);
                    }
                }
            }
            void ReflectRefractRenderComponent::drawEnvReflInst() {
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
                            material.uvScale = (m_normals[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_normals[i].getMaterial().getTexture()->getSize().x(), 1.f / m_normals[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                            material.uvOffset = math::Vec2f(0, 0);
                            materialDatas[p].push_back(material);
                        }
                        ModelData model;
                        TransformMatrix tm;
                        model.worldMat = toVulkanMatrix(tm.getMatrix().transpose());
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
                        //for (unsigned int d = 0; d < 6; d++)
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
                            model.worldMat = toVulkanMatrix(tm[j]->getMatrix())/*.transpose()*/;
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
                        //for (unsigned int d = 0; d < 6; d++)
                            drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                        firstIndex[p] += vertexCount;
                        baseInstance[p] += tm.size();
                    }
                }
                RenderStates currentStates;
                currentStates.blendMode = BlendNone;
                currentStates.shader = &sLinkedList;
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
                        createCommandBuffersIndirect(p, drawArraysIndirectCommands[p].size(), sizeof(DrawArraysIndirectCommand), RRRCNODEPTHNOSTENCIL, currentStates);
                    }
                }
            }
            void ReflectRefractRenderComponent::drawEnvReflIndexedInst() {
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    vbBindlessTex[p].clear();
                    materialDatas[p].clear();
                    modelDatas[p].clear();
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
                ////std::cout<<"normal indexed : "<<m_normalIndexed.size()<<std::endl;
                for (unsigned int i = 0; i < m_normalIndexed.size(); i++) {
                    if (m_normalIndexed[i].getAllVertices().getVertexCount() > 0) {

                        DrawElementsIndirectCommand drawElementsIndirectCommand;
                        unsigned int p = m_normalIndexed[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_normalIndexed[i].getMaterial().getTexture() != nullptr) ? m_normalIndexed[i].getMaterial().getTexture()->getId() : 0;
                        material.layer = m_normalIndexed[i].getMaterial().getLayer();
                        material.uvScale = (m_normalIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_normalIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_normalIndexed[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        materialDatas[p].push_back(material);
                        ModelData model;
                        TransformMatrix tm;
                        model.worldMat = toVulkanMatrix(tm.getMatrix())/*.transpose()*/;
                        modelDatas[p].push_back(model);
                        unsigned int vertexCount = 0, indexCount = 0;
                        for (unsigned int j = 0; j < m_normalIndexed[i].getAllVertices().getVertexCount(); j++) {
                            ////std::cout<<"add vertex"<<std::endl;
                            vertexCount++;
                            vbBindlessTex[p].append(m_normalIndexed[i].getAllVertices()[j]);
                        }
                        for (unsigned int j = 0; j < m_normalIndexed[i].getAllVertices().getIndexes().size(); j++) {
                            ////std::cout<<"add index"<<std::endl;
                            indexCount++;
                            vbBindlessTex[p].addIndex(m_normalIndexed[i].getAllVertices().getIndexes()[j]);
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
                for (unsigned int i = 0; i < m_indexed.size(); i++) {
                    if (m_indexed[i].getAllVertices().getVertexCount() > 0) {
                        DrawElementsIndirectCommand drawElementsIndirectCommand;
                        unsigned int p = m_indexed[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_indexed[i].getMaterial().getTexture() != nullptr) ? m_indexed[i].getMaterial().getTexture()->getId() : 0;
                        material.layer = m_indexed[i].getMaterial().getLayer();
                        material.uvScale = (m_indexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_indexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_indexed[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        materialDatas[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_indexed[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            ModelData model;
                            model.worldMat = toVulkanMatrix(tm[j]->getMatrix())/*.transpose()*/;
                            modelDatas[p].push_back(model);
                        }
                        ////////std::cout<<"prim type : "<<p<<std::endl<<"model datas size : "<<modelDatas[p].size()<<std::endl;
                        unsigned int vertexCount = 0, indexCount = 0;

                        if (m_indexed[i].getVertexArrays().size() > 0) {
                            Entity* entity = m_indexed[i].getVertexArrays()[0]->getEntity();

                            for (unsigned int j = 0; j < m_indexed[i].getVertexArrays().size(); j++) {

                                if (entity == m_indexed[i].getVertexArrays()[j]->getEntity()) {

                                    unsigned int p = m_indexed[i].getVertexArrays()[j]->getPrimitiveType();
                                    for (unsigned int k = 0; k < m_indexed[i].getVertexArrays()[j]->getVertexCount(); k++) {

                                        vertexCount++;
                                        vbBindlessTex[p].append((*m_indexed[i].getVertexArrays()[j])[k]);
                                    }
                                    for (unsigned int k = 0; k < m_indexed[i].getVertexArrays()[j]->getIndexes().size(); k++) {

                                        indexCount++;
                                        vbBindlessTex[p].addIndex(m_indexed[i].getVertexArrays()[j]->getIndexes()[k]);
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
                currentStates.shader = &sLinkedList;
                currentStates.texture = nullptr;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        vbBindlessTex[p].update();
                        VkDeviceSize bufferSize = sizeof(ModelData) * modelDatas[p].size();
                        ////////std::cout<<"prim type : "<<p<<std::endl<<"model datas size : "<<modelDatas[p].size()<<std::endl;
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

                        createCommandBuffersIndirect(p, drawElementsIndirectCommands[p].size(), sizeof(DrawElementsIndirectCommand), RRRCNODEPTHNOSTENCIL, currentStates);
                    }
                }
            }
            void ReflectRefractRenderComponent::drawReflInst(Entity* reflectEntity) {
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
                for (unsigned int i = 0; i < m_reflNormals.size(); i++) {
                    if (m_reflNormals[i].getVertexArrays().size() > 0) {
                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        unsigned int p = m_reflNormals[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_reflNormals[i].getMaterial().getTexture() != nullptr) ? m_reflNormals[i].getMaterial().getTexture()->getId() : 0;
                        material.materialType = m_reflNormals[i].getMaterial().getType();
                        material.uvScale = (m_reflNormals[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_reflNormals[i].getMaterial().getTexture()->getSize().x(), 1.f / m_reflNormals[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        materialDatas[p].push_back(material);
                        ModelData model;
                        TransformMatrix tm;
                        model.worldMat = toVulkanMatrix(tm.getMatrix())/*.transpose()*/;
                        modelDatas[p].push_back(model);
                        unsigned int vertexCount = 0;
                        for (unsigned int j = 0; j < m_reflNormals[i].getVertexArrays().size(); j++) {
                            Entity* entity = m_reflNormals[i].getVertexArrays()[j]->getEntity()->getRootEntity();
                            if (entity == reflectEntity) {
                                for (unsigned int k = 0; k < m_reflNormals[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                    vertexCount++;
                                    math::Vec3f t = m_reflNormals[i].getVertexArrays()[j]->getEntity()->getTransform().transform(math::Vec4f((*m_reflNormals[i].getVertexArrays()[j])[k].position));
                                    Vertex v (t, (*m_reflNormals[i].getVertexArrays()[j])[k].color, (*m_reflNormals[i].getVertexArrays()[j])[k].texCoords);
                                    v.normal = (*m_reflNormals[i].getVertexArrays()[j])[k].normal;
                                    vbBindlessTex[p].append(v);
                                }
                            }
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
                for (unsigned int i = 0; i < m_reflInstances.size(); i++) {
                    if (m_reflInstances[i].getAllVertices().getVertexCount() > 0) {
                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        unsigned int p = m_reflInstances[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_reflInstances[i].getMaterial().getTexture() != nullptr) ? m_reflInstances[i].getMaterial().getTexture()->getId() : 0;
                        material.materialType = m_reflInstances[i].getMaterial().getType();
                        material.uvScale = (m_reflInstances[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_reflInstances[i].getMaterial().getTexture()->getSize().x(), 1.f / m_reflInstances[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        materialDatas[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_reflInstances[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            ModelData model;
                            model.worldMat = toVulkanMatrix(tm[j]->getMatrix())/*.transpose()*/;
                            modelDatas[p].push_back(model);
                        }
                        unsigned int vertexCount = 0;
                        if (m_reflInstances[i].getVertexArrays().size() > 0) {
                            Entity* entity = m_reflInstances[i].getVertexArrays()[0]->getEntity();
                            for (unsigned int j = 0; j < m_reflInstances[i].getVertexArrays().size(); j++) {
                                if (entity == m_reflInstances[i].getVertexArrays()[j]->getEntity() && entity->getRootEntity() == reflectEntity) {
                                    for (unsigned int k = 0; k < m_reflInstances[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                        vertexCount++;
                                        vbBindlessTex[p].append((*m_reflInstances[i].getVertexArrays()[j])[k]);
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
                currentStates.shader = &sReflectRefract;
                currentStates.texture = &environmentMap.getTexture(environmentMap.getImageIndex());
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
                        createCommandBuffersIndirect(p, drawArraysIndirectCommands[p].size(), sizeof(DrawArraysIndirectCommand), RRRCDEPTHNOSTENCIL, currentStates);
                    }
                }
            }
            void ReflectRefractRenderComponent::drawReflIndexedInst(Entity* reflectEntity) {
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    vbBindlessTex[p].clear();
                    materialDatas[p].clear();
                    modelDatas[p].clear();
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
                for (unsigned int i = 0; i < m_reflNormalIndexed.size(); i++) {
                    if (m_reflNormalIndexed[i].getAllVertices().getVertexCount() > 0) {
                        DrawElementsIndirectCommand drawElementsIndirectCommand;
                        unsigned int p = m_reflNormalIndexed[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_reflNormalIndexed[i].getMaterial().getTexture() != nullptr) ? m_reflNormalIndexed[i].getMaterial().getTexture()->getId() : 0;
                        material.materialType = m_reflNormalIndexed[i].getMaterial().getType();
                        material.uvScale = (m_reflNormalIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_reflNormalIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_reflNormalIndexed[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        materialDatas[p].push_back(material);
                        ModelData model;
                        TransformMatrix tm;
                        model.worldMat = toVulkanMatrix(tm.getMatrix())/*.transpose()*/;
                        modelDatas[p].push_back(model);
                        unsigned int vertexCount = 0, indexCount = 0;
                        for (unsigned int j = 0; j < m_reflNormalIndexed[i].getVertexArrays().size(); j++) {
                            Entity* entity = m_reflNormalIndexed[i].getVertexArrays()[j]->getEntity()->getRootEntity();
                            if (entity == reflectEntity) {
                                for (unsigned int k = 0; k < m_reflNormalIndexed[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                    vertexCount++;
                                    math::Vec3f t = m_reflNormalIndexed[i].getVertexArrays()[j]->getEntity()->getTransform().transform(math::Vec4f((*m_reflNormalIndexed[i].getVertexArrays()[j])[k].position));
                                    Vertex v (t, (*m_reflNormalIndexed[i].getVertexArrays()[j])[k].color, (*m_reflNormalIndexed[i].getVertexArrays()[j])[k].texCoords);
                                    v.normal = (*m_reflNormalIndexed[i].getVertexArrays()[j])[k].normal;
                                    vbBindlessTex[p].append(v);
                                }
                                for (unsigned int k = 0; k < m_reflNormalIndexed[i].getVertexArrays()[j]->getIndexes().size(); k++) {
                                    ////std::cout<<"add refl norm indexed"<<std::endl;
                                    indexCount++;
                                    vbBindlessTex[p].addIndex(m_reflNormalIndexed[i].getVertexArrays()[j]->getIndexes()[j]);
                                }
                            }
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
                for (unsigned int i = 0; i < m_reflIndexed.size(); i++) {
                    if (m_reflIndexed[i].getAllVertices().getVertexCount() > 0) {
                        DrawElementsIndirectCommand drawElementsIndirectCommand;
                        unsigned int p = m_reflIndexed[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_reflIndexed[i].getMaterial().getTexture() != nullptr) ? m_reflIndexed[i].getMaterial().getTexture()->getId() : 0;
                        material.materialType = m_reflIndexed[i].getMaterial().getType();
                        material.uvScale = (m_reflIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_reflIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_reflIndexed[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        materialDatas[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_reflIndexed[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            ModelData model;
                            model.worldMat = toVulkanMatrix(tm[j]->getMatrix())/*.transpose()*/;
                            modelDatas[p].push_back(model);
                        }
                        ////////std::cout<<"prim type : "<<p<<std::endl<<"model datas size : "<<modelDatas[p].size()<<std::endl;
                        unsigned int vertexCount = 0, indexCount = 0;

                        if (m_reflIndexed[i].getVertexArrays().size() > 0) {
                            Entity* entity = m_reflIndexed[i].getVertexArrays()[0]->getEntity();

                            for (unsigned int j = 0; j < m_reflIndexed[i].getVertexArrays().size(); j++) {

                                if (entity == m_reflIndexed[i].getVertexArrays()[j]->getEntity() && entity->getRootEntity() == reflectEntity) {

                                    unsigned int p = m_reflIndexed[i].getVertexArrays()[j]->getPrimitiveType();
                                    for (unsigned int k = 0; k < m_reflIndexed[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                        ////std::cout<<"add refl inst vert"<<std::endl;
                                        vertexCount++;
                                        vbBindlessTex[p].append((*m_reflIndexed[i].getVertexArrays()[j])[k]);
                                    }
                                    for (unsigned int k = 0; k < m_reflIndexed[i].getVertexArrays()[j]->getIndexes().size(); k++) {
                                        ////std::cout<<"add refl inst indexed"<<std::endl;
                                        indexCount++;
                                        vbBindlessTex[p].addIndex(m_reflIndexed[i].getVertexArrays()[j]->getIndexes()[k]);
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
                currentStates.shader = &sReflectRefract;
                currentStates.texture = &environmentMap.getTexture(environmentMap.getImageIndex());
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        vbBindlessTex[p].update();
                        VkDeviceSize bufferSize = sizeof(ModelData) * modelDatas[p].size();
                        ////////std::cout<<"prim type : "<<p<<std::endl<<"model datas size : "<<modelDatas[p].size()<<std::endl;
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
                        ////std::cout<<"draw refl refr"<<std::endl;

                        createCommandBuffersIndirect(p, drawElementsIndirectCommands[p].size(), sizeof(DrawElementsIndirectCommand), RRRCNODEPTHNOSTENCIL, currentStates);
                    }
                }
            }

            void ReflectRefractRenderComponent::drawNextFrame() {
                ////////std::cout<<"draw next frame"<<std::endl;
                {
                    std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                    if (datasReady.load()) {
                        datasReady = false;
                        m_instances = batcher.getInstances();
                        m_normals = normalBatcher.getInstances();
                        m_reflInstances = reflBatcher.getInstances();
                        m_reflNormals = reflNormalBatcher.getInstances();
                        m_skyboxInstance = skyboxBatcher.getInstances();
                        m_indexed = indexedBatcher.getInstances();
                        m_normalIndexed = normalIndexedBatcher.getInstances();
                        m_reflIndexed = reflIndexedBatcher.getInstances();
                        m_reflNormalIndexed = reflNormalIndexedBatcher.getInstances();
                    }
                }
                RenderStates currentStates;
                math::Matrix4f viewMatrix = view.getViewMatrix().getMatrix();
                math::Matrix4f projMatrix = view.getProjMatrix().getMatrix();
                indirectRenderingPC.projMatrix = toVulkanMatrix(projMatrix);
                indirectRenderingPC.viewMatrix = toVulkanMatrix(viewMatrix);
                ////std::cout<<"view matrix : "<<viewMatrix<<std::endl;
                if (useThread) {
                    //commandBufferReady = false;

                    std::unique_lock<std::mutex> lock(mtx);
                    cv.wait(lock, [this] { return registerFrameJob[depthBuffer.getCurrentFrame()].load() || stop.load(); });
                    registerFrameJob[depthBuffer.getCurrentFrame()] = false;
                    //std::cout<<"wait job : "<<depthBuffer.getCurrentFrame()<<std::endl;
                    //std::cout<<"fill buffers"<<std::endl;
                    resetBuffers();

                    fillBufferReflMT();
                    fillBufferReflIndexedMT();
                    fillNonReflBufferMT();
                    fillNonReflIndexedBufferMT();
                    vb2.clear();
                    //skyboxVB.name = "SKYBOXVB";
                    for (unsigned int i = 0; i < m_skyboxInstance.size(); i++) {
                        if (m_skyboxInstance[i].getAllVertices().getVertexCount() > 0) {
                            vb2.setPrimitiveType(m_skyboxInstance[i].getAllVertices().getPrimitiveType());
                            for (unsigned int j = 0; j < m_skyboxInstance[i].getAllVertices().getVertexCount(); j++) {
                                vb2.append(m_skyboxInstance[i].getAllVertices()[j]);
                            }
                        }
                    }
                    unsigned int currentFrame = depthBuffer.getCurrentFrame();
                    if (skybox != nullptr) {
                        //std::cout<<"copy command skybox"<<std::endl;

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

                        vkResetCommandBuffer(copySkyboxCommandBuffer[currentFrame], 0);
                        if (vkBeginCommandBuffer(copySkyboxCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                            throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                        }
                        //std::cout<<"copy skybox vb"<<std::endl;
                        vb2.update(currentFrame, copySkyboxCommandBuffer[currentFrame]);
                        if (vkEndCommandBuffer(copySkyboxCommandBuffer[currentFrame]) != VK_SUCCESS) {
                            throw core::Erreur(0, "failed to record command buffer!", 1);
                        }
                    }
                    //std::cout<<"fill vb"<<std::endl;
                    vb.clear();
                    vb.setPrimitiveType(Triangles);
                    Vertex v1 (math::Vec3f(0, 0, quad.getSize().z()));
                    Vertex v2 (math::Vec3f(quad.getSize().x(),0, quad.getSize().z()));
                    Vertex v3 (math::Vec3f(quad.getSize().x(), quad.getSize().y(), quad.getSize().z()));
                    Vertex v4 (math::Vec3f(0, quad.getSize().y(), quad.getSize().z()));
                    vb.append(v1);
                    vb.append(v2);
                    vb.append(v3);
                    vb.append(v1);
                    vb.append(v3);
                    vb.append(v4);
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
                    beginInfo.pInheritanceInfo = &inheritanceInfo; // obligatoire pour secondaire
                    vkResetCommandBuffer(copyVbEnvPass2BufferCommandBuffer[currentFrame], 0);
                    if (vkBeginCommandBuffer(copyVbEnvPass2BufferCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                        throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                    }
                    vb.update(currentFrame, copyVbEnvPass2BufferCommandBuffer[currentFrame]);
                    if (vkEndCommandBuffer(copyVbEnvPass2BufferCommandBuffer[currentFrame]) != VK_SUCCESS) {
                        throw core::Erreur(0, "failed to record command buffer!", 1);
                    }

                    math::Matrix4f matrix = quad.getTransform().getMatrix();
                    linkedList2PC.worldMat = toVulkanMatrix(matrix);
                    View reflectView;
                    if (view.isOrtho()) {
                        reflectView = View (squareSize, squareSize, view.getViewport().getPosition().z(), view.getViewport().getSize().z());
                    } else {
                        reflectView = View (squareSize, squareSize, 80, view.getViewport().getPosition().z(), view.getViewport().getSize().z());
                    }
                    rootEntities.clear();
                    ubos.clear();
                    //std::cout<<"create uniform buffers"<<std::endl;
                    for (unsigned int t = 0; t < 4; t++) {

                        std::vector<Instance> instances;
                        if(t == 0)
                            instances = m_reflInstances;
                        else if (t == 1)
                            instances = m_reflNormals;
                        else if (t == 2)
                            instances = m_reflIndexed;
                        else
                            instances = m_reflNormalIndexed;
                        for (unsigned int n = 0; n < instances.size(); n++) {
                            if (instances[n].getAllVertices().getVertexCount() > 0) {
                                std::vector<Entity*> entities = instances[n].getEntities();
                                for (unsigned int e = 0; e < entities.size(); e++) {
                                    Entity* entity = entities[e]->getRootEntity();
                                    bool contains = false;
                                    for (unsigned int r = 0; r < rootEntities.size() && !contains; r++) {
                                        if (rootEntities[r] == entity)
                                            contains = true;
                                    }
                                    if (!contains) {
                                        nbReflRefrEntities++;
                                        if (nbReflRefrEntities > environmentMapCommandBuffer.size()) {

                                            VkCommandBufferAllocateInfo allocInfo{};
                                            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                                            allocInfo.commandPool = secondaryBufferCommandPool;
                                            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
                                            allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
                                            environmentMapCommandBuffer.resize(nbReflRefrEntities);
                                            reflectRefractCommandBuffer.resize(nbReflRefrEntities);
                                            skyboxCommandBuffer.resize(nbReflRefrEntities);
                                            if (vkAllocateCommandBuffers(vkDevice.getDevice(), &allocInfo, environmentMapCommandBuffer[nbReflRefrEntities-1].data()) != VK_SUCCESS) {
                                                throw core::Erreur(0, "failed to allocate command buffers!", 1);
                                            }
                                            if (vkAllocateCommandBuffers(vkDevice.getDevice(), &allocInfo, reflectRefractCommandBuffer[nbReflRefrEntities-1].data()) != VK_SUCCESS) {
                                                throw core::Erreur(0, "failed to allocate command buffers!", 1);
                                            }
                                            if (vkAllocateCommandBuffers(vkDevice.getDevice(), &allocInfo, skyboxCommandBuffer[nbReflRefrEntities-1].data()) != VK_SUCCESS) {
                                                throw core::Erreur(0, "failed to allocate command buffers!", 1);
                                            }

                                            createUniformBuffersMT();
                                        }
                                        rootEntities.push_back(entity);
                                        fillReflEntityBufferMT(entity);
                                        fillIndexedReflEntityBufferMT(entity);

                                        if (entity->getType() != "E_BIGTILE")
                                            reflectView.setCenter(entity->getPosition()+entity->getSize()*0.5f);
                                        else
                                            reflectView.setCenter(view.getPosition());
                                        math::Matrix4f projMatrices[6];
                                        math::Matrix4f viewMatrices[6];
                                        math::Matrix4f sbProjMatrices[6];
                                        math::Matrix4f sbViewMatrices[6];

                                        environmentMap.setView(reflectView);
                                        for (unsigned int m = 0; m < 6; m++) {
                                            math::Vec3f target = reflectView.getPosition() + dirs[m];
                                            reflectView.lookAt(target.x(), target.y(), target.z(), ups[m]);
                                            projMatrix = reflectView.getProjMatrix().getMatrix();
                                            viewMatrix = reflectView.getViewMatrix().getMatrix();
                                            projMatrices[m] = projMatrix;
                                            viewMatrices[m] = viewMatrix;
                                        }
                                        UniformBufferObject ubo;
                                        for (unsigned int f = 0; f < 6; f++) {
                                            MatricesData matrices;
                                            matrices.projMatrix = toVulkanMatrix(projMatrices[f]);
                                            matrices.viewMatrix = toVulkanMatrix(viewMatrices[f]);
                                            ubo.matrices[f] = matrices;
                                        }
                                        ubos.push_back(ubo);
                                    }
                                }
                            }
                        }
                    }
                    if (nbReflRefrEntities > 0) {

                        ////std::cout<<"nb refl entities  : "<<nbReflRefrEntities<<std::endl<<"size : "<<ubos.size()<<std::endl;
                        updateUniformBuffer(currentFrame, ubos);

                    } else {

                        nbReflRefrEntities = 1;
                        createUniformBuffersMT();
                        UniformBufferObject dummy;
                        ubos.push_back(dummy);
                        ////std::cout<<"dummy nb refl entities  : "<<nbReflRefrEntities<<std::endl<<"size : "<<ubos.size()<<std::endl;

                        updateUniformBuffer(currentFrame, ubos);
                        nbReflRefrEntities = 0;
                    }
                    buildFrameBufferPC.cameraPos = math::Vec4f(view.getPosition().x(), view.getPosition().y(), view.getPosition().z(), 1);
                    if (skybox != nullptr) {
                        currentStates.shader = &skyboxShader;
                        for (unsigned int p = 0; p < (Batcher::nbPrimitiveTypes - 1); p++)
                            updateDescriptorSets(currentFrame, p, currentStates);
                    }
                    //std::cout<<"draw buffers"<<std::endl;
                    drawBuffers();
                    //std::cout<<"command buffer wake up : "<<currentFrame<<std::endl;
                    commandBufferReady[depthBuffer.getCurrentFrame()] = true;
                    cv.notify_one();
                } else {
                    ////std::cout<<"draw no thread"<<std::endl;

                    drawDepthReflInst();
                    drawDepthReflIndexedInst();



                    drawAlphaInst();
                    drawAlphaIndexedInst();


                    View reflectView;
                    if (view.isOrtho()) {
                        reflectView = View (squareSize, squareSize, view.getViewport().getPosition().z(), view.getViewport().getSize().z());
                    } else {
                        reflectView = View (squareSize, squareSize, 80, view.getViewport().getPosition().z(), view.getViewport().getSize().z());
                    }
                    rootEntities.clear();
                    for (unsigned int t = 0; t < 4; t++) {

                        std::vector<Instance> instances;
                        if(t == 0)
                            instances = m_reflInstances;
                        else if (t == 1)
                            instances = m_reflNormals;
                        else if (t == 2)
                            instances = m_reflIndexed;
                        else
                            instances = m_reflNormalIndexed;
                        for (unsigned int n = 0; n < instances.size(); n++) {
                            if (instances[n].getAllVertices().getVertexCount() > 0) {
                                std::vector<Entity*> entities = instances[n].getEntities();
                                for (unsigned int e = 0; e < entities.size(); e++) {
                                    Entity* entity = entities[e]->getRootEntity();
                                    bool contains = false;
                                    for (unsigned int r = 0; r < rootEntities.size() && !contains; r++) {
                                        if (rootEntities[r] == entity)
                                            contains = true;
                                    }
                                    if (!contains) {
                                        rootEntities.push_back(entity);
                                        if (entity->getType() != "E_BIGTILE")
                                            reflectView.setCenter(entity->getPosition()+entity->getSize()*0.5f);
                                        else
                                            reflectView.setCenter(view.getPosition());
                                        math::Matrix4f projMatrices[6];
                                        math::Matrix4f viewMatrices[6];

                                        environmentMap.setView(reflectView);
                                        for (unsigned int m = 0; m < 6; m++) {
                                            math::Vec3f target = reflectView.getPosition() + dirs[m];
                                            reflectView.lookAt(target.x(), target.y(), target.z(), ups[m]);
                                            projMatrix = reflectView.getProjMatrix().getMatrix()/*.transpose()*/;
                                            viewMatrix = reflectView.getViewMatrix().getMatrix()/*.transpose()*/;
                                            projMatrices[m] = projMatrix;
                                            viewMatrices[m] = viewMatrix;


                                        }


                                        UniformBufferObject ubo;
                                        for (unsigned int f = 0; f < 6; f++) {
                                            MatricesData matrices;
                                            matrices.projMatrix = toVulkanMatrix(projMatrices[f]);
                                            matrices.viewMatrix = toVulkanMatrix(viewMatrices[f]);
                                            ubo.matrices[f] = matrices;
                                        }
                                        updateUniformBuffer(environmentMap.getCurrentFrame(), ubo);
                                        environmentMap.clear(Color::Transparent);
                                        environmentMap.beginRecordCommandBuffers();
                                        VkClearColorValue clearColor;
                                        clearColor.uint32[0] = 0xffffffff;
                                        std::vector<VkCommandBuffer> commandBuffers = environmentMap.getCommandBuffers();
                                        for (unsigned int j = 0; j < commandBuffers.size(); j++) {
                                            VkImageSubresourceRange subresRange {};
                                            subresRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                                            subresRange.levelCount = 1;
                                            subresRange.layerCount = 6;
                                            vkCmdClearColorImage(commandBuffers[j], headPtrTextureImage[j], VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &subresRange);
                                            for (unsigned int f = 0; f < 6; f++)
                                                vkCmdFillBuffer(commandBuffers[j], counterShaderStorageBuffers[j], f * sizeof(uint32_t), sizeof(uint32_t), 0);
                                            VkMemoryBarrier memoryBarrier;
                                            memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                                            memoryBarrier.pNext = VK_NULL_HANDLE;
                                            memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                                            memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                                            vkCmdPipelineBarrier(commandBuffers[j], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

                                        }


                                        drawEnvReflInst();
                                        drawEnvReflIndexedInst();


                                        vb.clear();
                                        vb.setPrimitiveType(Triangles);
                                        Vertex v1 (math::Vec3f(0, 0, quad.getSize().z()));
                                        Vertex v2 (math::Vec3f(quad.getSize().x(),0, quad.getSize().z()));
                                        Vertex v3 (math::Vec3f(quad.getSize().x(), quad.getSize().y(), quad.getSize().z()));
                                        Vertex v4 (math::Vec3f(0, quad.getSize().y(), quad.getSize().z()));
                                        vb.append(v1);
                                        vb.append(v2);
                                        vb.append(v3);
                                        vb.append(v1);
                                        vb.append(v3);
                                        vb.append(v4);
                                        vb.update();
                                        math::Matrix4f matrix = quad.getTransform().getMatrix()/*.transpose()*/;
                                        linkedList2PC.worldMat = toVulkanMatrix(matrix);
                                        currentStates.shader = &sLinkedList2;
                                        currentStates.texture = nullptr;
                                        createCommandBufferVertexBuffer(currentStates);


                                        buildFrameBufferPC.cameraPos = math::Vec4f(view.getPosition().x(), view.getPosition().y(), view.getPosition().z(), 1);
                                        reflectRefractTex.beginRecordCommandBuffers();
                                        drawReflInst(entity);
                                        drawReflIndexedInst(entity);
                                    }
                                }
                            }

                        }
                    }
                    if (!isSomethingDrawn) {
                        std::vector<VkSemaphore> waitSemaphores;
                        waitSemaphores.push_back(offscreenRenderingFinishedSemaphore[depthBuffer.getCurrentFrame()]);
                        std::vector<VkPipelineStageFlags> waitStages;
                        waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                        std::vector<uint64_t> waitValues;
                        waitValues.push_back(values[depthBuffer.getCurrentFrame()]);
                        std::vector<VkSemaphore> signalSemaphores;
                        signalSemaphores.push_back(offscreenDepthPassFinishedSemaphore[depthBuffer.getCurrentFrame()]);
                        std::vector<uint64_t> signalValues;
                        depthBuffer.submit(true, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);
                        reflectRefractTex.beginRecordCommandBuffers();
                        waitSemaphores.clear();
                        waitStages.clear();
                        signalSemaphores.clear();
                        waitValues.clear();
                        signalValues.clear();
                        waitSemaphores.push_back(offscreenDepthPassFinishedSemaphore[reflectRefractTex.getCurrentFrame()]);
                        waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                        values[reflectRefractTex.getCurrentFrame()]++;
                        signalSemaphores.push_back(offscreenRenderingFinishedSemaphore[reflectRefractTex.getCurrentFrame()]);
                        signalValues.push_back(values[reflectRefractTex.getCurrentFrame()]);
                        reflectRefractTex.submit(true, signalSemaphores, waitSemaphores, waitStages, signalValues);
                    }
                    isSomethingDrawn  = false;
                }
            }
            void ReflectRefractRenderComponent::draw(RenderTarget& target, RenderStates states) {
                if (useThread) {
                    //std::cout<<"current frame : "<<depthBuffer.getCurrentFrame()<<std::endl;
                    std::unique_lock<std::mutex> lock(mtx);

                    cv.wait(lock, [this] { return commandBufferReady[depthBuffer.getCurrentFrame()].load() || stop.load(); });
                    commandBufferReady[depthBuffer.getCurrentFrame()] = false;
                    // std::cout<<"wait command buffers : "<<depthBuffer.getCurrentFrame()<<std::endl;
                    //std::cout<<"soumission"<<std::endl;
                    depthBuffer.beginRecordCommandBuffers();
                    std::vector<VkCommandBuffer> commandBuffers = depthBuffer.getCommandBuffers();
                    unsigned int currentFrame = depthBuffer.getCurrentFrame();

                    vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &copyModelDataBufferCommandBuffer[currentFrame]);

                    vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &copyMaterialDataBufferCommandBuffer[currentFrame]);

                    vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &copyDrawBufferCommandBuffer[currentFrame]);
                    vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &copyVbBufferCommandBuffer[currentFrame]);

                    vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &copyDrawIndexedBufferCommandBuffer[currentFrame]);
                    vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &copyVbIndexedBufferCommandBuffer[currentFrame]);
                    vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &copyVbEnvPass2BufferCommandBuffer[currentFrame]);
                    if (skybox != nullptr)
                        vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &copySkyboxCommandBuffer[currentFrame]);
                    VkBufferMemoryBarrier bufferMemoryBarrier{};
                    bufferMemoryBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
                    bufferMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    bufferMemoryBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
                    bufferMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    bufferMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    bufferMemoryBarrier.offset = 0;
                    bufferMemoryBarrier.size = VK_WHOLE_SIZE;
                    bufferMemoryBarrier.buffer = vb.getVertexBuffer(currentFrame);
                    vkCmdPipelineBarrier(
                    commandBuffers[currentFrame],
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                    0,
                    0, nullptr,
                    1, &bufferMemoryBarrier,
                    0, nullptr
                    );
                    if (skybox != nullptr) {
                        bufferMemoryBarrier.buffer = vb2.getVertexBuffer(currentFrame);
                        vkCmdPipelineBarrier(
                        commandBuffers[currentFrame],
                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                        0,
                        0, nullptr,
                        1, &bufferMemoryBarrier,
                        0, nullptr
                        );
                    }
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
                    VkBufferMemoryBarrier uboBarrier{};
                    uboBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
                    uboBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
                    uboBarrier.dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT;
                    uboBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    uboBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    uboBarrier.buffer = uniformBuffer[currentFrame];
                    uboBarrier.offset = 0;
                    uboBarrier.size = VK_WHOLE_SIZE;

                    vkCmdPipelineBarrier(
                        commandBuffers[currentFrame],
                        VK_PIPELINE_STAGE_HOST_BIT,
                        VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                        0,
                        0, nullptr,
                        1, &uboBarrier,
                        0, nullptr
                    );
                    std::vector<VkSemaphore> signalSemaphores;
                    signalSemaphores.push_back(copyFinishedSemaphore[currentFrame]);
                    std::vector<VkSemaphore> waitSemaphores;
                    waitSemaphores.push_back(offscreenRenderingFinishedSemaphore[reflectRefractTex.getCurrentFrame()]);
                    std::vector<VkPipelineStageFlags> waitStages;
                    waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                    std::vector<uint64_t> signalValues;
                    std::vector<uint64_t> waitValues;
                    waitValues.push_back(values[reflectRefractTex.getCurrentFrame()]);
                    valuesCopy[currentFrame]++;
                    signalValues.push_back(valuesCopy[depthBuffer.getCurrentFrame()]);
                    //std::cout<<"value : "<<depthBuffer.getCurrentFrame()<<","<<valuesCopy[depthBuffer.getCurrentFrame()]<<std::endl;
                    depthBuffer.submit(false, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);
                    //std::cout<<"copy ok"<<std::endl;


                    depthBuffer.beginRecordCommandBuffers();

                    signalSemaphores.clear();
                    signalSemaphores.push_back(offscreenDepthPassFinishedSemaphore[currentFrame]);
                    signalValues.clear();
                    waitSemaphores.clear();
                    waitSemaphores.push_back(copyFinishedSemaphore[currentFrame]);
                    waitStages.clear();
                    waitStages.push_back(VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
                    waitValues.clear();
                    waitValues.push_back(valuesCopy[currentFrame]);
                    depthBuffer.beginRenderPass();
                    vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &depthBufferCommandBuffer[currentFrame]);
                    depthBuffer.endRenderPass();
                    depthBuffer.submit(true, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);

                    alphaBuffer.beginRecordCommandBuffers();

                    const_cast<Texture&>(depthBuffer.getTexture(depthBuffer.getImageIndex())).toShaderReadOnlyOptimal(alphaBuffer.getCommandBuffers()[alphaBuffer.getCurrentFrame()]);
                    commandBuffers = alphaBuffer.getCommandBuffers();
                    currentFrame = alphaBuffer.getCurrentFrame();
                    BlendMode blendNone = BlendNone;
                    blendNone.updateIds();
                    for (unsigned int p = 0; p < (Batcher::nbPrimitiveTypes -1); p++) {
                        if (nbDrawCommandBuffer[p][1] > 0 || nbIndexedDrawCommandBuffer[p][1] > 0) {
                            //std::cout<<"ids : "<<(sBuildAlphaBuffer.getId() * (Batcher::nbPrimitiveTypes - 1) + p)<<","<<(RRRCNODEPTHNOSTENCIL*blendNone.nbBlendModes+blendNone.id)<<std::endl;
                            vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS,alphaBuffer.getGraphicPipeline()[sBuildAlphaBuffer.getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][RRRCNODEPTHNOSTENCIL*blendNone.nbBlendModes+blendNone.id]);
                            vkCmdPushConstants(commandBuffers[currentFrame], alphaBuffer.getPipelineLayout()[sBuildAlphaBuffer.getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][RRRCNODEPTHNOSTENCIL*blendNone.nbBlendModes+blendNone.id], VK_SHADER_STAGE_FRAGMENT_BIT, 148, sizeof(unsigned int), &alphaBuffer.getImageIndex());
                        }
                    }
                    VkMemoryBarrier memoryBarrier{};
                    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                    memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                    memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                    vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                    alphaBuffer.beginRenderPass();
                    vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &alphaBufferCommandBuffer[currentFrame]);
                    alphaBuffer.endRenderPass();
                    const_cast<Texture&>(depthBuffer.getTexture(depthBuffer.getImageIndex())).toColorAttachmentOptimal(alphaBuffer.getCommandBuffers()[alphaBuffer.getCurrentFrame()]);
                    signalSemaphores.clear();
                    signalSemaphores.push_back(offscreenAlphaPassFinishedSemaphore[currentFrame]);
                    waitSemaphores.clear();
                    waitStages.clear();
                    waitSemaphores.push_back(offscreenDepthPassFinishedSemaphore[currentFrame]);
                    waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

                    alphaBuffer.submit(true, signalSemaphores, waitSemaphores, waitStages);
                    //std::cout<<"alpha buffer current frame : "<<currentFrame<<std::endl;

                    for (unsigned int i = 0; i < environmentMapCommandBuffer.size(); i++) {

                        commandBuffers = environmentMap.getCommandBuffers();
                        currentFrame = environmentMap.getCurrentFrame();

                        environmentMap.clear(Color::Transparent);
                        VkClearColorValue clearColor;
                        clearColor.uint32[0] = 0xffffffff;
                        VkImageSubresourceRange subresRange {};
                        subresRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                        subresRange.levelCount = 1;
                        subresRange.layerCount = 6;
                        vkCmdClearColorImage(commandBuffers[currentFrame], headPtrTextureImage[currentFrame], VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &subresRange);
                        for (unsigned int f = 0; f < 6; f++)
                            vkCmdFillBuffer(commandBuffers[currentFrame], counterShaderStorageBuffers[currentFrame], f * sizeof(uint32_t), sizeof(uint32_t), 0);
                        VkMemoryBarrier memoryBarrier;
                        memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                        memoryBarrier.pNext = VK_NULL_HANDLE;
                        memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                        memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                        vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);


                        if (skybox != nullptr) {
                            environmentMap.beginRecordCommandBuffers();
                            environmentMap.beginRenderPass();
                            vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &skyboxCommandBuffer[i][currentFrame]);
                            environmentMap.endRenderPass();
                            signalSemaphores.clear();
                            signalSemaphores.push_back(offscreenRenderingFinishedSemaphore[currentFrame]);
                            signalValues.clear();
                            waitSemaphores.clear();
                            waitSemaphores.push_back(copyFinishedSemaphore[currentFrame]);
                            waitStages.clear();
                            waitStages.push_back(VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
                            waitValues.clear();
                            waitValues.push_back(valuesCopy[currentFrame]);
                            values[currentFrame]++;
                            signalValues.push_back(values[currentFrame]);
                            environmentMap.submit(false, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);
                        }

                        environmentMap.beginRecordCommandBuffers();
                        environmentMap.beginRenderPass();
                        vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &environmentMapCommandBuffer[i][currentFrame]);
                        environmentMap.endRenderPass();
                        signalSemaphores.clear();
                        waitSemaphores.clear();
                        waitStages.clear();

                        signalSemaphores.clear();
                        signalSemaphores.push_back(offscreenRenderingFinishedSemaphore[reflectRefractTex.getCurrentFrame()]);
                        waitSemaphores.clear();
                        waitSemaphores.push_back(offscreenRenderingFinishedSemaphore[reflectRefractTex.getCurrentFrame()]);
                        waitSemaphores.push_back(copyFinishedSemaphore[depthBuffer.getCurrentFrame()]);
                        waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                        waitStages.push_back(VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
                        std::vector<uint64_t> signalValues, waitValues;
                        waitValues.push_back(values[reflectRefractTex.getCurrentFrame()]);
                        waitValues.push_back(valuesCopy[depthBuffer.getCurrentFrame()]);
                        values[reflectRefractTex.getCurrentFrame()]++;
                        signalValues.push_back(values[reflectRefractTex.getCurrentFrame()]);
                        environmentMap.submit(false, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);

                        environmentMap.beginRecordCommandBuffers();
                        vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);

                        memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                        memoryBarrier.pNext = VK_NULL_HANDLE;
                        memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                        memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                        vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                        environmentMap.beginRenderPass();
                        vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &environmentMapPass2CommandBuffer[currentFrame]);
                        environmentMap.endRenderPass();
                        waitSemaphores.clear();
                        waitStages.clear();
                        waitSemaphores.push_back(offscreenRenderingFinishedSemaphore[reflectRefractTex.getCurrentFrame()]);
                        waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                        waitValues.clear();
                        waitValues.push_back(values[reflectRefractTex.getCurrentFrame()]);

                        values[reflectRefractTex.getCurrentFrame()]++;
                        signalValues.clear();
                        signalValues.push_back(values[reflectRefractTex.getCurrentFrame()]);
                        environmentMap.submit(true, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);

                        reflectRefractTex.beginRecordCommandBuffers();

                        const_cast<Texture&>(environmentMap.getTexture(environmentMap.getImageIndex())).toShaderReadOnlyOptimal(reflectRefractTex.getCommandBuffers()[reflectRefractTex.getCurrentFrame()]);
                        const_cast<Texture&>(alphaBuffer.getTexture(alphaBuffer.getImageIndex())).toShaderReadOnlyOptimal(reflectRefractTex.getCommandBuffers()[reflectRefractTex.getCurrentFrame()]);
                        commandBuffers = reflectRefractTex.getCommandBuffers();
                        currentFrame = reflectRefractTex.getCurrentFrame();
                        for (unsigned int p = 0; p < (Batcher::nbPrimitiveTypes -1); p++) {
                            if (nbDrawCommandBuffer[p][i+2] > 0 || nbIndexedDrawCommandBuffer[p][i+2] > 0) {
                                vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS,reflectRefractTex.getGraphicPipeline()[sReflectRefract.getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][RRRCDEPTHNOSTENCIL*blendNone.nbBlendModes+blendNone.id]);
                                vkCmdPushConstants(commandBuffers[currentFrame], reflectRefractTex.getPipelineLayout()[sReflectRefract.getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][RRRCDEPTHNOSTENCIL*blendNone.nbBlendModes+blendNone.id], VK_SHADER_STAGE_FRAGMENT_BIT, 160, sizeof(unsigned int), &environmentMap.getImageIndex());
                            }
                        }
                        vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);

                        memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                        memoryBarrier.pNext = VK_NULL_HANDLE;
                        memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                        memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                        vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                        reflectRefractTex.beginRenderPass();
                        vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &reflectRefractCommandBuffer[i][currentFrame]);
                        reflectRefractTex.endRenderPass();
                        const_cast<Texture&>(alphaBuffer.getTexture(alphaBuffer.getImageIndex())).toColorAttachmentOptimal(reflectRefractTex.getCommandBuffers()[reflectRefractTex.getCurrentFrame()]);
                        const_cast<Texture&>(environmentMap.getTexture(environmentMap.getImageIndex())).toColorAttachmentOptimal(reflectRefractTex.getCommandBuffers()[reflectRefractTex.getCurrentFrame()]);
                        waitValues.clear();
                        waitValues.push_back(values[currentFrame]);
                        waitSemaphores.push_back(offscreenAlphaPassFinishedSemaphore[currentFrame]);
                        waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                        waitValues.push_back(0);
                        values[currentFrame]++;
                        signalValues.clear();
                        signalValues.push_back(values[currentFrame]);
                        reflectRefractTex.submit((i == nbReflRefrEntities - 1) ? true : false, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);

                        isSomethingDrawn = true;
                    }
                    if (!isSomethingDrawn) {

                        reflectRefractTex.beginRecordCommandBuffers();
                        waitSemaphores.clear();
                        waitSemaphores.push_back(offscreenAlphaPassFinishedSemaphore[reflectRefractTex.getCurrentFrame()]);
                        signalSemaphores.clear();
                        signalSemaphores.push_back(offscreenRenderingFinishedSemaphore[reflectRefractTex.getCurrentFrame()]);
                        waitStages.clear();
                        waitValues.clear();
                        waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                        values[reflectRefractTex.getCurrentFrame()]++;
                        signalValues.clear();
                        signalValues.push_back(values[reflectRefractTex.getCurrentFrame()]);
                        reflectRefractTex.submit(true, signalSemaphores, waitSemaphores, waitStages, signalValues);
                        //std::cout<<"reflect refract current frame : "<<reflectRefractTex.getCurrentFrame()<<std::endl;
                    }
                    isSomethingDrawn  = false;
                }
                target.beginRecordCommandBuffers();
                const_cast<Texture&>(reflectRefractTex.getTexture(reflectRefractTex.getImageIndex())).toShaderReadOnlyOptimal(window.getCommandBuffers()[window.getCurrentFrame()]);
                reflectRefractTexSprite.setCenter(target.getView().getPosition());
                reflectRefractTexSprite.setTexture(reflectRefractTex.getTexture(reflectRefractTex.getImageIndex()));
                /*if (&target == &window)
                    window.beginRenderPass();*/
                states.blendMode = BlendAlpha;
                target.draw(reflectRefractTexSprite, states);
                /*if (&target == &window)
                    window.endRenderPass();*/
                std::vector<VkSemaphore> signalSemaphores;
                std::vector<VkSemaphore> waitSemaphores;
                std::vector<VkPipelineStageFlags> waitStages;
                std::vector<uint64_t> waitValues, signalValues;
                waitSemaphores.push_back(offscreenRenderingFinishedSemaphore[reflectRefractTex.getCurrentFrame()]);
                waitStages.push_back(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                waitValues.push_back(values[reflectRefractTex.getCurrentFrame()]);
                signalSemaphores.push_back(offscreenRenderingFinishedSemaphore[reflectRefractTex.getCurrentFrame()]);
                values[reflectRefractTex.getCurrentFrame()]++;
                signalValues.push_back(values[reflectRefractTex.getCurrentFrame()]);
                target.submit(false, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);

                depthBuffer.display();
                alphaBuffer.display();
                environmentMap.display();
                reflectRefractTex.display();
                //std::cout<<"job wake up : "<<depthBuffer.getCurrentFrame()<<std::endl;
                registerFrameJob[depthBuffer.getCurrentFrame()] = true;
                cv.notify_one();
            }
            std::string ReflectRefractRenderComponent::getExpression() {
                return expression;
            }
            void ReflectRefractRenderComponent::setExpression (std::string expression) {
                this->expression = expression;
            }
            void ReflectRefractRenderComponent::setView(View view) {
                depthBuffer.setView(view);
                alphaBuffer.setView(view);
                reflectRefractTex.setView(view);
                this->view = view;
            }
            View& ReflectRefractRenderComponent::getView() {
                return view;
            }
            RenderTexture* ReflectRefractRenderComponent::getFrameBuffer() {
                return &reflectRefractTex;
            }
            int ReflectRefractRenderComponent::getLayer() {
                return getPosition().z();
            }
            bool ReflectRefractRenderComponent::needToUpdate() {
                return update;
            }
            void ReflectRefractRenderComponent::pushEvent(window::IEvent event, RenderWindow& rw) {
                if (&rw == &window && event.type == window::IEvent::WINDOW_EVENT && event.window.type == window::IEvent::WINDOW_EVENT_CLOSED) {
                    stop = true;
                    cv.notify_all();
                    getListener().stop();
                }
            }
            bool ReflectRefractRenderComponent::loadEntitiesOnComponent(std::vector<Entity*> vEntities) {
                {
                    std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                    datasReady = false;
                    batcher.clear();
                    normalBatcher.clear();
                    reflBatcher.clear();
                    reflNormalBatcher.clear();
                    indexedBatcher.clear();
                    normalIndexedBatcher.clear();
                    reflIndexedBatcher.clear();
                    reflNormalIndexedBatcher.clear();
                    skyboxBatcher.clear();
                }
                if (skybox != nullptr) {
                    for (unsigned int i = 0; i < skybox->getFaces().size(); i++) {
                        skyboxBatcher.addFace(skybox->getFace(i));
                    }
                }

                for (unsigned int i = 0; i < vEntities.size(); i++) {
                    if ( vEntities[i] != nullptr && vEntities[i]->isLeaf()) {
                        ////////std::cout<<"add entity"<<std::endl;
                        for (unsigned int j = 0; j <  vEntities[i]->getNbFaces(); j++) {
                            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                            if (vEntities[i]->getFace(j)->getMaterial().isReflectable() || vEntities[i]->getFace(j)->getMaterial().isRefractable()) {
                                if (vEntities[i]->getFace(j)->getVertexArray().getIndexes().size() == 0) {
                                    if (vEntities[i]->getDrawMode() == Entity::INSTANCED) {
                                        ////////std::cout<<"add refl face"<<std::endl;
                                        reflBatcher.addFace( vEntities[i]->getFace(j));
                                    } else {
                                        reflNormalBatcher.addFace(vEntities[i]->getFace(j));
                                    }
                                } else {
                                   if (vEntities[i]->getDrawMode() == Entity::INSTANCED) {
                                        ////////std::cout<<"add refl face"<<std::endl;
                                        reflIndexedBatcher.addFace( vEntities[i]->getFace(j));
                                    } else {
                                        reflNormalIndexedBatcher.addFace(vEntities[i]->getFace(j));
                                    }
                                }
                            } else {
                                if (vEntities[i]->getFace(j)->getVertexArray().getIndexes().size() == 0) {
                                    if (vEntities[i]->getDrawMode() == Entity::INSTANCED) {
                                        batcher.addFace( vEntities[i]->getFace(j));
                                    } else {
                                        normalBatcher.addFace(vEntities[i]->getFace(j));
                                    }
                                } else {
                                    if (vEntities[i]->getDrawMode() == Entity::INSTANCED) {
                                        indexedBatcher.addFace( vEntities[i]->getFace(j));
                                    } else {
                                        normalIndexedBatcher.addFace(vEntities[i]->getFace(j));
                                    }
                                }
                            }
                        }
                    }
                }
                std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                datasReady = true;
                return true;
            }
            ReflectRefractRenderComponent::~ReflectRefractRenderComponent() {
                vkDestroyCommandPool(vkDevice.getDevice(), commandPool, nullptr);

                for (unsigned int i = 0; i < offscreenDepthPassFinishedSemaphore.size(); i++) {
                    vkDestroySemaphore(vkDevice.getDevice(), offscreenDepthPassFinishedSemaphore[i], nullptr);
                }
                for (unsigned int i = 0; i < offscreenAlphaPassFinishedSemaphore.size(); i++) {
                    vkDestroySemaphore(vkDevice.getDevice(), offscreenAlphaPassFinishedSemaphore[i], nullptr);
                }
                for (unsigned int i = 0; i < copyFinishedSemaphore.size(); i++) {
                    vkDestroySemaphore(vkDevice.getDevice(), copyFinishedSemaphore[i], nullptr);
                }
                for (unsigned int i = 0; i < offscreenRenderingFinishedSemaphore.size(); i++) {
                    vkDestroySemaphore(vkDevice.getDevice(), offscreenRenderingFinishedSemaphore[i], nullptr);
                }


                for (unsigned int i = 0; i < events.size(); i++) {
                    vkDestroyEvent(vkDevice.getDevice(), events[i], nullptr);
                }
                for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    vkDestroySampler(vkDevice.getDevice(), headPtrTextureSampler[i], nullptr);
                    vkDestroyImageView(vkDevice.getDevice(), headPtrTextureImageView[i], nullptr);
                    vkDestroyImage(vkDevice.getDevice(), headPtrTextureImage[i], nullptr);
                    vkFreeMemory(vkDevice.getDevice(), headPtrTextureImageMemory[i], nullptr);

                    vkDestroySampler(vkDevice.getDevice(), depthTextureSampler[i], nullptr);
                    vkDestroyImageView(vkDevice.getDevice(), depthTextureImageView[i], nullptr);
                    vkDestroyImage(vkDevice.getDevice(), depthTextureImage[i], nullptr);
                    vkFreeMemory(vkDevice.getDevice(), depthTextureImageMemory[i], nullptr);

                    vkDestroySampler(vkDevice.getDevice(), alphaTextureSampler[i], nullptr);
                    vkDestroyImageView(vkDevice.getDevice(), alphaTextureImageView[i], nullptr);
                    vkDestroyImage(vkDevice.getDevice(), alphaTextureImage[i], nullptr);
                    vkFreeMemory(vkDevice.getDevice(), alphaTextureImageMemory[i], nullptr);
                }
                vkDestroyBuffer(vkDevice.getDevice(), vboCount, nullptr);
                vkFreeMemory(vkDevice.getDevice(), vboCountMemory, nullptr);
                for (size_t i = 0; i < counterShaderStorageBuffers.size(); i++) {
                    if (counterShaderStorageBuffers[i] != VK_NULL_HANDLE) {
                        vkDestroyBuffer(vkDevice.getDevice(), counterShaderStorageBuffers[i], nullptr);
                        vkFreeMemory(vkDevice.getDevice(), counterShaderStorageBuffersMemory[i], nullptr);
                    }
                }
                //////std::cout<<"counter ssbo destroyed"<<std::endl;
                for (unsigned int i = 0; i < linkedListShaderStorageBuffers.size(); i++) {
                    vkDestroyBuffer(vkDevice.getDevice(), linkedListShaderStorageBuffers[i], nullptr);
                    vkFreeMemory(vkDevice.getDevice(), linkedListShaderStorageBuffersMemory[i], nullptr);
                }
                //////std::cout<<"linked list ssbo destroyed"<<std::endl;
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
                for (size_t i = 0; i < uniformBuffer.size(); i++) {
                    vkDestroyBuffer(vkDevice.getDevice(), uniformBuffer[i], nullptr);
                    vkFreeMemory(vkDevice.getDevice(), uniformBufferMemory[i], nullptr);
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
                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, copyModelDataBufferCommandBuffer.size(), copyModelDataBufferCommandBuffer.data());

                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, copyMaterialDataBufferCommandBuffer.size(), copyMaterialDataBufferCommandBuffer.data());

                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, copyDrawBufferCommandBuffer.size(), copyDrawBufferCommandBuffer.data());

                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, copyDrawIndexedBufferCommandBuffer.size(), copyDrawIndexedBufferCommandBuffer.data());
                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, copyVbBufferCommandBuffer.size(), copyVbBufferCommandBuffer.data());
                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, copyVbIndexedBufferCommandBuffer.size(), copyVbIndexedBufferCommandBuffer.data());
                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, copyVbEnvPass2BufferCommandBuffer.size(), copyVbEnvPass2BufferCommandBuffer.data());
                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, depthBufferCommandBuffer.size(), depthBufferCommandBuffer.data());
                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, alphaBufferCommandBuffer.size(), alphaBufferCommandBuffer.data());
                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, environmentMapPass2CommandBuffer.size(), environmentMapPass2CommandBuffer.data());
                vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, copySkyboxCommandBuffer.size(), copySkyboxCommandBuffer.data());
                for (unsigned int i = 0; i < environmentMapCommandBuffer.size(); i++) {
                    vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, skyboxCommandBuffer[i].size(), skyboxCommandBuffer[i].data());
                    vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, environmentMapCommandBuffer[i].size(), environmentMapCommandBuffer[i].data());
                    vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, reflectRefractCommandBuffer[i].size(), reflectRefractCommandBuffer[i].data());
                }
                vkDestroyCommandPool(vkDevice.getDevice(), secondaryBufferCommandPool, nullptr);
                //////std::cout<<"indirect vbo destroyed"<<std::endl;
            }
        #else
        ReflectRefractRenderComponent::ReflectRefractRenderComponent (RenderWindow& window, int layer, std::string expression, window::ContextSettings settings) :
            HeavyComponent(window, math::Vec3f(window.getView().getPosition().x(), window.getView().getPosition().y(), layer),
                          math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0),
                          math::Vec3f(window.getView().getSize().x() + window.getView().getSize().x() * 0.5f, window.getView().getPosition().y() + window.getView().getSize().y() * 0.5f, layer)),
            view(window.getView()),
            expression(expression)
            {
            if (window.getView().getSize().x() > window.getView().getSize().y()) {
                squareSize = window.getView().getSize().x();
            } else {
                squareSize = window.getView().getSize().y();
            }
            datasReady = false;
            quad = RectangleShape(math::Vec3f(squareSize, squareSize, squareSize * 0.5f));
            quad.move(math::Vec3f(-squareSize * 0.5f, -squareSize * 0.5f, 0));
            dirs[0] = math::Vec3f(1, 0, 0);
            dirs[1] = math::Vec3f(-1, 0, 0);
            dirs[2] = math::Vec3f(0, 1, 0);
            dirs[3] = math::Vec3f(0, -1, 0);
            dirs[4] = math::Vec3f(0, 0, 1);
            dirs[5] = math::Vec3f(0, 0, -1);
            ups[0] = math::Vec3f(0, -1, 0);
            ups[1] = math::Vec3f(0, -1, 0);
            ups[2] = math::Vec3f(0, 0, 1);
            ups[3] = math::Vec3f(0, 0, -1);
            ups[4] = math::Vec3f(0, -1, 0);
            ups[5] = math::Vec3f(0, -1, 0);
            depthBuffer.create(window.getView().getSize().x(), window.getView().getSize().y(), settings);
            glCheck(glGenTextures(1, &depthTex));
            glCheck(glBindTexture(GL_TEXTURE_2D, depthTex));
            glCheck(glTexStorage2D(GL_TEXTURE_2D,1,GL_RGBA32F,window.getView().getSize().x(), window.getView().getSize().y()));
            glCheck(glBindImageTexture(0, depthTex, 1, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F));
            glCheck(glBindTexture(GL_TEXTURE_2D, 0));
            std::vector<GLfloat> depthClearBuf(window.getView().getSize().x()*window.getView().getSize().y()*4, 0);
            glCheck(glGenBuffers(1, &clearBuf2));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf2));
            glCheck(glBufferData(GL_PIXEL_UNPACK_BUFFER, depthClearBuf.size() * sizeof(GLfloat),
            &depthClearBuf[0], GL_STATIC_COPY));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
            depthBufferSprite = Sprite(depthBuffer.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0), IntRect(0, 0, window.getView().getSize().x(), window.getView().getSize().y()));
            settings.depthBits = 0;
            alphaBuffer.create(window.getView().getSize().x(), window.getView().getSize().y(), settings);
            glCheck(glGenTextures(1, &alphaTex));
            glCheck(glBindTexture(GL_TEXTURE_2D, alphaTex));
            glCheck(glTexStorage2D(GL_TEXTURE_2D,1,GL_RGBA32F,window.getView().getSize().x(), window.getView().getSize().y()));
            glCheck(glBindImageTexture(0, alphaTex, 1, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F));
            glCheck(glBindTexture(GL_TEXTURE_2D, 0));
            std::vector<GLfloat> alphaClearBuf(window.getView().getSize().x()*window.getView().getSize().y()*4, 0);
            glCheck(glGenBuffers(1, &clearBuf3));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf3));
            glCheck(glBufferData(GL_PIXEL_UNPACK_BUFFER, alphaClearBuf.size() * sizeof(GLfloat),
            &alphaClearBuf[0], GL_STATIC_COPY));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
            alphaBufferSprite = Sprite(alphaBuffer.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0), IntRect(0, 0, window.getView().getSize().x(), window.getView().getSize().y()));
            math::Vec3f resolution ((int) window.getSize().x(), (int) window.getSize().y(), window.getView().getSize().z());
            settings.depthBits = 24;
            settings.stencilBits = 8;
            reflectRefractTex.create(window.getView().getSize().x(), window.getView().getSize().y(), settings);
            reflectRefractTex.setEnableCubeMap(true);
            reflectRefractTexSprite = Sprite(reflectRefractTex.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0), IntRect(0, 0, window.getView().getSize().x(), window.getView().getSize().y()));
            settings.depthBits = 0;
            environmentMap.create(squareSize, squareSize, settings, GL_TEXTURE_CUBE_MAP);
            GLuint maxNodes = 20 * squareSize * squareSize;
            GLint nodeSize = 5 * sizeof(GLfloat) + sizeof(GLuint);
            glCheck(glGenBuffers(1, &atomicBuffer));
            glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicBuffer));
            glCheck(glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint) * 6, nullptr, GL_DYNAMIC_DRAW));
            glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0));
            glCheck(glGenBuffers(1, &linkedListBuffer));
            glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, linkedListBuffer));
            glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, maxNodes * nodeSize * 6, NULL, GL_DYNAMIC_DRAW));
            glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
            glCheck(glEnable(GL_TEXTURE_3D));
            glCheck(glGenTextures(1, &headPtrTex));
            glCheck(glBindTexture(GL_TEXTURE_3D, headPtrTex));
            glCheck(glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, squareSize, squareSize, 6));
            glCheck(glBindImageTexture(0, headPtrTex, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI));
            glCheck(glBindTexture(GL_TEXTURE_3D, 0));
            glCheck(glDisable(GL_TEXTURE_3D));
            std::vector<GLuint> headPtrClearBuf(squareSize*squareSize*6, 0xffffffff);
            glCheck(glGenBuffers(1, &clearBuf));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf));
            glCheck(glBufferData(GL_PIXEL_UNPACK_BUFFER, headPtrClearBuf.size() * sizeof(GLuint),
            &headPtrClearBuf[0], GL_STATIC_COPY));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
            core::FastDelegate<bool> signal (&ReflectRefractRenderComponent::needToUpdate, this);
            core::FastDelegate<void> slot (&ReflectRefractRenderComponent::drawNextFrame, this);
            core::Command cmd(signal, slot);
            getListener().connect("UPDATE", cmd);
            glCheck(glGenBuffers(1, &vboIndirect));
            glCheck(glGenBuffers(1, &modelDataBuffer));
            glCheck(glGenBuffers(1, &materialDataBuffer));
            if (settings.versionMajor >= 4 && settings.versionMinor >= 3) {
                const std::string skyboxVertexShader = R"(#version 460
                                                         layout (location = 0) in vec3 aPos;
                                                         out vec3 vTexCoord;
                                                         void main()
                                                         {
                                                             vTexCoord = aPos;
                                                             gl_Position = vec4(aPos, 1.0);
                                                         }
                                                         )";
                const std::string linkedListIndirectRenderingVertexShader = R"(#version 460
                                                                               layout (location = 0) in vec3 position;
                                                                               layout (location = 1) in vec4 color;
                                                                               layout (location = 2) in vec2 texCoords;
                                                                               layout (location = 3) in vec3 normals;
                                                                               uniform mat4 textureMatrix[)"+core::conversionUIntString(Texture::getAllTextures().size())+R"(];
                                                                               struct ModelData {
                                                                                   mat4 modelMatrix;
                                                                               };
                                                                               struct MaterialData {
                                                                                   uint textureIndex;
                                                                                   uint layer;
                                                                                   uint materialType;
                                                                               };
                                                                               layout(binding = 1, std430) buffer modelData {
                                                                                   ModelData modelDatas[];
                                                                               };
                                                                               layout(binding = 2, std430) buffer materialData {
                                                                                   MaterialData materialDatas[];
                                                                               };
                                                                               out vec4 vColor;
                                                                               out vec2 vTexCoord;
                                                                               out uint tIndex;
                                                                               void main() {
                                                                                    MaterialData material = materialDatas[gl_DrawID];
                                                                                    ModelData model = modelDatas[gl_BaseInstance + gl_InstanceID];
                                                                                    uint textureIndex = material.textureIndex;
                                                                                    gl_Position = model.modelMatrix * vec4(position, 1.f);
                                                                                    vTexCoord = (textureIndex != 0) ? (textureMatrix[textureIndex-1] * vec4(texCoords, 1.f, 1.f)).xy : texCoords;
                                                                                    vColor = color;
                                                                                    tIndex = textureIndex;
                                                                               }
                                                                               )";
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
                                                                         uint materialType;
                                                                     };
                                                                     layout(binding = 1, std430) buffer modelData {
                                                                         ModelData modelDatas[];
                                                                     };
                                                                     layout(binding = 2, std430) buffer materialData {
                                                                         MaterialData materialDatas[];
                                                                     };
                                                                     out vec2 fTexCoords;
                                                                     out vec4 frontColor;
                                                                     out uint texIndex;
                                                                     out uint layer;
                                                                     void main() {
                                                                         MaterialData material = materialDatas[gl_DrawID];
                                                                         ModelData model = modelDatas[gl_BaseInstance + gl_InstanceID];
                                                                         uint textureIndex = material.textureIndex;
                                                                         uint l = material.layer;
                                                                         gl_Position = projectionMatrix * viewMatrix * model.modelMatrix * vec4(position, 1.f);
                                                                         fTexCoords = (textureIndex != 0) ? (textureMatrix[textureIndex-1] * vec4(texCoords, 1.f, 1.f)).xy : texCoords;
                                                                         frontColor = color;
                                                                         texIndex = textureIndex;
                                                                         layer = l;
                                                                     }
                                                                     )";
                const std::string  linkedListVertexShader2 = R"(#version 460
                                                                layout (location = 0) in vec3 position;
                                                                uniform mat4 projectionMatrix;
                                                                uniform mat4 viewMatrix;
                                                                uniform mat4 worldMat;
                                                                void main () {
                                                                    gl_Position = projectionMatrix * viewMatrix * worldMat * vec4(position, 1.f);
                                                                })";
                const std::string perPixReflectRefractIndirectRenderingVertexShader = R"(#version 460
                                                                                         layout (location = 0) in vec3 position;
                                                                                         layout (location = 1) in vec4 color;
                                                                                         layout (location = 2) in vec2 texCoords;
                                                                                         layout (location = 3) in vec3 normals;
                                                                                         uniform mat4 projectionMatrix;
                                                                                         uniform mat4 viewMatrix;
                                                                                         struct ModelData {
                                                                                             mat4 modelMatrix;
                                                                                         };
                                                                                         struct MaterialData {
                                                                                             uint textureIndex;
                                                                                             uint layer;
                                                                                             uint materialType;
                                                                                         };
                                                                                         layout(binding = 1, std430) buffer modelData {
                                                                                             ModelData modelDatas[];
                                                                                         };
                                                                                         layout(binding = 2, std430) buffer materialData {
                                                                                             MaterialData materialDatas[];
                                                                                         };
                                                                                         out vec3 pos;
                                                                                         out vec4 frontColor;
                                                                                         out vec3 normal;
                                                                                         out uint materialType;
                                                                                         void main () {
                                                                                             MaterialData material = materialDatas[gl_DrawID];
                                                                                             ModelData model = modelDatas[gl_BaseInstance + gl_InstanceID];
                                                                                             uint materialT = material.materialType;
                                                                                             pos = vec3(model.modelMatrix * vec4(position, 1.0));
                                                                                             gl_Position = projectionMatrix * viewMatrix * model.modelMatrix * vec4(position, 1.f);
                                                                                             frontColor = color;
                                                                                             normal = mat3(transpose(inverse(model.modelMatrix))) * normals;
                                                                                             materialType = materialT;
                                                                                         }
                                                                                         )";
                const std::string geometryShader = R"(#version 460
                                                      #extension GL_EXT_geometry_shader4 : enable
                                                      layout (triangles) in;
                                                      layout (triangle_strip, max_vertices = 18) out;
                                                      in vec4 vColor[];
                                                      in vec2 vTexCoord[];
                                                      in uint tIndex[];
                                                      out vec4 frontColor;
                                                      out vec2 fTexCoords;
                                                      out uint layer;
                                                      out uint texIndex;
                                                      uniform mat4 projMatrices[6];
                                                      uniform mat4 viewMatrices[6];
                                                      void main() {
                                                        for (int face = 0; face < 6; face++) {
                                                            gl_Layer = face;
                                                            for (uint i = 0; i < 3; i++) {
                                                                gl_Position = projMatrices[face] * viewMatrices[face] * gl_in[i].gl_Position;
                                                                frontColor = vColor[i];
                                                                fTexCoords = vTexCoord[i];
                                                                texIndex = tIndex[i];
                                                                layer = face;
                                                                EmitVertex();
                                                            }
                                                            EndPrimitive();
                                                        }
                                                      }
                                                      )";
                const std::string geometryShader2 = R"(#version 460
                                                      #extension GL_EXT_geometry_shader4 : enable
                                                      layout (triangles) in;
                                                      layout (triangle_strip, max_vertices = 18) out;
                                                      out uint layer;
                                                      void main() {
                                                        for (int face = 0; face < 6; face++) {
                                                            gl_Layer = face;
                                                            for (uint i = 0; i < 3; i++) {
                                                                gl_Position = gl_in[i].gl_Position;
                                                                layer = face;
                                                                EmitVertex();
                                                            }
                                                            EndPrimitive();
                                                        }
                                                      }
                                                      )";
                const std::string skyboxGeometryShader = R"(#version 460
                                                      #extension GL_EXT_geometry_shader4 : enable
                                                      layout (triangles) in;
                                                      layout (triangle_strip, max_vertices = 18) out;
                                                      in vec3 vTexCoord[];
                                                      out vec3 fTexCoords;
                                                      uniform mat4 projMatrices[6];
                                                      uniform mat4 viewMatrices[6];
                                                      void main() {
                                                        for (int face = 0; face < 6; face++) {
                                                            gl_Layer = face;
                                                            for (int i = 0; i < 3; i++) {
                                                                gl_Position = projMatrices[face] * viewMatrices[face] * gl_in[i].gl_Position;
                                                                fTexCoords = vTexCoord[i];
                                                                EmitVertex();
                                                            }
                                                            EndPrimitive();
                                                        }
                                                      }
                                                      )";
                const std::string skyboxFragmentShader = R"(#version 460
                                                            layout (location = 0) out vec4 fcolor;
                                                            in vec3 fTexCoords;
                                                            uniform samplerCube skybox;
                                                            void main() {
                                                                fcolor = texture(skybox, fTexCoords);
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
                                                                          uniform sampler2D texture;
                                                                          uniform float haveTexture;
                                                                          uniform uint nbLayers;

                                                                          layout(binding = 0, rgba32f) coherent uniform image2D depthBuffer;
                                                                          layout (location = 0) out vec4 fColor;

                                                                          void main () {
                                                                              vec4 texel = (texIndex != 0) ? frontColor * texture2D(textures[texIndex-1], fTexCoords.xy) : frontColor;
                                                                              float z = gl_FragCoord.z;
                                                                              float l = float(layer) / float(nbLayers);
                                                                              beginInvocationInterlockARB();
                                                                              vec4 depth = imageLoad(depthBuffer,ivec2(gl_FragCoord.xy));
                                                                              if (l > depth.y || l == depth.y && z > depth.z) {
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
                                                                      #extension GL_ARB_bindless_texture : enable
                                                                      #extension GL_ARB_fragment_shader_interlock : require
                                                                      layout(std140, binding=0) uniform ALL_TEXTURES {
                                                                        sampler2D textures[200];
                                                                      };
                                                                      layout(binding = 0, rgba32f) coherent uniform image2D alphaBuffer;
                                                                      layout (location = 0) out vec4 fColor;
                                                                      uniform sampler2D texture;
                                                                      uniform sampler2D depthBuffer;
                                                                      uniform float haveTexture;
                                                                      uniform uint nbLayers;
                                                                      uniform vec3 resolution;
                                                                      in vec4 frontColor;
                                                                      in vec2 fTexCoords;
                                                                      in flat uint texIndex;
                                                                      in flat uint layer;
                                                                      void main() {
                                                                          vec4 texel = (texIndex != 0) ? frontColor * texture2D(textures[texIndex-1], fTexCoords.xy) : frontColor;
                                                                          float current_alpha = texel.a;
                                                                          vec2 position = (gl_FragCoord.xy / resolution.xy);
                                                                          vec4 depth = texture2D (depthBuffer, position);
                                                                          beginInvocationInterlockARB();
                                                                          vec4 alpha = imageLoad(alphaBuffer,ivec2(gl_FragCoord.xy));
                                                                          float l = float(layer) / float(nbLayers);
                                                                          float z = gl_FragCoord.z;
                                                                          if ((l > depth.y || l == depth.y && z > depth.z) && current_alpha > alpha.a) {
                                                                              imageStore(alphaBuffer,ivec2(gl_FragCoord.xy),vec4(0, l, z, current_alpha));
                                                                              memoryBarrier();
                                                                              fColor = vec4(0, l, z, current_alpha);
                                                                          } else {
                                                                              fColor = alpha;
                                                                          }
                                                                          endInvocationInterlockARB();
                                                                      }
                                                                      )";
                const std::string buildFramebufferShader = R"(#version 460
                                                                in vec4 frontColor;
                                                                in vec3 pos;
                                                                in flat vec3 normal;
                                                                in flat uint materialType;
                                                                uniform vec3 cameraPos;
                                                                uniform samplerCube sceneBox;
                                                                uniform sampler2D alphaBuffer;
                                                                uniform vec3 resolution;
                                                                layout (location = 0) out vec4 fColor;
                                                                void main () {
                                                                    vec2 position = (gl_FragCoord.xy / resolution.xy);
                                                                    vec4 alpha = texture2D(alphaBuffer, position);
                                                                    bool refr = false;
                                                                    float ratio = 1;
                                                                    if (materialType == 1) {
                                                                        ratio = 1.00 / 1.33;
                                                                        refr = true;
                                                                    } else if (materialType == 2) {
                                                                        ratio = 1.00 / 1.309;
                                                                        refr = true;
                                                                    } else if (materialType == 3) {
                                                                        ratio = 1.00 / 1.52;
                                                                        refr = true;
                                                                    } else if (materialType == 4) {
                                                                        ratio = 1.00 / 2.42;
                                                                        refr = true;
                                                                    }
                                                                    vec3 i = (pos - cameraPos);
                                                                    if (refr) {
                                                                        vec3 r = refract (i, normalize(normal), ratio);
                                                                        fColor = texture(sceneBox, r) * (1 - alpha.a);
                                                                    } else {
                                                                        vec3 r = reflect (i, normalize(normal));
                                                                        fColor = texture(sceneBox, r) * (1 - alpha.a);
                                                                    }
                                                                }
                                                              )";
                const std::string fragmentShader = R"(#version 460
                                                      #extension GL_ARB_bindless_texture : enable
                                                      struct NodeType {
                                                          vec4 color;
                                                          float depth;
                                                          uint next;
                                                      };
                                                      layout(binding = 0, offset = 0) uniform atomic_uint nextNodeCounter1;
                                                      layout(binding = 0, offset = 4) uniform atomic_uint nextNodeCounter2;
                                                      layout(binding = 0, offset = 8) uniform atomic_uint nextNodeCounter3;
                                                      layout(binding = 0, offset = 12) uniform atomic_uint nextNodeCounter4;
                                                      layout(binding = 0, offset = 16) uniform atomic_uint nextNodeCounter5;
                                                      layout(binding = 0, offset = 20) uniform atomic_uint nextNodeCounter6;
                                                      layout(binding = 0, r32ui) uniform uimage3D headPointers;
                                                      layout(binding = 0, std430) buffer linkedLists {
                                                          NodeType nodes[];
                                                      };
                                                      layout(std140, binding=0) uniform ALL_TEXTURES {
                                                          sampler2D textures[200];
                                                      };
                                                      uniform uint maxNodes;
                                                      uniform float haveTexture;
                                                      uniform sampler2D texture;
                                                      in vec4 frontColor;
                                                      in vec2 fTexCoords;
                                                      in flat uint texIndex;
                                                      in flat uint layer;
                                                      layout (location = 0) out vec4 fcolor;
                                                      void main() {
                                                           uint nodeIdx;
                                                           if (layer == 0)
                                                                nodeIdx = atomicCounterIncrement(nextNodeCounter1);
                                                           else if (layer == 1)
                                                                nodeIdx = atomicCounterIncrement(nextNodeCounter2);
                                                           else if (layer == 2)
                                                                nodeIdx = atomicCounterIncrement(nextNodeCounter3);
                                                           else if (layer == 3)
                                                                nodeIdx = atomicCounterIncrement(nextNodeCounter4);
                                                           else if (layer == 4)
                                                                nodeIdx = atomicCounterIncrement(nextNodeCounter5);
                                                           else
                                                                nodeIdx = atomicCounterIncrement(nextNodeCounter6);
                                                           vec4 texel = (texIndex != 0) ? frontColor * texture2D(textures[texIndex-1], fTexCoords.xy) : frontColor;
                                                           if (nodeIdx < maxNodes) {
                                                                uint prevHead = imageAtomicExchange(headPointers, ivec3(gl_FragCoord.xy, layer), nodeIdx);
                                                                nodes[nodeIdx+layer*maxNodes].color = texel;
                                                                nodes[nodeIdx+layer*maxNodes].depth = gl_FragCoord.z;
                                                                nodes[nodeIdx+layer*maxNodes].next = prevHead;
                                                           }
                                                           fcolor = vec4(0, 0, 0, 0);
                                                      })";
                 const std::string fragmentShader2 =
                   R"(
                   #version 460
                   #define MAX_FRAGMENTS 20
                   struct NodeType {
                      vec4 color;
                      float depth;
                      uint next;
                   };
                   layout(binding = 0, r32ui) uniform uimage3D headPointers;
                   layout(binding = 0, std430) buffer linkedLists {
                       NodeType nodes[];
                   };
                   layout(location = 0) out vec4 fcolor;
                   uniform uint maxNodes;
                   in flat uint layer;
                   void main() {
                      NodeType frags[MAX_FRAGMENTS];
                      int count = 0;
                      uint n = imageLoad(headPointers, ivec3(gl_FragCoord.xy, layer)).r;
                      while( n != 0xffffffffu && count < MAX_FRAGMENTS) {
                           frags[count] = nodes[n+maxNodes*layer];
                           n = frags[count].next/*+maxNodes*layer*/;
                           count++;
                      }
                      //merge sort
                      /*int i, j1, j2, k;
                      int a, b, c;
                      int step = 1;
                      NodeType leftArray[MAX_FRAGMENTS/2]; //for merge sort
                      while (step <= count)
                      {
                          i = 0;
                          while (i < count - step)
                          {
                              ////////////////////////////////////////////////////////////////////////
                              //merge(step, i, i + step, min(i + step + step, count));
                              a = i;
                              b = i + step;
                              c = (i + step + step) >= count ? count : (i + step + step);
                              for (k = 0; k < step; k++)
                                  leftArray[k] = frags[a + k];
                              j1 = 0;
                              j2 = 0;
                              for (k = a; k < c; k++)
                              {
                                  if (b + j1 >= c || (j2 < step && leftArray[j2].depth > frags[b + j1].depth))
                                      frags[k] = leftArray[j2++];
                                  else
                                      frags[k] = frags[b + j1++];
                              }
                              ////////////////////////////////////////////////////////////////////////
                              i += 2 * step;
                          }
                          step *= 2;
                      }*/
                      //Insertion sort.
                      for (int i = 0; i < count - 1; i++) {
                        for (int j = i + 1; j > 0; j--) {
                            if (frags[j - 1].depth > frags[j].depth) {
                                NodeType tmp = frags[j - 1];
                                frags[j - 1] = frags[j];
                                frags[j] = tmp;
                            }
                        }
                      }
                      vec4 color = vec4(0, 0, 0, 0);
                      for( int i = 0; i < count; i++)
                      {
                        color.rgb = frags[i].color.rgb * frags[i].color.a + color.rgb * (1 - frags[i].color.a);
                        color.a = frags[i].color.a + color.a * (1 - frags[i].color.a);
                      }
                      fcolor = color;
                   })";
                if (!skyboxShader.loadFromMemory(skyboxVertexShader, skyboxFragmentShader, skyboxGeometryShader)) {
                    throw core::Erreur(49, "Failed to load skybox shader");
                }
                if (!sBuildDepthBuffer.loadFromMemory(indirectRenderingVertexShader, buildDepthBufferFragmentShader)) {
                    throw core::Erreur(50, "Error, failed to load build depth buffer shader", 3);
                }
                if (!sReflectRefract.loadFromMemory(perPixReflectRefractIndirectRenderingVertexShader, buildFramebufferShader)) {
                    throw core::Erreur(57, "Error, failed to load reflect refract shader", 3);
                }
                if (!sLinkedList.loadFromMemory(linkedListIndirectRenderingVertexShader, fragmentShader, geometryShader)) {
                    throw core::Erreur(58, "Error, failed to load per pixel linked list shader", 3);
                }
                if (!sLinkedList2.loadFromMemory(linkedListVertexShader2, fragmentShader2, geometryShader2)) {
                    throw core::Erreur(59, "Error, failed to load per pixel linked list 2 shader", 3);
                }
                if (!sBuildAlphaBuffer.loadFromMemory(indirectRenderingVertexShader,buildAlphaBufferFragmentShader)) {
                    throw core::Erreur(60, "Error, failed to load build alpha buffer shader", 3);
                }
                skyboxShader.setParameter("skybox", Shader::CurrentTexture);
                sBuildDepthBuffer.setParameter("texture", Shader::CurrentTexture);
                sBuildAlphaBuffer.setParameter("texture", Shader::CurrentTexture);
                sBuildAlphaBuffer.setParameter("depthBuffer", depthBuffer.getTexture());
                sBuildAlphaBuffer.setParameter("resolution", resolution.x(), resolution.y(), resolution.z());
                sReflectRefract.setParameter("resolution", resolution.x(), resolution.y(), resolution.z());
                sReflectRefract.setParameter("alphaBuffer", alphaBuffer.getTexture());
                sReflectRefract.setParameter("sceneBox", Shader::CurrentTexture);
                sLinkedList.setParameter("maxNodes", maxNodes);
                sLinkedList.setParameter("texture", Shader::CurrentTexture);
                View defaultView = window.getDefaultView();
                defaultView.setPerspective(-squareSize * 0.5f, squareSize * 0.5f, -squareSize * 0.5f, squareSize * 0.5f, 0, 1000);
                math::Matrix4f viewMatrix = defaultView.getViewMatrix().getMatrix().transpose();
                math::Matrix4f projMatrix = defaultView.getProjMatrix().getMatrix().transpose();
                sLinkedList2.setParameter("viewMatrix", viewMatrix);
                sLinkedList2.setParameter("projectionMatrix", projMatrix);
                sLinkedList2.setParameter("maxNodes", maxNodes);
            }
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
            sBuildDepthBuffer.setParameter("textureMatrix", textureMatrices);
            sBuildAlphaBuffer.setParameter("textureMatrix", textureMatrices);
            sLinkedList.setParameter("textureMatrix", textureMatrices);



            ////////std::cout<<"ubid : "<<ubid<<std::endl;
            backgroundColor = Color::Transparent;
            glCheck(glGenBuffers(1, &ubo));
            glCheck(glBindBuffer(GL_UNIFORM_BUFFER, ubo));
            glCheck(glBufferData(GL_UNIFORM_BUFFER, sizeof(Samplers),allSamplers.tex, GL_STATIC_DRAW));
            glCheck(glBindBuffer(GL_UNIFORM_BUFFER, 0));
            glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
            glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicBuffer));
            glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, linkedListBuffer));
            glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, modelDataBuffer));
            glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, materialDataBuffer));
            depthBuffer.setActive();
            glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
            glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, modelDataBuffer));
            glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, materialDataBuffer));
            alphaBuffer.setActive();
            glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
            glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, modelDataBuffer));
            glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, materialDataBuffer));
            reflectRefractTex.setActive();
            ////////std::cout<<"size : "<<sizeof(Samplers)<<" "<<alignof (alignas(16) uint64_t[200])<<std::endl;

            /*glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo2));
            glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo3));*/
            glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, modelDataBuffer));
            glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, materialDataBuffer));

            for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                vbBindlessTex[i].setPrimitiveType(static_cast<PrimitiveType>(i));
            }
            skybox = nullptr;
        }
        void ReflectRefractRenderComponent::loadTextureIndexes() {
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
        void ReflectRefractRenderComponent::onVisibilityChanged(bool visible) {
            if (visible) {
                glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicBuffer));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, linkedListBuffer));
            } else {
                glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicBuffer));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, linkedListBuffer));
            }
        }
        void ReflectRefractRenderComponent::drawDepthReflInst() {
            for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                vbBindlessTex[p].clear();
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
            for (unsigned int i = 0; i < m_reflNormals.size(); i++) {
                if (m_reflNormals[i].getAllVertices().getVertexCount() > 0) {
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    unsigned int p = m_reflNormals[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = (m_reflNormals[i].getMaterial().getTexture() != nullptr) ? m_reflNormals[i].getMaterial().getTexture()->getNativeHandle() : 0;
                    material.layer = m_reflNormals[i].getMaterial().getLayer();
                    materials[p].push_back(material);
                    ModelData model;
                    TransformMatrix tm;
                    model.worldMat = tm.getMatrix().transpose();
                    matrices[p].push_back(model);
                    unsigned int vertexCount = 0;
                    for (unsigned int j = 0; j < m_reflNormals[i].getAllVertices().getVertexCount(); j++) {
                        vertexCount++;
                        vbBindlessTex[p].append(m_reflNormals[i].getAllVertices()[j]);
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
            for (unsigned int i = 0; i < m_reflInstances.size(); i++) {
                if (m_reflInstances[i].getAllVertices().getVertexCount() > 0) {
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    unsigned int p = m_reflInstances[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = (m_reflInstances[i].getMaterial().getTexture() != nullptr) ? m_reflInstances[i].getMaterial().getTexture()->getNativeHandle() : 0;
                    material.layer = m_reflInstances[i].getMaterial().getLayer();
                    materials[p].push_back(material);
                    std::vector<TransformMatrix*> tm = m_reflInstances[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData model;
                        model.worldMat = tm[j]->getMatrix().transpose();
                        matrices[p].push_back(model);
                    }
                    unsigned int vertexCount = 0;
                    if (m_reflInstances[i].getVertexArrays().size() > 0) {
                        Entity* entity = m_reflInstances[i].getVertexArrays()[0]->getEntity();
                        for (unsigned int j = 0; j < m_reflInstances[i].getVertexArrays().size(); j++) {
                            if (entity == m_reflInstances[i].getVertexArrays()[j]->getEntity()) {

                                unsigned int p = m_reflInstances[i].getVertexArrays()[j]->getPrimitiveType();
                                for (unsigned int k = 0; k < m_reflInstances[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                    vertexCount++;
                                    vbBindlessTex[p].append((*m_reflInstances[i].getVertexArrays()[j])[k]);
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
            currentStates.shader = &sBuildDepthBuffer;
            currentStates.texture = nullptr;
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
                    depthBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), currentStates, vboIndirect);
                    vbBindlessTex[p].clear();
                }
            }
            glCheck(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
            depthBuffer.display();
        }
        void ReflectRefractRenderComponent::drawAlphaInst() {
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
                    ////////std::cout<<"layer : "<<layer<<" nb layers : "<<Entity::getNbLayers()<<std::endl;
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
            currentStates.shader = &sBuildAlphaBuffer;
            currentStates.texture = nullptr;
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
                    vbBindlessTex[p].clear();
                }
            }
            glCheck(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
            alphaBuffer.display();
        }
        void ReflectRefractRenderComponent::drawEnvReflInst() {
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
                    unsigned int p = m_instances[i].getAllVertices().getPrimitiveType();MaterialData material;
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
            currentStates.shader = &sLinkedList;
            currentStates.texture = nullptr;
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
                    environmentMap.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), currentStates, vboIndirect);
                    vbBindlessTex[p].clear();
                }
            }
        }
        void ReflectRefractRenderComponent::drawReflInst(Entity* reflectEntity) {
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
            for (unsigned int i = 0; i < m_reflNormals.size(); i++) {
                if (m_reflNormals[i].getVertexArrays().size() > 0) {
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    unsigned int p = m_reflNormals[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = (m_reflNormals[i].getMaterial().getTexture() != nullptr) ? m_reflNormals[i].getMaterial().getTexture()->getNativeHandle() : 0;
                    material.materialType = m_reflNormals[i].getMaterial().getType();
                    materials[p].push_back(material);
                    ModelData model;
                    TransformMatrix tm;
                    model.worldMat = tm.getMatrix().transpose();
                    matrices[p].push_back(model);
                    unsigned int vertexCount = 0;
                    for (unsigned int j = 0; j < m_reflNormals[i].getVertexArrays().size(); j++) {
                        Entity* entity = m_reflNormals[i].getVertexArrays()[j]->getEntity()->getRootEntity();
                        if (entity == reflectEntity) {
                            for (unsigned int k = 0; k < m_reflNormals[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                vertexCount++;
                                math::Vec3f t = m_reflNormals[i].getVertexArrays()[j]->getEntity()->getTransform().transform((*m_reflNormals[i].getVertexArrays()[j])[k].position);
                                Vertex v (t, (*m_reflNormals[i].getVertexArrays()[j])[k].color, (*m_reflNormals[i].getVertexArrays()[j])[k].texCoords);
                                v.normal = (*m_reflNormals[i].getVertexArrays()[j])[k].normal;
                                vbBindlessTex[p].append(v);
                            }
                        }
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
            for (unsigned int i = 0; i < m_reflInstances.size(); i++) {
                if (m_reflInstances[i].getAllVertices().getVertexCount() > 0) {
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    unsigned int p = m_reflInstances[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = (m_reflInstances[i].getMaterial().getTexture() != nullptr) ? m_reflInstances[i].getMaterial().getTexture()->getNativeHandle() : 0;
                    material.materialType = m_reflInstances[i].getMaterial().getType();
                    materials[p].push_back(material);
                    std::vector<TransformMatrix*> tm = m_reflInstances[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData model;
                        model.worldMat = tm[j]->getMatrix().transpose();
                        matrices[p].push_back(model);
                    }
                    unsigned int vertexCount = 0;
                    if (m_reflInstances[i].getVertexArrays().size() > 0) {
                        Entity* entity = m_reflInstances[i].getVertexArrays()[0]->getEntity();
                        for (unsigned int j = 0; j < m_reflInstances[i].getVertexArrays().size(); j++) {
                            if (entity == m_reflInstances[i].getVertexArrays()[j]->getEntity() && entity->getRootEntity() == reflectEntity) {
                                for (unsigned int k = 0; k < m_reflInstances[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                    vertexCount++;
                                    vbBindlessTex[p].append((*m_reflInstances[i].getVertexArrays()[j])[k]);
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
            currentStates.shader = &sReflectRefract;
            currentStates.texture = &environmentMap.getTexture();
            glCheck(glDepthFunc(GL_GREATER));
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
                    reflectRefractTex.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), currentStates, vboIndirect);
                    vbBindlessTex[p].clear();
                }
            }
            glCheck(glDepthFunc(GL_ALWAYS));
        }
        void ReflectRefractRenderComponent::drawNextFrame() {
            if (reflectRefractTex.getSettings().versionMajor >= 4 && reflectRefractTex.getSettings().versionMinor >= 3) {
                {
                    std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                    if(datasReady) {
                        datasReady = false;
                        m_instances = batcher.getInstances();
                        m_normals = normalBatcher.getInstances();
                        m_reflInstances = reflBatcher.getInstances();
                        m_reflNormals = reflNormalBatcher.getInstances();
                        m_skyboxInstance = skyboxBatcher.getInstances();
                    }
                }

                /*glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicBuffer));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, linkedListBuffer));*/
                RenderStates currentStates;
                math::Matrix4f viewMatrix = view.getViewMatrix().getMatrix().transpose();
                math::Matrix4f projMatrix = view.getProjMatrix().getMatrix().transpose();
                sBuildDepthBuffer.setParameter("viewMatrix", viewMatrix);
                sBuildDepthBuffer.setParameter("projectionMatrix", projMatrix);
                sBuildDepthBuffer.setParameter("nbLayers",GameObject::getNbLayers());

                drawDepthReflInst();


                sBuildAlphaBuffer.setParameter("viewMatrix", viewMatrix);
                sBuildAlphaBuffer.setParameter("projectionMatrix", projMatrix);
                sBuildAlphaBuffer.setParameter("nbLayers",GameObject::getNbLayers());


                drawAlphaInst();

                View reflectView;
                if (view.isOrtho()) {
                    reflectView = View (squareSize, squareSize, view.getViewport().getPosition().z(), view.getViewport().getSize().z());
                } else {
                    reflectView = View (squareSize, squareSize, 80, view.getViewport().getPosition().z(), view.getViewport().getSize().z());
                }
                rootEntities.clear();
                for (unsigned int t = 0; t < 2; t++) {
                    std::vector<Instance> instances = (t == 0) ? m_reflInstances : m_reflNormals;
                    for (unsigned int n = 0; n < instances.size(); n++) {
                        if (instances[n].getAllVertices().getVertexCount() > 0) {
                            std::vector<Entity*> entities = instances[n].getEntities();
                            for (unsigned int e = 0; e < entities.size(); e++) {
                                Entity* entity = entities[e]->getRootEntity();
                                bool contains = false;
                                for (unsigned int r = 0; r < rootEntities.size() && !contains; r++) {
                                    if (rootEntities[r] == entity)
                                        contains = true;
                                }
                                if (!contains) {
                                    rootEntities.push_back(entity);
                                    /*math::Vec3f scale(1, 1, 1);
                                    if (entity->getSize().x() > squareSize) {
                                        scale.x = entity->getSize().x() / squareSize;
                                    }
                                    if (entity->getSize().y() > squareSize) {
                                        scale.y = entity->getSize().y() / squareSize;
                                    }*/
                                    ////////std::cout<<"scale : "<<scale<<"position : "<<entity->getPosition()<<std::endl;
                                    //reflectView.setScale(scale.x, scale.y, scale.z);
                                    if (entity->getFaces().size() > 0 && !entity->getFaces()[0].getMaterial().getType() == Material::WATER)
                                        reflectView.setCenter(entity->getPosition()+entity->getSize()*0.5f);
                                    else
                                        reflectView.setCenter(view.getPosition());
                                    std::vector<math::Matrix4f> projMatrices;
                                    std::vector<math::Matrix4f> viewMatrices;
                                    std::vector<math::Matrix4f> sbProjMatrices;
                                    std::vector<math::Matrix4f> sbViewMatrices;
                                    projMatrices.resize(6);
                                    viewMatrices.resize(6);
                                    sbProjMatrices.resize(6);
                                    sbViewMatrices.resize(6);
                                    environmentMap.setView(reflectView);
                                    for (unsigned int m = 0; m < 6; m++) {
                                        math::Vec3f target = reflectView.getPosition() + dirs[m];
                                        reflectView.lookAt(target.x(), target.y(), target.z(), ups[m]);
                                        projMatrix = reflectView.getProjMatrix().getMatrix().transpose();
                                        viewMatrix = reflectView.getViewMatrix().getMatrix().transpose();
                                        projMatrices[m] = projMatrix;
                                        viewMatrices[m] = viewMatrix;
                                        float zNear = reflectView.getViewport().getPosition().z();
                                        if (!reflectView.isOrtho())
                                            reflectView.setPerspective(80, view.getViewport().getSize().x() / view.getViewport().getSize().y(), 0.1f, view.getViewport().getSize().z());
                                        viewMatrix = reflectView.getViewMatrix().getMatrix().transpose();
                                        projMatrix = reflectView.getProjMatrix().getMatrix().transpose();
                                        math::Matrix4f sbViewMatrix = math::Matrix4f(math::Matrix3f(viewMatrix));
                                        sbViewMatrices[m] = sbViewMatrix;
                                        sbProjMatrices[m] = projMatrix;
                                        if (!reflectView.isOrtho())
                                            reflectView.setPerspective(80, view.getViewport().getSize().x() / view.getViewport().getSize().y(), zNear, view.getViewport().getSize().z());

                                    }
                                    environmentMap.clear(Color::Transparent);
                                    GLuint zero = 0;
                                    std::vector<GLuint> clearAtomicBuffer;
                                    for (unsigned int i = 0; i < 6; i++) {
                                        clearAtomicBuffer.push_back(zero);
                                    }
                                    glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicBuffer));
                                    glCheck(glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint)*6, &clearAtomicBuffer[0]));
                                    glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0));
                                    glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf));
                                    glCheck(glEnable(GL_TEXTURE_3D));
                                    glCheck(glBindTexture(GL_TEXTURE_3D, headPtrTex));
                                    glCheck(glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, squareSize, squareSize,6, GL_RED_INTEGER,
                                    GL_UNSIGNED_INT, NULL));
                                    glCheck(glBindTexture(GL_TEXTURE_3D, 0));
                                    glCheck(glDisable(GL_TEXTURE_3D));
                                    glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
                                    skyboxShader.setParameter("projMatrices", sbProjMatrices);
                                    skyboxShader.setParameter("viewMatrices", sbViewMatrices);
                                    vb.clear();
                                    //vb.name = "SKYBOXVB";
                                    for (unsigned int i = 0; i < m_skyboxInstance.size(); i++) {
                                        if (m_skyboxInstance[i].getAllVertices().getVertexCount() > 0) {
                                            vb.setPrimitiveType(m_skyboxInstance[i].getAllVertices().getPrimitiveType());
                                            for (unsigned int j = 0; j < m_skyboxInstance[i].getAllVertices().getVertexCount(); j++) {
                                                ////////std::cout<<"append"<<std::endl;
                                                vb.append(m_skyboxInstance[i].getAllVertices()[j]);
                                            }
                                        }
                                    }
                                    currentStates.blendMode = BlendAlpha;
                                    currentStates.shader = &skyboxShader;
                                    currentStates.texture = (skybox == nullptr ) ? nullptr : &static_cast<g3d::Skybox*>(skybox)->getTexture();
                                    vb.update();
                                    environmentMap.drawVertexBuffer(vb, currentStates);
                                    sLinkedList.setParameter("viewMatrices", viewMatrices);
                                    sLinkedList.setParameter("projMatrices", projMatrices);
                                    drawEnvReflInst();
                                    glCheck(glFinish());
                                    glCheck(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));
                                    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
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
                                    sLinkedList2.setParameter("worldMat", matrix);
                                    currentStates.shader = &sLinkedList2;
                                    currentStates.texture = nullptr;
                                    environmentMap.drawVertexBuffer(vb, currentStates);
                                    glCheck(glFinish());
                                    glCheck(glMemoryBarrier(GL_ALL_BARRIER_BITS));

                                    viewMatrix = view.getViewMatrix().getMatrix().transpose();
                                    projMatrix = view.getProjMatrix().getMatrix().transpose();
                                    sReflectRefract.setParameter("viewMatrix", viewMatrix);
                                    sReflectRefract.setParameter("projectionMatrix", projMatrix);
                                    sReflectRefract.setParameter("cameraPos", view.getPosition().x(), view.getPosition().y(), view.getPosition().z());
                                    drawReflInst(entity);
                                }
                            }
                        }

                    }
                }


                reflectRefractTex.display();
                /*glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0));
                glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, 0));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0));*/
            }
        }
        std::vector<Entity*> ReflectRefractRenderComponent::getEntities() {
            return visibleEntities;
        }
        void ReflectRefractRenderComponent::loadSkybox(Entity* skybox) {
            this->skybox = skybox;
        }
        bool ReflectRefractRenderComponent::loadEntitiesOnComponent(std::vector<Entity*> vEntities) {
            {
                std::lock_guard<std::recursive_mutex> lock (rec_mutex);
                datasReady = false;
                batcher.clear();
                normalBatcher.clear();
                reflBatcher.clear();
                reflNormalBatcher.clear();
                skyboxBatcher.clear();
            }

            if (skybox != nullptr) {
                for (unsigned int i = 0; i < skybox->getFaces().size(); i++) {
                    skyboxBatcher.addFace(skybox->getFace(i));
                }
            }
            for (unsigned int i = 0; i < vEntities.size(); i++) {
                if ( vEntities[i] != nullptr && vEntities[i]->isLeaf()) {
                    for (unsigned int j = 0; j <  vEntities[i]->getNbFaces(); j++) {
                        std::lock_guard<std::recursive_mutex> lock (rec_mutex);
                        if (vEntities[i]->getFace(j)->getMaterial().isReflectable() || vEntities[i]->getFace(j)->getMaterial().isRefractable()) {

                            if (vEntities[i]->getDrawMode() == Entity::INSTANCED) {
                                ////////std::cout<<"add refl face"<<std::endl;
                                reflBatcher.addFace( vEntities[i]->getFace(j));
                            } else {
                                reflNormalBatcher.addFace(vEntities[i]->getFace(j));
                            }
                        } else {
                            if (vEntities[i]->getDrawMode() == Entity::INSTANCED) {
                                batcher.addFace( vEntities[i]->getFace(j));
                            } else {
                                normalBatcher.addFace(vEntities[i]->getFace(j));
                            }
                        }
                    }
                }
            }
            visibleEntities = vEntities;
            update = true;
            std::lock_guard<std::recursive_mutex> lock (rec_mutex);
            datasReady = true;
            return true;
        }
        bool ReflectRefractRenderComponent::needToUpdate() {
            return update;
        }
        void ReflectRefractRenderComponent::clear() {
            depthBuffer.clear(Color::Transparent);
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf2));
            glCheck(glBindTexture(GL_TEXTURE_2D, depthTex));
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
            reflectRefractTex.clear(Color::Transparent);

        }
        void ReflectRefractRenderComponent::setBackgroundColor (Color color) {
            this->backgroundColor = color;
        }
        void ReflectRefractRenderComponent::setExpression (std::string expression) {
            this->expression = expression;
        }
        void ReflectRefractRenderComponent::draw (Drawable& drawable, RenderStates states) {

        }
        void ReflectRefractRenderComponent::draw (RenderTarget& target, RenderStates states) {
            reflectRefractTexSprite.setCenter(target.getView().getPosition());
            target.draw(reflectRefractTexSprite, states);
        }
        std::string ReflectRefractRenderComponent::getExpression() {
            return expression;
        }
        int ReflectRefractRenderComponent::getLayer() {
            return getPosition().z();
        }
        void ReflectRefractRenderComponent::pushEvent(window::IEvent event, RenderWindow& rw) {
            if (event.type == window::IEvent::WINDOW_EVENT && event.window.type == window::IEvent::WINDOW_EVENT_RESIZED && &getWindow() == &rw && isAutoResized()) {
                //////std::cout<<"recompute size"<<std::endl;
                recomputeSize();
                getListener().pushEvent(event);
                getView().reset(physic::BoundingBox(getView().getViewport().getPosition().x(), getView().getViewport().getPosition().y(), getView().getViewport().getPosition().z(), event.window.data1, event.window.data2, getView().getViewport().getDepth()));
            }
        }
        void ReflectRefractRenderComponent::setView (View view) {
            depthBuffer.setView(view);
            alphaBuffer.setView(view);
            reflectRefractTex.setView(view);
            this->view = view;
        }
        View& ReflectRefractRenderComponent::getView() {
            return view;
        }
        RenderTexture* ReflectRefractRenderComponent::getFrameBuffer() {
            return &reflectRefractTex;
        }
        ReflectRefractRenderComponent::~ReflectRefractRenderComponent() {
            glDeleteBuffers(1, &modelDataBuffer);
            glDeleteBuffers(1, &materialDataBuffer);
            glDeleteBuffers(1, &atomicBuffer);
            glDeleteBuffers(1, &linkedListBuffer);
            glDeleteBuffers(1, &clearBuf);
            glDeleteTextures(1, &headPtrTex);
            glDeleteTextures(1, &depthTex);
            glDeleteTextures(1, &alphaTex);
            glDeleteBuffers(1, &clearBuf2);
            glDeleteBuffers(1, &clearBuf3);
            glDeleteBuffers(1, &ubo);
        }
        #endif // VULKAN
    }
}
