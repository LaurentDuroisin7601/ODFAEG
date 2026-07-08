module;
#include <vector>
#include <vulkan/vulkan.hpp>
#include <iostream>
#include <meshoptimizer.h>
#include "odfaeg/config.hpp"
#include "vk_mem_alloc.h"
//import odfaeg.graphic.vertexBuffer;
module odfaeg.graphic.vertexBuffer;
import odfaeg.math.vec;
import odfaeg.graphic.gpuContext;
import odfaeg.graphic.renderTarget;
import odfaeg.graphic.renderStates;
namespace odfaeg {
    namespace graphic {        
        VertexBuffer::VertexBuffer(Device& device, unsigned int nbBuffers)  : /*Drawable(), */ device(device), nbBuffers(nbBuffers), m_primitiveType(Points),
            needToUpdateVertexBuffer(nbBuffers, true), needToUpdateIndexBuffer(nbBuffers, true),
            maxVerticesSize(nbBuffers, 0), maxIndexSize(nbBuffers, 0),
            commandPool(device) {
            commandBuffersCreated = false;
            vertexBuffer.reserve(nbBuffers);
            indexBuffer.reserve(nbBuffers);
            vertexStaggingBuffer.reserve(nbBuffers);
            indexStaggingBuffer.reserve(nbBuffers);
            for (unsigned int i = 0; i < nbBuffers; i++) {
                vertexBuffer.emplace_back(device);
                //vertexBuffer[i].create(sizeof(Vertex), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
                indexBuffer.emplace_back(device);
                //indexBuffer[i].create(sizeof(unsigned int), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
                vertexStaggingBuffer.emplace_back(device);
                indexStaggingBuffer.emplace_back(device);
            }
            /*Device::QueueFamilyIndices queueFamilyIndices = GPUContext::instance().getDevice().findQueueFamilies(GPUContext::instance().getDevice().getPhysicalDevice());
            commandPool.create(queueFamilyIndices.graphicsFamily.value());
            commandPool.createCommandBuffers(true, nbBuffers);*/
        }
        VertexBuffer::VertexBuffer(Device& device, PrimitiveType primitiveType, unsigned int nbBuffers) : device(device), m_primitiveType(primitiveType), nbBuffers(nbBuffers),
            needToUpdateVertexBuffer(nbBuffers, true), needToUpdateIndexBuffer(nbBuffers, true),
            maxVerticesSize(nbBuffers, 0), maxIndexSize(nbBuffers, 0),
            commandPool(device) {
            commandBuffersCreated = false;
            vertexBuffer.reserve(nbBuffers);
            indexBuffer.reserve(nbBuffers);
            vertexStaggingBuffer.reserve(nbBuffers);
            indexStaggingBuffer.reserve(nbBuffers);
            //std::cout<<"set primitive type : "<<m_primitiveType<<std::endl;
            for (unsigned int i = 0; i < nbBuffers; i++) {
                vertexBuffer.emplace_back(device);
                //vertexBuffer[i].create(sizeof(Vertex), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
                indexBuffer.emplace_back(device);
                //indexBuffer[i].create(sizeof(unsigned int), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
                vertexStaggingBuffer.emplace_back(device);
                indexStaggingBuffer.emplace_back(device);
            }
            /*Device::QueueFamilyIndices queueFamilyIndices = GPUContext::instance().getDevice().findQueueFamilies(GPUContext::instance().getDevice().getPhysicalDevice());
            commandPool.create(queueFamilyIndices.graphicsFamily.value());
            commandPool.createCommandBuffers(true, nbBuffers);*/
        }
        VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept : device(other.device), nbBuffers(other.nbBuffers), commandPool(other.device) {
            vertexBuffer = std::move(other.vertexBuffer);
            indexBuffer = std::move(other.indexBuffer);
            indices = other.indices;
            m_vertices = other.m_vertices;
            m_primitiveType = other.m_primitiveType;
        }
        unsigned int VertexBuffer::getNbBuffers() const {
            return nbBuffers;
        }
        VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept {
            if (this != &other) {
                nbBuffers = other.nbBuffers;
                vertexBuffer = std::move(other.vertexBuffer);
                indexBuffer = std::move(other.indexBuffer);
                indices = other.indices;
                m_vertices = other.m_vertices;
                m_primitiveType = other.m_primitiveType;
                commandPool = std::move(other.commandPool);
            }
            return *this;
        }
        void VertexBuffer::copyFrom(VertexBuffer& vertexBuffer) {
            m_vertices = vertexBuffer.m_vertices;
            indices = vertexBuffer.indices;
            m_primitiveType = vertexBuffer.m_primitiveType;
            nbBuffers = vertexBuffer.nbBuffers;
            /*for (unsigned int i = 0; i < nbBuffers; i++) {
                this->vertexBuffer.emplace_back(device);
                this->vertexBuffer[i].create(sizeof(Vertex), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
                indexBuffer.emplace_back(device);
                indexBuffer[i].create(sizeof(unsigned int), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
                vertexStaggingBuffer.emplace_back(device);
                indexStaggingBuffer.emplace_back(device);
            }*/
            for (unsigned int i = 0; i < nbBuffers; i++) {
                needToUpdateVertexBuffer[i] = true;
                needToUpdateIndexBuffer[i] = true;
            }
        }
        unsigned int VertexBuffer::getIndex(unsigned int i) {
            return indices[i];
        }
        Device& VertexBuffer::getDevice() {
            return device;
        }
        void VertexBuffer::resize(unsigned int size, unsigned int indexSize) {
            m_vertices.resize(size);
            indices.resize(indexSize);
        }
        void VertexBuffer::swap(VertexBuffer& vb) {
            std::swap(m_vertices, vb.m_vertices);
            std::swap(indices, vb.indices);
            std::swap(m_primitiveType, vb.m_primitiveType);

            for (unsigned int i = 0; i < nbBuffers; i++) {
                std::swap(vertexBuffer[i], vb.vertexBuffer[i]);
                std::swap(indexBuffer[i], vb.indexBuffer[i]);
            }
        }
        physic::BoundingBox VertexBuffer::getBounds()
        {
            if (!m_vertices.empty())
            {
                float left = m_vertices[0].position.x();
                float top = m_vertices[0].position.y();
                float right = m_vertices[0].position.x();
                float bottom = m_vertices[0].position.y();
                float nearest = m_vertices[0].position.z();
                float farest = m_vertices[0].position.z();

                for (std::size_t i = 1; i < m_vertices.size(); ++i)
                {
                    math::Vec3f position = m_vertices[i].position;

                    // Update left and right
                    if (position.x() < left)
                        left = position.x();
                    else if (position.x() > right)
                        right = position.x();

                    // Update top and bottom
                    if (position.y() < top)
                        top = position.y();
                    else if (position.y() > bottom)
                        bottom = position.y();

                    //Update the near and the far.

                    if (position.z() > farest)
                        farest = position.z();
                    else if (position.z() < nearest)
                        nearest = position.z();

                }

                return physic::BoundingBox(left, top, nearest, right - left, bottom - top, farest - nearest);
            }
            else
            {
                // Array is empty
                return physic::BoundingBox();
            }
        }
        physic::BoundingBox VertexBuffer::getGlobalBounds(entity::TransformMatrix& tm)
        {
            if (!m_vertices.empty())
            {
                math::Vec3f v = tm.transform(m_vertices[0].position);
                float left = v.x();
                float top = v.y();
                float right = v.x();
                float bottom = v.y();
                float nearest = v.z();
                float farest = v.z();

                for (std::size_t i = 1; i < m_vertices.size(); ++i)
                {
                    math::Vec3f position = tm.transform(m_vertices[i].position);

                    // Update left and right
                    if (position.x() < left)
                        left = position.x();
                    else if (position.x() > right)
                        right = position.x();

                    // Update top and bottom
                    if (position.y() < top)
                        top = position.y();
                    else if (position.y() > bottom)
                        bottom = position.y();

                    //Update the near and the far.

                    if (position.z() > farest)
                        farest = position.z();
                    else if (position.z() < nearest)
                        nearest = position.z();

                }

                return physic::BoundingBox(left, top, nearest, right - left, bottom - top, farest - nearest);
            }
            else
            {
                // Array is empty
                return physic::BoundingBox();
            }
        }
        bool VertexBuffer::operator==(const VertexBuffer& other) {
            if (getVertexCount() != other.getVertexCount())
                return false;
            if (m_primitiveType != other.m_primitiveType)
                return false;
            for (unsigned int i = 0; i < getVertexCount(); i++)
                if (!(m_vertices[i] == other.m_vertices[i]))
                    return false;
            return true;
        }

