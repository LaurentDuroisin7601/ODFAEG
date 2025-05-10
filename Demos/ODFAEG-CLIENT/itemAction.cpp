#include "itemAction.hpp"
#include "application.h"
namespace sorrok {
    using namespace odfaeg::network;
    using namespace odfaeg::core;
    void ItemAction::useHpPotion(Hero* hero, Item hpPotion) {
        if (hero->isAlive()) {
            std::cout<<"use hp potion action"<<std::endl;
            static_cast<MyAppli*>(MyAppli::app)->retractFromInventory(hpPotion);
            SymEncPacket packet;
            std::string request = "ADDLIFE*"+conversionIntString(hero->getId())+"*"+conversionFloatString(10);
            packet<<request;
            Network::sendTcpPacket(packet);
        }
    }
}
