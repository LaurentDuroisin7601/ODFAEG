#ifndef VEC4
#define VEC4
#include <iostream>
#include "maths.h"

#include <SFML/System.hpp>
#include "../../../include/odfaeg/Core/erreur.h"
#include "../Core/serialization.h"
#include "vec2f.h"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace math {
        class Matrix3f;
        /**
          * \file vec4.h
          * \class Vec3f
          * \brief Manage a 3D vector
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          *
          * This class defines 3D vectors of floating numbers and all operations about vectors.
          * This class redefines the class sf::Vector3f and offers more operations.
          * Vectors are very usefull for physic's and mathematic's algorithms.
          * In ODFAEG, vectors are used to define a direction or a point.
          */
        class ODFAEG_API_EXPORT Vec3f : public sf::Vector3f {
            public :
            static const Vec3f xAxis; /**< Vec3f the x axis.*/
            static const Vec3f yAxis; /**< Vec2f the y axis.*/
            static const Vec3f zAxis; /**< Vec3f the z axis.*/
            /**\fn Vec3f()
            *  \brief default constructror.
            * construct a null vector (with 0, 0, 0 as coordinates)
            */
            Vec3f ();
            /**\fn Vec2f(float x, float y, float)
            *  \brief constructror.
            * construct a vector with the given coordinates.
            *  \param x : the x coordinate.
            *  \param y : the y coordinate.
            *  \param z : the z coordinate.
            */
            Vec3f (float x, float y, float z);
            /**\fn Vec2f(float x, float y, float)
            *  \brief constructror.
            * construct a vector with the given coordinates.
            *  \param x : the x coordinate.
            *  \param y : the y coordinate.
            *  \param z : the z coordinate.
            *  \param w : the w coordinate. (Used for 3D projections)
            */
            Vec3f (float x, float y, float z, float w);
            Vec3f (const Vec2f &other);
            /**\fn void set (float x, float y, float)
            *  \brief set the x, y and z coordinates of the vector.
            *  \param x : the x coordinate.
            *  \param y : the y coordinate.
            *  \param z : the z coordinate.
            */
            void set(float x, float y, float z);
            /**
            * \fn bool isNulVector() const
            * \brief return true if the vector is null, false otherwise.
            * \return true if the vector is null (0, 0, 0).
            */
            bool isNulVector () const;
            /**
            * \fn float operator[] (int i)
            * \brief retrieve one coordinate from the vector.
            * \param i : the indice of the vector coordinate. (0 = x, 1 = y and 2 = z)
            * \return the value of the coordinate.
            */
            float& operator[] (unsigned int i);
            /**
            * \fn Vec3f operator+ (const Vec3f &other)
            * \brief add a vector to another and return the resulting vector.
            * \param the vector to be added with.
            * \return the resulting vector.
            */
            Vec3f operator+ (const Vec3f &other);
            /** \fn Vec3f operator- (const Vec3f other)
            *   \brief substract the vector from another one and return the resulting vector.
            *   \param the other vector to be substracted from.
            *   \return the resulting vector.
            */
            Vec3f operator- (const Vec3f &other);
            /** \fn Vec3f operator* (const Vec3f other)
            *   \brief multiply the vector by another one and return the resulting vector.
            *   \param the other vector to be multiplied by.
            *   \return the resulting vector.
            */
            Vec3f operator* (const Vec3f &other);
            /** \fn Vec2f operator* (const Vec2f other)
            *   \brief divide the vector by another one and return the resulting vector.
            *   \param the other vector to be devided by.
            *   \return the resulting vector.
            */
            Vec3f operator/ (const Vec3f &other);
            /** \fn bool operator== (const Vec3f &other)
            *   \brief compare if two vectors ar equals. (They are equals of the coordiates are the same)
            *   \param the other vector to be compared with.
            *   \return true if the two vectors are equal, false otherwise.
            */
            Vec3f operator/ (float scalar);
            Vec3f& operator= (const Vec2f &other);
            Vec3f projOnVector(Vec3f other);
            bool  isOpposite (const Vec3f &other) const;
            /** \fn bool operator== (const Vec3f &other)
            *   \brief compare if two vectors ar equals. (They are equals of the coordiates are the same)
            *   \param the other vector to be compared with.
            *   \return true if the two vectors are equal, false otherwise.
            */
            bool operator== (const Vec3f &other);
            bool operator!= (const Vec3f& other);
            /** \fn Vec2f operator-()
            *   \brief return the opposite of the vector
            *   \return the opposite of the vector.
            */
            Vec3f operator- () const;
            /** \fn void operator-= (const Vec2f other)
            *   \brief substract the vector from another one.
            *   \param the other vector to be added with.
            */
            Vec3f operator+ (const float number);
            Vec3f& operator += (const Vec3f &other);
            /** \fn void operator*= (const Vec2f other)
            *   \brief multiply the vector by another one.
            *   \param the other vector to be multiplied by.
            */
            Vec3f& operator -= (const Vec3f &other);
            /** \fn void operator*= (const Vec2f other)
            *   \brief multiply the vector by another one.
            *   \param the other vector to be multiplied by.
            */
            Vec3f& operator *= (const Vec3f &other);
            /** \fn void operator/= (const Vec2f other)
            *   \brief devide the vector by another one.
            *   \param the other vector to be devided by.
            */
            Vec3f& operator *= (const float scale);
            /** \fn void operator/= (const Vec2f other)
            *   \brief divide the vector by another one.
            *   \param the other vector to be devided by.
            */
            Vec3f& operator /= (const Vec3f &other);
            /** \fn Vec3f fabs()
            *   \brief return absolute coordinates of the vector.
            *   \return the vector with absolute coordinates.
            */
            Vec3f fabs () const;
            /** \fn void operator*= (const float scalar)
            *   \brief multiply the vector by a scalar.
            *   \param the scalar to be multiplied by.
            */
            Vec3f operator* (const float scale);
             /** \fn float computeDist (const Vec3f &other)
            *   \brief compute the distance between the two vectors.
            *   \param the other vector.
            *   \return the distance between the two vectors.
            */
            float computeDist (const Vec3f &other);
            /** \fn float magnitude ()
            *   \brief compute the length of the vector.
            *   \return the length of the vector.
            */
            float magnitude () const;
            /** \fn float magnSquared ()
            *   \brief compute the squared length of the vector.
            *   \return the squared length of the vector.
            */
            float magnSquared () const;
            float computeDistSquared(const Vec3f& other);
            /** \fn Vec2f normalize ()
            *   \brief transform the vector to a vector with a length of 1 and return the resulting vector.
            *   \return the resulting vector.
            */
            Vec3f normalize () const ;
            /** \fn Vec2f normalize ()
            *   \brief transform the vector to a 3D vector by dividing the x, y and z component by the w component.
            *   \return the resulting vector.
            */
            Vec3f normalizeToVec3 () const;
            /** \fn float dot2 (const Vec3f &other)
            *   \brief compute the dot product between to vectors. (using the first method)
            *    the dot product is the cosinus of the angle between the vector and another one.
            *    the length of the two vectors needs to be equal to 1 before performing the dot product.
            *   \return the dot product between the two vectors.
            */
            float dot3 (const Vec3f &other);
            float dot2 (const Vec3f &other);
            /** \fn float dot2 (const Vec3f &other)
            *   \brief get the angle between the vector and another one.
            *   the angle is given in radians and he's always between -2PI and 2PI.
            *   \return the angle between the vectors.
            */
            float dot (const Vec3f &other) const;
            /** \fn Vec3f cross (const Vec3f &other)
            *   \brief do the cross product and return the resulting vector (The perpendicular to the vector and the other vector.)
            *   \param the other vector.
            *   \return the vector witch is perpendicular to another one.
            */
            Vec3f cross (const Vec3f &other) const;
            /** \fn float getAngleBetween (const Vec3f &other, const Vec3f &n)
            *   \brief return the angle between two vectors. (depending of the plane's orientation)
            *   the angle is given in radians and is always between -2PI and 2PI.
            *   \param other : the other vector.
            *   \param n : the orientation of the plane formed by the two vectors. (the normal of the plane)
            *   \return the angle between the two vectors.
            */
            float getAngleBetween (const Vec3f &other, const Vec3f &n);
            /** \fn float projectOnAxis (const Vec3f &other)
            *   \brief Project an other vector on the vector and return the result.
            *   The projection result is the dot product of the two vectors multiplied by the length
            *   of the other vector.
            *   \return the result of the projection.
            */
            float projOnAxis (const Vec3f &other);
            /** \fn float* getVec3 () const;
            *   \brief return the vector's components to an array.
            */
            float* getVec3 () const;
            /** \fn sf::Vector3f getVec3sf () const;
            *   \brief transform the vector to an SFML vector.
            *   \return an SFML vector.
            */
            Vec3f& operator= (const Vec3f& other);
            sf::Vector3f getVec3sf() const;
            template <typename Archive>
            void serialize (Archive &ar) {
                ar(x);
                ar(y);
                ar(z);
                ar(w);
            }
            //virtual ~Vec3f() {}
            /** \fn std::ostream& operator<< (std::ostream &out, const Vec3f &vec3)
            *   \brief set the vector coordinates to an output stream.
            *   \param &out : the output stream.
            *   \param &vec3 : the vector.
            *   \return the final output stream.
            */
            friend std::ostream& operator<< (std::ostream  &out, const Vec3f &vec3);
            float w; /** < the w component of the vector.*/
        };
        std::ostream& operator<< (std::ostream  &out, const Vec3f &vec4);
    }
}
#endif




