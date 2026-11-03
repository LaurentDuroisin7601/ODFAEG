#ifndef ODFEAG_BIGINT_HPP
#define ODFEAG_BIGINT_HPP

/*using namespace std;
namespace odfaeg
{
    namespace math {
        BigInt(const std::string &number, unsigned int base);
        void setStr(const std::string &number, unsigned int base);
        BigInt(unsigned long long integer, bool positif, unsigned int b);
        BigInt sqrt();
        void computeNbC(unsigned long long int c);
        unsigned int getNbChiffres();
        BigInt genRandom(unsigned int nbBits, unsigned int base);
        BigInt genPrime(unsigned int nbBits, unsigned int base);
        bool isPrime(unsigned int nbBits, BigInt &p);
        BigInt pgcd(BigInt a, BigInt b);
        bool isPositif();
        void shorten();
        void clear();
        bool isNull();
        unsigned int size();
        unsigned int operator[](unsigned int i);
        BigInt operator++(int i);
        void insert(unsigned int c, int pos);
        BigInt operator--(int i);
        int comparaison(const BigInt &b);
        bool operator==(const BigInt &a);
        bool operator!=(const BigInt &a);
        bool operator<(const BigInt &a);
        bool operator<=(const BigInt &a);
        bool operator>(const BigInt &a);
        bool operator>=(const BigInt &a);
        BigInt operator-();
        BigInt add(const BigInt &bi);
        BigInt operator+(const BigInt &bi);
        BigInt operator+=(const BigInt &bi);
        BigInt sub(const BigInt &bi);
        BigInt operator-(const BigInt &bi);
        BigInt operator-=(const BigInt &bi);
        BigInt addZeros(unsigned int n);
        BigInt multiply(const BigInt &bi);
        BigInt karatsuba(const BigInt &bi);
        BigInt operator*(const BigInt &bi);
        BigInt operator*=(const BigInt &bi);
        BigInt operator/(const BigInt &bi);
        BigInt operator/=(const BigInt &bi);
        BigInt operator%(const BigInt &bi);
        BigInt pow(const BigInt &exp);
        BigInt prodMod(const BigInt &b, const BigInt &n);
        BigInt modOfPow(const BigInt &exp, const BigInt &mod);
        BigInt operator%=(const BigInt &bi);
        BigInt arraySub(int n, int length);
        BigInt operator<<(int n);
        BigInt operator>>(int n);
        BigInt operator<<=(int n);
        BigInt operator>>=(int n);
        BigInt operator&(const BigInt &n);
        BigInt scaleUp(unsigned int n);
        BigInt scaleDown(unsigned long long int n, BigInt &r);
        BigInt burnikel_ziegler(const BigInt &bi, BigInt &r);
        BigInt m_invert(const BigInt &b);       
        BigInt convert(unsigned int b);
        // Conversion valeur -> symbole pour la sortie
        char symbole(unsigned int valeur);
        std::string BigInt::toStr(unsigned int base);
        // Op´erateur de sortie (afficher les chiffres, ou bien "0" pour une suite vide)
        ostream &operator<<(ostream &out, const BigInt &bi);
    }
}*/
#endif // ODFEAG_BIGINT_HPP