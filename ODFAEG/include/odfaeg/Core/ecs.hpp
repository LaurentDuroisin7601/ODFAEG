#ifndef ODFAEG_ECS_HPP
#define ODFAEG_ECS_HPP
#include <vector>
#include "../Graphics/ECS/components.hpp"
#include <optional>
#include "entityFactory.hpp"
namespace odfaeg {
    namespace graphic {
        namespace ecs {


            template <typename T>
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
                    /*if (name == "test")
                        std::cout<<"return nullptr"<<std::endl;*/
                    return nullptr;
                } else {
                    /*if (name == "test")
                        std::cout<<"return : "<<&get<T>()[entity].value()<<std::endl;*/
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
            }
            class ComponentMapping {

                std::vector<std::vector<EntityId>> childrenMapping;
                std::vector<std::optional<EntityId>> parentMapping;
                public :
                void addChild(EntityId parentId, EntityId child) {
                    if (parentId >= childrenMapping.size())
                        childrenMapping.resize(parentId + 1);
                    if (child >= parentMapping.size())
                        parentMapping.resize(child + 1);
                    childrenMapping[parentId].push_back(child);
                    parentMapping[child] = parentId;
                }
                EntityId getRoot(EntityId entityId) {
                    EntityId root = entityId;
                    if (entityId >= parentMapping.size())
                        return root;
                    while (parentMapping[root].has_value()) {
                        root = parentMapping[root].value();
                    }
                    return root;
                }
                std::vector<EntityId> getChildren(EntityId entity) {
                    if (entity >= childrenMapping.size())
                        return std::vector<EntityId>();
                    return childrenMapping[entity];
                }
                EntityId getParent(EntityId entity) {
                    if (entity >= parentMapping.size())
                        return -1;
                    if(parentMapping[entity].has_value());
                       return parentMapping[entity].value();
                    return -1;
                }
                template <typename Archive, typename... Components>
                void readEntities(EntityFactory& factory, Archive& ar, std::vector<EntityId>& entities) {
                    size_t nbEntities;
                    ar(nbEntities);
                    for (unsigned int i = 0; i < nbEntities; i++) {
                        EntityId entity;
                        readEntity<Archive, Components...>(factory, ar, entity);
                        entities.push_back(entity);
                    }
                }
                template <typename Archive, typename... Components>
                void readEntity(EntityFactory& factory, Archive& ar, EntityId& entity) {
                    ar(entity);
                    readComponents<0, Archive, Components...>(ar, entity);
                    factory.createEntity(getComponent<EntityInfoComponent>(entity)->groupName);
                    size_t nbChildren;
                    ar(nbChildren);
                    for (unsigned int i = 0; i < nbChildren; i++) {
                        EntityId child;
                        readEntity<Archive, Components...>(factory, ar, child);
                        addChild(entity, child);
                    }
                }
                template <size_t I, typename Archive, typename... Components, class = typename std::enable_if_t<(sizeof...(Components) != 0 && I < sizeof...(Components)-1)>>
                void readComponents(Archive& ar, EntityId entity) {
                    std::tuple_element_t<I, std::tuple<Components...>>* component;
                    ar(component);
                    if (component != nullptr) {
                        addComponent(entity, *component);
                    }
                    readComponents<I+1, Archive, Components...>(ar, entity);
                }
                template <size_t I, typename Archive, typename... Components, class... D, class = typename std::enable_if_t<(sizeof...(Components) != 0 && I == sizeof...(Components)-1)>>
                void readComponents(Archive& ar, EntityId entity) {
                    std::tuple_element_t<I, std::tuple<Components...>>* component;
                    ar(component);
                    if (component != nullptr) {
                        addComponent(entity, *component);
                    }
                }
                template <size_t I, typename Archive, typename... Components, class... D, class... E, class = typename std::enable_if_t<(sizeof...(Components) == 0)>>
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
                    ar(entity);
                    writeComponents<0, Archive, Components...>(ar, entity);
                    unsigned int size = getChildren(entity).size();
                    ar(size);
                    for (unsigned int i = 0; i < getChildren(entity).size(); i++) {
                        writeEntity<Archive, Components...>(ar, getChildren(entity)[i]);
                    }
                }
                template <size_t I, typename Archive, typename... Components, class = typename std::enable_if_t<(sizeof...(Components) != 0 && I < sizeof...(Components)-1)>>
                void writeComponents(Archive& ar, EntityId entity) {
                    ar(getComponent<std::tuple_element_t<I, std::tuple<Components...>>>(entity));
                    writeComponents<I+1, Archive, Components...>(ar, entity);
                }
                template <size_t I, typename Archive, typename... Components, class... D, class = typename std::enable_if_t<(sizeof...(Components) != 0 && I == sizeof...(Components)-1)>>
                void writeComponents(Archive& ar, EntityId entity) {
                    ar(getComponent<std::tuple_element_t<I, std::tuple<Components...>>>(entity));
                }

