#ifndef BIG_INT_HPP
#define BIG_INT_HPP
#include <string>
#include <vector>
#include <iostream>
#include <limits>
#include "../../../include/odfaeg/Math/maths.h"
namespace odfaeg {
    class BigInt {
    public :
        friend char symbole( unsigned int valeur);
        friend std::ostream& operator<< (std::ostream& out, const BigInt& bi);
        BigInt(const std::string& number, unsigned int base = 10);
        BigInt(unsigned long long integer=0, bool positif=true, unsigned int b=10);
        static BigInt genRandom (unsigned int nbBits, unsigned int base=2);
        static BigInt genPrime(unsigned int nbBits, unsigned int base=2);
        static BigInt abs(const BigInt& a);
        void setStr(const std::string& number, unsigned int base = 10);
        bool isPositif();
        BigInt operator+(const BigInt& bi) const;
        BigInt& operator+=(const BigInt& bi);
        BigInt operator-(const BigInt& bi) const;
        BigInt& operator-=(const BigInt& bi);
        BigInt operator*(const BigInt& bi) const;
        BigInt& operator*=(const BigInt& bi);
        BigInt operator/(const BigInt& bi) const;
        BigInt& operator/=(const BigInt& bi);
        BigInt operator%(const BigInt& bi) const;
        BigInt& operator%=(const BigInt& bi);
        bool operator== (const BigInt& bi ) const;
        bool operator!= (const BigInt& bi ) const;
        bool operator<= (const BigInt& bi ) const;
        bool operator< (const BigInt& bi ) const;
        bool operator>= (const BigInt& bi ) const;
        bool operator> (const BigInt& bi ) const;
        BigInt operator<< (int n) const;
        BigInt operator>> (int n) const;
        BigInt& operator<<= (int n);
        BigInt& operator>>= (int n);
        BigInt operator& (const BigInt& n) const;
        BigInt operator-() const;
        BigInt pow (const BigInt& exp);
        BigInt prodMod (const BigInt &b, const BigInt &n) const;
        BigInt modOfPow (const BigInt& exp, const BigInt& mod) const;
        bool isNull() const;
        unsigned int getNbChiffres();
        BigInt convert(unsigned int base) const;
        BigInt  m_invert (const BigInt& b) const;
        BigInt sqrt();
        static bool isMersennePrime(const BigInt& p, unsigned int base);
        static BigInt pgcd(BigInt a, BigInt b);
        unsigned int size() const;
        std::string toStr(unsigned int base);
    private :
        static const unsigned int karatsuba_treshold = 20;
        static bool isPrime(unsigned int nbBits, BigInt& p);

        static bool miller (unsigned int nbBits, BigInt& n);

        static int jacobien(BigInt& a, BigInt& b);
        void computeNbC(unsigned long long int c);
        BigInt scaleUp (unsigned int n) const;
        BigInt scaleDown (unsigned long long int n, BigInt &r) const;
        BigInt arraySub (int n, int length) const;
        BigInt addZeros(unsigned int n);
        void insert (unsigned int c, int pos);
        unsigned int operator[](unsigned int i) const;
        void shorten();
        void clear();

        BigInt add(const BigInt& bi) const;
        BigInt sub(const BigInt& bi) const;
        int   comparaison(const BigInt& b) const;
        BigInt operator++(int i);
        BigInt operator--(int i);
        BigInt multiply(const BigInt& bi) const;
        BigInt karatsuba(const BigInt& bi) const;
        BigInt burnikel_ziegler (const BigInt& bi, BigInt &r) const;
        std::vector<unsigned int> chiffres;
        unsigned int base;
        unsigned long long int nbChiffres;
        bool positif;
    };
}
#endif // BIG_INT
