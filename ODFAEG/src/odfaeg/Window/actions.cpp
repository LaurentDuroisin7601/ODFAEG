#include "../../../include/odfaeg/Window/action.h"
#include "../../../include/odfaeg/Window/command.h"
#include <SFML/Window.hpp>
#include <chrono>
#include <thread>
using namespace std;
namespace odfaeg {
    namespace core {
        Action::Action (EVENT_TYPE type) : type(type) {
            leaf = true;
            pressed = false;
            switch (type) {
                case CLOSED :
                    startEvent.type = window::IEvent::WINDOW_EVENT;
                    startEvent.window.type = window::IEvent::WINDOW_EVENT_CLOSED;
                    break;
                case RESIZED :
                    startEvent.type = window::IEvent::WINDOW_EVENT;
                    startEvent.window.type = window::IEvent::WINDOW_EVENT_RESIZED;
                    break;
                case LOST_FOCUS :
                    startEvent.type = window::IEvent::WINDOW_EVENT;
                    startEvent.window.type = window::IEvent::WINDOW_EVENT_FOCUS_LOST;
                    break;
                case GAIGNED_FOCUS :
                    startEvent.type = window::IEvent::WINDOW_EVENT;
                    startEvent.window.type = window::IEvent::WINDOW_EVENT_FOCUS_GAIGNED;
                    break;
                case TEXT_ENTERED :
                    startEvent.type = window::IEvent::TEXT_INPUT_EVENT;
                    startEvent.text.type = window::IEvent::TEXT_INPUT_EVENT;
                    startEvent.text.unicode = 0;
                    break;
                case MOUSE_WHEEL_MOVED :
                    startEvent.type = window::IEvent::MOUSE_WHEEL_EVENT;
                    break;
                case MOUSE_MOVED_ :
                    startEvent.type = window::IEvent::MOUSE_MOTION_EVENT;
                    break;
                case MOUSE_ENTERED :
                    startEvent.type = window::IEvent::MOUSE_EVENT_ENTER;
                    break;
                case MOUSE_LEFT :
                    startEvent.type = window::IEvent::MOUSE_EVENT_LEAVE;
                    break;
                default :
                    throw Erreur (1, "Invalid type!", 2);
            }
            is_not = false;
        }
        Action::Action (EVENT_TYPE type, window::IKeyboard::Key key) : type(type) {
            leaf = true;
            pressed = false;
            switch (type) {
            case KEY_PRESSED_ONCE :
                startEvent.type = window::IEvent::KEYBOARD_EVENT;
                startEvent.keyboard.type = window::IEvent::KEY_EVENT_PRESSED;
                startEvent.keyboard.code = key;
                break;
            case KEY_HELD_DOWN :
                startEvent.type = window::IEvent::KEYBOARD_EVENT;
                startEvent.keyboard.type = window::IEvent::KEY_EVENT_PRESSED;
                startEvent.keyboard.code = key;
                break;
            case KEY_RELEASED :
                startEvent.type = window::IEvent::KEYBOARD_EVENT;
                startEvent.keyboard.type = window::IEvent::KEY_EVENT_RELEASED;
                startEvent.keyboard.code = key;
                break;
            default :
                throw Erreur (1, "Invalid type!", 2);
            }
            is_not = false;
        }
        Action::Action (EVENT_TYPE type, window::IMouse::Button button) : type(type) {
            leaf = true;
            pressed = false;
            switch (type) {
                case MOUSE_BUTTON_PRESSED_ONCE :
                    startEvent.type = window::IEvent::MOUSE_BUTTON_EVENT;
                    startEvent.mouseButton.type = window::IEvent::BUTTON_EVENT_PRESSED;
                    startEvent.mouseButton.button = button;
                    break;
                case MOUSE_BUTTON_HELD_DOWN :
                    startEvent.type = window::IEvent::MOUSE_BUTTON_EVENT;
                    startEvent.mouseButton.type = window::IEvent::BUTTON_EVENT_PRESSED;
                    startEvent.mouseButton.button = button;
                    break;
                case MOUSE_BUTTON_RELEASED :
                    startEvent.type = window::IEvent::MOUSE_BUTTON_EVENT;
                    startEvent.mouseButton.type = window::IEvent::BUTTON_EVENT_RELEASED;
                    startEvent.mouseButton.button = button;
                    break;
                default :
                    throw Erreur (1, "Invalid type!", 2);
            }
            is_not = false;
        }
        Action::Action(const Action& other) {
            leaf = other.leaf;
            pressed = other.pressed;
            startEvent = other.startEvent;
            is_not = other.is_not;
            comparator = other.comparator;
            type = other.type;
            name = other.name;
            if (!other.leaf) {
                leftChild = std::make_unique<Action>(*other.leftChild);
                rightChild = std::make_unique<Action>(*other.rightChild);
            }
        }
        bool Action::andComparator (Action& a1, Action& a2) {
            /*if (name == "SkillCombined")
                std::cout<<"skill combined"<<std::endl;*/
            return a1.isTriggered() && a2.isTriggered();
        }
        bool Action::orComparator (Action& a1, Action& a2) {
            return a1.isTriggered() || a2.isTriggered();
        }
        bool Action::xorComparator (Action& a1, Action& a2) {
            return a1.isTriggered() | a2.isTriggered();
        }
        void Action::operator! () {
            if (!is_not)
                is_not = true;
            else
                is_not = false;
        }
        Action Action::operator| (Action other) {
            Action result (COMBINED_WITH_XOR, *this, other);
            return result;
        }
        Action Action::operator|| (Action other) {
            Action result (COMBINED_WITH_OR, *this, other);
            return result;

        }
        Action Action::operator&& (Action other) {
            Action result (COMBINED_WITH_AND, *this, other);
            return result;
        }
        bool Action::isTriggered () {

            if (!leaf) {
                return comparator(this, std::ref(*leftChild), std::ref(*rightChild));
            } else {
                /*if (startEvent.type == window::IEvent::TextEntered)
                    std::cout<<"text entered : "<<startEvent.text.unicode<<std::endl;*/
                //std::cout<<"triggered"<<std::endl;

                if (type == KEY_HELD_DOWN && !is_not) {
                    return window::IKeyboard::isKeyPressed(static_cast<window::IKeyboard::Key>(startEvent.keyboard.code));
                }
                if (type == KEY_HELD_DOWN && is_not) {
                    return !window::IKeyboard::isKeyPressed(static_cast<window::IKeyboard::Key>(startEvent.keyboard.code));
                }
                if (type == MOUSE_BUTTON_HELD_DOWN && !is_not) {
                    /*if (name == "SkillButtonHeldDown")
                        std::cout<<"skill move mouse button held down ? "<<window::IMouse::isButtonPressed(static_cast<window::IMouse::Button>(startEvent.mouseButton.button))<<std::endl;*/
                    return window::IMouse::isButtonPressed(static_cast<window::IMouse::Button>(startEvent.mouseButton.button));
                }
                if (type == MOUSE_BUTTON_HELD_DOWN && is_not) {
                    return !window::IMouse::isButtonPressed(static_cast<window::IMouse::Button>(startEvent.mouseButton.button));
                }
                /*else if (type == JOYSTICK_BUTTON_HELD_DOWN)
                    return sf::Joystick::isButtonPressed(startEvent.joystickButton.button);*/

                //vector<window::IEvent> events = Command::getEvents();
                /*if (name == "SkillMouseMoved")
                                std::cout<<"events ? "<<events.size()<<std::endl;*/
                for (unsigned int i = 0; i < events.size(); i++) {

                    /*if (type == KEY_HELD_DOWN || type == MOUSE_BUTTON_HELD_DOWN) {
                        if (!is_not)
                            return Command::equalEvent(events[i], startEvent);
                        else
                            return !Command::equalEvent(events[i], startEvent);
                    } else {*/
                        /*Sometimes the event stored to startEvent is deleted*/
                        /*if (name == "ANAMECTEXTENTERED" && events[i].type == window::IEvent::EventType::TEXT_INPUT_EVENT)
                            std::cout<<"text input event ta name"<<std::endl;*/
                        //std::cout<<"test event"<<std::endl;
                        if (!is_not && Command::equalEvent(events[i], startEvent) && !pressed) {
                            if (events[i].type == window::IEvent::KEYBOARD_EVENT && events[i].keyboard.type == window::IEvent::KEY_EVENT_PRESSED
                                && startEvent.type == window::IEvent::KEYBOARD_EVENT && startEvent.keyboard.type == window::IEvent::KEY_EVENT_PRESSED
                                || events[i].type == window::IEvent::MOUSE_BUTTON_EVENT && events[i].mouseButton.type == window::IEvent::BUTTON_EVENT_PRESSED
                                && startEvent.type == window::IEvent::MOUSE_BUTTON_EVENT && startEvent.mouseButton.type == window::IEvent::BUTTON_EVENT_PRESSED)
                                pressed = true;
                            if (name == "SkillMouseMoved")
                                std::cout<<"skill move mouse move triggered!"<<std::endl;
                            return true;
                        } else if (is_not && !Command::equalEvent(events[i], startEvent) && !pressed) {
                            if (events[i].type == window::IEvent::KEYBOARD_EVENT && events[i].keyboard.type == window::IEvent::KEY_EVENT_RELEASED
                                && startEvent.type == window::IEvent::KEYBOARD_EVENT && startEvent.keyboard.type == window::IEvent::KEY_EVENT_RELEASED
                                || events[i].type == window::IEvent::MOUSE_BUTTON_EVENT && events[i].mouseButton.type == window::IEvent::BUTTON_EVENT_RELEASED
                                && startEvent.type == window::IEvent::MOUSE_BUTTON_EVENT && startEvent.mouseButton.type == window::IEvent::BUTTON_EVENT_RELEASED)
                                pressed = true;
                            return true;
                        }
                    //}
                }
                return false;

            }
        }
        Action::Action (EVENT_TYPE type, Action leftChild, Action rightChild)  {
            leaf = false;
            this->type = type;
            is_not = false;
            pressed = false;
            if (type == COMBINED_WITH_AND) {
                comparator = &Action::andComparator;
            } else if (type == COMBINED_WITH_OR) {
                comparator = &Action::orComparator;
            } else {
                comparator = &Action::xorComparator;
            }
            this->leftChild = std::make_unique<Action>(leftChild);
            this->rightChild = std::make_unique<Action>(rightChild);


        }

