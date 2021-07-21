#include "player.hpp"
using namespace odfaeg::network;
using namespace odfaeg::physic;
using namespace odfaeg::math;
using namespace odfaeg::core;
using namespace odfaeg::graphic;
Player::Player(std::string pseudo, Vec2f position) :
    Entity (position, Vec2f(10, 50), Vec2f(5, 25), "E_PLAYER"),
    m_player(Vec2f(10, 50)),
    m_pseudo(pseudo) {
    m_player.setPosition(Vec3f(position.x, position.y, 0));
    BoundingVolume* racketArea = new BoundingBox(position.x, position.y, 0, 10, 50, 0);
    setCollisionVolume(racketArea);
    m_partyId = -1;
    m_speed = 0.0001f;
    m_score = 0;
}
void Player::setPseudo(std::string pseudo) {
    m_pseudo = pseudo;
}
void Player::setPartyId(int partyId) {
    m_partyId = partyId;
}
int Player::partyId() {
    return m_partyId;
}
void Player::setSpeed(float speed) {
    m_speed = speed;
}
float Player::speed() {
    return m_speed;
}
std::string Player::pseudo() {
    return m_pseudo;
}
void Player::increaseScore() {
    m_score++;
}
void Player::resetScore() {
    m_score = 0;
}
unsigned int Player::score() {
    return m_score;
}
void Player::setMoving(bool moving) {
    this->moving = moving;
}
bool Player::isMoving() {
    return moving;
}
void Player::onMove(Vec3f& t) {
    Entity::onMove(t);
    getCollisionVolume()->move(t);
    m_player.move(t);
}
void Player::onDraw(RenderTarget& target, RenderStates states) {
    //std::cout<<"player position : "<<m_player.getPosition()<<std::endl;
    target.draw(m_player, states);
}
void Player::setDir(Vec2f dir) {
    m_dir = dir;
}
Vec2f Player::dir() {
    return m_dir;
}
