#include "../../../../include/odfaeg/Graphics/2D/decor.h"
namespace odfaeg {
    namespace graphic {
        namespace g2d {
            using namespace sf;
            Decor::Decor (Tile *t, Light *light, EntityFactory& factory) : Model(t->getPosition(), t->getSize(), t->getSize() * 0.5f, factory, "E_DECOR") {
                addChild(t);
                t->setParent(this);
                float sy = light->getHeight() / (light->getHeight() * 0.75f);
                setShadowScale(math::Vec3f(1.f, -sy, 1.f));
                setShadowOrigin(math::Vec3f(t->getPosition().x, t->getPosition().y, 0));
            }
            Entity* Decor::clone() {
                Decor* decor = factory.make_entity<Decor>(factory);
                GameObject::copy(decor);
                return decor;
            }
            void Decor::onMove(math::Vec3f &t) {
                GameObject::onMove(t);
            }
            bool Decor::operator==(Entity &other) {
                return GameObject::operator==(other);
            }
        }
    }
}
