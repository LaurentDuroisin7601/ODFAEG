module;
#include <cstdint>
#include <meshoptimizer.h>
#include <vector>
module odfaeg.entity.vertexArray;
import odfaeg.math.vec;
namespace odfaeg {
    namespace entity {
        VertexArray::VertexArray() : m_primitiveType(Points) {
            
        }
        VertexArray::VertexArray(PrimitiveType primitiveType) : m_primitiveType(primitiveType) {

        }
        void VertexArray::clear() {
            m_vertices.clear();
            indices.clear();
        }
        void VertexArray::append(const Vertex& vertex) {
            m_vertices.push_back(vertex);
        }
        void VertexArray::addIndex(std::uint32_t index) {
            indices.push_back(index);
        }
        unsigned int VertexArray::getIndex(unsigned int i) {
            return indices[i];
        }
        size_t VertexArray::getIndexCount() const {
            return indices.size();
        }
        size_t VertexArray::getVertexCount() const {
            return m_vertices.size();
        }
        void VertexArray::setPrimitiveType(PrimitiveType type) {
            m_primitiveType = type;
        }
        void VertexArray::resize(unsigned int size, unsigned int indexSize) {
            m_vertices.resize(size);
            indices.resize(indexSize);
        }
        bool VertexArray::operator== (const VertexArray& other) {
            if (getVertexCount() != other.getVertexCount())
                return false;
            if (m_primitiveType != other.m_primitiveType)
                return false;
            for (unsigned int i = 0; i < getVertexCount(); i++)
                if (!(m_vertices[i] == other.m_vertices[i]))
                    return false;
            return true;
        }
        PrimitiveType VertexArray::getPrimitiveType() const {
            return m_primitiveType;
        } 
        Vertex& VertexArray::operator [](unsigned int index) {
            return m_vertices[index];
        }           
        void VertexArray::setIndex(unsigned int pos, unsigned int idx) {
            indices[pos] = idx;
        }
        physic::BoundingBox VertexArray::getBounds()
        {
            if (!m_vertices.empty())
            {
                float left = m_vertices[0].position.x();
                float top = m_vertices[0].position.y();
                float right = m_vertices[0].position.x();
                float bottom = m_vertices[0].position.y();
                float nearest = m_vertices[0].position.z();
                float farest = m_vertices[0].position.z();
                //std::cout<<"first position : "<<m_vertices[0].position<<std::endl;

                for (size_t i = 1; i < m_vertices.size(); ++i)
                {
                    math::Vec3f position = m_vertices[i].position;
                    /*std::cout<<"position : "<<position<<std::endl;
                    system("PAUSE");*/

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
        physic::BoundingBox VertexArray::getGlobalBounds(math::TransformMatrix& tm)
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

                for (size_t i = 1; i < m_vertices.size(); ++i)
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
        void VertexArray::updateLods() {
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
        std::array<VertexArray::LODLevel, 5> VertexArray::getLODs() const {
            return lods;
        }
    }
}