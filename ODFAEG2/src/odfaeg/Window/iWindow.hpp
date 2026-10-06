 #include <vulkan/vulkan.hpp>
 namespace odfaeg {
    namespace graphic {
        class IWindow {
            virtual void create(WindowHandle handle) = 0;
            virtual void create(VideoMode mode, const core::String& title, std::uint32_t style, ContextSettings settings = ContextSettings()) = 0;
            virtual void close() = 0;
            virtual bool isOpen() const = 0;            
            virtual bool  waitEvent(IEvent& event) = 0;
            virtual bool  pollEvent(IEvent& event) = 0;
            virtual math::Vector2i getPosition() const = 0;
            virtual void setPosition(const math::Vector2i& position) = 0;
            virtual math::Vector2u getSize() const = 0;
            virtual void setSize(const math::Vector2u& size) = 0;
            virtual void setTitle(const core::String& title) = 0;
            virtual void setVisible(bool visible) = 0;
            virtual void display() = 0;
            virtual VkSurfaceKHR createSurface(VkInstance instance) {
                return VK_NULL_HANDLE;
            }
            virtual bool setActive(bool acive) {
                
            }
            virtual void getFramebufferSize(int& width, int& height) = 0;
            ////////////////////////////////////////////////////////////
           /// \brief Wait for an event and return it
           ///
           /// This function is blocking: if there's no pending event then
           /// it will wait until an event is received.
           /// After this function returns (and no error occurred),
           /// the \a event object is always valid and filled properly.
           /// This function is typically used when you have a thread that
           /// is dedicated to events handling: you want to make this thread
           /// sleep as long as no new event is received.
           /// \code
           /// window::IEvent event;
           /// if (window.waitEvent(event))
           /// {
           ///    // process event...
           /// }
           /// \endcode
           ///
           /// \param event Event to be returned
           ///
           /// \return False if any error occurred
           ///
           /// \see pollEvent
           ///
           ////////////////////////////////////////////////////////////
           //bool waitEvent(Event& event);
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
            virtual void setIcon(unsigned int width, unsigned int height, const std::uint8_t* pixels) = 0;			
            ////////////////////////////////////////////////////////////
            /// \brief Enable or disable vertical synchronization
            ///
            /// Activating vertical synchronization will limit the number
            /// of frames displayed to the refresh rate of the monitor.
            /// This can avoid some visual artifacts, and limit the framerate
            /// to a good value (but not constant across different computers).
            ///
            /// Vertical synchronization is disabled by default.
            ///
            /// \param enabled True to enable v-sync, false to deactivate it
            ///
            ////////////////////////////////////////////////////////////
            virtual void setVerticalSyncEnabled(bool enabled) = 0;
            ////////////////////////////////////////////////////////////
            /// \brief Show or hide the mouse cursor
            ///
            /// The mouse cursor is visible by default.
            ///
            /// \param visible True to show the mouse cursor, false to hide it
            ///
            ////////////////////////////////////////////////////////////
            virtual void setMouseCursorVisible(bool visible) = 0;

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
            virtual void setMouseCursorGrabbed(bool grabbed) = 0;

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
            virtual void setMouseCursor(const Cursor& cursor) = 0;

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
            virtual void setKeyRepeatEnabled(bool enabled) = 0;

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
            virtual void setFramerateLimit(unsigned int limit) = 0;

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
            void setJoystickThreshold(float threshold) = 0;

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
            void requestFocus() = 0;

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
            bool hasFocus() const = 0;
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
            virtual WindowHandle getSystemHandle() const = 0;
            virtual WindowImpl& getImpl() const = 0;
            virtual bool isVerticalSynchEnabled() = 0;
			virtual void drawVulkanFrame() {
                
            }
            virtual void destroy() = 0;
        };
    }
}