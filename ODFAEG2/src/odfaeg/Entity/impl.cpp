module;
#include <optional>
#include <array>
#include <vector>
#include <entt.hpp>
#include <map>
import odfaeg.entity.impl;
module odfaeg.entity.impl;
namespace odfaeg {
	namespace entity {
        class  EntityFactory {
        public:

            EntityFactory() {
                nbEntities = 0;
                nbEntitiesTypes = 0;
            }
            int getIntOfType(std::string sType) {
                std::map<int, std::string>::iterator it;
                for (it = types.begin(); it != types.end(); ++it) {
                    if (it->second == sType) {
                        return it->first;
                    }
                }
                return -1;
            }
            std::pair<int, std::string> updateTypes(std::string sType) {
                int iType = getIntOfType(sType);
                /*if (sType == "E_WALL")
                    //////std::cout<<"i type : "<<iType<<std::endl;*/
                if (iType == -1) {
                    /*if (sType == "E_WALL")
                        //////std::cout<<"i type  : "<<nbEntitiesTypes<<std::endl;*/
                    std::pair<int, std::string> type = std::pair<int, std::string>(nbEntitiesTypes, sType);
                    types.insert(type);
                    nbEntitiesTypes++;
                    return type;
                }
                else {
                    std::map<int, std::string>::iterator it = types.find(iType);
                    /*if (it->second == "E_WALL")
                        //////std::cout<<"i type : "<<it->first<<std::endl;*/
                    return *it;
                }
            }
            std::string getTypeOfInt(int type) {
                std::map<int, std::string>::iterator it = types.find(type);
                return it != types.end() ? it->second : "";
            }
            static int getNbEntitiesTypes() {
                return nbEntitiesTypes;
            }
            static int getNbEntities() {
                return nbEntities;
            }
            void decreaseNbEntities() {
                nbEntities--;
            }
            template <typename D, typename... Args>
            D* make_entity(Args&&... args) {
                return new D(std::forward<Args>(args)...);
            }
            int getUniqueId() {
                nbEntities++;
                return nbEntities - 1;
            }
            entt::entity getEnttID() {
                return m_Registery.create();
            }
            entt::registry& getRegistery() {
                return m_Registery;
            }
            static std::vector<int> getEntitiesTypesInts() {
                std::vector<int> types;
                for (unsigned int i = 0; i < types.size(); i++) {
                    types.push_back(i);
                }
                return types;
            }
        private:
            EntityFactory(const EntityFactory& entity) = delete; /**> an entity if not copiable.*/
            EntityFactory& operator=(const EntityFactory& entity) = delete; /**> an entity is not affectable*/
            inline static unsigned int nbEntities = 0, nbEntitiesTypes = 0;
            std::map<int, std::string> types;
            entt::registry m_Registery;

        };
        struct Relation {
            std::vector<entt::entity> children;
            entt::entity parent = entt::null;
        };

