#include "pnj.hpp"
using namespace odfaeg::graphic;
namespace sorrok {
    Pnj::Pnj(EntityFactory& factory) : Caracter("E_PNJ", "QUESTY", "MapTest", "Novice", 1, factory) {
    }
    void Pnj::addQuest (Quest quest) {
        quests.push_back(quest);
    }
    std::vector<Quest> Pnj::getQuests() {
        return quests;
    }
    bool Pnj::isMovingFromKeyboard() {
        return false;
    }
    bool Pnj::isMonster() {
        return false;
    }
    Entity* Pnj::clone() {
        Pnj* pnj = factory.make_entity<Pnj>(factory);
        Caracter::copy(pnj);
        pnj->quests = quests;
        return pnj;
    }
}
