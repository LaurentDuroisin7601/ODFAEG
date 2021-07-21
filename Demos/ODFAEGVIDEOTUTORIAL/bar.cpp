#include "bar.hpp"
using namespace odfaeg::graphic;
using namespace odfaeg::math;
using namespace odfaeg::physic;
Bar::Bar(Vec2f position, Vec2f size) : Entity(position, size, size * 0.5f, "E_BAR") {
    BoundingVolume* bv = new BoundingBox(position.x, position.y, 0 ,size.x, size.y, 0);
    setCollisionVolume(bv);
}
