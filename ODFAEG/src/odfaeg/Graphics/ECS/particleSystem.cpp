
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

#include "../../../../include/odfaeg/Graphics/ECS/particleSystem.hpp"


#include "../../../../include/odfaeg/Graphics/renderWindow.h"
#include "../../../../include/odfaeg/Graphics/texture.h"
#include "../../../../include/odfaeg/Math/transformMatrix.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cassert>


namespace odfaeg
{
    namespace physic {
            namespace ecs
            {
                namespace {

                    // Erases emitter/affector at itr from ctr, if its time has expired. itr will point to the next element.
                    template <class Container>
                    void incrementCheckExpiry(Container& ctr, typename Container::iterator& itr, core::Time dt)
                    {
                        // itr->second is the remaining time of the emitter/affector.
                        // Time::zero means infinite time (no removal).
                        if (itr->timeUntilRemoval != core::Time::zero && (itr->timeUntilRemoval -= dt) <= core::Time::zero)
                        itr = ctr.erase(itr);
                        else
                        ++itr;
                    }

                    graphic::IntRect getFullRect(const graphic::Texture& texture)
                    {
                        return graphic::IntRect(0, 0, texture.getSize().x(), texture.getSize().y());
                    }

                } // namespace

        // ---------------------------------------------------------------------------------------------------------------------------


            ParticleSystem::ParticleSystem(math::Vec3f position, math::Vec3f size, graphic::ecs::ComponentMapping& cmapping)
            : mParticles()
            , mAffectors()
            , mEmitters()
            , mNeedsVertexUpdate(true)
            , mQuads()
            , mNeedsQuadUpdate(true)
            , cmapping(cmapping)
            {
                entity = cmapping.getEntityFactory().createEntity("E_PARTICLES");
                graphic::ecs::EntityInfoComponent eic;
                eic.id = cmapping.getEntityFactory().getNbEntities() - 1;
                eic.groupName = "E_PARTICLES";
                graphic::TransformMatrix tm;
                graphic::ecs::TransformComponent tc;
                tc.localBounds = physic::BoundingBox(0, 0, 0, size.x(), size.y(), size.z());
                tc.position = position;
                tc.origin = tc.size * 0.5f;
                tc.center = tc.position + tc.origin;
                tm.setOrigin(tc.origin);
                tm.setRotation(math::Vec3f::zAxis, 0);
                tm.setTranslation(tc.center);
                math::Vec3f scale(1.f, 1.f, 1.f);
                tm.setScale(scale);
                tc.globalBounds = tc.localBounds.transform(tm);
                tc.transformMatrix = tm;
                graphic::Material material;
                material.addTexture(nullptr);
                graphic::VertexArray va(graphic::Quads);
                va.setEntityId(entity);
                graphic::Face face (va, material, tm);
                graphic::ecs::MeshComponent mc;
                mc.faces.push_back(face);
                cmapping.addComponent(entity, eic);
                cmapping.addComponent(entity, tc);
                cmapping.addComponent(entity, mc);
            }
            void ParticleSystem::setTexture(const graphic::Texture& texture)
            {
                cmapping.getComponent<graphic::ecs::MeshComponent>(entity)->faces[0].getMaterial().clearTextures();
                cmapping.getComponent<graphic::ecs::MeshComponent>(entity)->faces[0].getMaterial().addTexture(&texture);
                mNeedsQuadUpdate = true;
            }

            unsigned int ParticleSystem::addTextureRect(const graphic::IntRect& textureRect)
            {
                cmapping.getComponent<graphic::ecs::MeshComponent>(entity)->faces[0].getMaterial().addTexture(nullptr, textureRect);

                return cmapping.getComponent<graphic::ecs::MeshComponent>(entity)->faces[0].getMaterial().getNbTextures() - 2;
            }

            void ParticleSystem::swap(ParticleSystem& other)
            {
                // Use ADL
                using std::swap;

                swap(mParticles,	other.mParticles);
                swap(mAffectors,	other.mAffectors);
                swap(mEmitters,	other.mEmitters);
                swap(entity,	other.entity);
                swap(mNeedsVertexUpdate,	other.mNeedsVertexUpdate);
                swap(mQuads,	other.mQuads);
                swap(mNeedsQuadUpdate,	other.mNeedsQuadUpdate);
            }

            void ParticleSystem::addAffector(std::function<void(Particle&, core::Time)> affector)
            {
                addAffector(std::move(affector), core::Time::zero);
            }

            void ParticleSystem::addAffector(std::function<void(Particle&, core::Time)> affector, core::Time timeUntilRemoval)
            {
                mAffectors.push_back( Affector(std::move(affector), timeUntilRemoval) );
            }

            void ParticleSystem::clearAffectors()
            {
                mAffectors.clear();
            }

            void ParticleSystem::addEmitter(std::function<void(EmissionInterface&, core::Time)> emitter)
            {
                addEmitter(emitter, core::Time::zero);
            }

            void ParticleSystem::addEmitter(std::function<void(EmissionInterface&, core::Time)> emitter, core::Time timeUntilRemoval)
            {
                mEmitters.push_back( Emitter(std::move(emitter), timeUntilRemoval) );
            }

