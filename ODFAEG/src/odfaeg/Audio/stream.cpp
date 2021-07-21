#include "../../../include/odfaeg/Audio/stream.h"
#include "SoundFile.hpp"
using namespace sf;
namespace odfaeg {
    namespace audio {
        void Stream::load(const sf::SoundBuffer& buffer)
        {
            m_file = nullptr;
            // extract the audio samples from the sound buffer to our own container
            m_samples.assign(buffer.getSamples(), buffer.getSamples() + buffer.getSampleCount());

            // reset the current playing position
            m_currentSample = 0;

            // initialize the base class
            SoundStream::initialize(buffer.getChannelCount(), buffer.getSampleRate());
        }
        ////////////////////////////////////////////////////////////
        bool Stream::openFromFile(const std::string& filename)
        {
            // First stop the music if it was already running
            m_file = new priv::SoundFile();
            stop();

            // Open the underlying sound file
            if (!m_file->openRead(filename))
                return false;

            // Perform common initializations
            initialize();

            return true;
        }


        ////////////////////////////////////////////////////////////
        bool Stream::openFromMemory(const void* data, std::size_t sizeInBytes)
        {
            m_file = new priv::SoundFile();
            // First stop the music if it was already running
            stop();

            // Open the underlying sound file
            if (!m_file->openRead(data, sizeInBytes))
                return false;

            // Perform common initializations
            initialize();

            return true;
        }


        ////////////////////////////////////////////////////////////
        bool Stream::openFromStream(InputStream& stream)
        {
            m_file = new priv::SoundFile();
            // First stop the music if it was already running
            stop();

            // Open the underlying sound file
            if (!m_file->openRead(stream))
                return false;

            // Perform common initializations
            initialize();

            return true;
        }
        bool Stream::onGetData(Chunk& data)
        {
            Lock lock(m_mutex);
            if (m_file == nullptr) {
                // number of samples to stream every time the function is called;
                // in a more robust implementation, it would rather be a fixed
                // amount of time rather than an arbitrary number of samples
                const int samplesToStream = 50000;

                // set the pointer to the next audio samples to be played
                data.samples = &m_samples[m_currentSample];

                // have we reached the end of the sound?
                if (m_currentSample + samplesToStream <= m_samples.size())
                {
                    // end not reached: stream the samples and continue
                    data.sampleCount = samplesToStream;
                    m_currentSample += samplesToStream;
                    return true;
                }
                else
                {
                    // end of stream reached: stream the remaining samples and stop playback
                    data.sampleCount = m_samples.size() - m_currentSample;
                    m_currentSample = m_samples.size();
                    return false;
                }
            } else {
                // Fill the chunk parameters
                data.samples     = &m_samples[0];
                data.sampleCount = m_file->read(&m_samples[0], m_samples.size());

                // Check if we have reached the end of the audio file
                return data.sampleCount == m_samples.size();
            }
        }
        bool Stream::isFinished() {
            return m_currentSample == m_samples.size() - 1;
        }
        void Stream::onSeek(sf::Time timeOffset)
        {
            // compute the corresponding sample index according to the sample rate and channel count
            m_currentSample = static_cast<std::size_t>(timeOffset.asSeconds() * getSampleRate() * getChannelCount());
        }
        bool Stream::stereo() {
            return m_file->getChannelCount() > 1;
        }
        void Stream::initialize()
        {
            // Compute the music duration
            m_duration = seconds(static_cast<float>(m_file->getSampleCount()) / m_file->getSampleRate() / m_file->getChannelCount());

            // Resize the internal buffer so that it can contain 1 second of audio samples
            m_samples.resize(m_file->getSampleRate() * m_file->getChannelCount());

            // Initialize the stream
            SoundStream::initialize(m_file->getChannelCount(), m_file->getSampleRate());
        }
        const sf::Int16* Stream::getSamples() {
            return &m_samples[0];
        }
    }
}

