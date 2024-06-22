#ifndef PLANE_H
#define PLANE_H
#include "vec4.h"
namespace odfaeg {
    namespace math {
        class Plane {
        public :
            Plane() {}
            Plane (Vec3f n, Vec3f p) : n(n), p(p) {
                a = n.x;
                b = n.y;
                c = n.z;
                d = -(a * p.x + b * p.y + c * p.z);
            }
            float whichSide (Vec3f other) {
                return a * other.x + b * other.y + c * other.z + d;
            }
            bool intersects(Ray &r, Vec3f& intersection) {
                float v1 = a * r.getOrig().x + b * r.getOrig().y + c * r.getOrig().z + d;
                float v2 = a * r.getDir().x + b * r.getDir().y + c * r.getDir().z;
                if (v2 == 0)
                    return false;
                float t = v1 / v2;
                Vec3f di = Vec3f(r.getOrig().x + r.getDir().x * t, r.getOrig().y + r.getDir().y * t, r.getOrig().z + r.getDir().z * t);
                float l1 = (r.getExt() - r.getOrig()).magnSquared();
                float l2 = (di - r.getOrig()).magnSquared();
                if((l2 / l1) >= 0 && (l2 / l1) <= 1) {
                    intersection = di;
                    return true;
                }
                return false;
            }
            bool intersects(Ray& r, float& i) {
                float d = n.dot2(r.getDir());
                if (d == 0) {
                    i = -1;
                    if (whichSide(r.getOrig()) == 0) {
                        return true;
                    }
                    return false;
                }
                i = n.dot2(p - r.getOrig()) / d;
                if (i < 0)
                    return false;
                return true;
            }
            float computeDist(Vec3f point) {
                Vec3f v = point - p;
                return Math::abs(v.dot2(n.normalize()));
            }
            Vec3f project(Vec3f point) {
                Vec3f v = point - p;
                float dist = v.dot2(n.normalize());
                return point - n * dist;
            }
            bool isParallelConf (Plane& plane) {
                 float dp = n.dot2(plane.n);
                 float v = plane.a * p.x + plane.b * p.y + plane.c * p.z + plane.d;
                 return (dp == 1 || dp == -1) && v == 0;
            }
            bool isParallel (Plane& plane) {
                 float dp = n.dot2(plane.n);
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
