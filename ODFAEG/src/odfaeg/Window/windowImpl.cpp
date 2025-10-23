#include "../../../include/odfaeg/Window/windowImpl.hpp"
#include <thread>

namespace
{
    const odfaeg::window::WindowImpl* fullscreenWindow = nullptr;
}
namespace odfaeg {
    namespace window {
        WindowImpl::WindowImpl() : WindowImplType()
        {
            m_frameTimeLimit = core::Time::zero;

        }
        ////////////////////////////////////////////////////////////
        void WindowImpl::create(VideoMode mode, const core::String& title, std::uint32_t style, const ContextSettings& settings)
        {
            WindowImplType::destroy();

            // Fullscreen style requires some tests
            if (style & Style::Fullscreen)
            {
                // Make sure there's not already a fullscreen window (only one is allowed)
                if (fullscreenWindow)
                {
                    std::cerr << "Creating two fullscreen windows is not allowed, switching to windowed mode" << std::endl;
                    style &= ~Style::Fullscreen;
                }
                else
                {
                    // Make sure that the chosen video mode is compatible
                    if (!mode.isValid())
                    {
                        std::cerr << "The requested video mode is not available, switching to a valid mode" << std::endl;
                        mode = VideoMode::getFullscreenModes()[0];
                    }

                    // Update the fullscreen window
                    fullscreenWindow = this;
                }
            }

            // Check validity of style according to the underlying platform
            #if defined(ODFAEG_SYSTEM_IOS) || defined(ODFAEG_SYSTEM_ANDROID)
                if (style & Style::Fullscreen)
                    style &= ~Style::Titlebar;
                else
                    style |= Style::Titlebar;
            #else
                if ((style & Style::Close) || (style & Style::Resize))
                    style |= Style::Titlebar;
            #endif
            // Recreate the WindowImpl implementation*/
            WindowImplType::create(mode, title, style, settings);
            #ifndef VULKAN
            m_context.create(getSystemHandle(), settings, nullptr, mode.bitsPerPixel);
            #endif
            initialize();
        }
        ////////////////////////////////////////////////////////////
        void WindowImpl::create(WindowHandle handle, const ContextSettings& settings)
        {
            // Recreate the WindowImpl implementation
            WindowImplType::destroy();
            WindowImplType::create(handle, settings);
            #ifndef VULKAN
            m_context.create(handle, settings);
            #endif
            initialize();
        }
        ////////////////////////////////////////////////////////////
        void WindowImpl::close()
        {
             // Update the fullscreen window
            if (this == fullscreenWindow)
                fullscreenWindow = NULL;
            WindowImplType::close();
        }
        ////////////////////////////////////////////////////////////
        bool WindowImpl::isOpen() const
        {
            return WindowImplType::isOpen();
        }
        ////////////////////////////////////////////////////////////
        bool WindowImpl::pollEvent(IEvent& event)
        {
            return popEvent(event);
        }
        ////////////////////////////////////////////////////////////
        bool WindowImpl::waitEvent(IEvent& event) {
            //For later.
            return false;
        }
        ////////////////////////////////////////////////////////////
        bool WindowImpl::filterEvent(const IEvent& event)
        {
            // Notify resize events to the derived class
            if (event.type == IEvent::WINDOW_EVENT && event.window.type == IEvent::WINDOW_EVENT_RESIZED)
            {
                // Cache the new size
                m_size[0] = event.window.data1;
                m_size[1] = event.window.data2;

            }
            return true;
        }
        ////////////////////////////////////////////////////////////
        math::Vector2i WindowImpl::getPosition() const
        {
            return WindowImplType::getPosition();
        }


