module;
#include<array>
#include <iostream>
#include <tuple>
#include <cmath>
export module odfaeg.math.matrix;
import odfaeg.math.vec;
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
export namespace odfaeg {
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
       
        template <typename T, unsigned int R, unsigned int C>
        class Mat {
        private:
            std::array<std::array<T, R>, C> data;
            /**
              * \fn Matrix4f()
              * \brief constructor. (Construct the identity matrix)
            */
        public:
            Mat();
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
            template <typename... Args>
            Mat(Args... args) requires (sizeof...(Args) == R * C);

            Mat(const Mat<T, R, C>& other);
            template <unsigned int R2, unsigned int C2>
            Mat(const Mat<T, R2, C2>& other);
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
            template <typename... Args, unsigned int RI = 0>
            void set(Args... args) requires (sizeof...(Args) == R * C);
            
            /**
            * \fn zero()
            * \brief set the null matrix.
            */
            void zero();
            /**
            * \fn identity()
            * \brief set the identity matrix
            */
            void identity();
            /**
            * \fn Matrix4f operator+ (const Matrix4f &other)
            * \brief add the matrix to another matrix and return the resulting matrix.
            * \param the matrix to be added with.
            * \return the resulting matrix.
            */
            Mat<T, R, C> operator+ (const Mat<T, R, C>& other);
            /**
            * \fn Matrix4f operator- (const Matrix4f &other)
            * \brief substract the matrix from another matrix and return the resulting matrix.
            * \param the matrix to be substracted with.
            * \return the resulting matrix.
            */
            Mat<T, R, C> operator- (const Mat<T, R, C>& other);
            /**
            * \fn Matrix4f operator* (const Matrix4f &other)
            * \brief multiply the matrix by another matrix and return the resulting matrix.
            * \param the matrix to be multiplied by.
            * \return the resulting matrix.
            */
            Mat<T, R, C> operator* (const Mat<T, R, C>& other);
            /**
            * \fn Vec3f operator* (const Vec3f &vec3)
            * \brief multiply the matrix by a vector and return the resulting vector.
            * \param the vector to be multiplied by.
            * \return the resulting vector.
            */
            VecN<T, R> operator* (const VecN<T, C>& vec);
            /**
            * \fn Matrix4f operator* (float scalar)
            * \brief multiply the matrix by a scalar and return the resulting matrix.
            * \param the scalar to be multiplied by.
            * \return the resulting matrix.
            */
            Mat<T, R, C> operator* (float scalar);                
            /**
            * \fn Matrix3f inverse() throw (std::exception&)
            * \brief return the inverse of the matrix, throw an exception if the matrix isn't inversible.
            * \return the inversed matrix.
            */
            template <unsigned int N>
            Mat<T, N - 1, N - 1> getMinorMatrix(const Mat<T, N, N>& mat, unsigned int row, unsigned int col);
            Mat<T, R, C> inverse();
            /**
            * \fn float getDet() const
            * \brief return the value of the determiant of the matrix.
            * \return the value of the determinant.
            */
            template <unsigned int N>
            float getDetOfMinorMatrix() requires (N == 2);
            template <unsigned int N, class...D>
            float getDetOfMinorMatrix() requires (N > 2);
            float getDet();
            /**
            * \fn Matrix4f& operator= (const Matrix4f &other)
            * \brief set the matrix elements from the other matrix elements and return a reference to this matrix.
            * \param a refenrence to the current matrix.
            */
            Mat<T, R, C>& operator= (const Mat<T, R, C>& other);
            template <unsigned int R2, unsigned int C2>
            Mat<T, R, C>& operator= (const Mat<T, R2, C2>& other);
            std::array<T, C>& operator[] (unsigned int i);
            std::array<T, C>& operator[] (unsigned int i) const;
            bool operator==(const Mat<T, R, C>& other) const;
            std::array<float, 16> toGlMatrix();
            Mat<T, R, C> transpose();
            template <typename Archive>
            void serialize(Archive& ar);
        };
        template <typename T, unsigned int R, unsigned int C>
        std::ostream& operator<< (std::ostream& out, const Mat<T, R, C>& mat);
        using Matrix2f =  Mat<float, 2, 2>;
        using Matrix3f =  Mat<float, 3, 3>;
        using Matrix4f =  Mat<float, 4, 4>;
    }
}
module : private;
#include "matrix.inl"