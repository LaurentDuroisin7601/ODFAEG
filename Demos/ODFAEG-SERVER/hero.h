#ifndef HERO_HPP
#define HERO_HPP
#include "caracter.h"
#include "odfaeg/Network/network.h"
#include "quest.hpp"
#include "skill.hpp"
namespace sorrok {
    class Hero : public Caracter {
    public :
        Hero(odfaeg::graphic::EntityFactory& factory) : Caracter("E_HERO", "", "", "", 0, factory) {}
        Hero(odfaeg::network::User* user, std::string factionName, std::string name, std::string sex, std::string currentMapName, std::string hairColor,
        std::string eyesColor, std::string skinColor, std::string faceType, std::string classs, int level, odfaeg::graphic::EntityFactory& factory);
        odfaeg::network::User* getUser();
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
        void addQuest(Quest quest);
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
            ar(stuff);
            std::cout<<"equipement : "<<stuff.size()<<std::endl;
        }
        void addSkill(Skill skill);
        std::vector<Skill> getSkills();
        void addItem(Item item);
        void addEquipment(Item equipment);
        std::vector<Item> getEquipment();
        std::map<Item::Type, std::vector<Item>>& getInventory();
        odfaeg::graphic::Entity* clone();
        ~Hero();
    private :
        std::string factionName, sex, hairColor, eyesColor, skinColor, faceType;
        bool moveFromKeyboard;
        int xp, xpReqForNextLevel;
        odfaeg::network::User* user;
        std::vector<Quest> diary;
        std::vector<Skill> skills;
        std::vector<Item> stuff;
        std::map<Item::Type, std::vector<Item>> inventory;
    };
}
#endif // HERO_HPP
