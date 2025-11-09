#include "../../../include/odfaeg/Network/tcplistener.hpp"
#include "../../../include/odfaeg/Network/tcplistener.hpp"
#include "../../../include/odfaeg/Network/tcpsocket.hpp"
#include "../../../include/odfaeg/Network/socketImpl.hpp"
#include <iostream>
namespace odfaeg {
    namespace network {
        ////////////////////////////////////////////////////////////
        TcpListener::TcpListener() :
        Socket(Tcp)
        {

        }


        ////////////////////////////////////////////////////////////
        unsigned short TcpListener::getLocalPort() const
        {
            if (getHandle() != SocketImpl::invalidSocket())
            {
                // Retrieve informations about the local end of the socket
                sockaddr_in address;
                SocketImpl::AddrLength size = sizeof(address);
                if (getsockname(getHandle(), reinterpret_cast<sockaddr*>(&address), &size) != -1)
                {
                    return ntohs(address.sin_port);
                }
            }

            // We failed to retrieve the port
            return 0;
        }


        ////////////////////////////////////////////////////////////
        Socket::Status TcpListener::listen(unsigned short port, const IpAddress& address)
        {
            // Close the socket if it is already bound
            close();

            // Create the internal socket if it doesn't exist
            create();

            // Check if the address is valid
            if ((address == IpAddress::None) || (address == IpAddress::Broadcast))
                return Error;

            // Bind the socket to the specified port
            sockaddr_in addr = SocketImpl::createAddress(address.toInteger(), port);
            if (bind(getHandle(), reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1)
            {
                // Not likely to happen, but...
                std::cerr << "Failed to bind listener socket to port " << port << std::endl;
                return Error;
            }

            // Listen to the bound port
            if (::listen(getHandle(), SOMAXCONN) == -1)
            {
                // Oops, socket is deaf
                std::cerr << "Failed to listen to port " << port << std::endl;
                return Error;
            }

            return Done;
        }


        ////////////////////////////////////////////////////////////
        void TcpListener::close()
        {
            // Simply close the socket
            Socket::close();
        }


        ////////////////////////////////////////////////////////////
        Socket::Status TcpListener::accept(TcpSocket& socket)
        {
            // Make sure that we're listening
            if (getHandle() == SocketImpl::invalidSocket()) {
                std::cerr << "Failed to accept a new connection, the socket is not listening" << std::endl;
                return Error;
            }

            // Accept a new connection
            sockaddr_in address;
            SocketImpl::AddrLength length = sizeof(address);
            SocketHandle remote = ::accept(getHandle(), reinterpret_cast<sockaddr*>(&address), &length);

            // Check for errors
            if (remote == SocketImpl::invalidSocket())
                return SocketImpl::getErrorStatus();

            // Initialize the new connected socket
            socket.close();
            socket.create(remote);

            return Done;
        }
    }
}
