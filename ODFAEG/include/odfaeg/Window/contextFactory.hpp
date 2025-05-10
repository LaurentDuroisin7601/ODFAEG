#ifndef ODFAEG_WINDOW_FACTORY_HPP
#define ODFAEG_WINDOW_FACTORY_HPP
#include "iContext.hpp"
namespace odfaeg {
    namespace window {
        class ODFAEG_WINDOW_API ContextFactory {
            public :
            static IContext* create();
        };
    }
}
#endif
