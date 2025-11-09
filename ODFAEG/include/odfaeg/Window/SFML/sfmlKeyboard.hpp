#ifndef ODFAEG_ODFAEG_KEYBOARD_HPP
#define ODFAEG_ODFAEG_KEYBOARD_HPP
#include "../iKeyboard.hpp"
#include <ODFAEG/Window/Keyboard.hpp>
namespace odfaeg {
    namespace window {
        class ODFAEG_WINDOW_API ODFAEGKeyboard {
            public :
            static IKeyboard::Key sfToODFAEGKey(sf::Keyboard::Key key);
            static sf::Keyboard::Key odfaegToSfKey(IKeyboard::Key key);
            static bool isKeyPressed(IKeyboard::Key key);
        };
    }
}
#endif
