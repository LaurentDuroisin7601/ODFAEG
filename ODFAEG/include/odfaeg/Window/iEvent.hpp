#ifndef ODFAEG_IEVENT_HPP
#define ODFAEG_IEVENT_HPP
#include "../../../include/odfaeg/Math/vec2f.h"
namespace odfaeg {
    namespace window {
        struct IEvent {
            enum EventType {
                WINDOW_EVENT, KEYBOARD_EVENT, TEXT_INPUT_EVENT, MOUSE_MOTION_EVENT, MOUSE_BUTTON_EVENT, MOUSE_WHEEL_EVENT, MOUSE_EVENT_ENTER,
                MOUSE_EVENT_LEAVE
            };
            enum WindowEventID {
                WINDOW_EVENT_CLOSED, WINDOW_EVENT_HIDDEN, WINDOW_EVENT_SHOWN, WINDOW_EVENT_EXPOSED,
                WINDOW_EVENT_RESIZED, WINDOW_EVENT_FOCUS_LOST, WINDOW_EVENT_FOCUS_GAIGNED, WINDOW_EVENT_MINIMIZED, WINDOW_EVENT_MAXIMIZED
            };
            enum KeyEventID {
                KEY_EVENT_PRESSED, KEY_EVENT_RELEASED
            };
            enum MouseEventID {
                BUTTON_EVENT_PRESSED, BUTTON_EVENT_RELEASED, MOUSE_WHEEL_MOVED
            };
            struct WindowEvent {
                unsigned int type;
                unsigned int timestamp;
                unsigned long long int windowID;
                int data1;
                int data2;
            };
            struct TextInputEvent {
                unsigned int type;
                unsigned int timestamp;
                unsigned int windowID;
                long long int unicode;
            };
            struct KeyboardEvent {
                unsigned int type;
                unsigned int timestamp;
                unsigned int windowID;
                unsigned int state;
                unsigned int repeat;
                unsigned int scancode;
                unsigned int code;
                bool alt;
                bool control;
                bool shift;
                bool system;
            };
            struct MouseMotionEvent {
                unsigned int type;
                unsigned int timestamp;
                unsigned int windowID;
                unsigned int which;
                unsigned int state;
                int x;
                int y;
                int xrel;
                int yrel;
            };
            struct MouseButtonEvent {
                unsigned int type;
                unsigned int timestamp;
                unsigned int windowID;
                unsigned int which;
                unsigned int button;
                unsigned int state;
                unsigned int clicks;
                int x;
                int y;
            };
            struct MouseWheelEvent {
                unsigned int type;
                unsigned int timestamp;
                unsigned int windowID;
                unsigned int which;
                int x;
                int y;
                unsigned int direction;
            };
            union {
                struct WindowEvent window;
                struct TextInputEvent text;
                struct KeyboardEvent keyboard;
                struct MouseMotionEvent mouseMotion;
                struct MouseButtonEvent mouseButton;
                struct MouseWheelEvent mouseWheel;
            };
            unsigned int type;
        };
    }
}
#endif
