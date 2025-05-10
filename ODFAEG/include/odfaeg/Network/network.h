#ifndef NETWORK
#define NETWORK
#include <SFML/Network.hpp>
#include "client.h"
#include "user.h"
#include "srkserveur.h"
#include "export.hpp"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace network {
        /**
          * \file network.h
          * \class Network
          * \brief All network utilities functions.
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          * The network can start a server or a client (not both) per application,
          * receive the messages of the clients (or the server) and store them into a queue.
          * this class can also send messages to a client (or to the server) or to each clients connected to a server.
          * messages can also have tags, which identify the messages if we need to wait until a message is received by example.
          * we can also set up a timeout when waiting after messages.
          */
        class ODFAEG_NETWORK_API Network {
            public :
                /**
                * \fn bool startCli (int portTCP, int portUDP, sf::IpAddress, bool useThread = false, bool useSecuredConnexion = true)
                * \brief start the client.
                * \param portTCP : the tcp port's number of the server.
                * \param portUDP : the udp port's number of the server.
                * \param ipAddress : the ip address of the server.
                * \param useThread : use a thread to get messages from the server ?
                * \param useSecurisedConnexion : use an SSL connexion. (For the tcp protocol)
                */
                static bool startCli (int portTCP, int portUDP, sf::IpAddress, bool useSecuredConnexion = true);
                /**
                * \fn bool startSrv (int portTCP, int portUDP, bool useThread = false)
                * \brief start the server.
                * \param portTCP : the tcp port of the server.
                * \param portUDP : the udp port of the server.
                * \param useThread : use a thread to get the messages from the clients.
                */
                static bool startSrv (int portTCP, int portUDP);
                /**
                * \fn void stopCli ()
                * \brief stop the client.
                */
                static void stopCli ();
                /**
                * \fn void addUser(sf::TcpSocket& tcpSocket, sf::UdpSocket& udpSocket)
                * \brief add a user to the network.
                * \param tcpSocket : the tcp socket to add.
                * \param udpSocket : the udp socket to add.
                */
                static void addUser(sf::TcpSocket& tcpSocket, sf::UdpSocket& udpSocket);
                /**
                * \fn User* getUser(sf::IpAddress address)
                * \brief get a user from the network with the given up address.
                * \param the ip address of the user.
                * \return a pointer to the user (null if the user isn't connected)
                */
                static User* getUser(sf::TcpSocket& socket);
                static User* getUser(sf::IpAddress address, short unsigned int remoteUDPPort);
                static std::vector<User*> getUsers();
                /**
                * \fn void removeUser(sf::IpAddress address)
                * \brief remove a user from the network.
                * \param address : the ip address of the user to remove.
                */
                static void removeUser(sf::TcpSocket& tcpSocket);
                /**
                * \fn void sendPbKeyRsa(User& user)
                * \brief send the public key to the user for the rsa encryption.
                * \param the user.
                */
                static void sendPbKeyRsa(User& user);
                /**
                * \fn void sendPbKeyRsa(User& user)
                * \brief send the public key to the user for the aes encryption.
                * \param the user.
                */
                static void sendPbKey(User& user);
                static void sendPbIv(User& user);
                /**
                * \fn void addRequest(User *user, std::string request)
                * \brief add a request to the queue.
                * \param user : the user which have send the request.
                * \param request : the request.
                */
                static void addRequest(User *user, std::string request);
                /**
                * \fn void addResponse(std::string response)
                * \brief add a response to the queue.
                * \param response : the response.
                */
                static void addResponse (std::string response);
                /**
                * \fn void sendTcpPacket (sf::Packet& packet)
                * \brief send a packet over the network (using the tcp protocol), if the application is a client,
                * the packet is send to the server, if the application is a server, the packet is send to each clients
                * connected to the server.
                * \param packet : the packet to send.
                */
                static void sendTcpPacket (sf::Packet& packet);
                /**
                * \fn void sendUdpPacket (sf::Packet& packet)
                * \brief send a packet over the network (using the udp protocol), if the application is a client,
                * the packet is send to the server, if the application is a server, the packet is send to each clients
                * connected to the server.
                * \param packet : the packet to send.
                */
                static void sendUdpPacket (sf::Packet& packet);
                /**
                * \fn bool hasResponse()
                * \brief if there is a response in the queue.
                * \return is there a response in the message's queue ?
                */
                static bool hasResponse();
                /**
                * \fn bool hasRequest()
                * \brief if there is a request in the queue.
                * \return is there a request in the message's queue ?
                */
                static bool hasRequest();
                /**
                * \fn std::string getLastResponse ()
                * \brief get the last response received by the server.
                */
                static std::string getLastResponse (sf::Time timeOut = sf::seconds(5.f));
                /**
                * \fn std::string getLastRequest ()
                * \brief get the last request received by a client.
                */
                static std::string getLastRequest (User **user = nullptr, sf::Time timeOut = sf::seconds(5.f));
                /**
                * \fn void setPbKey(std::string pbKey)
                * \brief set the public key for rsa encryption.
                * \param pbKey : the public key.
                */
                static void setPbKey(std::string pbKey);
                static void setCliPbKey(std::string cliPbKey);
                /**
                * \fn void setSymPbKey(std::string pbKey)
                * \brief set the public key for aes encryption.
                * \param pbKey : the public key.
                */
                static void setSymPbKey (std::string pbKey);
                static void setSymPbIv (std::string pbIv);
                static void sendCertifiateClient(User &user);
                static void sendClientCertifiate(User &user);
                /**
                * \fn bool hasPbKey(sf::IpAddress address)
                * \brief if the user have the public key. (for aes encryption)
                * \param address : the ip address of the user.
                * \return if the user have the public key. (for aes encryption)
                */
                static bool hasPbKey(sf::TcpSocket& socket);
                static bool hasPbIv(sf::TcpSocket& socket);
                /**
                * \fn bool hasPbKeyRsa(sf::IpAddress address)
                * \brief if the user have the public key. (for rsa encryption)
                * \param address : the ip address of the user.
                * \return if the user have the public key. (for aes encryption)
                */
                static bool hasPbKeyRsa(sf::TcpSocket& socket);
                static bool isCliPbKeyReceived(sf::TcpSocket& socket);
                static bool isAuthenticClient(sf::TcpSocket& socket);
                /**
                * \fn bool getResponse(std::string tag, std::string &response)
                * \brief get a response with the given tag.
                * \param tag : the tag.
                * \param response : the response.
                */
                static bool getResponse(std::string tag, std::string &response);
                /**
                * \fn std::string waitForLastResponse(std::string tag)
                * \brief wait for a response with the given tag.
                * \param tag : the tag.
                * \return response.
                */
                static std::string waitForLastResponse(std::string tag, sf::Time timeOut = sf::seconds(5.f));
                /**
                * \fn SrkClient& getCliInstance ()
                * \brief return an instance of the client.
                * \return an instance of the client.
                */
                static SrkClient& getCliInstance () {
                    if (&cli == nullptr) {
                        static SrkClient* client = new SrkClient();
                        return *client;
                    } else {
                        return cli;
                    }
                }
                /**
                * \fn void setTimeBtw2Pings(sf::Time time)
                * \brief set the time interval between two ping requests.
                * \param the time interval between two ping requests.
                */
                static void setTimeBtw2Pings(sf::Time time) {
                    timeBtw2Pings = time.asMicroseconds();
                }
                /**
                * \fn sf::Int64 getTimeBtw2Pings()
                * \brief get the time interval between two pings.
                * \return the time interval between two pings.
                */
                static sf::Int64 getTimeBtw2Pings() {
                    return timeBtw2Pings;
                }
                /**
                * \fn sf::Clock& getTimeBtw2PingsClk()
                * \brief get the clock measuring the time interval between two pings.
                * \return the clock measuring the time interval between two pings.
                */
                static sf::Clock& getTimeBtw2PingsClk() {
                    return timeBtw2PingsClk;
                }
                /**
                * \fn SrkServer& getSrvInstance ()
                * \brief return an instance of the server.
                * \return an instance of the server.
                */
                static SrkServer& getSrvInstance() {
                    if (&srv == nullptr) {
                        static SrkServer* server = new SrkServer();
                        return *server;
                    } else {
                        return srv;
                    }
                }
                static sf::Clock& getTimeBtw2SyncClk() {
                    return timeBtw2SyncClk;
                }
                static sf::Int64 getTimeBtw2Sync() {
                    return timeBtw2Sync;
                }
                static void setCertifiateClientMess(std::string mess);
                static std::string getCertifiateClientMess();
                static bool simpleSHA256(unsigned char* input, unsigned long length, unsigned char* md);
            private :
                static sf::Clock timeoutClk; /**>clock.*/
                static SrkClient &cli; /**>The client.*/
                static SrkServer &srv; /**>The server.*/
                static std::vector<std::string> responses; /**> responses.*/
                static std::vector<std::pair<User*, std::string>> requests; /**> requests.*/
                static std::vector<User*> users; /**> users.*/
                static sf::Int64 timeBtw2Pings; /**> time between two pings.*/
                static sf::Int64 timeBtw2Sync;
                static sf::Clock timeBtw2SyncClk;
                static sf::Clock timeBtw2PingsClk; /**> clock measuring the interval between two pings.*/
                static std::string certifiateClientMess;
                static bool isServer;
        };
    }
}
#endif
