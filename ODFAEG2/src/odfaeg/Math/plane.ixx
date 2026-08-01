module;
export module odfaeg.math.plane;
import odfaeg.math.vec;
import odfaeg.math.ray;
import odfaeg.math.maths;
export namespace odfaeg {
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
module : private;
#include "plane.inl"