#ifndef SOCKET_IMPL_WINDOWS_HPP
#define SOCKET_IMPL_WINDOWS_HPP
#ifdef _WIN32_WINDOWS
    #undef _WIN32_WINDOWS
#endif
#ifdef _WIN32_WINNT
    #undef _WIN32_WINNT
#endif
#define _WIN32_WINDOWS 0x0501
#define _WIN32_WINNT   0x0501
#include "../socket.hpp"
#include <winsock2.h>
#include <ws2tcpip.h>
namespace odfaeg {
    namespace network {
        ////////////////////////////////////////////////////////////
        /// \brief Helper class implementing all the non-portable
        ///        socket stuff; this is the Windows version
        ///
        ////////////////////////////////////////////////////////////
        class SocketImpl
        {
        public:

            ////////////////////////////////////////////////////////////
            // Types
            ////////////////////////////////////////////////////////////
            typedef int AddrLength;

            ////////////////////////////////////////////////////////////
            /// \brief Create an internal sockaddr_in address
            ///
            /// \param address Target address
            /// \param port    Target port
            ///
            /// \return sockaddr_in ready to be used by socket functions
            ///
            ////////////////////////////////////////////////////////////
            static sockaddr_in createAddress(std::uint32_t address, unsigned short port);

            ////////////////////////////////////////////////////////////
            /// \brief Return the value of the invalid socket
            ///
            /// \return Special value of the invalid socket
            ///
            ////////////////////////////////////////////////////////////
            static SocketHandle invalidSocket();

            ////////////////////////////////////////////////////////////
            /// \brief Close and destroy a socket
            ///
            /// \param sock Handle of the socket to close
            ///
            ////////////////////////////////////////////////////////////
            static void close(SocketHandle sock);

            ////////////////////////////////////////////////////////////
            /// \brief Set a socket as blocking or non-blocking
            ///
            /// \param sock  Handle of the socket
            /// \param block New blocking state of the socket
            ///
            ////////////////////////////////////////////////////////////
            static void setBlocking(SocketHandle sock, bool block);

            ////////////////////////////////////////////////////////////
            /// Get the last socket error status
            ///
            /// \return Status corresponding to the last socket error
            ///
            ////////////////////////////////////////////////////////////
            static Socket::Status getErrorStatus();
        };

    }
}
#endif // SOCKET_IMPL_HPP
