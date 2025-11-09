#include "../../../../include/odfaeg/Graphics/3D/wall.hpp"
namespace odfaeg {
    namespace graphic {
        namespace g3d {

            Wall::Wall (math::Vec3f position, math::Vec3f size, Type type, Light *light, EntityFactory& factory) : Model (position, size, position + size * 0.5f, factory, "E_WALL")  {
                float sy = light->getHeight() / (light->getHeight() * 0.75f);
                setShadowScale(math::Vec3f(1.f, sy, 1.f));
                int c = getSize().y() * sy;
                setShadowCenter(math::Vec3f(0, 0, -c));
                wType = type;
            }
            void Wall::setMesh(Entity *mesh) {
                std::vector<Entity*> children = getChildren();
                if (getChildren().size() == 0) {
                    addChild(mesh);
                    mesh->setParent(this);
                } else {
                    removeChild(getChildren()[0]);
                    addChild(mesh);
                    mesh->setParent(this);
                }
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




