// *** ADDED BY HEADER FIXUP ***
// *** END ***
#ifndef CARACTER
#define CARACTER
#include <vector>
#include "odfaeg/Math/vec2f.h"
#include "odfaeg/Graphics/anim.h"
#include "odfaeg/Graphics/tile.h"
#include "odfaeg/Graphics/GUI/progressBar.hpp"
#include "odfaeg/Graphics/world.h"
#include <string>
#include <SFML/Graphics.hpp>
#include "odfaeg/Graphics/boneAnimation.hpp"
namespace sorrok {
    class MyAppli;
    class Caracter : public odfaeg::graphic::BoneAnimation {
    public :
        enum ANIMS {
            WALKING, ATTACKING = 8, TIPPING_OVER = 16
        };
        Caracter() : BoneAnimation(odfaeg::math::Vec3f(0, 0, 0), odfaeg::math::Vec3f(0, 0, 0),"E_CARACTER") {
            currentAnimIndex = 0;
        }
        Caracter (std::string type, std::string name, std::string currentMapName, std::string classs, int level);
        Entity* clone();
        void setXpHpBar(odfaeg::graphic::gui::ProgressBar* hpBar, odfaeg::graphic::gui::ProgressBar* xpBar, odfaeg::graphic::gui::ProgressBar* manaBar);
        bool isMovable() const {
            return true;
        }
        bool selectable() const {
            return false;
        }
        bool operator== (Entity& other);
        void addAnimation (odfaeg::graphic::Anim *anim);
        odfaeg::graphic::Tile& getCurrentTile();
        void setMoving(bool b);
        bool isMoving ();
        void setDir(odfaeg::math::Vec2f dir);
        odfaeg::math::Vec2f getDir();
        void setPath(std::vector<odfaeg::math::Vec2f> path);
        std::vector<odfaeg::math::Vec2f> getPath();
        float getSpeed();
        void setSpeed(float speed);
        odfaeg::graphic::Anim* getAnimation(unsigned int index);
        unsigned int getCurrentPathIndex ();
        void setCurrentPathIndex (unsigned int index);
        virtual bool isMonster() = 0;
        void setRange(int range);
        int getRange();
        void setLife(int life);
        void setMaxLife(int life);
        int getLife ();
        int getMaxLife();
        int getManaMax();
        int getLevel();
        std::string getClass();
        int getAttackMin();
        int getAttackMax();
        void setAttackSpeed(float attackSpeed);
        float getAttackSpeed();
        void setFightingMode(bool b);
        bool isInFightingMode();
        bool isAttacking ();
        void setAlive(bool b);
        bool isAlive();
        void setAttacking(bool b);
        void setCurrentXp(int xp);
        void setXpReqForNextLevel(int xpReqForNextLevel);
        void setLevel (int level);
        sf::Time getTimeOfLastAttack();
        sf::Time getTimeOfLastHpRegen();
        sf::Time getTimeOfLastManaRegen();
        void attackFocusedCaracter(int attack);
        void setFocusedCaracter(Caracter* caracter);
        Caracter* getFocusedCaracter();
        void up (int xp);
        int getCurrentXp ();
        int getXpReqForNextLevel ();
        float getRegenHpSpeed();
        float getRegenManaSpeed();
        void setRegenHpSpeed(float regenHpSpeed);
        int getRegenHpAmountMin();
        void setRegenHpAmountMin(int regenHpAmount);
        int getRegenHpAmountMax();
        void setRegenHpAmountMax(int regenHpAmount);
        Entity* getCurrentFrame() const;
        void onDraw(odfaeg::graphic::RenderTarget&, odfaeg::graphic::RenderStates);
        void onMove(odfaeg::math::Vec3f& t);
        virtual void setIsMovingFromKeyboard(bool b) {}
        virtual bool isMovingFromKeyboard() = 0;
        sf::Clock& getClkTransfertTime();
        sf::Time getTimeBeforeLastRespawn();
        sf::Time getTimeSinceLastRespawn();
        void restartAttackSpeed();
        void restartRespawn();
        std::vector<int>& getDamages();
        void setDamages(std::vector<int> damages);
        void setDmgTransferTime(sf::Int64 time);
        void setRgnTransferTime(sf::Int64 time);
        void setRgnManaTransferTime(sf::Int64 time);
        sf::Int64 getDmgTransferTime();
        sf::Int64 getRgnTransferTime();
        sf::Int64 getRgnManaTransferTime();
        std::string getName();
        template <typename Archive>
        void vtserialize(Archive & ar) {
            Entity::vtserialize(ar);
            ar(moving);
            ar(name);
            ar(classs);
            ar(level);
            ar(speed);
            ar(attackMin);
            ar(attackMax);
            ar(range);
            ar(attackSpeed);
            ar(regenHpSpeed);
            ar(alive);
            ar(regenHpAmountMin);
            ar(regenHpAmountMax);
            ar(life);
            ar(mana);
            ar(regenManaSpeed);
            ar(regenManaAmountMin);
            ar(regenManaAmountMax);
            sf::Int64 i = 0;
            addAttribute("isAlive"+odfaeg::core::conversionIntString(getId()), i);
            addAttribute("isMoving"+odfaeg::core::conversionIntString(getId()),i);
            addAttribute("isInFightingMode"+odfaeg::core::conversionIntString(getId()), i);
            addAttribute("isAttacking"+odfaeg::core::conversionIntString(getId()), i);
            addAttribute("life"+odfaeg::core::conversionIntString(getId()), i);
            addAttribute("mana"+odfaeg::core::conversionIntString(getId()), i);
            addAttribute("position"+odfaeg::core::conversionIntString(getId()), i);
        }
        std::pair<odfaeg::math::Vec2f, odfaeg::math::Vec2f> interpolation;
        void setRegen(std::vector<int> regen);
        void setManaRegen (std::vector<int> regen);
        std::vector<int>& getRegen();
        std::vector<int>& getManaRegen();
        void restartRegenHP();
        void restartRegenMana();
        virtual ~Caracter();
        bool isAttacked();
        odfaeg::graphic::gui::ProgressBar* getXpBar();
        int getMana();
        void setMana(int mana);
        private :
        std::string name, currentMapName, classs;
        int level, attackMin, attackMax, sterk, range;
        float attackSpeed, regenHpSpeed, regenManaSpeed, speed;
        bool moving, alive;
        odfaeg::math::Vec2f dir;
        std::vector<odfaeg::math::Vec2f> path;
        std::vector<odfaeg::graphic::Anim*> anims;
        int currentAnimIndex;
        int life, maxLife, regenHpAmountMin, regenHpAmountMax, mana, regenManaAmountMin, regenManaAmountMax, maxMana;
        bool attacking, fightingMode;
        sf::Clock clockAtkSpeed, clockRegenHp, clockTransfertTime, clockBeforeLastRespawn, clockManaRegen;
        Caracter* focusedCaracter;
        ANIMS baseAnimIndex;
        sf::Time timeBefLastRespawn;
        std::vector<int> damages, regen, manaRegen;
        sf::Int64 dmgTransferTime, rgnTransferTime, rgnManaTransferTime;
        odfaeg::graphic::gui::ProgressBar *hpBar, *xpBar, *manaBar;
        bool attacked;
    };
}
#endif


