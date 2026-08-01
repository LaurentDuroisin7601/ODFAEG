namespace odfaeg {
    namespace core {
        ////////////////////////////////////////////////////////////
        /// \brief Implementation of input stream based on a file
        ///
        ////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////
        FileInputStream::FileInputStream()
            : m_file(NULL)
        {

        }


        ////////////////////////////////////////////////////////////
        FileInputStream::~FileInputStream()
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
        bool FileInputStream::open(const std::string& filename)
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
        std::int64_t FileInputStream::read(void* data, std::int64_t size)
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
        std::int64_t FileInputStream::seek(std::int64_t position)
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
        std::int64_t FileInputStream::tell()
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
        std::int64_t FileInputStream::getSize()
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
        ////////////////////////////////////////////////////////////
        MemoryInputStream::MemoryInputStream() :
            m_data(NULL),
            m_size(0),
            m_offset(0)
        {
        }


        ////////////////////////////////////////////////////////////
        void MemoryInputStream::open(const void* data, std::size_t sizeInBytes)
        {
            m_data = static_cast<const char*>(data);
            m_size = sizeInBytes;
            m_offset = 0;
        }


        ////////////////////////////////////////////////////////////
        std::int64_t MemoryInputStream::read(void* data, std::int64_t size)
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
        std::int64_t MemoryInputStream::seek(std::int64_t position)
        {
            if (!m_data)
                return -1;

            m_offset = position < m_size ? position : m_size;
            return m_offset;
        }


        ////////////////////////////////////////////////////////////
        std::int64_t MemoryInputStream::tell()
        {
            if (!m_data)
                return -1;

            return m_offset;
        }


        ////////////////////////////////////////////////////////////
        std::int64_t MemoryInputStream::getSize()
        {
            if (!m_data)
                return -1;

            return m_size;
        }
    }
}