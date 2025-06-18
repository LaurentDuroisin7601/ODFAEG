#ifndef CLIENCPACKET
#define CLIENCPACKET
#include <SFML/Network.hpp>
#include "rsa.h"
#include "export.hpp"
namespace odfaeg {
    namespace network {
        class SrkClient;
        class ODFAEG_NETWORK_API CliEncryptedPacket : public sf::Packet {
            public :
                static Rsa& getRsa();
                static int getCertificate(unsigned char** out);
                static void setCertificate(const unsigned char* in, int l);
                static void setServerRunning(bool srvRunning);
            private :
                /**
                * \fn const void* onSend(std::size_t& dataSize)
                * \brief redefinition of the onSend method.
                * \param std::size_t : the size of the datas.
                */
                virtual const void* onSend(std::size_t& dataSize);
                /**
                * \fn const void* onReceive(const void* data, std::size_t dataSize)
                * \brief redefinition of the onReceive method.
                * \param const void* data : the data.
                * \param std::size_t : the size of the datas.
                */
                virtual void onReceive (const void* data, std::size_t dataSize);
                static Rsa& rsa;
                static bool isSrvRunning;
        };
    }
}
#endif
