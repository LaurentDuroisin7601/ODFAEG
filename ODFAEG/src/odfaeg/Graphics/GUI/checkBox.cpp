#include "../../../../include/odfaeg/Graphics/GUI/checkBox.hpp"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            CheckBox::CheckBox(RenderWindow& window, math::Vec3f position, math::Vec3f size) :
                LightComponent(window, position, size, size * 0.5f)  {
                rect = RectangleShape(size);
                backgroundColor = sf::Color::White;
                checkedColor = sf::Color::Magenta;
                rect.setFillColor(backgroundColor);
                rect.setOutlineThickness(1);
                rect.setOutlineColor(sf::Color::Green);
                mousePosition = math::Vec3f(0, 0, 0);
                checked = false;
                core::Action a(core::Action::MOUSE_BUTTON_PRESSED_ONCE, window::IMouse::Left);
                core::Command cmd(a, core::FastDelegate<bool>(&CheckBox::isMouseInside, this), core::FastDelegate<void>(&CheckBox::switchState, this));
                getListener().connect("CHANGESTATE", cmd);
            }
            void CheckBox::clear() {
                if (checked)
                    rect.setFillColor(checkedColor);
                else
                    rect.setFillColor(backgroundColor);
            }
            void CheckBox::onDraw(RenderTarget& target, RenderStates states) {
                rect.setPosition(getPosition());
                rect.setSize(getSize());
                target.draw(rect);
            }
            void CheckBox::setBackgroundColor(sf::Color color)  {
                backgroundColor = color;
            }
            sf::Color CheckBox::getBackgroundColor() {
                return backgroundColor;
            }
            void CheckBox::setCheckedColor(sf::Color color) {
                checkedColor = color;
            }
            sf::Color CheckBox::getCheckedColor() {
                return checkedColor;
            }
            void CheckBox::onUpdate(RenderWindow* window, window::IEvent& event) {
                if (&getWindow() == window) {
                    if (event.type == window::IEvent::MOUSE_MOTION_EVENT)
                        mousePosition = math::Vec3f(event.mouseMotion.x, event.mouseMotion.y, 0);
                }
            }
            bool CheckBox::isMouseInside() {
                physic::BoundingBox bb(getPosition().x, getPosition().y, 0, getSize().x, getSize().y, 0);
                return bb.isPointInside(mousePosition);
            }
            void CheckBox::setChecked(bool checked) {
                this->checked = checked;
            }
            bool CheckBox::isChecked() {
                return checked;
            }
            void CheckBox::switchState() {
                checked = !checked;
                setChecked(checked);
            }
        }
    }
}
