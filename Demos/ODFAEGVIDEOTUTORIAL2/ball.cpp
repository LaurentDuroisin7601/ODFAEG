#include "ball.hpp"
using namespace odfaeg::physic;
using namespace odfaeg::math;
using namespace odfaeg::core;
using namespace odfaeg::network;
using namespace odfaeg::graphic;
Ball::Ball() :
    Entity (Vec2f(395, 295), Vec2f(10, 10),Vec2f(5, 5),"E_BALL"),
    m_ball(5) {
    m_ball.setPosition(Vec3f(395, 295, 0));
    m_dir = Vec2f(1, 1).normalize();
    m_dir = m_dir.normalize();
    BoundingVolume* ballArea = new BoundingSphere(Vec3f(400, 300, 0), 5);
    setCollisionVolume(ballArea);
    m_speed = 0.0001f;
}
void Ball::setPartyId(int partyId) {
    m_partyId = partyId;
}
int Ball::partyId() {
    return m_partyId;
}
Ball::Historic Ball::getHistoric() {
    return historic;
}
void Ball::updateSpeed(float speed) {
    m_speed = speed;
}
void Ball::updateDir(Vec2f dir) {
    m_dir = dir;
}
float Ball::speed() {
    return m_speed;
}
Vec2f Ball::dir() {
    return m_dir;
}
void Ball::onMove(Vec3f& t) {
    Entity::onMove(t);
    getCollisionVolume()->move(t);
    m_ball.move(t);
}
void Ball::onDraw(RenderTarget& target, RenderStates states) {
    //std::cout<<"ball position : "<<m_ball.getPosition()<<std::endl;
    target.draw(m_ball, states);
}
