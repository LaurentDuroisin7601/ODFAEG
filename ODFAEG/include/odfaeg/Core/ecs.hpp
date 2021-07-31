#ifndef ODFAEG_ECS_HPP
#define ODFAEG_ECS_HPP
#include "dynamicTuple.hpp"
#include "../Graphics/entity.h"
namespace odfaeg {
    namespace graphic {
        template <typename T>
        struct atomwrapper
        {
          std::atomic<T> _a;

          atomwrapper()
            :_a()
          {}

          atomwrapper(const std::atomic<T> &a)
            :_a(a.load())
          {}

          atomwrapper(const atomwrapper &other)
            :_a(other._a.load())
          {}

          atomwrapper &operator=(const atomwrapper &other)
          {
            _a.store(other._a.load());
            return *this;
          }
          std::atomic<T>& get() {
            return _a;
          }
        };
        template <typename T>
        struct atomwrapper<T*>
        {
          std::atomic<T*> _a;

          atomwrapper()
            :_a()
          {}

          atomwrapper(const std::atomic<T> &a)
            :_a(a.load())
          {}

          atomwrapper(const atomwrapper &other)
            :_a(other._a.load())
          {}

          atomwrapper &operator=(const atomwrapper &other)
          {
            _a.store(other._a.load());
            return *this;
          }
          std::atomic<T*>& get() {
            return _a;
          }
          void destroy() {
            T* ptr = _a.load();
            delete ptr;
          }
        };

        size_t getGroupId (std::string groupName) {
            static size_t nbGroups=0;
            std::map<std::string, size_t> mapping;
            std::map<std::string, size_t>::iterator it = mapping.find(groupName);
            if (it == mapping.end()) {
                size_t groupId = nbGroups;
                mapping.insert(std::make_pair(groupName, groupId));
                nbGroups++;
                return groupId;
            }
            return it->second;
        }
        using EntityId = atomwrapper<std::size_t*>;
        class ComponentMapping {
                    template <class>
                    friend class World;
                    friend class CloningSystem;
                    private :
                    template <typename Component, typename DynamicTuple>
                    auto addFlag(DynamicTuple& tuple) {
                        auto newTuple = tuple.template addType<Component>();
                        for (unsigned int i = 0; i < componentMapping.size(); i++) {
                            componentMapping[i].resize(newTuple.nbTypes());
                        }
                        return newTuple;
                    }
                    template <typename Component, typename DynamicTuple, typename Factory>
                    auto addFlag(EntityId entity, DynamicTuple& tuple, Component component, Factory& factory) {
                        auto newTuple = tuple.add(component);
                        componentMapping.resize(factory.getNbEntities());
                        childrenMapping.resize(factory.getNbEntities());
                        nbLevels.resize(factory.getNbEntities());
                        branchIds.resize(factory.getNbEntities());
                        treeLevels.resize(factory.getNbEntities());
                        for (unsigned int i = 0; i < componentMapping.size(); i++) {
                            componentMapping[i].resize(newTuple.nbTypes());
                        }

                        componentMapping[*entity.get().load()][newTuple.template getIndexOfTypeT<Component>()] = newTuple.template vectorSize<Component>()-1;
                        return newTuple;
                    }