        void Action::setPressed(window::IEvent event) {
            if (!leaf) {
                leftChild->setPressed(event);
                rightChild->setPressed(event);
            } else {
                if ((type == KEY_PRESSED_ONCE && event.type == window::IEvent::KEYBOARD_EVENT && event.keyboard.type == window::IEvent::KEY_EVENT_PRESSED && event.keyboard.code == startEvent.keyboard.code) ||
                    (type == MOUSE_BUTTON_PRESSED_ONCE && event.type == window::IEvent::MOUSE_BUTTON_EVENT && event.mouseButton.type == window::IEvent::BUTTON_EVENT_PRESSED && event.mouseButton.button == startEvent.mouseButton.button)) {
                     pressed = false;
                }
            }
        }
        void Action::getActions(std::vector<Action*>& actions) {
            if (!leaf) {
                leftChild->getActions(actions);
                rightChild->getActions(actions);
            } else {
                actions.push_back(this);
            }
        }
        bool Action::containsEvent(window::IEvent& event) {
            if (!leaf) {
                return leftChild->containsEvent(event) || rightChild->containsEvent(event);
            } else {
                return Command::equalEvent(event, startEvent);
            }
        }
        void Action::getEvents(std::vector<window::IEvent>& events) {
            if (!leaf) {
                leftChild->getEvents(events);
                rightChild->getEvents(events);
            } else {
                events.push_back(startEvent);
            }
        }
        Action& Action::operator=(const Action& other) {
            leaf = other.leaf;
            pressed = other.pressed;
            startEvent = other.startEvent;
            is_not = other.is_not;
            comparator = other.comparator;
            type = other.type;
            if (!leaf) {
                leftChild = std::make_unique<Action>(*other.leftChild);
                rightChild = std::make_unique<Action>(*other.rightChild);
            }
            return *this;
        }
        void Action::pushEvent(window::IEvent& event) {
            std::vector<window::IEvent>::iterator it;
            bool containsEvent = false;
            for (it = events.begin(); it != events.end(); it++)
            {
                if (Command::equalEvent(event, *it))
                    containsEvent = true;
            }
            if (!containsEvent) {
                events.push_back(event);
            }
            if (!leaf) {
                leftChild->pushEvent(event);
                rightChild->pushEvent(event);
            }
        }
        void Action::clearEvents() {
            events.clear();
            if (!leaf) {
                leftChild->clearEvents();
                rightChild->clearEvents();
            }
        }
        void Action::removeEvent(window::IEvent& event) {
            std::vector<window::IEvent>::iterator it;
            for (it = events.begin(); it != events.end();) {
                if (Command::equalEvent(*it, event))
                    it = events.erase(it);
                else
                    it++;
            }
        }
    }
}

