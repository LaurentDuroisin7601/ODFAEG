#include "../../../include/odfaeg/Graphics/vertex.h"

namespace odfaeg {
    namespace graphic {
        using namespace sf;
        Vertex::Vertex() :
        position (0, 0, 0),
        color    (255, 255, 255),
        texCoords(0, 0)
        {
        }


        ////////////////////////////////////////////////////////////
        Vertex::Vertex(const Vector3f& thePosition) :
        position (thePosition),
        color    (255, 255, 255),
        texCoords(0, 0)
        {
        }


        ////////////////////////////////////////////////////////////
        Vertex::Vertex(const Vector3f& thePosition, const Color& theColor) :
        position (thePosition),
        color    (theColor),
        texCoords(0, 0)
        {
        }


        ////////////////////////////////////////////////////////////
        Vertex::Vertex(const Vector3f& thePosition, const Vector2f& theTexCoords) :
        position (thePosition),
        color    (255, 255, 255),
        texCoords(theTexCoords)
        {
        }


        ////////////////////////////////////////////////////////////
        Vertex::Vertex(const Vector3f& thePosition, const Color& theColor, const Vector2f& theTexCoords) :
        position (thePosition),
        color    (theColor),
        texCoords(theTexCoords)
        {
        }
        bool Vertex::operator== (const Vertex& other) const {
            return position == other.position && color == other.color && texCoords == other.texCoords;
        }
        bool Vertex::operator!= (const Vertex& other) const {
            return !(*this == other);
        }
    }
} // namespace sf



