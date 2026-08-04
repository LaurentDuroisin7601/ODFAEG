#ifndef ODFAEG_WIN32CURSOR_HPP
#define ODFAEG_WIN32CURSOR_HPP
#include <windows.h>
#include <cstdint>
#include "../iCursorType.hpp"
#include "../../Math/vec.hpp"
namespace odfaeg {
	namespace window {
		class Win32Cursor {
        public:

            ////////////////////////////////////////////////////////////
            /// \brief Default constructor
            ///
            /// Refer to sf::Cursor::Cursor().
            ///
            ////////////////////////////////////////////////////////////
            Win32Cursor();

            ////////////////////////////////////////////////////////////
            /// \brief Destructor
            ///
            /// Refer to sf::Cursor::~Cursor().
            ///
            ////////////////////////////////////////////////////////////
            ~Win32Cursor();

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
            HCURSOR getCursor() const;
        private:
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
#endif