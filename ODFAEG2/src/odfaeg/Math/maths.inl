/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace math {        
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
        float Math::asinus(float value) {
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
        float Math::atang(float value) {
            float result = (float)atan(value);
            return (result < EPSILON && result > -EPSILON) ? 0 : result;
        }
        //Donne le sinus d'un angle donn en radian.
        float Math::sinus(float value) {
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
        float Math::cosinus(float value) {
            float result = (float)sin(value + HALF_PI);
            return (result < EPSILON && result > -EPSILON) ? 0 : result;
        }
        //Donne la tangeante d'un angle donn en radians.
        float Math::tang(float value) {
            float result = (float)tan(value);
            return (result < EPSILON && result > -EPSILON) ? 0 : result;
        }
        float Math::atang2(float value1, float value2) {
            return (float)std::atan2(value1, value2);
        }
        //Donne la valeur absolue d'un nombre.
        float Math::abs(float value) {
            if (value >= 0)
                return value;
            return -value;
        }
        //Renvoie la racine carr�e d'un nombre.
        double Math::sqrt(float value) {
            return std::sqrt(value);
        }
        //Donne l'inverse de la racine carre d'un nombre.
        float Math::inversSqrt(float value) {
            return 1.f / sqrt(value);
        }
        //Donne le logarithme d'un nombre. (En base 10.)
        float Math::log10(float value) {
            return (float)log(value);
        }
        //Donne le logarithme d'un nombre en base base.
        float Math::logn(float value, int base) {
            return (float)(log(value) / log(base));
        }
        //Donne le nombre � la puissance n.
        double Math::power(float value, float exp) {
            return (float)pow(value, exp);
        }
        //Converti un angle en radian.
        float Math::toRadians(float value) {
            return value * DEG_TO_RAD;
        }
        //Convertis un angle en degrer.
        float Math::toDegrees(float value) {
            return value * RAD_TO_DEG;
        }
        //Arrondis un nombre la prcision p.
        float Math::round(float value, int p) {
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
        int Math::roundToInt(float value) {
            int pe = value;
            int pd = value - pe;
            return value - pd;
        }
        //Renvoie l'exponetielle d'un nombre.
        float exp(float value) {
            return (float)std::exp(value);
        }
        //Convertis des coordonn�e polaire en coordon�e cart�sinnes.
        Vec3f Math::toCartesian(float teta, float phi) {
            float rTemp = cosinus(phi);
            float x = rTemp * cosinus(teta);
            float y = rTemp * sinus(teta);
            float z = sinus(phi);
            return Vec3f(x, y, z);
        }
        float Math::clamp(float value, float min, float max) {
            if (value < min)
                value = min;
            if (value > max)
                value = max;
            return value;
        }   
    }
}