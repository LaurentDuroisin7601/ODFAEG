#include "monster.h"
#include <iostream>
using namespace std;
using namespace odfaeg::math;
using namespace odfaeg::graphic;
using namespace odfaeg::core;
using namespace odfaeg::physic;
namespace sorrok {
    Monster::Monster (std::string name, std::string currentMapName, std::string classs, int level, BoundingPolyhedron zone)
     : Caracter ("E_MONSTER", name, currentMapName, classs, level) {
        this->zone = zone;
        time1 = sf::seconds(5.f);
        time2 = sf::seconds(7.f);
        time3 = sf::seconds(Math::random(time1.asSeconds(), time2.asSeconds()));
        xp = 500;
    }
    Vec3f Monster::respawn() {
         std::array<std::array<float, 2>, 3> exts = Computer::getExtends(zone.getPoints());
         int x = Math::random(exts[0][1], exts[0][0]);
         int y = Math::random(exts[1][1], exts[1][0]);
         int z = y;
         setAlive(true);
         restartRespawn();
         return Vec3f(x, y, z);
    }
    sf::Time Monster::getMoveTimeInterval1() {
        return time1;
    }
    sf::Time Monster::getMoveTimeInterval2() {
        return time2;
    }
    void Monster::setTimeUntilNextMove(sf::Time time) {
        time3 = time;
    }
    sf::Clock& Monster::getClkLastMove() {
        return clockLastMove;
    }
    sf::Time Monster::getTimeUntilNextMove() {
        return time3;
    }
    BoundingPolyhedron& Monster::getZone() {
        return zone;
    }
    bool Monster::isMovingFromKeyboard() {
        return false;
    }
    void Monster::setXp(unsigned int xp) {
        this->xp = xp;
    }
    unsigned int Monster::getXp() {
        return xp;
    }
    Monster::~Monster() {
    }
}





