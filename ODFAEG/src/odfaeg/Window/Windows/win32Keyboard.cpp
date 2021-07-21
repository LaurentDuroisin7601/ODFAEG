#ifdef _WIN32_WINDOWS
    #undef _WIN32_WINDOWS
#endif
#ifdef _WIN32_WINNT
    #undef _WIN32_WINNT
#endif
#define _WIN32_WINDOWS 0x0501
#define _WIN32_WINNT   0x0501
#include "../../../../include/odfaeg/Window/Windows/w32Keyboard.hpp"
#include <windows.h>
namespace odfaeg {
    namespace window {
        ////////////////////////////////////////////////////////////
        bool Win32Keyboard::isKeyPressed(IKeyboard::Key key)
        {
            int vkey = 0;
            switch (key)
            {
                default:                   vkey = 0;             break;
                case IKeyboard::A:          vkey = 'A';           break;
                case IKeyboard::B:          vkey = 'B';           break;
                case IKeyboard::C:          vkey = 'C';           break;
                case IKeyboard::D:          vkey = 'D';           break;
                case IKeyboard::E:          vkey = 'E';           break;
                case IKeyboard::F:          vkey = 'F';           break;
                case IKeyboard::G:          vkey = 'G';           break;
                case IKeyboard::H:          vkey = 'H';           break;
                case IKeyboard::I:          vkey = 'I';           break;
                case IKeyboard::J:          vkey = 'J';           break;
                case IKeyboard::K:          vkey = 'K';           break;
                case IKeyboard::L:          vkey = 'L';           break;
                case IKeyboard::M:          vkey = 'M';           break;
                case IKeyboard::N:          vkey = 'N';           break;
                case IKeyboard::O:          vkey = 'O';           break;
                case IKeyboard::P:          vkey = 'P';           break;
                case IKeyboard::Q:          vkey = 'Q';           break;
                case IKeyboard::R:          vkey = 'R';           break;
                case IKeyboard::S:          vkey = 'S';           break;
                case IKeyboard::T:          vkey = 'T';           break;
                case IKeyboard::U:          vkey = 'U';           break;
                case IKeyboard::V:          vkey = 'V';           break;
                case IKeyboard::W:          vkey = 'W';           break;
                case IKeyboard::X:          vkey = 'X';           break;
                case IKeyboard::Y:          vkey = 'Y';           break;
                case IKeyboard::Z:          vkey = 'Z';           break;
                case IKeyboard::Num0:       vkey = '0';           break;
                case IKeyboard::Num1:       vkey = '1';           break;
                case IKeyboard::Num2:       vkey = '2';           break;
                case IKeyboard::Num3:       vkey = '3';           break;
                case IKeyboard::Num4:       vkey = '4';           break;
                case IKeyboard::Num5:       vkey = '5';           break;
                case IKeyboard::Num6:       vkey = '6';           break;
                case IKeyboard::Num7:       vkey = '7';           break;
                case IKeyboard::Num8:       vkey = '8';           break;
                case IKeyboard::Num9:       vkey = '9';           break;
                case IKeyboard::Escape:     vkey = VK_ESCAPE;     break;
                case IKeyboard::LControl:   vkey = VK_LCONTROL;   break;
                case IKeyboard::LShift:     vkey = VK_LSHIFT;     break;
                case IKeyboard::LAlt:       vkey = VK_LMENU;      break;
                case IKeyboard::LSystem:    vkey = VK_LWIN;       break;
                case IKeyboard::RControl:   vkey = VK_RCONTROL;   break;
                case IKeyboard::RShift:     vkey = VK_RSHIFT;     break;
                case IKeyboard::RAlt:       vkey = VK_RMENU;      break;
                case IKeyboard::RSystem:    vkey = VK_RWIN;       break;
                case IKeyboard::Menu:       vkey = VK_APPS;       break;
                case IKeyboard::LBracket:   vkey = VK_OEM_4;      break;
                case IKeyboard::RBracket:   vkey = VK_OEM_6;      break;
                case IKeyboard::Semicolon:  vkey = VK_OEM_1;      break;
                case IKeyboard::Comma:      vkey = VK_OEM_COMMA;  break;
                case IKeyboard::Period:     vkey = VK_OEM_PERIOD; break;
                case IKeyboard::Quote:      vkey = VK_OEM_7;      break;
                case IKeyboard::Slash:      vkey = VK_OEM_2;      break;
                case IKeyboard::Backslash:  vkey = VK_OEM_5;      break;
                case IKeyboard::Tilde:      vkey = VK_OEM_3;      break;
                case IKeyboard::Equal:      vkey = VK_OEM_PLUS;   break;
                case IKeyboard::Hyphen:     vkey = VK_OEM_MINUS;  break;
                case IKeyboard::Space:      vkey = VK_SPACE;      break;
                case IKeyboard::Enter:      vkey = VK_RETURN;     break;
                case IKeyboard::Backspace:  vkey = VK_BACK;       break;
                case IKeyboard::Tab:        vkey = VK_TAB;        break;
                case IKeyboard::PageUp:     vkey = VK_PRIOR;      break;
                case IKeyboard::PageDown:   vkey = VK_NEXT;       break;
                case IKeyboard::End:        vkey = VK_END;        break;
                case IKeyboard::Home:       vkey = VK_HOME;       break;
                case IKeyboard::Insert:     vkey = VK_INSERT;     break;
                case IKeyboard::Delete:     vkey = VK_DELETE;     break;
                case IKeyboard::Add:        vkey = VK_ADD;        break;
                case IKeyboard::Subtract:   vkey = VK_SUBTRACT;   break;
                case IKeyboard::Multiply:   vkey = VK_MULTIPLY;   break;
                case IKeyboard::Divide:     vkey = VK_DIVIDE;     break;
                case IKeyboard::Left:       vkey = VK_LEFT;       break;
                case IKeyboard::Right:      vkey = VK_RIGHT;      break;
                case IKeyboard::Up:         vkey = VK_UP;         break;
                case IKeyboard::Down:       vkey = VK_DOWN;       break;
                case IKeyboard::Numpad0:    vkey = VK_NUMPAD0;    break;
                case IKeyboard::Numpad1:    vkey = VK_NUMPAD1;    break;
                case IKeyboard::Numpad2:    vkey = VK_NUMPAD2;    break;
                case IKeyboard::Numpad3:    vkey = VK_NUMPAD3;    break;
                case IKeyboard::Numpad4:    vkey = VK_NUMPAD4;    break;
                case IKeyboard::Numpad5:    vkey = VK_NUMPAD5;    break;
                case IKeyboard::Numpad6:    vkey = VK_NUMPAD6;    break;
                case IKeyboard::Numpad7:    vkey = VK_NUMPAD7;    break;
                case IKeyboard::Numpad8:    vkey = VK_NUMPAD8;    break;
                case IKeyboard::Numpad9:    vkey = VK_NUMPAD9;    break;
                case IKeyboard::F1:         vkey = VK_F1;         break;
                case IKeyboard::F2:         vkey = VK_F2;         break;
                case IKeyboard::F3:         vkey = VK_F3;         break;
                case IKeyboard::F4:         vkey = VK_F4;         break;
                case IKeyboard::F5:         vkey = VK_F5;         break;
                case IKeyboard::F6:         vkey = VK_F6;         break;
                case IKeyboard::F7:         vkey = VK_F7;         break;
                case IKeyboard::F8:         vkey = VK_F8;         break;
                case IKeyboard::F9:         vkey = VK_F9;         break;
                case IKeyboard::F10:        vkey = VK_F10;        break;
                case IKeyboard::F11:        vkey = VK_F11;        break;
                case IKeyboard::F12:        vkey = VK_F12;        break;
                case IKeyboard::F13:        vkey = VK_F13;        break;
                case IKeyboard::F14:        vkey = VK_F14;        break;
                case IKeyboard::F15:        vkey = VK_F15;        break;
                case IKeyboard::Pause:      vkey = VK_PAUSE;      break;
            }

            return (GetAsyncKeyState(vkey) & 0x8000) != 0;
        }

    }
}
