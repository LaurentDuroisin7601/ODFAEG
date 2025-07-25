#include "../../../include/odfaeg/Network/socketselector.hpp"
#include "../../../include/odfaeg/Network/socket.hpp"
#include "../../../include/odfaeg/Network/socketImpl.hpp"
#include <iostream>
namespace odfaeg {
    namespace network {
        ////////////////////////////////////////////////////////////
        struct SocketSelector::SocketSelectorImpl
        {
            fd_set allSockets;   ///< Set containing all the sockets handles
            fd_set socketsReady; ///< Set containing handles of the sockets that are ready
            int    maxSocket;    ///< Maximum socket handle
            int    socketCount;  ///< Number of socket handles
        };


        ////////////////////////////////////////////////////////////
        SocketSelector::SocketSelector() :
        m_impl(new SocketSelectorImpl)
        {
            clear();
        }


        ////////////////////////////////////////////////////////////
        SocketSelector::SocketSelector(const SocketSelector& copy) :
        m_impl(new SocketSelectorImpl(*copy.m_impl))
        {

        }


        ////////////////////////////////////////////////////////////
        SocketSelector::~SocketSelector()
        {
            delete m_impl;
        }


        ////////////////////////////////////////////////////////////
        void SocketSelector::add(Socket& socket)
        {
            SocketHandle handle = socket.getHandle();
            if (handle != SocketImpl::invalidSocket())
            {

        #if defined(ODFAEG_SYSTEM_WINDOWS)

                if (m_impl->socketCount >= FD_SETSIZE)
                {
                    std::cerr << "The socket can't be added to the selector because the "
                          << "selector is full. This is a limitation of your operating "
                          << "system's FD_SETSIZE setting.";
                    return;
                }

                if (FD_ISSET(handle, &m_impl->allSockets))
                    return;

                m_impl->socketCount++;

        #else

                if (handle >= FD_SETSIZE)
                {
                    std::cerr << "The socket can't be added to the selector because its "
                          << "ID is too high. This is a limitation of your operating "
                          << "system's FD_SETSIZE setting.";
                    return;
                }

                // SocketHandle is an int in POSIX
                m_impl->maxSocket = std::max(m_impl->maxSocket, handle);

        #endif

                FD_SET(handle, &m_impl->allSockets);
            }
        }


        ////////////////////////////////////////////////////////////
        void SocketSelector::remove(Socket& socket)
        {
            SocketHandle handle = socket.getHandle();
            if (handle != SocketImpl::invalidSocket())
            {

        #if defined(ODFAEG_SYSTEM_WINDOWS)

                if (!FD_ISSET(handle, &m_impl->allSockets))
                    return;

                m_impl->socketCount--;

        #else

                if (handle >= FD_SETSIZE)
                    return;

        #endif

                FD_CLR(handle, &m_impl->allSockets);
                FD_CLR(handle, &m_impl->socketsReady);
            }
        }


        ////////////////////////////////////////////////////////////
        void SocketSelector::clear()
        {
            FD_ZERO(&m_impl->allSockets);
            FD_ZERO(&m_impl->socketsReady);

            m_impl->maxSocket = 0;
            m_impl->socketCount = 0;
        }


        ////////////////////////////////////////////////////////////
        bool SocketSelector::wait(core::Time timeout)
        {
            // Setup the timeout
            timeval time;
            time.tv_sec  = static_cast<long>(timeout.asMicroseconds() / 1000000);
            time.tv_usec = static_cast<long>(timeout.asMicroseconds() % 1000000);

            // Initialize the set that will contain the sockets that are ready
            m_impl->socketsReady = m_impl->allSockets;

            // Wait until one of the sockets is ready for reading, or timeout is reached
            // The first parameter is ignored on Windows
            int count = select(m_impl->maxSocket + 1, &m_impl->socketsReady, NULL, NULL, timeout != core::Time::zero ? &time : NULL);

            return count > 0;
        }


        ////////////////////////////////////////////////////////////
        bool SocketSelector::isReady(Socket& socket) const
        {
            SocketHandle handle = socket.getHandle();
            if (handle != SocketImpl::invalidSocket())
            {

        #if !defined(ODFAEG_SYSTEM_WINDOWS)

                if (handle >= FD_SETSIZE)
                    return false;

        #endif

                return FD_ISSET(handle, &m_impl->socketsReady) != 0;
            }

            return false;
        }


        ////////////////////////////////////////////////////////////
        SocketSelector& SocketSelector::operator =(const SocketSelector& right)
        {
            SocketSelector temp(right);

            std::swap(m_impl, temp.m_impl);

            return *this;
        }
    }
}
