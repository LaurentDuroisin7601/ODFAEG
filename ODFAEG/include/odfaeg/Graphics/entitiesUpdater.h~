#ifndef ODFAEG_ENTITIES_UPDATER_HPP
#define ODFAEG_ENTITIES_UPDATER_HPP
#include "entitySystem.h"
#include "world.h"
#include "export.hpp"
/**
*\namespace odfaeg
* the namespace of the Opensource Development Framework Adapted for Every Games.
*/

namespace odfaeg {
/**
* \file entitiesUpdater.h
* \class EntitiesUpdater
* \brief update all the entities in the world which are in the current view with a thread.
* \author Duroisin.L
* \version 1.0
* \date 1/02/2014
*/
class ODFAEG_CORE_API EntitiesUpdater : public EntitySystem {
public :
    EntitiesUpdater(bool usingThread) : EntitySystem(usingThread) {}
    /**
    * \fn void onUpdate ()
    * \brief update all the entities which are in the current view.
    */
    void onUpdate () {
        World::checkVisibleEntities();
    }
private :

};
}

#endif // ENTITY_UPDATER
