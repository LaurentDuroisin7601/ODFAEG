#ifndef ODFAEG_VERTEXBUFFER_HPP
#define ODFAEG_VERTEXBUFFER_HPP
#include <vector>
#include <vulkan/vulkan.hpp>
#include "../Math/transformMatrix.hpp"
#include "../Entity/primitiveType.hpp"
#include "device.hpp"
#include "../Entity/vertex.hpp"
#include "buffer.hpp"
#include "../Physics/boundingBox.hpp"
#include "commandPool.hpp"
#include "../Core/nonCopyable.hpp"
#include <iostream>
namespace odfaeg {
    namespace graphic {
        struct VkVertex {
            alignas(16) float position[3]; ///< 3D position of the vertex
            entity::Color color; ///< Color of the vertex
            math::Vec2f texCoords; ///< Coordinates of the texture's pixel to map to the vertex
            alignas(16) float normal[3];
            alignas(16) float T[3]; 
            alignas(16) float B[3];
            alignas(16) float N[3];
            unsigned int drawableDataId; 
            //bone indexes which will influence this vertex
            int m_BoneIDs[MAX_BONES_INFLUENCE];
            //weights from each bone
            float m_Weights[MAX_BONES_INFLUENCE];
        };
        class  VertexBuffer : public core::NonCopyable {
        public:            
            VertexBuffer(Device& device, unsigned int nbBuffers=1);
            VertexBuffer(Device& device, entity::PrimitiveType primitiveType, unsigned int nbBuffers=1);
            VertexBuffer(VertexBuffer&& other) noexcept;
            VertexBuffer& operator=(VertexBuffer&& other) noexcept;
            static void toVkVertex(VkVertex& vkVertex, entity::Vertex vertex);
            void createCommandBuffers();
            void copyFrom(VertexBuffer& vertexBuffer);
            void append(const entity::Vertex& vertex);
            void addIndex(std::uint32_t index);
            unsigned int getIndex(unsigned int i);
            size_t getIndexCount() const;
            void clear();
            void swap(VertexBuffer& vb);
            Buffer& getVertexBuffer(unsigned int currentFrame);
            Buffer& getIndexBuffer(unsigned int currentFrame);
            Buffer& getStaggingVertexBuffer(unsigned int currentFrame);
            Buffer& getStaggingIndexBuffer(unsigned int currentFrame);
            size_t getVertexCount() const;          
            void setPrimitiveType(entity::PrimitiveType type);
            void resize(unsigned int size, unsigned int indexSize);
            physic::BoundingBox getBounds();
            physic::BoundingBox getGlobalBounds(math::TransformMatrix& transformMatrix);
            bool operator== (const VertexBuffer& other);
            unsigned int getNbBuffers() const;
            ////////////////////////////////////////////////////////////
            /// \brief Get the type of primitives drawn by the vertex array
            ///
            /// \return Primitive type
            ///
            ////////////////////////////////////////////////////////////
            entity::PrimitiveType getPrimitiveType() const;            
            void update(VkCommandBuffer& commandBuffer, unsigned int currentFrame=0);
            void update(unsigned int currentFrame=0);            
            entity::Vertex& operator [](unsigned int index);
            Device& getDevice();            
            void setIndex(unsigned int pos, unsigned int idx);
            static VkVertexInputBindingDescription getBindingDescription() {
                VkVertexInputBindingDescription bindingDescription{};
                bindingDescription.binding = 0;
                //std::cout<<"stride : "<<sizeof(entity::Vertex)<<std::endl;
                bindingDescription.stride = sizeof(entity::Vertex);
                bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                return bindingDescription;
            }
            static std::array<VkVertexInputAttributeDescription, 8> getAttributeDescriptions() {
                std::array<VkVertexInputAttributeDescription, 8> attributeDescriptions{};
                attributeDescriptions[0].binding = 0;
                attributeDescriptions[0].location = 0;
                attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[0].offset = offsetof(entity::Vertex, position);

                attributeDescriptions[1].binding = 0;
                attributeDescriptions[1].location = 1;
                attributeDescriptions[1].format = VK_FORMAT_R8G8B8A8_UNORM;
                attributeDescriptions[1].offset = offsetof(entity::Vertex, color);

                attributeDescriptions[2].binding = 0;
                attributeDescriptions[2].location = 2;
                attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
                attributeDescriptions[2].offset = offsetof(entity::Vertex, texCoords);
                /*std::cout<<"stride : "<<offsetof(Vertex, texCoords)<<std::endl;
                int pause;
                std::cin>>pause;*/

                attributeDescriptions[3].binding = 0;
                attributeDescriptions[3].location = 3;
                attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[3].offset = offsetof(entity::Vertex, normal);

             

                attributeDescriptions[4].binding = 0;
                attributeDescriptions[4].location = 4;
                attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[4].offset = offsetof(entity::Vertex, T);

                attributeDescriptions[5].binding = 0;
                attributeDescriptions[5].location = 5;
                attributeDescriptions[5].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[5].offset = offsetof(entity::Vertex, B);

                attributeDescriptions[6].binding = 0;
                attributeDescriptions[6].location = 6;
                attributeDescriptions[6].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[6].offset = offsetof(entity::Vertex, N);
                
                attributeDescriptions[7].binding = 0;
                attributeDescriptions[7].location = 7;
                attributeDescriptions[7].format = VK_FORMAT_R32_UINT;
                attributeDescriptions[7].offset = offsetof(entity::Vertex, drawableDataId);    
                return attributeDescriptions;
            }
            std::vector<std::uint32_t> getIndexes();
            std::vector<entity::Vertex> getVertices();
        private:
            
            bool commandBuffersCreated;            
            unsigned int nbBuffers;                     
            std::vector<entity::Vertex> m_vertices;            
            std::vector<Buffer> vertexBuffer, indexBuffer, vertexStaggingBuffer, indexStaggingBuffer;
            std::vector<std::uint32_t> indices;
            entity::PrimitiveType       m_primitiveType; ///< Type of primitives to draw
            std::vector<bool> needToUpdateVertexBuffer, needToUpdateIndexBuffer;
            std::vector<VkDeviceSize> maxVerticesSize, maxIndexSize;
            Device& device;
            CommandPool commandPool;
        };        
        void swap(VertexBuffer& a, VertexBuffer& b) noexcept;
    }
} // namespace sf
#endif