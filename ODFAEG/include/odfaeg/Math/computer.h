#ifndef COMPUTER
#define COMPUTER
#include "matrix3.h"
#include "vec4.h"
#include <limits.h>
#include <array>
#include "ray.h"
#include <vector>
#include "export.hpp"
#include "plane.h"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace math {
        /**
          * \file computer.h
          * \class Computer
          * \brief Compute the min and max points of a vector or an array of points.
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          *
          * Manage a computer to check the min and max x,y coordinates from a set of points or the average.
          * Return the average or a 2 dimentionnal array containing the min and max coordinates.
          * The index of min and max are : 0,0 for the min x; 0, 1 for the max x; 1,0 for the min Y, etc...
          */
        class ODFAEG_MATH_API Computer {
            public :
            /**\fn Vec2f getMoy (std::vector<Vec2f*> verts)
            *  \brief get the average from a set of 2D vectors.
            *  \param a list of vectors.
            *  \return the average vector.
            */
            static Vec2f getMoy (std::vector<Vec2f> verts);
            static Vec3f getMoy (std::vector<Vec3f> verts);
            /**\fn std::array<std::array<float, 3>,2> getExtends (std::vector<Vec3f*> verts);
            *  \brief get the minimum and the maximum x, y and z from a set of 3D vectors.
            *  \param the list of the vectors.
            *  \return an array containing the minimum and maximum values.
            */
            static std::array<std::array<float, 2>,3> getExtends (std::vector<Vec3f> verts);
            /**\fn std::array<std::array<float, 2>,2> getExtends (std::vector<Vec2f*> verts);
            *  \brief get the minimum and the maximum x and y from a set of 2D vectors.
            *  \param the list of the vectors.
            *  \return an array containing the minimum and maximum values.
            */
            static std::array<std::array<float, 2>,2> getExtends (std::vector<Vec2f> verts);
            /**\fn std::array<std::array<float, 2>,2> getExtends (const std::arrayr<Vec2f, N>& verts);
            *  \brief get the minimum and the maximum x and y from an array of 2D vectors.
            *  \param the array of the vectors.
            *  \return an array containing the minimum and maximum values.
            */
            template <std::size_t N>
            static std::array<std::array<float, 2>,2> getExtends (const std::array<Vec2f, N>& verts) {
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
            /**\fn std::array<std::array<float, 3>,2> getExtends (const std::array<Vec3f, N>& verts);
            *  \brief get the minimum and the maximum x, y and z from an array of 3D vectors.
            *  \param the array of the vectors.
            *  \return an array containing the minimum and maximum values.
            */
            template <std::size_t N>
            static std::array<std::array<float, 2>,3> getExtends (const std::array<Vec3f, N>& verts) {
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
            /**
             *\fn Vec3f getPosOnPathFromTime(Vec3f actualPos, std::vector<Vec3f> path, T time, float speed)
             *\brief check the position of an entity on a path (or on a curve) on a specified time.
             *If the time is positive, it checks the position in the future, otherwise it check the position on the past.
             *This function is often used for movement prediction, networking corrections and artificial intelligence.
             *\param
             * Vec3f actualPos : the actual position of the entity.
             * std::vector<Vec3f> path : the points of the path or of the curve.
             * T time : the time.
             * speed : the speed of the entity.
            */
            template <typename T>
            static Vec3f getPosOnPathFromTime(Vec3f actualPos, std::vector<Vec3f> path, T time, float speed) {
                 //The number of point must be greater than 1. (Otherwise it's not a path.)
                if (path.size() > 1) {
                        unsigned int currentPathIndex = 0;
                        //We check the direction of the first segment of the path and the direction between out actual position and the first point of the first segment of our path.
                        Vec3f dir1 = (path[currentPathIndex+1] - path[currentPathIndex]).normalize();
                        Vec3f dir2 = (actualPos - path[currentPathIndex]).normalize();
                        float d1 = (path[currentPathIndex+1] - path[currentPathIndex]).magnitude();
                        float d2 = (actualPos - path[currentPathIndex]).magnitude();
                        /*std::cout<<"dir 1 : "<<dir1<<" dir 2 : "<<dir2<<" d1 : "<<d1<<" d2 : "<<d2<<std::endl;
                        std::cout<<"angle : "<<Math::abs(Math::toDegrees(dir1.getAngleBetween(dir2, dir1.cross(dir2))))<<std::endl;*/
                        //If the number of point is greater than 2, we need to check on which segment of the path our actual position is.
                        //If the directions are not the same it means that the point is not on this segment of the path.
                        while (currentPathIndex < path.size() - 1 && !dir2.isNulVector() && !(Math::abs(Math::toDegrees(dir1.getAngleBetween(dir2, dir1.cross(dir2)))) >= 1 && d2 < d1)) {
                            //We need to check if the actual position is on the next segment of the path.
                            currentPathIndex++;
                            //If we are arrived on the last point of the path, we don't need to check anymore, it means that our position is not on the path. (Or we are already at the end of the path)
                            if (currentPathIndex < path.size() - 1) {
                                dir1 = (path[currentPathIndex+1] - path[currentPathIndex]).normalize();
                                dir2 = (actualPos - path[currentPathIndex]).normalize();
                                d1 = (path[currentPathIndex+1] - path[currentPathIndex]).magnitude();
                                d2 = (actualPos - path[currentPathIndex]).magnitude();
                               /* std::cout<<"dir 1 : "<<dir1<<" dir 2 : "<<dir2<<" d1 : "<<d1<<" d2 : "<<d2<<std::endl;
                                std::cout<<"angle : "<<Math::abs(Math::toDegrees(dir1.getAngleBetween(dir2, dir1.cross(dir2))))<<std::endl;*/
                            }
                        }
                        //std::cout<<"current index path : "<<currentPathIndex<<"size : "<<path.size()<<std::endl;
                       /*std::string str;
                        std::cin>>str;*/
                        //Our actual position is not on the path so we return our actual position.
                        if (currentPathIndex == path.size() -1 && time > 0 || currentPathIndex == 0 && time < 0)
                            return actualPos;
                        //If the time is positive, we need to check a position on the path in the future.
                        if (time > 0) {
                            /*We check the direction of the segment, the next position and the distances between the two point of the segment and
                            * the distance between the next position and the first point of the segment.
                            */
                            //dir1 = (path[currentPathIndex+1] - path[currentPathIndex]).normalize();
                            Vec3f nextPos = actualPos + dir1 * speed * time;
                            dir2 = (nextPos - path[currentPathIndex]).normalize();
                            d2 = (nextPos - path[currentPathIndex]).magnitude();
                            float aDot = Math::abs(dir1.dot(dir2));

                            //std::cout<<"actual pos : "<<actualPos<<"next pos : "<<nextPos<<"path : "<<path[currentPathIndex]<<std::endl;
                            //std::cout<<"actual pos : "<<actualPos<<" next pos : "<<nextPos<<std::endl;
                            //std::cout<<"actual pos : "<<actualPos<<"next pos : "<<nextPos<<"dir 1 : "<<dir1<<"dir 2 : "<<dir2<<"d1 : "<<d1<<"d2 : "<<d2<<std::endl;
                            /*If the actual position is not on the same segment of the path than the final position, we need to check on which segment of the path our next position"ll be.
                            * If the distance between the two points of the segment is shorter than the distance between the next position and the first point of the path, it means that
                            * the next position is not on this segment of the path.
                            */

                            while (currentPathIndex < path.size() - 1 && !dir2.isNulVector() && d2 > d1) {
                                //We need to check the next position on the next segment of the path.
                                currentPathIndex++;
                                //If we are arrived on the last point of the path, we don't need to check anymore because we are at the end of the path.
                                if (currentPathIndex < path.size() - 1) {
                                    dir1 = (path[currentPathIndex+1] - path[currentPathIndex]).normalize();
                                    nextPos = path[currentPathIndex] + dir1 * (d2 - d1) * aDot;

                                    //std::cout<<"next pos : "<<nextPos<<"dir 1 : "<<dir1<<" dir 2 : "<<dir2<<" d1 : "<<d1<<" d2 : "<<d2<<"d2 - d1 : "<<d2 - d1<<" dot : "<<dot<<std::endl;

                                    dir2 = (nextPos - path[currentPathIndex]).normalize();
                                    aDot = Math::abs(dir1.dot(dir2));
                                    d1 = (path[currentPathIndex+1] - path[currentPathIndex]).magnitude();
                                    d2 = (nextPos - path[currentPathIndex]).magnitude();
                                    //std::cout<<"dir 1 : "<<dir1<<" dir 2 : "<<dir2<<" d1 : "<<d1<<" d2 : "<<d2<<"d2 - d1 : "<<d2 - d1<<std::endl;
                                    /*std::string ok;
                                    std::cin>>ok;*/
                                }
                            }

                            //std::cout<<"next pos : "<<nextPos<<" current path index : "<<currentPathIndex<<"size : "<<path.size()<<std::endl;
                            //If we reach the last point of the path it means that we are at the end of the path, otherwise we return the next position.
                            return (currentPathIndex == path.size() - 1)  ? path[currentPathIndex] : nextPos;
                        }
                        //A negative time give a position on the path in the past, so, we need to do go back on our path.
                        if (time < 0) {
                            //The same as above except that we need to check the distance between the last point of the segment of the path.
                            dir1 = (path[currentPathIndex] - path[currentPathIndex+1]).normalize();
                            d1 = (path[currentPathIndex] - path[currentPathIndex+1]).magnitude();
                            Vec3f prevPos = actualPos - dir1 * speed * time;
                            dir2 = (prevPos - path[currentPathIndex+1]).normalize();
                            d2 = (prevPos - path[currentPathIndex+1]).magnitude();
                            float aDot = Math::abs(dir1.dot(dir2));
                            while (currentPathIndex > 0 && !dir2.isNulVector() && d2 > d1) {
                                currentPathIndex--;
                                //If the current index is 0, we don't need to check anymore because we are at the beginning of the path.
                                if (currentPathIndex > 0) {
                                    dir1 = (path[currentPathIndex] - path[currentPathIndex+1]).normalize();
                                    prevPos = path[currentPathIndex+1] - dir1 * (d1 - d2);
                                    dir2 = (prevPos - path[currentPathIndex+1]).normalize();
                                    d1 = (path[currentPathIndex+1] - path[currentPathIndex]).magnitude();
                                    d2 = (prevPos - path[currentPathIndex+1]).magnitude();
                                    aDot = Math::abs(dir1.dot(dir2));
                                }
                            }
                            //If we reach the last point of the path it means that we are at the end of the path, otherwise we return the next position.
                            return (currentPathIndex == 0)  ? path[currentPathIndex] : prevPos;
                        }
                    //If the time is equal to zero we have just to return the actual position.
                    return actualPos;
                }

                return actualPos;
            }
            template <typename T>
            static Vec2f getPosOnPathFromTime(Vec2f actualPos, std::vector<Vec2f> path, T time, float speed) {
                float txs = time * speed;
                if (path.size() > 1) {
                        unsigned int currentPathIndex = 0;
                        //We check the direction of the first segment of the path and the direction between out actual position and the first point of the first segment of our path.
                        Vec2f dir1 = (path[currentPathIndex+1] - path[currentPathIndex]).normalize();
                        Vec2f dir2 = (actualPos - path[currentPathIndex]).normalize();
                        float d1 = (path[currentPathIndex+1] - path[currentPathIndex]).magnitude();
                        float d2 = (actualPos - path[currentPathIndex]).magnitude();
                        /*std::cout<<"dir 1 : "<<dir1<<" dir 2 : "<<dir2<<" d1 : "<<d1<<" d2 : "<<d2<<std::endl;
                        std::cout<<"angle : "<<Math::abs(Math::toDegrees(dir1.getAngleBetween(dir2, dir1.cross(dir2))))<<std::endl;*/
                        //If the number of point is greater than 2, we need to check on which segment of the path our actual position is.
                        //If the directions are not the same it means that the point is not on this segment of the path.
                        //std::cout<<"pos : "<<actualPos<<"null vector ? "<<dir2.isNulVector()<<std::endl<<"d1 : "<<d1<<std::endl<<"d2 : "<<d2<<std::endl<<"angle : "<<Math::toDegrees(dir1.getAngleBetween(dir2))<<std::endl;
                        while (currentPathIndex < path.size() - 1 && !dir2.isNulVector() && !(Math::abs(Math::toDegrees(dir1.getAngleBetween(dir2))) <= 1 && d2 < d1)) {
                            //We need to check if the actual position is on the next segment of the path.
                            currentPathIndex++;
                            //If we are arrived on the last point of the path, we don't need to check anymore, it means that our position is not on the path. (Or we are already at the end of the path)
                            if (currentPathIndex < path.size() - 1) {
                                dir1 = (path[currentPathIndex+1] - path[currentPathIndex]).normalize();
                                dir2 = (actualPos - path[currentPathIndex]).normalize();
                                d1 = (path[currentPathIndex+1] - path[currentPathIndex]).magnitude();
                                d2 = (actualPos - path[currentPathIndex]).magnitude();
                                /*std::cout<<"dir 1 : "<<dir1<<" dir 2 : "<<dir2<<" d1 : "<<d1<<" d2 : "<<d2<<std::endl;
                                std::cout<<"angle : "<<Math::abs(Math::toDegrees(dir1.getAngleBetween(dir2, dir1.cross(dir2))))<<std::endl;*/
                            }
                        }
                        //std::cout<<"current index path : "<<currentPathIndex<<"size : "<<path.size()<<std::endl;
                       /*std::string str;
                        std::cin>>str;*/
                        //Our actual position is not on the path so we return our actual position.
                        if (currentPathIndex == path.size() -1 && time > 0 || currentPathIndex == 0 && time < 0)
                            return actualPos;
                        //If the time is positive, we need to check a position on the path in the future.
                        if (time > 0) {
                            /*We check the direction of the segment, the next position and the distances between the two point of the segment and
                            * the distance between the next position and the first point of the segment.
                            */
                            //dir1 = (path[currentPathIndex+1] - path[currentPathIndex]).normalize();
                            Vec2f nextPos = actualPos + dir1 * txs;
                            dir2 = (nextPos - path[currentPathIndex]).normalize();
                            d2 = (nextPos - path[currentPathIndex]).magnitude();
                            float aDot = Math::abs(dir1.dot(dir2));

                            //std::cout<<"actual pos : "<<actualPos<<"next pos : "<<nextPos<<"path : "<<path[currentPathIndex]<<std::endl;
                            //std::cout<<"actual pos : "<<actualPos<<" next pos : "<<nextPos<<std::endl;
                            //std::cout<<"actual pos : "<<actualPos<<"next pos : "<<nextPos<<"dir 1 : "<<dir1<<"dir 2 : "<<dir2<<"d1 : "<<d1<<"d2 : "<<d2<<std::endl;
                            /*If the actual position is not on the same segment of the path than the final position, we need to check on which segment of the path our next position"ll be.
                            * If the distance between the two points of the segment is shorter than the distance between the next position and the first point of the path, it means that
                            * the next position is not on this segment of the path.
                            */

                            while (currentPathIndex < path.size() - 1 && !dir2.isNulVector() && d2 > d1) {
                                //We need to check the next position on the next segment of the path.
                                currentPathIndex++;
                                //If we are arrived on the last point of the path, we don't need to check anymore because we are at the end of the path.
                                if (currentPathIndex < path.size() - 1) {
                                    dir1 = (path[currentPathIndex+1] - path[currentPathIndex]).normalize();
                                    nextPos = path[currentPathIndex] + dir1 * (d2 - d1) * aDot;

                                    //std::cout<<"next pos : "<<nextPos<<"dir 1 : "<<dir1<<" dir 2 : "<<dir2<<" d1 : "<<d1<<" d2 : "<<d2<<"d2 - d1 : "<<d2 - d1<<" dot : "<<aDot<<std::endl;

                                    dir2 = (nextPos - path[currentPathIndex]).normalize();
                                    aDot = Math::abs(dir1.dot(dir2));
                                    d1 = (path[currentPathIndex+1] - path[currentPathIndex]).magnitude();
                                    d2 = (nextPos - path[currentPathIndex]).magnitude();
                                    //std::cout<<"dir 1 : "<<dir1<<" dir 2 : "<<dir2<<" d1 : "<<d1<<" d2 : "<<d2<<"d2 - d1 : "<<d2 - d1<<std::endl;
                                    /*std::string ok;
                                    std::cin>>ok;*/
                                }
                            }

                            //std::cout<<"next pos : "<<nextPos<<" current path index : "<<currentPathIndex<<"size : "<<path.size()<<std::endl;
                            //If we reach the last point of the path it means that we are at the end of the path, otherwise we return the next position.
                            return (currentPathIndex == path.size() - 1)  ? path[currentPathIndex] : nextPos;
                        }
                        //A negative time give a position on the path in the past, so, we need to do go back on our path.
                        if (time < 0) {
                            //The same as above except that we need to check the distance between the last point of the segment of the path.
                            dir1 = (path[currentPathIndex] - path[currentPathIndex+1]).normalize();
                            d1 = (path[currentPathIndex] - path[currentPathIndex+1]).magnitude();
                            Vec2f prevPos = actualPos - dir1 * txs;
                            dir2 = (prevPos - path[currentPathIndex+1]).normalize();
                            d2 = (prevPos - path[currentPathIndex+1]).magnitude();
                            float aDot = Math::abs(dir1.dot(dir2));
                            while (currentPathIndex > 0 && !dir2.isNulVector() && d2 > d1) {
                                currentPathIndex--;
                                //If the current index is 0, we don't need to check anymore because we are at the beginning of the path.
                                if (currentPathIndex > 0) {
                                    dir1 = (path[currentPathIndex] - path[currentPathIndex+1]).normalize();
                                    prevPos = path[currentPathIndex+1] - dir1 * (d1 - d2);
                                    dir2 = (prevPos - path[currentPathIndex+1]).normalize();
                                    d1 = (path[currentPathIndex+1] - path[currentPathIndex]).magnitude();
                                    d2 = (prevPos - path[currentPathIndex+1]).magnitude();
                                    aDot = Math::abs(dir1.dot(dir2));
                                }
                            }
                            //If we reach the last point of the path it means that we are at the end of the path, otherwise we return the next position.
                            return (currentPathIndex == 0)  ? path[currentPathIndex] : prevPos;
                        }
                    //If the time is equal to zero we have just to return the actual position.
                    return actualPos;
                }

                return actualPos;
            }
            static bool overlap(Vec2f v1, Vec2f v2) {
                return (v1.y >= v2.x) || (v1.x >= v2.y);
            }
            static Vec2f projectShapeOnAxis(Vec3f axis, std::vector<Vec3f> vertices) {
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
            static int checkNearestVertexFromShape (Vec3f center, std::vector<Vec3f> points, std::vector<Vec3f> edgeBissectors, std::vector<Vec3f> edgeNormals, std::vector<Vec3f> faceBissectors, std::vector<Vec3f> faceNormals, std::vector<Vec3f> vertices,
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
                            Plane p (edgeNormals[i*nbEdgesPerFace+k], edgeBissectors[i*nbEdgesPerFace+k]);
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
            template <std::size_t N>
            static Vec2f projectShapeOnAxis(Vec3f axis, std::array<Vec3f, N> vertices) {
                float min = 0, max = 0;
                if (vertices.size() > 0) {
                    min = vertices[0].projOnAxis(axis);
                    max = min;
                    for (unsigned int i = 1; i < vertices.size(); i++) {
                        // NOTE: the axis must be normalized to get accurate projections
                        //if (!axis.isNulVector() && !vertices[i].isNulVector()) {
                            float p = vertices[i].projOnAxis(axis);
                            if (p < min) {
                                min = p;
                            }
                            if (p > max) {
                                max = p;
                            }
                        //}
                    }
                }
                return Vec2f(min, max);
            }
            static Plane computeSeparatingPlane(std::vector<Vec3f> points);
        };
    }
}
#endif
