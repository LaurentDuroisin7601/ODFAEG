#ifndef ODFAEG_IP_ADDRESS
#define ODFAEG_IP_ADDRESS
#include "export.hpp"
#include "../Core/time.h"
#include <string>
namespace odfaeg {
    namespace network {
        ////////////////////////////////////////////////////////////
        /// \brief Encapsulate an IPv4 network address
        ///
        ////////////////////////////////////////////////////////////
        class ODFAEG_NETWORK_API IpAddress
        {
        public:

            ////////////////////////////////////////////////////////////
            /// \brief Default constructor
            ///
            /// This constructor creates an empty (invalid) address
            ///
            ////////////////////////////////////////////////////////////
            IpAddress();

            ////////////////////////////////////////////////////////////
            /// \brief Construct the address from a string
            ///
            /// Here \a address can be either a decimal address
            /// (ex: "192.168.1.56") or a network name (ex: "localhost").
            ///
            /// \param address IP address or network name
            ///
            ////////////////////////////////////////////////////////////
            IpAddress(const std::string& address);

            ////////////////////////////////////////////////////////////
            /// \brief Construct the address from a string
            ///
            /// Here \a address can be either a decimal address
            /// (ex: "192.168.1.56") or a network name (ex: "localhost").
            /// This is equivalent to the constructor taking a std::string
            /// parameter, it is defined for convenience so that the
            /// implicit conversions from literal strings to IpAddress work.
            ///
            /// \param address IP address or network name
            ///
            ////////////////////////////////////////////////////////////
            IpAddress(const char* address);

            ////////////////////////////////////////////////////////////
            /// \brief Construct the address from 4 bytes
            ///
            /// Calling IpAddress(a, b, c, d) is equivalent to calling
            /// IpAddress("a.b.c.d"), but safer as it doesn't have to
            /// parse a string to get the address components.
            ///
            /// \param byte0 First byte of the address
            /// \param byte1 Second byte of the address
            /// \param byte2 Third byte of the address
            /// \param byte3 Fourth byte of the address
            ///
            ////////////////////////////////////////////////////////////
            IpAddress(std::uint8_t byte0, std::uint8_t byte1, std::uint8_t byte2, std::uint8_t byte3);

            ////////////////////////////////////////////////////////////
            /// \brief Construct the address from a 32-bits integer
            ///
            /// This constructor uses the internal representation of
            /// the address directly. It should be used for optimization
            /// purposes, and only if you got that representation from
            /// IpAddress::toInteger().
            ///
            /// \param address 4 bytes of the address packed into a 32-bits integer
            ///
            /// \see toInteger
            ///
            ////////////////////////////////////////////////////////////
            explicit IpAddress(std::uint32_t address);

            ////////////////////////////////////////////////////////////
            /// \brief Get a string representation of the address
            ///
            /// The returned string is the decimal representation of the
            /// IP address (like "192.168.1.56"), even if it was constructed
            /// from a host name.
            ///
            /// \return String representation of the address
            ///
            /// \see toInteger
            ///
            ////////////////////////////////////////////////////////////
            std::string toString() const;

            ////////////////////////////////////////////////////////////
            /// \brief Get an integer representation of the address
            ///
            /// The returned number is the internal representation of the
            /// address, and should be used for optimization purposes only
            /// (like sending the address through a socket).
            /// The integer produced by this function can then be converted
            /// back to a sf::IpAddress with the proper constructor.
            ///
            /// \return 32-bits unsigned integer representation of the address
            ///
            /// \see toString
            ///
            ////////////////////////////////////////////////////////////
            std::uint32_t toInteger() const;

            ////////////////////////////////////////////////////////////
            /// \brief Get the computer's local address
            ///
            /// The local address is the address of the computer from the
            /// LAN point of view, i.e. something like 192.168.1.56. It is
            /// meaningful only for communications over the local network.
            /// Unlike getPublicAddress, this function is fast and may be
            /// used safely anywhere.
            ///
            /// \return Local IP address of the computer
            ///
            /// \see getPublicAddress
            ///
            ////////////////////////////////////////////////////////////
            static IpAddress getLocalAddress();

            ////////////////////////////////////////////////////////////
            /// \brief Get the computer's public address
            ///
            /// The public address is the address of the computer from the
            /// internet point of view, i.e. something like 89.54.1.169.
            /// It is necessary for communications over the world wide web.
            /// The only way to get a public address is to ask it to a
            /// distant website; as a consequence, this function depends on
            /// both your network connection and the server, and may be
            /// very slow. You should use it as few as possible. Because
            /// this function depends on the network connection and on a distant
            /// server, you may use a time limit if you don't want your program
            /// to be possibly stuck waiting in case there is a problem; this
            /// limit is deactivated by default.
            ///
            /// \param timeout Maximum time to wait
            ///
            /// \return Public IP address of the computer
            ///
            /// \see getLocalAddress
            ///
            ////////////////////////////////////////////////////////////
            static IpAddress getPublicAddress(core::Time timeout = core::Time::zero);

