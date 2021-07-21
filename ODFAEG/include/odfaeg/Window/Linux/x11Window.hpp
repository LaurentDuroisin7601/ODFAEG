#ifndef ODFAEG_X11WINDOW
#define ODFAEG_X11WINDOW
#include "../contextSettings.hpp"
#include "../iEvent.hpp"
#include <SFML/Window/WindowHandle.hpp>
#include <SFML/System/String.hpp>
#include <SFML/System/Vector2.hpp>
#include <deque>
#include <queue>
#include "../iWindow.hpp"
#include <SFML/Window/VideoMode.hpp>
#include <X11/extensions/Xrandr.h>
namespace odfaeg {
    namespace window {
        class X11Window : public IWindow {
        public :
            X11Window();
            X11Window(sf::VideoMode mode, const sf::String& title, sf::Uint32 style, const ContextSettings& settings);
            X11Window(sf::WindowHandle handle, const ContextSettings& settings);
            virtual void create (sf::WindowHandle handle, const ContextSettings& settings);
            virtual void create (sf::VideoMode mode, const sf::String& title, sf::Uint32 style, const ContextSettings& settings);
            virtual bool isOpen() const;
            virtual void close();
            virtual void setVisible(bool visible);
            virtual sf::WindowHandle getSystemHandle() const;
            unsigned long long int getID();
            virtual void setPosition(const sf::Vector2i& position);
            virtual sf::Vector2i getPosition() const;
            virtual sf::Vector2u getSize() const;
            virtual void setSize(const sf::Vector2u& size);
            virtual void setTitle(const sf::String& title);
            virtual bool pollEvent(IEvent& event);
            virtual bool waitEvent(IEvent& event) {
            }
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
            virtual void setMouseCursor(const sf::Cursor& cursor) {
            }
            ////////////////////////////////////////////////////////////
            /// \brief Set the displayed cursor to a native system cursor
            ///
            /// \param cursor Native system cursor type to display
            ///
            ////////////////////////////////////////////////////////////
            //void setMouseCursor(const sf::Cursor& cursor);

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
            virtual void destroy();
            virtual ~X11Window();
        protected :
            void processEvents();
            bool popEvent (IEvent& event);
        private :
            ////////////////////////////////////////////////////////////
            /// \brief Request the WM to make the current window active
            ///
            ////////////////////////////////////////////////////////////
            void grabFocus();

            ////////////////////////////////////////////////////////////
            /// \brief Set fullscreen video mode
            ///
            /// \param Mode video mode to switch to
            ///
            ////////////////////////////////////////////////////////////
            void setVideoMode(const sf::VideoMode& mode);

            ////////////////////////////////////////////////////////////
            /// \brief Reset to desktop video mode
            ///
            ////////////////////////////////////////////////////////////
            void resetVideoMode();

            ////////////////////////////////////////////////////////////
            /// \brief Switch to fullscreen mode
            ///
            ////////////////////////////////////////////////////////////
            void switchToFullscreen();

            ////////////////////////////////////////////////////////////
            /// \brief Set the WM protocols we support
            ///
            ////////////////////////////////////////////////////////////
            void setProtocols();

            ////////////////////////////////////////////////////////////
            /// \brief Update the last time we received user input
            ///
            /// \param time Last time we received user input
            ///
            ////////////////////////////////////////////////////////////
            void updateLastInputTime(::Time time);

            ////////////////////////////////////////////////////////////
            /// \brief Do some common initializations after the window has been created
            ///
            ////////////////////////////////////////////////////////////
            void initialize();

            ////////////////////////////////////////////////////////////
            /// \brief Create a transparent mouse cursor
            ///
            ////////////////////////////////////////////////////////////
            void createHiddenCursor();

            ////////////////////////////////////////////////////////////
            /// \brief Cleanup graphical resources attached to the window
            ///
            ////////////////////////////////////////////////////////////
            void cleanup();

            ////////////////////////////////////////////////////////////
            /// \brief Process an incoming event from the window
            ///
            /// \param windowEvent Event which has been received
            ///
            /// \return True if the event was processed, false if it was discarded
            ///
            ////////////////////////////////////////////////////////////
            bool processEvent(XEvent& windowEvent);

            ////////////////////////////////////////////////////////////
            /// \brief Check if a valid version of XRandR extension is present
            ///
            /// \param xRandRMajor XRandR major version
            /// \param xRandRMinor XRandR minor version
            ///
            /// \return True if a valid XRandR version found, false otherwise
            ///
            ////////////////////////////////////////////////////////////
            bool checkXRandR(int& xRandRMajor, int& xRandRMinor);

            ////////////////////////////////////////////////////////////
            /// \brief Get the RROutput of the primary monitor
            ///
            /// \param rootWindow the root window
            /// \param res screen resources
            /// \param xRandRMajor XRandR major version
            /// \param xRandRMinor XRandR minor version
            ///
            /// \return RROutput of the primary monitor
            ///
            ////////////////////////////////////////////////////////////
            RROutput getOutputPrimary(::Window& rootWindow, XRRScreenResources* res, int xRandRMajor, int xRandRMinor);

            ////////////////////////////////////////////////////////////
            /// \brief Get coordinates of the primary monitor
            ///
            /// \return Position of the primary monitor
            ///
            ////////////////////////////////////////////////////////////
            sf::Vector2i getPrimaryMonitorPosition();
            void pushEvent(IEvent& event);
            ::Display* m_display;
            ::Window m_window;
            int m_screen;
            bool m_opened;
            bool m_destroyed;
            XIM        m_inputMethod;  ///< Input method linked to the X display
            XIC        m_inputContext; ///< Input context used to get Unicode input in our window
            std::deque<XEvent> m_xevents;
            std::queue<IEvent> m_ievents;
            bool               m_isExternal;     ///< Tell whether the window has been created externally or by SFML
            int                m_oldVideoMode;   ///< Video mode in use before we switch to fullscreen
            RRCrtc             m_oldRRCrtc;      ///< RRCrtc in use before we switch to fullscreen
            ::Cursor           m_hiddenCursor;   ///< As X11 doesn't provide cursor hiding, we must create a transparent one
            ::Cursor           m_lastCursor;     ///< Last cursor used -- this data is not owned by the window and is required to be always valid
            bool               m_keyRepeat;      ///< Is the KeyRepeat feature enabled?
            sf::Vector2i           m_previousSize;   ///< Previous size of the window, to find if a ConfigureNotify event is a resize event (could be a move event only)
            bool               m_useSizeHints;   ///< Is the size of the window fixed with size hints?
            bool               m_fullscreen;     ///< Is the window in fullscreen?
            bool               m_cursorGrabbed;  ///< Is the mouse cursor trapped?
            bool               m_windowMapped;   ///< Has the window been mapped by the window manager?
            Pixmap             m_iconPixmap;     ///< The current icon pixmap if in use
            Pixmap             m_iconMaskPixmap; ///< The current icon mask pixmap if in use
            ::Time             m_lastInputTime;  ///< Last time we received user input
        };
    }
}
//#endif
#endif // ODFAEG_X11WINDOW
