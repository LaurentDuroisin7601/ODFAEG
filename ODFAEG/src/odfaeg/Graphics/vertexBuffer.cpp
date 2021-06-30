
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
            needToUpdateVertexBuffer = false;
            needToUpdateIndexBuffer = false;
            nbVerticesPerFace = 4;
            loop = true;
            oldVerticesSize = 0;
            oldIndexesSize = 0;
        }
        VertexBuffer::VertexBuffer(const VertexBuffer& va) {
            vboVertexBuffer = 0;
            vboNormalBuffer = 0;
            vboIndexBuffer = 0;
            vboTextureIndexesBuffer = 0;
            m_entity = va.m_entity;
            m_normals = va.m_normals;
            m_locals = va.m_locals;
            m_vertices = va.m_vertices;
            m_primitiveType = va.m_primitiveType;
            oldVerticesSize = va.oldVerticesSize;
            oldIndexesSize = va.oldIndexesSize;
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
            needToUpdateVertexBuffer = true;
            needToUpdateIndexBuffer = true;
        }
        VertexBuffer::VertexBuffer(const VertexBuffer&& va) {
            vboVertexBuffer = 0;
            vboNormalBuffer = 0;
            vboIndexBuffer = 0;
            vboTextureIndexesBuffer = 0;
            m_entity = va.m_entity;
            m_normals = va.m_normals;
            m_locals = va.m_locals;
            m_vertices = va.m_vertices;
            m_primitiveType = va.m_primitiveType;
            oldVerticesSize = va.oldVerticesSize;
            oldIndexesSize = va.oldIndexesSize;
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
            needToUpdateVertexBuffer = true;
            needToUpdateIndexBuffer = true;
        }
        VertexBuffer& VertexBuffer::operator= (const VertexBuffer& va) {
            vboVertexBuffer = 0;
            vboNormalBuffer = 0;
            vboIndexBuffer = 0;
            vboTextureIndexesBuffer = 0;
            m_entity = va.m_entity;
            m_normals = va.m_normals;
            m_locals = va.m_locals;
            m_vertices = va.m_vertices;
            m_primitiveType = va.m_primitiveType;
            oldVerticesSize = va.oldVerticesSize;
            oldIndexesSize = va.oldIndexesSize;
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
            needToUpdateVertexBuffer = true;
            needToUpdateIndexBuffer = true;
            return *this;
        }
        VertexBuffer& VertexBuffer::operator= (const VertexBuffer&& va) {
            vboVertexBuffer = 0;
            vboNormalBuffer = 0;
            vboIndexBuffer = 0;
            vboTextureIndexesBuffer = 0;
            m_entity = va.m_entity;
            m_normals = va.m_normals;
            m_locals = va.m_locals;
            m_vertices = va.m_vertices;
            m_primitiveType = va.m_primitiveType;
            oldVerticesSize = va.oldVerticesSize;
            oldIndexesSize = va.oldIndexesSize;
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
            needToUpdateVertexBuffer = true;
            needToUpdateIndexBuffer = true;
            return *this;
        }
        void VertexBuffer::computeNormals() {
            unsigned int size = 0;
            if (m_primitiveType == sf::PrimitiveType::Quads) {
                size = m_vertices.size() / 4;
            } else if (m_primitiveType == sf::PrimitiveType::Triangles) {
                size = m_vertices.size() / 3;
            } else if (m_primitiveType == sf::PrimitiveType::TriangleStrip || m_primitiveType == sf::PrimitiveType::TriangleFan) {
                size = (m_vertices.size() > 2) ? m_vertices.size() - 2 : 0;
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
            needToUpdateVertexBuffer = true;
            needToUpdateIndexBuffer = true;
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
                    needToUpdateVertexBuffer = true;            //
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
        void VertexBuffer::update() {
            computeNormals();
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
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboVertexBuffer));
                            glCheck(glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0], GL_DYNAMIC_DRAW));
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
                    oldVerticesSize = m_vertices.size();
                    oldIndexesSize = m_indexes.size();
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



