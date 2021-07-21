#include "../../../include/odfaeg/Network/client.h"
#include "../../../include/odfaeg/Network/network.h"
#include "../../../include/odfaeg/Network/cliEncryptedPacket.hpp"
#include "../../../include/odfaeg/Graphics/application.h"
namespace odfaeg {
    namespace network {
        using namespace sf;
        using namespace std;
        SrkClient::SrkClient () {
            running = false;
            remotePortUDP = 0;
        }
        void SrkClient::getPublicKey () {
            Packet packet;
            unsigned char* cliPbKey = nullptr;
            int size = CliEncryptedPacket::getCertificate(&cliPbKey);
            std::string message (reinterpret_cast<char*>(cliPbKey), size);
            message.insert(0, "SETCLIPBKEY");
            packet<<message;
            clientTCP.send(packet);
            bool certifiateMessageSend, pbKeyReceived, pbIvReceived, pbKeyRsaReceived, certifiate;
            certifiateMessageSend = pbKeyReceived = pbIvReceived = pbKeyRsaReceived = certifiate = false;
            EncryptedPacket enc_packet;
            std::cout<<"set cli pb key"<<std::endl;
            while (!certifiate || !certifiateMessageSend || !pbKeyReceived || !pbIvReceived || !pbKeyRsaReceived) {
                if (!certifiate && !certifiateMessageSend && !pbKeyRsaReceived && !pbKeyReceived && !pbIvReceived) {
                    CliEncryptedPacket cliEncryptedPacket;
                    if (clientTCP.receive(cliEncryptedPacket) == Socket::Done) {
                        std::string response;
                        cliEncryptedPacket>>response;
                        cliEncryptedPacket.clear();
                        if (response == "GETCERTIFIATECLIENT") {
                            message = Network::getCertifiateClientMess();
                            cliEncryptedPacket<<message;
                            clientTCP.send(cliEncryptedPacket);
                            certifiateMessageSend = true;
                            std::cout<<"certifiate message send"<<std::endl;
                        }
                    }
                }
                if (!certifiate && certifiateMessageSend && !pbKeyRsaReceived && !pbKeyReceived && !pbIvReceived) {
                    CliEncryptedPacket cliEncryptedPacket;
                    if (clientTCP.receive(cliEncryptedPacket) == sf::Socket::Done) {
                        std::string response;
                        cliEncryptedPacket>>response;
                        cliEncryptedPacket.clear();
                        if (response == "CERTIFIEDCLIENT") {
                            message = "GetPbKeyRsa";
                            cliEncryptedPacket<<message;
                            clientTCP.send(cliEncryptedPacket);
                            certifiate = true;
                            std::cout<<"get pb key rsa send"<<std::endl;
                        }
                    }
                }
                if (certifiate && certifiateMessageSend && !pbKeyRsaReceived && !pbKeyReceived && !pbIvReceived) {
                    packet.clear();
                    if (clientTCP.receive(packet) == Socket::Done) {
                        std::string response;
                        packet>>response;
                        if (response.find("RSAKEY") != std::string::npos) {
                            response.erase(0, 6);
                            Network::setPbKey(response);
                            pbKeyRsaReceived = true;
                            message = "GetPbKey";
                            enc_packet<<message;
                            clientTCP.send(enc_packet);
                            enc_packet.clear();
                        }
                    }
                }
                if (certifiate && certifiateMessageSend &&  pbKeyRsaReceived && !pbKeyReceived && !pbIvReceived)  {
                    if (clientTCP.receive(enc_packet) == Socket::Done) {
                        std::string response;
                        enc_packet>>response;
                        if (response.find("AESKEY")!= std::string::npos) {
                            response.erase(0, 6);
                            Network::setSymPbKey(response);
                            pbKeyReceived = true;
                            enc_packet.clear();
                            message = "GetPbIv";
                            enc_packet<<message;
                            clientTCP.send(enc_packet);
                            enc_packet.clear();
                        }
                    }
                }
                if (certifiate && certifiateMessageSend && pbKeyRsaReceived && pbKeyReceived && !pbIvReceived) {
                    if (clientTCP.receive(enc_packet) == Socket::Done) {
                        string response;
                        enc_packet>>response;
                        if (response.find("AESIV")!= std::string::npos) {
                            response.erase(0, 5);
                            Network::setSymPbIv(message);
                            pbIvReceived = true;
                            enc_packet.clear();
                        }
                    }
                }
            }
        }
        bool SrkClient::startCli(int portTCP, int portUDP, IpAddress address, bool useSecuredConnexion) {
             if (!running) {
                remotePortUDP = portUDP;
                this->srvAddress = address;
                this->useSecuredConnexion = useSecuredConnexion;
                std::cout<<"connect to client"<<std::endl;
                if(clientTCP.connect(address, portTCP) != Socket::Done) {
                    cout<<"Erreur : impossible de se connecter au serveur!"<<endl;
                    return false;
                }
                std::cout<<"bind udp port"<<std::endl;
                if (clientUDP.bind(Socket::AnyPort) != Socket::Done) {
                    cout<<"Erreur : impossible d'écouter sur le port : "<<portUDP<<endl;
                    return false;
                }
                std::cout<<"send upd port message"<<std::endl;
                Packet packet;
                std::string message = "updateUdpPort*" + core::conversionIntString(clientUDP.getLocalPort());
                packet<<message;
                clientTCP.send(packet);
                std::cout<<"get public key"<<std::endl;
                if (useSecuredConnexion)
                    getPublicKey();
                std::cout<<"public key get"<<std::endl;
                selector.add(clientTCP);
                selector.add(clientUDP);
                running = true;
                std::cout<<"Client started!"<<std::endl;
                return true;
            } else {
                cout<<"Client already started!"<<endl;
                return false;
            }
        }
        void SrkClient::stopCli() {
            if(running) {
                running = false;
            } else {
                cout<<"Client already stopped"<<endl;
            }
        }
        void SrkClient::sendTCPPacket(Packet &packet) {
            //if (useThread)
                //lock_guard<recursive_mutex> locker(rec_mutex);
            if(clientTCP.send(packet) != Socket::Done)
                cout<<"Erreur!"<<endl;
        }
        void SrkClient::sendUDPPacket(Packet &packet) {
            //if (useThread)
                //lock_guard<recursive_mutex> locker(rec_mutex);
            if(clientUDP.send(packet, srvAddress, remotePortUDP) != Socket::Done)
                cout<<"Erreur!"<<endl;
        }
        bool SrkClient::isRunning () {
            return running;
        }
        void SrkClient::checkMessages() {
            if (running) {
                short unsigned int port;
                IpAddress address;
                if (selector.wait(microseconds(1))) {
                     if (selector.isReady(clientTCP)) {
                        if (useSecuredConnexion) {
                            SymEncPacket packet;
                            if(clientTCP.receive(packet) == Socket::Done) {
                                string message;
                                packet>>message;
                                Network::addResponse(message);

                            }
                        } else {
                            Packet packet;
                            if(clientTCP.receive(packet) == Socket::Done) {

                                string message;
                                packet>>message;
                                Network::addResponse(message);

                            }
                        }
                    }

                    if (selector.isReady(clientUDP)) {
                        Packet packet;
                        if (clientUDP.receive(packet, address, port) == Socket::Done) {
                            if (address == srvAddress) {
                                string message;
                                packet>>message;
                                if (message == "PING") {
                                    packet.clear();
                                    message = "PONG";
                                    packet<<message;
                                    Network::sendUdpPacket(packet);
                                } else if (message.find("GET_TIME") != std::string::npos) {
                                    packet.clear();
                                    sf::Int64 time = core::Application::getTimeClk().getElapsedTime().asMicroseconds();
                                    message = "SET_TIME*"+core::conversionLongString(time);
                                    packet<<message;
                                    Network::sendUdpPacket(packet);
                                } else {
                                    Network::addResponse(message);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
