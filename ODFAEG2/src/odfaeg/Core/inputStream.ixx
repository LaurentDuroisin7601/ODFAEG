module;
#include <cstdint>
#include <cstdio>
#include <string>
#include <cstring>
export module odfaeg.core.inputStream;
export namespace odfaeg{
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
            FileInputStream()
                : m_file(NULL)
            {

            }


            ////////////////////////////////////////////////////////////
            ~FileInputStream()
            {
                /*#ifdef SFML_SYSTEM_ANDROID
                    if (m_file)
                        delete m_file;
                #else*/
                if (m_file)
                    std::fclose(m_file);
                //#endif
            }


            ////////////////////////////////////////////////////////////
            bool open(const std::string& filename)
            {
                /*#ifdef SFML_SYSTEM_ANDROID
                    if (m_file)
                        delete m_file;
                    m_file = new priv::ResourceStream(filename);
                    return m_file->tell() != -1;
                #else*/
                if (m_file)
                    std::fclose(m_file);

                m_file = std::fopen(filename.c_str(), "rb");

                return m_file != NULL;
                //#endif
            }


            ////////////////////////////////////////////////////////////
            std::int64_t read(void* data, std::int64_t size)
            {
                /*#ifdef SFML_SYSTEM_ANDROID
                    return m_file->read(data, size);
                #else*/
                if (m_file)
                    return std::fread(data, 1, static_cast<std::size_t>(size), m_file);
                else
                    return -1;
                //#endif
            }


            ////////////////////////////////////////////////////////////
            std::int64_t seek(std::int64_t position)
            {
                /*#ifdef SFML_SYSTEM_ANDROID
                    return m_file->seek(position);
                #else*/
                if (m_file)
                {
                    if (std::fseek(m_file, static_cast<long>(position), SEEK_SET))
                        return -1;

                    return tell();
                }
                else
                {
                    return -1;
                }
                //#endif
            }


            ////////////////////////////////////////////////////////////
            std::int64_t tell()
            {
                /*#ifdef SFML_SYSTEM_ANDROID
                    return m_file->tell();
                #else*/
                if (m_file)
                    return std::ftell(m_file);
                else
                    return -1;
                //#endif
            }


            ////////////////////////////////////////////////////////////
            std::int64_t getSize()
            {
                /*#ifdef SFML_SYSTEM_ANDROID
                    return m_file->getSize();
                #else*/
                if (m_file)
                {
                    std::int64_t position = tell();
                    std::fseek(m_file, 0, SEEK_END);
                    std::int64_t size = tell();
                    seek(position);
                    return size;
                }
                else
                {
                    return -1;
                }
                //#endif
            }

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
            MemoryInputStream() :
                m_data(NULL),
                m_size(0),
                m_offset(0)
            {
            }


            ////////////////////////////////////////////////////////////
            void open(const void* data, std::size_t sizeInBytes)
            {
                m_data = static_cast<const char*>(data);
                m_size = sizeInBytes;
                m_offset = 0;
            }


            ////////////////////////////////////////////////////////////
            std::int64_t read(void* data, std::int64_t size)
            {
                if (!m_data)
                    return -1;

                std::int64_t endPosition = m_offset + size;
                std::int64_t count = endPosition <= m_size ? size : m_size - m_offset;

                if (count > 0)
                {
                    std::memcpy(data, m_data + m_offset, static_cast<std::size_t>(count));
                    m_offset += count;
                }

                return count;
            }


            ////////////////////////////////////////////////////////////
            std::int64_t seek(std::int64_t position)
            {
                if (!m_data)
                    return -1;

                m_offset = position < m_size ? position : m_size;
                return m_offset;
            }


            ////////////////////////////////////////////////////////////
            std::int64_t tell()
            {
                if (!m_data)
                    return -1;

                return m_offset;
            }


            ////////////////////////////////////////////////////////////
            std::int64_t getSize()
            {
                if (!m_data)
                    return -1;

                return m_size;
            }
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