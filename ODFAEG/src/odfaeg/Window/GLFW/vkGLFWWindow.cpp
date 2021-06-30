#include "../../../../include/odfaeg/Window/GLFW/vkGLFWWindow.hpp"
#ifdef VULKAN
namespace odfaeg {
    namespace window {
        VKGLFWWindow::VKGLFWWindow() : sf::Window() {
            m_settings = ContextSettings(0, 0, 0, 0, 0);
        }
        VKGLFWWindow::VKGLFWWindow(sf::VideoMode mode, const sf::String& title, sf::Uint32 style, const ContextSettings& settings) :
        sf::Window (mode, title, style, sf::ContextSettings(settings.depthBits, settings.stencilBits, settings.antiAliasingLevel, settings.versionMajor, settings.versionMinor)) {
            m_settings = ContextSettings(settings.depthBits, settings.stencilBits, settings.antiAliasingLevel, settings.versionMajor, settings.versionMinor);
        }
        void VKGLFWWindow::create(sf::VideoMode mode, const sf::String& title, sf::Uint32 style, const ContextSettings& settings) {
            m_settings = ContextSettings(settings.depthBits, settings.stencilBits, settings.antiAliasingLevel, settings.versionMajor, settings.versionMinor);
            gflwInit();
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
            window = glfwCreateWindow(mode.width, mode.height, title.toAnsiString().c_str(), nullptr, nullptr);
            vkSettup.setCreateSurface(true, window);
            vkSettup.initVulkan();
            opened = true;
        }
        void VKGLFWWindow::create (sf::WindowHandle handle, const ContextSettings& settings) {
        }
        bool VKGLFWWindow::isOpen() const {
            return isOpen();
        }
        bool VKGLFWWindow::pollEvent (IEvent& event) {
            currentGLFWWindow = this;
            if (event.empty()) {
                glfwPollEvents();
                if (glfwWindowShouldClose()) {
                    IEvent closeEvent;
                    closeEvent.type = IEvent::WINDOW_EVENT;
                    closeEvent.window.type = IEvent::WINDOW_EVENT_CLOSED;
                    events.push(closeEvent);
                }
            }
            event = events.front();
            event.pop();
        }
        bool VKGLFWWindow::waitEvent (IEvent& event) {
        }
        void VKGLFWWindow::close () {
            opened = false;
        }
        void VKGLFWWindow::setVisible (bool visible) {
        }
        sf::Vector2i VKGLFWWindow::getPosition() const {
        }
        void VKGLFWWindow::setPosition(const sf::Vector2i& position) {
        }
        sf::Vector2u VKGLFWWindow::getSize() const {
        }
        void VKGLFWWindow::setSize(const sf::Vector2u& size) {
        }
        void VKGLFWWindow::setTitle(const sf::String& title) {
        }
        bool VKGLFWWindow::filterEvent(const IEvent& event) {
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
        }
        sf::WindowHandle VKGLFWWindow::getSystemHandle() const {
        }
        void VKGLFWWindow::destroy() {
            glfwDestroyWindow(window);

            glfwTerminate();
        }
        bool VKGLFWWindow::setActive(bool active=true) {

        }
        void VKGLFWWindow::setVerticalSyncEnabled(bool enabled) {
        }
        void VKGLFWWindow::display() {
            vkSettup.drawFrame();
            vkDeviceWaitIdle(vkSettup.getDevice());
        }
        const ContextSettings& VKGLFWWindow::getSettings() const {
            return m_settings;
        }
        VkSettup& VKGLFWWindow::getVkSettup() {
            return vkSettup
        }
    }
}
#endif
