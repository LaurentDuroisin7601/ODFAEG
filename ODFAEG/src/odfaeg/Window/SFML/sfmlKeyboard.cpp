#include "../../../../include/odfaeg/Window/SFML/sfmlKeyboard.hpp"
namespace odfaeg {
    namespace window {
        IKeyboard::Key SFMLKeyboard::sfToODFAEGKey(sf::Keyboard::Key key) {
            switch(key) {
                case sf::Keyboard::Unknown : return IKeyboard::Unknown;
                case sf::Keyboard::A : return IKeyboard::A;
                case sf::Keyboard::B : return IKeyboard::B;
                case sf::Keyboard::C : return IKeyboard::C;
                case sf::Keyboard::D : return IKeyboard::D;
                case sf::Keyboard::E : return IKeyboard::E;
                case sf::Keyboard::F : return IKeyboard::F;
                case sf::Keyboard::G : return IKeyboard::G;
                case sf::Keyboard::H : return IKeyboard::H;
                case sf::Keyboard::I : return IKeyboard::I;
                case sf::Keyboard::J : return IKeyboard::J;
                case sf::Keyboard::K : return IKeyboard::K;
                case sf::Keyboard::L : return IKeyboard::L;
                case sf::Keyboard::M : return IKeyboard::M;
                case sf::Keyboard::N : return IKeyboard::N;
                case sf::Keyboard::O : return IKeyboard::O;
                case sf::Keyboard::P : return IKeyboard::P;
                case sf::Keyboard::Q : return IKeyboard::Q;
                case sf::Keyboard::R : return IKeyboard::R;
                case sf::Keyboard::S : return IKeyboard::S;
                case sf::Keyboard::T : return IKeyboard::T;
                case sf::Keyboard::U : return IKeyboard::U;
                case sf::Keyboard::V : return IKeyboard::V;
                case sf::Keyboard::W : return IKeyboard::W;
                case sf::Keyboard::X : return IKeyboard::X;
                case sf::Keyboard::Y : return IKeyboard::Y;
                case sf::Keyboard::Z : return IKeyboard::Z;
                case sf::Keyboard::Num0 : return IKeyboard::Num0;
                case sf::Keyboard::Num1 : return IKeyboard::Num1;
                case sf::Keyboard::Num2 : return IKeyboard::Num2;
                case sf::Keyboard::Num3 : return IKeyboard::Num3;
                case sf::Keyboard::Num4 : return IKeyboard::Num4;
                case sf::Keyboard::Num5 : return IKeyboard::Num5;
                case sf::Keyboard::Num6 : return IKeyboard::Num6;
                case sf::Keyboard::Num7 : return IKeyboard::Num7;
                case sf::Keyboard::Num8 : return IKeyboard::Num8;
                case sf::Keyboard::Num9 : return IKeyboard::Num9;
                case sf::Keyboard::Escape : return IKeyboard::Escape;      ///< The Escape key
                case sf::Keyboard::LControl : return IKeyboard::LControl;     ///< The left Control key
                case sf::Keyboard::LShift : return IKeyboard::LShift;       ///< The left Shift key
                case sf::Keyboard::LAlt : return IKeyboard::LAlt;         ///< The left Alt key
                case sf::Keyboard::LSystem : return IKeyboard::LSystem;      ///< The left OS specific key: window (Windows and Linux), apple (MacOS X), ...
                case sf::Keyboard::RControl : return IKeyboard::RControl;     ///< The right Control key
                case sf::Keyboard::RShift : return IKeyboard::RShift;       ///< The right Shift key
                case sf::Keyboard::RAlt : return IKeyboard::RAlt;         ///< The right Alt key
                case sf::Keyboard::RSystem : return IKeyboard::RSystem;      ///< The right OS specific key: window (Windows and Linux), apple (MacOS X), ...
                case sf::Keyboard::Menu : return IKeyboard::Menu;         ///< The Menu key
                case sf::Keyboard::LBracket : return IKeyboard::LBracket;     ///< The [ key
                case sf::Keyboard::RBracket : return IKeyboard::RBracket;     ///< The ] key
                case sf::Keyboard::Semicolon : return IKeyboard::Semicolon;    ///< The ; key
                case sf::Keyboard::Comma : return IKeyboard::Comma;        ///< The , key
                case sf::Keyboard::Period : return IKeyboard::Period;       ///< The . key
                case sf::Keyboard::Quote : return IKeyboard::Quote;        ///< The ' key
                case sf::Keyboard::Slash : return IKeyboard::Slash;        ///< The / key
                case sf::Keyboard::Backslash : return IKeyboard::Backslash;    ///< The \ key
                case sf::Keyboard::Tilde : return IKeyboard::Tilde;        ///< The ~ key
                case sf::Keyboard::Equal : return IKeyboard::Equal;        ///< The = key
                case sf::Keyboard::Hyphen : return IKeyboard::Hyphen;       ///< The - key (hyphen)
                case sf::Keyboard::Space : return IKeyboard::Space;        ///< The Space key
                case sf::Keyboard::Enter : return IKeyboard::Enter;        ///< The Enter/Return keys
                case sf::Keyboard::Backspace : return IKeyboard::Backspace;    ///< The Backspace key
                case sf::Keyboard::Tab : return IKeyboard::Tab;          ///< The Tabulation key
                case sf::Keyboard::PageUp : return IKeyboard::PageUp;       ///< The Page up key
                case sf::Keyboard::PageDown : return IKeyboard::PageDown;     ///< The Page down key
                case sf::Keyboard::End : return IKeyboard::End;          ///< The End key
                case sf::Keyboard::Home : return IKeyboard::Home;         ///< The Home key
                case sf::Keyboard::Insert : return IKeyboard::Insert;      ///< The Insert key
                case sf::Keyboard::Delete : return IKeyboard::Delete;       ///< The Delete key
                case sf::Keyboard::Add : return IKeyboard::Add;          ///< The + key
                case sf::Keyboard::Subtract : return IKeyboard::Subtract;     ///< The - key (minus, usually from numpad)
                case sf::Keyboard::Multiply : return IKeyboard::Multiply;     ///< The * key
                case sf::Keyboard::Divide : return IKeyboard::Divide;       ///< The / key
                case sf::Keyboard::Left : return IKeyboard::Left;         ///< Left arrow
                case sf::Keyboard::Right : return IKeyboard::Right;        ///< Right arrow
                case sf::Keyboard::Up : return IKeyboard::Up;           ///< Up arrow
                case sf::Keyboard::Down : return IKeyboard::Down;         ///< Down arrow
                case sf::Keyboard::Numpad0 : return IKeyboard::Numpad0;      ///< The numpad 0 key
                case sf::Keyboard::Numpad1 : return IKeyboard::Numpad1;      ///< The numpad 1 key
                case sf::Keyboard::Numpad2 : return IKeyboard::Numpad2;      ///< The numpad 2 key
                case sf::Keyboard::Numpad3 : return IKeyboard::Numpad3;      ///< The numpad 3 key
                case sf::Keyboard::Numpad4 : return IKeyboard::Numpad4;      ///< The numpad 4 key
                case sf::Keyboard::Numpad5 : return IKeyboard::Numpad5;      ///< The numpad 5 key
                case sf::Keyboard::Numpad6 : return IKeyboard::Numpad6;      ///< The numpad 6 key
                case sf::Keyboard::Numpad7 : return IKeyboard::Numpad7;      ///< The numpad 7 key
                case sf::Keyboard::Numpad8 : return IKeyboard::Numpad8;      ///< The numpad 8 key
                case sf::Keyboard::Numpad9 : return IKeyboard::Numpad9;      ///< The numpad 9 key
                case sf::Keyboard::F1 : return IKeyboard::F1;           ///< The F1 key
                case sf::Keyboard::F2 : return IKeyboard::F2;           ///< The F2 key
                case sf::Keyboard::F3 : return IKeyboard::F3;           ///< The F3 key
                case sf::Keyboard::F4 : return IKeyboard::F4;           ///< The F4 key
                case sf::Keyboard::F5 : return IKeyboard::F5;           ///< The F5 key
                case sf::Keyboard::F6 : return IKeyboard::F6;           ///< The F6 key
                case sf::Keyboard::F7 : return IKeyboard::F7;           ///< The F7 key
                case sf::Keyboard::F8 : return IKeyboard::F8;           ///< The F8 key
                case sf::Keyboard::F9 : return IKeyboard::F9;           ///< The F9 key
                case sf::Keyboard::F10 : return IKeyboard::F10;           ///< The F10 key
                case sf::Keyboard::F11 : return IKeyboard::F11;           ///< The F11 key
                case sf::Keyboard::F12 : return IKeyboard::F12;           ///< The F12 key
                case sf::Keyboard::F13 : return IKeyboard::F13;           ///< The F13 key
                case sf::Keyboard::F14 : return IKeyboard::F14;           ///< The F14 key
                case sf::Keyboard::F15 : return IKeyboard::F15;           ///< The F15 key
                case sf::Keyboard::Pause : return IKeyboard::Pause;        ///< The Pause key
            }
        }
        sf::Keyboard::Key SFMLKeyboard::odfaegToSfKey(IKeyboard::Key key) {
            switch(key) {
                case IKeyboard::Unknown : return sf::Keyboard::Unknown;
                case IKeyboard::A : return sf::Keyboard::A;
                case IKeyboard::B : return sf::Keyboard::B;
                case IKeyboard::C : return sf::Keyboard::C;
                case IKeyboard::D : return sf::Keyboard::D;
                case IKeyboard::E : return sf::Keyboard::E;
                case IKeyboard::F : return sf::Keyboard::F;
                case IKeyboard::G : return sf::Keyboard::G;
                case IKeyboard::H : return sf::Keyboard::H;
                case IKeyboard::I : return sf::Keyboard::I;
                case IKeyboard::J : return sf::Keyboard::J;
                case IKeyboard::K : return sf::Keyboard::K;
                case IKeyboard::L : return sf::Keyboard::L;
                case IKeyboard::M : return sf::Keyboard::M;
                case IKeyboard::N : return sf::Keyboard::N;
                case IKeyboard::O : return sf::Keyboard::O;
                case IKeyboard::P : return sf::Keyboard::P;
                case IKeyboard::Q : return sf::Keyboard::Q;
                case IKeyboard::R : return sf::Keyboard::R;
                case IKeyboard::S : return sf::Keyboard::S;
                case IKeyboard::T : return sf::Keyboard::T;
                case IKeyboard::U : return sf::Keyboard::U;
                case IKeyboard::V : return sf::Keyboard::V;
                case IKeyboard::W : return sf::Keyboard::W;
                case IKeyboard::X : return sf::Keyboard::X;
                case IKeyboard::Y : return sf::Keyboard::Y;
                case IKeyboard::Z : return sf::Keyboard::Z;
                case IKeyboard::Num0 : return sf::Keyboard::Num0;
                case IKeyboard::Num1 : return sf::Keyboard::Num1;
                case IKeyboard::Num2 : return sf::Keyboard::Num2;
                case IKeyboard::Num3 : return sf::Keyboard::Num3;
                case IKeyboard::Num4 : return sf::Keyboard::Num4;
                case IKeyboard::Num5 : return sf::Keyboard::Num5;
                case IKeyboard::Num6 : return sf::Keyboard::Num6;
                case IKeyboard::Num7 : return sf::Keyboard::Num7;
                case IKeyboard::Num8 : return sf::Keyboard::Num8;
                case IKeyboard::Num9 : return sf::Keyboard::Num9;
                case IKeyboard::Escape : return sf::Keyboard::Escape;      ///< The Escape key
                case IKeyboard::LControl : return sf::Keyboard::LControl;     ///< The left Control key
                case IKeyboard::LShift : return sf::Keyboard::LShift;       ///< The left Shift key
                case IKeyboard::LAlt : return sf::Keyboard::LAlt;         ///< The left Alt key
                case IKeyboard::LSystem : return sf::Keyboard::LSystem;      ///< The left OS specific key: window (Windows and Linux), apple (MacOS X), ...
                case IKeyboard::RControl : return sf::Keyboard::RControl;     ///< The right Control key
                case IKeyboard::RShift : return sf::Keyboard::RShift;       ///< The right Shift key
                case IKeyboard::RAlt : return sf::Keyboard::RAlt;         ///< The right Alt key
                case IKeyboard::RSystem : return sf::Keyboard::RSystem;      ///< The right OS specific key: window (Windows and Linux), apple (MacOS X), ...
                case IKeyboard::Menu : return sf::Keyboard::Menu;         ///< The Menu key
                case IKeyboard::LBracket : return sf::Keyboard::LBracket;     ///< The [ key
                case IKeyboard::RBracket : return sf::Keyboard::RBracket;     ///< The ] key
                case IKeyboard::Semicolon : return sf::Keyboard::Semicolon;    ///< The ; key
                case IKeyboard::Comma : return sf::Keyboard::Comma;        ///< The , key
                case IKeyboard::Period : return sf::Keyboard::Period;       ///< The . key
                case IKeyboard::Quote : return sf::Keyboard::Quote;        ///< The ' key
                case IKeyboard::Slash : return sf::Keyboard::Slash;        ///< The / key
                case IKeyboard::Backslash : return sf::Keyboard::Backslash;    ///< The \ key
                case IKeyboard::Tilde : return sf::Keyboard::Tilde;        ///< The ~ key
                case IKeyboard::Equal : return sf::Keyboard::Equal;        ///< The = key
                case IKeyboard::Hyphen : return sf::Keyboard::Hyphen;       ///< The - key (hyphen)
                case IKeyboard::Space : return sf::Keyboard::Space;        ///< The Space key
                case IKeyboard::Enter : return sf::Keyboard::Enter;        ///< The Enter/Return keys
                case IKeyboard::Backspace : return sf::Keyboard::Backspace;    ///< The Backspace key
                case IKeyboard::Tab : return sf::Keyboard::Tab;          ///< The Tabulation key
                case IKeyboard::PageUp : return sf::Keyboard::PageUp;       ///< The Page up key
                case IKeyboard::PageDown : return sf::Keyboard::PageDown;     ///< The Page down key
                case IKeyboard::End : return sf::Keyboard::End;          ///< The End key
                case IKeyboard::Home : return sf::Keyboard::Home;         ///< The Home key
                case IKeyboard::Insert : return sf::Keyboard::Insert;      ///< The Insert key
                case IKeyboard::Delete : return sf::Keyboard::Delete;       ///< The Delete key
                case IKeyboard::Add : return sf::Keyboard::Add;          ///< The + key
                case IKeyboard::Subtract : return sf::Keyboard::Subtract;     ///< The - key (minus, usually from numpad)
                case IKeyboard::Multiply : return sf::Keyboard::Multiply;     ///< The * key
                case IKeyboard::Divide : return sf::Keyboard::Divide;       ///< The / key
                case IKeyboard::Left : return sf::Keyboard::Left;         ///< Left arrow
                case IKeyboard::Right : return sf::Keyboard::Right;        ///< Right arrow
                case IKeyboard::Up : return sf::Keyboard::Up;           ///< Up arrow
                case IKeyboard::Down : return sf::Keyboard::Down;         ///< Down arrow
                case IKeyboard::Numpad0 : return sf::Keyboard::Numpad0;      ///< The numpad 0 key
                case IKeyboard::Numpad1 : return sf::Keyboard::Numpad1;      ///< The numpad 1 key
                case IKeyboard::Numpad2 : return sf::Keyboard::Numpad2;      ///< The numpad 2 key
                case IKeyboard::Numpad3 : return sf::Keyboard::Numpad3;      ///< The numpad 3 key
                case IKeyboard::Numpad4 : return sf::Keyboard::Numpad4;      ///< The numpad 4 key
                case IKeyboard::Numpad5 : return sf::Keyboard::Numpad5;      ///< The numpad 5 key
                case IKeyboard::Numpad6 : return sf::Keyboard::Numpad6;      ///< The numpad 6 key
                case IKeyboard::Numpad7 : return sf::Keyboard::Numpad7;      ///< The numpad 7 key
                case IKeyboard::Numpad8 : return sf::Keyboard::Numpad8;      ///< The numpad 8 key
                case IKeyboard::Numpad9 : return sf::Keyboard::Numpad9;      ///< The numpad 9 key
                case IKeyboard::F1 : return sf::Keyboard::F1;           ///< The F1 key
                case IKeyboard::F2 : return sf::Keyboard::F2;           ///< The F2 key
                case IKeyboard::F3 : return sf::Keyboard::F3;           ///< The F3 key
                case IKeyboard::F4 : return sf::Keyboard::F4;           ///< The F4 key
                case IKeyboard::F5 : return sf::Keyboard::F5;           ///< The F5 key
                case IKeyboard::F6 : return sf::Keyboard::F6;           ///< The F6 key
                case IKeyboard::F7 : return sf::Keyboard::F7;           ///< The F7 key
                case IKeyboard::F8 : return sf::Keyboard::F8;           ///< The F8 key
                case IKeyboard::F9 : return sf::Keyboard::F9;           ///< The F9 key
                case IKeyboard::F10 : return sf::Keyboard::F10;           ///< The F10 key
                case IKeyboard::F11 : return sf::Keyboard::F11;           ///< The F11 key
                case IKeyboard::F12 : return sf::Keyboard::F12;           ///< The F12 key
                case IKeyboard::F13 : return sf::Keyboard::F13;           ///< The F13 key
                case IKeyboard::F14 : return sf::Keyboard::F14;           ///< The F14 key
                case IKeyboard::F15 : return sf::Keyboard::F15;           ///< The F15 key
                case IKeyboard::Pause : return sf::Keyboard::Pause;        ///< The Pause key
            }
        }
        bool SFMLKeyboard::isKeyPressed(IKeyboard::Key key) {
            return sf::Keyboard::isKeyPressed(odfaegToSfKey(key));
        }
    }
}
