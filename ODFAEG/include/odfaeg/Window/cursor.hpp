#ifndef ODFAEG_CURSOR_HPP
#define ODFAEG_CURSOR_HPP
#include "export.hpp"
#include "../Math/vec4.h"
namespace odfaeg {
    namespace window {
        class CursorImpl;
        ////////////////////////////////////////////////////////////
        /// \brief Cursor defines the appearance of a system cursor
        ///
        ////////////////////////////////////////////////////////////
        class ODFAEG_WINDOW_API Cursor
        {
        public:

            ////////////////////////////////////////////////////////////
            /// \brief Enumeration of the native system cursor types
            ///
            /// Refer to the following table to determine which cursor
            /// is available on which platform.
            ///
            ///  Type                               | Linux | Mac OS X | Windows  |
            /// ------------------------------------|:-----:|:--------:|:--------:|
            ///  sf::Cursor::Arrow                  |  yes  |    yes   |   yes    |
            ///  sf::Cursor::ArrowWait              |  no   |    no    |   yes    |
            ///  sf::Cursor::Wait                   |  yes  |    no    |   yes    |
            ///  sf::Cursor::Text                   |  yes  |    yes   |   yes    |
            ///  sf::Cursor::Hand                   |  yes  |    yes   |   yes    |
            ///  sf::Cursor::SizeHorizontal         |  yes  |    yes   |   yes    |
            ///  sf::Cursor::SizeVertical           |  yes  |    yes   |   yes    |
            ///  sf::Cursor::SizeTopLeftBottomRight |  no   |    yes*  |   yes    |
            ///  sf::Cursor::SizeBottomLeftTopRight |  no   |    yes*  |   yes    |
            ///  sf::Cursor::SizeAll                |  yes  |    no    |   yes    |
            ///  sf::Cursor::Cross                  |  yes  |    yes   |   yes    |
            ///  sf::Cursor::Help                   |  yes  |    yes*  |   yes    |
            ///  sf::Cursor::NotAllowed             |  yes  |    yes   |   yes    |
            ///
            ///  * These cursor types are undocumented so may not
            ///    be available on all versions, but have been tested on 10.13
            ///
            ////////////////////////////////////////////////////////////
            enum Type
            {
                Arrow,                  ///< Arrow cursor (default)
                ArrowWait,              ///< Busy arrow cursor
                Wait,                   ///< Busy cursor
                Text,                   ///< I-beam, cursor when hovering over a field allowing text entry
                Hand,                   ///< Pointing hand cursor
                SizeHorizontal,         ///< Horizontal double arrow cursor
                SizeVertical,           ///< Vertical double arrow cursor
                SizeTopLeftBottomRight, ///< Double arrow cursor going from top-left to bottom-right
                SizeBottomLeftTopRight, ///< Double arrow cursor going from bottom-left to top-right
                SizeAll,                ///< Combination of SizeHorizontal and SizeVertical
                Cross,                  ///< Crosshair cursor
                Help,                   ///< Help cursor
                NotAllowed              ///< Action not allowed cursor
            };

        public:

            ////////////////////////////////////////////////////////////
            /// \brief Default constructor
            ///
            /// This constructor doesn't actually create the cursor;
            /// initially the new instance is invalid and must not be
            /// used until either loadFromPixels() or loadFromSystem()
            /// is called and successfully created a cursor.
            ///
            ////////////////////////////////////////////////////////////
            Cursor();

            ////////////////////////////////////////////////////////////
            /// \brief Destructor
            ///
            /// This destructor releases the system resources
            /// associated with this cursor, if any.
            ///
            ////////////////////////////////////////////////////////////
            ~Cursor();

            ////////////////////////////////////////////////////////////
            /// \brief Create a cursor with the provided image
            ///
            /// \a pixels must be an array of \a width by \a height pixels
            /// in 32-bit RGBA format. If not, this will cause undefined behavior.
            ///
            /// If \a pixels is null or either \a width or \a height are 0,
            /// the current cursor is left unchanged and the function will
            /// return false.
            ///
            /// In addition to specifying the pixel data, you can also
            /// specify the location of the hotspot of the cursor. The
            /// hotspot is the pixel coordinate within the cursor image
            /// which will be located exactly where the mouse pointer
            /// position is. Any mouse actions that are performed will
            /// return the window/screen location of the hotspot.
            ///
            /// \warning On Unix, the pixels are mapped into a monochrome
            ///          bitmap: pixels with an alpha channel to 0 are
            ///          transparent, black if the RGB channel are close
            ///          to zero, and white otherwise.
            ///
            /// \param pixels   Array of pixels of the image
            /// \param size     Width and height of the image
            /// \param hotspot  (x,y) location of the hotspot
            /// \return true if the cursor was successfully loaded;
            ///         false otherwise
            ///
            ////////////////////////////////////////////////////////////
            bool loadFromPixels(const std::uint8_t* pixels, math::Vector2u size, math::Vector2u hotspot);

            ////////////////////////////////////////////////////////////
            /// \brief Create a native system cursor
            ///
            /// Refer to the list of cursor available on each system
            /// (see sf::Cursor::Type) to know whether a given cursor is
            /// expected to load successfully or is not supported by
            /// the operating system.
            ///
            /// \param type Native system cursor type
            /// \return true if and only if the corresponding cursor is
            ///         natively supported by the operating system;
            ///         false otherwise
            ///
            ////////////////////////////////////////////////////////////
            bool loadFromSystem(Type type);

        private:

            friend class Window;

            ////////////////////////////////////////////////////////////
            /// \brief Get access to the underlying implementation
            ///
            /// This is primarily designed for sf::Window::setMouseCursor,
            /// hence the friendship.
            ///
            /// \return a reference to the OS-specific implementation
            ///
            ////////////////////////////////////////////////////////////
            const CursorImpl& getImpl() const;

        private:

            ////////////////////////////////////////////////////////////
            // Member data
            ////////////////////////////////////////////////////////////
            CursorImpl* m_impl; ///< Platform-specific implementation of the cursor
        };

    }
}
#endif // ODFAEG_CURSOR_HPP
