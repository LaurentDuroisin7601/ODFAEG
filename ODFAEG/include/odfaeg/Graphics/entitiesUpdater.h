#ifndef ODFAEG_ENTITIES_UPDATER_HPP
#define ODFAEG_ENTITIES_UPDATER_HPP
#include "../Core/entitySystem.h"
//#include "../Graphics/application.h"
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
        class ODFAEG_GRAPHICS_API EntitiesUpdater : public core::EntitySystem {
        public :
            EntitiesUpdater(EntityFactory& factory, World& world, bool useThread = true) : world(world), factory(factory), EntitySystem(useThread) {}
            /**
            * \fn void onUpdate ()
            * \brief update all the entities which are in the current view.
            */
            void onUpdate () {
                //////std::cout<<"update world"<<std::endl;
                world.checkVisibleEntities(factory);
                //////std::cout<<"world updated"<<std::endl;
            }
        private :
            World& world;
            EntityFactory& factory;
        };
    }
}

#endif // ENTITY_UPDATER
