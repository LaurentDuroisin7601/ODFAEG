#ifndef MATRIX_2
#define MATRIX_2
#include "vec2f.h"
#include "export.hpp"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace math {
        /**
          * \file matrix2.h
          * \class Matrix2
          * \brief Manager a 2D matrix.
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          *
          * Manage a 2D matrix who is used to perform base changements.
          */
        class ODFAEG_MATH_API Matrix2f {
            public :
                float m11, m12; /** < the elements of the first row matrix.*/
                float m21, m22; /** < the elements of the second row matrix.*/
                /**
                * \fn Matrix2f()
                * \brief constructor. (Construct the identity matrix)
                */
                Matrix2f();
                /**
                * \fn Matrix2f (float, foat, float, float)
                * \brief  constructor.(Construct a matrix with the given elements)
                * \param m11 the first element of the first row of the matrix.
                * \param m12 the second element of the first row of the matrix.
                * \param m21 the first element of the second row of the matrix.
                * \param m22 the second element of the second row of the matrix.
                */
                Matrix2f(float m11, float m12, float m21, float m22);
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
                template <typename Archive>
                void serialize(Archive & ar) {
                    //std::cout<<"m11 : "<<m11<<std::endl;
                    ar(m11);
                    //std::cout<<"m12 : "<<m12<<std::endl;
                    ar(m12);
                    //std::cout<<"m21 : "<<m21<<std::endl;
                    ar(m21);
                    //std::cout<<"m22 : "<<m22<<std::endl;
                    ar(m22);
                }
                /**
                * \fn set (float m11, float m12, float m21, float m22)
                * \brief set the matrix with the given elements.
                * \param m11 the first element of the first row of the matrix.
                * \param m12 the second element of the first row of the matrix.
                * \param m21 the first element of the second row of the matrix.
                * \param m22 the second element of the second row of the matrix
                */
                void set(float m11, float m12, float m21, float m22);
                /**
                * \fn Matrix2f operator+ (const Matrix2f &other)
                * \brief add the matrix to another matrix and return the resulting matrix.
                * \param the matrix to be added with.
                * \return the resulting matrix.
                */
                Matrix2f operator+ (const Matrix2f &other);
                /**
                * \fn Matrix2f operator- (const Matrix2f &other)
                * \brief substract the matrix from another matrix and return the resulting matrix.
                * \param the matrix to be substracted with.
                * \return the resulting matrix.
                */
                Matrix2f operator- (const Matrix2f &other);
                /**
                * \fn Matrix2f operator* (const Matrix2f &other)
                * \brief multiply the matrix by another matrix and return the resulting matrix.
                * \param the matrix to be multiplied by.
                * \return the resulting matrix.
                */
                Matrix2f operator* (const Matrix2f &other);
                /**
                * \fn Vec2f operator* (const Vec2f &vec2)
                * \brief multiply a vector by the matrix and return the resulting vector.
                * \param the vector to be multiplied by.
                * \return the resulting vector.
                */
                Vec2f operator* (const Vec2f &vec2);
                /**
                * \fn Matrix2f operator* (const float scalar)
                * \brief multiply the matrix by a scalar and return the resulting matrix.
                * \param the scalar to be multiplied by.
                * \return the resulting matrix.
                */
                Matrix2f operator* (const float scalar);
                /**
                * \fn Matrix2f inverse() throw (std::exception&)
                * \brief return the inverse of the matrix, throw an exception if the matrix isn't inversible.
                * \return the inversed matrix.
                */
                Matrix2f inverse ();
                /**
                * \fn float getDet() const
                * \brief return the value of the determiant of the matrix.
                * \return the value of the determinant.
                */
                float getDet () const;
                /**
                * \fn Matrix2f& operator= (const Matrix2f &other)
                * \brief set the matrix elements from the other matrix elements and return a reference to this matrix.
                * \param a refenrence to the current matrix.
                */
                Matrix2f& operator= (const Matrix2f &other);
                /**
                * \fn std::ostream& operator<< (std::ostream &out, const Matrix2f &mat2)
                * \brief print the matrix elements
                * \param out : the outputstream.
                * \param mat2 : the matrix.
                * \return the final outputstream.
                */
                friend std::ostream& operator<< (std::ostream &out, const Matrix2f &mat2);
        };
        std::ostream& operator<< (std::ostream &out, const Matrix2f &mat2);
    }
}
#endif
