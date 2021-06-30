#ifndef VEC2
#define VEC2
#include <SFML/System.hpp>
#include <iostream>
#include "../../../include/odfaeg/Core/erreur.h"
#include "maths.h"
#include "export.hpp"
#include "../Core/serialization.h"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace math {
        class Vec3f;
        /**
          * \file vec2f.h
          * \class Vec2f
          * \brief Manage a 2D vector
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          *
          * This class defines 2D vectors of floating numbers and all operations about vectors.
          * This class redefines the class sf::Vector2f and offers more operations.
          * Vectors are very usefull for physic's and mathematic's algorithms. (Physics, geometry, ...)
          * In ODFAEG, vectors are used to define a direction or a point.
          */
        class ODFAEG_API_EXPORT Vec2f : public sf::Vector2f {
            public:
            /**\fn Vec2f()
            *  \brief default constructror.
            * construct a null vector (with 0, 0 as coordinates)
            */
            Vec2f ();
            /**\fn Vec2f(float x, float y)
            *  \brief constructror.
            * construct a vector with the given coordinates.
            *  \param x : the x coordinate.
            *  \param y : the y coordinate.
            */
            Vec2f (float x, float y);
            Vec2f (const Vec3f& vec3f);
            const static Vec2f down; /**< define the vector witch is pointing to the bottom*/
            /**
            * \fn bool isNulVector() const
            * \brief return true if the vector is null, false otherwise.
            * \return true if the vector is null (0, 0).
            */
            bool isNulVector () const;
            /**
            * \fn float operator[] (int i)
            * \brief retrieve one coordinate from the vector.
            * \param i : the indice of the vector coordinate. (0 = x, 1 = y)
            * \return the value of the coordinate.
            */
            float operator[] (int i);
            /** \fn Vec2f operator+ (const Vec2f other)
            *   \brief add the vector with another one and return the resulting vector.
            *   \param the other vector to be added with.
            *   \return the resulting vector.
            */
            Vec2f operator+ (const Vec2f &other);
            /** \fn Vec2f operator- (const Vec2f other)
            *   \brief substract the vector from another one and return the resulting vector.
            *   \param the other vector to be substracted from.
            *   \return the resulting vector.
            */
            Vec2f operator- (const Vec2f &other);
            /** \fn Vec2f operator* (const Vec2f other)
            *   \brief multiply the vector by another one and return the resulting vector.
            *   \param the other vector to be multiplied by.
            *   \return the resulting vector.
            */
            Vec2f operator* (const Vec2f &other);
            /** \fn Vec2f operator* (const Vec2f other)
            *   \brief multiply the vector by a number's value and return the resulting vector.
            *   \param the number's value to be multiplied by.
            *   \return the resulting vector.
            */
            Vec2f operator* (const float scale);
            /** \fn Vec2f operator* (const Vec2f other)
            *   \brief divide the vector by another one and return the resulting vector.
            *   \param the other vector to be devided by.
            *   \return the resulting vector.
            */
            Vec2f operator/ (const Vec2f &other);
            /** \fn Vec2f operator* (const Vec2f other)
            *   \brief divide the vector by a number's value and return the resulting vector.
            *   \param the number's value to be divided by.
            *   \return the resulting vector.
            */
            Vec2f operator/ (float scalar);
            /** \fn Vec2f operator= (const Vec2f &other)
            *   \brief the affects the other's vector elements to this vector elements and return a reference to this vector.
            *   \param the other vector
            *   \return a reference to this vector.
            */
            Vec2f& operator= (const Vec2f &other);
            Vec2f& operator= (const Vec3f& other);
            /** \fn bool operator== (const Vec2f &other)
            *   \brief compare if two vectors ar equals. (They are equals of the coordiates are the same)
            *   \param the other vector to be compared with.
            *   \return true if the two vectors are equal, false otherwise.
            */
            bool operator== (const Vec2f &other);
            bool operator!= (const Vec2f &other);
            /** \fn Vec2f operator-()
            *   \brief return the opposite of the vector
            *   \return the opposite of the vector.
            */
            Vec2f operator- () const;
            /** \fn void operator+= (const Vec2f other)
            *   \brief add the vector with another one.
            *   \param the other vector to be added with.
            */
            Vec2f& operator+= (const Vec2f &other);
            /** \fn void operator-= (const Vec2f other)
            *   \brief substract the vector from another one.
            *   \param the other vector to be added with.
            */
            Vec2f& operator-= (const Vec2f &other);
            /** \fn void operator*= (const Vec2f other)
            *   \brief multiply the vector by another one.
            *   \param the other vector to be multiplied by.
            */
            Vec2f& operator*= (const Vec2f &other);
            /** \fn void operator*= (const Vec2f other)
            *   \brief multiply the vector by another one.
            *   \param the other vector to be multiplied by.
            */
            Vec2f& operator*= (const float scale);
            /** \fn void operator/= (const Vec2f other)
            *   \brief divide the vector by another one.
            *   \param the other vector to be devided by.
            */
            Vec2f operator/= (const Vec2f &other);
            /** \fn float computeDist (const Vec2f &other)
            *   \brief compute the distance between the two vectors.
            *   \param the other vector.
            *   \return the distance between the two vectors.
            */
            float computeDist (const Vec2f &other);
            /** \fn float magnitude ()
            *   \brief compute the length of the vector.
            *   \return the length of the vector.
            */
            float magnitude () const;
            /** \fn float magnSquared ()
            *   \brief compute the squared length of the vector.
            *   \return the squared length of the vector.
            */
            float magnSquared ();
            /** \fn Vec2f normalize ()
            *   \brief transform the vector to a vector with a length of 1 and return the resulting vector.
            *   \return the resulting vector.
            */
            Vec2f normalize () const ;
            /** \fn float dot2 (const Vec2f &other)
            *   \brief compute the dot product between to vectors. (using the second method)
            *    the dot product is the cosinus of the angle between two vectors.
            *    the length of the two vectors needs to be equal to 1 before performing the dot product.
            *   \return the dot product between the two vectors.
            */
            float dot2 (const Vec2f &other);
            /** \fn float dot2 (const Vec2f &other)
            *   \brief compute the dot product between to vectors. (using the first method)
            *    the dot product is the cosinus of the angle between the vector and another one.
            *    the length of the two vectors needs to be equal to 1 before performing the dot product.
            *   \return the dot product between the two vectors.
            */
            float dot (const Vec2f &other) const;
            /** \fn float dot2 (const Vec2f &other)
            *   \brief get the angle between the vector and another one.
            *   the angle is given in radians and he's always between -2PI and 2PI.
            *   \return the angle between the vectors.
            */
            float getAngleBetween (const Vec2f &other);
            /** \fn float projectOnAxis (const Vec2f &other)
            *   \brief Project an other vector on the vector and return the result.
            *   The projection result is the dot product of the two vectors multiplied by the length
            *   of the other vector.
            *   \return the result of the projection.
            */
            float projOnAxis (const Vec2f &other);
            /** \fn sf::Vector2f getVec2sf() const
            *   \brief convert the vector to an sfml vector.
            *   \return an sfml vector.
            */
            sf::Vector2f getVec2sf() const;
            /** \fn float* getVec2sf() const
            *   \brief convert the vector to an array with the vector coordinates.
            *   \return an array witht he vector coordinates.
            */
            float* getVec2 () const;
            /** \fn bool isOpposite (const Vec2f &other)
            *   \brief test if two vectors are opposite. (they point in opposite directions)
            *   \return true if the two vectors are opposite.
            */
            bool isOpposite (const Vec2f &other) const;
            /** \fn std::ostream& operator<< (std::ostream &out, const Vec2f &vec2)
            *   \brief set the vector coordinates to an output stream.
            *   \param &out : the output stream.
            *   \param &vec2 : the vector.
            *   \return the final output stream.
            */
            template <typename Archive>
            void serialize (Archive &ar) {
                ar(x);
                ar(y);
            }
            friend std::ostream& operator<< (std::ostream &out, const Vec2f &vec2);
            static const Vec2f xAxis;
            static const Vec2f yAxis;
        };
        std::ostream& operator<< (std::ostream &out, const Vec2f &vec2);
    }
}
#endif
