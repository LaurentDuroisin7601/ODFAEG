#ifndef ODFAEG_COMPUTER_HPP
#define ODFAEG_COMPUTER_HPP
#include <limits.h>
#include  <array>
#include  <vector>
#include  <cmath>
#include "vec.hpp"
#include "matrix.hpp"
#include "plane.hpp"
#include "maths.hpp"
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
        class Computer {
        public:        
            static Vec2f getMoy(std::vector<Vec2f> verts);
            static Vec3f getMoy(std::vector<Vec3f> verts);
            //Calculs les minimum est maximum d'un vecteurs pass� et les stocke dans un tableau.
            static std::array<std::array<float, 2>, 3> getExtends(std::vector<Vec3f> verts);
            static std::array<std::array<float, 2>, 2> getExtends(std::vector<Vec2f> verts);
            static Plane computeSeparatingPlane(std::vector<Vec3f> points);
            template <std::size_t N>
            static std::array<std::array<float, 2>, 2> getExtends(const std::array<Vec2f, N>& verts);
            /**\fn std::array<std::array<float, 3>,2> getExtends (const std::array<Vec3f, N>& verts);
            *  \brief get the minimum and the maximum x, y and z from an array of 3D vectors.
            *  \param the array of the vectors.
            *  \return an array containing the minimum and maximum values.
            */
            template <std::size_t N>
            static std::array<std::array<float, 2>, 3> getExtends(const std::array<Vec3f, N>& verts);
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
            static Vec3f getPosOnPathFromTime(Vec3f actualPos, std::vector<Vec3f> path, T time, float speed);
            template <typename T>
            static Vec2f getPosOnPathFromTime(Vec2f actualPos, std::vector<Vec2f> path, T time, float speed);
            static bool overlapThisNormal(std::vector<Vec3f> verticesA, std::vector<Vec3f> verticesB, Vec3f normal);
            static bool overlap(float minA, float maxA, float minB, float maxB);
            static Vec2f projectShapeOnAxis(Vec3f axis, std::vector<Vec3f> vertices);
            static int checkNearestVertexFromShape(Vec3f center, std::vector<Vec3f> points, std::vector<Vec3f> edgeBissectors, std::vector<Vec3f> edgeNormals, std::vector<Vec3f> faceBissectors, std::vector<Vec3f> faceNormals, std::vector<Vec3f> vertices,
                float& distMin, int& ptIndex, int& edgeIndex, int& faceIndex, int nbEdgesPerFace);
            template <std::size_t N>
            static Vec2f projectShapeOnAxis(Vec3f axis, std::array<Vec3f, N> vertices);           
        };
    }
}
#endif