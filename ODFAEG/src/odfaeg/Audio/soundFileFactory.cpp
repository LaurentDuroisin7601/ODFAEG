////////////////////////////////////////////////////////////
//
// ODFAEG - Simple and Fast Multimedia Library
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
#include "../../../include/odfaeg/Audio/soundFileFactory.hpp"
#include "../../../include/odfaeg/Core/fileInputStream.hpp"
#include "../../../include/odfaeg/Core/memoryInputStream.hpp"
#include "soundFileReaderFlac.hpp"
#include "soundFileReaderOgg.hpp"
#include "soundFileReaderWav.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
namespace
{
    // Register all the built-in readers and writers if not already done
    void ensureDefaultReadersWritersRegistered()
    {
        static bool registered = false;
        if (!registered)
        {
            odfaeg::audio::SoundFileFactory::registerReader<odfaeg::audio::priv::SoundFileReaderFlac>();
            odfaeg::audio::SoundFileFactory::registerReader<odfaeg::audio::priv::SoundFileReaderOgg>();
            odfaeg::audio::SoundFileFactory::registerReader<odfaeg::audio::priv::SoundFileReaderWav>();
            registered = true;
        }
    }
}

namespace odfaeg {

    namespace audio {
    SoundFileFactory::ReaderFactoryArray SoundFileFactory::s_readers;



        ////////////////////////////////////////////////////////////
        SoundFileReader* SoundFileFactory::createReaderFromFilename(const std::string& filename)
        {
            // Register the built-in readers/writers on first call
            ensureDefaultReadersWritersRegistered();

            // Wrap the input file into a file stream
            core::FileInputStream stream;
            if (!stream.open(filename)) {
                std::cerr << "Failed to open sound file \"" << filename << "\" (couldn't open stream)" << std::endl;
                return NULL;
            }

            // Test the filename in all the registered factories
            for (ReaderFactoryArray::const_iterator it = s_readers.begin(); it != s_readers.end(); ++it)
            {
                stream.seek(0);
                if (it->check(stream))
                    return it->create();
            }

            // No suitable reader found
            std::cerr << "Failed to open sound file \"" << filename << "\" (format not supported)" << std::endl;
            return NULL;
        }


        ////////////////////////////////////////////////////////////
        SoundFileReader* SoundFileFactory::createReaderFromMemory(const void* data, std::size_t sizeInBytes)
        {
            // Register the built-in readers/writers on first call
            ensureDefaultReadersWritersRegistered();

            // Wrap the memory file into a file stream
            core::MemoryInputStream stream;
            stream.open(data, sizeInBytes);

            // Test the stream for all the registered factories
            for (ReaderFactoryArray::const_iterator it = s_readers.begin(); it != s_readers.end(); ++it)
            {
                stream.seek(0);
                if (it->check(stream))
                    return it->create();
            }

            // No suitable reader found
            std::cerr << "Failed to open sound file from memory (format not supported)" << std::endl;
            return NULL;
        }


        ////////////////////////////////////////////////////////////
        SoundFileReader* SoundFileFactory::createReaderFromStream(core::InputStream& stream)
        {
            // Register the built-in readers/writers on first call
            ensureDefaultReadersWritersRegistered();

            // Test the stream for all the registered factories
            for (ReaderFactoryArray::const_iterator it = s_readers.begin(); it != s_readers.end(); ++it)
            {
                stream.seek(0);
                if (it->check(stream))
                    return it->create();
            }

            // No suitable reader found
            std::cerr << "Failed to open sound file from stream (format not supported)" << std::endl;
            return NULL;
        }



    }

} // namespace sf
