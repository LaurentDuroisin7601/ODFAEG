#include "GL/glew.h"
#include <SFML/OpenGL.hpp>
#include "../../../include/odfaeg/Graphics/vertexArray.h"
#include "../../../include/odfaeg/Graphics/renderTarget.h"
#include "glCheck.h"
#include <string.h>
namespace odfaeg {
    namespace graphic {
        using namespace sf;
        ////////////////////////////////////////////////////////////
        VertexArray::VertexArray() :
        m_vertices     (),
        m_primitiveType(Points),
        m_entity(nullptr)
        {
            vboVertexBuffer = 0;
            vboNormalBuffer = 0;
            vboIndexBuffer = 0;
            needToUpdateVBOBuffer = false;
            nbVerticesPerFace = 4;
            loop = true;
        }
        VertexArray::VertexArray(const VertexArray& va) {
            vboVertexBuffer = 0;
            vboNormalBuffer = 0;
            vboIndexBuffer = 0;
            m_entity = va.m_entity;
            m_normals = va.m_normals;
            m_locals = va.m_locals;
            m_vertices = va.m_vertices;
            m_primitiveType = va.m_primitiveType;
            oldVerticesSize = va.oldVerticesSize;
            m_numIndexes = va.m_numIndexes;
            m_baseIndexes = va.m_baseIndexes;
            m_baseVertices = va.m_baseVertices;
            m_indexes = va.m_indexes;
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
            needToUpdateVBOBuffer = true;
        }
        VertexArray::VertexArray(const VertexArray&& va) {
            vboVertexBuffer = 0;
            vboNormalBuffer = 0;
            vboIndexBuffer = 0;
            m_entity = va.m_entity;
            m_normals = va.m_normals;
            m_locals = va.m_locals;
            m_vertices = va.m_vertices;
            m_primitiveType = va.m_primitiveType;
            oldVerticesSize = va.oldVerticesSize;
            m_numIndexes = va.m_numIndexes;
            m_baseIndexes = va.m_baseIndexes;
            m_baseVertices = va.m_baseVertices;
            m_indexes = va.m_indexes;
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
            needToUpdateVBOBuffer = true;
        }
        VertexArray& VertexArray::operator= (const VertexArray& va) {
            vboVertexBuffer = 0;
            vboNormalBuffer = 0;
            vboIndexBuffer = 0;
            m_entity = va.m_entity;
            m_normals = va.m_normals;
            m_locals = va.m_locals;
            m_vertices = va.m_vertices;
            m_primitiveType = va.m_primitiveType;
            oldVerticesSize = va.oldVerticesSize;
            m_numIndexes = va.m_numIndexes;
            m_baseIndexes = va.m_baseIndexes;
            m_baseVertices = va.m_baseVertices;
            m_indexes = va.m_indexes;
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
            needToUpdateVBOBuffer = true;
            return *this;
        }
        VertexArray& VertexArray::operator= (const VertexArray&& va) {
            vboVertexBuffer = 0;
            vboNormalBuffer = 0;
            vboIndexBuffer = 0;
            m_entity = va.m_entity;
            m_normals = va.m_normals;
            m_locals = va.m_locals;
            m_vertices = va.m_vertices;
            m_primitiveType = va.m_primitiveType;
            oldVerticesSize = va.oldVerticesSize;
            m_numIndexes = va.m_numIndexes;
            m_baseIndexes = va.m_baseIndexes;
            m_baseVertices = va.m_baseVertices;
            m_indexes = va.m_indexes;
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
            needToUpdateVBOBuffer = true;
            return *this;
        }
        bool VertexArray::isLoop() {
            return loop;
        }
        void VertexArray::transform(TransformMatrix tm) {
            //std::cout<<"transform"<<std::endl;
            for (unsigned int i = 0; i < m_vertices.size(); i++) {
                if (m_locals.size() < m_vertices.size())
                    m_locals.push_back(math::Vec3f(m_vertices[i].position.x, m_vertices[i].position.y,m_vertices[i].position.z));
                math::Vec3f t = tm.transform(m_locals[i]);
                m_vertices[i].position.x = t.x;
                m_vertices[i].position.y = t.y;
                m_vertices[i].position.z = t.z;
                //std::cout<<"vertex pos : "<<m_vertices[i].position.x<<" "<<m_vertices[i].position.y<<" "<<m_vertices[i].position.z<<std::endl;
            }
            needToUpdateVBOBuffer = true;
        }
        ////////////////////////////////////////////////////////////
        VertexArray::VertexArray(PrimitiveType type, unsigned int vertexCount, Entity* entity) :
        m_vertices     (vertexCount),
        m_primitiveType(type),
        m_entity(entity)
        {

        }
        void VertexArray::setEntity(Entity* entity) {
            m_entity = entity;
        }
        Entity* VertexArray::getEntity() {
            return m_entity;
        }
        void VertexArray::addInstancedRenderingInfos(unsigned int numIndexes, unsigned int baseVertex, unsigned int baseIndex) {
            m_numIndexes.push_back(numIndexes);
            m_baseVertices.push_back(baseVertex);
            m_baseIndexes.push_back(baseIndex);
        }
        void VertexArray::addIndex(unsigned int index) {
            m_indexes.push_back(index);
            //std::cout<<"va index : "<<index<<"size : "<<m_indexes.size()<<std::endl;

        }
        std::vector<unsigned int> VertexArray::getBaseIndexes() {
            return m_baseIndexes;
        }
        std::vector<unsigned int> VertexArray::getIndexes() {
            return m_indexes;
        }
        void VertexArray::computeNormals() {
            unsigned int size = 0;
            if (m_primitiveType == sf::PrimitiveType::Quads) {
                size = m_vertices.size() / 4;
            } else if (m_primitiveType == sf::PrimitiveType::Triangles) {
                size = m_vertices.size() / 3;
            } else if (m_primitiveType == sf::PrimitiveType::TriangleStrip || m_primitiveType == sf::PrimitiveType::TriangleFan) {
                size = m_vertices.size() - 2;
            }
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
                        m_normals.push_back(Vector3f(normal.x, normal.y, normal.z));
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
                        m_normals.push_back(Vector3f(normal.x, normal.y, normal.z));
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
                            m_normals.push_back(Vector3f(normal.x, normal.y, normal.z));
                        }
                    } else {
                        math::Vec3f v1 (m_vertices[i+2].position.x, m_vertices[i+2].position.y, m_vertices[i+2].position.z);
                        math::Vec3f v2 (m_vertices[i+1].position.x, m_vertices[i+1].position.y, m_vertices[i+1].position.z);
                        math::Vec3f v3 (m_vertices[i].position.x, m_vertices[i].position.y, m_vertices[i].position.z);
                        math::Vec3f dir1 = v2 - v1;
                        math::Vec3f dir2 = v3 - v1;
                        math::Vec3f normal = dir1.cross(dir2).normalize();
                        m_normals.push_back(Vector3f(normal.x, normal.y, normal.z));
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
                            m_normals.push_back(Vector3f(normal.x, normal.y, normal.z));
                        }
                    } else {
                        math::Vec3f v1 (m_vertices[i+2].position.x, m_vertices[i+2].position.y, m_vertices[i+2].position.z);
                        math::Vec3f v2 (m_vertices[i+1].position.x, m_vertices[i+1].position.y, m_vertices[i+1].position.z);
                        math::Vec3f v3 (m_vertices[0].position.x, m_vertices[0].position.y, m_vertices[0].position.z);
                        math::Vec3f dir1 = v2 - v1;
                        math::Vec3f dir2 = v3 - v1;
                        math::Vec3f normal = dir1.cross(dir2).normalize();
                        m_normals.push_back(Vector3f(normal.x, normal.y, normal.z));
                    }
                }
            }
        }
        ///////////////////////////////////////////////////////////
        unsigned int VertexArray::getVertexCount() const
        {
            return static_cast<unsigned int>(m_vertices.size());
        }


        ////////////////////////////////////////////////////////////
        Vertex& VertexArray::operator [](unsigned int index)
        {
            //std::cout<<"vertex position : "<<m_vertices[index].position.x<<" "<<m_vertices[index].position.y<<std::endl;
            return m_vertices[index];
        }


        ////////////////////////////////////////////////////////////
        const Vertex& VertexArray::operator [](unsigned int index) const
        {
            //std::cout<<"vertex position : "<<m_vertices[index].position.x<<" "<<m_vertices[index].position.y<<std::endl;
            return m_vertices[index];
        }


        ////////////////////////////////////////////////////////////
        void VertexArray::clear()
        {
            m_vertices.clear();
            m_indexes.clear();
            m_normals.clear();
            m_locals.clear();
        }


        ////////////////////////////////////////////////////////////
        void VertexArray::resize(unsigned int vertexCount)
        {
            m_vertices.resize(vertexCount);
        }


        ////////////////////////////////////////////////////////////
        void VertexArray::append(const Vertex& vertex)
        {
            /*bool contains = false;
            for (unsigned int i = 0; i < m_vertices.size(); i++) {
                if (m_vertices[i] == vertex)
                    contains = true;
            }*/
            //if (!contains)  {
                m_vertices.push_back(vertex);
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
                if (!needToUpdateVBOBuffer)
                    needToUpdateVBOBuffer = true;            //
        }
        math::Vec3f VertexArray::getLocal(unsigned int index) const {
            return m_locals[index];
        }
        void VertexArray::setLocal(unsigned int index, math::Vec3f v) {
            m_locals[index] = v;
        }
        ////////////////////////////////////////////////////////////
        void VertexArray::setPrimitiveType(PrimitiveType type)
        {
            m_primitiveType = type;
        }


        ////////////////////////////////////////////////////////////
        PrimitiveType VertexArray::getPrimitiveType() const
        {
            return m_primitiveType;
        }

        bool VertexArray::operator== (const VertexArray &other) const {
            if (getVertexCount() != other.getVertexCount())
                return false;
            if (m_primitiveType != other.m_primitiveType)
                return false;
            for (unsigned int i = 0; i < getVertexCount(); i++)
                if (!(m_vertices[i] == other.m_vertices[i]))
                    return false;
            return true;
        }
        ////////////////////////////////////////////////////////////
        void VertexArray::draw(RenderTarget& target, RenderStates states)
        {
            if (!m_vertices.empty()) {
                target.draw(&m_vertices[0], m_vertices.size(), m_primitiveType, states);
            }
        }
        ////////////////////////////////////////////////////////////
        physic::BoundingBox VertexArray::getBounds()
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
        void VertexArray::remove (unsigned int index) {
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
        VertexArray::~VertexArray() {

        }
    }
} // namespace sf3


