#ifndef ODFAEG_ACTION_HPP
#define ODFAEG_ACTION_HPP
#include "export.hpp"
#include <map>
#include "signal.h"
#include <functional>
#include <memory>
#include <iostream>
#include "../Window/iEvent.hpp"
#include "../Window/iKeyboard.hpp"
#include "../Window/iMouse.hpp"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace core {
        /**
          * \file action.h
          * \class Action
          * \brief Link SFML events to odfaeg actions.
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          * Link one or more SFML Events to ACTIONS, and determine when the action is triggered.
          * The window::IEvents are stored into the leafs of a BSP tree structure.
          * It means that combined actions contains two children :
            -The first action child is linked to the first SFML event.
            -The second action child is linked to the second SFML event.
            -The combined actions contains also a pointer to a function who determine when the combined action is triggered.
            by exemple the xor function return true if one of the two action's children is triggered. (not both)
          */
        class ODFAEG_CORE_API Action {
            public :
            std::string name;
            /** \enum EVENT_TYPE
            *   \brief the types of the events witch are linked to actions.
            *   COMBINED_WITH_OR, COMBINED_WITH_X_OR AND COMBINED_WITH_AND define a combination of actions.
            *   the action is triggered if one or/and the passed sf:Events are triggered.
            *   by exemple if CLOSED is passed, an window::IEvent of type Closed is stored.
            */
            enum EVENT_TYPE {
                KEY_PRESSED_ONCE, KEY_HELD_DOWN, KEY_RELEASED,
                MOUSE_BUTTON_PRESSED_ONCE, MOUSE_BUTTON_HELD_DOWN, MOUSE_BUTTON_RELEASED,
                JOYSTICK_BUTTON_PRESSED_ONCE, JOYSTICK_BUTTON_HELD_DOWN, JOYSTICK_BUTTON_RELEASED, CLOSED, RESIZED, LOST_FOCUS, GAIGNED_FOCUS, TEXT_ENTERED, MOUSE_WHEEL_MOVED, MOUSE_MOVED_, MOUSE_ENTERED, MOUSE_LEFT, JOYSTICK_MOVED, JOYSTICK_CONNECTED, JOYSTICK_DISCONNECTED,
                COMBINED_WITH_OR, COMBINED_WITH_XOR, COMBINED_WITH_AND
            };
            /**\fn Action (EVENT_TYPE)
            * \brief constructor (define an action with an event type)
            * \param an enum that is related to the sfml event type who's linked to the action.
            */
            Action (EVENT_TYPE type);
            /**\fn Action (EVENT_TYPE type, sf::Keyboard key)
            *  \brief constructor (define an action with a key event)
            *  \param type : the type of the event. (KEY_PRESSED_ONCE, KEY_HELD_DOWN or KEY_RELEASED, other types are invalid)
            *  \param key : the key of the keyevent.
            */
            Action (EVENT_TYPE type, window::IKeyboard::Key key);
            /**\fn Action (EVENT_TYPE type, window::IMouse::Button button)
            *  \brief constructor (define an action with a mouse event)
            *  \param type : the type of the event. (MOUSE_BUTTON_PRESSED_ONCE, MOUSE_BUTTON_HELD_DOWN or MOUSE_BUTTON_RELEASED, other types are invalid)
            *  \param button : the button of the mouseevent.
            */
            Action (EVENT_TYPE type, window::IMouse::Button button);
            /**\fn bool andComparator (Action *a1, Action *a2)
            *  \brief check if two actions are triggered.
            *  \param a1 : the first action. (the left child)
            *  \param a2 : the second action. (the right child)
            *  \return true if the two actions are triggered.
            */
            Action (const Action& action);
            bool andComparator (Action& a1, Action& a2);
            /**\fn bool orComparator (Action *a1, Action *a2)
            *  \brief check if one action (ot both) is/are triggered.
            *  \param a1 : the first action. (the left child)
            *  \param a2 : the second action. (the right child)
            *  \return true if one action o both are triggered.
            */
            bool orComparator (Action& a1, Action& a2);
            /**\fn bool xorComparator (Action *a1, Action *a2)
            *  \brief check if one action is triggered. (only one, not both)
            *  \param a1 : the first action. (the left child)
            *  \param a2 : the second action. (the right child)
            *  \return true if one of the two actions is triggered. (Not both)
            */
            bool xorComparator (Action& a1, Action& a2);
            /**\fn void operator!()
            *  \brief redefines the not operator, the function isTriggered return true if the action is NOT triggered.
            */
            void operator! ();
            /**\fn Action& operator| (Action &other)
            *  \brief redefines the operator| (xor) to build a combined action.
            *  \param the other action.
            *  \return the combined action.
            */
            Action operator| (Action other);
            /**\fn Action& operator|| (Action &other)
            *  \brief redefines the operator|| (or) to build a combined action.
            *  \param the other action.
            *  \return the combined action.
            */
            Action operator|| (Action other);
            /**\fn Action& operator&& (Action &other)
            *  \brief redefines the operator&& (and) to build a combined action.
            *  \param the other action.
            *  \return the combined action.
            */
            Action operator&& (Action other);
            /**\fn bool containsEvent ();
            *  \brief return true if the action contains the passed event.
            *  \param window::IEvent& the passed event.
            *  \return true if the action is triggered, false otherwise.
            */
            bool containsEvent (window::IEvent& event);
            /**\fn bool isTriggered ();
            *  \brief return true if the action is triggered, the triggered window::IEvents are stored into the
            * ActionMap class, the window::IEvents are passed to this function and if an sf:Event is contained in
            * a leaf action, the window::IEvent is evaluated with this one.
            *  \return true if the action is triggered, false otherwise.
            */
            bool isTriggered ();
            /**\fn Action (EVENT_TYPE, Action *lefChild, Action *rightChild)
            *  \brief build a combined action. (This constructor is used by the redefinition of the logical operators)
            *  \param type : the type of the combined action. (COMBINED_WITH_AND, COMBINED_WITH_OR or COMBINED_WITH_XOR)
            *  \param leftChild : the left child.
            *  \param rightChild : the right child.
            */
            Action (EVENT_TYPE type, Action leftChild, Action rightChild);
            /**\fn setPressed (window::IEvent event, bool pressed)
            *  \brief reset the states of the event. (For key pressed events and mouse button pressed envents types only)
            *  this function is necessary to avoid that PRESSED_ONCE action's types occurs more than once while the key or a button is held down.
            *  \param window::IEvent : the event onwich we want to reset the state.
            **/
            void setPressed (window::IEvent event);
            void setPressed (bool pressed) {
                this->pressed = pressed;
            }
            /**\fn void getEvents(std::vector<window::IEvent>& events);
            *  \param the list of all window::IEvents contained into the BSP-tree.
            */
            void getEvents(std::vector<window::IEvent>& events);
            void getActions(std::vector<Action*>& actions);
            bool isPressed() {
                return pressed;
            }
            Action& operator=(const Action& action);
            void pushEvent(window::IEvent& event);
            void clearEvents();
            void removeEvent(window::IEvent& event);
            private :
            /**\fn bool equalEvent (window::IEvent event, window::IEvent other)
            *  \brief compare two window::IEvents. (The events are equal if the event's types and params are equal)
            *  \return true if the two window::IEvent are equal, false otherwise.
            */
            //bool equalEvent (window::IEvent event, window::IEvent other);
            bool pressed; /**< the state of the event contained into the BSP tree*/
            EVENT_TYPE type; /**< the type of the event.*/
            window::IEvent startEvent; /**< the window::IEvent linked to the action. (for single actions only)*/
            bool leaf; /**< determine if the action is a single or a combined action*/
            std::unique_ptr<Action> leftChild;
            std::unique_ptr<Action> rightChild;/**<pointers to the two action's children if the action is a combined action*/
            std::function<bool(Action*, Action&, Action&)> comparator;/**A pointer to the function which's used to compare two actions. (For combined actions only)*/
            bool is_not; /**<determine if the isTriggered function have to return true or false if the action is triggered*/
            std::vector<window::IEvent> events;
        };
    }
}
#endif // ACTION
