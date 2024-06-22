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

#include "../../../include/odfaeg/Physics/particuleSystem.h"


#include "../../../include/odfaeg/Graphics/renderWindow.h"
#include "../../../include/odfaeg/Graphics/texture.h"
#include "../../../include/odfaeg/Math/transformMatrix.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cassert>


namespace odfaeg
{
    namespace physic {
            namespace
            {

                // Erases emitter/affector at itr from ctr, if its time has expired. itr will point to the next element.
                template <class Container>
                void incrementCheckExpiry(Container& ctr, typename Container::iterator& itr, sf::Time dt)
                {
                    // itr->second is the remaining time of the emitter/affector.
                    // Time::Zero means infinite time (no removal).
                    if (itr->timeUntilRemoval != sf::Time::Zero && (itr->timeUntilRemoval -= dt) <= sf::Time::Zero)
                    itr = ctr.erase(itr);
                    else
                    ++itr;
                }

                sf::IntRect getFullRect(const graphic::Texture& texture)
                {
                    return sf::IntRect(0, 0, texture.getSize().x, texture.getSize().y);
                }

            } // namespace

        // ---------------------------------------------------------------------------------------------------------------------------

        ParticleSystem::ParticleSystem(graphic::EntityFactory& factory) : graphic::GameObject(math::Vec3f(0, 0, 0), math::Vec3f(0, 0, 0), math::Vec3f(0, 0, 0), "E_PARTICLES", factory), mParticles()
        , mAffectors()
        , mEmitters()
        , mTexture(nullptr)
        , mTextureRects()
        , mVertices(sf::Quads)
        , mNeedsVertexUpdate(true)
        , mQuads()
        , mNeedsQuadUpdate(true) {

        }
        ParticleSystem::ParticleSystem(math::Vec3f position, math::Vec3f size, graphic::EntityFactory& factory)
        : graphic::GameObject(position, size, size*0.5f, "E_PARTICLES", factory), mParticles()
        , mAffectors()
        , mEmitters()
        , mTexture(nullptr)
        , mTextureRects()
        , mVertices(sf::Quads)
        , mNeedsVertexUpdate(true)
        , mQuads()
        , mNeedsQuadUpdate(true)
        {
            graphic::Material material;
            material.addTexture(nullptr);
            graphic::Face face (mVertices, material, getTransform());
            addFace(face);
        }
        graphic::Entity* ParticleSystem::clone() {
            ParticleSystem* ps = factory.make_entity<ParticleSystem>(getPosition(), getSize(), factory);
            GameObject::copy(ps);
            ps->mAffectors = mAffectors;
            ps->mEmitters = mEmitters;
            ps->mTexture = mTexture;
            ps->mTextureRects = mTextureRects;
            ps->mVertices = mVertices;
            return ps;
        }
        void ParticleSystem::setTexture(const graphic::Texture& texture)
        {
            mTexture = &texture;
            getFaces()[0].getMaterial().clearTextures();
            getFaces()[0].getMaterial().addTexture(mTexture);
            mNeedsQuadUpdate = true;
        }

        unsigned int ParticleSystem::addTextureRect(const sf::IntRect& textureRect)
        {
            mTextureRects.push_back(textureRect);
            mNeedsQuadUpdate = true;

            return mTextureRects.size() - 1;
        }

        void ParticleSystem::swap(ParticleSystem& other)
        {
            // Use ADL
            using std::swap;

            swap(mParticles,	other.mParticles);
            swap(mAffectors,	other.mAffectors);
            swap(mEmitters,	other.mEmitters);
            swap(mTexture,	other.mTexture);
            swap(mTextureRects,	other.mTextureRects);
            swap(mVertices,	other.mVertices);
            swap(mNeedsVertexUpdate,	other.mNeedsVertexUpdate);
            swap(mQuads,	other.mQuads);
            swap(mNeedsQuadUpdate,	other.mNeedsQuadUpdate);
        }

        void ParticleSystem::addAffector(std::function<void(Particle&, sf::Time)> affector)
        {
            addAffector(std::move(affector), sf::Time::Zero);
        }

        void ParticleSystem::addAffector(std::function<void(Particle&, sf::Time)> affector, sf::Time timeUntilRemoval)
        {
            mAffectors.push_back( Affector(std::move(affector), timeUntilRemoval) );
        }

        void ParticleSystem::clearAffectors()
        {
            mAffectors.clear();
        }

        void ParticleSystem::addEmitter(std::function<void(EmissionInterface&, sf::Time)> emitter)
        {
            addEmitter(emitter, sf::Time::Zero);
        }

        void ParticleSystem::addEmitter(std::function<void(EmissionInterface&, sf::Time)> emitter, sf::Time timeUntilRemoval)
        {
            mEmitters.push_back( Emitter(std::move(emitter), timeUntilRemoval) );
        }

        void ParticleSystem::clearEmitters()
        {
            mEmitters.clear();
        }

        void ParticleSystem::update(sf::Time dt)
        {
        // Invalidate stored vertices
            mNeedsVertexUpdate = true;

            // Emit new particles and remove expiring emitters
            for (EmitterContainer::iterator itr = mEmitters.begin(); itr != mEmitters.end(); )
            {
                itr->function(*this, dt);
                incrementCheckExpiry(mEmitters, itr, dt);
            }

            // Affect existing particles
            ParticleContainer::iterator writer = mParticles.begin();
            for (ParticleContainer::iterator reader = mParticles.begin(); reader != mParticles.end(); ++reader)
            {
                // Apply movement and decrease lifetime
                updateParticle(*reader, dt);

                // If current particle is not dead
                if (reader->passedLifetime < reader->totalLifetime)
                {
                    // Only apply affectors to living particles
                    AffectorContainer::iterator it;
                    for (it = mAffectors.begin(); it < mAffectors.end(); it++) {
                        it->function(*reader, dt);
                    }
                    // Go ahead
                    *writer++ = *reader;
                }
            }

            // Remove particles dying this frame
            mParticles.erase(writer, mParticles.end());

            // Remove affectors expiring this frame
            for (AffectorContainer::iterator itr = mAffectors.begin(); itr != mAffectors.end(); )
            {
                incrementCheckExpiry(mAffectors, itr, dt);
            }
        }

