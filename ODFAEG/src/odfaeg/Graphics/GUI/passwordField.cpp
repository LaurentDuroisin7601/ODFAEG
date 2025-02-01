#include "../../../../include/odfaeg/Graphics/GUI/passwordField.hpp"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            PasswordField::PasswordField(math::Vec3f position, math::Vec3f size, const Font* font, std::string t, RenderWindow& rw) :
                LightComponent(rw, position, size, math::Vec3f(0, 0, 0)) {
                tmp_text = "";
                id_text = t;
                background = sf::Color::White;
                text.setFont(*font);
                text.setString(t);
                text.setColor(sf::Color::Black);
                text.setPosition(position);
                text.setSize(size);
                rect = RectangleShape(size);
                /*rect.setOutlineThickness(5.f);
                rect.setOutlineColor(sf::Color::Black);*/
                core::Action a2 (core::Action::MOUSE_BUTTON_PRESSED_ONCE, window::IMouse::Left);
                core::Command cmd2(a2, core::FastDelegate<bool>(&PasswordField::isMouseInTextArea, this), core::FastDelegate<void>(&PasswordField::gaignedFocus, this));
                core::Command cmd3(a2, core::FastDelegate<bool>(&PasswordField::isMouseOutTextArea, this), core::FastDelegate<void>(&PasswordField::lostFocus, this));
                getListener().connect("CGFOCUS"+t, cmd2);
                getListener().connect("CLFOCUS"+t, cmd3);
                core::Action a3 (core::Action::MOUSE_BUTTON_PRESSED_ONCE, window::IMouse::Left);
                core::Command cmd4(a3, core::FastDelegate<bool>(&PasswordField::isMouseInTextArea, this), core::FastDelegate<void>(&PasswordField::setCursorPos, this));
                getListener().connect("CMOUSECLICKED"+t, cmd4);
                core::Action a4 (core::Action::TEXT_ENTERED);
                core::Command cmd5(a4, core::FastDelegate<bool>(&PasswordField::hasFocus, this), core::FastDelegate<void>(&PasswordField::onTextEntered, this, 'a'));
                getListener().connect("CTEXTENTERED"+t, cmd5);
                currentIndex = 0;
                setSize(text.getSize());
                haveFocus = false;
            }
            void PasswordField::onEventPushed(window::IEvent event, RenderWindow& window) {
                if (&window == &getWindow()) {
                    getListener().pushEvent(event);
                }
            }
            bool PasswordField::hasFocus() {
                return haveFocus;
            }
            void PasswordField::setCursorPos() {
                math::Vec2f mousePos = math::Vec2f(window::IMouse::getPosition(getWindow()).x, window::IMouse::getPosition(getWindow()).y);
                bool found = false;
                for (unsigned int i = 0; i <= tmp_text.length() && !found; i++) {
                    sf::Vector2f pos = text.findCharacterPos(i);
                    if (pos.x > mousePos.x && pos.y > mousePos.y - text.getCharacterSize()) {
                        cursorPos = math::Vec3f(pos.x, pos.y, 0);
                        currentIndex = i;
                        found = true;
                    }
                }
            }
            void PasswordField::gaignedFocus() {
                haveFocus = true;
                onGaignedFocus();
            }
            void PasswordField::lostFocus() {
                haveFocus = false;
                onLostFocus();
            }
            void PasswordField::onGaignedFocus() {
            }
            void PasswordField::onLostFocus() {
            }
            void PasswordField::clear() {
                if (background != rect.getFillColor())
                    rect.setFillColor(background);
            }
            void PasswordField::setTextSize(unsigned int size) {
                text.setCharacterSize(size);
                setSize(text.getSize());
            }
            void PasswordField::setTextColor(sf::Color color) {
                text.setColor(color);
            }
            std::string PasswordField::getText() {
                return tmp_text;
            }
            void PasswordField::onDraw(RenderTarget& target, RenderStates states) {
                VertexArray va(sf::Lines);
                va.append(Vertex(sf::Vector3f(cursorPos.x, cursorPos.y, 0), sf::Color::Black));
                va.append(Vertex(sf::Vector3f(cursorPos.x, cursorPos.y + text.getCharacterSize(), 0), sf::Color::Black));
                rect.setPosition(getPosition());
                rect.setSize(getSize());
                target.draw(rect);
                target.draw(va);
            }
            bool PasswordField::isMouseInTextArea() {
                physic::BoundingBox bb = getGlobalBounds();
                math::Vec2f mousePos = math::Vec2f(window::IMouse::getPosition(getWindow()).x, window::IMouse::getPosition(getWindow()).y);
                if (bb.isPointInside(mousePos)) {
                    return true;
                }
                return false;
            }
            bool PasswordField::isMouseOutTextArea() {
                physic::BoundingBox bb = getGlobalBounds();
                math::Vec2f mousePos = math::Vec2f(window::IMouse::getPosition(getWindow()).x, window::IMouse::getPosition(getWindow()).y);
                if (bb.isPointInside(mousePos)) {
                    return false;
                }
                return true;
            }
            void PasswordField::onUpdate(RenderWindow* window, window::IEvent& event) {
                if (window == &getWindow() && event.type == window::IEvent::TEXT_INPUT_EVENT) {
                    getListener().setCommandSlotParams("CTEXTENTERED"+id_text, this, static_cast<char>(event.text.unicode));
                }
            }
            void PasswordField::onTextEntered(char caracter) {
                if (tmp_text.length() > 0 && caracter == 8) {
                    tmp_text.erase(currentIndex-1, 1);
                    currentIndex--;
                }
                else if (caracter != 8) {
                    tmp_text.insert(currentIndex, 1, caracter);
                    currentIndex++;
                }
                text.setString(tmp_text);
                sf::Vector2f pos = text.findCharacterPos(currentIndex);
                cursorPos = math::Vec3f(pos.x, pos.y, 0);
                setSize(this->text.getSize());
            }
            void PasswordField::setText(std::string text) {
                tmp_text = text;
                this->text.setString(tmp_text);
                setSize(this->text.getSize());
            }
        }
    }
}

