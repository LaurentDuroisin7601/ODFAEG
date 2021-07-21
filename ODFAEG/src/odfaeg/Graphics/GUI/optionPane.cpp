#include "../../../../include/odfaeg/Graphics/GUI/optionPane.hpp"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            OptionPane::OptionPane(math::Vec3f position, math::Vec3f size, const Font* font, sf::String t, TYPE type) :
                rw (sf::VideoMode(size.x, size.y), "Option Pane", sf::Style::Default, window::ContextSettings(3, 0, 0, 0, 0)),
                LightComponent (rw, position, size, size * 0.5f),
                type(type) {
                rw.getView().move(size.x * 0.5f, size.y * 0.5f, size.z * 0.5f);
                if (type == CONFIRMATION_DIALOG) {
                    yes = new Button(math::Vec2f(size.x / 100 * 70, size.y - 100), math::Vec2f(100, 50), font, "Yes", 15, rw);
                    no = new Button(math::Vec2f(size.x / 100 * 10, size.y - 100), math::Vec2f(100, 50), font, "No", 15, rw);
                    core::FastDelegate<bool> trigger1(&Button::isMouseInButton, yes);
                    core::FastDelegate<bool> trigger2(&Button::isMouseInButton, no);
                    core::FastDelegate<void> slot1(&OptionPane::onYesOption, this);
                    core::FastDelegate<void> slot2(&OptionPane::onNoOption, this);
                    core::Action a (core::Action::MOUSE_BUTTON_PRESSED_ONCE,window::IMouse::Left);
                    core::Command cmd1 (a, trigger1, slot1);
                    core::Command cmd2 (a, trigger2, slot2);
                    getListener().connect("YESOPTION", cmd1);
                    getListener().connect("NOOPTION", cmd2);
                } else {
                    yes = nullptr;
                    no = nullptr;
                    core::Action a (core::Action::KEY_PRESSED_ONCE, window::IKeyboard::Return);
                    core::FastDelegate<void> slot(&OptionPane::onEnter, this);
                    core::Command cmd (a, slot);
                    getListener().connect("ONENTER", cmd);
                }
                rw.setPosition(sf::Vector2i(position.x, position.y));
                option = UNDEFINED;
                text.setFont(*font);
                text.setString(t.toAnsiString());
                text.setColor(sf::Color::Black);
                text.setPosition(math::Vec3f(10, 10, position.z));
                text.setSize(size);
                backgroundColor = sf::Color::White;
                rw.clear(backgroundColor);
            }
            void OptionPane::onVisibilityChanged(bool visible) {
                rw.setVisible(visible);
            }
            OptionPane::OPTION OptionPane::getOption() {
                OPTION choosenOption = option;
                option = UNDEFINED;
                return choosenOption;
            }
            void OptionPane::clear() {
                rw.clear(backgroundColor);
            }
            void OptionPane::onDraw(RenderTarget& target, RenderStates states) {
                if (rw.isOpen()) {
                    rw.draw(text, states);
                    if (type == CONFIRMATION_DIALOG)  {
                        rw.draw(*yes, states);
                        rw.draw(*no, states);
                    }
                    rw.display();
                }
            }
            void OptionPane::onEnter() {
                setVisible(false);
                setEventContextActivated(false);
                rw.setVisible(false);
            }
            void OptionPane::onYesOption() {
                rw.setVisible(false);
                setEventContextActivated(false);
                /*yes->setEventContextActivated(false);
                no->setEventContextActivated(false);*/
                option = YES_OPTION;
            }
            void OptionPane::onNoOption() {
                rw.setVisible(false);
                setEventContextActivated(false);
                /*yes->setEventContextActivated(false);
                no->setEventContextActivated(false);*/
                option = NO_OPTION;
            }
            void OptionPane::setText(std::string t) {
                text.setString(t);
            }
            void OptionPane::onEventPushed (window::IEvent event, RenderWindow& window) {
                if (event.type == window::IEvent::WINDOW_EVENT && event.window.type == window::IEvent::WINDOW_EVENT_CLOSED) {
                    rw.setVisible(false);
                }
                yes->onUpdate(&getWindow(), event);
                no->onUpdate(&getWindow(), event);
                getListener().pushEvent(event);
            }
            OptionPane::~OptionPane() {
                rw.close();
                if (CONFIRMATION_DIALOG) {
                    delete yes;
                    delete no;
                }
            }
        }
    }
}