        ////////////////////////////////////////////////////////////
        void WindowImpl::setPosition(const math::Vector2i& position)
        {
            WindowImplType::setPosition(position);
        }
        ////////////////////////////////////////////////////////////
        math::Vector2u WindowImpl::getSize() const
        {
            return WindowImplType::getSize();
        }
        ////////////////////////////////////////////////////////////
        void WindowImpl::setSize(const math::Vector2u& size)
        {
            WindowImplType::setSize(size);
        }
        ////////////////////////////////////////////////////////////
        void WindowImpl::setTitle(const core::String& title)
        {
            WindowImplType::setTitle(title);
        }
        ////////////////////////////////////////////////////////////
        void WindowImpl::setIcon(unsigned int width, unsigned int height, const std::uint8_t* pixels)
        {
            WindowImplType::setIcon(width, height, pixels);
        }
        ////////////////////////////////////////////////////////////
        void WindowImpl::setVisible(bool visible)
        {
            WindowImplType::setVisible(visible);
        }
        ////////////////////////////////////////////////////////////
        void WindowImpl::setMouseCursorVisible(bool visible)
        {
            WindowImplType::setMouseCursorVisible(visible);
        }


        ////////////////////////////////////////////////////////////
        void WindowImpl::setMouseCursorGrabbed(bool grabbed)
        {
            WindowImplType::setMouseCursorGrabbed(grabbed);
        }


        ////////////////////////////////////////////////////////////
        void WindowImpl::setMouseCursor(const Cursor& cursor)
        {
            //For later.
        }
        ////////////////////////////////////////////////////////////
        void WindowImpl::setKeyRepeatEnabled(bool enabled)
        {
           WindowImplType::setKeyRepeatEnabled(enabled);
        }
        ////////////////////////////////////////////////////////////
        void WindowImpl::setJoystickThreshold(float threshold)
        {
            //For later.
        }
        ////////////////////////////////////////////////////////////
        void WindowImpl::requestFocus()
        {
            WindowImplType::requestFocus();
        }


        ////////////////////////////////////////////////////////////
        bool WindowImpl::hasFocus() const
        {
            return WindowImplType::hasFocus();
        }
        ////////////////////////////////////////////////////////////
        WindowHandle WindowImpl::getSystemHandle() const
        {
            return WindowImplType::getSystemHandle();
        }
        void WindowImpl::destroy() {
            WindowImplType::destroy();
        }
        bool WindowImpl::setActive(bool active) {
            #ifndef VULKAN
            return m_context.setActive(active);
            #else
            return false;
            #endif
        }
        void WindowImpl::setVerticalSyncEnabled(bool enabled) {
            //////std::cout<<"set vertical sync enable : "<<enabled<<std::endl;
            #ifndef VULKAN
            m_context.setVerticalSyncEnabled(enabled);
            #endif
        }
        void WindowImpl::display() {
            #ifndef VULKAN
            m_context.display();
            #endif
            // Limit the framerate if needed
            if (m_frameTimeLimit != core::Time::zero)
            {
                //////std::cout<<"limit the frame rate"<<std::endl;
                std::this_thread::sleep_for(std::chrono::duration<std::uint32_t, std::milli>((m_frameTimeLimit - m_clock.getElapsedTime()).asMilliseconds()));
                m_clock.restart();
            }
        }
        void WindowImpl::setFramerateLimit(unsigned int limit) {
            if (limit > 0)
                m_frameTimeLimit = core::seconds(1.f / limit);
            else
                m_frameTimeLimit = core::Time::zero;
        }
        void WindowImpl::initialize() {
            setVisible(true);
            setMouseCursorVisible(true);
            setVerticalSyncEnabled(false);
            setKeyRepeatEnabled(true);
            setFramerateLimit(0);

            // Get and cache the initial size of the window
            m_size = WindowImplType::getSize();

            // Reset frame time
            m_clock.restart();

            // Activate the window
            setActive();
        }
        const ContextSettings& WindowImpl::getSettings() const {
            #ifndef VULKAN
            return m_context.getSettings();
            #else
            return ContextSettings(0, 0, 0, 0, 0);
            #endif
        }
        WindowImpl::~WindowImpl()
        {
            close();
        }
    }
}
