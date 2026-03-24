#include "../../../../include/odfaeg/Graphics/3D/animator.hpp"
namespace odfaeg {
    namespace graphic {
        namespace g3d {
            Animator::Animator(window::Device& vkDevice, Animation* animation, EntityFactory& factory) :
                GameObject (animation->getModel()->getPosition(), animation->getModel()->getSize(), animation->getModel()->getOrigin(),"E_BONE_ANIMATION", factory),
                vkDevice(vkDevice),
                computeShader(vkDevice)
            {
                computeParams.entityId = getId();
                std::cout<<"animator id  : "<<getId()<<std::endl;
                animation->getModel()->setParent(this);
                addChild(animation->getModel());
                remapEntityId(this);


                m_CurrentTime = 0.0;
                m_CurrentAnimation = animation;
                setBoneParent(&m_CurrentAnimation->getRootNode());

                m_FinalBoneMatrices.reserve(MAX_BONES);

                for (int i = 0; i < MAX_BONES; i++)
                    m_FinalBoneMatrices.push_back(math::Matrix4f());
                for (int i = 0; i < MAX_BONES; i++)
                    m_FinalBoneGlobalMatrices.push_back(math::Matrix4f());
                createCommandPool();
                createDescriptorPool();
                createDescriptorSetLayout();
                allocateDescriptorSets();
                compileComputeShader();
                createComputePipeline();

            }
            void Animator::remapEntityId(Entity* entity) {
                for (unsigned int f = 0; f < entity->getFaces().size(); f++) {
                    entity->getFaces()[f].getVertexArray().setEntity(this);
                }
                for (unsigned int i = 0; i < entity->getChildren().size(); i++) {
                    remapEntityId(entity->getChildren()[i]);
                }
            }
            void Animator::setBoneParent(const Animation::AssimpNodeData* node) {
                std::string nodeName = node->name;
                Bone* bone = m_CurrentAnimation->findBone(nodeName);
                if (bone)
                {
                    ////////std::cout<<"update"<<std::endl;
                    addChild(bone);
                    bone->setParent(this);

                }
                for (int i = 0; i < node->childrenCount; i++)
                    setBoneParent(&node->children[i]);
            }
            void Animator::updateAnimation(float dt)
            {
                std::lock_guard<std::recursive_mutex> lock(rec_mutex);
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

                for (unsigned int layer = 0; layer < computeJob.size(); layer++) {
                    if (computeJob[layer][currentFrame[layer]].load()) {
                        std::unique_lock<std::mutex> lock(*mtx[layer]);
                        computeJob[layer][currentFrame[layer]] = false;
                        vkResetCommandBuffer(commandBuffers[layer][currentFrame[layer]], 0);
                        VkCommandBufferBeginInfo beginInfo{};
                        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

                        if (vkBeginCommandBuffer(commandBuffers[layer][currentFrame[layer]], &beginInfo) != VK_SUCCESS) {

                            throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                        }
                        if (vertexBuffer[layer].has_value() && vertexBuffer[layer].value().get().getVertexCount() > 0) {

                            for (unsigned int l = 0; l <= layer; l++) {

                                VkDeviceSize bufferSize = sizeof(math::Matrix4f) * m_FinalBoneMatrices.size();

                                if (bufferSize > maxFinalBonesMatricesSize[l][currentFrame[layer]]) {
                                    if (stagingFinalBonesMatrices[l][currentFrame[layer]] != nullptr) {
                                        vkDestroyBuffer(vkDevice.getDevice(), stagingFinalBonesMatrices[l][currentFrame[layer]], nullptr);
                                        vkFreeMemory(vkDevice.getDevice(), stagingFinalBonesMatricesMemory[l][currentFrame[layer]], nullptr);
                                    }

                                    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,stagingFinalBonesMatrices[l][currentFrame[layer]], stagingFinalBonesMatricesMemory[l][currentFrame[layer]]);

                                    if (finalBonesMatrices[l][currentFrame[layer]] != nullptr) {
                                        vkDestroyBuffer(vkDevice.getDevice(),finalBonesMatrices[l][currentFrame[layer]], nullptr);
                                        vkFreeMemory(vkDevice.getDevice(), finalBonesMatricesMemory[l][currentFrame[layer]], nullptr);
                                    }

                                    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, finalBonesMatrices[l][currentFrame[layer]], finalBonesMatricesMemory[l][currentFrame[layer]]);

                                    maxFinalBonesMatricesSize[l][currentFrame[layer]] = bufferSize;
                                    //needToUpdateDSs[p]  = true;
                                }
                            }

                            VkDeviceSize bufferSize = sizeof(math::Matrix4f) * m_FinalBoneMatrices.size();
                            void* data;
                            vkMapMemory(vkDevice.getDevice(), stagingFinalBonesMatricesMemory[layer][currentFrame[layer]], 0, bufferSize, 0, &data);
                            memcpy(data, m_FinalBoneMatrices.data(), (size_t)bufferSize);
                            vkUnmapMemory(vkDevice.getDevice(), stagingFinalBonesMatricesMemory[layer][currentFrame[layer]]);
                            copyBuffer(stagingFinalBonesMatrices[layer][currentFrame[layer]], finalBonesMatrices[layer][currentFrame[layer]], bufferSize, commandBuffers[layer][currentFrame[layer]]);
                            updateDescriptorSets(currentFrame[layer]);
                            VkBufferMemoryBarrier b{};
                            b.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
                            b.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
                            b.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // or VERTEX_ATTRIBUTE_READ
                            b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                            b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                            b.buffer = vertexBuffer[layer].value().get().getVertexBuffer(currentFrame[layer]);
                            b.offset = 0;
                            b.size = VK_WHOLE_SIZE;

                            vkCmdPipelineBarrier(
                              commandBuffers[layer][currentFrame[layer]], // or graphicsCmd if reading there
                              VK_PIPELINE_STAGE_HOST_BIT,
                              VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, // or VERTEX_SHADER/FRAGMENT_SHADER
                              0,
                              0, nullptr,
                              1, &b,
                              0, nullptr
                            );

                            vkCmdBindPipeline(commandBuffers[layer][currentFrame[layer]], VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);
                            std::vector<VkDescriptorSet> sets;
                            for (unsigned int i = 0; i < computeDescriptorSets.size(); i++) {
                                sets.push_back(computeDescriptorSets[i][currentFrame[layer]]);
                            }
                            vkCmdBindDescriptorSets(commandBuffers[layer][currentFrame[layer]], VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout, 0, sets.size(), sets.data(), 0, 0);
                            computeParams.layer = layer;
                            vkCmdPushConstants(commandBuffers[layer][currentFrame[layer]], computePipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ComputePC), &computeParams);
                            vkCmdDispatch(commandBuffers[layer][currentFrame[layer]], vertexBuffer[layer].value().get().getVertexCount(), 1, 1);
                        }
                        if (vkEndCommandBuffer(commandBuffers[layer][currentFrame[layer]]) != VK_SUCCESS) {
                            throw core::Erreur(0, "failed to record command buffer!", 1);
                        }
                        VkSubmitInfo submitInfo{};
                        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                        submitInfo.signalSemaphoreCount = 1;
                        submitInfo.pSignalSemaphores = &computeSemaphores[layer][currentFrame[layer]];
                        submitInfo.commandBufferCount = 1;
                        submitInfo.pCommandBuffers = &commandBuffers[layer][currentFrame[layer]];
                        window::Device::QueueFamilyIndices indices = vkDevice.findQueueFamilies(vkDevice.getPhysicalDevice(), nullptr);
                        if (vkQueueSubmit(vkDevice.getQueue(indices.computeFamily.value(), 1), 1, &submitInfo, computeFences[layer][currentFrame[layer]]) != VK_SUCCESS) {
                            throw core::Erreur(0, "échec de l'envoi d'un compute command buffer!", 1);
                        }
                        //std::cout<<"compute finished"<<std::endl;
                        computeFinished[layer][currentFrame[layer]] = true;
                        cv2[layer]->notify_one();
                    }
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
                //std::cout<<"bone : "<<bone<<std::endl;

