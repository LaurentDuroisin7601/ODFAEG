module;
#include <string>
export module odfaeg.entity.plane;
import odfaeg.entity.gameObject;
import odfaeg.entity.rect;
import odfaeg.math.vec;
namespace odfaeg {
    namespace entity {
        export class Plane : public GameObject {
        public :
            Plane(math::Vec3f position, math::Vec3f size);
            void setTexCoords(FloatRect texCoords);
            GameObject* clone();
            void setTexture(std::string textureId);
        private :
            std::string textureId;
        };
    }
}

