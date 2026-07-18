module;
#include <vector>
#include <vulkan/vulkan.hpp>
export module odfaeg.graphic.vertexBuffer;
import odfaeg.math.transformMatrix;
import odfaeg.entity.primitiveType;
import odfaeg.graphic.device;
import odfaeg.entity.vertex;
import odfaeg.graphic.buffer;
import odfaeg.physic.boundingBox;
import odfaeg.graphic.commandPool;
import odfaeg.core.nonCopyable;
namespace odfaeg {
    namespace graphic {

        export class  VertexBuffer : public core::NonCopyable {
        public:
            VertexBuffer(Device& device, unsigned int nbBuffers=1);
            VertexBuffer(Device& device, entity::PrimitiveType primitiveType, unsigned int nbBuffers=1);
            VertexBuffer(VertexBuffer&& other) noexcept;
            VertexBuffer& operator=(VertexBuffer&& other) noexcept;
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
                bindingDescription.stride = sizeof(entity::Vertex);
                bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                return bindingDescription;
            }
            static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions() {
                std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions{};
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
                attributeDescriptions[4].format = VK_FORMAT_R32_UINT;
                attributeDescriptions[4].offset = offsetof(entity::Vertex, drawableDataId);

                return attributeDescriptions;
            }
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
        void swap(VertexBuffer& a, VertexBuffer& b) noexcept {
            a.swap(b);
        }
    }
} // namespace sf