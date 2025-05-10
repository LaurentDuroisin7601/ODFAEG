#include "../../../../include/odfaeg/Graphics/GUI/button.hpp"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            Button::Button(math::Vec3f position, math::Vec3f size, const Font* font, std::string t, unsigned int charSize, RenderWindow& rw) :
                LightComponent(rw, position, size, size * 0.5f) {
                background = sf::Color::White;
                text.setFont(*font);
                text.setString(t);
                text.setColor(sf::Color::Black);
                //text.setSize(size);
                text.setCharacterSize(charSize);
                rect = RectangleShape(size);
                rect.setOutlineThickness(5.f);
                rect.setOutlineColor(sf::Color::Red);
            }
            void Button::setText(std::string text) {
                this->text.setString(text);
            }
            void Button::clear() {
                if (background != rect.getFillColor())
                    rect.setFillColor(background);
            }
            void Button::setTextSize(unsigned int size) {
                text.setCharacterSize(size);
            }
            void Button::setTextColor(sf::Color color) {
                text.setColor(color);
            }
            void Button::onDraw(RenderTarget& target, RenderStates states) {
                text.setPosition(getPosition());
                rect.setPosition(getPosition());
                //text.setSize(getSize());
                rect.setSize(getSize());
                target.draw(rect);
                target.draw(text);
            }
            std::string Button::getText() {
                return text.getString();
            }
            bool Button::isMouseInButton() {
                physic::BoundingBox bb = getGlobalBounds();
                //math::Vec2f mousePos = math::Vec2f(window::IMouse::getPosition(getWindow()).x, window::IMouse::getPosition(getWindow()).y);
                if (bb.isPointInside(mousePos)) {
                    if (getName() == "DENY")
                        std::cout<<"button deny : "<<bb.getPosition()<<bb.getSize()<<mousePos<<std::endl;
                    return true;
                }
                return false;
            }
            void Button::addActionListener (ActionListener *al) {
                core::FastDelegate<bool> trigger(&Button::isMouseInButton, this);
                core::FastDelegate<void> slot(&ActionListener::actionPerformed,al, this);
                core::Action a (core::Action::EVENT_TYPE::MOUSE_BUTTON_PRESSED_ONCE, window::IMouse::Left);
                core::Command cmd(a, trigger, slot);
                getListener().connect("CBUTTONCLICKED", cmd);
            }
            void Button::onEventPushed (window::IEvent event, RenderWindow& window) {
                if (&getWindow() == &window)
                    getListener().pushEvent(event);
            }
            void Button::onUpdate (RenderWindow* window, window::IEvent& event) {
                if (&getWindow() == window) {
                    if (event.type == window::IEvent::MOUSE_BUTTON_EVENT
                        && event.mouseButton.type == window::IEvent::BUTTON_EVENT_PRESSED
                        && event.mouseButton.button == window::IMouse::Left) {
                        mousePos = math::Vec3f(event.mouseButton.x, event.mouseButton.y, 0);
                    }
                }
            }
            void Button::setIcon(const Texture* text) {
                rect.setTexture(text);
            }
            const Texture* Button::getIcon() {
                return rect.getTexture();
            }
        }
    }
}
