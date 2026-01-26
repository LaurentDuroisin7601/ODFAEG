#include "../../../../include/odfaeg/Graphics/2D/ambientLight.h"
namespace odfaeg {
    namespace graphic {
        namespace g2d {
            AmbientLight AmbientLight::ambientLight = AmbientLight(math::Vec3f(1000, 1000, 1000),math::Vec3f(-0.5, -1.0, -0.3), 800, 2500, Color(255, 255, 255));
            int AmbientLight::id = -1;
            void AmbientLight::setAmbientLight(math::Vec3f center, math::Vec3f dir, float radius, int height, Color color) {
                 ambientLight = AmbientLight (center, dir, radius, height, color);
            }
            AmbientLight& AmbientLight::getAmbientLight () {
              return ambientLight;
            }
        }
    }
}
