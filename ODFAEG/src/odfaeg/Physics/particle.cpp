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

#include "../../../include/odfaeg/Physics/particule.h"


namespace odfaeg
{
    namespace physic {
        Particle::Particle(core::Time totalLifetime)
        : position()
        , velocity()
        , rotation()
        , rotationSpeed()
        , scale(1.f, 1.f, 1.f)
        , color(255, 255, 255)
        , textureIndex(0)
        , passedLifetime(core::Time::zero)
        , totalLifetime(totalLifetime)
        {
        }

        core::Time getElapsedLifetime(const Particle& particle)
        {
            return particle.passedLifetime;
        }

        core::Time getTotalLifetime(const Particle& particle)
        {
            return particle.totalLifetime;
        }

        core::Time getRemainingLifetime(const Particle& particle)
        {
            return getTotalLifetime(particle) - getElapsedLifetime(particle);
        }

        float getElapsedRatio(const Particle& particle)
        {
            return (getElapsedLifetime(particle).asSeconds() / getTotalLifetime(particle).asSeconds());
        }

        float getRemainingRatio(const Particle& particle)
        {
            return (getRemainingLifetime(particle).asSeconds() / getTotalLifetime(particle).asSeconds());
        }
    }
} // namespace thor


