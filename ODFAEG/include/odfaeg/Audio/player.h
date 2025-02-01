#ifndef ODFAEG_PLAYER_HPP
#define ODFAEG_PLAYER_HPP
#include "stream.h"
#include <memory>
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace audio {
        /**
        * \file player.h
        * \class Player
        * \brief play a sound or a music and set parameters for the sound spacialization.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        class Player {
        public :
            /** \fn Player()
            *\brief construct player with no sound stream.
            */
            Player();
            /** \fn Player(sf::SoundBuffer& soundBuffer)
            *\brief construct player and initialize the stream with the given sound buffer.
            *\param sf::SoundBuffer& soundBuffer : the sound buffer.
            */
            Player (sf::SoundBuffer& soundBuffer);
            /** \fn void setAudioStream(Stream* stream)
            *\brief set the audio stream.
            *\param Stream* stream : pointer to the audio stream.
            */
            void setAudioStream(Stream* stream);
            /** \fn Stream* getAudioStream()
            *   \brief get the pointer to the audio stream.
            *   \return the pointer to the audio stream.
            */
            Stream* getAudioStream();
            /** \fn void play (bool loop = false)
            *   \brief play the sound.
            *   \param bool loop : if the sound have to be played in loop.
            */
            void play (bool loop = false);
            /** \fn void stop()
            *   \brief stop the sound.
            */
            void stop ();
            /** \fn void pause()
            *   \brief set the sound in pause.
            */
            void pause ();
            /** \fn bool isPlaying()
            *   \brief check if the sound is playing.
            *   \return if the sound is playing or not.
            */
            bool isPlaying();
            /** \fn void setPitch(float pitch)
            *   \brief set the pitch.
            *   \param float pitch : the pitch.
            */
            void setPitch(float pitch);
            /** \fn void setVolume(float volume)
            *   \brief set the volume.
            *   \param float volume : the volume.
            */
            void setVolume(float volume);
            /**
            *   \fn void setPosition(float x, float y, float z)
            *   \brief set the position of the sound.
            *   \param x : the x position.
            *   \param y : the y position.
            *   \param z : the z position.
            */
            void setPosition(float x, float y, float z);
            /**
            *   \fn void setPosition(sf::Vector3f position)
            *   \brief set the position of the sound.
            *   \param sf::Vector3f position : the position of the sound.
            */
            void setPosition(sf::Vector3f position);
            /** \fn void setRelativeToListener(bool relative)
            *   \brief set if the sound's position is relative to the listener's position or not.
            *   \param bool relative : if the sound is relative to the listener's position.
            */
            void setRelativeToListener(bool relative);
            /** \fn void setMinDistance (float minDistance)
            *   \brief set the minimum distance from which the sound have to be heard.
            *   \param float minDistance : the minimum distance.
            */
            void setMinDistance (float minDistance);
            /** \fn void setAttenuation(float attenuation)
            *   \brief set the attenuation of the sound when we move away from the sound's position.
            *   \param float attenuation : the attenuation factor.
            */
            void setAttenuation(float attenuation);
            /** \fn float getPitch() const
            *   \brief get the pitch of the sound.
            *   \return the pitch of the sound.
            */
            float getPitch() const;
            /** \fn float getVolume() const
            *   \brief get the volume of the sound.
            *   \return the volume of the sound.
            */
            float getVolume() const;
            /** \fn sf::Vector3f getPosition() const
            *   \brief get the position of the sound.
            *   \return the position of the sound.
            */
            sf::Vector3f getPosition() const;
            /**
            *   \fn bool isRelativeToListener()
            *   \brief check if the sound's position is relative to the listener's position.
            *   \param return if the sound's position is relative to the listener's position.
            */
            bool isRelativeToListener() const;
            /**
            *   \fn float getMinDistance() const
            *   \brief get the minimum distance from which the sound is heard.
            *   \return the minimum distance.
            */
            float getMinDistance() const;
            /**
            *   \fn float getAttenuation() const
            *   \brief get the attenuation of the sound.
            *   \return the attenuation factor.
            */
            float getAttenuation() const;
            /**
            *  \fn bool stereo()
            *  \brief check if the sound is stereo.
            *  \return if the sound is stereo.
            */
            bool stereo();
            /**
            * destroy the sound stream.
            */
            ~Player();
        private:
            Stream* stream; /**> the sound's stream*/
            bool inPause; /**> if the sound is in pause.*/
        };

    }
}
#endif // ODFAEG_PLAYER_HPP
