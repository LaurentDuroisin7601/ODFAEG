module;
#include <vector>
export module odfaeg.entity.vertexArray;
import odfaeg.entity.vertex;
import odfaeg.entity.primitiveType;
import odfaeg.math.transformMatrix;
import odfaeg.physic.boundingBox;
namespace odfaeg {
    namespace entity {
        export class VertexArray {
            public :
            struct LODLevel {
                uint32_t indexOffset;
                uint32_t indexCount;
            };
            VertexArray();
            VertexArray(PrimitiveType PrimitiveType);
            void clear();
            void append(const Vertex& vertex);
            void addIndex(std::uint32_t index);
            unsigned int getIndex(unsigned int i);
            size_t getVertexCount() const;
            size_t getIndexCount() const;
            void setPrimitiveType(PrimitiveType type);
            void resize(unsigned int size, unsigned int indexSize);
            bool operator== (const VertexArray& other);
            PrimitiveType getPrimitiveType() const; 
            Vertex& operator [](unsigned int index);            
            void setIndex(unsigned int pos, unsigned int idx);
            physic::BoundingBox getBounds();
            physic::BoundingBox getGlobalBounds(math::TransformMatrix& transformMatrix);
            void updateLods();
            std::array<VertexArray::LODLevel, 5> getLODs() const;            
            private :            
            std::array<LODLevel, 5> lods={};
            std::vector<Vertex> m_vertices;  
            std::vector<std::uint32_t> indices;
            PrimitiveType       m_primitiveType; ///< Type of primitives to draw            
        };
    }
}