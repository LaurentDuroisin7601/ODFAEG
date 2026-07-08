module;
#include <X11/Xlib.h>
#include <cstdint>
export module odfaeg.window.x11Cursor;
import odfaeg.math.vec;
import odfaeg.window.display;
import odfaeg.window.iCursorType;
namespace odfaeg {
    namespace window {
        export class X11Cursor {
            public:
            ///////////////////////////////////////////////////////////
            /// \brief Default constructor
            ///
            /// Refer to sf::Cursor::Cursor().
            ///
            ////////////////////////////////////////////////////////////
            X11Cursor();

            ////////////////////////////////////////////////////////////
            /// \brief Destructor
            ///
            /// Refer to sf::Cursor::~Cursor().
            ///
            ////////////////////////////////////////////////////////////
            ~X11Cursor();

            ////////////////////////////////////////////////////////////
            /// \brief Create a cursor with the provided image
            ///
            /// Refer to sf::Cursor::loadFromPixels().
            ///
            ////////////////////////////////////////////////////////////
            virtual bool loadFromPixels(const std::uint8_t* pixels, math::Vector2u size, math::Vector2u hotspot);

            ////////////////////////////////////////////////////////////
            /// \brief Create a native system cursor
            ///
            /// Refer to sf::Cursor::loadFromSystem().
            ///
            ////////////////////////////////////////////////////////////
            virtual bool loadFromSystem(ICursorType type);
            ::Cursor getCursor() const;
        private:
            ////////////////////////////////////////////////////////////
            /// \brief Release the cursor, if we have loaded one.
            ///
            ////////////////////////////////////////////////////////////
            void release();

            ////////////////////////////////////////////////////////////
            // Member data
            ////////////////////////////////////////////////////////////
            ::Cursor m_cursor;
            ::Display *m_display;
        };
    }
}
