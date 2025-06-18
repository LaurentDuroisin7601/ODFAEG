#ifndef ODFAEG_WIN32_CURSOR_IMPL_HPP
#define ODFAEG_WIN32_CURSOR_IMPL_HPP
#include "../cursor.hpp"
#include "../../Math/vec4.h"
namespace odfaeg {
    namespace window {
        ////////////////////////////////////////////////////////////
        /// \brief Win32 implementation of Cursor
        ///
        ////////////////////////////////////////////////////////////
        class CursorImpl
        {
        public:

            ////////////////////////////////////////////////////////////
            /// \brief Default constructor
            ///
            /// Refer to sf::Cursor::Cursor().
            ///
            ////////////////////////////////////////////////////////////
            CursorImpl();

            ////////////////////////////////////////////////////////////
            /// \brief Destructor
            ///
            /// Refer to sf::Cursor::~Cursor().
            ///
            ////////////////////////////////////////////////////////////
            ~CursorImpl();

            ////////////////////////////////////////////////////////////
            /// \brief Create a cursor with the provided image
            ///
            /// Refer to sf::Cursor::loadFromPixels().
            ///
            ////////////////////////////////////////////////////////////
            bool loadFromPixels(const std::uint8_t* pixels, math::Vector2u size, math::Vector2u hotspot);

            ////////////////////////////////////////////////////////////
            /// \brief Create a native system cursor
            ///
            /// Refer to sf::Cursor::loadFromSystem().
            ///
            ////////////////////////////////////////////////////////////
            bool loadFromSystem(Cursor::Type type);

        private:

            friend class WindowImplWin32;

            ////////////////////////////////////////////////////////////
            /// \brief Release the cursor, if we have loaded one.
            ///
            ////////////////////////////////////////////////////////////
            void release();

            ////////////////////////////////////////////////////////////
            // Member data
            ////////////////////////////////////////////////////////////
            HCURSOR m_cursor;
        };
    }
}
#endif // ODFAEG_CURSOR_IMPL_HPP
