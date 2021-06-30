#ifndef PONG_PLAYER_HPP
#define PONG_PLAYER_HPP
#include "odfaeg/Network/user.h"
#include "odfaeg/Network/symEncPacket.h"
#include "odfaeg/Physics/boundingBox.h"
#include "odfaeg/Graphics/entity.h"
class Player : public odfaeg::graphic::Entity {
public :
    Player(odfaeg::network::User& user, std::string pseudo, odfaeg::math::Vec2f position);
    std::string pseudo();
    void setPartyId (int partyId);
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
    void onMove(odfaeg::math::Vec3f& t);
    bool isConnected();
    void setMoving(bool moving);
    bool isMoving();
    void setDir(odfaeg::math::Vec2f dir);
    odfaeg::math::Vec2f dir();
    void setSpeed(float speed);
    float speed();
    odfaeg::network::User& user();
    int partyId();
    void increaseScore();
    unsigned int score();
    void resetScore();
private :
    unsigned int m_score;
    int m_partyId;
    odfaeg::math::Vec2f m_dir;
    bool moving;
    std::string m_pseudo;
    odfaeg::network::User& m_user;
    float m_speed;
};
#endif // PLAYER_HPP
