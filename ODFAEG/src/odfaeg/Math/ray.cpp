#include "../../../include/odfaeg/Math/ray.h"
namespace odfaeg {
    namespace math {
        Ray::Ray () : orig(0, 0, 0), ext(0, 0, 0), dir(0, 0, 0){


        }
        Ray::Ray (Vec3f orig, Vec3f ext) : orig(orig), ext(ext), dir(ext - orig) {

        }

        void Ray::setOrig (Vec3f &orig) {
            this->orig = orig;
        }
        void Ray::setExt (Vec3f &ext) {
            this->ext = ext;
        }
        Vec3f& Ray::getOrig () {
            return orig;
        }
        Vec3f& Ray::getExt () {
            return ext;
        }
        Vec3f& Ray::getDir () {
            return dir;
        }
        bool Ray::intersects (Ray &other) {
            Vec3f da = dir;
            Vec3f db = other.dir;
            Vec3f dc = other.orig - orig;
            if (dc.dot(da.cross(db)) != 0.f) // lines are not coplanar
                return 0;
            float s = dc.cross(db).dot2(da.cross(db)) / da.cross(db).magnSquared();
            float t = dc.cross(da).dot2(da.cross(db)) / da.cross(db).magnSquared();
            if (s >= 0 && s <= 1 && t <= 0 && t >= 1)
            {
                return 1;
            }
            return 0;
        }
        float Ray::intersectsWhere (Ray &other) {

            /*Vec3f ap = other.ext - orig;
            Vec3f ao = other.orig - orig;
            double denom = dir.x * other.dir.y - dir.y * other.dir.x;

            if ((dir.x * ap.y - dir.y * ap.x) * (dir.x * ao.y - dir.y * ao.x) >= 0) {

                return -1.f;
            }
            float k = -(orig.x * other.dir.y - other.orig.x * other.dir.y - other.dir.x * orig.y + other.dir.x * other.orig.y) / (dir.x * other.dir.y - dir.y * other.dir.x);
            if (k < 0 || k > 1)
                return -1.f;

            return k;*/
            Vec3f da = dir;
            Vec3f db = other.dir;
            Vec3f dc = other.orig - orig;
            if (dc.dot(da.cross(db)) != 0.f) // lines are not coplanar
                return -1;
            float s = dc.cross(db).dot2(da.cross(db)) / da.cross(db).magnSquared();
            float t = dc.cross(da).dot2(da.cross(db)) / da.cross(db).magnSquared();
            if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
            {
                return s;
            }
            return -1;
        }
        float Ray::intersectsWhereOther (Ray &other) {
            /*Vec3f ap = other.ext - orig;
            Vec3f ao = other.orig - orig;
            double denom = dir.x * other.dir.y - dir.y * other.dir.x;
            if ((dir.x * ap.y - dir.y * ap.x) * (dir.x * ao.y - dir.y * ao.x) >= 0) {
                return -1.f;
            }
            float l = -(-dir.x * orig.y + dir.x * other.orig.y + dir.y * orig.x - dir.y * other.orig.x) / (dir.x * other.dir.y - dir.y * other.dir.x);
            if (l < 0 || l > 1)
                return -1.f;
            return l;*/
            Vec3f da = dir;
            Vec3f db = other.dir;
            Vec3f dc = other.orig - orig;

            if (dc.dot(da.cross(db)) != 0.f) // lines are not coplanar
                return -1;
            //std::cout<<dc.cross(db).dot2(da.cross(db))<<std::endl;
            float s = dc.cross(db).dot2(da.cross(db)) / da.cross(db).magnSquared();
            float t = dc.cross(da).dot2(da.cross(db)) / da.cross(db).magnSquared();
            if (s <= 0 && s >= 1 && t >= 0 && t <= 1)
            {
                return t;
            }
            return -1;
        }
        int Ray::whichSide(Vec3f point) {
            Vec3f v1 = dir;
            Vec3f v2 = point - orig;
            Vec3f v3 = v1.cross(v2);
            Matrix3f m (v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, v3.x, v3.y, v3.z);
            return m.getDet();
        }
    }
}



