#ifndef ODFAEG_ECS_HPP
#define ODFAEG_ECS_HPP
#include <vector>
#include "../Graphics/ECS/components.hpp"
#include "entityFactory.hpp"
namespace odfaeg {
    namespace graphic {
        namespace ecs {
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

            /*template <typename T>
            std::vector<std::optional<T>>& get(){
                static std::vector<std::optional<T>> tvec;
                return tvec;
            }
            template <typename T>
            void addComponent(EntityId entity, T component) {
                if (entity >= get<T>().size())
                    get<T>().resize(entity+1);
                get<T>()[entity] = component;
            }
            template <typename T>
            T* getComponent(EntityId entity) {
                if (entity >= get<T>().size() || !get<T>()[entity].has_value()) {
                    return nullptr;
                } else {

                    return &get<T>()[entity].value();
                }
            }
            template <typename T>
            void removeComponent(EntityId entity) {
                typename std::vector<T>::iterator it;
                unsigned int i;
                for (i = 0, it = get<T>().begin(); it != get<T>().end();i++) {
                    if (i == entity)
                        it = get<T>().erase(it);
                    else
                        it++;
                }
            }*/
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
                T* getComponent(EntityId entity) {
                    if (!factory.getRegistery().all_of<T>(entity))
                        return nullptr;
                    return &factory.getRegistery().get<T>(entity);
                }
                template <typename T>
                void removeComponent(EntityId entity) {
                    if (factory.getRegistery().all_of<T>(entity))
                        factory.getRegistery().remove<T>(entity);
                }
                void addChild(EntityId parentId, EntityId child) {

                    if (!getComponent<Relation>(parentId)) {
                        std::cout<<"add relation add child"<<std::endl;
                        Relation relation;
                        relation.children.push_back(child);
                        addComponent(parentId, relation);
                    } else {
                        std::cout<<"add child"<<std::endl;
                        getComponent<Relation>(parentId)->children.push_back(child);
                    }
                    if (!getComponent<Relation>(child)) {
                        std::cout<<"add relation set parent"<<std::endl;
                        Relation relation;
                        relation.parent = parentId;
                        addComponent(child, relation);
                    } else {
                        std::cout<<"set parent"<<std::endl;
                        getComponent<Relation>(child)->parent = child;
                    }

                }
                EntityId getRoot(EntityId entityId) {
                    if (getParent(entityId) == entt::null)
                        return entityId;
                    return getRoot(getParent(entityId));
                }
                std::vector<EntityId> getChildren(EntityId entity) {
                    std::vector<EntityId> children;
                    if (getComponent<Relation>(entity)) {
                       children =  getComponent<Relation>(entity)->children;
                    }
                    return children;
                }
                EntityId getParent(EntityId entity) {
                    EntityId parent = entt::null;
                    if(getComponent<Relation>(entity)) {
                       parent =  getComponent<Relation>(entity)->parent;
                    }
                    return parent;
                }
                template <typename Archive, typename... Components>
                void readEntities(Archive& ar, std::vector<EntityId>& entities) {
                    size_t nbEntities;
                    ar(nbEntities);
                    for (unsigned int i = 0; i < nbEntities; i++) {
                        EntityId entity = factory.createEntity();
                        readEntity<Archive, Components...>(ar, entity);
                        entities.push_back(entity);
                    }
                }
                template <typename Archive, typename... Components>
                void readEntity(Archive& ar, EntityId& entity) {
                    readComponents<0, Archive, Components...>(ar, entity);
                    if (getComponent<EntityInfoComponent>(entity)) {
                        getComponent<EntityInfoComponent>(entity)->id = factory.getNbEntities() -1;
                        factory.updateTypes(getComponent<EntityInfoComponent>(entity)->groupName);
                    }
                    if (getComponent<MeshComponent>(entity) != nullptr) {
                        MeshComponent* mc = getComponent<MeshComponent>(entity);
                        for (unsigned int i = 0; i < mc->faces.size(); i++) {
                            mc->faces[i].getVertexArray().setEntityId(entity);
                        }
                    }
                    size_t nbChildren;
                    ar(nbChildren);
                    for (unsigned int i = 0; i < nbChildren; i++) {
                        EntityId child = factory.createEntity();
                        readEntity<Archive, Components...>(ar, child);
                        addChild(entity, child);
                    }
                }

                template <size_t I, typename Archive, typename... Components> requires NOTLASTELEM<I, Components...>
                void readComponents(Archive& ar, EntityId entity) {
                    std::tuple_element_t<I, std::tuple<Components...>>* component;
                    std::cout<<"read component"<<std::endl;
                    ar(component);
                    std::cout<<"component : "<<component<<std::endl;
                    if (component != nullptr) {
                        addComponent(entity, *component);
                    }
                    readComponents<I+1, Archive, Components...>(ar, entity);
                }
                template <size_t I, typename Archive, typename... Components> requires LASTELEM<I, Components...>
                void readComponents(Archive& ar, EntityId entity) {
                    std::tuple_element_t<I, std::tuple<Components...>>* component;
                    std::cout<<"read component"<<std::endl;
                    ar(component);
                    std::cout<<"component : "<<component<<std::endl;
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
                    std::cout<<"component :  "<<getComponent<std::tuple_element_t<I, std::tuple<Components...>>>(entity)<<std::endl;
                    ar(getComponent<std::tuple_element_t<I, std::tuple<Components...>>>(entity));
                    writeComponents<I+1, Archive, Components...>(ar, entity);
                }
                template <size_t I, typename Archive, typename... Components> requires LASTELEM<I, Components...>
                void writeComponents(Archive& ar, EntityId entity) {
                    std::cout<<"component :  "<<getComponent<std::tuple_element_t<I, std::tuple<Components...>>>(entity)<<std::endl;
                    ar(getComponent<std::tuple_element_t<I, std::tuple<Components...>>>(entity));
                }

