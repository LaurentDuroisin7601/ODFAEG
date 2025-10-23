#ifndef MATRIX4
#define MATRIX4
#include "vec4.h"
#include "../../../include/odfaeg/Core/erreur.h"
#include "../../../include/odfaeg/Core/tmp.hpp"
#include <array>
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

        template <typename T, unsigned int R, unsigned int C>
        class Mat {
            private :
            std::array<std::array<T, R>, C> data;
            /**
              * \fn Matrix4f()
              * \brief constructor. (Construct the identity matrix)
            */
            public :
            Mat () {zero();}
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
            template <typename... Args,
            typename First = std::tuple_element_t<0, std::tuple<Args...>>,
            typename = std::enable_if_t<
              sizeof...(Args) != 1 || !std::is_same_v<Mat<T, R, C>, std::decay_t<First>>>>
            Mat (Args... args) {
                set(args...);
            }

            Mat (const Mat<T, R, C> &other) {
                zero();

                for (unsigned int i = 0; i < R; i++) {
                    for (unsigned int j = 0; j < C; j++) {
                        data[i][j] = other[i][j];
                    }
                }
            }
            template <unsigned int R2, unsigned int C2>
            Mat(const Mat<T, R2, C2>& other) {
                zero();
                unsigned int M = std::min(R, R2);
                unsigned int N = std::min(C, C2);
                for (unsigned int i = 0; i < M; ++i)
                    for (unsigned int j = 0; j < N; ++j)
                        data[i][j] = other[i][j];
            }
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
            template <typename... Args, unsigned int RI=0>
            void set(Args... args) {
                const int n = sizeof...(Args);
                static_assert(n == R*C, "Invalid number of arguments for matrix type");
                std::tuple<Args...> tp = std::make_tuple(args...);
                //////std::cout<<"set : "<<std::endl;
                setRow(tp, std::make_index_sequence<C>());
            }
            template <unsigned int RI=0, unsigned int N = 0, typename TP, size_t... CI, class = std::enable_if_t<RI < R - 1>>
            void setRow(TP tp, std::index_sequence<CI...> seq) {
                data[RI] = {{std::get<CI>(tp)...}};
                setRow<RI+1, N+1>(tp, typename core::offset_sequence<(RI + 1-N) * C, std::index_sequence<CI...>>::type() );

            }
            template <unsigned int RI, unsigned int N, typename TP, size_t... CI, class... D, class = std::enable_if_t<RI == R - 1>>
            void setRow(TP tp, std::index_sequence<CI...> seq) {
                data[RI] = {{std::get<CI>(tp)...}};

            }
            /**
            * \fn zero()
            * \brief set the null matrix.
            */
            void zero () {
                for (unsigned int i = 0; i < R; i++) {
                    for (unsigned int j = 0; j < C; j++) {
                        data[i][j] = 0;
                    }
                }
            }
            /**
            * \fn identity()
            * \brief set the identity matrix
            */
            void identity () {
                static_assert(R == C, "identity matrix only apply to square matrices");
                zero();
                for (unsigned int i = 0; i < R; i++) {
                    data[i][i] = 1;
                }
            }
            /**
            * \fn Matrix4f operator+ (const Matrix4f &other)
            * \brief add the matrix to another matrix and return the resulting matrix.
            * \param the matrix to be added with.
            * \return the resulting matrix.
            */
            Mat<T, R, C> operator+ (const Mat<T, R, C> &other) {
                Mat<T, R, C> result;
                for (unsigned int i = 0; i < R; i++) {
                    for (unsigned int j = 0; j < C; j++) {
                        result[i][j] = data[i][j] + other.data[i][j];
                    }
                }
                return result;
            }
            /**
            * \fn Matrix4f operator- (const Matrix4f &other)
            * \brief substract the matrix from another matrix and return the resulting matrix.
            * \param the matrix to be substracted with.
            * \return the resulting matrix.
            */
            Mat<T, R, C> operator- (const Mat<T, R, C> &other) {
                Mat<T, R, C> result;
                for (unsigned int i = 0; i < R; i++) {
                    for (unsigned int j = 0; j < C; j++) {
                        result[i][j] = data[i][j] - other[i][j];
                    }
                }
                return result;
            }
            /**
            * \fn Matrix4f operator* (const Matrix4f &other)
            * \brief multiply the matrix by another matrix and return the resulting matrix.
            * \param the matrix to be multiplied by.
            * \return the resulting matrix.
            */
            Mat<T, R, C> operator* (const Mat<T, R, C> &other) {
                Mat<T, R, R> result;

                for (unsigned int i = 0; i < R; i++) {
                    for (unsigned int j = 0; j < R; j++) {
                        result.data[i][j] = 0; // Initialise à zéro
                        for (unsigned int k = 0; k < C; k++) {
                            result.data[i][j] += data[i][k] * other.data[k][j];
                        }
                    }
                }

                return result;
            }
            /**
            * \fn Vec3f operator* (const Vec3f &vec3)
            * \brief multiply the matrix by a vector and return the resulting vector.
            * \param the vector to be multiplied by.
            * \return the resulting vector.
            */
            VecN<T, R> operator* (const VecN<T, C>& vec) {
                VecN<T, R> result;
                for (unsigned int i = 0; i < R; i++) {
                    float sum = 0;
                    for (unsigned int j = 0; j < C; j++) {
                        sum += data[i][j] * vec[j];
                    }
                    result[i] = sum;
                }
                return result;
            }
            /**
            * \fn Matrix4f operator* (float scalar)
            * \brief multiply the matrix by a scalar and return the resulting matrix.
            * \param the scalar to be multiplied by.
            * \return the resulting matrix.
            */
            Mat<T, R, C> operator* (float scalar) {
                Mat<T, R, C>  result;
                for (unsigned int i = 0; i < R; i++) {
                    for (unsigned int j = 0; j < C; j++) {
                        result[i][j] = data[i][j] * scalar;
                    }
                }
                return result;
            }
            /**
            * \fn Matrix3f inverse() throw (std::exception&)
            * \brief return the inverse of the matrix, throw an exception if the matrix isn't inversible.
            * \return the inversed matrix.
            */
            template <unsigned int N>
            Mat<T, N-1, N-1> getMinorMatrix(const Mat<T, N, N>& mat, unsigned int row, unsigned int col) {
                Mat<T, N-1, N-1> minor;
                unsigned int newRow = 0, newCol = 0;

                for (unsigned int i = 0; i < N; i++) {
                    if (i == row) continue; // Skip row

                    newCol = 0;
                    for (unsigned int j = 0; j < N; j++) {
                        if (j == col) continue; // Skip column

                        minor[newRow][newCol] = mat.data[i][j];
                        newCol++;
                    }
                    newRow++;
                }

                return minor;
            }
            Mat<T, R, C> inverse () {
                static_assert(R == C, "inverse only apply to square matrices");
                float det = getDet();
                if (std::abs(det) <= 0.f) {
                    throw std::runtime_error("this matrix cannot be inverted");
                }
                Mat<T, R, C> cofactorMatrix;
                Mat<T, R, C> adjointMatrix;
                for (unsigned int i = 0; i < R; i++) {
                    for (unsigned int j = 0; j < C; j++) {
                        // Création du mineur en supprimant la ligne i et la colonne j
                        Mat<T, R-1, C-1> minorMatrix = getMinorMatrix(*this, i, j);
                        float minorDet = minorMatrix.getDet();

                        // Calcul du cofacteur (attention à l'ordre des indices)
                        cofactorMatrix[i][j] = std::pow(-1, i + j) * minorDet;
                    }
                }
                ////////std::cout<<"co factor matrix : "<<cofactorMatrix;
                adjointMatrix = cofactorMatrix.transpose();
                float invDet = 1.f / det;
                return adjointMatrix * invDet;
            }
            /**
            * \fn float getDet() const
            * \brief return the value of the determiant of the matrix.
            * \return the value of the determinant.
            */
            template <unsigned int N, class = std::enable_if_t<N == 2>>
            float getDetOfMinorMatrix() {
                return data[0][0] * data[1][1] - data[0][1] * data[1][0];
            }
            template <unsigned int N, class...D, class = std::enable_if_t<(N > 2)>>
            float getDetOfMinorMatrix() {
                Mat<T, N-1, N-1> minorMatrix;
                float sum = 0;
                // Développement selon la première ligne
                for (unsigned int minJ = 0; minJ < N; minJ++) {
                    Mat<T, N-1, N-1> minorMatrix = getMinorMatrix(*this, 0, minJ);
                    float det = minorMatrix.template getDetOfMinorMatrix<N-1>();
                    sum += std::pow(-1, minJ) * data[0][minJ] * det;
                }
                return sum;
            }
            float getDet () {
                static_assert(R == C, "determinant only apply to square matrices");
                return getDetOfMinorMatrix<R>();
            }
            /**
            * \fn Matrix4f& operator= (const Matrix4f &other)
            * \brief set the matrix elements from the other matrix elements and return a reference to this matrix.
            * \param a refenrence to the current matrix.
            */
            Mat<T, R, C>& operator= (const Mat<T, R, C> &other) {
                zero();
                for (unsigned int i = 0; i < R; i++) {
                    for (unsigned int j = 0; j < C; j++) {
                        data[i][j] = other[i][j];
                    }
                }
                return *this;
            }
            template <unsigned int R2, unsigned int C2>
            Mat<T, R, C>& operator= (const Mat<T, R2, C2> &other) {
                zero();
                unsigned int M = std::min(R, R2);
                unsigned int N = std::min(C, C2);
                for (unsigned int i = 0; i < M; i++) {
                    for (unsigned int j = 0; j < N; j++) {
                        data[i][j] = other[i][j];
                    }
                }
                return *this;
            }
            std::array<T, C>& operator[] (unsigned int i) {
                return data[i];
            }
            std::array<T, C>& operator[] (unsigned int i) const {
                return const_cast<std::array<std::array<T, R>, C>&>(data)[i];
            }
            bool operator==(const Mat<T, R, C>&  other) const {
                for (unsigned int i = 0; i < R; i++) {
                    for (unsigned int j = 0; j < C; j++) {
                        if(data[i][j] != other[i][j])
                            return false;
                    }
                }
                return true;
            }
            std::array<float, 16> toGlMatrix() {
                std::array<float, 16> matrix;
                matrix[0] = data[0][0];
                matrix[1] = data[0][1];
                matrix[2] = data[0][2];
                matrix[3] = data[0][3];
                matrix[4] = data[1][0];
                matrix[5] = data[1][1];
                matrix[6] = data[1][2];
                matrix[7] = data[1][3];
                matrix[8] = data[2][0];
                matrix[9] = data[2][1];
                matrix[10] = data[2][2];
                matrix[11] = data[2][3];
                matrix[12] = data[3][0];
                matrix[13] = data[3][1];
                matrix[14] = data[3][2];
                matrix[15] = data[3][3];
                return matrix;
            }
            Mat<T, R, C> transpose() {
                Mat<T, R, C> result;
                for (unsigned int i = 0; i < R; i++) {
                    for (unsigned int j = 0; j < C; j++) {
                        result.data[i][j] = data[j][i];
                    }
                }
                return result;
            }
            template <typename Archive>
            void serialize(Archive & ar) {
                for (unsigned int i = 0; i < R; i++) {
                    for (unsigned int j = 0; j < C; j++) {
                        ar(data[i][j]);
                    }
                }
            }
        };
        template <typename T, unsigned int R, unsigned int C>
        std::ostream& operator<< (std::ostream &out, const Mat<T, R, C>& mat) {
            out<<"{";
            for (unsigned int i = 0; i < R; i++) {
                out<<"{";
                for (unsigned int j = 0; j < C; j++) {
                    out<<mat[i][j];
                    if (j != C-1)
                        out<<",";
                }
                out<<"}";
                if (i != R-1) {
                    out<<",";
                }
            }
            out<<"}";
            return out;
        }
        typedef Mat<float, 2, 2> Matrix2f;
        typedef Mat<float, 3, 3> Matrix3f;
        typedef Mat<float, 4, 4> Matrix4f;
    }
}
#endif
