/////////////////////////////////////////////////////////////////////////////////
//
// Thor C++ Library
// Copyright (c) 2011-2014 Jan Haller
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
/////////////////////////////////////////////////////////////////////////////////

#include "../../../include/odfaeg/Physics/affector.h"
#include "../../../include/odfaeg/Physics/particule.h"

#include <cassert>
#include <cmath>


namespace odfaeg
{
    namespace physic {
        ForceAffector::ForceAffector(sf::Vector3f acceleration)
        : mAcceleration(acceleration)
        {
        }

        void ForceAffector::operator() (Particle& particle, sf::Time dt)
        {
            particle.velocity += dt.asSeconds() * mAcceleration;
        }

        void ForceAffector::setAcceleration(sf::Vector3f acceleration)
        {
            mAcceleration = acceleration;
        }

        sf::Vector3f ForceAffector::getAcceleration() const
        {
            return mAcceleration;
        }

        // ---------------------------------------------------------------------------------------------------------------------------


        TorqueAffector::TorqueAffector(float angularAcceleration)
        : mAngularAcceleration(angularAcceleration)
        {
        }

        void TorqueAffector::operator() (Particle& particle, sf::Time dt)
        {
            particle.rotationSpeed += dt.asSeconds() * mAngularAcceleration;
        }

        void TorqueAffector::setAngularAcceleration(float angularAcceleration)
        {
            mAngularAcceleration = angularAcceleration;
        }

        float TorqueAffector::getAngularAcceleration() const
        {
            return mAngularAcceleration;
        }

        // ---------------------------------------------------------------------------------------------------------------------------


        ScaleAffector::ScaleAffector(sf::Vector3f scaleFactor)
        : mScaleFactor(scaleFactor)
        {
        }

        void ScaleAffector::operator() (Particle& particle, sf::Time dt)
        {
            particle.scale += dt.asSeconds() * mScaleFactor;
        }

        void ScaleAffector::setScaleFactor(sf::Vector3f scaleFactor)
        {
            mScaleFactor = scaleFactor;
        }

        sf::Vector3f ScaleAffector::getScaleFactor() const
        {
        return mScaleFactor;
        }

        // ---------------------------------------------------------------------------------------------------------------------------


        AnimationAffector::AnimationAffector(std::function<void(Particle&, float)> particleAnimation)
        : mAnimation(std::move(particleAnimation))
        {
        }

        void AnimationAffector::operator() (Particle& particle, sf::Time)
        {
            mAnimation(particle, getElapsedRatio(particle));
        }
    }
} // namespace thor
