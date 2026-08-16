#ifndef ODFAEG_MATHS_HPP
#define ODFAEG_MATHS_HPP
#include <math.h>
#include <random>
#include <ctime>
#include "vec.hpp"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace math {
        constexpr double PI = 3.1415926535897932;
        constexpr double TWO_PI = 2.0 * PI;
        constexpr double HALF_PI = PI / 2.0;
        constexpr double INV_PI = 1.0 / PI;
        constexpr double INV_TWO_PI = 1.0 / TWO_PI;
        constexpr double RAD_TO_DEG = 180.0 / PI;
        constexpr double DEG_TO_RAD = PI / 180.0;
        constexpr float ONE_THIRD = 1.f / 3.f;
        constexpr float TWO_THIRD = 2.f / 3.f;
        constexpr float EPSILON = 0.00001f;
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
        class Math {
        private :
            inline static std::mt19937 mrs = std::mt19937(static_cast<unsigned long>(std::time(nullptr)));
            inline static bool isSeedInitialized = false;
        public:
            static void initSeed();
            static float random(float min, float max);
            static unsigned long long int ullirandom(unsigned long long int min, unsigned long long int max);

            static float acosinus(float value);
            //Donne l'arc sinus d'une valeur.
            static float asinus(float value);
            //Donne l'arc tangeante d'une valeur.
            static float atang(float value);
            //Donne le sinus d'un angle donn� en radian.
            static float sinus(float value);
            //Donne le cosinus d'un angle donn�e en radian.
            static float cosinus(float value);
            //Donne la tangeante d'un angle donn� en radians.
            static float tang(float value);
            static float atang2(float value1, float value2);
            //Donne la valeur absolue d'un nombre.
            static float abs(float value);
            //Renvoie la racine carr�e d'un nombre.
            static double sqrt(float value);
            //Donne l'inverse de la racine carr�e d'un nombre.
            static float inversSqrt(float value);
            //Donne le logarithme d'un nombre. (En base 10.)
            static float log10(float value);
            static float log2(float value);
            //Donne le logarithme d'un nombre en base base.
            static float logn(float value, int base);
            //Donne le nombre � la puissance n.
            static double power(float value, float exp);
            //Converti un angle en radian.
            static float toRadians(float value);
            //Convertis un angle en degrer.
            static float toDegrees(float value);
            //Arrondis un nombre � la pr�cision p.
            static float round(float value, int p);
            static int roundToInt(float value);
            //Renvoie l'exponetielle d'un nombre.
            static float exp(float value);
            //Convertis des coordonn�e polaire en coordon�e cart�sinnes.
            static Vec3f toCartesian(float teta, float phi);
            static float clamp(float value, float min, float max);
        };
    }
}
#endif