
#ifndef ODFAEG_PARTICLES_UPDATER_HPP
#define ODFAEG_PARTICLES_UPDATER_HPP
#include "../Physics/particuleSystem.h"
#include "../Graphics/world.h"
#include "export.hpp"
/**
*\namespace odfaeg
* the namespace of the Opensource Development Framework Adapted for Every Games.
*/
namespace odfaeg {
    namespace graphic {
        /**
        * \file entitiesUpdater.h
        * \class EntitiesUpdater
        * \brief update all the entities in the world which are in the current view with a thread.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        class ODFAEG_CORE_API ParticleSystemUpdater : public core::EntitySystem {
        public :
            ParticleSystemUpdater() : EntitySystem() {}
            /**
            * \fn void onUpdate ()
            * \brief update all the entities which are in the current view.
            */
            void addParticleSystem(Entity* ps) {
                particleSystems.push_back(ps);
            }
            void removeParticleSystem (Entity* ps) {
                std::vector<Entity*>::iterator it;
                for (it = particleSystems.begin(); it != particleSystems.end();) {
                    if (*it == ps) {
                        it = particleSystems.erase(it);
                    } else {
                        it++;
                    }
                }
            }
            std::vector<Entity*> getParticleSystems() {
                return particleSystems;
            }
            void onUpdate () {
                for (unsigned int i = 0; i < particleSystems.size(); i++) {
                    //std::cout<<"update particle"<<std::endl;
                    particleSystems[i]->update();
                    if (odfaeg::core::Application::app != nullptr)
                        particleSystems[i]->update(odfaeg::core::Application::app->getClock("LoopTime").getElapsedTime());
                }
                //World::changeVisibleEntity(nullptr, nullptr);
            }
        private :
            std::vector<Entity*> particleSystems;
        };
    }
}
#endif
