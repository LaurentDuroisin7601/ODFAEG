#include "../../../../include/odfaeg/Graphics/GUI/textArea.hpp"
#include "../../../../include/odfaeg/openGL.hpp"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            TextArea::TextArea(math::Vec3f position, math::Vec3f size, const Font* font, std::string t, RenderWindow& rw) :
                LightComponent(rw, position, size, math::Vec3f(0, 0, 0)) {
                this->font = font;
                caracterSize = 15;
                tmp_text = t;
                background = Color::White;
                text.setFont(*font);
                text.setString(t);
                text.setColor(Color::Black);
                text.setPosition(position);
                text.setCharacterSize(15);
                //text.setSize(math::Vec3f(size);
                rect = RectangleShape(size);
                /*rect.setOutlineThickness(5.f);
                rect.setOutlineColor(Color::Black);*/
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
                math::Vec2f pos = text.findCharacterPos(currentIndex);
                cursorPos = math::Vec3f(pos.x(), pos.y(), 0);
                //setSize(text.getSize());
                haveFocus = textChanged = viewUpdated = false;
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
                math::Vec2f mousePos = math::Vec2f(window::IMouse::getPosition(getWindow()).x(), window::IMouse::getPosition(getWindow()).y());
                currentIndex = text.findCharacterAt(mousePos);
                math::Vec2f pos = text.findCharacterPos(currentIndex);
                cursorPos = math::Vec3f(pos.x(), pos.y(), 0);
                text.setSelected(currentIndex, currentIndex);
                /*if (getName() == "TSCRIPTEDIT")
                    std::cout<<"index : "<<currentIndex<<std::endl<<"cursor pos : "<<pos<<std::endl;*/

            }
            void TextArea::setCursorPos2() {
                math::Vec2f mousePos = math::Vec2f(window::IMouse::getPosition(getWindow()).x(), window::IMouse::getPosition(getWindow()).y());
                currentIndex2 = text.findCharacterAt(mousePos);
                math::Vec2f pos = text.findCharacterPos(currentIndex2);
                cursorPos = math::Vec3f(pos.x(), pos.y(), 0);
                if (currentIndex2 > currentIndex) {
                    ////////std::cout<<"set selected text : "<<currentIndex<<","<<currentIndex2<<std::endl;
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
                /*if (getName() == "TSCRIPTEDIT")
                    std::cout<<"has focus : "<<haveFocus<<std::endl;*/
                onGaignedFocus();
            }
            void TextArea::lostFocus() {
                haveFocus = false;
                /*if (getName() == "TSCRIPTEDIT")
                    std::cout<<"has focus : "<<haveFocus<<std::endl;*/
                onLostFocus();
            }
            void TextArea::onGaignedFocus() {
            }
            void TextArea::onLostFocus() {
            }
            void TextArea::clear() {
                if (background != rect.getFillColor())
                    rect.setFillColor(background);
                if (views.size() > 0)
                    text.setColor(Color::Transparent);
            }
            void TextArea::setTextSize(unsigned int size) {
                caracterSize = size;
                text.setCharacterSize(size);
            }
            void TextArea::setTextColor(Color color) {
                text.setColor(color);
            }
            std::string TextArea::getText() {
                return tmp_text.toAnsiString();
            }
            math::Vec3f TextArea::getTextSize() {
                return text.getGlobalBounds().getSize();
            }
            void TextArea::onMove(math::Vec3f& t) {
                Transformable::onMove(t);
                cursorPos += math::Vec3f(t.x(), t.y(), 0);
                for (unsigned int i = 0; i < views.size(); i++) {
                    views[i].move(math::Vec3f(t.x(), t.y(), 0));
                }
                /*if (getName() == "TSCRIPTEDIT")
                    std::cout<<"index : "<<currentIndex<<std::endl<<"cursor pos : "<<cursorPos<<std::endl;*/
            }
            void TextArea::onDraw(RenderTarget& target, RenderStates states) {

                //math::Vec2f pos = text.findCharacterPos(currentIndex);
                /*if (name == "TSCRIPTEDIT")
                    std::cout<<"draw cursor pos : "<<cursorPos<<std::endl;*/
                //cursorPos = math::Vec3f(pos.x(), pos.y(), 0);
                VertexArray va(Lines);
                va.append(Vertex(math::Vec3f(cursorPos.x()+1, cursorPos.y(), getPosition().z()+300), Color::Black));
                va.append(Vertex(math::Vec3f(cursorPos.x()+1, cursorPos.y() + text.getCharacterSize(), getPosition().z()+300), Color::Black));
                rect.setPosition(getPosition());
                text.setPosition(math::Vec3f(getPosition().x() + scrollX, getPosition().y() + scrollY, getPosition().z()+100));


                rect.setSize(getSize());
                target.draw(rect);
                #ifndef VULKAN
                glCheck(glEnable(GL_SCISSOR_TEST));
                glCheck(glScissor(getPosition().x(), getWindow().getSize().y() - (getPosition().y() + getSize().y()), getSize().x(), getSize().y()));
                #endif
                #ifdef VULKAN
                target.submit(false);
                if (text.getString().getSize() != 0) {
                    if (getParent() != nullptr) {

                        target.getScissors()[1].offset = {(getPosition().x() < getParent()->getPosition().x()) ? getParent()->getPosition().x() : getPosition().x(), (getPosition().y() < getParent()->getPosition().y()) ? getParent()->getPosition().y() : getPosition().y()};
                        //std::cout<<"offsets : "<<target.getScissors()[0].offset.x<<","<<target.getScissors()[0].offset.y<<std::endl;
                        target.getScissors()[1].extent = {(getSize().x() > getParent()->getSize().x()) ? getParent()->getSize().x() : getSize().x(), (getSize().y() > getParent()->getSize().y()) ? getParent()->getSize().y() : getSize().y()};
                    } else {
                        target.getScissors()[1].offset = {getPosition().x(), getPosition().y()};
                        target.getScissors()[1].extent = {getSize().x(), getSize().y()};
                    }

                    #endif
                    target.draw(text);
                    if (viewUpdated) {
                        for (unsigned int i = 0; i < views.size(); i++) {
                            target.draw(views[i]);
                        }
                    }
                    #ifdef VULKAN
                    target.submit(false);
                    target.getScissors()[1].offset = {0, 0};
                    target.getScissors()[1].extent = {target.getSize().x(), target.getSize().y()};

                    #endif // VULKAN
                }
                //Il faut restaurer les paramètres d'avant si un scissor test a été défini avant de dessiner la TextArea.
                #ifndef VULKAN
                if (sctest == false) {
                    glCheck(glDisable(GL_SCISSOR_TEST));
                } else {
                    glCheck(glScissor(values[0], values[1], values[2], values[3]));
                }
                #endif
                if(haveFocus) {
                    target.draw(va);
                }
                #ifdef VULKAN
                if (haveFocus) {
                    target.submit(false);
                }
                #endif // VULKAN*/

            }
            bool TextArea::isMouseInTextArea() {
                physic::BoundingBox bb = getGlobalBounds();



                math::Vec3f mousePos = math::Vec3f(window::IMouse::getPosition(getWindow()).x(), window::IMouse::getPosition(getWindow()).y(), getPosition().z());
                /*if (name == "TSCRIPTEDIT")
                        std::cout<<"bb : "<<bb.getPosition()<<std::endl<<bb.getSize()<<std::endl<<"mouse pos : "<<mousePos<<std::endl<<bb.isPointInside(mousePos)<<std::endl;*/
                if (bb.isPointInside(mousePos)) {

                    return true;
                }
                return false;
            }
            bool TextArea::isMouseOutTextArea() {
                physic::BoundingBox bb = getGlobalBounds();
                math::Vec3f mousePos = math::Vec3f(window::IMouse::getPosition(getWindow()).x(), window::IMouse::getPosition(getWindow()).y(), getPosition().z());
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
                //if (getName() == "TSCRIPTEDIT") {

                //}
                if (tmp_text.getSize() > 0 && currentIndex-1 >= 0 && caracter == 8) {
                    //std::cout<<"erase"<<std::endl;
                    currentIndex--;
                    if (currentIndex < tmp_text.getSize()) {
                        tmp_text.erase(currentIndex, 1);
                    }
                }
                else if (caracter != 8) {
                    if (caracter == 13) {
                        tmp_text.insert(currentIndex, "\n");
                    } else {
                        //std::cout<<"insert : "<<currentIndex<<std::endl;
                        tmp_text.insert(currentIndex, core::String(caracter));
                        //std::cout<<"inserted : "<<currentIndex<<std::endl;
                    }
                    currentIndex++;
                }
                setText(tmp_text);
                math::Vec2f pos = text.findCharacterPos(currentIndex);
                cursorPos = math::Vec3f(pos.x(), pos.y(), 0);
                if (text.getGlobalBounds().getSize().x() > getSize().x()) {
                    scrollX = getSize().x() - text.getGlobalBounds().getSize().x();
                }
            }
            void TextArea::setText(std::string text) {
                tmp_text = text;
                this->text.setString(tmp_text);
                textChanged = true;
            }
            bool TextArea::isTextChanged() {
                bool b = textChanged;
                textChanged = false;
                return b;
            }
            unsigned int TextArea::getCharacterIndexAtCursorPos() {
                math::Vec2f pos = text.findCharacterPos(currentIndex);
                return text.findCharacterAt(pos);
            }
            math::Vec3f TextArea::getCursorPos() {
                return cursorPos;
            }
            unsigned int TextArea::getCharacterSize() {
                return text.getCharacterSize();
            }
            void TextArea::setCursorPosition(unsigned int currentIndex) {
                this->currentIndex = currentIndex;
                math::Vec2f pos = text.findCharacterPos(currentIndex);
                //std::cout<<"cursor pos : "<<pos<<std::endl;
                cursorPos = math::Vec3f(pos.x(), pos.y(), 0);
            }
            math::Vec3f TextArea::getCursorPositionLocal() {
                math::Vec2f pos = text.findCharacterPosLocal(currentIndex);
                return math::Vec3f(pos.x(), pos.y(), 0);
            }
            void TextArea::setFocus(bool focus) {
                haveFocus = focus;
            }
            void TextArea::resetTokens() {
                viewUpdated = false;
                tokens.clear();
                views.clear();
            }
            void TextArea::addToken(Token token) {
                tokens.push_back(token);
            }
            void TextArea::applySyntaxSuggar() {
                for (unsigned int i = 0; i < tokens.size(); i++) {
                    Text subText;
                    //std::cout<<"tokens : "<<tokens[i].startTok<<","<<tokens[i].endTok<<std::endl;
                    //std::cout<<"token text : "<<tmp_text.substring(tokens[i].startTok, tokens[i].endTok - tokens[i].startTok).toAnsiString()<<std::endl;
                    //subText.setString(core::String::fromUtf8(tokens[i].spelling.begin(), tokens[i].spelling.end()));
                    subText.setString(tmp_text.substring(tokens[i].startTok, tokens[i].endTok - tokens[i].startTok));
                    subText.setFont(*font);
                    subText.setColor(tokens[i].colorTok);
                    subText.setCharacterSize(caracterSize);
                    math::Vec2f position = text.findCharacterPos(tokens[i].startTok);
                    subText.setPosition(math::Vec3f(position.x(), position.y(), getPosition().z()+100));
                    views.push_back(subText);
                }
                viewUpdated = true;
            }
        }
    }
}
