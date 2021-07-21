#include "../../../../include/odfaeg/Graphics/GUI/icon.hpp"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            Icon::Icon(RenderWindow& window, math::Vec3f position, math::Vec3f size, Sprite icon) : LightComponent (window, position, size, size * 0.5f, position.z), icon(icon) {
                mousePos = math::Vec3f(0, 0, 0);
            }
            void Icon::onDraw (RenderTarget& target, RenderStates states) {
                icon.setPosition(getPosition());
                icon.setSize(getSize());
                target.draw(icon, states);
            }
            bool Icon::isMouseInside() {
                physic::BoundingBox bb(getPosition().x, getPosition().y, 0, getSize().x, getSize().y, 0);
                return bb.isPointInside(mousePos);
            }
            void Icon::onUpdate(RenderWindow* window, window::IEvent& event) {
                if (&getWindow() == window) {
                    if (event.type == window::IEvent::MOUSE_MOTION_EVENT) {
                        mousePos = math::Vec3f(event.mouseMotion.x, event.mouseMotion.y, 0);
                    }
                }
            }
            Sprite Icon::getSprite() {
                return icon;
            }
        }
    }
}
