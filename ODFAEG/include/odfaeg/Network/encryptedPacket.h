#ifndef ENCRYPTED_PACKET
#define ENCRYPTED_PACKET
#include <SFML/Network.hpp>
#include "rsa.h"
#include "export.hpp"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace network {
        /**
          * \file encryptedPacket.h
          * \class EncryptedPacket
          * \brief encrypted a packet using the rsa algorithm.
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          *
          */
        class SrkClient;
        class ODFAEG_API_EXPORT EncryptedPacket : public sf::Packet {
            public :
                static Rsa& getRsa();
                static int getCertificate(unsigned char** out);
                static void setCertificate(const unsigned char* in, int l);
                static void setServerRunning(bool isSrvRunning);
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
