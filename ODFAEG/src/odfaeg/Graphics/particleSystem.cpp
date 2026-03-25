/////////////////////////////////////////////////////////////////////////////////
//
// Thor C++ Library
// Copyright (c) 2011-2014 Jan Haller
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
/////////////////////////////////////////////////////////////////////////////////

#include "../../../include/odfaeg/Graphics/particuleSystem.h"


#include "../../../include/odfaeg/Graphics/renderWindow.h"
#include "../../../include/odfaeg/Graphics/texture.h"
#include "../../../include/odfaeg/Math/transformMatrix.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cassert>


namespace odfaeg
{
    namespace physic {
            namespace
            {

                // Erases emitter/affector at itr from ctr, if its time has expired. itr will point to the next element.
                template <class Container>
                void incrementCheckExpiry(Container& ctr, typename Container::iterator& itr, core::Time dt)
                {
                    // itr->second is the remaining time of the emitter/affector.
                    // Time::zero means infinite time (no removal).
                    if (itr->timeUntilRemoval != core::Time::zero && (itr->timeUntilRemoval -= dt) <= core::Time::zero)
                    itr = ctr.erase(itr);
                    else
                    ++itr;
                }

                graphic::IntRect getFullRect(const graphic::Texture& texture)
                {
                    return graphic::IntRect(0, 0, texture.getSize().x(), texture.getSize().y());
                }

            } // namespace

        // ---------------------------------------------------------------------------------------------------------------------------

