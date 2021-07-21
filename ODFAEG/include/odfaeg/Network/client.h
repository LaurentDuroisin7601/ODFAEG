#ifndef CLIENT
#define CLIENT
#include <SFML/Network.hpp>
#include <iostream>
#include "encryptedPacket.h"
#include "symEncPacket.h"
#include "../Core/utilities.h"
#include "../Core/singleton.h"
#include <thread>
#include "export.hpp"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace network {
        /**
          * \file client.h
          * \class SrkClient
          * \brief A client which can be connected to a server of an ODFAEG application.
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          *
          */
        class Network;
        class ODFAEG_NETWORK_API SrkClient {
            public :
                /**
                * \fn SrkClient ()
                * \brief constructor.
                */
                SrkClient ();
                /**
                * \fn bool isRunning()
                * \brief if the client is running.
                * \return if the client is running.
                */
                bool isRunning ();
                /**
                * \fn bool startCli(int portTCP, int portUDP, sf::IpAddress adress, bool useThread = false, bool useSecuredConnexion = false)
                * \brief try to connect to the server with the given ports and ipaddress and start the client. (return false if it fails)
                * \param portTCP : the port's number used to connect to the server for the TCP protocol.
                * \param portUDP : the port's number used to connect to the server for the UDP protocol.
                * \param useThread : use a thread to add the received messages to the queue.
                * \param useSecuredConnexion : tell if the client must use an SSL connection or not. (For the TCP protocol)
                * \return if the client is connected.
                */
                bool startCli(int portTCP, int portUDP, sf::IpAddress adress, bool useSecuredConnexion = true);
                /**
                * \fn void sendTCPPacket(sf::Packet& packet)
                * \brief send a packet with the tcp protocol.
                * \param packet : the packet to send.
                */
                void sendTCPPacket(sf::Packet &packet);
                /**
                * \fn void sendTCPPacket(sf::Packet& packet)
                * \brief send a packet with the udp protocol.
                * \param packet : the packet to send.
                */
                void sendUDPPacket(sf::Packet &packet);
                /**
                * \fn void sendUDPPacket(sf::Packet& packet)
                * \brief send a packet with the udp protocol.
                * \param packet : the packet to send.
                */
                void stopCli();
                /**
                * \fn void checkMessages();
                * \brief check if messages are received and add them to the queue. (For non-threaded version)
                */
                void checkMessages();
            private :
                /**
                * \fn void getPublicKey()
                * \brief ask the public keys to the server. (For SSL connections)
                */
                void getPublicKey();
                bool running; /**> if the client is running, and if the client is using a thread.*/
                sf::TcpSocket clientTCP; /**>The tcp socket of the client.*/
                sf::UdpSocket clientUDP; /**>The udp socket of the client.*/
                sf::SocketSelector selector; /**>The selector.*/
                short unsigned int remotePortUDP; /**>the remote udp port.*/
                sf::IpAddress srvAddress; /**> the ip address of the server.*/
                bool useSecuredConnexion; /**>if the client is using an SSL connection.*/
        };
    }
}
#endif
