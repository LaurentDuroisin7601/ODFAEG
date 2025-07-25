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

#ifndef ODFAEG_SOUNDFILEREADER_HPP
#define ODFAEG_SOUNDFILEREADER_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "export.hpp"
#include "../Core/inputStream.hpp"
#include <string>


namespace odfaeg
{
    namespace audio {


        ////////////////////////////////////////////////////////////
        /// \brief Abstract base class for sound file decoding
        ///
        ////////////////////////////////////////////////////////////
        class ODFAEG_AUDIO_API SoundFileReader
        {
        public:

            ////////////////////////////////////////////////////////////
            /// \brief Structure holding the audio properties of a sound file
            ///
            ////////////////////////////////////////////////////////////
            struct Info
            {
                std::uint64_t       sampleCount;  ///< Total number of samples in the file
                unsigned int channelCount; ///< Number of channels of the sound
                unsigned int sampleRate;   ///< Samples rate of the sound, in samples per second
            };

            ////////////////////////////////////////////////////////////
            /// \brief Virtual destructor
            ///
            ////////////////////////////////////////////////////////////
            virtual ~SoundFileReader() {}

            ////////////////////////////////////////////////////////////
            /// \brief Open a sound file for reading
            ///
            /// The provided stream reference is valid as long as the
            /// SoundFileReader is alive, so it is safe to use/store it
            /// during the whole lifetime of the reader.
            ///
            /// \param stream Source stream to read from
            /// \param info   Structure to fill with the properties of the loaded sound
            ///
            /// \return True if the file was successfully opened
            ///
            ////////////////////////////////////////////////////////////
            virtual bool open(core::InputStream& stream, Info& info) = 0;

            ////////////////////////////////////////////////////////////
            /// \brief Change the current read position to the given sample offset
            ///
            /// The sample offset takes the channels into account.
            /// If you have a time offset instead, you can easily find
            /// the corresponding sample offset with the following formula:
            /// `timeInSeconds * sampleRate * channelCount`
            /// If the given offset exceeds to total number of samples,
            /// this function must jump to the end of the file.
            ///
            /// \param sampleOffset Index of the sample to jump to, relative to the beginning
            ///
            ////////////////////////////////////////////////////////////
            virtual void seek(std::uint64_t sampleOffset) = 0;

            ////////////////////////////////////////////////////////////
            /// \brief Read audio samples from the open file
            ///
            /// \param samples  Pointer to the sample array to fill
            /// \param maxCount Maximum number of samples to read
            ///
            /// \return Number of samples actually read (may be less than \a maxCount)
            ///
            ////////////////////////////////////////////////////////////
            virtual std::uint64_t read(std::int16_t* samples, std::uint64_t maxCount) = 0;
        };
    }

} // namespace sf


#endif // ODFAEG_SOUNDFILEREADER_HPP


////////////////////////////////////////////////////////////
/// \class sf::SoundFileReader
/// \ingroup audio
///
/// This class allows users to read audio file formats not natively
/// supported by ODFAEG, and thus extend the set of supported readable
/// audio formats.
///
/// A valid sound file reader must override the open, seek and write functions,
/// as well as providing a static check function; the latter is used by
/// ODFAEG to find a suitable writer for a given input file.
///
/// To register a new reader, use the sf::SoundFileFactory::registerReader
/// template function.
///
/// Usage example:
/// \code
/// class MySoundFileReader : public sf::SoundFileReader
/// {
/// public:
///
///     static bool check(sf::InputStream& stream)
///     {
///         // typically, read the first few header bytes and check fields that identify the format
///         // return true if the reader can handle the format
///     }
///
///     virtual bool open(sf::InputStream& stream, Info& info)
///     {
///         // read the sound file header and fill the sound attributes
///         // (channel count, sample count and sample rate)
///         // return true on success
///     }
///
///     virtual void seek(sf::std:::uint64_t sampleOffset)
///     {
///         // advance to the sampleOffset-th sample from the beginning of the sound
///     }
///
///     virtual sf::std:::uint64_t read(sf::Int16* samples, sf::std:::uint64_t maxCount)
///     {
///         // read up to 'maxCount' samples into the 'samples' array,
///         // convert them (for example from normalized float) if they are not stored
///         // as 16-bits signed integers in the file
///         // return the actual number of samples read
///     }
/// };
///
/// sf::SoundFileFactory::registerReader<MySoundFileReader>();
/// \endcode
///
/// \see sf::InputSoundFile, sf::SoundFileFactory, sf::SoundFileWriter
///
////////////////////////////////////////////////////////////
