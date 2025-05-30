#include "../../../include/odfaeg/Graphics/vertex.h"

namespace odfaeg {
    namespace graphic {
        using namespace sf;
        Vertex::Vertex() :
        position (0, 0, 0),
        color    (255, 255, 255),
        texCoords(0, 0),
        normal(0, 0, 0)
        {
            for (unsigned int i = 0; i < MAX_BONE_INFLUENCE; i++) {
                m_BoneIDs[i] = -1;
            }
            for (unsigned int i = 0; i < MAX_BONE_INFLUENCE; i++) {
                 m_Weights[i] = 0;
            }
        }


        ////////////////////////////////////////////////////////////
        Vertex::Vertex(const Vector3f& thePosition) :
        position (thePosition),
        color    (255, 255, 255),
        texCoords(0, 0),
        normal(0, 0, 0)
        {
            for (unsigned int i = 0; i < MAX_BONE_INFLUENCE; i++) {
                m_BoneIDs[i] = -1;
            }
            for (unsigned int i = 0; i < MAX_BONE_INFLUENCE; i++) {
                 m_Weights[i] = 0;
            }
        }


        ////////////////////////////////////////////////////////////
        Vertex::Vertex(const Vector3f& thePosition, const Color& theColor) :
        position (thePosition),
        color    (theColor),
        texCoords(0, 0),
        normal(0, 0, 0)
        {
            for (unsigned int i = 0; i < MAX_BONE_INFLUENCE; i++) {
                m_BoneIDs[i] = -1;
            }
            for (unsigned int i = 0; i < MAX_BONE_INFLUENCE; i++) {
                m_Weights[i] = 0;
            }
        }


        ////////////////////////////////////////////////////////////
        Vertex::Vertex(const Vector3f& thePosition, const Vector2f& theTexCoords) :
        position (thePosition),
        color    (255, 255, 255),
        texCoords(theTexCoords),
        normal(0, 0, 0)
        {
            for (unsigned int i = 0; i < MAX_BONE_INFLUENCE; i++) {
                m_BoneIDs[i] = -1;
            }
            for (unsigned int i = 0; i < MAX_BONE_INFLUENCE; i++) {
                 m_Weights[i] = 0;
            }
        }


        ////////////////////////////////////////////////////////////
        Vertex::Vertex(const Vector3f& thePosition, const Color& theColor, const Vector2f& theTexCoords) :
        position (thePosition),
        color    (theColor),
        texCoords(theTexCoords),
        normal(0, 0, 0)
        {
            for (unsigned int i = 0; i < MAX_BONE_INFLUENCE; i++) {
                m_BoneIDs[i] = -1;
            }
            for (unsigned int i = 0; i < MAX_BONE_INFLUENCE; i++) {
                 m_Weights[i] = 0;
            }
        }
        bool Vertex::operator== (const Vertex& other) const {
            return position == other.position && color == other.color && texCoords == other.texCoords && normal == other.normal;
        }
        bool Vertex::operator!= (const Vertex& other) const {
            return !(*this == other);
        }
    }
} // namespace sf



