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
/// @brief Structure thor::Particle

#ifndef ODFAEG_PARTICLE_HPP
#define ODFAEG_PARTICLE_HPP

#include "export.hpp"
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector3.hpp>
#include <SFML/Graphics/Color.hpp>


namespace odfaeg
{
    namespace physic {

        /// @addtogroup Particles
        /// @{

        /// @brief %Particle class
        /// @details This class represents single particles of a particle system. It stores properties like position, rotation,
        /// scale, movement, color and life time.
        /// @n You normally don't have to work directly with particles outside of an affector or emitter.
        class ODFAEG_PHYSICS_API Particle
        {
        // ---------------------------------------------------------------------------------------------------------------------------
        // Public member functions
        public:
        /// @brief Constructor
        /// @param totalLifetime How long the particle totally exists.
        explicit	Particle(sf::Time totalLifetime);
        bool operator== (const Particle& other) const {
            return position.x == other.position.x && position.y == other.position.y && position.z == other.position.z
                   && velocity.x == other.velocity.x && velocity.y == other.velocity.y && velocity.z == other.velocity.z
                   && rotation == other.rotation && rotationSpeed == other.rotationSpeed && scale.x == other.scale.x
                   && scale.y == other.scale.y && scale.z == other.scale.z && color.a == other.color.a && color.b == other.color.b
                   && color.g == other.color.g && color.r == other.color.r && textureIndex == other.textureIndex;
        }        // ---------------------------------------------------------------------------------------------------------------------------
        // Public variables
        public:
        sf::Vector3f	position;	///< Current position.
        sf::Vector3f	velocity;	///< Velocity (change in position per second).
        float	rotation;	///< Current rotation angle.
        float	rotationSpeed;	///< Angular velocity (change in rotation per second).
        sf::Vector3f	scale;	///< Scale, where (1,1) represents the original size.
        sf::Color	color;	///< %Particle color.
        unsigned int	textureIndex;	///< Index of the used texture rect, returned by ParticleSystem::addTextureRect()


        // ---------------------------------------------------------------------------------------------------------------------------
        // Private variables
        private:
        sf::Time	passedLifetime;	// Time passed since emitted.
        sf::Time	totalLifetime;	// Total time to live.


        // ---------------------------------------------------------------------------------------------------------------------------
        // Friends
        /// @cond FriendsAreAnImplementationDetail
        friend class ParticleSystem;
        friend sf::Time ODFAEG_PHYSICS_API getElapsedLifetime(const Particle& particle);
        friend sf::Time ODFAEG_PHYSICS_API getTotalLifetime(const Particle& particle);
        /// @endcond
        };

        /// @relates Particle
        /// @brief Returns the time passed since the particle has been emitted.
        sf::Time ODFAEG_PHYSICS_API	getElapsedLifetime(const Particle& particle);

        /// @relates Particle
        /// @brief Returns the total time the particle is alive.
        sf::Time ODFAEG_PHYSICS_API	getTotalLifetime(const Particle& particle);

        /// @relates Particle
        /// @brief Returns the time left until the particle dies.
        sf::Time ODFAEG_PHYSICS_API	getRemainingLifetime(const Particle& particle);

        /// @relates Particle
        /// @brief Returns <b>elapsed lifetime / total lifetime</b>.
        float ODFAEG_PHYSICS_API	getElapsedRatio(const Particle& particle);

        /// @relates Particle
        /// @brief Returns <b>remaining lifetime / total lifetime</b>.
        float ODFAEG_PHYSICS_API	getRemainingRatio(const Particle& particle);

        /// @}
    }

} // namespace thor

#endif // THOR_PARTICLE_HPP