                    template <typename DynamicTuple, typename Component, typename Factory>
                    void addAgregate(EntityId entityId, DynamicTuple& tuple, Component component, Factory& factory) {
                        tuple.add(component);
                        componentMapping.resize(factory.getNbEntities());
                        childrenMapping.resize(factory.getNbEntities());
                        nbLevels.resize(factory.getNbEntities());
                        treeLevels.resize(factory.getNbEntities());
                        branchIds.resize(factory.getNbEntities());
                        for (unsigned int i = 0; i < componentMapping.size(); i++) {
                            componentMapping[i].resize(tuple.nbTypes());
                        }
                        componentMapping[*entityId.get().load()][tuple.template getIndexOfTypeT<Component>()] = tuple.template vectorSize<Component>()-1;
                    }
                    void addChild(EntityId rootId, EntityId parentId, EntityId child, size_t treeLevel) {

                        if (treeLevel >= nbLevels[*rootId.get().load()]) {
                            nbLevels[*rootId.get().load()]++;
                            for (unsigned int i = 0; i < childrenMapping.size(); i++) {
                                childrenMapping[*rootId.get().load()].resize(nbLevels[*rootId.get().load()]);
                            }
                        }

                        childrenMapping[*rootId.get().load()][treeLevel].push_back(child);
                        for (unsigned int i = 0; i < childrenMapping[*rootId.get().load()].size(); i++) {
                            for (unsigned int j = 0; j < childrenMapping[*rootId.get().load()][i].size(); j++) {
                                nbLevels[*childrenMapping[*rootId.get().load()][i][j].get().load()] = nbLevels[*rootId.get().load()];
                            }
                        }
                        treeLevels[*child.get().load()] = treeLevel;
                        branchIds[*child.get().load()] = parentId;
                    }
                    template <class T>
                    void removeInVector(std::vector<T>& vec, size_t index) {
                        unsigned int i;
                        for (auto it = vec.begin(), i = 0; it != vec.end();i++) {
                            if (index == i) {
                                std::cout<<"remove nb levels : "<<index<<std::endl;
                                vec.erase(it);
                            } else {
                                it++;
                            }
                        }
                    }
                    void removeTreeInfos(size_t i) {
                        removeInVector(treeLevels, i);
                        removeInVector(nbLevels, i);
                        //removeInVector(branchIds, i);
                    }
                    EntityId getRoot(EntityId entityId) {
                        EntityId parentId = entityId;
                        while (treeLevels[*parentId.get().load()].has_value()) {
                            parentId = branchIds[*parentId.get().load()];
                        }
                        return std::move(parentId);
                    }
                    bool sameBranch (size_t id1, size_t id2) {
                        while (treeLevels[id2].has_value()) {
                            EntityId parent;
                            parent = branchIds[id2];
                            if (*parent.get().load() == id1)
                                return true;
                            id2 = *parent.get().load();
                        }
                        return false;
                    }
                    void checkMaxlevels(EntityId rootId) {
                        size_t maxLevel = 0;
                        for (unsigned int i = 0; i < componentMapping.size(); i++) {
                            if (treeLevels[i] > maxLevel)
                                maxLevel++;
                        }
                        nbLevels[*rootId.get().load()] = maxLevel;
                        for (unsigned int i = 0; i < childrenMapping[*rootId.get().load()].size(); i++) {
                            for (unsigned int j = 0; j < childrenMapping[*rootId.get().load()][i].size(); j++) {
                                nbLevels[*childrenMapping[*rootId.get().load()][i][j].get().load()] = nbLevels[*rootId.get().load()];
                            }
                        }
                    }
                    void removeMapping(EntityId entityId) {

                        bool found = false;
                        std::vector<std::vector<std::optional<size_t>>>::iterator itToFind;
                        unsigned int i;
                        for (itToFind = componentMapping.begin(), i = 0; itToFind != componentMapping.end() && !found; itToFind++, i++) {

                            if (*entityId.get().load() == i) {
                                found = true;
                            }
                        }
                        if (found) {
                            i--;
                            std::optional<size_t> treeLevel = treeLevels[*entityId.get().load()];

                            //Met à jour les informations sur la branche si l'entité possèdes des enfants.
                            if (treeLevel.has_value()) {
                                EntityId rootId= getRoot(entityId);
                                size_t level = treeLevel.value();
                                unsigned int j;
                                std::vector<std::vector<std::optional<size_t>>>::iterator it;
                                //Recherche du mapping des enfants à supprimer.
                                for (it = componentMapping.begin(), j = 0; it != componentMapping.end(); j++) {
                                    //si l'enfant est située à un niveau en dessous sur la même branche, on supprime le mapping!

                                    if (sameBranch(*entityId.get().load(), j) && treeLevels[j].has_value() && treeLevels[j].value() > level) {
                                        std::vector<std::vector<EntityId>>::iterator it2;
                                        unsigned int k;
                                        //Supression du mapping dans la children map.
                                        for (it2 = childrenMapping[*rootId.get().load()].begin(), k = 0; it2 != childrenMapping[*rootId.get().load()].end(); k++) {
                                            std::vector<EntityId>::iterator it3;
                                            unsigned int c;
                                            for (it3 = childrenMapping[*rootId.get().load()][k].begin(), c = 0; it3 != childrenMapping[*rootId.get().load()][k].end(); c++) {
                                                if (sameBranch(*entityId.get().load(), *childrenMapping[*rootId.get().load()][k][c].get().load()) && nbLevels[*childrenMapping[*rootId.get().load()][k][c].get().load()] > level) {
                                                    childrenMapping[*rootId.get().load()][k].erase(it3);
                                                } else {
                                                    it3++;
                                                }
                                            }
                                        }
                                        removeTreeInfos(j);
                                        componentMapping.erase(it);
                                    } else {
                                        it++;
                                    }
                                }
                                checkMaxlevels(rootId);
                            }
                            //Supprime le mapping de l'entité.
                            std::vector<std::vector<std::vector<EntityId>>>::iterator it;
                            unsigned int i = 0;
                            for (it = childrenMapping.begin(), i = 0; it != childrenMapping.end(); i++) {
                                if (i == *entityId.get().load()) {
                                    childrenMapping.erase(it);
                                } else {
                                    it++;
                                }
                            }
                            removeTreeInfos(i);
                            componentMapping.erase(itToFind);

                        }

                    }
                    public :
                    template <typename T, typename DynamicTuple>
                    T* getAgregate(DynamicTuple& tuple, EntityId entityId) {
                        //std::cout<<"id : "<<*entityId<<","<<"size : "<<componentMapping.size()<<std::endl;
                        if (componentMapping[*entityId.get().load()][tuple.template getIndexOfTypeT<T>()].has_value())
                            return tuple.template get<T>(componentMapping[*entityId.get().load()][tuple.template getIndexOfTypeT<T>()].value());
                        return nullptr;
                    }
                    template <typename... Signature, typename DynamicTuple, typename System, typename... Params>
                    void apply(DynamicTuple& tuple, System& system, std::vector<EntityId>& entities, std::tuple<Params...>& params) {
                      for (unsigned int i = 0; i < entities.size(); i++) {
                        this->template apply_impl<Signature...>(entities[i], tuple, system, params, std::index_sequence_for<Signature...>());
                        size_t level = (treeLevels[*entities[i].get().load()].has_value()) ? treeLevels[*entities[i].get().load()].value() : 0;
                        for (unsigned int j = level; j < nbLevels[*entities[i].get().load()]; j++) {
                          for(unsigned int k = 0; k < childrenMapping[*getRoot(entities[i]).get().load()][j].size(); k++)
                            this->template apply_impl<Signature...>(childrenMapping[*getRoot(entities[i]).get().load()][j][k], tuple, system, params, std::index_sequence_for<Signature...>());
                        }
                      }
                    }