        void VertexBuffer::clear() {
            m_vertices.clear();
            indices.clear();
        }
        void VertexBuffer::addIndex(uint32_t index) {
            indices.push_back(index);
            for (unsigned int i = 0; i < nbBuffers; i++)
                needToUpdateIndexBuffer[i] = true;
        }
        void VertexBuffer::append(const Vertex& vertex) {
            m_vertices.push_back(vertex);
            for (unsigned int i = 0; i < nbBuffers; i++)
                needToUpdateVertexBuffer[i] = true;
        }        
        Buffer& VertexBuffer::getVertexBuffer(unsigned int currentFrame) {
            return vertexBuffer[currentFrame];
        }
        Buffer& VertexBuffer::getIndexBuffer(unsigned int currentFrame) {
            return indexBuffer[currentFrame];
        }
        Buffer& VertexBuffer::getStaggingVertexBuffer(unsigned int currentFrame) {
            return vertexStaggingBuffer[currentFrame];
        }
        Buffer& VertexBuffer::getStaggingIndexBuffer(unsigned int currentFrame) {
            return indexStaggingBuffer[currentFrame];
        }
        size_t VertexBuffer::getVertexCount() const {
            return m_vertices.size();
        }
        size_t VertexBuffer::getIndexCount() const {
            return indices.size();
        }        
        ////////////////////////////////////////////////////////////
        void VertexBuffer::setPrimitiveType(PrimitiveType type)
        {
            m_primitiveType = type;
        }


