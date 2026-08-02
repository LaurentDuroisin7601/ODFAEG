#ifndef ODFAEG_WIN32MOUSE_HPP
#define ODFAEG_WIN32MOUSE_HPP
#include <windows.h>
#ifdef _WIN32_WINDOWS
#undef _WIN32_WINDOWS
#endif
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINDOWS 0x0501
#define _WIN32_WINNT   0x0501
#include <odfaeg/Window/windowHandle.hpp>
#include "../iMouse.hpp"
#include "win32Window.hpp"
#include "../../Math/vec.hpp"
namespace odfaeg {
    namespace window {
        class Win32Mouse {
        public:

            ////////////////////////////////////////////////////////////
            /// \brief Check if a mouse button is pressed
            ///
            /// \param button Button to check
            ///
            /// \return True if the button is pressed, false otherwise
            ///
            ////////////////////////////////////////////////////////////
            static bool isButtonPressed(IMouse::Button button);

            ////////////////////////////////////////////////////////////
            /// \brief Get the current position of the mouse in desktop coordinates
            ///
            /// This function returns the global position of the mouse
            /// cursor on the desktop.
            ///
            /// \return Current position of the mouse
            ///
            ////////////////////////////////////////////////////////////
            static math::Vector2i getPosition();

            ////////////////////////////////////////////////////////////
            /// \brief Get the current position of the mouse in window coordinates
            ///
            /// This function returns the current position of the mouse
            /// cursor, relative to the given window.
            ///
            /// \param relativeTo Reference window
            ///
            /// \return Current position of the mouse
            ///
            ////////////////////////////////////////////////////////////
            static math::Vector2i getPosition(const Win32Window& relativeTo);
        };
    }
}
#include "win32Mouse.inl"
#endif