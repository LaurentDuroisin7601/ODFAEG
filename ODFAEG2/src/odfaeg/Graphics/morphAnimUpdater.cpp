module;
#include <stdexcept>
#include <vector>
#include <odfaeg/config.hpp>
#include <vulkan/vulkan_core.h>
#include <iostream>
#include <condition_variable>
#include "vk_mem_alloc.h"
//import odfaeg.graphic.morphAnimUpdater;
module odfaeg.graphic.morphAnimUpdater;
import odfaeg.graphic.descriptor;
import odfaeg.graphic.gpuContext;
import odfaeg.graphic.device;
import odfaeg.graphic.gameObject;
import odfaeg.graphic.primitiveType;
namespace odfaeg {
    namespace graphic {
        MorphAnimUpdater& MorphAnimUpdater::instance(std::condition_variable& cv, std::mutex& mtx) {
            static MorphAnimUpdater morphAnimUpdater(cv, mtx);
            return morphAnimUpdater;
        }
        MorphAnimUpdater::MorphAnimUpdater(std::condition_variable& cv, std::mutex& mtx) :
        cv2(cv),
        mtx2(mtx),
        staggingMorphAnims(GPUContext::instance().getDevice()),
        staggingFramesAnims(GPUContext::instance().getDevice()),
        staggingSubmeshesFramesAnims(GPUContext::instance().getDevice()),
        morphAnimShader(GPUContext::instance().getDevice()),
        updateAnimIndexShader(GPUContext::instance().getDevice()),
        commandPool(GPUContext::instance().getDevice())
        {
            //setInterval(core::seconds(0.1f));
            morphAnims.emplace_back(GPUContext::instance().getDevice());
            framesAnims.emplace_back(GPUContext::instance().getDevice());
            submeshesFramesAnims.emplace_back(GPUContext::instance().getDevice());
            for (unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {
                verticesFramesAnims.emplace_back(GPUContext::instance().getDevice());
            }
            std::string shaderDir = std::string(ODFAEG_INSTALL_DIR) + "/Shader";
            if (!morphAnimShader.loadFromFile(shaderDir + "/morphAnim.comp")) {
                throw std::runtime_error("Could not load morph anim shader");
            }
            if (!updateAnimIndexShader.loadFromFile(shaderDir + "/updateAnimCurrentFrame.comp")) {
                throw std::runtime_error("Could not load update anim index shader");
            }
            std::vector<VkPushConstantRange> pushConstants;
            VkPushConstantRange pushConstant;
            pushConstant.offset = 0;
            pushConstant.size = sizeof(DeltaTimePC);
            pushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
            pushConstants.push_back(pushConstant);
            DescriptorSetLayout& updateAnimIndexLayout = GPUContext::instance().getDescriptorSetLayout(updateAnimIndexShader, 1);
            updateAnimIndexLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
            updateAnimIndexLayout.update();
            GPUContext::instance().getComputePipeline(updateAnimIndexShader).createComputePipeline(updateAnimIndexShader, GPUContext::instance().getDescriptorSetLayout(updateAnimIndexShader), pushConstants);
            DescriptorPool& updateAnimIndexPool = GPUContext::instance().getDescriptorPool(updateAnimIndexShader, 1);
            updateAnimIndexPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
            updateAnimIndexPool.update();
            DescriptorSet::allocate(updateAnimIndexPool, updateAnimIndexLayout, GPUContext::instance().getDescriptorSets(updateAnimIndexShader, 1, 1));
            DescriptorSetLayout& morphAnimsLayout = GPUContext::instance().getDescriptorSetLayout(morphAnimShader, 7);
            for (unsigned int i = 0; i < 4; i++) {
                morphAnimsLayout.updateLayout(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
            }
            for (unsigned int i = 4; i < 6; i++) {
                morphAnimsLayout.updateLayout(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES, VK_SHADER_STAGE_COMPUTE_BIT);
            }
            morphAnimsLayout.updateLayout(6, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
            morphAnimsLayout.update();
            GPUContext::instance().getComputePipeline(morphAnimShader).createComputePipeline(morphAnimShader, GPUContext::instance().getDescriptorSetLayout(morphAnimShader));
            DescriptorPool& morphAnimsPool = GPUContext::instance().getDescriptorPool(morphAnimShader, 7);
            for (unsigned int i = 0; i < 4; i++) {
                morphAnimsPool.updatePoolSize(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
            }
            for (unsigned int i = 4; i < 6; i++) {
                morphAnimsPool.updatePoolSize(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES);
            }
            morphAnimsPool.updatePoolSize(6, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
            morphAnimsPool.update();
            DescriptorSet::allocate(morphAnimsPool, morphAnimsLayout, GPUContext::instance().getDescriptorSets(morphAnimShader, 7, 1));
            Device::QueueFamilyIndices indices = GPUContext::instance().getDevice().findQueueFamilies(GPUContext::instance().getDevice().getPhysicalDevice());
            commandPool.create(indices.graphicsFamily.value());
            commandPool.createCommandBuffers(true, 1);
            GPUContext::instance().getSharedFence(1).emplace_back(GPUContext::instance().getDevice());
            GPUContext::instance().getSharedFence(1)[0].create();
            /*GPUContext::instance().getSharedSemaphore(1).emplace_back(GPUContext::instance().getDevice());
            GPUContext::instance().getSharedSemaphore(1)[0].create(true, 0);*/
            ubo.emplace_back(GPUContext::instance().getDevice());
            ubo[0].create(sizeof(AABB), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
            needToUpdateDescriptorSets = needToUpdateBuffers = false;
            ready.store(true);
            start();
            //cv.notify_one();
        }
        void MorphAnimUpdater::addMorphAnim(MorphAnim* morphAnim) {
            anims.push_back(morphAnim);
            needToUpdateBuffers = true;
        }
        void MorphAnimUpdater::updateDescriptorSets() {
            DescriptorSet& updateAnimIndexSet = GPUContext::instance().getDescriptorSets(updateAnimIndexShader, 1, 1)[0];
            updateAnimIndexSet.updateBufferInfos(0, morphAnims, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            updateAnimIndexSet.updateDescriptorSet();
            DescriptorSet& morphAnimSet = GPUContext::instance().getDescriptorSets(morphAnimShader, 7, 1)[0];
            morphAnimSet.updateBufferInfos(0, morphAnims, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            morphAnimSet.updateBufferInfos(1, framesAnims, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            morphAnimSet.updateBufferInfos(2, submeshesFramesAnims, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            morphAnimSet.updateBufferInfos(3, GPUContext::instance().getSharedBuffers(SUBMESHES_BUFFER), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            morphAnimSet.updateBufferInfos(4, true, verticesFramesAnims, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            morphAnimSet.updateBufferInfos(5, true, GPUContext::instance().getSharedVertexBuffer(VERTEX_BUFFER), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            morphAnimSet.updateBufferInfos(6, ubo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
            morphAnimSet.updateDescriptorSet();
        }
        void MorphAnimUpdater::updateBuffers() {
            if (needToUpdateBuffers) {
                std::unique_lock<std::mutex> lock(mtx3);
                cv3.wait(lock, [this]{return buffersReady.load();});                
                std::unique_lock<std::mutex> lock2(mtx2);
                buffersReady.store(false);
                std::vector<MorphAnimData> morphAnimsData;
                std::vector<FramesAnimData> framesAnimsData;
                std::vector<SubMeshData> framesSubMeshes;
                unsigned int currentFramesOffset = 0, currentFrameSubmeshOffset = 0, currentVertexOffset = 0;;
                /*for (unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {
                    verticesFramesAnims[i].clear();
                }*/
                for (unsigned int i = 0; i < anims.size(); i++) {
                    MorphAnimData morphAnimData;
                    morphAnimData.subMeshesOffset = anims[i]->subMeshOffset;
                    morphAnimData.nbSubMeshes = anims[i]->getSubMeshesCount();
                    //std::cout<<"nb submeshses : "<<anims[i]-getSubMeshesCount()<<std::endl;

                    morphAnimData.offsetInFramesBuffer = currentFramesOffset;
                    morphAnimData.nbFrames = anims[i]->getFrames().size();
                    morphAnimData.intPerc = 0;
                    morphAnimData.intLevels = anims[i]->getIntLevels();
                    morphAnimData.passedTime = 0;
                    morphAnimData.currentFrameIndex = 0;
                    morphAnimsData.push_back(morphAnimData);
                    for (unsigned int j = 0; j < anims[i]->getFrames().size(); j++) {
                        FramesAnimData framesAnimData;
                        framesAnimData.submeshOffset = currentFrameSubmeshOffset;
                        //std::cout<<"current frame submesh offset  :"<<currentFrameSubmeshOffset<<std::endl;
                        framesAnimsData.push_back(framesAnimData);
                        for (unsigned int k = 0; k < anims[i]->getFrames()[j]->getSubMeshesCount(); k++) {
                            SubMeshData subMeshData;
                            subMeshData.id = currentFrameSubmeshOffset;
                            subMeshData.materialId = anims[i]->getFrames()[j]->getSubMeshes()[k].getMaterial().getId();
                            subMeshData.primitiveType = anims[i]->getFrames()[j]->getSubMeshes()[k].getVertexBuffer().getPrimitiveType();
                            //std::cout<<"material id = "<<subMeshData.materialId<<std::endl;
                            subMeshData.vertexOffset = currentVertexOffset;
                            subMeshData.nbVertices = anims[i]->getFrames()[j]->getSubMeshes()[k].getVertexBuffer().getVertexCount();
                            //verticesFramesAnims[anims[i]->getFrames()[j]->getSubMeshes()[k].getVertexBuffer().getPrimitiveType()].setPrimitiveType(anims[i]->getFrames()[j]->getSubMeshes()[k].getVertexBuffer().getPrimitiveType());
                            unsigned int vertexBase = currentVertexOffset;
                            for (unsigned int l = 0; l < anims[i]->getFrames()[j]->getSubMeshes()[k].getVertexBuffer().getVertexCount(); l++) {
                                verticesFramesAnims[anims[i]->getFrames()[j]->getSubMeshes()[k].getVertexBuffer().getPrimitiveType()].append(anims[i]->getFrames()[j]->getSubMeshes()[k].getVertexBuffer()[l]);
                                verticesFramesAnims[anims[i]->getFrames()[j]->getSubMeshes()[k].getVertexBuffer().getPrimitiveType()][vertexBase+l].drawableDataId = currentFrameSubmeshOffset;
                                currentVertexOffset++;
                            }
                            //std::cout<<"vertex offset : "<<subMeshData.vertexOffset<<std::endl;
                            framesSubMeshes.push_back(subMeshData);
                            currentFrameSubmeshOffset++;
                        }
                    }
                    currentFramesOffset += anims[i]->getFrames().size();
                }
                for (unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {
                    verticesFramesAnims[i].setPrimitiveType(static_cast<PrimitiveType>(i));
                    verticesFramesAnims[i].update(commandPool.getHandle(0));
                }
                staggingMorphAnims.create(sizeof(MorphAnimData) * morphAnimsData.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
                staggingMorphAnims.update(morphAnimsData.data(), sizeof(MorphAnimData) * morphAnimsData.size());
                morphAnims[0].create(sizeof(MorphAnimData) * morphAnimsData.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
                Buffer::copyBuffer(staggingMorphAnims, morphAnims[0], sizeof(MorphAnimData) * morphAnimsData.size(), commandPool.getHandle(0));
                staggingFramesAnims.create(sizeof(FramesAnimData) * framesAnimsData.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
                staggingFramesAnims.update(framesAnimsData.data(), sizeof(FramesAnimData) * framesAnimsData.size());
                framesAnims[0].create(sizeof(FramesAnimData) * framesAnimsData.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
                Buffer::copyBuffer(staggingFramesAnims, framesAnims[0], sizeof(FramesAnimData) * framesAnimsData.size(), commandPool.getHandle(0));
                staggingSubmeshesFramesAnims.create(sizeof(SubMeshData)*framesSubMeshes.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
                staggingSubmeshesFramesAnims.update(framesSubMeshes.data(), sizeof(SubMeshData)*framesSubMeshes.size());
                submeshesFramesAnims[0].create(sizeof(SubMeshData)*framesSubMeshes.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
                Buffer::copyBuffer(staggingSubmeshesFramesAnims, submeshesFramesAnims[0], sizeof(SubMeshData)*framesSubMeshes.size(), commandPool.getHandle(0));
                needToUpdateDescriptorSets = true;
                needToUpdateBuffers = false;
            }
        }
        void MorphAnimUpdater::setBuffersReady(bool r) {
            buffersReady.store(r);
        }
        bool MorphAnimUpdater::areBuffersReady() {
            return buffersReady.load();
        }
        void MorphAnimUpdater::setSubmitReady(bool r) {
            submitReady.store(r);
        }
        bool MorphAnimUpdater::isSubmitReady() {
            return submitReady.load();
        }
        void MorphAnimUpdater::setReady(bool r) {
            ready.store(r);
        }
        void MorphAnimUpdater::onUpdate() {
            std::unique_lock<std::mutex> lock(mtx);           
                //std::lock_guard<std::recursive_mutex> lock2(getGlobalMutex());
            cv.wait(lock, [this]() {
                return (ready.load() || !isRunning());
            });
            
            std::unique_lock<std::mutex> lock2(mtx2);

            //std::cout<<"update : "<<isRunning()<<std::endl;
            ready.store(false);
            if (isRunning()) {

                deltaTimePC.dt = getElapsedTime().asSeconds();
                restart();
                commandPool.beginRecordCommandBuffer(0);
                if (anims.size() > 0) {
                    updateBuffers();
                    if (needToUpdateDescriptorSets) {
                        //std::cout<<"update ds"<<std::endl;
                        updateDescriptorSets();
                        needToUpdateDescriptorSets = false;
                    }
                    vkCmdBindPipeline(commandPool.getHandle(0), VK_PIPELINE_BIND_POINT_COMPUTE, GPUContext::instance().getComputePipeline(updateAnimIndexShader).getHandle());
                    std::vector<VkDescriptorSet> sets;
                    for (unsigned int i = 0; i < GPUContext::instance().getDescriptorSets(updateAnimIndexShader).size(); i++) {
                        sets.push_back(GPUContext::instance().getDescriptorSets(updateAnimIndexShader)[i][0].getHandle());
                    }
                    vkCmdBindDescriptorSets(commandPool.getHandle(0), VK_PIPELINE_BIND_POINT_COMPUTE, GPUContext::instance().getComputePipeline(updateAnimIndexShader).getLayout(), 0, sets.size(), sets.data(), 0, 0);
                    vkCmdPushConstants(commandPool.getHandle(0), GPUContext::instance().getComputePipeline(updateAnimIndexShader).getLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(DeltaTimePC), &deltaTimePC);
                    VkMemoryBarrier barrier{};
                    barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

                    VkBufferMemoryBarrier buf{};
                    buf.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
                    buf.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    buf.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                    buf.buffer = morphAnims[0].getHandle();
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
                    vkCmdDispatch(commandPool.getHandle(0), anims.size(), 1, 1);
                    vkCmdBindPipeline(commandPool.getHandle(0), VK_PIPELINE_BIND_POINT_COMPUTE, GPUContext::instance().getComputePipeline(morphAnimShader).getHandle());
                    sets.clear();
                    for (unsigned int i = 0; i < GPUContext::instance().getDescriptorSets(morphAnimShader).size(); i++) {
                        sets.push_back(GPUContext::instance().getDescriptorSets(morphAnimShader)[i][0].getHandle());
                    }
                    vkCmdBindDescriptorSets(commandPool.getHandle(0), VK_PIPELINE_BIND_POINT_COMPUTE, GPUContext::instance().getComputePipeline(morphAnimShader).getLayout(), 0, sets.size(), sets.data(), 0, 0);
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
                    //std::cout<<"total frames vertices : "<<framesTotalVertices<<std::endl;
                    for (unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {
                        if (verticesFramesAnims[i].getVertexCount() > 0) {
                            //std::cout<<"vertex count : "<<verticesFramesAnims[i].getVertexCount()<<std::endl;
                            vkCmdDispatch(commandPool.getHandle(0), anims.size(), verticesFramesAnims[i].getVertexCount(), 1);
                        }
                    }
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
                if (vkQueueSubmit(GPUContext::instance().getDevice().getQueue(indices.graphicsFamily.value(), 1), 1, &submitInfo, GPUContext::instance().getSharedFence(1)[0].getHandle()) != VK_SUCCESS) {
                    throw std::runtime_error("Echec de l'envoi d'un command buffer!");
                }
                //std::cout<<"buffers ready ma!"<<std::endl;
                submitReady.store(true);
                cv2.notify_all();
                //GPUContext::instance().getSharedFence(0)[0].waitForFences(VK_TRUE, UINT64_MAX);
            }

        }
    }
}
