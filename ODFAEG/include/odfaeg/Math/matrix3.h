#ifndef MATRIX3
#define MATRIX3
#include "vec4.h"
#include "vec2f.h"
#include "../../../include/odfaeg/Core/erreur.h"
#include "export.hpp"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace math {
        class Matrix4f;
        /**
          * \file matrix3.h
          * \class Matrix3f
          * \brief Manager a 3D matrix.
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          *
          * Manage a 3D matrix who is used to perform 2D transformations.
          */
        class ODFAEG_MATH_API Matrix3f {
            friend class Matrix4f;
            public :
                float m11, m12, m13; /** < the elements of the first row matrix.*/
                float m21, m22, m23; /** < the elements of the second row matrix.*/
                float m31, m32, m33; /** < the elements of the thirst row matrix.*/
                /**
                * \fn Matrix3f()
                * \brief constructor. (Construct the identity matrix)
                */
                Matrix3f();
                /**
                * \fn Matrix3f (float, float, float, float, float, float, float, float, float)
                * \brief  constructor.(Construct a matrix with the given elements)
                * \param m11 the first element of the first row of the matrix.
                * \param m12 the second element of the first row of the matrix.
                * \param m13 the thirst element of the first row of the matrix.
                * \param m21 the first element of the second row of the matrix.
                * \param m22 the second element of the second row of the matrix.
                * \param m23 the thirst element of the second row of the matrix.
                * \param m31 the first element of the thirst row of the matrix.
                * \param m32 the second element of the thirst row of the matrix.
                * \param m33 the thirst element of the thirst row of the matrix.
                */
                Matrix3f(float m11, float m12, float m13, float m21, float m22, float m23, float m31, float m32, float m33);
                Matrix3f(Matrix4f matrix4f);
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
                * \fn set (float m11, float m12, float m13, float m21, float m22, float m23, float m31, float m32, float m33)
                * \brief set the matrix with the given elements.
                * \param m11 the first element of the first row of the matrix.
                * \param m12 the second element of the first row of the matrix.
                * \param m13 the thirst element of the first tow of the matrix.
                * \param m21 the first element of the second row of the matrix.
                * \param m22 the second element of the second row of the matrix.
                * \param m23 the thirst element of the second tow of the matrix.
                * \param m31 the first element of the thirst row of the matrix
                * \param m32 the second element of the thirst row of the matrix.
                * \param m33 the thirst element of the thirst row of the matrix.
                */
                void set(float m11, float m12, float m13, float m21, float m22, float m23, float m31, float m32, float m33);
                /**
                * \fn Matrix3f operator+ (const Matrix3f &other)
                * \brief add the matrix to another matrix and return the resulting matrix.
                * \param the matrix to be added with.
                * \return the resulting matrix.
                */
                Matrix3f operator+ (const Matrix3f &other);
                /**
                * \fn Matrix3f operator- (const Matrix3f &other)
                * \brief substract the matrix from another matrix and return the resulting matrix.
                * \param the matrix to be substracted with.
                * \return the resulting matrix.
                */
                Matrix3f operator- (const Matrix3f &other);
                /**
                * \fn Matrix3f operator* (const Matrix3f &other)
                * \brief multiply the matrix by another matrix and return the resulting matrix.
                * \param the matrix to be multiplied by.
                * \return the resulting matrix.
                */
                Matrix3f operator* (const Matrix3f &other);
                /**
                * \fn Matrix3f operator* (float scalar)
                * \brief multiply the matrix by a scalar and return the resulting matrix.
                * \param the scalar to be multiplied by.
                * \return the resulting matrix.
                */
                Matrix3f operator* (const float scalar);
                /**
                * \fn Vec3f operator* (const Vec3f &vec3)
                * \brief multiply a vector by the matrix and return the resulting vector.
                * \param the vector to be multiplied by.
                * \return the resulting vector.
                */
                Vec3f operator* (const Vec3f &vec3);
                /**
                * \fn Vec2f operator* (const Vec2f &vec2)
                * \brief multiply a 2D vector by the matrix and return the resulting vector.
                * \param the vector to be multiplied by.
                * \return the resulting vector.
                */
                Vec2f operator* (const Vec2f &vec2);
                /**
                * \fn Vec2f operator* (const Vec3f &vec3)
                * \brief multiply a 3D vector by the matrix and return the resulting 2D vector.
                * \param the vector to be multiplied by.
                * \return the resulting vector.
                */
                Vec2f mult (const Vec3f &vec3);

                /**
                * \fn Matrix3f inverse() throw (std::exception&)
                * \brief return the inverse of the matrix, throw an exception if the matrix isn't inversible.
                * \return the inversed matrix.
                */
                Matrix3f inverse ();
                /**
                * \fn float getDet() const
                * \brief return the value of the determiant of the matrix.
                * \return the value of the determinant.
                */
                float getDet () const;
                /**
                * \fn Matrix3f& operator= (const Matrix2f &other)
                * \brief set the matrix elements from the other matrix elements and return a reference to this matrix.
                * \param a refenrence to the current matrix.
                */
                Matrix3f& operator= (const Matrix3f &ohter);
                /**
                * \fn std::ostream& operator<< (std::ostream &out, const Matrix2f &mat2)
                * \brief print the matrix elements
                * \param out : the outputstream.
                * \param mat2 : the matrix.
                * \return the final outputstream.
                */
                friend std::ostream& operator<< (std::ostream &out, const Matrix3f &mat3);
        };
        std::ostream& operator<< (std::ostream &out, const Matrix3f &matrix3);
    }
}
#endif
