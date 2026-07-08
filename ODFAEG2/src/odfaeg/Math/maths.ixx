module;
#include <math.h>
#include <random>
#include <ctime>
export module odfaeg.math.maths;
import odfaeg.math.vec;
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
export namespace odfaeg {
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
            static void initSeed() {
                if (!isSeedInitialized) {
                    mrs = std::mt19937(static_cast<unsigned long>(std::time(nullptr)));
                    isSeedInitialized = true;
                }
            }
            static float random(float min, float max) {

                std::uniform_real_distribution<float> distribution(min, max);
                return distribution(mrs);
            }
            static unsigned long long int ullirandom(unsigned long long int min, unsigned long long int max) {

                std::uniform_int_distribution<unsigned long long int> distribution(min, max);
                return distribution(mrs);
            }

            static float acosinus(float value) {
                float result;
                if (-1.f < value) {
                    if (value < 1.f)
                        result = (float)acos(value);
                    else
                        result = 0.f;
                }
                else
                    result = PI;
                return (result < EPSILON && result > -EPSILON) ? 0 : result;
            }
            //Donne l'arc sinus d'une valeur.
            static float asinus(float value) {
                float result;
                if (-1.f < value) {
                    if (value < 1.f)
                        result = (float)asin(value);
                    else
                        result = HALF_PI;
                }
                else
                    result = -HALF_PI;
                return (result < EPSILON && result > -EPSILON) ? 0 : result;
            }
            //Donne l'arc tangeante d'une valeur.
            static float atang(float value) {
                float result = (float)atan(value);
                return (result < EPSILON && result > -EPSILON) ? 0 : result;
            }
            //Donne le sinus d'un angle donn� en radian.
            static float sinus(float value) {
                float result, radians = value / TWO_PI;
                if (abs(radians) > PI)
                    radians -= TWO_PI;
                if (abs(radians) > HALF_PI)
                    radians = PI - radians;
                if (abs(radians) <= PI / 4) {
                    result = (float)sin(value);
                }
                else {
                    result = (float)cos(PI / 2 - value);
                }
                return (result < EPSILON && result > -EPSILON) ? 0 : result;
            }
            //Donne le cosinus d'un angle donn�e en radian.
            static float cosinus(float value) {
                float result = (float)sin(value + HALF_PI);
                return (result < EPSILON && result > -EPSILON) ? 0 : result;
            }
            //Donne la tangeante d'un angle donn� en radians.
            static float tang(float value) {
                float result = (float)tan(value);
                return (result < EPSILON && result > -EPSILON) ? 0 : result;
            }
            static float atang2(float value1, float value2) {
                return (float)std::atan2(value1, value2);
            }
            //Donne la valeur absolue d'un nombre.
            static float abs(float value) {
                if (value >= 0)
                    return value;
                return -value;
            }
            //Renvoie la racine carr�e d'un nombre.
            static double sqrt(float value) {
                return std::sqrt(value);
            }
            //Donne l'inverse de la racine carr�e d'un nombre.
            static float inversSqrt(float value) {
                return 1.f / sqrt(value);
            }
            //Donne le logarithme d'un nombre. (En base 10.)
            static float log10(float value) {
                return (float)log(value);
            }
            //Donne le logarithme d'un nombre en base base.
            static float logn(float value, int base) {
                return (float)(log(value) / log(base));
            }
            //Donne le nombre � la puissance n.
            static double power(float value, float exp) {
                return (float)pow(value, exp);
            }
            //Converti un angle en radian.
            static float toRadians(float value) {
                return value * DEG_TO_RAD;
            }
            //Convertis un angle en degrer.
            static float toDegrees(float value) {
                return value * RAD_TO_DEG;
            }
            //Arrondis un nombre � la pr�cision p.
            static float round(float value, int p) {
                int mult = (int)pow(10, p + 1);
                int numberToRound = (int)(value * mult);

                int lastChiffer = numberToRound % 10;

                if (numberToRound > 0) {
                    if (lastChiffer >= 5)
                        numberToRound += 10;
                }
                else {
                    if (lastChiffer <= -5)
                        numberToRound -= 10;
                }

                numberToRound = numberToRound - lastChiffer;
                return numberToRound / mult;;
            }
            static int roundToInt(float value) {
                int pe = value;
                int pd = value - pe;
                return value - pd;
            }
            //Renvoie l'exponetielle d'un nombre.
            static float exp(float value) {
                return (float)std::exp(value);
            }
            //Convertis des coordonn�e polaire en coordon�e cart�sinnes.
            static Vec3f toCartesian(float teta, float phi) {
                float rTemp = cosinus(phi);
                float x = rTemp * cosinus(teta);
                float y = rTemp * sinus(teta);
                float z = sinus(phi);
                return Vec3f(x, y, z);
            }
            static float clamp(float value, float min, float max) {
                if (value < min)
                    value = min;
                if (value > max)
                    value = max;
                return value;
            }        
        };
    }
}