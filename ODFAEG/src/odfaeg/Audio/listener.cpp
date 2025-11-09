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
#include "../../../include/odfaeg/Audio/listener.hpp"
#include "audioDevice.hpp"


namespace odfaeg {

    namespace audio {
        ////////////////////////////////////////////////////////////
        void Listener::setGlobalVolume(float volume)
        {
            priv::AudioDevice::setGlobalVolume(volume);
        }


        ////////////////////////////////////////////////////////////
        float Listener::getGlobalVolume()
        {
            return priv::AudioDevice::getGlobalVolume();
        }


        ////////////////////////////////////////////////////////////
        void Listener::setPosition(float x, float y, float z)
        {
            setPosition(math::Vec3f(x, y, z));
        }


        ////////////////////////////////////////////////////////////
        void Listener::setPosition(const math::Vec3f& position)
        {
            priv::AudioDevice::setPosition(position);
        }


        ////////////////////////////////////////////////////////////
        math::Vec3f Listener::getPosition()
        {
            return priv::AudioDevice::getPosition();
        }


        ////////////////////////////////////////////////////////////
        void Listener::setDirection(float x, float y, float z)
        {
            setDirection(math::Vec3f(x, y, z));
        }


        ////////////////////////////////////////////////////////////
        void Listener::setDirection(const math::Vec3f& direction)
        {
            priv::AudioDevice::setDirection(direction);
        }


        ////////////////////////////////////////////////////////////
        math::Vec3f Listener::getDirection()
        {
            return priv::AudioDevice::getDirection();
        }


        ////////////////////////////////////////////////////////////
        void Listener::setUpVector(float x, float y, float z)
        {
            setUpVector(math::Vec3f(x, y, z));
        }


        ////////////////////////////////////////////////////////////
        void Listener::setUpVector(const math::Vec3f& upVector)
        {
            priv::AudioDevice::setUpVector(upVector);
        }


        ////////////////////////////////////////////////////////////
        math::Vec3f Listener::getUpVector()
        {
            return priv::AudioDevice::getUpVector();
        }
    }

} // namespace sf
