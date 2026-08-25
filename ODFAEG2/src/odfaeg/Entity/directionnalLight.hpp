#ifndef ODFAEG_DIRECTIONNAL_LIGHT_HPP
#define ODFAEG_DIRECTIONNAL_LIGHT_HPP
#include "gameObject.hpp"
#include "matrix.hpp"
#include "color.hpp"
namespace odfaeg {
    namespace entity {
        class DirectionnalLight : public GameObject {
            public :
                DirectionnalLight(math::Vec3f direction, Color color = entity::Color::Yellow);
                void setLightSpace(std::array<math::Matrix4f, NB_CASCADES+1> lightSpace);
                std::array<math::Matrix4f, NB_CASCADES+1> getLightSpace();
                void setShadowMapId(int shadowMapId);
                int getShadowMapId();
                math::Vec3f getDir();
                GameObject* clone();
            private :
                int shadowMapId;
                math::Vec3f dir;
                Color color;
                std::array<math::Matrix4f, NB_CASCADES+1> lightSpace;
        };
    }
}
#endif