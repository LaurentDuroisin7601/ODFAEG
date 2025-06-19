#ifndef ODFAEG_PACKET_HPP
#define ODFAEG_PACKET_HPP
#include "../Core/string.hpp"
#include <string>
#include <vector>
#include "export.hpp"
namespace odfaeg {
    namespace network {
        ////////////////////////////////////////////////////////////
        /// \brief Utility class to build blocks of data to transfer
        ///        over the network
        ///
        ////////////////////////////////////////////////////////////
        class ODFAEG_NETWORK_API Packet
        {
            // A bool-like type that cannot be converted to integer or pointer types
            typedef bool (Packet::*BoolType)(std::size_t);

        public:

            ////////////////////////////////////////////////////////////
            /// \brief Default constructor
            ///
            /// Creates an empty packet.
            ///
            ////////////////////////////////////////////////////////////
            Packet();

            ////////////////////////////////////////////////////////////
            /// \brief Virtual destructor
            ///
            ////////////////////////////////////////////////////////////
            virtual ~Packet();

            ////////////////////////////////////////////////////////////
            /// \brief Append data to the end of the packet
            ///
            /// \param data        Pointer to the sequence of bytes to append
            /// \param sizeInBytes Number of bytes to append
            ///
            /// \see clear
            ///
            ////////////////////////////////////////////////////////////
            void append(const void* data, std::size_t sizeInBytes);

            ////////////////////////////////////////////////////////////
            /// \brief Clear the packet
            ///
            /// After calling Clear, the packet is empty.
            ///
            /// \see append
            ///
            ////////////////////////////////////////////////////////////
            void clear();

            ////////////////////////////////////////////////////////////
            /// \brief Get a pointer to the data contained in the packet
            ///
            /// Warning: the returned pointer may become invalid after
            /// you append data to the packet, therefore it should never
            /// be stored.
            /// The return pointer is NULL if the packet is empty.
            ///
            /// \return Pointer to the data
            ///
            /// \see getDataSize
            ///
            ////////////////////////////////////////////////////////////
            const void* getData() const;

            ////////////////////////////////////////////////////////////
            /// \brief Get the size of the data contained in the packet
            ///
            /// This function returns the number of bytes pointed to by
            /// what getData returns.
            ///
            /// \return Data size, in bytes
            ///
            /// \see getData
            ///
            ////////////////////////////////////////////////////////////
            std::size_t getDataSize() const;

            ////////////////////////////////////////////////////////////
            /// \brief Tell if the reading position has reached the
            ///        end of the packet
            ///
            /// This function is useful to know if there is some data
            /// left to be read, without actually reading it.
            ///
            /// \return True if all data was read, false otherwise
            ///
            /// \see operator bool
            ///
            ////////////////////////////////////////////////////////////
            bool endOfPacket() const;

        public:

            ////////////////////////////////////////////////////////////
            /// \brief Test the validity of the packet, for reading
            ///
            /// This operator allows to test the packet as a boolean
            /// variable, to check if a reading operation was successful.
            ///
            /// A packet will be in an invalid state if it has no more
            /// data to read.
            ///
            /// This behavior is the same as standard C++ streams.
            ///
            /// Usage example:
            /// \code
            /// float x;
            /// packet >> x;
            /// if (packet)
            /// {
            ///    // ok, x was extracted successfully
            /// }
            ///
            /// // -- or --
            ///
            /// float x;
            /// if (packet >> x)
            /// {
            ///    // ok, x was extracted successfully
            /// }
            /// \endcode
            ///
            /// Don't focus on the return type, it's equivalent to bool but
            /// it disallows unwanted implicit conversions to integer or
            /// pointer types.
            ///
            /// \return True if last data extraction from packet was successful
            ///
            /// \see endOfPacket
            ///
            ////////////////////////////////////////////////////////////
            operator BoolType() const;

            ////////////////////////////////////////////////////////////
            /// Overload of operator >> to read data from the packet
            ///
            ////////////////////////////////////////////////////////////
            Packet& operator >>(bool&         data);

            ////////////////////////////////////////////////////////////
            /// \overload
            ////////////////////////////////////////////////////////////
            Packet& operator >>(std::int8_t&         data);

            ////////////////////////////////////////////////////////////
            /// \overload
            ////////////////////////////////////////////////////////////
            Packet& operator >>(std::uint8_t&        data);

            ////////////////////////////////////////////////////////////
            /// \overload
            ////////////////////////////////////////////////////////////
            Packet& operator >>(std::int16_t&        data);

            ////////////////////////////////////////////////////////////
            /// \overload
            ////////////////////////////////////////////////////////////
            Packet& operator >>(std::uint16_t&       data);

            ////////////////////////////////////////////////////////////
            /// \overload
            ////////////////////////////////////////////////////////////
            Packet& operator >>(std::int32_t&        data);

            ////////////////////////////////////////////////////////////
            /// \overload
            ////////////////////////////////////////////////////////////
            Packet& operator >>(std::uint32_t&       data);

            ////////////////////////////////////////////////////////////
            /// \overload
            ////////////////////////////////////////////////////////////
            Packet& operator >>(std::int64_t&        data);

            ////////////////////////////////////////////////////////////
            /// \overload
            ////////////////////////////////////////////////////////////
            Packet& operator >>(std::uint64_t&       data);

            ////////////////////////////////////////////////////////////
            /// \overload
            ////////////////////////////////////////////////////////////
            Packet& operator >>(float&        data);

            ////////////////////////////////////////////////////////////
            /// \overload
            ////////////////////////////////////////////////////////////
            Packet& operator >>(double&       data);

