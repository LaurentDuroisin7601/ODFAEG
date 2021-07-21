#ifndef SRKSERVEUR_H
#define SRKSERVEUR_H
#include<SFML/Network.hpp>
#include <thread>
#include <vector>
#include "encryptedPacket.h"
#include "cliEncryptedPacket.hpp"
#include "export.hpp"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace network {
        class Network;
        class ODFAEG_NETWORK_API SrkServer
        {
            /**
            * \file srkserver.h
            * \class SrkServer
            * \brief A server of an ODFAEG application.
            * \author Duroisin.L
            * \version 1.0
            * \date 1/02/2014
            *
            */
            public :
                /**
                * \fn SrkServer()
                * \brief constructor.
                */
                SrkServer();
                /**
                * \fn sf::TcpListener& getListner() const
                * \brief get the listener used to accept connections. (for the tcp protocol)
                * \return the listener.
                */
                const sf::TcpListener& getListner() const;
                /**
                * \fn  bool isRunning();
                * \brief if the server is running.
                * \return if the server is running.
                */
                bool isRunning();
                /**
                * \fn bool startSrv(int portTCP, int portUDP, bool useThread = false)
                * \brief start the server.
                * \param portTCP : the tcp port of the server.
                * \param portUDP : the udp port of the server.
                * \param useThread : use a thread to get the received messages ?
                */
                bool startSrv(int portTCP, int portUDP);
                /**
                * \fn void stopSrv()
                * \brief stop the server.
                */
                void stopSrv();
                /**
                * \fn void checkMessages()
                * \brief check if messages are received and store them into the queue.
                */
                void checkMessages();
                std::string getCertificate();
                /**
                * \fn ~SrkServer()
                * \brief destructor.
                */
                virtual ~SrkServer();
            private :
                /**
                * \fn void run ()
                * \brief method used by the thread to get messages.
                */
                bool running; /**> if the thread is running, and if we use a thread.*/
                /**
                * \fn void removeClient (sf::TcpSocket* socket);
                * \brief remove a client from the server.
                * \param socket : the socket of the client to remove.
                */
                void removeClient (sf::TcpSocket* socket);
                std::vector<sf::TcpSocket*> clients; /**> The list of the clients connected to the server.*/
                sf::SocketSelector selector; /**> The selector.*/
                sf::TcpListener  listener; /**> The listener used to accept new connections.*/
                sf::UdpSocket udpSocket; /**> The udp socket.*/
        };
    }
}
#endif // SRKSERVEUR_H
