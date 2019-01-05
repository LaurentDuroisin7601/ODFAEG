#ifndef CARACTER_HPP
#define CARACTER_HPP
#include <vector>
#include "odfaeg/Math/vec2f.h"
#include "odfaeg/Graphics/anim.h"
#include "odfaeg/Graphics/tile.h"
#include "odfaeg/Network/user.h"
#include <string>
#include <SFML/Graphics.hpp>
namespace sorrok {
    class Caracter : public odfaeg::graphic::AnimatedEntity {
    public :
        Caracter (std::string type, std::string name, std::string currentMapName, std::string classs, int level);
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
        void setRange(int range);
        int getRange();
        void setLife(int life);
        void setMaxLife(int life);
        int getLife ();
        int getMaxLife();
        int getLevel();
        std::string getClass();
        std::string getName();
        int getAttackMin();
        int getAttackMax();
        void setAttack(int attack);
        void setAttackSpeed(float attackSpeed);
        float getAttackSpeed();
        void setFightingMode(bool b);
        bool isInFightingMode();
        bool isAttacking ();
        void setAlive(bool b);
        bool isAlive();
        void setAttacking(bool b);
        void setLevel (int level);
        sf::Time getTimeOfLastAttack();
        sf::Time getTimeOfLastHpRegen();
        void setFocusedCaracter(Caracter* caracter);
        Caracter* getFocusedCaracter();
        void attackFocusedCaracter(int dmg);
        float getRegenHpSpeed();
        void setRegenHpSpeed(float regenHpSpeed);
        int getRegenHpAmountMin();
        int getRegenHpAmountMax();
        void setRegenHpAmountMin(int regenHpAmountMin);
        void setRegenHpAmountMax(int regenHpAmountMax);
        Entity* getCurrentFrame() const;
        void onDraw(odfaeg::graphic::RenderTarget&, odfaeg::graphic::RenderStates);
        void onMove(odfaeg::math::Vec3f& t);
        virtual void setIsMovingFromKeyboard(bool b) {}
        virtual bool isMovingFromKeyboard() = 0;
        bool isAttacked();
        bool isAgressif();
        void setIsAttacked (bool attacked);
        void setIsAgressif (bool agressif);
        sf::Time getTimeBeforeLastRespawn();
        sf::Time getTimeSinceLastRespawn();
        template <typename Archive>
        void vtserialize(Archive & ar) {
            Entity::vtserialize(ar);
            ar(name);
            ar(classs);
            ar(level);
            ar(speed);
            ar(path);
            ar(attackMin);
            ar(attackMax);
            ar(range);
            ar(attackSpeed);
            ar(regenHpSpeed);
            ar(alive);
            ar(regenHpAmountMin);
            ar(regenHpAmountMax);
        }
        virtual bool isMonster() = 0;
        void restartRespawn();
        void restartAttackSpeed();
        int getDefMin();
        int getDefMax();
        int getFleeRate();
        int getCriticalChanceRate();
        int getCriticalAddDamagesRate();
        std::vector<int> getDamages();
        void setDamages(std::vector<int> damages);
        void setRegen(std::vector<int> regen);
        std::vector<int> getRegen();
        void restartRegenHP();
        virtual ~Caracter();
    private :
        std::vector<int> damages, regen;
        std::string name, currentMapName, classs;
        int level, attackMin, attackMax, range, sterk, defMin, defMax;
        float attackSpeed, regenHpSpeed, speed;
        bool moving, alive, moveFromKeyboard;
        odfaeg::math::Vec2f dir;
        std::vector<odfaeg::math::Vec2f> path;
        std::vector<odfaeg::graphic::Anim*> anims;
        int currentAnimIndex;
        int life, maxLife, regenHpAmountMin, regenHpAmountMax, fleeRate, criticalChanceRate, criticalAddDamagesRate;
        bool attacking, fightingMode, attacked, agressif;
        sf::Clock clockAtkSpeed, clockRegenHp, clockBeforeRespawn;
        sf::Time timeBefLastRespawn;
        Caracter* focusedCaracter;
    };
}
#endif
