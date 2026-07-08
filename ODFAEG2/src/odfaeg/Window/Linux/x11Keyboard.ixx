export module odfaeg.window.x11Keyboard;
import odfaeg.window.iKeyboard;
namespace odfaeg {
    namespace window {
        export class X11Keyboard {
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
// Created by laurent on 21/05/2026.
//
