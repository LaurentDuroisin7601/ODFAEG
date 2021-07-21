#ifndef PONG_PARTIE_HPP
#define PONG_PARTIE_HPP
#include "player.hpp"
#include "ball.hpp"
class Partie {
public :
    Partie (Player& p1, Player& p2, Ball& ball);
    void start();
    void finished();
    bool isRunning();
    Player& player1();
    Player& player2();
    Ball& ball();
    int id();
private :
    bool running;
    Ball& m_ball;
    Player& m_player1;
    Player& m_player2;
    int m_id;
    static int nbParties;
};
#endif // PONG_PARTIE_HPP
