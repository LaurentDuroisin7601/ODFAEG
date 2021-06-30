#ifndef ODFAEG_SFML_KEYBOARD_HPP
#define ODFAEG_SFML_KEYBOARD_HPP
#include "../iKeyboard.hpp"
#include <SFML/Window/Keyboard.hpp>
namespace odfaeg {
    namespace window {
        class SFMLKeyboard {
            public :
            static IKeyboard::Key sfToODFAEGKey(sf::Keyboard::Key key);
            static sf::Keyboard::Key odfaegToSfKey(IKeyboard::Key key);
            static bool isKeyPressed(IKeyboard::Key key);
        };
    }
}
#endif