        ////////////////////////////////////////////////////////////
        PrimitiveType VertexBuffer::getPrimitiveType() const
        {
            return m_primitiveType;
        }
        void VertexBuffer::update(VkCommandBuffer& commandBuffer, unsigned int currentFrame) {
            //commandPool.beginRecordCommandBuffer(currentFrame);
            //std::cout<<"primitive type : "<<m_primitiveType<<std::endl;

            VkDeviceSize bufferSize = (m_vertices .size() == 0) ? sizeof(Vertex) : sizeof(Vertex) * m_vertices.size();
            //std::cout<<"buffer size : "<<sizeof(Vertex)<<","<<m_vertices.size()<<std::endl;
            if (needToUpdateVertexBuffer[currentFrame]) {
                if (bufferSize > maxVerticesSize[currentFrame]) {
                    //std::cout<<"update!"<<std::endl;
                    vertexStaggingBuffer[currentFrame].create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
                    vertexBuffer[currentFrame].create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
                    maxVerticesSize[currentFrame] = bufferSize;
                }
                /*std::vector<VulkanVertex> vulkanVertices;
                vulkanVertices.resize(getVertexCount());
                for (unsigned int i = 0; i < vulkanVertices.size(); i++) {
                    VulkanVertex vertex;
                    vertex.position.x = m_vertices[i].position.x();
                    vertex.position.y = m_vertices[i].position.y();
                    vertex.position.z = m_vertices[i].position.z();
                    vertex.color = m_vertices[i].color;
                    vertex.texCoords.x = m_vertices[i].texCoords.x();
                    vertex.texCoords.y = m_vertices[i].texCoords.y();
                    vertex.normal.x = m_vertices[i].normal.x();
                    vertex.normal.y = m_vertices[i].normal.y();
                    vertex.normal.z = m_vertices[i].normal.z();
                    for (unsigned int j = 0; j < MAX_BONES_INFLUENCE; j++) {
                        vertex.m_BoneIDs[j] = m_vertices[i].m_BoneIDs[j];
                        vertex.m_Weights[j] = m_vertices[i].m_Weights[j];
                    }
                    //std::cout<<"cpu text coords : "<<vertex.texCoords.x<<","<<vertex.texCoords.y<<std::endl;
                    vulkanVertices[i] = vertex;
                }*/
                needToUpdateVertexBuffer[currentFrame] = false;
            }
            //std::cout<<"update vertex buffer!"<<std::endl;
            if (m_vertices.size() > 0) {
                vertexStaggingBuffer[currentFrame].update(m_vertices.data(), (size_t)bufferSize);
                Buffer::copyBuffer(vertexStaggingBuffer[currentFrame], vertexBuffer[currentFrame], bufferSize, commandBuffer);
            }
            bufferSize = (indices.size() == 0) ? sizeof(std::uint32_t) : sizeof(std::uint32_t) * indices.size();
            if (needToUpdateIndexBuffer[currentFrame]) {

                if (bufferSize > maxIndexSize[currentFrame]) {
                    //std::cout<<"update index buffer!"<<std::endl;
                    indexStaggingBuffer[currentFrame].create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
                    indexBuffer[currentFrame].create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
                    maxIndexSize[currentFrame] = bufferSize;
                }
                needToUpdateIndexBuffer[currentFrame] = false;
                ////////std::cout<<"index buffer : "<<indexBuffer<<std::endl;
            }
            if (indices.size() > 0) {
                indexStaggingBuffer[currentFrame].update(indices.data(), (size_t)bufferSize);
                Buffer::copyBuffer(indexStaggingBuffer[currentFrame], indexBuffer[currentFrame], bufferSize, commandBuffer);
            }
            /*commandPool.endRecordCommandBuffer(currentFrame);
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandPool.getHandle(currentFrame);
            Device::QueueFamilyIndices indices = device.findQueueFamilies(device.getPhysicalDevice(), VK_NULL_HANDLE);
            if (vkQueueSubmit(device.getQueue(indices.graphicsFamily.value(), 0), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
                throw std::runtime_error("�chec de l'envoi d'un command buffer!");
            }
            vkDeviceWaitIdle(device.getDevice());*/
        }
        void VertexBuffer::update(unsigned int currentFrame) {
            //std::cout<<"update vb!"<<std::endl;
            if(!commandBuffersCreated) {
                Device::QueueFamilyIndices queueFamilyIndices = GPUContext::instance().getDevice().findQueueFamilies(GPUContext::instance().getDevice().getPhysicalDevice());
                commandPool.create(queueFamilyIndices.graphicsFamily.value());
                commandPool.createCommandBuffers(true, nbBuffers);
                commandBuffersCreated = true;
            }
            //std::cout<<"primitive type : "<<m_primitiveType<<std::endl;
            commandPool.beginRecordCommandBuffer(currentFrame);
            VkDeviceSize bufferSize = (m_vertices.size() == 0) ? sizeof(Vertex) : sizeof(Vertex) * m_vertices.size();
            //std::cout<<"buffer size : "<<bufferSize<<std::endl;
            if (needToUpdateVertexBuffer[currentFrame]) {
                if (bufferSize > maxVerticesSize[currentFrame]) {
                    //std::cout<<"create vb"<<std::endl;
                    vertexStaggingBuffer[currentFrame].create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
                    vertexBuffer[currentFrame].create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
                    maxVerticesSize[currentFrame] = bufferSize;
                }
                /*std::vector<VulkanVertex> vulkanVertices;
                vulkanVertices.resize(getVertexCount());
                for (unsigned int i = 0; i < vulkanVertices.size(); i++) {
                    VulkanVertex vertex;
                    vertex.position.x = m_vertices[i].position.x();
                    vertex.position.y = m_vertices[i].position.y();
                    vertex.position.z = m_vertices[i].position.z();
                    vertex.color = m_vertices[i].color;
                    vertex.texCoords.x = m_vertices[i].texCoords.x();
                    vertex.texCoords.y = m_vertices[i].texCoords.y();
                    vertex.normal.x = m_vertices[i].normal.x();
                    vertex.normal.y = m_vertices[i].normal.y();
                    vertex.normal.z = m_vertices[i].normal.z();
                    for (unsigned int j = 0; j < MAX_BONES_INFLUENCE; j++) {
                        vertex.m_BoneIDs[j] = m_vertices[i].m_BoneIDs[j];
                        vertex.m_Weights[j] = m_vertices[i].m_Weights[j];
                    }
                    //std::cout<<"cpu text coords : "<<vertex.texCoords.x<<","<<vertex.texCoords.y<<std::endl;
                    vulkanVertices[i] = vertex;
                }*/
                needToUpdateVertexBuffer[currentFrame] = false;
            }
            //std::cout<<"update vertex buffer!"<<std::endl;
            if (m_vertices.size() > 0) {
                vertexStaggingBuffer[currentFrame].update(m_vertices.data(), (size_t)bufferSize);
                Buffer::copyBuffer(vertexStaggingBuffer[currentFrame], vertexBuffer[currentFrame], bufferSize, commandPool.getHandle(currentFrame));
            }
            bufferSize = (indices.size() == 0) ? sizeof(std::uint32_t) : sizeof(std::uint32_t) * indices.size();
            //std::cout<<"buffer index size : "<<bufferSize<<std::endl;
            if (needToUpdateIndexBuffer[currentFrame]) {

                if (bufferSize > maxIndexSize[currentFrame]) {
                    //std::cout<<"update index buffer!"<<std::endl;
                    indexStaggingBuffer[currentFrame].create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
                    indexBuffer[currentFrame].create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
                    maxIndexSize[currentFrame] = bufferSize;
                }
                needToUpdateIndexBuffer[currentFrame] = false;
                ////////std::cout<<"index buffer : "<<indexBuffer<<std::endl;
            }
            if (indices.size() > 0) {
                indexStaggingBuffer[currentFrame].update(indices.data(), (size_t)bufferSize);
                Buffer::copyBuffer(indexStaggingBuffer[currentFrame], indexBuffer[currentFrame], bufferSize, commandPool.getHandle(currentFrame));
            }
            commandPool.endRecordCommandBuffer(currentFrame);
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandPool.getHandle(currentFrame);
            Device::QueueFamilyIndices indices = device.findQueueFamilies(device.getPhysicalDevice(), VK_NULL_HANDLE);
            if (vkQueueSubmit(device.getQueue(indices.graphicsFamily.value(), 0), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
                throw std::runtime_error("�chec de l'envoi d'un command buffer!");
            }
            vkDeviceWaitIdle(device.getDevice());
        }
        void VertexBuffer::updateLods() {
            float ratios[] = {1.0f, 0.5f, 0.25f, 0.12f, 0.06f};
            std::vector<uint32_t> current = indices; // copie
            indices.clear();
            unsigned int level = 0;

            for (float r : ratios)
            {
                std::vector<unsigned int> lod(current.size());
                size_t count = meshopt_simplify(
                lod.data(),
                current.data(),
                current.size(),
                reinterpret_cast<const float*>(m_vertices.data()),
                m_vertices.size(),
                sizeof(Vertex),
                r,
                1e-2f);
                lod.resize(count);
                LODLevel lodLevel;
                lodLevel.indexOffset = indices.size();
                lodLevel.indexCount = lod.size();
                lods[level] = lodLevel;
                indices.insert(indices.end(), lod.begin(), lod.end());
                current = lod;
                /*std::cout<<"indices : "<<indices.size()<<std::endl;
                std::cout<<"level : "<<lods[level].indexOffset<<","<<lods[level].indexCount<<std::endl;*/
                level++;
            }
        }
        std::array<VertexBuffer::LODLevel, 5> VertexBuffer::getLODs() const {
            return lods;
        }
        Vertex& VertexBuffer::operator [](unsigned int index)
        {
            for (unsigned int i = 0; i < nbBuffers; i++)
                needToUpdateVertexBuffer[i] = true;
            return m_vertices[index];
        }
    }	
}
