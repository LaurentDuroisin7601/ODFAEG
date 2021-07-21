#include "hero.h"
using namespace odfaeg::network;
namespace sorrok {
    Hero::Hero(User* user, std::string factionName, std::string name, std::string sex, std::string currentMapName, std::string hairColor,
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
        this->user = user;
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
    std::map<Item::Type, std::vector<Item>>& Hero::getInventory() {
        return inventory;
    }
    void Hero::addQuest(Quest quest) {
        diary.push_back(quest);
    }
    User* Hero::getUser() {
        return user;
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
        this->xp += xp;
        if (this->xp >= xpReqForNextLevel) {
            setLevel(getLevel() + 1);
            xpReqForNextLevel *= 2;
            this->xp = this->xp - xpReqForNextLevel;
        }
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
    void Hero::addSkill(Skill skill) {
        skills.push_back(skill);
    }
    std::vector<Skill> Hero::getSkills() {
        return skills;
    }
    Hero::~Hero() {
    }
}
