#ifndef ODFAEG_LIGHT_2D_HPP
#define ODFAEG_LIGHT_2D_HPP
#include "../../../include/odfaeg/Math/vec4.h"
#include <SFML/Graphics.hpp>
#include "export.hpp"
namespace odfaeg {
    namespace graphic {
class ODFAEG_GRAPHICS_API Light : public core::Registered<Light> {
    protected :
        Light (math::Vec3f center, int height, sf::Color color) {
            this->height = height;
            this->center = center;
            this->color = color;
        }
    public :
        template <typename Archive>
        void vtserialize(Archive & ar) {
            ar(height);
            ar(center);
            ar(color.r);
            ar(color.g);
            ar(color.b);
            ar(color.a);
        }
        void setColor (sf::Color color) {
            this->color = color;
        }
        sf::Color getColor () {
            return color;
        }
        void setLightCenter(math::Vec3f center) {
            this->center = center;
        }
        math::Vec3f getLightCenter() {
            return center;
        }
        int getHeight() {
            return height;
        }
        void setHeight(int height) {
            this->height = height;
        }
        virtual int getLightId() = 0;
    protected :
        int height;
        math::Vec3f center;
        sf::Color color;
};
}
}
#endif
