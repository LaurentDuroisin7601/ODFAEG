#ifndef VEC4
#define VEC4
#include <iostream>
#include "maths.h"
#include <array>

#include "../../../include/odfaeg/Core/erreur.h"
#include "../Core/serialization.h"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace math {
        /**
          * \file vec4.h
          * \class Vec4
          * \brief Manage a 3D vector
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          *
          * This class defines 3D vectors of floating numbers and all operations about vectors.
          * This class redefines the class math::Vec3f and offers more operations.
          * Vectors are very usefull for physic's and mathematic's algorithms.
          * In ODFAEG, vectors are used to define a direction or a point.
          */
        template<typename T, unsigned int N>
        class VecN {
            public :
            static const VecN<T, 3> xAxis;
            static const VecN<T, 3> yAxis;
            static const VecN<T, 3> zAxis;
            /**\fn Vec4()
            *  \brief default constructror.
            * construct a null vector (with 0, 0, 0 as coordinates)
            */
            VecN () {
                for (unsigned int i = 0; i < N; i++) {
                    data[i] = 0;
                }
            }
            /**\fn Vec2f(float x, float y, float)
            *  \brief constructror.
            * construct a vector with the given coordinates.
            *  \param x : the x coordinate.
            *  \param y : the y coordinate.
            *  \param z : the z coordinate.
            */
            template<class... Args, typename = std::enable_if_t<(std::is_arithmetic_v<Args> && ...)>,
            typename First = std::tuple_element_t<0, std::tuple<Args...>>,
            typename = std::enable_if_t<
              sizeof...(Args) != 1 || !std::is_same_v<VecN<T, N>, std::decay_t<First>>>>
            VecN(Args... args)
            {
                set(args...);
            }
            VecN(const VecN<T, N>& other) {
                for (unsigned int i = 0; i < N; i++) {
                    data[i] = other[i];
                }
            }
            template <unsigned int N2>
            VecN(VecN<T, N2> other) {
                for (unsigned int i = 0; i < N; i++) {
                    data[i] = 0;
                }
                unsigned int M = std::min(N, N2);
                for (unsigned int i = 0; i < M; i++) {
                    data[i] = other[i];
                }
            }
            template<class... Args>
            void set(Args... args)
            {
                const int n = sizeof...(Args);
                static_assert(n == N, "Invalid number of arguments for vector type");

                data = { { args... } };
            }
            constexpr T& x()
            {
                static_assert(1 <= N, "Invalid number of arguments for vector type");
                return data[0];
            }

            constexpr T& y()
            {
                static_assert(2 <= N, "Invalid number of arguments for vector type");
                return data[1];
            }

            constexpr T& z()
            {
                static_assert(3 <= N, "Invalid number of arguments for vector type");
                return data[2];
            }

            constexpr T& w()
            {
                static_assert(4 <= N, "Invalid number of arguments for vector type");
                return data[3];
            }
            constexpr T x() const
            {
                static_assert(1 <= N, "Invalid number of arguments for vector type");
                return data[0];
            }

            constexpr T y() const
            {
                static_assert(2 <= N, "Invalid number of arguments for vector type");
                return data[1];
            }

            constexpr T z() const
            {
                static_assert(3 <= N, "Invalid number of arguments for vector type");
                return data[2];
            }

            constexpr T w() const
            {
                static_assert(4 <= N, "Invalid number of arguments for vector type");
                return data[3];
            }
            /**
            * \fn bool isNulVector() const
            * \brief return true if the vector is null, false otherwise.
            * \return true if the vector is null (0, 0, 0).
            */
            bool isNulVector () const {
                for (unsigned int i = 0; i < N; i++) {
                    if (data[i] != 0)
                        return false;
                }
                return true;
            }
            /**
            * \fn float operator[] (int i)
            * \brief retrieve one coordinate from the vector.
            * \param i : the indice of the vector coordinate. (0 = x, 1 = y and 2 = z)
            * \return the value of the coordinate.
            */
            T& operator[] (unsigned int i) {
                return data[i];
            }
            T& operator[] (unsigned int i) const {
                return const_cast<std::array<T, N>&>(data)[i];
            }
            /**
            * \fn Vec4 operator+ (const Vec4 &other)
            * \brief add a vector to another and return the resulting vector.
            * \param the vector to be added with.
            * \return the resulting vector.
            */
            VecN<T, N> operator+ (const VecN<T, N> other) {
                VecN<T, N> result;
                for (unsigned int i = 0; i < N; i++) {
                    result[i] = data[i] + other[i];
                }
                return result;
            }
            /** \fn Vec4 operator- (const Vec4 other)
            *   \brief substract the vector from another one and return the resulting vector.
            *   \param the other vector to be substracted from.
            *   \return the resulting vector.
            */
            VecN<T, N> operator- (const VecN<T, N> &other) {
                VecN<T, N> result;
                for (unsigned int i = 0; i < N; i++) {
                    result[i] = data[i] - other[i];
                }
                return result;
            }
            /** \fn Vec4 operator* (const Vec4 other)
            *   \brief multiply the vector by another one and return the resulting vector.
            *   \param the other vector to be multiplied by.
            *   \return the resulting vector.
            */
            VecN<T, N> operator* (const VecN<T, N> &other) {
                VecN<T, N> result;
                for (unsigned int i = 0; i < N; i++) {
                    result[i] = data[i] * other[i];
                }
                return result;
            }
            /** \fn Vec2f operator* (const Vec2f other)
            *   \brief divide the vector by another one and return the resulting vector.
            *   \param the other vector to be devided by.
            *   \return the resulting vector.
            */
            VecN<T, N> operator/ (const VecN<T, N> &other) {
                VecN<T, N> result;
                for (unsigned int i = 0; i < N; i++) {
                    result[i] = data[i] / other[i];
                }
                return result;
            }
            /** \fn bool operator== (const Vec4 &other)
            *   \brief compare if two vectors ar equals. (They are equals of the coordiates are the same)
            *   \param the other vector to be compared with.
            *   \return true if the two vectors are equal, false otherwise.
            */
            VecN<T, N> operator/ (T scalar) {
                VecN<T, N> result;
                for (unsigned int i = 0; i < N; i++) {
                    result[i] = data[i] / scalar;
                }
                return result;
            }
            VecN<T, N>& operator= (const VecN<T, N> &other) {
                for (unsigned int i = 0; i < N; i++) {
                    data[i] = other[i];
                }
                return *this;
            }
            template <unsigned int N2>
            VecN<T, N>& operator= (const VecN<T, N2> &other) {
                unsigned int M = std::min(N, N2);
                for (unsigned int i = 0; i < M; i++) {
                    data[i] = other[i];
                }
                return *this;
            }
            VecN<T, N> projOnVector(VecN<T, N> other) {
                VecN<T, N> proj;
                float dp = dot(other);
                for (unsigned int i = 0; i < N; i++) {
                    proj.data[i] = dp / other.magnSquared() * other[i];
                }
                return proj;
            }
            bool  isOpposite (const VecN<T, N> &other) const {
                if (isNulVector() || other.isNulVector())
                    return false;
                for (unsigned int i = 0; i < N; i++) {
                    if (data[i] != -other[i])
                        return false;
                }
                return true;
            }
            /** \fn bool operator== (const Vec4 &other)
            *   \brief compare if two vectors ar equals. (They are equals of the coordiates are the same)
            *   \param the other vector to be compared with.
            *   \return true if the two vectors are equal, false otherwise.
            */
            bool operator== (const VecN<T, N> &other) {
                for (unsigned int i = 0; i < N; i++) {
                    if (data[i] != other[i])
                        return false;
                }
                return true;
            }
            bool operator!= (const VecN<T, N>& other) {
                return !(*this == other);
            }
            /** \fn Vec2f operator-()
            *   \brief return the opposite of the vector
            *   \return the opposite of the vector.
            */
            VecN<T, N> operator- () const {
                VecN<T, N> result;
                for (unsigned int i = 0; i < N; i++) {
                    result[i] = -data[i];
                }
                return result;
            }
            /** \fn void operator-= (const Vec2f other)
            *   \brief substract the vector from another one.
            *   \param the other vector to be added with.
            */
            VecN<T, N> operator+ (T number) {
                VecN<T, N> result;
                for (unsigned int i = 0; i < N; i++) {
                    result[i] += number;
                }
                return result;
            }
            VecN<T, N>& operator += (const VecN<T, N> &other) {
                for (unsigned int i = 0; i < N; i++) {
                    data[i] += other[i];
                }
                return *this;
            }
            /** \fn void operator*= (const Vec2f other)
            *   \brief multiply the vector by another one.
            *   \param the other vector to be multiplied by.
            */
            VecN<T, N>& operator -= (const VecN<T, N>& other) {
                for (unsigned int i = 0; i < N; i++) {
                    data[i] -= other[i];
                }
                return *this;
            }
            /** \fn void operator*= (const Vec2f other)
            *   \brief multiply the vector by another one.
            *   \param the other vector to be multiplied by.
            */
            VecN<T, N>& operator *= (const VecN<T, N> &other) {
                for (unsigned int i = 0; i < N; i++) {
                    data[i] *= other[i];
                }
                return *this;
            }
            /** \fn void operator/= (const Vec2f other)
            *   \brief devide the vector by another one.
            *   \param the other vector to be devided by.
            */
            VecN<T, N>& operator *= (const T scale) {
                for (unsigned int i = 0; i < N; i++) {
                    data[i] *= scale;
                }
                return *this;
            }
            /** \fn void operator/= (const Vec2f other)
            *   \brief divide the vector by another one.
            *   \param the other vector to be devided by.
            */
            VecN<T, N>& operator /= (const VecN<T, N>& other) {
                for (unsigned int i = 0; i < N; i++) {
                    data[i] /= other[i];
                }
                return *this;
            }
            /** \fn Vec4 fabs()
            *   \brief return absolute coordinates of the vector.
            *   \return the vector with absolute coordinates.
            */
            VecN<T, N> fabs () const {
                VecN<T, N> result;
                for (unsigned int i = 0; i < N; i++) {
                    result[i] = Math::abs(data[i]);
                }
                return result;
            }
            /** \fn void operator*= (const float scalar)
            *   \brief multiply the vector by a scalar.
            *   \param the scalar to be multiplied by.
            */
            VecN<T, N> operator* (T scale) {
                VecN<T, N> result;
                for (unsigned int i = 0; i < N; i++) {
                    result[i] = data[i] * scale;
                }
                return result;
            }
             /** \fn float computeDist (const Vec4 &other)
            *   \brief compute the distance between the two vectors.
            *   \param the other vector.
            *   \return the distance between the two vectors.
            */
            float computeDist (const VecN<T, N>& other) {
                return math::Math::sqrt(computeDistSquared(other));
            }
            /** \fn float magnitude ()
            *   \brief compute the length of the vector.
            *   \return the length of the vector.
            */
            float magnitude () const {
                return Math::sqrt(magnSquared());
            }
            /** \fn float magnSquared ()
            *   \brief compute the squared length of the vector.
            *   \return the squared length of the vector.
            */
            float magnSquared () const {
                float sum = 0;
                for (unsigned int i = 0; i < N; i++) {
                    sum += Math::power(data[i], 2);
                }
                return sum;
            }
            float computeDistSquared(const VecN<T, N>& other) {
                float sum = 0;
                for (unsigned int i = 0; i < N; i++) {
                    sum += Math::power(data[i] - other[i], 2);
                }
                return sum;
            }
            /** \fn Vec2f normalize ()
            *   \brief transform the vector to a vector with a length of 1 and return the resulting vector.
            *   \return the resulting vector.
            */
            VecN<T, N> normalize () const {
                VecN<T, N> result;
                float length = magnitude();
                if (length == 0) {
                    for (unsigned int i = 0; i < N; i++) {
                        result[i] = 0;
                    }
                    return result;
                }
                for (unsigned int i = 0; i < N; i++) {
                    result[i] = data[i] / length;
                }
                return result;
            }
            /** \fn Vec2f normalize ()
            *   \brief transform the vector to a 3D vector by dividing the x, y and z component by the w component.
            *   \return the resulting vector.
            */
            VecN<T, N> normalizeToVec3 () const {
                static_assert(4 <= N, "Invalid number of arguments for vector type");
                VecN<T, N> result;
                if (data[3] != 0) {
                    for (unsigned int i = 0; i < 3; i++) {
                        result[i] = data[i] / data[3];
                    }
                    return result;
                }
                return *this;
            }
            /** \fn float dot (const Vec4 &other)
            *   \brief compute the dot product between to vectors. (using the first method)
            *    the dot product is the cosinus of the angle between the vector and another one.
            *    the length of the two vectors needs to be equal to 1 before performing the dot product.
            *   \return the dot product between the two vectors.
            */
            float dot (const VecN<T, N> &other) const {
                if (isNulVector ())
                    return 0.f;
                float sum = 0;
                for (unsigned int i = 0; i < N; i++) {
                    sum += data[i] * other[i];
                }
                return sum;
            }
            /** \fn Vec4 cross (const Vec4 &other)
            *   \brief do the cross product and return the resulting vector (The perpendicular to the vector and the other vector.)
            *   \param the other vector.
            *   \return the vector witch is perpendicular to another one.
            */
            VecN<T, N> cross (const VecN<T, N> &other) const {
                static_assert(N == 3, "Invalid number of arguments for vector type");
                VecN<T, N> result;
                result[0] = data[1] * other[2] - data[2] * other[1];
                result[1] = data[2] * other[0] - data[0] * other[2];
                result[2] = data[0] * other[1] - data[1] * other[0];
                return result;
            }
            /** \fn float getAngleBetween (const Vec4 &other, const Vec4 &n)
            *   \brief return the angle between two vectors. (depending of the plane's orientation)
            *   the angle is given in radians and is always between -2PI and 2PI.
            *   \param other : the other vector.
            *   \param n : the orientation of the plane formed by the two vectors. (the normal of the plane)
            *   \return the angle between the two vectors.
            */
             float getAngleBetween (const VecN<T, N> &other) {
                if (isNulVector() || other.isNulVector() || *this == other)
                return 0;
                if (*this == -other)
                    return PI;
                float dotProduct = dot(other);
                Vec3f v1 (x(), y(), 0);
                Vec3f v2 (other.x(), other.y(), 0);
                Vec3f v3 = v1.cross(v2);
                if (v3.z() >= 0)
                    return Math::acosinus(dotProduct);
                else
                    return -Math::acosinus(dotProduct);
            }
            float getAngleBetween (const VecN<T, N> &other, const VecN<T, N> &n) {
                if(isNulVector() || other.isNulVector())
                return 0;
                float cosinus = dot(other);
                if (cosinus == 1)
                    return 0;
                if (cosinus == -1)
                    return PI;
                VecN<T, N> crs = cross(other);
                if (n.dot(crs) < 0)
                    return Math::acosinus(cosinus);
                return -Math::acosinus(cosinus);
            }

            /** \fn float projectOnAxis (const Vec4 &other)
            *   \brief Project an other vector on the vector and return the result.
            *   The projection result is the dot product of the two vectors multiplied by the length
            *   of the other vector.
            *   \return the result of the projection.
            */
            float projOnAxis (const VecN<T, N> &other) {
                return dot(other) * magnitude();
            }
            /** \fn float* getVec3 () const;
            *   \brief return the vector's components to an array.
            */
            std::array<T, N> getVec () const {
                return data;
            }
            VecN<T, N> mix (VecN<T, N>& other, float a) {
                VecN<T, N> result;
                result = (*this) * (1 - a) + other * a;
                return result;
            }
            template <typename Archive>
            void serialize (Archive &ar) {
                for (unsigned int i = 0; i < N; i++) {
                    ar(data[i]);
                }
            }
            //virtual ~Vec4() {}
            /** \fn std::ostream& operator<< (std::ostream &out, const Vec4 &vec3)
            *   \brief set the vector coordinates to an output stream.
            *   \param &out : the output stream.
            *   \param &vec3 : the vector.
            *   \return the final output stream.
            */
            private :
            std::array<T, N> data;
        };
        typedef VecN<float, 4> Vec4f;
        typedef VecN<float, 3> Vec3f;
        typedef VecN<float, 2> Vec2f;
        typedef VecN<int, 2> Vector2i;
        typedef VecN<unsigned int, 2> Vector2u;
        template <typename T, unsigned int N>
        std::ostream& operator<< (std::ostream  &out, const VecN<T, N>& vec) {
            out<<"{";
            for (unsigned int i = 0; i < N; i++) {
               out<<vec[i];
               if (i != N - 1)
                    out<<",";
            }
            out<<"}";
            return out;
        }
    }
}
#endif




