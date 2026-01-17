#include "../../../../include/odfaeg/Graphics/3D/animator.hpp"
namespace odfaeg {
    namespace graphic {
        namespace g3d {
            Animator::Animator(window::Device& vkDevice, Animation* animation, EntityFactory& factory) :
                GameObject (math::Vec3f(0, 0, 0), animation->getSize(), animation->getSize()*0.5f,"E_BONE_ANIMATION", factory),
                vkDevice(vkDevice),
                computeShader(vkDevice)
            {
                computeParams.entityId = animation->getModel()->getId();
                animation->getModel()->setParent(this);
                addChild(animation->getModel());

                m_CurrentTime = 0.0;
                m_CurrentAnimation = animation;

                m_FinalBoneMatrices.reserve(MAX_BONES);

                for (int i = 0; i < MAX_BONES; i++)
                    m_FinalBoneMatrices.push_back(math::Matrix4f());
                createCommandPool();
                createCommandBuffers();
                createDescriptorPool();
                createDescriptorSetLayout();
                allocateDescriptorSets();
                compileComputeShader();
                createComputePipeline();

            }

            void Animator::updateAnimation(float dt)
            {
                ////////std::cout<<"update anim"<<std::endl;
                m_DeltaTime = dt;
                if (m_CurrentAnimation)
                {
                    m_CurrentTime += m_CurrentAnimation->getTicksPerSecond() * dt;
                    m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->getDuration());
                    math::Matrix4f rootNodeTransform;
                    rootNodeTransform.identity();
                    calculateBoneTransform(&m_CurrentAnimation->getRootNode(), rootNodeTransform);
                }
                if (computeJob[currentFrame].load()) {
                    std::unique_lock<std::mutex> lock(*mtx);
                    computeJob[currentFrame] = false;
                    vkResetCommandBuffer(commandBuffers[currentFrame], 0);
                    VkCommandBufferBeginInfo beginInfo{};
                    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                    /*if (m_name == "depthBuffer")
                        //////std::cout<<"begin cmd : "<<commandBuffers.size()<<std::endl;*/
                    if (vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo) != VK_SUCCESS) {

                        throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                    }
                    if (vertexBuffer.value().get().getVertexCount() > 0) {



                        VkDeviceSize bufferSize = sizeof(math::Matrix4f) * m_FinalBoneMatrices.size();

                        if (bufferSize > maxFinalBonesMatricesSize) {
                            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                                if (stagingFinalBonesMatrices[i] != nullptr) {
                                    vkDestroyBuffer(vkDevice.getDevice(), stagingFinalBonesMatrices[i], nullptr);
                                    vkFreeMemory(vkDevice.getDevice(), stagingFinalBonesMatricesMemory[i], nullptr);
                                }

                                createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,stagingFinalBonesMatrices[i], stagingFinalBonesMatricesMemory[i]);

                                if (finalBonesMatrices[i] != nullptr) {
                                    vkDestroyBuffer(vkDevice.getDevice(),finalBonesMatrices[i], nullptr);
                                    vkFreeMemory(vkDevice.getDevice(), finalBonesMatricesMemory[i], nullptr);
                                }

                                createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, finalBonesMatrices[i], finalBonesMatricesMemory[i]);
                            }

                            maxFinalBonesMatricesSize = bufferSize;
                            //needToUpdateDSs[p]  = true;
                        }

                        void* data;
                        vkMapMemory(vkDevice.getDevice(), stagingFinalBonesMatricesMemory[currentFrame], 0, bufferSize, 0, &data);
                        memcpy(data, m_FinalBoneMatrices.data(), (size_t)bufferSize);
                        vkUnmapMemory(vkDevice.getDevice(), stagingFinalBonesMatricesMemory[currentFrame]);
                        copyBuffer(stagingFinalBonesMatrices[currentFrame], finalBonesMatrices[currentFrame], bufferSize, commandBuffers[currentFrame]);
                        updateDescriptorSets();
                        VkBufferMemoryBarrier b{};
                        b.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
                        b.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
                        b.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // or VERTEX_ATTRIBUTE_READ
                        b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                        b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                        b.buffer = vertexBuffer.value().get().getVertexBuffer(currentFrame);
                        b.offset = 0;
                        b.size = VK_WHOLE_SIZE;

                        vkCmdPipelineBarrier(
                          commandBuffers[currentFrame], // or graphicsCmd if reading there
                          VK_PIPELINE_STAGE_HOST_BIT,
                          VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, // or VERTEX_SHADER/FRAGMENT_SHADER
                          0,
                          0, nullptr,
                          1, &b,
                          0, nullptr
                        );