        void ParticleSystem::clearParticles()
        {
            mParticles.clear();
        }

        void ParticleSystem::draw(graphic::RenderTarget& target, graphic::RenderStates states)
        {
            // Check cached rectangles
            if (mNeedsQuadUpdate)
            {
                computeQuads();
                mNeedsQuadUpdate = false;
            }

            // Check cached vertices
            if (mNeedsVertexUpdate)
            {
                computeVertices();
                mNeedsVertexUpdate = false;
            }
            // Draw the vertex array with our texture
            states.texture = mTexture;
            target.draw(mVertices, states);
        }

        void ParticleSystem::emitParticle(const Particle& particle)
        {
            mParticles.push_back(particle);
        }

        void ParticleSystem::updateParticle(Particle& particle, sf::Time dt)
        {
            particle.passedLifetime += dt;
            particle.position += dt.asSeconds() * particle.velocity;
            particle.rotation += dt.asSeconds() * particle.rotationSpeed;
        }
        void ParticleSystem::update () {
            // Check cached rectangles
            if (mNeedsQuadUpdate)
            {
                computeQuads();
                mNeedsQuadUpdate = false;
            }

            // Check cached vertices
            if (mNeedsVertexUpdate)
            {
                computeVertices();
                mNeedsVertexUpdate = false;
            }
        }
        void ParticleSystem::computeVertices() const
        {

            // Clear vertex array (keeps memory allocated)
            /*if (scene != nullptr)
                scene->removeVertices(mVertices);*/
            mVertices.clear();
            mVertices.setEntity(const_cast<ParticleSystem*>(this));
            // Fill vertex array
            ParticleContainer::const_iterator it;
            graphic::TransformMatrix tm;
            for(it = mParticles.begin(); it < mParticles.end(); it++)
            {
                tm.setRotation(math::Vec3f(0, 0, 1), it->rotation);
                tm.setScale(math::Vec3f(it->scale.x, it->scale.y, it->scale.z));
                tm.setTranslation(math::Vec3f(it->position.x, it->position.y, it->position.z));
                // Ensure valid index -- if this fails, you have not called addTextureRect() enough times, or p.textureIndex is simply wrong
                assert(it->textureIndex == 0 || it->textureIndex < mTextureRects.size());

                const std::array<graphic::Vertex, 4>& quad = mQuads[it->textureIndex];
                for (unsigned int i = 0; i < 4; i++)
                {
                    graphic::Vertex vertex;
                    math::Vec3f pos = tm.transform(math::Vec3f(quad[i].position.x, quad[i].position.y,quad[i].position.z));
                    vertex.position.x = pos.x;
                    vertex.position.y = pos.y;
                    vertex.position.z = pos.z;
                    vertex.texCoords = quad[i].texCoords;
                    vertex.color = it->color;
                    mVertices.append(vertex);
                }
                const_cast<ParticleSystem*>(this)->getFace(0)->setVertexArray(mVertices);
                tm.reset3D();
            }
            /*if (scene != nullptr)
                scene->addVertices(mVertices, const_cast<ParticleSystem*>(this)->getTransform().getTransformId());*/
        }

        void ParticleSystem::computeQuads() const
        {
            // Ensure setTexture() has been called
            assert(mTexture);

            // No texture rects: Use full texture, cache single rectangle
            if (mTextureRects.empty())
            {
                mQuads.resize(1);
                computeQuad(mQuads[0], getFullRect(*mTexture));
            }

            // Specified texture rects: Cache every one
            else
            {
                mQuads.resize(mTextureRects.size());
                for (std::size_t i = 0; i < mTextureRects.size(); ++i)
                    computeQuad(mQuads[i], mTextureRects[i]);
            }
        }

        void ParticleSystem::computeQuad(Quad& quad, const sf::IntRect& textureRect) const
        {
            sf::FloatRect rect(textureRect);

            quad[0].texCoords = sf::Vector2f(rect.left, rect.top);
            quad[1].texCoords = sf::Vector2f(rect.left + rect.width, rect.top);
            quad[2].texCoords = sf::Vector2f(rect.left + rect.width, rect.top + rect.height);
            quad[3].texCoords = sf::Vector2f(rect.left, rect.top + rect.height);

            quad[0].position = sf::Vector3f(-rect.width, -rect.height, rect.height) / 2.f;
            quad[1].position = sf::Vector3f( rect.width, -rect.height, rect.height) / 2.f;
            quad[2].position = sf::Vector3f( rect.width, rect.height, -rect.height) / 2.f;
            quad[3].position = sf::Vector3f(-rect.width, rect.height, -rect.height) / 2.f;
        }
        void ParticleSystem::removeEmitter(std::function<void(EmissionInterface&, sf::Time)>& em) {
            std::vector<Emitter>::iterator it;
            for (it = mEmitters.begin(); it != mEmitters.end();) {
                if (&em == &it->function)
                    it = mEmitters.erase(it);
                else
                    it++;
            }
        }
    }

} // namespace thor
