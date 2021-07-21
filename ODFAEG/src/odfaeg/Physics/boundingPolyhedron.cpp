#include "../../../include/odfaeg/Physics/boundingPolyhedron.h"
#include "../../../include/odfaeg/Physics/boundingSphere.h"
#include "../../../include/odfaeg/Physics/boundingEllipsoid.h"
#include "../../../include/odfaeg/Physics/boundingBox.h"
#include "../../../include/odfaeg/Physics/orientedBoundingBox.h"
#include "../../../include/odfaeg/Math/transformMatrix.h"
namespace odfaeg {
    namespace physic {
        BoundingPolyhedron::BoundingPolyhedron() {
            flat = true;
        }
        void BoundingPolyhedron::setFlat (bool flat) {
            this->flat = flat;
        }
        BoundingPolyhedron::BoundingPolyhedron (math::Vec3f p1, math::Vec3f p2, math::Vec3f p3, bool flat) {
            this->flat = flat;
            addTriangle(p1, p2, p3);
        }
        bool BoundingPolyhedron::intersects(math::Ray& ray, bool segment, CollisionResultSet::Info& info) {
            for (unsigned int i = 0; i < points.size(); i+=3) {
                   math::Triangle t (points[i], points[i+1], points[i+2]);
                   if(t.intersects(ray))
                    return true;
            }
            return false;
        }
        bool BoundingPolyhedron::intersectsWhere(math::Ray& ray, math::Vec3f& i1, math::Vec3f& i2, CollisionResultSet::Info& info) {
            for (unsigned int i = 0; i < points.size(); i+=3) {
                   math::Triangle t (points[i], points[i+1], points[i+2]);
                   if(t.intersectsWhere(ray, i1, i2))
                      return true;
            }
            return false;
        }
        void BoundingPolyhedron::addTriangle(math::Vec3f p1, math::Vec3f p2, math::Vec3f p3) {
            points.push_back(p1);
            points.push_back(p2);
            points.push_back(p3);
            computeVectors();
            std::array<std::array<float, 2>, 3> store = math::Computer::getExtends(points);
            size = math::Vec3f(store[0][1] - store[0][0], store[1][1] - store[1][0], store[2][1] - store[2][0]);
        }
        bool BoundingPolyhedron::intersects (BoundingSphere &bs, CollisionResultSet::Info& info) {
            return bs.intersects (*this, info);
        }
        bool BoundingPolyhedron::intersects (BoundingEllipsoid &be, CollisionResultSet::Info& info) {
            return be.intersects (*this, info);
        }
        bool BoundingPolyhedron::intersects (BoundingBox& bx, CollisionResultSet::Info& info) {
            return bx.intersects (*this, info);
        }
        bool BoundingPolyhedron::intersects (OrientedBoundingBox &obx, CollisionResultSet::Info& info) {
            return obx.intersects (*this, info);
        }
        bool BoundingPolyhedron::intersects (BoundingPolyhedron &bp, CollisionResultSet::Info& info) {
            float distMin1, distMin2;
            int ptIndex1, ptIndex2;
            int edgeIndex1, edgeIndex2;
            int faceIndex1, faceIndex2;
            /* Check the nearest vertex of the polygon 2 from the regions of polygon 2 and give the distance and
            *  return the point or the face's bissector of the region.
            */
            int vertexIndex1 = math::Computer::checkNearestVertexFromShape(center, points, edgeBissectors, edgeNormals, faceBissectors, faceNormals, bp.points, distMin1, ptIndex1, edgeIndex1, faceIndex1, 3);
            info.nearestVertexIndex1 = vertexIndex1;
            info.nearestPtIndex1 = ptIndex1;
            info.nearestEdgeIndex1 = edgeIndex1;
            info.nearestFaceIndex1 = faceIndex1;
            /* Check the nearest vertex of the polygon 1 from the regions of polygon 2 and give the distance and
            *  return the point or face's bissector of the region.
            */
            int vertexIndex2 = math::Computer::checkNearestVertexFromShape(bp.center, bp.points, bp.edgeBissectors, bp.edgeNormals, bp.faceBissectors, faceNormals, points, distMin2, ptIndex2, edgeIndex2, faceIndex2, 3);
            info.nearestVertexIndex2 = vertexIndex2;
            info.nearestPtIndex2 = ptIndex2;
            info.nearestEdgeIndex2 = edgeIndex2;
            info.nearestFaceIndex2 = faceIndex2;
            if (ptIndex1 != -1) {
                return (points[ptIndex1] - center).magnSquared() >= (bp.points[vertexIndex1] - center).magnSquared();
            }
            if (ptIndex2 != -1) {
                return (points[ptIndex2] - center).magnSquared() >= (bp.points[vertexIndex2] - center).magnSquared();
            }
            if (flat && bp.flat) {
                if (faceIndex1 != -1 && faceIndex2 != -1)  {
                    math::Triangle t1(points[faceIndex1*3], points[faceIndex1*3+1], points[faceIndex1*3+2]);
                    math::Triangle t2(bp.points[faceIndex1*3], bp.points[faceIndex1*3+1], bp.points[faceIndex1*3+2]);
                    return t1.intersects(t2);
                }
                if (faceIndex1 != -1 && faceIndex2 == -1) {
                    math::Triangle t(points[faceIndex1*3], points[faceIndex1*3+1], points[faceIndex1*3+2]);
                    math::Ray r1 (bp.points[edgeIndex2], bp.points[(edgeIndex2 % 3 == 2) ? edgeIndex2 - 2 : edgeIndex2 + 1]);
                    math::Ray r2 (bp.points[(edgeIndex2 % 3 == 2) ? edgeIndex2 - 2 : edgeIndex2 + 1], bp.points[edgeIndex2]);
                    return (t.intersects(r1) && t.intersects(r2));
                }
                if (faceIndex1 == -1 && faceIndex2 != -1) {
                    math::Triangle t(bp.points[faceIndex2*3], bp.points[faceIndex2*3+1], bp.points[faceIndex2*3+2]);
                    math::Ray r1 (points[edgeIndex1], points[(edgeIndex1 % 3 == 2) ? edgeIndex1 - 2 : edgeIndex1 + 1]);
                    math::Ray r2 (points[(edgeIndex1 % 3 == 2) ? edgeIndex1 - 2 : edgeIndex1 + 1], points[edgeIndex1]);
                    return (t.intersects(r1) && t.intersects(r2));
                }
                return false;
            }
            if (!flat && !bp.flat) {
                if (distMin1 < distMin2) {
                    if (faceIndex1 != -1) {
                        math::Vec3f bpn = (faceBissectors[faceIndex1] - center).projOnVector(faceNormals[faceIndex1]);
                        math::Vec3f d = points[vertexIndex1] - center;
                        float p = d.projOnAxis(bpn);
                        if (p * p > bpn.magnSquared()) {
                            return false;
                        }
                        return true;
                    }
                    math::Vec3f bpn = (edgeBissectors[edgeIndex1] - center).projOnVector(edgeNormals[edgeIndex1]);
                    math::Vec3f d = points[vertexIndex1] - center;
                    float p = d.projOnAxis(bpn);
                    return p * p > bpn.magnSquared();
                }
                if (faceIndex2 != -1) {
                    math::Vec3f bpn = (faceBissectors[faceIndex2] - center).projOnVector(faceNormals[faceIndex2]);
                    math::Vec3f d = points[vertexIndex2] - center;
                    float p = d.projOnAxis(bpn);
                    return p * p > bpn.magnSquared();
                }
                math::Vec3f bpn = (edgeBissectors[edgeIndex2] - center).projOnVector(edgeNormals[edgeIndex2]);
                math::Vec3f d = points[vertexIndex2] - center;
                float p = d.projOnAxis(bpn);
                return p * p > bpn.magnSquared();
            }
            if (flat && !bp.flat) {
                if (faceIndex1 != -1 && faceIndex2 != -1)  {
                    math::Triangle t1(points[faceIndex1*3], points[faceIndex1*3+1], points[faceIndex1*3+2]);
                    math::Triangle t2(bp.points[faceIndex2*3], bp.points[faceIndex2*3+1], bp.points[faceIndex2*3+2]);
                    return bp.isPointInside(t1.getP1())
                            || bp.isPointInside(t1.getP2())
                            || bp.isPointInside(t1.getP3())
                            || t1.intersects(t2);
                }
                if (faceIndex1 != -1 && faceIndex2 == -1) {
                    math::Triangle t(points[faceIndex1*3], points[faceIndex1*3+1], points[faceIndex1*3+2]);
                    math::Ray r1 (points[edgeIndex2], points[(edgeIndex2 % 3 == 2) ? edgeIndex2 - 2 : edgeIndex2 + 1]);
                    math::Ray r2 (points[(edgeIndex2 % 3 == 2) ? edgeIndex2 - 2 : edgeIndex2 + 1], points[edgeIndex2]);
                    return bp.isPointInside(t.getP1())
                           || bp.isPointInside(t.getP2())
                           || bp.isPointInside(t.getP3())
                           || (t.intersects(r1) && t.intersects(r2));
                }
                if (faceIndex1 == -1 && faceIndex2 != -1) {
                    math::Triangle t(points[faceIndex2*3], points[faceIndex2*3+1], points[faceIndex2*3+2]);
                    math::Ray r1 (points[edgeIndex1], points[(edgeIndex1 % 3 == 2) ? edgeIndex1 - 2 : edgeIndex1 + 1]);
                    math::Ray r2 (points[(edgeIndex1 % 3 == 2) ? edgeIndex1 - 2 : edgeIndex1 + 1], points[edgeIndex1]);
                    return bp.isPointInside(r1.getOrig())
                            || bp.isPointInside(r1.getExt())
                            || (t.intersects(r1) && t.intersects(r2));
                }
                return false;
            }
            if (!flat && bp.flat) {
                if (faceIndex1 != -1 && faceIndex2 != -1)  {
                    math::Triangle t1(points[faceIndex1*3], points[faceIndex1*3+1], points[faceIndex1*3+2]);
                    math::Triangle t2(bp.points[faceIndex2*3], bp.points[faceIndex2*3+1], bp.points[faceIndex2*3+2]);
                    return isPointInside(t2.getP1())
                            || bp.isPointInside(t2.getP2())
                            || bp.isPointInside(t2.getP3())
                            || t1.intersects(t2);
                }
                if (faceIndex1 != -1 && faceIndex2 == -1) {
                    math::Triangle t(points[faceIndex1*3], points[faceIndex1*3+1], points[faceIndex1*3+2]);
                    math::Ray r1 (points[edgeIndex2], points[(edgeIndex2 % 3 == 2) ? edgeIndex2 - 2 : edgeIndex2 + 1]);
                    math::Ray r2 (points[(edgeIndex2 % 3 == 2) ? edgeIndex2 - 2 : edgeIndex2 + 1], points[edgeIndex2]);
                    return bp.isPointInside(r1.getOrig())
                           || bp.isPointInside(r1.getExt())
                           || (t.intersects(r1) && t.intersects(r2));
                }
                if (faceIndex1 == -1 && faceIndex2 != -1) {
                    math::Triangle t(points[faceIndex2*3], points[faceIndex2*3+1], points[faceIndex2*3+2]);
                    math::Ray r1 (points[edgeIndex1], points[(edgeIndex1 % 3 == 2) ? edgeIndex1 - 2 : edgeIndex1 + 1]);
                    math::Ray r2 (points[(edgeIndex1 % 3 == 2) ? edgeIndex1 - 2 : edgeIndex1 + 1], points[edgeIndex1]);
                    return bp.isPointInside(t.getP1())
                            || bp.isPointInside(t.getP2())
                            || bp.isPointInside(t.getP3())
                            || (t.intersects(r1) && t.intersects(r2));
                }
                return false;
            }
        }
        bool BoundingPolyhedron::isPointInside(math::Vec3f point) {
            float distMin;
            int index;
            int edgeIndex, faceIndex;
            std::vector<math::Vec3f> vertices = {point};
            //Check the nearest face from the point.
            int ptIndex = math::Computer::checkNearestVertexFromShape(center, points, edgeBissectors, edgeNormals, faceBissectors, faceNormals, vertices, distMin, index,edgeIndex, faceIndex, 3);
            if (index != -1) {
                return (points[index] - center).magnSquared() >= (vertices[ptIndex] - center).magnSquared();
            }
            if (flat) {
                if (faceIndex == -1)
                    return false;
                math::Plane p(faceNormals[faceIndex], faceBissectors[faceIndex]);
                if (p.whichSide(point) == 0)
                    return true;
                return false;
            }

            math::Vec3f bpn = (faceBissectors[faceIndex] - center).projOnVector(faceNormals[faceIndex]);
            math::Vec3f d = point - center;
            float p = d.projOnAxis(bpn);
            if (p * p > bpn.magnSquared()) {
                return false;
            }
            return true;
        }
        bool BoundingPolyhedron::isFlat() {
            return flat;
        }
        int BoundingPolyhedron::nbPoints () {
            return points.size();
        }
        void BoundingPolyhedron::computeVectors () {
            math::Vec3f sum(0, 0, 0);
            std::vector<math::Vec3f> vertices = getPoints();
            for (unsigned int i = 0; i < vertices.size(); i++)
                sum += vertices[i];
            center = sum / vertices.size();
            faceNormals.clear();
            edgeNormals.clear();
            faceBissectors.clear();
            edgeBissectors.clear();
            for (unsigned int i = 0; i < points.size(); i+=3) {
                math::Vec3f a = points[i];
                math::Vec3f b = points[i+1];
                math::Vec3f c = points[i+2];
                math::Vec3f bissector = (a + b + c) / 3;
                math::Vec3f v1 = b - a;
                math::Vec3f v2 = c - b;
                math::Vec3f v3 = c - a;
                math::Vec3f fn = v1.cross(v2).normalize();
                if (fn.dot2(bissector - center) < 0)
                    fn = -fn;
                faceBissectors.push_back(bissector);
                faceNormals.push_back(fn);
                bissector = (a + b) * 0.5f;
                math::Vec3f e1n = v1.cross(fn).normalize();
                if (e1n.dot2(bissector - center) < 0)
                    e1n = -e1n;
                edgeBissectors.push_back(bissector);
                edgeNormals.push_back(e1n);
                bissector = (b + c) * 0.5f;
                math::Vec3f e2n = v2.cross(fn).normalize();
                if (e2n.dot2(bissector - center) < 0)
                    e2n = -e2n;
                edgeBissectors.push_back(bissector);
                edgeNormals.push_back(e2n);
                bissector = (c + a) * 0.5f;
                math::Vec3f e3n = v3.cross(fn).normalize();
                if (e3n.dot2(bissector - center) < 0)
                    e3n = -e3n;
                edgeBissectors.push_back(bissector);
                edgeNormals.push_back(e3n);
            }
        }
        std::vector<math::Vec3f> BoundingPolyhedron::getFaceNormals() {
            return faceNormals;
        }
        std::vector<math::Vec3f> BoundingPolyhedron::getEdgeNormals() {
            return edgeNormals;
        }
        std::vector<math::Vec3f> BoundingPolyhedron::getFaceBissectors() {
            return faceBissectors;
        }
        std::vector<math::Vec3f> BoundingPolyhedron::getEdgeBissectors() {
            return edgeBissectors;
        }
        std::vector<math::Vec3f> BoundingPolyhedron::getPoints() {
            std::vector<math::Vec3f> vertices;
            for (unsigned int i = 0; i < points.size(); i++) {
                bool contains = false;
                for (unsigned int j = 0; j < vertices.size() && !contains; j++) {
                    if (points[i] == vertices[j])
                        contains = true;
                }
                if (!contains) {
                    vertices.push_back(points[i]);
                }
            }
            return vertices;
        }
        BoundingPolyhedron::~BoundingPolyhedron () {
            faceNormals.clear();
            edgeNormals.clear();
            faceBissectors.clear();
            edgeBissectors.clear();
            points.clear();
        }
        bool BoundingPolyhedron::operator== (const BoundingPolyhedron &bp) {
            if(points.size() != bp.points.size())
                return false;
            for (unsigned int i = 0; i < points.size(); i++) {
                if (!(points[i] == bp.points[i])) {
                    return false;
                }
            }
            return true;
        }
        math::Vec3f BoundingPolyhedron::getPoint(unsigned  int index) {
            if (index >= 0 && index < points.size())
                return points[index];
            else
                return math::Vec3f (0, 0, 0);
        }

