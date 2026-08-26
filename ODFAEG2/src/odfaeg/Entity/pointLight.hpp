#ifndef ODFAEG_POINT_LIGHT_HPP
#define ODFAEG_POINT_LIGHT_HPP
#include "gameObject.hpp"
namespace odfaeg {
    namespace entity {
        class PointLight : public GameObject {
            public :
                PointLight(math::Vec3f position, Color color = entity::Color::Yellow);
                void setLightSpace(std::array<math::Matrix4f, 6> lightSpace);
                std::array<math::Matrix4f, 6> getLightSpace();
                void setShadowMapId(int shadowMapId);
                int getShadowMapId();
                GameObject* clone();
                math::Vec3f getPosition();
            private :
                int shadowMapId;
                math::Vec3f position;
                Color color;
                std::array<math::Matrix4f, 6> lightSpace;
        };
    }
}
#endif