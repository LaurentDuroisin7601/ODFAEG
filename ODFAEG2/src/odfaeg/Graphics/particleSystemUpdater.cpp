module;
#include <deque>
#include <entt.hpp>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <mutex>
#include <condition_variable>
#include <future>
#include <iostream>
#include <odfaeg/config.hpp>
#include "vk_mem_alloc.h"
//import odfaeg.graphic.particleSystemUpdater;
module odfaeg.graphic.particleSystemUpdater;
import odfaeg.graphic.gpuContext;
import odfaeg.graphic.descriptor;
import odfaeg.entity.primitiveType;
import odfaeg.entity.emittors;
import odfaeg.graphic.device;
import odfaeg.core.clock;
namespace odfaeg {
    namespace graphic {
        ParticleSystemUpdater& ParticleSystemUpdater::instance(std::condition_variable& cv, std::mutex& mtx) {
            static ParticleSystemUpdater instance(cv, mtx);
            return instance;
        }
        ParticleSystemUpdater::ParticleSystemUpdater(std::condition_variable& cv, std::mutex& mtx) :
        cv2(cv),
        mtx2(mtx),
        particlesEmittorShader(GPUContext::instance().getDevice()),
        particlesUpdaterShader(GPUContext::instance().getDevice()),
        particlesVerticesShader(GPUContext::instance().getDevice()),
        particlesSystemsStaggingBuffer(GPUContext::instance().getDevice()),
        particlesQuadsStaggingBuffer(GPUContext::instance().getDevice()),
        particlesEmittorsStaggingBuffer(GPUContext::instance().getDevice()),
        aliveCountStaggingBuffer(GPUContext::instance().getDevice()),
        particlesStaggingBuffer(GPUContext::instance().getDevice()),
        commandPool(GPUContext::instance().getDevice())
        {
            setInterval(core::seconds(0.1f));
            particlesSystemsBuffer.emplace_back(GPUContext::instance().getDevice());
            particlesBuffer.emplace_back(GPUContext::instance().getDevice());
            particlesQuadsBuffer.emplace_back(GPUContext::instance().getDevice());
            particlesEmittorsBuffer.emplace_back(GPUContext::instance().getDevice());
            aliveCountBuffer.emplace_back(GPUContext::instance().getDevice());
            particlesSystemsBuffer[0].create(sizeof(ParticlesSystemData), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY);
            particlesQuadsBuffer[0].create(sizeof(Quad), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY);
            particlesEmittorsBuffer[0].create(sizeof(entity::UniversalEmittor), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY);
            particlesBuffer[0].create(sizeof(entity::Particle), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY);
            aliveCountBuffer[0].create(sizeof(unsigned int),VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY);
            needToUpdateBuffers = false;
            std::string shaderDir = std::string(ODFAEG_INSTALL_DIR) + "/Shader";
            if (!particlesEmittorShader.loadFromFile(shaderDir + "/particleEmission.comp")) {
                throw std::runtime_error("Failed to compile particle emission compute shader!");
            }
            if (!particlesUpdaterShader.loadFromFile(shaderDir + "/particleUpdate.comp")) {
                throw std::runtime_error("Failed to compile particle update compute shader!");
            }
            if (!particlesVerticesShader.loadFromFile(shaderDir + "/particleVerticesUpdater.comp")) {
                throw std::runtime_error("Failed to compile particle vertices compute shader!");
            }
            std::vector<VkPushConstantRange> pushConstants;
            VkPushConstantRange pushConstant;
            pushConstant.offset = 0;
            pushConstant.size = sizeof(DeltaTimePC);
            pushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
            pushConstants.push_back(pushConstant);
            DescriptorSetLayout& particlesEmittorsLayout = GPUContext::instance().getDescriptorSetLayout(particlesEmittorShader, 4);
            for (unsigned int i = 0; i < 4; i++) {
                particlesEmittorsLayout.updateLayout(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
            }
            particlesEmittorsLayout.update();
            GPUContext::instance().getComputePipeline(particlesEmittorShader).createComputePipeline(particlesEmittorShader, GPUContext::instance().getDescriptorSetLayout(particlesEmittorShader), pushConstants);
            DescriptorSetLayout& particlesUpdaterLayout = GPUContext::instance().getDescriptorSetLayout(particlesUpdaterShader, 3);
            for (unsigned int i = 0; i < 3; i++) {
                particlesUpdaterLayout.updateLayout(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
            }
            particlesUpdaterLayout.update();
            GPUContext::instance().getComputePipeline(particlesUpdaterShader).createComputePipeline(particlesUpdaterShader, GPUContext::instance().getDescriptorSetLayout(particlesUpdaterShader), pushConstants);
            DescriptorSetLayout& particlesVerticesLayout = GPUContext::instance().getDescriptorSetLayout(particlesVerticesShader, 7);
            for (unsigned int i = 0; i < 5; i++) {
                particlesVerticesLayout.updateLayout(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
            }
            particlesVerticesLayout.updateLayout(5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES, VK_SHADER_STAGE_COMPUTE_BIT);
            particlesVerticesLayout.updateLayout(6, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
            particlesVerticesLayout.update();
            GPUContext::instance().getComputePipeline(particlesVerticesShader).createComputePipeline(particlesVerticesShader, GPUContext::instance().getDescriptorSetLayout(particlesVerticesShader));
            DescriptorPool& particlesEmittorsDescriptorPool = GPUContext::instance().getDescriptorPool(particlesEmittorShader, 4);
            for (unsigned int i = 0; i < 4; i++) {
                particlesEmittorsDescriptorPool.updatePoolSize(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
            }
            particlesEmittorsDescriptorPool.update();
            DescriptorPool& particlesUpdaterDescriptorPool = GPUContext::instance().getDescriptorPool(particlesUpdaterShader, 3);
            for (unsigned int i = 0; i < 3; i++) {
                particlesUpdaterDescriptorPool.updatePoolSize(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
            }
            particlesUpdaterDescriptorPool.update();
            DescriptorPool& particlesVerticesDescriptorPool = GPUContext::instance().getDescriptorPool(particlesVerticesShader, 7);
            for (unsigned int i = 0; i < 5; i++) {
                particlesVerticesDescriptorPool.updatePoolSize(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
            }
            particlesVerticesDescriptorPool.updatePoolSize(5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES);
            particlesVerticesDescriptorPool.updatePoolSize(6, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
            particlesVerticesDescriptorPool.update();
            DescriptorSet::allocate(particlesEmittorsDescriptorPool, particlesEmittorsLayout, GPUContext::instance().getDescriptorSets(particlesEmittorShader, 4, 1));
            DescriptorSet::allocate(particlesUpdaterDescriptorPool, particlesUpdaterLayout, GPUContext::instance().getDescriptorSets(particlesUpdaterShader, 3, 1));
            DescriptorSet::allocate(particlesVerticesDescriptorPool, particlesVerticesLayout, GPUContext::instance().getDescriptorSets(particlesVerticesShader, 7, 1));
            Device::QueueFamilyIndices indices = GPUContext::instance().getDevice().findQueueFamilies(GPUContext::instance().getDevice().getPhysicalDevice());
            commandPool.create(indices.graphicsFamily.value());
            commandPool.createCommandBuffers(true, 1);
            //std::cout<<"command buffer : "<<commandPool.getHandle(0)<<std::endl;
            GPUContext::instance().getSharedFence(0).emplace_back(GPUContext::instance().getDevice());
            GPUContext::instance().getSharedFence(0)[0].create();
            /*GPUContext::instance().getSharedSemaphore(0).emplace_back(GPUContext::instance().getDevice());
            GPUContext::instance().getSharedSemaphore(0)[0].create(true, 0);*/
            ubo.emplace_back(GPUContext::instance().getDevice());
            ubo[0].create(sizeof(AABB), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
            needToUpdateDescriptorSets = needToUpdateBuffers = false;
            ready.store(true);  
            //buffersReady.store(false);
            start();
            //cv.notify_one();            
        }
        void ParticleSystemUpdater::addParticleSystem(entity::ParticleSystem* particleSystem) {
            particleSystem->computeQuads();
            particleSystem->computeVertices();
            particlesSystems.push_back(particleSystem);
            needToUpdateBuffers = true;
        }
        void ParticleSystemUpdater::updateDescriptorSets() {

            DescriptorSet& particlesEmittorSet = GPUContext::instance().getDescriptorSets(particlesEmittorShader, 4, 1)[0];
            //std::cout<<"update offset in output model data"<<std::endl;
            particlesEmittorSet.updateBufferInfos(0, particlesBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            particlesEmittorSet.updateBufferInfos(1, particlesEmittorsBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            particlesEmittorSet.updateBufferInfos(2, aliveCountBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            particlesEmittorSet.updateBufferInfos(3, particlesSystemsBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            particlesEmittorSet.updateDescriptorSet();
            DescriptorSet& particlesUpdaterSet = GPUContext::instance().getDescriptorSets(particlesUpdaterShader, 3, 1)[0];
            //std::cout<<"update offset in output model data"<<std::endl;
            particlesUpdaterSet.updateBufferInfos(0, particlesBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            particlesUpdaterSet.updateBufferInfos(1, aliveCountBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            particlesUpdaterSet.updateBufferInfos(2, particlesSystemsBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            particlesUpdaterSet.updateDescriptorSet();
            DescriptorSet& particlesVerticesSet = GPUContext::instance().getDescriptorSets(particlesVerticesShader, 7, 1)[0];
            /*std::cout<<"sizes : "<<particlesQuadsBuffer.size()<<","<<particlesBuffer.size()<<","<<particlesSystemsBuffer.size()<<","
            <<GPUContext::instance().getSharedBuffers(SUBMESHES_BUFFER).size()<<","<<GPUContext::instance().getSharedVertexBuffer(VERTEX_BUFFER)[Triangles].size()<<","
            <<aliveCountBuffer.size()<<","<<ubo.size()<<std::endl;*/

            particlesVerticesSet.updateBufferInfos(0, particlesQuadsBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            particlesVerticesSet.updateBufferInfos(1, particlesBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            particlesVerticesSet.updateBufferInfos(2, particlesSystemsBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            particlesVerticesSet.updateBufferInfos(3, GPUContext::instance().getSharedBuffers(SUBMESHES_BUFFER), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            particlesVerticesSet.updateBufferInfos(4, aliveCountBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            particlesVerticesSet.updateBufferInfos(5, true, GPUContext::instance().getSharedVertexBuffer(VERTEX_BUFFER), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            particlesVerticesSet.updateBufferInfos(6, ubo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
            particlesVerticesSet.updateDescriptorSet();
        }
        void ParticleSystemUpdater::updateBuffers() {
            if (needToUpdateBuffers) {
                std::unique_lock<std::mutex> lock(mtx3);
                cv3.wait(lock, [this]{return buffersReady.load();});
                
                std::unique_lock<std::mutex> lock2(mtx2);
                buffersReady.store(false);
                maxParticles = 0;
                unsigned int currentQuadOffset = 0;
                std::vector<ParticlesSystemData> particlesSystemsData;
                std::vector<Quad> particlesQuadsData;
                emittors.clear();
                for (unsigned int i = 0; i < particlesSystems.size(); i++) {
                    ParticlesSystemData psData;
                    psData.id = i;
                    psData.offsetInSubMeshBuffer = particlesSystems[i]->subMeshOffset;
                    psData.offsetInAliveCountBuffer = i;
                    psData.offsetInParticleBuffer = maxParticles;
                    psData.offsetInQuadBuffer = currentQuadOffset;
                    psData.subMeshCount = particlesSystems[i]->getSubMeshesCount();
                    //std::cout<<"submesh count : "<<psData.subMeshCount<<std::endl;
                    particlesSystemsData.push_back(psData);
                    currentQuadOffset += particlesSystems[i]->getQuads().size();
                    maxParticles += particlesSystems[i]->computeMaxParticles();
                    particlesQuadsData.insert(particlesQuadsData.end(), particlesSystems[i]->getQuads().begin(), particlesSystems[i]->getQuads().end());
                    for (unsigned int j =0; j < particlesSystems[i]->getEmittors<entity::UniversalEmittor>().size(); j++) {
                        particlesSystems[i]->getEmittors<entity::UniversalEmittor>()[j].particleSystemId = i;
                    }
                    emittors.insert(emittors.end(), particlesSystems[i]->getEmittors<entity::UniversalEmittor>().begin(), particlesSystems[i]->getEmittors<entity::UniversalEmittor>().end());
                }
                //std::cout<<"CPU max particles : "<<maxParticles<<", nb mittors : "<<emittors.size()<<std::endl;
                std::vector<unsigned int> aliveCounts;
                for (unsigned int i = 0; i < particlesSystems.size(); i++) {
                    aliveCounts.push_back(0);
                }
                entity::Particle particle;
                particle.alive = 0;
                std::vector<entity::Particle> particles;
                for (unsigned int i = 0; i < maxParticles; i++) {
                    particles.push_back(particle);
                }
                particlesEmittorsStaggingBuffer.create(sizeof(entity::UniversalEmittor)*emittors.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
                particlesEmittorsStaggingBuffer.update(emittors.data(), emittors.size()*sizeof(entity::UniversalEmittor));
                particlesSystemsStaggingBuffer.create(sizeof(ParticlesSystemData)*particlesSystemsData.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
                particlesSystemsStaggingBuffer.update(particlesSystemsData.data(), sizeof(ParticlesSystemData)*particlesSystemsData.size());
                particlesQuadsStaggingBuffer.create(sizeof(Quad) * particlesQuadsData.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
                particlesQuadsStaggingBuffer.update(particlesQuadsData.data(), sizeof(Quad)*particlesQuadsData.size());
                particlesStaggingBuffer.create(sizeof(entity::Particle) * particles.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
                particlesStaggingBuffer.update(particles.data(), sizeof(entity::Particle)*particles.size());
                aliveCountStaggingBuffer.create(sizeof(unsigned int) * particlesSystems.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
                aliveCountStaggingBuffer.update(aliveCounts.data(), aliveCounts.size());
                particlesBuffer[0].create(sizeof(entity::Particle)*maxParticles, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
                particlesQuadsBuffer[0].create(sizeof(Quad)*particlesQuadsData.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
                particlesEmittorsBuffer[0].create(sizeof(entity::UniversalEmittor) * emittors.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
                particlesSystemsBuffer[0].create(sizeof(ParticlesSystemData) * particlesSystemsData.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
                aliveCountBuffer[0].create(sizeof(unsigned int) * particlesSystems.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

                //commandPool.beginRecordCommandBuffer(0);
                Buffer::copyBuffer(particlesEmittorsStaggingBuffer, particlesEmittorsBuffer[0], sizeof(entity::UniversalEmittor)*emittors.size(), commandPool.getHandle(0));
                Buffer::copyBuffer(particlesSystemsStaggingBuffer, particlesSystemsBuffer[0], sizeof(ParticlesSystemData)*particlesSystemsData.size(), commandPool.getHandle(0));
                Buffer::copyBuffer(particlesQuadsStaggingBuffer, particlesQuadsBuffer[0], sizeof(Quad)*particlesQuadsData.size(), commandPool.getHandle(0));
                Buffer::copyBuffer(aliveCountStaggingBuffer, aliveCountBuffer[0], sizeof(unsigned int)*particlesSystems.size(), commandPool.getHandle(0));
                Buffer::copyBuffer(particlesStaggingBuffer, particlesBuffer[0], sizeof(entity::Particle)*particles.size(), commandPool.getHandle(0));
                //commandPool.endRecordCommandBuffer(0);
                /*VkSubmitInfo submitInfo{};
                submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submitInfo.commandBufferCount = commandPool.getHandles().size();
                submitInfo.pCommandBuffers = commandPool.getHandles().data();
                Device::QueueFamilyIndices indices = GPUContext::instance().getDevice().findQueueFamilies(GPUContext::instance().getDevice().getPhysicalDevice());
                if (vkQueueSubmit(GPUContext::instance().getDevice().getQueue(indices.graphicsFamily.value(), 1), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
                    throw std::runtime_error("Echec de l'envoi d'un command buffer!");
                }
                vkDeviceWaitIdle(GPUContext::instance().getDevice().getDevice());*/
                needToUpdateDescriptorSets = true;
                needToUpdateBuffers = false;
            }
        }
        void ParticleSystemUpdater::setReady(bool r) {
            ready.store(r);
        }
        void ParticleSystemUpdater::setBuffersReady(bool r) {
            buffersReady.store(r);
        }
        bool ParticleSystemUpdater::areBuffersReady() {
            return buffersReady.load();
        }
        void ParticleSystemUpdater::setSubmitReady(bool r) {
            submitReady.store(r);
        }
        bool ParticleSystemUpdater::isSubmitReady() {
            return submitReady.load();
        }
        bool ParticleSystemUpdater::isReady() {
            return ready.load();
        }
        void ParticleSystemUpdater::onUpdate() {
            //std::cout<<"update ! "<<std::endl; 
            std::unique_lock<std::mutex> lock(mtx);
                
                //std::lock_guard<std::recursive_mutex> lock2(getGlobalMutex());
                //cv.wait_for(lock, std::chrono::milliseconds(100));
                //std::cout<<"lock"<<std::endl;
            cv.wait(lock, [this]() {
                return (ready.load() || !isRunning());
            });
            
            std::unique_lock<std::mutex> lock2(mtx2);
            
            ready.store(false);
            if (isRunning()) {


                deltaTimePC.dt = getElapsedTime().asSeconds();
                //std::cout<<"CPU dt : "<<getElapsedTime().asSeconds()<<std::endl;
                restart();

                commandPool.beginRecordCommandBuffer(0);
                if (particlesSystems.size() > 0) {
                    updateBuffers();
                    if (needToUpdateDescriptorSets) {
                        updateDescriptorSets();
                        needToUpdateDescriptorSets = false;
                    }

                    vkCmdBindPipeline(commandPool.getHandle(0), VK_PIPELINE_BIND_POINT_COMPUTE, GPUContext::instance().getComputePipeline(particlesEmittorShader).getHandle());
                    std::vector<VkDescriptorSet> sets;
                    for (unsigned int i = 0; i < GPUContext::instance().getDescriptorSets(particlesEmittorShader).size(); i++) {
                        sets.push_back(GPUContext::instance().getDescriptorSets(particlesEmittorShader)[i][0].getHandle());
                    }
                    vkCmdBindDescriptorSets(commandPool.getHandle(0), VK_PIPELINE_BIND_POINT_COMPUTE, GPUContext::instance().getComputePipeline(particlesEmittorShader).getLayout(), 0, sets.size(), sets.data(), 0, 0);
                    vkCmdPushConstants(commandPool.getHandle(0), GPUContext::instance().getComputePipeline(particlesEmittorShader).getLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(DeltaTimePC), &deltaTimePC);
                    VkMemoryBarrier barrier{};
                    barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

                    VkBufferMemoryBarrier buf{};
                    buf.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
                    buf.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    buf.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                    buf.buffer = particlesBuffer[0].getHandle();
                    buf.offset = 0;
                    buf.size = VK_WHOLE_SIZE;

                    vkCmdPipelineBarrier(
                        commandPool.getHandle(0),
                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                        0,
                        1, &barrier,
                        1, &buf,
                        0, nullptr
                    );
                    buf.buffer = aliveCountBuffer[0].getHandle();
                    vkCmdPipelineBarrier(
                        commandPool.getHandle(0),
                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                        0,
                        1, &barrier,
                        1, &buf,
                        0, nullptr
                    );
                    buf.buffer = GPUContext::instance().getSharedVertexBuffer(VERTEX_BUFFER)[entity::PrimitiveType::Triangles].getVertexBuffer(0).getHandle();
                    vkCmdPipelineBarrier(
                        commandPool.getHandle(0),
                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                        0,
                        1, &barrier,
                        1, &buf,
                        0, nullptr
                    );
                    vkCmdDispatch(commandPool.getHandle(0), particlesSystems.size(), emittors.size(), 1);
                    vkCmdBindPipeline(commandPool.getHandle(0), VK_PIPELINE_BIND_POINT_COMPUTE, GPUContext::instance().getComputePipeline(particlesUpdaterShader).getHandle());
                    sets.clear();
                    for (unsigned int i = 0; i < GPUContext::instance().getDescriptorSets(particlesUpdaterShader).size(); i++) {
                        sets.push_back(GPUContext::instance().getDescriptorSets(particlesUpdaterShader)[i][0].getHandle());
                    }
                    vkCmdBindDescriptorSets(commandPool.getHandle(0), VK_PIPELINE_BIND_POINT_COMPUTE, GPUContext::instance().getComputePipeline(particlesUpdaterShader).getLayout(), 0, sets.size(), sets.data(), 0, 0);
                    vkCmdPushConstants(commandPool.getHandle(0), GPUContext::instance().getComputePipeline(particlesUpdaterShader).getLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(DeltaTimePC), &deltaTimePC);
                    VkMemoryBarrier mem{};
                    mem.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                    mem.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                    mem.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                    vkCmdPipelineBarrier(
                        commandPool.getHandle(0),
                        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                        0,
                        1, &mem,
                        0, nullptr,
                        0, nullptr
                    );
                    vkCmdDispatch(commandPool.getHandle(0), particlesSystems.size(), maxParticles, 1);
                    sets.clear();
                    for (unsigned int i = 0; i < GPUContext::instance().getDescriptorSets(particlesVerticesShader).size(); i++) {
                        sets.push_back(GPUContext::instance().getDescriptorSets(particlesVerticesShader)[i][0].getHandle());
                    }
                    vkCmdBindPipeline(commandPool.getHandle(0), VK_PIPELINE_BIND_POINT_COMPUTE, GPUContext::instance().getComputePipeline(particlesVerticesShader).getHandle());
                    vkCmdBindDescriptorSets(commandPool.getHandle(0), VK_PIPELINE_BIND_POINT_COMPUTE, GPUContext::instance().getComputePipeline(particlesVerticesShader).getLayout(), 0, sets.size(), sets.data(), 0, 0);
                    vkCmdPipelineBarrier(
                        commandPool.getHandle(0),
                        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                        0,
                        1, &mem,
                        0, nullptr,
                        0, nullptr
                    );
                    vkCmdDispatch(commandPool.getHandle(0), particlesSystems.size(), maxParticles, 1);
                    mem.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                    mem.dstAccessMask =
                        VK_ACCESS_SHADER_READ_BIT |            // SSBO dans VS/FS
                        VK_ACCESS_INDIRECT_COMMAND_READ_BIT;   // draw indirect

                    vkCmdPipelineBarrier(
                        commandPool.getHandle(0),
                        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                        VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT |
                        VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                        0,
                        1, &mem,
                        0, nullptr,
                        0, nullptr
                    );
                }
                //std::cout<<"update particles"<<std::endl;
                commandPool.endRecordCommandBuffer(0);
               /* std::vector<std::uint64_t> signalValues;


                signalValues.push_back(GPUContext::instance().getSharedSemaphore(0)[0].getValue()+1);
                VkTimelineSemaphoreSubmitInfo timelineInfo{};
                timelineInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
                timelineInfo.signalSemaphoreValueCount = signalValues.size();
                timelineInfo.pSignalSemaphoreValues = signalValues.data();*/

                VkSubmitInfo submitInfo{};
                //submitInfo.pNext = &timelineInfo;
                submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submitInfo.commandBufferCount = commandPool.getHandles().size();
                submitInfo.pCommandBuffers = commandPool.getHandles().data();

                /*submitInfo.signalSemaphoreCount = 1;
                submitInfo.pSignalSemaphores = &GPUContext::instance().getSharedSemaphore(0)[0].getHandle();
                GPUContext::instance().getSharedSemaphore(0)[0].incrementValue();*/
                Device::QueueFamilyIndices indices = GPUContext::instance().getDevice().findQueueFamilies(GPUContext::instance().getDevice().getPhysicalDevice());
                if (vkQueueSubmit(GPUContext::instance().getDevice().getQueue(indices.graphicsFamily.value(), 2), 1, &submitInfo, GPUContext::instance().getSharedFence(0)[0].getHandle()) != VK_SUCCESS) {
                    throw std::runtime_error("Echec de l'envoi d'un command buffer!");
                }
                //
                //std::cout<<"buffers ready!"<<std::endl;
                //std::this_thread::sleep_for(std::chrono::duration<float>(1000.f));
                submitReady.store(true);
                cv2.notify_all();
                //GPUContext::instance().getSharedFence(0)[0].waitForFences(VK_TRUE, UINT64_MAX);
            }
            //std::cout<<"update : "<<isRunning()<<std::endl;
        }
        void ParticleSystemUpdater::setCamera(Camera camera) {
            cullingInfo.center = camera.getViewVolume().getCenter();
            cullingInfo.size = camera.getViewVolume().getSize();
            ubo[0].update(&cullingInfo, sizeof(AABB));
            this->camera = camera;
            //needToUpdateDescriptorSets = true;
        }
    }
}
