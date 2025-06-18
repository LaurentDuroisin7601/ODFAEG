#ifndef PONG_HPP
#define PONG_HPP
#include "odfaeg/Core/application.h"
#include "odfaeg/Network/network.h"
#include "partie.hpp"
#include "odfaeg/Graphics/world.h"
#include "odfaeg/Graphics/map.h"
#include "bar.hpp"
class Pong : public odfaeg::core::Application {
public :
    void onInit();
    void onExec();
    void onDisconnected(odfaeg::network::User* user);
    ~Pong();
private :
    sf::Int64 checkLastBallPos (Ball* ball, sf::Int64 lastSrvTime, sf::Int64 elapsedTime);
    void moveBall(Ball* ball, sf::Int64 elapsedTime);
    void movePlayer(Player* player, sf::Int64 elapsedTime);
    std::map<int, Partie*> parties;
    std::map<int, Player*> players;
};
#endif // PONG_HPP
