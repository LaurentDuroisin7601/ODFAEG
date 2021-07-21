#include "../../../../include/odfaeg/Graphics/GUI/menuItem.hpp"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            MenuItem::MenuItem(RenderWindow& rw, const Font* font, std::string t) :
                LightComponent(rw, math::Vec3f(0, 0, 0), math::Vec3f(t.length() * 10, 20, 0), math::Vec3f(0, 0, 0), -1)
                {
                    text.setFont(*font);
                    text.setCharacterSize(15);
                    text.setString(t);
                    text.setColor(sf::Color::Black);
                    rect = RectangleShape(getSize());
                    rect.setOutlineThickness(1);
                    rect.setOutlineColor(sf::Color::Black);
                    background = sf::Color(50, 50, 50);
                    setVisible(false);
                    setEventContextActivated(false);
                }
                void MenuItem::clear() {
                    if (background != rect.getFillColor())
                        rect.setFillColor(background);
                }
                void MenuItem::setPosition(math::Vec3f position) {
                    Transformable::setPosition(position);
                    text.setPosition(position);
                    rect.setPosition(position);
                }
                void MenuItem::onDraw(RenderTarget& target, RenderStates states) {
                    rect.setPosition(getPosition());
                    rect.setSize(getSize());
                    target.draw(rect);
                    target.draw(text);
                }
                void MenuItem::addMenuItemListener (MenuItemListener *mil) {
                    core::FastDelegate<bool> trigger(&MenuItem::isMouseOnMenu, this);
                    core::FastDelegate<void> slot(&MenuItemListener::actionPerformed,mil, this);
                    core::Action a (core::Action::EVENT_TYPE::MOUSE_BUTTON_PRESSED_ONCE, window::IMouse::Left);
                    core::Command cmd(a, trigger, slot);
                    getListener().connect("CITEMCLICKED"+getText(), cmd);
                }
                bool MenuItem::isMouseOnMenu() {
                    physic::BoundingBox bb = getGlobalBounds();
                    math::Vec2f mousePos = math::Vec2f(window::IMouse::getPosition(getWindow()).x, window::IMouse::getPosition(getWindow()).y);
                    if (bb.isPointInside(mousePos)) {
                        return true;
                    }
                    return false;
                }
                std::string MenuItem::getText() {
                    return text.getString();
                }
                void MenuItem::setText(std::string t) {
                    text.setString(t);
                }
                void MenuItem::onEventPushed(window::IEvent event, RenderWindow& window) {
                    if (&window == &getWindow())
                        getListener().pushEvent(event);
                }
        }
    }
}