                template <size_t I, typename Archive, typename... Components, class... D, class... E, class = typename std::enable_if_t<(sizeof...(Components) == 0)>>
                void writeComponents(Archive& ar, EntityId entity) {

                }
                bool remove(EntityId entity) {
                    if (entity <  childrenMapping.size()) {
                        childrenMapping[entity].clear();
                    }
                    if (entity < parentMapping.size()) {
                        std::vector<std::optional<EntityId>>::iterator it;
                        for (it = parentMapping.begin(); it != parentMapping.end(); it++) {
                            if (it->has_value() && it->value() == entity)
                                it->reset();
                        }
                    }
                }
                template <typename... Signature>
                EntityId clone(EntityFactory& factory, EntityId toClone, EntityId parent = -1) {
                    EntityId clonedId = factory.createEntity(getComponent<EntityInfoComponent>(toClone)->groupName);
                    clone_impl<0, Signature...>(toClone, clonedId);

                    if (parent != -1) {
                        if (parent >= childrenMapping.size())
                            childrenMapping.resize(parent + 1);
                        if (clonedId >= parentMapping.size())
                            parentMapping.resize(clonedId + 1);
                        childrenMapping[parent].push_back(clonedId);
                        parentMapping[clonedId] = parent;
                    }
                    for (unsigned int i = 0; i < getChildren(toClone).size(); i++) {
                        clone<Signature...>(factory, childrenMapping[toClone][i], clonedId);
                    }
                    if (parent == -1)
                        return clonedId;
                }
                template <size_t I, typename... Components, class = typename std::enable_if_t<(sizeof...(Components) != 0 && I < sizeof...(Components)-1)>>
                void clone_impl(EntityId toCloneId, EntityId clonedId) {
                    //si l'entité possède le composant en question on le clône.
                    if (getComponent<std::tuple_element_t<I, std::tuple<Components...>>>(toCloneId)) {
                        addComponent(clonedId, *getComponent<std::tuple_element_t<I, std::tuple<Components...>>>(toCloneId));

                    }
                    clone_impl<I+1, Components...>(toCloneId, clonedId);
                }
                template <size_t I, typename... Components, class... D, class = typename std::enable_if_t<(sizeof...(Components) != 0 && I == sizeof...(Components)-1)>>
                void clone_impl(EntityId toCloneId, EntityId clonedId) {

                    if (getComponent<std::tuple_element_t<I, std::tuple<Components...>>>(toCloneId)) {
                        addComponent(clonedId, *getComponent<std::tuple_element_t<I, std::tuple<Components...>>>(toCloneId));

                    }
                }
                template <size_t I, typename... Components, class... D, class...E, class = typename std::enable_if_t<(sizeof...(Components) == 0)>>
                void clone_impl(EntityId tocloneId, EntityId clonedId) {
                }
                template <typename... Signature>
                EntityId merge(EntityFactory& factory, EntityId toMerge1, EntityId toMerge2, std::string groupName, EntityId parent=-1) {
                    EntityId merged = factory.createEntity(groupName);
                    EntityId merged1, merged2;
                    if (parent == -1) {
                        merged1 = factory.createEntity(getComponent<EntityInfoComponent>(toMerge1)->groupName);
                        merged2 = factory.createEntity(getComponent<EntityInfoComponent>(toMerge2)->groupName);
                        merge_node<0, Signature...>(merged, toMerge1);
                        merge_node<0, Signature...>(merged, toMerge2);
                        if(merged >= childrenMapping.size())
                            childrenMapping.resize(parent+1);
                        if (merged1 >= parentMapping.size())
                            parentMapping.resize(merged1 + 1);
                        if (merged2 >= parentMapping.size())
                            parentMapping.resize(merged2 + 1);
                        childrenMapping[merged].push_back(merged1);
                        parentMapping[merged1] = merged;
                        childrenMapping[merged].push_back(merged2);
                        parentMapping[merged2] = merged;
                    }
                    if (parent != -1 && toMerge1 != -1) {
                        if(parent >= childrenMapping.size())
                            childrenMapping.resize(parent+1);
                        if (merged >= parentMapping.size())
                            parentMapping.resize(merged + 1);
                        childrenMapping[parent].push_back(merged);
                        parentMapping[merged] = parent;
                        merge_node<0, Signature...>(merged, toMerge1);
                    }
                    if (parent != -1 && toMerge2 != -1) {
                        if(parent >= childrenMapping.size())
                            childrenMapping.resize(parent+1);
                        if (merged >= parentMapping.size())
                            parentMapping.resize(merged + 1);
                        childrenMapping[parent].push_back(merged);
                        parentMapping[merged] = parent;
                        merge_node<0, Signature...>(merged, toMerge2);
                    }
                    for (unsigned int i = 0; i < childrenMapping[toMerge1].size(); i++) {
                        merge<Signature...>(factory, childrenMapping[toMerge1][i], -1, getComponent<EntityInfoComponent>(childrenMapping[toMerge1][i])->groupName, (parent == -1) ? merged1 : merged);
                    }
                    for (unsigned int i = 0; i < childrenMapping[toMerge2].size(); i++) {
                        merge<Signature...>(factory, -1, childrenMapping[toMerge2][i], getComponent<EntityInfoComponent>(childrenMapping[toMerge2][i])->groupName, (parent == -1) ? merged2 : merged);
                    }
                    if (parent == -1)
                        return merged;
                }
                template <size_t I, typename... Signature, class = typename std::enable_if_t<(sizeof...(Signature) != 0 && I < sizeof...(Signature)-1)>>
                void merge_node(EntityId entityId, EntityId toMergeId) {
                    //si l'entité possède le composant en question on le clône.
                    if (getComponent<std::tuple_element_t<I, std::tuple<Signature...>>>(toMergeId)) {
                        addComponent(entityId, *getComponent<std::tuple_element_t<I, std::tuple<Signature...>>>(toMergeId));
                    }
                    merge_node<I+1, Signature...>(entityId, toMergeId);
                }
                template < size_t I, typename... Signature, class... D, class = typename std::enable_if_t<(sizeof...(Signature) != 0 && I == sizeof...(Signature)-1)>>
                void merge_node(EntityId entityId, EntityId toMergeId) {
                    if (getComponent<std::tuple_element_t<I, std::tuple<Signature...>>>(toMergeId)) {
                        addComponent(entityId, *getComponent<std::tuple_element_t<I, std::tuple<Signature...>>>(toMergeId));
                    }
                }
                template <size_t I, typename... Signature, class... D, class...E, class = typename std::enable_if_t<(sizeof...(Signature) == 0)>>
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
                    system.template operator()<Signature...>(entityId, params);
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

                    rets.push_back(system.template operator()<Signature...>(entityId, params));
                }

            };

        }
    }
}
#endif
