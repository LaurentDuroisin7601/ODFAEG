#include "../../../include/odfaeg/Math/computer.h"
namespace odfaeg {
    namespace math {
        using namespace std;
        Vec2f Computer::getMoy (vector<Vec2f> verts) {
            Vec2f sum(0, 0);
            for (unsigned int i = 0; i < verts.size(); i++) {
                sum += verts[i];
            }
            return sum / verts.size();
        }
        Vec3f Computer::getMoy (std::vector<Vec3f> verts) {
            Vec3f sum(0, 0, 0);
            for (unsigned int i = 0; i < verts.size(); i++) {
                sum += verts[i];
            }
            return sum / verts.size();
        }
        //Calculs les minimum est maximum d'un vecteurs passé et les stocke dans un tableau.
        std::array<std::array<float, 2>, 3> Computer::getExtends (std::vector<Vec3f> verts) {
            float minX = 0;
            float maxX = 0;
            float minY = 0;
            float maxY = 0;
            float minZ = 0;
            float maxZ = 0;
            if (verts.size() > 0) {
                minX = verts[0].x;
                maxX = verts[0].x;
                minY = verts[0].y;
                maxY = verts[0].y;
                minZ = verts[0].z;
                maxZ = verts[0].z;
            }
            std::array<std::array<float, 2>, 3> store;
            for (unsigned int i(1); i < verts.size(); i++) {


                    if (verts[i].x > maxX) {
                        maxX = verts[i].x;
                    }
                    if (verts[i].x < minX) {
                        minX = verts[i].x;
                    }
                    if (verts[i].y > maxY) {
                        maxY = verts[i].y;
                    }
                    if (verts[i].y < minY) {
                        minY = verts[i].y;
                    }
                    if (verts[i].z > maxZ) {
                        maxZ = verts[i].z;
                    }
                    if (verts[i].z < minZ) {
                        minZ = verts[i].z;
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
        std::array<std::array<float, 2>, 2> Computer::getExtends (std::vector<Vec2f> verts) {
            float minX = 0;
            float maxX = 0;
            float minY = 0;
            float maxY = 0;
            if (verts.size() > 0) {
                minX = verts[0].x;
                maxX = verts[0].x;
                minY = verts[0].y;
                maxY = verts[0].y;
            }
            std::array<std::array<float, 2>, 2> store;
            for (unsigned int i(1); i < verts.size(); i++) {

                    if (verts[i].x > maxX) {
                        maxX = verts[i].x;
                    }
                    if (verts[i].x < minX) {
                        minX = verts[i].x;
                    }
                    if (verts[i].y > maxY) {
                        maxY = verts[i].y;
                    }
                    if (verts[i].y < minY) {
                        minY = verts[i].y;
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
            Vec3f center1 = Computer::getMoy(points);
            for (unsigned int i = 0; i < points.size(); i++) {
                if (points[i].x > center1.x)
                    rightPoints.push_back(points[i]);
            }
            Vec3f center2 = Computer::getMoy(rightPoints);
            Vec3f n = center2 - center1;
            return Plane(n, center1);
        }
    }
}
