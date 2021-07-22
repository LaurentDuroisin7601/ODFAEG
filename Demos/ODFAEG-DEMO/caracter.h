// *** ADDED BY HEADER FIXUP ***
#include <vector>
// *** END ***
#ifndef CARACTER
#define CARACTER
#include "odfaeg/Math/vec2f.h"
#include "odfaeg/Graphics/anim.h"
#include "odfaeg/Graphics/boneAnimation.hpp"
#include "odfaeg/Graphics/tile.h"
#include <string>
#include <SFML/Graphics.hpp>
namespace sorrok {
    class Caracter : public odfaeg::graphic::BoneAnimation {
    public :
        enum ANIMS {
            WALKING, ATTACKING = 8
        };
        Caracter(odfaeg::graphic::EntityFactory& factory) : BoneAnimation(odfaeg::math::Vec3f(0, 0, 0), odfaeg::math::Vec3f(0, 0, 0),"E_CARACTER", factory) {
            currentAnimIndex = 0;
        }
        Caracter (std::string type, std::string name, std::string currentMapName, std::string classs, int level, odfaeg::graphic::EntityFactory& factory);
        bool isMovable() const {
            return true;
        }
        bool selectable() const {
            return false;
        }
        Entity* clone();
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
        int getLevel();
        std::string getClass();
        int getAttack();
        void setAttack(int attack);
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
        void attackFocusedCaracter();
        void setFocusedCaracter(Caracter* caracter);
        Caracter* getFocusedCaracter();
        void up (int xp);
        int getCurrentXp ();
        int getXpReqForNextLevel ();
        float getRegenHpSpeed();
        void setRegenHpSpeed(float regenHpSpeed);
        int getRegenHpAmount();
        void setRegenHpAmount(int regenHpAmount);
        Entity* getCurrentFrame() const;
        void onDraw(odfaeg::graphic::RenderTarget&, odfaeg::graphic::RenderStates);
        void onMove(odfaeg::math::Vec3f& t);
        virtual void setIsMovingFromKeyboard(bool b) {}
        virtual bool isMovingFromKeyboard() = 0;
        sf::Clock& getClkTransfertTime();

        template <typename Archive>
        void vtserialize(Archive & ar) {
            GameObject::vtserialize(ar);
            ar(name);
            ar(classs);
            ar(level);
            ar(speed);
            ar(path);
            ar(attack);
            ar(range);
            ar(attackSpeed);
            ar(regenHpSpeed);
            ar(alive);
            ar(regenHpAmount);
        }
        std::pair<odfaeg::math::Vec2f, odfaeg::math::Vec2f> interpolation;
        virtual ~Caracter();
        private :
        std::string name, currentMapName, classs;
        int level, attack, range;
        float attackSpeed, regenHpSpeed, speed;
        bool moving, alive;
        odfaeg::math::Vec2f dir;
        std::vector<odfaeg::math::Vec2f> path;
        std::vector<odfaeg::graphic::Anim*> anims;
        int currentAnimIndex;
        int life, maxLife, regenHpAmount;
        bool attacking, fightingMode;
        sf::Clock clockAtkSpeed, clockRegenHp, clockTransfertTime;
        Caracter* focusedCaracter;
        ANIMS baseAnimIndex;
    };
}
#endif


