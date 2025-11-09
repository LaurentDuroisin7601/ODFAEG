#include "../../../include/odfaeg/Physics/boundingCircle.hpp"
namespace odfaeg {
    namespace physic {
        BoundingCircle::BoundingCircle (math::Vec3f center, float radius, math::Vec3f normal) : center(center), radius(radius), normal(normal) {

        }
        bool BoundingCircle::intersects(math::Ray ray) {
            math::Plane p (normal, center);
            math::Vec3f inters;
            if (!p.intersects(ray, inters)) {
                return false;
            }
            if (inters.computeDistSquared(center) > radius * radius) {
                return false;
            }
            return true;
        }
        bool BoundingCircle::intersectsWhere(math::Ray ray, math::Vec3f& inters) {
            math::Plane p (normal, center);
            if (!p.intersects(ray, inters)) {
                return false;
            }
            if (inters.computeDistSquared(center) > radius * radius) {
                return false;
            }
            return true;
        }
    }
}
