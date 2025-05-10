#include "../../../include/odfaeg/Math/bigInt.hpp"

using namespace std;
namespace odfaeg {
    BigInt::BigInt(const std::string& number, unsigned int base) {
        this->base = base;
        positif = (number.at(number.length()-1) != '-') ? true : false;
        nbChiffres = (positif) ? number.length() : number.length() - 1;

        for (unsigned int i = 0; i < nbChiffres; i++) {
            unsigned int c;
            if (number.at(i) >= 48 && number.at(i) <= 57){
                c = number.at(i) - 48;
            } else if (number.at(i) >= 65) {
                c = number.at(i) - 55;
            }
            chiffres.push_back(c);
        }
    }
    void BigInt::setStr(const std::string& number, unsigned int base) {
        chiffres.clear();
        this->base = base;
        positif = (number.at(number.length()-1) != '-') ? true : false;
        nbChiffres = (positif) ? number.length() : number.length() - 1;
        for (unsigned int i = 0; i < nbChiffres; i++) {
            unsigned int c;
            if (number.at(i) >= 48 && number.at(i) <= 57){
                c = number.at(i) - 48;
            } else if (number.at(i) >= 65) {
                c = number.at(i) - 55;
            }
            chiffres.push_back(c);
        }
    }
    BigInt::BigInt(unsigned long long integer, bool positif, unsigned int b) {
        this->positif = positif;
        this->base = b;
        nbChiffres = 0;
        if (integer > 0) {
            if (base < std::numeric_limits<unsigned int>::max()) {
                unsigned long long int max = 1LL;
                while (integer >= max)
                    max *= base;
                max /= base;
                while (max > 0) {
                    unsigned long long int c;
                    c = integer / max;
                    computeNbC(c);
                    chiffres.push_back(c);
                    integer %= max;
                    max /= b;
                }
            } else {
                unsigned long long int c = integer / base;
                computeNbC(c);
                chiffres.push_back(c);
                integer %= base;
                computeNbC(integer);
            }
        }
    }
    BigInt BigInt::sqrt() {
        BigInt z = *this;
        BigInt rst;
        int max = 8;     // to define maximum digit
        int i;
        BigInt j(1, true, base);
        BigInt dix(10, true, base);
        BigInt bi (max, true, base);
        BigInt two(2, true, base);
        for(i = max ; i >= 0 ; i--){
            // value must be bigger then 0
            if(z - ((two * rst ) + ( j * dix.pow(i))*( j * dix.pow(i))) >= 0)
            {
                while( z - (( two * rst ) + ( j * dix.pow(i)))*( j * dix.pow(i)) >= 0
                      && j >= 1)
                {
                    j++;
                }
                j--; //correct the extra value by minus one to j
                z -= (( two * rst ) + ( j * dix.pow(i)))*( j * dix.pow(i)); //find value of z25:
                rst += j * dix.pow(i);     // find sum of a
                j = 1;
            }
        }
        return rst;
    }
    void BigInt::computeNbC (unsigned long long int c) {
        unsigned int nb = base;
        unsigned int i = 1;
        while (c >= nb) {
            nb *= base;
            i++;
        }
        nbChiffres += i;
    }
    unsigned int BigInt::getNbChiffres() {
        return nbChiffres;
    }
    BigInt BigInt::genRandom (unsigned int nbBits, unsigned int base) {
       math::Math::initSeed();
       unsigned int nearestPowerOfTwo = math::Math::logn(base, 2);
       base = math::Math::power(2, nearestPowerOfTwo);
       BigInt n;
       n.base = base;
       n.nbChiffres = (nearestPowerOfTwo == 1) ? nbBits : nbBits / nearestPowerOfTwo;
       do {
            for (unsigned long long int i = 0; i < n.nbChiffres; ++i) {
                unsigned int c = math::Math::random(base);
                n.chiffres.push_back(c);
            }

        } while (n.isNull());
        n.shorten();
        return n;
    }
    BigInt BigInt::genPrime(unsigned int nbBits, unsigned int base) {
        BigInt p;
        do {
            p = genRandom(nbBits, base);
        } while(!miller(nbBits, p));
        return p;
    }
    bool BigInt::isPrime(unsigned int nbBits, BigInt& p) {
        // on effectue NB_ITER fois la boucle.
        if (p <= 1) return false;
        if (p % 2 == 0) return false;
        BigInt i (3, true, p.base);
        for (; i*i <= p; i+= 2) {
            //std::cout<<"i : "<<i<<std::endl;
            if (p % i == 0)
                return false;
        }
        // le nombre est considéré comme premier
        return true;
    }
    int BigInt::jacobien(BigInt& a, BigInt& b)
    {
        if(a == 0)
            return 0;
        int J = 1;
        while(a != 1)
        {
            if(a % 2 == 0)
            {
                if( ((b * b - 1) >> 3) % 2 != 0)
                    J *= -1;

                a >>= 1;
            }
            else
            {
                if( ((a - 1) * (b - 1) >> 2) % 2 != 0)
                    J *= -1;

                BigInt temp = b % a;
                b = a;
                a = temp;
            }
        }

        return J;
    }
    BigInt BigInt::pgcd(BigInt a, BigInt b)
    {
       return (b > 0) ?  pgcd(b,a%b) : a;
    }
    bool BigInt::isPositif () {
        return positif;
    }
    void BigInt::shorten () {

        vector<unsigned int>::iterator it;
        for (it = chiffres.begin(); it != chiffres.end(); ) {
            if (*it == 0) {
                it = chiffres.erase(it);
                nbChiffres--;
            } else
                break;
        }
    }
    void BigInt::clear()
    {
        chiffres.clear();
    }
    bool BigInt::isNull() const {
        return chiffres.empty();
    }
    unsigned int BigInt::size() const {
        return chiffres.size();
    }
    unsigned int BigInt::operator[] ( unsigned int i ) const {
        return (i>=chiffres.size()) ? chiffres[chiffres.size() - 1] : chiffres[i];
    }
    BigInt BigInt::operator++ (int i) {
        BigInt un(1, true, base);
        *this += un;
        return *this;
    }
    void BigInt::insert(unsigned int c, int pos) {

        if (pos <= size()) {
            BigInt result(0, true, base);
            result.chiffres.resize(size() + 1, 0);
            for (unsigned int i = 0, j = 0; i <= size(); i++, j++) {
                if (i != pos) {
                    result.chiffres[i] = chiffres[j];
                } else {
                    j--;
                }
            }
            result.chiffres[pos] = c;
            computeNbC(c);
            *this = result;
        }
    }
    BigInt BigInt::operator-- (int i) {
        BigInt un(1, true, base);
        *this -= un;
        return *this;
    }
    int BigInt::comparaison (const BigInt &b) const {
        /*std::cout<<"sizes : "<<chiffres.size()<<" "<<b.chiffres.size()<<std::endl;
        std::string s;
        std::cin>>s;*/
        if (positif && !b.positif)
            return +1;
        if (!positif && b.positif)
            return -1;
        if ((chiffres.size() > b.chiffres.size() && positif && b.positif)
            || (chiffres.size() < b.chiffres.size() && !positif && !b.positif))
            return +1;
        if ((chiffres.size() < b.chiffres.size() && positif && b.positif)
            || (chiffres.size() > b.chiffres.size() && !positif && !b.positif))
            return -1;

        for(unsigned int i= 0; i < chiffres.size(); i++) {
            if ((chiffres[i] > b.chiffres[i] && positif && b.positif)
                || (chiffres[i] < b.chiffres[i] && !positif && !b.positif))
                return +1;
            if ((chiffres[i] < b.chiffres[i] && positif && b.positif)
                || (chiffres[i] > b.chiffres[i] && !positif && !b.positif))
                return -1;
        }
        return 0;
    }
    bool BigInt::operator== ( const BigInt& a ) const {
        return ( comparaison(a) == 0 );
    }
    bool BigInt::operator!= ( const BigInt& a) const {
        return ( comparaison(a) != 0 );
    }
    bool BigInt::operator< ( const BigInt& a ) const {
        return ( comparaison(a) < 0 );
    }
    bool BigInt::operator<= ( const BigInt& a ) const {
        return ( comparaison(a) <= 0 );
    }
    bool BigInt::operator> ( const BigInt& a) const {
        return ( comparaison(a) > 0 );
    }
    bool BigInt::operator>= ( const BigInt& a) const {
        return ( comparaison(a) >= 0 );
    }
    BigInt BigInt::operator-() const {
        BigInt result = *this;
        result.positif = !positif;
        return result;
    }
    BigInt BigInt::add (const BigInt& bi) const {
        if (isNull())
            return bi;
        if (bi.isNull())
            return *this;
        BigInt somme = *this;
        unsigned int retenue= 0;
        int i, j;
        for(i=size() - 1, j = bi.size() - 1; j >= 0; i--, j--)  {

            unsigned long long int temp = (unsigned long long int) (*this)[i] + (unsigned long long int) bi[j] + (unsigned long long int) retenue;

            if ( temp < base) {
                somme.chiffres[i] = (unsigned int) temp;
                retenue = 0;
            }
            else {
                somme.chiffres[i]= (unsigned int) temp - base;
                retenue = 1;
            }
        }
        while (retenue != 0) {
            if (i >= 0) {
                unsigned long long int temp = (unsigned long long int) (*this)[i] + (unsigned long long int) retenue;

                if ( temp < base) {
                    somme.chiffres[i] = (unsigned int) temp;
                    retenue = 0;
                } else {
                    somme.chiffres[i]= (unsigned int) temp - base;

                }
                i--;
            } else {
                somme.chiffres.insert(somme.chiffres.begin(), retenue);
                retenue = 0;
            }
        }
        somme.shorten();
        return somme;
    }

