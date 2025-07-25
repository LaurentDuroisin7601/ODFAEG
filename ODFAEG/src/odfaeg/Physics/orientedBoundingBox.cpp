#include "../../../include/odfaeg/Physics/boundingBox.h"
#include "../../../include/odfaeg/Physics/boundingSphere.h"
#include "../../../include/odfaeg/Physics/boundingEllipsoid.h"
#include "../../../include/odfaeg/Physics/orientedBoundingBox.h"
#include "../../../include/odfaeg/Physics/boundingPolyhedron.h"
#include "../../../include/odfaeg/Math/transformMatrix.h"
namespace odfaeg {
    namespace physic {
        OrientedBoundingBox::OrientedBoundingBox (math::Vec3f p1, math::Vec3f p2, math::Vec3f p3, math::Vec3f p4, math::Vec3f p5, math::Vec3f p6, math::Vec3f p7, math::Vec3f p8) {
            points.resize(24);
            std::array<math::Vec3f, 8> corners;
            std::array<math::Vec3f, 4> frontCorners;
            std::array<math::Vec3f, 4> backCorners;
            corners[0] = p1;
            corners[1] = p2;
            corners[2] = p3;
            corners[3] = p4;
            corners[4] = p5;
            corners[5] = p6;
            corners[6] = p7;
            corners[7] = p8;
            std::sort (corners.begin(), corners.end(), MonFoncteur());

            frontCorners[0] = corners[0];
            frontCorners[1] = corners[1];
            frontCorners[2] = corners[2];
            frontCorners[3] = corners[3];
            backCorners[0] = corners[4];
            backCorners[1] = corners[5];
            backCorners[2] = corners[6];
            backCorners[3] = corners[7];
            std::array<std::array<float, 2>, 3> store;
            store = math::Computer::getExtends(corners);
            x = store[0][0];
            y = store[1][0];
            z = store[2][0];
            store = math::Computer::getExtends(frontCorners);
            math::Vec3f position (store[0][0], store[1][0], store[2][0]);
            //int p1Index=0, p2Index=0,  p3Index=0, p4Index, p5Index=0, p6Index = 0, p7Index = 0, p8Index = 8;
            //Check the right order of the points to find the normals of the oriented bounding rectangle.
            /*for (unsigned int i = 0; i < 4; i++) {
                if (corners[i].x == position.x)
                    p1Index = i;
                else if (corners[i].y <= position.y && corners[i].x != store[0][1])
                    p2Index = i;
                else if (corners[i].y == store[1][1])
                    p4Index = i;
                else
                    p3Index = i;
            }*/

            //Check the right order of the points to find the normals of the oriented bounding rectangle.
            /*for (unsigned int i = 4; i < 8; i++) {
                ////std::cout<<(corners[i].y <= position.y && corners[i].x != store[0][1])<<std::endl;
                if (corners[i].x == position.x)
                    p5Index = i;
                else if (corners[i].y <= position.y && corners[i].x != store[0][1])
                    p6Index = i;
                else if (corners[i].y == store[1][1])
                    p7Index = i;
                else
                    p8Index = i;
            }*/
            //////std::cout<<"pt index 6 : "<<p6Index<<std::endl;
            for (unsigned int i = 0 ; i < 8; i++) {
                points[i] = corners[i];
                ////std::cout<<"point : "<<points[i]<<std::endl;
            }
            /*for (unsigned int i = 0; i < 8; i++)
                ////std::cout<<"points : "<<points[i]<<std::endl;*/

            points[8] = points[4];
            points[9] = points[5];
            points[10] = points[1];
            points[11] = points[0];
            points[12] = points[7];
            points[13] = points[6];
            points[14] = points[2];
            points[15] = points[3];
            points[16] = points[1];
            points[17] = points[5];
            points[18] = points[6];
            points[19] = points[2];
            points[20] = points[0];
            points[21] = points[4];
            points[22] = points[7];
            points[23] = points[3];
            flat = false;
            center = (points[0] + points[1] + points[2] + points[3]
                          + points[4] + points[5] + points[6] + points[7]) * 0.125f;
            ////std::cout<<"center : "<<center<<std::endl;
            computeVectors();
        }
        OrientedBoundingBox::OrientedBoundingBox(math::Vec3f p1, math::Vec3f p2, math::Vec3f p3, math::Vec3f p4) {
            points.resize(4);
            std::array<math::Vec3f, 4> corners;
            std::array<std::array<float, 2>, 3> store;
            store = math::Computer::getExtends(corners);
            x = store[0][0];
            y = store[1][0];
            z = store[2][0];
            width = store[0][1] - x;
            height = store[1][1] - y;
            depth = store[2][1] - z;
            math::Vec3f position = math::Vec3f (store[0][0], store[1][0], store[2][0]);
            for (unsigned int i = 0 ; i < 4; i++) {
                points[i] = corners[i];
                ////std::cout<<"point : "<<points[i]<<std::endl;
            }
            center = (points[0] + points[1] + points[2] + points[3]) * 0.25f;
            flat = true;
            computeVectors();
        }
        void OrientedBoundingBox::computeVectors() {
            edgeNormals.clear();
            edgeBissectors.clear();
            faceNormals.clear();
            faceBissectors.clear();
            if (depth == 0) {
                math::Vec3f v1 = points[1] - points[0];
                math::Vec3f v2 = points[2] - points[1];
                math::Vec3f v3 = points[3] - points[2];
                math::Vec3f v4 = points[3] - points[0];
                math::Vec3f n = v1.cross(v2).normalize();
                faceNormals.push_back(n);
                math::Vec3f n1 = n.cross(v1).normalize();
                math::Vec3f n2 = n.cross(v2).normalize();
                math::Vec3f n3 = n.cross(v3).normalize();
                math::Vec3f n4 = n.cross(v4).normalize();
                math::Vec3f b1 = (points[0] + points[1]) * 0.5f;
                if (n1.dot(b1 - center) < 0)
                    n1 = -n1;
                math::Vec3f b2 = (points[1] + points[2]) * 0.5f;
                if (n2.dot(b2 - center) < 0)
                    n2 = -n2;
                math::Vec3f b3 = (points[2] + points[3]) * 0.5f;
                if (n3.dot(b3 - center) < 0)
                    n3 = -n3;
                math::Vec3f b4 = (points[3] + points[0]) * 0.5f;
                if (n4.dot(b4 - center) < 0)
                    n4 = -n4;
                edgeNormals.push_back(n1);
                edgeNormals.push_back(n2);
                edgeNormals.push_back(n3);
                edgeNormals.push_back(n4);
                edgeBissectors.push_back(b1);
                edgeBissectors.push_back(b2);
                edgeBissectors.push_back(b3);
                edgeBissectors.push_back(b4);
                faceBissectors.push_back(center);
            } else {
                //Left face.
                math::Vec3f v1 = points[1] - points[5];
                math::Vec3f v2 = points[5] - points[6];
                math::Vec3f v3 = points[6] - points[2];
                math::Vec3f v4 = points[2] - points[1];
                math::Vec3f fb = (points[1] + points[2] + points[5] + points[6]) * 0.25f;
                math::Vec3f n = v1.cross(v2).normalize();
                if (n.dot(fb - center) < 0)
                    n = -n;
                faceNormals.push_back(n);
                math::Vec3f n1 = n.cross(v1).normalize();
                math::Vec3f n2 = n.cross(v2).normalize();
                math::Vec3f n3 = n.cross(v3).normalize();
                math::Vec3f n4 = n.cross(v4).normalize();
                math::Vec3f b1 = (points[1] + points[5]) * 0.5f;
                if (n1.dot(b1 - center) < 0)
                    n1 = -n1;
                math::Vec3f b2 = (points[5] + points[6]) * 0.5f;
                if (n2.dot(b2 - center) < 0)
                    n2 = -n2;
                math::Vec3f b3 = (points[6] + points[2]) * 0.5f;
                if (n3.dot(b3 - center) < 0)
                    n3 = -n3;
                math::Vec3f b4 = (points[2] + points[1]) * 0.5f;
                if (n4.dot(b4 - center) < 0)
                    n4 = -n4;
                math::Vec3f faceCenter = (points[1]  + points[2] + points[5] + points[6]) * 0.25f;
                width = (faceCenter - center).magnitude() * 2;
                edgeBissectors.push_back(b1);
                edgeBissectors.push_back(b2);
                edgeBissectors.push_back(b3);
                edgeBissectors.push_back(b4);
                edgeNormals.push_back(n1);
                edgeNormals.push_back(n2);
                edgeNormals.push_back(n3);
                edgeNormals.push_back(n4);
                faceBissectors.push_back(fb);
                //Top face.
                v1 = points[4] - points[5];
                v2 = points[5] - points[1];
                v3 = points[1] - points[0];
                v4 = points[0] - points[4];
                fb = (points[0] + points[1] + points[4] + points[5]) * 0.25f;
                n = v1.cross(v2).normalize();

                if (n.dot(fb - center) < 0)
                    n = -n;
                faceNormals.push_back(n);
                n1 = n.cross(v1).normalize();
                n2 = n.cross(v2).normalize();
                n3 = n.cross(v3).normalize();
                n4 = n.cross(v4).normalize();
                b1 = (points[4] + points[5]) * 0.5f;
                if (n1.dot(b1 - center) < 0)
                    n1 = -n1;
                b2 = (points[5] + points[1]) * 0.5f;
                if (n2.dot(b2 - center) < 0)
                    n2 = -n2;
                b3 = (points[1] + points[0]) * 0.5f;
                if (n3.dot(b3 - center) < 0)
                    n3 = -n3;
                b4 = (points[0] + points[4]) * 0.5f;
                if (n4.dot(b4 - center) < 0)
                    n4 = -n4;
                faceCenter = (points[0] + points[1] + points[4] + points[5]) * 0.25f;
                height = (faceCenter - center).magnitude() * 2;
                edgeBissectors.push_back(b1);
                edgeBissectors.push_back(b2);
                edgeBissectors.push_back(b3);
                edgeBissectors.push_back(b4);
                edgeNormals.push_back(n1);
                edgeNormals.push_back(n2);
                edgeNormals.push_back(n3);
                edgeNormals.push_back(n4);
                faceBissectors.push_back(fb);
                //Front face.
                v1 = points[1] - points[0];
                v2 = points[2] - points[1];
                v3 = points[3] - points[2];
                v4 = points[3] - points[0];
                fb = (points[0] + points[1] + points[2] + points[3]) * 0.25f;
                n = v1.cross(v2).normalize();
                if (n.dot(fb - center) < 0)
                    n = -n;
                faceNormals.push_back(n);
                n1 = n.cross(v1).normalize();
                n2 = n.cross(v2).normalize();
                n3 = n.cross(v3).normalize();
                n4 = n.cross(v4).normalize();
                b1 = (points[0] + points[1]) * 0.5f;
                if (n1.dot(b1 - center) < 0)
                    n1 = -n1;
                b2 = (points[1] + points[2]) * 0.5f;
                if (n2.dot(b2 - center) < 0)
                    n2 = -n2;
                b3 = (points[2] + points[3]) * 0.5f;
                if (n3.dot(b3 - center) < 0)
                    n3 = -n3;
                b4 = (points[3] + points[0]) * 0.5f;
                if (n4.dot(b4 - center) < 0)
                    n4 = -n4;
                faceCenter = (points[0]  + points[1] + points[2] + points[3]) * 0.25f;
                depth = (faceCenter - center).magnitude() * 2;
                edgeBissectors.push_back(b1);
                edgeBissectors.push_back(b2);
                edgeBissectors.push_back(b3);
                edgeBissectors.push_back(b4);
                edgeNormals.push_back(n1);
                edgeNormals.push_back(n2);
                edgeNormals.push_back(n3);
                edgeNormals.push_back(n4);
                faceBissectors.push_back(fb);
                //Right face.
                v1 = points[1] - points[5];
                v2 = points[5] - points[6];
                v3 = points[6] - points[2];
                v4 = points[2] - points[1];
                fb = (points[1] + points[2] + points[5] + points[6]) * 0.25f;
                n = v1.cross(v2).normalize();

                if (n.dot(fb - center) < 0)
                    n = -n;

                faceNormals.push_back(n);
                n1 = n.cross(v1).normalize();
                n2 = n.cross(v2).normalize();
                n3 = n.cross(v3).normalize();
                n4 = n.cross(v4).normalize();
                b1 = (points[1] + points[5]) * 0.5f;
                if (n1.dot(b1 - center) < 0)
                    n1 = -n1;
                b2 = (points[5] + points[6]) * 0.5f;
                if (n2.dot(b2 - center) < 0)
                    n2 = -n2;
                b3 = (points[6] + points[2]) * 0.5f;
                if (n3.dot(b3 - center) < 0)
                    n3 = -n3;
                b4 = (points[2] + points[1]) * 0.5f;
                if (n4.dot(b4 - center) < 0)
                    n4 = -n4;
                edgeBissectors.push_back(b1);
                edgeBissectors.push_back(b2);
                edgeBissectors.push_back(b3);
                edgeBissectors.push_back(b4);
                edgeNormals.push_back(n1);
                edgeNormals.push_back(n2);
                edgeNormals.push_back(n3);
                edgeNormals.push_back(n4);
                faceBissectors.push_back(fb);
                //Bottom face.
                v1 = points[7] - points[6];
                v2 = points[6] - points[2];
                v3 = points[2] - points[3];
                v4 = points[3] - points[7];
                fb = (points[2] + points[3] + points[6] + points[7]) * 0.25f;
                n = v1.cross(v2).normalize();
                if (n.dot(fb - center) < 0)
                    n = -n;
                faceNormals.push_back(n);
                n1 = n.cross(v1).normalize();
                n2 = n.cross(v2).normalize();
                n3 = n.cross(v3).normalize();
                n4 = n.cross(v4).normalize();
                b1 = (points[7] + points[6]) * 0.5f;
                if (n1.dot(b1 - center) < 0)
                    n1 = -n1;
                b2 = (points[6] + points[2]) * 0.5f;
                if (n2.dot(b2 - center) < 0)
                    n2 = -n2;
                b3 = (points[2] + points[3]) * 0.5f;
                if (n3.dot(b3 - center) < 0)
                    n3 = -n3;
                b4 = (points[3] + points[7]) * 0.5f;
                if (n4.dot(b4 - center) < 0)
                    n4 = -n4;
                edgeBissectors.push_back(b1);
                edgeBissectors.push_back(b2);
                edgeBissectors.push_back(b3);
                edgeBissectors.push_back(b4);
                edgeNormals.push_back(n1);
                edgeNormals.push_back(n2);
                edgeNormals.push_back(n3);
                edgeNormals.push_back(n4);
                faceBissectors.push_back(fb);
                //Back face.
                v1 = points[4] - points[5];
                v2 = points[5] - points[6];
                v3 = points[6] - points[7];
                v4 = points[7] - points[4];
                fb = (points[0] + points[4] + points[5] + points[6]) * 0.25f;
                n = v1.cross(v2).normalize();
                if (n.dot(fb - center) < 0)
                    n = -n;
                faceNormals.push_back(n);
                n1 = n.cross(v1).normalize();
                n2 = n.cross(v2).normalize();
                n3 = n.cross(v3).normalize();
                n4 = n.cross(v4).normalize();
                b1 = (points[4] + points[5]) * 0.5f;
                if (n1.dot(b1 - center) < 0)
                    n1 = -n1;
                b2 = (points[5] + points[6]) * 0.5f;
                if (n2.dot(b2 - center) < 0)
                    n2 = -n2;
                b3 = (points[6] + points[7]) * 0.5f;
                if (n3.dot(b3 - center) < 0)
                    n3 = -n3;
                b4 = (points[7] + points[4]) * 0.5f;
                if (n4.dot(b4 - center) < 0)
                    n4 = -n4;
                edgeBissectors.push_back(b1);
                edgeBissectors.push_back(b2);
                edgeBissectors.push_back(b3);
                edgeBissectors.push_back(b4);
                edgeNormals.push_back(n1);
                edgeNormals.push_back(n2);
                edgeNormals.push_back(n3);
                edgeNormals.push_back(n4);
                faceBissectors.push_back(fb);

            }
        }
        bool OrientedBoundingBox::intersects(BoundingSphere &bs, CollisionResultSet::Info& info) {
            return bs.intersects(*this, info);
        }
        bool OrientedBoundingBox::intersects(BoundingEllipsoid &be, CollisionResultSet::Info& info) {
            return be.intersects(*this, info);
        }
        bool OrientedBoundingBox::intersects (math::Ray& ray, bool segment, CollisionResultSet::Info& info) {
            float tFar = std::numeric_limits<float>::max();
            float tNear = std::numeric_limits<float>::min();
            int hi[3];
            if (flat) {
                hi[0] = (edgeBissectors[0] - center).magnitude();
                hi[1] = (edgeBissectors[1] - center).magnitude();
                hi[2] = 0;
            } else {
                hi[0] = (faceBissectors[0] - center).magnitude();
                hi[2] = (faceBissectors[2] - center).magnitude();
                hi[4] = (faceBissectors[4] - center).magnitude();
            }
            math::Vec3f bi[3];
            if (flat) {
                bi[0] = edgeNormals[0];
                bi[1] = edgeNormals[1];
                bi[2] = math::Vec3f(0, 0, 0);
            } else {
                bi[0] = faceNormals[0];
                bi[1] = faceNormals[2];
                bi[2] = faceNormals[4];
            }
            for (unsigned int i = 0; i < 3; i++) {
                math::Vec3f p1 = getCenter() + bi[i] * hi[i];
                math::Vec3f p2 = getCenter() - bi[i] * hi[i];
                float d1 = bi[i].dot(ray.getDir());
                float d2 = -bi[i].dot(ray.getDir());
                if (d1 != 0 && d2 !=0) {
                    float t1 = bi[i].dot(p1 - ray.getOrig()) / d1;
                    float t2 = -bi[i].dot(p2 - ray.getOrig()) / d2;
                    float tmp;
                    if (t1 > t2) {
                        tmp = t1;
                        t1 = t2;
                        t2 = tmp;
                    } else
                        tmp = t1;
                    if (t1 > tNear)
                        tNear = t1;
                    if (t2 < tFar)
                        tFar = t2;

                    if (tNear > tFar)
                        return false;

                    if(tFar < 0)
                        return false;
                }
            }
            return true;
        }
        bool OrientedBoundingBox::intersectsWhere (math::Ray& ray, math::Vec3f& i1, math::Vec3f& i2, CollisionResultSet::Info& info) {
            float tFar = std::numeric_limits<float>::max();
            float tNear = std::numeric_limits<float>::min();
            int hi[3];
            if (flat) {
                hi[0] = (edgeBissectors[0] - center).magnitude();
                hi[1] = (edgeBissectors[1] - center).magnitude();
                hi[2] = 0;
            } else {
                hi[0] = (faceBissectors[0] - center).magnitude();
                hi[2] = (faceBissectors[2] - center).magnitude();
                hi[4] = (faceBissectors[4] - center).magnitude();
            }
            math::Vec3f bi[3];
            if (flat) {
                bi[0] = edgeNormals[0];
                bi[1] = edgeNormals[1];
                bi[2] = math::Vec3f(0, 0, 0);
            } else {
                bi[0] = faceNormals[0];
                bi[1] = faceNormals[2];
                bi[2] = faceNormals[4];
            }
            for (unsigned int i = 0; i < 3; i++) {
                math::Vec3f p1 = getCenter() + bi[i] * hi[i];
                math::Vec3f p2 = getCenter() - bi[i] * hi[i];
                float d1 = bi[i].dot(ray.getDir());
                float d2 = -bi[i].dot(ray.getDir());
                if (d1 != 0 && d2 !=0) {
                    float t1 = bi[i].dot(p1 - ray.getOrig()) / d1;
                    float t2 = -bi[i].dot(p2 - ray.getOrig()) / d2;
                    float tmp;
                    if (t1 > t2) {
                        tmp = t1;
                        t1 = t2;
                        t2 = tmp;
                    } else
                        tmp = t1;
                    if (t1 > tNear)
                        tNear = t1;
                    if (t2 < tFar)
                        tFar = t2;

                    if (tNear > tFar)
                        return false;

                    if(tFar < 0)
                        return false;
                }
            }
            if (tNear > 0)
                i1 = ray.getOrig() + ray.getDir() * tNear;
            else
                i1 = ray.getOrig();
            i2 = ray.getOrig() + ray.getDir() * tFar;
            return true;
        }
        bool OrientedBoundingBox::intersects (BoundingBox &bx, CollisionResultSet::Info& info) {
            //////std::cout<<"vertices : "<<bx.getVertices()[0]<<bx.getVertices()[1]<<bx.getVertices()[2]<<std::endl;
            float distMin1, distMin2;
            int ptIndex1, ptIndex2;
            int edgeIndex1, edgeIndex2;
            int faceIndex1, faceIndex2;
            /* Check the nearest vertex of the polygon 2 from the regions of polygon 2 and give the distance and
            *  return the point or the face's bissector of the region.
            */
            int vertexIndex1 = math::Computer::checkNearestVertexFromShape(center, points, edgeBissectors, edgeNormals, faceBissectors, faceNormals, bx.getVertices(), distMin1, ptIndex1, edgeIndex1, faceIndex1, 3);
            info.nearestVertexIndex1 = vertexIndex1;
            info.nearestPtIndex1 = ptIndex1;
            info.nearestEdgeIndex1 = edgeIndex1;
            info.nearestFaceIndex1 = faceIndex1;
            /* Check the nearest vertex of the polygon 1 from the regions of polygon 2 and give the distance and
            *  return the point or face's bissector of the region.
            */
            int vertexIndex2 = math::Computer::checkNearestVertexFromShape(bx.getCenter(), bx.getVertices(), bx.getEdgeBissectors(), bx.getEdgeNormals(), bx.getFaceBissectors(), bx.getFaceNormals(), points, distMin2, ptIndex2, edgeIndex2, faceIndex2, 3);
            info.nearestVertexIndex2 = vertexIndex2;
            info.nearestPtIndex2 = ptIndex2;
            info.nearestEdgeIndex2 = edgeIndex2;
            info.nearestFaceIndex2 = faceIndex2;
            if (flat && bx.isFlat()) {
                if (faceIndex1 != -1 && faceIndex2 != -1)  {
                    math::Triangle t1(points[faceIndex1*3], points[faceIndex1*3+1], points[faceIndex1*3+2]);
                    math::Triangle t2(points[faceIndex1*3], points[faceIndex1*3+1], points[faceIndex1*3+3]);
                    math::Triangle t3(bx.getVertices()[faceIndex1*3], bx.getVertices()[faceIndex1*3+1], bx.getVertices()[faceIndex1*3+2]);
                    return t1.intersects(t3) || t2.intersects(t3);
                }
                if (faceIndex1 != -1 && faceIndex2 == -1) {
                    math::Ray r1 (bx.getVertices()[edgeIndex2], bx.getVertices()[(edgeIndex2 % 3 == 2) ? edgeIndex2 - 2 : edgeIndex2 + 1]);
                    math::Ray r2 (bx.getVertices()[(edgeIndex2 % 3 == 2) ? edgeIndex2 - 2 : edgeIndex2 + 1], bx.getVertices()[edgeIndex2]);
                    return (intersects(r1, false, info) && intersects(r2, false, info));
                }
                if (faceIndex1 == -1 && faceIndex2 != -1) {
                    math::Triangle t(bx.getVertices()[faceIndex2*3], bx.getVertices()[faceIndex2*3+1], bx.getVertices()[faceIndex2*3+2]);
                    math::Ray r1 (points[edgeIndex1], points[(edgeIndex1 % 3 == 2) ? edgeIndex1 - 2 : edgeIndex1 + 1]);
                    math::Ray r2 (points[(edgeIndex1 % 3 == 2) ? edgeIndex1 - 2 : edgeIndex1 + 1], points[edgeIndex1]);
                    return (t.intersects(r1) && t.intersects(r2));
                }
                return false;
            } else if (!flat && !bx.isFlat()) {
                for (unsigned int n = 0; n < faceNormals.size(); n++) {
                    math::Vec3f normal = faceNormals[n];
                    if (!math::Computer::overlapThisNormal(points, bx.getVertices(), normal))
                        return false;
                }
                for (unsigned int n = 0; n < bx.getFaceNormals().size(); n++) {
                    math::Vec3f normal = bx.getFaceNormals()[n];
                    if (!math::Computer::overlapThisNormal(bx.getVertices(), points, normal))
                        return false;
                }
                return true;
            } else if (flat && !bx.isFlat()) {
                if (faceIndex1 != -1 && faceIndex2 != -1)  {
                    math::Triangle t1(points[faceIndex1*3], points[faceIndex1*3+1], points[faceIndex1*3+2]);
                    math::Triangle t2(points[faceIndex1*3], points[faceIndex1*3+2], points[faceIndex1*3+3]);
                    math::Triangle t3(bx.getVertices()[faceIndex2*3], bx.getVertices()[faceIndex2*3+1], bx.getVertices()[faceIndex2*3+2]);
                    return bx.isPointInside(t1.getP1())
                            || bx.isPointInside(t1.getP2())
                            || bx.isPointInside(t1.getP3())
                            || bx.isPointInside(t2.getP1())
                            || bx.isPointInside(t2.getP2())
                            || bx.isPointInside(t2.getP3())
                            || t1.intersects(t3)
                            || t2.intersects(t3);
                }
                if (faceIndex1 != -1 && faceIndex2 == -1) {
                    math::Triangle t1(points[faceIndex1*3], points[faceIndex1*3+1], points[faceIndex1*3+2]);
                    math::Triangle t2(points[faceIndex1*3], points[faceIndex1*3+2], points[faceIndex1*3+3]);
                    math::Ray r1 (bx.getVertices()[edgeIndex2], bx.getVertices()[(edgeIndex2 % 3 == 2) ? edgeIndex2 - 2 : edgeIndex2 + 1]);
                    math::Ray r2 (bx.getVertices()[(edgeIndex2 % 3 == 2) ? edgeIndex2 - 2 : edgeIndex2 + 1], bx.getVertices()[edgeIndex2]);
                    return bx.isPointInside(t1.getP1())
                           || bx.isPointInside(t1.getP2())
                           || bx.isPointInside(t1.getP3())
                           || bx.isPointInside(t2.getP1())
                           || bx.isPointInside(t2.getP2())
                           || bx.isPointInside(t2.getP3())
                           || (t1.intersects(r1) && t1.intersects(r2))
                           || (t2.intersects(r1) && t2.intersects(r2));
                }
                if (faceIndex1 == -1 && faceIndex2 != -1) {
                    math::Triangle t(bx.getVertices()[faceIndex2*3], bx.getVertices()[faceIndex2*3+1], bx.getVertices()[faceIndex2*3+2]);
                    math::Ray r1 (points[edgeIndex1], points[(edgeIndex1 % 3 == 2) ? edgeIndex1 - 2 : edgeIndex1 + 1]);
                    math::Ray r2 (points[(edgeIndex1 % 3 == 2) ? edgeIndex1 - 2 : edgeIndex1 + 1], points[edgeIndex1]);
                    return bx.isPointInside(r1.getOrig())
                            || bx.isPointInside(r1.getExt())
                            || (t.intersects(r1) && t.intersects(r2));
                }
                return false;
            } else {
                BoundingPolyhedron bp;
                for (unsigned int i = 0; i < points.size(); i++) {
                    math::Vec3f p1 = points[i];
                    math::Vec3f p2 = (i + 1 < points.size()) ? points[i+1] : points[0];
                    bp.addTriangle(math::Vec3f(center.x(), center.y(), 0), math::Vec3f(p1.x(), p1.y(), 0), math::Vec3f(p2.x(), p2.y(), 0));
                }
                for (unsigned int n = 0; n < bp.getEdgeNormals().size(); n++) {
                    math::Vec3f normal = bp.getEdgeNormals()[n];
                    if (!math::Computer::overlapThisNormal(bp.getPoints(), bx.getVertices(), normal))
                        return false;
                }
                for (unsigned int n = 0; n < bx.getEdgeNormals().size(); n++) {
                    math::Vec3f normal = bx.getEdgeNormals()[n];
                    if (!math::Computer::overlapThisNormal(bx.getVertices(), bp.getPoints(), normal))
                        return false;
                }
                return true;
            }
        }
        bool OrientedBoundingBox::intersects (OrientedBoundingBox &obx, CollisionResultSet::Info& info) {
            float distMin1, distMin2;
            int ptIndex1, ptIndex2;
            int edgeIndex1, edgeIndex2;
            int faceIndex1, faceIndex2;

            int vertexIndex1 = math::Computer::checkNearestVertexFromShape(center, points, edgeBissectors, edgeNormals, faceBissectors, faceNormals, obx.getVertices(), distMin1, ptIndex1, edgeIndex1, faceIndex1, 4);
            info.nearestVertexIndex1 = vertexIndex1;
            info.nearestPtIndex1 = ptIndex1;
            info.nearestEdgeIndex1 = edgeIndex1;
            info.nearestFaceIndex1 = faceIndex1;

            int vertexIndex2 = math::Computer::checkNearestVertexFromShape(obx.getCenter(), obx.getVertices(), obx.getEdgeBissectors(), obx.getEdgeNormals(), obx.getFaceBissectors(), obx.getFaceNormals(), points, distMin2, ptIndex2, edgeIndex2, faceIndex2, 4);
            info.nearestVertexIndex2 = vertexIndex2;
            info.nearestPtIndex2 = ptIndex2;
            info.nearestEdgeIndex2 = edgeIndex2;
            info.nearestFaceIndex2 = faceIndex2;
            if (ptIndex1 != -1) {
                return (points[ptIndex1] - center).magnSquared() >= (obx.getVertices()[vertexIndex1] - center).magnSquared();
            }
            if (ptIndex2 != -1) {
                return (points[ptIndex2] - center).magnSquared() >= (obx.getVertices()[vertexIndex2] - center).magnSquared();
            }
            if (flat && obx.isFlat()) {
                if (faceIndex1 != -1 && faceIndex2 != -1)  {
                    math::Triangle t1(points[faceIndex1*3], points[faceIndex1*3+1], points[faceIndex1*3+2]);
                    math::Triangle t2(points[faceIndex1*3], points[faceIndex1*3+1], points[faceIndex1*3+3]);
                    math::Triangle t3(obx.getVertices()[faceIndex1*3], obx.getVertices()[faceIndex1*3+1], obx.getVertices()[faceIndex1*3+2]);
                    return t1.intersects(t3) || t2.intersects(t3);
                }
                if (faceIndex1 != -1 && faceIndex2 == -1) {
                    math::Ray r1 (obx.getVertices()[edgeIndex2], obx.getVertices()[(edgeIndex2 % 3 == 2) ? edgeIndex2 - 2 : edgeIndex2 + 1]);
                    math::Ray r2 (obx.getVertices()[(edgeIndex2 % 3 == 2) ? edgeIndex2 - 2 : edgeIndex2 + 1], obx.getVertices()[edgeIndex2]);
                    return (intersects(r1, false, info) && intersects(r2, false, info));
                }
                if (faceIndex1 == -1 && faceIndex2 != -1) {
                    math::Triangle t(obx.getVertices()[faceIndex2*3], obx.getVertices()[faceIndex2*3+1], obx.getVertices()[faceIndex2*3+2]);
                    math::Ray r1 (points[edgeIndex1], points[(edgeIndex1 % 3 == 2) ? edgeIndex1 - 2 : edgeIndex1 + 1]);
                    math::Ray r2 (points[(edgeIndex1 % 3 == 2) ? edgeIndex1 - 2 : edgeIndex1 + 1], points[edgeIndex1]);
                    return (t.intersects(r1) && t.intersects(r2));
                }
                return false;
            } else if (!flat && !obx.isFlat()) {
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
            } else if (flat && !obx.isFlat()) {
                if (faceIndex1 != -1 && faceIndex2 != -1)  {
                    math::Triangle t1(points[faceIndex1*3], points[faceIndex1*3+1], points[faceIndex1*3+2]);
                    math::Triangle t2(points[faceIndex1*3], points[faceIndex1*3+1], points[faceIndex1*3+3]);
                    math::Triangle t3(obx.getVertices()[faceIndex2*3], obx.getVertices()[faceIndex2*3+1], obx.getVertices()[faceIndex2*3+2]);
                    return obx.isPointInside(t1.getP1())
                            || obx.isPointInside(t1.getP2())
                            || obx.isPointInside(t1.getP3())
                            || obx.isPointInside(t2.getP1())
                            || obx.isPointInside(t2.getP2())
                            || obx.isPointInside(t2.getP3())
                            || t1.intersects(t3)
                            || t2.intersects(t3);
                }
                if (faceIndex1 != -1 && faceIndex2 == -1) {
                    math::Triangle t1(points[faceIndex1*3], points[faceIndex1*3+1], points[faceIndex1*3+2]);
                    math::Triangle t2(points[faceIndex1*3], points[faceIndex1*3+1], points[faceIndex1*3+3]);
                    math::Ray r1 (obx.getVertices()[edgeIndex2], obx.getVertices()[(edgeIndex2 % 3 == 2) ? edgeIndex2 - 2 : edgeIndex2 + 1]);
                    math::Ray r2 (obx.getVertices()[(edgeIndex2 % 3 == 2) ? edgeIndex2 - 2 : edgeIndex2 + 1], obx.getVertices()[edgeIndex2]);
                    return obx.isPointInside(t1.getP1())
                           || obx.isPointInside(t1.getP2())
                           || obx.isPointInside(t1.getP3())
                           || obx.isPointInside(t2.getP1())
                           || obx.isPointInside(t2.getP2())
                           || obx.isPointInside(t2.getP3())
                           || (t1.intersects(r1) && t1.intersects(r2))
                           || (t2.intersects(r1) && t2.intersects(r2));
                } else if (faceIndex1 == -1 && faceIndex2 != -1) {
                    math::Triangle t(obx.getVertices()[faceIndex2*3], obx.getVertices()[faceIndex2*3+1], obx.getVertices()[faceIndex2*3+2]);
                    math::Ray r1 (points[edgeIndex1], points[(edgeIndex1 % 3 == 2) ? edgeIndex1 - 2 : edgeIndex1 + 1]);
                    math::Ray r2 (points[(edgeIndex1 % 3 == 2) ? edgeIndex1 - 2 : edgeIndex1 + 1], points[edgeIndex1]);
                    return obx.isPointInside(r1.getOrig())
                            || obx.isPointInside(r1.getExt())
                            || (t.intersects(r1) && t.intersects(r2));
                }
                return false;
            } else {
                if (faceIndex1 != -1 && faceIndex2 != -1)  {
                    math::Triangle t1(points[faceIndex1*3], points[faceIndex1*3+1], points[faceIndex1*3+2]);
                    math::Triangle t2(points[faceIndex1*3], points[faceIndex1*3+1], points[faceIndex1*3+3]);
                    math::Triangle t3(obx.getVertices()[faceIndex2*3], obx.getVertices()[faceIndex2*3+1], obx.getVertices()[faceIndex2*3+2]);
                    return isPointInside(t2.getP1())
                            || isPointInside(t2.getP2())
                            || isPointInside(t2.getP3())
                            || t1.intersects(t3)
                            || t2.intersects(t3);
                }
                if (faceIndex1 != -1 && faceIndex2 == -1) {
                    math::Triangle t1(points[faceIndex1*3], points[faceIndex1*3+1], points[faceIndex1*3+2]);
                    math::Triangle t2(points[faceIndex1*3], points[faceIndex1*3+1], points[faceIndex1*3+3]);
                    math::Ray r1 (obx.getVertices()[edgeIndex2], obx.getVertices()[(edgeIndex2 % 3 == 2) ? edgeIndex2 - 2 : edgeIndex2 + 1]);
                    math::Ray r2 (obx.getVertices()[(edgeIndex2 % 3 == 2) ? edgeIndex2 - 2 : edgeIndex2 + 1], obx.getVertices()[edgeIndex2]);
                    return isPointInside(r1.getOrig())
                           || isPointInside(r1.getExt())
                           || (t1.intersects(r1) && t1.intersects(r2))
                           || (t2.intersects(r1) && t2.intersects(r2));
                }
                if (faceIndex1 == -1 && faceIndex2 != -1) {
                    math::Triangle t(obx.getVertices()[faceIndex2*3], obx.getVertices()[faceIndex2*3+1], obx.getVertices()[faceIndex2*3+2]);
                    math::Ray r1 (points[edgeIndex1], points[(edgeIndex1 % 3 == 2) ? edgeIndex1 - 2 : edgeIndex1 + 1]);
                    math::Ray r2 (points[(edgeIndex1 % 3 == 2) ? edgeIndex1 - 2 : edgeIndex1 + 1], points[edgeIndex1]);
                    return isPointInside(t.getP1())
                            || isPointInside(t.getP2())
                            || isPointInside(t.getP3())
                            || (t.intersects(r1) && t.intersects(r2));
                }
                return false;
            }
        }
        bool OrientedBoundingBox::intersects (BoundingPolyhedron &bp, CollisionResultSet::Info& info) {


            float distMin1 = 0, distMin2 = 0;
            int edgeIndex1 = 0, edgeIndex2 = 0;
            int faceIndex1 = 0, faceIndex2 = 0;
            int ptIndex1 = math::Computer::checkNearestVertexFromShape(center, points, edgeBissectors, edgeNormals, faceBissectors, faceNormals, bp.getPoints(), distMin1, ptIndex1, edgeIndex1, faceIndex1, 3);
            int ptIndex2 = math::Computer::checkNearestVertexFromShape(bp.getCenter(), bp.getPoints(), bp.getEdgeBissectors(), bp.getEdgeNormals(), bp.getFaceBissectors(), bp.getFaceNormals(), points, distMin2, ptIndex2, edgeIndex2, faceIndex2, 4);
            if (distMin1 < distMin2) {
                math::Vec3f bpn = (bp.getFaceBissectors()[faceIndex2] - bp.getCenter()).projOnVector(bp.getFaceNormals()[faceIndex2]);
                math::Vec3f d = points[ptIndex1] - bp.getCenter();
                float p = d.projOnAxis(bpn);
                if (p * p > bpn.magnSquared()) {
                    return false;
                }
                return true;
            } else {
                math::Vec3f bpn = faceNormals[faceIndex1];
                math::Vec3f d = bp.getPoints()[ptIndex2] - center;
                float p = d.projOnAxis(bpn);
                if (p * p > bpn.magnSquared()) {
                    return false;
                }
                return true;
            }
        }

