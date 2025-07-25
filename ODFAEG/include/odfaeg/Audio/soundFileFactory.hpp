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

#ifndef ODFAEG_SOUNDFILEFACTORY_HPP
#define ODFAEG_SOUNDFILEFACTORY_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "export.hpp"
#include "../Core/inputStream.hpp"
#include <string>
#include <vector>


namespace odfaeg {

    namespace audio {
        class SoundFileReader;

        ////////////////////////////////////////////////////////////
        /// \brief Manages and instantiates sound file readers and writers
        ///
        ////////////////////////////////////////////////////////////
        class ODFAEG_AUDIO_API SoundFileFactory
        {
        public:

            ////////////////////////////////////////////////////////////
            /// \brief Register a new reader
            ///
            /// \see unregisterReader
            ///
            ////////////////////////////////////////////////////////////
            template <typename T>
            static void registerReader();

            ////////////////////////////////////////////////////////////
            /// \brief Unregister a reader
            ///
            /// \see registerReader
            ///
            ////////////////////////////////////////////////////////////
            template <typename T>
            static void unregisterReader();


            ////////////////////////////////////////////////////////////
            /// \brief Instantiate the right reader for the given file on disk
            ///
            /// It's up to the caller to release the returned reader
            ///
            /// \param filename Path of the sound file
            ///
            /// \return A new sound file reader that can read the given file, or null if no reader can handle it
            ///
            /// \see createReaderFromMemory, createReaderFromStream
            ///
            ////////////////////////////////////////////////////////////
            static SoundFileReader* createReaderFromFilename(const std::string& filename);

            ////////////////////////////////////////////////////////////
            /// \brief Instantiate the right codec for the given file in memory
            ///
            /// It's up to the caller to release the returned reader
            ///
            /// \param data        Pointer to the file data in memory
            /// \param sizeInBytes Total size of the file data, in bytes
            ///
            /// \return A new sound file codec that can read the given file, or null if no codec can handle it
            ///
            /// \see createReaderFromFilename, createReaderFromStream
            ///
            ////////////////////////////////////////////////////////////
            static SoundFileReader* createReaderFromMemory(const void* data, std::size_t sizeInBytes);

            ////////////////////////////////////////////////////////////
            /// \brief Instantiate the right codec for the given file in stream
            ///
            /// It's up to the caller to release the returned reader
            ///
            /// \param stream Source stream to read from
            ///
            /// \return A new sound file codec that can read the given file, or null if no codec can handle it
            ///
            /// \see createReaderFromFilename, createReaderFromMemory
            ///
            ////////////////////////////////////////////////////////////
            static SoundFileReader* createReaderFromStream(core::InputStream& stream);



        private:

            ////////////////////////////////////////////////////////////
            // Types
            ////////////////////////////////////////////////////////////
            struct ReaderFactory
            {
                bool (*check)(core::InputStream&);
                SoundFileReader* (*create)();
            };
            typedef std::vector<ReaderFactory> ReaderFactoryArray;
            ////////////////////////////////////////////////////////////
            // Static member data
            ////////////////////////////////////////////////////////////
            static ReaderFactoryArray s_readers; ///< List of all registered readers

        };
    }

} // namespace sf

#include "soundFileFactory.inl"

#endif // ODFAEG_SOUNDFILEFACTORY_HPP


////////////////////////////////////////////////////////////
/// \class sf::SoundFileFactory
/// \ingroup audio
///
/// This class is where all the sound file readers and writers are
/// registered. You should normally only need to use its registration
/// and unregistration functions; readers/writers creation and manipulation
/// are wrapped into the higher-level classes sf::InputSoundFile and
/// sf::OutputSoundFile.
///
/// To register a new reader (writer) use the sf::SoundFileFactory::registerReader
/// (registerWriter) static function. You don't have to call the unregisterReader
/// (unregisterWriter) function, unless you want to unregister a format before your
/// application ends (typically, when a plugin is unloaded).
///
/// Usage example:
/// \code
/// sf::SoundFileFactory::registerReader<MySoundFileReader>();
/// sf::SoundFileFactory::registerWriter<MySoundFileWriter>();
/// \endcode
///
/// \see sf::InputSoundFile, sf::OutputSoundFile, sf::SoundFileReader, sf::SoundFileWriter
///
////////////////////////////////////////////////////////////