                    template <typename... Signature, typename DynamicTuple, typename System, size_t... I, typename... Params>
                    void apply_impl(EntityId entityId, DynamicTuple& tuple, System& system, std::tuple<Params...>& params, std::index_sequence<I...>) {
                        auto tp = std::make_tuple(getAgregate<std::tuple_element_t<I, std::tuple<Signature...>>>(tuple, entityId)...);
                        system(tp, entityId, params);
                    }
                    template <typename... Signature, typename DynamicTuple, typename System, typename... Params, class R>
                    void apply(DynamicTuple& tuple, System& system, std::vector<EntityId>& entities, std::tuple<Params...>& params, std::vector<R>& ret) {
                      for (unsigned int i = 0; i < entities.size(); i++) {
                        for (unsigned int j = 0; j < nbLevels[*entities[i].get().load()]; j++) {
                          for(unsigned int k = 0; k < childrenMapping[*entities[i].get().load()][j].size(); k++) {
                            this->template apply_impl<Signature...>(childrenMapping[*entities[i].get().load()][j][k], tuple, system, params, std::index_sequence_for<Signature...>(), ret);
                          }
                        }
                        this->template apply_impl<Signature...>(entities[i], tuple, system, params, std::index_sequence_for<Signature...>());
                      }
                    }
                    template <typename... Signature, typename DynamicTuple, typename System, size_t... I, typename... Params, typename R>
                    void apply_impl(EntityId entityId, DynamicTuple& tuple, System& system, std::tuple<Params...>& params, std::index_sequence<I...>, std::vector<R>& rets) {
                        auto tp = std::make_tuple(getAgregate<std::tuple_element_t<I, std::tuple<Signature...>>>(tuple, entityId)...);
                        rets.push_back(system(tp, entityId, params));
                    }
                    private :
                    std::vector<std::vector<std::optional<size_t>>> componentMapping;
                    std::vector<std::vector<std::vector<EntityId>>> childrenMapping;
                    std::vector<size_t> nbLevels;
                    std::vector<std::optional<size_t>> treeLevels;
                    std::vector<EntityId> branchIds;
                };
        struct EntityFactory {
            template <typename T>
            friend class World;
            friend class ComponentMapping;
            size_t nbEntities=0;
            std::vector<EntityId> ids;
            EntityId createEntity() {

                EntityId id;
                id.get().store(new size_t(nbEntities));
                ids.push_back(id);
                nbEntities++;
                return id;
            }
            size_t getNbEntities() {
                return nbEntities;
            }
            ~EntityFactory() {
                for (unsigned int i = 0; i < ids.size(); i++) {
                    ids[i].destroy();
                }
            }
            private :
            void destroyEntity(EntityId id) {
                const auto itToFind =
                    std::find_if(ids.begin(), ids.end(),
                                 [&](auto& p) { return p.get().load() == id.get().load(); });
                const bool found = (itToFind != ids.end());
                if (found) {
                    for (auto it = itToFind; it != ids.end(); it++) {
                        *it->get().load() =  *it->get().load() - 1;
                    }
                    itToFind->destroy();
                    ids.erase(itToFind);

                    nbEntities--;
                }
            }
        };
        template <typename T>
        atomwrapper<bool>& getProducerStatus() {
            static atomwrapper<bool> task;
            return task;
        }
        template <typename T>
        void setProducerStatus(bool status) {
             getProducerStatus<T>().get().store(status);
        }

        template <typename T>
        bool isProduced() {
             return getProducerStatus<T>().get().load();
        }
        template <typename T>
        atomwrapper<bool>& getConsumerStatus() {
            static atomwrapper<bool> task(true);
            return task;
        }
        template <typename T>
        void setConsumerStatus(bool status) {
             getConsumerStatus<T>().get().store(status);
        }
        template <typename T>
        bool isConsumed() {
             return getConsumerStatus<T>().get().load();
        }
    }
}
#endif
