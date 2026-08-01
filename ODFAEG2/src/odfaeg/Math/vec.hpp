#include <iostream>
#include <array>
#include <cmath>
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
        public:
            static inline double PI = 3.1415926535897932;
            /*static const VecN<T, N> xAxis;
            static const VecN<T, N> yAxis;
            static const VecN<T, N> zAxis;*/
            /**\fn Vec4()
            *  \brief default constructror.
            * construct a null vector (with 0, 0, 0 as coordinates)
            */
            VecN();
            template <typename... U>
            requires (sizeof...(U) == N)
            VecN(U... u);
            /**\fn Vec2f(float x, float y, float)
            *  \brief constructror.
            * construct a vector with the given coordinates.
            *  \param x : the x coordinate.
            *  \param y : the y coordinate.
            *  \param z : the z coordinate.
            */
            /*template<class... Args>
			VecN(Args... args) requires (sizeof...(Args) == N)
            {
                set(args...);
            }*/
            VecN(const VecN<T, N>& other);
            template <unsigned int N2>
            VecN(VecN<T, N2> other);
            template<class... Args>
            void set(Args... args);
            constexpr T& x();

            constexpr T& y();

            constexpr T& z();

            constexpr T& w();
            constexpr T x() const;

            constexpr T y() const;

            constexpr T z() const;

            constexpr T w() const;
            /**
            * \fn bool isNulVector() const
            * \brief return true if the vector is null, false otherwise.
            * \return true if the vector is null (0, 0, 0).
            */
            bool isNulVector() const;
            /**
            * \fn float operator[] (int i)
            * \brief retrieve one coordinate from the vector.
            * \param i : the indice of the vector coordinate. (0 = x, 1 = y and 2 = z)
            * \return the value of the coordinate.
            */
            T& operator[] (unsigned int i);
            const T& operator[] (unsigned int i) const;
            /**
            * \fn Vec4 operator+ (const Vec4 &other)
            * \brief add a vector to another and return the resulting vector.
            * \param the vector to be added with.
            * \return the resulting vector.
            */
            VecN<T, N> operator+ (const VecN<T, N> other);
            /** \fn Vec4 operator- (const Vec4 other)
            *   \brief substract the vector from another one and return the resulting vector.
            *   \param the other vector to be substracted from.
            *   \return the resulting vector.
            */
            VecN<T, N> operator- (const VecN<T, N>& other);
            /** \fn Vec4 operator* (const Vec4 other)
            *   \brief multiply the vector by another one and return the resulting vector.
            *   \param the other vector to be multiplied by.
            *   \return the resulting vector.
            */
            VecN<T, N> operator* (const VecN<T, N>& other);
            /** \fn Vec2f operator* (const Vec2f other)
            *   \brief divide the vector by another one and return the resulting vector.
            *   \param the other vector to be devided by.
            *   \return the resulting vector.
            */
            VecN<T, N> operator/ (const VecN<T, N>& other);
            /** \fn bool operator== (const Vec4 &other)
            *   \brief compare if two vectors ar equals. (They are equals of the coordiates are the same)
            *   \param the other vector to be compared with.
            *   \return true if the two vectors are equal, false otherwise.
            */
            VecN<T, N> operator/ (T scalar);
            VecN<T, N> operator/= (T scalar);
            VecN<T, N>& operator= (const VecN<T, N>& other);
            template <unsigned int N2>
            VecN<T, N>& operator= (const VecN<T, N2>& other);
            VecN<T, N> projOnVector(VecN<T, N> other);
            bool  isOpposite(const VecN<T, N>& other) const;
            /** \fn bool operator== (const Vec4 &other)
            *   \brief compare if two vectors ar equals. (They are equals of the coordiates are the same)
            *   \param the other vector to be compared with.
            *   \return true if the two vectors are equal, false otherwise.
            */
            bool operator== (const VecN<T, N>& other);
            bool operator!= (const VecN<T, N>& other);
            /** \fn Vec2f operator-()
            *   \brief return the opposite of the vector
            *   \return the opposite of the vector.
            */
            VecN<T, N> operator- () const;
            /** \fn void operator-= (const Vec2f other)
            *   \brief substract the vector from another one.
            *   \param the other vector to be added with.
            */
            VecN<T, N> operator+ (T number);
            VecN<T, N>& operator += (const VecN<T, N>& other);
            /** \fn void operator*= (const Vec2f other)
            *   \brief multiply the vector by another one.
            *   \param the other vector to be multiplied by.
            */
            VecN<T, N>& operator -= (const VecN<T, N>& other);
            /** \fn void operator*= (const Vec2f other)
            *   \brief multiply the vector by another one.
            *   \param the other vector to be multiplied by.
            */
            VecN<T, N>& operator *= (const VecN<T, N>& other);
            /** \fn void operator/= (const Vec2f other)
            *   \brief devide the vector by another one.
            *   \param the other vector to be devided by.
            */
            VecN<T, N>& operator *= (const T scale);
            /** \fn void operator/= (const Vec2f other)
            *   \brief divide the vector by another one.
            *   \param the other vector to be devided by.
            */
            VecN<T, N>& operator /= (const VecN<T, N>& other);
            /** \fn Vec4 fabs()
            *   \brief return absolute coordinates of the vector.
            *   \return the vector with absolute coordinates.
            */
            VecN<T, N> fabs() const;
            /** \fn void operator*= (const float scalar)
            *   \brief multiply the vector by a scalar.
            *   \param the scalar to be multiplied by.
            */
            VecN<T, N> operator* (T scale);
            /** \fn float computeDist (const Vec4 &other)
           *   \brief compute the distance between the two vectors.
           *   \param the other vector.
           *   \return the distance between the two vectors.
           */
            float computeDist(const VecN<T, N>& other);
            /** \fn float magnitude ()
            *   \brief compute the length of the vector.
            *   \return the length of the vector.
            */
            float magnitude() const;
            /** \fn float magnSquared ()
            *   \brief compute the squared length of the vector.
            *   \return the squared length of the vector.
            */
            float magnSquared() const;
            float computeDistSquared(const VecN<T, N>& other);
            /** \fn Vec2f normalize ()
            *   \brief transform the vector to a vector with a length of 1 and return the resulting vector.
            *   \return the resulting vector.
            */
            VecN<T, N> normalize() const;
            /** \fn Vec2f normalize ()
            *   \brief transform the vector to a 3D vector by dividing the x, y and z component by the w component.
            *   \return the resulting vector.
            */
            VecN<T, N> normalizeToVec3() const;
            /** \fn float dot (const Vec4 &other)
            *   \brief compute the dot product between to vectors. (using the first method)
            *    the dot product is the cosinus of the angle between the vector and another one.
            *    the length of the two vectors needs to be equal to 1 before performing the dot product.
            *   \return the dot product between the two vectors.
            */
            float dot(const VecN<T, N>& other) const;
            /** \fn Vec4 cross (const Vec4 &other)
            *   \brief do the cross product and return the resulting vector (The perpendicular to the vector and the other vector.)
            *   \param the other vector.
            *   \return the vector witch is perpendicular to another one.
            */
            VecN<T, N> cross(const VecN<T, N>& other) const;
            /** \fn float getAngleBetween (const Vec4 &other, const Vec4 &n)
            *   \brief return the angle between two vectors. (depending of the plane's orientation)
            *   the angle is given in radians and is always between -2PI and 2PI.
            *   \param other : the other vector.
            *   \param n : the orientation of the plane formed by the two vectors. (the normal of the plane)
            *   \return the angle between the two vectors.
            */
            float getAngleBetween(const VecN<T, N>& other);
            float getAngleBetween(const VecN<T, N>& other, const VecN<T, N>& n);

            /** \fn float projectOnAxis (const Vec4 &other)
            *   \brief Project an other vector on the vector and return the result.
            *   The projection result is the dot product of the two vectors multiplied by the length
            *   of the other vector.
            *   \return the result of the projection.
            */
            float projOnAxis(const VecN<T, N>& other);
            /** \fn float* getVec3 () const;
            *   \brief return the vector's components to an array.
            */
            std::array<T, N> getVec() const;
            VecN<T, N> mix(VecN<T, N>& other, float a);
            template <typename Archive>
            void serialize(Archive& ar);
            //virtual ~Vec4() {}
            /** \fn std::ostream& operator<< (std::ostream &out, const Vec4 &vec3)
            *   \brief set the vector coordinates to an output stream.
            *   \param &out : the output stream.
            *   \param &vec3 : the vector.
            *   \return the final output stream.
            */
        private:
            std::array<T, N> data;
        };
        template <typename... U>
                VecN(U...) -> VecN<float, sizeof...(U)>;
        using Vec4f = VecN<float, 4>;
        using Vec3f = VecN<float, 3>;
        using Vec2f =  VecN<float, 2>;
        using Vector2i = VecN<int, 2>;
        using Vector2u =  VecN<unsigned int, 2>;
        using Vector4i = VecN<int, 4>;
        using Vector3i = VecN<int, 3>;
        template <typename T, unsigned int N>
        std::ostream& operator<< (std::ostream& out, const VecN<T, N>& vec);
    }
}
#include "vec.inl"




