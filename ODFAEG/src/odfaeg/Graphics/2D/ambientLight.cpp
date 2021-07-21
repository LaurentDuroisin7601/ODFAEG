#include "../../../../include/odfaeg/Graphics/2D/ambientLight.h"
namespace odfaeg {
    namespace graphic {
        namespace g2d {
            AmbientLight AmbientLight::ambientLight = AmbientLight(math::Vec3f(0, 1000, 1000),2500, sf::Color(255, 255, 255));
            int AmbientLight::id = -1;
            void AmbientLight::setAmbientLight(math::Vec3f center, int height, sf::Color color) {
                 ambientLight = AmbientLight (center, height, color);
            }
            AmbientLight& AmbientLight::getAmbientLight () {
              return ambientLight;
            }
        }
    }
}
