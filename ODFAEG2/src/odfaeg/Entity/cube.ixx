module;
#include <string>
export module odfaeg.entity.cube;
import odfaeg.entity.gameObject;
import odfaeg.entity.color;
import odfaeg.entity.rect;
import odfaeg.math.vec;
namespace odfaeg {
    namespace entity {
        export class Cube : public GameObject {
        public :
            Cube (math::Vec3f position, float w, float h, float d, Color color);
            bool operator== (GameObject& other) {
                if (!GameObject::operator==(other))
                    return false;
                return true;
            }
            void setTexCoords (FloatRect texRect);
            GameObject* clone();
            Color getColor();
            void setTexture(std::string textureId);
        private:
            Color m_color;
            FloatRect m_textRect;
            std::string m_textureId;            
        };
    }
}