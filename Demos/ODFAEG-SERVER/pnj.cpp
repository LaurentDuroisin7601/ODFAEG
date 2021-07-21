#include "pnj.hpp"
namespace sorrok {
    Pnj::Pnj() : Caracter("E_PNJ", "QUESTY", "MapTest", "Novice", 1) {
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
}
