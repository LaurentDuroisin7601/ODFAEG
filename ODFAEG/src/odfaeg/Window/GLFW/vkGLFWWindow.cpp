#include "../../../../include/odfaeg/Window/GLFW/vkGLFWWindow.hpp"
#include "../../../../include/odfaeg/Window/GLFW/glfwKeyboard.hpp"
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
        VKGLFWWindow* VKGLFWWindow::currentGLFWWindow = nullptr;
        VKGLFWWindow::VKGLFWWindow() {
            m_settings = ContextSettings(0, 0, 0, 0, 0);
        }
        VKGLFWWindow::VKGLFWWindow(sf::VideoMode mode, const sf::String& title, sf::Uint32 style, const ContextSettings& settings)  {
            m_settings = ContextSettings(settings.depthBits, settings.stencilBits, settings.antiAliasingLevel, settings.versionMajor, settings.versionMinor);
        }
        void VKGLFWWindow::create(sf::VideoMode mode, const sf::String& title, sf::Uint32 style, const ContextSettings& settings) {
            m_settings = ContextSettings(settings.depthBits, settings.stencilBits, settings.antiAliasingLevel, settings.versionMajor, settings.versionMinor);
            glfwInit();
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
            window = glfwCreateWindow(mode.width, mode.height, title.toAnsiString().c_str(), nullptr, nullptr);
            glfwSetKeyCallback(window, key_callback);
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
        void VKGLFWWindow::setTitle(const sf::String& title) {
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
        GLFWwindow* VKGLFWWindow::getWindow() {
            return window;
        }
    }
}
#endif
