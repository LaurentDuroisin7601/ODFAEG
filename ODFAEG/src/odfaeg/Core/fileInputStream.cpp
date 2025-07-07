////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2018 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "../../../include/odfaeg/Core/fileInputStream.hpp"
/*#ifdef SFML_SYSTEM_ANDROID
#include <SFML/System/Android/ResourceStream.hpp>
#endif*/


namespace odfaeg
{
    namespace core {
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
    }

} // namespace sf
