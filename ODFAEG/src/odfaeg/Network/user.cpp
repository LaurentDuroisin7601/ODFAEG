#include "../../../include/odfaeg/Network/user.h"
#include "../../../include/odfaeg/Core/singleton.h"
#include "../../../include/odfaeg/Network/network.h"
#include <iostream>
namespace odfaeg {
    namespace network {
        using namespace std;
        using namespace sf;
        User::User (TcpSocket &socketTCP, UdpSocket &socketUDP) : clientTCP(socketTCP), clientUDP(socketUDP) {
            hasPbKey = hasPbKeyRsa = certifiate = cliPbKeyReceived = hasPbIv = false;
            address = socketTCP.getRemoteAddress();
            remotePortUDP = 0;
            useSecuredConnexion = false;
            nbPings = 10;
        }
        void User::addPingTime(sf::Int64 time) {
            std::vector<sf::Int64>::iterator it;
            if (pings.size() > nbPings)  {
                it = pings.erase(pings.begin());
            }
            pings.push_back(time);
            pingAvg = 0;
            for (unsigned int i = 0; i < pings.size(); i++)
                pingAvg += pings[i];
            pingAvg /= pings.size();
        }
        void User::setHasPbIv(bool iv) {
            hasPbIv = iv;
        }
        bool User::hPbIv() {
            return hasPbIv;
        }
        Clock& User::getPingClock() {
            return pingClock;
        }
        void User::setNbPingsToStore(unsigned int nbPings) {
            this->nbPings = nbPings;
        }
        sf::Int64 User::getPingAvg() {
            return pingAvg;
        }
        unsigned int User::getNbPingsToStore() {
            return nbPings;
        }
        bool User::hPbKey () {
            return hasPbKey;
        }
        bool User::hPbKeyRsa () {
            return hasPbKeyRsa;
        }
        void User::setHasPbKeyRsa (bool hasPbKeyRsa) {
            this->hasPbKeyRsa = hasPbKeyRsa;
        }
        void User::setHasPbKey(bool hasPbKey) {
            this->hasPbKey = hasPbKey;
        }
        void User::setRemotePortUDP(short unsigned int port) {
            remotePortUDP = port;
        }
        short unsigned int User::getRemotePortUDP() {
            return remotePortUDP;
        }
        IpAddress User::getIpAddress() {
            return address;
        }
        TcpSocket& User::getTcpSocket() {
            return clientTCP;
        }
        UdpSocket& User::getUdpSocket() {
            return clientUDP;
        }
        void User::sendTcpPacket(Packet &packet) {
            if(clientTCP.send(packet) != Socket::Done) {
                std::string message;
                packet>>message;
                cerr<<"Error : can't send this message : "<<message<<" !"<<endl;
            }
        }
        void User::sendUdpPacket(Packet &packet) {
            if (clientUDP.send(packet, address, remotePortUDP) != Socket::Done) {
                std::string message;
                packet>>message;
                cerr<<"Error : can't send this message : "<<message<<" !"<<endl;
            }
        }
        void User::setUseSecuredConnexion(bool b) {
            useSecuredConnexion = b;
        }
        bool User::isUsingSecuredConnexion() {
            return useSecuredConnexion;
        }
        void User::setLastSrvTime(sf::Int64 time) {
            lastSrvTime = time;
        }
        sf::Int64 User::getLastSrvTime () {
            return lastSrvTime;
        }
        void User::setClientTime(sf::Int64 time) {
            elapsedTime.restart();
            clientTime = time;
        }
        sf::Int64 User::getClientTime () {
            return clientTime + elapsedTime.getElapsedTime().asMicroseconds();
        }
        void User::setCertificate(std::string certificate) {
            this->certificate = certificate;
        }
        std::string User::getCertificate() {
            return certificate;
        }
        void User::setCertifiate(bool b) {
            certifiate = b;
        }
        bool User::isCertifiate() {
            return certifiate;
        }
        void User::setCliPbKeyReceived(bool b) {
            cliPbKeyReceived = b;
        }
        bool User::isCliPbKeyReceived() {
            return cliPbKeyReceived;
        }
        User::~User () {

        }
    }
}
