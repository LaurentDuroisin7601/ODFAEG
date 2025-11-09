#include "../../../include/odfaeg/Network/updsocket.hpp"
#include "../../../include/odfaeg/Network/ipaddress.hpp"
#include "../../../include/odfaeg/Network/packet.hpp"
#include "../../../include/odfaeg/Network/socketImpl.hpp"
#include <algorithm>
#include <iostream>
namespace odfaeg {
    namespace network {
        ////////////////////////////////////////////////////////////
        UdpSocket::UdpSocket() :
        Socket  (Udp),
        m_buffer(MaxDatagramSize)
        {

        }


        ////////////////////////////////////////////////////////////
        unsigned short UdpSocket::getLocalPort() const
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
        Socket::Status UdpSocket::bind(unsigned short port, const IpAddress& address)
        {
            // Close the socket if it is already bound
            close();

            // Create the internal socket if it doesn't exist
            create();

            // Check if the address is valid
            if ((address == IpAddress::None) || (address == IpAddress::Broadcast))
                return Error;

            // Bind the socket
            sockaddr_in addr = SocketImpl::createAddress(address.toInteger(), port);
            if (::bind(getHandle(), reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1)
            {
                std::cerr << "Failed to bind socket to port " << port << std::endl;
                return Error;
            }

            return Done;
        }


        ////////////////////////////////////////////////////////////
        void UdpSocket::unbind()
        {
            // Simply close the socket
            close();
        }


        ////////////////////////////////////////////////////////////
        Socket::Status UdpSocket::send(const void* data, std::size_t size, const IpAddress& remoteAddress, unsigned short remotePort)
        {
            // Create the internal socket if it doesn't exist
            create();

            // Make sure that all the data will fit in one datagram
            if (size > MaxDatagramSize)
            {
                std::cerr << "Cannot send data over the network "
                      << "(the number of bytes to send is greater than sf::UdpSocket::MaxDatagramSize)" << std::endl;
                return Error;
            }

            // Build the target address
            sockaddr_in address = SocketImpl::createAddress(remoteAddress.toInteger(), remotePort);

            // Send the data (unlike TCP, all the data is always sent in one call)
            int sent = sendto(getHandle(), static_cast<const char*>(data), static_cast<int>(size), 0, reinterpret_cast<sockaddr*>(&address), sizeof(address));

            // Check for errors
            if (sent < 0)
                return SocketImpl::getErrorStatus();

            return Done;
        }


        ////////////////////////////////////////////////////////////
        Socket::Status UdpSocket::receive(void* data, std::size_t size, std::size_t& received, IpAddress& remoteAddress, unsigned short& remotePort)
        {
            // First clear the variables to fill
            received      = 0;
            remoteAddress = IpAddress();
            remotePort    = 0;

            // Check the destination buffer
            if (!data)
            {
                std::cerr << "Cannot receive data from the network (the destination buffer is invalid)" << std::endl;
                return Error;
            }

            // Data that will be filled with the other computer's address
            sockaddr_in address = SocketImpl::createAddress(INADDR_ANY, 0);

            // Receive a chunk of bytes
            SocketImpl::AddrLength addressSize = sizeof(address);
            int sizeReceived = recvfrom(getHandle(), static_cast<char*>(data), static_cast<int>(size), 0, reinterpret_cast<sockaddr*>(&address), &addressSize);

            // Check for errors
            if (sizeReceived < 0)
                return SocketImpl::getErrorStatus();

            // Fill the sender informations
            received      = static_cast<std::size_t>(sizeReceived);
            remoteAddress = IpAddress(ntohl(address.sin_addr.s_addr));
            remotePort    = ntohs(address.sin_port);

            return Done;
        }


        ////////////////////////////////////////////////////////////
        Socket::Status UdpSocket::send(Packet& packet, const IpAddress& remoteAddress, unsigned short remotePort)
        {
            // UDP is a datagram-oriented protocol (as opposed to TCP which is a stream protocol).
            // Sending one datagram is almost safe: it may be lost but if it's received, then its data
            // is guaranteed to be ok. However, splitting a packet into multiple datagrams would be highly
            // unreliable, since datagrams may be reordered, dropped or mixed between different sources.
            // That's why ODFAEG imposes a limit on packet size so that they can be sent in a single datagram.
            // This also removes the overhead associated to packets -- there's no size to send in addition
            // to the packet's data.

            // Get the data to send from the packet
            std::size_t size = 0;
            const void* data = packet.onSend(size);

            // Send it
            return send(data, size, remoteAddress, remotePort);
        }


        ////////////////////////////////////////////////////////////
        Socket::Status UdpSocket::receive(Packet& packet, IpAddress& remoteAddress, unsigned short& remotePort)
        {
            // See the detailed comment in send(Packet) above.

            // Receive the datagram
            std::size_t received = 0;
            Status status = receive(&m_buffer[0], m_buffer.size(), received, remoteAddress, remotePort);

            // If we received valid data, we can copy it to the user packet
            packet.clear();
            if ((status == Done) && (received > 0))
                packet.onReceive(&m_buffer[0], received);

            return status;
        }
    }
}
