
#include <iostream>
#include <string>
#include <vector>
// *** END ***
#ifndef MY_APPLI
#define MY_APPLI
#include "odfaeg/Graphics/application.h"
#include "odfaeg/Graphics/convexShape.h"
#include "odfaeg/Graphics/rectangleShape.h"
#include "odfaeg/Graphics/circleShape.h"
#include "odfaeg/Graphics/tile.h"
#include "odfaeg/Graphics/world.h"
#include "odfaeg/Graphics/map.h"
#include "odfaeg/Graphics//2D/decor.h"
#include "odfaeg/Graphics/anim.h"
#include "odfaeg/Graphics/2D/ambientLight.h"
#include "odfaeg/Graphics/2D/ponctualLight.h"
#include "odfaeg/Graphics/2D/wall.h"
#include "odfaeg/Graphics/tGround.h"
#include "odfaeg/Graphics/entitiesUpdater.h"
#include "odfaeg/Graphics/animationUpdater.h"
#include "odfaeg/Network/network.h"
#include "odfaeg/Physics/particuleSystem.h"
#include "hero.h"
#include "monster.h"
#include <fstream>
#include "MySQL/mysql/jdbc.h"
#include "pnj.hpp"
namespace sorrok {
    class MyAppli : public odfaeg::core::Application {
    private :
        const float speed = 0.2f;
        bool running;
        odfaeg::graphic::g2d::Wall *w;
        Caracter* caracter;
        sf::Keyboard::Key actualKey, previousKey;
        std::vector<odfaeg::graphic::Tile*> tiles;
        std::vector<odfaeg::graphic::Tile*> walls;
        odfaeg::graphic::Map* theMap;
        odfaeg::graphic::g2d::PonctualLight* light2;
        static const unsigned int PATH_ERROR_MARGIN = 5;
        odfaeg::math::Vec3f tmpPosition;
        sql::Driver *driver;
        sql::Connection *connection;
        std::map<int, std::pair<std::vector<Item>, std::pair<sf::Time, sf::Time>>> itemsLooted;
        static int spotID;
    public :
        MyAppli();
        ~MyAppli();
        void onLoad();
        void onInit ();
        void onExec ();
        void onDisconnected(odfaeg::network::User* user);
    };
}
#endif // MY_APPLI

