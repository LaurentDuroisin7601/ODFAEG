#include "enttImpl.hpp"
namespace odfaeg {
	namespace entity {
       
	    ComponentMapping& getComponentMapping() {
	        static ComponentMapping componentMapping;
	        return componentMapping;
	    }
	    
        void EnttEntity::initEntity(Entity& entity) {
            ComponentMapping& componentMapping = getComponentMapping();
            entity.setTypes(componentMapping.getEntityFactory().updateTypes(entity.getType()));
            entity.setId(componentMapping.getEntityFactory().getUniqueId());
            entity.setEnttID((uint32_t)componentMapping.getEntityFactory().getEnttID());
	        Entity::setNbEntitiesTypes(componentMapping.getEntityFactory().getNbEntitiesTypes());
        }
	}
}



	