module;
export module odfaeg.graphic.cube;
import odfaeg.graphic.gameObject;
import odfaeg.graphic.color;
import odfaeg.graphic.rect;
import odfaeg.graphic.texture;
import odfaeg.math.vec;
namespace odfaeg {
    namespace graphic {
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
            void setTexture(const Texture* texture);
        private:
            Color m_color;
            FloatRect m_textRect;
            Texture* m_texture;
        };
    }
}