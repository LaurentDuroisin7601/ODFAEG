#ifndef PLANE_H
#define PLANE_H
#include "vec4.h"
#include "ray.h"
namespace odfaeg {
    namespace math {
        class ODFAEG_MATH_API Plane {
        public :
            Plane() {}
            Plane (Vec3f n, Vec3f p) : n(n), p(p) {
                a = n.x();
                b = n.y();
                c = n.z();
                d = -n.dot(p);
            }
            float whichSide (Vec3f other) {
                return a * other.x() + b * other.y() + c * other.z() + d;
            }
            bool intersects(Ray &r, Vec3f& intersection) {
                float denom = n.dot(r.getDir());

                // Prevent divide by zero:
                if (Math::abs(denom) <= 1e-4f)
                    return false;

                // If you want to ensure the ray reflects off only
                // the "top" half of the plane, use this instead:
                //
                // if (-denom <= 1e-4f)
                //     return std::nullopt;

                float t = -(n.dot(r.getOrig()) + d) / n.dot(r.getDir());
                // Use pointy end of the ray.
                // It is technically correct to compare t < 0,
                // but that may be undesirable in a raytracer.
                if (t <= 1e-4)
                    return false;
                intersection = r.getOrig() + r.getDir() * t;
                return true;
            }
            bool intersects(Ray& r, float& i) {
                float d = n.dot(r.getDir());
                if (d == 0) {
                    i = -1;
                    if (whichSide(r.getOrig()) == 0) {
                        return true;
                    }
                    return false;
                }
                i = n.dot(p - r.getOrig()) / d;
                if (i < 0)
                    return false;
                return true;
            }
            float computeDist(Vec3f point) {
                Vec3f v = point - p;
                return Math::abs(v.dot(n.normalize()));
            }
            Vec3f project(Vec3f point) {
                Vec3f v = point - p;
                float dist = v.dot(n.normalize());
                return point - n * dist;
            }
            bool isParallelConf (Plane& plane) {
                 float dp = n.dot(plane.n);
                 float v = plane.a * p.x() + plane.b * p.y() + plane.c * p.z() + plane.d;
                 return (dp == 1 || dp == -1) && v == 0;
            }
            bool isParallel (Plane& plane) {
                 float dp = n.dot(plane.n);
                 return (dp == 1 || dp == -1);
            }
        private :
            float a, b, c, d;
            Vec3f n;
            Vec3f p;
        };
    }
}
#endif // PLANE_H
