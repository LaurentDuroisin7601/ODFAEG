#ifndef ODFAEG_IMOUSE_HPP
#define ODFAEG_IMOUSE_HPP
#include <SFML/System/Vector2.hpp>
namespace odfaeg {
    namespace window {
        class Window;
        class IMouse {
            public :
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
            static sf::Vector2i getPosition();
            static sf::Vector2i getPosition(const Window& window);
        };
    }
}
#endif
