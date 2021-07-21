#include "../../../include/odfaeg/Network/network.h"
#include "../../../include/odfaeg/Graphics/application.h"
namespace odfaeg {
    namespace network {
        using namespace sf;
        using namespace std;
        SrkClient& Network::cli = Network::getCliInstance();
        SrkServer& Network::srv = Network::getSrvInstance();
        Int64 Network::timeBtw2Pings = sf::seconds(1.f).asMicroseconds();
        Int64 Network::timeBtw2Sync = sf::seconds(1.f).asMicroseconds();
        Clock Network::timeBtw2SyncClk = Clock();
        Clock Network::timeBtw2PingsClk = sf::Clock();
        Clock Network::timeoutClk = Clock();
        std::vector<std::pair<User*, std::string>> Network::requests = std::vector<std::pair<User*, std::string>>();
        std::vector<User*> Network::users = std::vector<User*>();
        vector<string> Network::responses = vector<string>();
        std::string Network::certifiateClientMess = std::string();
        bool Network::isServer = false;
        bool Network::startCli (int portTCP, int portUDP, IpAddress address, bool useSecuredConnexion) {

            if (srv.isRunning()) {
                return false;
            }
            isServer = false;
            return cli.startCli(portTCP, portUDP, address, useSecuredConnexion);
        }
        bool Network::startSrv (int portTCP, int portUDP) {
            if (cli.isRunning()) {
                return false;
            }
            isServer = true;
            return srv.startSrv(portTCP, portUDP);
        }
        void Network::addRequest(User* user, std::string request) {
            std::pair<User*, std::string> userreq(user, request);
            requests.push_back(userreq);
        }

        void Network::stopCli () {
            cli.stopCli ();
        }
        void Network::addResponse(string response) {
            responses.push_back(response);
        }
        void Network::sendTcpPacket (Packet &packet) {
            if(cli.isRunning()) {
               cli.sendTCPPacket(packet);
            } else if (srv.isRunning()) {
               for (unsigned int i = 0; i < users.size(); i++) {
                    users[i]->sendTcpPacket(packet);
               }
            }
        }
        void Network::sendUdpPacket(Packet &packet) {
           if(cli.isRunning()) {
               cli.sendUDPPacket(packet);
           } else if (srv.isRunning()) {
               for (unsigned int i = 0; i < users.size(); i++) {
                    users[i]->sendUdpPacket(packet);
               }
           }
        }
        bool Network::getResponse(string tag, string &response) {
            vector<string>::iterator it;
            for (it = responses.begin(); it != responses.end();it++) {
                if (it->find(tag) != string::npos) {
                    response = it->substr(tag.length(), it->length() - tag.length());
                    it = responses.erase(it);
                    return true;
                }
            }
            return false;
        }
        std::string Network::getLastRequest(User** user, sf::Time timeOut) {
            std::string request="";
            timeoutClk.restart();
            std::vector<std::pair<User*, std::string>>::iterator it;
            //std::cout<<"get last request"<<std::endl;
            while (request == "" && timeoutClk.getElapsedTime().asSeconds() < timeOut.asSeconds()) {
                //std::cout<<"check messages"<<std::endl;
                srv.checkMessages();
                if (requests.size() > 0) {
                    it = requests.begin();
                    request = it->second;
                    *user = it->first;
                    it = requests.erase(it);
                }
            }
            return request;
        }
        bool Network::hasPbKey(TcpSocket& socket) {
            for (unsigned int i = 0; i < users.size(); i++) {
                if (&users[i]->getTcpSocket() == &socket) {
                    return users[i]->hPbKey();
                }
            }
            return false;
        }
        bool Network::hasPbIv(TcpSocket& socket) {
            for (unsigned int i = 0; i < users.size(); i++) {
                if (&users[i]->getTcpSocket() == &socket) {
                    return users[i]->hPbIv();
                }
            }
            return false;
        }
        bool Network::hasPbKeyRsa(TcpSocket& socket) {
            for (unsigned int i = 0; i < users.size(); i++) {
                if (&users[i]->getTcpSocket() == &socket) {
                    return users[i]->hPbKeyRsa();
                }
            }
            return false;
        }
        bool Network::isAuthenticClient(TcpSocket& socket) {
            for (unsigned int i = 0; i < users.size(); i++) {
                if (&users[i]->getTcpSocket() == &socket) {
                    return users[i]->isCertifiate();
                }
            }
            return false;
        }
        bool Network::isCliPbKeyReceived(TcpSocket& socket) {
            for (unsigned int i = 0; i < users.size(); i++) {
                if (&users[i]->getTcpSocket() == &socket) {
                    return users[i]->isCliPbKeyReceived();
                }
            }
            return false;
        }
        bool Network::hasResponse() {
            return responses.size() != 0;
        }
        bool Network::hasRequest() {
            return requests.size() != 0;
        }
        string Network::getLastResponse (sf::Time timeOut) {
            string response = "";
            timeoutClk.restart();
            vector<string>::iterator it;
            while (response == "" && timeoutClk.getElapsedTime().asSeconds() < timeOut.asSeconds()) {
                cli.checkMessages();
                if (responses.size() > 0) {
                    it = responses.begin();
                    response = *it;
                    it = responses.erase(it);
                }
            }
            return response;
        }

