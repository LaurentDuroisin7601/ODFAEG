#ifndef ODFAEG_SOCKET_IMPL_HPP
#define ODFAEG_SOCKET_IMPL_HPP
#include "../config.hpp"


#if defined(ODFAEG_SYSTEM_WINDOWS)

    #include "Windows/socketImpl.hpp"

#else

    #include "Unix/SocketImpl.hpp"

#endif
#endif // ODFAEG_SOCKET_IMPL_HPP
