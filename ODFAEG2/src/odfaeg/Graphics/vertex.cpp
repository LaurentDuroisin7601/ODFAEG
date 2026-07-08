module;
#include "../../../include/odfaeg/config.hpp"
//import odfaeg.graphic.vertex;
module odfaeg.graphic.vertex;
import odfaeg.graphic.color;
import odfaeg.math.vec;
namespace odfaeg {
    namespace graphic {
        Vertex::Vertex() :
            position(0.f, 0.f, 0.f),
            color(255, 255, 255),
            texCoords(0.f, 0.f),
            normal(0.f, 0.f, 0.f)            
        {
            for (unsigned int i = 0; i < MAX_BONES_INFLUENCE; i++) {
                m_BoneIDs[i] = -1;
                m_Weights[i] = 0;
            }
        }


        ////////////////////////////////////////////////////////////
        Vertex::Vertex(const math::Vec3f& thePosition) :
            position(thePosition),
            color(255, 255, 255),
            texCoords(0.f, 0.f),
            normal(0.f, 0.f, 0.f)
        {
            for (unsigned int i = 0; i < MAX_BONES_INFLUENCE; i++) {
                m_BoneIDs[i] = -1;
                m_Weights[i] = 0;
            }
        }


        ////////////////////////////////////////////////////////////
        Vertex::Vertex(const math::Vec3f& thePosition, const Color& theColor) :
            position(thePosition),
            color(theColor),
            texCoords(0.f, 0.f),
            normal(0.f, 0.f, 0.f)
        {
            for (unsigned int i = 0; i < MAX_BONES_INFLUENCE; i++) {
                m_BoneIDs[i] = -1;
                m_Weights[i] = 0;
            }
        }


        ////////////////////////////////////////////////////////////
        Vertex::Vertex(const math::Vec3f& thePosition, const math::Vec2f& theTexCoords) :
            position(thePosition),
            color(255, 255, 255),
            texCoords(theTexCoords),
            normal(0.f, 0.f, 0.f)
        {
            for (unsigned int i = 0; i < MAX_BONES_INFLUENCE; i++) {
                m_BoneIDs[i] = -1;
                m_Weights[i] = 0;
            }
        }


        ////////////////////////////////////////////////////////////
        Vertex::Vertex(const math::Vec3f& thePosition, const Color& theColor, const math::Vec2f& theTexCoords) :
            position(thePosition),
            color(theColor),
            texCoords(theTexCoords),
            normal(0.f, 0.f, 0.f)
        {
            for (unsigned int i = 0; i < MAX_BONES_INFLUENCE; i++) {
                m_BoneIDs[i] = -1;
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