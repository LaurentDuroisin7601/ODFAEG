//
// Created by laurent on 09/06/2026
module;
#include <vector>
#include <vulkan/vulkan.h>
#include <string>
#include <stdexcept>
#include "odfaeg/config.hpp"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <condition_variable>
#include "vk_mem_alloc.h"
//import odfaeg.graphic.boneAnimUpdater;
module odfaeg.graphic.boneAnimUpdater;
import odfaeg.graphic.gpuContext;
import odfaeg.graphic.descriptor;
import odfaeg.math.matrix;
import odfaeg.graphic.device;
import odfaeg.graphic.vertex;
import odfaeg.graphic.primitiveType;
namespace odfaeg {
    namespace graphic {
        BoneAnimUpdater& BoneAnimUpdater::instance(std::condition_variable& cv, std::mutex& mtx) {
            static BoneAnimUpdater instance(cv, mtx);
            return instance;
        }
        BoneAnimUpdater::BoneAnimUpdater(std::condition_variable& cv, std::mutex& mtx) :
        cv2(cv),
        mtx2(mtx),
        staggingBoneAnims(GPUContext::instance().getDevice()),
        staggingFinalBonesMatrices(GPUContext::instance().getDevice()),
        staggingAnimsSubmeshes(GPUContext::instance().getDevice()),
        boneAnimShader(GPUContext::instance().getDevice()),
        commandPool(GPUContext::instance().getDevice()) {
            boneAnims.emplace_back(GPUContext::instance().getDevice());
            finalBonesMatrices.emplace_back(GPUContext::instance().getDevice());
            animsSubmeshes.emplace_back(GPUContext::instance().getDevice());
            std::string shaderDir = std::string(ODFAEG_INSTALL_DIR) + "/Shader";
            if (!boneAnimShader.loadFromFile(shaderDir + "/boneAnim.comp")) {
                throw std::runtime_error("Could not load bone anim shader");
            }
            DescriptorSetLayout& boneLayout = GPUContext::instance().getDescriptorSetLayout(boneAnimShader, 6);
            for (unsigned int i = 0; i < 4; i++) {
                boneLayout.updateLayout(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
            }
            for (unsigned int i = 4; i < 6; i++) {
                boneLayout.updateLayout(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES, VK_SHADER_STAGE_COMPUTE_BIT);
            }
            boneLayout.update();
            GPUContext::instance().getComputePipeline(boneAnimShader).createComputePipeline(boneAnimShader, GPUContext::instance().getDescriptorSetLayout(boneAnimShader));
            DescriptorPool& bonePool = GPUContext::instance().getDescriptorPool(boneAnimShader, 6);
            for (unsigned int i = 0; i < 4; i++) {
                bonePool.updatePoolSize(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
            }
            for (unsigned int i = 4; i < 6; i++) {
                bonePool.updatePoolSize(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES);
            }
            bonePool.update();
            DescriptorSet::allocate(bonePool, boneLayout, GPUContext::instance().getDescriptorSets(boneAnimShader, 6, 1));
            GPUContext::instance().getSharedFence(2).emplace_back(GPUContext::instance().getDevice());
            GPUContext::instance().getSharedFence(2)[0].create();
            /*GPUContext::instance().getSharedSemaphore(0).emplace_back(GPUContext::instance().getDevice());
            GPUContext::instance().getSharedSemaphore(0)[0].create(true);*/
            Device::QueueFamilyIndices indices = GPUContext::instance().getDevice().findQueueFamilies(GPUContext::instance().getDevice().getPhysicalDevice());
            commandPool.create(indices.graphicsFamily.value());
            commandPool.createCommandBuffers(true, 1);
            for (unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {
                verticesIn.emplace_back(GPUContext::instance().getDevice());
            }
            needToUpdateBuffers = needToUpdateDescriptorSets = false;
            ready.store(true);
            start();
            //cv.notify_one();
        }
        void BoneAnimUpdater::setReady(bool r) {
            ready.store(r);
        }
        void BoneAnimUpdater::addBoneAnim(Animator* boneAnim) {
            anims.push_back(boneAnim);
            needToUpdateBuffers = true;
        }
        void BoneAnimUpdater::setBuffersReady(bool r) {
            buffersReady.store(r);
        }
        bool BoneAnimUpdater::areBuffersReady() {
            return buffersReady.load();
        }
        void BoneAnimUpdater::setSubmitReady(bool r) {
            submitReady.store(r);
        }
        bool BoneAnimUpdater::isSubmitReady() {
            return submitReady.load();
        }
        void BoneAnimUpdater::updateBuffers() {
            if (needToUpdateBuffers) {
                //std::cout<<"wait update buffers!"<<std::endl;
                std::unique_lock<std::mutex> lock(mtx3);
                   
                cv3.wait(lock, [this]{return buffersReady.load();});
                
                std::unique_lock<std::mutex> lock2(mtx2);
                buffersReady.store(false);
                std::cout<<"update buffers!"<<std::endl;
                std::vector<BoneAnimData> boneAnimsData;
                std::vector<SubMesh> animsSubmeshDatas;
                unsigned int finalBoneMatricesSize = 0, currentBoneAnimId = 0, currentVertexOffset = 0, currentAnimSubmeshOffset = 0;
                for (unsigned int i = 0; i < anims.size(); i++) {
                    SubMesh animsSubmeshData;
                    animsSubmeshData.vertexOffset = currentVertexOffset;
                    animsSubmeshDatas.push_back(animsSubmeshData);
                    BoneAnimData animData;
                    animData.id = currentBoneAnimId;
                    animData.animsSubmeshOffset = currentAnimSubmeshOffset;
                    currentAnimSubmeshOffset += anims[i]->getSubMeshesCount();
                    animData.subMeshesOffset = anims[i]->subMeshOffset;
                    //std::cout<<"current submeshes offset : "<<anims[i]->subMeshOffset<<std::endl;
                    animData.nbSubmeshes = anims[i]->getSubMeshesCount();
                    boneAnimsData.push_back(animData);
                    finalBoneMatricesSize += anims[i]->getFinalBoneMatrices().size();
                    currentBoneAnimId++;
                    for (unsigned int j = 0; j < anims[i]->getSubMeshesCount(); j++) {
                        for (unsigned int k = 0; k < anims[i]->getSubMeshes()[j].getVertexBuffer().getVertexCount(); k++) {
                            verticesIn[anims[i]->getSubMeshes()[j].getVertexBuffer().getPrimitiveType()].append(anims[i]->getSubMeshes()[j].getVertexBuffer()[k]);
                            currentVertexOffset++;
                        }
                    }
                }
                for (unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {
                    verticesIn[i].setPrimitiveType(static_cast<PrimitiveType>(i));
                    verticesIn[i].update(commandPool.getHandle(0));
                }
                staggingBoneAnims.create(sizeof(BoneAnimData) * boneAnimsData.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
                staggingBoneAnims.update(boneAnimsData.data(), sizeof(BoneAnimData) * boneAnimsData.size());
                boneAnims[0].create(sizeof(BoneAnimData) * boneAnimsData.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
                Buffer::copyBuffer(staggingBoneAnims, boneAnims[0], sizeof(BoneAnimData) * boneAnimsData.size(), commandPool.getHandle(0));
                //std::cout<<"size : "<<sizeof(glm::mat4) * finalBoneMatricesSize<<std::endl;
                staggingFinalBonesMatrices.create(sizeof(glm::mat4) * finalBoneMatricesSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
                finalBonesMatrices[0].create(sizeof(glm::mat4) * finalBoneMatricesSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
                staggingAnimsSubmeshes.create(sizeof(SubMesh) * animsSubmeshDatas.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
                staggingAnimsSubmeshes.update(animsSubmeshDatas.data(), sizeof(SubMesh) * animsSubmeshDatas.size());
                animsSubmeshes[0].create(sizeof(SubMesh) * animsSubmeshDatas.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
                Buffer::copyBuffer(staggingAnimsSubmeshes, animsSubmeshes[0], sizeof(SubMesh) * animsSubmeshDatas.size(), commandPool.getHandle(0));
                needToUpdateBuffers = false;
                needToUpdateDescriptorSets = true;
            }
            for (unsigned int i = 0; i < anims.size(); i++) {
                anims[i]->updateAnimation(getElapsedTime().asSeconds());

            }
            restart();
            std::vector<glm::mat4> finalBoneMatrices;
            for (unsigned int i = 0; i < anims.size(); i++) {
                //std::cout<<"final bones matrices size : "<<finalBoneMatrices.size()<<std::endl;
                for (unsigned int j = 0 ; j < anims[i]->getFinalBoneMatrices().size(); j++) {
                    finalBoneMatrices.push_back(anims[i]->getFinalBoneMatrices()[j]);
                    //std::cout<<"i : "<<j<<",final bone matrix : "<<glm::to_string(anims[i]->getFinalBoneMatrices()[j])<<std::endl;
                }
                //std::cout<<"vertex count : "<<anims[i]->getSubMeshes()[0].getVertexBuffer().getVertexCount()<<std::endl;
                /*for (unsigned int v = 0; v < anims[i]->getSubMeshes()[0].getVertexBuffer().getVertexCount() ; v++) {
                    glm::vec4 totalPosition = glm::vec4(0.0f);
                    Vertex vertex = anims[i]->getSubMeshes()[0].getVertexBuffer()[v];
                    for(unsigned int j = 0 ; j < 4; j++) {


                        if (vertex.m_BoneIDs[j] == -1) {
                            continue;
                        }
                        if(vertex.m_Weights[j] >= MAX_BONES) {
                            //debugPrintfEXT("Max!");
                            totalPosition = glm::vec4(vertex.position.x(), vertex.position.y(), vertex.position.z(), 1);
                            break;
                        }
                        glm::mat4 finalBoneMatrix = finalBoneMatrices[vertex.m_BoneIDs[j]];
                        glm::vec4 localPosition = finalBoneMatrix * glm::vec4(vertex.position.x(), vertex.position.y(), vertex.position.z(), 1);
                        //std::cout<<"bone id : "<<vertex.m_BoneIDs[j]<<", final bone matrix"<<glm::to_string(finalBoneMatrix)<<std::endl;
                        totalPosition += localPosition  * vertex.m_Weights[j];
                        //std::cout<<"bone id : "<<vertex.m_BoneIDs[j]<<"position : "<<vertex.position<<", bone weight : "<<vertex.m_Weights[j]<<", total position : "<<glm::to_string(totalPosition)<<", final bone matrix"<<glm::to_string(finalBoneMatrix)<<std::endl;

                        //verticesBuffers[sm.primitiveType].vertices[vertexId].normal = mat3(finalBoneMatrix) * vertex.normal;
                    }
                    //std::cout<<"vertex idx : "<<v<<",total position : "<<glm::to_string(totalPosition)<<std::endl;
                }*/
            }
            //std::cout<<"update size : "<<sizeof(glm::mat4) * finalBoneMatrices.size()<<std::endl;
            staggingFinalBonesMatrices.update(finalBoneMatrices.data(), sizeof(glm::mat4) * finalBoneMatrices.size());
            Buffer::copyBuffer(staggingFinalBonesMatrices, finalBonesMatrices[0], sizeof(glm::mat4) * finalBoneMatrices.size(), commandPool.getHandle(0));

            //std::cout<<"final bones matrices size : "<<finalBoneMatrices.size()<<std::endl;

        }
        void BoneAnimUpdater::updateDescriptorSets() {
            DescriptorSet& boneAnimSet = GPUContext::instance().getDescriptorSets(boneAnimShader, 6, 1)[0];
            boneAnimSet.updateBufferInfos(0, boneAnims, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            boneAnimSet.updateBufferInfos(1, GPUContext::instance().getSharedBuffers(SUBMESHES_BUFFER), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            boneAnimSet.updateBufferInfos(2, finalBonesMatrices, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            boneAnimSet.updateBufferInfos(3, animsSubmeshes, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            boneAnimSet.updateBufferInfos(4, true, verticesIn, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            boneAnimSet.updateBufferInfos(5, true, GPUContext::instance().getSharedVertexBuffer(VERTEX_BUFFER), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            boneAnimSet.updateDescriptorSet();
        }
        void BoneAnimUpdater::onUpdate() {
            //std::cout<<"wait ba : "<<ready.load()<<std::endl;
            std::unique_lock<std::mutex> lock(mtx);
                //
                //std::lock_guard<std::recursive_mutex> lock2(getGlobalMutex());
                
            cv.wait(lock, [this]() {
               return (ready.load() || !isRunning());
            });
           
            std::unique_lock<std::mutex> lock2(mtx2);

            //std::cout<<"update : "<<isRunning()<<std::endl;
            ready.store(false);
            if (isRunning()) {
                commandPool.beginRecordCommandBuffer(0);
                if (anims.size() > 0) {
                    updateBuffers();
                    if (needToUpdateDescriptorSets) {
                        //std::cout<<"update ds"<<std::endl;
                        updateDescriptorSets();
                        needToUpdateDescriptorSets = false;
                    }
                    vkCmdBindPipeline(commandPool.getHandle(0), VK_PIPELINE_BIND_POINT_COMPUTE, GPUContext::instance().getComputePipeline(boneAnimShader).getHandle());
                    std::vector<VkDescriptorSet> sets;
                    for (unsigned int i = 0; i < GPUContext::instance().getDescriptorSets(boneAnimShader).size(); i++) {
                        sets.push_back(GPUContext::instance().getDescriptorSets(boneAnimShader)[i][0].getHandle());
                    }
                    vkCmdBindDescriptorSets(commandPool.getHandle(0), VK_PIPELINE_BIND_POINT_COMPUTE, GPUContext::instance().getComputePipeline(boneAnimShader).getLayout(), 0, sets.size(), sets.data(), 0, 0);
                    VkMemoryBarrier barrier{};
                    barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

                    VkBufferMemoryBarrier buf{};
                    buf.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
                    buf.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    buf.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                    buf.buffer = boneAnims[0].getHandle();
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
                    buf.buffer = GPUContext::instance().getSharedVertexBuffer(VERTEX_BUFFER)[Triangles].getVertexBuffer(0).getHandle();
                    vkCmdPipelineBarrier(
                        commandPool.getHandle(0),
                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                        0,
                        1, &barrier,
                        1, &buf,
                        0, nullptr
                    );
                    buf.buffer = finalBonesMatrices[0].getHandle();
                    vkCmdPipelineBarrier(
                        commandPool.getHandle(0),
                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                        0,
                        1, &barrier,
                        1, &buf,
                        0, nullptr
                    );
                    for (unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {
                        if (verticesIn[i].getVertexCount() > 0) {
                            vkCmdDispatch(commandPool.getHandle(0), anims.size(), verticesIn[i].getVertexCount(), 1);
                        }
                    }
                    VkMemoryBarrier mem{};
                    mem.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                    mem.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                    mem.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
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
                /*std::vector<std::uint64_t> signalValues;
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
                if (vkQueueSubmit(GPUContext::instance().getDevice().getQueue(indices.graphicsFamily.value(), 3), 1, &submitInfo, GPUContext::instance().getSharedFence(2)[0].getHandle()) != VK_SUCCESS) {
                    throw std::runtime_error("Echec de l'envoi d'un command buffer!");
                }
                //std::cout<<"buffers ready ba !"<<std::endl;
                submitReady.store(true);
                cv2.notify_all();
            }
        }
    } // graphic
} // odfaeg