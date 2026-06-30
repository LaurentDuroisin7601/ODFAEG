module;
export module odfaeg.graphic.plane;
import odfaeg.graphic.gameObject;
import odfaeg.graphic.rect;
import odfaeg.math.vec;
import odfaeg.graphic.texture;
namespace odfaeg {
    namespace graphic {
        export class Plane : public GameObject {
        public :
            Plane(math::Vec3f position, math::Vec3f size);
            void setTexCoords(FloatRect texCoords);
            GameObject* clone();
            void setTexture(Texture* texture);
        private :
            Texture* texture;
        };
    }
}

