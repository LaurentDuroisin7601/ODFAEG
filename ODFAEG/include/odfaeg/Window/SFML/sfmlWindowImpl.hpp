#ifndef ODFAEG_SDLWINDOWIMPL
#define ODFAEG_SDLWINDOWIMPL
#include "../iWindow.hpp"
#include "../contextSettings.hpp"
#include <SFML/Window.hpp>
namespace odfaeg {
    namespace window {
        class SFMLWindowImpl : public IWindow, public sf::Window {
        public :
            SFMLWindowImpl();
            SFMLWindowImpl(sf::VideoMode mode, const sf::String& title, sf::Uint32 style, const ContextSettings& settings);
            void create (sf::VideoMode mode, const sf::String& title, sf::Uint32 style, const ContextSettings& settings);
            void create (sf::WindowHandle handle, const ContextSettings& settings);
            bool isOpen() const;
            bool pollEvent(IEvent& event);
            bool waitEvent(IEvent& event);
            void close();
            void setVisible(bool visible);
            sf::Vector2i getPosition() const;
            void setPosition(const sf::Vector2i& position);
            sf::Vector2u getSize() const;
            void setSize(const sf::Vector2u& size);
            void setTitle(const sf::String& title);
			bool filterEvent(const IEvent& event);

            ////////////////////////////////////////////////////////////
            /// \brief Change the window's icon
            ///
            /// \a pixels must be an array of \a width x \a height pixels
            /// in 32-bits RGBA format.
            ///
            /// The OS default icon is used by default.
            ///
            /// \param width  Icon's width, in pixels
            /// \param height Icon's height, in pixels
            /// \param pixels Pointer to the array of pixels in memory. The
            ///               pixels are copied, so you need not keep the
            ///               source alive after calling this function.
            ///
            /// \see setTitle
            ///
            ////////////////////////////////////////////////////////////
            void setIcon(unsigned int width, unsigned int height, const sf::Uint8* pixels);
            ////////////////////////////////////////////////////////////
            /// \brief Show or hide the mouse cursor
            ///
            /// The mouse cursor is visible by default.
            ///
            /// \param visible True to show the mouse cursor, false to hide it
            ///
            ////////////////////////////////////////////////////////////
            void setMouseCursorVisible(bool visible);

            ////////////////////////////////////////////////////////////
            /// \brief Grab or release the mouse cursor
            ///
            /// If set, grabs the mouse cursor inside this window's client
            /// area so it may no longer be moved outside its bounds.
            /// Note that grabbing is only active while the window has
            /// focus.
            ///
            /// \param grabbed True to enable, false to disable
            ///
            ////////////////////////////////////////////////////////////
            void setMouseCursorGrabbed(bool grabbed);

            ////////////////////////////////////////////////////////////
            /// \brief Set the displayed cursor to a native system cursor
            ///
            /// Upon window creation, the arrow cursor is used by default.
            ///
            /// \warning The cursor must not be destroyed while in use by
            ///          the window.
            ///
            /// \warning Features related to Cursor are not supported on
            ///          iOS and Android.
            ///
            /// \param cursor Native system cursor type to display
            ///
            /// \see sf::Cursor::loadFromSystem
            /// \see sf::Cursor::loadFromPixels
            ///
            ////////////////////////////////////////////////////////////
            void setMouseCursor(const sf::Cursor& cursor);

            ////////////////////////////////////////////////////////////
            /// \brief Enable or disable automatic key-repeat
            ///
            /// If key repeat is enabled, you will receive repeated
            /// KeyPressed events while keeping a key pressed. If it is disabled,
            /// you will only get a single event when the key is pressed.
            ///
            /// Key repeat is enabled by default.
            ///
            /// \param enabled True to enable, false to disable
            ///
            ////////////////////////////////////////////////////////////
            void setKeyRepeatEnabled(bool enabled);

            ////////////////////////////////////////////////////////////
            /// \brief Limit the framerate to a maximum fixed frequency
            ///
            /// If a limit is set, the window will use a small delay after
            /// each call to display() to ensure that the current frame
            /// lasted long enough to match the framerate limit.
            /// SFML will try to match the given limit as much as it can,
            /// but since it internally uses sf::sleep, whose precision
            /// depends on the underlying OS, the results may be a little
            /// unprecise as well (for example, you can get 65 FPS when
            /// requesting 60).
            ///
            /// \param limit Framerate limit, in frames per seconds (use 0 to disable limit)
            ///
            ////////////////////////////////////////////////////////////
            void setFramerateLimit(unsigned int limit);

            ////////////////////////////////////////////////////////////
            /// \brief Change the joystick threshold
            ///
            /// The joystick threshold is the value below which
            /// no JoystickMoved event will be generated.
            ///
            /// The threshold value is 0.1 by default.
            ///
            /// \param threshold New threshold, in the range [0, 100]
            ///
            ////////////////////////////////////////////////////////////
            void setJoystickThreshold(float threshold);

            ////////////////////////////////////////////////////////////
            /// \brief Activate or deactivate the window as the current target
            ///        for OpenGL rendering
            ///
            /// A window is active only on the current thread, if you want to
            /// make it active on another thread you have to deactivate it
            /// on the previous thread first if it was active.
            /// Only one window can be active on a thread at a time, thus
            /// the window previously active (if any) automatically gets deactivated.
            /// This is not to be confused with requestFocus().
            ///
            /// \param active True to activate, false to deactivate
            ///
            /// \return True if operation was successful, false otherwise
            ///
            ////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////
            /// \brief Request the current window to be made the active
            ///        foreground window
            ///
            /// At any given time, only one window may have the input focus
            /// to receive input events such as keystrokes or mouse events.
            /// If a window requests focus, it only hints to the operating
            /// system, that it would like to be focused. The operating system
            /// is free to deny the request.
            /// This is not to be confused with setActive().
            ///
            /// \see hasFocus
            ///
            ////////////////////////////////////////////////////////////
            void requestFocus();

            ////////////////////////////////////////////////////////////
            /// \brief Check whether the window has the input focus
            ///
            /// At any given time, only one window may have the input focus
            /// to receive input events such as keystrokes or most mouse
            /// events.
            ///
            /// \return True if window has focus, false otherwise
            /// \see requestFocus
            ///
            ////////////////////////////////////////////////////////////
            bool hasFocus() const;
             ////////////////////////////////////////////////////////////
            /// \brief Get the OS-specific handle of the window
            ///
            /// The type of the returned handle is sf::WindowHandle,
            /// which is a typedef to the handle type defined by the OS.
            /// You shouldn't need to use this function, unless you have
            /// very specific stuff to implement that SFML doesn't support,
            /// or implement a temporary workaround until a bug is fixed.
            ///
            /// \return System handle of the window
            ///
            ////////////////////////////////////////////////////////////
            sf::WindowHandle getSystemHandle() const;
            void destroy();
            bool setActive(bool active=true);
            void setVerticalSyncEnabled(bool enabled);
            void display();
            const ContextSettings& getSettings() const;
        private :
            ContextSettings m_settings;
        };
    }
}
#endif // ODFAEG_SDLWINDOWIMPL

