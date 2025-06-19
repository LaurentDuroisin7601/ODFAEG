#include "../../../include/odfaeg/Window/iMouse.hpp"
#include "../../../include/odfaeg/Window/window.hpp"
#if defined (ODFAEG)
#include "../../../include/odfaeg/Window/ODFAEG/sfmlMouse.hpp"
#include "../../../include/odfaeg/Window/ODFAEG/sfmlWindowImpl.hpp"
#else
#if defined (ODFAEG_SYSTEM_LINUX)
#include "../../../include/odfaeg/Window/Linux/x11Mouse.hpp"
#elif defined (ODFAEG_SYSTEM_WINDOWS)
#include "../../../include/odfaeg/Window/Windows/win32Mouse.hpp"
#endif
#endif
namespace odfaeg {
    namespace window {
        bool IMouse::isButtonPressed (Button button) {
            #if defined(ODFAEG)
                return ODFAEGMouse::isButtonPressed(button);;
            #else
            #if defined (ODFAEG_SYSTEM_LINUX)
                return X11Mouse::isButtonPressed(button);
            #elif defined (ODFAEG_SYSTEM_WINDOWS)
                return Win32Mouse::isButtonPressed(button);
            #endif
            #endif
        }
        math::Vector2i IMouse::getPosition() {
            #if defined(ODFAEG)
                return ODFAEGMouse::getPosition();
            #else
            #if defined (ODFAEG_SYSTEM_LINUX)
                return X11Mouse::getPosition();
            #elif defined (ODFAEG_SYSTEM_WINDOWS)
                return Win32Mouse::getPosition();
            #endif
            #endif
        }
        math::Vector2i IMouse::getPosition(const Window& window) {
            #if defined (ODFAEG)
                return ODFAEGMouse::getPosition(static_cast<const ODFAEGWindowImpl&>(*window.getImpl()));
            #else
            #if defined (ODFAEG_SYSTEM_LINUX)
                return X11Mouse::getPosition(static_cast<const X11Window&>(*window.getImpl()));
            #elif defined (ODFAEG_SYSTEM_WINDOWS)
                return Win32Mouse::getPosition(static_cast<const Win32Window&> (*window.getImpl()));
            #endif
            #endif
        }
    }
}

