#include "enttImpl.hpp"
namespace odfaeg {
	namespace entity {       
	    ComponentMapping& EnttEntity::getComponentMapping() {
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
		template<typename T>
		void EnttEntity::addComponent(Entity& entity, T component) {
			getComponentMapping().addComponent((entt::entity) entity.getEnttID(), component);
		}
		template<typename T>
		std::optional<std::reference_wrapper<T>> EnttEntity::getComponent(Entity& entity) {
			return getComponentMapping().getComponent<T>((entt::entity) entity.getEnttID());
		}
		template<typename T>
		void EnttEntity::removeComponent(Entity& entity) {
			getComponentMapping().removeComponent<T>((entt::entity) entity.getEnttID());
		}
		void EnttEntity::addChild(Entity& parent, Entity& child) {
			getComponentMapping().addChild((entt::entity) parent.getEnttID(), (entt::entity) child.getEnttID());
		}		
	}
}



	