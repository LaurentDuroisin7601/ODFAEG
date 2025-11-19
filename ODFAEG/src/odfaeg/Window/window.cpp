#include "../../../include/odfaeg/Window/window.hpp"


namespace odfaeg {
    namespace window {
        Window::Window() :
        m_window          (NULL)
        {

        }
        ////////////////////////////////////////////////////////////
        Window::Window(VideoMode mode, const core::String& title, std::uint32_t style, const ContextSettings& settings) :
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
        void Window::create(VideoMode mode, const core::String& title, std::uint32_t style, const ContextSettings& settings)
        {


            m_window = WindowFactory::create();
            ////std::cout<<"factory create"<<std::endl;
            m_window->create(mode, title, style, settings);
            ////std::cout<<"window created"<<std::endl;
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
        math::Vector2i Window::getPosition() const
        {
            return m_window ? m_window->getPosition() : math::Vector2i(0, 0);
        }
        ////////////////////////////////////////////////////////////
        void Window::setPosition(const math::Vector2i& position)
        {
            if (m_window)
                m_window->setPosition(position);
        }
        ////////////////////////////////////////////////////////////
        math::Vector2u Window::getSize() const
        {
            return (m_window) ? m_window->getSize() : math::Vector2u(0, 0);
        }


        ////////////////////////////////////////////////////////////
        void Window::setSize(const math::Vector2u& size)
        {
            if (m_window)
            {
                m_window->setSize(size);
                // Notify the derived class
                onResize();
            }
        }


        ////////////////////////////////////////////////////////////
        void Window::setTitle(const core::String& title)
        {
            if (m_window)
                m_window->setTitle(title);
        }


        ////////////////////////////////////////////////////////////
        void Window::setIcon(unsigned int width, unsigned int height, const std::uint8_t* pixels)
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
            drawVulkanFrame();
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
        VkSurfaceKHR Window::createSurface(VkInstance instance) {
            if (m_window)
                return m_window->createSurface(instance);
            return nullptr;
        }
        void Window::getFramebufferSize(int& width, int& height) {
            if (m_window)
                return m_window->getFramebufferSize(width, height);
        }
        #endif
        Window::~Window()
        {
            close();
        }
    }
}
