#ifndef HERO_HPP
#define HERO_HPP
#include "caracter.h"
#include "item.hpp"
#include "quest.hpp"
#include "skill.hpp"
namespace sorrok {
    class Hero : public Caracter {
    public :
        Hero() : Caracter("E_HERO", "", "", "", 0) {}
        Hero(std::string factionName, std::string name, std::string sex, std::string currentMapName, std::string hairColor,
        std::string eyesColor, std::string skinColor, std::string faceType, std::string classs, int level);
        void setIsMovingFromKeyboard(bool b);
        bool isMovingFromKeyboard();
        bool isMonster() {
            return false;
        }
        void setCurrentXp(int xp);
        void setXpReqForNextLevel(int xpReqForNextLevel);
        void up (int xp);
        int getCurrentXp ();
        int getXpReqForNextLevel ();
        std::vector<Skill> getSkills();
        template <typename Archive>
        void vtserialize(Archive& ar) {
            Caracter::vtserialize(ar);
            ar(factionName);
            ar(sex);
            ar(hairColor);
            ar(eyesColor);
            ar(skinColor);
            ar(faceType);
            ar(xp);
            ar(xpReqForNextLevel);
            ar(skills);
        }
        void addItem (Item item);
        bool containsQuest(std::string name);
        void addQuest(Quest quest);
        void removeQuest (Quest quest);
        Quest* getQuest(std::string name);
        std::vector<Quest> getDiary();
        std::map<Item::Type, std::vector<Item>>& getInventory();
        void addSkill (Skill skill);
        ~Hero();
    private :
        std::string factionName, sex, hairColor, eyesColor, skinColor, faceType;
        bool moveFromKeyboard;
        int xp, xpReqForNextLevel;
        std::map<Item::Type, std::vector<Item>> inventory;
        std::vector<Skill> skills;
        std::vector<Quest> diary;
    };
}
#endif // HERO_HPP
