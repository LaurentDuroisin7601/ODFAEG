#include "../../../include/odfaeg/Audio/player.h"
#include <iostream>

namespace odfaeg {
    namespace audio {
        Player::Player() {
            stream = nullptr;
            inPause = true;
        }
        Player::Player(SoundBuffer& buffer) {
            Stream* stream = new Stream();
            stream->load(buffer);
            this->stream = stream;
            inPause = true;
        }
        bool Player::stereo() {
            return stream->stereo();
        }
        void Player::setAudioStream(Stream* stream) {
            this->stream = stream;
        }
        Stream* Player::getAudioStream() {
            return stream;
        }
        void Player::play(bool loop) {
            inPause = false;
            stream->setLoop(loop);
            stream->play();
        }
        void Player::stop() {
            inPause = true;
            stream->stop();
        }
        void Player::pause() {
            inPause = true;
            stream->pause();
        }
        bool Player::isPlaying() {
            return !inPause && !stream->isFinished();
        }
        void Player::setPitch(float pitch) {
            stream->setPitch(pitch);
        }
        void Player::setVolume(float volume) {
            stream->setVolume(volume);
        }
        void Player::setPosition(float x, float y, float z) {
            stream->setPosition(x, y, z);
        }
        void Player::setPosition(math::Vec3f position) {
            stream->setPosition(math::Vec3f(position.x(), position.y(), position.z()));
        }
        void Player::setRelativeToListener(bool relative) {
            stream->setRelativeToListener(relative);
        }
        void Player::setMinDistance(float minDistance) {
            stream->setMinDistance(minDistance);
        }
        void Player::setAttenuation(float attenuation) {
            stream->setAttenuation(attenuation);
        }
        float Player::getPitch() const {
            return stream->getPitch();
        }
        float Player::getVolume() const {
            return stream->getVolume();
        }
        math::Vec3f Player::getPosition() const {
            return math::Vec3f(stream->getPosition().x(), stream->getPosition().y(), stream->getPosition().z()) ;
        }
        bool Player::isRelativeToListener() const {
            return stream->isRelativeToListener();
        }
        float Player::getMinDistance() const {
            return stream->getMinDistance();
        }
        float Player::getAttenuation() const {
            return stream->getAttenuation();
        }
        Player::~Player() {
            delete stream;
        }
    }
}

