#include "hero.h"
using namespace odfaeg::network;
using namespace odfaeg::graphic;
namespace sorrok {
    Hero::Hero(User* user, std::string factionName, std::string name, std::string sex, std::string currentMapName, std::string hairColor,
        std::string eyesColor, std::string skinColor, std::string faceType, std::string classs, int level, EntityFactory& factory)
        : Caracter("E_HERO", name, currentMapName, classs, level, factory) {
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
    void Hero::addEquipment(Item equipment) {
        if (equipment.getName() == "Novice Head") {
            setDefMin(getDefMin() + 2);
            setDefMax(getDefMax() + 2);
        }
        if (equipment.getName() == "Novice Armor") {
            setDefMin(getDefMin() + 3);
            setDefMax(getDefMax() + 3);
        }
        if (equipment.getName() == "Novice Gloves") {
            setDefMin(getDefMin() + 1);
            setDefMax(getDefMax() + 1);
        }
        if (equipment.getName() == "Novice Boots") {
            setDefMin(getDefMin() + 2);
            setDefMax(getDefMax() + 2);
        }
        if (equipment.getName() == "Novice Weapon Right") {
            setAttackMin(getAttackMin() + 5);
            setAttackMax(getAttackMax() + 5);
        }
        if (equipment.getName() == "Novice Weapon Left") {
            setAttackMin(getAttackMin() + 3);
            setAttackMax(getAttackMax() + 3);
        }
        stuff.push_back(equipment);
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
    Entity* Hero::clone() {
        Hero* h = factory.make_entity<Hero>(factory);
        Caracter::copy(h);
        h->factionName = factionName;
        h->sex = sex;
        h->hairColor = hairColor;
        h->eyesColor = eyesColor;
        h->skinColor = skinColor;
        h->faceType = faceType;
        h->xp = xp;
        h-> xpReqForNextLevel = xpReqForNextLevel;
        h->moveFromKeyboard = moveFromKeyboard;
        h->user = user;
        return h;
    }
    std::vector<Item> Hero::getEquipment() {
        return stuff;
    }
    Hero::~Hero() {
    }
}