                if (bone)
                {
                    ////////std::cout<<"update"<<std::endl;
                    bone->update(m_CurrentTime);
                    nodeTransform = bone->getLocalTransform();

                }
                //std::cout<<"bone update done : "<<bone<<std::endl;

                //std::cout<<"node transform : "<<std::endl;
                math::Matrix4f globalTransformation = parentTransform * nodeTransform;

                //std::cout<<"get bone info map : "<<m_CurrentAnimation<<std::endl;
                auto boneInfoMap = m_CurrentAnimation->getBoneIDMap();
                //std::cout<<"bone info map"<<std::endl;
                if (boneInfoMap.find(nodeName) != boneInfoMap.end())
                {
                    int index = boneInfoMap[nodeName].id;
                    //std::cout<<"index : "<<index<<std::endl;
                    math::Matrix4f offset = boneInfoMap[nodeName].offset;
                    m_FinalBoneMatrices[index] = globalTransformation * offset;
                    m_FinalBoneGlobalMatrices[index] = globalTransformation;
                    //std::cout<<"final bone transform : "<<m_FinalBoneMatrices[index]<<std::endl;
                }
                //std::cout<<"bone info map done"<<std::endl;
                if (bone) {
                    //std::cout<<"bone children : "<<bone->getChildren().size()<<std::endl;
                    for (unsigned int i = 0; i < bone->getChildren().size(); i++) {
                        TransformMatrix tm =  m_CurrentAnimation->getModel()->getTransform();
                        tm.update();
                        tm.combine(m_FinalBoneGlobalMatrices[bone->getBoneID()]);
                        bone->getChildren()[i]->setTransform(tm.getMatrix());
                    }
                    //std::cout<<"bone children transform ok : "<<std::endl;
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
            void Animator::computeParticles(std::mutex* mtx, std::condition_variable* cv2, VertexBuffer& frameVertexBuffer, unsigned int currentFrame, TransformMatrix tm, bool instanced, std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> computeSemaphore, std::array<VkFence, MAX_FRAMES_IN_FLIGHT> computeFence, unsigned int layer) {
                std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                computeFinished[layer][currentFrame] = false;
                if (layer >= computeFinished.size()) {
                    size_t oldSize = computeFinished.size();
                    computeFinished.resize(layer+1);
                    for (size_t i = oldSize; i < computeFinished.size(); ++i) {
                        for (auto& flag : computeFinished[i]) {
                            flag.store(false, std::memory_order_relaxed);
                        }
                    }
                    oldSize = computeJob.size();
                    computeJob.resize(layer+1);
                    for (size_t i = oldSize; i < computeJob.size(); ++i) {
                        for (auto& flag : computeJob[i]) {
                            flag.store(false, std::memory_order_relaxed);
                        }
                    }
                    vertexBuffer.resize(layer + 1);
                    this->currentFrame.resize(layer + 1);
                    this->mtx.resize(layer + 1);
                    this->cv2.resize(layer + 1);
                    finalBonesMatrices.resize(layer + 1);
                    finalBonesMatricesMemory.resize(layer + 1);
                    stagingFinalBonesMatrices.resize(layer + 1);
                    stagingFinalBonesMatricesMemory.resize(layer + 1);
                    maxFinalBonesMatricesSize.resize(layer + 1);
                    oldSize = commandBuffers.size();
                    commandBuffers.resize(layer + 1);
                    computeSemaphores.resize(layer + 1);
                    computeFences.resize(layer + 1);
                    createCommandBuffers(oldSize);
                    this->mtx[layer] = mtx;
                    this->cv2[layer] = cv2;

                }


                this->vertexBuffer[layer] = std::ref(frameVertexBuffer);
                for (unsigned int i = 0; i < layer; i++) {
                    if (!this->vertexBuffer[i].has_value())
                        this->vertexBuffer[i] = std::ref(frameVertexBuffer);
                }
                /*for (unsigned int i = 0; i < vertexBuffer[layer].value().get().getVertexCount(); i++) {
                    std::cout<<"entity id : "<<vertexBuffer[layer].value().get()[i].entityId<<std::endl;
                    for (unsigned int j = 0; j < MAX_BONE_INFLUENCE; j++) {
                        if (vertexBuffer[layer].value().get()[i].m_BoneIDs[j] != -1)
                            std::cout<<"bones : "<<vertexBuffer[layer].value().get()[i].m_BoneIDs[j]<<std::endl;
                    }
                }*/
                this->currentFrame[layer] = currentFrame;
                this->computeSemaphores[layer] = computeSemaphore;
                this->computeFences[layer] = computeFence;
                computeParams.instanced = (instanced) ? 1 : 0;
                tm.update();
                computeParams.transform = tm.getMatrix();

                computeJob[layer][currentFrame] = true;
            }
            bool Animator::isComputeFinished(unsigned int currentFrame, unsigned int layer) {
                if (layer >= computeFinished.size())
                    return true;
                return computeFinished[layer][currentFrame].load();
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
                                                   #extension GL_EXT_nonuniform_qualifier : enable
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
                                                   layout (std430, set = 0, binding = 0) buffer verticesSSBO {
                                                       Vertex vertices[];
                                                   } verticesBuffers[];
                                                   layout (std430, set = 1, binding = 0) buffer finalBonesMatricesSSBO {
                                                       mat4 finalBonesMatrices[MAX_BONES];
                                                   } finalBonesMatricesBuffers[];
                                                   layout (push_constant) uniform PushConsts {
                                                         int entityId;
                                                         int instanced;
                                                         int layer;
                                                         int padding2;
                                                         mat4 transform;
                                                   } pushConsts;
                                                   layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
                                                   void main() {
                                                       uint vertexIndex = gl_GlobalInvocationID.x;

                                                       if (pushConsts.entityId == verticesBuffers[pushConsts.layer].vertices[vertexIndex].entityId) {
                                                           vec4 totalPosition = vec4(0.0f);
                                                           for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
                                                           {
                                                              debugPrintfEXT("Bone id : %i", verticesBuffers[pushConsts.layer].vertices[vertexIndex].boneIds[i]);
                                                              if(verticesBuffers[pushConsts.layer].vertices[vertexIndex].boneIds[i] == -1)
                                                                 continue;
                                                              if(verticesBuffers[pushConsts.layer].vertices[vertexIndex].boneIds[i] >=MAX_BONES)
                                                              {
                                                                  totalPosition = vec4(verticesBuffers[pushConsts.layer].vertices[vertexIndex].position.xyz, 1);
                                                                  break;
                                                              }
                                                              totalPosition += verticesBuffers[pushConsts.layer].vertices[vertexIndex].weights[i] * (vec4(verticesBuffers[pushConsts.layer].vertices[vertexIndex].position.xyz, 1) * finalBonesMatricesBuffers[pushConsts.layer].finalBonesMatrices[verticesBuffers[pushConsts.layer].vertices[vertexIndex].boneIds[i]]);

                                                            }
                                                            verticesBuffers[pushConsts.layer].vertices[vertexIndex].position = vec3(totalPosition.xyz);
                                                            if (pushConsts.instanced == 0) {
                                                                //debugPrintfEXT("m1 : %v4f\nm2 : %v4f\nm3 : %v4f\nm4 : %v4f\n", pushConsts.transform[0],pushConsts.transform[1],pushConsts.transform[2],pushConsts.transform[3]);
                                                                verticesBuffers[pushConsts.layer].vertices[vertexIndex].position = (vec4(verticesBuffers[pushConsts.layer].vertices[vertexIndex].position.xyz, 1) * pushConsts.transform).xyz;
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
            void Animator::createCommandBuffers(unsigned int oldSize) {
                for (unsigned int i = oldSize; i < commandBuffers.size(); i++) {
                    VkCommandBufferAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                    allocInfo.commandPool = commandPool;
                    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                    allocInfo.commandBufferCount = (uint32_t) commandBuffers[i].size();
                    if (vkAllocateCommandBuffers(vkDevice.getDevice(), &allocInfo, commandBuffers[i].data()) != VK_SUCCESS) {
                        throw core::Erreur(0, "failed to allocate command buffers!", 1);
                    }
                }
            }
            void Animator::createDescriptorPool() {
                std::array<VkDescriptorPoolSize, 1> poolSizes1{};
                std::array<VkDescriptorPoolSize, 1> poolSizes2{};
                poolSizes1[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                poolSizes1[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)*MAX_RENDER_COMPONENT_LAYERS;
                poolSizes2[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                poolSizes2[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)*MAX_RENDER_COMPONENT_LAYERS;
                VkDescriptorPoolCreateInfo poolInfo{};
                poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes1.size());
                poolInfo.pPoolSizes = poolSizes1.data();
                poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
                if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &computeDescriptorPool[0]) != VK_SUCCESS) {
                    throw std::runtime_error("echec de la creation de la pool de descripteurs!");
                }
                poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes2.size());
                poolInfo.pPoolSizes = poolSizes2.data();
                if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &computeDescriptorPool[1]) != VK_SUCCESS) {
                    throw std::runtime_error("echec de la creation de la pool de descripteurs!");
                }
            }
            void Animator::createDescriptorSetLayout() {
                std::array<VkDescriptorSetLayoutBinding, 1> layoutBindings1{};
                std::array<VkDescriptorSetLayoutBinding, 1> layoutBindings2{};
                layoutBindings1[0].binding = 0;
                layoutBindings1[0].descriptorCount = MAX_RENDER_COMPONENT_LAYERS;
                layoutBindings1[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                layoutBindings1[0].pImmutableSamplers = nullptr;
                layoutBindings1[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

                layoutBindings2[0].binding = 0;
                layoutBindings2[0].descriptorCount = MAX_RENDER_COMPONENT_LAYERS;
                layoutBindings2[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                layoutBindings2[0].pImmutableSamplers = nullptr;
                layoutBindings2[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

                std::vector<VkDescriptorBindingFlags> bindingFlags1(1, 0); // 6 bindings, flags par défaut à 0
                bindingFlags1[0] = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                          VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT; // seulement pour sampler[]

                std::vector<VkDescriptorBindingFlags> bindingFlags2(1, 0); // 6 bindings, flags par défaut à 0
                bindingFlags2[0] = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                          VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT; // seulement pour sampler[]

                VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
                bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
                bindingFlagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags1.size());
                bindingFlagsInfo.pBindingFlags = bindingFlags1.data();


                VkDescriptorSetLayoutCreateInfo layoutInfo{};
                layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                layoutInfo.pNext = &bindingFlagsInfo;
                layoutInfo.bindingCount = layoutBindings1.size();
                layoutInfo.pBindings = layoutBindings1.data();

                if (vkCreateDescriptorSetLayout(vkDevice.getDevice(), &layoutInfo, nullptr, &computeDescriptorSetLayout[0]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create compute descriptor set layout!");
                }

                bindingFlagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags2.size());
                bindingFlagsInfo.pBindingFlags = bindingFlags2.data();

                layoutInfo.bindingCount = layoutBindings2.size();
                layoutInfo.pBindings = layoutBindings2.data();

                if (vkCreateDescriptorSetLayout(vkDevice.getDevice(), &layoutInfo, nullptr, &computeDescriptorSetLayout[1]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create compute descriptor set layout!");
                }
            }
            void Animator::allocateDescriptorSets() {
                std::vector<uint32_t> variableCounts1(MAX_FRAMES_IN_FLIGHT, static_cast<uint32_t>(MAX_RENDER_COMPONENT_LAYERS));
                std::vector<uint32_t> variableCounts2(MAX_FRAMES_IN_FLIGHT, static_cast<uint32_t>(MAX_RENDER_COMPONENT_LAYERS));
                std::vector<VkDescriptorSetLayout> layouts1(MAX_FRAMES_IN_FLIGHT, computeDescriptorSetLayout[0]);

                VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
                variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
                variableCountInfo.descriptorSetCount = static_cast<uint32_t>(variableCounts1.size());;
                variableCountInfo.pDescriptorCounts = variableCounts1.data();

                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.pNext = &variableCountInfo;

                allocInfo.descriptorPool = computeDescriptorPool[0];
                allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
                allocInfo.pSetLayouts = layouts1.data();
                if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, computeDescriptorSets[0].data()) != VK_SUCCESS) {
                    throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
                }

                std::vector<VkDescriptorSetLayout> layouts2(MAX_FRAMES_IN_FLIGHT, computeDescriptorSetLayout[1]);


                variableCountInfo.descriptorSetCount = static_cast<uint32_t>(variableCounts2.size());;
                variableCountInfo.pDescriptorCounts = variableCounts2.data();



                allocInfo.descriptorPool = computeDescriptorPool[1];
                allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
                allocInfo.pSetLayouts = layouts2.data();
                if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, computeDescriptorSets[1].data()) != VK_SUCCESS) {
                    throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
                }
            }
            void Animator::updateDescriptorSets(unsigned int currentFrame) {
                std::array<VkWriteDescriptorSet, 1> descriptorWrites1{};
                std::array<VkWriteDescriptorSet, 1> descriptorWrites2{};

                std::vector<VkDescriptorBufferInfo> verticesStorageBuffersInfoLastFrame;
                for (unsigned int i = 0; i < vertexBuffer.size(); i++) {
                    VkDescriptorBufferInfo verticesStorageBufferInfoLastFrame{};
                    verticesStorageBufferInfoLastFrame.buffer = vertexBuffer[i].value().get().getVertexBuffer(currentFrame);
                    verticesStorageBufferInfoLastFrame.offset = 0;
                    verticesStorageBufferInfoLastFrame.range = vertexBuffer[i].value().get().getVertexCount() * sizeof(graphic::Vertex);
                    verticesStorageBuffersInfoLastFrame.push_back(verticesStorageBufferInfoLastFrame);
                }

                descriptorWrites1[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites1[0].dstSet = computeDescriptorSets[0][currentFrame];
                descriptorWrites1[0].dstBinding = 0;
                descriptorWrites1[0].dstArrayElement = 0;
                descriptorWrites1[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                descriptorWrites1[0].descriptorCount = vertexBuffer.size();
                descriptorWrites1[0].pBufferInfo = verticesStorageBuffersInfoLastFrame.data();

                std::vector<VkDescriptorBufferInfo> finalBonesMatricesStorageBuffersInfoLastFrame{};
                for (unsigned int i = 0; i < finalBonesMatrices.size(); i++) {
                    VkDescriptorBufferInfo finalBonesMatricesStorageBufferInfoLastFrame{};
                    finalBonesMatricesStorageBufferInfoLastFrame.buffer = finalBonesMatrices[i][currentFrame];
                    finalBonesMatricesStorageBufferInfoLastFrame.offset = 0;
                    finalBonesMatricesStorageBufferInfoLastFrame.range = m_FinalBoneMatrices.size() * sizeof(math::Matrix4f);
                    finalBonesMatricesStorageBuffersInfoLastFrame.push_back(finalBonesMatricesStorageBufferInfoLastFrame);
                }

                descriptorWrites2[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites2[0].dstSet = computeDescriptorSets[1][currentFrame];
                descriptorWrites2[0].dstBinding = 0;
                descriptorWrites2[0].dstArrayElement = 0;
                descriptorWrites2[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                descriptorWrites2[0].descriptorCount = finalBonesMatrices.size();
                descriptorWrites2[0].pBufferInfo = finalBonesMatricesStorageBuffersInfoLastFrame.data();

                vkUpdateDescriptorSets(vkDevice.getDevice(), descriptorWrites1.size(), descriptorWrites1.data(), 0, nullptr);
                vkUpdateDescriptorSets(vkDevice.getDevice(), descriptorWrites2.size(), descriptorWrites2.data(), 0, nullptr);

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
                pipelineLayoutInfo.setLayoutCount = computeDescriptorSetLayout.size();
                pipelineLayoutInfo.pSetLayouts = computeDescriptorSetLayout.data();
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
                for (unsigned int i = 0; i < commandBuffers.size(); i++) {
                    vkFreeCommandBuffers(vkDevice.getDevice(), commandPool, commandBuffers[i].size(), commandBuffers[i].data());
                }
                vkDestroyCommandPool(vkDevice.getDevice(), commandPool, nullptr);
                vkDestroyPipelineLayout(vkDevice.getDevice(), computePipelineLayout, nullptr);
                vkDestroyPipeline(vkDevice.getDevice(), computePipeline, nullptr);
                for (unsigned int i = 0; i < computeDescriptorSetLayout.size(); i++) {
                    vkDestroyDescriptorSetLayout(vkDevice.getDevice(), computeDescriptorSetLayout[i], nullptr);
                }
                for (unsigned int i = 0; i < computeDescriptorPool.size(); i++) {
                    vkDestroyDescriptorPool(vkDevice.getDevice(), computeDescriptorPool[i], nullptr);
                }
                for (unsigned int l = 0; l < finalBonesMatrices.size(); l++) {
                    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                        vkDestroyBuffer(vkDevice.getDevice(), finalBonesMatrices[l][i], nullptr);
                        vkFreeMemory(vkDevice.getDevice(), finalBonesMatricesMemory[l][i], nullptr);
                        vkDestroyBuffer(vkDevice.getDevice(), stagingFinalBonesMatrices[l][i], nullptr);
                        vkFreeMemory(vkDevice.getDevice(), stagingFinalBonesMatricesMemory[l][i], nullptr);
                    }
                }
                delete m_CurrentAnimation;
            }
        }
    }
}
