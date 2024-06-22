#ifndef PONG_BALL_HPP
#define PONG_BALL_HPP
#include "odfaeg/Physics/boundingSphere.h"
#include "odfaeg/Graphics/entity.h"
#include "odfaeg/Network/user.h"
#include "odfaeg/Network/symEncPacket.h"
class Ball : public odfaeg::graphic::Entity {
public :
    Ball();
    void updateSpeed(float speed);
    void updateDir (odfaeg::math::Vec2f dir);
    odfaeg::math::Vec2f dir ();
    float speed();
    struct Historic {
        sf::Int64 time;
        odfaeg::math::Vec2f pos;
    };
    Ball::Historic getHistoric();
    void onMove(odfaeg::math::Vec3f& t);
    bool isLeaf() const {
        return true;
    }
    bool operator== (odfaeg::graphic::Entity& entity) {
        return false;
    }
    bool isAnimated() const {
        return false;
    }
    bool isModel() const {
        return false;
    }
    bool selectable() const {
        return false;
    }
    bool isLight() const {
        return false;
    }
    bool isShadow() const {
        return false;
    }
    void registerBallStates(Historic historic) {
        this->historic = historic;
    }
    void setPartyId(int partyId);
    int partyId();
    Historic ballStates() {
        return historic;
    }
private :
    int m_partyId;
    Historic historic;
    odfaeg::math::Vec2f m_dir;
    float m_speed;
};
#endif // PONG_BALL_HPP
