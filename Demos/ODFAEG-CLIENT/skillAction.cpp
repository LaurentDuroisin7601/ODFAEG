#include "skillAction.hpp"
#include "application.h"
using namespace odfaeg::core;
using namespace odfaeg::network;
namespace sorrok {
    void SkillAction::launchLastHeal(Hero* hero, Skill skill) {
        if (hero->isAlive()) {
            static_cast<MyAppli*>(MyAppli::app)->launchSkillAnim(skill.getName());
            std::string request = "ADDLIFE*"+conversionIntString(hero->getId())+"*"+conversionFloatString(10);
            SymEncPacket packet;
            packet<<request;
            Network::sendTcpPacket(packet);
            request = "RETRACTMANA*"+conversionIntString(hero->getId())+"*"+conversionFloatString(skill.getManaCost());
            packet.clear();
            packet<<request;
            Network::sendTcpPacket(packet);
        }
    }
}
