#include "caracter.h"
#include <iostream>
using namespace std;
using namespace odfaeg::core;
using namespace odfaeg::math;
using namespace odfaeg::graphic;
namespace sorrok {
    Caracter::Caracter (std::string type, string name, string currentMapName, string classs, int level)
    : AnimatedEntity (Vec3f(-25, -50, 0), Vec3f (50, 100, 0), Vec3f(25, 50, 0), type) {
        currentAnimIndex = 0;
        this->name = name;
        this->classs = classs;
        this->level = level;
        speed = 0.0001f;
        moving = false;
        dir = Vec2f(0, 1);
        this->life = 100;
        this->maxLife = 100;
        range = 100;
        fleeRate = 1;
        criticalChanceRate = 1;
        criticalAddDamagesRate = 10;
        defMin = 2;
        defMax = 4;
        attackSpeed = 1.f;
        attackMin = 5;
        attackMax = 10;
        fightingMode = attacking = attacked = agressif = false;
        alive = true;
        attacked = false;
        regenHpSpeed = 1.f;
        regenHpAmountMin = 1;
        regenHpAmountMax = 5;
        regenManaAmountMin = 1;
        regenManaAmountMax = 2;
        regenManaSpeed = 1.f;
        focusedCaracter = nullptr;
        timeBefLastRespawn = sf::seconds(10.f);
        manaMax = 100;
        mana = 100;
    }
    Entity* Caracter::clone() {
    }
    float Caracter::getRegenManaSpeed() {
        return regenManaSpeed;
    }
    int Caracter::getRegenManaAmountMin() {
        return regenManaAmountMin;
    }
    int Caracter::getRegenManaAmountMax() {
        return regenManaAmountMax;
    }
    void Caracter::setMana(int mana) {
        if (mana < 0)
            mana = 0;
        if (mana > manaMax)
            mana = manaMax;
        this->mana = mana;
        //clockRegenMana.restart();
    }
    int Caracter::getMana() {
        return mana;
    }
    void Caracter::setIsAgressif(bool agressif) {
        this->agressif = agressif;
    }
    bool Caracter::isAttacked () {
        return attacked;
    }
    bool Caracter::isAgressif() {
        return agressif;
    }
    void Caracter::onMove(Vec3f& t) {
        Entity::onMove(t);
        if (getCollisionVolume() != nullptr) {
            getCollisionVolume()->move(Vec3f(t.x, t.y, 0));
        }
    }
    float Caracter::getRegenHpSpeed () {
        return regenHpSpeed;
    }
    void Caracter::setRegenHpSpeed(float regenHpSpeed) {
        this->regenHpSpeed = regenHpSpeed;
    }
    sf::Time Caracter::getTimeOfLastHpRegen() {
        return clockRegenHp.getElapsedTime();
    }
    sf::Time Caracter::getTimeOfLastManaRegen() {
        return clockRegenMana.getElapsedTime();
    }
    void Caracter::setLevel(int level) {
        this->level = level;
    }
    void Caracter::setSpeed(float speed) {
        this->speed = speed;
    }
    float Caracter::getSpeed() {
        return speed;
    }
    int Caracter::getRegenHpAmountMin() {
        return regenHpAmountMin;
    }
    int Caracter::getRegenHpAmountMax() {
        return regenHpAmountMax;
    }
    void Caracter::setRegenHpAmountMin(int regenHpAmountMin) {
        this->regenHpAmountMin = regenHpAmountMin;
    }
    void Caracter::setRegenHpAmountMax(int regenHpAmountMax) {
        this->regenHpAmountMax = regenHpAmountMax;
    }
    void Caracter::setLife(int life) {
        this->life = life;
        if (life < 0)
            life = 0;
        if (life > maxLife)
            life = maxLife;
        //clockRegenHp.restart();
    }
    int Caracter::getLife() {
    return life;
    }
    void Caracter::setRange(int range) {
        this->range = range;
    }
    int Caracter::getRange() {
        return range;
    }
    void Caracter::setAttackSpeed (float attackSpeed) {
        this->attackSpeed = attackSpeed;
    }
    float Caracter::getAttackSpeed () {
        return attackSpeed;
    }
    void Caracter::setAttacking(bool b) {
        if (attacking == true && b == false) {
            damages.clear();
        }
        this->attacking = b;
    }
    void Caracter::setAlive(bool b) {
        if(alive == false && b == true) {
            setLife(getMaxLife());
        }
        if (alive == true && b == false) {
            damages.clear();
            regen.clear();
            manaRegen.clear();
        }
        alive = b;
    }
    bool Caracter::isAlive () {
        return alive;
    }
    bool Caracter::isAttacking() {
        return attacking;
    }
    void Caracter::setFightingMode(bool b) {
        if (fightingMode == false && b == true) {
            regen.clear();
            manaRegen.clear();
            if (focusedCaracter)
                focusedCaracter->attacked = b;
        }
        if (fightingMode == true && b == false) {
            if (!moving && !attacked)
                clockRegenHp.restart();
            if (focusedCaracter)
                focusedCaracter->attacked = b;
            if (!focusedCaracter->moving && !focusedCaracter->fightingMode && !focusedCaracter->attacked)
                focusedCaracter->clockRegenHp.restart();
        }
        this->fightingMode = b;
    }
    bool Caracter::operator== (Entity &other) {
        if (getType() != other.getType())
            return false;
            Caracter& caracter = static_cast<Caracter&>(other);
            if (anims.size() != caracter.anims.size())
                return false;
            for (unsigned int i = 0; i < anims.size(); i++) {
                if (anims[i] != caracter.anims[i])
                    return false;
            }
        return true;
    }
    bool Caracter::isInFightingMode() {
        return fightingMode;
    }
    int Caracter::getAttackMin() {
        return attackMin;
    }
    int Caracter::getAttackMax() {
        return attackMax;
    }
    sf::Time Caracter::getTimeOfLastAttack() {
        return clockAtkSpeed.getElapsedTime();
    }
    void Caracter::setDir (Vec2f dir) {
        this->dir = dir;
    }
    Vec2f Caracter::getDir () {
        return dir;
    }
    void Caracter::setMoving (bool b) {
        if (moving == true && b == false) {
            regen.clear();
            manaRegen.clear();
            if (!fightingMode && !attacked) {
                clockRegenHp.restart();
                clockRegenMana.restart();
            }
        }
        this->moving = b;
    }
    bool Caracter::isMoving () {
        return moving;
    }
    void Caracter::setPath(vector<Vec2f> path) {
        this->path = path;
    }
    vector<Vec2f> Caracter::getPath() {
        return path;
    }
    void Caracter::addAnimation (Anim *anim) {
        addChild(anim);
        anim->setParent(this);
        anims.push_back(anim);
    }
    Anim* Caracter::getAnimation(unsigned int index) {
        if (index >= 0 && index < anims.size())
            return anims[index];
        return NULL;
    }
    void Caracter::setMaxLife(int life) {
        this->maxLife = maxLife;
    }
    int Caracter::getManaMax() {
        return manaMax;
    }
    int Caracter::getMaxLife() {
        return maxLife;
    }
    int Caracter::getLevel() {
        return level;
    }
    std::string Caracter::getName() {
        return name;
    }
    string Caracter::getClass () {
        return classs;
    }
    void Caracter::onDraw(RenderTarget &target, RenderStates states) {
        target.draw(*getCurrentFrame(), states);
    }
    Entity* Caracter::getCurrentFrame() const {
        return (anims.size() > currentAnimIndex) ? anims[currentAnimIndex] : nullptr;
    }
    void Caracter::setFocusedCaracter(Caracter* focusedCaracter) {
        this->focusedCaracter = focusedCaracter;
    }
    Caracter* Caracter::getFocusedCaracter() {
        return focusedCaracter;
    }
    void Caracter::attackFocusedCaracter(int dmg) {
        focusedCaracter->setLife(focusedCaracter->getLife() - dmg);
        if (focusedCaracter->getLife() <= 0 && focusedCaracter->isAlive()) {
            std::cout<<focusedCaracter->getType()<<" is death"<<std::endl;
            focusedCaracter->setLife(0);
            focusedCaracter->setAlive(false);
            focusedCaracter->setAttacking(false);
            focusedCaracter->setFightingMode(false);
            focusedCaracter->restartRespawn();
            setAttacking(false);
            setFightingMode(false);
        }
    }
    sf::Time Caracter::getTimeBeforeLastRespawn() {
        return timeBefLastRespawn;
    }
    sf::Time Caracter::getTimeSinceLastRespawn() {
        return clockBeforeRespawn.getElapsedTime();
    }
    void Caracter::restartRespawn() {
        clockBeforeRespawn.restart();
    }
    void Caracter::restartAttackSpeed() {
        clockAtkSpeed.restart();
    }
    int Caracter::getDefMin() {
        return defMin;
    }
    int Caracter::getDefMax() {
        return defMax;
    }
    int Caracter::getFleeRate() {
        return fleeRate;
    }
    int Caracter::getCriticalChanceRate() {
        return criticalChanceRate;
    }
    int Caracter::getCriticalAddDamagesRate () {
        return criticalAddDamagesRate;
    }
    vector<int>& Caracter::getDamages() {
        return damages;
    }
    void Caracter::setDamages(std::vector<int> damages) {
        this->damages = damages;
    }
    void Caracter::setRegen(std::vector<int> regen) {
        this->regen = regen;
    }
    std::vector<int>& Caracter::getRegen() {
        return regen;
    }
    std::vector<int>& Caracter::getRegenMana() {
        return manaRegen;
    }
    void Caracter::setRegenMana(std::vector<int> manaRegen) {
        this->manaRegen = manaRegen;
    }
    void Caracter::restartRegenHP() {
        clockRegenHp.restart();
    }
    void Caracter::restartRegenMana() {
        clockRegenMana.restart();
    }
    Caracter::~Caracter() {
    }
}




