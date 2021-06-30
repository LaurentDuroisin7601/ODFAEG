#ifndef AES_ENC_H
#define AES_ENC_H
#include <iostream>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <ctime>
#include <openssl/rand.h>
#include <openssl/aes.h>
#include "../Core/utilities.h"
#include "../Math/maths.h"
#include "export.hpp"
#include <string.h>
#include <stdlib.h>
#include "../Math/bigInt.hpp"
#include <map>
#include <memory>
#include <exception>
#include <cstring>
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace network {
        /**
          * \file aes.h
          * \class AES_ENC
          * \brief Perform symetric encryptions with the aes algorithm.
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          *
          */
        class ODFAEG_API_EXPORT AES_ENC {
        public:
            //Operation Modes
            //The Electronic Code Book (ECB), Cipher Block Chaining (CBC) and Cipher Feedback Block (CFB) modes
            //are implemented.
            //In ECB mode if the same block is encrypted twice with the same key, the resulting
            //ciphertext blocks are the same.
            //In CBC Mode a ciphertext block is obtained by first xoring the
            //plaintext block with the previous ciphertext block, and encrypting the resulting value.
            //In CFB mode a ciphertext block is obtained by encrypting the previous ciphertext block
            //and xoring the resulting value with the plaintext.
            enum { ECB=0, CBC=1, CFB=2 };
            AES_ENC() : m_bKeyInit (false) {
                ossl_key = new unsigned char[32];
                iv = new unsigned char[32];
                aesSalt = new unsigned char[8];
                e_ctx = EVP_CIPHER_CTX_new();
                d_ctx = EVP_CIPHER_CTX_new();
                aesPass = ossl_generateKey(256);
                generateKey(256);
            }
            /**
            * \fn unisgned char encrypt(const unsigned char* data, int dataSize, int* newSize)
            * \brief encrypt the data.
            * \param data : the data to encrypt.
            * \param dataSize : the size of the data to encrypt.
            * \param newSize : the size of the encrypted data.
            * \return the encrypted data.
            */
            unsigned char* ossl_encrypt(const unsigned char* data, unsigned int dataSize, unsigned int& newSize);
            /**
            * \fn unsigned char* decrypt(const unsigned char* encData, int dataSize, int* newSize)
            * \brief decrypt the data.
            * \param encData : the encrypted data.
            * \param dataSize : the size of the encrypted data.
            * \param newSize : the size of the data.
            * \return the data.
            */
            unsigned char* ossl_decrypt(const unsigned char* encData, unsigned int dataSize, unsigned int& newSize);
            /**
            * \fn char* getKey()
            * \brief get the key used to hash the aes key.
            * \return the key.
            */
            char* ossl_getKey ();
            /**
            * \fn char* getIv()
            * \brief get the iv used to hash the key.
            * \return the iv.
            */
            char* ossl_getIv();
            /**
            * \fn void setKey (char* key);
            * \brief set the key used to hash the aes key.
            * \param key : the key.
            */
            void ossl_setKey (char* key);
            /**
            * \fn void setIv (char* iv);
            * \brief set the iv used to hash the key.
            * \param iv : the iv.
            */
            void ossl_setIv (char* iv);
            /*std::string getKey();
            std::string getIv();
            void setKey(std::string key);
            void setIv(std::string iv);*/
            ~AES_ENC() {
                delete[] ossl_key;
                delete[] iv;
                delete[] aesSalt;
                delete[] aesPass;
                EVP_CIPHER_CTX_free(d_ctx);
                EVP_CIPHER_CTX_free(e_ctx);
                /*delete e_ctx;
                delete d_ctx;*/
            //Expand a user-supplied key material into a session key.
            }
            // key        - The 128/192/256-bit user-key to use.
            // chain      - initial chain block for CBC and CFB modes.
            // keylength  - 16, 24 or 32 bytes
            // blockSize  - The block size in bytes of this Rijndael (16, 24 or 32 bytes).
            void MakeKey(char const* key, char const* chain, int keylength=DEFAULT_BLOCK_SIZE, int blockSize=DEFAULT_BLOCK_SIZE);
            //Encrypt exactly one block of plaintext.
            // in           - The plaintext.
            // result       - The ciphertext generated from a plaintext using the key.
            void EncryptBlock(char const* in, char* result);

            //Decrypt exactly one block of ciphertext.
            // in         - The ciphertext.
            // result     - The plaintext generated from a ciphertext using the session key.
            void DecryptBlock(char const* in, char* result);

            void Encrypt(char const* in, char* result, size_t n, int iMode=ECB);

            void Decrypt(char const* in, char* result, size_t n, int iMode=ECB);

            //Get Key Length
            int GetKeyLength()
            {
                return m_keylength;
            }

            //Block Size
            int	GetBlockSize()
            {
                return BLOCK_SIZE;
            }

            //Number of Rounds
            int GetRounds()
            {
                return m_iROUNDS;
            }

            void ResetChain()
            {
                memcpy(m_chain, m_chain0, BLOCK_SIZE);
            }
            static char const* sm_chain0;
    private:
            enum { DEFAULT_BLOCK_SIZE=16 };
            enum { MAX_BLOCK_SIZE=32, MAX_ROUNDS=14, MAX_KC=8, MAX_BC=8 };

            //Auxiliary Functions
            //Multiply two elements of GF(2^m)
            static int Mul(int a, int b)
            {
                return (a != 0 && b != 0) ? sm_alog[(sm_log[a & 0xFF] + sm_log[b & 0xFF]) % 255] : 0;
            }

            //Convenience method used in generating Transposition Boxes
            static int Mul4(int a, char b[])
            {
                if(a == 0)
                    return 0;
                a = sm_log[a & 0xFF];
                int a0 = (b[0] != 0) ? sm_alog[(a + sm_log[b[0] & 0xFF]) % 255] & 0xFF : 0;
                int a1 = (b[1] != 0) ? sm_alog[(a + sm_log[b[1] & 0xFF]) % 255] & 0xFF : 0;
                int a2 = (b[2] != 0) ? sm_alog[(a + sm_log[b[2] & 0xFF]) % 255] & 0xFF : 0;
                int a3 = (b[3] != 0) ? sm_alog[(a + sm_log[b[3] & 0xFF]) % 255] & 0xFF : 0;
                return a0 << 24 | a1 << 16 | a2 << 8 | a3;
            }
            //Auxiliary Function
            void Xor(char* buff, char const* chain)
            {
                for(int i=0; i<BLOCK_SIZE; i++)
                    *(buff++) ^= *(chain++);
            }

            //Convenience method to encrypt exactly one block of plaintext, assuming
            //Rijndael's default block size (128-bit).
            // in         - The plaintext
            // result     - The ciphertext generated from a plaintext using the key
            void DefEncryptBlock(char const* in, char* result);

            //Convenience method to decrypt exactly one block of plaintext, assuming
            //Rijndael's default block size (128-bit).
            // in         - The ciphertext.
            // result     - The plaintext generated from a ciphertext using the session key.
            void DefDecryptBlock(char const* in, char* result);
            static const int sm_alog[256];
            static const int sm_log[256];
            static const char sm_S[256];
            static const char sm_Si[256];
            static const int sm_T1[256];
            static const int sm_T2[256];
            static const int sm_T3[256];
            static const int sm_T4[256];
            static const int sm_T5[256];
            static const int sm_T6[256];
            static const int sm_T7[256];
            static const int sm_T8[256];
            static const int sm_U1[256];
            static const int sm_U2[256];
            static const int sm_U3[256];
            static const int sm_U4[256];
            static const char sm_rcon[30];
            static const int sm_shifts[3][4][2];
            //Error Messages
            static char const* sm_szErrorMsg1;
            static char const* sm_szErrorMsg2;
            //Key Initialization Flag
            bool m_bKeyInit;
            //Encryption (m_Ke) round key
            int m_Ke[MAX_ROUNDS+1][MAX_BC];
            //Decryption (m_Kd) round key
            int m_Kd[MAX_ROUNDS+1][MAX_BC];
            int m_blockSize;
            //Key Length
            int m_keylength;
            //Number of Rounds
            int m_iROUNDS;
            //Chain Block
            char m_chain0[MAX_BLOCK_SIZE];
            char m_chain[MAX_BLOCK_SIZE];
            //Auxiliary private use buffers
            int tk[MAX_KC];
            int a[MAX_BC];
            int t[MAX_BC];
            /**
            * \fn unsigned char* generateKey(int length)
            * \brief generate an aes key with the given length.
            * \param the length of the key.
            * \return the pass used to hash the key.
            */
            unsigned char* ossl_generateKey(int length) {
                size = length;
                aesPass = new unsigned char[size / 8];
                if (RAND_bytes(aesPass, size / 8) == 0)
                    std::cerr<<"fail to generate aes pass"<<std::endl;
                int nrounds = 5;
                if (RAND_bytes(aesSalt, 8) == 0)
                    std::cerr<<"fail to generate random salt"<<std::endl;
                /*

                * Gen key & IV for AES 256 CBC mode. A SHA1 digest is used to hash the 			* supplied key material.
                * nrounds is the number of times the we hash the material. More rounds 			* are more secure but
                * slower.
                */
                if (EVP_BytesToKey(EVP_aes_256_ecb(), EVP_sha1(), aesSalt, aesPass, 			    size / 8, nrounds, ossl_key, iv) == 0)
                    std::cerr<<"Failed to hash the aes key."<<std::endl;
                EVP_CIPHER_CTX_init(e_ctx);
                //EVP_EncryptInit_ex(e_ctx, EVP_aes_256_cbc(), NULL, key, iv);
                EVP_CIPHER_CTX_init(d_ctx);
                //EVP_DecryptInit_ex(d_ctx, EVP_aes_256_cbc(), NULL, key, iv);
                return aesPass;
            }
            void generateKey(unsigned int size);
            int size; /**> the size of the key.*/
            std::string aes_key; /**> the aes key.*/
            unsigned char *ossl_key, *iv, *aesSalt, *aesPass; /**> the key, the iv, the salt and the pass used to hash the aes key.*/
            EVP_CIPHER_CTX* e_ctx, *d_ctx; /**>The ciphers used to encrypt and decrypt the data. */
            const unsigned int BLOCK_SIZE = 32;
        };
    }
}
#endif // AES_ENC
