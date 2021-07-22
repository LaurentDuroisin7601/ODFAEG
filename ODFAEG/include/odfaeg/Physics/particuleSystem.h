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
/// @brief Class thor::ParticleSystem

#ifndef ODFAEG_PARTICLESYSTEM_HPP
#define ODFAEG_PARTICLESYSTEM_HPP

#include "particule.h"
#include "emissionInterface.h"
#include "emmiters.h"
#include "export.hpp"
#include "affector.h"

#include <SFML/System/NonCopyable.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Rect.hpp>
#include "../Graphics/drawable.h"
#include "../Graphics/vertexArray.h"
#include "../Graphics/entity.h"
#include "../Graphics/entityManager.h"
#include "../Math/vec4.h"
#include "../Graphics/gameObject.hpp"

#include <vector>
#include <utility>
#include <functional>
#include <memory>
#include <array>




namespace odfaeg
{
    namespace physic {
        /// @addtogroup Particles
        /// @{

        /// @brief Class for particle systems.
        /// @details A particle system stores, updates and draws particles. It also stores emitter and affector functions that control
        /// how particles are generated and modified over time. To represent particles graphically, a particle system requires a texture,
        /// and optionally one or multiple texture rectangles.
        /// @n@n This class is noncopyable.
        class ODFAEG_PHYSICS_API ParticleSystem : public graphic::GameObject, private sf::NonCopyable, private EmissionInterface
        {
        // ---------------------------------------------------------------------------------------------------------------------------
        // Private types
        private:
        // Type to store affector or emitter + time until removal + id for removal
        template <typename Signature>
        struct Function
        {
        Function(std::function<Signature> function, sf::Time timeUntilRemoval)
        : function(std::move(function))
        , timeUntilRemoval(timeUntilRemoval)
        {
        }
        std::function<Signature>	function;
        sf::Time	timeUntilRemoval;
        };

        // Vertex quads, used to cache texture rectangles
        typedef std::array<graphic::Vertex, 4>	Quad;

        // Function typedefs
        typedef Function<void(Particle&, sf::Time)>	Affector;
        typedef Function<void(EmissionInterface&, sf::Time)>	Emitter;

        // Container typedefs
        typedef std::vector<Particle>	ParticleContainer;
        typedef std::vector<Affector>	AffectorContainer;
        typedef std::vector<Emitter>	EmitterContainer;


        // ---------------------------------------------------------------------------------------------------------------------------
        // Public member functions
        public:
        ParticleSystem(graphic::EntityFactory& factory);
        /// @brief Default constructor
        /// @details Requires a call to setTexture() and possibly addTextureRect() before the particle system can be used.
        ParticleSystem(math::Vec3f position, math::Vec3f size, graphic::EntityFactory& factory);
        graphic::Entity* clone();
        /// @brief Swaps the contents of two instances in constant time.
        ///
        void	swap(ParticleSystem& other);
        //void update();

        /// @brief Sets the used texture.
        /// @details Only one texture can be used at a time. If you need multiple particle representations, specify different texture
        /// rectangles using the method addTextureRect(). If no texture rect is added, the whole texture will be used.
        /// @param texture Reference to the sf::Texture used as particle texture. The texture must remain valid as long as the
        /// particle system uses it.
        void	setTexture(const graphic::Texture& texture);

        /// @brief Defines a new texture rect to represent a particle.
        /// @details Can be used to create different visual representations of a particle, for example different shapes of debris.
        /// @param textureRect Area of the texture that is used to draw the particle.
        /// @return Index of the texture rect just added. This number shall be assigned to the Particle::textureIndex variable,
        /// in order to render a particle using @a textureRect.
        /// @see setTexture()
        unsigned int	addTextureRect(const sf::IntRect& textureRect);

        /// @brief Adds a particle affector to the system.
        /// @details Be aware that multiple affectors can interfere with each other. The affectors are applied in the order they were
        /// added to the system, therefore affectors at the end may overwrite particle states set by earlier affectors. To completely
        /// avoid the issue, only add orthogonal affectors (e.g. one for color, one for acceleration...).
        /// @param affector Affector function object which is copied into the particle system.
        /// @return Object that can be used to disconnect (remove) the affector from the system.
        void addAffector(std::function<void(Particle&, sf::Time)> affector);