                template <size_t I, typename Archive, typename... Components> requires EMPTY<I, Components...>
                void writeComponents(Archive& ar, EntityId entity) {

                }
                template <typename... Components>
                void remove(EntityId entity) {
                    if (getComponent<Relation>(entity)) {
                        EntityId parent = getComponent<Relation>(entity)->parent;
                        if (getComponent<Relation>(parent)) {
                            std::vector<EntityId>& children = getComponent<Relation>(parent)->children;
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
                    EntityId clonedId = factory.createEntity(getComponent<EntityInfoComponent>(toClone)->groupName);
                    clone_impl<0, Signature...>(toClone, clonedId);
                    getComponent<EntityInfoComponent>(clonedId)->id = factory.getNbEntities() - 1;
                    if (getComponent<MeshComponent>(clonedId) != nullptr) {
                        MeshComponent* mc = getComponent<MeshComponent>(clonedId);
                        for (unsigned int i = 0; i < mc->faces.size(); i++) {
                            mc->faces[i].getVertexArray().setEntityId(clonedId);
                        }
                    }
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
                    if (getComponent<std::tuple_element_t<I, std::tuple<Components...>>>(toCloneId)) {
                        addComponent(clonedId, *getComponent<std::tuple_element_t<I, std::tuple<Components...>>>(toCloneId));

                    }
                    clone_impl<I+1, Components...>(toCloneId, clonedId);
                }
                template <size_t I, typename... Components> requires LASTELEM<I, Components...>
                void clone_impl(EntityId toCloneId, EntityId clonedId) {

                    if (getComponent<std::tuple_element_t<I, std::tuple<Components...>>>(toCloneId)) {
                        addComponent(clonedId, *getComponent<std::tuple_element_t<I, std::tuple<Components...>>>(toCloneId));

                    }
                }
                template <size_t I, typename... Components> requires EMPTY<I, Components...>
                void clone_impl(EntityId tocloneId, EntityId clonedId) {
                }
                template <typename... Signature>
                EntityId merge(EntityId toMerge1, EntityId toMerge2, std::string groupName, EntityId parent=entt::null) {
                    EntityId merged = factory.createEntity(groupName);
                    EntityId merged1, merged2;
                    if (parent == entt::null) {
                        merged1 = factory.createEntity(getComponent<EntityInfoComponent>(toMerge1)->groupName);
                        merged2 = factory.createEntity(getComponent<EntityInfoComponent>(toMerge2)->groupName);
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
                    getComponent<EntityInfoComponent>(merged)->id = factory.getNbEntities() - 1;
                    if (getComponent<MeshComponent>(merged) != nullptr) {
                        MeshComponent* mc = getComponent<MeshComponent>(merged);
                        for (unsigned int i = 0; i < mc->faces.size(); i++) {
                            mc->faces[i].getVertexArray().setEntityId(merged);
                        }
                    }
                    for (unsigned int i = 0; i < getChildren(toMerge1).size(); i++) {
                        merge<Signature...>(factory, getChildren(toMerge1)[i], entt::null, getComponent<EntityInfoComponent>(getChildren(toMerge1)[i])->groupName, (parent == entt::null) ? merged1 : merged);
                    }
                    for (unsigned int i = 0; i < getChildren(toMerge2).size(); i++) {
                        merge<Signature...>(factory, entt::null, getChildren(toMerge2)[i], getComponent<EntityInfoComponent>(getChildren(toMerge2)[i])->groupName, (parent == entt::null) ? merged2 : merged);
                    }
                    return merged;
                }
                template <size_t I, typename... Signature> requires NOTLASTELEM<I, Signature...>
                void merge_node(EntityId entityId, EntityId toMergeId) {
                    //si l'entité possède le composant en question on le clône.
                    if (getComponent<std::tuple_element_t<I, std::tuple<Signature...>>>(toMergeId)) {
                        addComponent(entityId, *getComponent<std::tuple_element_t<I, std::tuple<Signature...>>>(toMergeId));
                    }
                    merge_node<I+1, Signature...>(entityId, toMergeId);
                }
                template < size_t I, typename... Signature> requires LASTELEM<I, Signature...>
                void merge_node(EntityId entityId, EntityId toMergeId) {
                    if (getComponent<std::tuple_element_t<I, std::tuple<Signature...>>>(toMergeId)) {
                        addComponent(entityId, *getComponent<std::tuple_element_t<I, std::tuple<Signature...>>>(toMergeId));
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
}
#endif
