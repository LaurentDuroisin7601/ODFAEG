#ifndef ODFAEG_SOCKET_HANDLE_HPP
#define ODFAEG_SOCKET_HANDLE_HPP
#include "../config.hpp"
#if defined(ODFAEG_SYSTEM_WINDOWS)
    #include <basetsd.h>
#endif

namespace odfaeg {
    namespace network {
        ////////////////////////////////////////////////////////////
        // Define the low-level socket handle type, specific to
        // each platform
        ////////////////////////////////////////////////////////////
        #if defined(ODFAEG_SYSTEM_WINDOWS)

            typedef UINT_PTR SocketHandle;

        #else

            typedef int SocketHandle;

        #endif
    }
}
#endif // ODFAEG_SOCEKET_HANDLE_HPP
