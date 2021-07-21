#ifndef CONSTANTS
#define CONSTANTS
#define PI 3.1415926535897932
#define TWO_PI 2*PI
#define HALF_PI PI/2
#define INV_PI = 1/PI
#define INV_TWO_PI = TWO_PI / 2
#define RAD_TO_DEG 180/PI
#define DEG_TO_RAD PI/180
#define ONE_THIRD 1.f / 3.f
#define TWO_THIRD 2.f / 3.f
#define EPSILON 0.00001
#include <math.h>
#include "export.hpp"
#include <random>
#include <ctime>
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace math {
        class Vec3f;
        class BigInt;
        /**
          * \file math.h
          * \class Math
          * \brief Do some math tricks.
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          *
          * Contains some arithmetic functions for floatting numbers.
          * Defines also some variable :
          * PI is the value of the number PI.
          * TWO_PI is the value of the double of PI.
          * HALF_PI is the value of the half of PI.
          * INV_PI is the value of the inverse of PI.
          * INV_TWO_PI is the value of the inverse of the double of PI.
          * RAD_TO_DEG is the value used to convert radians to degrees.
          * DEG_TO_RAD is the value used to convert degress to radians.
          * ONE_THIRD is the value of one divided by 3.
          * TWO_THIRD is the value of two divided by 3.
          * EPSILON is a very little value used to avoid to have overflow problems with floatting numbers.
          */
        class ODFAEG_API_EXPORT Math {
            public:
            static void initSeed();
            /**
            *  \fn float acosinus (float value)
            *  \brief return the arc cosinus of a cosinus value.
            *  \param the value of the cosinus.
            *  \return the arc cosinus of the angle. (given in radians)
            */
            static float acosinus(float value);
            /**
            *  \fn float asinus (float value)
            *  \brief return the arc sinus of a sinus value.
            *  \param the value of the sinus.
            *  \return the arc sinus of the angle. (given in radians)
            */
            static float asinus(float value);
            /**
            *  \fn float sinus (float value)
            *  \brief return the sinus of an angle. (given in radians)
            *  \param the angle's value. (given in radians)
            *  \return the sinus of the angle
            */
            static float sinus (float value);
            /**
            *  \fn float cosinus (float value)
            *  \brief return the cosinus of an angle. (given in radians)
            *  \param the angle's value. (given in radians)
            *  \return the cosinus of the angle.
            */
            static float cosinus (float value);
            /**
            *  \fn float atang (float value)
            *  \brief return the arc tangeant of a tangeant value.
            *  \param the value of the tangeant.
            *  \return the tangeant.
            */
            static float atang(float value);
            static float atang2 (float value1, float value2);
            /**
            *  \fn float abs (float value)
            *  \brief return the absolute value of a value.
            *  \param the value.
            *  \return the absolute value of the value. (The absolute value is always positive)
            */
            static float abs (float value);
            /**
            *  \fn float tang (float value)
            *  \brief return the tangeant of an angle. (given in radians)
            *  \param the value of the angle. (given in radian)
            *  \return the tangeant of the angle.
            */
            static float tang (float value);
            /**
            * \fn float sqrt (float value)
            * \brief return the square root of a number.
            * \param the value of the number.
            * \return the square root of the number.
            */
            static double sqrt (float value);
            /**
            * \fn float log10 (float value)
            * \brief return the logarythm of a number in base 10.
            * \param the value of the logarythm.
            * \return the logarythm of the value. (Example : the logarythm of 100 in base 10 is 2 because 10² = 100.)
            */
            static float log10(float value);
            /**
            * \fn float logn (float value)
            * \brief return the logarythm of a number in base n.
            * \param the value of the logarythm.
            * \return the logarythm of the value.
            */
            static float logn(float value, int base);
            /**
            * \fn float power (float value, float p)
            * \brief return the value of a number exponent p.
            * \param value : the value of the number.
            * \param p : the value of the exponent.
            * \return the value of the number exponent p.
            */
            static double power(float value, float p);
            /**
            * \fn round a number.
            * \brief round a number with the given precision.
            * \param value : the value of the number.
            * \param p : the precision. (The number of decimals after the floatting point)
            * \return the number rounded with p precision.
            */
            static float round(float value, int p);
            /**
            * \fn float inversSqrt(float value)
            * \brief return the inverse of the square root of a number.
            * \param the value of the number.
            * \return the value of the inverse of the square root of the number.
            */
            static float inversSqrt(float value);
            /**
            * \fn float toRadian (float value)
            * \brief return the radians value of a given angle.
            * \param the value of the angle.
            * \return the radians value of the angle.
            */
            static float toRadians (float value);
            /**
            * \fn float toDegrees (float value)
            * \brief return the gegrees value of a given angle.
            * \param the value of the angle.
            * \return the degrees value of the angle.
            */
            static float toDegrees (float value);
            /**
            * \fn float exp (float value)
            * \brief return the exponent of the value.
            * \return the value of the number exponent.
            */
            static float exp (float value);
            /**
            * \fn Vec3f toCartesian (float teta, float phi)
            * \brief convert polar coordinates to cartesian coordinates.
            * \param teta : the value of the angle teta. (in radian)
            * \param phi : the value of the angle phi. (in radian)
            * \return the Vector of the cartesian coordinates.
            */
            static Vec3f toCartesian (float teta, float phi);
            /**
            * \fn int clamp (int value, int min, int max)
            * \brief restrict the value of a number in a specified interval.
            * \param value : the value of the number.
            * \param min : the minimum value of the interval.
            * \param max : the maximum value of the interval.
            * \return the value of the number in the specified interval.
            */
            static int clamp (int value, int min, int max);
            /**
            * \fn float random (float max, float min = 0)
            * \brief get a random value in a specified interval.
            * \param max : the maximum value of the interval.
            * \param min : the minimum value of the interval. (0 by default)
            * \return the random value in the specified interval
            */
            static float random(float max, float min = 0);
            static unsigned long long int ullirandom(unsigned long long int min, unsigned long long int max);
            static int roundToInt(float value);
            private :
            static std::mt19937 mrs;
            static bool isSeedInitialized;
        };
    }
}
#endif


