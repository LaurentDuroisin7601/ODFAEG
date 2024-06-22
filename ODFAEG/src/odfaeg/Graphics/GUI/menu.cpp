#include "../../../../include/odfaeg/Graphics/GUI/menu.hpp"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            Menu::Menu(RenderWindow& rw, const Font* font, std::string t) :
                LightComponent(rw, math::Vec3f(0, 0, 0), math::Vec3f(t.length() * 10, 20, 0), math::Vec3f(0, 0, 0), -2)
{
                text.setString(t);
                text.setCharacterSize(15);
                text.setFont(*font);
                text.setColor(sf::Color::Black);
                rect = RectangleShape(getSize());
                rect.setOutlineThickness(1);
                rect.setOutlineColor(sf::Color::Black);
                core::Action a (odfaeg::core::Action::MOUSE_BUTTON_PRESSED_ONCE, window::IMouse::Left);
                std::cout<<"address of this in menu : "<<this<<std::endl;
                core::Command menuSelected (a, core::FastDelegate<void> (&Menu::onClick, this));
                //La fonction connect n'est pas appelée.
                getListener().connect("MenuSelected"+t, menuSelected);
                background = sf::Color(50, 50, 50);
            }
            void Menu::clear() {
                if (background != rect.getFillColor())
                    rect.setFillColor(background);
            }
            void Menu::addMenuItem(MenuItem* item) {
                unsigned int yPos = getSize().y;
                for (unsigned int i = 0; i < items.size(); i++) {
                    yPos += items[i]->getSize().y;
                }
                item->setPosition(math::Vec3f(getPosition().x, yPos, 0));
                items.push_back(item);
            }
            void Menu::setPosition(math::Vec3f position) {
                Transformable::setPosition(position);
                text.setPosition(position);
                rect.setPosition(position);
                unsigned int yPos = getSize().y;
                for (unsigned int i = 0; i < items.size(); i++) {
                    items[i]->setPosition(math::Vec3f(position.x, yPos, position.z));
                    yPos += items[i]->getSize().y;
                }
            }
            void Menu::onDraw(RenderTarget& target, RenderStates states) {
                rect.setPosition(getPosition());
                rect.setSize(getSize());
                text.setPosition(getPosition());
                text.setSize(getSize());
                target.draw(rect);
                target.draw(text);
            }
            void Menu::onClick() {
                if (isMouseOnMenu()) {
                    for (unsigned int i = 0; i < items.size(); i++) {
                        items[i]->setVisible(true);
                        items[i]->setEventContextActivated(true);
                    }
                } else {
                    for (unsigned int i = 0; i < items.size(); i++) {
                        items[i]->setVisible(false);
                        items[i]->setEventContextActivated(false);
                    }
                }
            }
            bool Menu::isMouseOnMenu() {
                physic::BoundingBox bb = getGlobalBounds();
                math::Vec2f mousePos = math::Vec2f(window::IMouse::getPosition(getWindow()).x, window::IMouse::getPosition(getWindow()).y);
                if (bb.isPointInside(mousePos)) {
                    return true;
                }
                return false;
            }
            void Menu::onEventPushed(window::IEvent event, RenderWindow& window) {
                if (&getWindow() == &window) {
                    getListener().pushEvent(event);
                }
            }
        }
    }
}
