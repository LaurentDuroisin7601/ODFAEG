namespace odfaeg {
    namespace network {
        void RSA::generateKeys(unsigned int nbBits, unsigned int base) {
            BigInt p = BigInt::genPrime(nbBits, base);
            BigInt q = p.nextPrime(nbBits, base);
            BigInt n = p * q;
            BigInt phi = (p - 1) * (q - 1);
            e = BigInt(65537, true, base);
            d = e.m_invert(phi);
            this->n = n;    
        }
    }
}