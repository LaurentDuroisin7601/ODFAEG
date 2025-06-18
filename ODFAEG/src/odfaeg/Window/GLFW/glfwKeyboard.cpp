#include "../../../../include/odfaeg/Window/GLFW/glfwKeyboard.hpp"

namespace odfaeg {
    namespace window {
        IKeyboard::Key GLFWKeyboard::glfwToODFAEGKey(int key) {
            switch(key) {
                case GLFW_KEY_A : return IKeyboard::A;
                case GLFW_KEY_B : return IKeyboard::B;
                case GLFW_KEY_C : return IKeyboard::C;
                case GLFW_KEY_D : return IKeyboard::D;
                case GLFW_KEY_E : return IKeyboard::E;
                case GLFW_KEY_F : return IKeyboard::F;
                case GLFW_KEY_G : return IKeyboard::G;
                case GLFW_KEY_H : return IKeyboard::H;
                case GLFW_KEY_I : return IKeyboard::I;
                case GLFW_KEY_J : return IKeyboard::J;
                case GLFW_KEY_K : return IKeyboard::K;
                case GLFW_KEY_L : return IKeyboard::L;
                case GLFW_KEY_M : return IKeyboard::M;
                case GLFW_KEY_N : return IKeyboard::N;
                case GLFW_KEY_O : return IKeyboard::O;
                case GLFW_KEY_P : return IKeyboard::P;
                case GLFW_KEY_Q : return IKeyboard::Q;
                case GLFW_KEY_R : return IKeyboard::R;
                case GLFW_KEY_S : return IKeyboard::S;
                case GLFW_KEY_T : return IKeyboard::T;
                case GLFW_KEY_U : return IKeyboard::U;
                case GLFW_KEY_V : return IKeyboard::V;
                case GLFW_KEY_W : return IKeyboard::W;
                case GLFW_KEY_X : return IKeyboard::X;
                case GLFW_KEY_Y : return IKeyboard::Y;
                case GLFW_KEY_Z : return IKeyboard::Z;
                case GLFW_KEY_0 : return IKeyboard::Num0;
                case GLFW_KEY_1 : return IKeyboard::Num1;
                case GLFW_KEY_2 : return IKeyboard::Num2;
                case GLFW_KEY_3 : return IKeyboard::Num3;
                case GLFW_KEY_4 : return IKeyboard::Num4;
                case GLFW_KEY_5 : return IKeyboard::Num5;
                case GLFW_KEY_6 : return IKeyboard::Num6;
                case GLFW_KEY_7 : return IKeyboard::Num7;
                case GLFW_KEY_8 : return IKeyboard::Num8;
                case GLFW_KEY_9 : return IKeyboard::Num9;
                case GLFW_KEY_ESCAPE : return IKeyboard::Escape;      ///< The Escape key
                case GLFW_KEY_LEFT_CONTROL : return IKeyboard::LControl;     ///< The left Control key
                case GLFW_KEY_LEFT_SHIFT : return IKeyboard::LShift;       ///< The left Shift key
                case GLFW_KEY_LEFT_ALT : return IKeyboard::LAlt;         ///< The left Alt key
                case GLFW_KEY_LEFT_SUPER : return IKeyboard::LSystem;      ///< The left OS specific key: window (Windows and Linux), apple (MacOS X), ...
                case GLFW_KEY_RIGHT_CONTROL : return IKeyboard::RControl;     ///< The right Control key
                case GLFW_KEY_RIGHT_SHIFT : return IKeyboard::RShift;       ///< The right Shift key
                case GLFW_KEY_RIGHT_ALT : return IKeyboard::RAlt;         ///< The right Alt key
                case GLFW_KEY_RIGHT_SUPER : return IKeyboard::RSystem;      ///< The right OS specific key: window (Windows and Linux), apple (MacOS X), ...
                case GLFW_KEY_MENU : return IKeyboard::Menu;         ///< The Menu key
                case GLFW_KEY_LEFT_BRACKET : return IKeyboard::LBracket;     ///< The [ key
                case GLFW_KEY_RIGHT_BRACKET : return IKeyboard::RBracket;     ///< The ] key
                case GLFW_KEY_SEMICOLON : return IKeyboard::Semicolon;    ///< The ; key
                case GLFW_KEY_COMMA : return IKeyboard::Comma;        ///< The , key
                case GLFW_KEY_PERIOD : return IKeyboard::Period;       ///< The . key
                //case GFLW_KEY_QUOTE : return IKeyboard::Quote;        ///< The ' key
                case GLFW_KEY_SLASH : return IKeyboard::Slash;        ///< The / key
                case GLFW_KEY_BACKSLASH : return IKeyboard::Backslash;    ///< The \ key
                //case GLFW_KEY_TILDE : return IKeyboard::Tilde;        ///< The ~ key
                case GLFW_KEY_KP_EQUAL : return IKeyboard::Equal;        ///< The = key
                //case GLFW_KEY_HYPHEN : return IKeyboard::Hyphen;       ///< The - key (hyphen)
                case GLFW_KEY_SPACE : return IKeyboard::Space;        ///< The Space key
                case GLFW_KEY_KP_ENTER : return IKeyboard::Enter;        ///< The Enter/Return keys
                case GLFW_KEY_BACKSPACE : return IKeyboard::Backspace;    ///< The Backspace key
                case GLFW_KEY_TAB : return IKeyboard::Tab;          ///< The Tabulation key
                case GLFW_KEY_PAGE_UP : return IKeyboard::PageUp;       ///< The Page up key
                case GLFW_KEY_PAGE_DOWN : return IKeyboard::PageDown;     ///< The Page down key
                case GLFW_KEY_END : return IKeyboard::End;          ///< The End key
                case GLFW_KEY_HOME : return IKeyboard::Home;         ///< The Home key
                case GLFW_KEY_INSERT : return IKeyboard::Insert;      ///< The Insert key
                case GLFW_KEY_DELETE : return IKeyboard::Delete;       ///< The Delete key
                case GLFW_KEY_KP_ADD : return IKeyboard::Add;          ///< The + key
                case GLFW_KEY_KP_SUBTRACT : return IKeyboard::Subtract;     ///< The - key (minus, usually from numpad)
                case GLFW_KEY_KP_MULTIPLY : return IKeyboard::Multiply;     ///< The * key
                case GLFW_KEY_KP_DIVIDE : return IKeyboard::Divide;       ///< The / key
                case GLFW_KEY_LEFT : return IKeyboard::Left;         ///< Left arrow
                case GLFW_KEY_RIGHT : return IKeyboard::Right;        ///< Right arrow
                case GLFW_KEY_UP : return IKeyboard::Up;           ///< Up arrow
                case GLFW_KEY_DOWN : return IKeyboard::Down;         ///< Down arrow
                case GLFW_KEY_KP_0 : return IKeyboard::Numpad0;      ///< The numpad 0 key
                case GLFW_KEY_KP_1 : return IKeyboard::Numpad1;      ///< The numpad 1 key
                case GLFW_KEY_KP_2 : return IKeyboard::Numpad2;      ///< The numpad 2 key
                case GLFW_KEY_KP_3 : return IKeyboard::Numpad3;      ///< The numpad 3 key
                case GLFW_KEY_KP_4 : return IKeyboard::Numpad4;      ///< The numpad 4 key
                case GLFW_KEY_KP_5 : return IKeyboard::Numpad5;      ///< The numpad 5 key
                case GLFW_KEY_KP_6 : return IKeyboard::Numpad6;      ///< The numpad 6 key
                case GLFW_KEY_KP_7 : return IKeyboard::Numpad7;      ///< The numpad 7 key
                case GLFW_KEY_KP_8 : return IKeyboard::Numpad8;      ///< The numpad 8 key
                case GLFW_KEY_KP_9 : return IKeyboard::Numpad9;      ///< The numpad 9 key
                case GLFW_KEY_F1 : return IKeyboard::F1;           ///< The F1 key
                case GLFW_KEY_F2 : return IKeyboard::F2;           ///< The F2 key
                case GLFW_KEY_F3 : return IKeyboard::F3;           ///< The F3 key
                case GLFW_KEY_F4 : return IKeyboard::F4;           ///< The F4 key
                case GLFW_KEY_F5 : return IKeyboard::F5;           ///< The F5 key
                case GLFW_KEY_F6 : return IKeyboard::F6;           ///< The F6 key
                case GLFW_KEY_F7 : return IKeyboard::F7;           ///< The F7 key
                case GLFW_KEY_F8 : return IKeyboard::F8;           ///< The F8 key
                case GLFW_KEY_F9 : return IKeyboard::F9;           ///< The F9 key
                case GLFW_KEY_F10 : return IKeyboard::F10;           ///< The F10 key
                case GLFW_KEY_F11 : return IKeyboard::F11;           ///< The F11 key
                case GLFW_KEY_F12 : return IKeyboard::F12;           ///< The F12 key
                case GLFW_KEY_F13 : return IKeyboard::F13;           ///< The F13 key
                case GLFW_KEY_F14 : return IKeyboard::F14;           ///< The F14 key
                case GLFW_KEY_F15 : return IKeyboard::F15;           ///< The F15 key
                case GLFW_KEY_PAUSE : return IKeyboard::Pause;        ///< The Pause key
                default : return IKeyboard::Unknown;
            }
        }
        int GLFWKeyboard::odfaegToGlfwKey(IKeyboard::Key key) {
            switch(key) {
                //case IKeyboard::Unknown : return sf::Keyboard::Unknown;
                case IKeyboard::A : return GLFW_KEY_A;
                case IKeyboard::B : return GLFW_KEY_B;
                case IKeyboard::C : return GLFW_KEY_C;
                case IKeyboard::D : return GLFW_KEY_D;
                case IKeyboard::E : return GLFW_KEY_E;
                case IKeyboard::F : return GLFW_KEY_F;
                case IKeyboard::G : return GLFW_KEY_G;
                case IKeyboard::H : return GLFW_KEY_H;
                case IKeyboard::I : return GLFW_KEY_I;
                case IKeyboard::J : return GLFW_KEY_J;
                case IKeyboard::K : return GLFW_KEY_K;
                case IKeyboard::L : return GLFW_KEY_L;
                case IKeyboard::M : return GLFW_KEY_M;
                case IKeyboard::N : return GLFW_KEY_N;
                case IKeyboard::O : return GLFW_KEY_O;
                case IKeyboard::P : return GLFW_KEY_P;
                case IKeyboard::Q : return GLFW_KEY_Q;
                case IKeyboard::R : return GLFW_KEY_R;
                case IKeyboard::S : return GLFW_KEY_S;
                case IKeyboard::T : return GLFW_KEY_T;
                case IKeyboard::U : return GLFW_KEY_U;
                case IKeyboard::V : return GLFW_KEY_V;
                case IKeyboard::W : return GLFW_KEY_W;
                case IKeyboard::X : return GLFW_KEY_X;
                case IKeyboard::Y : return GLFW_KEY_Y;
                case IKeyboard::Z : return GLFW_KEY_Z;
                case IKeyboard::Num0 : return GLFW_KEY_0;
                case IKeyboard::Num1 : return GLFW_KEY_1;
                case IKeyboard::Num2 : return GLFW_KEY_2;
                case IKeyboard::Num3 : return GLFW_KEY_3;
                case IKeyboard::Num4 : return GLFW_KEY_4;
                case IKeyboard::Num5 : return GLFW_KEY_5;
                case IKeyboard::Num6 : return GLFW_KEY_6;
                case IKeyboard::Num7 : return GLFW_KEY_7;
                case IKeyboard::Num8 : return GLFW_KEY_8;
                case IKeyboard::Num9 : return GLFW_KEY_9;
                case IKeyboard::Escape : return GLFW_KEY_ESCAPE;      ///< The Escape key
                case IKeyboard::LControl : return GLFW_KEY_LEFT_CONTROL;     ///< The left Control key
                case IKeyboard::LShift : return GLFW_KEY_LEFT_SHIFT;       ///< The left Shift key
                case IKeyboard::LAlt : return GLFW_KEY_LEFT_ALT;         ///< The left Alt key
                case IKeyboard::LSystem : return GLFW_KEY_LEFT_SUPER;      ///< The left OS specific key: window (Windows and Linux), apple (MacOS X), ...
                case IKeyboard::RControl : return GLFW_KEY_RIGHT_CONTROL;     ///< The right Control key
                case IKeyboard::RShift : return GLFW_KEY_RIGHT_SHIFT;       ///< The right Shift key
                case IKeyboard::RAlt : return GLFW_KEY_RIGHT_ALT;         ///< The right Alt key
                case IKeyboard::RSystem : return GLFW_KEY_RIGHT_SUPER;      ///< The right OS specific key: window (Windows and Linux), apple (MacOS X), ...
                case IKeyboard::Menu : return GLFW_KEY_MENU;         ///< The Menu key
                case IKeyboard::LBracket : return GLFW_KEY_LEFT_BRACKET;     ///< The [ key
                case IKeyboard::RBracket : return GLFW_KEY_RIGHT_BRACKET;     ///< The ] key
                case IKeyboard::Semicolon : return GLFW_KEY_SEMICOLON;    ///< The ; key
                case IKeyboard::Comma : return GLFW_KEY_COMMA;        ///< The , key
                case IKeyboard::Period : return GLFW_KEY_PERIOD;       ///< The . key
                //case IKeyboard::Quote : return sf::Keyboard::Quote;        ///< The ' key
                case IKeyboard::Slash : return GLFW_KEY_SLASH;        ///< The / key
                case IKeyboard::Backslash : return GLFW_KEY_BACKSLASH;    ///< The \ key
                //case IKeyboard::Tilde : return sf::Keyboard::Tilde;        ///< The ~ key
                case IKeyboard::Equal : return GLFW_KEY_KP_EQUAL;        ///< The = key
                //case IKeyboard::Hyphen : return sf::Keyboard::Hyphen;       ///< The - key (hyphen)
                case IKeyboard::Space : return GLFW_KEY_SPACE;        ///< The Space key
                case IKeyboard::Enter : return GLFW_KEY_KP_ENTER;        ///< The Enter/Return keys
                case IKeyboard::Tab : return GLFW_KEY_TAB;          ///< The Tabulation key
                case IKeyboard::PageUp : return GLFW_KEY_PAGE_UP;       ///< The Page up key
                case IKeyboard::PageDown : return GLFW_KEY_PAGE_DOWN;     ///< The Page down key
                case IKeyboard::End : return GLFW_KEY_END;          ///< The End key
                case IKeyboard::Home : return GLFW_KEY_HOME;         ///< The Home key
                case IKeyboard::Insert : return GLFW_KEY_INSERT;      ///< The Insert key
                case IKeyboard::Delete : return GLFW_KEY_DELETE;       ///< The Delete key
                case IKeyboard::Add : return GLFW_KEY_KP_ADD;          ///< The + key
                case IKeyboard::Subtract : return GLFW_KEY_KP_SUBTRACT;     ///< The - key (minus, usually from numpad)
                case IKeyboard::Multiply : return GLFW_KEY_KP_MULTIPLY;     ///< The * key
                case IKeyboard::Divide : return GLFW_KEY_KP_DIVIDE;       ///< The / key
                case IKeyboard::Left : return GLFW_KEY_LEFT;         ///< Left arrow
                case IKeyboard::Right : return GLFW_KEY_RIGHT;        ///< Right arrow
                case IKeyboard::Up : return GLFW_KEY_UP;           ///< Up arrow
                case IKeyboard::Down : return GLFW_KEY_DOWN;         ///< Down arrow
                case IKeyboard::Numpad0 : return GLFW_KEY_KP_0;      ///< The numpad 0 key
                case IKeyboard::Numpad1 : return GLFW_KEY_KP_1;      ///< The numpad 1 key
                case IKeyboard::Numpad2 : return GLFW_KEY_KP_2;      ///< The numpad 2 key
                case IKeyboard::Numpad3 : return GLFW_KEY_KP_3;      ///< The numpad 3 key
                case IKeyboard::Numpad4 : return GLFW_KEY_KP_4;      ///< The numpad 4 key
                case IKeyboard::Numpad5 : return GLFW_KEY_KP_5;      ///< The numpad 5 key
                case IKeyboard::Numpad6 : return GLFW_KEY_KP_6;      ///< The numpad 6 key
                case IKeyboard::Numpad7 : return GLFW_KEY_KP_7;      ///< The numpad 7 key
                case IKeyboard::Numpad8 : return GLFW_KEY_KP_8;      ///< The numpad 8 key
                case IKeyboard::Numpad9 : return GLFW_KEY_KP_9;      ///< The numpad 9 key
                case IKeyboard::F1 : return GLFW_KEY_F1;           ///< The F1 key
                case IKeyboard::F2 : return GLFW_KEY_F2;           ///< The F2 key
                case IKeyboard::F3 : return GLFW_KEY_F3;           ///< The F3 key
                case IKeyboard::F4 : return GLFW_KEY_F4;           ///< The F4 key
                case IKeyboard::F5 : return GLFW_KEY_F5;           ///< The F5 key
                case IKeyboard::F6 : return GLFW_KEY_F6;           ///< The F6 key
                case IKeyboard::F7 : return GLFW_KEY_F7;           ///< The F7 key
                case IKeyboard::F8 : return GLFW_KEY_F8;           ///< The F8 key
                case IKeyboard::F9 : return GLFW_KEY_F9;           ///< The F9 key
                case IKeyboard::F10 : return GLFW_KEY_F10;           ///< The F10 key
                case IKeyboard::F11 : return GLFW_KEY_F11;           ///< The F11 key
                case IKeyboard::F12 : return GLFW_KEY_F12;           ///< The F12 key
                case IKeyboard::F13 : return GLFW_KEY_F13;           ///< The F13 key
                case IKeyboard::F14 : return GLFW_KEY_F14;           ///< The F14 key
                case IKeyboard::F15 : return GLFW_KEY_F15;           ///< The F15 key
                case IKeyboard::Pause : return GLFW_KEY_PAUSE;        ///< The Pause key
            }
        }
        bool GLFWKeyboard::isKeyPressed(GLFWwindow* window, IKeyboard::Key key) {
            int state = glfwGetKey(window, odfaegToGlfwKey(key));
            if (state == GLFW_PRESS)
                return true;
            return false;
        }
    }
}
