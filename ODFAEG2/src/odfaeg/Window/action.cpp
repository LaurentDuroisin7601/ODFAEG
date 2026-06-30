module;
#include <stdexcept>
#include <memory>
#include <vector>
import odfaeg.window.action;
module odfaeg.window.action;
import odfaeg.window.iEvent;
import odfaeg.window.iKeyboard;
import odfaeg.window.iMouse;
import odfaeg.window.command;
namespace odfaeg {
    namespace window {
        Action::Action(EVENT_TYPE type) : type(type) {
            leaf = true;
            pressed = false;
            switch (type) {
            case CLOSED:
                startEvent.type = IEvent::WINDOW_EVENT;
                startEvent.window.type = IEvent::WINDOW_EVENT_CLOSED;
                break;
            case RESIZED:
                startEvent.type = IEvent::WINDOW_EVENT;
                startEvent.window.type = IEvent::WINDOW_EVENT_RESIZED;
                break;
            case LOST_FOCUS:
                startEvent.type = IEvent::WINDOW_EVENT;
                startEvent.window.type = IEvent::WINDOW_EVENT_FOCUS_LOST;
                break;
            case GAIGNED_FOCUS:
                startEvent.type = IEvent::WINDOW_EVENT;
                startEvent.window.type = IEvent::WINDOW_EVENT_FOCUS_GAIGNED;
                break;
            case TEXT_ENTERED:
                startEvent.type = IEvent::TEXT_INPUT_EVENT;
                startEvent.text.type = IEvent::TEXT_INPUT_EVENT;
                startEvent.text.unicode = 0;
                break;
            case MOUSE_WHEEL_MOVED:
                startEvent.type = IEvent::MOUSE_WHEEL_EVENT;
                break;
            case MOUSE_MOVED_:
                startEvent.type = IEvent::MOUSE_MOTION_EVENT;
                break;
            case MOUSE_ENTERED:
                startEvent.type = IEvent::MOUSE_EVENT_ENTER;
                break;
            case MOUSE_LEFT:
                startEvent.type = IEvent::MOUSE_EVENT_LEAVE;
                break;
            default:
                throw std::runtime_error("Invalid type!");
            }
            is_not = false;
        }
        Action::Action(EVENT_TYPE type, window::IKeyboard::Key key) : type(type) {
            leaf = true;
            pressed = false;
            switch (type) {
            case KEY_PRESSED_ONCE:
                startEvent.type = IEvent::KEYBOARD_EVENT;
                startEvent.keyboard.type = IEvent::KEY_EVENT_PRESSED;
                startEvent.keyboard.code = key;
                break;
            case KEY_HELD_DOWN:
                startEvent.type = IEvent::KEYBOARD_EVENT;
                startEvent.keyboard.type = IEvent::KEY_EVENT_PRESSED;
                startEvent.keyboard.code = key;
                break;
            case KEY_RELEASED:
                startEvent.type = IEvent::KEYBOARD_EVENT;
                startEvent.keyboard.type = IEvent::KEY_EVENT_RELEASED;
                startEvent.keyboard.code = key;
                break;
            default:
                throw std::runtime_error("Invalid type!");
            }
            is_not = false;
        }
        Action::Action(EVENT_TYPE type, IMouse::Button button) : type(type) {
            leaf = true;
            pressed = false;
            switch (type) {
            case MOUSE_BUTTON_PRESSED_ONCE:
                startEvent.type = IEvent::MOUSE_BUTTON_EVENT;
                startEvent.mouseButton.type = IEvent::BUTTON_EVENT_PRESSED;
                startEvent.mouseButton.button = button;
                break;
            case MOUSE_BUTTON_HELD_DOWN:
                startEvent.type = IEvent::MOUSE_BUTTON_EVENT;
                startEvent.mouseButton.type = IEvent::BUTTON_EVENT_PRESSED;
                startEvent.mouseButton.button = button;
                break;
            case MOUSE_BUTTON_RELEASED:
                startEvent.type = IEvent::MOUSE_BUTTON_EVENT;
                startEvent.mouseButton.type = IEvent::BUTTON_EVENT_RELEASED;
                startEvent.mouseButton.button = button;
                break;
            default:
                throw std::runtime_error("Invalid type!");
            }
            is_not = false;
        }
        Action::Action(EVENT_TYPE type, Action leftChild, Action rightChild) {
            leaf = false;
            this->type = type;
            is_not = false;
            pressed = false;
            if (type == COMBINED_WITH_AND) {
                comparator = &Action::andComparator;
            }
            else if (type == COMBINED_WITH_OR) {
                comparator = &Action::orComparator;
            }
            else {
                comparator = &Action::xorComparator;
            }
            this->leftChild = std::make_unique<Action>(leftChild);
            this->rightChild = std::make_unique<Action>(rightChild);


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
        bool Action::andComparator(Action& a1, Action& a2) {            
            return a1.isTriggered() && a2.isTriggered();
        }
        bool Action::orComparator(Action& a1, Action& a2) {
            return a1.isTriggered() || a2.isTriggered();
        }
        bool Action::xorComparator(Action& a1, Action& a2) {
            return a1.isTriggered() | a2.isTriggered();
        }
        void Action::operator! () {
            if (!is_not)
                is_not = true;
            else
                is_not = false;
        }
        Action Action::operator| (Action other) {
            Action result(COMBINED_WITH_XOR, *this, other);
            return result;
        }
        Action Action::operator|| (Action other) {
            Action result(COMBINED_WITH_OR, *this, other);
            return result;

        }
        Action Action::operator&& (Action other) {
            Action result(COMBINED_WITH_AND, *this, other);
            return result;
        }
        bool Action::isTriggered() {

            if (!leaf) {
                return comparator(this, std::ref(*leftChild), std::ref(*rightChild));
            }
            else {

                if (type == KEY_HELD_DOWN && !is_not) {
                    return IKeyboard::isKeyPressed(static_cast<IKeyboard::Key>(startEvent.keyboard.code));
                }
                if (type == KEY_HELD_DOWN && is_not) {
                    return !IKeyboard::isKeyPressed(static_cast<IKeyboard::Key>(startEvent.keyboard.code));
                }
                if (type == MOUSE_BUTTON_HELD_DOWN && !is_not) {
                    return IMouse::isButtonPressed(static_cast<IMouse::Button>(startEvent.mouseButton.button));
                }
                if (type == MOUSE_BUTTON_HELD_DOWN && is_not) {
                    return !IMouse::isButtonPressed(static_cast<IMouse::Button>(startEvent.mouseButton.button));

                    for (unsigned int i = 0; i < events.size(); i++) {

                        if (!is_not && Command::equalEvent(events[i], startEvent) && !pressed) {
                            if (events[i].type == IEvent::KEYBOARD_EVENT && events[i].keyboard.type == IEvent::KEY_EVENT_PRESSED
                                && startEvent.type == IEvent::KEYBOARD_EVENT && startEvent.keyboard.type == IEvent::KEY_EVENT_PRESSED
                                || events[i].type == IEvent::MOUSE_BUTTON_EVENT && events[i].mouseButton.type == IEvent::BUTTON_EVENT_PRESSED
                                && startEvent.type == IEvent::MOUSE_BUTTON_EVENT && startEvent.mouseButton.type == IEvent::BUTTON_EVENT_PRESSED)
                                pressed = true;
                            return true;
                        }
                        else if (is_not && !Command::equalEvent(events[i], startEvent) && !pressed) {
                            if (events[i].type == IEvent::KEYBOARD_EVENT && events[i].keyboard.type == IEvent::KEY_EVENT_RELEASED
                                && startEvent.type == IEvent::KEYBOARD_EVENT && startEvent.keyboard.type == IEvent::KEY_EVENT_RELEASED
                                || events[i].type == IEvent::MOUSE_BUTTON_EVENT && events[i].mouseButton.type == IEvent::BUTTON_EVENT_RELEASED
                                && startEvent.type == IEvent::MOUSE_BUTTON_EVENT && startEvent.mouseButton.type == IEvent::BUTTON_EVENT_RELEASED)
                                pressed = true;
                            return true;
                        }
                    }
                    return false;

                }
            }
        }
        void Action::setPressed(IEvent event) {
            if (!leaf) {
                leftChild->setPressed(event);
                rightChild->setPressed(event);
            }
            else {
                if ((type == KEY_PRESSED_ONCE && event.type == window::IEvent::KEYBOARD_EVENT && event.keyboard.type == window::IEvent::KEY_EVENT_PRESSED && event.keyboard.code == startEvent.keyboard.code) ||
                    (type == MOUSE_BUTTON_PRESSED_ONCE && event.type == window::IEvent::MOUSE_BUTTON_EVENT && event.mouseButton.type == window::IEvent::BUTTON_EVENT_PRESSED && event.mouseButton.button == startEvent.mouseButton.button)) {
                    pressed = false;
                }
            }
        }
        void Action::setPressed(bool pressed) {
            if (!leaf) {
                leftChild->setPressed(pressed);
                rightChild->setPressed(pressed);
            }
            else {
                this->pressed = pressed;
            }
		}
        bool Action::isPressed() {
            if (!leaf) {
                return leftChild->isPressed() || rightChild->isPressed();
            }
            else {
                return pressed;
            }
		}
        void Action::getActions(std::vector<Action*>& actions) {
            if (!leaf) {
                leftChild->getActions(actions);
                rightChild->getActions(actions);
            }
            else {
                actions.push_back(this);
            }
        }
        bool Action::containsEvent(IEvent& event) {
            if (!leaf) {
                return leftChild->containsEvent(event) || rightChild->containsEvent(event);
            }
            else {
                return Command::equalEvent(event, startEvent);
            }
        }        
        void Action::getEvents(std::vector<IEvent>& events) {
            if (!leaf) {
                leftChild->getEvents(events);
                rightChild->getEvents(events);
            }
            else {
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
        void Action::pushEvent(IEvent& event) {
            std::vector<IEvent>::iterator it;
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
        void Action::removeEvent(IEvent& event) {
            std::vector<IEvent>::iterator it;
            for (it = events.begin(); it != events.end();) {
                if (Command::equalEvent(*it, event))
                    it = events.erase(it);
                else
                    it++;
            }
        }
    }
}