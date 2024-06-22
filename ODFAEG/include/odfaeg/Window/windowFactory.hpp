#ifndef ODFAEG_WINDOW_FACTORY_HPP
#define ODFAEG_WINDOW_FACTORY_HPP
#include "iWindow.hpp"
namespace odfaeg {
    namespace window {
        class WindowFactory {
            public :
            static IWindow* create();
        };
    }
}
#endif
