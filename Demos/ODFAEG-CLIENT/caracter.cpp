#include "caracter.h"
#include <iostream>
#include "application.h"
using namespace std;
using namespace odfaeg::core;
using namespace odfaeg::graphic;
using namespace odfaeg::graphic::gui;
using namespace odfaeg::physic;
using namespace odfaeg::math;
namespace sorrok {
    Caracter::Caracter (std::string type, std::string name, string currentMapName, string classs, int level, EntityFactory& factory) : BoneAnimation (Vec3f(-25, -50, 0), Vec3f (50, 100, 0), type, factory) {
        currentAnimIndex = 0;
        this->name = name;
        this->currentMapName = currentMapName;
        this->classs = classs;
        this->level = level;
        speed = 0.0001f;
        moving = false;
        dir = Vec2f(0, 1);
        this->life = 100;
        this->maxLife = 100;
        range = 100;
        attackSpeed = 5.f;
        attackMin = 5;
        attackMax = 10;
        fightingMode = attacking;
        alive = true;
        attacked = false;
        regenHpSpeed = 1.f;
        regenHpAmountMin = 1;
        regenHpAmountMax = 2;
        regenManaAmountMin = 1;
        regenManaAmountMax = 2;
        regenManaSpeed = 1.f;
        focusedCaracter = nullptr;
        baseAnimIndex = WALKING;
        timeBefLastRespawn = sf::seconds(10.f);
        dmgTransferTime = rgnTransferTime = 0;
        mana = 100;
        maxMana = 100;
    }
    void Caracter::setXpHpBar(ProgressBar* xpBar, ProgressBar* hpBar, ProgressBar* manaBar) {
        this->xpBar = xpBar;
        this->hpBar = hpBar;
        this->manaBar = manaBar;
    }
    void Caracter::setDmgTransferTime(sf::Int64 time) {
        dmgTransferTime = time;
    }
    void Caracter::setRgnTransferTime(sf::Int64 time) {
        rgnTransferTime = time;
    }
    sf::Int64 Caracter::getDmgTransferTime() {
        return dmgTransferTime;
    }
    sf::Int64 Caracter::getRgnTransferTime() {
        return rgnTransferTime;
    }
    void Caracter::setRgnManaTransferTime(sf::Int64 time) {
        rgnManaTransferTime = time;
    }
    sf::Int64 Caracter::getRgnManaTransferTime() {
        return rgnManaTransferTime;
    }
    void Caracter::onMove(Vec3f& t) {
        GameObject::onMove(t);
        if (getCollisionVolume() != nullptr) {
            getCollisionVolume()->move(Vec3f(t.x, t.y, 0));
        }
        changeAttribute("position"+conversionIntString(getId()), Application::app->getClock("TimeClock").getElapsedTime().asMicroseconds());
    }
    float Caracter::getRegenHpSpeed () {
        return regenHpSpeed;
    }
    float Caracter::getRegenManaSpeed() {
        return regenManaSpeed;
    }
    void Caracter::setRegenHpSpeed(float regenHpSpeed) {
        this->regenHpSpeed = regenHpSpeed;
    }
    sf::Time Caracter::getTimeOfLastHpRegen() {
        return clockRegenHp.getElapsedTime();
    }
    sf::Time Caracter::getTimeOfLastManaRegen() {
        return clockManaRegen.getElapsedTime();
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
    void Caracter::setRegenHpAmountMin(int regenHpAmount) {
        this->regenHpAmountMin = regenHpAmount;
    }
    int Caracter::getRegenHpAmountMax() {
        return regenHpAmountMax;
    }
    void Caracter::setRegenHpAmountMax(int regenHpAmount) {
        this->regenHpAmountMax = regenHpAmount;
    }
    void Caracter::setLife(int life) {
        changeAttribute("life"+conversionIntString(getId()), Application::app->getClock("TimeClock").getElapsedTime().asMicroseconds());
        if (life < 0)
            life = 0;
        if (life >= maxLife)
            life = maxLife;
        this->life = life;
        if (hpBar != nullptr) {
            hpBar->setValue(life);
        }
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
        if (attacking != b) {
            this->attacking = b;
            if (attacking) {
                changeAttribute("isAttacking"+conversionIntString(getId()), Application::app->getClock("TimeClock").getElapsedTime().asMicroseconds());
                anims[baseAnimIndex + currentAnimIndex]->stop();
                //anims[baseAnimIndex + currentAnimIndex]->setCurrentFrame(0);
                baseAnimIndex = ATTACKING;
                BoneAnimation::setBoneAnimationIndex(baseAnimIndex + currentAnimIndex);
                anims[baseAnimIndex + currentAnimIndex]->setCurrentFrame(0);
            } else {
                changeAttribute("isAttacking"+conversionIntString(getId()), Application::app->getClock("TimeClock").getElapsedTime().asMicroseconds());
                anims[baseAnimIndex + currentAnimIndex]->stop();
                //anims[baseAnimIndex + currentAnimIndex]->setCurrentFrame(0);
                damages.clear();
            }
        }
    }
    void Caracter::setAlive(bool b) {
        if (alive == true && b == false) {
            changeAttribute("isAlive"+conversionIntString(getId()),Application::app->getClock("TimeClock").getElapsedTime().asMicroseconds());
            anims[baseAnimIndex + currentAnimIndex]->stop();
            //anims[baseAnimIndex + currentAnimIndex]->setCurrentFrame(0);
            baseAnimIndex = TIPPING_OVER;
            anims[baseAnimIndex + currentAnimIndex]->play(false);
            BoneAnimation::setBoneAnimationIndex(baseAnimIndex + currentAnimIndex);
            damages.clear();
            regen.clear();
            manaRegen.clear();
            restartRespawn();
            //World::update();
        } else if (alive == false && b == true) {
            changeAttribute("isAlive"+conversionIntString(getId()),Application::app->getClock("TimeClock").getElapsedTime().asMicroseconds());
            baseAnimIndex = WALKING;
            BoneAnimation::setBoneAnimationIndex(baseAnimIndex + currentAnimIndex);
            setLife(getMaxLife());
            //World::update();
        }
        alive = b;
    }
    Entity* Caracter::clone() {
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
            changeAttribute("isInFightingMode"+conversionIntString(getId()),Application::app->getClock("TimeClock").getElapsedTime().asMicroseconds());
        }
        if (fightingMode == true && b == false) {
            if (focusedCaracter)
                focusedCaracter->attacked = b;
            changeAttribute("isInFightingMode"+conversionIntString(getId()),Application::app->getClock("TimeClock").getElapsedTime().asMicroseconds());
            if (!moving && !attacked)
            clockRegenHp.restart();
            if (focusedCaracter) {
                focusedCaracter->attacked = b;
                if (!focusedCaracter->moving && !focusedCaracter->fightingMode && !focusedCaracter->attacked)
                    focusedCaracter->clockRegenHp.restart();
            }
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
        unsigned int previousAnimIndex = baseAnimIndex + currentAnimIndex;
        float angleRadians = dir.getAngleBetween(Vec2f::yAxis);
        int angle = Math::toDegrees(angleRadians);
        //Sud
        if (angle >= -10 && angle <= 10)
            currentAnimIndex = 4;
        //Sud ouest
        else if (angle > -80 && angle < -10)
            currentAnimIndex = 6;
        //Ouest
        else if (angle >= -100 && angle <= -80)
            currentAnimIndex = 7;
        //Nord ouest
        else if (angle > -170 && angle < -100)
            currentAnimIndex = 3;
        //Nors est
        else if (angle > 100 && angle < 170)
            currentAnimIndex = 2;
        //Est
        else if (angle >= 80 && angle <= 100)
            currentAnimIndex = 0;
        //Sud est
        else if (angle > 10 && angle < 80)
            currentAnimIndex = 5;
        //Nord
        else
            currentAnimIndex = 1;
        this->dir = dir;
        if (previousAnimIndex != baseAnimIndex + currentAnimIndex) {
            anims[previousAnimIndex]->stop();
            if (moving)
                anims[baseAnimIndex + currentAnimIndex]->play(true);
            else
                anims[baseAnimIndex + currentAnimIndex]->play(false);
            BoneAnimation::setBoneAnimationIndex(baseAnimIndex+currentAnimIndex);
        }
        //World::update();
    }
    Vec2f Caracter::getDir () {
        return dir;
    }

    void Caracter::setMoving (bool b) {
        if (moving != b) {
            if (moving == true && b == false) {
                regen.clear();
                manaRegen.clear();
                if (!fightingMode && !attacked) {
                    clockRegenHp.restart();
                    clockManaRegen.restart();
                }
            }
            this->moving = b;
            if (moving ) {
               /* if (getRootType() == "E_MONSTER")
                std::cout<<"start moving"<<std::endl;*/
                changeAttribute("isMoving"+conversionIntString(getId()),Application::app->getClock("TimeClock").getElapsedTime().asMicroseconds());
                anims[baseAnimIndex + currentAnimIndex]->stop();
                //anims[baseAnimIndex + currentAnimIndex]->setCurrentFrame(0);
                baseAnimIndex = WALKING;
                anims[baseAnimIndex + currentAnimIndex]->play(true);
                BoneAnimation::setBoneAnimationIndex(baseAnimIndex + currentAnimIndex);
            } else {
                /* if (getRootType() == "E_MONSTER")
                std::cout<<"stop moving"<<std::endl;*/
                changeAttribute("isMoving"+conversionIntString(getId()),Application::app->getClock("TimeClock").getElapsedTime().asMicroseconds());
                anims[baseAnimIndex + currentAnimIndex]->stop();
                //anims[baseAnimIndex + currentAnimIndex]->setCurrentFrame(0);
            }
        }
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
    int Caracter::getMaxLife() {
        return maxLife;
    }
    int Caracter::getManaMax() {
        return maxMana;
    }
    int Caracter::getLevel() {
        return level;
    }
    string Caracter::getClass () {
        return classs;
    }
    std::string Caracter::getName() {
        return name;
    }
    void Caracter::onDraw(RenderTarget &target, RenderStates states) {
        target.draw(*getCurrentFrame(), states);
    }
    Entity* Caracter::getCurrentFrame() const {
        return anims[baseAnimIndex + currentAnimIndex];
    }
    sf::Clock& Caracter::getClkTransfertTime() {
        return clockTransfertTime;
    }
    void Caracter::setFocusedCaracter(Caracter* focusedCaracter) {
        this->focusedCaracter = focusedCaracter;
    }
    Caracter* Caracter::getFocusedCaracter() {
        return focusedCaracter;
    }
    void Caracter::attackFocusedCaracter(int attack) {
        anims[baseAnimIndex + currentAnimIndex]->play(false);
        focusedCaracter->setLife(focusedCaracter->getLife() - attack);
    }
    sf::Time Caracter::getTimeBeforeLastRespawn() {
        return timeBefLastRespawn;
    }
    sf::Time Caracter::getTimeSinceLastRespawn() {
        return clockBeforeLastRespawn.getElapsedTime();
    }
    void Caracter::restartRespawn() {
        clockBeforeLastRespawn.restart();
    }
    void Caracter::setDamages(std::vector<int> damages) {
        this->damages = damages;
    }
    std::vector<int>& Caracter::getDamages() {
        return damages;
    }
    void Caracter::setRegen(std::vector<int> regen) {
        this->regen = regen;
    }
    std::vector<int>& Caracter::getRegen() {
        return regen;
    }
    void Caracter::setManaRegen(std::vector<int> manaRegen) {
        this->manaRegen = manaRegen;
    }
    std::vector<int>& Caracter::getManaRegen() {
        return manaRegen;
    }
    void Caracter::restartAttackSpeed() {
        clockAtkSpeed.restart();
    }
    void Caracter::restartRegenHP() {
        clockRegenHp.restart();
    }
    void Caracter::restartRegenMana() {
        clockManaRegen.restart();
    }
    bool Caracter::isAttacked() {
        return attacked;
    }
    ProgressBar* Caracter::getXpBar() {
        return xpBar;
    }
    int Caracter::getMana() {
        return mana;
    }
    void Caracter::setMana(int mana) {
        changeAttribute("mana"+conversionIntString(getId()), Application::app->getClock("TimeClock").getElapsedTime().asMicroseconds());
        if (mana < 0)
            mana = 0;
        if (mana > maxMana)
            mana = maxMana;
        this->mana = mana;
        if (manaBar != nullptr) {
            manaBar->setValue(mana);
        }
        //clockManaRegen.restart();
    }
    Caracter::~Caracter() {
    }
}





