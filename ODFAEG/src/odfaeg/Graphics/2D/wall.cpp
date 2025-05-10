#include "../../../../include/odfaeg/Graphics/2D/wall.h"
namespace odfaeg {
    namespace graphic {
        namespace g2d {

            Wall::Wall (Tile *tile, Type type, Light *light, EntityFactory& factory) : Model (tile->getPosition(),tile->getSize(), tile->getOrigin(), factory, "E_WALL")  {
                addChild(tile);
                tile->setParent(this);
                float sy = light->getHeight() / (light->getHeight() * 0.75f);
                setShadowScale(math::Vec3f(1.f, sy, 1.f));
                int c = getSize().y * sy;
                setShadowCenter(math::Vec3f(0, 0, -c));
                wType = type;
            }
            int Wall::getWallType() {
                return wType;
            }
            void Wall::setWallType(int type) {
                wType = static_cast<Type>(type);
            }
            Entity* Wall::clone() {
                Wall* w = factory.make_entity<Wall>(factory);
                GameObject::copy(w);
                w->wType = wType;
                return w;
            }
            bool Wall::isLeaf() const {
                return false;
            }
            bool Wall::operator== (Entity &other) {
                if (!GameObject::operator==(other))
                    return false;
                return wType == other.getWallType();
            }
            bool Wall::selectable () const {
                return true;
            }
            bool Wall::isLight() const {
                return false;
            }
            bool Wall::isShadow() const {
                return false;
            }
        }
    }
}



