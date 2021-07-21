#ifndef SSL_RSA
#define SSL_RSA
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include "../Core/utilities.h"
#include <fstream>
#include <ctime>
#include "../Math/maths.h"
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include <openssl/engine.h>
#include <openssl/conf.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "export.hpp"
//#include <gmpxx.h>
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace network {
        class ODFAEG_NETWORK_API Rsa {
        /**
          * \file rsa.h
          * \class RSA
          * \brief rsa encryption, with openssl, the keys are contained in a certificate.
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          */
            public :
                Rsa();
                /*const char* encryptWithPbKey(const char* data, std::size_t dataSize, std::size_t& newSize);
                const char* decryptWithPrKey(const char* data, std::size_t dataSize, std::size_t& newSize);
                const char* encryptWithPrKey(const char* data, std::size_t dataSize, std::size_t& newSize);
                const char* decryptWithPbKey(const char* data, std::size_t dataSize, std::size_t& newSize);
                std::string getCertificate();
                void setCertificate(std::string certificate);*/
                /**
                * \fn unsigned char* decryptWithPrKey (const unsigned char* encData, int dataSize, int* newSize)
                * \brief decrypt a message with the private key.
                * \param encData : the encrypted data.
                * \param dataSize : the size of the encrypted data.
                * \param newSize : the size of the data.
                */
                unsigned char* ossl_decryptWithPrKey (const unsigned char* encData, int dataSize, int& newSize);
                /**
                * \fn unsigned char* encryptWithPrKey (const unsigned char* encData, int dataSize, int* newSize)
                * \brief encrypt a message with the private key.
                * \param data : the data.
                * \param dataSize : the size of the data.
                * \param newSize : the size of the encrypted data.
                */
                unsigned char* ossl_encryptWithPrKey (const unsigned char* data, int dataSize, int& newSize);
                 /**
                * \fn unsigned char* decryptWithPbKey (const unsigned char* encData, int dataSize, int* newSize)
                * \brief decrypt a message with the public key.
                * \param encData : the encrypted data.
                * \param dataSize : the size of the encrypted data.
                * \param newSize : the size of the data.
                */
                unsigned char* ossl_decryptWithPbKey (const unsigned char* encData, int dataSize, int& newSize);
                /**
                * \fn unsigned char* encryptWithPbKey (const unsigned char* encData, int dataSize, int* newSize)
                * \brief encrypt a message with the public key.
                * \param data : the data.
                * \param dataSize : the size of the data.
                * \param newSize : the size of the encrypted data.
                */
                unsigned char* ossl_encryptWithPbKey (const unsigned char* data, int dataSize, int& newSize);
                /**
                * \fn int getCertificate(unsigned char** out)
                * \brief get the certificate which contains the keys.
                * \param the certificate.
                * \return the size of the certificate.
                */
                int ossl_getCertificate(unsigned char** out);
                /**
                * \fn void setCertificate(const unsigned char* certificate, int length)
                * \brief set the certificate containing the keys.
                * \param certificate : the certificate.
                * \param length : the length.
                */
                void ossl_setCertificate(const unsigned char* in, int l);
                ~Rsa();
            private :
                static const int BLOC_SIZE = 100;
                //void generateKeys(unsigned int size);
                /**
                * \fn RSA* generateKeys (int size)
                * \brief generate keys with the given size.
                * \param size : the size of the keys.
                * \return the keys.
                */

                RSA* ssl_generateKeys (int size);
                /*static void callback(int p, int n, void *arg)
                {
                    char c='B';
                    if (p == 0) c='.';
                    if (p == 1) c='+';
                    if (p == 2) c='*';
                    if (p == 3) c='\n';
                };*/
                /*static int add_ext(X509 *cert, int nid, char *value) {
                    X509_EXTENSION *ex;
                    X509V3_CTX ctx;

                    X509V3_set_ctx_nodb(&ctx);

                    X509V3_set_ctx(&ctx, cert, cert, NULL, NULL, 0);
                    ex = X509V3_EXT_conf_nid(NULL, &ctx, nid, value);
                    if (!ex)
                        return 0;

                    X509_add_ext(cert,ex,-1);
                    X509_EXTENSION_free(ex);
                    return 1;
                }*/
                int size; /**> the size of the keys.(in bytes)*/
                RSA* keypair; /**> the keys.*/
                X509 *x; /**> the certificate.*/
                EVP_PKEY* evp_pkey; /**The envelop*/
                //mpz_t e, d, n;
        };
    }
}
#endif
