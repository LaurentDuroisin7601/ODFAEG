#ifndef ODFAEG_WIN32KEYBOARD_HPP
#define ODFAEG_WIN32KEYBOARD_HPP
#include <windows.h>
#ifdef _WIN32_WINDOWS
#undef _WIN32_WINDOWS
#endif
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINDOWS 0x0501
#define _WIN32_WINNT   0x0501
#include "../iKeyBoard.hpp"
namespace odfaeg {
    namespace window {
        class Win32Keyboard {
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
#include "win32Keyboard.inl"
#endif