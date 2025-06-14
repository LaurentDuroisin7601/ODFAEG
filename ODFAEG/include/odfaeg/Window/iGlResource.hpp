#ifndef ODFAEG_IGLRESOURCE_HPP
#define ODFAEG_IGLRESOURCE_HPP
#include "../../../include/odfaeg/config.hpp"
#ifdef SFML
#include "export.hpp"
#include <SFML/Window/GlResource.hpp>
typedef sf::GlResource GLResourceType;
#else
#include "glResource.hpp"
typedef odfaeg::window::GLResource GLResourceType;
#endif
namespace odfaeg {
    namespace window {
        class ODFAEG_WINDOW_API IGLResource : public  GLResourceType {

        };
    }
}
#endif // defined
