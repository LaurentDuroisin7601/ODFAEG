#ifndef ODFAEG_ECS_HPP
#define ODFAEG_ECS_HPP
#include "components.hpp"
#include "entityFactory.hpp"

namespace odfaeg {
    namespace graphic {
        template <size_t I, typename... Components>
        concept NOTLASTELEM = requires {
            typename std::enable_if_t<(sizeof...(Components) != 0 && I < sizeof...(Components)-1)>;
        };
        template <size_t I, typename... Components>
        concept LASTELEM = requires {
            typename std::enable_if_t<(sizeof...(Components) != 0 && I == sizeof...(Components)-1)>;
        };
        template <size_t I, typename... Components>
        concept EMPTY = requires  {
            typename std::enable_if_t<(sizeof...(Components) == 0)>;
        };


        class ComponentMapping {

            EntityFactory& factory;
            public :
            ComponentMapping(EntityFactory& factory) : factory(factory) {

            }
            EntityFactory& getEntityFactory() {
                return factory;
            }
            template <typename T>
            void addComponent(EntityId entity, T component) {
                if (!factory.getRegistery().all_of<T>(entity))
                    factory.getRegistery().emplace<T>(entity, component);
            }
            template <typename T>
            std::optional<std::reference_wrapper<T>> getComponent(EntityId entity) {
                std::optional<std::reference_wrapper<T>> component;
                if (!factory.getRegistery().all_of<T>(entity))
                    return component;
                return std::ref(factory.getRegistery().get<T>(entity));
            }
            template <typename T>
            void removeComponent(EntityId entity) {
                if (factory.getRegistery().all_of<T>(entity))
                    factory.getRegistery().remove<T>(entity);
            }
            void addChild(EntityId parentId, EntityId child) {

                if (!getComponent<Relation>(parentId).has_value()) {
                    //////std::cout<<"add relation add child"<<std::endl;
                    Relation relation;
                    relation.children.push_back(child);
                    addComponent(parentId, relation);
                } else {
                    //////std::cout<<"add child"<<std::endl;
                    getComponent<Relation>(parentId).value().get().children.push_back(child);
                }
                if (!getComponent<Relation>(child).has_value()) {
                    //////std::cout<<"add relation set parent"<<std::endl;
                    Relation relation;
                    relation.parent = parentId;
                    addComponent(child, relation);
                } else {
                    //////std::cout<<"set parent"<<std::endl;
                    getComponent<Relation>(child).value().get().parent = child;
                }

            }
            EntityId getRoot(EntityId entityId) {
                if (getParent(entityId) == entt::null)
                    return entityId;
                return getRoot(getParent(entityId));
            }
            std::vector<EntityId> getChildren(EntityId entity) {
                std::vector<EntityId> children;
                if (getComponent<Relation>(entity).has_value()) {
                   children =  getComponent<Relation>(entity).value().get().children;
                }
                return children;
            }
            EntityId getParent(EntityId entity) {
                EntityId parent = entt::null;
                if(getComponent<Relation>(entity).has_value()) {
                   parent =  getComponent<Relation>(entity).value().get().parent;
                }
                return parent;
            }
            template <typename Archive, typename... Components>
            void readEntities(Archive& ar, std::vector<EntityId>& entities) {
                size_t nbEntities;
                ar(nbEntities);
                for (unsigned int i = 0; i < nbEntities; i++) {
                    EntityId entity = factory.getEnttID();
                    readEntity<Archive, Components...>(ar, entity);
                    entities.push_back(entity);
                }
            }
            template <typename Archive, typename... Components>
            void readEntity(Archive& ar, EntityId& entity) {
                readComponents<0, Archive, Components...>(ar, entity);
                size_t nbChildren;
                ar(nbChildren);
                for (unsigned int i = 0; i < nbChildren; i++) {
                    EntityId child = factory.getEnttID();
                    readEntity<Archive, Components...>(ar, child);
                    addChild(entity, child);
                }
            }

