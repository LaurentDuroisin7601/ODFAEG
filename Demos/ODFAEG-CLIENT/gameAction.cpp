#include "gameAction.hpp"
using namespace odfaeg::network;
using namespace odfaeg::core;
namespace sorrok {
    void GameAction::operator()(Item& item, Hero* hero) {
        SymEncPacket packet;
        std::string request = "CANUSE*"+conversionIntString(hero->getId())+"*"+conversionIntString(item.getType());
        packet<<request;
        Network::sendTcpPacket(packet);
        std::string response = Network::waitForLastResponse("CANUSE");
        if (response == "OK") {
            item.applyBehaviour();
        }
    }
    void GameAction::operator()(Skill& skill, Hero* hero) {
        SymEncPacket packet;
        std::string request = "CANLAUNCH*"+conversionIntString(hero->getId())+"*"+skill.getName();
        packet<<request;
        Network::sendTcpPacket(packet);
        std::string response = Network::waitForLastResponse("CANLAUNCH");
        std::cout<<"response : "<<response<<std::endl;
        if (response == "OK") {
            std::cout<<"apply skill behaviour"<<std::endl;
            skill.applySkillBehaviour();
            std::cout<<"skill behaviour updated"<<std::endl;
        }
    }
}
