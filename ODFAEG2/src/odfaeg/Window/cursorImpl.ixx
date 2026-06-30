module;
#include <cstdint>
export module odfaeg.window.cursorImpl;
import odfaeg.window.iCursorType;
import odfaeg.math.vec;
#if defined(ODFAEG_SYSTEM_WINDOWS)
import odfaeg.window.win32Cursor;
typedef odfaeg::window::Win32Cursor CursorImplType;
#else if defined(ODFAEG_SYSTEM_LINUX)
import odfaeg.window.x11Cursor;
typedef odfaeg::window::X11Cursor CursorImplType;
#endif
namespace odfaeg {
    namespace window {
        ////////////////////////////////////////////////////////////
        /// \brief Cursor defines the appearance of a system cursor
        ///
        ////////////////////////////////////////////////////////////
		export class CursorImpl : public CursorImplType
        { 
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
            CursorImpl();
            

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
            bool loadFromSystem(ICursorType type);

        private:

            friend class WindowImpl;

            ////////////////////////////////////////////////////////////
            /// \brief Get access to the underlying implementation
            ///
            /// This is primarily designed for sf::Window::setMouseCursor,
            /// hence the friendship.
            ///
            /// \return a reference to the OS-specific implementation
            ///
            ////////////////////////////////////////////////////////////
            const CursorImplType& getImplType() const;        
        };

    }
}