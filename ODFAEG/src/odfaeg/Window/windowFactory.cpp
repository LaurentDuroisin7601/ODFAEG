#include "../../../include/odfaeg/Window/windowFactory.hpp"
#if defined(SFML)
#include "../../../include/odfaeg/Window/SFML/sfmlWindowImpl.hpp"
typedef odfaeg::window::SFMLWindowImpl WindowType;
#elif defined(SDL)
#include "../../../include/odfaeg/Window/SDL/sdlWindowImpl.hpp"
typedef odfaeg::window::SDLWindowImpl WindowType;
#elif defined(GLFW)
#include "../../../include/odfaeg/Window/GLFW/glfwWindowImpl.hpp"
typedef odfaeg::window::GLFWWindowImpl WindowType;
#elif defined (VULKAN)
#include "../../../include/odfaeg/Window/GLFW/vkGLFWWindow.hpp"
typedef odfaeg::window::VKGLFWWindow WindowType;
#else
#include "../../../include/odfaeg/Window/windowImpl.hpp"
typedef odfaeg::window::WindowImpl WindowType;
#endif
namespace odfaeg {
    namespace window {
        IWindow* WindowFactory::create() {
            return new WindowType();
        }
    }
}
