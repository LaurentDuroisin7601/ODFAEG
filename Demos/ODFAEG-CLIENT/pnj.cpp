#include "pnj.hpp"
using namespace odfaeg::graphic;
namespace sorrok {
    Pnj::Pnj(EntityFactory& factory) : Caracter("E_PNJ", "QUESTY", "MapTest", "Novice", 1, factory) {

    }
    void Pnj::addQuest (Quest quest) {
        bool found = false;
        for (unsigned int i = 0; i < quests.size() && !found; i++) {
            if (quest.getName() == quests[i].getName()) {
                found=true;
            }
        }
        if (!found)
            quests.push_back(quest);
    }
    bool Pnj::isMovingFromKeyboard() {
        return false;
    }
    bool Pnj::isMonster() {
        return false;
    }
    std::vector<Quest>& Pnj::getQuests() {
        return quests;
    }
}
