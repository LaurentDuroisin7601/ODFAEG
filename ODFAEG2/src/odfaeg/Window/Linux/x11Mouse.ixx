module;
export module odfaeg.window.x11Mouse;
import odfaeg.window.iMouse;
import odfaeg.window.x11Window;
import odfaeg.math.vec;
namespace odfaeg {
    namespace window {
        export class X11Mouse {
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
            static math::Vector2i getPosition(const X11Window& relativeTo);
        };
    }
}
// Created by laurent on 21/05/2026.
//
