#include "../../../include/odfaeg/Graphics/vertex.h"

namespace odfaeg {
    namespace graphic {

        Vertex::Vertex() :
        position (0, 0, 0),
        color    (255, 255, 255),
        texCoords(0, 0),
        normal(0, 0, 0),
        entityId(-1),
        particleId(-1)
        {
            for (unsigned int i = 0; i < MAX_BONE_INFLUENCE; i++) {
                m_BoneIDs[i] = -1;
            }
            for (unsigned int i = 0; i < MAX_BONE_INFLUENCE; i++) {
                 m_Weights[i] = 0;
            }
        }


        ////////////////////////////////////////////////////////////
        Vertex::Vertex(const math::Vec3f& thePosition) :
        position (thePosition),
        color    (255, 255, 255),
        texCoords(0, 0),
        normal(0, 0, 0),
        entityId(-1),
        particleId(-1)
        {
            for (unsigned int i = 0; i < MAX_BONE_INFLUENCE; i++) {
                m_BoneIDs[i] = -1;
            }
            for (unsigned int i = 0; i < MAX_BONE_INFLUENCE; i++) {
                 m_Weights[i] = 0;
            }
        }


        ////////////////////////////////////////////////////////////
        Vertex::Vertex(const math::Vec3f& thePosition, const Color& theColor) :
        position (thePosition),
        color    (theColor),
        texCoords(0, 0),
        normal(0, 0, 0),
        entityId(-1),
        particleId(-1)
        {
            for (unsigned int i = 0; i < MAX_BONE_INFLUENCE; i++) {
                m_BoneIDs[i] = -1;
            }
            for (unsigned int i = 0; i < MAX_BONE_INFLUENCE; i++) {
                m_Weights[i] = 0;
            }
        }


        ////////////////////////////////////////////////////////////
        Vertex::Vertex(const math::Vec3f& thePosition, const math::Vec2f& theTexCoords) :
        position (thePosition),
        color    (255, 255, 255),
        texCoords(theTexCoords),
        normal(0, 0, 0),
        entityId(-1),
        particleId(-1)
        {
            for (unsigned int i = 0; i < MAX_BONE_INFLUENCE; i++) {
                m_BoneIDs[i] = -1;
            }
            for (unsigned int i = 0; i < MAX_BONE_INFLUENCE; i++) {
                 m_Weights[i] = 0;
            }
        }


        ////////////////////////////////////////////////////////////
        Vertex::Vertex(const math::Vec3f& thePosition, const Color& theColor, const math::Vec2f& theTexCoords) :
        position (thePosition),
        color    (theColor),
        texCoords(theTexCoords),
        normal(0, 0, 0),
        entityId(-1),
        particleId(-1)
        {
            for (unsigned int i = 0; i < MAX_BONE_INFLUENCE; i++) {
                m_BoneIDs[i] = -1;
            }
            for (unsigned int i = 0; i < MAX_BONE_INFLUENCE; i++) {
                 m_Weights[i] = 0;
            }
        }
        bool Vertex::operator== (const Vertex& other) {
            return position == other.position && color == other.color && texCoords == other.texCoords && normal == other.normal;
        }
        bool Vertex::operator!= (const Vertex& other) {
            return !(*this == other);
        }
    }
} // namespace sf