    BigInt BigInt::operator+ (const BigInt &bi) const {
        BigInt a = *this;
        BigInt b = bi;
        BigInt result(0, true, base);
        if (positif && bi.positif) {
            if (b > a) {
                a = bi;
                b = *this;
            }
            result = a.add(b);
        } else if (!positif && bi.positif) {
            if (abs(*this) >= abs(bi)) {
                result = a.sub(b);
            } else {
                result = b.sub(a);
                result.positif = true;
            }
        } else if (positif && !bi.positif) {
            if (abs(*this) >= abs(bi)) {
                result = a.sub(b);
            } else {
                result = b.sub(a);
                result.positif = false;
            }
        } else {
           if (abs(b) > abs(a)) {
                a = bi;
                b = *this;
           }
           result = a.add(b);
        }
        return result;
    }
    BigInt& BigInt::operator+= (const BigInt &bi) {
        *this = *this + bi;
        return *this;
    }
    BigInt BigInt::sub (const BigInt &bi) const {
        if (bi.isNull())
            return *this;
        BigInt diff = *this;
        // Calculer la somme chiffre par chiffre en tenant compte des retenues
        unsigned int retenue= 0;
        int i, j;
        for(i=size() - 1, j = bi.size() - 1; j>=0; i--, j--)  {
            long long temp = (long long) ((unsigned long long int) (*this)[i] - (unsigned long long int) bi[j] - (unsigned long long int) retenue);

            if ( temp >= 0 ) {
                diff.chiffres[i]= (unsigned int) temp;
                retenue = 0;
            }
            else {
                diff.chiffres[i]= (unsigned int) (temp + base);
                retenue = 1;
            }
        }
        while (retenue > 0) {
            long long int temp = (long long int) ((unsigned long long int) (*this)[i] - (unsigned long long int) retenue);

            if ( temp >= 0 ) {
                diff.chiffres[i]= (unsigned int) temp;
                retenue = 0;
            }
            else {

                diff.chiffres[i]= (unsigned int) (temp + base);
            }
            i--;
        }
        diff.shorten();
        return diff;
    }
    BigInt BigInt::abs(const BigInt& a) {
        BigInt b = a;
        b.positif = true;
        return b;
    }
    BigInt BigInt::operator- (const BigInt &bi) const {
        BigInt result(0, true, base);
        BigInt a = *this;
        BigInt b = bi;
        if (positif && bi.positif) {
            if (*this == bi)
                return result;
            if (*this > bi) {
                result = a.sub(b);
            } else {
                result = b.sub(a);
                result.positif = false;
            }
        } else if (!positif && bi.positif) {
            if (abs(b) > abs(a)) {
                a = bi;
                b = *this;
            }
            result = a.add(b);
        } else if (positif && !bi.positif) {
            if (abs(b) > abs(a)) {
                a = bi;
                b = *this;
            }
            result = a.add(b);
        } else {
            if (abs(*this) > abs(bi)) {
                result = a.sub(b);
            } else {
                result = b.sub(a);
                result.positif = true;
            }
        }
        return result;
    }
    BigInt& BigInt::operator-= (const BigInt &bi) {
        *this = *this - bi;
        return *this;
    }
    BigInt BigInt::addZeros (unsigned int n) {
        BigInt result(0, positif, base);
        result.chiffres.resize(size() + n, 0);
        nbChiffres += n;
        for (unsigned int i = 0; i < size(); i++) {
             result.chiffres[i] = chiffres[i];
        }
        return result;
    }
    BigInt BigInt::multiply (const BigInt &bi) const {

        BigInt produit(0, true, this->base);
        produit.clear();
        if ( this->size() == 0 || bi.size() == 0 ) {
            return produit;
        }

        int i = 0, j = bi.size() - 1;
        produit = scaleUp(bi.chiffres[i]).addZeros(j);
        while ( j > 0)
        {
           i++; j--;
           produit += scaleUp (bi.chiffres[i]).addZeros(j);

        }
        produit.shorten();
        return produit;
    }
    BigInt BigInt::karatsuba (const BigInt &bi) const {
        if (size() >= bi.size()) {
            unsigned int n = size() / 2;
            BigInt a = arraySub(0, size() - n);
            BigInt b = arraySub(size() - n, n);
            if (bi.size() > n) {
                BigInt c = bi.arraySub(0, bi.size() - n);
                BigInt d = bi.arraySub(bi.size() - n, n);
                BigInt ac = a * c;
                BigInt bd = b * d;
                BigInt ad_bc = (a + b) * (c + d) - ac - bd;
                ac = ac.addZeros(2*n);
                ad_bc = ad_bc.addZeros(n);
                return ac + ad_bc + bd;
            } else {
                BigInt aq = a * bi;
                BigInt bq = b * bi;
                aq = aq.addZeros(n);
                return aq + bq;
            }
        }
        return *this;
    }
    BigInt BigInt::operator* (const BigInt &bi) const {
        // D´el´eguer les petites multiplications `a la m´ethode scolaire

        BigInt result(0, true, base);
        if (size() < bi.size())
            result = bi * *this;
        else if (bi.size() < karatsuba_treshold)
            result = multiply(bi);
        else
            result = karatsuba(bi);
        if (!positif && bi.positif || positif && !bi.positif)
            result.positif = false;
        else
            result.positif = true;
        return result;
    }
    BigInt& BigInt::operator*= (const BigInt &bi) {
        *this = *this * bi;
        return *this;
    }


