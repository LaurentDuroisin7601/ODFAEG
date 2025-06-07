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
        PerPixelLinkedListRenderComponent::PerPixelLinkedListRenderComponent(RenderWindow& window, int layer, std::string expression, window::ContextSettings settings) :
            HeavyComponent(window, math::Vec3f(window.getView().getPosition().x, window.getView().getPosition().y, layer),
                          math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0),
                          math::Vec3f(window.getView().getSize().x + window.getView().getSize().x * 0.5f, window.getView().getPosition().y + window.getView().getSize().y * 0.5f, layer)),
            view(window.getView()),
            expression(expression),
            quad(math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, window.getSize().y * 0.5f)),
            layer(layer),
            frameBuffer(window.getDevice()),
            vkDevice(window.getDevice()),
            indirectRenderingShader(window.getDevice()),
            perPixelLinkedListP2(window.getDevice()),
            vb(window.getDevice()),
            vbBindlessTex {VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice())},
            descriptorPool(window.getDescriptorPool()),
            descriptorSetLayout(window.getDescriptorSetLayout()),
            descriptorSets(window.getDescriptorSet()),
            vboIndirect(nullptr) {
            needToUpdateDS = false;
            maxVboIndirectSize = maxModelDataSize = maxMaterialDataSize = 0;
            vboIndirectStagingBuffer = modelDataStagingBuffer = materialDataStagingBuffer = nullptr;
            quad.move(math::Vec3f(-window.getView().getSize().x * 0.5f, -window.getView().getSize().y * 0.5f, 0));
            maxNodes = 20 * window.getView().getSize().x * window.getView().getSize().y;
            unsigned int nodeSize = 5 * sizeof(float) + sizeof(unsigned int);
            resolution = math::Vec3f((int) window.getSize().x, (int) window.getSize().y, window.getView().getSize().z);
            frameBuffer.create(window.getView().getSize().x, window.getView().getSize().y);
            frameBufferSprite = Sprite(frameBuffer.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), sf::IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));
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
            imageInfo.extent.width = static_cast<uint32_t>(window.getView().getSize().x);
            imageInfo.extent.height = static_cast<uint32_t>(window.getView().getSize().y);
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
            if (vkCreateImage(window.getDevice().getDevice(), &imageInfo, nullptr, &headPtrTextureImage) != VK_SUCCESS) {
                throw std::runtime_error("echec de la creation d'une image!");
            }

            VkMemoryRequirements memRequirements;
            vkGetImageMemoryRequirements(window.getDevice().getDevice(), headPtrTextureImage, &memRequirements);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);


            if (vkAllocateMemory(window.getDevice().getDevice(), &allocInfo, nullptr, &headPtrTextureImageMemory) != VK_SUCCESS) {
                throw std::runtime_error("echec de l'allocation de la memoire d'une image!");
            }
            vkBindImageMemory(window.getDevice().getDevice(), headPtrTextureImage, headPtrTextureImageMemory, 0);
            createHeadPtrImageView();
            createHeadPtrSampler();



            for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                vbBindlessTex[i].setPrimitiveType(static_cast<sf::PrimitiveType>(i));
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

            createDescriptorsAndPipelines();

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
            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            barrier.image = headPtrTextureImage;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.layerCount = 1;
            vkCmdPipelineBarrier(frameBuffer.getCommandBuffers()[currentFrame], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);


            frameBuffer.beginRenderPass();
            frameBuffer.display();
            datasReady = false;

        }
        void PerPixelLinkedListRenderComponent::createDescriptorsAndPipelines() {
            RenderStates states;
            states.shader = &indirectRenderingShader;
            createDescriptorPool(states);
            createDescriptorSetLayout(states);
            allocateDescriptorSets(states);
            states.shader = &perPixelLinkedListP2;
            createDescriptorPool2(states);
            createDescriptorSetLayout2(states);
            allocateDescriptorSets2(states);
            createDescriptorSets2(states);
            states.shader = &indirectRenderingShader;
            std::vector<std::vector<std::vector<VkPipelineLayoutCreateInfo>>>& pipelineLayoutInfo = frameBuffer.getPipelineLayoutCreateInfo();
            std::vector<std::vector<std::vector<VkPipelineDepthStencilStateCreateInfo>>>& depthStencilCreateInfo = frameBuffer.getDepthStencilCreateInfo();
            pipelineLayoutInfo.resize((Batcher::nbPrimitiveTypes - 1) * indirectRenderingShader.getNbShaders());
            depthStencilCreateInfo.resize((Batcher::nbPrimitiveTypes - 1) * indirectRenderingShader.getNbShaders());
            for (unsigned int i = 0; i < (Batcher::nbPrimitiveTypes - 1) * indirectRenderingShader.getNbShaders(); i++) {
                pipelineLayoutInfo[i].resize(frameBuffer.getNbRenderTargets());
                depthStencilCreateInfo[i].resize(frameBuffer.getNbRenderTargets());
            }
            for (unsigned int i = 0; i < (Batcher::nbPrimitiveTypes - 1) * indirectRenderingShader.getNbShaders(); i++) {
                for (unsigned int j = 0; j < frameBuffer.getNbRenderTargets(); j++) {
                    pipelineLayoutInfo[i][j].resize(NBDEPTHSTENCIL);
                    depthStencilCreateInfo[i][j].resize(NBDEPTHSTENCIL);
                }
            }
            frameBuffer.enableDepthTest(true);
            for (unsigned int j = 0; j < NBDEPTHSTENCIL; j++) {
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

                       pipelineLayoutInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHNOSTENCIL].pPushConstantRanges = &push_constant;
                       pipelineLayoutInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHNOSTENCIL].pushConstantRangeCount = 1;
                       depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHNOSTENCIL].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                       depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHNOSTENCIL].front = {}; // Optional
                       depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHNOSTENCIL].back = {}; // Optional
                       frameBuffer.createGraphicPipeline(static_cast<sf::PrimitiveType>(i), states, NODEPTHNOSTENCIL, NBDEPTHSTENCIL);


                   } else if (j == 1) {
                       frameBuffer.enableStencilTest(true);
                       depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCIL].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                       depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCIL].back.compareOp = VK_COMPARE_OP_ALWAYS;
                       depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCIL].back.failOp = VK_STENCIL_OP_REPLACE;
                       depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCIL].back.depthFailOp = VK_STENCIL_OP_REPLACE;
                       depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCIL].back.passOp = VK_STENCIL_OP_REPLACE;
                       depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCIL].back.compareMask = 0xff;
                       depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCIL].back.writeMask = 0xff;
                       depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCIL].back.reference = 1;
                       depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCIL].front = depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCIL].back;
                       VkPushConstantRange push_constant;
                       //this push constant range starts at the beginning
                       push_constant.offset = 0;
                       //this push constant range takes up the size of a MeshPushConstants struct
                       push_constant.size = sizeof(IndirectDrawPushConsts);
                       //this push constant range is accessible only in the vertex shader
                       push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                       pipelineLayoutInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCIL].pPushConstantRanges = &push_constant;
                       pipelineLayoutInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCIL].pushConstantRangeCount = 1;
                       frameBuffer.createGraphicPipeline(static_cast<sf::PrimitiveType>(i), states, NODEPTHSTENCIL, NBDEPTHSTENCIL);

                   } else {
                       frameBuffer.enableStencilTest(true);

                       depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCILOUTLINE].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                       depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCILOUTLINE].back.compareOp = VK_COMPARE_OP_NOT_EQUAL;
                       depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCILOUTLINE].back.failOp = VK_STENCIL_OP_KEEP;
                       depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCILOUTLINE].back.depthFailOp = VK_STENCIL_OP_KEEP;
                       depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCILOUTLINE].back.passOp = VK_STENCIL_OP_REPLACE;
                       depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCILOUTLINE].back.compareMask = 0xff;
                       depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCILOUTLINE].back.writeMask = 0xff;
                       depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCILOUTLINE].back.reference = 1;
                       depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCILOUTLINE].front = depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCILOUTLINE].back;
                       VkPushConstantRange push_constant;
                       //this push constant range starts at the beginning
                       push_constant.offset = 0;
                       //this push constant range takes up the size of a MeshPushConstants struct
                       push_constant.size = sizeof(IndirectDrawPushConsts);
                       //this push constant range is accessible only in the vertex shader
                       push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                       pipelineLayoutInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCILOUTLINE].pPushConstantRanges = &push_constant;
                       pipelineLayoutInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCILOUTLINE].pushConstantRangeCount = 1;
                       frameBuffer.createGraphicPipeline(static_cast<sf::PrimitiveType>(i), states, NODEPTHSTENCILOUTLINE, NBDEPTHSTENCIL);
                   }
                }
            }
            states.shader = &perPixelLinkedListP2;
            for (unsigned int j = 0; j < NBDEPTHSTENCIL; j++) {
                for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes - 1; i++) {
                    if (j == 0) {
                       frameBuffer.enableStencilTest(false);

                       VkPushConstantRange push_constant;
                       push_constant.offset = 0;
                       //this push constant range takes up the size of a MeshPushConstants struct
                       push_constant.size = sizeof(Ppll2PushConsts);
                       //this push constant range is accessible only in the vertex shader
                       push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                       pipelineLayoutInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHNOSTENCIL].pPushConstantRanges = &push_constant;
                       pipelineLayoutInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHNOSTENCIL].pushConstantRangeCount = 1;
                       depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHNOSTENCIL].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                       depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHNOSTENCIL].front = {}; // Optional
                       depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHNOSTENCIL].back = {}; // Optional
                       frameBuffer.createGraphicPipeline(static_cast<sf::PrimitiveType>(i), states, NODEPTHNOSTENCIL, NBDEPTHSTENCIL);
                   } else if (j == 1) {
                       frameBuffer.enableStencilTest(true);

                       depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCIL].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                       depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCIL].back.compareOp = VK_COMPARE_OP_ALWAYS;
                       depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCIL].back.failOp = VK_STENCIL_OP_REPLACE;
                       depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCIL].back.depthFailOp = VK_STENCIL_OP_REPLACE;
                       depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCIL].back.passOp = VK_STENCIL_OP_REPLACE;
                       depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCIL].back.compareMask = 0xff;
                       depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCIL].back.writeMask = 0xff;
                       depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCIL].back.reference = 1;
                       depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCIL].front = depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCIL].back;
                       VkPushConstantRange push_constant;
                       push_constant.offset = 0;
                       //this push constant range takes up the size of a MeshPushConstants struct
                       push_constant.size = sizeof(Ppll2PushConsts);
                       //this push constant range is accessible only in the vertex shader
                       push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                       pipelineLayoutInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCIL].pPushConstantRanges = &push_constant;
                       pipelineLayoutInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCIL].pushConstantRangeCount = 1;
                       frameBuffer.createGraphicPipeline(static_cast<sf::PrimitiveType>(i), states, NODEPTHSTENCIL, NBDEPTHSTENCIL);
                    } else {
                       frameBuffer.enableStencilTest(true);
                       depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCILOUTLINE].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                       depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCILOUTLINE].back.compareOp = VK_COMPARE_OP_NOT_EQUAL;
                       depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCILOUTLINE].back.failOp = VK_STENCIL_OP_KEEP;
                       depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCILOUTLINE].back.depthFailOp = VK_STENCIL_OP_KEEP;
                       depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCILOUTLINE].back.passOp = VK_STENCIL_OP_REPLACE;
                       depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCILOUTLINE].back.compareMask = 0xff;
                       depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCILOUTLINE].back.writeMask = 0xff;
                       depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCILOUTLINE].back.reference = 1;
                       depthStencilCreateInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCILOUTLINE].front = depthStencilCreateInfo[indirectRenderingShader.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCILOUTLINE].back;
                       VkPushConstantRange push_constant;
                       push_constant.offset = 0;
                       //this push constant range takes up the size of a MeshPushConstants struct
                       push_constant.size = sizeof(Ppll2PushConsts);
                       //this push constant range is accessible only in the vertex shader
                       push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                       pipelineLayoutInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCILOUTLINE].pPushConstantRanges = &push_constant;
                       pipelineLayoutInfo[perPixelLinkedListP2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][frameBuffer.getId()][NODEPTHSTENCILOUTLINE].pushConstantRangeCount = 1;
                       frameBuffer.createGraphicPipeline(static_cast<sf::PrimitiveType>(i), states, NODEPTHSTENCILOUTLINE, NBDEPTHSTENCIL);
                    }
                }
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
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = headPtrTextureImage;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = VK_FORMAT_R32_UINT;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;
            if (vkCreateImageView(vkDevice.getDevice(), &viewInfo, nullptr, &headPtrTextureImageView) != VK_SUCCESS) {
                throw std::runtime_error("failed to create head ptr texture image view!");
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
            if (vkCreateSampler(vkDevice.getDevice(), &samplerInfo, nullptr, &headPtrTextureSampler) != VK_SUCCESS) {
                throw std::runtime_error("failed to create texture sampler!");
            }

        }
        void PerPixelLinkedListRenderComponent::clear() {
            frameBuffer.clear(sf::Color::Transparent);
            //firstDraw = true;
            frameBuffer.display();
            frameBuffer.beginRecordCommandBuffers();
            unsigned int currentFrame = frameBuffer.getCurrentFrame();
            VkClearColorValue clearColor;
            clearColor.uint32[0] = 0xffffffff;
            for (unsigned int i = 0; i < frameBuffer.getCommandBuffers().size(); i++) {
                VkImageSubresourceRange subresRange = {};
                subresRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                subresRange.levelCount = 1;
                subresRange.layerCount = 1;
                vkCmdClearColorImage(frameBuffer.getCommandBuffers()[i], headPtrTextureImage, VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &subresRange);
                vkCmdFillBuffer(frameBuffer.getCommandBuffers()[i], counterShaderStorageBuffers[i], 0, sizeof(uint32_t), 0);
                VkMemoryBarrier memoryBarrier;
                memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                memoryBarrier.pNext = VK_NULL_HANDLE;
                memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                vkCmdPipelineBarrier(frameBuffer.getCommandBuffers()[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

            }
            frameBuffer.beginRenderPass();
            frameBuffer.display();

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
        void PerPixelLinkedListRenderComponent::createDescriptorPool(RenderStates states) {
            Shader* shader = const_cast<Shader*>(states.shader);
            if (shader->getNbShaders()*frameBuffer.getNbRenderTargets() > descriptorPool.size())
                descriptorPool.resize(shader->getNbShaders()*frameBuffer.getNbRenderTargets());
            unsigned int descriptorId = frameBuffer.getId() * shader->getNbShaders() + shader->getId();
            //std::cout<<"ppll descriptor id : "<<frameBuffer.getId()<<","<<shader->getId()<<","<<frameBuffer.getId() * shader->getNbShaders() + shader->getId()<<std::endl;
            std::vector<Texture*> allTextures = Texture::getAllTextures();
            std::array<VkDescriptorPoolSize, 6> poolSizes;
            poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            poolSizes[0].descriptorCount = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight());
            poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            poolSizes[1].descriptorCount = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight());
            poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            poolSizes[2].descriptorCount = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight());
            poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSizes[3].descriptorCount = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight() * allTextures.size());
            poolSizes[4].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            poolSizes[4].descriptorCount = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight());
            poolSizes[5].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            poolSizes[5].descriptorCount = static_cast<uint32_t>(frameBuffer.getMaxFramesInFlight());

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
            Shader* shader = const_cast<Shader*>(states.shader);
            if (shader->getNbShaders()*frameBuffer.getNbRenderTargets() > descriptorSetLayout.size())
                descriptorSetLayout.resize(shader->getNbShaders()*frameBuffer.getNbRenderTargets());
            unsigned int descriptorId = frameBuffer.getId() * shader->getNbShaders() + shader->getId();
            //std::cout<<"ppll descriptor id : "<<descriptorId<<std::endl;
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

            VkDescriptorSetLayoutBinding samplerLayoutBinding{};
            samplerLayoutBinding.binding = 3;
            samplerLayoutBinding.descriptorCount = allTextures.size();
            samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            samplerLayoutBinding.pImmutableSamplers = nullptr;
            samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

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

            if (descriptorSetLayout[descriptorId] != nullptr) {
                vkDestroyDescriptorSetLayout(vkDevice.getDevice(), descriptorSetLayout[descriptorId], nullptr);
            }

            std::array<VkDescriptorSetLayoutBinding, 6> bindings = {counterLayoutBinding, headPtrImageLayoutBinding, linkedListLayoutBinding, samplerLayoutBinding, modelDataLayoutBinding, materialDataLayoutBinding};
            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            //layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
            layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());;
            layoutInfo.pBindings = bindings.data();

            if (vkCreateDescriptorSetLayout(vkDevice.getDevice(), &layoutInfo, nullptr, &descriptorSetLayout[descriptorId]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor set layout!");
            }
        }
        void PerPixelLinkedListRenderComponent::allocateDescriptorSets(RenderStates states) {
            Shader* shader = const_cast<Shader*>(states.shader);
            if (shader->getNbShaders()*frameBuffer.getNbRenderTargets() > descriptorSets.size())
                descriptorSets.resize(shader->getNbShaders()*frameBuffer.getNbRenderTargets());
            unsigned int descriptorId = frameBuffer.getId() * shader->getNbShaders() + shader->getId();
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
        void PerPixelLinkedListRenderComponent::createDescriptorSets(unsigned int p, RenderStates states) {
            Shader* shader = const_cast<Shader*>(states.shader);
            std::vector<Texture*> allTextures = Texture::getAllTextures();

            unsigned int descriptorId = frameBuffer.getId() * shader->getNbShaders() + shader->getId();
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
                headPtrDescriptorImageInfo.imageView = headPtrTextureImageView;
                headPtrDescriptorImageInfo.sampler = headPtrTextureSampler;

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


                descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[3].dstSet = descriptorSets[descriptorId][i];
                descriptorWrites[3].dstBinding = 3;
                descriptorWrites[3].dstArrayElement = 0;
                descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrites[3].descriptorCount = allTextures.size();
                descriptorWrites[3].pImageInfo = descriptorImageInfos.data();

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
        void PerPixelLinkedListRenderComponent::createDescriptorPool2(RenderStates states) {
            Shader* shader = const_cast<Shader*>(states.shader);
            if (shader->getNbShaders()*frameBuffer.getNbRenderTargets() > descriptorPool.size())
                descriptorPool.resize(shader->getNbShaders()*frameBuffer.getNbRenderTargets());
            unsigned int descriptorId = frameBuffer.getId() * shader->getNbShaders() + shader->getId();
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
            Shader* shader = const_cast<Shader*>(states.shader);
            if (shader->getNbShaders()*frameBuffer.getNbRenderTargets() > descriptorSetLayout.size())
                descriptorSetLayout.resize(shader->getNbShaders()*frameBuffer.getNbRenderTargets());
            unsigned int descriptorId = frameBuffer.getId() * shader->getNbShaders() + shader->getId();
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
            Shader* shader = const_cast<Shader*>(states.shader);
            if (shader->getNbShaders()*frameBuffer.getNbRenderTargets() > descriptorSets.size())
                descriptorSets.resize(shader->getNbShaders()*frameBuffer.getNbRenderTargets());
            unsigned int descriptorId = frameBuffer.getId() * shader->getNbShaders() + shader->getId();
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
            Shader* shader = const_cast<Shader*>(states.shader);

            unsigned int descriptorId = frameBuffer.getId() * shader->getNbShaders() + shader->getId();
            for (size_t i = 0; i < frameBuffer.getMaxFramesInFlight(); i++) {

                std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

                VkDescriptorImageInfo headPtrDescriptorImageInfo;
                headPtrDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                headPtrDescriptorImageInfo.imageView = headPtrTextureImageView;
                headPtrDescriptorImageInfo.sampler = headPtrTextureSampler;

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
        void PerPixelLinkedListRenderComponent::compileShaders() {
            const std::string indirectDrawVertexShader = R"(#version 460
                                                            #define M_PI 3.1415926535897932384626433832795
                                                            #define FPI M_PI/4
                                                            #extension GL_EXT_debug_printf : enable
                                                            layout (location = 0) in vec3 position;
                                                            layout (location = 1) in vec4 color;
                                                            layout (location = 2) in vec2 texCoords;
                                                            layout (location = 3) in vec3 normals;
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
                                                                uint textureIndex;
                                                                uint materialType;
                                                            };
                                                            layout(set = 0, binding = 4) buffer modelData {
                                                                ModelData modelDatas[];
                                                            };
                                                            layout(set = 0, binding = 5) buffer materialData {
                                                                MaterialData materialDatas[];
                                                            };
                                                            layout(location = 0) out vec4 frontColor;
                                                            layout(location = 1) out vec2 fTexCoords;
                                                            layout(location = 2) out uint texIndex;
                                                            layout(location = 3) out vec3 normal;
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
                                                                fTexCoords = texCoords;
                                                                frontColor = color;
                                                                texIndex = textureIndex;
                                                                normal = normals;
                                                            }
                                                            )";
                const std::string  simpleVertexShader = R"(#version 460
                                                        layout (location = 0) in vec3 position;
                                                        layout (location = 1) in vec4 color;
                                                        layout (location = 2) in vec2 texCoords;
                                                        layout (location = 3) in vec3 normals;
                                                        layout(location = 0) out vec4 frontColor;
                                                        layout(location = 1) out vec2 fTexCoords;
                                                        layout(location = 2) out vec3 normal;
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
                                                        })";
                const std::string fragmentShader = R"(#version 460
                                                      #extension GL_ARB_separate_shader_objects : enable
                                                      #extension GL_EXT_nonuniform_qualifier : enable
                                                      #extension GL_EXT_debug_printf : enable
                                                      struct NodeType {
                                                          vec4 color;
                                                          float depth;
                                                          uint next;
                                                      };
                                                      layout(set = 0, binding = 0) buffer CounterSSBO {
                                                          uint count;
                                                          uint maxNodes;
                                                      };
                                                      layout(set = 0, binding = 1, r32ui) uniform coherent uimage2D headPointers;
                                                      layout(set = 0, binding = 2) buffer LinkedLists {
                                                          NodeType nodes[];
                                                      };
                                                      layout(set = 0, binding = 3) uniform sampler2D textures[];
                                                      layout(location = 0) in vec4 frontColor;
                                                      layout(location = 1) in vec2 fTexCoords;
                                                      layout(location = 2) in flat uint texIndex;
                                                      layout(location = 3) in vec3 normal;
                                                      void main() {
                                                           uint nodeIdx = atomicAdd(count, 1);
                                                           vec4 color = (texIndex != 0) ? frontColor * textureLod(textures[texIndex-1], fTexCoords.xy, 0) : frontColor;
                                                           if (nodeIdx < maxNodes) {
                                                                uint prevHead = imageAtomicExchange(headPointers, ivec2(gl_FragCoord.xy), nodeIdx);
                                                                nodes[nodeIdx].color = color;
                                                                nodes[nodeIdx].depth = gl_FragCoord.z;
                                                                nodes[nodeIdx].next = prevHead;
                                                                //debugPrintfEXT("prev head : %i, next : %i, node Idx : %i\n", prevHead, nodes[nodeIdx].next, nodeIdx);
                                                           }
                                                      })";
                 const std::string fragmentShader2 =
                                                   R"(
                                                   #version 460
                                                   #extension GL_ARB_separate_shader_objects : enable
                                                   #extension GL_EXT_debug_printf : enable
                                                   #define MAX_FRAGMENTS 20
                                                   struct NodeType {
                                                      vec4 color;
                                                      float depth;
                                                      uint next;
                                                   };
                                                   layout(set = 0, binding = 0, r32ui) uniform uimage2D headPointers;
                                                   layout(set = 0, binding = 1) buffer linkedLists {
                                                       NodeType nodes[];
                                                   };
                                                   layout(location = 0) in vec4 frontColor;
                                                   layout(location = 1) in vec2 fTexCoords;
                                                   layout(location = 2) in vec3 normal;
                                                   layout(location = 0) out vec4 fcolor;
                                                   void main() {
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
                                                      fcolor = color;
                                                   })";
            if (!indirectRenderingShader.loadFromMemory(indirectDrawVertexShader, fragmentShader)) {
                throw core::Erreur(57, "Failed to load indirect rendering shader");
            }
            if (!perPixelLinkedListP2.loadFromMemory(simpleVertexShader, fragmentShader2)) {
                throw core::Erreur(55, "Failed to load per pixel linked list pass 2 shader");
            }


            math::Matrix4f viewMatrix = getWindow().getDefaultView().getViewMatrix().getMatrix()/*.transpose()*/;
            math::Matrix4f projMatrix = getWindow().getDefaultView().getProjMatrix().getMatrix()/*.transpose()*/;
            ppll2PushConsts.viewMatrix = viewMatrix;
            ppll2PushConsts.projMatrix = projMatrix;
            //ppll2PushConsts.projMatrix.m22 *= -1;
            indirectDrawPushConsts.resolution = resolution;
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
                    //std::cout<<"next frame draw normal"<<std::endl;
                    float time = timeClock.getElapsedTime().asSeconds();
                    indirectDrawPushConsts.time = time;

                    unsigned int p = m_normals[i].getAllVertices().getPrimitiveType();

                    /*if (m_normals[i].getVertexArrays()[0]->getEntity()->getRootType() == "E_MONSTER") {
                            std::cout<<"tex coords : "<<(*m_normals[i].getVertexArrays()[0])[0].texCoords.x<<","<<(*m_normals[i].getVertexArrays()[0])[0].texCoords.y<<std::endl;
                        }*/
                    unsigned int vertexCount = 0;
                    MaterialData material;
                    material.textureIndex = (m_normals[i].getMaterial().getTexture() != nullptr) ? m_normals[i].getMaterial().getTexture()->getId() : 0;
                    material.materialType = m_normals[i].getMaterial().getType();
                    materialDatas[p].push_back(material);
                    for (unsigned int j = 0; j < m_normals[i].getAllVertices().getVertexCount(); j++) {
                        vbBindlessTex[p].append(m_normals[i].getAllVertices()[j]);
                        vertexCount++;
                    }
                    TransformMatrix tm;
                    ModelData modelData;
                    modelData.worldMat = tm.getMatrix()/*.transpose()*/;

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
                                std::cout<<"position hero : "<<(*m_normals[i].getVertexArrays()[j])[n].position.x<<","<<(*m_normals[i].getVertexArrays()[j])[n].position.y<<","<<(*m_normals[i].getVertexArrays()[j])[n].position.z<<std::endl;
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
                        modelData.worldMat = tm[j]->getMatrix()/*.transpose()*/;
                        modelDatas[p].push_back(modelData);
                    }
                    MaterialData materialData;
                    materialData.textureIndex = (m_instances[i].getMaterial().getTexture() != nullptr) ? m_instances[i].getMaterial().getTexture()->getId() : 0;
                    materialData.materialType = m_instances[i].getMaterial().getType();
                    materialDatas[p].push_back(materialData);
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
                    //std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                    //std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;

                }
            }
            RenderStates currentStates;
            currentStates.blendMode = sf::BlendNone;
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
                    createCommandBuffersIndirect(p, drawArraysIndirectCommands[p].size(), sizeof(DrawArraysIndirectCommand), NODEPTHNOSTENCIL, currentStates);

                }
            }
        }
        void PerPixelLinkedListRenderComponent::drawInstancesIndexed() {
            for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                vbBindlessTex[i].clear();
                materialDatas[i].clear();
                modelDatas[i].clear();
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
                    float time = timeClock.getElapsedTime().asSeconds();
                    indirectDrawPushConsts.time = time;
                    unsigned int p = m_normalsIndexed[i].getAllVertices().getPrimitiveType();

                    MaterialData material;
                    material.textureIndex = (m_normalsIndexed[i].getMaterial().getTexture() != nullptr) ? m_normalsIndexed[i].getMaterial().getTexture()->getId() : 0;
                    material.materialType = m_normalsIndexed[i].getMaterial().getType();
                    materialDatas[p].push_back(material);

                    TransformMatrix tm;
                    ModelData modelData;
                    modelData.worldMat = tm.getMatrix()/*.transpose()*/;
                    modelDatas[p].push_back(modelData);
                    unsigned int indexCount = 0, vertexCount = 0;
                    for (unsigned int j = 0; j < m_normalsIndexed[i].getAllVertices().getVertexCount(); j++) {
                        vbBindlessTex[p].append(m_normalsIndexed[i].getAllVertices()[j]);
                        vertexCount++;
                    }
                    for (unsigned int j = 0; j < m_normalsIndexed[i].getAllVertices().getIndexes().size(); j++) {
                        vbBindlessTex[p].addIndex(m_normalsIndexed[i].getAllVertices().getIndexes()[j]);
                        indexCount++;
                        //std::cout<<"index : "<<m_normalsIndexed[i].getAllVertices().getIndexes()[j]<<std::endl;
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
                    materialDatas[p].push_back(material);

                    std::vector<TransformMatrix*> tm = m_instancesIndexed[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData model;
                        model.worldMat = tm[j]->getMatrix()/*.transpose()*/;
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
                    //std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                    //std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;
                }
            }
            RenderStates currentStates;
            currentStates.blendMode = sf::BlendNone;
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
                    //std::cout<<"size : "<<sizeof(DrawElementsIndirectCommand)<<std::endl;
                    createCommandBuffersIndirect(p, drawElementsIndirectCommands[p].size(), sizeof(DrawElementsIndirectCommand), NODEPTHNOSTENCIL, currentStates);

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
                    //std::cout<<"next frame draw normal"<<std::endl;
                    float time = timeClock.getElapsedTime().asSeconds();
                    indirectDrawPushConsts.time = time;

                    unsigned int p = m_selected[i].getAllVertices().getPrimitiveType();

                    /*if (m_selecteds[i].getVertexArrays()[0]->getEntity()->getRootType() == "E_MONSTER") {
                            std::cout<<"tex coords : "<<(*m_selecteds[i].getVertexArrays()[0])[0].texCoords.x<<","<<(*m_selecteds[i].getVertexArrays()[0])[0].texCoords.y<<std::endl;
                        }*/
                    unsigned int vertexCount = 0;
                    MaterialData material;
                    material.textureIndex = (m_selected[i].getMaterial().getTexture() != nullptr) ? m_selected[i].getMaterial().getTexture()->getId() : 0;
                    material.materialType = m_selected[i].getMaterial().getType();
                    materialDatas[p].push_back(material);
                    for (unsigned int j = 0; j < m_selected[i].getAllVertices().getVertexCount(); j++) {
                        vbBindlessTex[p].append(m_selected[i].getAllVertices()[j]);
                        vertexCount++;
                    }
                    TransformMatrix tm;
                    ModelData modelData;
                    modelData.worldMat = tm.getMatrix()/*.transpose()*/;
                    modelDatas[p].push_back(modelData);

                    drawArraysIndirectCommand.count = vertexCount;
                    drawArraysIndirectCommand.firstIndex = firstIndex[p];
                    drawArraysIndirectCommand.baseInstance = baseInstance[p];
                    drawArraysIndirectCommand.instanceCount = 1;
                    drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                    firstIndex[p] += vertexCount;
                    baseInstance[p] += 1;
                    /*for (unsigned int j = 0; j < m_selecteds[i].getVertexArrays().size(); j++) {
                        if (m_selecteds[i].getVertexArrays()[j]->getEntity() != nullptr && m_selecteds[i].getVertexArrays()[j]->getEntity()->getRootType() == "E_HERO") {
                            for (unsigned int n = 0; n < m_selecteds[i].getVertexArrays()[j]->getVertexCount(); n++)
                                std::cout<<"position hero : "<<(*m_selecteds[i].getVertexArrays()[j])[n].position.x<<","<<(*m_selecteds[i].getVertexArrays()[j])[n].position.y<<","<<(*m_selecteds[i].getVertexArrays()[j])[n].position.z<<std::endl;
                        }
                    }*/
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
                        modelData.worldMat = tm[j]->getMatrix()/*.transpose()*/;
                        modelDatas[p].push_back(modelData);
                    }
                    MaterialData materialData;
                    materialData.textureIndex = (m_selectedInstance[i].getMaterial().getTexture() != nullptr) ? m_selectedInstance[i].getMaterial().getTexture()->getId() : 0;
                    materialData.materialType = m_selectedInstance[i].getMaterial().getType();
                    materialDatas[p].push_back(materialData);
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
                    //std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                    //std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;

                }
            }
            RenderStates currentStates;
            currentStates.blendMode = sf::BlendNone;
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
                    createCommandBuffersIndirect(p, drawArraysIndirectCommands[p].size(), sizeof(DrawArraysIndirectCommand), NODEPTHSTENCIL, currentStates);

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
                    //std::cout<<"next frame draw normal"<<std::endl;
                    /*if (core::Application::app != nullptr) {
                        float time = core::Application::getTimeClk().getElapsedTime().asSeconds();
                        perPixelLinkedList2.setParameter("time", time);
                    }*/
                    unsigned int p = m_selectedScale[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = 0;
                    material.materialType = m_selectedScale[i].getMaterial().getType();
                    materialDatas[p].push_back(material);
                    TransformMatrix tm;
                    ModelData model;
                    model.worldMat = tm.getMatrix()/*.transpose()*/;
                    modelDatas[p].push_back(model);

                    unsigned int vertexCount = 0;
                    for (unsigned int j = 0; j < m_selectedScale[i].getAllVertices().getVertexCount(); j++) {
                        vertexCount++;
                        vbBindlessTex[p].append(m_selectedScale[i].getAllVertices()[j]);
                        //std::cout<<"color : "<<(int) m_selectedScale[i].getAllVertices()[j].color.r<<","<<(int) m_selectedScale[i].getAllVertices()[j].color.g<<","<<(int) m_selectedScale[i].getAllVertices()[j].color.b<<std::endl;
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
                    material.materialType = m_selectedScaleInstance[i].getMaterial().getType();
                    materialDatas[p].push_back(material);
                    std::vector<TransformMatrix*> tm = m_selectedScaleInstance[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData model;
                        model.worldMat = tm[j]->getMatrix()/*.transpose()*/;
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
                    //std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                    //std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;

                }
            }
            currentStates.blendMode = sf::BlendNone;
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
                    createCommandBuffersIndirect(p, drawArraysIndirectCommands[p].size(), sizeof(DrawArraysIndirectCommand), NODEPTHSTENCILOUTLINE, currentStates);

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
                    //std::cout<<"next frame draw normal"<<std::endl;

                    float time = timeClock.getElapsedTime().asSeconds();
                    indirectDrawPushConsts.time = time;

                    unsigned int p = m_selectedIndexed[i].getAllVertices().getPrimitiveType();

                    MaterialData material;
                    material.textureIndex = (m_selectedIndexed[i].getMaterial().getTexture() != nullptr) ? m_selectedIndexed[i].getMaterial().getTexture()->getId() : 0;
                    material.materialType = m_selectedIndexed[i].getMaterial().getType();
                    materialDatas[p].push_back(material);

                    TransformMatrix tm;
                    ModelData model;
                    model.worldMat = tm.getMatrix()/*.transpose()*/;
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
                    materialDatas[p].push_back(material);
                    std::vector<TransformMatrix*> tm = m_selectedInstanceIndexed[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData model;
                        model.worldMat = tm[j]->getMatrix()/*.transpose()*/;
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
                    //std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                    //std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;
                }
            }
            RenderStates currentStates;
            currentStates.blendMode = sf::BlendNone;
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
                    //std::cout<<"size : "<<sizeof(DrawElementsIndirectCommand)<<std::endl;
                    createCommandBuffersIndirect(p, drawElementsIndirectCommands[p].size(), sizeof(DrawElementsIndirectCommand), NODEPTHNOSTENCIL, currentStates);

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
            for (unsigned int i = 0; i < drawElementsIndirectCommands.size(); i++) {
                drawElementsIndirectCommands[i].clear();
            }
            for (unsigned int i = 0; i < m_selectedScaleIndexed.size(); i++) {
                if (m_selectedScaleIndexed[i].getAllVertices().getVertexCount() > 0) {
                    DrawElementsIndirectCommand drawElementsIndirectCommand;
                    //std::cout<<"next frame draw normal"<<std::endl;
                    unsigned int p = m_selectedScaleIndexed[i].getAllVertices().getPrimitiveType();
                    MaterialData material;
                    material.textureIndex = 0;
                    material.materialType = m_selectedScaleIndexed[i].getMaterial().getType();
                    materialDatas[p].push_back(material);

                    TransformMatrix tm;
                    ModelData model;
                    model.worldMat = tm.getMatrix().transpose();
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
                    /*if (core::Application::app != nullptr) {
                        float time = core::Application::getTimeClk().getElapsedTime().asSeconds();
                        perPixelLinkedList.setParameter("time", time);
                    }*/
                    MaterialData material;
                    material.textureIndex = 0;
                    material.materialType = m_selectedScaleInstanceIndexed[i].getMaterial().getType();
                    materialDatas[p].push_back(material);
                    std::vector<TransformMatrix*> tm = m_selectedScaleInstanceIndexed[i].getTransforms();
                    for (unsigned int j = 0; j < tm.size(); j++) {
                        tm[j]->update();
                        ModelData model;
                        model.worldMat = tm[j]->getMatrix().transpose();
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
                    //std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                    //std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;
                }
            }
            currentStates.blendMode = sf::BlendNone;
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
                    //std::cout<<"size : "<<sizeof(DrawElementsIndirectCommand)<<std::endl;
                    createCommandBuffersIndirect(p, drawElementsIndirectCommands[p].size(), sizeof(DrawElementsIndirectCommand), NODEPTHNOSTENCIL, currentStates);

                }
            }
        }
        void PerPixelLinkedListRenderComponent::drawNextFrame() {
            {
                std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                if (datasReady) {
                    datasReady = false;
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
            math::Matrix4f projMatrix = view.getProjMatrix().getMatrix()/*.transpose()*/;
            math::Matrix4f viewMatrix = view.getViewMatrix().getMatrix()/*.transpose()*/;
            indirectDrawPushConsts.projMatrix = projMatrix;
            indirectDrawPushConsts.viewMatrix = viewMatrix;
            //indirectDrawPushConsts.projMatrix.m22 *= -1;



            drawInstances();
            drawInstancesIndexed();
            drawSelectedInstances();
            drawSelectedInstancesIndexed();



            vb.clear();
            vb.setPrimitiveType(sf::Triangles);
            Vertex v1 (sf::Vector3f(0, 0, quad.getSize().z));
            Vertex v2 (sf::Vector3f(quad.getSize().x,0, quad.getSize().z));
            Vertex v3 (sf::Vector3f(quad.getSize().x, quad.getSize().y, quad.getSize().z));
            Vertex v4 (sf::Vector3f(0, quad.getSize().y, quad.getSize().z));
            vb.append(v1);
            vb.append(v2);
            vb.append(v3);
            vb.append(v1);
            vb.append(v3);
            vb.append(v4);
            vb.update();
            math::Matrix4f matrix = quad.getTransform().getMatrix()/*.transpose()*/;
            ppll2PushConsts.worldMat = matrix;
            RenderStates currentStates;
            currentStates.shader = &perPixelLinkedListP2;
            currentStates.blendMode = sf::BlendNone;
            frameBuffer.enableStencilTest(false);
            //createDescriptorSets2(currentStates);
            createCommandBufferVertexBuffer(currentStates);

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
        void PerPixelLinkedListRenderComponent::createCommandBuffersIndirect(unsigned int p, unsigned int nbIndirectCommands, unsigned int stride, DepthStencilID depthStencilID, RenderStates currentStates) {

            if (needToUpdateDS) {
                createDescriptorSets(p, currentStates);
                needToUpdateDS = false;
            }
            unsigned int currentFrame = frameBuffer.getCurrentFrame();
            frameBuffer.beginRecordCommandBuffers();

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

            Shader* shader = const_cast<Shader*>(currentStates.shader);
            std::vector<Texture*> allTextures = Texture::getAllTextures();
            vkCmdResetEvent(frameBuffer.getCommandBuffers()[currentFrame], events[currentFrame],  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
            vkCmdSetEvent(frameBuffer.getCommandBuffers()[currentFrame], events[currentFrame],  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
            frameBuffer.beginRenderPass();

            vkCmdPushConstants(frameBuffer.getCommandBuffers()[currentFrame], frameBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][frameBuffer.getId()][depthStencilID], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(IndirectDrawPushConsts), &indirectDrawPushConsts);



            frameBuffer.drawIndirect(frameBuffer.getCommandBuffers()[currentFrame], currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], vboIndirect, depthStencilID,currentStates);



            /*frameBuffer.beginRecordCommandBuffers();
            frameBuffer.beginRenderPass();
            vkCmdExecuteCommands(frameBuffer.getCommandBuffers()[currentFrame], static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());*/
            frameBuffer.display();

        }
        void PerPixelLinkedListRenderComponent::createCommandBufferVertexBuffer(RenderStates currentStates) {
            frameBuffer.beginRecordCommandBuffers();

            unsigned int currentFrame = frameBuffer.getCurrentFrame();
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
            Shader* shader = const_cast<Shader*>(currentStates.shader);

            //for (size_t i = 0; i < commandBuffers.size(); i++) {
                /*vkResetCommandBuffer(commandBuffers[currentFrame], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

                if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                }*/
                /*std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

                VkDescriptorImageInfo headPtrDescriptorImageInfo;
                headPtrDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                headPtrDescriptorImageInfo.imageView = headPtrTextureImageView;
                headPtrDescriptorImageInfo.sampler = headPtrTextureSampler;

                descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[0].dstSet = 0;
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
                descriptorWrites[1].dstSet = 0;
                descriptorWrites[1].dstBinding = 1;
                descriptorWrites[1].dstArrayElement = 0;
                descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                descriptorWrites[1].descriptorCount = 1;
                descriptorWrites[1].pBufferInfo = &linkedListStorageBufferInfoLastFrame;*/
                vkCmdPipelineBarrier(frameBuffer.getCommandBuffers()[currentFrame], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);


                VkMemoryBarrier memoryBarrier;
                memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                memoryBarrier.pNext = VK_NULL_HANDLE;
                memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

                vkCmdWaitEvents(frameBuffer.getCommandBuffers()[currentFrame], 1, &events[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

                //vkCmdPipelineBarrier(frameBuffer.getCommandBuffers()[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                //vkCmdPushDescriptorSetKHR(frameBuffer.getCommandBuffers()[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, frameBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + vb.getPrimitiveType()][frameBuffer.getId()][NODEPTHNOSTENCIL], 0, 2, descriptorWrites.data());

                frameBuffer.beginRenderPass();

                vkCmdPushConstants(frameBuffer.getCommandBuffers()[currentFrame], frameBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + vb.getPrimitiveType()][frameBuffer.getId()][NODEPTHNOSTENCIL], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Ppll2PushConsts), &ppll2PushConsts);

                frameBuffer.drawVertexBuffer(frameBuffer.getCommandBuffers()[currentFrame], currentFrame, vb, NODEPTHNOSTENCIL, currentStates);


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
            frameBuffer.display();

        }
        bool PerPixelLinkedListRenderComponent::loadEntitiesOnComponent(std::vector<Entity*> vEntities) {
            {
                std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                datasReady = false;
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
                            if (vEntities[i]->getFace(j)->getVertexArray().getIndexes().size() == 0)
                                batcher.addFace( vEntities[i]->getFace(j));
                            else
                                batcherIndexed.addFace(vEntities[i]->getFace(j));
                         } else if (vEntities[i]->getDrawMode() == Entity::NORMAL && !vEntities[i]->isSelected()) {
                             if (vEntities[i]->getFace(j)->getVertexArray().getIndexes().size() == 0) {
                                normalBatcher.addFace( vEntities[i]->getFace(j));
                             } else
                                normalBatcherIndexed.addFace( vEntities[i]->getFace(j));
                        } else if (vEntities[i]->getDrawMode() == Entity::INSTANCED && vEntities[i]->isSelected()) {

                            if (vEntities[i]->getFace(j)->getVertexArray().getIndexes().size() == 0) {
                                selectedInstanceBatcher.addFace(vEntities[i]->getFace(j));
                           // std::cout<<"remove texture"<<std::endl;

                            //std::cout<<"get va"<<std::endl;
                                VertexArray& va = border->getFace(j)->getVertexArray();
                                //std::cout<<"change color"<<std::endl;
                                for (unsigned int j = 0; j < va.getVertexCount(); j++) {

                                    va[j].color = sf::Color::Cyan;
                                }
                                Entity* root = (vEntities[i]->getRootEntity()->isAnimated()) ? vEntities[i]->getRootEntity() : vEntities[i];
                                math::Vec3f oldSize = root->getSize();
                                border->setOrigin(border->getSize() * 0.5f);
                                border->setScale(math::Vec3f(1.1f, 1.1f, 1.1f));
                                math::Vec3f offset =  root->getSize() - oldSize;
                                border->setPosition(root->getPosition() - offset * 0.5f);
                               // std::cout<<"add to batcher"<<std::endl;
                                selectedInstanceScaleBatcher.addFace(border->getFace(j));
                           // std::cout<<"face added"<<std::endl;
                             } else {
                                 selectedInstanceIndexBatcher.addFace(vEntities[i]->getFace(j));
                               // std::cout<<"remove texture"<<std::endl;

                            //std::cout<<"get va"<<std::endl;
                                VertexArray& va = border->getFace(j)->getVertexArray();
                                //std::cout<<"change color"<<std::endl;
                                for (unsigned int j = 0; j < va.getVertexCount(); j++) {

                                    va[j].color = sf::Color::Cyan;
                                }
                                Entity* root = (vEntities[i]->getRootEntity()->isAnimated()) ? vEntities[i]->getRootEntity() : vEntities[i];
                                math::Vec3f oldSize = root->getSize();
                                border->setOrigin(root->getSize() * 0.5f);
                                border->setSize(root->getSize() * 1.1f);
                                math::Vec3f offset =  root->getSize() - oldSize;
                                border->setPosition(root->getPosition() - offset * 0.5f);

                               // std::cout<<"add to batcher"<<std::endl;
                                selectedInstanceIndexScaleBatcher.addFace(border->getFace(j));
                             }
                        } else {
                            if (vEntities[i]->getFace(j)->getVertexArray().getIndexes().size() == 0) {

                                selectedBatcher.addFace(vEntities[i]->getFace(j));
                           // std::cout<<"remove texture"<<std::endl;

                            //std::cout<<"get va"<<std::endl;
                                VertexArray& va = border->getFace(j)->getVertexArray();
                                //std::cout<<"change color"<<std::endl;
                                for (unsigned int j = 0; j < va.getVertexCount(); j++) {

                                    va[j].color = sf::Color::Cyan;
                                }
                                Entity* root = (vEntities[i]->getRootEntity()->isAnimated()) ? vEntities[i]->getRootEntity() : vEntities[i];
                                math::Vec3f oldSize = root->getSize();
                                border->setOrigin(root->getSize() * 0.5f);
                                border->setSize(root->getSize() * 1.1f);
                                math::Vec3f offset =  root->getSize() - oldSize;
                                border->setPosition(root->getPosition() - offset * 0.5f);
                                selectedScaleBatcher.addFace(border->getFace(j));

                               // std::cout<<"face added"<<std::endl;
                             } else {
                                 selectedIndexBatcher.addFace(vEntities[i]->getFace(j));
                               // std::cout<<"remove texture"<<std::endl;

                            //std::cout<<"get va"<<std::endl;
                                VertexArray& va = border->getFace(j)->getVertexArray();
                                //std::cout<<"change color"<<std::endl;
                                for (unsigned int j = 0; j < va.getVertexCount(); j++) {

                                    va[j].color = sf::Color::Cyan;
                                }

                                border->setOrigin(border->getSize() * 0.5f);
                                border->setScale(math::Vec3f(1.1f, 1.1f, 1.1f));
                               // std::cout<<"add to batcher"<<std::endl;
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

            //std::cout<<"instances added"<<std::endl;
            visibleEntities = vEntities;
            update = true;
            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
            datasReady = true;
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
                std::cout<<"recompute size"<<std::endl;
                recomputeSize();
                getListener().pushEvent(event);
                getView().reset(physic::BoundingBox(getView().getViewport().getPosition().x, getView().getViewport().getPosition().y, getView().getViewport().getPosition().z, event.window.data1, event.window.data2, getView().getViewport().getDepth()));
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
            frameBufferSprite.setCenter(target.getView().getPosition());
            //std::cout<<"view position : "<<view.getPosition()<<std::endl;
            //std::cout<<"sprite position : "<<frameBufferSprite.getCenter()<<std::endl;
            target.draw(frameBufferSprite, states);
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
        }
        PerPixelLinkedListRenderComponent::~PerPixelLinkedListRenderComponent() {
            std::cout<<"ppll destructor"<<std::endl;
            for (unsigned int i = 0; i < events.size(); i++) {
                vkDestroyEvent(vkDevice.getDevice(), events[i], nullptr);
            }
            vkDestroyCommandPool(vkDevice.getDevice(), commandPool, nullptr);
            vkDestroySampler(vkDevice.getDevice(), headPtrTextureSampler, nullptr);
            vkDestroyImageView(vkDevice.getDevice(), headPtrTextureImageView, nullptr);
            vkDestroyImage(vkDevice.getDevice(), headPtrTextureImage, nullptr);
            vkFreeMemory(vkDevice.getDevice(), headPtrTextureImageMemory, nullptr);

            std::cout<<"image destroyed"<<std::endl;
            for (size_t i = 0; i < counterShaderStorageBuffers.size(); i++) {
                if (counterShaderStorageBuffers[i] != VK_NULL_HANDLE) {
                    vkDestroyBuffer(vkDevice.getDevice(), counterShaderStorageBuffers[i], nullptr);
                    vkFreeMemory(vkDevice.getDevice(), counterShaderStorageBuffersMemory[i], nullptr);
                }
            }
            std::cout<<"counter ssbo destroyed"<<std::endl;
            for (unsigned int i = 0; i < linkedListShaderStorageBuffers.size(); i++) {
                vkDestroyBuffer(vkDevice.getDevice(), linkedListShaderStorageBuffers[i], nullptr);
                vkFreeMemory(vkDevice.getDevice(), linkedListShaderStorageBuffersMemory[i], nullptr);
            }
            std::cout<<"linked list ssbo destroyed"<<std::endl;
            for (size_t i = 0; i < modelDataShaderStorageBuffers.size(); i++) {
                vkDestroyBuffer(vkDevice.getDevice(), modelDataShaderStorageBuffers[i], nullptr);
                vkFreeMemory(vkDevice.getDevice(), modelDataShaderStorageBuffersMemory[i], nullptr);
            }
            if (modelDataStagingBuffer != nullptr) {
                vkDestroyBuffer(vkDevice.getDevice(), modelDataStagingBuffer, nullptr);
                vkFreeMemory(vkDevice.getDevice(), modelDataStagingBufferMemory, nullptr);
            }
            std::cout<<"model data ssbo destroyed"<<std::endl;
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
            std::cout<<"material data ssbo destroyed"<<std::endl;
            if (vboIndirect != VK_NULL_HANDLE) {
                vkDestroyBuffer(vkDevice.getDevice(),vboIndirect, nullptr);
                vkFreeMemory(vkDevice.getDevice(), vboIndirectMemory, nullptr);
            }
            std::cout<<"indirect vbo destroyed"<<std::endl;
        }
        #else
        PerPixelLinkedListRenderComponent::PerPixelLinkedListRenderComponent(RenderWindow& window, int layer, std::string expression, window::ContextSettings settings) :
            HeavyComponent(window, math::Vec3f(window.getView().getPosition().x, window.getView().getPosition().y, layer),
                          math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0),
                          math::Vec3f(window.getView().getSize().x + window.getView().getSize().x * 0.5f, window.getView().getPosition().y + window.getView().getSize().y * 0.5f, layer)),
            view(window.getView()),
            expression(expression),
            quad(math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, window.getSize().y * 0.5f)),
            layer(layer) {
            maxModelDataSize = maxMaterialDataSize = maxVboIndirectSize = 0;
            if (!(settings.versionMajor >= 4 && settings.versionMinor >= 6))
                throw core::Erreur(53, "opengl version not supported for this renderer type");
            //std::cout<<"move quad"<<std::endl;
            datasReady = false;
            quad.move(math::Vec3f(-window.getView().getSize().x * 0.5f, -window.getView().getSize().y * 0.5f, 0));
            maxNodes = 20 * window.getView().getSize().x * window.getView().getSize().y;
            GLint nodeSize = 5 * sizeof(GLfloat) + sizeof(GLuint);
            //std::cout<<"stencil bits : "<<settings.stencilBits<<std::endl;
            frameBuffer.create(window.getView().getSize().x, window.getView().getSize().y, settings);
            frameBufferSprite = Sprite(frameBuffer.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), sf::IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));
            frameBuffer.setView(view);
            resolution = sf::Vector3i((int) window.getSize().x, (int) window.getSize().y, window.getView().getSize().z);
            //window.setActive();
            glCheck(glGenBuffers(1, &atomicBuffer));
            glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicBuffer));
            glCheck(glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW));
            glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0));
            glCheck(glGenBuffers(1, &linkedListBuffer));
            glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, linkedListBuffer));
            glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, maxNodes * nodeSize, nullptr, GL_DYNAMIC_DRAW));
            glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
            glCheck(glGenTextures(1, &headPtrTex));
            glCheck(glBindTexture(GL_TEXTURE_2D, headPtrTex));
            glCheck(glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, window.getView().getSize().x, window.getView().getSize().y));
            glCheck(glBindImageTexture(0, headPtrTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI));
            glCheck(glBindTexture(GL_TEXTURE_2D, 0));
            std::vector<GLuint> headPtrClearBuf(window.getView().getSize().x*window.getView().getSize().y, 0xffffffff);
            glCheck(glGenBuffers(1, &clearBuf));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf));
            glCheck(glBufferData(GL_PIXEL_UNPACK_BUFFER, headPtrClearBuf.size() * sizeof(GLuint),
            &headPtrClearBuf[0], GL_STATIC_COPY));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
            //std::cout<<"buffers : "<<atomicBuffer<<" "<<linkedListBuffer<<" "<<headPtrTex<<" "<<clearBuf<<std::endl;
            core::FastDelegate<bool> signal (&PerPixelLinkedListRenderComponent::needToUpdate, this);
            core::FastDelegate<void> slot (&PerPixelLinkedListRenderComponent::drawNextFrame, this);
            core::Command cmd(signal, slot);
            getListener().connect("UPDATE", cmd);


            glCheck(glGenBuffers(1, &modelDataBuffer));
            glCheck(glGenBuffers(1, &materialDataBuffer));
            glCheck(glGenBuffers(1, &vboIndirect));
            compileShaders();

            std::vector<Texture*> allTextures = Texture::getAllTextures();
            Samplers allSamplers{};
            std::vector<math::Matrix4f> textureMatrices;
            for (unsigned int i = 0; i < allTextures.size(); i++) {
                textureMatrices.push_back(allTextures[i]->getTextureMatrix());
                GLuint64 handle_texture = allTextures[i]->getTextureHandle();
                allTextures[i]->makeTextureResident(handle_texture);
                allSamplers.tex[i].handle = handle_texture;
                //std::cout<<"add texture i : "<<i<<" id : "<<allTextures[i]->getId()<<std::endl;
            }
            indirectRenderingShader.setParameter("textureMatrix", textureMatrices);
            glCheck(glGenBuffers(1, &ubo));

            backgroundColor = sf::Color::Transparent;
            glCheck(glBindBuffer(GL_UNIFORM_BUFFER, ubo));
            glCheck(glBufferData(GL_UNIFORM_BUFFER, sizeof(Samplers),allSamplers.tex, GL_STATIC_DRAW));
            glCheck(glBindBuffer(GL_UNIFORM_BUFFER, 0));
            //std::cout<<"size : "<<sizeof(Samplers)<<" "<<alignof (alignas(16) uint64_t[200])<<std::endl;
            backgroundColor = sf::Color::Transparent;
            glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
            glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicBuffer));
            glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, linkedListBuffer));
            glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, modelDataBuffer));
            glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, materialDataBuffer));

            for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                vbBindlessTex[i].setPrimitiveType(static_cast<sf::PrimitiveType>(i));
            }
            skybox = nullptr;
            frameBuffer.setActive(false);
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
                //std::cout<<"add texture i : "<<i<<" id : "<<allTextures[i]->getId()<<std::endl;
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
                                                                    //vec3 localNormal = mat3(finalBonesMatrices[boneIds[i]]) * normals;
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
                                                           vec4 color = (texIndex != 0) ? frontColor * texture(textures[texIndex-1], fTexCoords.xy) : frontColor;
                                                           //beginInvocationInterlockARB();
                                                           if (nodeIdx < maxNodes) {
                                                                uint prevHead = imageAtomicExchange(headPointers, ivec2(gl_FragCoord.xy), nodeIdx);
                                                                nodes[nodeIdx].color = color;
                                                                nodes[nodeIdx].depth = (color == vec4(0, 1, 1, 1)) ? 1.0f : gl_FragCoord.z;
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
                   struct NodeType {
                      vec4 color;
                      float depth;
                      uint next;
                   };
                   layout(binding = 0, r32ui) uniform uimage2D headPointers;
                   layout(std430, binding = 0) buffer linkedLists {
                       NodeType nodes[];
                   };
                   layout(location = 0) out vec4 fcolor;
                   void main() {
                      NodeType frags[MAX_FRAGMENTS];
                      int count = 0;
                      uint n = imageLoad(headPointers, ivec2(gl_FragCoord.xy)).r;
                      while( n != 0xffffffffu && count < MAX_FRAGMENTS) {
                           frags[count] = nodes[n];
                           n = frags[count].next;
                           count++;
                      }
                      //merge sort
                      /*int i, j1, j2, k;
                      int a, b, c;
                      int step = 1;
                      NodeType leftArray[MAX_FRAGMENTS/2]; //for merge sort
                      //NodeType fgs[2];
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
                                  //bool idx = (b + j1 >= c || (j2 < step && leftArray[j2].depth > frags[b + j1].depth));
                                  //fgs[1] = leftArray[j2++];
                                  //fgs[0] = frags[b + j1++];
                                  //frags[k] = fgs[int(idx)];
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
                        //color = mix (color, frags[i].color, frags[i].color.a);
                      }
                      fcolor = color;
                   })";
                   if (!skyboxShader.loadFromMemory(skyboxVertexShader, skyboxFragmentShader)) {
                        throw core::Erreur(53, "Failed to load skybox shader");
                   }
                   if (!perPixelLinkedListP2.loadFromMemory(simpleVertexShader, fragmentShader2)) {
                        throw core::Erreur(55, "Failed to load per pixel linked list pass 2 shader");
                   }
                   if (!indirectRenderingShader.loadFromMemory(indirectDrawVertexShader, fragmentShader)) {
                       throw core::Erreur(57, "Failed to load indirect rendering shader");
                   }
                   skyboxShader.setParameter("skybox", Shader::CurrentTexture);
                   indirectRenderingShader.setParameter("maxNodes", maxNodes);
                   indirectRenderingShader.setParameter("currentTex", Shader::CurrentTexture);
                   indirectRenderingShader.setParameter("resolution", resolution.x, resolution.y, resolution.z);
                   math::Matrix4f viewMatrix = getWindow().getDefaultView().getViewMatrix().getMatrix().transpose();
                   math::Matrix4f projMatrix = getWindow().getDefaultView().getProjMatrix().getMatrix().transpose();
                   perPixelLinkedListP2.setParameter("viewMatrix", viewMatrix);
                   perPixelLinkedListP2.setParameter("projectionMatrix", projMatrix);
        }
        void PerPixelLinkedListRenderComponent::setBackgroundColor(sf::Color color) {
            backgroundColor = color;
        }
        void PerPixelLinkedListRenderComponent::clear() {
            //frameBuffer.setActive();
            frameBuffer.clear(backgroundColor);
            //getWindow().setActive();
            GLuint zero = 0;
            glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicBuffer));
            glCheck(glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero));
            glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf));
            glCheck(glBindTexture(GL_TEXTURE_2D, headPtrTex));
            glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, view.getSize().x, view.getSize().y, GL_RED_INTEGER,
            GL_UNSIGNED_INT, NULL));
            glCheck(glBindTexture(GL_TEXTURE_2D, 0));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));

            frameBuffer.resetGLStates();


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
                    //std::cout<<"next frame draw normal"<<std::endl;

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
                    //std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                    //std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;

                }
            }
            currentStates.blendMode = sf::BlendNone;
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
                    //std::cout<<"next frame draw normal"<<std::endl;
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
                    //std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                    //std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;

                }
            }
            currentStates.blendMode = sf::BlendNone;
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
                    //std::cout<<"next frame draw normal"<<std::endl;

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
                    //std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                    //std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;
                }
            }
            currentStates.blendMode = sf::BlendNone;
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
                    //std::cout<<"next frame draw normal"<<std::endl;
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
                    //std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                    //std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;
                }
            }
            currentStates.blendMode = sf::BlendNone;
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
            //std::cout<<"draw nex frame"<<std::endl;
            //basicView.setPerspective(-1, 1, -1, 1, 0, 1);


            {

                std::lock_guard<std::recursive_mutex> lock(rec_mutex);


                /*if (!datasReady) {
                    //std::cout<<"wait"<<std::endl;
                    cv.wait(lock, [this] { return datasReady; });


                }
                datasReady = false;
                cv.notify_all();*/
                if(datasReady) {
                    datasReady = false;



                    //std::cout<<"data ready"<<std::endl;
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
                }
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
            glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, view.getSize().x, view.getSize().y, GL_RED_INTEGER,
            GL_UNSIGNED_INT, NULL));
            glCheck(glBindTexture(GL_TEXTURE_2D, 0));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));

            frameBuffer.resetGLStates();*/
            float zNear = view.getViewport().getPosition().z;
            if (!view.isOrtho())
                view.setPerspective(80, view.getViewport().getSize().x / view.getViewport().getSize().y, 0.1f, view.getViewport().getSize().z);
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
            currentStates.blendMode = sf::BlendAlpha;
            currentStates.shader = &skyboxShader;
            currentStates.texture = (skybox == nullptr ) ? nullptr : &static_cast<g3d::Skybox*>(skybox)->getTexture();
            vb.update();
            frameBuffer.drawVertexBuffer(vb, currentStates);
            vb.clear();
            if (!view.isOrtho())
                view.setPerspective(80, view.getViewport().getSize().x / view.getViewport().getSize().y, zNear, view.getViewport().getSize().z);
            projMatrix = view.getProjMatrix().getMatrix().transpose();
            viewMatrix = view.getViewMatrix().getMatrix().transpose();
            indirectRenderingShader.setParameter("projectionMatrix", projMatrix);
            indirectRenderingShader.setParameter("viewMatrix", viewMatrix);

            drawInstances();
            drawInstancesIndexed();
            drawSelectedInstances();
            drawSelectedInstancesIndexed();
            //std::cout<<"nb instances : "<<m_normals.size()<<std::endl;


            glCheck(glFinish());
            glCheck(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));
            //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            vb.clear();
            //vb.name = "";
            vb.setPrimitiveType(sf::Quads);
            Vertex v1 (sf::Vector3f(0, 0, quad.getSize().z));
            Vertex v2 (sf::Vector3f(quad.getSize().x,0, quad.getSize().z));
            Vertex v3 (sf::Vector3f(quad.getSize().x, quad.getSize().y, quad.getSize().z));
            Vertex v4 (sf::Vector3f(0, quad.getSize().y, quad.getSize().z));
            vb.append(v1);
            vb.append(v2);
            vb.append(v3);
            vb.append(v4);
            vb.update();
            math::Matrix4f matrix = quad.getTransform().getMatrix().transpose();
            perPixelLinkedListP2.setParameter("worldMat", matrix);
            currentStates.shader = &perPixelLinkedListP2;
            frameBuffer.drawVertexBuffer(vb, currentStates);
            glCheck(glFinish());
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
                    //std::cout<<"next frame draw normal"<<std::endl;
                    float time = timeClock.getElapsedTime().asSeconds();
                    indirectRenderingShader.setParameter("time", time);

                    unsigned int p = m_normals[i].getAllVertices().getPrimitiveType();
                    /*if (m_normals[i].getVertexArrays()[0]->getEntity()->getRootType() == "E_MONSTER") {
                            std::cout<<"tex coords : "<<(*m_normals[i].getVertexArrays()[0])[0].texCoords.x<<","<<(*m_normals[i].getVertexArrays()[0])[0].texCoords.y<<std::endl;
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
                                std::cout<<"vertex position : "<<(*m_normals[i].getVertexArrays()[v])[n].position.x<<std::endl;
                                std::cout<<"vertex color : "<<(int) (*m_normals[i].getVertexArrays()[v])[n].color.r<<std::endl;
                                std::cout<<"vertex tex coords : "<<(*m_normals[i].getVertexArrays()[v])[n].texCoords.x<<std::endl;

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
                                std::cout<<"position hero : "<<(*m_normals[i].getVertexArrays()[j])[n].position.x<<","<<(*m_normals[i].getVertexArrays()[j])[n].position.y<<","<<(*m_normals[i].getVertexArrays()[j])[n].position.z<<std::endl;
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
                    //std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                    //std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;

                }
            }
            RenderStates currentStates;
            currentStates.blendMode = sf::BlendNone;
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
                            std::cout<<"tex coords : "<<(*m_normals[i].getVertexArrays()[0])[0].texCoords.x<<","<<(*m_normals[i].getVertexArrays()[0])[0].texCoords.y<<std::endl;
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
                    //std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                    //std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;
                }
            }
            currentStates.blendMode = sf::BlendNone;
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

            {
                std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                datasReady = false;

                //if (datasReady) {
                    //std::cout<<"wait 2 "<<std::endl;
                    //cv.wait(lock, [this]() {return !datasReady;});
                //}
                //std::cout<<"data unready"<<std::endl;
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

                           // std::cout<<"remove texture"<<std::endl;

                            //std::cout<<"get va"<<std::endl;
                                VertexArray& va = border->getFace(j)->getVertexArray();
                                //std::cout<<"change color"<<std::endl;
                                for (unsigned int j = 0; j < va.getVertexCount(); j++) {

                                    va[j].color = sf::Color::Cyan;
                                }

                                border->setOrigin(border->getSize() * 0.5f);
                                border->setScale(math::Vec3f(1.1f, 1.1f, 1.1f));
                               // std::cout<<"add to batcher"<<std::endl;
                                selectedInstanceScaleBatcher.addFace(border->getFace(j));
                           // std::cout<<"face added"<<std::endl;
                             } else {
                                 {
                                    std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                                    selectedInstanceIndexBatcher.addFace(vEntities[i]->getFace(j));
                                 }

                               // std::cout<<"remove texture"<<std::endl;

                            //std::cout<<"get va"<<std::endl;
                                VertexArray& va = border->getFace(j)->getVertexArray();
                                //std::cout<<"change color"<<std::endl;
                                for (unsigned int j = 0; j < va.getVertexCount(); j++) {

                                    va[j].color = sf::Color::Cyan;
                                }
                                Entity* root = (vEntities[i]->getRootEntity()->isAnimated()) ? vEntities[i]->getRootEntity() : vEntities[i];
                                math::Vec3f oldSize = root->getSize();
                                border->setOrigin(root->getSize() * 0.5f);
                                border->setSize(root->getSize() * 1.1f);
                                math::Vec3f offset =  root->getSize() - oldSize;
                                //border->setPosition(root->getPosition() - offset * 0.5f);

                               // std::cout<<"add to batcher"<<std::endl;
                                selectedInstanceIndexScaleBatcher.addFace(border->getFace(j));
                             }
                        } else {
                            if (vEntities[i]->getFace(j)->getVertexArray().getIndexes().size() == 0) {
                                {
                                    std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                                    selectedBatcher.addFace(vEntities[i]->getFace(j));
                                }

                           // std::cout<<"remove texture"<<std::endl;

                            //std::cout<<"get va"<<std::endl;
                                VertexArray& va = border->getFace(j)->getVertexArray();
                                //std::cout<<"change color"<<std::endl;
                                for (unsigned int j = 0; j < va.getVertexCount(); j++) {

                                    va[j].color = sf::Color::Cyan;
                                }
                                Entity* root = (vEntities[i]->getRootEntity()->isAnimated()) ? vEntities[i]->getRootEntity() : vEntities[i];
                                math::Vec3f oldSize = root->getSize();
                                border->setOrigin(root->getSize() * 0.5f);
                                border->setSize(root->getSize() * 1.1f);
                                math::Vec3f offset =  root->getSize() - oldSize;
                                //border->setPosition(root->getPosition() - offset * 0.5f);
                                selectedScaleBatcher.addFace(border->getFace(j));

                               // std::cout<<"face added"<<std::endl;
                             } else {
                                 {
                                    std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                                    selectedIndexBatcher.addFace(vEntities[i]->getFace(j));
                                 }

                               // std::cout<<"remove texture"<<std::endl;

                            //std::cout<<"get va"<<std::endl;
                                VertexArray& va = border->getFace(j)->getVertexArray();
                                //std::cout<<"change color"<<std::endl;
                                for (unsigned int j = 0; j < va.getVertexCount(); j++) {

                                    va[j].color = sf::Color::Cyan;
                                }

                                border->setOrigin(border->getSize() * 0.5f);
                                border->setScale(math::Vec3f(1.1f, 1.1f, 1.1f));
                               // std::cout<<"add to batcher"<<std::endl;
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

            //std::cout<<"instances added"<<std::endl;
            visibleEntities = vEntities;
            update = true;
            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
            datasReady = true;



            return true;
        }
        void PerPixelLinkedListRenderComponent::pushEvent(window::IEvent event, RenderWindow& rw) {
            if (event.type == window::IEvent::WINDOW_EVENT && event.window.type == window::IEvent::WINDOW_EVENT_RESIZED && &getWindow() == &rw && isAutoResized()) {
                std::cout<<"recompute size"<<std::endl;
                recomputeSize();
                getListener().pushEvent(event);
                getView().reset(physic::BoundingBox(getView().getViewport().getPosition().x, getView().getViewport().getPosition().y, getView().getViewport().getPosition().z, event.window.data1, event.window.data2, getView().getViewport().getDepth()));
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
