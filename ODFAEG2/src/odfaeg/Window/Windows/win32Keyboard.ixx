module;
export module odfaeg.window.win32Keyboard;
import odfaeg.window.iKeyboard;
namespace odfaeg {
    namespace window {
        export class Win32Keyboard {
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