        math::Vec3f BoundingPolyhedron::getCenter () {
            return center;
        }
        math::Vec3f BoundingPolyhedron::getSize() {
            return size;
        }
        math::Vec3f BoundingPolyhedron::getPosition() {
            return math::Vec3f (center.x - size.x * 0.5f, center.y - size.y * 0.5f, center.z - size.z * 0.5f);
        }
        void BoundingPolyhedron::move(math::Vec3f v) {
            graphic::TransformMatrix tm;
            tm.setTranslation(v);
            tm.setOrigin(center);
            for (unsigned int i = 0; i < points.size(); i++)
                points[i] = tm.transform(points[i]);
            computeVectors();
            std::array<std::array<float, 2>, 3> store = math::Computer::getExtends(points);
            size = math::Vec3f(store[0][1] - store[0][0], store[1][1] - store[1][0], store[2][1] - store[2][0]);
        }
        void BoundingPolyhedron::scale(math::Vec3f s) {
            graphic::TransformMatrix tm;
            tm.setScale(s);
            tm.setOrigin(center);
            for (unsigned int i = 0; i < points.size(); i++)
                points[i] = tm.transform(points[i]);
            computeVectors();
            std::array<std::array<float, 2>, 3> store = math::Computer::getExtends(points);
            size = math::Vec3f(store[0][1] - store[0][0], store[1][1] - store[1][0], store[2][1] - store[2][0]);
        }
        void BoundingPolyhedron::rotate(float angle, math::Vec3f axis) {
            graphic::TransformMatrix tm;
            tm.setRotation(axis, angle);
            tm.setOrigin(center);
            for (unsigned int i = 0; i < points.size(); i++)
                points[i] = tm.transform(points[i]);
            computeVectors();
            std::array<std::array<float, 2>, 3> store = math::Computer::getExtends(points);
            size = math::Vec3f(store[0][1] - store[0][0], store[1][1] - store[1][0], store[2][1] - store[2][0]);
        }
    }
}


