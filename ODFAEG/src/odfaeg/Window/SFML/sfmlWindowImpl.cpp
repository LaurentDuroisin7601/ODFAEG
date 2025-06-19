#include "../../../../include/odfaeg/Window/ODFAEG/sfmlWindowImpl.hpp"
#include "../../../../include/odfaeg/Window/ODFAEG/sfmlKeyboard.hpp"
#include "../../../../include/odfaeg/Window/ODFAEG/sfmlMouse.hpp"
namespace odfaeg {
    namespace window {
        ODFAEGWindowImpl::ODFAEGWindowImpl() : sf::Window() {
            m_settings = ContextSettings(0, 0, 0, 0, 0);
        }
        ODFAEGWindowImpl::ODFAEGWindowImpl(sf::VideoMode mode, const core::String& title, sf::Uint32 style, const ContextSettings& settings) :
        sf::Window (mode, title, style, sf::ContextSettings(settings.depthBits, settings.stencilBits, settings.antiAliasingLevel, settings.versionMajor, settings.versionMinor)) {
            m_settings = ContextSettings(settings.depthBits, settings.stencilBits, settings.antiAliasingLevel, settings.versionMajor, settings.versionMinor);
        }
        void ODFAEGWindowImpl::create (sf::VideoMode mode, const core::String& title, sf::Uint32 style, const ContextSettings& settings) {
            sf::Window::create (mode, title, style, sf::ContextSettings(settings.depthBits, settings.stencilBits, settings.antiAliasingLevel, settings.versionMajor, settings.versionMinor));
            m_settings = ContextSettings(settings.depthBits, settings.stencilBits, settings.antiAliasingLevel, settings.versionMajor, settings.versionMinor);
        }
        void ODFAEGWindowImpl::create (sf::WindowHandle handle, const ContextSettings& settings) {
            sf::Window::create (handle, sf::ContextSettings(settings.depthBits, settings.stencilBits, settings.antiAliasingLevel, settings.versionMajor, settings.versionMinor));
            m_settings = ContextSettings(settings.depthBits, settings.stencilBits, settings.antiAliasingLevel, settings.versionMajor, settings.versionMinor);
        }
        bool ODFAEGWindowImpl::isOpen() const {
            return sf::Window::isOpen();
        }
        bool ODFAEGWindowImpl::pollEvent(IEvent& event) {
            sf::Event sfevent;
            if (sf::Window::pollEvent(sfevent)) {
                if (sfevent.type == sf::Event::Closed) {
                    event.type = IEvent::WINDOW_EVENT;
                    event.window.type = IEvent::WINDOW_EVENT_CLOSED;
                } else if (sfevent.type == sf::Event::Resized) {
                    event.type = IEvent::WINDOW_EVENT;
                    event.window.type = IEvent::WINDOW_EVENT_RESIZED;
                    event.window.data1 = sfevent.size.width;
                    event.window.data2 = sfevent.size.height;
                } else if (sfevent.type == sf::Event::GainedFocus) {
                    event.type = IEvent::WINDOW_EVENT;
                    event.window.type = IEvent::WINDOW_EVENT_FOCUS_GAIGNED;
                } else if (sfevent.type == sf::Event::LostFocus) {
                    event.type = IEvent::WINDOW_EVENT;
                    event.window.type = IEvent::WINDOW_EVENT_FOCUS_LOST;
                } else if (sfevent.type == sf::Event::TextEntered) {
                    event.type = IEvent::TEXT_INPUT_EVENT;
                    event.text.type =  IEvent::TEXT_INPUT_EVENT;
                    event.text.unicode = sfevent.text.unicode;
                } else if (sfevent.type == sf::Event::KeyPressed) {
                    event.type = IEvent::KEYBOARD_EVENT;
                    event.keyboard.type = IEvent::KEY_EVENT_PRESSED;
                    event.keyboard.code = ODFAEGKeyboard::sfToODFAEGKey(sfevent.key.code);
                } else if (sfevent.type == sf::Event::KeyReleased) {
                    event.type = IEvent::KEYBOARD_EVENT;
                    event.keyboard.type = IEvent::KEY_EVENT_RELEASED;
                    event.keyboard.code = ODFAEGKeyboard::sfToODFAEGKey(sfevent.key.code);
                } else if (sfevent.type == sf::Event::MouseButtonPressed) {
                    event.type = IEvent::MOUSE_BUTTON_EVENT;
                    event.mouseButton.type = IEvent::BUTTON_EVENT_PRESSED;
                    event.mouseButton.button = ODFAEGMouse::sfToODFAEGButton(sfevent.mouseButton.button);
                    event.mouseButton.x = sfevent.mouseButton.x;
                    event.mouseButton.y = sfevent.mouseButton.y;
                } else if (sfevent.type == sf::Event::MouseButtonReleased) {
                    event.type = IEvent::MOUSE_BUTTON_EVENT;
                    event.mouseButton.type = IEvent::BUTTON_EVENT_RELEASED;
                    event.mouseButton.button = ODFAEGMouse::sfToODFAEGButton(sfevent.mouseButton.button);
                    event.mouseButton.x = sfevent.mouseButton.x;
                    event.mouseButton.y = sfevent.mouseButton.y;
                } else if (sfevent.type == sf::Event::MouseWheelMoved) {
                    event.type = IEvent::MOUSE_WHEEL_EVENT;
                    event.mouseWheel.type = IEvent::MOUSE_WHEEL_EVENT;
                    event.mouseWheel.direction = sfevent.mouseWheel.delta;
                } else if (sfevent.type == sf::Event::MouseMoved) {
                    event.type = IEvent::MOUSE_MOTION_EVENT;
                    event.mouseMotion.type = IEvent::MOUSE_MOTION_EVENT;
                    event.mouseMotion.x = sfevent.mouseMove.x;
                    event.mouseMotion.y = sfevent.mouseMove.y;
                }
                return true;
            }
            return false;
        }
        bool ODFAEGWindowImpl::filterEvent(const IEvent& event) {
            return true;
        }
        bool ODFAEGWindowImpl::waitEvent(IEvent& event) {
            sf::Event sfevent;
            if (sf::Window::waitEvent(sfevent)) {
                if (sfevent.type == sf::Event::Closed) {
                    event.type = IEvent::WINDOW_EVENT;
                    event.window.type = IEvent::WINDOW_EVENT_CLOSED;
                } else if (sfevent.type == sf::Event::Resized) {
                    event.type = IEvent::WINDOW_EVENT;
                    event.window.type = IEvent::WINDOW_EVENT_RESIZED;
                    event.window.data1 = sfevent.size.width;
                    event.window.data2 = sfevent.size.height;
                } else if (sfevent.type == sf::Event::GainedFocus) {
                    event.type = IEvent::WINDOW_EVENT;
                    event.window.type = IEvent::WINDOW_EVENT_FOCUS_GAIGNED;
                } else if (sfevent.type == sf::Event::LostFocus) {
                    event.type = IEvent::WINDOW_EVENT;
                    event.window.type = IEvent::WINDOW_EVENT_FOCUS_LOST;
                } else if (sfevent.type == sf::Event::TextEntered) {
                    event.type = IEvent::TEXT_INPUT_EVENT;
                    event.text.type =  IEvent::TEXT_INPUT_EVENT;
                    event.text.unicode = sfevent.text.unicode;
                } else if (sfevent.type == sf::Event::KeyPressed) {
                    event.type = IEvent::KEYBOARD_EVENT;
                    event.keyboard.type = IEvent::KEY_EVENT_PRESSED;
                    event.keyboard.code = ODFAEGKeyboard::sfToODFAEGKey(sfevent.key.code);
                } else if (sfevent.type == sf::Event::KeyReleased) {
                    event.type = IEvent::KEYBOARD_EVENT;
                    event.keyboard.type = IEvent::KEY_EVENT_RELEASED;
                    event.keyboard.code = ODFAEGKeyboard::sfToODFAEGKey(sfevent.key.code);
                } else if (sfevent.type == sf::Event::MouseButtonPressed) {
                    event.type = IEvent::MOUSE_BUTTON_EVENT;
                    event.mouseButton.type = IEvent::BUTTON_EVENT_PRESSED;
                    event.mouseButton.button = ODFAEGMouse::sfToODFAEGButton(sfevent.mouseButton.button);
                    event.mouseButton.x = sfevent.mouseButton.x;
                    event.mouseButton.y = sfevent.mouseButton.y;
                }  else if (sfevent.type == sf::Event::MouseButtonReleased) {
                    event.type = IEvent::MOUSE_BUTTON_EVENT;
                    event.mouseButton.type = IEvent::BUTTON_EVENT_RELEASED;
                    event.mouseButton.button = ODFAEGMouse::sfToODFAEGButton(sfevent.mouseButton.button);
                    event.mouseButton.x = sfevent.mouseButton.x;
                    event.mouseButton.y = sfevent.mouseButton.y;
                } else if (sfevent.type == sf::Event::MouseWheelMoved) {
                    event.type = IEvent::MOUSE_WHEEL_EVENT;
                    event.mouseWheel.type = IEvent::MOUSE_WHEEL_EVENT;
                    event.mouseWheel.direction = sfevent.mouseWheel.delta;
                } else if (sfevent.type == sf::Event::MouseMoved) {
                    event.type = IEvent::MOUSE_MOTION_EVENT;
                    event.mouseMotion.type = IEvent::MOUSE_MOTION_EVENT;
                    event.mouseMotion.x = sfevent.mouseMove.x;
                    event.mouseMotion.y = sfevent.mouseMove.y;
                }
                return true;
            }
            return false;
        }
        void ODFAEGWindowImpl::close() {
            sf::Window::close();
        }
        void ODFAEGWindowImpl::setVisible(bool visible) {
            sf::Window::setVisible(visible);
        }
        sf::Vector2i ODFAEGWindowImpl::getPosition() const {
            return sf::Window::getPosition();
        }
        void ODFAEGWindowImpl::setPosition(const sf::Vector2i& position) {
            sf::Window::setPosition(position);
        }
        sf::Vector2u ODFAEGWindowImpl::getSize() const {
            return sf::Window::getSize();
        }
        void ODFAEGWindowImpl::setSize(const sf::Vector2u& size) {
            sf::Window::setSize(size);
        }
        void ODFAEGWindowImpl::setTitle(const core::String& title) {
            sf::Window::setTitle(title);
        }
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
        void ODFAEGWindowImpl::setIcon(unsigned int width, unsigned int height, const sf::Uint8* pixels) {
            sf::Window::setIcon(width, height, pixels);
        }
        ////////////////////////////////////////////////////////////
        /// \brief Show or hide the mouse cursor
        ///
        /// The mouse cursor is visible by default.
        ///
        /// \param visible True to show the mouse cursor, false to hide it
        ///
        ////////////////////////////////////////////////////////////
        void ODFAEGWindowImpl::setMouseCursorVisible(bool visible) {
            sf::Window::setMouseCursorVisible(visible);
        }

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
        void ODFAEGWindowImpl::setMouseCursorGrabbed(bool grabbed) {
            sf::Window::setMouseCursorGrabbed(grabbed);
        }

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
        void ODFAEGWindowImpl::setMouseCursor(const sf::Cursor& cursor) {
            sf::Window::setMouseCursor(cursor);
        }

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
        void ODFAEGWindowImpl::setKeyRepeatEnabled(bool enabled) {
            sf::Window::setKeyRepeatEnabled(enabled);
        }
        ////////////////////////////////////////////////////////////
        /// \brief Limit the framerate to a maximum fixed frequency
        ///
        /// If a limit is set, the window will use a small delay after
        /// each call to display() to ensure that the current frame
        /// lasted long enough to match the framerate limit.
        /// ODFAEG will try to match the given limit as much as it can,
        /// but since it internally uses sf::sleep, whose precision
        /// depends on the underlying OS, the results may be a little
        /// unprecise as well (for example, you can get 65 FPS when
        /// requesting 60).
        ///
        /// \param limit Framerate limit, in frames per seconds (use 0 to disable limit)
        ///
        ////////////////////////////////////////////////////////////
        void ODFAEGWindowImpl::setFramerateLimit(unsigned int limit) {
            sf::Window::setFramerateLimit(limit);
        }

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
        void ODFAEGWindowImpl::setJoystickThreshold(float threshold) {
            sf::Window::setJoystickThreshold(threshold);
        }

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
        void ODFAEGWindowImpl::requestFocus() {
            sf::Window::requestFocus();
        }

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
        bool ODFAEGWindowImpl::hasFocus() const {
            return sf::Window::hasFocus();
        }
         ////////////////////////////////////////////////////////////
        /// \brief Get the OS-specific handle of the window
        ///
        /// The type of the returned handle is sf::WindowHandle,
        /// which is a typedef to the handle type defined by the OS.
        /// You shouldn't need to use this function, unless you have
        /// very specific stuff to implement that ODFAEG doesn't support,
        /// or implement a temporary workaround until a bug is fixed.
        ///
        /// \return System handle of the window
        ///
        ////////////////////////////////////////////////////////////
        sf::WindowHandle ODFAEGWindowImpl::getSystemHandle() const {
            return sf::Window::getSystemHandle();
        }
        void ODFAEGWindowImpl::destroy() {
            sf::Window::close();
        }
        bool ODFAEGWindowImpl::setActive(bool active) {
            return sf::Window::setActive(active);
        }
        void ODFAEGWindowImpl::setVerticalSyncEnabled(bool enabled) {
            sf::Window::setVerticalSyncEnabled(enabled);
        }
        void ODFAEGWindowImpl::display() {
            sf::Window::display();
        }
        const ContextSettings& ODFAEGWindowImpl::getSettings() const {
            return m_settings;
        }
    }
}
