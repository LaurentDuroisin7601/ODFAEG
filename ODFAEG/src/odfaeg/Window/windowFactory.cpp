#include "../../../include/odfaeg/Window/windowFactory.hpp"
/*#if defined(SFML)
#include "../../../include/odfaeg/Window/ODFAEG/sfmlWindowImpl.hpp"
typedef odfaeg::window::ODFAEGWindowImpl WindowType;
#elif defined(SDL)
#include "../../../include/odfaeg/Window/SDL/sdlWindowImpl.hpp"
typedef odfaeg::window::SDLWindowImpl WindowType;
#elif defined(GLFW)
#include "../../../include/odfaeg/Window/GLFW/glfwWindowImpl.hpp"
typedef odfaeg::window::GLFWWindowImpl WindowType;
#elif defined (VULKAN)
#include "../../../include/odfaeg/Window/SDL/vkSDLWindow.hpp"
typedef odfaeg::window::SDLWindow WindowType;
#else*/
#include "../../../include/odfaeg/Window/windowImpl.hpp"
typedef odfaeg::window::WindowImpl WindowType;
//#endif
namespace odfaeg {
    namespace window {
        IWindow* WindowFactory::create() {
            //
            try {
                IWindow* window = new WindowType();
                ////std::cout << "create factory" << std::endl;
                return window;
            } catch (const std::bad_alloc& e) {
                std::cerr << "Exception : " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "Exception inconnue pendant new WindowType()" << std::endl;
            }
        }
    }
}
