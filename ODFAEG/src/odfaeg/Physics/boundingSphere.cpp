#include "../../../include/odfaeg/Physics/boundingSphere.h"
#include "../../../include/odfaeg/Physics/boundingEllipsoid.h"
#include "../../../include/odfaeg/Physics/boundingBox.h"
#include "../../../include/odfaeg/Physics/orientedBoundingBox.h"
#include "../../../include/odfaeg/Physics/boundingPolyhedron.h"
#include "../../../include/odfaeg/Math/transformMatrix.h"
using namespace std;
namespace odfaeg {
    namespace physic {
        /*Crée un cercle englobant, utilisé pour les détection de collisions, et la sélection
        * d'objets.
        */
        //Crée un cercle de centre center et de rayon radius.
        BoundingSphere::BoundingSphere() {
            center = math::Vec3f(0, 0, 0);
            radius = 1;
        }
        BoundingSphere::BoundingSphere (math::Vec3f center, float radius) {
            this->center = center;
            this->radius = radius;
        }
        bool BoundingSphere::intersects (BoundingSphere &other, CollisionResultSet::Info& info) {
            float d = center.computeDist(other.center);
            float rSum = radius + other.radius;
            return (d <= rSum);
        }
        bool BoundingSphere::intersects (BoundingEllipsoid &be, CollisionResultSet::Info& info) {
            return be.intersects(*this, info);
        }
        bool BoundingSphere::intersects (BoundingBox &bx, CollisionResultSet::Info& info) {
            float dist;
            int pIndex, eIndex, fIndex;
            std::vector<math::Vec3f> point = {center};
            int vIndex = math::Computer::checkNearestVertexFromShape(bx.getCenter(), bx.getVertices(), bx.getEdgeBissectors(), bx.getEdgeNormals(),
                                                                     bx.getFaceBissectors(), bx.getFaceNormals(),point,dist,pIndex,eIndex,fIndex,4);
            info.nearestVertexIndex1 = vIndex;
            info.nearestPtIndex1 = pIndex;
            info.nearestEdgeIndex1 = eIndex;
            info.nearestFaceIndex1 = fIndex;
            math::Vec3f v = center - bx.getCenter();
            if (pIndex != -1)
                return v.magnitude() <= radius + (bx.getVertices()[pIndex] - bx.getCenter()).magnitude();
            math::Vec3f n = bx.getEdgeNormals()[eIndex];
            float pn = math::Math::abs(v.projOnAxis(n));
            info.mtu = n * (radius + (bx.getEdgeBissectors()[eIndex] - bx.getCenter()).magnitude() - pn);
            return pn  <= radius  + (bx.getEdgeBissectors()[eIndex] - bx.getCenter()).magnitude();
        }

        bool BoundingSphere::intersects (OrientedBoundingBox &obx, CollisionResultSet::Info& info) {
             math::Ray ray(obx.getCenter(), center);
             math::Vec3f _near, _far;
             if (!obx.intersectsWhere(ray, _near, _far, info)) {
                return false;
             }
             math::Vec3f d = _far - obx.getCenter();
             return (center.computeDistSquared(obx.getCenter()) - radius * radius - d.magnSquared()) <= 0;
        }
        bool BoundingSphere::intersects (BoundingPolyhedron &bp, CollisionResultSet::Info& info) {
             math::Ray ray(bp.getCenter(), center);
             math::Vec3f _near, _far;
             if (!bp.intersectsWhere(ray, _near, _far, info)) {
                return false;
             }
             math::Vec3f d = _far - bp.getCenter();
             return (center.computeDistSquared(bp.getCenter()) - radius * radius - d.magnSquared()) <= 0;
        }
        bool BoundingSphere::intersects (math::Ray &ray, bool segment, CollisionResultSet::Info& info) {
            if (isPointInside(ray.getOrig()))
                return true;
            math::Vec3f d1 = center - ray.getOrig();
            float dp = d1.dot2(ray.getDir());
            if (dp < 0)
                return false;
            math::Vec3f p = ray.getOrig() + ray.getDir().normalize() * d1.projOnAxis(ray.getDir());
            math::Vec3f d2 = p - center;
            if (!segment) {
                return d2.magnSquared() <= radius * radius;
            } else {
                if(d2.magnSquared() > radius * radius) {
                    return false;
                } else {
                    math::Vec3f _near, _far;
                    intersectsWhere(ray, _near, _far, info);
                    math::Vec3f v1 = _near - ray.getOrig();
                    math::Vec3f v2 = _far - ray.getOrig();
                    math::Vec3f d = ray.getExt() - ray.getOrig();
                    float i1 = v1.magnSquared() / d.magnSquared();
                    float i2 = v2.magnSquared() / d.magnSquared();
                    if (i1 <= 1 || i2 <= 1)
                            return true;
                    return false;
                }
            }
        }
        bool BoundingSphere::intersectsWhere (math::Ray& r, math::Vec3f& _near, math::Vec3f &_far, CollisionResultSet::Info& info) {
            math::Vec3f p = r.getOrig();
            math::Vec3f d = r.getDir().normalize();
            math::Vec3f c = center;
             // this is the vector from p to c
            math::Vec3f vpc = c - p;
            if (vpc.dot2(d) < 0) {
                // when the sphere is behind the origin p
                // note that this case may be dismissed if it is
                // considered that p is outside the sphere
                if (vpc.magnSquared() > radius * radius) {
                    return false; // there is no intersection
                } else if (vpc.magnSquared() == radius * radius) {
                    _near = _far = p;
                    return true;
                } else {
                    // occurs when p is inside the sphere
                    math::Vec3f pc = p + d * vpc.projOnAxis(d);
                    float dist = math::Math::sqrt(radius * radius - pc.computeDistSquared(c));
                    float di1 = dist - pc.computeDist(p);
                    _near = p;
                    _far = p + d * di1;
                    return true;
                }
            } else {
                // center of sphere projects on the ray
                math::Vec3f pc = p + d * vpc.projOnAxis(d);
                if (c.computeDistSquared(pc) > radius * radius) {
                    // there is no intersection
                    return false;
                } else {
                    // distance from pc to i1 and to i2.
                    float dist = math::Math::sqrt((radius * radius) - pc.computeDistSquared(c));
                    float di1, di2;
                    if(vpc.magnSquared() > radius * radius) {
                        di1 = pc.computeDist(p) - dist;
                        _near = p + d * di1;
                        di2 = pc.computeDist(p) + dist;
                        _far = p + d * di2;
                    } else {
                        // origin is inside the sphere
                        di1 = pc.computeDist(p) + dist;
                        _near = p;
                        _far = p + d * di1;
                    }
                    return true;
                }
            }
        }
        //Test si un point est à l'intérieur du cercle.
        bool BoundingSphere::isPointInside (math::Vec3f &point) {
            float d = center.computeDist(point);
            return d <= radius;
        }
        math::Vec3f BoundingSphere::getPosition() {
            return math::Vec3f(center.x - radius, center.y - radius, center.z - radius);
        }
        math::Vec3f BoundingSphere::getCenter() {
            return center;
        }
        math::Vec3f BoundingSphere::getSize() {
            return math::Vec3f(radius * 2, radius * 2, radius * 2);
        }
        float BoundingSphere::getRadius () {
            return radius;
        }
        void BoundingSphere::scale(float s) {
            radius *= s;
        }
        void BoundingSphere::move(math::Vec3f t) {
            center += t;
        }
    }
}

