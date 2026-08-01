#include <math.h>
#include "plane.hpp"
#include "vec.hpp"
#include "ray.hpp"
namespace odfaeg {
    namespace math {
        class Triangle {
        public:
            Triangle(Vec3f p1, Vec3f p2, Vec3f p3);
            bool intersects(Ray& ray);
            bool intersectsWhere(Ray& ray, math::Vec3f& i1, Vec3f& i2);
            bool intersects(Triangle other);
            Vec3f getP1();
            Vec3f getP2();
            Vec3f getP3();
        private:
            Vec3f p1, p2, p3;
        };
    }
}
#include "triangle.inl"