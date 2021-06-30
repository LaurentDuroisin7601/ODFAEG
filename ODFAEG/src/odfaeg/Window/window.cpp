#include "../../../include/odfaeg/Window/window.hpp"

using namespace sf;
namespace odfaeg {
    namespace window {
        Window::Window() :
        m_window          (NULL)
        {

        }
        ////////////////////////////////////////////////////////////
        Window::Window(VideoMode mode, const String& title, Uint32 style, const ContextSettings& settings) :
        m_window         (NULL)
        {
            create(mode, title, style, settings);
        }
        ////////////////////////////////////////////////////////////
        Window::Window(WindowHandle handle, const ContextSettings& settings) :
        m_window          (NULL)
        {
            create(handle, settings);
        }////////////////////////////////////////////////////////////
        void Window::create(VideoMode mode, const String& title, Uint32 style, const ContextSettings& settings)
        {

            m_window = WindowFactory::create();
            m_window->create(mode, title, style, settings);
            // Perform common initializations
            initialize();
        }
        ////////////////////////////////////////////////////////////
        void Window::create(WindowHandle handle, const ContextSettings& settings)
        {
            m_window = WindowFactory::create();
            m_window->create(handle, settings);
            initialize();
        }
        ////////////////////////////////////////////////////////////
        void Window::close()
        {
            m_window->close();
        }
        ////////////////////////////////////////////////////////////
        bool Window::isOpen() const
        {
            return m_window != NULL && m_window->isOpen();
        }
        ////////////////////////////////////////////////////////////
        const ContextSettings& Window::getSettings() const
        {
            static const ContextSettings empty(0, 0, 0, 0, 0);

            return m_window ? m_window->getSettings() : empty;
        }
        ////////////////////////////////////////////////////////////
        bool Window::pollEvent(IEvent& event)
        {
            if (m_window && m_window->pollEvent(event))
            {
                return m_window->filterEvent(event);
            }
            else
            {
                return false;
            }
        }
        ////////////////////////////////////////////////////////////
        bool Window::waitEvent(IEvent& event)
        {
            if (m_window && m_window->waitEvent(event))
            {
                return m_window->filterEvent(event);
            }
            else
            {
                return false;
            }
        }
        ////////////////////////////////////////////////////////////
        Vector2i Window::getPosition() const
        {
            return m_window ? m_window->getPosition() : Vector2i();
        }
        ////////////////////////////////////////////////////////////
        void Window::setPosition(const Vector2i& position)
        {
            if (m_window)
                m_window->setPosition(position);
        }
        ////////////////////////////////////////////////////////////
        Vector2u Window::getSize() const
        {
            return (m_window) ? m_window->getSize() : sf::Vector2u(0, 0);
        }


        ////////////////////////////////////////////////////////////
        void Window::setSize(const Vector2u& size)
        {
            if (m_window)
            {
                m_window->setSize(size);
                // Notify the derived class
                onResize();
            }
        }


        ////////////////////////////////////////////////////////////
        void Window::setTitle(const String& title)
        {
            if (m_window)
                m_window->setTitle(title);
        }


        ////////////////////////////////////////////////////////////
        void Window::setIcon(unsigned int width, unsigned int height, const Uint8* pixels)
        {
            if (m_window)
                m_window->setIcon(width, height, pixels);
        }


        ////////////////////////////////////////////////////////////
        void Window::setVisible(bool visible)
        {
            if (m_window)
                m_window->setVisible(visible);
        }


        ////////////////////////////////////////////////////////////
        void Window::setVerticalSyncEnabled(bool enabled)
        {
            if (m_window->setActive())
                m_window->setVerticalSyncEnabled(enabled);
        }


        ////////////////////////////////////////////////////////////
        void Window::setMouseCursorVisible(bool visible)
        {
            if (m_window)
                m_window->setMouseCursorVisible(visible);
        }


        ////////////////////////////////////////////////////////////
        void Window::setMouseCursorGrabbed(bool grabbed)
        {
            if (m_window)
                m_window->setMouseCursorGrabbed(grabbed);
        }


        ////////////////////////////////////////////////////////////
        void Window::setMouseCursor(const Cursor& cursor)
        {
            if (m_window)
                m_window->setMouseCursor(cursor);
        }


        ////////////////////////////////////////////////////////////
        void Window::setKeyRepeatEnabled(bool enabled)
        {
            if (m_window)
                m_window->setKeyRepeatEnabled(enabled);
        }


        ////////////////////////////////////////////////////////////
        void Window::setFramerateLimit(unsigned int limit)
        {
            m_window->setFramerateLimit(limit);
        }


        ////////////////////////////////////////////////////////////
        void Window::setJoystickThreshold(float threshold)
        {
            if (m_window)
                m_window->setJoystickThreshold(threshold);
        }


        ////////////////////////////////////////////////////////////
        bool Window::setActive(bool active)
        {
            if (m_window)
            {
                if (m_window->setActive(active)) {
                    return true;
                }
                else
                {
                    std::cerr << "Failed to activate the window's context" << std::endl;
                    return false;
                }
            }
            else
            {
                return false;
            }
        }


        ////////////////////////////////////////////////////////////
        void Window::requestFocus()
        {
            if (m_window)
                m_window->requestFocus();
        }


        ////////////////////////////////////////////////////////////
        bool Window::hasFocus() const
        {
            return m_window && m_window->hasFocus();
        }


        ////////////////////////////////////////////////////////////

        void Window::display()
        {
            if (m_window->setActive())
                m_window->display();
        }


        ////////////////////////////////////////////////////////////
        WindowHandle Window::getSystemHandle() const
        {
            return m_window ? m_window->getSystemHandle() : 0;
        }


        ////////////////////////////////////////////////////////////
        void Window::onCreate()
        {
            // Nothing by default
        }


        ////////////////////////////////////////////////////////////
        void Window::onResize()
        {
            // Nothing by default
        }
        ////////////////////////////////////////////////////////////
        void Window::initialize()
        {
            // Notify the derived class
            onCreate();
        }
        IWindow* Window::getImpl() const {
            return m_window;
        }
        #ifdef VULKAN
        VkSettup& Window::getVkSettup() {
            return m_window->getVkSettup();
        }
        #endif // VULKAN
        Window::~Window()
        {
            close();
        }
    }
}
