#include "hero.h"
namespace sorrok {
    using namespace odfaeg::core;
    using namespace odfaeg::graphic::gui;
    Hero::Hero(std::string factionName, std::string name, std::string sex, std::string currentMapName, std::string hairColor,
        std::string eyesColor, std::string skinColor, std::string faceType, std::string classs, int level)
        : Caracter("E_HERO", name, currentMapName, classs, level) {
        this->factionName = factionName;
        this->sex = sex;
        this->hairColor = hairColor;
        this->eyesColor = eyesColor;
        this->skinColor = skinColor;
        this->faceType = faceType;
        xp = 0;
        xpReqForNextLevel = 1500;
        moveFromKeyboard = false;
    }
    void Hero::addItem(Item item) {
        std::map<Item::Type, std::vector<Item>>::iterator it;
        it = inventory.find(item.getType());
        if (it != inventory.end()) {
            it->second.push_back(item);
        } else {
            std::vector<Item> items;
            items.push_back(item);
            inventory.insert(std::make_pair(item.getType(), items));
        }
    }
    bool Hero::isMovingFromKeyboard() {
        return moveFromKeyboard;
    }
    void Hero::setCurrentXp(int xp) {
        this->xp = xp;
    }
    void Hero::setXpReqForNextLevel(int xpReqForNextLevel) {
        this->xpReqForNextLevel = xpReqForNextLevel;
    }
    void Hero::up (int xp) {
        getXpBar()->setName("XPBAR");
        this->xp += xp;
        if (this->xp >= xpReqForNextLevel) {
            setLevel(getLevel() + 1);
            this->xp = this->xp - xpReqForNextLevel;
            xpReqForNextLevel *= 1.1f;
        }
        getXpBar()->setValue(this->xp);
    }
    int Hero::getCurrentXp () {
        return xp;
    }
    int Hero::getXpReqForNextLevel () {
        return xpReqForNextLevel;
    }
    void Hero::setIsMovingFromKeyboard(bool b) {
        moveFromKeyboard = b;
    }
    std::map<Item::Type, std::vector<Item>>& Hero::getInventory() {
        return inventory;
    }
    bool Hero::containsQuest(std::string name) {
        for (unsigned int i = 0; i < diary.size(); i++) {
            if (diary[i].getName() == name)
                return true;
        }
        return false;
    }
    Quest* Hero::getQuest(std::string name) {
        for (unsigned int i = 0; i < diary.size(); i++) {
            if (diary[i].getName() == name) {
                return &diary[i];
            }
        }
        return nullptr;
    }
    void Hero::addQuest(Quest quest) {
        diary.push_back(quest);
    }
    void Hero::removeQuest(Quest quest) {
        std::vector<Quest>::iterator it;
        for (it = diary.begin(); it != diary.end(); ){
            if (it->getName() == quest.getName()) {
                it = diary.erase(it);
            } else {
                it++;
            }
        }
    }
    std::vector<Quest> Hero::getDiary() {
        return diary;
    }
    void Hero::addSkill(Skill skill) {
        skills.push_back(skill);
    }
    std::vector<Skill> Hero::getSkills() {
        return skills;
    }
    Hero::~Hero() {
    }
}