        /// @brief Adds a particle affector for a certain amount of time.
        /// @details Be aware that multiple affectors can interfere with each other. The affectors are applied in the order they were
        /// added to the system, therefore affectors at the end may overwrite particle states set by earlier affectors. To completely
        /// avoid the issue, only add orthogonal affectors (e.g. one for color, one for acceleration...).
        /// @param affector Affector function object which is copied into the particle system.
        /// @param timeUntilRemoval Time after which the affector is automatically removed from the system.
        /// @return Object that can be used to disconnect (remove) the affector from the system.
        void addAffector(std::function<void(Particle&, sf::Time)> affector, sf::Time timeUntilRemoval);

        /// @brief Removes all affector instances from the system.
        /// @details All particles lose the influence of any external affectors. Movement and lifetime is still computed.
        void	clearAffectors();

        /// @brief Adds a particle emitter to the system.
        /// @param emitter Emitter function object which is copied into the particle system.
        /// @return Object that can be used to disconnect (remove) the emitter from the system.
        void	addEmitter(std::function<void(EmissionInterface&, sf::Time)> emitter);

        /// @brief Adds a particle emitter for a certain amount of time.
        /// @param emitter Emitter function object which is copied into the particle system.
        /// @param timeUntilRemoval Time after which the emitter is automatically removed from the system.
        /// @return Object that can be used to disconnect (remove) the emitter from the system.
        void	addEmitter(std::function<void(EmissionInterface&, sf::Time)> emitter, sf::Time timeUntilRemoval);

        /// @brief Removes all emitter instances from the system.
        /// @details Particles that are currently in the system are still processed, but no new ones
        /// are emitted until you add another emitter.
        void	clearEmitters();

        /// @brief Updates all particles in the system.
        /// @details Invokes all emitters and applies all affectors. The lifetime of every particle is decreased,
        /// dead particles are removed.
        /// @param dt Frame duration.
        void	update(sf::Time dt);

        /// @brief Removes all particles that are currently in the system.
        ///
        void	clearParticles();

        void update();
        void setScene(graphic::EntityManager* scene);
        bool isAnimated () const {
            return false;
        }
        bool isModel() const {
            return false;
        }
        bool selectable() const {
            return true;
        }
        bool isLight() const {
            return false;
        }
        bool isShadow() const {
            return false;
        }
        bool isLeaf() const {
            return true;
        }
        template<typename Archive>
        void vtserialize(Archive& ar) {
            GameObject::vtserialize(ar);
            if (!ar.isInputArchive()) {
                unsigned int size = mTextureRects.size();
                ar(size);
                for (unsigned int i = 0; i < size; i++) {
                    ar(mTextureRects[i].left);
                    ar(mTextureRects[i].top);
                    ar(mTextureRects[i].width);
                    ar(mTextureRects[i].height);
                }
            } else {
                unsigned int size;
                ar(size);
                for (unsigned int i = 0; i < size; i++) {
                    sf::IntRect rect;
                    ar(rect.left);
                    ar(rect.top);
                    ar(rect.width);
                    ar(rect.height);
                    addTextureRect(rect);
                }
            }
        }
        // ---------------------------------------------------------------------------------------------------------------------------
        // Private member functions
        private:
        /// @brief Draws all particles in the system.
        /// @param target The render target to which the particles are drawn.
        /// @param states Current render states (shader, blend mode, ...)
        virtual void	draw(graphic::RenderTarget& target, graphic::RenderStates states);

        /// @brief Emits a particle into the system.
        /// @param particle Particle to emit.
        virtual void	emitParticle(const Particle& particle);

        // Updates a single particle.
        void	updateParticle(Particle& particle, sf::Time dt);

        // Recomputes the vertex array.
        void	computeVertices() const;

        // Recomputes the cached rectangles (position and texCoords quads)
        void	computeQuads() const;
        void	computeQuad(Quad& quad, const sf::IntRect& textureRect) const;
        void removeEmitter (std::function<void(EmissionInterface&, sf::Time)>& em);
        bool operator== (Entity& other) {
            return GameObject::operator==(other);
        }
        // ---------------------------------------------------------------------------------------------------------------------------
        // Private variables
        private:
        ParticleContainer	mParticles;
        AffectorContainer	mAffectors;
        EmitterContainer	mEmitters;
        const graphic::Texture*	mTexture;
        std::vector<sf::IntRect>	mTextureRects;
        mutable graphic::VertexArray	mVertices;
        mutable bool	mNeedsVertexUpdate;
        mutable std::vector<Quad>	mQuads;
        mutable bool	mNeedsQuadUpdate;
        };
    }

} // namespace thor

#endif // THOR_PARTICLESYSTEM_HPP
