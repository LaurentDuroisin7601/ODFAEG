#ifndef BOUNDING_CIRCLE_HPP
#define BOUNDING_CIRCLE_HPP
#include "../Math/plane.h"
#include "export.hpp"
namespace odfaeg {
    namespace physic {
        class ODFAEG_PHYSICS_API BoundingCircle {
        public :
            BoundingCircle(math::Vec3f center = math::Vec3f(0, 0, 0), float radius = 1, math::Vec3f normal = math::Vec3f(0, 0, 1));
            bool intersects(math::Ray ray);
            bool intersectsWhere(math::Ray ray, math::Vec3f& inters);
        private :
            math::Vec3f center, normal;
            float radius;
        };
    }
}
#endif // BOUNDING_CIRCLE_HPP
