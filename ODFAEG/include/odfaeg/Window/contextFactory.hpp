#ifndef ODFAEG_WINDOW_FACTORY_HPP
#define ODFAEG_WINDOW_FACTORY_HPP
#include "iContext.hpp"
namespace odfaeg {
    namespace window {
        class ContextFactory {
            public :
            static IContext* create();
        };
    }
}
#endif
