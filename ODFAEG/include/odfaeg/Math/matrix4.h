#ifndef MATRIX4
#define MATRIX4
#include "vec4.h"
#include "../../../include/odfaeg/Core/erreur.h"
#include "matrix3.h"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */

namespace odfaeg {
    namespace math {
        /**
          * \file matrix4.h
          * \class Matrix4f
          * \brief Manage a 4D matrix.
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          *
          * Manage a 3D matrix who is used to perform 3D transformations and projections.
          */
        class ODFAEG_MATH_API Matrix4f  {
            friend class Matrix3f;
            public :
            float m11, m12, m13, m14, /** < the elements of the first row matrix.*/
                  m21, m22, m23, m24, /** < the elements of the second row matrix.*/
                  m31, m32, m33, m34, /** < the elements of the thirst row matrix.*/
                  m41, m42, m43, m44; /** < the elements of the fourth row matrix.*/
            /**
              * \fn Matrix4f()
              * \brief constructor. (Construct the identity matrix)
            */
            Matrix4f ();
            /**
            * \fn Matrix4f (float, float, float, float, float, float, float, float, float, float, float, float, float, float)
            * \brief  constructor.(Construct a matrix with the given elements)
            * \param m11 the first element of the first row of the matrix.
            * \param m12 the second element of the first row of the matrix.
            * \param m13 the thirst element of the first row of the matrix.
            * \param m14 the fourth element of the first row of the matrix.
            * \param m21 the first element of the second row of the matrix.
            * \param m22 the second element of the second row of the matrix.
            * \param m23 the thirst element of the second row of the matrix.
            * \param m24 the fourth element of the second row of the matrix.
            * \param m31 the first element of the thirst row of the matrix.
            * \param m32 the second element of the thirst row of the matrix.
            * \param m33 the thirst element of the thirst row of the matrix.
            * \param m34 the fourth element of the thirst row of the matrix.
            * \param m41 the fourth element of the first row of the matrix.
            * \param m42 the fourth element of the second row of the matrix.
            * \param m43 the fourth element of the thirst row of the matrix.
            * \param m44 the fourth element of the fourth row of the matrix.
            */
            Matrix4f (float m11, float m12, float m13, float m14, float m21, float m22, float m23, float m24,
            float m31, float m32, float m33, float m34, float m41, float m42, float m43, float m44);
            Matrix4f (Matrix3f matrix3f);
            /**
            * \fn setM4f (float, float, float, float, float, float, float, float, float, float, float, float, float, float, float, float, flaot)
            * \brief  constructor.(Construct a matrix with the given elements)
            * \param m11 the first element of the first row of the matrix.
            * \param m12 the second element of the first row of the matrix.
            * \param m13 the thirst element of the first row of the matrix.
            * \param m14 the fourth element of the first row of the matrix.
            * \param m21 the first element of the second row of the matrix.
            * \param m22 the second element of the second row of the matrix.
            * \param m23 the thirst element of the second row of the matrix.
            * \param m24 the fourth element of the second row of the matrix.
            * \param m31 the first element of the thirst row of the matrix.
            * \param m32 the second element of the thirst row of the matrix.
            * \param m33 the thirst element of the thirst row of the matrix.
            * \param m34 the fourth element of the thirst row of the matrix.
            * \param m41 the fourth element of the first row of the matrix.
            * \param m42 the fourth element of the second row of the matrix.
            * \param m43 the fourth element of the thirst row of the matrix.
            * \param m44 the fourth element of the fourth row of the matrix.
            */
            void setM4f(float m11, float m12, float m13, float m14, float m21, float m22, float m23, float m24,
            float m31, float m32, float m33, float m34, float m41, float m42, float m43, float m44);
            /**
            * \fn zero()
            * \brief set the null matrix.
            */
            void zero ();
            /**
            * \fn identity()
            * \brief set the identity matrix
            */
            void identity ();
            /**
            * \fn Matrix4f operator+ (const Matrix4f &other)
            * \brief add the matrix to another matrix and return the resulting matrix.
            * \param the matrix to be added with.
            * \return the resulting matrix.
            */
            Matrix4f operator+ (const Matrix4f &other);
            /**
            * \fn Matrix4f operator- (const Matrix4f &other)
            * \brief substract the matrix from another matrix and return the resulting matrix.
            * \param the matrix to be substracted with.
            * \return the resulting matrix.
            */
            Matrix4f operator- (const Matrix4f &other);
            /**
            * \fn Matrix4f operator* (const Matrix4f &other)
            * \brief multiply the matrix by another matrix and return the resulting matrix.
            * \param the matrix to be multiplied by.
            * \return the resulting matrix.
            */
            Matrix4f operator* (const Matrix4f &other);
            /**
            * \fn Vec3f operator* (const Vec3f &vec3)
            * \brief multiply the matrix by a vector and return the resulting vector.
            * \param the vector to be multiplied by.
            * \return the resulting vector.
            */
            Vec3f operator* (const Vec3f &vec3);
            /**
            * \fn Matrix4f operator* (float scalar)
            * \brief multiply the matrix by a scalar and return the resulting matrix.
            * \param the scalar to be multiplied by.
            * \return the resulting matrix.
            */
            Matrix4f operator* (float scalar);
            /**
            * \fn Matrix3f inverse() throw (std::exception&)
            * \brief return the inverse of the matrix, throw an exception if the matrix isn't inversible.
            * \return the inversed matrix.
            */
            Matrix4f inverse ();
            /**
            * \fn float getDet() const
            * \brief return the value of the determiant of the matrix.
            * \return the value of the determinant.
            */
            float getDet ();
            /**
            * \fn Matrix4f& operator= (const Matrix4f &other)
            * \brief set the matrix elements from the other matrix elements and return a reference to this matrix.
            * \param a refenrence to the current matrix.
            */
            Matrix4f& operator= (const Matrix4f &ohter);
            friend std::ostream& operator<< (std::ostream &out, const Matrix4f &mat4);
            bool operator==(const Matrix4f& other);
            std::array<float, 16> toGlMatrix();
            Matrix4f transpose();
            template <typename Archive>
            void serialize(Archive & ar) {
                ar(m11);
                ar(m12);
                ar(m13);
                ar(m14);
                ar(m21);
                ar(m22);
                ar(m23);
                ar(m24);
                ar(m31);
                ar(m32);
                ar(m33);
                ar(m34);
                ar(m41);
                ar(m42);
                ar(m43);
                ar(m44);
            }
        };
        std::ostream& operator<< (std::ostream &out, const Matrix4f &mat4);
        std::istream& operator>> (std::istream &in, Matrix4f& mat4);
    }
}
#endif