        ParticleSystem::ParticleSystem(window::Device& vkDevice, graphic::EntityFactory& factory) : graphic::GameObject(math::Vec3f(0, 0, 0), math::Vec3f(0, 0, 0), math::Vec3f(0, 0, 0), "E_PARTICLES", factory), mParticles()
        , mAffectors()
        , mEmitters()
        , mTexture(nullptr)
        , mTextureRects()
        , mVertices(graphic::Quads)
        , mNeedsVertexUpdate(true)
        , mQuads()
        , mNeedsQuadUpdate(true)
        ,vkDevice(vkDevice)
        ,computeShader(vkDevice) {
            psUpdater = "";
            graphic::Material material;
            material.addTexture(nullptr);
            graphic::Face face (mVertices, material, getTransform());
            addFace(face);
            createCommandPool();
            createDescriptorPool();
            createDescriptorSetLayout();
            allocateDescriptorSets();
            compileComputeShader();
            createComputePipeline();
            mVertices.setEntity(this);
            computeParams.entityId = getId();
            #ifdef VULKAN
            mVertices.setPrimitiveType(graphic::Triangles);
            #endif // VULKAN
        }
        ParticleSystem::ParticleSystem(window::Device& vkDevice, math::Vec3f position, math::Vec3f size, graphic::EntityFactory& factory)
        : graphic::GameObject(position, size, size*0.5f, "E_PARTICLES", factory), mParticles()
        , mAffectors()
        , mEmitters()
        , mTexture(nullptr)
        , mTextureRects()
        , mVertices(graphic::Quads)
        , mNeedsVertexUpdate(true)
        , mQuads()
        , mNeedsQuadUpdate(true)
        ,vkDevice(vkDevice)
        ,computeShader(vkDevice)
        {
            graphic::Material material;
            material.addTexture(nullptr);
            graphic::Face face (mVertices, material, getTransform());
            addFace(face);
            psUpdater = "";
            createCommandPool();
            createDescriptorPool();
            createDescriptorSetLayout();
            allocateDescriptorSets();
            compileComputeShader();
            createComputePipeline();
            mVertices.setEntity(this);
            computeParams.entityId = getId();
            #ifdef VULKAN
            mVertices.setPrimitiveType(graphic::Triangles);
            #endif // VULKAN
        }
        void ParticleSystem::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
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
        void ParticleSystem::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandBuffer cmd) {
            //std::cout<<"opy buffers"<<std::endl;
            if (srcBuffer != nullptr && dstBuffer != nullptr) {
                VkBufferCopy copyRegion{};
                copyRegion.size = size;
                vkCmdCopyBuffer(cmd, srcBuffer, dstBuffer, 1, &copyRegion);
            }
        }
        uint32_t ParticleSystem::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
            VkPhysicalDeviceMemoryProperties memProperties;
            vkGetPhysicalDeviceMemoryProperties(vkDevice.getPhysicalDevice(), &memProperties);
            for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
                if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                    return i;
                }
            }
            throw std::runtime_error("aucun type de memoire ne satisfait le buffer!");
        }
        void ParticleSystem::computeParticles(std::mutex* mtx, std::condition_variable* cv2, graphic::VertexBuffer& frameVertexBuffer, unsigned int currentFrame, graphic::TransformMatrix tm, bool instanced, std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> computeSemaphore, std::array<VkFence, MAX_FRAMES_IN_FLIGHT> computeFence, unsigned int  layer) {

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
                quads.resize(layer + 1);
                particles.resize(layer + 1);
                quadsMemory.resize(layer + 1);
                particlesMemory.resize(layer + 1);
                stagingQuads.resize(layer + 1);
                stagingParticles.resize(layer + 1);
                stagingQuadsMemory.resize(layer + 1);
                stagingParticlesMemory.resize(layer + 1);
                maxParticlesSize.resize(layer + 1);
                maxQuadsSize.resize(layer + 1);
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
            this->currentFrame[layer] = currentFrame;
            this->computeSemaphores[layer] = computeSemaphore;
            this->computeFences[layer] = computeFence;
            computeParams.instanced = (instanced) ? 1 : 0;
            tm.update();
            computeParams.transform = tm.getMatrix();

            computeJob[layer][currentFrame] = true;
        }
        bool ParticleSystem::isComputeFinished(unsigned int currentFrame, unsigned int layer) {
            if (layer >= computeFinished.size())
                return true;
            return computeFinished[layer][currentFrame].load();
        }
        void ParticleSystem::compileComputeShader() {
            const std::string computeShaderCode = R"(#version 460
                                                     const int MAX_BONE_INFLUENCE = 4;
                                                     #extension GL_EXT_debug_printf : enable
                                                     #extension GL_EXT_nonuniform_qualifier : enable
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
                                                         int padding;
                                                     };
                                                     struct Quad {
                                                         Vertex quad[6];
                                                     };
                                                     layout (std430, set = 0, binding = 0) buffer particleSSBO {
                                                         Particle particles[];
                                                     } particlesBuffers[];
                                                     layout (std430, set = 1, binding = 0) buffer verticesSSBO {
                                                         Vertex vertices[];
                                                     } verticesBuffers[];
                                                     layout (std430, set = 2, binding = 0) buffer quadSSBO {
                                                         Quad quads[];
                                                     } quadsBuffers[];
                                                     layout (push_constant) uniform PushConsts {
                                                         float dt;
                                                         int entityId;
                                                         int instanced;
                                                         int layer;
                                                         mat4 transform;
                                                     } pushConsts;
                                                     layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
                                                     void main() {
                                                         uint particleIndex = gl_GlobalInvocationID.x;
                                                         uint vertexIndex = gl_GlobalInvocationID.y;

                                                         if (pushConsts.entityId == verticesBuffers[pushConsts.layer].vertices[vertexIndex].entityId && particlesBuffers[pushConsts.layer].particles[particleIndex].id == verticesBuffers[pushConsts.layer].vertices[vertexIndex].particleId) {
                                                            float angle = radians(particlesBuffers[pushConsts.layer].particles[particleIndex].rotation);
                                                            vec3 scale = particlesBuffers[pushConsts.layer].particles[particleIndex].scale;
                                                            vec3 translation = particlesBuffers[pushConsts.layer].particles[particleIndex].position;
                                                            mat4 transformMatrix = mat4 (cos(angle)*scale.x, -sin(angle)*scale.y, 0,       translation.x,
                                                                                         sin(angle)*scale.x, cos(angle)*scale.y , 0,       translation.y,
                                                                                         0                 , 0                  , scale.z, translation.z,
                                                                                         0                 , 0                  , 0      , 1);
                                                            Quad quad = quadsBuffers[pushConsts.layer].quads[particlesBuffers[pushConsts.layer].particles[particleIndex].textureIndex];
                                                            vec4 transformed = transpose(transformMatrix) * vec4(quad.quad[vertexIndex%6].position, 1);

                                                            verticesBuffers[pushConsts.layer].vertices[vertexIndex].position = verticesBuffers[pushConsts.layer].vertices[vertexIndex].position + vec3(transformed.xyz);
                                                            if (pushConsts.instanced == 0)
                                                                verticesBuffers[pushConsts.layer].vertices[vertexIndex].position = (vec4(verticesBuffers[pushConsts.layer].vertices[vertexIndex].position.xyz, 1) * pushConsts.transform).xyz;
                                                            //debugPrintfEXT("position : %v3f", vertices[vertexIndex].position);
                                                            verticesBuffers[pushConsts.layer].vertices[vertexIndex].texCoords = quad.quad[vertexIndex%6].texCoords;
                                                            verticesBuffers[pushConsts.layer].vertices[vertexIndex].color = particlesBuffers[pushConsts.layer].particles[particleIndex].color;
                                                         }
                                                     }
                                                     )";
            if (!computeShader.loadFromMemory(computeShaderCode)) {
                throw core::Erreur(60, "Failed to compile compute particle shader");
            }
        }
        void ParticleSystem::createCommandPool() {
            window::Device::QueueFamilyIndices queueFamilyIndices = vkDevice.findQueueFamilies(vkDevice.getPhysicalDevice(), VK_NULL_HANDLE);

            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.queueFamilyIndex = queueFamilyIndices.computeFamily.value();
            poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optionel
            if (vkCreateCommandPool(vkDevice.getDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
                throw core::Erreur(0, "échec de la création d'une command pool!", 1);
            }
        }
        void ParticleSystem::createCommandBuffers(unsigned int oldSize) {

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
        void ParticleSystem::createDescriptorPool() {
            std::array<VkDescriptorPoolSize, 1> poolSizes1{};
            std::array<VkDescriptorPoolSize, 1> poolSizes2{};
            std::array<VkDescriptorPoolSize, 1> poolSizes3{};
            poolSizes1[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            poolSizes1[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * MAX_RENDER_COMPONENT_LAYERS;
            poolSizes2[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            poolSizes2[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * MAX_RENDER_COMPONENT_LAYERS;
            poolSizes3[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            poolSizes3[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * MAX_RENDER_COMPONENT_LAYERS;
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
            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes3.size());
            poolInfo.pPoolSizes = poolSizes3.data();
            if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &computeDescriptorPool[2]) != VK_SUCCESS) {
                throw std::runtime_error("echec de la creation de la pool de descripteurs!");
            }
        }
        void ParticleSystem::createDescriptorSetLayout() {
            std::array<VkDescriptorSetLayoutBinding, 1> layoutBindings1{};
            std::array<VkDescriptorSetLayoutBinding, 1> layoutBindings2{};
            std::array<VkDescriptorSetLayoutBinding, 1> layoutBindings3{};
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

            layoutBindings3[0].binding = 0;
            layoutBindings3[0].descriptorCount = MAX_RENDER_COMPONENT_LAYERS;
            layoutBindings3[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            layoutBindings3[0].pImmutableSamplers = nullptr;
            layoutBindings3[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

            std::vector<VkDescriptorBindingFlags> bindingFlags1(1, 0); // 6 bindings, flags par défaut à 0
            bindingFlags1[0] = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                      VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT; // seulement pour sampler[]

            std::vector<VkDescriptorBindingFlags> bindingFlags2(1, 0); // 6 bindings, flags par défaut à 0
            bindingFlags2[0] = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                      VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT; // seulement pour sampler[]

            std::vector<VkDescriptorBindingFlags> bindingFlags3(1, 0); // 6 bindings, flags par défaut à 0
            bindingFlags3[0] = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
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

            bindingFlagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags3.size());
            bindingFlagsInfo.pBindingFlags = bindingFlags3.data();


            layoutInfo.bindingCount = layoutBindings3.size();
            layoutInfo.pBindings = layoutBindings3.data();

            if (vkCreateDescriptorSetLayout(vkDevice.getDevice(), &layoutInfo, nullptr, &computeDescriptorSetLayout[2]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create compute descriptor set layout!");
            }
        }
        void ParticleSystem::allocateDescriptorSets() {
            std::vector<uint32_t> variableCounts1(MAX_FRAMES_IN_FLIGHT, static_cast<uint32_t>(MAX_RENDER_COMPONENT_LAYERS));
            std::vector<uint32_t> variableCounts2(MAX_FRAMES_IN_FLIGHT, static_cast<uint32_t>(MAX_RENDER_COMPONENT_LAYERS));
            std::vector<uint32_t> variableCounts3(MAX_FRAMES_IN_FLIGHT, static_cast<uint32_t>(MAX_RENDER_COMPONENT_LAYERS));

            VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
            variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
            variableCountInfo.descriptorSetCount = static_cast<uint32_t>(variableCounts1.size());;
            variableCountInfo.pDescriptorCounts = variableCounts1.data();

            std::vector<VkDescriptorSetLayout> layouts1(MAX_FRAMES_IN_FLIGHT, computeDescriptorSetLayout[0]);
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.pNext = &variableCountInfo;

            allocInfo.descriptorPool = computeDescriptorPool[0];
            allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
            allocInfo.pSetLayouts = layouts1.data();
            if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, computeDescriptorSets[0].data()) != VK_SUCCESS) {
                throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
            }

            variableCountInfo.descriptorSetCount = static_cast<uint32_t>(variableCounts2.size());;
            variableCountInfo.pDescriptorCounts = variableCounts2.data();
            std::vector<VkDescriptorSetLayout> layouts2(MAX_FRAMES_IN_FLIGHT, computeDescriptorSetLayout[1]);


            allocInfo.descriptorPool = computeDescriptorPool[1];
            allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
            allocInfo.pSetLayouts = layouts2.data();
            if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, computeDescriptorSets[1].data()) != VK_SUCCESS) {
                throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
            }

            variableCountInfo.descriptorSetCount = static_cast<uint32_t>(variableCounts3.size());;
            variableCountInfo.pDescriptorCounts = variableCounts3.data();

            std::vector<VkDescriptorSetLayout> layouts3(MAX_FRAMES_IN_FLIGHT, computeDescriptorSetLayout[2]);


            allocInfo.descriptorPool = computeDescriptorPool[2];
            allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
            allocInfo.pSetLayouts = layouts2.data();
            if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, computeDescriptorSets[2].data()) != VK_SUCCESS) {
                throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
            }
        }
        void ParticleSystem::updateDescriptorSets(unsigned int currentFrame) {
            std::array<VkWriteDescriptorSet, 1> descriptorWrites1{};
            std::array<VkWriteDescriptorSet, 1> descriptorWrites2{};
            std::array<VkWriteDescriptorSet, 1> descriptorWrites3{};

            std::vector<VkDescriptorBufferInfo> particleStorageBuffersInfoLastFrame;
            for (unsigned int i = 0; i < particles.size(); i++) {
                VkDescriptorBufferInfo particleStorageBufferInfoLastFrame{};
                particleStorageBufferInfoLastFrame.buffer = particles[i][currentFrame];
                particleStorageBufferInfoLastFrame.offset = 0;
                particleStorageBufferInfoLastFrame.range = mParticles.size() * sizeof(Particle);
                particleStorageBuffersInfoLastFrame.push_back(particleStorageBufferInfoLastFrame);
            }

            descriptorWrites1[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites1[0].dstSet = computeDescriptorSets[0][currentFrame];
            descriptorWrites1[0].dstBinding = 0;
            descriptorWrites1[0].dstArrayElement = 0;
            descriptorWrites1[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            descriptorWrites1[0].descriptorCount = particles.size();
            descriptorWrites1[0].pBufferInfo = particleStorageBuffersInfoLastFrame.data();

            std::vector<VkDescriptorBufferInfo> verticesStorageBuffersInfoLastFrame;
            for (unsigned int i = 0; i < vertexBuffer.size(); i++) {
                //std::cout<<"i : "<<i<<",buffer : "<<vertexBuffer[i].value().get().getVertexBuffer(currentFrame)<<","<<vertexBuffer[i].value().get().getVertexCount()<<std::endl;
                VkDescriptorBufferInfo verticesStorageBufferInfoLastFrame{};
                verticesStorageBufferInfoLastFrame.buffer = vertexBuffer[i].value().get().getVertexBuffer(currentFrame);
                verticesStorageBufferInfoLastFrame.offset = 0;
                verticesStorageBufferInfoLastFrame.range = vertexBuffer[i].value().get().getVertexCount() * sizeof(graphic::Vertex);
                verticesStorageBuffersInfoLastFrame.push_back(verticesStorageBufferInfoLastFrame);
            }


            descriptorWrites2[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites2[0].dstSet = computeDescriptorSets[1][currentFrame];
            descriptorWrites2[0].dstBinding = 0;
            descriptorWrites2[0].dstArrayElement = 0;
            descriptorWrites2[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            descriptorWrites2[0].descriptorCount = vertexBuffer.size();
            descriptorWrites2[0].pBufferInfo = verticesStorageBuffersInfoLastFrame.data();

            std::vector<VkDescriptorBufferInfo> quadsStorageBuffersInfoCurrentFrame;
            for (unsigned int i = 0; i < quads.size(); i++) {
                VkDescriptorBufferInfo quadsStorageBufferInfoCurrentFrame{};
                quadsStorageBufferInfoCurrentFrame.buffer = quads[i][currentFrame];
                quadsStorageBufferInfoCurrentFrame.offset = 0;
                quadsStorageBufferInfoCurrentFrame.range = mQuads.size() * sizeof(Quad);
                quadsStorageBuffersInfoCurrentFrame.push_back(quadsStorageBufferInfoCurrentFrame);
            }

            descriptorWrites3[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites3[0].dstSet = computeDescriptorSets[2][currentFrame];
            descriptorWrites3[0].dstBinding = 0;
            descriptorWrites3[0].dstArrayElement = 0;
            descriptorWrites3[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            descriptorWrites3[0].descriptorCount = quads.size();
            descriptorWrites3[0].pBufferInfo = quadsStorageBuffersInfoCurrentFrame.data();

            vkUpdateDescriptorSets(vkDevice.getDevice(), descriptorWrites1.size(), descriptorWrites1.data(), 0, nullptr);
            vkUpdateDescriptorSets(vkDevice.getDevice(), descriptorWrites2.size(), descriptorWrites2.data(), 0, nullptr);
            vkUpdateDescriptorSets(vkDevice.getDevice(), descriptorWrites3.size(), descriptorWrites3.data(), 0, nullptr);

        }
        void ParticleSystem::createComputePipeline() {
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
        graphic::Entity* ParticleSystem::clone() {
            ParticleSystem* ps = factory.make_entity<ParticleSystem>(vkDevice, getPosition(), getSize(), factory);
            GameObject::copy(ps);
            ps->mAffectors = mAffectors;
            ps->mEmitters = mEmitters;
            ps->mTexture = mTexture;
            ps->mTextureRects = mTextureRects;
            ps->mVertices = mVertices;
            return ps;
        }
        void ParticleSystem::setTexture(const graphic::Texture& texture)
        {
            mTexture = &texture;
            getFaces()[0].getMaterial().clearTextures();
            getFaces()[0].getMaterial().addTexture(mTexture);
            mNeedsQuadUpdate = true;
        }

        unsigned int ParticleSystem::addTextureRect(const graphic::IntRect& textureRect)
        {
            mTextureRects.push_back(textureRect);
            mNeedsQuadUpdate = true;

            return mTextureRects.size() - 1;
        }

        void ParticleSystem::swap(ParticleSystem& other)
        {
            // Use ADL
            using std::swap;

            swap(mParticles,	other.mParticles);
            swap(mAffectors,	other.mAffectors);
            swap(mEmitters,	other.mEmitters);
            swap(mTexture,	other.mTexture);
            swap(mTextureRects,	other.mTextureRects);
            swap(mVertices,	other.mVertices);
            swap(mNeedsVertexUpdate,	other.mNeedsVertexUpdate);
            swap(mQuads,	other.mQuads);
            swap(mNeedsQuadUpdate,	other.mNeedsQuadUpdate);
        }

        void ParticleSystem::addAffector(std::function<void(Particle&, core::Time)> affector)
        {
            addAffector(std::move(affector), core::Time::zero);
        }

        void ParticleSystem::addAffector(std::function<void(Particle&, core::Time)> affector, core::Time timeUntilRemoval)
        {
            mAffectors.push_back( Affector(std::move(affector), timeUntilRemoval) );
        }

        void ParticleSystem::clearAffectors()
        {
            mAffectors.clear();
        }

        void ParticleSystem::addEmitter(std::function<void(EmissionInterface&, core::Time)> emitter)
        {
            addEmitter(emitter, core::Time::zero);
        }

        void ParticleSystem::addEmitter(std::function<void(EmissionInterface&, core::Time)> emitter, core::Time timeUntilRemoval)
        {
            mEmitters.push_back( Emitter(std::move(emitter), timeUntilRemoval) );
        }

        void ParticleSystem::clearEmitters()
        {
            mEmitters.clear();
        }

        void ParticleSystem::update(core::Time dt)
        {

            mNeedsVertexUpdate = true;
            // Emit new particles and remove expiring emitters
            for (EmitterContainer::iterator itr = mEmitters.begin(); itr != mEmitters.end(); )
            {
                itr->function(*this, dt);
                incrementCheckExpiry(mEmitters, itr, dt);
            }

            // Affect existing particles
            ParticleContainer::iterator writer = mParticles.begin();
            for (ParticleContainer::iterator reader = mParticles.begin(); reader != mParticles.end(); ++reader)
            {
                // Apply movement and decrease lifetime
                updateParticle(*reader, dt);

                // If current particle is not dead
                if (reader->passedLifetime < reader->totalLifetime)
                {
                    // Only apply affectors to living particles
                    AffectorContainer::iterator it;
                    for (it = mAffectors.begin(); it < mAffectors.end(); it++) {
                        it->function(*reader, dt);
                    }
                    // Go ahead
                    *writer++ = *reader;
                }
            }

            // Remove particles dying this frame
            mParticles.erase(writer, mParticles.end());

            // Remove affectors expiring this frame
            for (AffectorContainer::iterator itr = mAffectors.begin(); itr != mAffectors.end(); )
            {
                incrementCheckExpiry(mAffectors, itr, dt);
            }
            update();

            for (unsigned int layer = 0; layer < computeJob.size(); layer++) {

                if (computeJob[layer][currentFrame[layer]].load()) {

                    std::unique_lock<std::mutex> lock(*mtx[layer]);

                    computeJob[layer][currentFrame[layer]] = false;
                    vkResetCommandBuffer(commandBuffers[layer][currentFrame[layer]], 0);
                    VkCommandBufferBeginInfo beginInfo{};
                    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                    /*if (m_name == "depthBuffer")
                        //////std::cout<<"begin cmd : "<<commandBuffers.size()<<std::endl;*/
                    if (vkBeginCommandBuffer(commandBuffers[layer][currentFrame[layer]], &beginInfo) != VK_SUCCESS) {

                        throw core::Erreur(0, "failed to begin recording command buffer!", 1);
                    }
                    if (vertexBuffer[layer].has_value() && vertexBuffer[layer].value().get().getVertexCount() > 0 && mParticles.size() > 0) {
                        for (unsigned int l = 0; l <= layer; l++) {
                            VkDeviceSize bufferSize = sizeof(Particle) * mParticles.size();
                            if (bufferSize > maxParticlesSize[l][currentFrame[layer]]) {
                                if (stagingParticles[l][currentFrame[layer]] != nullptr) {
                                    vkDestroyBuffer(vkDevice.getDevice(), stagingParticles[l][currentFrame[layer]], nullptr);
                                    vkFreeMemory(vkDevice.getDevice(), stagingParticlesMemory[l][currentFrame[layer]], nullptr);
                                }

                                createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,stagingParticles[l][currentFrame[layer]], stagingParticlesMemory[l][currentFrame[layer]]);

                                if (particles[l][currentFrame[layer]] != nullptr) {
                                    vkDestroyBuffer(vkDevice.getDevice(),particles[l][currentFrame[layer]], nullptr);
                                    vkFreeMemory(vkDevice.getDevice(), particlesMemory[l][currentFrame[layer]], nullptr);
                                }

                                createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, particles[l][currentFrame[layer]], particlesMemory[l][currentFrame[layer]]);


                                maxParticlesSize[l][currentFrame[layer]] = bufferSize;
                                //needToUpdateDSs[p]  = true;
                            }

                            bufferSize = sizeof(Quad) * mQuads.size();

                            if (bufferSize > maxQuadsSize[l][currentFrame[layer]]) {
                                if (stagingQuads[l][currentFrame[layer]] != nullptr) {
                                    vkDestroyBuffer(vkDevice.getDevice(), stagingQuads[l][currentFrame[layer]], nullptr);
                                    vkFreeMemory(vkDevice.getDevice(), stagingQuadsMemory[l][currentFrame[layer]], nullptr);
                                }

                                createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,stagingQuads[l][currentFrame[layer]], stagingQuadsMemory[l][currentFrame[layer]]);

                                if (quads[l][currentFrame[layer]] != nullptr) {
                                    vkDestroyBuffer(vkDevice.getDevice(),quads[l][currentFrame[layer]], nullptr);
                                    vkFreeMemory(vkDevice.getDevice(), quadsMemory[l][currentFrame[layer]], nullptr);
                                }

                                createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, quads[l][currentFrame[layer]], quadsMemory[l][currentFrame[layer]]);


                                maxQuadsSize[l][currentFrame[layer]] = bufferSize;
                                //needToUpdateDSs[p]  = true;
                            }

                        }
                        VkDeviceSize bufferSize = sizeof(Particle) * mParticles.size();
                        void* data;
                        vkMapMemory(vkDevice.getDevice(), stagingParticlesMemory[layer][currentFrame[layer]], 0, bufferSize, 0, &data);
                        memcpy(data, mParticles.data(), (size_t)bufferSize);
                        vkUnmapMemory(vkDevice.getDevice(), stagingParticlesMemory[layer][currentFrame[layer]]);
                        copyBuffer(stagingParticles[layer][currentFrame[layer]], particles[layer][currentFrame[layer]], bufferSize, commandBuffers[layer][currentFrame[layer]]);
                        bufferSize = sizeof(Quad) * mQuads.size();

                        vkMapMemory(vkDevice.getDevice(), stagingQuadsMemory[layer][currentFrame[layer]], 0, bufferSize, 0, &data);
                        memcpy(data, mQuads.data(), (size_t)bufferSize);
                        vkUnmapMemory(vkDevice.getDevice(), stagingQuadsMemory[layer][currentFrame[layer]]);
                        copyBuffer(stagingQuads[layer][currentFrame[layer]], quads[layer][currentFrame[layer]], bufferSize, commandBuffers[layer][currentFrame[layer]]);

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
                        computeParams.deltaTime = dt.asSeconds();
                        computeParams.layer = layer;
                        vkCmdPushConstants(commandBuffers[layer][currentFrame[layer]], computePipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ComputePC), &computeParams);
                        vkCmdDispatch(commandBuffers[layer][currentFrame[layer]], mParticles.size(), vertexBuffer[layer].value().get().getVertexCount(), 1);
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
                    if (vkQueueSubmit(vkDevice.getQueue(indices.computeFamily.value(), 0), 1, &submitInfo, computeFences[layer][currentFrame[layer]]) != VK_SUCCESS) {
                        throw core::Erreur(0, "échec de l'envoi d'un command buffer!", 1);
                    }
                    //std::cout<<"compute finished"<<std::endl;
                    computeFinished[layer][currentFrame[layer]] = true;
                    cv2[layer]->notify_one();
                }
            }

            // Invalidate stored vertices
            /*mNeedsVertexUpdate = true;
            // Emit new particles and remove expiring emitters
            for (EmitterContainer::iterator itr = mEmitters.begin(); itr != mEmitters.end(); )
            {
                itr->function(*this, dt);
                incrementCheckExpiry(mEmitters, itr, dt);
            }

            // Affect existing particles
            ParticleContainer::iterator writer = mParticles.begin();
            for (ParticleContainer::iterator reader = mParticles.begin(); reader != mParticles.end(); ++reader)
            {
                // Apply movement and decrease lifetime
                updateParticle(*reader, dt);

                // If current particle is not dead
                if (reader->passedLifetime < reader->totalLifetime)
                {
                    // Only apply affectors to living particles
                    AffectorContainer::iterator it;
                    for (it = mAffectors.begin(); it < mAffectors.end(); it++) {
                        it->function(*reader, dt);
                    }
                    // Go ahead
                    *writer++ = *reader;
                }
            }

            // Remove particles dying this frame
            mParticles.erase(writer, mParticles.end());

            // Remove affectors expiring this frame
            for (AffectorContainer::iterator itr = mAffectors.begin(); itr != mAffectors.end(); )
            {
                incrementCheckExpiry(mAffectors, itr, dt);
            }*/
        }

        void ParticleSystem::clearParticles()
        {
            mParticles.clear();
        }

        void ParticleSystem::draw(graphic::RenderTarget& target, graphic::RenderStates states)
        {
            // Check cached rectangles
            if (mNeedsQuadUpdate)
            {
                computeQuads();
                mNeedsQuadUpdate = false;
            }

            // Check cached vertices
            if (mNeedsVertexUpdate)
            {
                computeVertices();
                mNeedsVertexUpdate = false;
            }
            // Draw the vertex array with our texture
            states.texture = mTexture;
            target.draw(mVertices, states);
        }

        void ParticleSystem::emitParticle(const Particle& particle)
        {
            mParticles.push_back(particle);
        }

        void ParticleSystem::updateParticle(Particle& particle, core::Time dt)
        {
            particle.passedLifetime += dt;
            particle.position += particle.velocity * dt.asSeconds();
            particle.rotation += particle.rotationSpeed * dt.asSeconds();
        }
        void ParticleSystem::update () {
            // Check cached rectangles
            if (mNeedsQuadUpdate)
            {
                computeQuads();
                mNeedsQuadUpdate = false;
            }

            // Check cached vertices
            if (mNeedsVertexUpdate)
            {
                computeVertices();
                mNeedsVertexUpdate = false;
            }
        }
        void ParticleSystem::computeVertices()
        {

            // Clear vertex array (keeps memory allocated)
            mVertices.clear();


            ParticleContainer::const_iterator it;
            graphic::TransformMatrix tm;
            unsigned int particleId = 0;
            for(it = mParticles.begin(); it < mParticles.end(); it++)
            {
                it->id = particleId;
                for (unsigned int i = 0; i < 6; i++)
                {
                    graphic::Vertex vertex;
                    vertex.particleId = particleId;
                    mVertices.append(vertex);
                }
                particleId++;
            }
            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
            this->getFace(0)->setVertexArray(mVertices);
            /*mVertices.clear();
            // Fill vertex array
            ParticleContainer::const_iterator it;
            graphic::TransformMatrix tm;
            for(it = mParticles.begin(); it < mParticles.end(); it++)
            {
                tm.setRotation(math::Vec3f(0, 0, 1), it->rotation);
                tm.setScale(math::Vec3f(it->scale.x(), it->scale.y(), it->scale.z()));
                tm.setTranslation(math::Vec3f(it->position.x(), it->position.y(), it->position.z()));
                // Ensure valid index -- if this fails, you have not called addTextureRect() enough times, or p.textureIndex is simply wrong
                assert(it->textureIndex == 0 || it->textureIndex < mTextureRects.size());
                #ifdef VULKAN
                const std::array<graphic::Vertex, 6>& quad = mQuads[it->textureIndex];
                for (unsigned int i = 0; i < 6; i++)
                {
                    graphic::Vertex vertex;
                    math::Vec3f pos = tm.transform(quad[i].position);
                    vertex.position[0] = pos.x();
                    vertex.position[1] = pos.y();
                    vertex.position[2] = pos.z();
                    vertex.texCoords = quad[i].texCoords;
                    vertex.color = it->color;
                    mVertices.append(vertex);
                }
                #else
                const std::array<graphic::Vertex, 4>& quad = mQuads[it->textureIndex];
                for (unsigned int i = 0; i < 4; i++)
                {
                    graphic::Vertex vertex;
                    math::Vec3f pos = tm.transform(quad[i].position);
                    vertex.position[0] = pos.x();
                    vertex.position[1] = pos.y();
                    vertex.position[2] = pos.z();
                    vertex.texCoords = quad[i].texCoords;
                    vertex.color = it->color;
                    mVertices.append(vertex);
                }
                #endif // VULKAN
                std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                const_cast<ParticleSystem*>(this)->getFace(0)->setVertexArray(mVertices);
                tm.reset3D();
            }*/

        }

        void ParticleSystem::computeQuads() const
        {
            // Ensure setTexture() has been called
            assert(mTexture);
            // No texture rects: Use full texture, cache single rectangle
            if (mTextureRects.empty())
            {
                mQuads.resize(1);
                computeQuad(mQuads[0], getFullRect(*mTexture));
            }

            // Specified texture rects: Cache every one
            else
            {

                mQuads.resize(mTextureRects.size());
                for (std::size_t i = 0; i < mTextureRects.size(); ++i)
                    computeQuad(mQuads[i], mTextureRects[i]);
            }
        }

        void ParticleSystem::computeQuad(Quad& quad, const graphic::IntRect& textureRect) const
        {
            graphic::FloatRect rect(textureRect);
            #ifdef VULKAN
            quad[0].texCoords = math::Vec2f(rect.left, rect.top);
            quad[1].texCoords = math::Vec2f(rect.left + rect.width, rect.top);
            quad[2].texCoords = math::Vec2f(rect.left + rect.width, rect.top + rect.height);
            quad[3].texCoords = math::Vec2f(rect.left, rect.top);
            quad[4].texCoords = math::Vec2f(rect.left + rect.width, rect.top + rect.height);
            quad[5].texCoords = math::Vec2f(rect.left, rect.top + rect.height);

            quad[0].position = math::Vec3f(-rect.width, -rect.height, rect.height) / 2.f;
            quad[1].position = math::Vec3f( rect.width, -rect.height, rect.height) / 2.f;
            quad[2].position = math::Vec3f( rect.width, rect.height, -rect.height) / 2.f;
            quad[3].position = math::Vec3f(-rect.width, -rect.height, rect.height) / 2.f;
            quad[4].position = math::Vec3f( rect.width, rect.height, -rect.height) / 2.f;
            quad[5].position = math::Vec3f(-rect.width, rect.height, -rect.height) / 2.f;
            #else
            quad[0].texCoords = math::Vec2f(rect.left, rect.top);
            quad[1].texCoords = math::Vec2f(rect.left + rect.width, rect.top);
            quad[2].texCoords = math::Vec2f(rect.left + rect.width, rect.top + rect.height);
            quad[3].texCoords = math::Vec2f(rect.left, rect.top + rect.height);

            quad[0].position = math::Vec3f(-rect.width, -rect.height, rect.height) / 2.f;
            quad[1].position = math::Vec3f( rect.width, -rect.height, rect.height) / 2.f;
            quad[2].position = math::Vec3f( rect.width, rect.height, -rect.height) / 2.f;
            quad[3].position = math::Vec3f(-rect.width, rect.height, -rect.height) / 2.f;
            #endif // VULKAN
        }
        void ParticleSystem::removeEmitter(std::function<void(EmissionInterface&, core::Time)>& em) {
            std::vector<Emitter>::iterator it;
            for (it = mEmitters.begin(); it != mEmitters.end();) {
                if (&em == &it->function)
                    it = mEmitters.erase(it);
                else
                    it++;
            }
        }
        void ParticleSystem::setPsUpdater(std::string name) {
            psUpdater = name;
        }
        std::string ParticleSystem::getPsUpdater() {
            return psUpdater;
        }

        ParticleSystem::~ParticleSystem() {
            for (unsigned int l = 0; l < commandBuffers.size(); l++) {
                vkFreeCommandBuffers(vkDevice.getDevice(), commandPool, commandBuffers[l].size(), commandBuffers[l].data());
            }
            vkDestroyCommandPool(vkDevice.getDevice(), commandPool, nullptr);
            vkDestroyPipelineLayout(vkDevice.getDevice(), computePipelineLayout, nullptr);
            vkDestroyPipeline(vkDevice.getDevice(), computePipeline, nullptr);

            for (unsigned int l = 0; l < computeDescriptorSetLayout.size(); l++)
                vkDestroyDescriptorSetLayout(vkDevice.getDevice(), computeDescriptorSetLayout[l], nullptr);
            for (unsigned int l = 0; l < computeDescriptorPool.size(); l++)
                vkDestroyDescriptorPool(vkDevice.getDevice(), computeDescriptorPool[l], nullptr);
            for (unsigned int l = 0; l < particles.size(); l++) {
                for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    vkDestroyBuffer(vkDevice.getDevice(), particles[l][i], nullptr);
                    vkFreeMemory(vkDevice.getDevice(), particlesMemory[l][i], nullptr);
                    vkDestroyBuffer(vkDevice.getDevice(), stagingParticles[l][i], nullptr);
                    vkFreeMemory(vkDevice.getDevice(), stagingParticlesMemory[l][i], nullptr);
                    vkDestroyBuffer(vkDevice.getDevice(),quads[l][i], nullptr);
                    vkFreeMemory(vkDevice.getDevice(), quadsMemory[l][i], nullptr);
                    vkDestroyBuffer(vkDevice.getDevice(), stagingQuads[l][i], nullptr);
                    vkFreeMemory(vkDevice.getDevice(), stagingQuadsMemory[l][i], nullptr);
                }
            }

        }
    }

} // namespace thor
