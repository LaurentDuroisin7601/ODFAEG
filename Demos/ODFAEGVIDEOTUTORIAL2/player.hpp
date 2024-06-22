#ifndef PONG_PLAYER_HPP
#define PONG_PLAYER_HPP
#include "odfaeg/Network/user.h"
#include "odfaeg/Network/symEncPacket.h"
#include "odfaeg/Physics/boundingBox.h"
#include "odfaeg/Graphics/entity.h"
#include "odfaeg/Graphics/rectangleShape.h"
class Player : public odfaeg::graphic::Entity {
public :
    Player(std::string pseudo, odfaeg::math::Vec2f position);
    std::string pseudo();
    void setPseudo(std::string pseudo);
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
    void setMoving(bool moving);
    bool isMoving();
    void setDir(odfaeg::math::Vec2f dir);
    odfaeg::math::Vec2f dir();
    void setSpeed(float speed);
    float speed();
    int partyId();
    void increaseScore();
    unsigned int score();
    void resetScore();
    void onDraw(odfaeg::graphic::RenderTarget &target, odfaeg::graphic::RenderStates states);
    std::pair<odfaeg::math::Vec2f, odfaeg::math::Vec2f> m_interpolation;
private :
    unsigned int m_score;
    int m_partyId;
    odfaeg::math::Vec2f m_dir;
    bool moving;
    std::string m_pseudo;
    float m_speed;
    odfaeg::graphic::RectangleShape m_player;
};
#endif // PLAYER_HPP
