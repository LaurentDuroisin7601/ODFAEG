#include "../../../include/odfaeg/Math/segment.h"
namespace odfaeg {
    namespace math {
        Segment::Segment () : orig(0, 0), ext(0, 0), dir(0, 0){


        }
        Segment::Segment (Vec2f orig, Vec2f ext) : orig(orig), ext(ext), dir(ext - orig) {

        }

        void Segment::setOrig (Vec2f &orig) {
            this->orig = orig;
        }
        void Segment::setExt (Vec2f &ext) {
            this->ext = ext;
        }
        Vec2f& Segment::getOrig () {
            return orig;
        }
        Vec2f& Segment::getExt () {
            return ext;
        }
        Vec2f& Segment::getDir () {
            return dir;
        }
        float Segment::intersects (Segment &other) {
            double denom = dir.x * other.dir.y - dir.y * other.dir.x;

            if (denom == 0)
                return -1;
            float t = -(orig.x * other.dir.y - other.orig.x * other.dir.y - other.dir.x * orig.y + other.dir.x * other.orig.y) / denom;
            if (t < 0 || t >= 1)
                return 0;
            float u = -(-dir.x * orig.y + dir.x  * other.orig.y + dir.y * orig.x - dir.y * other.orig.x) / denom;
            if (u < 0 || u >= 1)
                return 0;
            return 1;
        }
        float Segment::intersectsWhere (Segment &other) {

            Vec2f ap = other.ext - orig;
            Vec2f ao = other.orig - orig;


            if ((dir.x * ap.y - dir.y * ap.x) * (dir.x * ao.y - dir.y * ao.x) >= 0) {

                return -1.f;
            }
            float k = -(orig.x * other.dir.y - other.orig.x * other.dir.y - other.dir.x * orig.y + other.dir.x * other.orig.y) / (dir.x * other.dir.y - dir.y * other.dir.x);

            if (k < 0 || k > 1)
                return -1.f;

            return k;
        }
        float Segment::intersectWhereOther (Segment &other) {
            Vec2f ap = other.ext - orig;
            Vec2f ao = other.orig - orig;
            if ((dir.x * ap.y - dir.y * ap.x) * (dir.x * ao.y - dir.y * ao.x) >= 0) {

                return -1.f;
            }
            float l = -(-dir.x * orig.y + dir.x * other.orig.y + dir.y * orig.x - dir.y * other.orig.x) / (dir.x * other.dir.y - dir.y * other.dir.x);
            if (l < 0 || l > 1)
                return -1.f;

            return l;
        }
    }
}