            template <size_t I, typename Archive, typename... Components> requires NOTLASTELEM<I, Components...>
            void readComponents(Archive& ar, EntityId entity) {
                std::tuple_element_t<I, std::tuple<Components...>>* component;
                //////std::cout<<"read component"<<std::endl;
                ar(component);
                //////std::cout<<"component : "<<component<<std::endl;
                if (component != nullptr) {
                    addComponent(entity, *component);
                }
                readComponents<I+1, Archive, Components...>(ar, entity);
            }
            template <size_t I, typename Archive, typename... Components> requires LASTELEM<I, Components...>
            void readComponents(Archive& ar, EntityId entity) {
                std::tuple_element_t<I, std::tuple<Components...>>* component;
                //////std::cout<<"read component"<<std::endl;
                ar(component);
                //////std::cout<<"component : "<<component<<std::endl;
                if (component != nullptr) {
                    addComponent(entity, *component);
                }
            }
            template <size_t I, typename Archive, typename... Components> requires EMPTY<I, Components...>
            void readComponents(Archive& ar, EntityId entity) {

            }
            template <typename Archive, typename... Components>
            void writeEntities(Archive& ar, std::vector<EntityId> entities) {
                unsigned int size = entities.size();
                ar(size);
                for (unsigned int i = 0; i < entities.size(); i++) {
                    writeEntity<Archive, Components...>(ar, entities[i]);
                }
            }
            template <typename Archive, typename... Components>
            void writeEntity(Archive& ar, EntityId entity) {
                writeComponents<0, Archive, Components...>(ar, entity);
                unsigned int size = getChildren(entity).size();
                ar(size);
                for (unsigned int i = 0; i < getChildren(entity).size(); i++) {
                    writeEntity<Archive, Components...>(ar, getChildren(entity)[i]);
                }
            }
            template <size_t I, typename Archive, typename... Components> requires NOTLASTELEM<I, Components...>
            void writeComponents(Archive& ar, EntityId entity) {
                if (getComponent<std::tuple_element_t<I, std::tuple<Components...>>>(entity).has_value())
                    ar(getComponent<std::tuple_element_t<I, std::tuple<Components...>>>(entity));
                else
                    ar(nullptr);
                writeComponents<I+1, Archive, Components...>(ar, entity);
            }
            template <size_t I, typename Archive, typename... Components> requires LASTELEM<I, Components...>
            void writeComponents(Archive& ar, EntityId entity) {
                if(getComponent<std::tuple_element_t<I, std::tuple<Components...>>>(entity).has_value())
                    ar(getComponent<std::tuple_element_t<I, std::tuple<Components...>>>(entity));
                else
                    ar(nullptr);
            }

