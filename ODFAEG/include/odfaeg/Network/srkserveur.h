#ifndef SRKSERVEUR_H
#define SRKSERVEUR_H
#include<SFML/Network.hpp>
#include <thread>
#include <vector>
#include "encryptedPacket.h"
#include "cliEncryptedPacket.hpp"
#include "export.hpp"
#include "network.h"

/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace network {

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
                template<typename A, typename T>
                void checkMessages() {
                    //std::cout<<"check messages"<<std::endl;
                    if (running) {
                        std::vector<sf::TcpSocket*>::iterator it;
                        if (Network::getTimeBtw2PingsClk().getElapsedTime().asMicroseconds() >= Network::getTimeBtw2Pings()) {
                            for (it = clients.begin(); it != clients.end();it++) {
                                sf::TcpSocket& client = **it;
                                User* user = Network::getUser(client);
                                if (user != nullptr && user->getRemotePortUDP()) {
                                    sf::Packet packet;
                                    packet<<"PING";
                                    user->getPingClock().restart();
                                    user->sendUdpPacket(packet);
                                }
                            }
                            Network::getTimeBtw2PingsClk().restart();
                        }
                        if (Network::getTimeBtw2SyncClk().getElapsedTime().asMicroseconds() >= Network::getTimeBtw2Sync()) {
                            for (it = clients.begin(); it != clients.end();it++) {
                                sf::TcpSocket& client = **it;
                                User* user = Network::getUser(client);
                                if (user != nullptr && user->getRemotePortUDP()) {
                                    sf::Packet packet;
                                    sf::Int64 lastSrvTime = clock.getElapsedTime().asMicroseconds();
                                    packet<<"GET_TIME*"+core::conversionLongString(lastSrvTime);
                                    user->setLastSrvTime(lastSrvTime);
                                    user->sendUdpPacket(packet);
                                }
                            }
                            Network::getTimeBtw2SyncClk().restart();
                        }
                        if (selector.wait(sf::milliseconds(10))) {
                            if (selector.isReady(listener)) {
                                sf::TcpSocket *client = new sf::TcpSocket();
                                if (listener.accept(*client) == sf::Socket::Done) {
                                    std::cout<<"client connected!"<<std::endl;
                                    selector.add(*client);
                                    clients.push_back(client);
                                    Network::addUser(*client, udpSocket);
                                } else {
                                    delete client;
                                }
                            }
                            for (it = clients.begin(); it != clients.end();it++) {

                                sf::TcpSocket& client = **it;
                                if (selector.isReady(client)) {
                                    //std::cout<<"ready"<<std::endl;
                                    User* user = Network::getUser(client);
                                    bool pbKeyRsaSend = user->hPbKeyRsa();
                                    bool pbKeySend = user->hPbKey();
                                    bool pbIvSend = user->hPbIv();
                                    bool authentic = user->isCertifiate();
                                    bool cliPbKeyReceived = user->isCliPbKeyReceived();
                                    //std::cout<<"user get"<<std::endl;
                                    if (!authentic && !cliPbKeyReceived && !pbKeyRsaSend && !pbKeySend && !pbIvSend && user != nullptr &&
                                        (!user->getRemotePortUDP() || !user->isUsingSecuredConnexion())) {
                                        sf::Packet packet;
                                        if (client.receive(packet) == sf::Socket::Done) {
                                            //std::cout<<"packet received"<<std::endl;
                                            std::string request;
                                            packet>>request;
                                            if (request.find("SETCLIPBKEY") != std::string::npos) {
                                                request.erase(0, 11);
                                                Network::setCliPbKey(request);
                                                Network::sendCertifiateClient(*user);
                                                user->setCliPbKeyReceived(true);
                                                user->setUseSecuredConnexion(true);
                                            } else  if (request.find("updateUdpPort") != std::string::npos) {
                                                std::vector<std::string> parts = core::split(request, "*");
                                                user->setRemotePortUDP(core::conversionStringInt(parts[1]));
                                            } else {
                                                Network::addRequest (user, request);
                                            }
                                            //std::cout<<"packet send"<<std::endl;
                                        } else {
                                            Network::removeUser<A, T>(client);
                                            selector.remove(client);
                                            it = clients.erase(it);
                                            delete *it;
                                            it--;
                                        }
                                    }
                                    if (!authentic && cliPbKeyReceived && !pbKeyRsaSend && !pbKeySend && !pbIvSend && user != nullptr && user->getRemotePortUDP() && user->isUsingSecuredConnexion()) {
                                        CliEncryptedPacket packet;
                                        if (client.receive(packet) == sf::Socket::Done) {
                                            //std::cout<<"packet received send client certifiate message"<<std::endl;
                                            std::string request;
                                            packet>>request;
                                            if (request == Network::getCertifiateClientMess()) {
                                                Network::sendClientCertifiate(*user);
                                                user->setCertifiate(true);
                                            } else {
                                                Network::removeUser<A, T>(client);
                                                selector.remove(client);
                                                it = clients.erase(it);
                                                delete *it;
                                                it--;
                                            }
                                            //std::cout<<"packet send client certifiate message send"<<std::endl;
                                        }
                                    }
                                    if (authentic && cliPbKeyReceived && !pbKeyRsaSend && !pbKeySend && !pbIvSend &&  user != nullptr && user->getRemotePortUDP() && user->isUsingSecuredConnexion()) {
                                        CliEncryptedPacket cliEncryptedPacket;
                                        if (client.receive(cliEncryptedPacket) == sf::Socket::Done) {
                                            //std::cout<<"packet received send pb key rsa"<<std::endl;
                                            std::string request;
                                            cliEncryptedPacket>>request;
                                            if (request == "GetPbKeyRsa") {
                                                Network::sendPbKeyRsa(*user);
                                                user->setHasPbKeyRsa(true);
                                            } else  if (request.find("updateUdpPort") != std::string::npos) {
                                                std::vector<std::string> parts = core::split(request, "*");
                                                user->setRemotePortUDP(core::conversionStringInt(parts[1]));
                                            } else {
                                                Network::addRequest (user, request);
                                            }
                                            //std::cout<<"packet send pb key rsa"<<std::endl;
                                        } else {
                                            Network::removeUser<A, T>(client);
                                            selector.remove(client);
                                            it = clients.erase(it);
                                            delete *it;
                                            it--;
                                        }
                                    }
                                    if (authentic && cliPbKeyReceived && pbKeyRsaSend && !pbKeySend && !pbIvSend && user != nullptr && user->getRemotePortUDP() && user->isUsingSecuredConnexion()) {
                                        EncryptedPacket packet;
                                        if (client.receive(packet) == sf::Socket::Done) {
                                            //std::cout<<"packet received set pb key"<<std::endl;
                                            std::string request;
                                            packet>>request;
                                            if (request == "GetPbKey") {
                                                 Network::sendPbKey(*user);
                                                 user->setHasPbKey(true);
                                            } else {
                                                Network::removeUser<A, T>(client);
                                                selector.remove(client);
                                                it = clients.erase(it);
                                                delete *it;
                                                it--;
                                            }
                                            //std::cout<<"packet send set pb key"<<std::endl;
                                        }
                                    }
                                    if (authentic && cliPbKeyReceived && pbKeySend && pbKeyRsaSend && !pbIvSend && user != nullptr && user->getRemotePortUDP() && user->isUsingSecuredConnexion()) {
                                        EncryptedPacket packet;
                                        if (client.receive(packet) == sf::Socket::Done) {
                                            //std::cout<<"packet received set iv"<<std::endl;
                                            std::string request;
                                            packet>>request;
                                            if (request == "GetPbIv") {
                                                Network::sendPbIv(*user);
                                                user->setHasPbIv(true);
                                            } else {
                                                Network::removeUser<A, T>(client);
                                                selector.remove(client);
                                                it = clients.erase(it);
                                                delete *it;
                                                it--;
                                            }
                                            //std::cout<<"packet send set pb key iv"<<std::endl;
                                        }
                                        //std::cout<<"end pb key iv"<<std::endl;
                                    }
                                    if (authentic && cliPbKeyReceived && pbKeySend && pbIvSend && pbKeyRsaSend && user != nullptr && user->getRemotePortUDP() && user->isUsingSecuredConnexion()) {
                                        //std::cout<<"sym enc packet"<<std::endl;
                                        SymEncPacket packet;
                                        if (client.receive(packet) == sf::Socket::Done) {
                                            //std::cout<<"packet received sym enc packet"<<std::endl;
                                            std::string request;
                                            packet>>request;
                                            Network::addRequest (user, request);
                                            //std::cout<<"packet send sym enc packet"<<std::endl;
                                        } else {
                                            Network::removeUser<A, T>(client);
                                            selector.remove(client);
                                            it = clients.erase(it);
                                            delete *it;
                                            it--;
                                        }
                                        //std::cout<<"end sym enc packet"<<std::endl;
                                    }
                                }

                            }
                            //std::cout<<"end tcp"<<std::endl;
                            if (selector.isReady(udpSocket)) {
                                sf::Packet packet;
                                std::string request;
                                sf::IpAddress sender;
                                short unsigned int port;
                                if (udpSocket.receive(packet, sender, port) == sf::Socket::Done) {
                                    packet>>request;
                                    User* user = Network::getUser(sender, port);
                                    if (user != nullptr) {
                                        std::vector<std::string> infos = core::split(request, "*");
                                        if (infos[0] == "PONG") {
                                            user->addPingTime(user->getPingClock().getElapsedTime().asMicroseconds() * 0.5f);
                                        } else if (infos[0] == "SET_TIME") {
                                            sf::Int64 cliTime = core::conversionStringLong(infos[1]);
                                            sf::Int64 srvTime = clock.getElapsedTime().asMicroseconds();
                                            sf::Int64 syncTime = cliTime + (srvTime - user->getLastSrvTime()) * 0.5f;
                                            user->setClientTime(syncTime);
                                        } else {
                                            Network::addRequest(user, request);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    //std::cout<<"end check messages"<<std::endl;
                }
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
                sf::Clock clock;
        };
    }
}
#endif // SRKSERVEUR_H