        template <size_t I, typename... Components>
        concept NOTLASTELEM = requires {
            typename std::enable_if_t<(sizeof...(Components) != 0 && I < sizeof...(Components) - 1)>;
        };
        template <size_t I, typename... Components>
        concept LASTELEM = requires {
            typename std::enable_if_t<(sizeof...(Components) != 0 && I == sizeof...(Components) - 1)>;
        };
        template <size_t I, typename... Components>
        concept EMPTY = requires  {
            typename std::enable_if_t<(sizeof...(Components) == 0)>;
        };
        class ComponentMapping {
        private:
            EntityFactory factory;
        public:
            ComponentMapping() {

            }
            EntityFactory& getEntityFactory() {
                return factory;
            }
            template <typename T>
            void addComponent(entt::entity entity, T component) {
                if (!factory.getRegistery().all_of<T>(entity))
                    factory.getRegistery().emplace<T>(entity, component);
            }
            template <typename T>
            std::optional<std::reference_wrapper<T>> getComponent(entt::entity entity) {
                std::optional<std::reference_wrapper<T>> component;
                if (!factory.getRegistery().all_of<T>(entity))
                    return component;
                return std::ref(factory.getRegistery().get<T>(entity));
            }
            template <typename T>
            void removeComponent(entt::entity entity) {
                if (factory.getRegistery().all_of<T>(entity))
                    factory.getRegistery().remove<T>(entity);
            }
            void addChild(entt::entity parentId, entt::entity child) {

                if (!getComponent<Relation>(parentId).has_value()) {
                    //////std::cout<<"add relation add child"<<std::endl;
                    Relation relation;
                    relation.children.push_back(child);
                    addComponent(parentId, relation);
                }
                else {
                    //////std::cout<<"add child"<<std::endl;
                    getComponent<Relation>(parentId).value().get().children.push_back(child);
                }
                if (!getComponent<Relation>(child).has_value()) {
                    //////std::cout<<"add relation set parent"<<std::endl;
                    Relation relation;
                    relation.parent = parentId;
                    addComponent(child, relation);
                }
                else {
                    //////std::cout<<"set parent"<<std::endl;
                    getComponent<Relation>(child).value().get().parent = child;
                }

            }
            entt::entity getRoot(entt::entity entityId) {
                if (getParent(entityId) == entt::null)
                    return entityId;
                return getRoot(getParent(entityId));
            }
            std::vector<entt::entity> getChildren(entt::entity entity) {
                std::vector<entt::entity> children;
                if (getComponent<Relation>(entity).has_value()) {
                    children = getComponent<Relation>(entity).value().get().children;
                }
                return children;
            }
            entt::entity getParent(entt::entity entity) {
                entt::entity parent = entt::null;
                if (getComponent<Relation>(entity).has_value()) {
                    parent = getComponent<Relation>(entity).value().get().parent;
                }
                return parent;
            }
            template <typename Archive, typename... Components>
            void readEntities(Archive& ar, std::vector<entt::entity>& entities) {
                size_t nbEntities;
                ar(nbEntities);
                for (unsigned int i = 0; i < nbEntities; i++) {
                    entt::entity entity = factory.getEnttID();
                    readEntity<Archive, Components...>(ar, entity);
                    entities.push_back(entity);
                }
            }
            template <typename Archive, typename... Components>
            void readEntity(Archive& ar, entt::entity& entity) {
                readComponents<0, Archive, Components...>(ar, entity);
                size_t nbChildren;
                ar(nbChildren);
                for (unsigned int i = 0; i < nbChildren; i++) {
                    entt::entity child = factory.getEnttID();
                    readEntity<Archive, Components...>(ar, child);
                    addChild(entity, child);
                }
            }

            template <size_t I, typename Archive, typename... Components> requires NOTLASTELEM<I, Components...>
            void readComponents(Archive& ar, entt::entity entity) {
                std::tuple_element_t<I, std::tuple<Components...>>* component;
                //////std::cout<<"read component"<<std::endl;
                ar(component);
                //////std::cout<<"component : "<<component<<std::endl;
                if (component != nullptr) {
                    addComponent(entity, *component);
                }
                readComponents<I + 1, Archive, Components...>(ar, entity);
            }
            template <size_t I, typename Archive, typename... Components> requires LASTELEM<I, Components...>
            void readComponents(Archive& ar, entt::entity entity) {
                std::tuple_element_t<I, std::tuple<Components...>>* component;
                //////std::cout<<"read component"<<std::endl;
                ar(component);
                //////std::cout<<"component : "<<component<<std::endl;
                if (component != nullptr) {
                    addComponent(entity, *component);
                }
            }
            template <size_t I, typename Archive, typename... Components> requires EMPTY<I, Components...>
            void readComponents(Archive& ar, entt::entity entity) {

            }
            template <typename Archive, typename... Components>
            void writeEntities(Archive& ar, std::vector<entt::entity> entities) {
                unsigned int size = entities.size();
                ar(size);
                for (unsigned int i = 0; i < entities.size(); i++) {
                    writeEntity<Archive, Components...>(ar, entities[i]);
                }
            }
            template <typename Archive, typename... Components>
            void writeEntity(Archive& ar, entt::entity entity) {
                writeComponents<0, Archive, Components...>(ar, entity);
                unsigned int size = getChildren(entity).size();
                ar(size);
                for (unsigned int i = 0; i < getChildren(entity).size(); i++) {
                    writeEntity<Archive, Components...>(ar, getChildren(entity)[i]);
                }
            }
            template <size_t I, typename Archive, typename... Components> requires NOTLASTELEM<I, Components...>
            void writeComponents(Archive& ar, entt::entity entity) {
                if (getComponent<std::tuple_element_t<I, std::tuple<Components...>>>(entity).has_value())
                    ar(getComponent<std::tuple_element_t<I, std::tuple<Components...>>>(entity));
                else
                    ar(nullptr);
                writeComponents<I + 1, Archive, Components...>(ar, entity);
            }
            template <size_t I, typename Archive, typename... Components> requires LASTELEM<I, Components...>
            void writeComponents(Archive& ar, entt::entity entity) {
                if (getComponent<std::tuple_element_t<I, std::tuple<Components...>>>(entity).has_value())
                    ar(getComponent<std::tuple_element_t<I, std::tuple<Components...>>>(entity));
                else
                    ar(nullptr);
            }