            template <size_t I, typename Archive, typename... Components> requires EMPTY<I, Components...>
            void writeComponents(Archive& ar, EntityId entity) {

            }
            template <typename... Components>
            void remove(EntityId entity) {
                if (getComponent<Relation>(entity)) {
                    EntityId parent = getComponent<Relation>(entity).value().get().parent;
                    if (getComponent<Relation>(parent).has_value()) {
                        std::vector<EntityId>& children = getComponent<Relation>(parent).value().get().children;
                        std::vector<EntityId>::iterator it;
                        for (it = children.begin(); it != children.end();) {
                            if (*it == entity) {
                                it = children.erase(it);
                            } else {
                                it++;
                            }
                        }
                    }
                    entity = entt::null;
                    removeComponents<0, Components...>(entity);
                }
            }
            template <size_t I, typename... Components> requires NOTLASTELEM<I, Components...>
            void removeComponents(EntityId entity) {
                removeComponent<std::tuple_element_t<I, std::tuple<Components...>>>(entity);
                removeComponents<I+1, Components...>(entity);
            }
            template <size_t I, typename... Components> requires LASTELEM<I, Components...>
            void removeComponents(EntityId entity) {
                removeComponent<std::tuple_element_t<I, std::tuple<Components...>>>(entity);
            }
            template <size_t I, typename... Components> requires EMPTY<I, Components...>
            void removeComponents(EntityId entity) {
            }
            template <typename... Signature>
            EntityId clone(EntityId toClone, EntityId parent = entt::null) {
                EntityId clonedId = factory.getEnttID();
                clone_impl<0, Signature...>(toClone, clonedId);
                if (parent != entt::null) {
                    addChild(parent,clonedId);
                }
                for (unsigned int i = 0; i < getChildren(toClone).size(); i++) {
                    clone<Signature...>(getChildren(toClone)[i], clonedId);
                }
                return clonedId;
            }
            template <size_t I, typename... Components> requires NOTLASTELEM<I, Components...>
            void clone_impl(EntityId toCloneId, EntityId clonedId) {
                //si l'entité possède le composant en question on le clône.
                if (getComponent<std::tuple_element_t<I, std::tuple<Components...>>>(toCloneId).has_value()) {
                    addComponent(clonedId, getComponent<std::tuple_element_t<I, std::tuple<Components...>>>(toCloneId).value().get());

                }
                clone_impl<I+1, Components...>(toCloneId, clonedId);
            }
            template <size_t I, typename... Components> requires LASTELEM<I, Components...>
            void clone_impl(EntityId toCloneId, EntityId clonedId) {

                if (getComponent<std::tuple_element_t<I, std::tuple<Components...>>>(toCloneId).has_value()) {
                    addComponent(clonedId, getComponent<std::tuple_element_t<I, std::tuple<Components...>>>(toCloneId).value().get());

                }
            }
            template <size_t I, typename... Components> requires EMPTY<I, Components...>
            void clone_impl(EntityId tocloneId, EntityId clonedId) {
            }
            template <typename... Signature>
            EntityId merge(EntityId toMerge1, EntityId toMerge2, EntityId parent=entt::null) {
                EntityId merged = factory.getEnttID();
                EntityId merged1, merged2;
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
            void merge_node(EntityId entityId, EntityId toMergeId) {
                //si l'entité possède le composant en question on le clône.
                if (getComponent<std::tuple_element_t<I, std::tuple<Signature...>>>(toMergeId).has_value()) {
                    addComponent(entityId, getComponent<std::tuple_element_t<I, std::tuple<Signature...>>>(toMergeId).value().get());
                }
                merge_node<I+1, Signature...>(entityId, toMergeId);
            }
            template < size_t I, typename... Signature> requires LASTELEM<I, Signature...>
            void merge_node(EntityId entityId, EntityId toMergeId) {
                if (getComponent<std::tuple_element_t<I, std::tuple<Signature...>>>(toMergeId).has_value()) {
                    addComponent(entityId, getComponent<std::tuple_element_t<I, std::tuple<Signature...>>>(toMergeId).value().get());
                }
            }
            template <size_t I, typename... Signature> requires EMPTY<I, Signature...>
            void merge_node(EntityId tocloneId, EntityId clonedId) {
            }
            template <typename... Signature, typename System, typename... Params>
            void apply(System& system, std::vector<EntityId>& entities, std::tuple<Params...>& params) {
              for (unsigned int i = 0; i < entities.size(); i++) {
                    apply<Signature...>(system, entities[i], params);
              }
            }
            template <typename... Signature, typename System, typename... Params>
            void apply(System& system, EntityId entity, std::tuple<Params...>& params) {
                this->template apply_impl<Signature...>(entity, system, params);
                for (unsigned int i = 0; i < getChildren(entity).size(); i++) {
                    apply<Signature...>(system, getChildren(entity)[i], params);
                }
            }
            template <typename... Signature, typename System, typename... Params>
            void apply_impl(EntityId entityId, System& system, std::tuple<Params...>& params) {
                system.template operator()<Signature...>(entityId, *this, params);
            }
            template <typename... Signature, typename System, typename... Params, typename R>
            void apply(System& system, std::vector<EntityId>& entities, std::tuple<Params...>& params, std::vector<R>& ret) {

              for (unsigned int i = 0; i < entities.size(); i++) {
                    apply<Signature...>(system, entities[i], params, ret);
              }
            }
            template <typename... Signature, typename System, typename... Params, typename R>
            void apply(System& system, EntityId entity, std::tuple<Params...>& params, std::vector<R>& ret) {
                this->template apply_impl<Signature...>(entity, system, params, ret);
                for (unsigned int i = 0; i < getChildren(entity).size(); i++) {
                    apply<Signature...>(system, getChildren(entity)[i], params, ret);
                }
            }
            template <typename... Signature, typename System, typename... Params, typename R>
            void apply_impl(EntityId entityId, System& system, std::tuple<Params...>& params, std::vector<R>& rets) {

                rets.push_back(system.template operator()<Signature...>(entityId, *this, params));
            }

        };
    }
}
#endif
