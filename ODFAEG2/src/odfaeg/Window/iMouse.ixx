module;
#include <odfaeg/config.hpp>
//import odfaeg.window.iMouse;
export module odfaeg.window.iMouse;
#if defined (ODFAEG_SYSTEM_WINDOWS)
import odfaeg.window.win32Mouse;
typedef odfaeg::window::Win32Mouse MouseType;
#else if defined(ODFAEG_SYSTEM_LINUX)
import odfaeg.window.x11Mouse;
typedef odfaeg::window::X11Mouse MouseType;
#endif
//import odfaeg.window.window;
import odfaeg.math.vec;
namespace odfaeg {
    namespace window {        
        export class  IMouse {
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
module : private;
#include "iMouse.inl"