            void ParticleSystem::clearEmitters()
            {
                mEmitters.clear();
            }

            void ParticleSystem::update(core::Time dt)
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
                states.texture = cmapping.getComponent<graphic::ecs::MeshComponent>(entity)->faces[0].getMaterial().getTexture();
                target.draw(cmapping.getComponent<graphic::ecs::MeshComponent>(entity)->faces[0].getVertexArray(), states);
            }

            void ParticleSystem::emitParticle(const Particle& particle)
            {
                mParticles.push_back(particle);
            }

            void ParticleSystem::updateParticle(Particle& particle, core::Time dt)
            {
                particle.passedLifetime += dt;
                particle.position += particle.velocity * dt.asSeconds();
                particle.rotation += particle.rotationSpeed * dt.asSeconds();
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
                graphic::VertexArray& va = cmapping.getComponent<graphic::ecs::MeshComponent>(entity)->faces[0].getVertexArray();
                va.clear();
                // Fill vertex array
                ParticleContainer::const_iterator it;
                graphic::TransformMatrix tm;
                for(it = mParticles.begin(); it < mParticles.end(); it++)
                {
                    tm.setRotation(math::Vec3f(0, 0, 1), it->rotation);
                    tm.setScale(math::Vec3f(it->scale.x(), it->scale.y(), it->scale.z()));
                    tm.setTranslation(math::Vec3f(it->position.x(), it->position.y(), it->position.z()));
                    // Ensure valid index -- if this fails, you have not called addTextureRect() enough times, or p.textureIndex is simply wrong
                    assert(it->textureIndex == 0 || it->textureIndex < cmapping.getComponent<graphic::ecs::MeshComponent>(entity)->faces[0].getMaterial().getNbTextures()-1);

                    const std::array<graphic::Vertex, 4>& quad = mQuads[it->textureIndex];
                    for (unsigned int i = 0; i < 4; i++)
                    {
                        graphic::Vertex vertex;
                        math::Vec3f pos = tm.transform(math::Vec3f(quad[i].position.x(), quad[i].position.y(),quad[i].position.z()));
                        vertex.position[0] = pos.x();
                        vertex.position[1] = pos.y();
                        vertex.position[2] = pos.z();
                        vertex.texCoords = quad[i].texCoords;
                        vertex.color = it->color;
                        va.append(vertex);
                    }
                    tm.reset3D();
                }
                /*if (scene != nullptr)
                    scene->addVertices(mVertices, const_cast<ParticleSystem*>(this)->getTransform().getTransformId());*/
            }

            void ParticleSystem::computeQuads() const
            {
                // Ensure setTexture() has been called
                assert(cmapping.getComponent<graphic::ecs::MeshComponent>(entity)->faces[0].getMaterial().getTexture());

                // No texture rects: Use full texture, cache single rectangle
                if (cmapping.getComponent<graphic::ecs::MeshComponent>(entity)->faces[0].getMaterial().getNbTextures() < 2)
                {
                    mQuads.resize(1);
                    computeQuad(mQuads[0], getFullRect(*cmapping.getComponent<graphic::ecs::MeshComponent>(entity)->faces[0].getMaterial().getTexture()));
                }

                // Specified texture rects: Cache every one
                else
                {
                    mQuads.resize(cmapping.getComponent<graphic::ecs::MeshComponent>(entity)->faces[0].getMaterial().getNbTextures()-1);
                    for (std::size_t i = 0; i < cmapping.getComponent<graphic::ecs::MeshComponent>(entity)->faces[0].getMaterial().getNbTextures()-1; ++i)
                        computeQuad(mQuads[i], cmapping.getComponent<graphic::ecs::MeshComponent>(entity)->faces[0].getMaterial().getTexRect(i+1));
                }
            }

            void ParticleSystem::computeQuad(Quad& quad, const graphic::IntRect& textureRect) const
            {
                graphic::FloatRect rect(textureRect);

                quad[0].texCoords = math::Vec2f(rect.left, rect.top);
                quad[1].texCoords = math::Vec2f(rect.left + rect.width, rect.top);
                quad[2].texCoords = math::Vec2f(rect.left + rect.width, rect.top + rect.height);
                quad[3].texCoords = math::Vec2f(rect.left, rect.top + rect.height);

                quad[0].position = math::Vec3f(-rect.width, -rect.height, rect.height) / 2.f;
                quad[1].position = math::Vec3f( rect.width, -rect.height, rect.height) / 2.f;
                quad[2].position = math::Vec3f( rect.width, rect.height, -rect.height) / 2.f;
                quad[3].position = math::Vec3f(-rect.width, rect.height, -rect.height) / 2.f;
            }
            void ParticleSystem::removeEmitter(std::function<void(EmissionInterface&, core::Time)>& em) {
                std::vector<Emitter>::iterator it;
                for (it = mEmitters.begin(); it != mEmitters.end();) {
                    if (&em == &it->function)
                        it = mEmitters.erase(it);
                    else
                        it++;
                }
            }
            void ParticleSystem::onUpdate() {
                update();
                update(clock.getElapsedTime());
                clock.restart();
            }
            graphic::ecs::EntityId ParticleSystem::getEntity() {
                return entity;
            }
        }
    }

} // namespace thor
