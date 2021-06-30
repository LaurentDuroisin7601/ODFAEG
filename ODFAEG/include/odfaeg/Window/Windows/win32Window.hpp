#ifndef WIN32WINDOW
#define WIN32WINDOW

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif
#include <windows.h>
#include "../contextSettings.hpp"
#include "../iEvent.hpp"
#include <SFML/Window/WindowHandle.hpp>
#include <SFML/System/String.hpp>
#include <SFML/System/Vector2.hpp>
#include <queue>
#include "../iWindow.hpp"
#include <SFML/Window/VideoMode.hpp>
#include "../../../../include/odfaeg/Window/iKeyboard.hpp"
#include "../../../../include/odfaeg/Window/iMouse.hpp"


namespace odfaeg {
    namespace window {
        class Win32Window : public IWindow {
        public :
            Win32Window();
            ////////////////////////////////////////////////////////////
            /// \brief Construct the window implementation from an existing control
            ///
            /// \param handle Platform-specific handle of the control
            ///
            ////////////////////////////////////////////////////////////
            Win32Window(sf::WindowHandle handle, const ContextSettings& settings);

            ////////////////////////////////////////////////////////////
            /// \brief Create the window implementation
            ///
            /// \param mode  Video mode to use
            /// \param title Title of the window
            /// \param style Window style
            /// \param settings Additional settings for the underlying OpenGL context
            ///
            ////////////////////////////////////////////////////////////
            Win32Window(sf::VideoMode mode, const sf::String& title, sf::Uint32 style, const ContextSettings& settings);
            void create (sf::VideoMode mode, const sf::String& title, sf::Uint32 style, const ContextSettings& settings);
            void create(sf::WindowHandle handle, const ContextSettings& settings);

            ////////////////////////////////////////////////////////////
            /// \brief Destructor
            ///
            ////////////////////////////////////////////////////////////
            ~Win32Window();

            ////////////////////////////////////////////////////////////
            /// \brief Get the OS-specific handle of the window
            ///
            /// \return Handle of the window
            ///
            ////////////////////////////////////////////////////////////
            virtual sf::WindowHandle getSystemHandle() const;

            ////////////////////////////////////////////////////////////
            /// \brief Get the position of the window
            ///
            /// \return Position of the window, in pixels
            ///
            ////////////////////////////////////////////////////////////
            virtual sf::Vector2i getPosition() const;

            ////////////////////////////////////////////////////////////
            /// \brief Change the position of the window on screen
            ///
            /// \param position New position of the window, in pixels
            ///
            ////////////////////////////////////////////////////////////
            virtual void setPosition(const sf::Vector2i& position);

            ////////////////////////////////////////////////////////////
            /// \brief Get the client size of the window
            ///
            /// \return Size of the window, in pixels
            ///
            ////////////////////////////////////////////////////////////
            virtual sf::Vector2u getSize() const;

            ////////////////////////////////////////////////////////////
            /// \brief Change the size of the rendering region of the window
            ///
            /// \param size New size, in pixels
            ///
            ////////////////////////////////////////////////////////////
            virtual void setSize(const sf::Vector2u& size);

            ////////////////////////////////////////////////////////////
            /// \brief Change the title of the window
            ///
            /// \param title New title
            ///
            ////////////////////////////////////////////////////////////
            virtual void setTitle(const sf::String& title);

            ////////////////////////////////////////////////////////////
            /// \brief Change the window's icon
            ///
            /// \param width  Icon's width, in pixels
            /// \param height Icon's height, in pixels
            /// \param pixels Pointer to the pixels in memory, format must be RGBA 32 bits
            ///
            ////////////////////////////////////////////////////////////
            virtual void setIcon(unsigned int width, unsigned int height, const sf::Uint8* pixels);

            ////////////////////////////////////////////////////////////
            /// \brief Show or hide the window
            ///
            /// \param visible True to show, false to hide
            ///
            ////////////////////////////////////////////////////////////
            virtual void setVisible(bool visible);

            ////////////////////////////////////////////////////////////
            /// \brief Show or hide the mouse cursor
            ///
            /// \param visible True to show, false to hide
            ///
            ////////////////////////////////////////////////////////////
            virtual void setMouseCursorVisible(bool visible);

            ////////////////////////////////////////////////////////////
            /// \brief Grab or release the mouse cursor
            ///
            /// \param grabbed True to enable, false to disable
            ///
            ////////////////////////////////////////////////////////////
            virtual void setMouseCursorGrabbed(bool grabbed);

            ////////////////////////////////////////////////////////////
            /// \brief Set the displayed cursor to a native system cursor
            ///
            /// \param cursor Native system cursor type to display
            ///
            ////////////////////////////////////////////////////////////
            virtual void setMouseCursor(const sf::Cursor& cursor) {
            }

