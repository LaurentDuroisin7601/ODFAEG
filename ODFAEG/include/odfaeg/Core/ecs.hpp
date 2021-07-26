#ifndef ODFAEG_ECS_HPP
#define ODFAEG_ECS_HPP
#include "dynamicTuple.hpp"
#include "../Graphics/entity.h"
namespace odfaeg {
    namespace core {
        struct IComponent : IDynamicTupleElement {
        };
        struct ISystem {
            size_t positionInVector;
        };
        using EntityId = std::size_t;
        class ComponentMapping {
            public :
            template <typename Component, typename DynamicTuple>
            auto addFlag(DynamicTuple& tuple) {
                auto newTuple = tuple.template addType<Component>();
                for (unsigned int i = 0; i < componentMapping.size(); i++) {
                    componentMapping[i].resize(newTuple.nbTypes());
                }
                return newTuple;
            }
            template <typename Component, typename DynamicTuple, typename Factory>
            auto addFlag(EntityId entity, DynamicTuple& tuple, Component* component, Factory& factory) {
                auto newTuple = tuple.add(component);
                componentMapping.resize(factory.getNbEntities());
                childrenMapping.resize(factory.getNbEntities());
                nbLevels.resize(factory.getNbEntities());
                for (unsigned int i = 0; i < componentMapping.size(); i++) {
                    componentMapping[i].resize(tuple.nbTypes());
                }
                componentMapping[entity][component->positionInTemplateParameterPack] = component->positionInVector;
                return newTuple;
            }
            template <typename DynamicTuple, typename Component, typename Factory>
            void addAgregate(size_t entityId, DynamicTuple& tuple, Component* component, Factory& factory) {
                tuple.add(component);
                componentMapping.resize(factory.getNbEntities());
                childrenMapping.resize(factory.getNbEntities());
                nbLevels.resize(factory.getNbEntities());
                for (unsigned int i = 0; i < componentMapping.size(); i++) {
                    componentMapping[i].resize(tuple.nbTypes());
                }
                componentMapping[entityId][component->positionInTemplateParameterPack] = component->positionInVector;
            }
            void addChild(size_t parentId, size_t child, size_t treeLevel) {
                if (treeLevel >= nbLevels[parentId]) {
                    nbLevels[parentId]++;
                    for (unsigned int i = 0; i < childrenMapping.size(); i++) {
                        childrenMapping[parentId].resize(nbLevels[parentId]);
                    }
                }
                childrenMapping[parentId][treeLevel].push_back(child);
            }
            template <typename T, typename DynamicTuple>
            T* getAgregate(DynamicTuple& tuple, size_t entityId) {
                if (componentMapping[entityId][tuple.template getIndexOfTypeT<T>()].has_value())
                    return tuple.template get<T>(componentMapping[entityId][tuple.template getIndexOfTypeT<T>()].value());
                return nullptr;
            }
            template <typename... Signature, typename DynamicTuple, typename System, typename... Params>
            void apply(DynamicTuple& tuple, System& system, std::vector<size_t>& entities, std::tuple<Params...>& params) {
              for (unsigned int i = 0; i < entities.size(); i++) {
                this->template apply_impl<Signature...>(entities[i], tuple, system, params, std::index_sequence_for<Signature...>());
                for (unsigned int j = 0; j < nbLevels[entities[i]]; j++) {
                  for(unsigned int k = 0; k < childrenMapping[entities[i]][j].size(); k++)
                    this->template apply_impl<Signature...>(childrenMapping[entities[i]][j][k], tuple, system, params, std::index_sequence_for<Signature...>());
                }
              }
            }
            template <typename... Signature, typename DynamicTuple, typename System, size_t... I, typename... Params>
            void apply_impl(size_t entityId, DynamicTuple& tuple, System& system, std::tuple<Params...>& params, std::index_sequence<I...>) {
                auto tp = std::make_tuple(getAgregate<std::tuple_element_t<I, std::tuple<Signature...>>>(tuple, entityId)...);
                system(tp, params);
            }
            private :
            std::vector<std::vector<std::optional<size_t>>> componentMapping;
            std::vector<std::vector<std::vector<size_t>>> childrenMapping;
            std::vector<size_t> nbLevels;
        };

        struct EntityFactory {
            size_t nbEntities=0;
            EntityId createEntity() {
                nbEntities++;
                return nbEntities-1;
            }
            size_t getNbEntities() {
                return nbEntities;
            }
        };
    }
}
#endif
