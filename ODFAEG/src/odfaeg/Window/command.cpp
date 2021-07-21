#include "../../../include/odfaeg/Window/command.h"
namespace odfaeg {
    namespace core {
        //std::vector<window::IEvent> Command::events = std::vector<window::IEvent> ();

        Command::Command (Action action, FastDelegate<void> slot) : slot(slot)
        {
            this->action = std::make_unique<Action>(action);
        }
        void Command::setName(std::string name) {
            this->name = name;
        }
        Command::Command (Action action, FastDelegate<bool> trigger, FastDelegate<void> slot) : slot(slot) {
            this->action = std::make_unique<Action>(action);
            this->trigger = std::make_unique<FastDelegate<bool>>(trigger);
        }
        Command::Command (FastDelegate<bool> trigger, FastDelegate<void> slot) : slot(slot) {
            this->trigger = std::make_unique<FastDelegate<bool>>(trigger);
        }
        Command::Command(const Command& other) : slot(other.slot) {
           if (other.action != nullptr) {
               action = std::make_unique<Action>(*other.action);
           }
           if (other.trigger != nullptr)
               trigger = std::make_unique<FastDelegate<bool>>(*other.trigger);
            name = other.name;
        }
        /*bool Command::containsEvent (window::IEvent &event)
        {
            std::vector<window::IEvent>::iterator it;
            for (it = events.begin(); it != events.end(); it++)
            {
                if (equalEvent(event, *it))
                    return true;
            }
            return false;
        }*/

        bool Command::isTriggered()
        {
            /*if (name == "TANAMECTEXTENTERED")
                action->name = "ANAMECTEXTENTERED";*/
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

        void Command::clearEventsStack ()
        {
            if (action != nullptr)
                action->clearEvents();
            //events.clear();

        }

        void Command::pushEvent (window::IEvent& event)
        {
            if (action != nullptr)
                action->pushEvent(event);

            /*std::vector<window::IEvent>::iterator it;
            bool containsEvent = false;
            for (it = events.begin(); it != events.end(); it++)
            {
                if (equalEvent(event, *it))
                    containsEvent = true;
            }
            if (!containsEvent) {
                //std::cout<<"push event!"<<std::endl;
                events.push_back(event);
            }*/
        }
        Action* Command::getAction() {
            return action.get();
        }
        /*std::vector<window::IEvent> Command::getEvents()
        {
            return events;
        }*/


        void Command::operator()()
        {
            slot();
        }

        void Command::removeEvent(window::IEvent& event) {
            /*std::vector<window::IEvent>::iterator it;
            for (it = events.begin(); it != events.end();) {
                if (equalEvent(*it, event))
                    it = events.erase(it);
                else
                    it++;
            }*/
            if (action != nullptr)
                action->removeEvent(event);
        }
        bool Command::equalEvent (window::IEvent event, window::IEvent other) {
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
                if (other.keyboard.code == window::IKeyboard::Unknown)
                    return true;
                return event.keyboard.code == other.keyboard.code;
            } else if (event.type == window::IEvent::KEYBOARD_EVENT) {
                return false;
            }
            if (event.type == window::IEvent::MOUSE_BUTTON_EVENT && event.mouseButton.type == window::IEvent::BUTTON_EVENT_PRESSED && other.mouseButton.type == window::IEvent::BUTTON_EVENT_PRESSED
                || event.type == window::IEvent::MOUSE_BUTTON_EVENT && event.mouseButton.type == window::IEvent::BUTTON_EVENT_RELEASED && other.mouseButton.type == window::IEvent::BUTTON_EVENT_RELEASED) {
                if (event.mouseButton.button == -1)
                    return true;
                return event.mouseButton.button == other.mouseButton.button;
            } else if (event.type == window::IEvent::MOUSE_BUTTON_EVENT) {
                return false;
            }
            return true;
        }
        Command& Command::operator=(const Command& other) {
            if (other.action != nullptr)
                action = std::make_unique<Action>(*other.action);
            if (other.trigger != nullptr)
                trigger = std::make_unique<FastDelegate<bool>>(*other.trigger);
            slot = FastDelegate<void>(other.slot);
            return *this;
        }
    }
}

