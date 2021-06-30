#include "../../../include/odfaeg/Window/contextFactory.hpp"
#include "../../../include/odfaeg/config.hpp"
#if defined(SFML)
#include "../../../include/odfaeg/Window/SFML/sfmlContextImpl.hpp"
typedef odfaeg::window::SFMLContextImpl ContextType;
#elif defined(SDL)
#include "../../../include/odfaeg/Window/SDL/sdlContextImpl.hpp"
typedef odfaeg::window::sdlContextImpl ContextType;
#elif defined(GLFW)
#include "../../../include/odfaeg/Window/GLFW/glfwContextImpl.hpp"
typedef odfaeg::window::glfwContextImpl ContextType;
#else
#include "../../../include/odfaeg/Window/contextImpl.hpp"
typedef odfaeg::window::ContextImpl ContextType;
#endif
namespace odfaeg {
    namespace window {
        IContext* ContextFactory::create() {
            return new ContextType();
        }
    }
}
