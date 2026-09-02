namespace odfaeg
{
    namespace window {
        Command::Command(Action action, core::FastDelegate<void> slot) : slot(slot), action(action)
        {
                      
        }
        void Command::setName(std::string name) {
            this->name = name;
        }       
        Command::Command(const Command& other) : slot(other.slot), action(action) {           
          
        }
        bool Command::isTriggered()
        {              
            return action.isTriggered();
        }
        bool Command::containsBufferEvent(window::IEvent& event) {
            return action.containsEvent(event);            
        }

        void Command::clearEventsStack()
        {
           action.clearEvents(); 
        }

        void Command::pushEvent(window::IEvent& event)
        {
           action.pushEvent(event);            
        }
        Action& Command::getAction() {
            return action;
        }
        void Command::operator()()
        {
            slot();
        }

        void Command::removeEvent(IEvent& event) {            
             action.removeEvent(event);
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
                action = other.action;
                slot = core::FastDelegate<void>(other.slot);
            }
            return *this;
        }
    }
}