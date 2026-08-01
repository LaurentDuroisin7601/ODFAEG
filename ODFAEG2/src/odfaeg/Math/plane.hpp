#include "vec.hpp"
#include "ray.hpp"
namespace odfaeg {
    namespace math {
        class Plane {
        public:
            Plane();
            Plane(Vec3f n, Vec3f p);
            float whichSide(Vec3f other);
            bool intersects(Ray& r, Vec3f& intersection);                
            bool intersects(Ray& r, float& i);
            float computeDist(Vec3f point);
            Vec3f project(Vec3f point);
            bool isParallelConf(Plane& plane;
            bool isParallel(Plane& plane);
        private:
            float a, b, c, d;
            Vec3f n;
            Vec3f p;
        };
    }
}
#include "plane.inl"