        math::Vec3f OrientedBoundingBox::getCenter() {
            return center;
        }
        int OrientedBoundingBox::getWidth() {
            return width;
        }
        int OrientedBoundingBox::getHeight() {
            return height;
        }
        int OrientedBoundingBox::getDepth() {
            return depth;
        }
        math::Vec3f OrientedBoundingBox::getSize() {
            return math::Vec3f(width, height, depth);
        }
        math::Vec3f OrientedBoundingBox::getPosition() {
            return math::Vec3f(x, y, z);
        }
        void OrientedBoundingBox::setCorners (math::Vec3f p1, math::Vec3f p2, math::Vec3f p3, math::Vec3f p4, math::Vec3f p5, math::Vec3f p6, math::Vec3f p7, math::Vec3f p8) {
            points.resize(8);
            std::array<math::Vec3f, 8> corners;
            std::array<math::Vec3f, 4> frontCorners;
            std::array<math::Vec3f, 4> backCorners;
            corners[0] = p1;
            corners[1] = p2;
            corners[2] = p3;
            corners[3] = p4;
            corners[4] = p5;
            corners[5] = p6;
            corners[6] = p7;
            corners[7] = p8;
            points[8] = points[4];
            points[9] = points[5];
            points[10] = points[1];
            points[11] = points[0];
            points[12] = points[7];
            points[13] = points[6];
            points[14] = points[2];
            points[15] = points[3];
            points[16] = points[1];
            points[17] = points[5];
            points[18] = points[6];
            points[19] = points[2];
            points[20] = points[0];
            points[21] = points[4];
            points[22] = points[7];
            points[23] = points[3];
            for (unsigned int i = 0; i < corners.size() - 1; i++) {
                for (unsigned int j = 1; j < corners.size(); j++) {
                    if (corners[j].z() < corners[i].z()) {
                        float tmp = corners[i].z();
                        corners[i].z() = corners[j].z();
                        corners[j][2] = tmp;
                    }
                }
            }
            frontCorners[0] = corners[0];
            frontCorners[1] = corners[1];
            frontCorners[2] = corners[2];
            frontCorners[3] = corners[3];
            backCorners[0] = corners[4];
            backCorners[1] = corners[5];
            backCorners[2] = corners[6];
            backCorners[3] = corners[7];
            std::array<std::array<float, 2>, 3> store;
            store = math::Computer::getExtends(corners);
            x = store[0][0];
            y = store[1][0];
            z = store[2][0];
            width = store[0][1] - x;
            height = store[1][1] - y;
            depth = store[2][1] - z;
            store = math::Computer::getExtends(frontCorners);
            math::Vec3f position (store[0][0], store[1][0], store[2][0]);

            store = math::Computer::getExtends(backCorners);
            position = math::Vec3f (store[0][0], store[1][0], store[2][0]);
            //Check the right order of the points to find the normals of the oriented bounding rectangle.
            for (unsigned int i = 0; i < 8; i++) {
                points[i] = corners[i];
            }
            flat = false;
            computeVectors();
        }
        void OrientedBoundingBox::move(math::Vec3f t) {
            center += t;
            for (unsigned int i = 0; i < points.size(); i++)
                points[i]+= t;
            std::array<std::array<float, 2>, 3> extends = math::Computer::getExtends(points);
            x = extends[0][0];
            y = extends[1][0];
            z = extends[2][0];
            computeVectors();
        }
        void OrientedBoundingBox::scale(math::Vec3f s) {
            for (unsigned int i = 0; i < points.size(); i++)
                points[i] = (points[i] - center) * s + center;
            std::array<std::array<float, 2>, 3> extends = math::Computer::getExtends(points);
            x = extends[0][0];
            y = extends[1][0];
            z = extends[2][0];
            computeVectors();
        }
        void OrientedBoundingBox::rotate(float angle, math::Vec3f v) {
            graphic::TransformMatrix tm;
            tm.setRotation(v, angle);
            tm.setOrigin(center);
            for (unsigned int i = 0; i < points.size(); i++)
                points[i] = tm.transform(points[i]);
            std::array<std::array<float, 2>, 3> extends = math::Computer::getExtends(points);
            x = extends[0][0];
            y = extends[1][0];
            z = extends[2][0];
            width = extends[0][1] - extends[0][0];
            height = extends[1][1] - extends[1][0];
            depth = extends[2][1] - extends[2][0];
            computeVectors();
        }
        std::vector<math::Vec3f> OrientedBoundingBox::getEdgeBissectors() {
            return edgeBissectors;
        }
        std::vector<math::Vec3f> OrientedBoundingBox::getEdgeNormals() {
            return edgeNormals;
        }
        std::vector<math::Vec3f> OrientedBoundingBox::getFaceBissectors() {
            return faceBissectors;
        }
        std::vector<math::Vec3f> OrientedBoundingBox::getFaceNormals() {
            return faceNormals;
        }
        bool OrientedBoundingBox::isFlat() {
            return flat;
        }
        bool OrientedBoundingBox::isPointInside(math::Vec3f point) {
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
            //////std::cout<<"bissector : "<<faceBissectors[faceIndex]<<std::endl;
            math::Vec3f d = point - center;
            float p = d.projOnAxis(bpn);
            if (p * p > bpn.magnSquared()) {
                return false;
            }
            return true;
           /*if (flat) {
                float minX = - width * 0.5f;
                float maxX = width * 0.5f;
                float minY = -height * 0.5f;
                float maxY = height * 0.5f;
                math::Vec3f d = point - center;
                const float epsilon = 1e-6f;
                for (unsigned int i = 0; i < 2; i++) {
                    float p = d.projOnAxis(edgeNormals[i]);
                    //////std::cout<<"d  :"<<d<<"normal : "<<faceNormals[i]<<std::endl;
                    if (i == 0) {
                        //////std::cout<<"p : "<<p<<",minX : "<<minX<<", maxX : "<<maxX<<",x : "<<(minX <= p+epsilon && p-epsilon <= maxX)<<std::endl;
                        if (!(minX <= p+epsilon && p-epsilon <= maxX))
                            return false;
                    } else {
                        //////std::cout<<"p : "<<p<<",minY : "<<minY<<", maxY : "<<maxY<<std::endl;
                        if(!(minY <= p+epsilon && p-epsilon <= maxY))
                           return false;
                    }
                }
                return true;
            }
            float minX = - width * 0.5f;
            float maxX = width * 0.5f;
            float minY = -height * 0.5f;
            float maxY = height * 0.5f;
            float minZ = - depth * 0.5f;
            float maxZ = depth * 0.5f;

            const float epsilon = 1e-6f;
            math::Vec3f d = point - center;
            for (unsigned int i = 0; i < 3; i++) {
                float p = d.projOnAxis(faceNormals[i]);
                //////std::cout<<"d  :"<<d<<"normal : "<<faceNormals[i]<<std::endl;
                if (i == 0) {
                    //////std::cout<<"p : "<<p<<",minX : "<<minX<<", maxX : "<<maxX<<",x : "<<(minX <= p+epsilon && p-epsilon <= maxX)<<std::endl;
                    if (!(minX <= p+epsilon && p-epsilon <= maxX))
                        return false;
                } else if (i == 1) {
                    //////std::cout<<"p : "<<p<<",minY : "<<minY<<", maxY : "<<maxY<<std::endl;
                    if(!(minY <= p+epsilon && p-epsilon <= maxY))
                       return false;
                } else {
                    //////std::cout<<"p : "<<p<<",minZ : "<<minZ<<", maxZ : "<<maxZ<<std::endl;
                    if(!(minZ <= p+epsilon && p-epsilon <= maxZ))
                       return false;
                }
            }
            return true;*/
        }
    }
}
