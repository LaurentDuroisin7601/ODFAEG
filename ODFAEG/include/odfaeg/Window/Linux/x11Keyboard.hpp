#ifndef ODFAEG_X11KEYBOARD_HPP
#define ODFAEG_X11KEYBOARD_HPP
#include "../iKeyboard.hpp"
namespace odfaeg {
    namespace window {
        class X11Keyboard {
            public:
                ////////////////////////////////////////////////////////////
                /// \brief Check if a key is pressed
                ///
                /// \param key Key to check
                ///
                /// \return True if the key is pressed, false otherwise
                ///
                ////////////////////////////////////////////////////////////
                static bool isKeyPressed(IKeyboard::Key key);
        };
    }
}
#endif
