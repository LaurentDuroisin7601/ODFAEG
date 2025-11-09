
#ifndef ODFAEG_PARTICLES_UPDATER_HPP
#define ODFAEG_PARTICLES_UPDATER_HPP
#include "particuleSystem.h"
#include "world.h"
#include "export.hpp"
//#include "application.h"
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
        class ODFAEG_GRAPHICS_API ParticleSystemUpdater : public core::Timer {
        public :
            ParticleSystemUpdater(bool useThread = true);
            /**
            * \fn void onUpdate ()
            * \brief update all the entities which are in the current view.
            */
            void addParticleSystem(Entity* ps);
            void removeParticleSystem (Entity* ps);
            std::vector<Entity*> getParticleSystems();
            void onUpdate ();
        private :
            std::vector<Entity*> particleSystems;
            core::Clock timeClock;
        };
    }
}
#endif
