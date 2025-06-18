#include "../../../../include/odfaeg/Window/GLFW/vkGLFWWindow.hpp"
#include "../../../../include/odfaeg/Window/GLFW/glfwKeyboard.hpp"
#include "../../../../include/odfaeg/Window/GLFW/glfwMouse.hpp"
#ifdef VULKAN

namespace odfaeg {
    namespace window {
        void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            IEvent event;
            event.type = IEvent::KEYBOARD_EVENT;
            if (mods == GLFW_MOD_SHIFT) {
                event.keyboard.shift = true;
            }
            if (mods == GLFW_MOD_CONTROL) {
                event.keyboard.control = true;
            }
            if (mods == GLFW_MOD_ALT) {
                event.keyboard.alt = true;
            }
            if (mods == GLFW_MOD_SUPER) {
                event.keyboard.system = true;
            }
            if (action == GLFW_PRESS) {
                event.keyboard.type    = IEvent::KeyEventID::KEY_EVENT_PRESSED;
            }
            if (action == GLFW_RELEASE) {
                event.keyboard.type    = IEvent::KeyEventID::KEY_EVENT_RELEASED;
            }
            event.keyboard.code = GLFWKeyboard::glfwToODFAEGKey(key);
            VKGLFWWindow::currentGLFWWindow->pushEvent(event);
        }
        void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
        {
            IEvent event;
            event.type = IEvent::EventType::MOUSE_MOTION_EVENT;
            event.mouseMotion.type = IEvent::EventType::MOUSE_MOTION_EVENT;
            event.mouseMotion.x = xpos;
            event.mouseMotion.y = ypos;
            VKGLFWWindow::currentGLFWWindow->pushEvent(event);
        }
        void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
        {
            IEvent event;
            event.type = IEvent::EventType::MOUSE_BUTTON_EVENT;
            if (action == GLFW_PRESS) {
                event.mouseButton.type = IEvent::MouseEventID::BUTTON_EVENT_PRESSED;
            }
            if (action == GLFW_RELEASE) {
                event.mouseButton.type = IEvent::MouseEventID::BUTTON_EVENT_RELEASED;
            }
            event.mouseButton.button = GLFWMouse::glfwToODFAEGButton(button);
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            event.mouseButton.x = xpos;
            event.mouseButton.y = ypos;
            VKGLFWWindow::currentGLFWWindow->pushEvent(event);
        }
        VKGLFWWindow* VKGLFWWindow::currentGLFWWindow = nullptr;
        VKGLFWWindow::VKGLFWWindow() {
            m_settings = ContextSettings(0, 0, 0, 0, 0);
        }
        VKGLFWWindow::VKGLFWWindow(sf::VideoMode mode, const core::String& title, sf::Uint32 style, const ContextSettings& settings)  {
            m_settings = ContextSettings(settings.depthBits, settings.stencilBits, settings.antiAliasingLevel, settings.versionMajor, settings.versionMinor);
        }
        void VKGLFWWindow::create(sf::VideoMode mode, const core::String& title, sf::Uint32 style, const ContextSettings& settings) {
            m_settings = ContextSettings(settings.depthBits, settings.stencilBits, settings.antiAliasingLevel, settings.versionMajor, settings.versionMinor);
            glfwInit();
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
            window = glfwCreateWindow(mode.width, mode.height, title.toAnsiString().c_str(), nullptr, nullptr);
            glfwSetKeyCallback(window, key_callback);
            glfwSetCursorPosCallback(window, cursor_position_callback);
            glfwSetMouseButtonCallback(window, mouse_button_callback);
            opened = true;
        }
        void VKGLFWWindow::create (sf::WindowHandle handle, const ContextSettings& settings) {
        }
        bool VKGLFWWindow::isOpen() const {
            return opened;
        }
        void VKGLFWWindow::pushEvent(IEvent event) {
            events.push(event);
        }
        bool VKGLFWWindow::pollEvent (IEvent& event) {
            currentGLFWWindow = this;
            if (events.empty()) {
                glfwPollEvents();
                if (glfwWindowShouldClose(window)) {
                    IEvent closeEvent;
                    closeEvent.type = IEvent::WINDOW_EVENT;
                    closeEvent.window.type = IEvent::WINDOW_EVENT_CLOSED;
                    events.push(closeEvent);
                    glfwSetWindowShouldClose(window, 0);
                }
            }
            if (!events.empty()) {
                event = events.front();
                events.pop();
                return true;
            }
            return false;
        }
        bool VKGLFWWindow::waitEvent (IEvent& event) {
            return true;
        }
        void VKGLFWWindow::close () {
            opened = false;
        }
        void VKGLFWWindow::setVisible (bool visible) {
        }
        sf::Vector2i VKGLFWWindow::getPosition() const {
            return sf::Vector2i(0, 0);
        }
        void VKGLFWWindow::setPosition(const sf::Vector2i& position) {
        }
        sf::Vector2u VKGLFWWindow::getSize() const {
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            return sf::Vector2u(width, height);
        }
        void VKGLFWWindow::setSize(const sf::Vector2u& size) {
        }
        void VKGLFWWindow::setTitle(const core::String& title) {
        }
        bool VKGLFWWindow::filterEvent(const IEvent& event) {
            return true;
        }
        void VKGLFWWindow::setIcon(unsigned int width, unsigned int height, const sf::Uint8* pixels){
        }
        void VKGLFWWindow::setMouseCursorVisible(bool visible) {
        }
        void VKGLFWWindow::setMouseCursorGrabbed(bool grabbed) {
        }
        void VKGLFWWindow::setMouseCursor(const sf::Cursor& cursor) {
        }
        void VKGLFWWindow::setKeyRepeatEnabled(bool enabled) {
        }
        void VKGLFWWindow::setFramerateLimit(unsigned int limit) {
        }
        void VKGLFWWindow::setJoystickThreshold(float threshold) {
        }
        void VKGLFWWindow::requestFocus() {
        }
        bool VKGLFWWindow::hasFocus() const {
            return true;
        }
        sf::WindowHandle VKGLFWWindow::getSystemHandle() const {
            return sf::WindowHandle();
        }
        void VKGLFWWindow::destroy() {
            glfwDestroyWindow(window);

            glfwTerminate();
        }
        bool VKGLFWWindow::setActive(bool active) {
            return true;
        }
        void VKGLFWWindow::setVerticalSyncEnabled(bool enabled) {
        }
        void VKGLFWWindow::display() {


        }
        const ContextSettings& VKGLFWWindow::getSettings() const {
            return m_settings;
        }
        VkSurfaceKHR VKGLFWWindow::createSurface(VkInstance instance) {
            VkSurfaceKHR surface;
            VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
            if (result != VK_SUCCESS) {
                throw core::Erreur(0, "failed to create window surface!", 1);
            }
            return surface;
        }
        void VKGLFWWindow::getFramebufferSize(int& width, int& height) {
            glfwGetFramebufferSize(window, &width, &height);
        }
    }
}
#endif