    BigInt BigInt::operator/ (const BigInt &bi) const {
        if (bi == 0) {
            cerr<<"Error : b is null!";
            return 0;
        }
        if (bi > *this) {
            return 0;
        }
        BigInt result(0, true, base);
        BigInt reste(0, true, base);
        result = burnikel_ziegler(bi, reste);
        if (!positif && bi.positif || positif && !bi.positif)
            result.positif = false;
        else
            result.positif = true;
        return result;

    }
    BigInt& BigInt::operator/= (const BigInt &bi) {
        *this = *this / bi;
        return *this;
    }

    BigInt BigInt::operator% (const BigInt& bi) const {

        if (bi == BigInt(0)) {
            cerr<<"Error : b is null!";
            return 0;
        }
        if (bi > *this)
            return *this;
        BigInt reste(0, true, base);
        burnikel_ziegler(bi, reste);
        reste.shorten();
        if (!positif && bi.positif || positif && !bi.positif)
            reste.positif = false;
        else
            reste.positif = true;
        return reste;
    }
    BigInt BigInt::pow (const BigInt& exp) {
        BigInt un(1, true, base);
        BigInt deux (2, true, base);
        BigInt zero (0, true, base);
        if (exp.isNull()) {
            return un;
        } else if (exp % deux == zero) {
            BigInt a = pow(exp / deux);
            return (a * a);
        } else {
            return (*this) * pow (exp - un);
        }
    }
    BigInt BigInt::prodMod (const BigInt &b, const BigInt &n) const {
      if(isNull()) {
        return 0;
      } else {
        // a = 2 * q  + e
        BigInt q = *this / BigInt(2, true, base);
        BigInt e = *this % BigInt(2, true, base);
        BigInt r = BigInt(2, true, base) * q.prodMod(b, n) % n;
        return e == 0 ? r : (r + b) % n;
      }
    }
    BigInt BigInt::modOfPow (const BigInt& exp, const BigInt& mod) const {
        /*BigInt result (1, true, base);
        BigInt one(1, true, base);
        BigInt two (2, true, base);
        BigInt zero(0, true, base);
        BigInt e = exp, bs;
        bs = *this;
        while (e > zero) {

            if (e % two == one)
                result = result * bs % mod;
            e /= two;
            //std::cout<<"e : "<<e<<"base : "<<bs<<"result : "<<result<<std::endl;
            bs = bs * bs % mod;
        }
        return result;*/
        BigInt e = exp.convert(2);
        BigInt bs = *this;
        BigInt result (1, true, base);
        BigInt zero(0, true, 2);
        while (e > zero) {
            if ((e & BigInt(1, true, 2)) > zero) {
                result = result * bs % mod;
            }
            e >>= 1;
            bs = bs * bs % mod;
        }
        return result;
    }
    BigInt& BigInt::operator%= (const BigInt &bi) {
        *this = *this % bi;
        return *this;
    }
    BigInt BigInt::arraySub (int n, int length) const {
        const_cast<BigInt*>(this)->shorten();
        BigInt result(0, true, this->base);
        if (n >= 0 && n + length <= size() && length  > 0) {
            result.chiffres.resize(length, 0);
            for (int i = n, j = 0; j < length; i++, j++) {
                result.chiffres[j] = chiffres[i];
            }
        }
        return result;
    }
    BigInt BigInt::operator<< (int n) const {
        BigInt result(0, positif, base);
        result.chiffres.resize(size() + n, 0);
        for (unsigned int i = 0; i < size(); i++) {
             result.chiffres[i] = chiffres[i];
        }
        return result;
    }
    BigInt BigInt::operator>> (int n) const {
        if (n < 0)
            return *this;
        if (isNull())
            return *this;
        if (n > size())
            n = size();
        BigInt result (0, true, base);
        int l = ((int) size() - n) < 0 ? 0 : size() - n;
        result.chiffres.resize(l, 0);
        for (unsigned int i = 0; i < result.size(); i++)
            result.chiffres[i] = chiffres[i];
        return result;
    }
    BigInt& BigInt::operator<<= (int n) {
        BigInt result(0, positif, base);
        result.chiffres.resize(size() + n, 0);
        for (unsigned int i = 0; i < size(); i++) {
             result.chiffres[i] = chiffres[i];
        }
        *this = result;
        return *this;
    }
    BigInt& BigInt::operator>>= (int n) {
        if (n < 0)
            return *this;
        if (isNull())
            return *this;
        if (n > size())
            n = size();
        BigInt result (0, true, base);
        int l = ((int) size() - n) < 0 ? 0 : size() - n;
        result.chiffres.resize(l, 0);
        for (unsigned int i = 0; i < result.size(); i++)
            result.chiffres[i] = chiffres[i];
        *this = result;
        return *this;
    }
    BigInt BigInt::operator& (const BigInt& n) const {
        BigInt result (0, true, base);
        unsigned int max = (n.size() < size()) ? size() : n.size();
        BigInt b = n;
        BigInt a = *this;
        for (unsigned int i = 0; i < max; i++) {
            if (i >= a.size())
                a.insert(0, 0);
            if (i >= b.size())
                b.insert(0, 0);
        }
        result.chiffres.resize(max, 0);
        for (unsigned int i = 0; i < max; ++i) {
            result.chiffres[i] = a.chiffres[i] & b.chiffres[i];
        }
        result.shorten();
        return result;
    }
    BigInt BigInt::scaleUp (unsigned int n) const {

        unsigned long long int accu = 0;
        unsigned int retenue = 0;
        BigInt result(0, true, base);
        result.chiffres.resize(size() + 1, 0);

        if (n >= 0 && n < base) {
            if (n == 0)
                return 0;
            for (int i = size(); i > 0; i--) {
                accu = (unsigned long long int) chiffres[i-1] * n + (unsigned long long int) retenue;
                result.chiffres[i] = (unsigned int) (accu % base);
                retenue = (unsigned int) (accu / base);
            }
            result.chiffres[0] = retenue;
            if (retenue == 0)
                result.shorten();
        }
        return result;
    }
    BigInt BigInt::scaleDown (unsigned long long int n, BigInt& r) const {

        unsigned long long int accu = 0;
        unsigned int retenue = 0;
        BigInt result = *this;
        if (n >= 0 && n < base * base) {
            for (unsigned int i = 0; i < size(); i++) {
                accu = (unsigned long long) chiffres[i] + (unsigned long long int) retenue * base;
                result.chiffres[i] = (unsigned int) (accu / n);
                retenue = (unsigned int) (accu % n);
            }

        }
        r = BigInt(retenue, true, base);
        result.shorten();
        return result;
    }
    BigInt BigInt::burnikel_ziegler(const BigInt &bi, BigInt &r) const {
        BigInt q(0, true, base);
        if (bi.size() <= 2) {
            unsigned long long int b2 = (bi.size() < 2) ? bi.chiffres[0] : bi.chiffres[0] * base + bi.chiffres[1];
            q = scaleDown(b2, r);
            return q;
        }
        unsigned int n = (bi.size() - 1) / 2;

        // D´ecouper a et b en deux moiti´es
        BigInt a0, a1;
        a0 = arraySub(size() - n, n);
        a1 = arraySub(0, size() - n);
        if (a1 >= bi) {

            BigInt q1(0, true, base), r1(0, true, base), q0(0, true, base), r0(0, true, base);
            q1 = a1.burnikel_ziegler (bi, r1);
            r1 = r1.addZeros(n);
            q0 = (r1 + a0).burnikel_ziegler (bi, r0);
            q1 = q1.addZeros(n);
            r = r0;
            return q1 + q0;
        }
        BigInt b0(0, true, base), b1(0, true, base), q1(0, true, base), r1(0, true, base),
        a0_r1(0, true, base), b0_q1(0, true, base);
        b0 = bi.arraySub(bi.size() - n, n);
        b1 = bi.arraySub(0, bi.size() - n);
        q1 = a1.burnikel_ziegler(b1, r1);
        r1 = r1.addZeros(n);
        a0_r1 = r1 + a0;
        b0_q1 = b0 * q1;
        if (a0_r1 >= b0_q1) {
            r = a0_r1 - b0_q1;
            return q1;
        }
        BigInt minus_x = b0_q1 - a0_r1;
        r = bi - minus_x;
        return q1 - BigInt(1, true, base);
    }
    BigInt BigInt::m_invert(const BigInt& b) const
    {
        BigInt n0 = b;
        BigInt b0 = *this;
        BigInt t0 (0, true, base);
        BigInt t (1, true, base);
        BigInt q = n0/b0;
        BigInt r = n0 - (q * b0);
        BigInt temp = 0;
        while (r > 0)
        {
            temp = t0 - (q * t);
            if (temp >= 0)
                temp = temp % b;
            else
                temp = b - ((-temp) % b);
            t0 = t;
            t = temp;
            n0 = b0;
            b0 = r;
            q = n0/b0;
            r = n0 - (q * b0);
        }
        if (b0 != 1)
            return 0;
        return t;
    }
    bool BigInt::miller(unsigned int nbBits, BigInt& n) {
        BigInt n2 = n.convert(2);
        if ((n2 & 1) == 0)
            return false;
        BigInt n1 = n-1;
        BigInt m = n1;
        unsigned int k = 1;
        BigInt zero(0, true, n.base);
        BigInt one(1, true, n.base);
        //std::cout<<"cond 1 : "<<(n1 % (BigInt(1 << (k + 2), true, n.base)) == zero)<<std::endl;
        while (n1 % (BigInt(1 << (k + 2), true, n.base)) == zero) {
            k += 2;
            m >>= 2;
        }
        for (unsigned int i = 0; i < 10; i++) {
            BigInt a = genRandom(nbBits, n.base);
            //std::cout<<"a : "<<a<<" a >= n ?"<<(a >= n)<<std::endl;
            while (a >= n) {
                a = genRandom(nbBits, n.base);
            }
            BigInt b = a.modOfPow(m, n);
            if (b % n != one) {
                unsigned int j;
                for (j = 0; j < k && b % n != n1; ++j) {
                    b *= b;
                }
                if (j >= k)
                    return false;
            }
        }
        return true;
        /*BigInt d=(n2-BigInt(1, true, 2))>>1;
        int s = 1;
        while ((d & 1) == 0) {
            s += 1;
            d >>= 1;
        }
        int k = 10;
        while (k) {
            k--;
            std::cout<<"k : "<<k<<std::endl;
            BigInt a = genRandom(nbBits, 2);
            if (a.modOfPow(d, n2) != BigInt(1, true, 2)) {
                while (s) {
                    s--;
                    std::cout<<"d : "<<d<<std::endl<<"s : "<<s<<std::endl<<"d<<s : "<<(d<<s)<<std::endl;
                    if (a.modOfPow(d<<s, n2) != n2 - BigInt(1, true, 2))
                        return false;
                }
            }
        }
        std::cout<<"true"<<std::endl;
        return true;*/
    }
    bool BigInt::isMersennePrime(const BigInt& p, unsigned int base) {
        if (p == 2) {
            return true;
        } else {
            BigInt s(4, true, base);
            BigInt div = BigInt(2, true, base).pow(p) - 1;
            for (BigInt i = 3; i <= p; i++) {
                s = (s * s - BigInt(2, true, base)) % div;

            }
            return s == 0;
        }
    }
    BigInt BigInt::convert (unsigned int b) const {

        BigInt newBi = *this;
        if (b != base) {

            newBi = BigInt (0, true, 10);
            BigInt bs = BigInt(base, true, 10);

            for (int i = 0; i < size(); i++) {
                BigInt exp (size() - i - 1, true, 10);
                BigInt c1 (chiffres[i], true, 10);
                BigInt c2 = c1 * bs.pow(exp);
                newBi += c2;
            }
            if (b != 10) {
                BigInt max(1, true, 10);
                bs = BigInt (b, true, 10);
                BigInt r(0, true, 10);

                while (newBi >= max) {
                    max = max * bs;
                }

                max /= bs;

                newBi.shorten();
                BigInt a = newBi;
                newBi.clear();
                newBi.nbChiffres = 0;
                while (max > 0) {
                    BigInt c;
                    c = a / max;
                    if (c == 0) {
                        newBi.chiffres.push_back(0);
                        newBi.computeNbC(0);
                    } else {
                        unsigned int somme = 0;
                        for (unsigned int i = 0; i < c.size(); i++) {

                            somme += c.chiffres[i] * math::Math::power(10, c.size() - i - 1);

                        }
                        newBi.chiffres.push_back(somme);
                        a %= max;
                        newBi.computeNbC(somme);
                    }
                    max /= bs;
                }
            }
            newBi.shorten();
            newBi.base = b;
            return newBi;

        }
        return newBi;
    }
    // Conversion valeur -> symbole pour la sortie
    char symbole( unsigned int valeur) {

        return ((valeur< 10) ? 48 + valeur : 55 + valeur);
    }
    std::string BigInt::toStr(unsigned int base) {
        BigInt bi = *this;
        if (base != this->base) {
            bi = convert(base);
        }
        if ( bi.size()== 0 ) return ( "0" );
        std::string str="";
        if (!bi.positif)
                str+="-";
        for( int i = 0; i < bi.size(); i++) {
            str+=symbole(bi.chiffres[i]);
        }
        return str;
    }

    // Op´erateur de sortie (afficher les chiffres, ou bien "0" pour une suite vide)
    ostream& operator<< ( ostream& out, const BigInt& bi) {

        if ( bi.size()== 0 ) return ( out << 0 );
        if (!bi.positif)
                out<<"-";
        for( int i = 0; i < bi.size(); i++) {

            out<<symbole(bi.chiffres[i]);
            if (i != bi.size()-1)
                out<<" ";
        }
        return out;
    }
}

