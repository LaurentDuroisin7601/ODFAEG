module;
#include <memory>
#include <string>
//import odfaeg.window.command;
module odfaeg.window.command;
import odfaeg.core.delegate;
import odfaeg.window.action;
import odfaeg.window.iEvent;
import odfaeg.window.iKeyboard;
namespace odfaeg
{
    namespace window {
        Command::Command(Action action, core::FastDelegate<void> slot) : slot(slot)
        {
            this->action = std::make_unique<Action>(action);
            this->trigger = nullptr;
        }
        void Command::setName(std::string name) {
            this->name = name;
        }
        Command::Command(Action action, core::FastDelegate<bool> trigger, core::FastDelegate<void> slot) : slot(slot) {
            this->action = std::make_unique<Action>(action);
            this->trigger = std::make_unique<core::FastDelegate<bool>>(trigger);
        }
        Command::Command(core::FastDelegate<bool> trigger, core::FastDelegate<void> slot) : slot(slot) {
            this->trigger = std::make_unique<core::FastDelegate<bool>>(trigger);
        }
        Command::Command(const Command& other) : slot(other.slot) {
            if (other.action != nullptr) {
                action = std::make_unique<Action>(*other.action);
            }
            if (other.trigger != nullptr)
                trigger = std::make_unique <core::FastDelegate<bool >> (*other.trigger);
            name = other.name;
        }
        

        bool Command::isTriggered()
        {            
            if (trigger == nullptr && action != nullptr)
            {                
                return action->isTriggered();
            }
            if (trigger != nullptr && action != nullptr) {
                return (*trigger)() && action->isTriggered();
            }
            if (trigger != nullptr && action == nullptr) {
                return (*trigger)();
            }
            return false;
        }

        bool Command::containsBufferEvent(window::IEvent& event) {
            if (action != nullptr)
                return action->containsEvent(event);
            return false;
        }

        void Command::clearEventsStack()
        {
            if (action != nullptr)
                action->clearEvents(); 
        }

        void Command::pushEvent(window::IEvent& event)
        {
            if (action != nullptr)
                action->pushEvent(event);            
        }
        Action* Command::getAction() {
            return action.get();
        }
        void Command::operator()()
        {
            slot();
        }

        void Command::removeEvent(IEvent& event) {            
            if (action != nullptr)
                action->removeEvent(event);
        }
        bool Command::equalEvent(IEvent event, IEvent other) {
            if (event.type != other.type) {
                return false;
            }
            if (event.type == window::IEvent::EventType::TEXT_INPUT_EVENT) {
                if (other.text.unicode == 0) {
                    return true;
                }
                return event.text.unicode == other.text.unicode;
            }
            if (event.type == window::IEvent::KEYBOARD_EVENT && event.keyboard.type == window::IEvent::KEY_EVENT_PRESSED && other.keyboard.type == window::IEvent::KEY_EVENT_PRESSED
                || event.type == window::IEvent::KEYBOARD_EVENT && event.keyboard.type == window::IEvent::KEY_EVENT_RELEASED && other.keyboard.type == window::IEvent::KEY_EVENT_RELEASED) {
                if (event.keyboard.code == IKeyboard::Unknown)
                    return true;
                return event.keyboard.code == other.keyboard.code;
            }
            else if (event.type == window::IEvent::KEYBOARD_EVENT) {
                return false;
            }
            if (event.type == window::IEvent::MOUSE_BUTTON_EVENT && event.mouseButton.type == window::IEvent::BUTTON_EVENT_PRESSED && other.mouseButton.type == window::IEvent::BUTTON_EVENT_PRESSED
                || event.type == window::IEvent::MOUSE_BUTTON_EVENT && event.mouseButton.type == window::IEvent::BUTTON_EVENT_RELEASED && other.mouseButton.type == window::IEvent::BUTTON_EVENT_RELEASED) {
                if (event.mouseButton.button == -1)
                    return true;
                return event.mouseButton.button == other.mouseButton.button;
            }
            else if (event.type == window::IEvent::MOUSE_BUTTON_EVENT) {
                return false;
            }
            return true;
        }
        Command& Command::operator=(const Command& other) {
            if (this != &other) {
                name = other.name;
                if (other.action != nullptr)
                    action = std::make_unique<Action>(*other.action);
                if (other.trigger != nullptr)
                    trigger = std::make_unique<core::FastDelegate<bool>>(*other.trigger);
                slot = core::FastDelegate<void>(other.slot);
            }
            return *this;
        }
    }
}