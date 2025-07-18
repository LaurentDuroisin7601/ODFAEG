
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

/// @file
/// @brief Concrete particle emitter classes

#ifndef ODFAEG_EMITTER_HPP
#define ODFAEG_EMITTER_HPP

#include "../Math/distribution.h"
#include "export.hpp"

#include "../Core/time.h"
#include "../Math/vec4.h"
#include "../Graphics/color.hpp"


namespace odfaeg
{
    namespace physic {

        class EmissionInterface;


        /// @addtogroup Particles
        /// @{

        /// @brief Creates a functor that references an emitter.
        /// @param referenced Emitter functor to reference.
        /// @details Use this function if you do not want to copy the emitter, but reference it, when you pass it to thor::ParticleSystem.
        /// This allows you to modify the original object after it has been added, and effects are still visible. However, you are responsible
        /// to ensure the lifetime of the referenced object. Example:
        /// @code
        /// // Create emitter and particle system
        /// thor::UniversalEmitter emitter;
        /// thor::ParticleSystem system(...);
        ///
        /// // Add emitter to particle system
        /// system.addEmitter(thor::refEmitter(emitter));
        ///
        /// // Change emitter properties later
        /// emitter.setEmissionRate(20);
        /// @endcode
        template <typename Emitter>
        std::function<void(EmissionInterface&, core::Time)> refEmitter(Emitter& referenced)
        {
            return [&referenced] (EmissionInterface& system, core::Time dt)
            {
                return referenced(system, dt);
            };
        }


        /// @brief Class that emits particles with customizable initial conditions.
        /// @details This emitter is universal with respect to the initial conditions of each emitted particle. It works with callbacks
        /// that return initial values for the particle attributes (position, rotation, color, ...). So you can pass constants, random
        /// distributions, or any functions that compute the value in an arbitrary way. Have a look at the thor::Distributions namespace
        /// for useful predefined functions.
        class ODFAEG_PHYSICS_API UniversalEmitter
        {
        // ---------------------------------------------------------------------------------------------------------------------------
        // Public member functions
        public:
            /// @brief Default constructor
            ///
            UniversalEmitter();

            /// @brief Emits particles into a particle system.
            /// @param system Indirection to the particle system that stores the particles.
            /// @param dt Time interval during which particles are emitted.
            void	operator() (EmissionInterface& system, core::Time dt);

            /// @brief Sets the particle emission rate.
            /// @param particlesPerSecond How many particles are emitted in 1 second. The type is not integral to allow
            /// more flexibility (e.g. 0.5 yields one particle every 2 seconds).
            void	setEmissionRate(float particlesPerSecond);

            /// @brief Sets the lifetime (time between emission and death) of the particle.
            /// @param particleLifetime Constant or function returning the particle lifetime.
            void	setParticleLifetime(math::Distribution<core::Time> particleLifetime);

            /// @brief Sets the initial particle position.
            /// @param particlePosition Constant or function returning the initial particle position.
            void	setParticlePosition(math::Distribution<math::Vec3f> particlePosition);

            /// @brief Sets the initial particle velocity.
            /// @param particleVelocity Constant or function returning the initial particle velocity.
            void	setParticleVelocity(math::Distribution<math::Vec3f> particleVelocity);

            /// @brief Sets the initial particle rotation.
            /// @param particleRotation Constant or function returning the initial particle rotation.
            void	setParticleRotation(math::Distribution<float> particleRotation);

            /// @brief Sets the initial particle rotation speed.
            /// @param particleRotationSpeed Constant or function returning the initial particle rotation speed.
            void	setParticleRotationSpeed(math::Distribution<float> particleRotationSpeed);

            /// @brief Sets the initial particle scale.
            /// @param particleScale Constant or function returning the initial particle scale.
            void	setParticleScale(math::Distribution<math::Vec3f> particleScale);

            /// @brief Sets the initial particle color.
            /// @param particleColor Constant or function returning the initial particle color.
            void	setParticleColor(math::Distribution<graphic::Color> particleColor);

            /// @brief Sets the initial particle texture index.
            /// @param particleTextureIndex Constant or function returning the initial index of the particle's texture rectangle.
            void	setParticleTextureIndex(math::Distribution<unsigned int> particleTextureIndex);


        // ---------------------------------------------------------------------------------------------------------------------------
        // Private member functions
        private:
        // Returns the number of particles to emit during this frame.
        unsigned int	computeParticleCount(core::Time dt);


        // ---------------------------------------------------------------------------------------------------------------------------
        // Private variables
        private:
            float	mEmissionRate;
            float	mEmissionDifference;

            math::Distribution<core::Time>	mParticleLifetime;
            math::Distribution<math::Vec3f>	mParticlePosition;
            math::Distribution<math::Vec3f>	mParticleVelocity;
            math::Distribution<float>	mParticleRotation;
            math::Distribution<float>	mParticleRotationSpeed;
            math::Distribution<math::Vec3f>	mParticleScale;
            math::Distribution<graphic::Color>	mParticleColor;
            math::Distribution<unsigned int>	mParticleTextureIndex;
        };
    }
/// @}

} // namespace thor

#endif // THOR_EMITTER_HPP
