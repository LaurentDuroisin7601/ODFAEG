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
            createCommandBuffers();
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
            createCommandBuffers();
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
        void ParticleSystem::computeParticles(std::mutex* mtx, std::condition_variable* cv2, graphic::VertexBuffer& frameVertexBuffer, unsigned int currentFrame, VkSemaphore computeSemaphore, VkFence computeFence) {

            computeFinished[currentFrame] = false;
            this->mtx = mtx;
            this->cv2 = cv2;
            this->vertexBuffer = std::ref(frameVertexBuffer);
            this->currentFrame = currentFrame;
            this->computeSemaphores = computeSemaphore;
            this->computeFences = computeFence;
            computeJob[currentFrame] = true;
        }
        bool ParticleSystem::isComputeFinished(unsigned int currentFrame) {
            return computeFinished[currentFrame].load();
        }
        void ParticleSystem::compileComputeShader() {
            const std::string computeShaderCode = R"(#version 460
                                                     const int MAX_BONE_INFLUENCE = 4;
                                                     #extension GL_EXT_debug_printf : enable
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
                                                     layout (std430, binding = 0) buffer particleSSBO {
                                                         Particle particles[];
                                                     };
                                                     layout (std430, binding = 1) buffer verticesSSBO {
                                                         Vertex vertices[];
                                                     };
                                                     layout (std430, binding = 2) buffer quadSSBO {
                                                         Quad quads[];
                                                     };
                                                     layout (push_constant) uniform PushConsts {
                                                         float dt;
                                                         int entityId;
                                                     } pushConsts;
                                                     layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
                                                     void main() {
                                                         uint particleIndex = gl_GlobalInvocationID.x;
                                                         uint vertexIndex = gl_GlobalInvocationID.y;
                                                         /*if (particleIndex == 0) {
                                                            debugPrintfEXT("indexes : %i, %i, %i, %i", pushConsts.entityId, vertices[vertexIndex].entityId, particles[particleIndex].id, vertices[vertexIndex].particleId);
                                                         }*/
                                                         if (pushConsts.entityId == vertices[vertexIndex].entityId && particles[particleIndex].id == vertices[vertexIndex].particleId) {
                                                            float angle = radians(particles[particleIndex].rotation);
                                                            vec3 scale = particles[particleIndex].scale;
                                                            vec3 translation = particles[particleIndex].position;
                                                            mat4 transformMatrix = mat4 (cos(angle)*scale.x, -sin(angle)*scale.y, 0,       translation.x,
                                                                                         sin(angle)*scale.x, cos(angle)*scale.y , 0,       translation.y,
                                                                                         0                 , 0                  , scale.z, translation.z,
                                                                                         0                 , 0                  , 0      , 1);
                                                            Quad quad = quads[particles[particleIndex].textureIndex];
                                                            vec4 transformed = transpose(transformMatrix) * vec4(quad.quad[vertexIndex%6].position, 1);
                                                            vertices[vertexIndex].position = vertices[vertexIndex].position + vec3(transformed.xyz);
                                                            vertices[vertexIndex].texCoords = quad.quad[vertexIndex%6].texCoords;
                                                            vertices[vertexIndex].color = particles[particleIndex].color;
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
        void ParticleSystem::createCommandBuffers() {

            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = commandPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();
            if (vkAllocateCommandBuffers(vkDevice.getDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to allocate command buffers!", 1);
            }
        }
        void ParticleSystem::createDescriptorPool() {
            std::array<VkDescriptorPoolSize, 3> poolSizes{};
            poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
            poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
            poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            poolSizes[2].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
            VkDescriptorPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            poolInfo.pPoolSizes = poolSizes.data();
            poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
            if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &computeDescriptorPool) != VK_SUCCESS) {
                throw std::runtime_error("echec de la creation de la pool de descripteurs!");
            }
        }
        void ParticleSystem::createDescriptorSetLayout() {
            std::array<VkDescriptorSetLayoutBinding, 3> layoutBindings{};
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

            layoutBindings[2].binding = 2;
            layoutBindings[2].descriptorCount = 1;
            layoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            layoutBindings[2].pImmutableSamplers = nullptr;
            layoutBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = layoutBindings.size();
            layoutInfo.pBindings = layoutBindings.data();

            if (vkCreateDescriptorSetLayout(vkDevice.getDevice(), &layoutInfo, nullptr, &computeDescriptorSetLayout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create compute descriptor set layout!");
            }
        }
        void ParticleSystem::allocateDescriptorSets() {
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
        void ParticleSystem::updateDescriptorSets() {
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

                VkDescriptorBufferInfo particleStorageBufferInfoLastFrame{};
                particleStorageBufferInfoLastFrame.buffer = particles[i];
                particleStorageBufferInfoLastFrame.offset = 0;
                particleStorageBufferInfoLastFrame.range = mParticles.size() * sizeof(Particle);

                descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[0].dstSet = computeDescriptorSets[i];
                descriptorWrites[0].dstBinding = 0;
                descriptorWrites[0].dstArrayElement = 0;
                descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                descriptorWrites[0].descriptorCount = 1;
                descriptorWrites[0].pBufferInfo = &particleStorageBufferInfoLastFrame;

                VkDescriptorBufferInfo verticesStorageBufferInfoLastFrame{};
                verticesStorageBufferInfoLastFrame.buffer = vertexBuffer.value().get().getVertexBuffer(i);
                verticesStorageBufferInfoLastFrame.offset = 0;
                verticesStorageBufferInfoLastFrame.range = vertexBuffer.value().get().getVertexCount() * sizeof(graphic::Vertex);

                descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[1].dstSet = computeDescriptorSets[i];
                descriptorWrites[1].dstBinding = 1;
                descriptorWrites[1].dstArrayElement = 0;
                descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                descriptorWrites[1].descriptorCount = 1;
                descriptorWrites[1].pBufferInfo = &verticesStorageBufferInfoLastFrame;

                VkDescriptorBufferInfo quadsStorageBufferInfoCurrentFrame{};
                quadsStorageBufferInfoCurrentFrame.buffer = quads[i];
                quadsStorageBufferInfoCurrentFrame.offset = 0;
                quadsStorageBufferInfoCurrentFrame.range = mQuads.size() * sizeof(Quad);

                descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[2].dstSet = computeDescriptorSets[i];
                descriptorWrites[2].dstBinding = 2;
                descriptorWrites[2].dstArrayElement = 0;
                descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                descriptorWrites[2].descriptorCount = 1;
                descriptorWrites[2].pBufferInfo = &quadsStorageBufferInfoCurrentFrame;

                vkUpdateDescriptorSets(vkDevice.getDevice(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
            }
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
                if (vertexBuffer.has_value() && vertexBuffer.value().get().getVertexCount() > 0 && mParticles.size() > 0) {
                    VkDeviceSize bufferSize = sizeof(Particle) * mParticles.size();

                    if (bufferSize > maxParticlesSize) {
                        for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                            if (stagingParticles[i] != nullptr) {
                                vkDestroyBuffer(vkDevice.getDevice(), stagingParticles[i], nullptr);
                                vkFreeMemory(vkDevice.getDevice(), stagingParticlesMemory[i], nullptr);
                            }

                            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,stagingParticles[i], stagingParticlesMemory[i]);

                            if (particles[i] != nullptr) {
                                vkDestroyBuffer(vkDevice.getDevice(),particles[i], nullptr);
                                vkFreeMemory(vkDevice.getDevice(), particlesMemory[i], nullptr);
                            }

                            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, particles[i], particlesMemory[i]);
                        }

                        maxParticlesSize = bufferSize;
                        //needToUpdateDSs[p]  = true;
                    }
                    void* data;
                    vkMapMemory(vkDevice.getDevice(), stagingParticlesMemory[currentFrame], 0, bufferSize, 0, &data);
                    memcpy(data, mParticles.data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), stagingParticlesMemory[currentFrame]);
                    copyBuffer(stagingParticles[currentFrame], particles[currentFrame], bufferSize, commandBuffers[currentFrame]);

                    bufferSize = sizeof(Quad) * mQuads.size();

                    if (bufferSize > maxQuadsSize) {
                        for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                            if (stagingQuads[i] != nullptr) {
                                vkDestroyBuffer(vkDevice.getDevice(), stagingQuads[i], nullptr);
                                vkFreeMemory(vkDevice.getDevice(), stagingQuadsMemory[i], nullptr);
                            }

                            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,stagingQuads[i], stagingQuadsMemory[i]);

                            if (quads[i] != nullptr) {
                                vkDestroyBuffer(vkDevice.getDevice(),quads[i], nullptr);
                                vkFreeMemory(vkDevice.getDevice(), quadsMemory[i], nullptr);
                            }

                            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, quads[i], quadsMemory[i]);
                        }

                        maxQuadsSize = bufferSize;
                        //needToUpdateDSs[p]  = true;
                    }
                    vkMapMemory(vkDevice.getDevice(), stagingQuadsMemory[currentFrame], 0, bufferSize, 0, &data);
                    memcpy(data, mQuads.data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), stagingQuadsMemory[currentFrame]);
                    copyBuffer(stagingQuads[currentFrame], quads[currentFrame], bufferSize, commandBuffers[currentFrame]);
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
                    computeParams.deltaTime = dt.asSeconds();
                    vkCmdPushConstants(commandBuffers[currentFrame], computePipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ComputePC), &computeParams);
                    vkCmdDispatch(commandBuffers[currentFrame], mParticles.size(), vertexBuffer.value().get().getVertexCount(), 1);
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
            vkFreeCommandBuffers(vkDevice.getDevice(), commandPool, commandBuffers.size(), commandBuffers.data());
            vkDestroyCommandPool(vkDevice.getDevice(), commandPool, nullptr);
            vkDestroyPipelineLayout(vkDevice.getDevice(), computePipelineLayout, nullptr);
            vkDestroyPipeline(vkDevice.getDevice(), computePipeline, nullptr);
            vkDestroyDescriptorSetLayout(vkDevice.getDevice(), computeDescriptorSetLayout, nullptr);
            vkDestroyDescriptorPool(vkDevice.getDevice(), computeDescriptorPool, nullptr);
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                vkDestroyBuffer(vkDevice.getDevice(), particles[i], nullptr);
                vkFreeMemory(vkDevice.getDevice(), particlesMemory[i], nullptr);
                vkDestroyBuffer(vkDevice.getDevice(), stagingParticles[i], nullptr);
                vkFreeMemory(vkDevice.getDevice(), stagingParticlesMemory[i], nullptr);
                vkDestroyBuffer(vkDevice.getDevice(),quads[i], nullptr);
                vkFreeMemory(vkDevice.getDevice(), quadsMemory[i], nullptr);
                vkDestroyBuffer(vkDevice.getDevice(), stagingQuads[i], nullptr);
                vkFreeMemory(vkDevice.getDevice(), stagingQuadsMemory[i], nullptr);
            }

        }
    }

} // namespace thor
