#include "../../../../include/odfaeg/Window/Linux/x11Keyboard.hpp"
#include "../../../../include/odfaeg/Window/Linux/display.hpp"
#include <X11/keysym.h>
namespace odfaeg {
    namespace window {
        ////////////////////////////////////////////////////////////
        bool X11Keyboard::isKeyPressed(IKeyboard::Key key)
        {
            // Get the corresponding X11 keysym
            KeySym keysym = 0;
            switch (key)
            {
                case IKeyboard::LShift:     keysym = XK_Shift_L;      break;
                case IKeyboard::RShift:     keysym = XK_Shift_R;      break;
                case IKeyboard::LControl:   keysym = XK_Control_L;    break;
                case IKeyboard::RControl:   keysym = XK_Control_R;    break;
                case IKeyboard::LAlt:       keysym = XK_Alt_L;        break;
                case IKeyboard::RAlt:       keysym = XK_Alt_R;        break;
                case IKeyboard::LSystem:    keysym = XK_Super_L;      break;
                case IKeyboard::RSystem:    keysym = XK_Super_R;      break;
                case IKeyboard::Menu:       keysym = XK_Menu;         break;
                case IKeyboard::Escape:     keysym = XK_Escape;       break;
                case IKeyboard::Semicolon:  keysym = XK_semicolon;    break;
                case IKeyboard::Slash:      keysym = XK_slash;        break;
                case IKeyboard::Equal:      keysym = XK_equal;        break;
                case IKeyboard::Hyphen:     keysym = XK_minus;        break;
                case IKeyboard::LBracket:   keysym = XK_bracketleft;  break;
                case IKeyboard::RBracket:   keysym = XK_bracketright; break;
                case IKeyboard::Comma:      keysym = XK_comma;        break;
                case IKeyboard::Period:     keysym = XK_period;       break;
                case IKeyboard::Quote:      keysym = XK_apostrophe;   break;
                case IKeyboard::Backslash:  keysym = XK_backslash;    break;
                case IKeyboard::Tilde:      keysym = XK_grave;        break;
                case IKeyboard::Space:      keysym = XK_space;        break;
                case IKeyboard::Enter:      keysym = XK_Return;       break;
                case IKeyboard::Backspace:  keysym = XK_BackSpace;    break;
                case IKeyboard::Tab:        keysym = XK_Tab;          break;
                case IKeyboard::PageUp:     keysym = XK_Prior;        break;
                case IKeyboard::PageDown:   keysym = XK_Next;         break;
                case IKeyboard::End:        keysym = XK_End;          break;
                case IKeyboard::Home:       keysym = XK_Home;         break;
                case IKeyboard::Insert:     keysym = XK_Insert;       break;
                case IKeyboard::Delete:     keysym = XK_Delete;       break;
                case IKeyboard::Add:        keysym = XK_KP_Add;       break;
                case IKeyboard::Subtract:   keysym = XK_KP_Subtract;  break;
                case IKeyboard::Multiply:   keysym = XK_KP_Multiply;  break;
                case IKeyboard::Divide:     keysym = XK_KP_Divide;    break;
                case IKeyboard::Pause:      keysym = XK_Pause;        break;
                case IKeyboard::F1:         keysym = XK_F1;           break;
                case IKeyboard::F2:         keysym = XK_F2;           break;
                case IKeyboard::F3:         keysym = XK_F3;           break;
                case IKeyboard::F4:         keysym = XK_F4;           break;
                case IKeyboard::F5:         keysym = XK_F5;           break;
                case IKeyboard::F6:         keysym = XK_F6;           break;
                case IKeyboard::F7:         keysym = XK_F7;           break;
                case IKeyboard::F8:         keysym = XK_F8;           break;
                case IKeyboard::F9:         keysym = XK_F9;           break;
                case IKeyboard::F10:        keysym = XK_F10;          break;
                case IKeyboard::F11:        keysym = XK_F11;          break;
                case IKeyboard::F12:        keysym = XK_F12;          break;
                case IKeyboard::F13:        keysym = XK_F13;          break;
                case IKeyboard::F14:        keysym = XK_F14;          break;
                case IKeyboard::F15:        keysym = XK_F15;          break;
                case IKeyboard::Left:       keysym = XK_Left;         break;
                case IKeyboard::Right:      keysym = XK_Right;        break;
                case IKeyboard::Up:         keysym = XK_Up;           break;
                case IKeyboard::Down:       keysym = XK_Down;         break;
                case IKeyboard::Numpad0:    keysym = XK_KP_Insert;    break;
                case IKeyboard::Numpad1:    keysym = XK_KP_End;       break;
                case IKeyboard::Numpad2:    keysym = XK_KP_Down;      break;
                case IKeyboard::Numpad3:    keysym = XK_KP_Page_Down; break;
                case IKeyboard::Numpad4:    keysym = XK_KP_Left;      break;
                case IKeyboard::Numpad5:    keysym = XK_KP_Begin;     break;
                case IKeyboard::Numpad6:    keysym = XK_KP_Right;     break;
                case IKeyboard::Numpad7:    keysym = XK_KP_Home;      break;
                case IKeyboard::Numpad8:    keysym = XK_KP_Up;        break;
                case IKeyboard::Numpad9:    keysym = XK_KP_Page_Up;   break;
                case IKeyboard::A:          keysym = XK_a;            break;
                case IKeyboard::B:          keysym = XK_b;            break;
                case IKeyboard::C:          keysym = XK_c;            break;
                case IKeyboard::D:          keysym = XK_d;            break;
                case IKeyboard::E:          keysym = XK_e;            break;
                case IKeyboard::F:          keysym = XK_f;            break;
                case IKeyboard::G:          keysym = XK_g;            break;
                case IKeyboard::H:          keysym = XK_h;            break;
                case IKeyboard::I:          keysym = XK_i;            break;
                case IKeyboard::J:          keysym = XK_j;            break;
                case IKeyboard::K:          keysym = XK_k;            break;
                case IKeyboard::L:          keysym = XK_l;            break;
                case IKeyboard::M:          keysym = XK_m;            break;
                case IKeyboard::N:          keysym = XK_n;            break;
                case IKeyboard::O:          keysym = XK_o;            break;
                case IKeyboard::P:          keysym = XK_p;            break;
                case IKeyboard::Q:          keysym = XK_q;            break;
                case IKeyboard::R:          keysym = XK_r;            break;
                case IKeyboard::S:          keysym = XK_s;            break;
                case IKeyboard::T:          keysym = XK_t;            break;
                case IKeyboard::U:          keysym = XK_u;            break;
                case IKeyboard::V:          keysym = XK_v;            break;
                case IKeyboard::W:          keysym = XK_w;            break;
                case IKeyboard::X:          keysym = XK_x;            break;
                case IKeyboard::Y:          keysym = XK_y;            break;
                case IKeyboard::Z:          keysym = XK_z;            break;
                case IKeyboard::Num0:       keysym = XK_0;            break;
                case IKeyboard::Num1:       keysym = XK_1;            break;
                case IKeyboard::Num2:       keysym = XK_2;            break;
                case IKeyboard::Num3:       keysym = XK_3;            break;
                case IKeyboard::Num4:       keysym = XK_4;            break;
                case IKeyboard::Num5:       keysym = XK_5;            break;
                case IKeyboard::Num6:       keysym = XK_6;            break;
                case IKeyboard::Num7:       keysym = XK_7;            break;
                case IKeyboard::Num8:       keysym = XK_8;            break;
                case IKeyboard::Num9:       keysym = XK_9;            break;
                default:                   keysym = 0;               break;
            }

            // Sanity checks
            if (key < 0 || key >= IKeyboard::KeyCount)
                return false;

            // Open a connection with the X server
            ::Display* display = odfaeg::window::Display::openDisplay();
            // Convert to keycode
            KeyCode keycode = XKeysymToKeycode(display, keysym);
            if (keycode != 0)
            {
                // Get the whole X11Keyboard state
                char keys[32];
                XQueryKeymap(display, keys);
                odfaeg::window::Display::closeDisplay(display);
                // Check our keycode
                return (keys[keycode / 8] & (1 << (keycode % 8))) != 0;
            } else {
                odfaeg::window::Display::closeDisplay(display);
                return false;
            }
        }
    }
}