            template <size_t I, typename Archive, typename... Components> requires EMPTY<I, Components...>
            void writeComponents(Archive& ar, entt::entity entity) {

            }
            template <typename... Components>
            void remove(entt::entity entity) {
                if (getComponent<Relation>(entity)) {
                    entt::entity parent = getComponent<Relation>(entity).value().get().parent;
                    if (getComponent<Relation>(parent).has_value()) {
                        std::vector<entt::entity>& children = getComponent<Relation>(parent).value().get().children;
                        std::vector<entt::entity>::iterator it;
                        for (it = children.begin(); it != children.end();) {
                            if (*it == entity) {
                                it = children.erase(it);
                            }
                            else {
                                it++;
                            }
                        }
                    }
                    removeComponents<0, Components...>(entity);
                    entity = entt::null;
                }
            }
            template <size_t I, typename... Components> requires NOTLASTELEM<I, Components...>
            void removeComponents(entt::entity entity) {
                removeComponent<std::tuple_element_t<I, std::tuple<Components...>>>(entity);
                removeComponents<I + 1, Components...>(entity);
            }
            template <size_t I, typename... Components> requires LASTELEM<I, Components...>
            void removeComponents(entt::entity entity) {
                removeComponent<std::tuple_element_t<I, std::tuple<Components...>>>(entity);
            }
            template <size_t I, typename... Components> requires EMPTY<I, Components...>
            void removeComponents(entt::entity entity) {
            }
            template <typename... Signature>
            entt::entity clone(entt::entity toClone, entt::entity parent = entt::null) {
                entt::entity clonedId = factory.getEnttID();
                clone_impl<0, Signature...>(toClone, clonedId);
                if (parent != entt::null) {
                    addChild(parent, clonedId);
                }
                for (unsigned int i = 0; i < getChildren(toClone).size(); i++) {
                    clone<Signature...>(getChildren(toClone)[i], clonedId);
                }
                return clonedId;
            }
            template <size_t I, typename... Components> requires NOTLASTELEM<I, Components...>
            void clone_impl(entt::entity toCloneId, entt::entity clonedId) {
                //si l'entit� poss�de le composant en question on le cl�ne.
                if (getComponent<std::tuple_element_t<I, std::tuple<Components...>>>(toCloneId).has_value()) {
                    addComponent(clonedId, getComponent<std::tuple_element_t<I, std::tuple<Components...>>>(toCloneId).value().get());

                }
                clone_impl<I + 1, Components...>(toCloneId, clonedId);
            }
            template <size_t I, typename... Components> requires LASTELEM<I, Components...>
            void clone_impl(entt::entity toCloneId, entt::entity clonedId) {

                if (getComponent<std::tuple_element_t<I, std::tuple<Components...>>>(toCloneId).has_value()) {
                    addComponent(clonedId, getComponent<std::tuple_element_t<I, std::tuple<Components...>>>(toCloneId).value().get());

                }
            }
            template <size_t I, typename... Components> requires EMPTY<I, Components...>
            void clone_impl(entt::entity tocloneId, entt::entity clonedId) {
            }
            template <typename... Signature>
            entt::entity merge(entt::entity toMerge1, entt::entity toMerge2, entt::entity parent = entt::null) {
                entt::entity merged = factory.getEnttID();
                entt::entity merged1, merged2;
                if (parent == entt::null) {
                    merged1 = factory.getEnttID();
                    merged2 = factory.getEnttID();
                    merge_node<0, Signature...>(merged, toMerge1);
                    merge_node<0, Signature...>(merged, toMerge2);
                    addChild(merged, merged1);
                    addChild(merged, merged2);
                }
                if (parent != entt::null && toMerge1 != entt::null) {
                    addChild(parent, merged);
                    merge_node<0, Signature...>(merged, toMerge1);
                }
                if (parent != entt::null && toMerge2 != entt::null) {
                    addChild(parent, merged);
                    merge_node<0, Signature...>(merged, toMerge2);
                }
                for (unsigned int i = 0; i < getChildren(toMerge1).size(); i++) {
                    merge<Signature...>(factory, getChildren(toMerge1)[i], (parent == entt::null) ? merged1 : merged);
                }
                for (unsigned int i = 0; i < getChildren(toMerge2).size(); i++) {
                    merge<Signature...>(factory, entt::null, getChildren(toMerge2)[i], (parent == entt::null) ? merged2 : merged);
                }
                return merged;
            }
            template <size_t I, typename... Signature> requires NOTLASTELEM<I, Signature...>
            void merge_node(entt::entity entityId, entt::entity toMergeId) {
                //si l'entit� poss�de le composant en question on le cl�ne.
                if (getComponent<std::tuple_element_t<I, std::tuple<Signature...>>>(toMergeId).has_value()) {
                    addComponent(entityId, getComponent<std::tuple_element_t<I, std::tuple<Signature...>>>(toMergeId).value().get());
                }
                merge_node<I + 1, Signature...>(entityId, toMergeId);
            }
            template < size_t I, typename... Signature> requires LASTELEM<I, Signature...>
            void merge_node(entt::entity entityId, entt::entity toMergeId) {
                if (getComponent<std::tuple_element_t<I, std::tuple<Signature...>>>(toMergeId).has_value()) {
                    addComponent(entityId, getComponent<std::tuple_element_t<I, std::tuple<Signature...>>>(toMergeId).value().get());
                }
            }
            template <size_t I, typename... Signature> requires EMPTY<I, Signature...>
            void merge_node(entt::entity tocloneId, entt::entity clonedId) {
            }
            template <typename... Signature, typename System, typename... Params>
            void apply(System& system, std::vector<entt::entity>& entities, std::tuple<Params...>& params) {
                for (unsigned int i = 0; i < entities.size(); i++) {
                    apply<Signature...>(system, entities[i], params);
                }
            }
            template <typename... Signature, typename System, typename... Params>
            void apply(System& system, entt::entity entity, std::tuple<Params...>& params) {
                this->template apply_impl<Signature...>(entity, system, params);
                for (unsigned int i = 0; i < getChildren(entity).size(); i++) {
                    apply<Signature...>(system, getChildren(entity)[i], params);
                }
            }
            template <typename... Signature, typename System, typename... Params>
            void apply_impl(entt::entity entityId, System& system, std::tuple<Params...>& params) {
                system.template operator() < Signature... > (entityId, *this, params);
            }
            template <typename... Signature, typename System, typename... Params, typename R>
            void apply(System& system, std::vector<entt::entity>& entities, std::tuple<Params...>& params, std::vector<R>& ret) {

                for (unsigned int i = 0; i < entities.size(); i++) {
                    apply<Signature...>(system, entities[i], params, ret);
                }
            }
            template <typename... Signature, typename System, typename... Params, typename R>
            void apply(System& system, entt::entity entity, std::tuple<Params...>& params, std::vector<R>& ret) {
                this->template apply_impl<Signature...>(entity, system, params, ret);
                for (unsigned int i = 0; i < getChildren(entity).size(); i++) {
                    apply<Signature...>(system, getChildren(entity)[i], params, ret);
                }
            }
            template <typename... Signature, typename System, typename... Params, typename R>
            void apply_impl(entt::entity entityId, System& system, std::tuple<Params...>& params, std::vector<R>& rets) {

                rets.push_back(system.template operator() < Signature... > (entityId, *this, params));
            }
            static int getNbEntities() {
                return EntityFactory::getNbEntities();
            }
            static int getNbEntitiesTypes() {
                return EntityFactory::getNbEntitiesTypes();
            }
        };
	    ComponentMapping& getComponentMapping() {
	        static ComponentMapping componentMapping;
	        return componentMapping;
	    }
	    /*template <typename D, typename... Args>
	    D* EnttEntity::make_entity(Args... args) {
	        return getComponentMapping().getEntityFactory().make_entity<D>(args...);
	    }*/
        void EnttEntity::initEntity(Entity& entity) {
            ComponentMapping& componentMapping = getComponentMapping();
            entity.setTypes(componentMapping.getEntityFactory().updateTypes(entity.getType()));
            entity.setId(componentMapping.getEntityFactory().getUniqueId());
            entity.setEnttID((uint32_t)componentMapping.getEntityFactory().getEnttID());
	        Entity::setNbEntitiesTypes(componentMapping.getEntityFactory().getNbEntitiesTypes());
        }
	}
}



	