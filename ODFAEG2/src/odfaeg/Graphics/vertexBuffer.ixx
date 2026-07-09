module;
#include <vector>
#include <vulkan/vulkan.hpp>
export module odfaeg.graphic.vertexBuffer;
import odfaeg.entity.transformMatrix;
import odfaeg.graphic.primitiveType;
import odfaeg.graphic.device;
import odfaeg.graphic.vertex;
import odfaeg.graphic.buffer;
import odfaeg.physic.boundingBox;
import odfaeg.graphic.commandPool;
import odfaeg.core.nonCopyable;
namespace odfaeg {
    namespace graphic {

        export class  VertexBuffer : public core::NonCopyable {
        public:
            struct LODLevel {
                uint32_t indexOffset;
                uint32_t indexCount;
            };
            VertexBuffer(Device& device, unsigned int nbBuffers=1);
            VertexBuffer(Device& device, PrimitiveType primitiveType, unsigned int nbBuffers=1);
            VertexBuffer(VertexBuffer&& other) noexcept;
            VertexBuffer& operator=(VertexBuffer&& other) noexcept;
            void createCommandBuffers();
            void copyFrom(VertexBuffer& vertexBuffer);
            void append(const Vertex& vertex);
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
            void setPrimitiveType(PrimitiveType type);
            void resize(unsigned int size, unsigned int indexSize);
            physic::BoundingBox getBounds();
            physic::BoundingBox getGlobalBounds(entity::TransformMatrix& transformMatrix);
            bool operator== (const VertexBuffer& other);
            unsigned int getNbBuffers() const;
            ////////////////////////////////////////////////////////////
            /// \brief Get the type of primitives drawn by the vertex array
            ///
            /// \return Primitive type
            ///
            ////////////////////////////////////////////////////////////
            PrimitiveType getPrimitiveType() const;            
            void update(VkCommandBuffer& commandBuffer, unsigned int currentFrame=0);
            void update(unsigned int currentFrame=0);
            void updateLods();
            Vertex& operator [](unsigned int index);
            Device& getDevice();
            std::array<LODLevel, 5> getLODs() const;
            void setIndex(unsigned int pos, unsigned int idx);
        private:
            bool commandBuffersCreated;
            std::array<LODLevel, 5> lods={};
            unsigned int nbBuffers;                     
            std::vector<Vertex> m_vertices;            
            std::vector<Buffer> vertexBuffer, indexBuffer, vertexStaggingBuffer, indexStaggingBuffer;
            std::vector<std::uint32_t> indices;
            PrimitiveType       m_primitiveType; ///< Type of primitives to draw
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