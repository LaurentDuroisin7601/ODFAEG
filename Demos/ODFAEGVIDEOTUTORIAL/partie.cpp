#include "partie.hpp"
int Partie::nbParties = 0;
Partie::Partie(Player& p1, Player& p2, Ball& ball) :
    m_player1(p1), m_player2(p2), m_ball(ball) {
        m_id = nbParties;
        nbParties++;
        running = true;
}
void Partie::start() {
    running = true;
}
void Partie::finished() {
    running = false;
}
bool Partie::isRunning() {
    return running;
}
Player& Partie::player1() {
    return m_player1;
}
Player& Partie::player2() {
    return m_player2;
}
Ball& Partie::ball() {
    return m_ball;
}
int Partie::id() {
    return m_id;
}


