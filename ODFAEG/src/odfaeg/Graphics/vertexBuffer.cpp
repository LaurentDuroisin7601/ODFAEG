
#include "../../../include/odfaeg/Graphics/vertexBuffer.hpp"
#include "../../../include/odfaeg/Graphics/renderTarget.h"
#ifndef VULKAN
#include "GL/glew.h"
#include <SFML/OpenGL.hpp>
#include "glCheck.h"
#endif
#include <string.h>
namespace odfaeg {
    namespace graphic {
        using namespace sf;
        #ifdef VULKAN
            VertexBuffer::VertexBuffer(window::Device& vkDevice) : vkDevice(vkDevice),
            needToUpdateVertexBuffer(false), needToUpdateIndexBuffer(false), vertexBuffer(VK_NULL_HANDLE), indexBuffer(VK_NULL_HANDLE) {
                createCommandPool();
            }
            /*VertexBuffer::VertexBuffer(const VertexBuffer& vb) : vkDevice(vb.vkDevice) {
                createCommandPool();
                m_vertices = vb.m_vertices;
                indices = vb.indices;
                needToUpdateVertexBuffer = (m_vertices.size() > 0) ? true : false;
                needToUpdateIndexBuffer = (indices.size() > 0) ? true : false;
                update();
            }*/
            VertexBuffer::VertexBuffer(VertexBuffer&& vb) : vkDevice(vb.vkDevice)
            , vertexBuffer(std::exchange(vb.vertexBuffer, nullptr))
            , indexBuffer(std::exchange(vb.indexBuffer, nullptr))
            , m_vertices(std::move(vb.m_vertices))
            , indices(std::move(vb.indices))
            , needToUpdateVertexBuffer((m_vertices.size() > 0) ? true : false)
            , needToUpdateIndexBuffer((indices.size() > 0) ? true : false) {
                createCommandPool();
                update();
            }
            /*VertexBuffer& VertexBuffer::operator=(const VertexBuffer& vb) {
                commandPool = vb.commandPool;
                m_vertices = vb.m_vertices;
                indices = vb.indices;
                needToUpdateVertexBuffer = (m_vertices.size() > 0) ? true : false;
                needToUpdateIndexBuffer = (indices.size() > 0) ? true : false;
                update();
                return *this;
            }*/
            VertexBuffer& VertexBuffer::operator=(VertexBuffer&& vb) {

                commandPool = vb.commandPool;
                m_vertices = std::move(vb.m_vertices);
                indices = std::move(vb.indices);
                vertexBuffer = std::exchange(vertexBuffer, nullptr);
                indexBuffer = std::exchange(indexBuffer, nullptr);
                needToUpdateIndexBuffer = std::move(vb.needToUpdateVertexBuffer);
                needToUpdateVertexBuffer = std::move(vb.needToUpdateVertexBuffer);
                update();
                return *this;
            }
            void VertexBuffer::clear() {
                m_vertices.clear();
            }
            void VertexBuffer::clearIndexes() {
                indices.clear();
            }
            void VertexBuffer::addIndex(uint16_t index) {
                indices.push_back(index);
                needToUpdateIndexBuffer = true;
            }
            void VertexBuffer::append(const Vertex& vertex) {
                m_vertices.push_back(vertex);
                needToUpdateVertexBuffer = true;
            }
            void VertexBuffer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
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
            void VertexBuffer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
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
            uint32_t VertexBuffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
                VkPhysicalDeviceMemoryProperties memProperties;
                vkGetPhysicalDeviceMemoryProperties(vkDevice.getPhysicalDevice(), &memProperties);
                for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
                    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                        return i;
                    }
                }
                throw std::runtime_error("aucun type de memoire ne satisfait le buffer!");
            }
            VkBuffer VertexBuffer::getVertexBuffer() {
                return vertexBuffer;
            }
            size_t VertexBuffer::getSize() {
                return m_vertices.size();
            }
            size_t VertexBuffer::getVertexCount() {
                return m_vertices.size();
            }
            size_t VertexBuffer::getIndicesSize() {
                return indices.size();
            }
            VkBuffer VertexBuffer::getIndexBuffer() {
                return indexBuffer;
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
            void VertexBuffer::update() {


                if (needToUpdateVertexBuffer) {
                    if (vertexBuffer != VK_NULL_HANDLE) {
                        vkDestroyBuffer(vkDevice.getDevice(), vertexBuffer, nullptr);
                        vkFreeMemory(vkDevice.getDevice(), vertexBufferMemory, nullptr);
                    }
                    VkDeviceSize bufferSize = sizeof(Vertex) * m_vertices.size();

                    VkBuffer stagingBuffer;
                    VkDeviceMemory stagingBufferMemory;
                    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

                    void* data;
                    vkMapMemory(vkDevice.getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
                        memcpy(data, m_vertices.data(), (size_t) bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), stagingBufferMemory);

                    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

                    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

                    vkDestroyBuffer(vkDevice.getDevice(), stagingBuffer, nullptr);
                    vkFreeMemory(vkDevice.getDevice(), stagingBufferMemory, nullptr);
                    needToUpdateVertexBuffer = false;
                    //std::cout<<"vertex buffer : "<<indexBuffer<<std::endl;
                }
                if (needToUpdateIndexBuffer) {
                    if (indexBuffer != VK_NULL_HANDLE) {
                        vkDestroyBuffer(vkDevice.getDevice(), indexBuffer, nullptr);
                        vkFreeMemory(vkDevice.getDevice(), indexBufferMemory, nullptr);
                    }
                    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

                    VkBuffer stagingBuffer;
                    VkDeviceMemory stagingBufferMemory;
                    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

                    void* data;
                    vkMapMemory(vkDevice.getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, indices.data(), (size_t) bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), stagingBufferMemory);

                    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

                    copyBuffer(stagingBuffer, indexBuffer, bufferSize);

                    vkDestroyBuffer(vkDevice.getDevice(), stagingBuffer, nullptr);
                    vkFreeMemory(vkDevice.getDevice(), stagingBufferMemory, nullptr);
                    needToUpdateIndexBuffer  = false;
                    //std::cout<<"index buffer : "<<indexBuffer<<std::endl;
                }

            }
            Vertex& VertexBuffer::operator [](unsigned int index)
            {
                return m_vertices[index];
            }
            void VertexBuffer::createCommandPool() {

                window::Device::QueueFamilyIndices queueFamilyIndices = vkDevice.findQueueFamilies(vkDevice.getPhysicalDevice(), VK_NULL_HANDLE);

                VkCommandPoolCreateInfo poolInfo{};
                poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
                poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optionel
                if (vkCreateCommandPool(vkDevice.getDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
                    throw core::Erreur(0, "échec de la création d'une command pool!", 1);
                }
            }
            VertexBuffer::~VertexBuffer() {
                vkDestroyCommandPool(vkDevice.getDevice(), commandPool, nullptr);
                if (vertexBuffer != VK_NULL_HANDLE) {
                    vkDestroyBuffer(vkDevice.getDevice(), vertexBuffer, nullptr);
                    vkFreeMemory(vkDevice.getDevice(), vertexBufferMemory, nullptr);
                }
                if (indexBuffer != VK_NULL_HANDLE) {
                    vkDestroyBuffer(vkDevice.getDevice(), indexBuffer, nullptr);
                    vkFreeMemory(vkDevice.getDevice(), indexBufferMemory, nullptr);
                }
            }
        #else
        ////////////////////////////////////////////////////////////
        VertexBuffer::VertexBuffer() :
        m_vertices     (),
        m_primitiveType(Points),
        m_entity(nullptr)
        {
            vboVertexBuffer = 0;
            vboNormalBuffer = 0;
            vboIndexBuffer = 0;
            vboTextureIndexesBuffer = 0;
            vboMaterialType = 0;
            vboLayer = 0;
            vboSpecular = 0;
            vboLightInfos = 0;
            needToUpdateVertexBuffer = false;
            needToUpdateIndexBuffer = false;
            needToUpdateLayersBuffer  = false;
            needToUpdateMaterialTypeBuffer = false;
            needToUpdateSpecularBuffer = false;
            needToUpdateLightInfos = false;
            needToUpdateNormals = false;
            nbVerticesPerFace = 4;
            loop = true;
            oldVerticesSize = 0;
            oldIndexesSize = 0;
            oldMaterialTypeSize = 0;
            oldLayerSize = 0;
            oldSpecularSize = 0;
            oldLightInfosSize = 0;
        }
        VertexBuffer::VertexBuffer(const VertexBuffer& va) {
            vboVertexBuffer = 0;
            vboNormalBuffer = 0;
            vboIndexBuffer = 0;
            vboTextureIndexesBuffer = 0;
            vboMaterialType = 0;
            vboLayer = 0;
            vboSpecular = 0;
            vboLightInfos = 0;
            m_entity = va.m_entity;
            m_normals = va.m_normals;
            m_locals = va.m_locals;
            m_vertices = va.m_vertices;
            m_layers = va.m_layers;
            m_specular = va.m_specular;
            m_primitiveType = va.m_primitiveType;
            oldVerticesSize = 0;
            oldIndexesSize = 0;
            oldMaterialTypeSize = 0;
            oldLayerSize = 0;
            oldSpecularSize = 0;
            oldLightInfosSize = 0;
            m_numIndexes = va.m_numIndexes;
            m_baseIndexes = va.m_baseIndexes;
            m_baseVertices = va.m_baseVertices;
            m_indexes = va.m_indexes;
            m_layers = va.m_layers;
            m_lightInfos = va.m_lightInfos;
            loop = va.loop;
            m_vPosX = va.m_vPosX;
            m_vPosY = va.m_vPosY;
            m_vPosZ = va.m_vPosZ;
            m_vPosW = va.m_vPosW;
            m_vcRed = va.m_vcRed;
            m_vcGreen = va.m_vcGreen;
            m_vcBlue = va.m_vcBlue;
            m_vcAlpha = va.m_vcAlpha;
            m_ctX = va.m_ctX;
            m_ctY = va.m_ctY;
            nbVerticesPerFace = va.nbVerticesPerFace;
            needToUpdateVertexBuffer = true;
            needToUpdateIndexBuffer = true;
            needToUpdateMaterialTypeBuffer = true;
            needToUpdateLayersBuffer = true;
            needToUpdateSpecularBuffer = true;
            needToUpdateLightInfos = true;
            needToUpdateNormals = true;
        }
        VertexBuffer::VertexBuffer(const VertexBuffer&& va) {
            vboVertexBuffer = 0;
            vboNormalBuffer = 0;
            vboIndexBuffer = 0;
            vboTextureIndexesBuffer = 0;
            vboMaterialType = 0;
            vboLayer = 0;
            vboSpecular = 0;
            vboLightInfos = 0;
            m_entity = va.m_entity;
            m_normals = va.m_normals;
            m_locals = va.m_locals;
            m_vertices = va.m_vertices;
            m_primitiveType = va.m_primitiveType;
            m_layers = va.m_layers;
            m_specular = va.m_specular;
            oldVerticesSize = 0;
            oldIndexesSize = 0;
            oldMaterialTypeSize = 0;
            oldLayerSize = 0;
            oldSpecularSize = 0;
            oldLightInfosSize = 0;
            m_numIndexes = va.m_numIndexes;
            m_baseIndexes = va.m_baseIndexes;
            m_baseVertices = va.m_baseVertices;
            m_indexes = va.m_indexes;
            m_lightInfos = va.m_lightInfos;
            loop = va.loop;
            m_vPosX = va.m_vPosX;
            m_vPosY = va.m_vPosY;
            m_vPosZ = va.m_vPosZ;
            m_vPosW = va.m_vPosW;
            m_vcRed = va.m_vcRed;
            m_vcGreen = va.m_vcGreen;
            m_vcBlue = va.m_vcBlue;
            m_vcAlpha = va.m_vcAlpha;
            m_ctX = va.m_ctX;
            m_ctY = va.m_ctY;
            nbVerticesPerFace = va.nbVerticesPerFace;
            needToUpdateVertexBuffer = true;
            needToUpdateIndexBuffer = true;
            needToUpdateMaterialTypeBuffer = true;
            needToUpdateLayersBuffer = true;
            needToUpdateSpecularBuffer = true;
            needToUpdateLightInfos = true;
            needToUpdateNormals = true;
        }
        VertexBuffer& VertexBuffer::operator= (const VertexBuffer& va) {
            vboVertexBuffer = 0;
            vboNormalBuffer = 0;
            vboIndexBuffer = 0;
            vboTextureIndexesBuffer = 0;
            vboMaterialType = 0;
            vboLayer = 0;
            vboSpecular = 0;
            vboLightInfos = 0;
            m_entity = va.m_entity;
            m_normals = va.m_normals;
            m_locals = va.m_locals;
            m_vertices = va.m_vertices;
            m_layers = va.m_layers;
            m_specular = va.m_specular;
            m_primitiveType = va.m_primitiveType;
            oldVerticesSize = 0;
            oldIndexesSize = 0;
            oldMaterialTypeSize = 0;
            oldLayerSize = 0;
            oldSpecularSize = 0;
            oldLightInfosSize = 0;
            m_numIndexes = va.m_numIndexes;
            m_baseIndexes = va.m_baseIndexes;
            m_baseVertices = va.m_baseVertices;
            m_indexes = va.m_indexes;
            m_lightInfos = va.m_lightInfos;
            loop = va.loop;
            m_vPosX = va.m_vPosX;
            m_vPosY = va.m_vPosY;
            m_vPosZ = va.m_vPosZ;
            m_vPosW = va.m_vPosW;
            m_vcRed = va.m_vcRed;
            m_vcGreen = va.m_vcGreen;
            m_vcBlue = va.m_vcBlue;
            m_vcAlpha = va.m_vcAlpha;
            m_ctX = va.m_ctX;
            m_ctY = va.m_ctY;
            nbVerticesPerFace = va.nbVerticesPerFace;
            needToUpdateVertexBuffer = true;
            needToUpdateIndexBuffer = true;
            needToUpdateMaterialTypeBuffer = true;
            needToUpdateLayersBuffer = true;
            needToUpdateSpecularBuffer = true;
            needToUpdateLightInfos = true;
            needToUpdateNormals = true;
            return *this;
        }
        VertexBuffer& VertexBuffer::operator= (const VertexBuffer&& va) {
            vboVertexBuffer = 0;
            vboNormalBuffer = 0;
            vboIndexBuffer = 0;
            vboTextureIndexesBuffer = 0;
            vboMaterialType = 0;
            vboLayer = 0;
            vboSpecular = 0;
            vboLightInfos = 0;
            m_entity = va.m_entity;
            m_normals = va.m_normals;
            m_locals = va.m_locals;
            m_vertices = va.m_vertices;
            m_layers = va.m_layers;
            m_specular = va.m_specular;
            m_primitiveType = va.m_primitiveType;
            oldVerticesSize = 0;
            oldIndexesSize = 0;
            oldMaterialTypeSize = 0;
            oldLayerSize = 0;
            oldSpecularSize = 0;
            oldLightInfosSize = 0;
            m_numIndexes = va.m_numIndexes;
            m_baseIndexes = va.m_baseIndexes;
            m_baseVertices = va.m_baseVertices;
            m_indexes = va.m_indexes;
            m_lightInfos = va.m_lightInfos;
            loop = va.loop;
            m_vPosX = va.m_vPosX;
            m_vPosY = va.m_vPosY;
            m_vPosZ = va.m_vPosZ;
            m_vPosW = va.m_vPosW;
            m_vcRed = va.m_vcRed;
            m_vcGreen = va.m_vcGreen;
            m_vcBlue = va.m_vcBlue;
            m_vcAlpha = va.m_vcAlpha;
            m_ctX = va.m_ctX;
            m_ctY = va.m_ctY;
            nbVerticesPerFace = va.nbVerticesPerFace;
            needToUpdateVertexBuffer = true;
            needToUpdateIndexBuffer = true;
            needToUpdateMaterialTypeBuffer = true;
            needToUpdateLayersBuffer = true;
            needToUpdateSpecularBuffer = true;
            needToUpdateLightInfos = true;
            needToUpdateNormals = true;
            return *this;
        }
        void VertexBuffer::addNormal(sf::Vector3f normal) {
            m_normals.push_back(normal);
        }
        void VertexBuffer::computeNormals() {
            if (needToUpdateNormals) {
                unsigned int size = 0;
                if (m_primitiveType == sf::PrimitiveType::Quads) {
                    size = m_vertices.size() / 4;
                } else if (m_primitiveType == sf::PrimitiveType::Triangles) {
                    size = m_vertices.size() / 3;
                } else if (m_primitiveType == sf::PrimitiveType::TriangleStrip || m_primitiveType == sf::PrimitiveType::TriangleFan) {
                    size = (m_vertices.size() > 2) ? m_vertices.size() - 2 : 0;
                }
                m_normals.resize(m_vertices.size());
                for (unsigned int i = 0; i < size; i++) {
                    if (m_primitiveType == sf::PrimitiveType::Quads) {
                        for (unsigned int n = 0; n < 4; n++) {
                            math::Vec3f v1 (m_vertices[i*4+n].position.x, m_vertices[i*4+n].position.y, m_vertices[i*4+n].position.z);
                            math::Vec3f v2;
                            math::Vec3f v3;
                            if (n == 0) {
                                v2 = math::Vec3f (m_vertices[i*4+3].position.x, m_vertices[i*4+3].position.y, m_vertices[i*4+3].position.z);
                            } else {
                                v2 = math::Vec3f (m_vertices[i*4+n-1].position.x, m_vertices[i*4+n-1].position.y, m_vertices[i*4+n-1].position.z);
                            }
                            if (n == 3) {
                                v3 = math::Vec3f (m_vertices[i*4].position.x, m_vertices[i*4].position.y, m_vertices[i*4].position.z);
                            } else {
                                v3 = math::Vec3f (m_vertices[i*4+n+1].position.x, m_vertices[i*4+n+1].position.y, m_vertices[i*4+n+1].position.z);
                            }
                            math::Vec3f dir1 = v2 - v1;
                            math::Vec3f dir2 = v3 - v1;
                            math::Vec3f normal = dir1.cross(dir2).normalize();
                            m_normals[i*4+n] = Vector3f(normal.x, normal.y, normal.z);
                        }
                    } else if (m_primitiveType == sf::PrimitiveType::Triangles) {
                        for (unsigned int n = 0; n < 3; n++) {
                            math::Vec3f v1 (m_vertices[i*3+n].position.x, m_vertices[i*3+n].position.y, m_vertices[i*3+n].position.z);
                            math::Vec3f v2;
                            math::Vec3f v3;
                            if (n == 0) {
                                v2 = math::Vec3f (m_vertices[i*3+2].position.x, m_vertices[i*3+2].position.y, m_vertices[i*3+2].position.z);
                            } else {
                                v2 = math::Vec3f (m_vertices[i*3+n-1].position.x, m_vertices[i*3+n-1].position.y, m_vertices[i*3+n-1].position.z);
                            }
                            if (n == 2) {
                                v3 = math::Vec3f (m_vertices[i*3].position.x, m_vertices[i*3].position.y, m_vertices[i*3].position.z);
                            } else {
                                v3 = math::Vec3f (m_vertices[i*3+n+1].position.x, m_vertices[i*3+n+1].position.y, m_vertices[i*3+n+1].position.z);
                            }
                            math::Vec3f dir1 = v2 - v1;
                            math::Vec3f dir2 = v3 - v1;
                            math::Vec3f normal = dir1.cross(dir2).normalize();
                            m_normals[i*3+n] = Vector3f(normal.x, normal.y, normal.z);
                        }
                    } else if (m_primitiveType == sf::PrimitiveType::TriangleStrip) {
                        if (i == 0) {
                            for (unsigned int n = 0; n < 3; n++) {
                                math::Vec3f v1 (m_vertices[n].position.x, m_vertices[n].position.y, m_vertices[n].position.z);
                                math::Vec3f v2;
                                math::Vec3f v3;
                                if (n == 0) {
                                    v2 = math::Vec3f (m_vertices[2].position.x, m_vertices[2].position.y, m_vertices[2].position.z);
                                } else {
                                    v2 = math::Vec3f (m_vertices[n-1].position.x, m_vertices[n-1].position.y, m_vertices[n-1].position.z);
                                }
                                if (n == 2) {
                                    v3 = math::Vec3f (m_vertices[0].position.x, m_vertices[0].position.y, m_vertices[0].position.z);
                                } else {
                                    v3 = math::Vec3f (m_vertices[n+1].position.x, m_vertices[n+1].position.y, m_vertices[n+1].position.z);
                                }
                                math::Vec3f dir1 = v2 - v1;
                                math::Vec3f dir2 = v3 - v1;
                                math::Vec3f normal = dir1.cross(dir2).normalize();
                                m_normals[i*3+n] = Vector3f(normal.x, normal.y, normal.z);
                            }
                        } else {
                            math::Vec3f v1 (m_vertices[i+2].position.x, m_vertices[i+2].position.y, m_vertices[i+2].position.z);
                            math::Vec3f v2 (m_vertices[i+1].position.x, m_vertices[i+1].position.y, m_vertices[i+1].position.z);
                            math::Vec3f v3 (m_vertices[i].position.x, m_vertices[i].position.y, m_vertices[i].position.z);
                            math::Vec3f dir1 = v2 - v1;
                            math::Vec3f dir2 = v3 - v1;
                            math::Vec3f normal = dir1.cross(dir2).normalize();
                            m_normals[i+3] = Vector3f(normal.x, normal.y, normal.z);
                        }
                    } else if (m_primitiveType == sf::TriangleFan) {
                        if (i == 0) {
                            for (unsigned int n = 0; n < 3; n++) {
                                math::Vec3f v1 (m_vertices[n].position.x, m_vertices[n].position.y, m_vertices[n].position.z);
                                math::Vec3f v2;
                                math::Vec3f v3;
                                if (n == 0) {
                                    v2 = math::Vec3f (m_vertices[2].position.x, m_vertices[2].position.y, m_vertices[2].position.z);
                                } else {
                                    v2 = math::Vec3f (m_vertices[n-1].position.x, m_vertices[n-1].position.y, m_vertices[n-1].position.z);
                                }
                                if (n == 2) {
                                    v3 = math::Vec3f (m_vertices[0].position.x, m_vertices[0].position.y, m_vertices[0].position.z);
                                } else {
                                    v3 = math::Vec3f (m_vertices[n+1].position.x, m_vertices[n+1].position.y, m_vertices[n+1].position.z);
                                }
                                math::Vec3f dir1 = v2 - v1;
                                math::Vec3f dir2 = v3 - v1;
                                math::Vec3f normal = dir1.cross(dir2).normalize();
                                m_normals[i*3+n] = Vector3f(normal.x, normal.y, normal.z);
                            }
                        } else {
                            math::Vec3f v1 (m_vertices[i+2].position.x, m_vertices[i+2].position.y, m_vertices[i+2].position.z);
                            math::Vec3f v2 (m_vertices[i+1].position.x, m_vertices[i+1].position.y, m_vertices[i+1].position.z);
                            math::Vec3f v3 (m_vertices[0].position.x, m_vertices[0].position.y, m_vertices[0].position.z);
                            math::Vec3f dir1 = v2 - v1;
                            math::Vec3f dir2 = v3 - v1;
                            math::Vec3f normal = dir1.cross(dir2).normalize();
                            m_normals[i+3] = Vector3f(normal.x, normal.y, normal.z);
                        }
                    }
                }
            }
            needToUpdateNormals = false;
        }
        bool VertexBuffer::isLoop() {
            return loop;
        }
        void VertexBuffer::transform(TransformMatrix tm) {
            for (unsigned int i = 0; i < m_vertices.size(); i++) {
                if (m_locals.size() < m_vertices.size())
                    m_locals.push_back(math::Vec3f(m_vertices[i].position.x, m_vertices[i].position.y,m_vertices[i].position.z));
                math::Vec3f t = tm.transform(m_locals[i]);
                m_vertices[i].position.x = t.x;
                m_vertices[i].position.y = t.y;
                m_vertices[i].position.z = t.z;
            }
            needToUpdateVertexBuffer = true;
        }
        ////////////////////////////////////////////////////////////
        VertexBuffer::VertexBuffer(PrimitiveType type, unsigned int vertexCount, Entity* entity) :
        m_vertices     (vertexCount),
        m_primitiveType(type),
        m_entity(entity)
        {
            vboVertexBuffer = 0;
            vboNormalBuffer = 0;
            vboIndexBuffer = 0;
            oldVerticesSize = 0;
            oldIndexesSize = 0;
            oldMaterialTypeSize = 0;
            oldLayerSize = 0;
            oldSpecularSize = 0;
            oldLightInfosSize = 0;
            vboMaterialType = 0;
            vboLayer = 0;
            vboSpecular = 0;
            vboLightInfos = 0;
            needToUpdateVertexBuffer = true;
            needToUpdateIndexBuffer = true;
            needToUpdateMaterialTypeBuffer = true;
            needToUpdateLayersBuffer = true;
            needToUpdateSpecularBuffer = true;
            needToUpdateLightInfos = true;
            loop = true;
            nbVerticesPerFace = 4;
        }
        void VertexBuffer::setEntity(Entity* entity) {
            m_entity = entity;
        }
        Entity* VertexBuffer::getEntity() {
            return m_entity;
        }
        void VertexBuffer::addInstancedRenderingInfos(unsigned int numIndexes, unsigned int baseVertex, unsigned int baseIndex) {
            m_numIndexes.push_back(numIndexes);
            m_baseVertices.push_back(baseVertex);
            m_baseIndexes.push_back(baseIndex);
        }
        void VertexBuffer::addIndex(unsigned int index) {
            m_indexes.push_back(index);
            //std::cout<<"vb index : "<<index<<"size : "<<m_indexes.size()<<std::endl;
            if (!needToUpdateIndexBuffer)
                needToUpdateIndexBuffer = true;
        }
        void VertexBuffer::addLightInfos (math::Vec3f center, sf::Color color) {
            //std::cout<<"add light info : "<<center<<(int) color.r<<","<<(int) color.g<<","<<(int) color.b<<","<<(int) color.a<<std::endl;
            LightInfos li;
            li.center = center;
            li.color = color;
            m_lightInfos.push_back(li);
            if (!needToUpdateLightInfos)
                needToUpdateLightInfos = true;
        }
        void VertexBuffer::update(VertexArray& va) {
            for (unsigned int i = 0; i < va.getVertexCount(); i++) {
                m_vertices[va.m_indexes[i]] = va[i];
            }
            if (!needToUpdateIndexBuffer)
                needToUpdateVertexBuffer = true;
        }
        void VertexBuffer::remove(VertexArray& va) {
            for (unsigned int i = 0; i < va.getVertexCount(); i++) {
                unsigned int first = va.m_indexes[0];
                std::vector<Vertex>::iterator itv;
                std::vector<sf::Vector3f>::iterator itn;
                std::vector<unsigned int>::iterator iti;
                for (iti = m_indexes.begin(); iti != m_indexes.end();) {
                    if (*iti == va.m_indexes[i]) {
                        for (itn = m_normals.begin(); itn != m_normals.end();) {
                            if (*iti == m_normals[i].y) {
                                itn = m_normals.erase(itn);
                            } else {
                                itn++;
                            }
                        }
                        for (itv = m_vertices.begin(); itv != m_vertices.end();) {
                            if (*iti == va.m_indexes[i]) {
                                itv = m_vertices.erase(itv);
                            } else {
                                itv++;
                            }
                        }
                        iti = m_indexes.erase(iti);
                    } else {
                        iti++;
                    }
                }
                for (unsigned int i = 0; i < m_indexes.size(); i++) {
                    if (m_indexes[i] > first) {
                        m_indexes[i] -= va.getVertexCount();
                    }
                }
                if(!needToUpdateVertexBuffer)
                    needToUpdateVertexBuffer  = true;
                if(!needToUpdateIndexBuffer)
                    needToUpdateIndexBuffer  = true;
            }
        }
        std::vector<unsigned int> VertexBuffer::getBaseIndexes() {
            return m_baseIndexes;
        }
        std::vector<unsigned int> VertexBuffer::getIndexes() {
            return m_indexes;
        }
        ///////////////////////////////////////////////////////////
        unsigned int VertexBuffer::getVertexCount() const
        {
            return static_cast<unsigned int>(m_vertices.size());
        }


        ////////////////////////////////////////////////////////////
        Vertex& VertexBuffer::operator [](unsigned int index)
        {
            return m_vertices[index];
        }


        ////////////////////////////////////////////////////////////
        const Vertex& VertexBuffer::operator [](unsigned int index) const
        {
            return m_vertices[index];
        }


        ////////////////////////////////////////////////////////////
        void VertexBuffer::clear()
        {
            m_vertices.clear();
            m_indexes.clear();
            m_normals.clear();
            m_locals.clear();
            m_texturesIndexes.clear();
            m_MaterialType.clear();
            m_layers.clear();
            m_specular.clear();
            m_lightInfos.clear();
        }
        void VertexBuffer::resetIndexes(std::vector<unsigned int> indexes) {
            m_indexes = indexes;
            needToUpdateIndexBuffer = true;
        }
        ////////////////////////////////////////////////////////////
        void VertexBuffer::resize(unsigned int vertexCount)
        {
            m_vertices.resize(vertexCount);
        }
        ////////////////////////////////////////////////////////////
        void VertexBuffer::append(const Vertex& vertex, unsigned int textureIndex)
        {
            /*bool contains = false;
            for (unsigned int i = 0; i < m_vertices.size(); i++) {
                if (m_vertices[i] == vertex)
                    contains = true;
            }*/
            //if (!contains)  {
                //std::cout<<"position : "<<vertex.position.x<<" "<<vertex.position.y<<" "<<vertex.position.z<<std::endl;
                m_vertices.push_back(vertex);
                m_texturesIndexes.push_back(textureIndex);
                /*m_vPosX.push_back(vertex.position.x);
                m_vPosY.push_back(vertex.position.y);
                m_vPosZ.push_back(vertex.position.z);
                m_vPosW.push_back(1);
                m_vcRed.push_back(vertex.color.r);
                m_vcBlue.push_back(vertex.color.g);
                m_vcGreen.push_back(vertex.color.b);
                m_vcAlpha.push_back(vertex.color.a);
                m_ctX.push_back(vertex.texCoords.x);
                m_ctY.push_back(vertex.texCoords.y);
                m_locals.push_back(math::Vec3f(vertex.position.x, vertex.position.y, vertex.position.z));*/
                if (!needToUpdateVertexBuffer)
                    needToUpdateVertexBuffer = true;
                needToUpdateNormals = true;
        }
        void VertexBuffer::addMaterialType(unsigned int materialType) {
            m_MaterialType.push_back(materialType);
            needToUpdateMaterialTypeBuffer = true;
        }
        void VertexBuffer::addLayer(unsigned int layer) {
            m_layers.push_back(layer);
            needToUpdateLayersBuffer = true;
        }
        void VertexBuffer::addSpecular(float specularIntensity, float specularPower) {
            m_specular.push_back(math::Vec2f(specularIntensity, specularPower));
            needToUpdateSpecularBuffer = true;
        }
        math::Vec3f VertexBuffer::getLocal(unsigned int index) const {
            return m_locals[index];
        }
        void VertexBuffer::setLocal(unsigned int index, math::Vec3f v) {
            m_locals[index] = v;
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

        bool VertexBuffer::operator== (const VertexBuffer &other) const {
            if (getVertexCount() != other.getVertexCount())
                return false;
            if (m_primitiveType != other.m_primitiveType)
                return false;
            for (unsigned int i = 0; i < getVertexCount(); i++)
                if (!(m_vertices[i] == other.m_vertices[i]))
                    return false;
            return true;
        }
        void VertexBuffer::addTransformId(unsigned int id, unsigned int vertexId) {
            m_normals.push_back(sf::Vector3f(id, vertexId, 0));
        }
        void VertexBuffer::resizeVertices(unsigned int newSize) {
            m_vertices.resize(newSize);
            m_texturesIndexes.resize(newSize);
            needToUpdateVertexBuffer = true;
            needToUpdateNormals = true;
        }
        void VertexBuffer::resizeMaterialTypes(unsigned int newSize) {
            m_MaterialType.resize(newSize);
            needToUpdateMaterialTypeBuffer = true;
        }
        void VertexBuffer::resizeLayers(unsigned int newSize) {
            m_layers.resize(newSize);
            needToUpdateLayersBuffer = true;
        }
        void VertexBuffer::resizeSpeculars(unsigned int newSize) {
            m_specular.resize(newSize);
            needToUpdateSpecularBuffer = true;
        }
        void VertexBuffer::resizeLightInfos(unsigned int newSize) {
            m_lightInfos.resize(newSize);
            needToUpdateLightInfos = true;
        }
        std::vector<Vertex>& VertexBuffer::getVertices() {
            return m_vertices;
        }
        std::vector<unsigned int>& VertexBuffer::getTextureIndexes() {
            return m_texturesIndexes;
        }
        std::vector<unsigned int>& VertexBuffer::getLayers() {
            return m_layers;
        }
        std::vector<unsigned int>& VertexBuffer::getMaterialTypes () {
            return m_MaterialType;
        }
        std::vector<math::Vec2f>& VertexBuffer::getSpeculars() {
            return m_specular;
        }
        std::vector<VertexBuffer::LightInfos>& VertexBuffer::getLightInfos() {
            return m_lightInfos;
        }
        void VertexBuffer::update() {
            if (!m_vertices.empty()) {
                computeNormals();
                if (GLEW_ARB_vertex_buffer_object) {
                    if (needToUpdateVertexBuffer) {

                        if (vboVertexBuffer == 0) {

                            GLuint vbo;
                            glCheck(glGenBuffers(1, &vbo));
                            vboVertexBuffer = static_cast<unsigned int>(vbo);
                        }
                        if (oldVerticesSize != m_vertices.size()) {

                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboVertexBuffer));
                            glCheck(glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0], GL_DYNAMIC_DRAW));
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));

                        } else {

                            GLvoid *pos_vbo = nullptr;
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboVertexBuffer));
                            pos_vbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
                            if (pos_vbo != nullptr) {
                                memcpy(pos_vbo,&m_vertices[0],  m_vertices.size() * sizeof(Vertex));
                                glCheck(glUnmapBuffer(GL_ARRAY_BUFFER));
                                pos_vbo = nullptr;
                            }
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        }
                        if (vboNormalBuffer == 0) {
                            //std::cout<<"create vbo normal buffer"<<std::endl;
                            GLuint vbo;
                            glCheck(glGenBuffers(1, &vbo));
                            vboNormalBuffer = static_cast<unsigned int>(vbo);
                        }
                        if (oldVerticesSize != m_vertices.size()) {
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboNormalBuffer));
                            glCheck(glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(sf::Vector3f), &m_normals[0], GL_DYNAMIC_DRAW));
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        } else {
                            GLvoid *pos_vbo = nullptr;
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboNormalBuffer));
                            glCheck(pos_vbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
                            if (pos_vbo != nullptr) {
                                memcpy(pos_vbo,&m_normals[0],  m_normals.size() * sizeof(sf::Vector3f));
                                glCheck(glUnmapBuffer(GL_ARRAY_BUFFER));
                                pos_vbo = nullptr;
                            }
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        }
                        if (vboTextureIndexesBuffer == 0) {
                            GLuint vbo;
                            glCheck(glGenBuffers(1, &vbo));
                            vboTextureIndexesBuffer = static_cast<unsigned int>(vbo);
                        }
                        if (oldVerticesSize != m_vertices.size()) {
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboTextureIndexesBuffer));
                            glCheck(glBufferData(GL_ARRAY_BUFFER, m_texturesIndexes.size() * sizeof(unsigned int), &m_texturesIndexes[0], GL_DYNAMIC_DRAW));
                            //std::cout<<"vbo index texture : "<<vboTextureIndexesBuffer<<std::endl;
                            /*for (unsigned int i = 0; i < m_texturesIndexes.size(); i++) {
                                if (m_texturesIndexes[i] > 1)
                                    std::cout<<"texture indexes size : "<<m_texturesIndexes.size()<<" index : "<<m_texturesIndexes[i]<<std::endl;
                            }*/
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        } else {
                            GLvoid *pos_vbo = nullptr;
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboTextureIndexesBuffer));
                            glCheck(pos_vbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
                            if (pos_vbo != nullptr) {
                                memcpy(pos_vbo, &m_texturesIndexes[0], m_texturesIndexes.size() * sizeof(unsigned int));
                                glCheck(glUnmapBuffer(GL_ARRAY_BUFFER));
                                pos_vbo = nullptr;
                            }
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        }
                        needToUpdateVertexBuffer = false;
                    }
                    if (needToUpdateIndexBuffer) {
                        if (vboIndexBuffer == 0) {
                            //std::cout<<"create index vbo buffer"<<std::endl;
                            GLuint vbo;
                            glCheck(glGenBuffers(1, &vbo));
                            vboIndexBuffer = static_cast<unsigned int>(vbo);
                        }
                        if (oldIndexesSize != m_indexes.size()) {
                            //std::cout<<"size changed : update index vbo buffer"<<std::endl;
                            glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndexBuffer));
                            glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexes.size() * sizeof(unsigned int), &m_indexes[0], GL_DYNAMIC_DRAW));
                            glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
                        } else {
                            //std::cout<<"update index vbo buffer"<<std::endl;
                            GLvoid *pos_vbo = nullptr;
                            glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboVertexBuffer));
                            glCheck(pos_vbo = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));
                            if (pos_vbo != nullptr) {
                                memcpy(pos_vbo,&m_indexes[0],  m_indexes.size() * sizeof(unsigned int));
                                glCheck(glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER));
                                pos_vbo = nullptr;
                            }
                            glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
                        }
                        needToUpdateIndexBuffer = false;
                    }
                    if (needToUpdateMaterialTypeBuffer) {
                        if (vboMaterialType == 0) {
                            GLuint vbo;
                            glCheck(glGenBuffers(1, &vbo));
                            vboMaterialType = static_cast<unsigned int>(vbo);
                        }
                        if (oldMaterialTypeSize != m_MaterialType.size()) {
                            //std::cout<<"size changed : update index vbo buffer"<<std::endl;
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboMaterialType));
                            glCheck(glBufferData(GL_ARRAY_BUFFER, m_MaterialType.size() * sizeof(unsigned int), &m_MaterialType[0], GL_DYNAMIC_DRAW));
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        } else {
                            //std::cout<<"update index vbo buffer"<<std::endl;
                            GLvoid *pos_vbo = nullptr;
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboMaterialType));
                            glCheck(pos_vbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
                            if (pos_vbo != nullptr) {
                                memcpy(pos_vbo,&m_MaterialType[0],  m_MaterialType.size() * sizeof(unsigned int));
                                glCheck(glUnmapBuffer(GL_ARRAY_BUFFER));
                                pos_vbo = nullptr;
                            }
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        }
                        needToUpdateMaterialTypeBuffer = false;
                    }
                    if (needToUpdateLayersBuffer) {
                        if (vboLayer == 0) {
                            GLuint vbo;
                            glCheck(glGenBuffers(1, &vbo));
                            vboLayer = static_cast<unsigned int>(vbo);
                        }
                        if (oldLayerSize != m_layers.size()) {
                            //std::cout<<"size changed : update index vbo buffer"<<std::endl;
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboLayer));
                            glCheck(glBufferData(GL_ARRAY_BUFFER, m_layers.size() * sizeof(unsigned int), &m_layers[0], GL_DYNAMIC_DRAW));
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        } else {
                            //std::cout<<"update index vbo buffer"<<std::endl;
                            GLvoid *pos_vbo = nullptr;
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboLayer));
                            glCheck(pos_vbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
                            if (pos_vbo != nullptr) {
                                memcpy(pos_vbo,&m_layers[0],  m_layers.size() * sizeof(unsigned int));
                                glCheck(glUnmapBuffer(GL_ARRAY_BUFFER));
                                pos_vbo = nullptr;
                            }
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        }
                        needToUpdateLayersBuffer = false;
                    }
                    if (needToUpdateSpecularBuffer) {
                        if (vboSpecular == 0) {
                            GLuint vbo;
                            glCheck(glGenBuffers(1, &vbo));
                            vboSpecular = static_cast<unsigned int>(vbo);
                        }
                        if (oldSpecularSize != m_specular.size()) {
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboSpecular));
                            glCheck(glBufferData(GL_ARRAY_BUFFER, m_specular.size() * sizeof(math::Vec2f), &m_specular[0], GL_DYNAMIC_DRAW));
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                         } else {
                            //std::cout<<"update index vbo buffer"<<std::endl;
                            GLvoid *pos_vbo = nullptr;
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboSpecular));
                            glCheck(pos_vbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
                            if (pos_vbo != nullptr) {
                                memcpy(pos_vbo,&m_specular[0],  m_specular.size() * sizeof(math::Vec2f));
                                glCheck(glUnmapBuffer(GL_ARRAY_BUFFER));
                                pos_vbo = nullptr;
                            }
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        }
                    }
                    if (needToUpdateLightInfos) {
                        if (vboLightInfos == 0) {
                            GLuint vbo;
                            glCheck(glGenBuffers(1, &vbo));
                            vboLightInfos = static_cast<unsigned int>(vbo);
                        }
                        if (oldLightInfosSize != m_lightInfos.size()) {
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboLightInfos));
                            glCheck(glBufferData(GL_ARRAY_BUFFER, m_lightInfos.size() * sizeof(LightInfos), &m_lightInfos[0], GL_DYNAMIC_DRAW));
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        } else {
                            //std::cout<<"size : "<<m_lightInfos.size()<<std::endl;
                            GLvoid *pos_vbo = nullptr;
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboLightInfos));
                            glCheck(pos_vbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
                            if (pos_vbo != nullptr) {
                                memcpy(pos_vbo,&m_lightInfos[0],  m_lightInfos.size() * sizeof(LightInfos));
                                glCheck(glUnmapBuffer(GL_ARRAY_BUFFER));
                                pos_vbo = nullptr;
                            }
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        }
                    }
                    oldVerticesSize = m_vertices.size();
                    oldIndexesSize = m_indexes.size();
                    oldMaterialTypeSize = m_MaterialType.size();
                    oldLayerSize = m_layers.size();
                    oldSpecularSize = m_specular.size();
                    oldLightInfosSize = m_lightInfos.size();
                }
            }
        }
        ////////////////////////////////////////////////////////////
        void VertexBuffer::draw(RenderTarget& target, RenderStates states)
        {
            if (!m_vertices.empty()) {

                if (GLEW_ARB_vertex_buffer_object) {
                    if (needToUpdateVertexBuffer) {
                        if (vboVertexBuffer == 0) {
                            //std::cout<<"create vbo vertex buffer"<<std::endl;
                            GLuint vbo;
                            glCheck(glGenBuffers(1, &vbo));
                            vboVertexBuffer = static_cast<unsigned int>(vbo);
                        }
                        if (oldVerticesSize != m_vertices.size()) {
                            //std::cout<<"size changed : update vbo vertex buffer"<<std::endl;
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboVertexBuffer));
                            glCheck(glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0], GL_STREAM_DRAW));
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        } else {
                            //std::cout<<"update vbo vertex buffer"<<std::endl;
                            GLvoid *pos_vbo = nullptr;
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboVertexBuffer));
                            pos_vbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
                            if (pos_vbo != nullptr) {
                                memcpy(pos_vbo,&m_vertices[0],  m_vertices.size() * sizeof(Vertex));
                                glCheck(glUnmapBuffer(GL_ARRAY_BUFFER));
                                pos_vbo = nullptr;
                            }
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        }
                        if (vboNormalBuffer == 0) {
                            //std::cout<<"create vbo normal buffer"<<std::endl;
                            GLuint vbo;
                            glCheck(glGenBuffers(1, &vbo));
                            vboNormalBuffer = static_cast<unsigned int>(vbo);
                        }
                        if (oldVerticesSize != m_vertices.size()) {
                            //std::cout<<"size changed : update vbo normal buffer"<<std::endl;
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboNormalBuffer));
                            glCheck(glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(math::Vec3f), &m_normals[0], GL_STREAM_DRAW));
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        } else {
                            //std::cout<<"update vbo normal buffer"<<std::endl;
                            GLvoid *pos_vbo = nullptr;
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboNormalBuffer));
                            pos_vbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
                            if (pos_vbo != nullptr) {
                                memcpy(pos_vbo,&m_normals[0],  m_normals.size() * sizeof(math::Vec3f));
                                glCheck(glUnmapBuffer(GL_ARRAY_BUFFER));
                                pos_vbo = nullptr;
                            }
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        }
                        if (vboTextureIndexesBuffer == 0) {
                            GLuint vbo;
                            glCheck(glGenBuffers(1, &vbo));
                            vboTextureIndexesBuffer = static_cast<unsigned int>(vbo);
                        }
                        if (oldVerticesSize != m_vertices.size()) {
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboTextureIndexesBuffer));
                            glCheck(glBufferData(GL_ARRAY_BUFFER, m_texturesIndexes.size() * sizeof(unsigned int), &m_texturesIndexes[0], GL_DYNAMIC_DRAW));
                            /*for (unsigned int i = 0; i < m_texturesIndexes.size(); i++) {
                                std::cout<<"texture indexes size : "<<m_texturesIndexes.size()<<" index : "<<m_texturesIndexes[i]<<std::endl;
                            }*/
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        } else {
                            GLvoid *pos_vbo = nullptr;
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboTextureIndexesBuffer));
                            glCheck(pos_vbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
                            if (pos_vbo != nullptr) {
                                memcpy(pos_vbo, &m_texturesIndexes[0], m_texturesIndexes.size() * sizeof(unsigned int));
                                glCheck(glUnmapBuffer(GL_ARRAY_BUFFER));
                                pos_vbo = nullptr;
                            }
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        }
                        needToUpdateVertexBuffer = false;
                    }
                    if (needToUpdateIndexBuffer) {
                        if (vboIndexBuffer == 0) {
                            //std::cout<<"create index vbo buffer"<<std::endl;
                            GLuint vbo;
                            glCheck(glGenBuffers(1, &vbo));
                            vboIndexBuffer = static_cast<unsigned int>(vbo);
                        }
                        if (oldIndexesSize != m_indexes.size()) {
                            //std::cout<<"size changed : update index vbo buffer"<<std::endl;
                            glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndexBuffer));
                            glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexes.size() * sizeof(unsigned int), &m_indexes[0], GL_STREAM_DRAW));
                            glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
                        } else {
                            //std::cout<<"update index vbo buffer"<<std::endl;
                            GLvoid *pos_vbo = nullptr;
                            glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboVertexBuffer));
                            glCheck(pos_vbo = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));
                            if (pos_vbo != nullptr) {
                                memcpy(pos_vbo,&m_indexes[0],  m_indexes.size() * sizeof(unsigned int));
                                glCheck(glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER));
                                pos_vbo = nullptr;
                            }
                            glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
                        }
                        needToUpdateIndexBuffer = false;
                    }
                    target.drawVertexBuffer(*this, states);
                }
                oldVerticesSize = m_vertices.size();
                oldIndexesSize = m_indexes.size();
            }
        }
        ////////////////////////////////////////////////////////////
        physic::BoundingBox VertexBuffer::getBounds()
        {
            if (!m_vertices.empty())
            {
                float left   = m_vertices[0].position.x;
                float top    = m_vertices[0].position.y;
                float right  = m_vertices[0].position.x;
                float bottom = m_vertices[0].position.y;
                float nearest = m_vertices[0].position.z;
                float farest = m_vertices[0].position.z;

                for (std::size_t i = 1; i < m_vertices.size(); ++i)
                {
                    Vector3f position = m_vertices[i].position;

                    // Update left and right
                    if (position.x < left)
                        left = position.x;
                    else if (position.x > right)
                        right = position.x;

                    // Update top and bottom
                    if (position.y < top)
                        top = position.y;
                    else if (position.y > bottom)
                        bottom = position.y;

                    //Update the near and the far.

                    if (position.z < farest)
                        farest = position.z;
                    else if (position.z > nearest)
                        nearest = position.z;

                }

                return physic::BoundingBox(left, top, nearest, right - left, bottom - top, farest - nearest);
            }
            else
            {
                // Array is empty
                return physic::BoundingBox();
            }
        }
        void VertexBuffer::updateVBOBuffer() {
            needToUpdateVertexBuffer = true;
        }
        void VertexBuffer::remove (unsigned int index) {
            std::vector<Vertex>::iterator it = m_vertices.begin();
            for (unsigned int i = 0; i < m_vertices.size(); i++) {
                if (i == index) {
                    it = m_vertices.erase(it);
                } else {
                    it++;
                }
                std::vector<unsigned int>::iterator it2 = m_indexes.begin();
                for (unsigned int j = 0; j < m_indexes.size(); j++) {
                    if (i == m_indexes[j]) {
                        it2 = m_indexes.erase(it2);
                    } else {
                        it2++;
                    }
                }
            }
        }
        VertexBuffer::~VertexBuffer() {
            if (VBO::isAvailable() && vboVertexBuffer != 0) {
                GLuint vbo = static_cast<GLuint>(vboVertexBuffer);
                glCheck(glDeleteBuffers(1, &vbo));
            }
            if (VBO::isAvailable() && vboNormalBuffer != 0) {
                GLuint vbo = static_cast<GLuint>(vboNormalBuffer);
                glCheck(glDeleteBuffers(1, &vbo));
            }
            if (VBO::isAvailable() && vboIndexBuffer != 0) {
                GLuint vbo = static_cast<GLuint>(vboIndexBuffer);
                glCheck(glDeleteBuffers(1, &vbo));
            }
        }
        #endif // VULKAN
    }
} // namespace sf3



