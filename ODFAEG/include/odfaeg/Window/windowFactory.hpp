#ifndef ODFAEG_WINDOW_FACTORY_HPP
#define ODFAEG_WINDOW_FACTORY_HPP
#include "iWindow.hpp"
namespace odfaeg {
    namespace window {
        class ODFAEG_WINDOW_API WindowFactory {
            public :
            static IWindow* create();
        };
    }
}
#endif
