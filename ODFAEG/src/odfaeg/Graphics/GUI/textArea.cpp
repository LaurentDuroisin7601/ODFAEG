#include "../../../../include/odfaeg/Graphics/GUI/textArea.hpp"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            TextArea::TextArea(math::Vec3f position, math::Vec3f size, const Font* font, std::string t, RenderWindow& rw) :
                LightComponent(rw, position, size, math::Vec3f(0, 0, 0)) {
                tmp_text = t;
                background = sf::Color::White;
                text.setFont(*font);
                text.setString(t);
                text.setColor(sf::Color::Black);
                text.setPosition(position);
                //text.setSize(math::Vec3f(size);
                rect = RectangleShape(size);
                /*rect.setOutlineThickness(5.f);
                rect.setOutlineColor(sf::Color::Black);*/
                core::Action a2 (core::Action::MOUSE_BUTTON_PRESSED_ONCE, window::IMouse::Left);
                core::Command cmd2(a2, core::FastDelegate<bool>(&TextArea::isMouseInTextArea, this), core::FastDelegate<void>(&TextArea::gaignedFocus, this));
                core::Command cmd3(a2, core::FastDelegate<bool>(&TextArea::isMouseOutTextArea, this), core::FastDelegate<void>(&TextArea::lostFocus, this));
                getListener().connect("CGFOCUS", cmd2);
                getListener().connect("CLFOCUS", cmd3);
                core::Action a3 (core::Action::MOUSE_BUTTON_PRESSED_ONCE, window::IMouse::Left);
                core::Command cmd4(a3, core::FastDelegate<bool>(&TextArea::isMouseInTextArea, this), core::FastDelegate<void>(&TextArea::setCursorPos, this));
                getListener().connect("CMOUSECLICKED", cmd4);
                core::Action a4 (core::Action::TEXT_ENTERED);
                core::Command cmd5(a4, core::FastDelegate<bool>(&TextArea::hasFocus, this), core::FastDelegate<void>(&TextArea::onTextEntered, this, 'a'));
                getListener().connect("CTEXTENTERED", cmd5);
                core::Action a5(core::Action::MOUSE_MOVED_);
                core::Action a6(core::Action::MOUSE_BUTTON_HELD_DOWN, window::IMouse::Left);
                core::Action a7 = a5 && a6;
                core::Command cmd6(a7, core::FastDelegate<bool>(&TextArea::isMouseInTextArea, this), core::FastDelegate<void>(&TextArea::setCursorPos2, this));
                getListener().connect("CMOUSEMOVED", cmd6);
                core::Action a8(core::Action::MOUSE_BUTTON_RELEASED, window::IMouse::Left);
                core::Command cmd7(a8, core::FastDelegate<bool>(&TextArea::isMouseInTextArea, this), core::FastDelegate<void>(&TextArea::setSelectedText, this));
                getListener().connect("CSELECTTEXT", cmd7);
                currentIndex = currentIndex2 = tmp_text.getSize();
                sf::Vector2f pos = text.findCharacterPos(currentIndex);
                cursorPos = math::Vec3f(pos.x, pos.y, 0);
                //setSize(text.getSize());
                haveFocus = textChanged = false;
                scrollX = scrollY = 0;
            }
            void TextArea::onEventPushed(window::IEvent event, RenderWindow& window) {
                if (&window == &getWindow()) {
                    getListener().pushEvent(event);
                }
            }
            bool TextArea::hasFocus() {
                return haveFocus;
            }
            void TextArea::setCursorPos() {
                math::Vec2f mousePos = math::Vec2f(window::IMouse::getPosition(getWindow()).x, window::IMouse::getPosition(getWindow()).y);
                bool found = false;
                for (unsigned int i = 0; i < tmp_text.getSize() && !found; i++) {
                    sf::Vector2f pos = text.findCharacterPos(i);
                    if (pos.x > mousePos.x && pos.y > mousePos.y - text.getCharacterSize()) {
                        cursorPos = math::Vec3f(pos.x, pos.y, 0);
                        currentIndex = i;
                        found = true;
                    }
                }
                if (!found) {
                    currentIndex = tmp_text.getSize();
                    sf::Vector2f pos = text.findCharacterPos(currentIndex);
                    cursorPos = math::Vec3f(pos.x, pos.y, 0);
                }
                text.setSelected(currentIndex, currentIndex);
            }
            void TextArea::setCursorPos2() {
                math::Vec2f mousePos = math::Vec2f(window::IMouse::getPosition(getWindow()).x, window::IMouse::getPosition(getWindow()).y);
                bool found = false;
                for (unsigned int i = 0; i < tmp_text.getSize() && !found; i++) {
                    sf::Vector2f pos = text.findCharacterPos(i);
                    if (pos.x > mousePos.x && pos.y > mousePos.y - text.getCharacterSize()) {
                        cursorPos = math::Vec3f(pos.x, pos.y, 0);
                        currentIndex2 = i;
                        found = true;
                    }
                }
                if (currentIndex2 > currentIndex) {
                    //std::cout<<"set selected text : "<<currentIndex<<","<<currentIndex2<<std::endl;
                    text.setSelected(currentIndex, currentIndex2);
                }
            }
            std::string TextArea::getSelectedText() {
                return selected_text.toAnsiString();
            }
            void TextArea::setSelectedText() {
                if (currentIndex2 > currentIndex) {
                    selected_text = tmp_text.substring(currentIndex, currentIndex2 - currentIndex);
                }
            }
            void TextArea::gaignedFocus() {
                haveFocus = true;
                onGaignedFocus();
            }
            void TextArea::lostFocus() {
                haveFocus = false;
                onLostFocus();
            }
            void TextArea::onGaignedFocus() {
            }
            void TextArea::onLostFocus() {
            }
            void TextArea::clear() {
                if (background != rect.getFillColor())
                    rect.setFillColor(background);
            }
            void TextArea::setTextSize(unsigned int size) {
                text.setCharacterSize(size);
            }
            void TextArea::setTextColor(sf::Color color) {
                text.setColor(color);
            }
            std::string TextArea::getText() {
                return tmp_text.toAnsiString();
            }
            math::Vec3f TextArea::getTextSize() {
                return text.getGlobalBounds().getSize();
            }
            void TextArea::onDraw(RenderTarget& target, RenderStates states) {
                VertexArray va(sf::Lines);
                va.append(Vertex(sf::Vector3f(cursorPos.x, cursorPos.y, 0), sf::Color::Black));
                va.append(Vertex(sf::Vector3f(cursorPos.x, cursorPos.y + text.getCharacterSize(), 0), sf::Color::Black));
                rect.setPosition(getPosition());
                text.setPosition(math::Vec3f(getPosition().x + scrollX, getPosition().y + scrollY, 0));
                rect.setSize(getSize());

                target.draw(rect);
                #ifndef VULKAN
                GLboolean sctest;
                glCheck(glGetBooleanv(GL_SCISSOR_TEST, &sctest));
                GLint values[4];
                glCheck(glGetIntegerv(GL_SCISSOR_BOX, values));
                glCheck(glEnable(GL_SCISSOR_TEST));
                glCheck(glScissor(getPosition().x, getWindow().getSize().y - (getPosition().y + getSize().y), getSize().x, getSize().y));
                #endif
                target.draw(text);
                //Il faut restaurer les paramètres d'avant si un scissor test a été défini avant de dessiner la TextArea.
                if (sctest == false) {
                    #ifndef VULKAN
                    glCheck(glDisable(GL_SCISSOR_TEST));
                    #endif
                } else {
                    #ifndef VULKAN
                    glCheck(glScissor(values[0], values[1], values[2], values[3]));
                    #endif
                }
                if(haveFocus)
                    target.draw(va);
            }
            bool TextArea::isMouseInTextArea() {
                physic::BoundingBox bb = getGlobalBounds();
                math::Vec2f mousePos = math::Vec2f(window::IMouse::getPosition(getWindow()).x, window::IMouse::getPosition(getWindow()).y);
                if (bb.isPointInside(mousePos)) {

                    return true;
                }
                return false;
            }
            bool TextArea::isMouseOutTextArea() {
                physic::BoundingBox bb = getGlobalBounds();
                math::Vec2f mousePos = math::Vec2f(window::IMouse::getPosition(getWindow()).x, window::IMouse::getPosition(getWindow()).y);
                if (bb.isPointInside(mousePos)) {
                    return false;
                }
                return true;
            }
            void TextArea::onUpdate(RenderWindow* window, window::IEvent& event) {
                if (window == &getWindow() && event.type == window::IEvent::TEXT_INPUT_EVENT) {
                    getListener().setCommandSlotParams("CTEXTENTERED", this, static_cast<char>(event.text.unicode));
                }
            }
            void TextArea::onTextEntered(char caracter) {
                if (getName() == "TANAME") {
                    getListener().name = "TANAMELISTENER";
                }
                if (tmp_text.getSize() > 0 && currentIndex-1 >= 0 && caracter == 8) {
                    currentIndex--;
                    tmp_text.erase(currentIndex, 1);
                }
                else if (caracter != 8) {
                    if (caracter == 13) {
                        tmp_text.insert(currentIndex, "\n");
                    } else {
                        tmp_text.insert(currentIndex, sf::String(caracter));
                    }
                    currentIndex++;
                }
                setText(tmp_text);
                sf::Vector2f pos = text.findCharacterPos(currentIndex);
                cursorPos = math::Vec3f(pos.x, pos.y, 0);
                if (text.getGlobalBounds().getSize().x > getSize().x) {
                    scrollX = getSize().x - text.getGlobalBounds().getSize().x;
                }
            }
            void TextArea::setText(std::string text) {
                tmp_text = text;
                this->text.setString(text);
                textChanged = true;
            }
            bool TextArea::isTextChanged() {
                bool b = textChanged;
                textChanged = false;
                return b;
            }
        }
    }
}
