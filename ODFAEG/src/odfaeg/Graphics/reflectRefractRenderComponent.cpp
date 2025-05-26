#include "../../../include/odfaeg/Graphics/reflectRefractRenderComponent.hpp"
#ifndef VULKAN
#include "glCheck.h"
#endif // VULKAN
namespace odfaeg {
    namespace graphic {
        #ifdef VULKAN
            ReflectRefractRenderComponent::ReflectRefractRenderComponent (RenderWindow& window, int layer, std::string expression, window::ContextSettings settings) :
            HeavyComponent(window, math::Vec3f(window.getView().getPosition().x, window.getView().getPosition().y, layer),
                          math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0),
                          math::Vec3f(window.getView().getSize().x + window.getView().getSize().x * 0.5f, window.getView().getPosition().y + window.getView().getSize().y * 0.5f, layer)),
            view(window.getView()),
            expression(expression),
            depthBuffer(window.getDevice()), alphaBuffer(window.getDevice()), environmentMap(window.getDevice()), reflectRefractTex(window.getDevice()),
            vb(window.getDevice()), vb2(window.getDevice()),
            vbBindlessTex {VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()),
             VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice()), VertexBuffer(window.getDevice())},
             descriptorSetLayout(window.getDescriptorSetLayout()),
             sBuildDepthBuffer(window.getDevice()), sBuildAlphaBuffer(window.getDevice()), sReflectRefract(window.getDevice()), sLinkedList(window.getDevice()),
             sLinkedList2(window.getDevice()), skyboxShader(window.getDevice()),
             vkDevice(window.getDevice()),
             descriptorPool(window.getDescriptorPool()),
             descriptorSets(window.getDescriptorSet())
             {
                vboIndirect = vboIndirectStagingBuffer = modelDataStagingBuffer = materialDataStagingBuffer = nullptr;
                maxVboIndirectSize = maxModelDataSize = maxMaterialDataSize = 0;
                createCommandPool();
                if (window.getView().getSize().x > window.getView().getSize().y) {
                    squareSize = window.getView().getSize().x;
                } else {
                    squareSize = window.getView().getSize().y;
                }
                quad = RectangleShape(math::Vec3f(squareSize, squareSize, squareSize * 0.5f));
                quad.move(math::Vec3f(-squareSize * 0.5f, -squareSize * 0.5f, 0));
                resolution = math::Vec3f((int) window.getSize().x, (int) window.getSize().y, window.getView().getSize().z);
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
                depthBuffer.m_name = "depthBuffer";
                depthBuffer.create(window.getView().getSize().x, window.getView().getSize().y);

                depthBufferSprite = Sprite(depthBuffer.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), sf::IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));
                alphaBuffer.create(window.getView().getSize().x, window.getView().getSize().y);
                //alphaBuffer.m_name = "alphaBuffer";
                alphaBufferSprite = Sprite(alphaBuffer.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), sf::IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));
                environmentMap.createCubeMap(squareSize, squareSize);
                //environmentMap.m_name = "environmentMap";
                reflectRefractTex.create(window.getView().getSize().x, window.getView().getSize().y);
                //reflectRefractTex.m_name = "relfectRefractTex";
                reflectRefractTexSprite = Sprite(reflectRefractTex.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), sf::IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));
                linkedListShaderStorageBuffers.resize(reflectRefractTex.getMaxFramesInFlight());
                linkedListShaderStorageBuffersMemory.resize(reflectRefractTex.getMaxFramesInFlight());
                maxNodes = 20;
                unsigned int nodeSize = 5  * sizeof(float) + sizeof(unsigned int);
                VkDeviceSize bufferSize = maxNodes * nodeSize;
                for (unsigned int i = 0; i < reflectRefractTex.getMaxFramesInFlight(); i++) {
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
                VkImageCreateInfo imageInfo{};
                imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                imageInfo.imageType = VK_IMAGE_TYPE_3D;
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
                transitionImageLayout(headPtrTextureImage, VK_FORMAT_R32_UINT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);



                imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                imageInfo.imageType = VK_IMAGE_TYPE_2D;
                imageInfo.extent.width = static_cast<uint32_t>(window.getView().getSize().x);
                imageInfo.extent.height = static_cast<uint32_t>(window.getView().getSize().y);
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


                vkGetImageMemoryRequirements(window.getDevice().getDevice(), depthTextureImage, &memRequirements);


                allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.allocationSize = memRequirements.size;
                allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);


                if (vkAllocateMemory(window.getDevice().getDevice(), &allocInfo, nullptr, &depthTextureImageMemory) != VK_SUCCESS) {
                    throw std::runtime_error("echec de l'allocation de la memoire d'une image!");
                }
                vkBindImageMemory(window.getDevice().getDevice(), depthTextureImage, depthTextureImageMemory, 0);
                transitionImageLayout(depthTextureImage, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);




                imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                imageInfo.imageType = VK_IMAGE_TYPE_2D;
                imageInfo.extent.width = static_cast<uint32_t>(window.getView().getSize().x);
                imageInfo.extent.height = static_cast<uint32_t>(window.getView().getSize().y);
                imageInfo.extent.depth = 1;
                imageInfo.mipLevels = 1;
                imageInfo.arrayLayers = 1;
                imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
                imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
                imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                imageInfo.flags = 0; // Optionnel
                if (vkCreateImage(window.getDevice().getDevice(), &imageInfo, nullptr, &alphaTextureImage) != VK_SUCCESS) {
                    throw std::runtime_error("echec de la creation d'une image!");
                }


                vkGetImageMemoryRequirements(window.getDevice().getDevice(), alphaTextureImage, &memRequirements);


                allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.allocationSize = memRequirements.size;
                allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);


                if (vkAllocateMemory(window.getDevice().getDevice(), &allocInfo, nullptr, &alphaTextureImageMemory) != VK_SUCCESS) {
                    throw std::runtime_error("echec de l'allocation de la memoire d'une image!");
                }
                vkBindImageMemory(window.getDevice().getDevice(), alphaTextureImage, alphaTextureImageMemory, 0);
                transitionImageLayout(alphaTextureImage, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
                createImageView();
                createSampler();
                createUniformBuffers();
                compileShaders();

                RenderStates states;
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

                std::vector<std::vector<std::vector<VkPipelineLayoutCreateInfo>>>& pipelineLayoutInfo = reflectRefractTex.getPipelineLayoutCreateInfo();
                std::vector<std::vector<std::vector<VkPipelineDepthStencilStateCreateInfo>>>& depthStencilCreateInfo = reflectRefractTex.getDepthStencilCreateInfo();
                pipelineLayoutInfo.resize((Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders());
                depthStencilCreateInfo.resize((Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders());
                for (unsigned int i = 0; i < (Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders(); i++) {
                    pipelineLayoutInfo[i].resize(reflectRefractTex.getNbRenderTargets());
                    depthStencilCreateInfo[i].resize(reflectRefractTex.getNbRenderTargets());
                }
                for (unsigned int i = 0; i < (Batcher::nbPrimitiveTypes - 1) * Shader::getNbShaders(); i++) {
                    for (unsigned int j = 0; j < reflectRefractTex.getNbRenderTargets(); j++) {
                        pipelineLayoutInfo[i][j].resize(NBDEPTHSTENCIL);
                        depthStencilCreateInfo[i][j].resize(NBDEPTHSTENCIL);
                    }
                }
                environmentMap.enableDepthTest(true);
                depthBuffer.enableDepthTest(true);
                alphaBuffer.enableDepthTest(true);
                reflectRefractTex.enableDepthTest(true);
                states.shader = &sLinkedList;
                for (unsigned int j = 0; j < NBDEPTHSTENCIL; j++) {
                    for (unsigned int i = 3; i < Batcher::nbPrimitiveTypes - 1; i++) {
                        if (j == 0) {
                           depthStencilCreateInfo[sLinkedList.getId() * (Batcher::nbPrimitiveTypes - 1)+i][environmentMap.getId()][NODEPTHNOSTENCIL].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                           depthStencilCreateInfo[sLinkedList.getId() * (Batcher::nbPrimitiveTypes - 1)+i][environmentMap.getId()][NODEPTHNOSTENCIL].front = {};
                           depthStencilCreateInfo[sLinkedList.getId() * (Batcher::nbPrimitiveTypes - 1)+i][environmentMap.getId()][NODEPTHNOSTENCIL].back = {};
                           environmentMap.createGraphicPipeline(static_cast<sf::PrimitiveType>(i), states, NODEPTHNOSTENCIL, NBDEPTHSTENCIL);
                        } else {
                           depthStencilCreateInfo[sLinkedList.getId() * (Batcher::nbPrimitiveTypes - 1)+i][environmentMap.getId()][DEPTHNOSTENCIL].depthCompareOp = VK_COMPARE_OP_GREATER;
                           depthStencilCreateInfo[sLinkedList.getId() * (Batcher::nbPrimitiveTypes - 1)+i][environmentMap.getId()][DEPTHNOSTENCIL].front = {};
                           depthStencilCreateInfo[sLinkedList.getId() * (Batcher::nbPrimitiveTypes - 1)+i][environmentMap.getId()][DEPTHNOSTENCIL].back = {};
                           environmentMap.createGraphicPipeline(static_cast<sf::PrimitiveType>(i), states, DEPTHNOSTENCIL, NBDEPTHSTENCIL);
                        }
                    }
                }
                states.shader = &sLinkedList2;
                for (unsigned int j = 0; j < NBDEPTHSTENCIL; j++) {
                    for (unsigned int i = 3; i < Batcher::nbPrimitiveTypes - 1; i++) {
                        if (j == 0) {
                           VkPushConstantRange push_constant;
                           //this push constant range starts at the beginning
                           push_constant.offset = 0;
                           //this push constant range takes up the size of a MeshPushConstants struct
                           push_constant.size = sizeof(LinkedList2PC);
                           //this push constant range is accessible only in the vertex shader
                           push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                           pipelineLayoutInfo[sLinkedList2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][environmentMap.getId()][NODEPTHNOSTENCIL].pPushConstantRanges = &push_constant;
                           pipelineLayoutInfo[sLinkedList2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][environmentMap.getId()][NODEPTHNOSTENCIL].pushConstantRangeCount = 1;
                           depthStencilCreateInfo[sLinkedList2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][environmentMap.getId()][NODEPTHNOSTENCIL].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                           depthStencilCreateInfo[sLinkedList2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][environmentMap.getId()][NODEPTHNOSTENCIL].front = {};
                           depthStencilCreateInfo[sLinkedList2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][environmentMap.getId()][NODEPTHNOSTENCIL].back = {};
                           environmentMap.createGraphicPipeline(static_cast<sf::PrimitiveType>(i), states, NODEPTHNOSTENCIL, NBDEPTHSTENCIL);
                        } else {
                           VkPushConstantRange push_constant;
                           //this push constant range starts at the beginning
                           push_constant.offset = 0;
                           //this push constant range takes up the size of a MeshPushConstants struct
                           push_constant.size = sizeof(LinkedList2PC);
                           //this push constant range is accessible only in the vertex shader
                           push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                           pipelineLayoutInfo[sLinkedList2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][environmentMap.getId()][DEPTHNOSTENCIL].pPushConstantRanges = &push_constant;
                           pipelineLayoutInfo[sLinkedList2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][environmentMap.getId()][DEPTHNOSTENCIL].pushConstantRangeCount = 1;
                           depthStencilCreateInfo[sLinkedList2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][environmentMap.getId()][DEPTHNOSTENCIL].depthCompareOp = VK_COMPARE_OP_GREATER;
                           depthStencilCreateInfo[sLinkedList2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][environmentMap.getId()][DEPTHNOSTENCIL].front = {};
                           depthStencilCreateInfo[sLinkedList2.getId() * (Batcher::nbPrimitiveTypes - 1)+i][environmentMap.getId()][DEPTHNOSTENCIL].back = {};
                           environmentMap.createGraphicPipeline(static_cast<sf::PrimitiveType>(i), states, DEPTHNOSTENCIL, NBDEPTHSTENCIL);
                        }
                    }
                }
                states.shader = &sBuildDepthBuffer;
                for (unsigned int j = 0; j < NBDEPTHSTENCIL; j++) {
                    for (unsigned int i = 3; i < Batcher::nbPrimitiveTypes - 1; i++) {
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
                            pipelineLayoutInfo[sBuildDepthBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][depthBuffer.getId()][NODEPTHNOSTENCIL].pPushConstantRanges = push_constants.data();
                            pipelineLayoutInfo[sBuildDepthBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][depthBuffer.getId()][NODEPTHNOSTENCIL].pushConstantRangeCount = 2;
                            depthStencilCreateInfo[sBuildDepthBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][depthBuffer.getId()][NODEPTHNOSTENCIL].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                            depthStencilCreateInfo[sBuildDepthBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][depthBuffer.getId()][NODEPTHNOSTENCIL].front = {};
                            depthStencilCreateInfo[sBuildDepthBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][depthBuffer.getId()][NODEPTHNOSTENCIL].back = {};
                            depthBuffer.createGraphicPipeline(static_cast<sf::PrimitiveType>(i), states, NODEPTHNOSTENCIL, NBDEPTHSTENCIL);
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
                            pipelineLayoutInfo[sBuildDepthBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][depthBuffer.getId()][DEPTHNOSTENCIL].pPushConstantRanges = push_constants.data();
                            pipelineLayoutInfo[sBuildDepthBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][depthBuffer.getId()][DEPTHNOSTENCIL].pushConstantRangeCount = 2;
                            depthStencilCreateInfo[sBuildDepthBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][depthBuffer.getId()][DEPTHNOSTENCIL].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                            depthStencilCreateInfo[sBuildDepthBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][depthBuffer.getId()][DEPTHNOSTENCIL].front = {};
                            depthStencilCreateInfo[sBuildDepthBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][depthBuffer.getId()][DEPTHNOSTENCIL].back = {};
                            depthBuffer.createGraphicPipeline(static_cast<sf::PrimitiveType>(i), states, DEPTHNOSTENCIL, NBDEPTHSTENCIL);
                        }
                    }
                }
                states.shader = &sBuildAlphaBuffer;
                for (unsigned int j = 0; j < NBDEPTHSTENCIL; j++) {
                    for (unsigned int i = 3; i < Batcher::nbPrimitiveTypes - 1; i++) {
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
                            push_constant2.size = sizeof(BuildAlphaPC);
                            //this push constant range is accessible only in the vertex shader
                            push_constant2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                            push_constants[1] = push_constant2;
                            pipelineLayoutInfo[sBuildAlphaBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][alphaBuffer.getId()][NODEPTHNOSTENCIL].pPushConstantRanges = push_constants.data();
                            pipelineLayoutInfo[sBuildAlphaBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][alphaBuffer.getId()][NODEPTHNOSTENCIL].pushConstantRangeCount = 2;
                            depthStencilCreateInfo[sBuildAlphaBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][alphaBuffer.getId()][NODEPTHNOSTENCIL].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                            depthStencilCreateInfo[sBuildAlphaBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][alphaBuffer.getId()][NODEPTHNOSTENCIL].front = {};
                            depthStencilCreateInfo[sBuildAlphaBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][alphaBuffer.getId()][NODEPTHNOSTENCIL].back = {};
                            alphaBuffer.createGraphicPipeline(static_cast<sf::PrimitiveType>(i), states, NODEPTHNOSTENCIL, NBDEPTHSTENCIL);
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
                            push_constant2.size = sizeof(BuildAlphaPC);
                            //this push constant range is accessible only in the vertex shader
                            push_constant2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                            push_constants[1] = push_constant2;
                            pipelineLayoutInfo[sBuildAlphaBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][alphaBuffer.getId()][DEPTHNOSTENCIL].pPushConstantRanges = push_constants.data();
                            pipelineLayoutInfo[sBuildAlphaBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][alphaBuffer.getId()][DEPTHNOSTENCIL].pushConstantRangeCount = 2;
                            depthStencilCreateInfo[sBuildAlphaBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][alphaBuffer.getId()][DEPTHNOSTENCIL].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                            depthStencilCreateInfo[sBuildAlphaBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][alphaBuffer.getId()][DEPTHNOSTENCIL].front = {};
                            depthStencilCreateInfo[sBuildAlphaBuffer.getId() * (Batcher::nbPrimitiveTypes - 1)+i][alphaBuffer.getId()][DEPTHNOSTENCIL].back = {};
                            alphaBuffer.createGraphicPipeline(static_cast<sf::PrimitiveType>(i), states, DEPTHNOSTENCIL, NBDEPTHSTENCIL);
                        }
                    }
                }
                states.shader = &sReflectRefract;
                for (unsigned int j = 0; j < NBDEPTHSTENCIL; j++) {
                    for (unsigned int i = 3; i < Batcher::nbPrimitiveTypes - 1; i++) {
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
                            push_constant2.size = sizeof(BuildFrameBufferPC);
                            //this push constant range is accessible only in the vertex shader
                            push_constant2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                            push_constants[1] = push_constant2;
                            pipelineLayoutInfo[sReflectRefract.getId() * (Batcher::nbPrimitiveTypes - 1)+i][reflectRefractTex.getId()][NODEPTHNOSTENCIL].pPushConstantRanges = push_constants.data();
                            pipelineLayoutInfo[sReflectRefract.getId() * (Batcher::nbPrimitiveTypes - 1)+i][reflectRefractTex.getId()][NODEPTHNOSTENCIL].pushConstantRangeCount = 2;
                            depthStencilCreateInfo[sReflectRefract.getId() * (Batcher::nbPrimitiveTypes - 1)+i][reflectRefractTex.getId()][NODEPTHNOSTENCIL].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                            depthStencilCreateInfo[sReflectRefract.getId() * (Batcher::nbPrimitiveTypes - 1)+i][reflectRefractTex.getId()][NODEPTHNOSTENCIL].front = {};
                            depthStencilCreateInfo[sReflectRefract.getId() * (Batcher::nbPrimitiveTypes - 1)+i][reflectRefractTex.getId()][NODEPTHNOSTENCIL].back = {};
                            reflectRefractTex.createGraphicPipeline(static_cast<sf::PrimitiveType>(i), states, NODEPTHNOSTENCIL, NBDEPTHSTENCIL);
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
                            push_constant2.size = sizeof(BuildFrameBufferPC);
                            //this push constant range is accessible only in the vertex shader
                            push_constant2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                            push_constants[1] = push_constant2;
                            pipelineLayoutInfo[sReflectRefract.getId() * (Batcher::nbPrimitiveTypes - 1)+i][reflectRefractTex.getId()][DEPTHNOSTENCIL].pPushConstantRanges = push_constants.data();
                            pipelineLayoutInfo[sReflectRefract.getId() * (Batcher::nbPrimitiveTypes - 1)+i][reflectRefractTex.getId()][DEPTHNOSTENCIL].pushConstantRangeCount = 2;
                            depthStencilCreateInfo[sReflectRefract.getId() * (Batcher::nbPrimitiveTypes - 1)+i][reflectRefractTex.getId()][DEPTHNOSTENCIL].depthCompareOp = VK_COMPARE_OP_ALWAYS;
                            depthStencilCreateInfo[sReflectRefract.getId() * (Batcher::nbPrimitiveTypes - 1)+i][reflectRefractTex.getId()][DEPTHNOSTENCIL].front = {};
                            depthStencilCreateInfo[sReflectRefract.getId() * (Batcher::nbPrimitiveTypes - 1)+i][reflectRefractTex.getId()][DEPTHNOSTENCIL].back = {};
                            reflectRefractTex.createGraphicPipeline(static_cast<sf::PrimitiveType>(i), states, DEPTHNOSTENCIL, NBDEPTHSTENCIL);
                        }
                    }
                }
                for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                    vbBindlessTex[i].setPrimitiveType(static_cast<sf::PrimitiveType>(i));
                }
                vkCmdPushDescriptorSetKHR = (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(vkDevice.getDevice(), "vkCmdPushDescriptorSetKHR");
                skybox = nullptr;
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
                    //std::cout<<"uniform buffer : "<<ubos[i]<<std::endl;
                }
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
                const std::string indirectRenderingVertexShader = R"(#version 460
                                                                     layout (location = 0) in vec3 position;
                                                                     layout (location = 1) in vec4 color;
                                                                     layout (location = 2) in vec2 texCoords;
                                                                     layout (location = 3) in vec3 normals;
                                                                     layout (push_constant)uniform PushConsts {
                                                                         mat4 projectionMatrix;
                                                                         mat4 viewMatrix;
                                                                     } pushConsts;
                                                                     struct ModelData {
                                                                         mat4 modelMatrix;
                                                                     };
                                                                     struct MaterialData {
                                                                         uint textureIndex;
                                                                         uint layer;
                                                                         uint materialType;
                                                                     };
                                                                     layout(set = 0, binding = 4) buffer modelData {
                                                                         ModelData modelDatas[];
                                                                     };
                                                                     layout(set = 0, binding = 5) buffer materialData {
                                                                         MaterialData materialDatas[];
                                                                     };
                                                                     layout (location = 0) out vec2 fTexCoords;
                                                                     layout (location = 1) out vec4 frontColor;
                                                                     layout (location = 2) out uint texIndex;
                                                                     layout (location = 3) out uint layer;
                                                                     layout (location = 4) out vec3 normal;
                                                                     void main() {
                                                                         MaterialData material = materialDatas[gl_DrawID];
                                                                         ModelData model = modelDatas[gl_InstanceIndex];
                                                                         uint textureIndex = material.textureIndex;
                                                                         uint l = material.layer;
                                                                         gl_Position = vec4(position, 1.f) * model.modelMatrix * pushConsts.viewMatrix * pushConsts.projectionMatrix;
                                                                         fTexCoords = texCoords;
                                                                         frontColor = color;
                                                                         texIndex = textureIndex;
                                                                         normal = normals;
                                                                         layer = l;
                                                                         gl_PointSize = 2.0f;
                                                                     }
                                                                     )";
                const std::string linkedListIndirectRenderingVertexShader = R"(#version 460
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
                                                                   uint materialType;
                                                               };
                                                               layout(set = 0, binding = 4) buffer modelData {
                                                                   ModelData modelDatas[];
                                                               };
                                                               layout(set = 0, binding = 5) buffer materialData {
                                                                   MaterialData materialDatas[];
                                                               };
                                                               layout (location = 0) out vec4 vColor;
                                                               layout (location = 1) out vec2 vTexCoord;
                                                               layout (location = 2) out uint tIndex;
                                                               layout (location = 3) out vec3 vNormal;
                                                               void main() {
                                                                    MaterialData material = materialDatas[gl_DrawID];
                                                                    ModelData model = modelDatas[gl_InstanceIndex];
                                                                    uint textureIndex = material.textureIndex;
                                                                    gl_Position = vec4(position, 1.f) * model.modelMatrix;
                                                                    vTexCoord = texCoords;
                                                                    vColor = color;
                                                                    tIndex = textureIndex;
                                                                    vNormal = normals;
                                                               }
                                                               )";
                const std::string  linkedListVertexShader2 = R"(#version 460
                                                                layout (location = 0) in vec3 position;
                                                                layout (location = 1) in vec4 color;
                                                                layout (location = 2) in vec2 texCoords;
                                                                layout (location = 3) in vec3 normals;
                                                                layout (push_constant)uniform PushConsts {
                                                                     mat4 projectionMatrix;
                                                                     mat4 viewMatrix;
                                                                     mat4 worldMat;
                                                                } pushConsts;
                                                                layout (location = 0) out vec4 vColor;
                                                                layout (location = 1) out vec2 vTexCoord;
                                                                layout (location = 2) out vec3 vNormal;
                                                                void main () {
                                                                    gl_Position = vec4(position, 1.f) * pushConsts.worldMat * pushConsts.viewMatrix * pushConsts.projectionMatrix;
                                                                    gl_PointSize = 2.0f;
                                                                    vColor = color;
                                                                    vTexCoord = texCoords;
                                                                    vNormal = normals;
                                                                })";
                const std::string perPixReflectRefractIndirectRenderingVertexShader = R"(#version 460
                                                                                         layout (location = 0) in vec3 position;
                                                                                         layout (location = 1) in vec4 color;
                                                                                         layout (location = 2) in vec2 texCoords;
                                                                                         layout (location = 3) in vec3 normals;
                                                                                         layout (push_constant)uniform PushConsts {
                                                                                             mat4 projectionMatrix;
                                                                                             mat4 viewMatrix;
                                                                                         } pushConsts;
                                                                                         struct ModelData {
                                                                                             mat4 modelMatrix;
                                                                                         };
                                                                                         struct MaterialData {
                                                                                             uint textureIndex;
                                                                                             uint layer;
                                                                                             uint materialType;
                                                                                         };
                                                                                         layout(set = 0, binding = 4) buffer modelData {
                                                                                             ModelData modelDatas[];
                                                                                         };
                                                                                         layout(set = 0, binding = 5) buffer materialData {
                                                                                             MaterialData materialDatas[];
                                                                                         };
                                                                                         layout (location = 0) out vec3 pos;
                                                                                         layout (location = 1) out vec4 frontColor;
                                                                                         layout (location = 2) out vec2 texCoord;
                                                                                         layout (location = 3) out uint materialType;
                                                                                         layout (location = 4) out vec3 normal;
                                                                                         void main () {
                                                                                             MaterialData material = materialDatas[gl_DrawID];
                                                                                             ModelData model = modelDatas[gl_InstanceIndex];
                                                                                             uint materialT = material.materialType;
                                                                                             pos = vec3(vec4(position, 1.0) * model.modelMatrix);
                                                                                             gl_Position = vec4(position, 1.f) * model.modelMatrix * pushConsts.viewMatrix * pushConsts.projectionMatrix;
                                                                                             frontColor = color;
                                                                                             texCoord = texCoords;
                                                                                             normal = mat3(transpose(inverse(model.modelMatrix))) * normals;
                                                                                             materialType = materialT;
                                                                                         }
                                                                                         )";
                const std::string geometryShader = R"(#version 460
                                                      layout (triangles) in;
                                                      layout (triangle_strip, max_vertices = 18) out;
                                                      layout (location = 0) in vec4 vColor[];
                                                      layout (location = 1) in vec2 vTexCoord[];
                                                      layout (location = 2) in uint tIndex[];
                                                      layout (location = 3) in vec3 vNormal[];
                                                      layout (location = 0) out vec4 frontColor;
                                                      layout (location = 1) out vec2 fTexCoords;
                                                      layout (location = 2) out uint layer;
                                                      layout (location = 3) out uint texIndex;
                                                      layout (location = 4) out vec3 normal;
                                                      struct MatricesDatas {
                                                          mat4 projectionMatrix;
                                                          mat4 viewMatrix;
                                                      };
                                                      layout (set = 0, binding = 6) uniform UniformBufferObject {
                                                         MatricesDatas datas[6];
                                                      };
                                                      void main() {
                                                        for (int face = 0; face < 6; face++) {
                                                            gl_Layer = face;
                                                            for (uint i = 0; i < 3; i++) {
                                                                gl_Position = gl_in[i].gl_Position * datas[face].viewMatrix * datas[face].projectionMatrix;
                                                                frontColor = vColor[i];
                                                                fTexCoords = vTexCoord[i];
                                                                texIndex = tIndex[i];
                                                                layer = face;
                                                                normal = vNormal[i];
                                                                EmitVertex();
                                                            }
                                                            EndPrimitive();
                                                        }
                                                      }
                                                      )";
                const std::string geometryShader2 = R"(#version 460
                                                      layout (triangles) in;
                                                      layout (triangle_strip, max_vertices = 18) out;
                                                      layout (location = 0) in vec4 vColor[];
                                                      layout (location = 1) in vec2 vTexCoord[];
                                                      layout (location = 2) in vec3 vNormal[];
                                                      layout (location = 0) out vec4 frontColor;
                                                      layout (location = 1) out vec2 fTexCoords;
                                                      layout (location = 2) out uint layer;
                                                      layout (location = 3) out vec3 normal;
                                                      void main() {
                                                        for (int face = 0; face < 6; face++) {
                                                            gl_Layer = face;
                                                            for (uint i = 0; i < 3; i++) {
                                                                gl_Position = gl_in[i].gl_Position;
                                                                frontColor = vColor[i];
                                                                fTexCoords = vTexCoord[i];
                                                                layer = face;
                                                                normal = vNormal[i];
                                                                EmitVertex();
                                                            }
                                                            EndPrimitive();
                                                        }
                                                      }
                                                      )";
                    const std::string buildDepthBufferFragmentShader = R"(#version 460
                                                                      #extension GL_ARB_fragment_shader_interlock : require
                                                                      #extension GL_EXT_nonuniform_qualifier : enable
                                                                          layout (location = 0) in vec2 fTexCoords;
                                                                          layout (location = 1) in vec4 frontColor;
                                                                          layout (location = 2) in flat uint texIndex;
                                                                          layout (location = 3) in flat uint layer;
                                                                          layout (location = 4) in vec3 normal;
                                                                          layout (push_constant) uniform PushConsts {
                                                                             layout (offset = 128) uint nbLayers;
                                                                          } pushConsts;
                                                                          layout(set = 0, binding = 0) uniform sampler2D textures[];
                                                                          layout(set = 0, binding = 1, rgba32f) uniform coherent image2D depthBuffer;
                                                                          layout(location = 0) out vec4 fColor;

                                                                          void main () {
                                                                              vec4 texel = (texIndex != 0) ? frontColor * texture(textures[texIndex-1], fTexCoords.xy) : frontColor;
                                                                              float z = gl_FragCoord.z;
                                                                              float l = float(layer) / float(pushConsts.nbLayers);
                                                                              beginInvocationInterlockARB();
                                                                              memoryBarrier();
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
                                                                      layout(set = 0, binding = 0) uniform sampler2D textures[];
                                                                      layout(set = 0, binding = 1, rgba32f) uniform coherent image2D alphaBuffer;
                                                                      layout (location = 0) out vec4 fColor;
                                                                      layout (set = 0, binding = 2) uniform sampler2D depthBuffer;
                                                                      layout (push_constant) uniform PushConsts {
                                                                            layout (offset = 128) vec4 resolution;
                                                                            layout (offset = 144) uint nbLayers;
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
                                                                          vec4 depth = textureLod (depthBuffer, position, 0);
                                                                          beginInvocationInterlockARB();
                                                                          memoryBarrier();
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
                                                                layout (location = 0) in vec3 pos;
                                                                layout (location = 1) in vec4 frontColor;
                                                                layout (location = 2) in vec2 texCoord;
                                                                layout (location = 3) in flat uint materialType;
                                                                layout (location = 4) in vec3 normal;
                                                                layout (push_constant) uniform PushConsts {
                                                                    layout(offset = 128) vec4 cameraPos;
                                                                    layout(offset = 144) vec4 resolution;
                                                                } pushConsts;
                                                                layout (set = 0, binding = 0) uniform samplerCube sceneBox;
                                                                layout (set = 0, binding = 1) uniform sampler2D alphaBuffer;
                                                                layout (location = 0) out vec4 fColor;
                                                                void main () {
                                                                    vec2 position = (gl_FragCoord.xy / pushConsts.resolution.xy);
                                                                    vec4 alpha = texture(alphaBuffer, position);
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
                                                                        vec3 r = refract (i, normalize(normal), ratio);
                                                                        fColor = texture(sceneBox, r) * (1 - alpha.a);
                                                                    } else {
                                                                        vec3 r = reflect (i, normalize(normal));
                                                                        fColor = texture(sceneBox, r) * (1 - alpha.a);
                                                                    }
                                                                }
                                                              )";
                const std::string fragmentShader = R"(#version 460
                                                      #extension GL_EXT_nonuniform_qualifier : enable
                                                      struct NodeType {
                                                          vec4 color;
                                                          float depth;
                                                          uint next;
                                                      };
                                                      layout(set = 0, binding = 0) buffer CounterSSBO {
                                                          uint count[6];
                                                          uint maxNodes;
                                                      };
                                                      layout(set = 0, binding = 1, r32ui) uniform coherent uimage3D headPointers;
                                                      layout(set = 0, binding = 2) buffer linkedLists {
                                                          NodeType nodes[];
                                                      };
                                                      layout(set = 0, binding = 3) uniform sampler2D textures[];
                                                      layout (location = 0) in vec4 frontColor;
                                                      layout (location = 1) in vec2 fTexCoords;
                                                      layout (location = 2) in flat uint layer;
                                                      layout (location = 3) in flat uint texIndex;
                                                      layout (location = 4) in vec3 normal;
                                                      layout(location = 0) out vec4 fcolor;
                                                      void main() {
                                                           uint nodeIdx = atomicAdd(count[layer], 1);
                                                           vec4 texel = (texIndex != 0) ? frontColor * texture(textures[texIndex-1], fTexCoords.xy) : frontColor;
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
                   layout(set = 0, binding = 0) buffer CounterSSBO {
                      uint count[6];
                      uint maxNodes;
                   };
                   layout(set = 0, binding = 1, r32ui) uniform uimage3D headPointers;
                   layout(set = 0, binding = 2) buffer linkedLists {
                       NodeType nodes[];
                   };
                   layout (location = 0) out vec4 fcolor;
                   layout (location = 0) in vec4 frontColor;
                   layout (location = 1) in vec2 fTexCoords;
                   layout (location = 2) in flat uint layer;
                   layout (location = 3) in vec3 normal;
                   void main() {
                      NodeType frags[MAX_FRAGMENTS];
                      int count = 0;
                      uint n = imageLoad(headPointers, ivec3(gl_FragCoord.xy, layer)).r;
                      while( n != 0xffffffffu && count < MAX_FRAGMENTS) {
                           frags[count] = nodes[n+maxNodes*layer];
                           n = frags[count].next+maxNodes*layer;
                           count++;
                      }
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
                    math::Matrix4f viewMatrix = getWindow().getDefaultView().getViewMatrix().getMatrix()/*.transpose()*/;
                    math::Matrix4f projMatrix = getWindow().getDefaultView().getProjMatrix().getMatrix()/*.transpose()*/;
                    linkedList2PC.viewMatrix = viewMatrix;
                    linkedList2PC.projMatrix = projMatrix;
            }
            void ReflectRefractRenderComponent::createImageView() {
                VkImageViewCreateInfo viewInfo{};
                viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                viewInfo.image = headPtrTextureImage;
                viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
                viewInfo.format = VK_FORMAT_R32_UINT;
                viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                viewInfo.subresourceRange.baseMipLevel = 0;
                viewInfo.subresourceRange.levelCount = 1;
                viewInfo.subresourceRange.baseArrayLayer = 0;
                viewInfo.subresourceRange.layerCount = 1;
                if (vkCreateImageView(vkDevice.getDevice(), &viewInfo, nullptr, &headPtrTextureImageView) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create head ptr texture image view!");
                }
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
                viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                viewInfo.image = alphaTextureImage;
                viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                viewInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                viewInfo.subresourceRange.baseMipLevel = 0;
                viewInfo.subresourceRange.levelCount = 1;
                viewInfo.subresourceRange.baseArrayLayer = 0;
                viewInfo.subresourceRange.layerCount = 1;
                if (vkCreateImageView(vkDevice.getDevice(), &viewInfo, nullptr, &alphaTextureImageView) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create head ptr texture image view!");
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
                if (vkCreateSampler(vkDevice.getDevice(), &samplerInfo, nullptr, &headPtrTextureSampler) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create texture sampler!");
                }

                samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                samplerInfo.magFilter = VK_FILTER_LINEAR;
                samplerInfo.minFilter = VK_FILTER_LINEAR;
                samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo.anisotropyEnable = VK_TRUE;
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
                samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                samplerInfo.magFilter = VK_FILTER_LINEAR;
                samplerInfo.minFilter = VK_FILTER_LINEAR;
                samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo.anisotropyEnable = VK_TRUE;
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
                if (vkCreateSampler(vkDevice.getDevice(), &samplerInfo, nullptr, &alphaTextureSampler) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create texture sampler!");
                }
            }
            void ReflectRefractRenderComponent::createDescriptorPool(RenderStates states) {
                Shader* shader = const_cast<Shader*>(states.shader);
                if (shader == &sLinkedList) {
                    unsigned int descriptorId = environmentMap.getId() * shader->getNbShaders() + shader->getId();
                    if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders()*environmentMap.getNbRenderTargets());
                    //std::cout<<"ppll descriptor id : "<<environmentMap.getId()<<","<<shader->getId()<<","<<environmentMap.getId() * shader->getNbShaders() + shader->getId()<<std::endl;
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    std::array<VkDescriptorPoolSize, 7> poolSizes;
                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[2].descriptorCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[3].descriptorCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight() * allTextures.size());
                    poolSizes[4].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[4].descriptorCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    poolSizes[5].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[5].descriptorCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    poolSizes[6].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    poolSizes[6].descriptorCount = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());

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
                    if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders()*environmentMap.getNbRenderTargets());
                    unsigned int descriptorId = environmentMap.getId() * shader->getNbShaders() + shader->getId();
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
                    poolInfo.maxSets = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de la creation de la pool de descripteurs!");
                    }
                } else if (shader == &sBuildAlphaBuffer) {
                    if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders()*alphaBuffer.getNbRenderTargets());
                    unsigned int descriptorId = alphaBuffer.getId() * shader->getNbShaders() + shader->getId();
                    std::array<VkDescriptorPoolSize, 5> poolSizes;
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[0].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight() * allTextures.size());
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    poolSizes[1].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());
                    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[2].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());
                    poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[3].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());
                    poolSizes[4].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes[4].descriptorCount = static_cast<uint32_t>(depthBuffer.getMaxFramesInFlight());

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
                } else {
                    if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorPool.size())
                        descriptorPool.resize(shader->getNbShaders()*reflectRefractTex.getNbRenderTargets());
                    unsigned int descriptorId = reflectRefractTex.getId() * shader->getNbShaders() + shader->getId();
                    std::array<VkDescriptorPoolSize, 4> poolSizes;
                    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[0].descriptorCount = 1;
                    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes[1].descriptorCount = 1;
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
                    poolInfo.maxSets = static_cast<uint32_t>(environmentMap.getMaxFramesInFlight());
                    if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("echec de la creation de la pool de descripteurs!");
                    }
                }
            }
            void ReflectRefractRenderComponent::createDescriptorSetLayout(RenderStates states) {
                Shader* shader = const_cast<Shader*>(states.shader);
                if (shader == &sLinkedList) {
                    if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorSetLayout.size())
                        descriptorSetLayout.resize(shader->getNbShaders()*environmentMap.getNbRenderTargets());
                    unsigned int descriptorId = environmentMap.getId() * shader->getNbShaders() + shader->getId();
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

                    VkDescriptorSetLayoutBinding uniformBufferLayoutBinding;
                    uniformBufferLayoutBinding.binding = 6;
                    uniformBufferLayoutBinding.descriptorCount = 1;
                    uniformBufferLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    uniformBufferLayoutBinding.pImmutableSamplers = nullptr;
                    uniformBufferLayoutBinding.stageFlags = VK_SHADER_STAGE_GEOMETRY_BIT;

                    if (descriptorSetLayout[descriptorId] != nullptr) {
                        vkDestroyDescriptorSetLayout(vkDevice.getDevice(), descriptorSetLayout[descriptorId], nullptr);
                    }

                    std::array<VkDescriptorSetLayoutBinding, 7> bindings = {counterLayoutBinding, headPtrImageLayoutBinding, linkedListLayoutBinding, samplerLayoutBinding, modelDataLayoutBinding, materialDataLayoutBinding, uniformBufferLayoutBinding};
                    VkDescriptorSetLayoutCreateInfo layoutInfo{};
                    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                    //layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
                    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());;
                    layoutInfo.pBindings = bindings.data();

                    if (vkCreateDescriptorSetLayout(vkDevice.getDevice(), &layoutInfo, nullptr, &descriptorSetLayout[descriptorId]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create descriptor set layout!");
                    }
                } else if (shader == &sLinkedList2) {
                    if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorSetLayout.size())
                        descriptorSetLayout.resize(shader->getNbShaders()*environmentMap.getNbRenderTargets());
                    unsigned int descriptorId = environmentMap.getId() * shader->getNbShaders() + shader->getId();
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
                    if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorSetLayout.size())
                        descriptorSetLayout.resize(shader->getNbShaders()*depthBuffer.getNbRenderTargets());
                    unsigned int descriptorId = depthBuffer.getId() * shader->getNbShaders() + shader->getId();
                    //std::cout<<"ppll descriptor id : "<<descriptorId<<std::endl;
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
                } else if (shader == &sBuildAlphaBuffer) {
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

                    VkDescriptorSetLayoutBinding sampler2LayoutBinding{};
                    sampler2LayoutBinding.binding = 2;
                    sampler2LayoutBinding.descriptorCount = 1;
                    sampler2LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    sampler2LayoutBinding.pImmutableSamplers = nullptr;
                    sampler2LayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

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

                    std::array<VkDescriptorSetLayoutBinding, 5> bindings = {samplerLayoutBinding, headPtrImageLayoutBinding, sampler2LayoutBinding, modelDataLayoutBinding, materialDataLayoutBinding};

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
                        descriptorSetLayout.resize(shader->getNbShaders()*reflectRefractTex.getNbRenderTargets());
                    unsigned int descriptorId = reflectRefractTex.getId() * shader->getNbShaders() + shader->getId();
                    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
                    samplerLayoutBinding.binding = 0;
                    samplerLayoutBinding.descriptorCount = 1;
                    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    samplerLayoutBinding.pImmutableSamplers = nullptr;
                    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                    VkDescriptorSetLayoutBinding sampler2LayoutBinding{};
                    sampler2LayoutBinding.binding = 1;
                    sampler2LayoutBinding.descriptorCount = 1;
                    sampler2LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    sampler2LayoutBinding.pImmutableSamplers = nullptr;
                    sampler2LayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

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
            void ReflectRefractRenderComponent::allocateDescriptorSets(RenderStates states) {
                Shader* shader = const_cast<Shader*>(states.shader);
                if (shader == &sLinkedList || shader == &sLinkedList2) {
                    if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorSets.size())
                        descriptorSets.resize(shader->getNbShaders()*environmentMap.getNbRenderTargets());
                    unsigned int descriptorId = environmentMap.getId() * shader->getNbShaders() + shader->getId();
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
                } else if (shader == &sBuildAlphaBuffer) {
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
                } else {
                    if (shader->getNbShaders()*RenderTarget::getNbRenderTargets() > descriptorSets.size())
                        descriptorSets.resize(shader->getNbShaders()*reflectRefractTex.getNbRenderTargets());
                    unsigned int descriptorId = reflectRefractTex.getId() * shader->getNbShaders() + shader->getId();
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
            void ReflectRefractRenderComponent::createDescriptorSets(RenderStates states) {
                Shader* shader = const_cast<Shader*>(states.shader);
                if (shader == &sLinkedList) {
                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    unsigned int descriptorId = environmentMap.getId() * shader->getNbShaders() + shader->getId();
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

                        VkDescriptorBufferInfo bufferInfo{};
                        bufferInfo.buffer = uniformBuffer[i];
                        bufferInfo.offset = 0;
                        bufferInfo.range = sizeof(UniformBufferObject);

                        descriptorWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[6].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[6].dstBinding = 6;
                        descriptorWrites[6].dstArrayElement = 0;
                        descriptorWrites[6].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                        descriptorWrites[6].descriptorCount = 1;
                        descriptorWrites[6].pBufferInfo = &bufferInfo;

                        vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
                    }
                } else if (shader == &sLinkedList2) {
                    unsigned int descriptorId = environmentMap.getId() * shader->getNbShaders() + shader->getId();
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
                        vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
                    }
                } else if (shader == &sBuildDepthBuffer) {
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
                } else if (shader == &sBuildAlphaBuffer) {
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
                            descriptorWrites[2].pImageInfo = descriptorImageInfos.data();

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
                    unsigned int descriptorId = reflectRefractTex.getId() * shader->getNbShaders() + shader->getId();
                    for (size_t i = 0; i < reflectRefractTex.getMaxFramesInFlight(); i++) {
                        VkDescriptorImageInfo   descriptorImageInfo;
                        descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        descriptorImageInfo.imageView = environmentMap.getTexture().getImageView();
                        descriptorImageInfo.sampler = environmentMap.getTexture().getSampler();

                        std::array<VkWriteDescriptorSet, 4> descriptorWrites{};
                        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[0].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[0].dstBinding = 0;
                        descriptorWrites[0].dstArrayElement = 0;
                        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[0].descriptorCount = 1;
                        descriptorWrites[0].pImageInfo = &descriptorImageInfo;

                        descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        descriptorImageInfo.imageView = alphaBuffer.getTexture().getImageView();
                        descriptorImageInfo.sampler = alphaBuffer.getTexture().getSampler();

                        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[1].dstSet = descriptorSets[descriptorId][i];
                        descriptorWrites[1].dstBinding = 1;
                        descriptorWrites[1].dstArrayElement = 0;
                        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[1].descriptorCount = 1;
                        descriptorWrites[1].pImageInfo = &descriptorImageInfo;

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
                }
            }
            void ReflectRefractRenderComponent::clear() {
                depthBuffer.clear(sf::Color::Transparent);
                std::vector<VkCommandBuffer>& commandBuffers = depthBuffer.getCommandBuffers();
                VkClearColorValue clearColor;
                clearColor.uint32[0] = 0;
                clearColor.uint32[1] = 0;
                clearColor.uint32[2] = 0;
                clearColor.uint32[3] = 0;
                VkImageSubresourceRange subresRange = {};
                subresRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                subresRange.levelCount = 1;
                subresRange.layerCount = 1;
                for (unsigned int i = 0; i < commandBuffers.size(); i++) {
                    vkCmdClearColorImage(commandBuffers[i], depthTextureImage, VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &subresRange);

                }
                depthBuffer.display();
                alphaBuffer.clear(sf::Color::Transparent);
                commandBuffers = alphaBuffer.getCommandBuffers();
                for (unsigned int i = 0; i < commandBuffers.size(); i++) {
                    vkCmdClearColorImage(commandBuffers[i], alphaTextureImage, VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &subresRange);

                }
                alphaBuffer.display();
                reflectRefractTex.clear(sf::Color::Transparent);
                reflectRefractTex.display();
            }
            void ReflectRefractRenderComponent::createCommandBuffersIndirect(unsigned int p, unsigned int nbIndirectCommands, unsigned int stride, DepthStencilID depthStencilID, RenderStates currentStates) {
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
                if (shader == &sBuildDepthBuffer) {
                    depthBuffer.beginRecordCommandBuffers();
                    std::vector<VkCommandBuffer> commandBuffers = depthBuffer.getCommandBuffers();
                    unsigned int currentFrame = depthBuffer.getCurrentFrame();
                    buildDepthPC.nbLayers = GameObject::getNbLayers();
                    vkCmdPushConstants(commandBuffers[currentFrame], depthBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][depthBuffer.getId()][depthStencilID], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(IndirectRenderingPC), &indirectRenderingPC);
                    vkCmdPushConstants(commandBuffers[currentFrame], depthBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][depthBuffer.getId()][depthStencilID], VK_SHADER_STAGE_FRAGMENT_BIT, 128, sizeof(BuildDepthPC), &buildDepthPC);
                    depthBuffer.drawIndirect(commandBuffers[currentFrame], currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], vboIndirect, depthStencilID,currentStates);
                    depthBuffer.display();
                } else if (shader == &sBuildAlphaBuffer) {
                    alphaBuffer.beginRecordCommandBuffers();
                    std::vector<VkCommandBuffer> commandBuffers = alphaBuffer.getCommandBuffers();
                    unsigned int currentFrame = alphaBuffer.getCurrentFrame();
                    buildAlphaPC.nbLayers = GameObject::getNbLayers();
                    buildAlphaPC.resolution = resolution;
                    vkCmdPushConstants(commandBuffers[currentFrame], alphaBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][alphaBuffer.getId()][depthStencilID], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(IndirectRenderingPC), &indirectRenderingPC);
                    vkCmdPushConstants(commandBuffers[currentFrame], alphaBuffer.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][alphaBuffer.getId()][depthStencilID], VK_SHADER_STAGE_FRAGMENT_BIT, 128, sizeof(BuildAlphaPC), &buildAlphaPC);
                    alphaBuffer.drawIndirect(commandBuffers[currentFrame], currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], vboIndirect, depthStencilID,currentStates);
                    alphaBuffer.display();
                } else if (shader == &sLinkedList) {
                    environmentMap.beginRecordCommandBuffers();
                    std::vector<VkCommandBuffer> commandBuffers = environmentMap.getCommandBuffers();
                    unsigned int currentFrame = environmentMap.getCurrentFrame();
                    environmentMap.drawIndirect(commandBuffers[currentFrame], currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], vboIndirect, depthStencilID,currentStates);
                    environmentMap.display();
                } else {
                    reflectRefractTex.beginRecordCommandBuffers();
                    std::vector<VkCommandBuffer> commandBuffers = reflectRefractTex.getCommandBuffers();
                    unsigned int currentFrame = reflectRefractTex.getCurrentFrame();
                    buildFrameBufferPC.resolution = resolution;
                    vkCmdPushConstants(commandBuffers[currentFrame], reflectRefractTex.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][reflectRefractTex.getId()][depthStencilID], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(IndirectRenderingPC), &indirectRenderingPC);
                    vkCmdPushConstants(commandBuffers[currentFrame], reflectRefractTex.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + p][reflectRefractTex.getId()][depthStencilID], VK_SHADER_STAGE_FRAGMENT_BIT, 128, sizeof(BuildFrameBufferPC), &buildFrameBufferPC);
                    reflectRefractTex.drawIndirect(commandBuffers[currentFrame], currentFrame, nbIndirectCommands, stride, vbBindlessTex[p], vboIndirect, depthStencilID,currentStates);
                    reflectRefractTex.display();
                }
            }
            void ReflectRefractRenderComponent::createCommandBufferVertexBuffer(RenderStates currentStates) {
                environmentMap.beginRecordCommandBuffers();
                Shader* shader = const_cast<Shader*>(currentStates.shader);
                unsigned int currentFrame = environmentMap.getCurrentFrame();
                std::vector<VkCommandBuffer> commandBuffers = environmentMap.getCommandBuffers();
                vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);


                VkMemoryBarrier memoryBarrier;
                memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                memoryBarrier.pNext = VK_NULL_HANDLE;
                memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;



                vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                vkCmdPushConstants(commandBuffers[currentFrame], environmentMap.getPipelineLayout()[shader->getId() * (Batcher::nbPrimitiveTypes - 1) + vb.getPrimitiveType()][environmentMap.getId()][NODEPTHNOSTENCIL], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(LinkedList2PC), &linkedList2PC);

                environmentMap.drawVertexBuffer(commandBuffers[currentFrame], currentFrame, vb, NODEPTHNOSTENCIL, currentStates);
                environmentMap.display();

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
                        materialDatas[p].push_back(material);
                        ModelData model;
                        TransformMatrix tm;
                        model.worldMat = tm.getMatrix().transpose();
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
                        materialDatas[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_reflInstances[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            ModelData model;
                            model.worldMat = tm[j]->getMatrix().transpose();
                            modelDatas[p].push_back(model);
                        }
                        std::cout<<"prim type : "<<p<<std::endl<<"model datas size : "<<modelDatas[p].size()<<std::endl;
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
                currentStates.blendMode = sf::BlendNone;
                currentStates.shader = &sBuildDepthBuffer;
                currentStates.texture = nullptr;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        vbBindlessTex[p].update();
                        VkDeviceSize bufferSize = sizeof(ModelData) * modelDatas[p].size();
                        std::cout<<"prim type : "<<p<<std::endl<<"model datas size : "<<modelDatas[p].size()<<std::endl;
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
                        //std::cout<<"layer : "<<layer<<" nb layers : "<<Entity::getNbLayers()<<std::endl;
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
                currentStates.blendMode = sf::BlendNone;
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
                        createCommandBuffersIndirect(p, drawArraysIndirectCommands[p].size(), sizeof(DrawArraysIndirectCommand), NODEPTHNOSTENCIL, currentStates);
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
                        material.textureIndex = (m_normals[i].getMaterial().getTexture() != nullptr) ? m_normals[i].getMaterial().getTexture()->getId() : 0;
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
                        unsigned int p = m_instances[i].getAllVertices().getPrimitiveType();MaterialData material;
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
                currentStates.blendMode = sf::BlendNone;
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
                        createCommandBuffersIndirect(p, drawArraysIndirectCommands[p].size(), sizeof(DrawArraysIndirectCommand), NODEPTHNOSTENCIL, currentStates);
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
                        materialDatas[p].push_back(material);
                        ModelData model;
                        TransformMatrix tm;
                        model.worldMat = tm.getMatrix().transpose();
                        modelDatas[p].push_back(model);
                        unsigned int vertexCount = 0;
                        for (unsigned int j = 0; j < m_reflNormals[i].getVertexArrays().size(); j++) {
                            Entity* entity = m_reflNormals[i].getVertexArrays()[j]->getEntity()->getRootEntity();
                            if (entity == reflectEntity) {
                                for (unsigned int k = 0; k < m_reflNormals[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                    vertexCount++;
                                    math::Vec3f t = m_reflNormals[i].getVertexArrays()[j]->getEntity()->getTransform().transform(math::Vec3f((*m_reflNormals[i].getVertexArrays()[j])[k].position.x, (*m_reflNormals[i].getVertexArrays()[j])[k].position.y, (*m_reflNormals[i].getVertexArrays()[j])[k].position.z));
                                    Vertex v (sf::Vector3f(t.x, t.y, t.z), (*m_reflNormals[i].getVertexArrays()[j])[k].color, (*m_reflNormals[i].getVertexArrays()[j])[k].texCoords);
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
                        materialDatas[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_reflInstances[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            ModelData model;
                            model.worldMat = tm[j]->getMatrix().transpose();
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
                currentStates.blendMode = sf::BlendNone;
                currentStates.shader = &sReflectRefract;
                currentStates.texture = &environmentMap.getTexture();
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
                        createCommandBuffersIndirect(p, drawArraysIndirectCommands[p].size(), sizeof(DrawArraysIndirectCommand), DEPTHNOSTENCIL, currentStates);
                    }
                }
            }
            void ReflectRefractRenderComponent::drawNextFrame() {
                RenderStates currentStates;
                math::Matrix4f viewMatrix = view.getViewMatrix().getMatrix()/*.transpose()*/;
                math::Matrix4f projMatrix = view.getProjMatrix().getMatrix()/*.transpose()*/;
                indirectRenderingPC.projMatrix = projMatrix;
                indirectRenderingPC.viewMatrix = viewMatrix;

                drawDepthReflInst();
                drawAlphaInst();

                View reflectView;
                if (view.isOrtho()) {
                    reflectView = View (squareSize, squareSize, view.getViewport().getPosition().z, view.getViewport().getSize().z);
                } else {
                    reflectView = View (squareSize, squareSize, 80, view.getViewport().getPosition().z, view.getViewport().getSize().z);
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
                                    if (entity->getSize().x > squareSize) {
                                        scale.x = entity->getSize().x / squareSize;
                                    }
                                    if (entity->getSize().y > squareSize) {
                                        scale.y = entity->getSize().y / squareSize;
                                    }*/
                                    //std::cout<<"scale : "<<scale<<"position : "<<entity->getPosition()<<std::endl;
                                    //reflectView.setScale(scale.x, scale.y, scale.z);
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
                                        reflectView.lookAt(target.x, target.y, target.z, ups[m]);
                                        projMatrix = reflectView.getProjMatrix().getMatrix().transpose();
                                        viewMatrix = reflectView.getViewMatrix().getMatrix().transpose();
                                        projMatrices[m] = projMatrix;
                                        viewMatrices[m] = viewMatrix;
                                        float zNear = reflectView.getViewport().getPosition().z;
                                        if (!reflectView.isOrtho())
                                            reflectView.setPerspective(80, view.getViewport().getSize().x / view.getViewport().getSize().y, 0.1f, view.getViewport().getSize().z);
                                        viewMatrix = reflectView.getViewMatrix().getMatrix().transpose();
                                        projMatrix = reflectView.getProjMatrix().getMatrix().transpose();
                                        math::Matrix4f sbViewMatrix = math::Matrix4f(math::Matrix3f(viewMatrix));
                                        sbViewMatrices[m] = sbViewMatrix;
                                        sbProjMatrices[m] = projMatrix;
                                        if (!reflectView.isOrtho())
                                            reflectView.setPerspective(80, view.getViewport().getSize().x / view.getViewport().getSize().y, zNear, view.getViewport().getSize().z);

                                    }
                                    environmentMap.clear(sf::Color::Transparent);
                                    VkClearColorValue clearColor;
                                    clearColor.uint32[0] = 0xffffffff;
                                    std::vector<VkCommandBuffer> commandBuffers = environmentMap.getCommandBuffers();
                                    for (unsigned int i = 0; i < commandBuffers.size(); i++) {
                                        VkImageSubresourceRange subresRange = {};
                                        subresRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                                        subresRange.levelCount = 1;
                                        subresRange.layerCount = 1;
                                        vkCmdClearColorImage(commandBuffers[i], headPtrTextureImage, VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &subresRange);
                                        for (unsigned int j = 0; j < 6; j++) {
                                            vkCmdFillBuffer(commandBuffers[i], counterShaderStorageBuffers[i], i*sizeof(uint32_t), sizeof(uint32_t), 0);
                                        }
                                    }
                                    environmentMap.display();
                                    /*vb.clear();
                                    //vb.name = "SKYBOXVB";
                                    for (unsigned int i = 0; i < m_skyboxInstance.size(); i++) {
                                        if (m_skyboxInstance[i].getAllVertices().getVertexCount() > 0) {
                                            vb.setPrimitiveType(m_skyboxInstance[i].getAllVertices().getPrimitiveType());
                                            for (unsigned int j = 0; j < m_skyboxInstance[i].getAllVertices().getVertexCount(); j++) {
                                                //std::cout<<"append"<<std::endl;
                                                vb.append(m_skyboxInstance[i].getAllVertices()[j]);
                                            }
                                        }
                                    }
                                    currentStates.blendMode = sf::BlendAlpha;
                                    currentStates.shader = &skyboxShader;
                                    currentStates.texture = (skybox == nullptr ) ? nullptr : &static_cast<g3d::Skybox*>(skybox)->getTexture();
                                    vb.update();
                                    environmentMap.drawVertexBuffer(vb, currentStates);*/
                                    UniformBufferObject ubo;
                                    for (unsigned int f = 0; f < 6; f++) {
                                        ubo.projMatrix[f] = projMatrices[f];
                                        ubo.viewMatrix[f] = viewMatrices[f];
                                    }
                                    updateUniformBuffer(environmentMap.getCurrentFrame(), ubo);
                                    drawEnvReflInst();
                                    vb.clear();
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
                                    math::Matrix4f matrix = quad.getTransform().getMatrix()/*.transpose()*/;
                                    linkedList2PC.worldMat = matrix;
                                    currentStates.shader = &sLinkedList2;
                                    currentStates.texture = nullptr;
                                    createCommandBufferVertexBuffer(currentStates);

                                    buildFrameBufferPC.cameraPos = math::Vec3f(view.getPosition().x, view.getPosition().y, view.getPosition().z);
                                    drawReflInst(entity);
                                }
                            }
                        }

                    }
                }
                //reflectRefractTex.display();
            }
            void ReflectRefractRenderComponent::draw(RenderTarget& target, RenderStates states) {
                reflectRefractTexSprite.setCenter(target.getView().getPosition());
                target.draw(reflectRefractTexSprite, states);
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
                return getPosition().z;
            }
            bool ReflectRefractRenderComponent::needToUpdate() {
                return update;
            }
            bool ReflectRefractRenderComponent::loadEntitiesOnComponent(std::vector<Entity*> vEntities) {
                batcher.clear();
                normalBatcher.clear();
                reflBatcher.clear();
                reflNormalBatcher.clear();
                skyboxBatcher.clear();
                if (skybox != nullptr) {
                    for (unsigned int i = 0; i < skybox->getFaces().size(); i++) {
                        skyboxBatcher.addFace(skybox->getFace(i));
                    }
                }
                for (unsigned int i = 0; i < vEntities.size(); i++) {
                    if ( vEntities[i] != nullptr && vEntities[i]->isLeaf()) {
                        //std::cout<<"add entity"<<std::endl;
                        for (unsigned int j = 0; j <  vEntities[i]->getNbFaces(); j++) {

                            if (vEntities[i]->getFace(j)->getMaterial().isReflectable() || vEntities[i]->getFace(j)->getMaterial().isRefractable()) {

                                if (vEntities[i]->getDrawMode() == Entity::INSTANCED) {
                                    //std::cout<<"add refl face"<<std::endl;
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
                m_instances = batcher.getInstances();
                m_normals = normalBatcher.getInstances();
                m_reflInstances = reflBatcher.getInstances();
                m_reflNormals = reflNormalBatcher.getInstances();
                m_skyboxInstance = skyboxBatcher.getInstances();
                visibleEntities = vEntities;
                update = true;
                return true;
            }
            ReflectRefractRenderComponent::~ReflectRefractRenderComponent() {
                vkDestroyCommandPool(vkDevice.getDevice(), commandPool, nullptr);

                vkDestroySampler(vkDevice.getDevice(), headPtrTextureSampler, nullptr);
                vkDestroyImageView(vkDevice.getDevice(), headPtrTextureImageView, nullptr);
                vkDestroyImage(vkDevice.getDevice(), headPtrTextureImage, nullptr);
                vkFreeMemory(vkDevice.getDevice(), headPtrTextureImageMemory, nullptr);

                vkDestroySampler(vkDevice.getDevice(), depthTextureSampler, nullptr);
                vkDestroyImageView(vkDevice.getDevice(), depthTextureImageView, nullptr);
                vkDestroyImage(vkDevice.getDevice(), depthTextureImage, nullptr);
                vkFreeMemory(vkDevice.getDevice(), depthTextureImageMemory, nullptr);

                vkDestroySampler(vkDevice.getDevice(), alphaTextureSampler, nullptr);
                vkDestroyImageView(vkDevice.getDevice(), alphaTextureImageView, nullptr);
                vkDestroyImage(vkDevice.getDevice(), alphaTextureImage, nullptr);
                vkFreeMemory(vkDevice.getDevice(), alphaTextureImageMemory, nullptr);

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
                std::cout<<"material data ssbo destroyed"<<std::endl;
                if (vboIndirect != VK_NULL_HANDLE) {
                    vkDestroyBuffer(vkDevice.getDevice(),vboIndirect, nullptr);
                    vkFreeMemory(vkDevice.getDevice(), vboIndirectMemory, nullptr);
                }
                std::cout<<"indirect vbo destroyed"<<std::endl;
            }
        #else
        ReflectRefractRenderComponent::ReflectRefractRenderComponent (RenderWindow& window, int layer, std::string expression, window::ContextSettings settings) :
            HeavyComponent(window, math::Vec3f(window.getView().getPosition().x, window.getView().getPosition().y, layer),
                          math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0),
                          math::Vec3f(window.getView().getSize().x + window.getView().getSize().x * 0.5f, window.getView().getPosition().y + window.getView().getSize().y * 0.5f, layer)),
            view(window.getView()),
            expression(expression)
            {
            if (window.getView().getSize().x > window.getView().getSize().y) {
                squareSize = window.getView().getSize().x;
            } else {
                squareSize = window.getView().getSize().y;
            }
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
            depthBuffer.create(window.getView().getSize().x, window.getView().getSize().y, settings);
            glCheck(glGenTextures(1, &depthTex));
            glCheck(glBindTexture(GL_TEXTURE_2D, depthTex));
            glCheck(glTexStorage2D(GL_TEXTURE_2D,1,GL_RGBA32F,window.getView().getSize().x, window.getView().getSize().y));
            glCheck(glBindImageTexture(0, depthTex, 1, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F));
            glCheck(glBindTexture(GL_TEXTURE_2D, 0));
            std::vector<GLfloat> depthClearBuf(window.getView().getSize().x*window.getView().getSize().y*4, 0);
            glCheck(glGenBuffers(1, &clearBuf2));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf2));
            glCheck(glBufferData(GL_PIXEL_UNPACK_BUFFER, depthClearBuf.size() * sizeof(GLfloat),
            &depthClearBuf[0], GL_STATIC_COPY));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
            depthBufferSprite = Sprite(depthBuffer.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), sf::IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));
            settings.depthBits = 0;
            alphaBuffer.create(window.getView().getSize().x, window.getView().getSize().y, settings);
            glCheck(glGenTextures(1, &alphaTex));
            glCheck(glBindTexture(GL_TEXTURE_2D, alphaTex));
            glCheck(glTexStorage2D(GL_TEXTURE_2D,1,GL_RGBA32F,window.getView().getSize().x, window.getView().getSize().y));
            glCheck(glBindImageTexture(0, alphaTex, 1, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F));
            glCheck(glBindTexture(GL_TEXTURE_2D, 0));
            std::vector<GLfloat> alphaClearBuf(window.getView().getSize().x*window.getView().getSize().y*4, 0);
            glCheck(glGenBuffers(1, &clearBuf3));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf3));
            glCheck(glBufferData(GL_PIXEL_UNPACK_BUFFER, alphaClearBuf.size() * sizeof(GLfloat),
            &alphaClearBuf[0], GL_STATIC_COPY));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
            alphaBufferSprite = Sprite(alphaBuffer.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), sf::IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));
            sf::Vector3i resolution ((int) window.getSize().x, (int) window.getSize().y, window.getView().getSize().z);
            settings.depthBits = 24;
            reflectRefractTex.create(window.getView().getSize().x, window.getView().getSize().y, settings);
            reflectRefractTex.setEnableCubeMap(true);
            reflectRefractTexSprite = Sprite(reflectRefractTex.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), sf::IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));
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
                           n = frags[count].next+maxNodes*layer;
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
                sBuildAlphaBuffer.setParameter("resolution", resolution.x, resolution.y, resolution.z);
                sReflectRefract.setParameter("resolution", resolution.x, resolution.y, resolution.z);
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
                //std::cout<<"add texture i : "<<i<<" id : "<<allTextures[i]->getNativeHandle()<<std::endl;
            }
            sBuildDepthBuffer.setParameter("textureMatrix", textureMatrices);
            sBuildAlphaBuffer.setParameter("textureMatrix", textureMatrices);
            sLinkedList.setParameter("textureMatrix", textureMatrices);



            //std::cout<<"ubid : "<<ubid<<std::endl;
            backgroundColor = sf::Color::Transparent;
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
            //std::cout<<"size : "<<sizeof(Samplers)<<" "<<alignof (alignas(16) uint64_t[200])<<std::endl;

            /*glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo2));
            glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo3));*/
            glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, modelDataBuffer));
            glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, materialDataBuffer));

            for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                vbBindlessTex[i].setPrimitiveType(static_cast<sf::PrimitiveType>(i));
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
                //std::cout<<"add texture i : "<<i<<" id : "<<allTextures[i]->getNativeHandle()<<std::endl;
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
            currentStates.blendMode = sf::BlendNone;
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
                    //std::cout<<"layer : "<<layer<<" nb layers : "<<Entity::getNbLayers()<<std::endl;
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
            currentStates.blendMode = sf::BlendNone;
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
            currentStates.blendMode = sf::BlendNone;
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
                                math::Vec3f t = m_reflNormals[i].getVertexArrays()[j]->getEntity()->getTransform().transform(math::Vec3f((*m_reflNormals[i].getVertexArrays()[j])[k].position.x, (*m_reflNormals[i].getVertexArrays()[j])[k].position.y, (*m_reflNormals[i].getVertexArrays()[j])[k].position.z));
                                Vertex v (sf::Vector3f(t.x, t.y, t.z), (*m_reflNormals[i].getVertexArrays()[j])[k].color, (*m_reflNormals[i].getVertexArrays()[j])[k].texCoords);
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
            currentStates.blendMode = sf::BlendNone;
            currentStates.shader = &sReflectRefract;
            currentStates.texture = &environmentMap.getTexture();
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
        }
        void ReflectRefractRenderComponent::drawNextFrame() {
            if (reflectRefractTex.getSettings().versionMajor >= 4 && reflectRefractTex.getSettings().versionMinor >= 3) {

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
                    reflectView = View (squareSize, squareSize, view.getViewport().getPosition().z, view.getViewport().getSize().z);
                } else {
                    reflectView = View (squareSize, squareSize, 80, view.getViewport().getPosition().z, view.getViewport().getSize().z);
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
                                    if (entity->getSize().x > squareSize) {
                                        scale.x = entity->getSize().x / squareSize;
                                    }
                                    if (entity->getSize().y > squareSize) {
                                        scale.y = entity->getSize().y / squareSize;
                                    }*/
                                    //std::cout<<"scale : "<<scale<<"position : "<<entity->getPosition()<<std::endl;
                                    //reflectView.setScale(scale.x, scale.y, scale.z);
                                    if (entity->getType() != "E_BIGTILE")
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
                                        reflectView.lookAt(target.x, target.y, target.z, ups[m]);
                                        projMatrix = reflectView.getProjMatrix().getMatrix().transpose();
                                        viewMatrix = reflectView.getViewMatrix().getMatrix().transpose();
                                        projMatrices[m] = projMatrix;
                                        viewMatrices[m] = viewMatrix;
                                        float zNear = reflectView.getViewport().getPosition().z;
                                        if (!reflectView.isOrtho())
                                            reflectView.setPerspective(80, view.getViewport().getSize().x / view.getViewport().getSize().y, 0.1f, view.getViewport().getSize().z);
                                        viewMatrix = reflectView.getViewMatrix().getMatrix().transpose();
                                        projMatrix = reflectView.getProjMatrix().getMatrix().transpose();
                                        math::Matrix4f sbViewMatrix = math::Matrix4f(math::Matrix3f(viewMatrix));
                                        sbViewMatrices[m] = sbViewMatrix;
                                        sbProjMatrices[m] = projMatrix;
                                        if (!reflectView.isOrtho())
                                            reflectView.setPerspective(80, view.getViewport().getSize().x / view.getViewport().getSize().y, zNear, view.getViewport().getSize().z);

                                    }
                                    environmentMap.clear(sf::Color::Transparent);
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
                                                //std::cout<<"append"<<std::endl;
                                                vb.append(m_skyboxInstance[i].getAllVertices()[j]);
                                            }
                                        }
                                    }
                                    currentStates.blendMode = sf::BlendAlpha;
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
                                    sReflectRefract.setParameter("cameraPos", view.getPosition().x, view.getPosition().y, view.getPosition().z);
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
            batcher.clear();
            normalBatcher.clear();
            reflBatcher.clear();
            reflNormalBatcher.clear();
            skyboxBatcher.clear();
            if (skybox != nullptr) {
                for (unsigned int i = 0; i < skybox->getFaces().size(); i++) {
                    skyboxBatcher.addFace(skybox->getFace(i));
                }
            }
            for (unsigned int i = 0; i < vEntities.size(); i++) {
                if ( vEntities[i] != nullptr && vEntities[i]->isLeaf()) {
                    for (unsigned int j = 0; j <  vEntities[i]->getNbFaces(); j++) {
                        if (vEntities[i]->getFace(j)->getMaterial().isReflectable() || vEntities[i]->getFace(j)->getMaterial().isRefractable()) {

                            if (vEntities[i]->getDrawMode() == Entity::INSTANCED) {
                                std::cout<<"add refl face"<<std::endl;
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
            m_instances = batcher.getInstances();
            m_normals = normalBatcher.getInstances();
            m_reflInstances = reflBatcher.getInstances();
            m_reflNormals = reflNormalBatcher.getInstances();
            m_skyboxInstance = skyboxBatcher.getInstances();
            visibleEntities = vEntities;
            update = true;
            return true;
        }
        bool ReflectRefractRenderComponent::needToUpdate() {
            return update;
        }
        void ReflectRefractRenderComponent::clear() {
            depthBuffer.clear(sf::Color::Transparent);
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf2));
            glCheck(glBindTexture(GL_TEXTURE_2D, depthTex));
            glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, view.getSize().x, view.getSize().y, GL_RGBA,
            GL_FLOAT, NULL));
            glCheck(glBindTexture(GL_TEXTURE_2D, 0));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
            alphaBuffer.clear(sf::Color::Transparent);
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf3));
            glCheck(glBindTexture(GL_TEXTURE_2D, alphaTex));
            glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, view.getSize().x, view.getSize().y, GL_RGBA,
            GL_FLOAT, NULL));
            glCheck(glBindTexture(GL_TEXTURE_2D, 0));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
            reflectRefractTex.clear(sf::Color::Transparent);

        }
        void ReflectRefractRenderComponent::setBackgroundColor (sf::Color color) {
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
            return getPosition().z;
        }
        void ReflectRefractRenderComponent::pushEvent(window::IEvent event, RenderWindow& rw) {
            if (event.type == window::IEvent::WINDOW_EVENT && event.window.type == window::IEvent::WINDOW_EVENT_RESIZED && &getWindow() == &rw && isAutoResized()) {
                std::cout<<"recompute size"<<std::endl;
                recomputeSize();
                getListener().pushEvent(event);
                getView().reset(physic::BoundingBox(getView().getViewport().getPosition().x, getView().getViewport().getPosition().y, getView().getViewport().getPosition().z, event.window.data1, event.window.data2, getView().getViewport().getDepth()));
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
