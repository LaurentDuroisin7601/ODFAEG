#include <utility>
#include <string>
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
#include "plane.inl"

