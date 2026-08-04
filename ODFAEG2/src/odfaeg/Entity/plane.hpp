#ifndef ODFAEG_PLANE_HPP
#define ODFAEG_PLANE_HPP
#include <utility>
#include <string>
#include "../Math/vec.hpp"
#include "../Entity/rect.hpp"
#include "../Entity/gameObject.hpp"
namespace odfaeg {
    namespace entity {
        class Plane : public GameObject {
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
#endif
