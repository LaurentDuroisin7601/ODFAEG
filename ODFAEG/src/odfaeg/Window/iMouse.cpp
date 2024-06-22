#include "../../../include/odfaeg/Window/iMouse.hpp"
#include "../../../include/odfaeg/Window/window.hpp"
#if defined (SFML)
#include "../../../include/odfaeg/Window/SFML/sfmlMouse.hpp"
#include "../../../include/odfaeg/Window/SFML/sfmlWindowImpl.hpp"
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
            #if defined(SFML)
                return SFMLMouse::isButtonPressed(button);;
            #else
            #if defined (ODFAEG_SYSTEM_LINUX)
                return X11Mouse::isButtonPressed(button);
            #elif defined (ODFAEG_SYSTEM_WINDOWS)
                return Win32Mouse::isButtonPressed(button);
            #endif
            #endif
        }
        sf::Vector2i IMouse::getPosition() {
            #if defined(SFML)
                return SFMLMouse::getPosition();
            #else
            #if defined (ODFAEG_SYSTEM_LINUX)
                return X11Mouse::getPosition();
            #elif defined (ODFAEG_SYSTEM_WINDOWS)
                return Win32Mouse::getPosition();
            #endif
            #endif
        }
        sf::Vector2i IMouse::getPosition(const Window& window) {
            #if defined (SFML)
                return SFMLMouse::getPosition(static_cast<const SFMLWindowImpl&>(*window.getImpl()));
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

