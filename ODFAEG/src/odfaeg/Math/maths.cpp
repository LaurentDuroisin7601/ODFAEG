#include "../../../include/odfaeg/Math/maths.h"
#include "../../../include/odfaeg/Math/vec4.h"
#include "../../../include/odfaeg/Math/bigInt.hpp"
/*Fonctions élémentaires.*/
//Donne l'arc cosinus d'une valeur.

namespace odfaeg {
    namespace math {
        std::mt19937 Math::mrs = std::mt19937(static_cast<unsigned long>(std::time(nullptr)));
        bool Math::isSeedInitialized = false;
        void Math::initSeed() {
            if (!isSeedInitialized) {
                mrs = std::mt19937(static_cast<unsigned long>(std::time(nullptr)));
                isSeedInitialized = true;
            }
        }
        float Math::random(float min, float max) {

            std::uniform_real_distribution<float> distribution(min, max);
            return distribution(mrs);
        }
        unsigned long long int Math::ullirandom(unsigned long long int min, unsigned long long int max) {

            std::uniform_int_distribution<unsigned long long int> distribution(min, max);
            return distribution(mrs);
        }

        float Math::acosinus(float value) {
           float result;
           if(-1.f < value) {
            if(value < 1.f)
               result = (float) acos(value);
            else
               result = 0.f;
           } else
            result = PI;
           return (result < EPSILON && result > -EPSILON) ? 0 : result;
        }
        //Donne l'arc sinus d'une valeur.
        float Math::asinus(float value) {
            float result;
            if(-1.f < value) {
                if(value < 1.f)
                    result = (float) asin(value);
                else
                    result = HALF_PI;
            } else
                result = -HALF_PI;
            return (result < EPSILON && result > -EPSILON) ? 0 : result;
        }
        //Donne l'arc tangeante d'une valeur.
        float Math::atang(float value) {
            float result = (float) atan(value);
            return (result < EPSILON && result > -EPSILON) ? 0 : result;
        }
        //Donne le sinus d'un angle donné en radian.
        float Math::sinus (float value) {
            float result, radians = value / TWO_PI;
            if(abs(radians) > PI)
                radians -= TWO_PI;
            if(abs(radians) > HALF_PI)
                radians = PI - radians;
            if(abs(radians) <= PI / 4) {
                result = (float) sin(value);
            } else {
                result = (float) cos (PI/2-value);
            }
            return (result < EPSILON && result > -EPSILON) ? 0 : result;
        }
        //Donne le cosinus d'un angle donnée en radian.
        float Math::cosinus (float value) {
            float result = (float) sin(value+HALF_PI);
            return (result < EPSILON && result > -EPSILON) ? 0 : result;
        }
        //Donne la tangeante d'un angle donné en radians.
        float Math::tang (float value) {
            float result = (float) tan(value);
            return (result < EPSILON && result > -EPSILON) ? 0 : result;
        }
        float Math::atang2 (float value1, float value2) {
            return (float) std::atan2(value1, value2);
        }
        //Donne la valeur absolue d'un nombre.
        float Math::abs(float value) {
            if (value >= 0)
                return value;
            return -value;
        }
        //Renvoie la racine carrée d'un nombre.
        double Math::sqrt(float value){
            return std::sqrt(value);
        }
        //Donne l'inverse de la racine carrée d'un nombre.
        float Math::inversSqrt(float value) {
            return 1.f / sqrt(value);
        }
        //Donne le logarithme d'un nombre. (En base 10.)
        float Math::log10 (float value) {
            return (float) log(value);
        }
        //Donne le logarithme d'un nombre en base base.
        float Math::logn (float value, int base) {
            return (float) (log(value)/log(base));
        }
        //Donne le nombre à la puissance n.
        double Math::power (float value, float exp) {
            return (float) pow(value, exp);
        }
        //Converti un angle en radian.
        float Math::toRadians(float value) {
            return value * DEG_TO_RAD;
        }
        //Convertis un angle en degrer.
        float Math::toDegrees(float value) {
            return value * RAD_TO_DEG;
        }
        //Arrondis un nombre à la précision p.
        float Math::round(float value, int p) {
            int mult = (int) pow(10, p + 1);
            int numberToRound = (int) (value * mult);

            int lastChiffer = numberToRound % 10;

            if (numberToRound > 0) {
                if (lastChiffer >= 5)
                    numberToRound += 10;
            } else {
                if (lastChiffer <= -5)
                    numberToRound -= 10;
            }

            numberToRound = numberToRound - lastChiffer;
            return numberToRound / mult;;
        }
        int Math::roundToInt(float value) {
            int pe = value;
            int pd = value - pe;
            return value - pd;
        }
        //Renvoie l'exponetielle d'un nombre.
        float Math::exp (float value) {
            return (float) std::exp(value);
        }
        //Convertis des coordonnée polaire en coordonée cartésinnes.
        Vec3f Math::toCartesian (float teta, float phi) {
            float rTemp = Math::cosinus(phi);
            float x = rTemp * Math::sinus(teta);
            float y = rTemp * Math::cosinus(teta);
            float z = Math::sinus(phi);
            return Vec3f (x, y, z);
        }
        int Math::clamp (int value, int min, int max) {
            if (value < min)
                value = min;
            if (value >= max)
                value = max - 1;
            return value;
        }
    }
}
