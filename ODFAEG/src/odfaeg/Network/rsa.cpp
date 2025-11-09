#include "../../../include/odfaeg/Network/rsa.h"
#include <stdlib.h>
#include <iostream>
#include <ctime>
using namespace std;
namespace odfaeg {
    namespace network {
        Rsa::Rsa() {
            x = X509_new();
            evp_pkey = EVP_PKEY_new();
            /*mpz_init(e);
            mpz_init(d);
            mpz_init(n);*/
            keypair = ssl_generateKeys(2048);
            //generateKeys(2048);
        }
        int Rsa::ossl_getCertificate (unsigned char** out) {
            return i2d_X509(x, out);
        }

        void Rsa::ossl_setCertificate(const unsigned char* in, int l) {
            x = d2i_X509(NULL,&in,l);
            EVP_PKEY_free(evp_pkey);
            evp_pkey = EVP_PKEY_new();
            evp_pkey = X509_get_pubkey(x);
            keypair = EVP_PKEY_get1_RSA(evp_pkey);
        }
        /*void Rsa::setCertificate(string certificate) {
            vector<string> pbKeyParts = core::split(certificate, "-");
            mpz_set_str(e, pbKeyParts[0].c_str(), 10);
            mpz_set_str(n, pbKeyParts[1].c_str(), 10);
        }
        std::string Rsa::getCertificate() {
            char *str1, *str2;
            str1 = new char[mpz_sizeinbase(e, 10)];
            str2 = new char[mpz_sizeinbase(n, 10)];
            mpz_get_str(str1,10,e);
            mpz_get_str(str2, 10,n);
            return std::string(str1)+"-"+std::string(str2);
        }
        void Rsa::generateKeys(unsigned int size) {
            mpz_t p;
            mpz_t q;
            gmp_randstate_t etat;              //Initialisation des nombres aleatoires
            gmp_randinit_default (etat);
            mpz_init(p);
            mpz_urandomb(p, etat, size);
            mpz_init(q);
            mpz_nextprime(p, p);
            mpz_urandomb(q, etat, size);
            mpz_nextprime(q, q);
            mpz_mul(n, p, q);
            mpz_t un;
            mpz_init(un);
            mpz_set_si(un, 1);
            mpz_t pmun;
            mpz_t qmun;
            mpz_init(pmun);
            mpz_init(qmun);
            mpz_sub(pmun,p,un);
            mpz_sub(qmun,q,un);
            mpz_t f;
            mpz_init(f);
            mpz_mul(f, pmun, qmun);
            mpz_t zero;
            mpz_init(zero);
            mpz_set_si(zero, 0);
            do {
                mpz_t gcd1, gcd2;
                mpz_init(gcd1);
                mpz_init(gcd2);
                do {
                    mpz_urandomb(e, etat, size);
                    mpz_gcd(gcd1, e, f);
                    mpz_gcd(gcd2, e, n);
                } while (mpz_cmp(gcd1, un) == 0 && mpz_cmp(gcd2, un));
                mpz_invert(d, e, f);
            } while (mpz_cmp(d, zero) == 0);
        }*/
        RSA* Rsa::ssl_generateKeys (int size) {
            Rsa::size = size;


            RSA* key_pair = RSA_generate_key(size, 3, NULL, NULL);
            /*PEM_write_bio_RSAPublicKey(pub, keypair);
            pub_len = BIO_pending(pub);
            pub_key = (char*) malloc(pub_len + 1);
            BIO_read(pub, pub_key, pub_len);
            pub_key[pub_len] = '\0';*/
            EVP_PKEY_set1_RSA(evp_pkey, key_pair);
            X509_set_version(x,2);
            ASN1_INTEGER_set(X509_get_serialNumber(x),0);
            X509_gmtime_adj(X509_get_notBefore(x),0);
            X509_gmtime_adj(X509_get_notAfter(x),(long)60*60*24*365);
            X509_set_pubkey(x,evp_pkey);
            //X509_NAME* name = X509_get_subject_name(x);

            /* This function creates and adds the entry, working out the
             * correct string type and performing checks on its length.
             * Normally we'd check the return value for errors...
             */
           /* X509_NAME_add_entry_by_txt(name,"C", MBSTRING_ASC, (unsigned char*) "BE", -1, -1, 0);
            X509_NAME_add_entry_by_txt(name,"Falior",
            MBSTRING_ASC, (unsigned char*) "Falior Group", -1, -1, 0);*/

            /* Its self signed so set the issuer name to be the same as the
                * subject.
            */
            //X509_set_issuer_name(x,name);
            /* Add various extensions: standard extensions */
            /*add_ext(x, NID_basic_constraints, "critical,CA:TRUE");
            add_ext(x, NID_key_usage, "critical,keyCertSign,cRLSign");

            add_ext(x, NID_subject_key_identifier, "hash");


            add_ext(x, NID_netscape_cert_type, "sslCA");

            add_ext(x, NID_netscape_comment, "example comment extension");*/
            X509_sign(x,evp_pkey,EVP_sha1());
            return key_pair;
        }
        /*const char* Rsa::encryptWithPbKey(const char* data, std::size_t dataSize, std::size_t& newSize) {
            newSize = BLOC_SIZE * dataSize;
            char* crypted = new char[newSize+1];
            unsigned int p = 0;
            for (unsigned int i = 0; i < dataSize; i++) {
                int ascii = data[i];
                mpz_t a;
                mpz_init(a);
                mpz_set_si(a,ascii);
                mpz_t c;
                mpz_init(c);
                mpz_powm(c, a, e, n);
                char* str = new char[mpz_sizeinbase(c, 10)];
                mpz_get_str(str, 10, c);
                for (unsigned int j = 0; j < strlen(str); j++) {
                    crypted[p] = str[j];
                    p++;
                }
                for (unsigned int i = 0; i < BLOC_SIZE - strlen(str); i++) {
                    crypted[p] = 0;
                    p++;
                }
            }
            crypted[p] = 32;
            return crypted;
        }
        const char* Rsa::decryptWithPrKey(const char* data, std::size_t dataSize, std::size_t& newSize) {
            unsigned int p = 0;
            std::vector<std::string> strs;
            while(data[p] != 32) {
                std::string str = "";
                while (data[p] != 0) {
                    str += data[p];
                    p++;
                }
                while (data[p] == 0) {
                    p++;
                }
                strs.push_back(str);
            }
            newSize = strs.size();
            char* decrypted = new char[newSize];
            for (unsigned int i = 0; i < strs.size(); i++) {
                mpz_t c;
                mpz_init(c);
                mpz_set_str(c, strs[i].c_str(), 10);
                mpz_t dec;
                mpz_init(dec);
                mpz_powm(dec,c,d, n);
                int ascii = mpz_get_si(dec);
                decrypted[i] = ascii;
            }
            return decrypted;
        }
        const char* Rsa::encryptWithPrKey(const char* data, std::size_t dataSize, std::size_t& newSize) {
           newSize = BLOC_SIZE * dataSize;
            char* crypted = new char[newSize+1];
            unsigned int p = 0;
            for (unsigned int i = 0; i < dataSize; i++) {
                int ascii = data[i];
                mpz_t a;
                mpz_init(a);
                mpz_set_si(a,ascii);
                mpz_t c;
                mpz_init(c);
                mpz_powm(c, a, d, n);
                char* str = new char[mpz_sizeinbase(c, 10)];
                mpz_get_str(str, 10, c);
                for (unsigned int j = 0; j < strlen(str); j++) {
                    crypted[p] = str[j];
                    p++;
                }
                for (unsigned int i = 0; i < BLOC_SIZE - strlen(str); i++) {
                    crypted[p] = 0;
                    p++;
                }
            }
            crypted[p] = 32;
            return crypted;
        }
        const char* Rsa::decryptWithPbKey(const char* data, std::size_t dataSize, std::size_t& newSize) {
            unsigned int p = 0;
            std::vector<std::string> strs;
            while(data[p] != 32) {
                std::string str = "";
                while (data[p] != 0) {
                    str += data[p];
                    p++;
                }
                while (data[p] == 0) {
                    p++;
                }
                strs.push_back(str);
            }
            newSize = strs.size();
            char* decrypted = new char[newSize];
            for (unsigned int i = 0; i < strs.size(); i++) {
                mpz_t c;
                mpz_init(c);
                mpz_set_str(c, strs[i].c_str(), 10);
                mpz_t dec;
                mpz_init(dec);
                mpz_powm(dec,c,e, n);
                int ascii = mpz_get_si(dec);
                decrypted[i] = ascii;
            }
            return decrypted;
        }*/
        unsigned char* Rsa::ossl_encryptWithPrKey (const unsigned char *data, int dataSize, int& newSize) {
            unsigned char* encData = new unsigned char[RSA_size(keypair)];
            newSize = RSA_private_encrypt(dataSize, data, encData, keypair, RSA_PKCS1_PADDING);
            if (newSize == -1) {
                char* err = (char*) malloc(130);
                ERR_load_crypto_strings();
                ERR_error_string(ERR_get_error(), err);
                fprintf(stderr, "Error encrypting message: %s\n", err);
                free(err);
                return nullptr;
            }
            return encData;
        }
        unsigned char* Rsa::ossl_decryptWithPrKey (const unsigned char *encData, int dataSize, int& newSize) {
            unsigned char *data = new unsigned char[RSA_size(keypair)];
            newSize = RSA_private_decrypt(RSA_size(keypair), encData, data, keypair, RSA_PKCS1_OAEP_PADDING);
            if (newSize == -1) {
                char* err = (char*) malloc(130);
                ERR_load_crypto_strings();
                ERR_error_string(ERR_get_error(), err);
                fprintf(stderr, "Error encrypting message: %s\n", err);
                free(err);
                return nullptr;
            }
            return data;
        }
        unsigned char* Rsa::ossl_encryptWithPbKey (const unsigned char *data,int dataSize, int& newSize) {
            unsigned char *encData = new unsigned char[RSA_size(keypair)];
            newSize = RSA_public_encrypt(dataSize, data, encData, keypair, RSA_PKCS1_OAEP_PADDING);
            if (newSize == -1) {
                char* err = (char*) malloc(130);
                ERR_load_crypto_strings();
                ERR_error_string(ERR_get_error(), err);
                fprintf(stderr, "Error encrypting message: %s\n", err);
                free(err);
                return nullptr;
            }
            return encData;
        }
        unsigned char* Rsa::ossl_decryptWithPbKey (const unsigned char *encData, int dataSize, int &newSize) {
            unsigned char *data = new unsigned char[RSA_size(keypair)];
            newSize = RSA_public_decrypt(RSA_size(keypair), encData, data, keypair, RSA_PKCS1_PADDING);
            if (newSize == -1) {
                char* err = (char*) malloc(130);

                ERR_load_crypto_strings();
                ERR_error_string(ERR_get_error(), err);
                fprintf(stderr, "Error encrypting message: %s\n", err);
                free(err);
                return nullptr;
            }
            return data;
        }
        Rsa::~Rsa() {
            EVP_PKEY_free(evp_pkey);
            RSA_free(keypair);
            X509_free(x);
        }
    }
}
