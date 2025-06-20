#include "../../../include/odfaeg/Window/iKeyboard.hpp"
#if defined (ODFAEG)
#include "../../../include/odfaeg/Window/ODFAEG/sfmlKeyboard.hpp"
#else
#if defined (ODFAEG_SYSTEM_LINUX)
#include "../../../include/odfaeg/Window/Linux/x11Keyboard.hpp"
#elif defined (ODFAEG_SYSTEM_WINDOWS)
#include "../../../include/odfaeg/Window/Windows/w32Keyboard.hpp"
#endif // defined
#endif
namespace odfaeg {
    namespace window {
        bool IKeyboard::isKeyPressed(Key key) {
            #if defined (ODFAEG)
                return ODFAEGKeyboard::isKeyPressed(key);
            #else
            #if defined(ODFAEG_SYSTEM_LINUX)
                return X11Keyboard::isKeyPressed(key);
            #elif defined (ODFAEG_SYSTEM_WINDOWS)
                return Win32Keyboard::isKeyPressed(key);
            #endif // defined
            #endif
        }
    }
}
