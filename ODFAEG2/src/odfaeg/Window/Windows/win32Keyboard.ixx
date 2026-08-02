module;
#include <windows.h>
#ifdef _WIN32_WINDOWS
#undef _WIN32_WINDOWS
#endif
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINDOWS 0x0501
#define _WIN32_WINNT   0x0501
//import odfaeg.window.win32Keyboard;
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
module : private;
#include "win32Keyboard.inl"