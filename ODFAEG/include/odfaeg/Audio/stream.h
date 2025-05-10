#ifndef ODFAEG_STREAM_HPP
#define ODFAEG_STREAM_HPP
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
namespace odfaeg {
    namespace audio {
        namespace priv {
            class SoundFile;
        }
        /**
        * \file stream.h
        * \class Stream
        * \brief redefines sf::SoundStream to load sounds or open musics and read samples.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        class Stream : public sf::SoundStream
        {
        public:
            /**
            *\fn void load(const sf::SoundBuffer& buffer)
            *\brief load a sound from a sound buffer and extract sampler.
            *\param sf::SoundBuffer buffer : the sound buffer.
            */
            void load(const sf::SoundBuffer& buffer);
            /**
            *\fn bool openFromFile(const std::string& file)
            *\brief open a file to read a music.
            *\param const std::string& file : path to the file.
            *\return true if the file is successfully opened false otherwise.
            */
            bool openFromFile(const std::string& file);
            /**
            *\fn bool openFromMemory(const void* data, std::size_t size)
            *\brief set samples from memory.
            *\param const void* data : sample datas.
            *\param std::size_t size : size of the data.
            *\return true if the file is successfully opened false otherwise.
            */
            bool openFromMemory(const void* data, std::size_t size);
            /**
            *\fn bool openFromStream(sf::InputStream& stream)
            *\brief set samples from an sf::InputStream.
            *\param sf::InputStream& stream : the input stream.
            *\return true if the file is successfully opened false otherwise.
            */
            bool openFromStream(sf::InputStream& stream);
            /**\fn bool isFinished()
            *\brief check if the stream has finished to be read.
            *\return true is the stream has finished to be read, false otherwise.
            */
            bool isFinished();
            /**
            *\fn bool stereo.
            *\brief check if the stream is mono or stereo.
            *\return true if the stream is a stereo stream, false otherwise.
            */
            bool stereo();
            /**
            *\fn const sf::Int16* getSamples()
            *\brief get samples.
            *\return sf::Int16* pointer to samples.
            */
            const sf::Int16* getSamples();
        private:
            /**
            *\fn bool onGetData(Chunk& data);
            *\brief redefines the onGetData function of the sf::SoundStream to pass the samples data to the SFML sound stream.
            *\return true if we haven't passed every samples, false otherwise.
            */
            virtual bool onGetData(Chunk& data);
            /**
            *\fn void onSeek(sf::Time timeOffset)
            *\brief redefines the onSeek function of the sf::SoundStream to read from the time passed.
            *\param sf::Time timeOffset : the offset time.
            */
            virtual void onSeek(sf::Time timeOffset);
            /**\fn void initialize()
            *  \brief perform some initialisations.
            */
            void initialize();
            priv::SoundFile* m_file; /**> the soundfile to get the music samples.*/
            sf::Time m_duration; /**> the duration of the stream.*/
            std::vector<sf::Int16> m_samples; /**>the samples.*/
            std::size_t m_currentSample; /**>The current sample which'll be read.*/
            sf::Mutex m_mutex; /**>Mutex to protect samples data because sounds are played with a thread.*/
        };
    }
}
#endif