            ////////////////////////////////////////////////////////////
            // Static member data
            ////////////////////////////////////////////////////////////
            static const IpAddress None;      ///< Value representing an empty/invalid address
            static const IpAddress Any;       ///< Value representing any address (0.0.0.0)
            static const IpAddress LocalHost; ///< The "localhost" address (for connecting a computer to itself locally)
            static const IpAddress Broadcast; ///< The "broadcast" address (for sending UDP messages to everyone on a local network)

        private:

            friend ODFAEG_NETWORK_API bool operator <(const IpAddress& left, const IpAddress& right);

            ////////////////////////////////////////////////////////////
            /// \brief Resolve the given address string
            ///
            /// \param address Address string
            ///
            ////////////////////////////////////////////////////////////
            void resolve(const std::string& address);

            ////////////////////////////////////////////////////////////
            // Member data
            ////////////////////////////////////////////////////////////
            std::uint32_t m_address; ///< Address stored as an unsigned 32 bits integer
            bool   m_valid;   ///< Is the address valid?
        };

        ////////////////////////////////////////////////////////////
        /// \brief Overload of == operator to compare two IP addresses
        ///
        /// \param left  Left operand (a IP address)
        /// \param right Right operand (a IP address)
        ///
        /// \return True if both addresses are equal
        ///
        ////////////////////////////////////////////////////////////
        ODFAEG_NETWORK_API bool operator ==(const IpAddress& left, const IpAddress& right);

        ////////////////////////////////////////////////////////////
        /// \brief Overload of != operator to compare two IP addresses
        ///
        /// \param left  Left operand (a IP address)
        /// \param right Right operand (a IP address)
        ///
        /// \return True if both addresses are different
        ///
        ////////////////////////////////////////////////////////////
        ODFAEG_NETWORK_API bool operator !=(const IpAddress& left, const IpAddress& right);

        ////////////////////////////////////////////////////////////
        /// \brief Overload of < operator to compare two IP addresses
        ///
        /// \param left  Left operand (a IP address)
        /// \param right Right operand (a IP address)
        ///
        /// \return True if \a left is lesser than \a right
        ///
        ////////////////////////////////////////////////////////////
        ODFAEG_NETWORK_API bool operator <(const IpAddress& left, const IpAddress& right);

        ////////////////////////////////////////////////////////////
        /// \brief Overload of > operator to compare two IP addresses
        ///
        /// \param left  Left operand (a IP address)
        /// \param right Right operand (a IP address)
        ///
        /// \return True if \a left is greater than \a right
        ///
        ////////////////////////////////////////////////////////////
        ODFAEG_NETWORK_API bool operator >(const IpAddress& left, const IpAddress& right);

        ////////////////////////////////////////////////////////////
        /// \brief Overload of <= operator to compare two IP addresses
        ///
        /// \param left  Left operand (a IP address)
        /// \param right Right operand (a IP address)
        ///
        /// \return True if \a left is lesser or equal than \a right
        ///
        ////////////////////////////////////////////////////////////
        ODFAEG_NETWORK_API bool operator <=(const IpAddress& left, const IpAddress& right);

        ////////////////////////////////////////////////////////////
        /// \brief Overload of >= operator to compare two IP addresses
        ///
        /// \param left  Left operand (a IP address)
        /// \param right Right operand (a IP address)
        ///
        /// \return True if \a left is greater or equal than \a right
        ///
        ////////////////////////////////////////////////////////////
        ODFAEG_NETWORK_API bool operator >=(const IpAddress& left, const IpAddress& right);

        ////////////////////////////////////////////////////////////
        /// \brief Overload of >> operator to extract an IP address from an input stream
        ///
        /// \param stream  Input stream
        /// \param address IP address to extract
        ///
        /// \return Reference to the input stream
        ///
        ////////////////////////////////////////////////////////////
        ODFAEG_NETWORK_API std::istream& operator >>(std::istream& stream, IpAddress& address);

        ////////////////////////////////////////////////////////////
        /// \brief Overload of << operator to print an IP address to an output stream
        ///
        /// \param stream  Output stream
        /// \param address IP address to print
        ///
        /// \return Reference to the output stream
        ///
        ////////////////////////////////////////////////////////////
        ODFAEG_NETWORK_API std::ostream& operator <<(std::ostream& stream, const IpAddress& address);
    }
}
#endif // ODFAEG_IP_ADDRESS
