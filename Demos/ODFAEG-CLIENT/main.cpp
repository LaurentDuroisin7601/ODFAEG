#include "application.h"
using namespace odfaeg::core;
using namespace odfaeg::graphic;
using namespace odfaeg::physic;
using namespace odfaeg::math;
using namespace odfaeg::network;
using namespace odfaeg::window;
using namespace sorrok;
using namespace std;
class Chien {
};
class Chat {
};
class Attaquer : public odfaeg::core::Visitor<> {
    public :
    void operator()(const Chat&, const Chat&, int i) const
    { std::cout <<  "griffe - griffe " <<i<<std::endl;}
    void operator()(const Chat&, const Chien&, int i) const
    { std::cout <<  "griffe - mord  " <<i<<std::endl;}
    void operator()(const Chien&, const Chat&, int i) const
    { std::cout <<  "mord - griffe " <<i<<std::endl;}
    void operator()(const Chien&, const Chien&, int i) const
    { std::cout << "mord - mord  " <<i<<std::endl;}
};
int main(int argv, char* argc[]) {
    /*Chien chien;
    Chat chat;
    std::vector<std::pair<odfaeg::core::Variant<Chien, Chat>,odfaeg::core::Variant<Chien, Chat>>> list;
    list.push_back(std::make_pair(chat, chat));
    list.push_back(std::make_pair(chat, chien));
    list.push_back(std::make_pair(chien, chat));
    list.push_back(std::make_pair(chien, chien));
    int nb = 10;
    for (unsigned int i = 0; i < list.size(); i++) {
      odfaeg::core::apply_nary_visitor(Attaquer(), list[i].first, list[i].second, nb);
    }
    return 0;*/
    /*bool b = true;
    bool b2;
    std::ostringstream oss;
    OTextArchive ota(oss);
    ota(b);
    std::istringstream iss;
    iss.str(oss.str());
    ITextArchive ita(iss);
    ita(b2);
    std::cout<<"b : "<<b2<<std::endl;
    return 0;*/
    /*RenderWindow window(sf::VideoMode(800, 600, 32), "Test ppll", sf::Style::Default, ContextSettings(0, 0, 8, 3, 0));
    Texture tex;
    tex.loadFromFile("tilesets/herbe.png");
    Tile t1 (nullptr, Vec3f(0, 0, 0),Vec3f(100, 50, 0),sf::IntRect(0, 0, 100, 50), sf::Color(255, 0, 0, 150));
    Tile t2 (nullptr, Vec3f(50, 0, 0),Vec3f(100, 50, 0),sf::IntRect(0, 0, 100, 50), sf::Color(0, 255, 0, 100));
    Tile t3 (nullptr, Vec3f(25, 25, 0),Vec3f(100, 50, 0),sf::IntRect(0, 0, 100, 50), sf::Color(0, 0, 255, 50));
    PerPixelLinkedListRenderComponent ppll(window, 0, "E_TILE", ContextSettings(24, 0, 8, 3, 0));
    std::vector<Entity*> entities = {&t1, &t2, &t3};
    ppll.loadEntitiesOnComponent(entities);
    ppll.drawNextFrame();
    while (window.isOpen()) {
        IEvent event;
        while(window.pollEvent(event)) {
            if (event.type == IEvent::WINDOW_EVENT && event.window.type == IEvent::WINDOW_EVENT_CLOSED) {
                window.close();
            }
        }
        window.clear();
        ppll.clear();
        window.draw(ppll);
        window.display();
    }*/

    MyAppli app(sf::VideoMode(800, 600, 32), "Test odfaeg");
    return app.exec();
    BoundingBox bx(264, 321, 21, 800, 600, 2800);
    BoundingBox bx2(275, 250, 300, 50, 100, 0);
    std::cout<<bx.intersects(bx2);

    /*TcpSocket clientTCP;
    bool running = false;
    if(clientTCP.connect(IpAddress::LocalHost, 10000) != Socket::Done) {
        cout<<"Erreur : impossible de se connecter au serveur!"<<endl;
        return false;
    } else {
        running = true;
    }
    bool certified = false;
    Packet packet;
    unsigned char* cliPbKey = nullptr;
    int size = CliEncryptedPacket::getCertificate(&cliPbKey);
    std::string message (reinterpret_cast<char*>(cliPbKey), size);
    message.insert(0, "SETCLIPBKEY");
    packet<<message;
    clientTCP.send(packet);
    Network::setCertifiateClientMess("SORROKCLIENT");
    bool certifiateMessageSend = false;
    while (running) {
        if (!certifiateMessageSend && !certified) {
            CliEncryptedPacket cliEncryptedPacket;
            if (clientTCP.receive(cliEncryptedPacket) == Socket::Done) {
                std::string response;
                cliEncryptedPacket>>response;
                cliEncryptedPacket.clear();
                if (response == "GETCERTIFIATECLIENT") {
                    std::string message = Network::getCertifiateClientMess();
                    std::cout<<"get client certified : "<<message<<std::endl;
                    cliEncryptedPacket<<message;
                    clientTCP.send(cliEncryptedPacket);
                    certifiateMessageSend = true;
                }
            }
        }
        if (certifiateMessageSend && !certified) {
            CliEncryptedPacket cliEncryptedPacket;
            if (clientTCP.receive(cliEncryptedPacket) == Socket::Done) {
                std::string response;
                cliEncryptedPacket>>response;
                cliEncryptedPacket.clear();
                if (response == "CERTIFIEDCLIENT") {
                    std::cout<<"client certified"<<std::endl;
                    certified = true;
                    running = false;
                }
            }
        }
    }*/
    return 0;
}
