#include "computer.hpp"
namespace odfaeg {
    namespace math {
        Vec2f Computer::getMoy(std::vector<Vec2f> verts) {
            Vec2f sum(0.f, 0.f);
            for (unsigned int i = 0; i < verts.size(); i++) {
                sum += verts[i];
            }
            return sum / verts.size();
        }
        Vec3f Computer::getMoy(std::vector<Vec3f> verts) {
            Vec3f sum(0.f, 0.f, 0.f);
            for (unsigned int i = 0; i < verts.size(); i++) {
                sum += verts[i];
            }
            return sum / verts.size();
        }
        //Calculs les minimum est maximum d'un vecteurs pass� et les stocke dans un tableau.
        std::array<std::array<float, 2>, 3> Computer::getExtends(std::vector<Vec3f> verts) {
            float minX = 0;
            float maxX = 0;
            float minY = 0;
            float maxY = 0;
            float minZ = 0;
            float maxZ = 0;
            if (verts.size() > 0) {
                minX = verts[0].x();
                maxX = verts[0].x();
                minY = verts[0].y();
                maxY = verts[0].y();
                minZ = verts[0].z();
                maxZ = verts[0].z();
            }
            std::array<std::array<float, 2>, 3> store;
            for (unsigned int i(1); i < verts.size(); i++) {


                if (verts[i].x() > maxX) {
                    maxX = verts[i].x();
                }
                if (verts[i].x() < minX) {
                    minX = verts[i].x();
                }
                if (verts[i].y() > maxY) {
                    maxY = verts[i].y();
                }
                if (verts[i].y() < minY) {
                    minY = verts[i].y();
                }
                if (verts[i].z() > maxZ) {
                    maxZ = verts[i].z();
                }
                if (verts[i].z() < minZ) {
                    minZ = verts[i].z();
                }
            }
            store[0][0] = minX;
            store[0][1] = maxX;
            store[1][0] = minY;
            store[1][1] = maxY;
            store[2][0] = minZ;
            store[2][1] = maxZ;
            return store;
        }
        std::array<std::array<float, 2>, 2> Computer::getExtends(std::vector<Vec2f> verts) {
            float minX = 0;
            float maxX = 0;
            float minY = 0;
            float maxY = 0;
            if (verts.size() > 0) {
                minX = verts[0].x();
                maxX = verts[0].x();
                minY = verts[0].y();
                maxY = verts[0].y();
            }
            std::array<std::array<float, 2>, 2> store;
            for (unsigned int i(1); i < verts.size(); i++) {

                if (verts[i].x() > maxX) {
                    maxX = verts[i].x();
                }
                if (verts[i].x() < minX) {
                    minX = verts[i].x();
                }
                if (verts[i].y() > maxY) {
                    maxY = verts[i].y();
                }
                if (verts[i].y() < minY) {
                    minY = verts[i].y();
                }


            }
            store[0][0] = minX;
            store[0][1] = maxX;
            store[1][0] = minY;
            store[1][1] = maxY;
            return store;
        }
        Plane Computer::computeSeparatingPlane(std::vector<Vec3f> points) {
            std::vector<Vec3f> rightPoints;
            Vec3f center1 = getMoy(points);
            for (unsigned int i = 0; i < points.size(); i++) {
                if (points[i].x() > center1.x())
                    rightPoints.push_back(points[i]);
            }
            Vec3f center2 = getMoy(rightPoints);
            Vec3f n = center2 - center1;
            return Plane(n, center1);
        }
        bool Computer::overlapThisNormal(std::vector<Vec3f> verticesA, std::vector<Vec3f> verticesB, Vec3f normal) {
            float minA = NAN;
            float maxA = NAN;
            for (unsigned int v = 0; v < verticesA.size(); v++) {
                math::Vec3f vertex = verticesA[v];
                float p = vertex.projOnAxis(normal);
                if (std::isnan(minA) || p < minA)
                    minA = p;
                if (std::isnan(maxA) || p > maxA)
                    maxA = p;
            }
            float minB = NAN;
            float maxB = NAN;
            for (unsigned int v = 0; v < verticesB.size(); v++) {
                math::Vec3f vertex = verticesB[v];
                float p = vertex.projOnAxis(normal);
                if (std::isnan(minB) || p < minB)
                    minB = p;
                if (std::isnan(maxB) || p > maxB)
                    maxB = p;
            }
            return overlap(minA, maxA, minB, maxB);
        }
        bool Computer::overlap(float minA, float maxA, float minB, float maxB) {
            float minOverlap = NAN;
            float maxOverlap = NAN;


            //If B contain in A
            if (minA <= minB && minB <= maxA) {
                if (std::isnan(minOverlap) || minB < minOverlap)
                    minOverlap = minB;
            }
            if (minA <= maxB && maxB <= maxA) {
                if (std::isnan(maxOverlap) || maxB > minOverlap)
                    maxOverlap = maxB;
            }

            //If A contain in B
            if (minB <= minA && minA <= maxB) {
                if (std::isnan(minOverlap) || minA < minOverlap)
                    minOverlap = minA;
            }
            if (minB <= maxA && maxA <= maxB) {
                if (std::isnan(maxOverlap) || maxA > minOverlap)
                    maxOverlap = maxA;
            }

            if (std::isnan(minOverlap) || std::isnan(maxOverlap))
                return false; //Pas d'intersection
            else 
                return true;
        }
        Vec2f Computer::projectShapeOnAxis(Vec3f axis, std::vector<Vec3f> vertices) {
            float min = 0, max = 0;
            if (vertices.size() > 0) {
                min = vertices[0].projOnAxis(axis);
                max = min;
                for (unsigned int i = 1; i < vertices.size(); i++) {
                    // NOTE: the axis must be normalized to get accurate projections
                    float p = vertices[i].projOnAxis(axis);
                    if (p < min) {
                        min = p;
                    }
                    if (p > max) {
                        max = p;
                    }
                }
            }
            return Vec2f(min, max);
        }
        int Computer::checkNearestVertexFromShape(Vec3f center, std::vector<Vec3f> points, std::vector<Vec3f> edgeBissectors, std::vector<Vec3f> edgeNormals, std::vector<Vec3f> faceBissectors, std::vector<Vec3f> faceNormals, std::vector<Vec3f> vertices,
            float& distMin, int& ptIndex, int& edgeIndex, int& faceIndex, int nbEdgesPerFace) {
            distMin = std::numeric_limits<float>::max();
            float nDistMin = std::numeric_limits<float>::max();
            edgeIndex = -1;
            faceIndex = -1;
            ptIndex = -1;
            int index = -1;
            for (unsigned int i = 0; i < edgeBissectors.size(); i++) {
                Vec3f p1 = edgeBissectors[i] - center;
                Vec3f n = edgeNormals[i];
                for (unsigned int j = 0; j < vertices.size(); j++) {
                    Vec3f p2 = vertices[j] - center;
                    float dist = p1.computeDistSquared(p2);
                    float nDist = n.computeDistSquared(p2);
                    Vec3f v1 = points[i] - center;
                    Vec3f v2 = points[(i + 1 == points.size()) ? 0 : i + 1] - center;
                    Vec3f n = v1.cross(v2);
                    float a1 = p2.getAngleBetween(v1, n);
                    float a2 = p2.getAngleBetween(v2, n);
                    if (a1 == 0) {
                        ptIndex = i;
                        return j;
                    }
                    if (a2 == 0) {
                        ptIndex = (i + 1 == points.size()) ? 0 : i + 1;
                        return j;
                    }
                    if ((dist <= distMin) && (nDist <= nDistMin) && (a1 > 0 && a2 < 0 || a1 < 0 && a2 > 0)) {
                        distMin = dist;
                        nDistMin = nDist;
                        edgeIndex = i;
                        index = j;
                    }
                }
            }
            distMin = std::numeric_limits<float>::max();
            nDistMin = std::numeric_limits<float>::max();
            for (unsigned int i = 0; i < faceBissectors.size(); i++) {
                Vec3f p1 = faceBissectors[i];
                Vec3f n = faceNormals[i];
                for (unsigned int j = 0; j < vertices.size(); j++) {
                    Vec3f p2 = vertices[j];
                    float dist = p1.computeDistSquared(p2);
                    float nDist = n.computeDistSquared(p2);
                    bool isInFaceRegion = true;
                    for (unsigned int k = 0; k < nbEdgesPerFace && isInFaceRegion; k++) {
                        Plane p(edgeNormals[i * nbEdgesPerFace + k], edgeBissectors[i * nbEdgesPerFace + k]);
                        if (p.whichSide(p2) > 0)
                            isInFaceRegion = false;
                    }
                    if (dist <= distMin && nDist <= nDistMin && isInFaceRegion) {
                        distMin = dist;
                        faceIndex = i;
                        nDistMin = nDist;
                        index = j;
                    }
                }
            }
            return index;
        }
    }
}