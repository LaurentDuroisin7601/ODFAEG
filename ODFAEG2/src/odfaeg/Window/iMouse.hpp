#ifndef ODFAEG_IMOUSE_HPP
#define ODFAEG_IMOUSE_HPP
#include <odfaeg/config.hpp>
/*#if defined(ODFAEG_SYSTEM_WINDOWS)
#include "Windows/win32Mouse.hpp"
typedef odfaeg::window::Win32Mouse MouseType;
#else if defined(ODFAEG_SYSTEM_LINUX)
#include "Linux/x11Mouse.hpp"
typedef odfaeg::window::X11Mouse MouseType;
#endif*/
#include <odfaeg/config.hpp>
//#include "window.hpp"
#include "../Math/vec.hpp"
namespace odfaeg {
    namespace window {        
        class  IMouse {
        public:
            ////////////////////////////////////////////////////////////
            /// \brief Mouse buttons
            ///
            ////////////////////////////////////////////////////////////
            enum Button
            {
                Left,       ///< The left mouse button
                Right,      ///< The right mouse button
                Middle,     ///< The middle (wheel) mouse button
                XButton1,   ///< The first extra mouse button
                XButton2,   ///< The second extra mouse button

                ButtonCount ///< Keep last -- the total number of mouse buttons
            };

            ////////////////////////////////////////////////////////////
            /// \brief Mouse wheels
            ///
            ////////////////////////////////////////////////////////////
            enum Wheel
            {
                VerticalWheel,  ///< The vertical mouse wheel
                HorizontalWheel ///< The horizontal mouse wheel
            };
            static bool isButtonPressed(Button button);
            static math::Vector2i getPosition();
            static math::Vector2i getPosition(const Window& window);
        };
    }
}
//#include "iMouse.inl"
#endif