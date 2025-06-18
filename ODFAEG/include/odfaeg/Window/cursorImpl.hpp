#ifndef ODFAEG_CURSOR_IMPL
#define ODFAEG_CURSOR_IMPL
#include "../config.hpp"
namespace odfaeg {
    namespace window {
        #if defined(ODFAEG_SYSTEM_WINDOWS)
        #include "Windows/cursorImpl.hpp"
        #endif
    }
}
#endif // ODFAEG_CURSOR_IMPL
