#include "../../../include/odfaeg/Network/srkserveur.h"

#include "srkserveur.h"
namespace odfaeg {
    namespace network {
        using namespace sf;
        using namespace std;

        SrkServer::SrkServer()
        {
            running = false;
        }
        bool SrkServer::startSrv(int portTCP, int portUDP) {
            if (!running) {
                if (!udpSocket.bind(portUDP) == Socket::Done) {
                    cerr<<"Impossible d'écouter sur ce port : "<<portUDP<<endl;
                    return false;
                } else {
                    selector.add(udpSocket);
                }
                if (!listener.listen(portTCP) == Socket::Done) {
                    cerr<<"Impossible d'écouter sur ce port : "<<portTCP<<endl;
                    return false;
                } else {
                    cout<<"Server started!"<<endl;
                    selector.add(listener);
                    running = true;
                    CliEncryptedPacket::setServerRunning(running);
                    EncryptedPacket::setServerRunning(running);
                    return true;
                }

            } else {
                cout<<"Server already started!"<<endl;
                return false;
            }

        }
        void SrkServer::stopSrv () {
            if (running) {
                running = false;
            } else {
                cout<<"Server already stopped!"<<endl;
            }
        }
        bool SrkServer::isRunning () {
            return running;
        }

        void SrkServer::removeClient(TcpSocket* client) {
            vector<TcpSocket*>::iterator it;
            for (it = clients.begin(); it != clients.end();) {
                if (*it == client)
                    it = clients.erase(it);
                else
                    it++;
            }
            delete client;
        }

        const TcpListener& SrkServer::getListner () const {
            return listener;
        }

        SrkServer::~SrkServer()
        {

            for (unsigned int i = 0; i < clients.size(); i++) {
                selector.remove(*clients[i]);
                delete clients[i];
            }
            listener.close();
            //dtor
        }
    }
}