        void Network::addUser(TcpSocket& tcpSocket, UdpSocket& udpSocket) {
            User* user = new User(tcpSocket, udpSocket);
            users.push_back(user);
        }
        void Network::removeUser(sf::TcpSocket& socket) {
            std::vector<User*>::iterator it;
            for (it = users.begin(); it != users.end();) {
                if (&(*it)->getTcpSocket() == &socket) {
                    if (core::Application::app != nullptr)
                        core::Application::app->onDisconnected(*it);
                    it = users.erase(it);
                } else
                    it++;
            }
        }
        void Network::sendPbKeyRsa(User &user) {
            unsigned char* out = nullptr;
            int length = EncryptedPacket::getCertificate(&out);
            std::string response (reinterpret_cast<char*>(out), length);
            response.insert(0, "RSAKEY");
            Packet packet;
            packet<<response;
            user.getTcpSocket().send(packet);
        }
        void Network::sendPbKey(User &user) {
            string response = string(SymEncPacket::getKey(), strlen(SymEncPacket::getKey()));
            response.insert(0, "AESKEY");
            EncryptedPacket packet;
            packet<<response;
            user.getTcpSocket().send(packet);
        }
        void Network::sendPbIv(User &user) {
            string response = string(SymEncPacket::getIv(), strlen(SymEncPacket::getIv()));
            response.insert(0, "AESIV");
            EncryptedPacket packet;
            packet<<response;
            user.getTcpSocket().send(packet);
        }
        void Network::sendCertifiateClient(User &user) {
            CliEncryptedPacket cliEncryptedPacket;
            std::string response = "GETCERTIFIATECLIENT";
            cliEncryptedPacket<<response;
            user.getTcpSocket().send(cliEncryptedPacket);
        }
        void Network::sendClientCertifiate(User &user) {
            CliEncryptedPacket cliEncryptedPacket;
            std::string response = "CERTIFIEDCLIENT";
            cliEncryptedPacket<<response;
            user.getTcpSocket().send(cliEncryptedPacket);
        }
        User* Network::getUser(sf::TcpSocket& socket) {
            for (unsigned int i = 0; i < users.size(); i++) {
                if (&users[i]->getTcpSocket() == &socket) {
                    return users[i];
                }
            }
            return nullptr;
        }
        User* Network::getUser(IpAddress address, short unsigned int remoteUDPPort) {
            for (unsigned int i = 0; i < users.size(); i++) {
                if (users[i]->getIpAddress() == address && users[i]->getRemotePortUDP() == remoteUDPPort) {
                    return users[i];
                }
            }
            return nullptr;
        }
        std::vector<User*> Network::getUsers () {
            return users;
        }
        string Network::waitForLastResponse(string tag, sf::Time timeOut) {
            string response = "";
            timeoutClk.restart();
            vector<string>::iterator it;
            while (response == "" && timeoutClk.getElapsedTime().asSeconds() <= timeOut.asSeconds()) {
                cli.checkMessages();
                if (responses.size() > 0) {
                        if(tag == "") {
                            it = responses.begin();
                            response = *it;
                            it = responses.erase(it);
                            return response;
                        } else if (getResponse(tag, response)) {
                            return response;
                        }
                }
            }
            return response;
        }
        void Network::setPbKey (string message) {
            EncryptedPacket::setCertificate(reinterpret_cast<const unsigned char*>(message.c_str()), message.size());
        }
        void Network::setCliPbKey(std::string message) {
            CliEncryptedPacket::setCertificate(reinterpret_cast<const unsigned char*>(message.c_str()), message.size());
        }
        void Network::setSymPbKey (string pbKey) {
            SymEncPacket::setKey(const_cast<char*>(pbKey.c_str()));
            //SymEncPacket::updateKeys(pbKey);
        }
        void Network::setSymPbIv (string iv) {
            SymEncPacket::setIv(const_cast<char*>(iv.c_str()));
        }
        void Network::setCertifiateClientMess(std::string mess) {
            certifiateClientMess = mess;
        }
        bool Network::simpleSHA256(unsigned char* input, unsigned long length, unsigned char* md)
        {
            SHA256_CTX context;
            if(!SHA256_Init(&context))
                return false;

            if(!SHA256_Update(&context, input, length))
                return false;

            if(!SHA256_Final(md, &context))
                return false;
            return true;
        }
        std::string Network::getCertifiateClientMess() {
            return certifiateClientMess;
        }
    }
}
