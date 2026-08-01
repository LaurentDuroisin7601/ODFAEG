#include <cstdint>
#include <cstdio>
#include <string>
#include <cstring>
namespace odfaeg{
	namespace core {
		class InputStream {
		public :
            ////////////////////////////////////////////////////////////
            /// \brief Virtual destructor
            ///
            ////////////////////////////////////////////////////////////
            virtual ~InputStream() {}

            ////////////////////////////////////////////////////////////
            /// \brief Read data from the stream
            ///
            /// After reading, the stream's reading position must be
            /// advanced by the amount of bytes read.
            ///
            /// \param data Buffer where to copy the read data
            /// \param size Desired number of bytes to read
            ///
            /// \return The number of bytes actually read, or -1 on error
            ///
            ////////////////////////////////////////////////////////////
            virtual std::int64_t read(void* data, std::int64_t size) = 0;

            ////////////////////////////////////////////////////////////
            /// \brief Change the current reading position
            ///
            /// \param position The position to seek to, from the beginning
            ///
            /// \return The position actually sought to, or -1 on error
            ///
            ////////////////////////////////////////////////////////////
            virtual std::int64_t seek(std::int64_t position) = 0;

            ////////////////////////////////////////////////////////////
            /// \brief Get the current reading position in the stream
            ///
            /// \return The current position, or -1 on error.
            ///
            ////////////////////////////////////////////////////////////
            virtual std::int64_t tell() = 0;

            ////////////////////////////////////////////////////////////
            /// \brief Return the size of the stream
            ///
            /// \return The total number of bytes available in the stream, or -1 on error
            ///
            ////////////////////////////////////////////////////////////
            virtual std::int64_t getSize() = 0;
		};
        ////////////////////////////////////////////////////////////
        /// \brief Implementation of input stream based on a file
        ///
        ////////////////////////////////////////////////////////////
        class  FileInputStream : public InputStream {
        public:
            ////////////////////////////////////////////////////////////
            FileInputStream();


            ////////////////////////////////////////////////////////////
            ~FileInputStream();


            ////////////////////////////////////////////////////////////
            bool open(const std::string& filename);


            ////////////////////////////////////////////////////////////
            std::int64_t read(void* data, std::int64_t size);


            ////////////////////////////////////////////////////////////
            std::int64_t seek(std::int64_t position);


            ////////////////////////////////////////////////////////////
            std::int64_t tell();


            ////////////////////////////////////////////////////////////
            std::int64_t getSize();

        private:

            ////////////////////////////////////////////////////////////
            // Member data
            ////////////////////////////////////////////////////////////
        /*#ifdef SFML_SYSTEM_ANDROID
            priv::ResourceStream* m_file;
        #else*/
            std::FILE* m_file; ///< stdio file stream
            //#endif
        };
        class MemoryInputStream : public InputStream {
            ////////////////////////////////////////////////////////////
            MemoryInputStream();


            ////////////////////////////////////////////////////////////
            void open(const void* data, std::size_t sizeInBytes);

            ////////////////////////////////////////////////////////////
            std::int64_t read(void* data, std::int64_t size);


            ////////////////////////////////////////////////////////////
            std::int64_t seek(std::int64_t position);


            ////////////////////////////////////////////////////////////
            std::int64_t tell();


            ////////////////////////////////////////////////////////////
            std::int64_t getSize();
        private:

            ////////////////////////////////////////////////////////////
            // Member data
            ////////////////////////////////////////////////////////////
            const char* m_data;   ///< Pointer to the data in memory
            std::int64_t       m_size;   ///< Total size of the data
            std::int64_t       m_offset; ///< Current reading position
        };
	}
}
#include "inputStream.inl"