                        vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);
                        vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout, 0, 1, &computeDescriptorSets[currentFrame], 0, 0);
                        vkCmdPushConstants(commandBuffers[currentFrame], computePipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ComputePC), &computeParams);
                        vkCmdDispatch(commandBuffers[currentFrame], vertexBuffer.value().get().getVertexCount(), 1, 1);
                    }
                    if (vkEndCommandBuffer(commandBuffers[currentFrame]) != VK_SUCCESS) {
                        throw core::Erreur(0, "failed to record command buffer!", 1);
                    }
                    VkSubmitInfo submitInfo{};
                    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                    submitInfo.signalSemaphoreCount = 1;
                    submitInfo.pSignalSemaphores = &computeSemaphores;
                    submitInfo.commandBufferCount = 1;
                    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
                    if (vkQueueSubmit(vkDevice.getComputeQueue(), 1, &submitInfo, computeFences) != VK_SUCCESS) {
                        throw core::Erreur(0, "échec de l'envoi d'un command buffer!", 1);
                    }
                    //std::cout<<"compute finished"<<std::endl;
                    computeFinished[currentFrame] = true;
                    cv2->notify_one();
                }
            }

            void Animator::playAnimation(Animation* pAnimation)
            {
                m_CurrentAnimation = pAnimation;
                m_CurrentTime = 0.0f;
            }

            void Animator::calculateBoneTransform(const Animation::AssimpNodeData* node, math::Matrix4f parentTransform)
            {
                std::string nodeName = node->name;
                math::Matrix4f nodeTransform = node->transformation;


                Bone* bone = m_CurrentAnimation->findBone(nodeName);

                if (bone)
                {
                    ////////std::cout<<"update"<<std::endl;
                    bone->update(m_CurrentTime);
                    nodeTransform = bone->getLocalTransform();

                }
                //std::cout<<"node transform : "<<nodeTransform<<std::endl;
                math::Matrix4f globalTransformation = parentTransform * nodeTransform;


                auto boneInfoMap = m_CurrentAnimation->getBoneIDMap();
                if (boneInfoMap.find(nodeName) != boneInfoMap.end())
                {
                    int index = boneInfoMap[nodeName].id;
                    math::Matrix4f offset = boneInfoMap[nodeName].offset;
                    m_FinalBoneMatrices[index] = globalTransformation * offset;
                    //std::cout<<"final bone transform : "<<m_FinalBoneMatrices[index]<<std::endl;
                }


                for (int i = 0; i < node->childrenCount; i++)
                    calculateBoneTransform(&node->children[i], globalTransformation);
            }

            std::vector<math::Matrix4f> Animator::getFinalBoneMatrices()
            {
                return m_FinalBoneMatrices;
            }
            Entity* Animator::clone() {
                Animator* a = factory.make_entity<Animator>(vkDevice, m_CurrentAnimation, factory);
                GameObject::copy(a);
                return a;
            }
            Entity* Animator::getCurrentFrame() const {
                return m_CurrentAnimation->getModel();
            }
            void Animator::computeParticles(std::mutex* mtx, std::condition_variable* cv2, VertexBuffer& frameVertexBuffer, unsigned int currentFrame, TransformMatrix tm, bool instanced, VkSemaphore computeSemaphore, VkFence computeFence) {
                computeFinished[currentFrame] = false;
                this->mtx = mtx;
                this->cv2 = cv2;
                this->vertexBuffer = std::ref(frameVertexBuffer);
                this->currentFrame = currentFrame;
                this->computeSemaphores = computeSemaphore;
                this->computeFences = computeFence;
                computeParams.instanced = (instanced) ? 1 : 0;
                tm.update();
                computeParams.transform = tm.getMatrix().transpose();
                //std::cout<<"matrix : "<<computeParams.transform<<std::endl;
                computeJob[currentFrame] = true;
            }
            bool Animator::isComputeFinished(unsigned int currentFrame) {
                return computeFinished[currentFrame].load();
            }
            void Animator::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
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
            void Animator::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandBuffer cmd) {
                if (srcBuffer != nullptr && dstBuffer != nullptr) {
                    VkBufferCopy copyRegion{};
                    copyRegion.size = size;
                    vkCmdCopyBuffer(cmd, srcBuffer, dstBuffer, 1, &copyRegion);
                }
            }
            uint32_t Animator::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
                VkPhysicalDeviceMemoryProperties memProperties;
                vkGetPhysicalDeviceMemoryProperties(vkDevice.getPhysicalDevice(), &memProperties);
                for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
                    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                        return i;
                    }
                }
                throw std::runtime_error("aucun type de memoire ne satisfait le buffer!");
            }
            void Animator::compileComputeShader() {
                std::string computeShaderCode = R"(#version 460
                                                   #extension GL_EXT_debug_printf : enable
                                                   const int MAX_BONE_INFLUENCE = 4;
                                                   const int MAX_BONES = 100;
                                                   struct Vertex {
                                                     vec3 position;
                                                     vec2 texCoords;
                                                     vec3 normal;
                                                     int boneIds[MAX_BONE_INFLUENCE];
                                                     float weights[MAX_BONE_INFLUENCE];
                                                     uint color;
                                                     int entityId;
                                                     int particleId;
                                                     int padding;
                                                   };
                                                   layout (std430, binding = 0) buffer verticesSSBO {
                                                       Vertex vertices[];
                                                   };
                                                   layout (std430, binding = 1) buffer finalBonesMatricesSSBO {
                                                       mat4 finalBonesMatrices[MAX_BONES];
                                                   };
                                                   layout (push_constant) uniform PushConsts {
                                                         int entityId;
                                                         int instanced;
                                                         int padding1;
                                                         int padding2;
                                                         mat4 transform;
                                                   } pushConsts;
                                                   layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
                                                   void main() {
                                                       uint vertexIndex = gl_GlobalInvocationID.x;

                                                       if (pushConsts.entityId == vertices[vertexIndex].entityId) {
                                                           vec4 totalPosition = vec4(0.0f);
                                                           for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
                                                           {
                                                              if(vertices[vertexIndex].boneIds[i] == -1)
                                                                 continue;
                                                              if(vertices[vertexIndex].boneIds[i] >=MAX_BONES)
                                                              {
                                                                  totalPosition = vec4(vertices[vertexIndex].position.xyz, 1);
                                                                  break;
                                                              }
                                                              totalPosition += vertices[vertexIndex].weights[i] * (vec4(vertices[vertexIndex].position.xyz, 1) * finalBonesMatrices[vertices[vertexIndex].boneIds[i]]);

                                                            }
                                                            vertices[vertexIndex].position = vec3(totalPosition.xyz);
                                                            if (pushConsts.instanced == 0) {
                                                                //debugPrintfEXT("m1 : %v4f\nm2 : %v4f\nm3 : %v4f\nm4 : %v4f\n", pushConsts.transform[0],pushConsts.transform[1],pushConsts.transform[2],pushConsts.transform[3]);
                                                                vertices[vertexIndex].position = (vec4(vertices[vertexIndex].position.xyz, 1) * pushConsts.transform).xyz;
                                                            }
                                                       }
                                                   })";
                if (!computeShader.loadFromMemory(computeShaderCode)) {
                    throw core::Erreur(61, "Failed to compile compute animator shader");
                }
            }
            void Animator::createCommandPool() {
                window::Device::QueueFamilyIndices queueFamilyIndices = vkDevice.findQueueFamilies(vkDevice.getPhysicalDevice(), VK_NULL_HANDLE);

                VkCommandPoolCreateInfo poolInfo{};
                poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                poolInfo.queueFamilyIndex = queueFamilyIndices.computeFamily.value();
                poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optionel
                if (vkCreateCommandPool(vkDevice.getDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
                    throw core::Erreur(0, "échec de la création d'une command pool!", 1);
                }
            }
            void Animator::createCommandBuffers() {
                VkCommandBufferAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocInfo.commandPool = commandPool;
                allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();
                if (vkAllocateCommandBuffers(vkDevice.getDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to allocate command buffers!", 1);
                }
            }
            void Animator::createDescriptorPool() {
                std::array<VkDescriptorPoolSize, 2> poolSizes{};
                poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
                poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
                VkDescriptorPoolCreateInfo poolInfo{};
                poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
                poolInfo.pPoolSizes = poolSizes.data();
                poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
                if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &computeDescriptorPool) != VK_SUCCESS) {
                    throw std::runtime_error("echec de la creation de la pool de descripteurs!");
                }
            }
            void Animator::createDescriptorSetLayout() {
                std::array<VkDescriptorSetLayoutBinding, 2> layoutBindings{};
                layoutBindings[0].binding = 0;
                layoutBindings[0].descriptorCount = 1;
                layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                layoutBindings[0].pImmutableSamplers = nullptr;
                layoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

                layoutBindings[1].binding = 1;
                layoutBindings[1].descriptorCount = 1;
                layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                layoutBindings[1].pImmutableSamplers = nullptr;
                layoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;


                VkDescriptorSetLayoutCreateInfo layoutInfo{};
                layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                layoutInfo.bindingCount = layoutBindings.size();
                layoutInfo.pBindings = layoutBindings.data();

                if (vkCreateDescriptorSetLayout(vkDevice.getDevice(), &layoutInfo, nullptr, &computeDescriptorSetLayout) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create compute descriptor set layout!");
                }
            }
            void Animator::allocateDescriptorSets() {
                std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, computeDescriptorSetLayout);
                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

                allocInfo.descriptorPool = computeDescriptorPool;
                allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
                allocInfo.pSetLayouts = layouts.data();
                if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, computeDescriptorSets.data()) != VK_SUCCESS) {
                    throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
                }
            }
            void Animator::updateDescriptorSets() {
                for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

                    VkDescriptorBufferInfo verticesStorageBufferInfoLastFrame{};
                    verticesStorageBufferInfoLastFrame.buffer = vertexBuffer.value().get().getVertexBuffer(i);
                    verticesStorageBufferInfoLastFrame.offset = 0;
                    verticesStorageBufferInfoLastFrame.range = vertexBuffer.value().get().getVertexCount() * sizeof(Vertex);

                    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[0].dstSet = computeDescriptorSets[i];
                    descriptorWrites[0].dstBinding = 0;
                    descriptorWrites[0].dstArrayElement = 0;
                    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descriptorWrites[0].descriptorCount = 1;
                    descriptorWrites[0].pBufferInfo = &verticesStorageBufferInfoLastFrame;

                    VkDescriptorBufferInfo finalBonesMatricesStorageBufferInfoLastFrame{};
                    finalBonesMatricesStorageBufferInfoLastFrame.buffer = finalBonesMatrices[i];
                    finalBonesMatricesStorageBufferInfoLastFrame.offset = 0;
                    finalBonesMatricesStorageBufferInfoLastFrame.range = m_FinalBoneMatrices.size() * sizeof(math::Matrix4f);

                    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[1].dstSet = computeDescriptorSets[i];
                    descriptorWrites[1].dstBinding = 1;
                    descriptorWrites[1].dstArrayElement = 0;
                    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descriptorWrites[1].descriptorCount = 1;
                    descriptorWrites[1].pBufferInfo = &finalBonesMatricesStorageBufferInfoLastFrame;

                    vkUpdateDescriptorSets(vkDevice.getDevice(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
                }
            }
            void Animator::createComputePipeline() {
                computeShader.createComputeShaderModule();
                VkShaderModule computeShaderModule = computeShader.getComputeShaderModule();
                VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
                computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
                computeShaderStageInfo.module = computeShaderModule;
                computeShaderStageInfo.pName = "main";

                VkPushConstantRange push_constant;
                push_constant.offset = 0;
                //this push constant range takes up the size of a MeshPushConstants struct
                push_constant.size = sizeof(ComputePC);
                //this push constant range is accessible only in the vertex shader
                push_constant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

                VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
                pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
                pipelineLayoutInfo.setLayoutCount = 1;
                pipelineLayoutInfo.pSetLayouts = &computeDescriptorSetLayout;
                pipelineLayoutInfo.pPushConstantRanges = &push_constant;
                pipelineLayoutInfo.pushConstantRangeCount = 1;

                if (vkCreatePipelineLayout(vkDevice.getDevice(), &pipelineLayoutInfo, nullptr, &computePipelineLayout) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create compute pipeline layout!");
                }
                VkComputePipelineCreateInfo pipelineInfo{};
                pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
                pipelineInfo.layout = computePipelineLayout;
                pipelineInfo.stage = computeShaderStageInfo;

                if (vkCreateComputePipelines(vkDevice.getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create compute pipeline!");
                }
                computeShader.cleanupComputeShaderModule();
            }
            Animator::~Animator() {
                vkFreeCommandBuffers(vkDevice.getDevice(), commandPool, commandBuffers.size(), commandBuffers.data());
                vkDestroyCommandPool(vkDevice.getDevice(), commandPool, nullptr);
                vkDestroyPipelineLayout(vkDevice.getDevice(), computePipelineLayout, nullptr);
                vkDestroyPipeline(vkDevice.getDevice(), computePipeline, nullptr);
                vkDestroyDescriptorSetLayout(vkDevice.getDevice(), computeDescriptorSetLayout, nullptr);
                vkDestroyDescriptorPool(vkDevice.getDevice(), computeDescriptorPool, nullptr);
                for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    vkDestroyBuffer(vkDevice.getDevice(), finalBonesMatrices[i], nullptr);
                    vkFreeMemory(vkDevice.getDevice(), finalBonesMatricesMemory[i], nullptr);
                    vkDestroyBuffer(vkDevice.getDevice(), stagingFinalBonesMatrices[i], nullptr);
                    vkFreeMemory(vkDevice.getDevice(), stagingFinalBonesMatricesMemory[i], nullptr);
                }
                delete m_CurrentAnimation;
            }
        }
    }
}
