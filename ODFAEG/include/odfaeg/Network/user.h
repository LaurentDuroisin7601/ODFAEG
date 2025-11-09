#ifndef ODFAEG_USER_HPP
#define ODFAEG_USER_HPP
#include "../Core/utilities.h"
#include "export.hpp"

#include <condition_variable>
#include "../Core/clock.h"
#include "tcpsocket.hpp"
#include "updsocket.hpp"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace network {
        /**
          * \file user.h
          * \class a user of the network. (Users are clients connected to the server)
          * \brief
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          */
        class ODFAEG_NETWORK_API User {
            public :
                /**
                * \fn User(sf::TcpSocket &socketTCP, sf::UdpSocket &socketUDP)
                * \brief constructor.
                */
                User(TcpSocket &socketTCP, UdpSocket &socketUDP);
                void setChannel(int channel);
                /**
                * \fn void setRemotePortUDP (short unsigned int port)
                * \brief set the remote udp port.
                * \param port : the remote udp port.
                */
                void setRemotePortUDP (short unsigned int port);
                /**
                * \fn void setUseSecuredConnexion(bool b)
                * \brief if the client use an SSL connection.
                * \param b : if the client use an SSL connection.
                */
                void setUseSecuredConnexion(bool b);
                /**
                * \fn short unsigned int getRemotePortUDP ()
                * \brief get the remote udp port.
                * \return the remote udp port.
                */
                short unsigned int getRemotePortUDP ();
                /**
                * \fn void sendTcpPacket(Packet &packet)
                * \brief send a Packet to the client using the tcp protocol.
                */
                void sendTcpPacket(Packet &packet);
                /**
                * \fn void sendUdpPacket(Packet &packet)
                * \brief send a Packet to the client using the udp protocol.
                */
                void sendUdpPacket(Packet &packet);
                /**
                * \fn bool hPbKey()
                * \brief if the user have a public key for aes encryption.
                * \return if the user have a public key for aes encryption.
                */
                bool hPbKey();
                bool hPbIv ();
                /**
                * \fn bool hPbKeyRsa()
                * \brief if the user have a public key for rsa encryption.
                * \return if the user have a public key for rsa encryption.
                */
                bool hPbKeyRsa();
                /**
                * \fn void setHasPbKey(bool b)
                * \brief if the client have the public key. (for aes encryption)
                * \param b : the client has the public key. (for aes encryption)
                */
                void setHasPbKey(bool b);
                void setHasPbIv(bool b);
                /**
                * \fn void setHasPbKey(bool b)
                * \brief if the client have the public key. (for rsa encryption)
                * \param b : the client has the public key. (for rsa encryption)
                */
                void setHasPbKeyRsa(bool b);
                /**
                * \fn void addPingTime(std::uint64_t ping);
                * \brief set the time between two ping requests.
                * \param the time between two ping requests.
                */
                void addPingTime(std::int64_t ping);
                /**
                * \fn void setNbPingsToStore(unsigned int nbPings)
                * \brief set the number of pings to store.
                * \param the number of pings to store to compute the ping average.
                */
                void setNbPingsToStore(unsigned int nbPings);
                /**
                * \fn unsigned int getNbPingsToStore()
                * \brief get the number of ping stored to make the average.
                */
                unsigned int getNbPingsToStore();
                /**
                * \fn std::uint64_t getPingAvg()
                * \brief get the average of the ping of each user.
                * \return the average of each ping.
                */
                std::int64_t getPingAvg();
                /**
                * \fn bool isUsingSecuredConnexion()
                * \brief if the user is using an SSL connection.
                * \return if the user is using an SSL connection.
                */
                bool isUsingSecuredConnexion();
                /**
                * \fn sf::IpAddress getIpAddress ()
                * \brief get the ip address of the user.
                * \return the ip address of the user.
                */
                IpAddress getIpAddress ();
                /**
                * \fn sf::TcpSocket& getTcpSocket()
                * \brief get the tcp socket of the user.
                * \return the tcp socket of the user.
                */
                TcpSocket& getTcpSocket();
                 /**
                * \fn sf::UdpSocket& getUdpSocket()
                * \brief get the udp socket of the user.
                * \return the udp socket of the user.
                */
                UdpSocket& getUdpSocket();
                /**
                * \fn core::Clock& getPingClock()
                * \brief get clock with measure the ping.
                * \return the clock.
                */
                core::Clock& getPingClock();
                void setLastSrvTime(std::int64_t time);
                std::int64_t getLastSrvTime ();
                void setClientTime(std::int64_t time);
                std::int64_t getClientTime ();
                std::string getCertificate();
                void setCertificate(std::string certificate);
                void setCertifiate(bool b);
                bool isCertifiate();
                void setCliPbKeyReceived(bool b);
                bool isCliPbKeyReceived();
                /**
                * \fn ~User ()
                * \brief destructor.
                */
                virtual ~User ();
            private :
                bool useSecuredConnexion; /**> Use an SSL connection ?*/
                TcpSocket &clientTCP; /**> The tcp socket.*/
                UdpSocket &clientUDP; /**> The udp socket.*/
                IpAddress address; /**> The ip address of the user. */
                short unsigned int remotePortUDP; /**> the remote upd port.*/
                bool hasPbKey, hasPbIv, hasPbKeyRsa, certifiate, cliPbKeyReceived; /**> Have a public key for the rsa and for the aes encryption . */
                std::vector<std::int64_t> pings; /**> The n last pings stored to make the average.*/
                std::uint64_t pingAvg; /** >The average of n pings.*/
                unsigned int nbPings; /**> the number of pings to store.*/
                core::Clock pingClock; /**> the clock which measure pings.*/
                std::condition_variable g_signal; /**> the condition variable used to wait until the remote udp port
                * is given before sending a message.*/
                //std::rec_mutex g_lock_send; /**> the rec_mutex used to lock the condition variable on.*/
                std::uint64_t lastSrvTime;
                std::uint64_t clientTime;
                core::Clock elapsedTime;
                std::string certificate;
        };
    }
}
#endif
