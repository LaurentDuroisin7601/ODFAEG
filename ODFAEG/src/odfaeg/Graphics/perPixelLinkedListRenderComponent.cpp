#include "../../../include/odfaeg/Graphics/perPixelLinkedListRenderComponent.hpp"

//#include "../../../include/odfaeg/Graphics/application.h"
#ifndef VULKAN
#include "glCheck.h"
#endif
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
namespace odfaeg {
    namespace graphic {
        #ifdef VULKAN
        PerPixelLinkedListRenderComponent::PerPixelLinkedListRenderComponent(RenderWindow& window, int layer, std::string expression, window::ContextSettings settings, bool useThread) :
            HeavyComponent(window, math::Vec3f(window.getView().getPosition().x(), window.getView().getPosition().y(), layer),
                          math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0),
                          math::Vec3f(window.getView().getSize().x() + window.getView().getSize().x() * 0.5f, window.getView().getPosition().y() + window.getView().getSize().y() * 0.5f, layer)),
            window(window),
            view(window.getView()),
            useThread(useThread),
            expression(expression),
            quad(math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), window.getSize().y() * 0.5f)),
            layer(layer),
            frameBuffer(window.getDevice()),
            vkDevice(window.getDevice()),
            indirectRenderingShader(window.getDevice()),
            perPixelLinkedListP2(window.getDevice()),
            skyboxShader(window.getDevice()),
            vb(window.getDevice()),
            skyboxVB(window.getDevice()),
            vbBindlessTex {VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice())},
            vbBindlessTexIndexed {VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice())},
            vboIndirect(nullptr) {
            needToUpdateDS = false;
            maxVboIndirectSize = maxModelDataSize = maxMaterialDataSize = 0;
            vboIndirectStagingBuffer = vboIndexedIndirectStagingBuffer = modelDataStagingBuffer = materialDataStagingBuffer = nullptr;
            quad.move(math::Vec3f(-window.getView().getSize().x() * 0.5f, -window.getView().getSize().y() * 0.5f, 0));
            maxNodes = 20 * window.getView().getSize().x() * window.getView().getSize().y();
            unsigned int nodeSize = 5 * sizeof(float) + sizeof(unsigned int);
            resolution = math::Vec4f((int) window.getSize().x(), (int) window.getSize().y(), window.getView().getSize().z(), 1);
            frameBuffer.create(window.getView().getSize().x(), window.getView().getSize().y());
            frameBufferSprite = Sprite(frameBuffer.getTexture(frameBuffer.getImageIndex()), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0), IntRect(0, 0, window.getView().getSize().x(), window.getView().getSize().y()));
            frameBuffer.setView(view);
            linkedListShaderStorageBuffers.resize(frameBuffer.getMaxFramesInFlight());
            linkedListShaderStorageBuffersMemory.resize(frameBuffer.getMaxFramesInFlight());
            VkDeviceSize bufferSize = maxNodes * nodeSize;
            for (unsigned int i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {
                createBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, linkedListShaderStorageBuffers[i], linkedListShaderStorageBuffersMemory[i]);
            }
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = static_cast<uint32_t>(window.getView().getSize().x());
            imageInfo.extent.height = static_cast<uint32_t>(window.getView().getSize().y());
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
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
            createHeadPtrImageView();
            createHeadPtrSampler();



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
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;

            compileShaders();
            createCommandPool();
            allocateCommandBuffers();
            //transitionImageLayout(headPtrTextureImage, VK_FORMAT_R32_UINT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

            AtomicCounterSSBO counter;
            counter.count = 0;
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
            counterShaderStorageBuffers.resize(frameBuffer.getMaxFramesInFlight());
            counterShaderStorageBuffersMemory.resize(frameBuffer.getMaxFramesInFlight());
            for (size_t i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {
                createBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, counterShaderStorageBuffers[i], counterShaderStorageBuffersMemory[i]);
                copyBuffer(stagingBuffer, counterShaderStorageBuffers[i], bufferSize);
            }
            vkDestroyBuffer(vkDevice.getDevice(), stagingBuffer, nullptr);
            vkFreeMemory(vkDevice.getDevice(), stagingBufferMemory, nullptr);

            core::FastDelegate<bool> signal (&PerPixelLinkedListRenderComponent::needToUpdate, this);
            core::FastDelegate<void> slot (&PerPixelLinkedListRenderComponent::drawNextFrame, this);
            core::Command cmd(signal, slot);
            getListener().connect("UPDATE", cmd);
            skybox = nullptr;

            //createDescriptorsAndPipelines();

            vkCmdPushDescriptorSetKHR = (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(vkDevice.getDevice(), "vkCmdPushDescriptorSetKHR");
            if (!vkCmdPushDescriptorSetKHR) {
                throw core::Erreur(0, "Could not get a valid function pointer for vkCmdPushDescriptorSetKHR", 1);
            }
            for (unsigned int i = 0; i < frameBuffer.getCommandBuffers().size(); i++) {
                VkEventCreateInfo eventInfo = {};
                eventInfo.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
                eventInfo.pNext = NULL;
                eventInfo.flags = 0;
                VkEvent event;
                vkCreateEvent(vkDevice.getDevice(), &eventInfo, NULL, &event);
                events.push_back(event);
            }

            unsigned int currentFrame =  frameBuffer.getCurrentFrame();
            frameBuffer.beginRecordCommandBuffers();
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                VkImageMemoryBarrier barrier = {};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
                barrier.image = headPtrTextureImage[i];
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.layerCount = 1;
                vkCmdPipelineBarrier(frameBuffer.getCommandBuffers()[currentFrame], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);


            }
            for (unsigned int i = 0; i < frameBuffer.getSwapchainImagesSize(); i++) {
                const_cast<Texture&>(frameBuffer.getTexture(i)).toShaderReadOnlyOptimal(frameBuffer.getCommandBuffers()[frameBuffer.getCurrentFrame()]);
            }
            VkSemaphoreCreateInfo semaphoreInfo{};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            VkSemaphoreTypeCreateInfo timelineCreateInfo{};
            timelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
            timelineCreateInfo.pNext = nullptr;
            timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
            semaphoreInfo.pNext = &timelineCreateInfo;            ;
            for (unsigned int i = 0; i < values.size(); i++) {
                values[i] = 0;
            }
            offscreenFinishedSemaphore.resize(frameBuffer.getMaxFramesInFlight());
            for (size_t i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {
                timelineCreateInfo.initialValue = values[i];
                if (vkCreateSemaphore(vkDevice.getDevice(), &semaphoreInfo, nullptr, &offscreenFinishedSemaphore[i]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create semaphore");
                }
                ////////std::cout<<"create semaphore : "<<i<<","<<renderFinishedSemaphore[i]<<std::endl;
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
            if (vkAllocateCommandBuffers(vkDevice.getDevice(), &bufferAllocInfo, copyVbPpllPass2CommandBuffer.data()) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to allocate command buffers!", 1);
            }
            if (vkAllocateCommandBuffers(vkDevice.getDevice(), &bufferAllocInfo, ppllCommandBuffer.data()) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to allocate command buffers!", 1);
            }
            if (vkAllocateCommandBuffers(vkDevice.getDevice(), &bufferAllocInfo, ppllSelectedCommandBuffer.data()) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to allocate command buffers!", 1);
            }
            if (vkAllocateCommandBuffers(vkDevice.getDevice(), &bufferAllocInfo, ppllOutlineCommandBuffer.data()) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to allocate command buffers!", 1);
            }
            if (vkAllocateCommandBuffers(vkDevice.getDevice(), &bufferAllocInfo, ppllPass2CommandBuffer.data()) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to allocate command buffers!", 1);
            }
            if (vkAllocateCommandBuffers(vkDevice.getDevice(), &bufferAllocInfo, skyboxCommandBuffer.data()) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to allocate command buffers!", 1);
            }
            if (vkAllocateCommandBuffers(vkDevice.getDevice(), &bufferAllocInfo, copySkyboxCommandBuffer.data()) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to allocate command buffers!", 1);
            }
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(vkDevice.getPhysicalDevice(), &deviceProperties);
            alignment = deviceProperties.limits.minStorageBufferOffsetAlignment;
            skyboxVB.append(Vertex(math::Vec3f(0, 0, 0)));
            skyboxVB.update();

            update = true;
            datasReady = false;

        }
        void PerPixelLinkedListRenderComponent::createDescriptorsAndPipelines() {
            RenderStates states;
            if (useThread) {
                states.shader = &skyboxShader;
                createDescriptorSetLayout(states);
                states.shader = &perPixelLinkedListP2;
                createDescriptorSetLayout2(states);
                states.shader = &skyboxShader;
                for (unsigned int p = 0; p < (Batcher::nbPrimitiveTypes - 1); p++) {
                    createDescriptorPool(p, states);
                    allocateDescriptorSets(p, states);
                }
                states.shader = &indirectRenderingShader;
                createDescriptorSetLayout(states);
                for (unsigned int p = 0; p < (Batcher::nbPrimitiveTypes - 1); p++) {
                    createDescriptorPool(p, states);
                    allocateDescriptorSets(p, states);
                }
                states.shader = &perPixelLinkedListP2;
                for (unsigned int p = 0; p < (Batcher::nbPrimitiveTypes - 1); p++) {
                    createDescriptorPool(p, states);
                    allocateDescriptorSets(p, states);
                    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
                        updateDescriptorSets(i, p, states);
                }

            } else {
                createDescriptorPool(states);
                createDescriptorSetLayout(states);
                allocateDescriptorSets(states);
                states.shader = &perPixelLinkedListP2;
                createDescriptorPool2(states);
                createDescriptorSetLayout2(states);
                allocateDescriptorSets2(states);
                createDescriptorSets2(states);
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
            std::vector<std::vector<std::vector<VkPipelineLayoutCreateInfo>>>& pipelineLayoutInfo = frameBuffer.getPipelineLayoutCreateInfo();
            std::vector<std::vector<std::vector<VkPipelineDepthStencilStateCreateInfo>>>& depthStencilCreateInfo = frameBuffer.getDepthStencilCreateInfo();
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
                    if (PPLLNBDEPTHSTENCIL*none.nbBlendModes > pipelineLayoutInfo[i][j].size()) {
                        pipelineLayoutInfo[i][j].resize(PPLLNBDEPTHSTENCIL*none.nbBlendModes);
                        depthStencilCreateInfo[i][j].resize(PPLLNBDEPTHSTENCIL*none.nbBlendModes);
                    }
                }
            }
            frameBuffer.enableDepthTest(true);
            states.shader = &skyboxShader;
            for (unsigned int b = 0; b < blendModes.size(); b++) {
                states.blendMode = blendModes[b];
                states.blendMode.updateIds();
                for (unsigned int j = 0; j < PPLLNBDEPTHSTENCIL; j++) {
                    for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes - 1; i++) {
                        if (j == 0) {

                           frameBuffer.enableStencilTest(false);

                           VkPushConstantRange push_constant;
                           push_constant.offset = 0;
                           //this push constant range takes up the size of a MeshPushConstants struct
                           push_constant.size = sizeof(SkyboxPushConsts);
                           //this push constant range is accessible only in the vertex shader
                           push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                           pipelineLayoutInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = &push_constant;
                           pipelineLayoutInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 1;
                           depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                           depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthWriteEnable = VK_FALSE;
                           depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.failOp = VK_STENCIL_OP_KEEP;
                           depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.depthFailOp = VK_STENCIL_OP_KEEP;
                           depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.passOp = VK_STENCIL_OP_REPLACE;
                           depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.compareMask = 0xff;
                           depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.writeMask = 0xff;
                           depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.reference = 0;
                           depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back;
                           frameBuffer.createGraphicPipeline(static_cast<PrimitiveType>(i), states, PPLLNODEPTHNOSTENCIL, PPLLNBDEPTHSTENCIL);
                       } else if (j == 1) {
                           frameBuffer.enableStencilTest(true);

                           depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                           depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthWriteEnable = VK_TRUE;
                           depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.compareOp = VK_COMPARE_OP_ALWAYS;
                           depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.failOp = VK_STENCIL_OP_REPLACE;
                           depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.depthFailOp = VK_STENCIL_OP_REPLACE;
                           depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.passOp = VK_STENCIL_OP_REPLACE;
                           depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.compareMask = 0xff;
                           depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.writeMask = 0xff;
                           depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.reference = 1;
                           depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back;
                           VkPushConstantRange push_constant;
                           push_constant.offset = 0;
                           //this push constant range takes up the size of a MeshPushConstants struct
                           push_constant.size = sizeof(SkyboxPushConsts);
                           //this push constant range is accessible only in the vertex shader
                           push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                           pipelineLayoutInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = &push_constant;
                           pipelineLayoutInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 1;
                           frameBuffer.createGraphicPipeline(static_cast<PrimitiveType>(i), states, PPLLNODEPTHSTENCIL, PPLLNBDEPTHSTENCIL);
                        } else {
                           frameBuffer.enableStencilTest(true);
                           depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_LESS;;
                           depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].depthWriteEnable = VK_FALSE;
                           depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].back.compareOp = VK_COMPARE_OP_NOT_EQUAL;
                           depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].back.failOp = VK_STENCIL_OP_KEEP;
                           depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].back.depthFailOp = VK_STENCIL_OP_KEEP;
                           depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].back.passOp = VK_STENCIL_OP_REPLACE;
                           depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].back.compareMask = 0xff;
                           depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].back.writeMask = 0xff;
                           depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].back.reference = 1;
                           depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].front = depthStencilCreateInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].back;
                           VkPushConstantRange push_constant;
                           push_constant.offset = 0;
                           //this push constant range takes up the size of a MeshPushConstants struct
                           push_constant.size = sizeof(SkyboxPushConsts);
                           //this push constant range is accessible only in the vertex shader
                           push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                           pipelineLayoutInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = &push_constant;
                           pipelineLayoutInfo[skyboxShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 1;
                           frameBuffer.createGraphicPipeline(static_cast<PrimitiveType>(i), states, PPLLNODEPTHSTENCILOUTLINE, PPLLNBDEPTHSTENCIL);
                        }
                    }
                }
            }
            states.shader = &indirectRenderingShader;
            for (unsigned int b = 0; b < blendModes.size(); b++) {

                states.blendMode = blendModes[b];
                states.blendMode.updateIds();
                for (unsigned int j = 0; j < PPLLNBDEPTHSTENCIL; j++) {
                    for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes - 1; i++) {
                       if (j == 0) {
                           frameBuffer.enableStencilTest(false);

                           VkPushConstantRange push_constant;
                           //this push constant range starts at the beginning
                           push_constant.offset = 0;
                           //this push constant range takes up the size of a MeshPushConstants struct
                           push_constant.size = sizeof(IndirectDrawPushConsts);
                           //this push constant range is accessible only in the vertex shader
                           push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                           pipelineLayoutInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = &push_constant;
                           pipelineLayoutInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 1;
                           depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                           depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthWriteEnable = VK_TRUE;


                           depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.failOp = VK_STENCIL_OP_KEEP;
                           depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.depthFailOp = VK_STENCIL_OP_KEEP;
                           depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.passOp = VK_STENCIL_OP_REPLACE;
                           depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.compareMask = 0xff;
                           depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.writeMask = 0xff;
                           depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.reference = 0;
                           depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back;
                           frameBuffer.createGraphicPipeline(static_cast<PrimitiveType>(i), states, PPLLNODEPTHNOSTENCIL, PPLLNBDEPTHSTENCIL);


                       } else if (j == 1) {
                           frameBuffer.enableStencilTest(true);
                           depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                           depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthWriteEnable = VK_TRUE;
                           depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.compareOp = VK_COMPARE_OP_ALWAYS;
                           depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.failOp = VK_STENCIL_OP_REPLACE;
                           depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.depthFailOp = VK_STENCIL_OP_REPLACE;
                           depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.passOp = VK_STENCIL_OP_REPLACE;
                           depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.compareMask = 0xff;
                           depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.writeMask = 0xff;
                           depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.reference = 1;
                           depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back;
                           VkPushConstantRange push_constant;
                           //this push constant range starts at the beginning
                           push_constant.offset = 0;
                           //this push constant range takes up the size of a MeshPushConstants struct
                           push_constant.size = sizeof(IndirectDrawPushConsts);
                           //this push constant range is accessible only in the vertex shader
                           push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                           pipelineLayoutInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = &push_constant;
                           pipelineLayoutInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 1;
                           frameBuffer.createGraphicPipeline(static_cast<PrimitiveType>(i), states, PPLLNODEPTHSTENCIL, PPLLNBDEPTHSTENCIL);

                       } else {
                           frameBuffer.enableStencilTest(true);

                           depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                           depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].depthWriteEnable = VK_FALSE;

                           depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].back.compareOp = VK_COMPARE_OP_NOT_EQUAL;
                           depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].back.failOp = VK_STENCIL_OP_KEEP;
                           depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].back.depthFailOp = VK_STENCIL_OP_KEEP;
                           depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].back.passOp = VK_STENCIL_OP_KEEP;
                           depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].back.compareMask = 0xff;
                           depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].back.writeMask = 0x00;
                           depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].back.reference = 1;
                           depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].front = depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].back;
                           VkPushConstantRange push_constant;
                           //this push constant range starts at the beginning
                           push_constant.offset = 0;
                           //this push constant range takes up the size of a MeshPushConstants struct
                           push_constant.size = sizeof(IndirectDrawPushConsts);
                           //this push constant range is accessible only in the vertex shader
                           push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                           ////std::cout<<"pipeline id : "<<PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id<<std::endl;

                           pipelineLayoutInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = &push_constant;
                           pipelineLayoutInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 1;
                           frameBuffer.createGraphicPipeline(static_cast<PrimitiveType>(i), states, PPLLNODEPTHSTENCILOUTLINE, PPLLNBDEPTHSTENCIL);
                       }
                    }
                }
            }

            states.shader = &perPixelLinkedListP2;
            for (unsigned int b = 0; b < blendModes.size(); b++) {
                states.blendMode = blendModes[b];
                states.blendMode.updateIds();
                for (unsigned int j = 0; j < PPLLNBDEPTHSTENCIL; j++) {
                    for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes - 1; i++) {
                        if (j == 0) {

                           frameBuffer.enableStencilTest(false);

                           VkPushConstantRange push_constant;
                           push_constant.offset = 0;
                           //this push constant range takes up the size of a MeshPushConstants struct
                           push_constant.size = sizeof(Ppll2PushConsts);
                           //this push constant range is accessible only in the vertex shader
                           push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                           pipelineLayoutInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = &push_constant;
                           pipelineLayoutInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 1;
                           depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                           depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHNOSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthWriteEnable = VK_TRUE;
                           depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.failOp = VK_STENCIL_OP_KEEP;
                           depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.depthFailOp = VK_STENCIL_OP_KEEP;
                           depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.passOp = VK_STENCIL_OP_REPLACE;
                           depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.compareMask = 0xff;
                           depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.writeMask = 0xff;
                           depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.reference = 0;
                           depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back;
                           frameBuffer.createGraphicPipeline(static_cast<PrimitiveType>(i), states, PPLLNODEPTHNOSTENCIL, PPLLNBDEPTHSTENCIL);
                       } else if (j == 1) {
                           frameBuffer.enableStencilTest(true);

                           depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                           depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].depthWriteEnable = VK_TRUE;
                           depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.compareOp = VK_COMPARE_OP_ALWAYS;
                           depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.failOp = VK_STENCIL_OP_REPLACE;
                           depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.depthFailOp = VK_STENCIL_OP_REPLACE;
                           depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.passOp = VK_STENCIL_OP_REPLACE;
                           depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.compareMask = 0xff;
                           depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.writeMask = 0xff;
                           depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back.reference = 1;
                           depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].front = depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].back;
                           VkPushConstantRange push_constant;
                           push_constant.offset = 0;
                           //this push constant range takes up the size of a MeshPushConstants struct
                           push_constant.size = sizeof(Ppll2PushConsts);
                           //this push constant range is accessible only in the vertex shader
                           push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                           pipelineLayoutInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = &push_constant;
                           pipelineLayoutInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCIL*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 1;
                           frameBuffer.createGraphicPipeline(static_cast<PrimitiveType>(i), states, PPLLNODEPTHSTENCIL, PPLLNBDEPTHSTENCIL);
                        } else {
                           frameBuffer.enableStencilTest(true);
                           depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].depthCompareOp = VK_COMPARE_OP_LESS;;
                           depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].depthWriteEnable = VK_FALSE;
                           depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].back.compareOp = VK_COMPARE_OP_NOT_EQUAL;
                           depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].back.failOp = VK_STENCIL_OP_KEEP;
                           depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].back.depthFailOp = VK_STENCIL_OP_KEEP;
                           depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].back.passOp = VK_STENCIL_OP_REPLACE;
                           depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].back.compareMask = 0xff;
                           depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].back.writeMask = 0xff;
                           depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].back.reference = 1;
                           depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].front = depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].back;
                           VkPushConstantRange push_constant;
                           push_constant.offset = 0;
                           //this push constant range takes up the size of a MeshPushConstants struct
                           push_constant.size = sizeof(Ppll2PushConsts);
                           //this push constant range is accessible only in the vertex shader
                           push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                           pipelineLayoutInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].pPushConstantRanges = &push_constant;
                           pipelineLayoutInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][0][PPLLNODEPTHSTENCILOUTLINE*states.blendMode.nbBlendModes+states.blendMode.id].pushConstantRangeCount = 1;
                           frameBuffer.createGraphicPipeline(static_cast<PrimitiveType>(i), states, PPLLNODEPTHSTENCILOUTLINE, PPLLNBDEPTHSTENCIL);
                        }
                    }
                }
            }
        }
        void PerPixelLinkedListRenderComponent::launchRenderer () {
            if (useThread) {
                stop = false;
                getListener().launch();
            }
        }
        void PerPixelLinkedListRenderComponent::stopRenderer() {
            stop = true;
            cv.notify_all();
            cv2.notify_all();
            getListener().stop();
        }
        void PerPixelLinkedListRenderComponent::loadSkybox(Entity* skybox) {
            this->skybox = skybox;
            RenderStates states;
            states.shader = &skyboxShader;
            for (unsigned int p = 0; p < (Batcher::nbPrimitiveTypes - 1); p++) {
                for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
                    updateDescriptorSets(i, p, states);
            }
        }
        uint32_t PerPixelLinkedListRenderComponent::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
            VkPhysicalDeviceMemoryProperties memProperties;
            vkGetPhysicalDeviceMemoryProperties(vkDevice.getPhysicalDevice(), &memProperties);
            for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
                if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                    return i;
                }
            }
            throw std::runtime_error("aucun type de memoire ne satisfait le buffer!");
        }
        void PerPixelLinkedListRenderComponent::createHeadPtrImageView() {
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                VkImageViewCreateInfo viewInfo{};
                viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                viewInfo.image = headPtrTextureImage[i];
                viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                viewInfo.format = VK_FORMAT_R32_UINT;
                viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                viewInfo.subresourceRange.baseMipLevel = 0;
                viewInfo.subresourceRange.levelCount = 1;
                viewInfo.subresourceRange.baseArrayLayer = 0;
                viewInfo.subresourceRange.layerCount = 1;

                if (vkCreateImageView(vkDevice.getDevice(), &viewInfo, nullptr, &headPtrTextureImageView[i]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create head ptr texture image view!");
                }
            }
        }
        void PerPixelLinkedListRenderComponent::createHeadPtrSampler() {
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
        }
        void PerPixelLinkedListRenderComponent::clear() {
            ////std::cout<<"clear"<<std::endl;
            frameBuffer.beginRecordCommandBuffers();
            const_cast<Texture&>(frameBuffer.getTexture(frameBuffer.getImageIndex())).toColorAttachmentOptimal(frameBuffer.getCommandBuffers()[frameBuffer.getCurrentFrame()]);
            frameBuffer.clear(Color::Transparent);
            unsigned int currentFrame = frameBuffer.getCurrentFrame();
            VkClearColorValue clearColor;
            clearColor.uint32[0] = 0xffffffff;
            VkImageSubresourceRange subresRange = {};
            subresRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subresRange.levelCount = 1;
            subresRange.layerCount = 1;
            vkCmdClearColorImage(frameBuffer.getCommandBuffers()[currentFrame], headPtrTextureImage[currentFrame], VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &subresRange);
            vkCmdFillBuffer(frameBuffer.getCommandBuffers()[currentFrame], counterShaderStorageBuffers[currentFrame], 0, sizeof(uint32_t), 0);
            VkMemoryBarrier memoryBarrier;
            memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
            memoryBarrier.pNext = VK_NULL_HANDLE;
            memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            vkCmdPipelineBarrier(frameBuffer.getCommandBuffers()[currentFrame], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);


            ////std::cout<<"cleared"<<std::endl;
        }
        void PerPixelLinkedListRenderComponent::resetBuffers() {
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
        VkCommandBuffer PerPixelLinkedListRenderComponent::beginSingleTimeCommands() {
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

        void PerPixelLinkedListRenderComponent::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
            vkEndCommandBuffer(commandBuffer);

            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;

            vkQueueSubmit(vkDevice.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(vkDevice.getGraphicsQueue());

            vkFreeCommandBuffers(vkDevice.getDevice(), commandPool, 1, &commandBuffer);
        }
        void PerPixelLinkedListRenderComponent::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
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
            } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
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
        void PerPixelLinkedListRenderComponent::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
            VkCommandBuffer commandBuffer = beginSingleTimeCommands();

            VkBufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = {0, 0, 0};
            region.imageExtent = {
                width,
                height,
                1
            };

            vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

            endSingleTimeCommands(commandBuffer);
        }
        unsigned int PerPixelLinkedListRenderComponent::align(unsigned int offset) {
            ////std::cout << "alignment = " << alignment << std::endl;
            return (offset + alignment - 1) & ~(alignment - 1);
        }
        void PerPixelLinkedListRenderComponent::createDescriptorPool(unsigned int p, RenderStates states) {
            std::vector<VkDescriptorPool>& descriptorPool = frameBuffer.getDescriptorPool();
            Shader* shader = const_cast<Shader*>(states.shader);
            if (shader == &indirectRenderingShader) {
                unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorPool.size())
                    descriptorPool.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                std::vector<Texture*> allTextures = Texture::getAllTextures();
                std::array<VkDescriptorPoolSize, 6> poolSizes;
                poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                poolSizes[0].descriptorCount = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight());
                poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                poolSizes[1].descriptorCount = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight());
                poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                poolSizes[2].descriptorCount = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight());
                poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                poolSizes[3].descriptorCount = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight());
                poolSizes[4].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                poolSizes[4].descriptorCount = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight());
                poolSizes[5].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                poolSizes[5].descriptorCount = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight() * MAX_TEXTURES);

                if (descriptorPool[descriptorId] != nullptr) {
                    vkDestroyDescriptorPool(vkDevice.getDevice(), descriptorPool[descriptorId], nullptr);
                }
                VkDescriptorPoolCreateInfo poolInfo{};
                poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
                poolInfo.pPoolSizes = poolSizes.data();
                poolInfo.maxSets = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight());
                if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool[descriptorId]) != VK_SUCCESS) {
                    throw std::runtime_error("echec de la creation de la pool de descripteurs!");
                }
            } else if (shader == &perPixelLinkedListP2) {
                if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorPool.size())
                    descriptorPool.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                unsigned int descriptorId =  p * shader->getNbShaders() + shader->getId();
                std::array<VkDescriptorPoolSize, 2> poolSizes{};
                poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                poolSizes[0].descriptorCount = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight());
                poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                poolSizes[1].descriptorCount = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight());

                if (descriptorPool[descriptorId] != nullptr) {
                    vkDestroyDescriptorPool(vkDevice.getDevice(), descriptorPool[descriptorId], nullptr);
                }


                VkDescriptorPoolCreateInfo poolInfo{};
                poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
                poolInfo.pPoolSizes = poolSizes.data();
                poolInfo.maxSets = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight());
                if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool[descriptorId]) != VK_SUCCESS) {
                    throw std::runtime_error("echec de la creation de la pool de descripteurs!");
                }
            } else {
                unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorPool.size())
                    descriptorPool.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                std::array<VkDescriptorPoolSize, 1> poolSizes;
                poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                poolSizes[0].descriptorCount = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight());
                if (descriptorPool[descriptorId] != nullptr) {
                    vkDestroyDescriptorPool(vkDevice.getDevice(), descriptorPool[descriptorId], nullptr);
                }
                VkDescriptorPoolCreateInfo poolInfo{};
                poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
                poolInfo.pPoolSizes = poolSizes.data();
                poolInfo.maxSets = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight());
                if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool[descriptorId]) != VK_SUCCESS) {
                    throw std::runtime_error("echec de la creation de la pool de descripteurs!");
                }
            }
        }
        void PerPixelLinkedListRenderComponent::createDescriptorPool(RenderStates states) {
            std::vector<VkDescriptorPool>& descriptorPool = frameBuffer.getDescriptorPool();
            Shader* shader = const_cast<Shader*>(states.shader);
            if (shader->getNbShaders() > descriptorPool.size())
                descriptorPool.resize(shader->getNbShaders());
            unsigned int descriptorId = shader->getId();
            ////////std::cout<<"ppll descriptor id : "<<frameBuffer.getId()<<","<<shader->getId()<<","<<frameBuffer.getId() * shader->getNbShaders() + shader->getId()<<std::endl;
            std::vector<Texture*> allTextures = Texture::getAllTextures();
            std::array<VkDescriptorPoolSize, 6> poolSizes;
            poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            poolSizes[0].descriptorCount = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight());
            poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            poolSizes[1].descriptorCount = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight());
            poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            poolSizes[2].descriptorCount = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight());
            poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            poolSizes[3].descriptorCount = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight());;
            poolSizes[4].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            poolSizes[4].descriptorCount = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight());
            poolSizes[5].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSizes[5].descriptorCount = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight() * allTextures.size());

            if (descriptorPool[descriptorId] != nullptr) {
                vkDestroyDescriptorPool(vkDevice.getDevice(), descriptorPool[descriptorId], nullptr);
            }
            VkDescriptorPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            poolInfo.pPoolSizes = poolSizes.data();
            poolInfo.maxSets = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight());
            if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool[descriptorId]) != VK_SUCCESS) {
                throw std::runtime_error("echec de la creation de la pool de descripteurs!");
            }
        }
        void PerPixelLinkedListRenderComponent::createDescriptorSetLayout(RenderStates states) {
            std::vector<VkDescriptorSetLayout>& descriptorSetLayout = frameBuffer.getDescriptorSetLayout();
            Shader* shader = const_cast<Shader*>(states.shader);
            if (shader == &indirectRenderingShader) {
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

                std::vector<VkDescriptorBindingFlags> bindingFlags(6, 0); // 6 bindings, flags par défaut à 0
                bindingFlags[5] = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                      VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT; // seulement pour sampler[]

                VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
                bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
                bindingFlagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
                bindingFlagsInfo.pBindingFlags = bindingFlags.data();

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
                samplerLayoutBinding.descriptorCount = MAX_TEXTURES;
                samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                samplerLayoutBinding.pImmutableSamplers = nullptr;
                samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;



                if (descriptorSetLayout[descriptorId] != nullptr) {
                    vkDestroyDescriptorSetLayout(vkDevice.getDevice(), descriptorSetLayout[descriptorId], nullptr);
                }

                std::array<VkDescriptorSetLayoutBinding, 6> bindings = {counterLayoutBinding, headPtrImageLayoutBinding, linkedListLayoutBinding, modelDataLayoutBinding, materialDataLayoutBinding, samplerLayoutBinding};
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
                if (shader->getNbShaders() > descriptorSetLayout.size())
                    descriptorSetLayout.resize(shader->getNbShaders());
                unsigned int descriptorId = shader->getId();
                //std::cout<<"shader id : "<<descriptorId<<std::endl;
                VkDescriptorSetLayoutBinding samplerLayoutBinding{};
                samplerLayoutBinding.binding = 0;
                samplerLayoutBinding.descriptorCount = 1;
                samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                samplerLayoutBinding.pImmutableSamplers = nullptr;
                samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                if (descriptorSetLayout[descriptorId] != nullptr) {
                    vkDestroyDescriptorSetLayout(vkDevice.getDevice(), descriptorSetLayout[descriptorId], nullptr);
                }

                std::array<VkDescriptorSetLayoutBinding, 1> bindings = {samplerLayoutBinding};
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
        void PerPixelLinkedListRenderComponent::allocateDescriptorSets(unsigned int p, RenderStates states) {
            std::vector<std::vector<VkDescriptorSet>>& descriptorSets = frameBuffer.getDescriptorSet();
            std::vector<VkDescriptorPool>& descriptorPool = frameBuffer.getDescriptorPool();
            std::vector<VkDescriptorSetLayout>& descriptorSetLayout = frameBuffer.getDescriptorSetLayout();
            Shader* shader = const_cast<Shader*>(states.shader);
            if (shader == &indirectRenderingShader) {
                if (shader->getNbShaders()  * (Batcher::nbPrimitiveTypes - 1) > descriptorSets.size())
                    descriptorSets.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();

                for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                    descriptorSets[i].resize(frameBuffer.getMaxFramesInFlight());
                }
                std::vector<Texture*> allTextures = Texture::getAllTextures();
                std::vector<uint32_t> variableCounts(frameBuffer.getMaxFramesInFlight(), static_cast<uint32_t>(MAX_TEXTURES));

                VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
                variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
                variableCountInfo.descriptorSetCount = static_cast<uint32_t>(variableCounts.size());;
                variableCountInfo.pDescriptorCounts = variableCounts.data();
                std::vector<VkDescriptorSetLayout> layouts(frameBuffer.getMaxFramesInFlight(), descriptorSetLayout[shader->getId()]);
                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.pNext = &variableCountInfo;
                allocInfo.descriptorPool = descriptorPool[descriptorId];
                allocInfo.descriptorSetCount = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight());
                allocInfo.pSetLayouts = layouts.data();
                if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, descriptorSets[descriptorId].data()) != VK_SUCCESS) {
                    throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
                }
            } else if (shader == &perPixelLinkedListP2) {
                if (shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1) > descriptorSets.size())
                    descriptorSets.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                    descriptorSets[i].resize(frameBuffer.getMaxFramesInFlight());
                }
                std::vector<VkDescriptorSetLayout> layouts(frameBuffer.getMaxFramesInFlight(), descriptorSetLayout[shader->getId()]);
                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

                allocInfo.descriptorPool = descriptorPool[descriptorId];
                allocInfo.descriptorSetCount = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight());
                allocInfo.pSetLayouts = layouts.data();
                if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, descriptorSets[descriptorId].data()) != VK_SUCCESS) {
                    throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
                }
            } else {
                if (shader->getNbShaders()  * (Batcher::nbPrimitiveTypes - 1)> descriptorSets.size())
                    descriptorSets.resize(shader->getNbShaders() * (Batcher::nbPrimitiveTypes - 1));
                unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                //std::cout<<"descriptor skybox ids : "<<descriptorId<<","<<shader->getId()<<std::endl;
                for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                    descriptorSets[i].resize(frameBuffer.getMaxFramesInFlight());
                }
                std::vector<VkDescriptorSetLayout> layouts(frameBuffer.getMaxFramesInFlight(), descriptorSetLayout[shader->getId()]);
                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = descriptorPool[descriptorId];
                allocInfo.descriptorSetCount = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight());
                allocInfo.pSetLayouts = layouts.data();
                if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, descriptorSets[descriptorId].data()) != VK_SUCCESS) {
                    throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
                }
            }
        }
        void PerPixelLinkedListRenderComponent::allocateDescriptorSets(RenderStates states) {
            std::vector<std::vector<VkDescriptorSet>>& descriptorSets = frameBuffer.getDescriptorSet();
            std::vector<VkDescriptorPool>& descriptorPool = frameBuffer.getDescriptorPool();
            std::vector<VkDescriptorSetLayout>& descriptorSetLayout = frameBuffer.getDescriptorSetLayout();
            Shader* shader = const_cast<Shader*>(states.shader);
            if (shader->getNbShaders() > descriptorSets.size())
                descriptorSets.resize(shader->getNbShaders());
            unsigned int descriptorId = shader->getId();
            for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                descriptorSets[i].resize(frameBuffer.getMaxFramesInFlight());
            }
            std::vector<Texture*> allTextures = Texture::getAllTextures();
            std::vector<uint32_t> variableCounts(frameBuffer.getMaxFramesInFlight(), static_cast<uint32_t>(allTextures.size()));

            VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
            variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
            variableCountInfo.descriptorSetCount = static_cast<uint32_t>(variableCounts.size());;
            variableCountInfo.pDescriptorCounts = variableCounts.data();
            std::vector<VkDescriptorSetLayout> layouts(frameBuffer.getMaxFramesInFlight(), descriptorSetLayout[descriptorId]);
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.pNext = &variableCountInfo;
            allocInfo.descriptorPool = descriptorPool[descriptorId];
            allocInfo.descriptorSetCount = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight());
            allocInfo.pSetLayouts = layouts.data();
            if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, descriptorSets[descriptorId].data()) != VK_SUCCESS) {
                throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
            }
        }
        void PerPixelLinkedListRenderComponent::updateDescriptorSets(unsigned int currentFrame, unsigned int p, RenderStates states) {
            std::vector<std::vector<VkDescriptorSet>>& descriptorSets = frameBuffer.getDescriptorSet();
            Shader* shader = const_cast<Shader*>(states.shader);
            std::vector<Texture*> allTextures = Texture::getAllTextures();
            if (shader == &indirectRenderingShader) {

                unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                std::vector<VkDescriptorImageInfo>	descriptorImageInfos;
                descriptorImageInfos.resize(allTextures.size());
                for (unsigned int j = 0; j < allTextures.size(); j++) {
                    descriptorImageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    descriptorImageInfos[j].imageView = allTextures[j]->getImageView();
                    descriptorImageInfos[j].sampler = allTextures[j]->getSampler();
                }
                std::array<VkWriteDescriptorSet, 6> descriptorWrites{};

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
                linkedListStorageBufferInfoLastFrame.range = maxNodes * nodeSize;

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

                descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[5].dstSet = descriptorSets[descriptorId][currentFrame];
                descriptorWrites[5].dstBinding = 5;
                descriptorWrites[5].dstArrayElement = 0;
                descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrites[5].descriptorCount = allTextures.size();
                descriptorWrites[5].pImageInfo = descriptorImageInfos.data();

                vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

            } else if (shader == &perPixelLinkedListP2) {
                unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                for (size_t i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {

                    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

                    VkDescriptorImageInfo headPtrDescriptorImageInfo;
                    headPtrDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                    headPtrDescriptorImageInfo.imageView = headPtrTextureImageView[currentFrame];
                    headPtrDescriptorImageInfo.sampler = headPtrTextureSampler[currentFrame];

                    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[0].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[0].dstBinding = 0;
                    descriptorWrites[0].dstArrayElement = 0;
                    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    descriptorWrites[0].descriptorCount = 1;
                    descriptorWrites[0].pImageInfo = &headPtrDescriptorImageInfo;

                    VkDescriptorBufferInfo linkedListStorageBufferInfoLastFrame{};
                    linkedListStorageBufferInfoLastFrame.buffer = linkedListShaderStorageBuffers[currentFrame];
                    linkedListStorageBufferInfoLastFrame.offset = 0;
                    unsigned int nodeSize = 5 * sizeof(float) + sizeof(unsigned int);
                    linkedListStorageBufferInfoLastFrame.range = maxNodes * nodeSize;

                    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[1].dstSet = descriptorSets[descriptorId][currentFrame];
                    descriptorWrites[1].dstBinding = 1;
                    descriptorWrites[1].dstArrayElement = 0;
                    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descriptorWrites[1].descriptorCount = 1;
                    descriptorWrites[1].pBufferInfo = &linkedListStorageBufferInfoLastFrame;

                    vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
                }
            } else {
                unsigned int descriptorId = p * shader->getNbShaders() + shader->getId();
                for (size_t i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {
                    VkDescriptorImageInfo	descriptorImageInfo;
                    std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
                    descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    descriptorImageInfo.imageView = skybox->getTexture().getImageView();
                    descriptorImageInfo.sampler = skybox->getTexture().getSampler();

                    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[0].dstSet = descriptorSets[descriptorId][i];
                    descriptorWrites[0].dstBinding = 0;
                    descriptorWrites[0].dstArrayElement = 0;
                    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrites[0].descriptorCount = 1;
                    descriptorWrites[0].pImageInfo = &descriptorImageInfo;

                    vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
                }
            }
        }
        void PerPixelLinkedListRenderComponent::createDescriptorSets(unsigned int p, RenderStates states) {
            std::vector<std::vector<VkDescriptorSet>>& descriptorSets = frameBuffer.getDescriptorSet();
            Shader* shader = const_cast<Shader*>(states.shader);
            std::vector<Texture*> allTextures = Texture::getAllTextures();

            unsigned int descriptorId = shader->getId();
            for (size_t i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {
                std::vector<VkDescriptorImageInfo>	descriptorImageInfos;
                descriptorImageInfos.resize(allTextures.size());
                for (unsigned int j = 0; j < allTextures.size(); j++) {
                    descriptorImageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    descriptorImageInfos[j].imageView = allTextures[j]->getImageView();
                    descriptorImageInfos[j].sampler = allTextures[j]->getSampler();
                }
                std::array<VkWriteDescriptorSet, 6> descriptorWrites{};

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
                linkedListStorageBufferInfoLastFrame.range = maxNodes * nodeSize;

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

                descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[5].dstSet = descriptorSets[descriptorId][i];
                descriptorWrites[5].dstBinding = 5;
                descriptorWrites[5].dstArrayElement = 0;
                descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrites[5].descriptorCount = allTextures.size();
                descriptorWrites[5].pImageInfo = descriptorImageInfos.data();

                vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
            }
        }
        void PerPixelLinkedListRenderComponent::createDescriptorPool2(RenderStates states) {
            std::vector<VkDescriptorPool>& descriptorPool = frameBuffer.getDescriptorPool();
            Shader* shader = const_cast<Shader*>(states.shader);
            if (shader->getNbShaders() > descriptorPool.size())
                descriptorPool.resize(shader->getNbShaders());
            unsigned int descriptorId =  shader->getId();
            std::array<VkDescriptorPoolSize, 2> poolSizes{};
            poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            poolSizes[0].descriptorCount = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight());
            poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            poolSizes[1].descriptorCount = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight());

            if (descriptorPool[descriptorId] != nullptr) {
                vkDestroyDescriptorPool(vkDevice.getDevice(), descriptorPool[descriptorId], nullptr);
            }


            VkDescriptorPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            poolInfo.pPoolSizes = poolSizes.data();
            poolInfo.maxSets = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight());
            if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool[descriptorId]) != VK_SUCCESS) {
                throw std::runtime_error("echec de la creation de la pool de descripteurs!");
            }
        }
        void PerPixelLinkedListRenderComponent::createDescriptorSetLayout2(RenderStates states) {
            std::vector<VkDescriptorSetLayout>& descriptorSetLayout = frameBuffer.getDescriptorSetLayout();
            Shader* shader = const_cast<Shader*>(states.shader);
            if (shader->getNbShaders() > descriptorSetLayout.size())
                descriptorSetLayout.resize(shader->getNbShaders());
            unsigned int descriptorId = shader->getId();
            VkDescriptorSetLayoutBinding headPtrImageLayoutBinding;
            headPtrImageLayoutBinding.binding = 0;
            headPtrImageLayoutBinding.descriptorCount = 1;
            headPtrImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            headPtrImageLayoutBinding.pImmutableSamplers = nullptr;
            headPtrImageLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            VkDescriptorSetLayoutBinding linkedListLayoutBinding{};
            linkedListLayoutBinding.binding = 1;
            linkedListLayoutBinding.descriptorCount = 1;
            linkedListLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            linkedListLayoutBinding.pImmutableSamplers = nullptr;
            linkedListLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            if (descriptorSetLayout[descriptorId] != nullptr) {
                vkDestroyDescriptorSetLayout(vkDevice.getDevice(), descriptorSetLayout[descriptorId], nullptr);
            }

            std::array<VkDescriptorSetLayoutBinding, 2> bindings = {headPtrImageLayoutBinding, linkedListLayoutBinding};
            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            //layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
            layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());;
            layoutInfo.pBindings = bindings.data();

            if (vkCreateDescriptorSetLayout(vkDevice.getDevice(), &layoutInfo, nullptr, &descriptorSetLayout[descriptorId]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor set layout!");
            }
        }
        void PerPixelLinkedListRenderComponent::allocateDescriptorSets2(RenderStates states) {
            std::vector<std::vector<VkDescriptorSet>>& descriptorSets = frameBuffer.getDescriptorSet();
            std::vector<VkDescriptorSetLayout>& descriptorSetLayout = frameBuffer.getDescriptorSetLayout();
            std::vector<VkDescriptorPool>& descriptorPool = frameBuffer.getDescriptorPool();
            Shader* shader = const_cast<Shader*>(states.shader);
            if (shader->getNbShaders() > descriptorSets.size())
                descriptorSets.resize(shader->getNbShaders());
            unsigned int descriptorId = shader->getId();
            for (unsigned int i = 0; i < descriptorSets.size(); i++) {
                descriptorSets[i].resize(frameBuffer.getMaxFramesInFlight());
            }
            std::vector<VkDescriptorSetLayout> layouts(frameBuffer.getMaxFramesInFlight(), descriptorSetLayout[descriptorId]);
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

            allocInfo.descriptorPool = descriptorPool[descriptorId];
            allocInfo.descriptorSetCount = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight());
            allocInfo.pSetLayouts = layouts.data();
            if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, descriptorSets[descriptorId].data()) != VK_SUCCESS) {
                throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
            }
        }
        void PerPixelLinkedListRenderComponent::createDescriptorSets2(RenderStates states) {
            std::vector<std::vector<VkDescriptorSet>>& descriptorSets = frameBuffer.getDescriptorSet();
            Shader* shader = const_cast<Shader*>(states.shader);

            unsigned int descriptorId = shader->getId();
            for (size_t i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {

                std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

                VkDescriptorImageInfo headPtrDescriptorImageInfo;
                headPtrDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                headPtrDescriptorImageInfo.imageView = headPtrTextureImageView[i];
                headPtrDescriptorImageInfo.sampler = headPtrTextureSampler[i];

                descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[0].dstSet = descriptorSets[descriptorId][i];
                descriptorWrites[0].dstBinding = 0;
                descriptorWrites[0].dstArrayElement = 0;
                descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                descriptorWrites[0].descriptorCount = 1;
                descriptorWrites[0].pImageInfo = &headPtrDescriptorImageInfo;

                VkDescriptorBufferInfo linkedListStorageBufferInfoLastFrame{};
                linkedListStorageBufferInfoLastFrame.buffer = linkedListShaderStorageBuffers[i];
                linkedListStorageBufferInfoLastFrame.offset = 0;
                unsigned int nodeSize = 5 * sizeof(float) + sizeof(unsigned int);
                linkedListStorageBufferInfoLastFrame.range = maxNodes * nodeSize;

                descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[1].dstSet = descriptorSets[descriptorId][i];
                descriptorWrites[1].dstBinding = 1;
                descriptorWrites[1].dstArrayElement = 0;
                descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                descriptorWrites[1].descriptorCount = 1;
                descriptorWrites[1].pBufferInfo = &linkedListStorageBufferInfoLastFrame;

                vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
            }
        }
        void PerPixelLinkedListRenderComponent::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
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
        void PerPixelLinkedListRenderComponent::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
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
        void PerPixelLinkedListRenderComponent::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandBuffer cmd) {
            //std::cout<<"opy buffers"<<std::endl;
            if (srcBuffer != nullptr && dstBuffer != nullptr) {
                VkBufferCopy copyRegion{};
                copyRegion.size = size;
                vkCmdCopyBuffer(cmd, srcBuffer, dstBuffer, 1, &copyRegion);
            }
        }
        void PerPixelLinkedListRenderComponent::compileShaders() {
            const std::string skyboxVertexShader = R"(#version 460
                                                         #extension GL_EXT_debug_printf : enable
                                                         layout (location = 0) in vec3 aPos;
                                                         layout (location = 1) in vec4 color;
                                                         layout (location = 2) in vec2 texCoords;
                                                         layout (location = 3) in vec3 normals;
                                                         layout (location = 4) in int drawableDataID;
                                                         layout(location = 0) out vec4 frontColor;
                                                         layout(location = 1) out vec3 fTexCoords;
                                                         layout(location = 2) out vec3 normal;
                                                         layout(location = 3) out int drawableID;

                                                         layout (push_constant) uniform PushConsts {
                                                            mat4 projectionMatrix;
                                                            mat4 viewMatrix;
                                                         } pushConsts;

                                                         void main()
                                                         {

                                                             gl_PointSize = 2.0f;
                                                             frontColor = color;
                                                             normal = normals;
                                                             fTexCoords = vec3(texCoords.xy, 0);
                                                             fTexCoords = aPos;
                                                             drawableID = drawableDataID;
                                                             gl_Position = vec4(aPos, 1.0) * mat4(mat3(pushConsts.viewMatrix)) * pushConsts.projectionMatrix;
                                                             //debugPrintfEXT("vertex %v4f", gl_Position);
                                                         }
                                                         )";
            const std::string indirectDrawVertexShader = R"(#version 460
                                                            #define M_PI 3.1415926535897932384626433832795
                                                            #define FPI M_PI/4
                                                            #extension GL_EXT_debug_printf : enable
                                                            layout (location = 0) in vec3 position;
                                                            layout (location = 1) in vec4 color;
                                                            layout (location = 2) in vec2 texCoords;
                                                            layout (location = 3) in vec3 normals;
                                                            layout (location = 4) in int drawableDataID;

                                                            layout (push_constant) uniform PushConsts {
                                                                mat4 projectionMatrix;
                                                                mat4 viewMatrix;
                                                                vec4 resolution;
                                                                float time;
                                                            } pushConsts;
                                                            struct ModelData {
                                                                mat4 modelMatrix;
                                                            };
                                                            struct MaterialData {
                                                                vec2 uvScale;
                                                                vec2 uvOffset;
                                                                uint textureIndex;
                                                                uint materialType;
                                                                uint _padding1;
                                                                uint _padding2;
                                                            };
                                                            layout(std430, set = 0, binding = 3) buffer modelData {
                                                                ModelData modelDatas[];
                                                            };
                                                            layout(std430, set = 0, binding = 4) buffer materialData {
                                                                MaterialData materialDatas[];
                                                            };
                                                            layout(location = 0) out vec4 frontColor;
                                                            layout(location = 1) out vec2 fTexCoords;
                                                            layout(location = 2) out uint texIndex;
                                                            layout(location = 3) out vec3 normal;
                                                            layout(location = 4) out int drawableID;


                                                            void main() {

                                                                gl_PointSize = 2.0f;
                                                                MaterialData materialData = materialDatas[gl_DrawID];
                                                                ModelData modelData = modelDatas[gl_InstanceIndex];

                                                                float xOff = 0;
                                                                float yOff = 0;
                                                                if (materialData.materialType == 1) {
                                                                    yOff = 0.05*sin(position.x*12+pushConsts.time*FPI)*pushConsts.resolution.y;
                                                                    xOff = 0.025*cos(position.x*12+pushConsts.time*FPI)*pushConsts.resolution.x;
                                                                }
                                                                uint textureIndex =  materialData.textureIndex;


                                                                gl_Position = vec4((position.x - xOff), (position.y + yOff), position.z, 1.f) * modelData.modelMatrix * pushConsts.viewMatrix * pushConsts.projectionMatrix;
                                                                fTexCoords = texCoords * materialData.uvScale + materialData.uvOffset;
                                                                frontColor = color;
                                                                texIndex = textureIndex;
                                                                normal = normals;
                                                                drawableID = drawableDataID;
                                                            }
                                                            )";
                const std::string  simpleVertexShader = R"(#version 460
                                                        layout (location = 0) in vec3 position;
                                                        layout (location = 1) in vec4 color;
                                                        layout (location = 2) in vec2 texCoords;
                                                        layout (location = 3) in vec3 normals;
                                                        layout (location = 4) in int drawableDataID;

                                                        layout(location = 0) out vec4 frontColor;
                                                        layout(location = 1) out vec2 fTexCoords;
                                                        layout(location = 2) out vec3 normal;
                                                        layout(location = 3) out int drawableID;
                                                        layout (push_constant) uniform PushConsts {
                                                            mat4 projectionMatrix;
                                                            mat4 viewMatrix;
                                                            mat4 worldMat;
                                                        } pushConsts;
                                                        void main () {

                                                             gl_Position = vec4(position, 1.f) * pushConsts.worldMat * pushConsts.viewMatrix * pushConsts.projectionMatrix;
                                                             gl_PointSize = 2.0f;
                                                             frontColor = color;
                                                             fTexCoords = texCoords;
                                                             normal = normals;
                                                             drawableID = drawableDataID;
                                                        })";
                const std::string skyboxFragmentShader = R"(#version 460
                                                            #extension GL_EXT_debug_printf : enable
                                                            #extension GL_EXT_nonuniform_qualifier : enable
                                                            layout (location = 0) out vec4 fcolor;
                                                            layout(location = 0) in vec4 frontColor;
                                                            layout(location = 1) in vec3 fTexCoords;
                                                            layout(location = 2) in vec3 normal;
                                                            layout(location = 3) in flat int drawableID;
                                                            layout (binding = 0) uniform samplerCube skybox;
                                                            void main() {
                                                                //debugPrintfEXT("cubemap fragment shader");
                                                                fcolor = texture(skybox, fTexCoords);
                                                            }
                                                            )";
                const std::string fragmentShader = R"(#version 460
                                                      #extension GL_ARB_separate_shader_objects : enable
                                                      #extension GL_EXT_nonuniform_qualifier : enable
                                                      #extension GL_EXT_debug_printf : enable
                                                      layout (early_fragment_tests) in;
                                                      struct NodeType {
                                                          vec4 color;
                                                          float depth;
                                                          uint next;
                                                      };
                                                      layout(std430, set = 0, binding = 0) buffer CounterSSBO {
                                                          uint count;
                                                          uint maxNodes;
                                                      };
                                                      layout(set = 0, binding = 1, r32ui) uniform coherent uimage2D headPointers;
                                                      layout(std430, set = 0, binding = 2) buffer LinkedLists {
                                                          NodeType nodes[];
                                                      };
                                                      layout(set = 0, binding = 5) uniform sampler2D textures[];
                                                      layout(location = 0) in vec4 frontColor;
                                                      layout(location = 1) in vec2 fTexCoords;
                                                      layout(location = 2) in flat uint texIndex;
                                                      layout(location = 3) in vec3 normal;
                                                      layout(location = 4) in flat int drawableID;
                                                      void main() {
                                                           uint nodeIdx = atomicAdd(count, 1);
                                                           vec4 color = (texIndex != 0) ? frontColor * texture(textures[texIndex-1], fTexCoords.xy) : frontColor;
                                                           /*if (texIndex == 8)
                                                                debugPrintfEXT("draw particles");*/

                                                           if (nodeIdx < maxNodes) {
                                                                uint prevHead = imageAtomicExchange(headPointers, ivec2(gl_FragCoord.xy), nodeIdx);
                                                                nodes[nodeIdx].color = color;
                                                                nodes[nodeIdx].depth = gl_FragCoord.z;
                                                                nodes[nodeIdx].next = prevHead;

                                                           }

                                                      })";
                 const std::string fragmentShader2 =
                                                   R"(
                                                   #version 460
                                                   #extension GL_ARB_separate_shader_objects : enable
                                                   #extension GL_EXT_nonuniform_qualifier : enable
                                                   #extension GL_EXT_debug_printf : enable
                                                   #define MAX_FRAGMENTS 20
                                                   struct NodeType {
                                                      vec4 color;
                                                      float depth;
                                                      uint next;
                                                   };
                                                   layout(set = 0, binding = 0, r32ui) uniform uimage2D headPointers;
                                                   layout(std430, set = 0, binding = 1) buffer linkedLists {
                                                       NodeType nodes[];
                                                   };
                                                   layout(location = 0) in vec4 frontColor;
                                                   layout(location = 1) in vec2 fTexCoords;
                                                   layout(location = 2) in vec3 normal;
                                                   layout(location = 3) in flat int drawableID;
                                                   layout(location = 0) out vec4 fcolor;
                                                   void main() {
                                                      //debugPrintfEXT("color = ");
                                                      NodeType frags[MAX_FRAGMENTS];
                                                      int count = 0;
                                                      uint n = imageLoad(headPointers, ivec2(gl_FragCoord.xy)).r;
                                                      while( n != 0xffffffffu && count < MAX_FRAGMENTS) {
                                                           frags[count] = nodes[n];
                                                           n = frags[count].next;
                                                           count++;
                                                      }

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
                                                        //color = mix (color, frags[i].color, frags[i].color.a);

                                                      }
                                                      /*if (color.r != 0 || color.g != 0 || color.b != 0 || color.a != 0)
                                                        debugPrintfEXT("color : %v4f", color);*/
                                                      fcolor = color;
                                                   })";
            if (!skyboxShader.loadFromMemory(skyboxVertexShader, skyboxFragmentShader)) {
                throw core::Erreur(57, "Failed to load skybox rendering shader");
            }
            if (!indirectRenderingShader.loadFromMemory(indirectDrawVertexShader, fragmentShader)) {
                throw core::Erreur(57, "Failed to load indirect rendering shader");
            }
            if (!perPixelLinkedListP2.loadFromMemory(simpleVertexShader, fragmentShader2)) {
                throw core::Erreur(55, "Failed to load per pixel linked list pass 2 shader");
            }


            math::Matrix4f viewMatrix = getWindow().getDefaultView().getViewMatrix().getMatrix()/*.transpose()*/;
            math::Matrix4f projMatrix = getWindow().getDefaultView().getProjMatrix().getMatrix()/*.transpose()*/;
            ppll2PushConsts.viewMatrix = toVulkanMatrix(viewMatrix);
            ppll2PushConsts.projMatrix = toVulkanMatrix(projMatrix);

            //ppll2PushConsts.projMatrix.m22 *= -1;
            indirectDrawPushConsts.resolution = resolution;
        }
        void PerPixelLinkedListRenderComponent::fillBuffersMT() {
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
                    ////////std::cout<<"next frame draw normal"<<std::endl;
                    float time = timeClock.getElapsedTime().asSeconds();
                    indirectDrawPushConsts.time = time;

                    unsigned int p = m_normals[i].getAllVertices().getPrimitiveType();


                    unsigned int vertexCount = 0;
                    MaterialData material;
                    {
                        std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                        material.textureIndex = (m_normals[i].getMaterial().getTexture() != nullptr) ? m_normals[i].getMaterial().getTexture()->getId() : 0;
                        material.materialType = m_normals[i].getMaterial().getType();
                        material.uvScale = (m_normals[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_normals[i].getMaterial().getTexture()->getSize().x(), 1.f / m_normals[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);

                    }

                    materialDatas[p].push_back(material);
                    for (unsigned int j = 0; j < m_normals[i].getAllVertices().getVertexCount(); j++) {

                        vbBindlessTex[p].append(m_normals[i].getAllVertices()[j]);
                        vertexCount++;
                    }
                    TransformMatrix tm;
                    ModelData modelData;
                    modelData.worldMat = toVulkanMatrix(tm.getMatrix());


                    modelDatas[p].push_back(modelData);

                    drawArraysIndirectCommand.count = vertexCount;
                    drawArraysIndirectCommand.firstIndex = firstIndex[p] + oldTotalVertexCount[p];;
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
                    float time = timeClock.getElapsedTime().asSeconds();
                    indirectDrawPushConsts.time = time;

                    std::vector<TransformMatrix*> tm = m_instances[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData modelData;
                        modelData.worldMat = toVulkanMatrix(tm[j]->getMatrix());

                        modelDatas[p].push_back(modelData);
                    }
                    MaterialData material;
                    {
                        std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                        material.textureIndex = (m_instances[i].getMaterial().getTexture() != nullptr) ? m_instances[i].getMaterial().getTexture()->getId() : 0;
                        material.materialType = m_instances[i].getMaterial().getType();
                        material.uvScale = (m_instances[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_instances[i].getMaterial().getTexture()->getSize().x(), 1.f / m_instances[i].getMaterial().getTexture()->getSize().y()): math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        //////std::cout<<"texture matrix : "<<m_instances[i].getMaterial().getTexture()->getTextureMatrix()<<std::endl;
                    }
                    materialDatas[p].push_back(material);
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
                    ////////std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                    ////////std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;

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
            unsigned int currentFrame = frameBuffer.getCurrentFrame();
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

                    /*vkMapMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemory, 0, totalBufferSizeDrawCommand[p], 0, &data);
                    memcpy(data, drawArraysIndirectCommands[p].data(), (size_t)totalBufferSizeDrawCommand[p]);
                    vkUnmapMemory(vkDevice.getDevice(), vboIndirectStagingBufferMemory);*/


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
        void PerPixelLinkedListRenderComponent::fillIndexedBuffersMT () {
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
                    ////std::cout<<"add instance"<<std::endl;

                    DrawElementsIndirectCommand drawElementsIndirectCommand;
                    float time = timeClock.getElapsedTime().asSeconds();
                    indirectDrawPushConsts.time = time;
                    unsigned int p = m_normalsIndexed[i].getAllVertices().getPrimitiveType();

                    MaterialData material;
                    {
                        std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                        material.textureIndex = (m_normalsIndexed[i].getMaterial().getTexture() != nullptr) ? m_normalsIndexed[i].getMaterial().getTexture()->getId() : 0;
                        material.materialType = m_normalsIndexed[i].getMaterial().getType();
                        material.uvScale = (m_normalsIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_normalsIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_normalsIndexed[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);

                    }

                    materialDatas[p].push_back(material);

                    TransformMatrix tm;
                    ModelData modelData;
                    modelData.worldMat = toVulkanMatrix(tm.getMatrix())/*.transpose()*/;

                    modelDatas[p].push_back(modelData);
                    unsigned int indexCount = 0, vertexCount = 0;
                    for (unsigned int j = 0; j < m_normalsIndexed[i].getAllVertices().getVertexCount(); j++) {
                        ////std::cout<<"add vertex"<<std::endl;
                        vbBindlessTexIndexed[p].append(m_normalsIndexed[i].getAllVertices()[j]);
                        vertexCount++;
                    }
                    for (unsigned int j = 0; j < m_normalsIndexed[i].getAllVertices().getIndexes().size(); j++) {
                        vbBindlessTexIndexed[p].addIndex(m_normalsIndexed[i].getAllVertices().getIndexes()[j]);
                        indexCount++;
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
                    float time = timeClock.getElapsedTime().asSeconds();
                    indirectDrawPushConsts.time = time;

                    MaterialData material;
                    material.textureIndex = (m_instancesIndexed[i].getMaterial().getTexture() != nullptr) ? m_instancesIndexed[i].getMaterial().getTexture()->getId() : 0;
                    material.materialType = m_instancesIndexed[i].getMaterial().getType();
                    material.uvScale = (m_instancesIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_instancesIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_instancesIndexed[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                    material.uvOffset = math::Vec2f(0, 0);
                    materialDatas[p].push_back(material);

                    std::vector<TransformMatrix*> tm = m_instancesIndexed[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData model;
                        model.worldMat = toVulkanMatrix(tm[j]->getMatrix())/*.transpose()*/;

                        modelDatas[p].push_back(model);
                    }

                    unsigned int indexCount = 0, vertexCount = 0;
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
            unsigned int currentFrame = frameBuffer.getCurrentFrame();
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
        void PerPixelLinkedListRenderComponent::fillSelectedBuffersMT() {
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
            for (unsigned int i = 0; i < m_selected.size(); i++) {
                if (m_selected[i].getAllVertices().getVertexCount() > 0) {
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    ////////std::cout<<"next frame draw normal"<<std::endl;
                    float time = timeClock.getElapsedTime().asSeconds();
                    indirectDrawPushConsts.time = time;

                    unsigned int p = m_selected[i].getAllVertices().getPrimitiveType();


                    unsigned int vertexCount = 0;
                    MaterialData material;
                    material.textureIndex = (m_selected[i].getMaterial().getTexture() != nullptr) ? m_selected[i].getMaterial().getTexture()->getId() : 0;
                    material.materialType = m_selected[i].getMaterial().getType();
                    material.uvScale = (m_selected[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_selected[i].getMaterial().getTexture()->getSize().x(), 1.f / m_selected[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                    material.uvOffset = math::Vec2f(0, 0);
                    materialDatas[p].push_back(material);
                    for (unsigned int j = 0; j < m_selected[i].getAllVertices().getVertexCount(); j++) {
                        vbBindlessTex[p].append(m_selected[i].getAllVertices()[j]);
                        vertexCount++;
                    }
                    TransformMatrix tm;
                    ModelData modelData;
                    modelData.worldMat = toVulkanMatrix(tm.getMatrix());

                    modelDatas[p].push_back(modelData);

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
            for (unsigned int i = 0; i < m_selectedInstance.size(); i++) {
                if (m_selectedInstance[i].getAllVertices().getVertexCount() > 0) {
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    unsigned int p = m_selectedInstance[i].getAllVertices().getPrimitiveType();
                    float time = timeClock.getElapsedTime().asSeconds();
                    indirectDrawPushConsts.time = time;

                    std::vector<TransformMatrix*> tm = m_selectedInstance[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData modelData;
                        modelData.worldMat = toVulkanMatrix(tm[j]->getMatrix());

                        modelDatas[p].push_back(modelData);
                    }
                    MaterialData material;
                    material.textureIndex = (m_selectedInstance[i].getMaterial().getTexture() != nullptr) ? m_selectedInstance[i].getMaterial().getTexture()->getId() : 0;
                    material.materialType = m_selectedInstance[i].getMaterial().getType();
                    material.uvScale = (m_selectedInstance[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_selectedInstance[i].getMaterial().getTexture()->getSize().x(), 1.f / m_selectedInstance[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                    material.uvOffset = math::Vec2f(0, 0);
                    materialDatas[p].push_back(material);
                    unsigned int vertexCount = 0;
                    if (m_selectedInstance[i].getEntities().size() > 0) {
                        Entity* firstInstance = m_selectedInstance[i].getEntities()[0];
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
                    ////////std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                    ////////std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;

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
            unsigned int currentFrame = frameBuffer.getCurrentFrame();
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
        void PerPixelLinkedListRenderComponent::fillSelectedIndexedBuffersMT() {
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
            for (unsigned int i = 0; i < m_selectedIndexed.size(); i++) {
                if (m_selectedIndexed[i].getAllVertices().getVertexCount() > 0) {
                    DrawElementsIndirectCommand drawElementsIndirectCommand;
                    ////std::cout<<"next frame draw normal"<<std::endl;

                    float time = timeClock.getElapsedTime().asSeconds();
                    indirectDrawPushConsts.time = time;

                    unsigned int p = m_selectedIndexed[i].getAllVertices().getPrimitiveType();

                    MaterialData material;
                    material.textureIndex = (m_selectedIndexed[i].getMaterial().getTexture() != nullptr) ? m_selectedIndexed[i].getMaterial().getTexture()->getId() : 0;
                    material.materialType = m_selectedIndexed[i].getMaterial().getType();
                    material.uvScale = (m_selectedIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_selectedIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_selectedIndexed[i].getMaterial().getTexture()->getSize().y()): math::Vec2f(0, 0);
                    material.uvOffset = math::Vec2f(0, 0);
                    materialDatas[p].push_back(material);

                    TransformMatrix tm;
                    ModelData model;
                    model.worldMat = toVulkanMatrix(tm.getMatrix())/*.transpose()*/;

                    modelDatas[p].push_back(model);
                    unsigned int vertexCount = 0, indexCount = 0;
                    for (unsigned int j = 0; j < m_selectedIndexed[i].getAllVertices().getVertexCount(); j++) {
                        vertexCount++;
                        vbBindlessTexIndexed[p].append(m_selectedIndexed[i].getAllVertices()[j]);
                    }
                    for (unsigned int j = 0; j < m_selectedIndexed[i].getAllVertices().getIndexes().size(); j++) {
                        indexCount++;
                        vbBindlessTexIndexed[p].addIndex(m_selectedIndexed[i].getAllVertices().getIndexes()[j]);
                    }
                    drawElementsIndirectCommand.index_count = indexCount;
                    drawElementsIndirectCommand.first_index = firstIndex[p] + oldTotalIndexCount[p];;
                    drawElementsIndirectCommand.instance_base = baseInstance[p];
                    drawElementsIndirectCommand.vertex_base = baseVertex[p] + oldTotalVertexIndexCount[p];;
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
            for (unsigned int i = 0; i < m_selectedInstanceIndexed.size(); i++) {

                if (m_selectedInstanceIndexed[i].getAllVertices().getVertexCount() > 0) {
                    DrawElementsIndirectCommand drawElementsIndirectCommand;
                    unsigned int p = m_selectedInstanceIndexed[i].getAllVertices().getPrimitiveType();

                    MaterialData material;
                    material.textureIndex = (m_selectedInstanceIndexed[i].getMaterial().getTexture() != nullptr) ? m_selectedInstanceIndexed[i].getMaterial().getTexture()->getId() : 0;
                    material.materialType = m_selectedInstanceIndexed[i].getMaterial().getType();
                    material.uvScale = (m_selectedInstanceIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_selectedInstanceIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_selectedInstanceIndexed[i].getMaterial().getTexture()->getSize().y()): math::Vec2f(0, 0);
                    material.uvOffset = math::Vec2f(0, 0);
                    materialDatas[p].push_back(material);
                    std::vector<TransformMatrix*> tm = m_selectedInstanceIndexed[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData model;
                        model.worldMat = toVulkanMatrix(tm[j]->getMatrix())/*.transpose()*/;

                        modelDatas[p].push_back(model);
                    }
                    unsigned int indexCount = 0, vertexCount = 0;
                    if (m_selectedInstanceIndexed[i].getEntities().size() > 0) {
                        Entity* firstInstance = m_selectedInstanceIndexed[i].getEntities()[0];
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
                    ////////std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                    ////////std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;
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
            unsigned int currentFrame = frameBuffer.getCurrentFrame();
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
        void PerPixelLinkedListRenderComponent::fillOutlineBuffersMT() {
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
            for (unsigned int i = 0; i < m_selectedScale.size(); i++) {
                if (m_selectedScale[i].getAllVertices().getVertexCount() > 0) {
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    ////////std::cout<<"next frame draw normal"<<std::endl;

                    unsigned int p = m_selectedScale[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = 0;
                    material.materialType = 0;
                    material.uvScale = math::Vec2f(0, 0);
                    material.uvOffset = math::Vec2f(0, 0);

                    materialDatas[p].push_back(material);
                    TransformMatrix tm;
                    ModelData model;
                    model.worldMat = toVulkanMatrix(tm.getMatrix());

                    modelDatas[p].push_back(model);

                    unsigned int vertexCount = 0;
                    for (unsigned int j = 0; j < m_selectedScale[i].getAllVertices().getVertexCount(); j++) {
                        vertexCount++;
                        vbBindlessTex[p].append(m_selectedScale[i].getAllVertices()[j]);
                        ////////std::cout<<"color : "<<(int) m_selectedScale[i].getAllVertices()[j].color.r<<","<<(int) m_selectedScale[i].getAllVertices()[j].color.g<<","<<(int) m_selectedScale[i].getAllVertices()[j].color.b<<std::endl;
                    }
                    drawArraysIndirectCommand.count = vertexCount;
                    drawArraysIndirectCommand.firstIndex = firstIndex[p] + oldTotalVertexCount[p];;
                    drawArraysIndirectCommand.baseInstance = baseInstance[p];
                    drawArraysIndirectCommand.instanceCount = 1;
                    drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                    firstIndex[p] += vertexCount;
                    baseInstance[p] += 1;
                    totalVertexCount[p] += vertexCount;
                    drawCommandCount[p]++;
                }
            }
            for (unsigned int i = 0; i < m_selectedScaleInstance.size(); i++) {
                unsigned int p = m_selectedScaleInstance[i].getAllVertices().getPrimitiveType();
                if (m_selectedScaleInstance[i].getAllVertices().getVertexCount() > 0) {
                    DrawArraysIndirectCommand drawArraysIndirectCommand;

                    MaterialData material;
                    material.textureIndex = 0;
                    material.materialType = 0;
                    material.uvScale = math::Vec2f(0, 0);
                    material.uvOffset = math::Vec2f(0, 0);
                    materialDatas[p].push_back(material);
                    std::vector<TransformMatrix*> tm = m_selectedScaleInstance[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData model;
                        model.worldMat = toVulkanMatrix(tm[j]->getMatrix());

                        modelDatas[p].push_back(model);
                    }
                    unsigned int vertexCount = 0;
                    if (m_selectedScaleInstance[i].getEntities().size() > 0) {
                        Entity* firstInstance = m_selectedScaleInstance[i].getEntities()[0];
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

                    ////////std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                    ////////std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;

                }
            }
            std::array<unsigned int, Batcher::nbPrimitiveTypes> alignedOffsetModelData, alignedOffsetMaterialData;
            for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                nbDrawCommandBuffer[p].push_back(drawCommandCount[p]);
                alignedOffsetModelData[p] = align(currentModelOffset[p]);
                modelDataOffsets[p].push_back(alignedOffsetModelData[p]);
                alignedOffsetMaterialData[p] = align(currentMaterialOffset[p]);
                materialDataOffsets[p].push_back(alignedOffsetMaterialData[p]);
                //std::cout<<"aligned model : "<<alignedOffsetModelData[p]<<std::endl<<"aligned material : "<<alignedOffsetMaterialData[p]<<std::endl;
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
            unsigned int currentFrame = frameBuffer.getCurrentFrame();
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
        void PerPixelLinkedListRenderComponent::fillOutlineIndexedBuffersMT () {
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
            for (unsigned int i = 0; i < m_selectedScaleIndexed.size(); i++) {

                if (m_selectedScaleIndexed[i].getAllVertices().getVertexCount() > 0) {
                    DrawElementsIndirectCommand drawElementsIndirectCommand;
                    ////////std::cout<<"next frame draw normal"<<std::endl;
                    unsigned int p = m_selectedScaleIndexed[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = 0;
                    material.materialType = 0;
                    material.uvScale = math::Vec2f(0, 0);
                    material.uvOffset = math::Vec2f(0, 0);
                    materialDatas[p].push_back(material);

                    TransformMatrix tm;
                    ModelData model;
                    model.worldMat = toVulkanMatrix(tm.getMatrix())/*.transpose()*/;

                    modelDatas[p].push_back(model);


                    unsigned int indexCount = 0, vertexCount = 0;
                    for (unsigned int j = 0; j < m_selectedScaleIndexed[i].getAllVertices().getVertexCount(); j++) {
                        vertexCount++;
                        vbBindlessTexIndexed[p].append(m_selectedScaleIndexed[i].getAllVertices()[j]);
                    }
                    for (unsigned int j = 0; j < m_selectedScaleIndexed[i].getAllVertices().getIndexes().size(); j++) {
                        indexCount++;
                        vbBindlessTexIndexed[p].addIndex(m_selectedScaleIndexed[i].getAllVertices().getIndexes()[j]);
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
            for (unsigned int i = 0; i < m_selectedScaleInstanceIndexed.size(); i++) {

                if (m_selectedScaleInstanceIndexed[i].getAllVertices().getVertexCount() > 0) {
                    DrawElementsIndirectCommand drawElementsIndirectCommand;
                    unsigned int p = m_selectedScaleInstanceIndexed[i].getAllVertices().getPrimitiveType();

                    MaterialData material;
                    material.textureIndex = 0;
                    material.materialType = m_selectedScaleInstanceIndexed[i].getMaterial().getType();
                    material.uvScale = (m_selectedScaleInstanceIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_selectedScaleInstanceIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_selectedScaleInstanceIndexed[i].getMaterial().getTexture()->getSize().y()): math::Vec2f(0, 0);
                    material.uvOffset = math::Vec2f(0, 0);
                    materialDatas[p].push_back(material);
                    std::vector<TransformMatrix*> tm = m_selectedScaleInstanceIndexed[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData model;
                        model.worldMat = toVulkanMatrix(tm[j]->getMatrix())/*.transpose()*/;

                        modelDatas[p].push_back(model);
                        ////std::cout<<modelDatas[p].size()<<std::endl;

                    }
                    unsigned int indexCount = 0, vertexCount = 0;
                    if (m_selectedScaleInstanceIndexed[i].getEntities().size() > 0) {
                        Entity* firstInstance = m_selectedScaleInstanceIndexed[i].getEntities()[0];
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
                    drawElementsIndirectCommand.first_index = firstIndex[p] + oldTotalIndexCount[p];;
                    drawElementsIndirectCommand.instance_base = baseInstance[p];
                    drawElementsIndirectCommand.vertex_base = baseVertex[p] + oldTotalVertexIndexCount[p];;
                    drawElementsIndirectCommand.instance_count = tm.size();
                    drawElementsIndirectCommands[p].push_back(drawElementsIndirectCommand);
                    firstIndex[p] += indexCount;
                    baseVertex[p] += vertexCount;
                    baseInstance[p] += tm.size();
                    totalIndexCount[p] += indexCount;
                    totalVertexIndexCount[p] += vertexCount;
                    drawCommandCount[p]++;
                    ////////std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                    ////////std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;
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
            unsigned int currentFrame = frameBuffer.getCurrentFrame();
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
        void PerPixelLinkedListRenderComponent::drawInstances() {
            for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                vbBindlessTex[i].clear();
                materialDatas[i].clear();
                modelDatas[i].clear();
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
                    ////////std::cout<<"next frame draw normal"<<std::endl;
                    float time = timeClock.getElapsedTime().asSeconds();
                    indirectDrawPushConsts.time = time;

                    unsigned int p = m_normals[i].getAllVertices().getPrimitiveType();

                    /*if (m_normals[i].getVertexArrays()[0]->getEntity()->getRootType() == "E_MONSTER") {
                            //////std::cout<<"tex coords : "<<(*m_normals[i].getVertexArrays()[0])[0].texCoords.x<<","<<(*m_normals[i].getVertexArrays()[0])[0].texCoords.y<<std::endl;
                        }*/
                    unsigned int vertexCount = 0;
                    MaterialData material;
                    {
                        std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                        material.textureIndex = (m_normals[i].getMaterial().getTexture() != nullptr) ? m_normals[i].getMaterial().getTexture()->getId() : 0;
                        material.materialType = m_normals[i].getMaterial().getType();
                        material.uvScale = (m_normals[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_normals[i].getMaterial().getTexture()->getSize().x(), 1.f / m_normals[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        ////std::cout<<"texture matrix : "<<m_normals[i].getMaterial().getTexture()->getTextureMatrix()<<std::endl;
                    }

                    materialDatas[p].push_back(material);
                    for (unsigned int j = 0; j < m_normals[i].getAllVertices().getVertexCount(); j++) {
                        ////std::cout<<"add vertex"<<std::endl;
                        vbBindlessTex[p].append(m_normals[i].getAllVertices()[j]);
                        vertexCount++;
                    }
                    TransformMatrix tm;
                    ModelData modelData;
                    modelData.worldMat = toVulkanMatrix(tm.getMatrix())/*.transpose()*/;

                    modelDatas[p].push_back(modelData);

                    drawArraysIndirectCommand.count = vertexCount;
                    drawArraysIndirectCommand.firstIndex = firstIndex[p];
                    drawArraysIndirectCommand.baseInstance = baseInstance[p];
                    drawArraysIndirectCommand.instanceCount = 1;
                    drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                    firstIndex[p] += vertexCount;
                    baseInstance[p] += 1;
                    /*for (unsigned int j = 0; j < m_normals[i].getVertexArrays().size(); j++) {
                        if (m_normals[i].getVertexArrays()[j]->getEntity() != nullptr && m_normals[i].getVertexArrays()[j]->getEntity()->getRootType() == "E_HERO") {
                            for (unsigned int n = 0; n < m_normals[i].getVertexArrays()[j]->getVertexCount(); n++)
                                //////std::cout<<"position hero : "<<(*m_normals[i].getVertexArrays()[j])[n].position.x<<","<<(*m_normals[i].getVertexArrays()[j])[n].position.y<<","<<(*m_normals[i].getVertexArrays()[j])[n].position.z<<std::endl;
                        }
                    }*/
                }
            }
            for (unsigned int i = 0; i < m_instances.size(); i++) {
                if (m_instances[i].getAllVertices().getVertexCount() > 0) {
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    unsigned int p = m_instances[i].getAllVertices().getPrimitiveType();
                    float time = timeClock.getElapsedTime().asSeconds();
                    indirectDrawPushConsts.time = time;

                    std::vector<TransformMatrix*> tm = m_instances[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData modelData;
                        modelData.worldMat = toVulkanMatrix(tm[j]->getMatrix())/*.transpose()*/;
                        modelDatas[p].push_back(modelData);
                    }
                    MaterialData material;
                    {
                        std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                        material.textureIndex = (m_instances[i].getMaterial().getTexture() != nullptr) ? m_instances[i].getMaterial().getTexture()->getId() : 0;
                        material.materialType = m_instances[i].getMaterial().getType();
                        material.uvScale = (m_instances[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_instances[i].getMaterial().getTexture()->getSize().x(), 1.f / m_instances[i].getMaterial().getTexture()->getSize().y()): math::Vec2f(0, 0);
                        material.uvOffset = math::Vec2f(0, 0);
                        //////std::cout<<"texture matrix : "<<m_instances[i].getMaterial().getTexture()->getTextureMatrix()<<std::endl;
                    }
                    materialDatas[p].push_back(material);
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
                    ////////std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                    ////////std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;

                }
            }
            RenderStates currentStates;
            currentStates.blendMode = BlendNone;
            currentStates.shader = &indirectRenderingShader;
            currentStates.texture = nullptr;
            frameBuffer.enableStencilTest(false);
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
                        modelDataShaderStorageBuffers.resize(frameBuffer.getMaxFramesInFlight());
                        modelDataShaderStorageBuffersMemory.resize(frameBuffer.getMaxFramesInFlight());
                        for (unsigned int i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {
                            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, modelDataShaderStorageBuffers[i], modelDataShaderStorageBuffersMemory[i]);
                        }
                        maxModelDataSize = bufferSize;
                        needToUpdateDS  = true;
                    }


                    void* data;
                    vkMapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, modelDatas[p].data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory);
                    for (unsigned int i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {
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
                        materialDataShaderStorageBuffers.resize(frameBuffer.getMaxFramesInFlight());
                        materialDataShaderStorageBuffersMemory.resize(frameBuffer.getMaxFramesInFlight());
                        for (unsigned int i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {
                            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, materialDataShaderStorageBuffers[i], materialDataShaderStorageBuffersMemory[i]);
                        }
                        maxMaterialDataSize = bufferSize;
                        needToUpdateDS = true;
                    }

                    vkMapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, materialDatas[p].data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory);
                    for (unsigned int i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {
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
                    createCommandBuffersIndirect(p, drawArraysIndirectCommands[p].size(), sizeof(DrawArraysIndirectCommand), PPLLNODEPTHNOSTENCIL, currentStates);

                }
            }
        }
        void PerPixelLinkedListRenderComponent::drawInstancesIndexed() {
            for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                vbBindlessTex[i].clear();
                materialDatas[i].clear();
                modelDatas[i].clear();
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
                    ////std::cout<<"add instance"<<std::endl;

                    DrawElementsIndirectCommand drawElementsIndirectCommand;
                    float time = timeClock.getElapsedTime().asSeconds();
                    indirectDrawPushConsts.time = time;
                    unsigned int p = m_normalsIndexed[i].getAllVertices().getPrimitiveType();

                    MaterialData material;
                    material.textureIndex = (m_normalsIndexed[i].getMaterial().getTexture() != nullptr) ? m_normalsIndexed[i].getMaterial().getTexture()->getId() : 0;
                    material.materialType = m_normalsIndexed[i].getMaterial().getType();
                    material.uvScale = (m_normalsIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_normalsIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_normalsIndexed[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                    material.uvOffset = math::Vec2f(0, 0);
                    materialDatas[p].push_back(material);

                    TransformMatrix tm;
                    ModelData modelData;
                    modelData.worldMat = toVulkanMatrix(tm.getMatrix())/*.transpose()*/;
                    modelDatas[p].push_back(modelData);
                    unsigned int indexCount = 0, vertexCount = 0;
                    for (unsigned int j = 0; j < m_normalsIndexed[i].getAllVertices().getVertexCount(); j++) {
                        ////std::cout<<"add vertex"<<std::endl;
                        vbBindlessTex[p].append(m_normalsIndexed[i].getAllVertices()[j]);
                        vertexCount++;
                    }
                    for (unsigned int j = 0; j < m_normalsIndexed[i].getAllVertices().getIndexes().size(); j++) {
                        vbBindlessTex[p].addIndex(m_normalsIndexed[i].getAllVertices().getIndexes()[j]);
                        indexCount++;
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
                    float time = timeClock.getElapsedTime().asSeconds();
                    indirectDrawPushConsts.time = time;

                    MaterialData material;
                    material.textureIndex = (m_instancesIndexed[i].getMaterial().getTexture() != nullptr) ? m_instancesIndexed[i].getMaterial().getTexture()->getId() : 0;
                    material.materialType = m_instancesIndexed[i].getMaterial().getType();
                    material.uvScale = (m_instancesIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_instancesIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_instancesIndexed[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                    material.uvOffset = math::Vec2f(0, 0);
                    materialDatas[p].push_back(material);

                    std::vector<TransformMatrix*> tm = m_instancesIndexed[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData model;
                        model.worldMat = toVulkanMatrix(tm[j]->getMatrix())/*.transpose()*/;
                        modelDatas[p].push_back(model);
                    }

                    unsigned int indexCount = 0, vertexCount = 0;
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
                    ////////std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                    ////////std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;
                }
            }
            RenderStates currentStates;
            currentStates.blendMode = BlendNone;
            currentStates.shader = &indirectRenderingShader;
            currentStates.texture = nullptr;
            frameBuffer.enableStencilTest(false);
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
                        modelDataShaderStorageBuffers.resize(frameBuffer.getMaxFramesInFlight());
                        modelDataShaderStorageBuffersMemory.resize(frameBuffer.getMaxFramesInFlight());
                        for (unsigned int i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {
                            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, modelDataShaderStorageBuffers[i], modelDataShaderStorageBuffersMemory[i]);
                        }
                        maxModelDataSize = bufferSize;
                        needToUpdateDS = true;
                    }


                    void* data;
                    vkMapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, modelDatas[p].data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory);
                    for (unsigned int i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {
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
                        materialDataShaderStorageBuffers.resize(frameBuffer.getMaxFramesInFlight());
                        materialDataShaderStorageBuffersMemory.resize(frameBuffer.getMaxFramesInFlight());
                        for (unsigned int i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {
                            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, materialDataShaderStorageBuffers[i], materialDataShaderStorageBuffersMemory[i]);
                        }
                        maxMaterialDataSize = bufferSize;
                        needToUpdateDS = true;
                    }

                    vkMapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, materialDatas[p].data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory);
                    for (unsigned int i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {
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
                    ////////std::cout<<"size : "<<sizeof(DrawElementsIndirectCommand)<<std::endl;
                    createCommandBuffersIndirect(p, drawElementsIndirectCommands[p].size(), sizeof(DrawElementsIndirectCommand), PPLLNODEPTHNOSTENCIL, currentStates);
                }
            }
        }
        void PerPixelLinkedListRenderComponent::drawSelectedInstances() {
            for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                vbBindlessTex[i].clear();
                materialDatas[i].clear();
                modelDatas[i].clear();
            }
            std::array<std::vector<DrawArraysIndirectCommand>, Batcher::nbPrimitiveTypes> drawArraysIndirectCommands;
            std::array<unsigned int, Batcher::nbPrimitiveTypes> firstIndex, baseInstance;
            for (unsigned int i = 0; i < firstIndex.size(); i++) {
                firstIndex[i] = 0;
            }
            for (unsigned int i = 0; i < baseInstance.size(); i++) {
                baseInstance[i] = 0;
            }
            for (unsigned int i = 0; i < m_selected.size(); i++) {
                if (m_selected[i].getAllVertices().getVertexCount() > 0) {
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    ////////std::cout<<"next frame draw normal"<<std::endl;
                    float time = timeClock.getElapsedTime().asSeconds();
                    indirectDrawPushConsts.time = time;

                    unsigned int p = m_selected[i].getAllVertices().getPrimitiveType();


                    unsigned int vertexCount = 0;
                    MaterialData material;
                    material.textureIndex = (m_selected[i].getMaterial().getTexture() != nullptr) ? m_selected[i].getMaterial().getTexture()->getId() : 0;
                    material.materialType = m_selected[i].getMaterial().getType();
                    material.uvScale = (m_selected[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_selected[i].getMaterial().getTexture()->getSize().x(), 1.f / m_selected[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                    material.uvOffset = math::Vec2f(0, 0);
                    materialDatas[p].push_back(material);
                    for (unsigned int j = 0; j < m_selected[i].getAllVertices().getVertexCount(); j++) {
                        vbBindlessTex[p].append(m_selected[i].getAllVertices()[j]);
                        vertexCount++;
                    }
                    TransformMatrix tm;
                    ModelData modelData;
                    modelData.worldMat = toVulkanMatrix(tm.getMatrix());
                    modelDatas[p].push_back(modelData);

                    drawArraysIndirectCommand.count = vertexCount;
                    drawArraysIndirectCommand.firstIndex = firstIndex[p];
                    drawArraysIndirectCommand.baseInstance = baseInstance[p];
                    drawArraysIndirectCommand.instanceCount = 1;
                    drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                    firstIndex[p] += vertexCount;
                    baseInstance[p] += 1;

                }
            }
            for (unsigned int i = 0; i < m_selectedInstance.size(); i++) {
                if (m_selectedInstance[i].getAllVertices().getVertexCount() > 0) {
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    unsigned int p = m_selectedInstance[i].getAllVertices().getPrimitiveType();
                    float time = timeClock.getElapsedTime().asSeconds();
                    indirectDrawPushConsts.time = time;

                    std::vector<TransformMatrix*> tm = m_selectedInstance[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData modelData;
                        modelData.worldMat = toVulkanMatrix(tm[j]->getMatrix());
                        modelDatas[p].push_back(modelData);
                    }
                    MaterialData material;
                    material.textureIndex = (m_selectedInstance[i].getMaterial().getTexture() != nullptr) ? m_selectedInstance[i].getMaterial().getTexture()->getId() : 0;
                    material.materialType = m_selectedInstance[i].getMaterial().getType();
                    material.uvScale = (m_selectedInstance[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_selectedInstance[i].getMaterial().getTexture()->getSize().x(), 1.f / m_selectedInstance[i].getMaterial().getTexture()->getSize().y()) : math::Vec2f(0, 0);
                    material.uvOffset = math::Vec2f(0, 0);
                    materialDatas[p].push_back(material);
                    unsigned int vertexCount = 0;
                    if (m_selectedInstance[i].getVertexArrays().size() > 0) {
                        Entity* entity = m_selectedInstance[i].getVertexArrays()[0]->getEntity();
                        for (unsigned int j = 0; j < m_selectedInstance[i].getVertexArrays().size(); j++) {
                            if (entity == m_selectedInstance[i].getVertexArrays()[j]->getEntity()) {
                                for (unsigned int k = 0; k < m_selectedInstance[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                    vertexCount++;
                                    vbBindlessTex[p].append((*m_selectedInstance[i].getVertexArrays()[j])[k]);
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
                    ////////std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                    ////////std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;

                }
            }
            RenderStates currentStates;
            currentStates.blendMode = BlendNone;
            currentStates.shader = &indirectRenderingShader;
            currentStates.texture = nullptr;
            //frameBuffer.enableStencilTest(true);
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
                        modelDataShaderStorageBuffers.resize(frameBuffer.getMaxFramesInFlight());
                        modelDataShaderStorageBuffersMemory.resize(frameBuffer.getMaxFramesInFlight());
                        for (unsigned int i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {
                            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, modelDataShaderStorageBuffers[i], modelDataShaderStorageBuffersMemory[i]);
                        }
                        maxModelDataSize = bufferSize;
                        needToUpdateDS = true;
                    }


                    void* data;
                    vkMapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, modelDatas[p].data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory);
                    for (unsigned int i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {
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
                        materialDataShaderStorageBuffers.resize(frameBuffer.getMaxFramesInFlight());
                        materialDataShaderStorageBuffersMemory.resize(frameBuffer.getMaxFramesInFlight());
                        for (unsigned int i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {
                            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, materialDataShaderStorageBuffers[i], materialDataShaderStorageBuffersMemory[i]);
                        }
                        maxMaterialDataSize = bufferSize;
                        needToUpdateDS = true;
                    }

                    vkMapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, materialDatas[p].data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory);
                    for (unsigned int i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {
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
                    createCommandBuffersIndirect(p, drawArraysIndirectCommands[p].size(), sizeof(DrawArraysIndirectCommand), PPLLNODEPTHSTENCIL, currentStates);

                }
            }
            for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                vbBindlessTex[i].clear();
            }
            for (unsigned int i = 0; i < firstIndex.size(); i++) {
                firstIndex[i] = 0;
            }
            for (unsigned int i = 0; i < baseInstance.size(); i++) {
                baseInstance[i] = 0;
            }
            for (unsigned int i = 0; i < materialDatas.size(); i++) {
                materialDatas[i].clear();
            }
            for (unsigned int i = 0; i < modelDatas.size(); i++) {
                modelDatas[i].clear();
            }
            for (unsigned int i = 0; i < drawArraysIndirectCommands.size(); i++) {
                drawArraysIndirectCommands[i].clear();
            }
            for (unsigned int i = 0; i < m_selectedScale.size(); i++) {
                if (m_selectedScale[i].getAllVertices().getVertexCount() > 0) {
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    ////////std::cout<<"next frame draw normal"<<std::endl;

                    unsigned int p = m_selectedScale[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = 0;
                    material.materialType = 0;
                    material.uvScale = math::Vec2f(0, 0);
                    material.uvOffset = math::Vec2f(0, 0);

                    materialDatas[p].push_back(material);
                    TransformMatrix tm;
                    ModelData model;
                    model.worldMat = toVulkanMatrix(tm.getMatrix());
                    modelDatas[p].push_back(model);

                    unsigned int vertexCount = 0;
                    for (unsigned int j = 0; j < m_selectedScale[i].getAllVertices().getVertexCount(); j++) {
                        vertexCount++;
                        vbBindlessTex[p].append(m_selectedScale[i].getAllVertices()[j]);
                        ////////std::cout<<"color : "<<(int) m_selectedScale[i].getAllVertices()[j].color.r<<","<<(int) m_selectedScale[i].getAllVertices()[j].color.g<<","<<(int) m_selectedScale[i].getAllVertices()[j].color.b<<std::endl;
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
            for (unsigned int i = 0; i < m_selectedScaleInstance.size(); i++) {
                unsigned int p = m_selectedScaleInstance[i].getAllVertices().getPrimitiveType();
                if (m_selectedScaleInstance[i].getAllVertices().getVertexCount() > 0) {
                    DrawArraysIndirectCommand drawArraysIndirectCommand;

                    MaterialData material;
                    material.textureIndex = 0;
                    material.materialType = 0;
                    material.uvScale = math::Vec2f(0, 0);
                    material.uvOffset = math::Vec2f(0, 0);
                    materialDatas[p].push_back(material);
                    std::vector<TransformMatrix*> tm = m_selectedScaleInstance[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData model;
                        model.worldMat = toVulkanMatrix(tm[j]->getMatrix());
                        modelDatas[p].push_back(model);
                    }
                    unsigned int vertexCount = 0;
                    if (m_selectedScaleInstance[i].getVertexArrays().size() > 0) {
                        Entity* entity = m_selectedScaleInstance[i].getVertexArrays()[0]->getEntity();
                        for (unsigned int j = 0; j < m_selectedScaleInstance[i].getVertexArrays().size(); j++) {
                            if (entity == m_selectedScaleInstance[i].getVertexArrays()[j]->getEntity()) {
                                for (unsigned int k = 0; k < m_selectedScaleInstance[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                    vertexCount++;
                                    vbBindlessTex[p].append((*m_selectedScaleInstance[i].getVertexArrays()[j])[k]);
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
                    ////////std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                    ////////std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;

                }
            }
            currentStates.blendMode = BlendNone;
            currentStates.shader = &indirectRenderingShader;
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
                        modelDataShaderStorageBuffers.resize(frameBuffer.getMaxFramesInFlight());
                        modelDataShaderStorageBuffersMemory.resize(frameBuffer.getMaxFramesInFlight());
                        for (unsigned int i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {
                            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, modelDataShaderStorageBuffers[i], modelDataShaderStorageBuffersMemory[i]);
                        }
                        maxModelDataSize = bufferSize;
                    }


                    void* data;
                    vkMapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, modelDatas[p].data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory);
                    for (unsigned int i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {
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
                        materialDataShaderStorageBuffers.resize(frameBuffer.getMaxFramesInFlight());
                        materialDataShaderStorageBuffersMemory.resize(frameBuffer.getMaxFramesInFlight());
                        for (unsigned int i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {
                            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, materialDataShaderStorageBuffers[i], materialDataShaderStorageBuffersMemory[i]);
                        }
                        maxMaterialDataSize = bufferSize;
                    }

                    vkMapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, materialDatas[p].data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory);
                    for (unsigned int i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {
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
                    ////std::cout<<"draw outline"<<std::endl;
                    createCommandBuffersIndirect(p, drawArraysIndirectCommands[p].size(), sizeof(DrawArraysIndirectCommand), PPLLNODEPTHSTENCILOUTLINE, currentStates);

                }
            }
        }
        void PerPixelLinkedListRenderComponent::drawSelectedInstancesIndexed() {
            for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                vbBindlessTex[i].clear();
                materialDatas[i].clear();
                modelDatas[i].clear();
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
            for (unsigned int i = 0; i < m_selectedIndexed.size(); i++) {
                if (m_selectedIndexed[i].getAllVertices().getVertexCount() > 0) {
                    DrawElementsIndirectCommand drawElementsIndirectCommand;
                    ////std::cout<<"next frame draw normal"<<std::endl;

                    float time = timeClock.getElapsedTime().asSeconds();
                    indirectDrawPushConsts.time = time;

                    unsigned int p = m_selectedIndexed[i].getAllVertices().getPrimitiveType();

                    MaterialData material;
                    material.textureIndex = (m_selectedIndexed[i].getMaterial().getTexture() != nullptr) ? m_selectedIndexed[i].getMaterial().getTexture()->getId() : 0;
                    material.materialType = m_selectedIndexed[i].getMaterial().getType();
                    material.uvScale = (m_selectedIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_selectedIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_selectedIndexed[i].getMaterial().getTexture()->getSize().y()): math::Vec2f(0, 0);
                    material.uvOffset = math::Vec2f(0, 0);
                    materialDatas[p].push_back(material);

                    TransformMatrix tm;
                    ModelData model;
                    model.worldMat = toVulkanMatrix(tm.getMatrix())/*.transpose()*/;
                    modelDatas[p].push_back(model);
                    unsigned int vertexCount = 0, indexCount = 0;
                    for (unsigned int j = 0; j < m_selectedIndexed[i].getAllVertices().getVertexCount(); j++) {
                        vertexCount++;
                        vbBindlessTex[p].append(m_selectedIndexed[i].getAllVertices()[j]);
                    }
                    for (unsigned int j = 0; j < m_selectedIndexed[i].getAllVertices().getIndexes().size(); j++) {
                        indexCount++;
                        vbBindlessTex[p].addIndex(m_selectedIndexed[i].getAllVertices().getIndexes()[j]);
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
            for (unsigned int i = 0; i < m_selectedInstanceIndexed.size(); i++) {

                if (m_selectedInstanceIndexed[i].getAllVertices().getVertexCount() > 0) {
                    DrawElementsIndirectCommand drawElementsIndirectCommand;
                    unsigned int p = m_selectedInstanceIndexed[i].getAllVertices().getPrimitiveType();

                    MaterialData material;
                    material.textureIndex = (m_selectedInstanceIndexed[i].getMaterial().getTexture() != nullptr) ? m_selectedInstanceIndexed[i].getMaterial().getTexture()->getId() : 0;
                    material.materialType = m_selectedInstanceIndexed[i].getMaterial().getType();
                    material.uvScale = (m_selectedInstanceIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_selectedInstanceIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_selectedInstanceIndexed[i].getMaterial().getTexture()->getSize().y()): math::Vec2f(0, 0);
                    material.uvOffset = math::Vec2f(0, 0);
                    materialDatas[p].push_back(material);
                    std::vector<TransformMatrix*> tm = m_selectedInstanceIndexed[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData model;
                        model.worldMat = toVulkanMatrix(tm[j]->getMatrix())/*.transpose()*/;
                        modelDatas[p].push_back(model);
                    }
                    unsigned int indexCount = 0, vertexCount = 0;
                    if (m_selectedInstanceIndexed[i].getVertexArrays().size() > 0) {
                        Entity* entity = m_selectedInstanceIndexed[i].getVertexArrays()[0]->getEntity();
                        for (unsigned int j = 0; j < m_selectedInstanceIndexed[i].getVertexArrays().size(); j++) {
                            if (entity == m_selectedInstanceIndexed[i].getVertexArrays()[j]->getEntity()) {

                                for (unsigned int k = 0; k < m_selectedInstanceIndexed[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                    vertexCount++;
                                    vbBindlessTex[p].append((*m_selectedInstanceIndexed[i].getVertexArrays()[j])[k]);
                                }
                                for (unsigned int k = 0; k < m_selectedInstanceIndexed[i].getVertexArrays()[j]->getIndexes().size(); k++) {
                                    indexCount++;
                                    vbBindlessTex[p].addIndex(m_selectedInstanceIndexed[i].getVertexArrays()[j]->getIndexes()[k]);
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
                    ////////std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                    ////////std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;
                }
            }
            RenderStates currentStates;
            currentStates.blendMode = BlendNone;
            currentStates.shader = &indirectRenderingShader;
            currentStates.texture = nullptr;
            frameBuffer.enableStencilTest(true);
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
                        modelDataShaderStorageBuffers.resize(frameBuffer.getMaxFramesInFlight());
                        modelDataShaderStorageBuffersMemory.resize(frameBuffer.getMaxFramesInFlight());
                        for (unsigned int i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {
                            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, modelDataShaderStorageBuffers[i], modelDataShaderStorageBuffersMemory[i]);
                        }
                        maxModelDataSize = bufferSize;
                        needToUpdateDS = true;
                    }


                    void* data;
                    vkMapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, modelDatas[p].data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory);
                    for (unsigned int i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {
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
                        materialDataShaderStorageBuffers.resize(frameBuffer.getMaxFramesInFlight());
                        materialDataShaderStorageBuffersMemory.resize(frameBuffer.getMaxFramesInFlight());
                        for (unsigned int i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {
                            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, materialDataShaderStorageBuffers[i], materialDataShaderStorageBuffersMemory[i]);
                        }
                        maxMaterialDataSize = bufferSize;
                        needToUpdateDS = true;
                    }

                    vkMapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, materialDatas[p].data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory);
                    for (unsigned int i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {
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
                    ////////std::cout<<"size : "<<sizeof(DrawElementsIndirectCommand)<<std::endl;
                    createCommandBuffersIndirect(p, drawElementsIndirectCommands[p].size(), sizeof(DrawElementsIndirectCommand), PPLLNODEPTHSTENCIL, currentStates);

                }
            }
            for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                vbBindlessTex[i].clear();
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
            for (unsigned int i = 0; i < materialDatas.size(); i++) {
                materialDatas[i].clear();
            }
            for (unsigned int i = 0; i < modelDatas.size(); i++) {
                modelDatas[i].clear();
            }
            for (unsigned int i = 0; i < drawElementsIndirectCommands.size(); i++) {
                drawElementsIndirectCommands[i].clear();
            }
            for (unsigned int i = 0; i < m_selectedScaleIndexed.size(); i++) {
                if (m_selectedScaleIndexed[i].getAllVertices().getVertexCount() > 0) {
                    DrawElementsIndirectCommand drawElementsIndirectCommand;
                    ////////std::cout<<"next frame draw normal"<<std::endl;
                    unsigned int p = m_selectedScaleIndexed[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = 0;
                    material.materialType = 0;
                    material.uvScale = math::Vec2f(0, 0);
                    material.uvOffset = math::Vec2f(0, 0);
                    materialDatas[p].push_back(material);

                    TransformMatrix tm;
                    ModelData model;
                    model.worldMat = toVulkanMatrix(tm.getMatrix())/*.transpose()*/;
                    modelDatas[p].push_back(model);

                    unsigned int indexCount = 0, vertexCount = 0;
                    for (unsigned int j = 0; j < m_selectedScaleIndexed[i].getAllVertices().getVertexCount(); j++) {
                        vertexCount++;
                        vbBindlessTex[p].append(m_selectedScaleIndexed[i].getAllVertices()[j]);
                    }
                    for (unsigned int j = 0; j < m_selectedScaleIndexed[i].getAllVertices().getIndexes().size(); j++) {
                        indexCount++;
                        vbBindlessTex[p].addIndex(m_selectedScaleIndexed[i].getAllVertices().getIndexes()[j]);
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
            for (unsigned int i = 0; i < m_selectedScaleInstanceIndexed.size(); i++) {

                if (m_selectedScaleInstanceIndexed[i].getAllVertices().getVertexCount() > 0) {
                    DrawElementsIndirectCommand drawElementsIndirectCommand;
                    unsigned int p = m_selectedScaleInstanceIndexed[i].getAllVertices().getPrimitiveType();

                    MaterialData material;
                    material.textureIndex = 0;
                    material.materialType = m_selectedScaleInstanceIndexed[i].getMaterial().getType();
                    material.uvScale = (m_selectedScaleInstanceIndexed[i].getMaterial().getTexture() != nullptr) ? math::Vec2f(1.f / m_selectedScaleInstanceIndexed[i].getMaterial().getTexture()->getSize().x(), 1.f / m_selectedScaleInstanceIndexed[i].getMaterial().getTexture()->getSize().y()): math::Vec2f(0, 0);
                    material.uvOffset = math::Vec2f(0, 0);
                    materialDatas[p].push_back(material);
                    std::vector<TransformMatrix*> tm = m_selectedScaleInstanceIndexed[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData model;
                        model.worldMat = toVulkanMatrix(tm[j]->getMatrix())/*.transpose()*/;
                        modelDatas[p].push_back(model);
                    }
                    unsigned int indexCount = 0, vertexCount = 0;
                    if (m_selectedScaleInstanceIndexed[i].getVertexArrays().size() > 0) {
                        Entity* entity = m_selectedScaleInstanceIndexed[i].getVertexArrays()[0]->getEntity();
                        for (unsigned int j = 0; j < m_selectedScaleInstanceIndexed[i].getVertexArrays().size(); j++) {
                            if (entity == m_selectedScaleInstanceIndexed[i].getVertexArrays()[j]->getEntity()) {
                                unsigned int p = m_selectedScaleInstanceIndexed[i].getVertexArrays()[j]->getPrimitiveType();
                                for (unsigned int k = 0; k < m_selectedScaleInstanceIndexed[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                    vertexCount++;
                                    vbBindlessTex[p].append((*m_selectedScaleInstanceIndexed[i].getVertexArrays()[j])[k]);
                                }
                                for (unsigned int k = 0; k < m_selectedScaleInstanceIndexed[i].getVertexArrays()[j]->getIndexes().size(); k++) {
                                    indexCount++;
                                    vbBindlessTex[p].addIndex(m_selectedScaleInstanceIndexed[i].getVertexArrays()[j]->getIndexes()[k]);
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
                    ////////std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                    ////////std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;
                }
            }
            currentStates.blendMode = BlendNone;
            currentStates.shader = &indirectRenderingShader;
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
                        modelDataShaderStorageBuffers.resize(frameBuffer.getMaxFramesInFlight());
                        modelDataShaderStorageBuffersMemory.resize(frameBuffer.getMaxFramesInFlight());
                        for (unsigned int i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {
                            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, modelDataShaderStorageBuffers[i], modelDataShaderStorageBuffersMemory[i]);
                        }
                        maxModelDataSize = bufferSize;
                    }


                    void* data;
                    vkMapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, modelDatas[p].data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), modelDataStagingBufferMemory);
                    for (unsigned int i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {
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
                        materialDataShaderStorageBuffers.resize(frameBuffer.getMaxFramesInFlight());
                        materialDataShaderStorageBuffersMemory.resize(frameBuffer.getMaxFramesInFlight());
                        for (unsigned int i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {
                            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, materialDataShaderStorageBuffers[i], materialDataShaderStorageBuffersMemory[i]);
                        }
                        maxMaterialDataSize = bufferSize;
                    }

                    vkMapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, materialDatas[p].data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), materialDataStagingBufferMemory);
                    for (unsigned int i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {
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
                    ////////std::cout<<"size : "<<sizeof(DrawElementsIndirectCommand)<<std::endl;
                    createCommandBuffersIndirect(p, drawElementsIndirectCommands[p].size(), sizeof(DrawElementsIndirectCommand), PPLLNODEPTHSTENCILOUTLINE, currentStates);

                }
            }
        }
        void PerPixelLinkedListRenderComponent::drawNextFrame() {
            {


                ////std::cout<<"next frame datasReady"<<datasReady<<std::endl;
                if (datasReady.load()) {

                    datasReady = false;
                    std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                    m_instances = batcher.getInstances();
                    m_normals = normalBatcher.getInstances();
                    m_instancesIndexed = batcherIndexed.getInstances();
                    m_normalsIndexed = normalBatcherIndexed.getInstances();
                    m_selected = selectedBatcher.getInstances();
                    m_selectedScale = selectedScaleBatcher.getInstances();
                    m_selectedIndexed = selectedIndexBatcher.getInstances();
                    m_selectedScaleIndexed = selectedIndexScaleBatcher.getInstances();
                    m_selectedInstance = selectedInstanceBatcher.getInstances();
                    m_selectedScaleInstance = selectedInstanceScaleBatcher.getInstances();
                    m_selectedInstanceIndexed = selectedInstanceIndexBatcher.getInstances();
                    m_selectedScaleInstanceIndexed = selectedInstanceIndexScaleBatcher.getInstances();
                    m_skyboxInstance = skyboxBatcher.getInstances();
                }
            }

            /*math::Matrix4f viewMatrix = view.getViewMatrix().getMatrix().transpose();
            math::Matrix4f projMatrix = view.getProjMatrix().getMatrix().transpose();
            viewMatrix = math::Matrix4f(math::Matrix3f(viewMatrix));


            vb.clear();
            //vb.name = "SKYBOXVB";
            for (unsigned int i = 0; i < m_skyboxInstance.size(); i++) {
                if (m_skyboxInstance[i].getAllVertices().getVertexCount() > 0) {
                    vb.setPrimitiveType(m_skyboxInstance[i].getAllVertices().getPrimitiveType());
                    for (unsigned int j = 0; j < m_skyboxInstance[i].getAllVertices().getVertexCount(); j++) {
                        //if (m_skyboxInstance[i].getAllVertices()[j].position.x != 0 && m_skyboxInstance[i].getAllVertices()[j].position.y != 0 && m_skyboxInstance[i].getAllVertices()[j].position.z != 0);
                        vb.append(m_skyboxInstance[i].getAllVertices()[j]);
                    }
                }
            }
            currentStates.blendMode = sf::BlendAlpha;
            currentStates.shader = &skyboxShader;
            currentStates.texture = (skybox == nullptr ) ? nullptr : &static_cast<g3d::Skybox*>(skybox)->getTexture();
            vb.update();
            frameBuffer.drawVertexBuffer(vb, currentStates);
            vb.clear();*/
            RenderStates currentStates;
            math::Matrix4f projMatrix = view.getProjMatrix().getMatrix()/*.transpose()*/;
            math::Matrix4f viewMatrix = view.getViewMatrix().getMatrix()/*.transpose()*/;
            indirectDrawPushConsts.projMatrix = toVulkanMatrix(projMatrix);
            indirectDrawPushConsts.viewMatrix = toVulkanMatrix(viewMatrix);
            //projMatrix.identity();
            skyboxPushConsts.projMatrix = toVulkanMatrix(projMatrix);
            skyboxPushConsts.viewMatrix = toVulkanMatrix(viewMatrix);
            //indirectDrawPushConsts.projMatrix.m22 *= -1;

            if (useThread) {
                std::unique_lock<std::mutex> lock(mtx);
                //std::unique_lock<std::mutex> lock2(mtx2);
                cv.wait(lock, [this](){return registerFrameJob[frameBuffer.getCurrentFrame()].load() || stop.load();});
                //std::cout<<"register frame"<<std::endl;
                registerFrameJob[frameBuffer.getCurrentFrame()] = false;
                resetBuffers();
                fillBuffersMT();
                fillIndexedBuffersMT();
                fillSelectedBuffersMT();
                fillSelectedIndexedBuffersMT();
                fillOutlineBuffersMT();
                fillOutlineIndexedBuffersMT();
                ////std::cout<<"buffer filled"<<std::endl;

                skyboxVB.clear();
                skyboxVB.name = "SKYBOXVB";
                for (unsigned int i = 0; i < m_skyboxInstance.size(); i++) {
                    if (m_skyboxInstance[i].getAllVertices().getVertexCount() > 0) {
                        skyboxVB.setPrimitiveType(m_skyboxInstance[i].getAllVertices().getPrimitiveType());
                        for (unsigned int j = 0; j < m_skyboxInstance[i].getAllVertices().getVertexCount(); j++) {
                            skyboxVB.append(m_skyboxInstance[i].getAllVertices()[j]);
                        }
                    }
                }
                if (skybox != nullptr) {

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
                    unsigned int currentFrame = frameBuffer.getCurrentFrame();
                    vkResetCommandBuffer(copySkyboxCommandBuffer[currentFrame], 0);
                    if (vkBeginCommandBuffer(copySkyboxCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                        throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                    }
                    //std::cout<<"copy skybox vb"<<std::endl;
                    skyboxVB.update(currentFrame, copySkyboxCommandBuffer[currentFrame]);
                    if (vkEndCommandBuffer(copySkyboxCommandBuffer[currentFrame]) != VK_SUCCESS) {
                        throw core::Erreur(0, "failed to record command buffer!", 1);
                    }
                }


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
                math::Matrix4f matrix = quad.getTransform().getMatrix();
                //////std::cout<<"world mat : "<<matrix<<std::endl;
                ppll2PushConsts.worldMat = toVulkanMatrix(matrix);

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
                unsigned int currentFrame = frameBuffer.getCurrentFrame();
                vkResetCommandBuffer(copyVbPpllPass2CommandBuffer[currentFrame], 0);
                if (vkBeginCommandBuffer(copyVbPpllPass2CommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }
                vb.update(currentFrame, copyVbPpllPass2CommandBuffer[currentFrame]);
                if (vkEndCommandBuffer(copyVbPpllPass2CommandBuffer[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }
                drawBuffers();

                commandBufferReady[frameBuffer.getCurrentFrame()] = true;
                cv.notify_one();
            } else {

                drawInstances();
                drawInstancesIndexed();
                drawSelectedInstances();
                drawSelectedInstancesIndexed();


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
                //////std::cout<<"world mat : "<<matrix<<std::endl;
                ppll2PushConsts.worldMat = toVulkanMatrix(matrix);
                //system("PAUSE");

                currentStates.shader = &perPixelLinkedListP2;
                currentStates.blendMode = BlendNone;
                //frameBuffer.enableStencilTest(false);
                //createDescriptorSets2(currentStates);
                createCommandBufferVertexBuffer(currentStates);
            }
        }
        void PerPixelLinkedListRenderComponent::allocateCommandBuffers() {
            /*commandBuffers.resize(frameBuffer.getSwapchainImages().size());


            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = commandPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
            allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();
            if (vkAllocateCommandBuffers(vkDevice.getDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to allocate command buffers!", 1);
            }*/
        }
        void PerPixelLinkedListRenderComponent::recordCommandBufferIndirect(unsigned int currentFrame, unsigned int p, unsigned int nbIndirectCommands, unsigned int stride, PPLLDepthStencilID depthStencilID, unsigned int vertexOffset, unsigned int indexOffset, unsigned int uboOffset, unsigned int modelDataOffset, unsigned int materialDataOffset, unsigned int drawCommandOffset, RenderStates currentStates, VkCommandBuffer commandBuffer) {
            //std::lock_guard<std::recursive_mutex> lock(rec_mutex);
            currentStates.blendMode.updateIds();
            Shader* shader = const_cast<Shader*>(currentStates.shader);

            vkCmdPushConstants(commandBuffer, frameBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(IndirectDrawPushConsts), &indirectDrawPushConsts);
            std::vector<unsigned int> dynamicBufferOffsets;
            dynamicBufferOffsets.push_back(modelDataOffset);
            dynamicBufferOffsets.push_back(materialDataOffset);

            //std::cout<<"nb commands : "<<nbIndirectCommands<<","<<drawCommandOffset<<std::endl;
            if (indexOffset == -1)
                frameBuffer.drawIndirect(commandBuffer, currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], drawCommandBufferMT[p][currentFrame], depthStencilID,currentStates, p, vertexOffset, drawCommandOffset, dynamicBufferOffsets);
            else
                frameBuffer.drawIndirect(commandBuffer, currentFrame, nbIndirectCommands, stride, vbBindlessTexIndexed[p], drawCommandBufferIndexedMT[p][currentFrame], depthStencilID,currentStates,p,  vertexOffset, drawCommandOffset, dynamicBufferOffsets, indexOffset);

        }
        void PerPixelLinkedListRenderComponent::recordCommandBufferVertexBuffer(unsigned int currentFrame, RenderStates currentStates, VkCommandBuffer commandBuffer) {
            //std::lock_guard<std::recursive_mutex> lock(rec_mutex);
            currentStates.blendMode.updateIds();


            Shader* shader = const_cast<Shader*>(currentStates.shader);
            if (shader == &perPixelLinkedListP2) {


                vkCmdPushConstants(commandBuffer, frameBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + vb.getPrimitiveType()][0][PPLLNODEPTHNOSTENCIL*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Ppll2PushConsts), &ppll2PushConsts);

                frameBuffer.drawVertexBuffer(commandBuffer, currentFrame, vb, PPLLNODEPTHNOSTENCIL, currentStates);
            } else if (shader == &skyboxShader) {
                vkCmdPushConstants(commandBuffer, frameBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + skyboxVB.getPrimitiveType()][0][PPLLNODEPTHNOSTENCIL*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(SkyboxPushConsts), &skyboxPushConsts);

                frameBuffer.drawVertexBuffer(commandBuffer, currentFrame, skyboxVB, PPLLNODEPTHNOSTENCIL, currentStates);
            }
        }
        void PerPixelLinkedListRenderComponent::drawBuffers() {
            unsigned int currentFrame = frameBuffer.getCurrentFrame();
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
            if (skybox != nullptr)
                skyboxVB.updateStagingBuffers(currentFrame);
            vb.updateStagingBuffers(currentFrame);


            VkCommandBufferInheritanceInfo inheritanceInfo{};

            VkCommandBufferBeginInfo beginInfo{};

            if (skybox != nullptr) {
                inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                inheritanceInfo.renderPass = frameBuffer.getRenderPass(1);
                inheritanceInfo.framebuffer = VK_NULL_HANDLE;
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.pInheritanceInfo = &inheritanceInfo;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
                vkResetCommandBuffer(skyboxCommandBuffer[currentFrame], 0);
                if (vkBeginCommandBuffer(skyboxCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }
                RenderStates currentStates;
                currentStates.blendMode = BlendNone;
                currentStates.shader = &skyboxShader;
                recordCommandBufferVertexBuffer(currentFrame, currentStates, skyboxCommandBuffer[currentFrame]);
                if (vkEndCommandBuffer(skyboxCommandBuffer[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }
            }
            inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
            inheritanceInfo.renderPass = frameBuffer.getRenderPass(1);
            inheritanceInfo.framebuffer = VK_NULL_HANDLE;
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.pInheritanceInfo = &inheritanceInfo;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
            vkResetCommandBuffer(ppllCommandBuffer[currentFrame], 0);
            if (vkBeginCommandBuffer(ppllCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                throw core::Erreur(0, "failed to begin recording command buffer!", 1);
            }


            RenderStates currentStates;
            currentStates.blendMode = BlendNone;
            currentStates.shader = &indirectRenderingShader;
            for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes-1; p++) {
                if (needToUpdateDSs[p][currentFrame])
                    updateDescriptorSets(currentFrame, p, currentStates);
                if (nbDrawCommandBuffer[p][0] > 0) {
                    recordCommandBufferIndirect(currentFrame, p, nbDrawCommandBuffer[p][0], sizeof(DrawArraysIndirectCommand), PPLLNODEPTHNOSTENCIL, 0, -1, -1, modelDataOffsets[p][0], materialDataOffsets[p][0],drawCommandBufferOffsets[p][0], currentStates, ppllCommandBuffer[currentFrame]);
                }
                if (nbIndexedDrawCommandBuffer[p][0] > 0) {
                    recordCommandBufferIndirect(currentFrame, p, nbIndexedDrawCommandBuffer[p][0], sizeof(DrawElementsIndirectCommand), PPLLNODEPTHNOSTENCIL, 0, 0, -1, modelDataOffsets[p][1], materialDataOffsets[p][1],drawIndexedCommandBufferOffsets[p][0], currentStates, ppllCommandBuffer[currentFrame]);
                }
            }
            if (vkEndCommandBuffer(ppllCommandBuffer[currentFrame]) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to record command buffer!", 1);
            }
            inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
            inheritanceInfo.renderPass = frameBuffer.getRenderPass(1);
            inheritanceInfo.framebuffer = VK_NULL_HANDLE;
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.pInheritanceInfo = &inheritanceInfo;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
            vkResetCommandBuffer(ppllSelectedCommandBuffer[currentFrame], 0);
            if (vkBeginCommandBuffer(ppllSelectedCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                throw core::Erreur(0, "failed to begin recording command buffer!", 1);
            }
            for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes-1; p++) {
                if (nbDrawCommandBuffer[p][1] > 0) {
                    recordCommandBufferIndirect(currentFrame, p, nbDrawCommandBuffer[p][1], sizeof(DrawArraysIndirectCommand), PPLLNODEPTHSTENCIL, 0, -1, -1, modelDataOffsets[p][2], materialDataOffsets[p][2],drawCommandBufferOffsets[p][1], currentStates, ppllSelectedCommandBuffer[currentFrame]);
                }
                if (nbIndexedDrawCommandBuffer[p][1] > 0) {
                    recordCommandBufferIndirect(currentFrame, p, nbIndexedDrawCommandBuffer[p][1], sizeof(DrawElementsIndirectCommand), PPLLNODEPTHSTENCIL, 0, 0, -1, modelDataOffsets[p][3], materialDataOffsets[p][3],drawIndexedCommandBufferOffsets[p][1], currentStates, ppllSelectedCommandBuffer[currentFrame]);
                }
            }
            if (vkEndCommandBuffer(ppllSelectedCommandBuffer[currentFrame]) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to record command buffer!", 1);
            }
            inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
            inheritanceInfo.renderPass = frameBuffer.getRenderPass(1);
            inheritanceInfo.framebuffer = VK_NULL_HANDLE;
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.pInheritanceInfo = &inheritanceInfo;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
            vkResetCommandBuffer(ppllOutlineCommandBuffer[currentFrame], 0);
            if (vkBeginCommandBuffer(ppllOutlineCommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {

                throw core::Erreur(0, "failed to begin recording command buffer!", 1);
            }
            for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes-1; p++) {
                if (nbDrawCommandBuffer[p][2] > 0) {
                    recordCommandBufferIndirect(currentFrame, p, nbDrawCommandBuffer[p][2], sizeof(DrawArraysIndirectCommand), PPLLNODEPTHSTENCILOUTLINE, 0, -1, -1, modelDataOffsets[p][4], materialDataOffsets[p][4],drawCommandBufferOffsets[p][2], currentStates, ppllOutlineCommandBuffer[currentFrame]);
                }
                if (nbIndexedDrawCommandBuffer[p][2] > 0) {
                    recordCommandBufferIndirect(currentFrame, p, nbIndexedDrawCommandBuffer[p][2], sizeof(DrawElementsIndirectCommand), PPLLNODEPTHSTENCILOUTLINE, 0, 0, -1, modelDataOffsets[p][5], materialDataOffsets[p][5],drawIndexedCommandBufferOffsets[p][2], currentStates, ppllOutlineCommandBuffer[currentFrame]);
                }
            }
            currentStates.blendMode = BlendAlpha;
            currentStates.shader = &perPixelLinkedListP2;
            if (vkEndCommandBuffer(ppllOutlineCommandBuffer[currentFrame]) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to record command buffer!", 1);
            }
            inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
            inheritanceInfo.renderPass = frameBuffer.getRenderPass(1);
            inheritanceInfo.framebuffer = VK_NULL_HANDLE;
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.pInheritanceInfo = &inheritanceInfo;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
            if (vkBeginCommandBuffer(ppllPass2CommandBuffer[currentFrame], &beginInfo) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to begin recording command buffer!", 1);
            }

            recordCommandBufferVertexBuffer(currentFrame, currentStates, ppllPass2CommandBuffer[currentFrame]);
            if (vkEndCommandBuffer(ppllPass2CommandBuffer[currentFrame]) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to record command buffer!", 1);
            }

            for (unsigned int p = 0; p < (Batcher::nbPrimitiveTypes - 1); p++)
                needToUpdateDSs[p][currentFrame] = false;
        }
        void PerPixelLinkedListRenderComponent::createCommandBuffersIndirect(unsigned int p, unsigned int nbIndirectCommands, unsigned int stride, PPLLDepthStencilID depthStencilID, RenderStates currentStates) {
            //////std::cout<<"draw indirect"<<std::endl;
            if (needToUpdateDS) {
                createDescriptorSets(p, currentStates);
                needToUpdateDS = false;
            }
            unsigned int currentFrame = frameBuffer.getCurrentFrame();
            currentStates.blendMode.updateIds();

            /*VkCommandBufferInheritanceInfo inheritanceInfo;
            inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
            inheritanceInfo.pNext = nullptr;
            inheritanceInfo.renderPass = frameBuffer.getRenderPass(1);
            inheritanceInfo.subpass = 0;
            inheritanceInfo.framebuffer = frameBuffer.getSwapchainFrameBuffers(1)[currentFrame];
            inheritanceInfo.occlusionQueryEnable = VK_FALSE;
            inheritanceInfo.queryFlags = 0;
            inheritanceInfo.pipelineStatistics = 0;
            VkCommandBufferBeginInfo commandBufferBeginInfo;
            commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            commandBufferBeginInfo.pNext = nullptr;
            commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
            commandBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;

            if (vkBeginCommandBuffer(commandBuffers[currentFrame], &commandBufferBeginInfo) != VK_SUCCESS) {
                std::runtime_error("Failed to begin recording command buffers");
            }*/
            frameBuffer.beginRecordCommandBuffers();
            Shader* shader = const_cast<Shader*>(currentStates.shader);
            std::vector<Texture*> allTextures = Texture::getAllTextures();

            vkCmdPushConstants(frameBuffer.getCommandBuffers()[currentFrame], frameBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][0][depthStencilID*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(IndirectDrawPushConsts), &indirectDrawPushConsts);

            frameBuffer.beginRenderPass();

            frameBuffer.drawIndirect(frameBuffer.getCommandBuffers()[currentFrame], currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], vboIndirect, depthStencilID,currentStates);
            frameBuffer.endRenderPass();
            std::vector<VkSemaphore> waitSemaphores, signalSemaphores;
            waitSemaphores.push_back(offscreenFinishedSemaphore[frameBuffer.getCurrentFrame()]);
            std::vector<VkPipelineStageFlags> waitStages;
            waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
            std::vector<uint64_t> waitValues, signalValues;
            waitValues.push_back(values[frameBuffer.getCurrentFrame()]);
            ////std::cout<<"wait value : "<<values[frameBuffer.getCurrentFrame()]<<std::endl;
            signalSemaphores.push_back(offscreenFinishedSemaphore[frameBuffer.getCurrentFrame()]);
            values[frameBuffer.getCurrentFrame()]++;
            signalValues.push_back(values[frameBuffer.getCurrentFrame()]);
            frameBuffer.submit(false, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);
            ////std::cout<<"signal value : "<<values[frameBuffer.getCurrentFrame()]<<std::endl;

            /*frameBuffer.beginRecordCommandBuffers();
            frameBuffer.beginRenderPass();
            vkCmdExecuteCommands(frameBuffer.getCommandBuffers()[currentFrame], static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());*/


        }
        void PerPixelLinkedListRenderComponent::createCommandBufferVertexBuffer(RenderStates currentStates) {

            frameBuffer.beginRecordCommandBuffers();
            currentStates.blendMode.updateIds();
            unsigned int currentFrame = frameBuffer.getCurrentFrame();

            Shader* shader = const_cast<Shader*>(currentStates.shader);

            vkCmdPipelineBarrier(frameBuffer.getCommandBuffers()[currentFrame], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);


                VkMemoryBarrier memoryBarrier;
                memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                memoryBarrier.pNext = VK_NULL_HANDLE;
                memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                //vkCmdWaitEvents(frameBuffer.getCommandBuffers()[currentFrame], 1, &events[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

                vkCmdPipelineBarrier(frameBuffer.getCommandBuffers()[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                //vkCmdPushDescriptorSetKHR(frameBuffer.getCommandBuffers()[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, frameBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + vb.getPrimitiveType()][frameBuffer.getId()][PPLLNODEPTHNOSTENCIL], 0, 2, descriptorWrites.data());

                frameBuffer.beginRenderPass();

                vkCmdPushConstants(frameBuffer.getCommandBuffers()[currentFrame], frameBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + vb.getPrimitiveType()][0][PPLLNODEPTHNOSTENCIL*currentStates.blendMode.nbBlendModes+currentStates.blendMode.id], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Ppll2PushConsts), &ppll2PushConsts);

                frameBuffer.drawVertexBuffer(frameBuffer.getCommandBuffers()[currentFrame], currentFrame, vb, PPLLNODEPTHNOSTENCIL, currentStates);


                /*if (vkEndCommandBuffer(commandBuffers[currentFrame]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to record command buffer!", 1);
                }*/

            //}
            /*if(vkEndCommandBuffer(commandBuffers[currentFrame]) != VK_SUCCESS) {
                std::runtime_error("Failed to record command buffers");
            }*/
            /*frameBuffer.beginRecordCommandBuffers();
            frameBuffer.beginRenderPass();
            vkCmdExecuteCommands(frameBuffer.getCommandBuffers()[currentFrame], static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());*/
            frameBuffer.endRenderPass();
            std::vector<VkSemaphore> waitSemaphores, signalSemaphores;
            waitSemaphores.push_back(offscreenFinishedSemaphore[frameBuffer.getCurrentFrame()]);
            std::vector<VkPipelineStageFlags> waitStages;
            waitStages.push_back(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
            std::vector<uint64_t> waitValues, signalValues;
            waitValues.push_back(values[frameBuffer.getCurrentFrame()]);
            signalSemaphores.push_back(offscreenFinishedSemaphore[frameBuffer.getCurrentFrame()]);
            ////std::cout<<"pass2 wait value : "<<values[frameBuffer.getCurrentFrame()]<<std::endl;
            values[frameBuffer.getCurrentFrame()]++;
            signalValues.push_back(values[frameBuffer.getCurrentFrame()]);
            frameBuffer.submit(true, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);
            ////std::cout<<"pass2 signal value : "<<values[frameBuffer.getCurrentFrame()]<<std::endl;

        }
        bool PerPixelLinkedListRenderComponent::loadEntitiesOnComponent(std::vector<Entity*> vEntities) {
            {

                datasReady = false;
                std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                batcher.clear();
                normalBatcher.clear();
                batcherIndexed.clear();
                normalBatcherIndexed.clear();
                selectedBatcher.clear();
                selectedScaleBatcher.clear();
                selectedIndexBatcher.clear();
                selectedIndexScaleBatcher.clear();
                selectedInstanceBatcher.clear();
                selectedInstanceScaleBatcher.clear();
                selectedInstanceIndexBatcher.clear();
                selectedInstanceIndexScaleBatcher.clear();
                skyboxBatcher.clear();
                visibleSelectedScaleEntities.clear();
            }
            if (skybox != nullptr) {
                for (unsigned int i = 0; i <  skybox->getNbFaces(); i++) {
                    skyboxBatcher.addFace(skybox->getFace(i));
                }
            }
            for (unsigned int i = 0; i < vEntities.size(); i++) {

                if ( vEntities[i] != nullptr && vEntities[i]->isLeaf()) {
                    Entity* border;
                    if (vEntities[i]->isSelected()) {
                        border = vEntities[i]->clone();
                        border->decreaseNbEntities();
                    }
                    for (unsigned int j = 0; j <  vEntities[i]->getNbFaces(); j++) {
                         std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                         if (vEntities[i]->getDrawMode() == Entity::INSTANCED && !vEntities[i]->isSelected()) {
                            if (vEntities[i]->getFace(j)->getVertexArray().getIndexes().size() == 0) {

                                batcher.addFace( vEntities[i]->getFace(j));
                            } else {

                                batcherIndexed.addFace(vEntities[i]->getFace(j));
                            }
                         } else if (vEntities[i]->getDrawMode() == Entity::NORMAL && !vEntities[i]->isSelected()) {
                             if (vEntities[i]->getFace(j)->getVertexArray().getIndexes().size() == 0) {
                                normalBatcher.addFace( vEntities[i]->getFace(j));
                             } else {
                                ////std::cout<<"add face"<<std::endl;
                                normalBatcherIndexed.addFace( vEntities[i]->getFace(j));
                             }
                        } else if (vEntities[i]->getDrawMode() == Entity::INSTANCED && vEntities[i]->isSelected()) {

                            if (vEntities[i]->getFace(j)->getVertexArray().getIndexes().size() == 0) {
                                selectedInstanceBatcher.addFace(vEntities[i]->getFace(j));
                           // //////std::cout<<"remove texture"<<std::endl;

                            ////////std::cout<<"get va"<<std::endl;
                                VertexArray& va = border->getFace(j)->getVertexArray();
                                ////////std::cout<<"change color"<<std::endl;
                                for (unsigned int j = 0; j < va.getVertexCount(); j++) {

                                    va[j].color = Color::Cyan;
                                }
                                Entity* root = (vEntities[i]->getRootEntity()->isAnimated()) ? vEntities[i]->getRootEntity() : vEntities[i];
                                math::Vec3f oldSize = root->getSize();
                                border->setOrigin(root->getSize() * 0.5f);
                                border->setSize(root->getSize() * 1.1f);
                                math::Vec3f offset =  root->getSize() - oldSize;
                                if (border->getSize().z() > 0) {
                                    border->setPosition(root->getPosition() - offset * 0.5f);
                                }
                               // //////std::cout<<"add to batcher"<<std::endl;
                                selectedInstanceScaleBatcher.addFace(border->getFace(j));
                           // //////std::cout<<"face added"<<std::endl;
                             } else {
                                 selectedInstanceIndexBatcher.addFace(vEntities[i]->getFace(j));
                               // //////std::cout<<"remove texture"<<std::endl;

                            ////////std::cout<<"get va"<<std::endl;
                                VertexArray& va = border->getFace(j)->getVertexArray();
                                ////////std::cout<<"change color"<<std::endl;
                                for (unsigned int j = 0; j < va.getVertexCount(); j++) {

                                    va[j].color = Color::Cyan;
                                }
                                Entity* root = (vEntities[i]->getRootEntity()->isAnimated()) ? vEntities[i]->getRootEntity() : vEntities[i];
                                math::Vec3f oldSize = root->getSize();
                                border->setOrigin(root->getSize() * 0.5f);
                                border->setSize(root->getSize() * 1.1f);
                                math::Vec3f offset =  root->getSize() - oldSize;
                                if (border->getSize().z() > 0) {
                                    border->setPosition(root->getPosition() - offset * 0.5f);
                                }
                               // //////std::cout<<"add to batcher"<<std::endl;

                               // //////std::cout<<"add to batcher"<<std::endl;
                                selectedInstanceIndexScaleBatcher.addFace(border->getFace(j));
                             }
                        } else {
                            if (vEntities[i]->getFace(j)->getVertexArray().getIndexes().size() == 0) {

                                selectedBatcher.addFace(vEntities[i]->getFace(j));
                           // //////std::cout<<"remove texture"<<std::endl;

                            ////////std::cout<<"get va"<<std::endl;
                                VertexArray& va = border->getFace(j)->getVertexArray();
                                ////////std::cout<<"change color"<<std::endl;
                                for (unsigned int j = 0; j < va.getVertexCount(); j++) {

                                    va[j].color = Color::Cyan;
                                }
                                Entity* root = (vEntities[i]->getRootEntity()->isAnimated()) ? vEntities[i]->getRootEntity() : vEntities[i];
                                math::Vec3f oldSize = root->getSize();
                                border->setOrigin(root->getSize() * 0.5f);
                                border->setSize(root->getSize() * 1.1f);
                                math::Vec3f offset =  root->getSize() - oldSize;
                                if (border->getSize().z() > 0) {
                                    border->setPosition(root->getPosition() - offset * 0.5f);
                                }
                                selectedScaleBatcher.addFace(border->getFace(j));

                                ////std::cout<<"face added"<<std::endl;
                             } else {
                                 selectedIndexBatcher.addFace(vEntities[i]->getFace(j));
                               // //////std::cout<<"remove texture"<<std::endl;

                            ////////std::cout<<"get va"<<std::endl;
                                VertexArray& va = border->getFace(j)->getVertexArray();
                                ////////std::cout<<"change color"<<std::endl;
                                for (unsigned int j = 0; j < va.getVertexCount(); j++) {

                                    va[j].color = Color::Cyan;
                                }

                                Entity* root = (vEntities[i]->getRootEntity()->isAnimated()) ? vEntities[i]->getRootEntity() : vEntities[i];
                                math::Vec3f oldSize = root->getSize();
                                border->setOrigin(root->getSize() * 0.5f);
                                border->setSize(root->getSize() * 1.1f);
                                math::Vec3f offset =  root->getSize() - oldSize;
                                if (border->getSize().z() > 0) {
                                    border->setPosition(root->getPosition() - offset * 0.5f);
                                }
                                ////std::cout<<"add to batcher"<<std::endl;
                                selectedIndexScaleBatcher.addFace(border->getFace(j));
                             }
                        }
                    }
                    if (vEntities[i]->isSelected()) {
                        std::unique_ptr<Entity> ptr;
                        ptr.reset(border);
                        visibleSelectedScaleEntities.push_back(std::move(ptr));
                    }
                }

            }

            ////////std::cout<<"instances added"<<std::endl;

            std::lock_guard<std::mutex> lock(mtx);
            visibleEntities = vEntities;
            datasReady = true;
            cv.notify_one();
            //std::cout<<"load entities data ready : "<<datasReady<<std::endl;
            return true;
        }
        bool PerPixelLinkedListRenderComponent::needToUpdate() {
            return update;
        }
        void PerPixelLinkedListRenderComponent::setExpression (std::string expression) {
            this->expression = expression;
        }
        std::string PerPixelLinkedListRenderComponent::getExpression() {
            return expression;
        }
        void PerPixelLinkedListRenderComponent::pushEvent(window::IEvent event, RenderWindow& rw) {

            if (event.type == window::IEvent::WINDOW_EVENT && event.window.type == window::IEvent::WINDOW_EVENT_RESIZED && &getWindow() == &rw && isAutoResized()) {
                //////std::cout<<"recompute size"<<std::endl;
                recomputeSize();
                getListener().pushEvent(event);
                getView().reset(physic::BoundingBox(getView().getViewport().getPosition().x(), getView().getViewport().getPosition().y(), getView().getViewport().getPosition().z(), event.window.data1, event.window.data2, getView().getViewport().getDepth()));
            }
            if (&rw == &getWindow() && event.type == window::IEvent::WINDOW_EVENT && event.window.type == window::IEvent::WINDOW_EVENT_CLOSED) {
                stop = true;
                cv.notify_all();
                cv2.notify_all();
                getListener().stop();
            }
        }
        void PerPixelLinkedListRenderComponent::setView(View view) {
            frameBuffer.setView(view);
            this->view = view;
        }
        View& PerPixelLinkedListRenderComponent::getView() {
            return view;
        }
        void PerPixelLinkedListRenderComponent::draw(RenderTarget& target, RenderStates states) {
            if (useThread) {
                std::unique_lock<std::mutex> lock(mtx);
                std::unique_lock<std::mutex> lock2(mtx2);
                cv.wait(lock, [this] { return commandBufferReady[frameBuffer.getCurrentFrame()].load() || stop.load(); });
                commandBufferReady[frameBuffer.getCurrentFrame()] = false;
                //std::cout<<"copy"<<std::endl;


                frameBuffer.beginRecordCommandBuffers();
                std::vector<VkCommandBuffer> commandBuffers = frameBuffer.getCommandBuffers();
                unsigned int currentFrame = frameBuffer.getCurrentFrame();
                vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &copyModelDataBufferCommandBuffer[currentFrame]);

                vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &copyMaterialDataBufferCommandBuffer[currentFrame]);

                vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &copyDrawBufferCommandBuffer[currentFrame]);
                vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &copyVbBufferCommandBuffer[currentFrame]);

                vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &copyDrawIndexedBufferCommandBuffer[currentFrame]);
                vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &copyVbIndexedBufferCommandBuffer[currentFrame]);
                vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &copyVbPpllPass2CommandBuffer[currentFrame]);
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
                    bufferMemoryBarrier.buffer = skyboxVB.getVertexBuffer(currentFrame);
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
                    buffersMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                    buffersMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    buffersMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    buffersMemoryBarrier.offset = 0;
                    buffersMemoryBarrier.size = VK_WHOLE_SIZE;
                    if (vbBindlessTex[p].getVertexBuffer(currentFrame) != nullptr) {
                        buffersMemoryBarrier.buffer = vbBindlessTex[p].getVertexBuffer(currentFrame);
                        vkCmdPipelineBarrier(
                        commandBuffers[currentFrame],
                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                        VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
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
                        VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
                        0,
                        0, nullptr,
                        1, &buffersMemoryBarrier,
                        0, nullptr
                        );
                        buffersMemoryBarrier.buffer = vbBindlessTexIndexed[p].getIndexBuffer(currentFrame);
                        vkCmdPipelineBarrier(
                        commandBuffers[currentFrame],
                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                        VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
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
                signalSemaphores.push_back(offscreenFinishedSemaphore[currentFrame]);
                std::vector<VkSemaphore> waitSemaphores;
                waitSemaphores.push_back(offscreenFinishedSemaphore[currentFrame]);
                std::vector<VkPipelineStageFlags> waitStages;
                waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                std::vector<uint64_t> signalValues;
                std::vector<uint64_t> waitValues;
                waitValues.push_back(values[currentFrame]);
                values[currentFrame]++;
                signalValues.push_back(values[currentFrame]);

                frameBuffer.submit(false, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);
                //std::cout<<"skybox"<<std::endl;
                if (skybox != nullptr) {
                    frameBuffer.beginRecordCommandBuffers();
                    frameBuffer.beginRenderPass();
                    vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &skyboxCommandBuffer[currentFrame]);
                    frameBuffer.endRenderPass();
                    signalSemaphores.clear();
                    signalSemaphores.push_back(offscreenFinishedSemaphore[currentFrame]);
                    signalValues.clear();
                    waitSemaphores.clear();
                    waitSemaphores.push_back(offscreenFinishedSemaphore[currentFrame]);
                    waitStages.clear();
                    waitStages.push_back(VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
                    waitValues.clear();
                    waitValues.push_back(values[currentFrame]);
                    values[currentFrame]++;
                    signalValues.push_back(values[currentFrame]);
                    frameBuffer.submit(false, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);
                }
                RenderStates states;
                states.shader = &indirectRenderingShader;


                //std::cout<<"copies ok"<<std::endl;
                frameBuffer.beginRecordCommandBuffers();
                frameBuffer.beginRenderPass();
                vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &ppllCommandBuffer[currentFrame]);
                vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &ppllSelectedCommandBuffer[currentFrame]);
                frameBuffer.endRenderPass();
                signalSemaphores.clear();
                signalSemaphores.push_back(offscreenFinishedSemaphore[currentFrame]);
                signalValues.clear();
                waitSemaphores.clear();
                waitSemaphores.push_back(offscreenFinishedSemaphore[currentFrame]);
                waitStages.clear();
                waitStages.push_back(VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
                waitValues.clear();
                waitValues.push_back(values[currentFrame]);
                values[currentFrame]++;
                signalValues.push_back(values[currentFrame]);
                std::vector<VkFence> fencesToWait;
                //std::cout<<"compute"<<std::endl;

                if (visibleEntities.size() > computeSemaphores.size()) {
                    computeSemaphores.resize(visibleEntities.size());
                    computeFences.resize(visibleEntities.size());
                    for (unsigned int i = 0; i < visibleEntities.size(); i++) {
                        for (unsigned int j = 0; j < MAX_FRAMES_IN_FLIGHT; j++) {
                            VkSemaphoreCreateInfo semaphoreInfo{};
                            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                            VkFenceCreateInfo fenceInfo{};
                            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

                            if (computeFences[i][j] == VK_NULL_HANDLE) {
                                if (vkCreateFence(vkDevice.getDevice(), &fenceInfo, nullptr, &computeFences[i][j]) != VK_SUCCESS) {
                                    throw core::Erreur(0, "échec de la création des objets de synchronisation pour une entité!", 1);
                                }
                            }
                            if(computeSemaphores[i][j] == VK_NULL_HANDLE) {
                                if(vkCreateSemaphore(vkDevice.getDevice(), &semaphoreInfo, nullptr, &computeSemaphores[i][j]) != VK_SUCCESS) {
                                    throw core::Erreur(0, "échec de la création des objets de synchronisation pour une entité!", 1);
                                }
                            }
                        }
                    }
                }
                for (unsigned int i = 0; i < visibleEntities.size(); i++) {
                    if (visibleEntities[i] != nullptr && (visibleEntities[i]->getType() == "E_PARTICLES"
                        || visibleEntities[i]->getType() == "E_BONE_ANIMATION")
                        && visibleEntities[i]->isComputeFinished(frameBuffer.getCurrentFrame())) {

                        visibleEntities[i]->computeParticles(&mtx2, &cv2, vbBindlessTex[Triangles], frameBuffer.getCurrentFrame(),computeSemaphores[i][frameBuffer.getCurrentFrame()], computeFences[i][frameBuffer.getCurrentFrame()]);



                        //std::cout<<"wait : "<<visibleEntities[i]<<" current frame : "<<frameBuffer.getCurrentFrame()<<std::endl;
                        cv2.wait(lock2, [&, this](){return visibleEntities[i]->isComputeFinished(frameBuffer.getCurrentFrame()) || stop.load();});
                        //std::cout<<"wait finished"<<std::endl;
                        waitSemaphores.push_back(computeSemaphores[i][frameBuffer.getCurrentFrame()]);


                        waitValues.push_back(0);
                        waitStages.push_back(VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
                        //if (!firstFrame[frameBuffer.getCurrentFrame()]) {
                            if (vbBindlessTex[Triangles].getVertexBuffer(currentFrame) != nullptr) {
                                bufferMemoryBarrier.buffer = vbBindlessTex[Triangles].getVertexBuffer(currentFrame);
                                bufferMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                                bufferMemoryBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
                                vkCmdPipelineBarrier(
                                commandBuffers[currentFrame],
                                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                                0,
                                0, nullptr,
                                1, &bufferMemoryBarrier,
                                0, nullptr
                                );
                            }
                            firstFrame[frameBuffer.getCurrentFrame()] = false;
                            fencesToWait.push_back(computeFences[i][currentFrame]);
                        //}
                    }



                    //std::cout<<"wait for fence"<<std::endl;

                    //std::cout<<"fence reseted"<<std::endl;
                }


                frameBuffer.submit(false, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues, fencesToWait);
                //std::cout<<"ppll ok"<<std::endl;

                //std::cout<<"draw"<<std::endl;
                frameBuffer.beginRecordCommandBuffers();
                frameBuffer.beginRenderPass();
                vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &ppllOutlineCommandBuffer[currentFrame]);
                frameBuffer.endRenderPass();
                signalSemaphores.clear();
                signalSemaphores.push_back(offscreenFinishedSemaphore[currentFrame]);
                signalValues.clear();
                waitSemaphores.clear();
                waitSemaphores.push_back(offscreenFinishedSemaphore[currentFrame]);

                waitStages.clear();
                waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                waitValues.clear();
                waitValues.push_back(values[currentFrame]);



                values[currentFrame]++;
                signalValues.push_back(values[currentFrame]);
                frameBuffer.submit(false, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);
                //std::cout<<"outline ok"<<std::endl;

                frameBuffer.beginRecordCommandBuffers();
                vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);
                VkMemoryBarrier memoryBarrier{};
                memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                memoryBarrier.pNext = VK_NULL_HANDLE;
                memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                frameBuffer.beginRenderPass();
                vkCmdExecuteCommands(commandBuffers[currentFrame], 1, &ppllPass2CommandBuffer[currentFrame]);
                frameBuffer.endRenderPass();
                signalSemaphores.clear();
                signalSemaphores.push_back(offscreenFinishedSemaphore[currentFrame]);
                signalValues.clear();
                waitSemaphores.clear();
                waitSemaphores.push_back(offscreenFinishedSemaphore[currentFrame]);
                waitStages.clear();
                waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                waitValues.clear();
                waitValues.push_back(values[currentFrame]);
                values[currentFrame]++;
                signalValues.push_back(values[currentFrame]);
                frameBuffer.submit(true, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);

                //std::cout<<"second pass ok"<<std::endl;
            }
            target.beginRecordCommandBuffers();
            const_cast<Texture&>(frameBuffer.getTexture(frameBuffer.getImageIndex())).toShaderReadOnlyOptimal(target.getCommandBuffers()[target.getCurrentFrame()]);
            frameBufferSprite.setCenter(target.getView().getPosition());
            frameBufferSprite.setTexture(frameBuffer.getTexture(frameBuffer.getImageIndex()));
            ////////std::cout<<"view position : "<<view.getPosition()<<std::endl;
            ////////std::cout<<"sprite position : "<<frameBufferSprite.getCenter()<<std::endl;
            /*if (&target == &window)
                window.beginRenderPass();*/
            states.blendMode = BlendAlpha;
            states.blendMode.updateIds();
            //////std::cout<<"blend mode : "<<states.blendMode.colorSrcFactor<<std::endl;

            target.draw(frameBufferSprite, states);
            /*if (&target == &window)
                window.endRenderPass();*/
            std::vector<VkSemaphore> waitSemaphores, signalSemaphores;
            std::vector<VkPipelineStageFlags> waitStages;
            std::vector<uint64_t> waitValues, signalValues;
            waitSemaphores.push_back(offscreenFinishedSemaphore[frameBuffer.getCurrentFrame()]);
            waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
            signalSemaphores.push_back(offscreenFinishedSemaphore[frameBuffer.getCurrentFrame()]);
            waitValues.push_back(values[frameBuffer.getCurrentFrame()]);
            values[frameBuffer.getCurrentFrame()]++;
            signalValues.push_back(values[frameBuffer.getCurrentFrame()]);
            target.submit(false, signalSemaphores, waitSemaphores, waitStages, signalValues, waitValues);
            frameBuffer.display();
            //std::cout<<"next frame"<<std::endl;
            registerFrameJob[frameBuffer.getCurrentFrame()] = true;
            cv.notify_one();


            //std::cout<<"submit ok"<<std::endl;
        }
        std::vector<Entity*> PerPixelLinkedListRenderComponent::getEntities() {
            return visibleEntities;
        }
        void PerPixelLinkedListRenderComponent::createCommandPool() {
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
        int PerPixelLinkedListRenderComponent::getLayer() {
            return getPosition().z();
        }
        PerPixelLinkedListRenderComponent::~PerPixelLinkedListRenderComponent() {
            //////std::cout<<"ppll destructor"<<std::endl;
            for (unsigned int i = 0; i < events.size(); i++) {
                vkDestroyEvent(vkDevice.getDevice(), events[i], nullptr);
            }
            vkDestroyCommandPool(vkDevice.getDevice(), commandPool, nullptr);
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                vkDestroySampler(vkDevice.getDevice(), headPtrTextureSampler[i], nullptr);
                vkDestroyImageView(vkDevice.getDevice(), headPtrTextureImageView[i], nullptr);
                vkDestroyImage(vkDevice.getDevice(), headPtrTextureImage[i], nullptr);
                vkFreeMemory(vkDevice.getDevice(), headPtrTextureImageMemory[i], nullptr);
            }

            //////std::cout<<"image destroyed"<<std::endl;
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
            for (unsigned int i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {
                vkDestroySemaphore(vkDevice.getDevice(), offscreenFinishedSemaphore[i], nullptr);
            }
            for (unsigned int i = 0; i < computeSemaphores.size(); i++) {
                for (unsigned int j = 0; j < MAX_FRAMES_IN_FLIGHT; j++)
                    vkDestroySemaphore(vkDevice.getDevice(), computeSemaphores[i][j], nullptr);
            }
            for (unsigned int i = 0; i < computeFences.size(); i++) {
                for (unsigned int j = 0; j < MAX_FRAMES_IN_FLIGHT; j++)
                    vkDestroyFence(vkDevice.getDevice(), computeFences[i][j], nullptr);
            }
            vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, copyModelDataBufferCommandBuffer.size(), copyModelDataBufferCommandBuffer.data());

            vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, copyMaterialDataBufferCommandBuffer.size(), copyMaterialDataBufferCommandBuffer.data());

            vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, copyDrawBufferCommandBuffer.size(), copyDrawBufferCommandBuffer.data());

            vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, copyDrawIndexedBufferCommandBuffer.size(), copyDrawIndexedBufferCommandBuffer.data());
            vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, copyVbBufferCommandBuffer.size(), copyVbBufferCommandBuffer.data());
            vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, copyVbIndexedBufferCommandBuffer.size(), copyVbIndexedBufferCommandBuffer.data());
            vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, copyVbPpllPass2CommandBuffer.size(), copyVbPpllPass2CommandBuffer.data());
            vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, ppllCommandBuffer.size(), ppllCommandBuffer.data());
            vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, ppllSelectedCommandBuffer.size(), ppllSelectedCommandBuffer.data());
            vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, ppllOutlineCommandBuffer.size(), ppllOutlineCommandBuffer.data());
            vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, ppllPass2CommandBuffer.size(), ppllPass2CommandBuffer.data());
            vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, skyboxCommandBuffer.size(), skyboxCommandBuffer.data());
            vkFreeCommandBuffers(vkDevice.getDevice(), secondaryBufferCommandPool, copySkyboxCommandBuffer.size(), copySkyboxCommandBuffer.data());
            vkDestroyCommandPool(vkDevice.getDevice(), secondaryBufferCommandPool, nullptr);
            //////std::cout<<"indirect vbo destroyed"<<std::endl;
        }
        #else
        PerPixelLinkedListRenderComponent::PerPixelLinkedListRenderComponent(RenderWindow& window, int layer, std::string expression, window::ContextSettings settings) :
            HeavyComponent(window, math::Vec3f(window.getView().getPosition().x(), window.getView().getPosition().y(), layer),
                          math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0),
                          math::Vec3f(window.getView().getSize().x() + window.getView().getSize().x() * 0.5f, window.getView().getPosition().y() + window.getView().getSize().y() * 0.5f, layer)),
            view(window.getView()),
            expression(expression),
            quad(math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0)),
            layer(layer) {
            maxModelDataSize = maxMaterialDataSize = maxVboIndirectSize = 0;
            if (!(settings.versionMajor >= 4 && settings.versionMinor >= 6))
                throw core::Erreur(53, "opengl version not supported for this renderer type");
            ////////std::cout<<"move quad"<<std::endl;
            datasReady = false;
            quad.move(math::Vec3f(-window.getView().getSize().x() * 0.5f, -window.getView().getSize().y() * 0.5f, 0));
            maxNodes = 20 * window.getView().getSize().x() * window.getView().getSize().y();
            GLint nodeSize = 5 * sizeof(GLfloat) + sizeof(GLuint);
            ////////std::cout<<"stencil bits : "<<settings.stencilBits<<std::endl;

            frameBuffer.create(window.getView().getSize().x(), window.getView().getSize().y(), settings);
            frameBufferSprite = Sprite(frameBuffer.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0), IntRect(0, 0, window.getView().getSize().x(), window.getView().getSize().y()));
            frameBuffer.setView(view);
            resolution = math::Vec3f((int) window.getSize().x(), (int) window.getSize().y(), window.getView().getSize().z());
            //window.setActive();
            /*glCheck(glGenBuffers(1, &atomicBuffer));
            glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicBuffer));
            glCheck(glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW));
            glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0));
            glCheck(glGenBuffers(1, &linkedListBuffer));
            glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, linkedListBuffer));
            glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, maxNodes * nodeSize, nullptr, GL_DYNAMIC_DRAW));
            glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
            glCheck(glGenTextures(1, &headPtrTex));
            glCheck(glBindTexture(GL_TEXTURE_2D, headPtrTex));
            glCheck(glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, window.getView().getSize().x(), window.getView().getSize().y()));
            glCheck(glBindImageTexture(0, headPtrTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI));
            glCheck(glBindTexture(GL_TEXTURE_2D, 0));
            std::vector<GLuint> headPtrClearBuf(window.getView().getSize().x()*window.getView().getSize().y(), 0xffffffff);
            glCheck(glGenBuffers(1, &clearBuf));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf));
            glCheck(glBufferData(GL_PIXEL_UNPACK_BUFFER, headPtrClearBuf.size() * sizeof(GLuint),
            &headPtrClearBuf[0], GL_STATIC_COPY));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));*/
            ////////std::cout<<"buffers : "<<atomicBuffer<<" "<<linkedListBuffer<<" "<<headPtrTex<<" "<<clearBuf<<std::endl;
            core::FastDelegate<bool> signal (&PerPixelLinkedListRenderComponent::needToUpdate, this);
            core::FastDelegate<void> slot (&PerPixelLinkedListRenderComponent::drawNextFrame, this);
            core::Command cmd(signal, slot);
            getListener().connect("UPDATE", cmd);


            /*glCheck(glGenBuffers(1, &modelDataBuffer));
            glCheck(glGenBuffers(1, &materialDataBuffer));
            glCheck(glGenBuffers(1, &vboIndirect));*/
            compileShaders();

           /* std::vector<Texture*> allTextures = Texture::getAllTextures();
            Samplers allSamplers{};
            std::vector<math::Matrix4f> textureMatrices;
            for (unsigned int i = 0; i < allTextures.size(); i++) {
                textureMatrices.push_back(allTextures[i]->getTextureMatrix());
                GLuint64 handle_texture = allTextures[i]->getTextureHandle();
                allTextures[i]->makeTextureResident(handle_texture);
                allSamplers.tex[i].handle = handle_texture;
                ////////std::cout<<"add texture i : "<<i<<" id : "<<allTextures[i]->getId()<<std::endl;
            }
            indirectRenderingShader.setParameter("textureMatrix", textureMatrices);
            glCheck(glGenBuffers(1, &ubo));

            backgroundColor = Color::Transparent;
            glCheck(glBindBuffer(GL_UNIFORM_BUFFER, ubo));
            glCheck(glBufferData(GL_UNIFORM_BUFFER, sizeof(Samplers),allSamplers.tex, GL_STATIC_DRAW));
            glCheck(glBindBuffer(GL_UNIFORM_BUFFER, 0));
            ////////std::cout<<"size : "<<sizeof(Samplers)<<" "<<alignof (alignas(16) uint64_t[200])<<std::endl;
            backgroundColor = Color::Transparent;
            glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
            glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicBuffer));
            glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, linkedListBuffer));
            glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, modelDataBuffer));
            glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, materialDataBuffer));

            for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                vbBindlessTex[i].setPrimitiveType(static_cast<PrimitiveType>(i));
            }
            skybox = nullptr;
            frameBuffer.setActive(false);*/
            renderFinished = false;
            cleared = true;
            //getListener().launch();
        }
        void PerPixelLinkedListRenderComponent::loadTextureIndexes() {
            compileShaders();
            std::vector<Texture*> allTextures = Texture::getAllTextures();
            Samplers allSamplers{};
            std::vector<math::Matrix4f> textureMatrices;
            for (unsigned int i = 0; i < allTextures.size(); i++) {
                textureMatrices.push_back(allTextures[i]->getTextureMatrix());
                GLuint64 handle_texture = allTextures[i]->getTextureHandle();
                allTextures[i]->makeTextureResident(handle_texture);
                allSamplers.tex[i].handle = handle_texture;
                ////////std::cout<<"add texture i : "<<i<<" id : "<<allTextures[i]->getId()<<std::endl;
            }
            indirectRenderingShader.setParameter("textureMatrix", textureMatrices);
            glCheck(glBindBuffer(GL_UNIFORM_BUFFER, ubo));
            glCheck(glBufferData(GL_UNIFORM_BUFFER, sizeof(Samplers),allSamplers.tex, GL_STATIC_DRAW));
            glCheck(glBindBuffer(GL_UNIFORM_BUFFER, 0));
        }
        void PerPixelLinkedListRenderComponent::compileShaders() {
               const std::string skyboxVertexShader = R"(#version 460
                                                         layout (location = 0) in vec3 aPos;
                                                         out vec3 texCoords;
                                                         uniform mat4 projection;
                                                         uniform mat4 view;
                                                         void main()
                                                         {
                                                             texCoords = aPos;
                                                             gl_Position = projection * view * vec4(aPos, 1.0);
                                                         }
                                                         )";
               const std::string indirectDrawVertexShader = R"(#version 460
                                                            #define M_PI 3.1415926535897932384626433832795
                                                            #define FPI M_PI/4
                                                            layout (location = 0) in vec3 position;
                                                            layout (location = 1) in vec4 color;
                                                            layout (location = 2) in vec2 texCoords;
                                                            layout (location = 3) in vec3 normals;
                                                            layout (location = 4) in ivec4 boneIds;
                                                            layout (location = 5) in vec4 weights;
                                                            uniform mat4 projectionMatrix;
                                                            uniform mat4 viewMatrix;
                                                            uniform mat4 textureMatrix[)"+core::conversionUIntString(Texture::getAllTextures().size())+R"(];
                                                            uniform float time;
                                                            uniform vec3 resolution;
                                                            const int MAX_BONES = 100;
                                                            const int MAX_BONE_INFLUENCE = 4;
                                                            struct ModelData {
                                                                mat4 modelMatrix;
                                                                mat4 finalBonesMatrices[MAX_BONES];
                                                            };
                                                            struct MaterialData {
                                                                uint textureIndex;
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
                                                            void main() {
                                                                MaterialData materialData = materialDatas[gl_DrawID];
                                                                ModelData modelData = modelDatas[gl_BaseInstance + gl_InstanceID];
                                                                mat4 finalBonesMatrices[MAX_BONES] = modelData.finalBonesMatrices;
                                                                float xOff = 0;
                                                                float yOff = 0;
                                                                if (materialData.materialType == 1) {
                                                                    yOff = 0.05*sin(position.x*12+time*FPI)*resolution.y;
                                                                    xOff = 0.025*cos(position.x*12+time*FPI)*resolution.x;
                                                                }
                                                                vec4 totalPosition = vec4(0.0f);
                                                                for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
                                                                {
                                                                    if(boneIds[i] == -1)
                                                                        continue;
                                                                    if(boneIds[i] >=MAX_BONES)
                                                                    {
                                                                        totalPosition = vec4(position,1.0f);
                                                                        break;
                                                                    }
                                                                    vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(position,1.0f);
                                                                    totalPosition += localPosition * weights[i];

                                                                }
                                                                uint textureIndex =  materialData.textureIndex;
                                                                gl_Position = projectionMatrix * viewMatrix * modelData.modelMatrix * vec4((position.x - xOff), (position.y + yOff), position.z, 1.f) + totalPosition;
                                                                fTexCoords = (textureIndex != 0) ? (textureMatrix[textureIndex-1] * vec4(texCoords, 1.f, 1.f)).xy : texCoords;
                                                                frontColor = color;
                                                                texIndex = textureIndex;
                                                            }
                                                            )";
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
                const std::string skyboxFragmentShader = R"(#version 460
                                                            layout (location = 0) out vec4 fcolor;
                                                            in vec3 texCoords;
                                                            uniform samplerCube skybox;
                                                            void main() {
                                                                fcolor = texture(skybox, texCoords);
                                                            }
                                                            )";

                const std::string fragmentShader = R"(#version 460
                                                      #extension GL_ARB_bindless_texture : enable
                                                      //#extension GL_ARB_fragment_shader_interlock : require
                                                      layout (early_fragment_tests) in;
                                                      struct NodeType {
                                                          vec4 color;
                                                          float depth;
                                                          uint next;
                                                      };
                                                      layout(binding = 0, offset = 0) uniform atomic_uint nextNodeCounter;
                                                      layout(binding = 0, r32ui) uniform uimage2D headPointers;
                                                      layout(std430, binding = 0) buffer linkedLists {
                                                          NodeType nodes[];
                                                      };
                                                      layout(std140, binding = 0) uniform ALL_TEXTURES {
                                                          sampler2D textures[200];
                                                      };
                                                      uniform uint maxNodes;
                                                      uniform sampler2D currentTex;
                                                      uniform float water;
                                                      in vec4 frontColor;
                                                      in vec2 fTexCoords;
                                                      in flat uint texIndex;
                                                      layout (location = 0) out vec4 fcolor;
                                                      /* fix: because of layout std140 16byte alignment, get uvec2 from array of uvec4 */
                                                      /*uvec2 GetTexture(uint index)
                                                      {
                                                          uint index_corrected = index / 2;
                                                          if (index % 2 == 0)
                                                              return maps[index_corrected].xy;
                                                          return maps[index_corrected].zw;
                                                      }*/
                                                      void main() {
                                                           uint nodeIdx = atomicCounterIncrement(nextNodeCounter);
                                                           vec4 color = /*(texIndex != 0) ? frontColor * texture(textures[texIndex-1], fTexCoords.xy) :*/ frontColor;
                                                           //beginInvocationInterlockARB();
                                                           if (nodeIdx < maxNodes) {
                                                                uint prevHead = imageAtomicExchange(headPointers, ivec2(gl_FragCoord.xy), nodeIdx);
                                                                nodes[nodeIdx].color = color;
                                                                nodes[nodeIdx].depth = /*(color == vec4(0, 1, 1, 1)) ? 1.0f :*/ gl_FragCoord.z;
                                                                nodes[nodeIdx].next = prevHead;
                                                                //fcolor = color;
                                                           }

                                                           fcolor = vec4(0, 0, 0, 0);
                                                           //endInvocationInterlockARB();
                                                      })";
                 const std::string fragmentShader2 =
                   R"(
                   #version 460
                   #define MAX_FRAGMENTS 20
                   /*struct NodeType {
                      vec4 color;
                      float depth;
                      uint next;
                   };
                   layout(binding = 0, r32ui) uniform uimage2D headPointers;
                   layout(std430, binding = 0) buffer linkedLists {
                       NodeType nodes[];
                   };*/
                   layout(location = 0) out vec4 fcolor;
                   void main() {
                      /*NodeType frags[MAX_FRAGMENTS];
                      int count = 0;
                      uint n = imageLoad(headPointers, ivec2(gl_FragCoord.xy)).r;
                      while( n != 0xffffffffu && count < MAX_FRAGMENTS) {
                           frags[count] = nodes[n];
                           n = frags[count].next;
                           count++;
                      }


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
                        //color = mix (color, frags[i].color, frags[i].color.a);
                      }*/
                      fcolor = vec4(1, 0, 0, 1);
                   })";
                   /*if (!skyboxShader.loadFromMemory(skyboxVertexShader, skyboxFragmentShader)) {
                        throw core::Erreur(53, "Failed to load skybox shader");
                   }*/
                   if (!perPixelLinkedListP2.loadFromMemory(simpleVertexShader, fragmentShader2)) {
                        throw core::Erreur(55, "Failed to load per pixel linked list pass 2 shader");
                   }
                   /*if (!indirectRenderingShader.loadFromMemory(indirectDrawVertexShader, fragmentShader)) {
                       throw core::Erreur(57, "Failed to load indirect rendering shader");
                   }*/
                   /*skyboxShader.setParameter("skybox", Shader::CurrentTexture);
                   indirectRenderingShader.setParameter("maxNodes", maxNodes);
                   indirectRenderingShader.setParameter("currentTex", Shader::CurrentTexture);
                   indirectRenderingShader.setParameter("resolution", resolution.x(), resolution.y(), resolution.z());*/
                   math::Matrix4f viewMatrix = getWindow().getDefaultView().getViewMatrix().getMatrix().transpose();
                   math::Matrix4f projMatrix = getWindow().getDefaultView().getProjMatrix().getMatrix().transpose();
                   perPixelLinkedListP2.setParameter("viewMatrix", viewMatrix);
                   perPixelLinkedListP2.setParameter("projectionMatrix", projMatrix);
                   ////std::cout<<"view matrix : "<<viewMatrix<<std::endl;
                   ////std::cout<<"projection matrix : "<<projMatrix<<std::endl;
        }
        void PerPixelLinkedListRenderComponent::setBackgroundColor(Color color) {
            backgroundColor = color;
        }
        void PerPixelLinkedListRenderComponent::clear() {
            //frameBuffer.setActive();
            frameBuffer.clear(backgroundColor);
            //getWindow().setActive();
            /*GLuint zero = 0;
            glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicBuffer));
            glCheck(glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero));
            glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf));
            glCheck(glBindTexture(GL_TEXTURE_2D, headPtrTex));
            glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, view.getSize().x(), view.getSize().y(), GL_RED_INTEGER,
            GL_UNSIGNED_INT, NULL));
            glCheck(glBindTexture(GL_TEXTURE_2D, 0));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));*/

            //frameBuffer.resetGLStates();


            //getWindow().resetGLStates();

        }
        void PerPixelLinkedListRenderComponent::drawSelectedInstances() {
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
            for (unsigned int i = 0; i < m_selected.size(); i++) {
                if (m_selected[i].getAllVertices().getVertexCount() > 0) {
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    ////////std::cout<<"next frame draw normal"<<std::endl;

                    float time = timeClock.getElapsedTime().asSeconds();
                    indirectRenderingShader.setParameter("time", time);

                    unsigned int p = m_selected[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    {
                        std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                        material.textureIndex = (m_selected[i].getMaterial().getTexture() != nullptr) ? m_selected[i].getMaterial().getTexture()->getId() : 0;
                        material.materialType = m_selected[i].getMaterial().getType();
                    }
                    materials[p].push_back(material);

                    TransformMatrix tm;
                    ModelData model;
                    model.worldMat = tm.getMatrix().transpose();
                    matrices[p].push_back(model);
                    unsigned int vertexCount = 0;
                    for (unsigned int j = 0; j < m_selected[i].getAllVertices().getVertexCount(); j++) {
                        vbBindlessTex[p].append(m_selected[i].getAllVertices()[j]);
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
            for (unsigned int i = 0; i < m_selectedInstance.size(); i++) {
                if (m_selectedInstance[i].getAllVertices().getVertexCount() > 0) {
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    unsigned int p = m_selectedInstance[i].getAllVertices().getPrimitiveType();

                    float time = timeClock.getElapsedTime().asSeconds();
                    indirectRenderingShader.setParameter("time", time);

                    MaterialData material;
                    material.textureIndex = (m_selectedInstance[i].getMaterial().getTexture() != nullptr) ? m_selectedInstance[i].getMaterial().getTexture()->getId() : 0;
                    material.materialType = m_selectedInstance[i].getMaterial().getType();
                    materials[p].push_back(material);

                    std::vector<TransformMatrix*> tm = m_selectedInstance[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData model;
                        model.worldMat = tm[j]->getMatrix().transpose();
                        matrices[p].push_back(model);
                    }
                    unsigned int vertexCount = 0;
                    if (m_selectedInstance[i].getVertexArrays().size() > 0) {
                        Entity* entity = m_selectedInstance[i].getVertexArrays()[0]->getEntity();
                        for (unsigned int j = 0; j < m_selectedInstance[i].getVertexArrays().size(); j++) {
                            if (entity == m_selectedInstance[i].getVertexArrays()[j]->getEntity()) {
                                for (unsigned int k = 0; k < m_selectedInstance[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                    vertexCount++;
                                    vbBindlessTex[p].append((*m_selectedInstance[i].getVertexArrays()[j])[k]);
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
                    ////////std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                    ////////std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;

                }
            }
            currentStates.blendMode = BlendNone;
            currentStates.shader = &indirectRenderingShader;
            currentStates.texture = nullptr;
            glCheck(glEnable(GL_STENCIL_TEST));
            glCheck(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE));
            glCheck(glStencilFunc(GL_ALWAYS, 1, 0xFF));
            glCheck(glStencilMask(0xFF));
            //glCheck(glDisable(GL_ALPHA_TEST));
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
                    frameBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), currentStates, vboIndirect);
                    vbBindlessTex[p].clear();
                }
            }
            //glCheck(glEnable(GL_ALPHA_TEST));
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
            for (unsigned int i = 0; i < m_selectedScale.size(); i++) {
                if (m_selectedScale[i].getAllVertices().getVertexCount() > 0) {
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    ////////std::cout<<"next frame draw normal"<<std::endl;
                    /*if (core::Application::app != nullptr) {
                        float time = core::Application::getTimeClk().getElapsedTime().asSeconds();
                        perPixelLinkedList2.setParameter("time", time);
                    }*/
                    unsigned int p = m_selectedScale[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = 0;
                    material.materialType = 0;
                    materials[p].push_back(material);
                    TransformMatrix tm;
                    ModelData model;
                    model.worldMat = tm.getMatrix().transpose();
                    matrices[p].push_back(model);

                    unsigned int vertexCount = 0;
                    for (unsigned int j = 0; j < m_selectedScale[i].getAllVertices().getVertexCount(); j++) {
                        vertexCount++;
                        vbBindlessTex[p].append(m_selectedScale[i].getAllVertices()[j], 0);
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
            for (unsigned int i = 0; i < m_selectedScaleInstance.size(); i++) {
                unsigned int p = m_selectedScaleInstance[i].getAllVertices().getPrimitiveType();
                if (m_selectedScaleInstance[i].getAllVertices().getVertexCount() > 0) {
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    /*if (core::Application::app != nullptr) {
                        float time = core::Application::getTimeClk().getElapsedTime().asSeconds();
                        perPixelLinkedList.setParameter("time", time);
                    }*/
                    MaterialData material;
                    material.textureIndex = 0;
                    material.materialType = 0;
                    materials[p].push_back(material);
                    std::vector<TransformMatrix*> tm = m_selectedScaleInstance[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData model;
                        model.worldMat = tm[j]->getMatrix().transpose();
                        matrices[p].push_back(model);
                    }
                    unsigned int vertexCount = 0;
                    if (m_selectedScaleInstance[i].getVertexArrays().size() > 0) {
                        Entity* entity = m_selectedScaleInstance[i].getVertexArrays()[0]->getEntity();
                        for (unsigned int j = 0; j < m_selectedScaleInstance[i].getVertexArrays().size(); j++) {
                            if (entity == m_selectedScaleInstance[i].getVertexArrays()[j]->getEntity()) {
                                for (unsigned int k = 0; k < m_selectedScaleInstance[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                    vertexCount++;
                                    vbBindlessTex[p].append((*m_selectedScaleInstance[i].getVertexArrays()[j])[k], 0);
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
                    ////////std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                    ////////std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;

                }
            }
            currentStates.blendMode = BlendNone;
            currentStates.shader = &indirectRenderingShader;
            currentStates.texture = nullptr;
            glCheck(glStencilFunc(GL_NOTEQUAL, 1, 0xFF));
            glCheck(glStencilMask(0x00));
            //glCheck(glDisable(GL_DEPTH_TEST));
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
                    frameBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), currentStates, vboIndirect);
                    vbBindlessTex[p].clear();
                }
            }
            glStencilMask(0xFF);
            glStencilFunc(GL_ALWAYS, 0, 0xFF);
            //glCheck(glEnable(GL_DEPTH_TEST));
            glCheck(glDisable(GL_STENCIL_TEST));
        }
        void PerPixelLinkedListRenderComponent::drawSelectedInstancesIndexed() {
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
            for (unsigned int i = 0; i < m_selectedIndexed.size(); i++) {
                if (m_selectedIndexed[i].getAllVertices().getVertexCount() > 0) {
                    DrawElementsIndirectCommand drawElementsIndirectCommand;
                    ////////std::cout<<"next frame draw normal"<<std::endl;

                    float time = timeClock.getElapsedTime().asSeconds();
                    indirectRenderingShader.setParameter("time", time);

                    unsigned int p = m_selectedIndexed[i].getAllVertices().getPrimitiveType();

                    MaterialData material;
                    material.textureIndex = (m_selectedIndexed[i].getMaterial().getTexture() != nullptr) ? m_selectedIndexed[i].getMaterial().getTexture()->getId() : 0;
                    material.materialType = m_selectedIndexed[i].getMaterial().getType();
                    materials[p].push_back(material);

                    TransformMatrix tm;
                    ModelData model;
                    model.worldMat = tm.getMatrix().transpose();
                    matrices[p].push_back(model);
                    unsigned int vertexCount = 0, indexCount = 0;
                    for (unsigned int j = 0; j < m_selectedIndexed[i].getAllVertices().getVertexCount(); j++) {
                        vertexCount++;
                        vbBindlessTex[p].append(m_selectedIndexed[i].getAllVertices()[j]);
                    }
                    for (unsigned int j = 0; j < m_selectedIndexed[i].getAllVertices().getIndexes().size(); j++) {
                        indexCount++;
                        vbBindlessTex[p].addIndex(m_selectedIndexed[i].getAllVertices().getIndexes()[j]);
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
            for (unsigned int i = 0; i < m_selectedInstanceIndexed.size(); i++) {

                if (m_selectedInstanceIndexed[i].getAllVertices().getVertexCount() > 0) {
                    DrawElementsIndirectCommand drawElementsIndirectCommand;
                    unsigned int p = m_selectedInstanceIndexed[i].getAllVertices().getPrimitiveType();
                    /*if (core::Application::app != nullptr) {
                        float time = core::Application::getTimeClk().getElapsedTime().asSeconds();
                        perPixelLinkedList.setParameter("time", time);
                    }*/
                    MaterialData material;
                    material.textureIndex = (m_selectedInstanceIndexed[i].getMaterial().getTexture() != nullptr) ? m_selectedInstanceIndexed[i].getMaterial().getTexture()->getId() : 0;
                    material.materialType = m_selectedInstanceIndexed[i].getMaterial().getType();
                    materials[p].push_back(material);
                    std::vector<TransformMatrix*> tm = m_selectedInstanceIndexed[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData model;
                        model.worldMat = tm[j]->getMatrix().transpose();
                        matrices[p].push_back(model);
                    }
                    unsigned int indexCount = 0, vertexCount = 0;
                    if (m_selectedInstanceIndexed[i].getVertexArrays().size() > 0) {
                        Entity* entity = m_selectedInstanceIndexed[i].getVertexArrays()[0]->getEntity();
                        for (unsigned int j = 0; j < m_selectedInstanceIndexed[i].getVertexArrays().size(); j++) {
                            if (entity == m_selectedInstanceIndexed[i].getVertexArrays()[j]->getEntity()) {

                                for (unsigned int k = 0; k < m_selectedInstanceIndexed[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                    vertexCount++;
                                    vbBindlessTex[p].append((*m_selectedInstanceIndexed[i].getVertexArrays()[j])[k]);
                                }
                                for (unsigned int k = 0; k < m_selectedInstanceIndexed[i].getVertexArrays()[j]->getIndexes().size(); k++) {
                                    indexCount++;
                                    vbBindlessTex[p].addIndex(m_selectedInstanceIndexed[i].getVertexArrays()[j]->getIndexes()[k]);
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
                    ////////std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                    ////////std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;
                }
            }
            currentStates.blendMode = BlendNone;
            currentStates.shader = &indirectRenderingShader;
            currentStates.texture = nullptr;
            glCheck(glEnable(GL_STENCIL_TEST));
            glCheck(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE));
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glStencilMask(0xFF);
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
                    frameBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawElementsIndirectCommands[p].size(), currentStates, vboIndirect);
                    vbBindlessTex[p].clear();
                }
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
            for (unsigned int i = 0; i < matrices.size(); i++) {
                matrices[i].clear();
            }
            for (unsigned int i = 0; i < materials.size(); i++) {
                materials[i].clear();
            }
            for (unsigned int i = 0; i < drawElementsIndirectCommands.size(); i++) {
                drawElementsIndirectCommands[i].clear();
            }
            for (unsigned int i = 0; i < m_selectedScaleIndexed.size(); i++) {
                if (m_selectedScaleIndexed[i].getAllVertices().getVertexCount() > 0) {
                    DrawElementsIndirectCommand drawElementsIndirectCommand;
                    ////////std::cout<<"next frame draw normal"<<std::endl;
                    unsigned int p = m_selectedScaleIndexed[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = 0;
                    material.materialType = m_selectedScaleIndexed[i].getMaterial().getType();
                    materials[p].push_back(material);

                    TransformMatrix tm;
                    ModelData model;
                    model.worldMat = tm.getMatrix().transpose();
                    matrices[p].push_back(model);

                    unsigned int indexCount = 0, vertexCount = 0;
                    for (unsigned int j = 0; j < m_selectedScaleIndexed[i].getAllVertices().getVertexCount(); j++) {
                        vertexCount++;
                        vbBindlessTex[p].append(m_selectedScaleIndexed[i].getAllVertices()[j]);
                    }
                    for (unsigned int j = 0; j < m_selectedScaleIndexed[i].getAllVertices().getIndexes().size(); j++) {
                        indexCount++;
                        vbBindlessTex[p].addIndex(m_selectedScaleIndexed[i].getAllVertices().getIndexes()[j]);
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
            for (unsigned int i = 0; i < m_selectedScaleInstanceIndexed.size(); i++) {

                if (m_selectedScaleInstanceIndexed[i].getAllVertices().getVertexCount() > 0) {
                    DrawElementsIndirectCommand drawElementsIndirectCommand;
                    unsigned int p = m_selectedScaleInstanceIndexed[i].getAllVertices().getPrimitiveType();
                    /*if (core::Application::app != nullptr) {
                        float time = core::Application::getTimeClk().getElapsedTime().asSeconds();
                        perPixelLinkedList.setParameter("time", time);
                    }*/
                    MaterialData material;
                    material.textureIndex = 0;
                    material.materialType = m_selectedScaleInstanceIndexed[i].getMaterial().getType();
                    materials[p].push_back(material);
                    std::vector<TransformMatrix*> tm = m_selectedScaleInstanceIndexed[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData model;
                        model.worldMat = tm[j]->getMatrix().transpose();
                        matrices[p].push_back(model);
                    }
                    unsigned int indexCount = 0, vertexCount = 0;
                    if (m_selectedScaleInstanceIndexed[i].getVertexArrays().size() > 0) {
                        Entity* entity = m_selectedScaleInstanceIndexed[i].getVertexArrays()[0]->getEntity();
                        for (unsigned int j = 0; j < m_selectedScaleInstanceIndexed[i].getVertexArrays().size(); j++) {
                            if (entity == m_selectedScaleInstanceIndexed[i].getVertexArrays()[j]->getEntity()) {
                                unsigned int p = m_selectedScaleInstanceIndexed[i].getVertexArrays()[j]->getPrimitiveType();
                                for (unsigned int k = 0; k < m_selectedScaleInstanceIndexed[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                    vertexCount++;
                                    vbBindlessTex[p].append((*m_selectedScaleInstanceIndexed[i].getVertexArrays()[j])[k]);
                                }
                                for (unsigned int k = 0; k < m_selectedScaleInstanceIndexed[i].getVertexArrays()[j]->getIndexes().size(); k++) {
                                    indexCount++;
                                    vbBindlessTex[p].addIndex(m_selectedScaleInstanceIndexed[i].getVertexArrays()[j]->getIndexes()[k]);
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
                    ////////std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                    ////////std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;
                }
            }
            currentStates.blendMode = BlendNone;
            currentStates.shader = &indirectRenderingShader;
            currentStates.texture = nullptr;
            glCheck(glStencilFunc(GL_NOTEQUAL, 1, 0xFF));
            glCheck(glStencilMask(0x00));
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
                    frameBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawElementsIndirectCommands[p].size(), currentStates, vboIndirect);
                    vbBindlessTex[p].clear();
                }
            }
            glStencilMask(0xFF);
            glStencilFunc(GL_ALWAYS, 0, 0xFF);
            glCheck(glDisable(GL_STENCIL_TEST));
        }
        void PerPixelLinkedListRenderComponent::drawNextFrame() {


            /*glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
            glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicBuffer));
            glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, linkedListBuffer));*/
            ////////std::cout<<"draw nex frame"<<std::endl;
            //basicView.setPerspective(-1, 1, -1, 1, 0, 1);


            {

                /*std::lock_guard<std::recursive_mutex> lock(rec_mutex);



                if(datasReady) {
                    datasReady = false;



                    ////////std::cout<<"data ready"<<std::endl;
                    m_instances = batcher.getInstances();
                    m_normals = normalBatcher.getInstances();
                    m_instancesIndexed = batcherIndexed.getInstances();
                    m_normalsIndexed = normalBatcherIndexed.getInstances();
                    m_selected = selectedBatcher.getInstances();
                    m_selectedScale = selectedScaleBatcher.getInstances();
                    m_selectedIndexed = selectedIndexBatcher.getInstances();
                    m_selectedScaleIndexed = selectedIndexScaleBatcher.getInstances();
                    m_selectedInstance = selectedInstanceBatcher.getInstances();
                    m_selectedScaleInstance = selectedInstanceScaleBatcher.getInstances();
                    m_selectedInstanceIndexed = selectedInstanceIndexBatcher.getInstances();
                    m_selectedScaleInstanceIndexed = selectedInstanceIndexScaleBatcher.getInstances();
                    m_skyboxInstance = skyboxBatcher.getInstances();
                    renderFinished = false;
                    //cv.notify_all();
                }*/
            }
            /*std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this]() {return !renderFinished && cleared;});*/
            //frameBuffer.setActive();
            /*frameBuffer.clear(backgroundColor);
            //getWindow().setActive();
            GLuint zero = 0;
            glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicBuffer));
            glCheck(glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero));
            glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf));
            glCheck(glBindTexture(GL_TEXTURE_2D, headPtrTex));
            glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, view.getSize().x(), view.getSize().y(), GL_RED_INTEGER,
            GL_UNSIGNED_INT, NULL));
            glCheck(glBindTexture(GL_TEXTURE_2D, 0));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));

            frameBuffer.resetGLStates();*/
            /*float zNear = view.getViewport().getPosition().z();
            if (!view.isOrtho())
                view.setPerspective(80, view.getViewport().getSize().x() / view.getViewport().getSize().y(), 0.1f, view.getViewport().getSize().z());
            math::Matrix4f viewMatrix = view.getViewMatrix().getMatrix().transpose();
            math::Matrix4f projMatrix = view.getProjMatrix().getMatrix().transpose();
            viewMatrix = math::Matrix4f(math::Matrix3f(viewMatrix));

            skyboxShader.setParameter("projection", projMatrix);
            skyboxShader.setParameter("view", viewMatrix);
            vb.clear();
            //vb.name = "SKYBOXVB";
            for (unsigned int i = 0; i < m_skyboxInstance.size(); i++) {
                if (m_skyboxInstance[i].getAllVertices().getVertexCount() > 0) {
                    vb.setPrimitiveType(m_skyboxInstance[i].getAllVertices().getPrimitiveType());
                    for (unsigned int j = 0; j < m_skyboxInstance[i].getAllVertices().getVertexCount(); j++) {
                        //if (m_skyboxInstance[i].getAllVertices()[j].position.x != 0 && m_skyboxInstance[i].getAllVertices()[j].position.y != 0 && m_skyboxInstance[i].getAllVertices()[j].position.z != 0);
                        vb.append(m_skyboxInstance[i].getAllVertices()[j]);
                    }
                }
            }
            currentStates.blendMode = BlendAlpha;
            currentStates.shader = &skyboxShader;
            currentStates.texture = (skybox == nullptr ) ? nullptr : &static_cast<g3d::Skybox*>(skybox)->getTexture();
            vb.update();
            frameBuffer.drawVertexBuffer(vb, currentStates);
            vb.clear();
            if (!view.isOrtho())
                view.setPerspective(80, view.getViewport().getSize().x() / view.getViewport().getSize().y(), zNear, view.getViewport().getSize().z());
            projMatrix = view.getProjMatrix().getMatrix().transpose();
            viewMatrix = view.getViewMatrix().getMatrix().transpose();
            indirectRenderingShader.setParameter("projectionMatrix", projMatrix);
            indirectRenderingShader.setParameter("viewMatrix", viewMatrix);

            /*drawInstances();
            drawInstancesIndexed();
            drawSelectedInstances();
            drawSelectedInstancesIndexed();*/
            ////////std::cout<<"nb instances : "<<m_normals.size()<<std::endl;


            /*glCheck(glFinish());
            glCheck(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));*/
            //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            vb.clear();
            //vb.name = "";
            vb.setPrimitiveType(Quads);
            Vertex v1 (math::Vec3f(0, 0, 0));
            Vertex v2 (math::Vec3f(quad.getSize().x(),0, 0));
            Vertex v3 (math::Vec3f(quad.getSize().x(), quad.getSize().y(), 0));
            Vertex v4 (math::Vec3f(0, quad.getSize().y(), 0));
            vb.append(v1);
            vb.append(v2);
            vb.append(v3);
            vb.append(v4);
            vb.update();

            math::Matrix4f matrix = quad.getTransform().getMatrix().transpose();
            ////std::cout<<"world mat : "<<matrix<<std::endl;
            perPixelLinkedListP2.setParameter("worldMat", matrix);
            currentStates.shader = &perPixelLinkedListP2;
            frameBuffer.drawVertexBuffer(vb, currentStates);
            //glCheck(glFinish());
            frameBuffer.display();
            /*renderFinished = true;
            cv.notify_all();*/



            //glCheck(glMemoryBarrier(GL_ALL_BARRIER_BITS));
            /*glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0));
            glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, 0));
            glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0));*/
        }
        void PerPixelLinkedListRenderComponent::drawInstances() {
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
                    ////////std::cout<<"next frame draw normal"<<std::endl;
                    float time = timeClock.getElapsedTime().asSeconds();
                    indirectRenderingShader.setParameter("time", time);

                    unsigned int p = m_normals[i].getAllVertices().getPrimitiveType();
                    /*if (m_normals[i].getVertexArrays()[0]->getEntity()->getRootType() == "E_MONSTER") {
                            //////std::cout<<"tex coords : "<<(*m_normals[i].getVertexArrays()[0])[0].texCoords.x<<","<<(*m_normals[i].getVertexArrays()[0])[0].texCoords.y<<std::endl;
                        }*/
                    unsigned int vertexCount = 0;
                    MaterialData material;
                    {
                        std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                        material.textureIndex = (m_normals[i].getMaterial().getTexture() != nullptr) ? m_normals[i].getMaterial().getTexture()->getId() : 0;
                        material.materialType = m_normals[i].getMaterial().getType();
                    }
                    materials[p].push_back(material);
                    /*for (unsigned int v = 0; v < m_normals[i].getVertexArrays().size(); v++) {
                        if (m_normals[i].getVertexArrays()[v]->getEntity()->getType() == "E_MESH") {
                            for (unsigned int n = 0; n < m_normals[i].getVertexArrays()[v]->getVertexCount(); n++) {
                                //////std::cout<<"vertex position : "<<(*m_normals[i].getVertexArrays()[v])[n].position.x<<std::endl;
                                //////std::cout<<"vertex color : "<<(int) (*m_normals[i].getVertexArrays()[v])[n].color.r<<std::endl;
                                //////std::cout<<"vertex tex coords : "<<(*m_normals[i].getVertexArrays()[v])[n].texCoords.x<<std::endl;

                            }
                        }
                    }*/
                    for (unsigned int j = 0; j < m_normals[i].getAllVertices().getVertexCount(); j++) {
                        vbBindlessTex[p].append(m_normals[i].getAllVertices()[j]);
                        vertexCount++;
                    }

                    TransformMatrix tm;
                    ModelData modelData;
                    modelData.worldMat = tm.getMatrix().transpose();

                    std::vector<math::Matrix4f> finalBoneMatrices = m_normals[i].getFinalBoneMatrices();
                    for (unsigned int b = 0; b < MAX_BONES && b < finalBoneMatrices.size(); b++) {
                        modelData.finalBoneMatrices[b] = finalBoneMatrices[b].transpose();
                    }

                    matrices[p].push_back(modelData);
                    drawArraysIndirectCommand.count = vertexCount;
                    drawArraysIndirectCommand.firstIndex = firstIndex[p];
                    drawArraysIndirectCommand.baseInstance = baseInstance[p];
                    drawArraysIndirectCommand.instanceCount = 1;
                    drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                    firstIndex[p] += vertexCount;
                    baseInstance[p] += 1;
                    /*for (unsigned int j = 0; j < m_normals[i].getVertexArrays().size(); j++) {
                        if (m_normals[i].getVertexArrays()[j]->getEntity() != nullptr && m_normals[i].getVertexArrays()[j]->getEntity()->getRootType() == "E_HERO") {
                            for (unsigned int n = 0; n < m_normals[i].getVertexArrays()[j]->getVertexCount(); n++)
                                //////std::cout<<"position hero : "<<(*m_normals[i].getVertexArrays()[j])[n].position.x<<","<<(*m_normals[i].getVertexArrays()[j])[n].position.y<<","<<(*m_normals[i].getVertexArrays()[j])[n].position.z<<std::endl;
                        }
                    }*/
                }
            }
            for (unsigned int i = 0; i < m_instances.size(); i++) {
                if (m_instances[i].getAllVertices().getVertexCount() > 0) {
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    unsigned int p = m_instances[i].getAllVertices().getPrimitiveType();
                    float time = timeClock.getElapsedTime().asSeconds();
                    indirectRenderingShader.setParameter("time", time);
                    std::vector<TransformMatrix*> tm = m_instances[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData modelData;
                        modelData.worldMat = tm[j]->getMatrix().transpose();
                        std::vector<math::Matrix4f> finalBoneMatrices = m_instances[i].getFinalBoneMatrices();
                        for (unsigned int m = 0, b = MAX_BONES*j; b < j*MAX_BONES+MAX_BONES && b < finalBoneMatrices.size(); b++, m++) {

                            modelData.finalBoneMatrices[m] = finalBoneMatrices[b].transpose();
                        }
                        matrices[p].push_back(modelData);
                    }
                    MaterialData materialData;
                    {
                        std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                        materialData.textureIndex = (m_instances[i].getMaterial().getTexture() != nullptr) ? m_instances[i].getMaterial().getTexture()->getId() : 0;
                        materialData.materialType = m_instances[i].getMaterial().getType();
                    }
                    materials[p].push_back(materialData);
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
                    ////////std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                    ////////std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;

                }
            }
            RenderStates currentStates;
            currentStates.blendMode = BlendNone;
            currentStates.shader = &indirectRenderingShader;
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
                    frameBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), currentStates, vboIndirect);
                    vbBindlessTex[p].clear();
                }
            }
        }
        void PerPixelLinkedListRenderComponent::drawInstancesIndexed() {
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
                    float time = timeClock.getElapsedTime().asSeconds();
                    indirectRenderingShader.setParameter("time", time);
                    unsigned int p = m_normalsIndexed[i].getAllVertices().getPrimitiveType();
                    /*if (m_normals[i].getVertexArrays()[0]->getEntity()->getRootType() == "E_MONSTER") {
                            //////std::cout<<"tex coords : "<<(*m_normals[i].getVertexArrays()[0])[0].texCoords.x<<","<<(*m_normals[i].getVertexArrays()[0])[0].texCoords.y<<std::endl;
                        }*/
                    MaterialData material;
                    material.textureIndex = (m_normalsIndexed[i].getMaterial().getTexture() != nullptr) ? m_normalsIndexed[i].getMaterial().getTexture()->getId() : 0;
                    material.materialType = m_normalsIndexed[i].getMaterial().getType();
                    materials[p].push_back(material);

                    TransformMatrix tm;
                    ModelData modelData;
                    modelData.worldMat = tm.getMatrix().transpose();
                    matrices[p].push_back(modelData);
                    unsigned int indexCount = 0, vertexCount = 0;
                    for (unsigned int j = 0; j < m_normalsIndexed[i].getAllVertices().getVertexCount(); j++) {
                        vbBindlessTex[p].append(m_normalsIndexed[i].getAllVertices()[j]);
                        vertexCount++;
                    }
                    for (unsigned int j = 0; j < m_normalsIndexed[i].getAllVertices().getIndexes().size(); j++) {
                        vbBindlessTex[p].addIndex(m_normalsIndexed[i].getAllVertices().getIndexes()[j]);
                        indexCount++;
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
                    /*if (core::Application::app != nullptr) {
                        float time = core::Application::getTimeClk().getElapsedTime().asSeconds();
                        perPixelLinkedList.setParameter("time", time);
                    }*/
                    MaterialData material;
                    material.textureIndex = (m_instancesIndexed[i].getMaterial().getTexture() != nullptr) ? m_instancesIndexed[i].getMaterial().getTexture()->getId() : 0;
                    material.materialType = m_instancesIndexed[i].getMaterial().getType();
                    materials[p].push_back(material);

                    std::vector<TransformMatrix*> tm = m_instancesIndexed[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData model;
                        model.worldMat = tm[j]->getMatrix().transpose();
                        matrices[p].push_back(model);
                    }

                    unsigned int indexCount = 0, vertexCount = 0;
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
                    ////////std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                    ////////std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;
                }
            }
            currentStates.blendMode = BlendNone;
            currentStates.shader = &indirectRenderingShader;
            currentStates.texture = nullptr;
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
                    frameBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawElementsIndirectCommands[p].size(), currentStates, vboIndirect);
                    vbBindlessTex[p].clear();
                }
            }
        }
        void PerPixelLinkedListRenderComponent::draw(RenderTarget& target, RenderStates states) {
            /*states.shader=&perPixelLinkedList;
            for (unsigned int i = 0; i < m_instances.size(); i++) {
               if (m_instances[i].getAllVertices().getVertexCount() > 0) {
                    if (m_instances[i].getMaterial().getTexture() == nullptr) {
                        perPixelLinkedList.setParameter("haveTexture", 0.f);
                        filterNotOpaque.setParameter("haveTexture", 0.f);
                    } else {
                        perPixelLinkedList.setParameter("haveTexture", 1.f);
                        filterNotOpaque.setParameter("haveTexture", 1.f);
                    }
                    states.texture = m_instances[i].getMaterial().getTexture();
                    target.draw(m_instances[i].getAllVertices(), states);
                }
            }
            glCheck(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));
            glCheck(glFinish());
            //glCheck(glTextureBarrier());
            states.shader = &perPixelLinkedListP2;
            //glCheck(glDepthMask(GL_FALSE));
            /*for (unsigned int i = 0; i < m_instances.size(); i++) {
               if (m_instances[i].getAllVertices().getVertexCount() > 0) {
                    target.draw(m_instances[i].getAllVertices(), states);
               }
            }*/

            /*target.draw(quad, states);
            glCheck(glFinish());*/
            /*std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this](){return renderFinished;});*/

            frameBufferSprite.setCenter(target.getView().getPosition());
            target.draw(frameBufferSprite, states);
            /*cleared = true;
            cv.notify_all();*/
        }
        int  PerPixelLinkedListRenderComponent::getLayer() {
            return layer;
        }
        void PerPixelLinkedListRenderComponent::draw(Drawable& drawable, RenderStates states) {
            //drawables.insert(std::make_pair(drawable, states));
        }
        void PerPixelLinkedListRenderComponent::setView(View view) {
            frameBuffer.setView(view);
            this->view = view;
        }
        std::vector<Entity*> PerPixelLinkedListRenderComponent::getEntities() {
            return visibleEntities;
        }
        std::string PerPixelLinkedListRenderComponent::getExpression() {
            return expression;
        }
        View& PerPixelLinkedListRenderComponent::getView() {
            return view;
        }
        bool PerPixelLinkedListRenderComponent::needToUpdate() {
            return update;
        }
        void PerPixelLinkedListRenderComponent::setExpression (std::string expression) {
            this->expression = expression;
        }
        void PerPixelLinkedListRenderComponent::loadSkybox(Entity* skybox) {
            this->skybox = skybox;
        }
        bool PerPixelLinkedListRenderComponent::loadEntitiesOnComponent(std::vector<Entity*> vEntities) {

            /*{
                std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                datasReady = false;

                //if (datasReady) {
                    ////////std::cout<<"wait 2 "<<std::endl;
                    //cv.wait(lock, [this]() {return !datasReady;});
                //}
                ////////std::cout<<"data unready"<<std::endl;
                batcher.clear();
                normalBatcher.clear();
                batcherIndexed.clear();
                normalBatcherIndexed.clear();
                selectedBatcher.clear();
                selectedScaleBatcher.clear();
                selectedIndexBatcher.clear();
                selectedIndexScaleBatcher.clear();
                selectedInstanceBatcher.clear();
                selectedInstanceScaleBatcher.clear();
                selectedInstanceIndexBatcher.clear();
                selectedInstanceIndexScaleBatcher.clear();
                skyboxBatcher.clear();
                visibleSelectedScaleEntities.clear();
            }
            if (skybox != nullptr) {
                for (unsigned int i = 0; i <  skybox->getNbFaces(); i++) {
                    skyboxBatcher.addFace(skybox->getFace(i));
                }
            }
            for (unsigned int i = 0; i < vEntities.size(); i++) {

                if ( vEntities[i] != nullptr && vEntities[i]->isLeaf()) {
                    Entity* border;
                    if (vEntities[i]->isSelected()) {
                        border = vEntities[i]->clone();
                        border->decreaseNbEntities();
                    }
                    for (unsigned int j = 0; j <  vEntities[i]->getNbFaces(); j++) {

                         if (vEntities[i]->getDrawMode() == Entity::INSTANCED && !vEntities[i]->isSelected()) {
                            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                            if (vEntities[i]->getFace(j)->getVertexArray().getIndexes().size() == 0)
                                batcher.addFace( vEntities[i]->getFace(j));
                            else
                                batcherIndexed.addFace(vEntities[i]->getFace(j));
                         } else if (vEntities[i]->getDrawMode() == Entity::NORMAL && !vEntities[i]->isSelected()) {
                             std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                             if (vEntities[i]->getFace(j)->getVertexArray().getIndexes().size() == 0) {
                                normalBatcher.addFace( vEntities[i]->getFace(j));
                             } else
                                normalBatcherIndexed.addFace( vEntities[i]->getFace(j));
                        } else if (vEntities[i]->getDrawMode() == Entity::INSTANCED && vEntities[i]->isSelected()) {

                            if (vEntities[i]->getFace(j)->getVertexArray().getIndexes().size() == 0) {
                                {
                                    std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                                    selectedInstanceBatcher.addFace(vEntities[i]->getFace(j));
                                }

                           // //////std::cout<<"remove texture"<<std::endl;

                            ////////std::cout<<"get va"<<std::endl;
                                VertexArray& va = border->getFace(j)->getVertexArray();
                                ////////std::cout<<"change color"<<std::endl;
                                for (unsigned int j = 0; j < va.getVertexCount(); j++) {

                                    va[j].color = Color::Cyan;
                                }

                                border->setOrigin(border->getSize() * 0.5f);
                                border->setScale(math::Vec3f(1.1f, 1.1f, 1.1f));
                               // //////std::cout<<"add to batcher"<<std::endl;
                                selectedInstanceScaleBatcher.addFace(border->getFace(j));
                           // //////std::cout<<"face added"<<std::endl;
                             } else {
                                 {
                                    std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                                    selectedInstanceIndexBatcher.addFace(vEntities[i]->getFace(j));
                                 }

                               // //////std::cout<<"remove texture"<<std::endl;

                            ////////std::cout<<"get va"<<std::endl;
                                VertexArray& va = border->getFace(j)->getVertexArray();
                                ////////std::cout<<"change color"<<std::endl;
                                for (unsigned int j = 0; j < va.getVertexCount(); j++) {

                                    va[j].color = Color::Cyan;
                                }
                                Entity* root = (vEntities[i]->getRootEntity()->isAnimated()) ? vEntities[i]->getRootEntity() : vEntities[i];
                                math::Vec3f oldSize = root->getSize();
                                border->setOrigin(root->getSize() * 0.5f);
                                border->setSize(root->getSize() * 1.1f);
                                math::Vec3f offset =  root->getSize() - oldSize;
                                //border->setPosition(root->getPosition() - offset * 0.5f);

                               // //////std::cout<<"add to batcher"<<std::endl;
                                selectedInstanceIndexScaleBatcher.addFace(border->getFace(j));
                             }
                        } else {
                            if (vEntities[i]->getFace(j)->getVertexArray().getIndexes().size() == 0) {
                                {
                                    std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                                    selectedBatcher.addFace(vEntities[i]->getFace(j));
                                }

                           // //////std::cout<<"remove texture"<<std::endl;

                            ////////std::cout<<"get va"<<std::endl;
                                VertexArray& va = border->getFace(j)->getVertexArray();
                                ////////std::cout<<"change color"<<std::endl;
                                for (unsigned int j = 0; j < va.getVertexCount(); j++) {

                                    va[j].color = Color::Cyan;
                                }
                                Entity* root = (vEntities[i]->getRootEntity()->isAnimated()) ? vEntities[i]->getRootEntity() : vEntities[i];
                                math::Vec3f oldSize = root->getSize();
                                border->setOrigin(root->getSize() * 0.5f);
                                border->setSize(root->getSize() * 1.1f);
                                math::Vec3f offset =  root->getSize() - oldSize;
                                if (border->getSize().z() > 0) {
                                    border->setPosition(root->getPosition() - offset * 0.5f);
                                }
                                selectedScaleBatcher.addFace(border->getFace(j));

                               // //////std::cout<<"face added"<<std::endl;
                             } else {
                                 {
                                    std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                                    selectedIndexBatcher.addFace(vEntities[i]->getFace(j));
                                 }

                               // //////std::cout<<"remove texture"<<std::endl;

                            ////////std::cout<<"get va"<<std::endl;
                                VertexArray& va = border->getFace(j)->getVertexArray();
                                ////////std::cout<<"change color"<<std::endl;
                                for (unsigned int j = 0; j < va.getVertexCount(); j++) {

                                    va[j].color = Color::Cyan;
                                }

                                border->setOrigin(border->getSize() * 0.5f);
                                border->setScale(math::Vec3f(1.1f, 1.1f, 1.1f));
                               // //////std::cout<<"add to batcher"<<std::endl;
                                selectedIndexScaleBatcher.addFace(border->getFace(j));
                             }
                        }
                    }
                    if (vEntities[i]->isSelected()) {
                        std::unique_ptr<Entity> ptr;
                        ptr.reset(border);
                        visibleSelectedScaleEntities.push_back(std::move(ptr));
                    }
                }

            }

            ////////std::cout<<"instances added"<<std::endl;
            visibleEntities = vEntities;*/
            update = true;
            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
            datasReady = true;



            return true;
        }
        void PerPixelLinkedListRenderComponent::pushEvent(window::IEvent event, RenderWindow& rw) {
            if (event.type == window::IEvent::WINDOW_EVENT && event.window.type == window::IEvent::WINDOW_EVENT_RESIZED && &getWindow() == &rw && isAutoResized()) {
                //////std::cout<<"recompute size"<<std::endl;
                recomputeSize();
                getListener().pushEvent(event);
                getView().reset(physic::BoundingBox(getView().getViewport().getPosition().x(), getView().getViewport().getPosition().y(), getView().getViewport().getPosition().z(), event.window.data1, event.window.data2, getView().getViewport().getDepth()));
            }
        }
        const Texture& PerPixelLinkedListRenderComponent::getFrameBufferTexture() {
            return frameBuffer.getTexture();
        }
        void PerPixelLinkedListRenderComponent::onVisibilityChanged(bool visible) {
            if (visible) {
                glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicBuffer));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, linkedListBuffer));
            } else {
                glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0));
                glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, 0));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0));
            }
        }
        RenderTexture* PerPixelLinkedListRenderComponent::getFrameBuffer() {
            return &frameBuffer;
        }
        PerPixelLinkedListRenderComponent::~PerPixelLinkedListRenderComponent() {
            glDeleteBuffers(1, &atomicBuffer);
            glDeleteBuffers(1, &linkedListBuffer);
            glDeleteBuffers(1, &clearBuf);
            glDeleteTextures(1, &headPtrTex);
            glDeleteBuffers(1, &modelDataBuffer);
            glDeleteBuffers(1, &materialDataBuffer);
            glDeleteBuffers(1, &vboIndirect);
            glDeleteBuffers(1, &ubo);
        }
        #endif // VULKAN
    }
}
