#ifndef ODFAEG_USER_HPP
#define ODFAEG_USER_HPP
#include <SFML/Network.hpp>
#include "../Core/utilities.h"
#include "export.hpp"
#include <mutex>
#include <condition_variable>
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
                User(sf::TcpSocket &socketTCP, sf::UdpSocket &socketUDP);
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
                * \fn void sendTcpPacket(sf::Packet &packet)
                * \brief send a sf::Packet to the client using the tcp protocol.
                */
                void sendTcpPacket(sf::Packet &packet);
                /**
                * \fn void sendUdpPacket(sf::Packet &packet)
                * \brief send a sf::Packet to the client using the udp protocol.
                */
                void sendUdpPacket(sf::Packet &packet);
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
                * \fn void addPingTime(sf::Int64 ping);
                * \brief set the time between two ping requests.
                * \param the time between two ping requests.
                */
                void addPingTime(sf::Int64 ping);
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
                * \fn sf::Int64 getPingAvg()
                * \brief get the average of the ping of each user.
                * \return the average of each ping.
                */
                sf::Int64 getPingAvg();
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
                sf::IpAddress getIpAddress ();
                /**
                * \fn sf::TcpSocket& getTcpSocket()
                * \brief get the tcp socket of the user.
                * \return the tcp socket of the user.
                */
                sf::TcpSocket& getTcpSocket();
                 /**
                * \fn sf::UdpSocket& getUdpSocket()
                * \brief get the udp socket of the user.
                * \return the udp socket of the user.
                */
                sf::UdpSocket& getUdpSocket();
                /**
                * \fn sf::Clock& getPingClock()
                * \brief get clock with measure the ping.
                * \return the clock.
                */
                sf::Clock& getPingClock();
                void setLastSrvTime(sf::Int64 time);
                sf::Int64 getLastSrvTime ();
                void setClientTime(sf::Int64 time);
                sf::Int64 getClientTime ();
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
                sf::TcpSocket &clientTCP; /**> The tcp socket.*/
                sf::UdpSocket &clientUDP; /**> The udp socket.*/
                sf::IpAddress address; /**> The ip address of the user. */
                short unsigned int remotePortUDP; /**> the remote upd port.*/
                bool hasPbKey, hasPbIv, hasPbKeyRsa, certifiate, cliPbKeyReceived; /**> Have a public key for the rsa and for the aes encryption . */
                std::vector<sf::Int64> pings; /**> The n last pings stored to make the average.*/
                sf::Int64 pingAvg; /** >The average of n pings.*/
                unsigned int nbPings; /**> the number of pings to store.*/
                sf::Clock pingClock; /**> the clock which measure pings.*/
                std::condition_variable g_signal; /**> the condition variable used to wait until the remote udp port
                * is given before sending a message.*/
                std::mutex g_lock_send; /**> the mutex used to lock the condition variable on.*/
                sf::Int64 lastSrvTime;
                sf::Int64 clientTime;
                sf::Clock elapsedTime;
                std::string certificate;
        };
    }
}
#endif