            ////////////////////////////////////////////////////////////
            /// \brief Enable or disable automatic key-repeat
            ///
            /// \param enabled True to enable, false to disable
            ///
            ////////////////////////////////////////////////////////////
            virtual void setKeyRepeatEnabled(bool enabled);

            ////////////////////////////////////////////////////////////
            /// \brief Request the current window to be made the active
            ///        foreground window
            ///
            ////////////////////////////////////////////////////////////
            virtual void requestFocus();

            ////////////////////////////////////////////////////////////
            /// \brief Check whether the window has the input focus
            ///
            /// \return True if window has focus, false otherwise
            ///
            ////////////////////////////////////////////////////////////
            virtual bool hasFocus() const;
            virtual bool isOpen() const;
            virtual void close();
            virtual void destroy();
            virtual bool pollEvent(IEvent& event);
            virtual bool waitEvent(IEvent& event) {
                return false;
            }
        protected:

            ////////////////////////////////////////////////////////////
            /// \brief Process incoming events from the operating system
            ///
            ////////////////////////////////////////////////////////////
            virtual void processEvents();
            bool popEvent (IEvent& event);

        private:
            void pushEvent(IEvent& event);
            ////////////////////////////////////////////////////////////
            /// Register the window class
            ///
            ////////////////////////////////////////////////////////////
            void registerWindowClass();

            ////////////////////////////////////////////////////////////
            /// \brief Switch to fullscreen mode
            ///
            /// \param mode Video mode to switch to
            ///
            ////////////////////////////////////////////////////////////
            void switchToFullscreen(const sf::VideoMode& mode);

            ////////////////////////////////////////////////////////////
            /// \brief Free all the graphical resources attached to the window
            ///
            ////////////////////////////////////////////////////////////
            void cleanup();

            ////////////////////////////////////////////////////////////
            /// \brief Process a Win32 event
            ///
            /// \param message Message to process
            /// \param wParam  First parameter of the event
            /// \param lParam  Second parameter of the event
            ///
            ////////////////////////////////////////////////////////////
            void processEvent(UINT message, WPARAM wParam, LPARAM lParam);

            ////////////////////////////////////////////////////////////
            /// \brief Enables or disables tracking for the mouse cursor leaving the window
            ///
            /// \param track True to enable, false to disable
            ///
            ////////////////////////////////////////////////////////////
            void setTracking(bool track);

            ////////////////////////////////////////////////////////////
            /// \brief Grab or release the mouse cursor
            ///
            /// This is not to be confused with setMouseCursorGrabbed.
            /// Here m_cursorGrabbed is not modified; it is used,
            /// for example, to release the cursor when switching to
            /// another application.
            ///
            /// \param grabbed True to enable, false to disable
            ///
            ////////////////////////////////////////////////////////////
            void grabCursor(bool grabbed);

            ////////////////////////////////////////////////////////////
            /// \brief Convert a Win32 virtual key code to a SFML key code
            ///
            /// \param key   Virtual key code to convert
            /// \param flags Additional flags
            ///
            /// \return SFML key code corresponding to the key
            ///
            ////////////////////////////////////////////////////////////
            static IKeyboard::Key virtualKeyCodeToODFAEG(WPARAM key, LPARAM flags);

            ////////////////////////////////////////////////////////////
            /// \brief Function called whenever one of our windows receives a message
            ///
            /// \param handle  Win32 handle of the window
            /// \param message Message received
            /// \param wParam  First parameter of the message
            /// \param lParam  Second parameter of the message
            ///
            /// \return True to discard the event after it has been processed
            ///
            ////////////////////////////////////////////////////////////
            static LRESULT CALLBACK globalOnEvent(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);

            ////////////////////////////////////////////////////////////
            // Member data
            ////////////////////////////////////////////////////////////
            HWND     m_handle;           ///< Win32 handle of the window
            LONG_PTR m_callback;         ///< Stores the original event callback function of the control
            bool     m_cursorVisible;    ///< Is the cursor visible or hidden?
            HCURSOR  m_lastCursor;       ///< Last cursor used -- this data is not owned by the window and is required to be always valid
            HICON    m_icon;             ///< Custom icon assigned to the window
            bool     m_keyRepeatEnabled; ///< Automatic key-repeat state for keydown events
            sf::Vector2u m_lastSize;         ///< The last handled size of the window
            bool     m_resizing;         ///< Is the window being resized?
            sf::Uint16   m_surrogate;        ///< First half of the surrogate pair, in case we're receiving a Unicode character in two events
            bool     m_mouseInside;      ///< Mouse is inside the window?
            bool     m_fullscreen;       ///< Is the window fullscreen?
            bool     m_cursorGrabbed;    ///< Is the mouse cursor trapped?
            std::queue<IEvent> m_ievents;
            bool     m_opened, m_destroyed;
        };
    }
}
#endif // WIN32WINDOW
