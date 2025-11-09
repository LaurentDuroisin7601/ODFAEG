#include "../../../../include/odfaeg/Window/SDL/vkSDLWindow.hpp"

#ifdef VULKAN

namespace odfaeg {
    namespace window {
        unsigned int SDLWindow::nbWindows = 0;
        SDLWindow::SDLWindow() {
            m_settings = ContextSettings(0, 0, 0, 0, 0);
        }
        SDLWindow::SDLWindow(VideoMode mode, const core::String& title, std::uint32_t style, const ContextSettings& settings)  {
            m_settings = ContextSettings(settings.depthBits, settings.stencilBits, settings.antiAliasingLevel, settings.versionMajor, settings.versionMinor);
        }
        void SDLWindow::create(VideoMode mode, const core::String& title, std::uint32_t style, const ContextSettings& settings) {
            m_settings = ContextSettings(settings.depthBits, settings.stencilBits, settings.antiAliasingLevel, settings.versionMajor, settings.versionMinor);
            if (nbWindows == 0) {
                 if (SDL_Init(SDL_INIT_VIDEO) < 0)
                 {
                    printf("Couldn't initialize SDL: %s\n", SDL_GetError());
                    exit(1);
                 }
            }
            Uint32 flags;
            if (style == Style::Fullscreen) {
                flags = SDL_WINDOW_FULLSCREEN;
            } else if (style == Style::None) {
                flags = SDL_WINDOW_BORDERLESS;
            } else {
                if (style == Style::Resize || style == Style::Default) {
                    flags = SDL_WINDOW_RESIZABLE;
                }
            }
            window = SDL_CreateWindow(title.toAnsiString().c_str(),  mode.width, mode.height, flags | SDL_WINDOW_VULKAN);
            if (window == NULL) {
                printf("Couldn't create window : %s\n", SDL_GetError());
                exit(1);
            }
            nbWindows++;
            opened = true;
        }
        void SDLWindow::create (WindowHandle handle, const ContextSettings& settings) {
        }
        bool SDLWindow::isOpen() const {
            return opened;
        }
        void SDLWindow::pushEvent(IEvent event) {
            events.push(event);
        }
        bool SDLWindow::pollEvent (IEvent& event) {
            if (events.empty()) {
                SDL_Event event;
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_EVENT_QUIT) {
                        IEvent closeEvent;
                        closeEvent.type = IEvent::WINDOW_EVENT;
                        closeEvent.window.type = IEvent::WINDOW_EVENT_CLOSED;
                        events.push(closeEvent);
                    }
                }
            }
            if (!events.empty()) {
                event = events.front();
                events.pop();
                return true;
            }
            return false;
        }
        bool SDLWindow::waitEvent (IEvent& event) {
            return true;
        }
        void SDLWindow::close () {
            opened = false;
        }
        void SDLWindow::setVisible (bool visible) {
        }
        math::Vector2i SDLWindow::getPosition() const {
            return math::Vector2i(0, 0);
        }
        void SDLWindow::setPosition(const math::Vector2i& position) {
        }
        math::Vector2u SDLWindow::getSize() const {
            int width, height;
            SDL_GetWindowSize(window, &width, &height);
            return math::Vector2u(width, height);
        }
        void SDLWindow::setSize(const math::Vector2u& size) {
        }
        void SDLWindow::setTitle(const core::String& title) {
        }
        bool SDLWindow::filterEvent(const IEvent& event) {
            return true;
        }
        void SDLWindow::setIcon(unsigned int width, unsigned int height, const std::uint8_t* pixels){
        }
        void SDLWindow::setMouseCursorVisible(bool visible) {
        }
        void SDLWindow::setMouseCursorGrabbed(bool grabbed) {
        }
        void SDLWindow::setMouseCursor(const Cursor& cursor) {
        }
        void SDLWindow::setKeyRepeatEnabled(bool enabled) {
        }
        void SDLWindow::setFramerateLimit(unsigned int limit) {
        }
        void SDLWindow::setJoystickThreshold(float threshold) {
        }
        void SDLWindow::requestFocus() {
        }
        bool SDLWindow::hasFocus() const {
            return true;
        }
        WindowHandle SDLWindow::getSystemHandle() const {
            return WindowHandle();
        }
        void SDLWindow::destroy() {
            SDL_DestroyWindow(window);
        }
        bool SDLWindow::setActive(bool active) {
            return true;
        }
        void SDLWindow::setVerticalSyncEnabled(bool enabled) {
        }
        void SDLWindow::display() {


        }
        const ContextSettings& SDLWindow::getSettings() const {
            return m_settings;
        }
        VkSurfaceKHR SDLWindow::createSurface(VkInstance instance) {
            VkSurfaceKHR surface;

            bool result = SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface);
            if (result < 0) {
                printf("Couldn't create vulkan surface: %s\n", SDL_GetError());
                exit(1);
            }
            return surface;
        }
        void SDLWindow::getFramebufferSize(int& width, int& height) {
            SDL_GetWindowSizeInPixels(window, &width, &height);
        }
    }
}
#endif
