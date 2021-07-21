#include "../../../include/odfaeg/Network/encryptedPacket.h"
#include "../../../include/odfaeg/Network/network.h"
namespace odfaeg {
    namespace network {
        using namespace std;
        using namespace sf;
        Rsa& EncryptedPacket::rsa = EncryptedPacket::getRsa();
        bool EncryptedPacket::isSrvRunning = false;
        void EncryptedPacket::setServerRunning (bool serverRunning) {
            isSrvRunning = serverRunning;
        }
        Rsa& EncryptedPacket::getRsa() {
            if (&rsa == nullptr) {
                static Rsa rsa;
                return rsa;
            }
            return rsa;
        }
        int EncryptedPacket::getCertificate(unsigned char** out) {
            return rsa.ossl_getCertificate(out);
        }
        void EncryptedPacket::setCertificate(const unsigned char* in, int length) {
            rsa.ossl_setCertificate(in, length);
        }
        const void* EncryptedPacket::onSend (size_t& dataSize) {
            unsigned char* buffer = nullptr;
            if (isSrvRunning)
                buffer = rsa.ossl_encryptWithPrKey(reinterpret_cast<const unsigned char*> (getData()), getDataSize(), reinterpret_cast<int&>(dataSize));
            else
                buffer = rsa.ossl_encryptWithPbKey(reinterpret_cast<const unsigned char*> (getData()), getDataSize(), reinterpret_cast<int&>(dataSize));
            return &buffer[0];
        }
        void EncryptedPacket::onReceive (const void* data, size_t dataSize) {
            unsigned char* buffer = nullptr;
            std::size_t dstSize = 0;
            if (isSrvRunning)
                buffer = rsa.ossl_decryptWithPrKey(reinterpret_cast<const unsigned char*> (data), dataSize, reinterpret_cast<int&>(dstSize));
            else
                buffer = rsa.ossl_decryptWithPbKey(reinterpret_cast<const unsigned char*> (data), dataSize, reinterpret_cast<int&>(dstSize));
            append(&buffer[0], dstSize);
        }
    }
}
