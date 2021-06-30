#ifndef MONSTER_HPP
#define MONSTER_HPP
#include <vector>
#include "odfaeg/Physics/boundingPolyhedron.h"
#include "caracter.h"
namespace sorrok {
    class Monster : public Caracter {
    public :
        Monster() : Caracter("E_MONSTER", "", "", "", 0) {}
        Monster (std::string name, std::string currentMapName, std::string classs, int level, odfaeg::physic::BoundingPolyhedron zone);
        odfaeg::physic::BoundingPolyhedron& getZone();
        odfaeg::math::Vec3f respawn ();
        bool isMonster() {
            return true;
        }
        template <typename Archive>
        void vtserialize(Archive & ar) {
            Caracter::vtserialize(ar);
            ar(zone);
            ar(xp);
        }
        sf::Time getMoveTimeInterval1();
        sf::Time getMoveTimeInterval2();
        sf::Time getTimeUntilNextMove();
        void setTimeUntilNextMove(sf::Time time);
        sf::Clock& getClkLastMove();
        bool isMovingFromKeyboard();
        void setXp (unsigned int xp);
        unsigned int getXp();
        ~Monster();
    private :
        unsigned int xp;
        sf::Time time1, time2, time3;
        sf::Clock clockLastMove;
        odfaeg::physic::BoundingPolyhedron zone;
    };
}
#endif