            ////////////////////////////////////////////////////////////
            /// \overload
            ////////////////////////////////////////////////////////////
            Packet& operator >>(char*         data);

            ////////////////////////////////////////////////////////////
            /// \overload
            ////////////////////////////////////////////////////////////
            Packet& operator >>(std::string&  data);

            ////////////////////////////////////////////////////////////
            /// \overload
            ////////////////////////////////////////////////////////////
            Packet& operator >>(wchar_t*      data);

            ////////////////////////////////////////////////////////////
            /// \overload
            ////////////////////////////////////////////////////////////
            Packet& operator >>(std::wstring& data);

            ////////////////////////////////////////////////////////////
            /// \overload
            ////////////////////////////////////////////////////////////
            Packet& operator >>(core::String&       data);

            ////////////////////////////////////////////////////////////
            /// Overload of operator << to write data into the packet
            ///
            ////////////////////////////////////////////////////////////
            Packet& operator <<(bool                data);

            ////////////////////////////////////////////////////////////
            /// \overload
            ////////////////////////////////////////////////////////////
            Packet& operator <<(std::int8_t                data);

            ////////////////////////////////////////////////////////////
            /// \overload
            ////////////////////////////////////////////////////////////
            Packet& operator <<(std::uint8_t               data);

            ////////////////////////////////////////////////////////////
            /// \overload
            ////////////////////////////////////////////////////////////
            Packet& operator <<(std::int16_t               data);

            ////////////////////////////////////////////////////////////
            /// \overload
            ////////////////////////////////////////////////////////////
            Packet& operator <<(std::uint16_t             data);

            ////////////////////////////////////////////////////////////
            /// \overload
            ////////////////////////////////////////////////////////////
            Packet& operator <<(std::int32_t               data);

            ////////////////////////////////////////////////////////////
            /// \overload
            ////////////////////////////////////////////////////////////
            Packet& operator <<(std::uint32_t              data);

            ////////////////////////////////////////////////////////////
            /// \overload
            ////////////////////////////////////////////////////////////
            Packet& operator <<(std::int64_t             data);

            ////////////////////////////////////////////////////////////
            /// \overload
            ////////////////////////////////////////////////////////////
            Packet& operator <<(std::uint64_t              data);

            ////////////////////////////////////////////////////////////
            /// \overload
            ////////////////////////////////////////////////////////////
            Packet& operator <<(float               data);

            ////////////////////////////////////////////////////////////
            /// \overload
            ////////////////////////////////////////////////////////////
            Packet& operator <<(double              data);

            ////////////////////////////////////////////////////////////
            /// \overload
            ////////////////////////////////////////////////////////////
            Packet& operator <<(const char*         data);

            ////////////////////////////////////////////////////////////
            /// \overload
            ////////////////////////////////////////////////////////////
            Packet& operator <<(const std::string&  data);

            ////////////////////////////////////////////////////////////
            /// \overload
            ////////////////////////////////////////////////////////////
            Packet& operator <<(const wchar_t*      data);

            ////////////////////////////////////////////////////////////
            /// \overload
            ////////////////////////////////////////////////////////////
            Packet& operator <<(const std::wstring& data);

            ////////////////////////////////////////////////////////////
            /// \overload
            ////////////////////////////////////////////////////////////
            Packet& operator <<(const core::String&       data);

        protected:

            friend class TcpSocket;
            friend class UdpSocket;

            ////////////////////////////////////////////////////////////
            /// \brief Called before the packet is sent over the network
            ///
            /// This function can be defined by derived classes to
            /// transform the data before it is sent; this can be
            /// used for compression, encryption, etc.
            /// The function must return a pointer to the modified data,
            /// as well as the number of bytes pointed.
            /// The default implementation provides the packet's data
            /// without transforming it.
            ///
            /// \param size Variable to fill with the size of data to send
            ///
            /// \return Pointer to the array of bytes to send
            ///
            /// \see onReceive
            ///
            ////////////////////////////////////////////////////////////
            virtual const void* onSend(std::size_t& size);

            ////////////////////////////////////////////////////////////
            /// \brief Called after the packet is received over the network
            ///
            /// This function can be defined by derived classes to
            /// transform the data after it is received; this can be
            /// used for decompression, decryption, etc.
            /// The function receives a pointer to the received data,
            /// and must fill the packet with the transformed bytes.
            /// The default implementation fills the packet directly
            /// without transforming the data.
            ///
            /// \param data Pointer to the received bytes
            /// \param size Number of bytes
            ///
            /// \see onSend
            ///
            ////////////////////////////////////////////////////////////
            virtual void onReceive(const void* data, std::size_t size);

        private:

            ////////////////////////////////////////////////////////////
            /// Disallow comparisons between packets
            ///
            ////////////////////////////////////////////////////////////
            bool operator ==(const Packet& right) const;
            bool operator !=(const Packet& right) const;

            ////////////////////////////////////////////////////////////
            /// \brief Check if the packet can extract a given number of bytes
            ///
            /// This function updates accordingly the state of the packet.
            ///
            /// \param size Size to check
            ///
            /// \return True if \a size bytes can be read from the packet
            ///
            ////////////////////////////////////////////////////////////
            bool checkSize(std::size_t size);

            ////////////////////////////////////////////////////////////
            // Member data
            ////////////////////////////////////////////////////////////
            std::vector<char> m_data;    ///< Data stored in the packet
            std::size_t       m_readPos; ///< Current reading position in the packet
            std::size_t       m_sendPos; ///< Current send position in the packet (for handling partial sends)
            bool              m_isValid; ///< Reading state of the packet
        };
    }
}
#endif // ODFAEG_PACKET_HPP
