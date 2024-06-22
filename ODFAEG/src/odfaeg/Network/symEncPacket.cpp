#include "../../../include/odfaeg/Network/symEncPacket.h"
#include <climits>
namespace odfaeg {
    namespace network {
        using namespace std;
        using namespace sf;
        AES_ENC SymEncPacket::aes = AES_ENC();
        const void* SymEncPacket::onSend (size_t& dataSize) {
            unsigned char* buffer;
            buffer = aes.ossl_encrypt(reinterpret_cast<const unsigned char*> (getData()), getDataSize(), reinterpret_cast<unsigned int&>(dataSize));
            return &buffer[0];
        }
        void SymEncPacket::onReceive (const void* data, size_t dataSize) {
            unsigned char* buffer;
            std::size_t dstSize = 0;
            buffer = aes.ossl_decrypt(reinterpret_cast<const unsigned char*> (data), dataSize, reinterpret_cast<unsigned int&>(dstSize));
            append(&buffer[0], dstSize);
        }
    }
}
