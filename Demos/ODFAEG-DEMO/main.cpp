#include "application.h"
#include "odfaeg/Core/mp.hpp"

/*using namespace odfaeg::core;
using namespace odfaeg::math;
using namespace odfaeg::physic;
using namespace odfaeg::graphic;
using namespace odfaeg::window;
using namespace odfaeg::audio;
using namespace sorrok;*/
template < typename Tp, typename... List >
struct contains : std::true_type {};

template < typename Tp, typename Head, typename... Rest >
struct contains<Tp, Head, Rest...>
: std::conditional< std::is_same<Tp, Head>::value,
    std::true_type,
    contains<Tp, Rest...>
>::type {};
template <typename T, typename Tuple>
struct has_type;

template <typename T>
struct has_type<T, std::tuple<>> : std::false_type {};

template <typename T, typename U, typename... Ts>
struct has_type<T, std::tuple<U, Ts...>> : has_type<T, std::tuple<Ts...>> {};

template <typename T, typename... Ts>
struct has_type<T, std::tuple<T, Ts...>> : std::true_type {};
template < typename Tp >
struct contains<Tp> : std::false_type {};

template <typename T, typename U=void, typename... Types>
constexpr size_t index() {
    return std::is_same<T, U>::value ? 0 : 1 + index<T, Types...>();
}
//Make index sequences from an offset.
template<std::size_t N, typename Seq> struct offset_sequence;

template<std::size_t N, std::size_t... Ints>
struct offset_sequence<N, std::index_sequence<Ints...>>
{
 using type = std::index_sequence<Ints + N...>;
};
template<std::size_t N, typename Seq>
using offset_sequence_t = typename offset_sequence<N, Seq>::type;
//Concatenate two sequences of indexes into one.
template<typename Seq1, typename Seq> struct cat_sequence;
template<std::size_t... Ints1, std::size_t... Ints2>
struct cat_sequence<std::index_sequence<Ints1...>,
                    std::index_sequence<Ints2...>>
{
 using type = std::index_sequence<Ints1..., Ints2...>;
};
template<typename Seq1, typename Seq2>
using cat_sequence_t = typename cat_sequence<Seq1, Seq2>::type;

template<std::size_t N, typename T>
auto remove_Nth(T& tuple);
template <typename... TupleTypes>
struct DynamicTuple {
    std::tuple<std::vector<TupleTypes>...> content;
    using types = typename std::tuple<TupleTypes...>;


    template <typename H, class = typename std::enable_if_t<contains<H, TupleTypes...>::value>>
    DynamicTuple add (H head) {
        std::get<std::vector<H>>(content).push_back(head);
        return *this;
    }
    template <typename H, class = typename std::enable_if_t<!contains<H, TupleTypes...>::value>>
    DynamicTuple <TupleTypes..., H> add (H head) {
        DynamicTuple<TupleTypes..., H> tuple;
        tuple.content = std::tuple_cat(content, std::make_tuple(std::vector<H>()));
        return tuple.add(head);
    }
    template <typename H, class = typename std::enable_if_t<!contains<H, TupleTypes...>::value>>
    DynamicTuple <TupleTypes..., H> addType () {
        DynamicTuple<TupleTypes..., H> tuple;
        tuple.content = std::tuple_cat(content, std::make_tuple(std::vector<H>()));
        return tuple;
    }
    template <typename T>
    constexpr size_t vectorSize() {
        return std::get<std::vector<T>>(content).size();
    }
    static constexpr size_t nbTypes() {
        return std::tuple_size<types>::value;
    }
    template <typename T>
    static constexpr size_t getIndexOfTypeT() {
        return index<T, TupleTypes...>();
    }
    template <typename T>
    T* get(unsigned int containerIdx) {
        constexpr size_t I = getIndexOfTypeT<T>();
        return (containerIdx < vectorSize<T>()) ? &std::get<I>(content)[containerIdx] : nullptr;
    }
    template <size_t I>
    auto get(unsigned int containerIdx) {
        return std::get<I>(content)[containerIdx];
    }
    template <size_t I>
    auto removeType() {
        return remove_Nth<I>(*this);
    }
    template <typename T>
    auto removeType() {
        return remove_Nth<index<T, TupleTypes...>()>(*this);
    }
    template <typename T>
    auto remove(T element) {
        std::vector<T> elements = std::get<index<T, TupleTypes...>()>(content);
        typename std::vector<T>::iterator it;
        for (it = elements.begin(); it != elements.end(); it++) {
            if (*it == &element) {
                elements.erase(it);
            } else {
                it++;
            }
        }
        if (std::get<index<T, TupleTypes...>()>(content).size() == 0) {
            return removeType<T>();
        }
        return *this;
    }
};
//Return a tuple with elements at indexes.
template <typename T, size_t... Ints>
auto select_tuple(T& tuple, std::index_sequence<Ints...>)
{
    DynamicTuple<std::tuple_element_t<Ints, typename T::types>...> newTuple;
    newTuple.content = std::make_tuple(std::get<Ints>(tuple.content)...);
    return newTuple;
}

//Remove the Nth elements of a tuple.
template<std::size_t N, typename T>
auto remove_Nth(T& tuple)
{
  constexpr auto size = tuple.nbTypes();
  using first = std::make_index_sequence<N>;
  using rest = offset_sequence_t<N+1,
                std::make_index_sequence<size-N-1>>;
  using indices = cat_sequence_t<first, rest>;
  return select_tuple(tuple, indices{});
}

using EntityId = std::size_t*;
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
                componentMapping[*entity][newTuple.template getIndexOfTypeT<Component>()] = newTuple.template vectorSize<Component>()-1;
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
                componentMapping[*entityId][tuple.template getIndexOfTypeT<Component>()] = tuple.template vectorSize<Component>()-1;
            }
            void addChild(EntityId rootId, EntityId parentId, EntityId child, size_t treeLevel) {
                //std::cout<<"id : "<<parentId<<std::endl;
                if (treeLevel >= nbLevels[*rootId]) {
                    nbLevels[*rootId]++;
                    for (unsigned int i = 0; i < childrenMapping.size(); i++) {
                        childrenMapping[*rootId].resize(nbLevels[*parentId]);
                    }
                }
                childrenMapping[*rootId][treeLevel].push_back(child);
                //std::cout<<"add child : "<<*child<<","<<treeLevels.size()<<","<<branchIds.size()<<std::endl;
                treeLevels[*child] = treeLevel;
                branchIds[*child] = parentId;
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
                removeInVector(branchIds, i);
            }
            EntityId getRoot(EntityId entityId) {
                EntityId parentId = entityId;
                while (treeLevels[*parentId].has_value()) {
                    parentId = branchIds[*parentId];
                }
                parentId;
            }
            bool sameBranch (size_t id1, size_t id2) {
                while (treeLevels[id2].has_value()) {
                    EntityId parent = branchIds[id2];
                    if (*parent == id1)
                        return true;
                    id2 = *parent;
                }
                return false;
            }
            void checkMaxlevels(EntityId rootId) {
                size_t maxLevel = 0;
                for (unsigned int i = 0; i < componentMapping.size(); i++) {
                    if (treeLevels[i] > maxLevel)
                        maxLevel++;
                }
                nbLevels[*rootId] = maxLevel;
            }
            void removeMapping(EntityId entityId) {

                bool found = false;
                std::vector<std::vector<std::optional<size_t>>>::iterator itToFind;
                unsigned int i;
                for (itToFind = componentMapping.begin(), i = 0; itToFind != componentMapping.end() && !found; itToFind++, i++) {

                    if (*entityId == i) {
                        found = true;
                    }
                }
                if (found) {
                    i--;
                    std::optional<size_t> treeLevel = treeLevels[*entityId];

                    //Met à jour les informations sur la branche si l'entité possèdes des enfants.
                    if (treeLevel.has_value()) {
                        EntityId rootId = getRoot(entityId);
                        size_t level = treeLevel.value();
                        unsigned int j;
                        std::vector<std::vector<std::optional<size_t>>>::iterator it;
                        //Recherche du mapping des enfants à supprimer.
                        for (it = componentMapping.begin(), j = 0; it != componentMapping.end(); j++) {
                            //si l'enfant est située à un niveau en dessous sur la même branche, on supprime le mapping!

                            if (sameBranch(*entityId, j) && treeLevels[j].has_value() && treeLevels[j].value() > level) {
                                std::vector<std::vector<EntityId>>::iterator it2;
                                unsigned int k;
                                //Supression du mapping dans la children map.
                                for (it2 = childrenMapping[*rootId].begin(), k = 0; it2 != childrenMapping[*rootId].end(); k++) {
                                    std::vector<EntityId>::iterator it3;
                                    unsigned int c;
                                    for (it3 = childrenMapping[*rootId][k].begin(), c = 0; it3 != childrenMapping[*rootId][k].end(); c++) {
                                        if (sameBranch(*entityId, *childrenMapping[*rootId][k][c]) && nbLevels[*childrenMapping[*rootId][k][c]] > level) {
                                            childrenMapping[*rootId][k].erase(it3);
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
                        if (i == *entityId) {
                            childrenMapping.erase(it);
                        } else {
                            it++;
                        }
                    }
                    componentMapping.erase(itToFind);
                    removeTreeInfos(i);
                }

            }
            static void cloneTree(ComponentMapping componentMapping, std::vector<EntityId> entities, std::vector<std::optional<size_t>> treeLevels, std::vector<EntityId> branchs) {
                EntityId rootId;
                //Recherche l'entité racine.
                for (unsigned int i = 0; i < entities.size(); i++) {
                    if (branchs[i]) {
                        rootId = entities[i];
                    }
                }
                for (unsigned int i = 0; i < entities.size(); i++) {
                    componentMapping.addChild(rootId, branchs[i], entities[i], treeLevels[i].value());
                }
            }
            public :
            template <typename T, typename DynamicTuple>
            T* getAgregate(DynamicTuple& tuple, EntityId entityId) {
                //std::cout<<"id : "<<*entityId<<","<<"size : "<<componentMapping.size()<<std::endl;
                if (componentMapping[*entityId][tuple.template getIndexOfTypeT<T>()].has_value())
                    return tuple.template get<T>(componentMapping[*entityId][tuple.template getIndexOfTypeT<T>()].value());
                return nullptr;
            }
            template <typename... Signature, typename DynamicTuple, typename System, typename... Params>
            void apply(DynamicTuple& tuple, System& system, std::vector<EntityId>& entities, std::tuple<Params...>& params) {
              for (unsigned int i = 0; i < entities.size(); i++) {
                this->template apply_impl<Signature...>(entities[i], tuple, system, params, std::index_sequence_for<Signature...>());
                for (unsigned int j = 0; j < nbLevels[*entities[i]]; j++) {
                  for(unsigned int k = 0; k < childrenMapping[*entities[i]][j].size(); k++)
                    this->template apply_impl<Signature...>(childrenMapping[*entities[i]][j][k], tuple, system, params, std::index_sequence_for<Signature...>());
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
                this->template apply_impl<Signature...>(entities[i], tuple, system, params, std::index_sequence_for<Signature...>());
                for (unsigned int j = 0; j < nbLevels[*entities[i]]; j++) {
                  for(unsigned int k = 0; k < childrenMapping[*entities[i]][j].size(); k++)
                    this->template apply_impl<Signature...>(childrenMapping[*entities[i]][j][k], tuple, system, params, std::index_sequence_for<Signature...>(), ret);
                }
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
    std::vector<std::unique_ptr<std::remove_pointer_t<EntityId>>> ids;
    EntityId createEntity() {
        nbEntities++;
        EntityId id = new std::remove_pointer_t<EntityId>(nbEntities-1);
        std::unique_ptr<std::remove_pointer_t<EntityId>> ptr;
        ptr.reset(id);
        ids.push_back(std::move(ptr));
        return id;
    }
    size_t getNbEntities() {
        return nbEntities;
    }
    private :
    void destroyEntity(EntityId id) {
        const auto itToFind =
            std::find_if(ids.begin(), ids.end(),
                         [&](auto& p) { return p.get() == id; });
        const bool found = (itToFind != ids.end());
        if (found) {
            for (auto it = itToFind; it != ids.end(); it++) {
                (**it)--;
            }
            ids.erase(itToFind);
            nbEntities--;
        }
    }
};
struct vec3 {
    vec3(float x,float y, float z) : x(x), y(y), z(z) {}
    float x, y, z;
};
struct transform_component {
    transform_component(vec3 position) : position(position) {}
    vec3 position;
};

struct MoveSystem  {
    template <typename... Components, typename... Params, class = typename std::enable_if_t<contains<transform_component*, Components...>::value>>
    void operator()(std::tuple<Components...>& tp, EntityId entityId, std::tuple<Params...>& params) {
        transform_component* tc = std::get<index<transform_component*,Components...>()>(tp);
        //std::cout<<"position : "<<tc->position.x<<","<<tc->position.y<<","<<tc->position.z<<std::endl;
    }
};
struct RenderType1 {

};
struct RenderType2 {

};
struct RenderSystemType1 {
    template <typename... Components, typename... Params, class = typename std::enable_if_t<contains<RenderType1*, Components...>::value>>
    void operator()(std::tuple<Components...>& tp, EntityId entityId, std::tuple<Params...>& params) {
        RenderType1* renderType1 = std::get<index<RenderType1*,Components...>()>(tp);
        if (renderType1 != nullptr) {
            std::cout<<"draw render type 1 "<<std::endl;
        }
    }
    template <typename... Components, typename... Params, class... D, class = typename std::enable_if_t<!contains<RenderType1*, Components...>::value>>
    void operator()(std::tuple<Components...>& tp,  EntityId entityId, std::tuple<Params...>& params) {
    }
};
struct RenderSystemType2 {
    template <typename... Components, typename... Params, class = typename std::enable_if_t<contains<RenderType2*, Components...>::value>>
    void operator()(std::tuple<Components...>& tp,  EntityId entityId, std::tuple<Params...>& params) {
        RenderType2* renderType2 = std::get<index<RenderType2*,Components...>()>(tp);
        if (renderType2 != nullptr) {
            std::cout<<"draw render type 2 "<<std::endl;
        }
    }
    template <typename... Components, typename... Params, class... D, class = typename std::enable_if_t<!contains<RenderType2*, Components...>::value>>
    void operator()(std::tuple<Components...>& tp,  EntityId entityId, std::tuple<Params...>& params) {
    }
};

struct SceneType1 {

};
struct SceneType2 {

};
struct LoadToRender {
    template <typename... Components, typename... Params>
    void operator()(std::tuple<Components...>& tp, EntityId entityId, std::tuple<Params...>& params) {
        std::cout<<"load entities from scene to render"<<std::endl;
    }
};
struct LoadSystem {
    template <typename... Components, typename... Params, class = typename std::enable_if_t<contains<SceneType1*, Components...>::value>>
    void operator()(std::tuple<Components...>& tp, EntityId entityId, std::tuple<Params...>& params) {
        SceneType1* scene = std::get<index<SceneType1*,Components...>()>(tp);
        if (scene != nullptr) {
            auto renders = std::get<3>(params);
            std::vector<EntityId> rendersIds = std::get<4>(params);
            ComponentMapping renderMapping = std::get<5>(params);
            auto tp = std::make_tuple(scene);
            LoadToRender loader;
            call_sub_system<typename std::remove_reference_t<decltype(renders)>::types>(renders, loader, renderMapping, rendersIds, params, std::make_index_sequence<renders.nbTypes()>());
        }
    }
    template <typename T, typename Array, typename System, typename Mapping, typename... Params, size_t... I>
    void call_sub_system(Array& array, System& system, Mapping& componentMapping, std::vector<EntityId> entities, std::tuple<Params...>& params, std::index_sequence<I...>) {
        componentMapping.template apply<std::tuple_element_t<I, T>...>(array, system, entities, params);
    }
    template <typename... Components, typename... Params, class... D, class = typename std::enable_if_t<!contains<SceneType1*, Components...>::value>>
    void operator()(std::tuple<Components...>& tp, EntityId entityId, std::tuple<Params...>& params) {

    }
};
struct MainSystem {
    template <size_t I=0, typename... Components, typename... Params, class = typename std::enable_if_t<(sizeof...(Components) != 0 && I == 0)>>
    void operator()(std::tuple<Components...>& tp, EntityId entityId, std::tuple<Params...>& params) {
        this->template operator()<I+1>(tp, entityId, params);
    }
    template <size_t I=0, typename... Components, typename... Params, class... D, class = typename std::enable_if_t<(sizeof...(Components) != 0 && I > 0 && I < sizeof...(Components)-1)>>
    void operator()(std::tuple<Components...>& tp, EntityId entityId, std::tuple<Params...>& params) {
        if (std::get<I>(tp) != nullptr) {
            auto& array = std::get<0>(params);
            std::vector<EntityId> entities = std::get<1>(params);
            auto& componentMapping = std::get<2>(params);
            //std::cout<<"call system : "<<std::get<I>(tp)->positionInTemplateParameterPack<<std::endl;
            call_system<typename std::remove_reference_t<decltype(array)>::types>(array, *std::get<I>(tp), componentMapping, entities, params, std::make_index_sequence<array.nbTypes()>());
        } else {
            this->template operator()<I+1>(tp, entityId, params);
        }
    }

    template <size_t I=0, typename... Components, typename... Params, class... D, class... E, class = typename std::enable_if_t<(sizeof...(Components) != 0 && I == sizeof...(Components)-1)>>
    void operator()(std::tuple<Components...>& tp, EntityId entityId, std::tuple<Params...>& params) {
        if (std::get<I>(tp) != nullptr) {
            auto& array = std::get<0>(params);
            std::vector<EntityId> entities = std::get<1>(params);
            auto& componentMapping = std::get<2>(params);
            //std::cout<<"call system : "<<std::get<I>(tp)->positionInTemplateParameterPack<<std::endl;
            call_system<typename std::remove_reference_t<decltype(array)>::types>(array, *std::get<I>(tp), componentMapping, entities, params, std::make_index_sequence<array.nbTypes()>());
        }
    }
    template <size_t I=0, typename... Components, typename... Params, class... D, class... E, class... F, class = typename std::enable_if_t<sizeof...(Components) == 0>>
    void operator()(std::tuple<Components...>& tp, EntityId entityId, std::tuple<Params...>& params) {
    }
    template <typename T, typename Array, typename System, typename Mapping, typename... Params, size_t... I>
    void call_system(Array& array, System& system, Mapping& componentMapping, std::vector<EntityId> entities, std::tuple<Params...>& params, std::index_sequence<I...>) {
        componentMapping.template apply<std::tuple_element_t<I, T>...>(array, system, entities, params);
    }

};
template <typename SceneAlias>
class World {
    public :
    enum SystemsQueues {
        MainSystemQueueIndex, RenderSystemQueueIndex, LoadSystemQueueIndex
    };
    World() {

    }
    void setCurrentScene(EntityId scene) {
        currentSceneId = scene;
    }
    EntityId getCurrentSceneId() {
        return currentSceneId;
    }
    std::vector<EntityId> getScenesIds() {
        return scenesIds;
    }
    /*template <typename SceneArray, typename SceneType>
    std::vector<CellMap*> getCasesMap(SceneArray& sceneArray) {
        return sceneMapping.getAgregate<SceneType>(sceneArray, currentSceneId)->gridMap.getCasesMap();
    }
    template <typename SceneArray, typename EntityComponentArray, SceneType>
    bool addEntity(SceneArray& sceneArray, EntityComponentArray& entityComponentArray, EntityId entity) {
        auto params = std::make_tuple(sceneMapping.getAgregate<SceneType>(sceneArray, currentSceneId));
        std::vector<bool> addeds;
        std::vector<EntityID> entityIDs(1);
        entityIds[1] = entity;
        sceneMapping.apply<TransformComponent>(entityComponentArray, AddEntityToSceneSystem(), entityIds, params, addeds);
        for(unsigned int i = 0; i < addeds.size(); i++)
            if (!addeds[i])
                return false;
        return true;
    }*/
    template <typename SystemArray>
    auto initSystems(SystemArray& systemsArray) {
        MainSystem mainSystem;
        LoadSystem loadSystem;
        RenderSystemType1 renderSystem1;
        RenderSystemType2 renderSystem2;
        auto systemsArray1 = addSystem(systemsArray, mainSystem, MainSystemQueueIndex);
        auto systemsArray2 = addSystem(systemsArray1, loadSystem, LoadSystemQueueIndex);
        auto systemsArray3 = addSystem(systemsArray2, renderSystem1, RenderSystemQueueIndex);
        auto systemsArray4 = addSystem(systemsArray3, renderSystem2, RenderSystemQueueIndex);
        return systemsArray4;
    }
    template <typename SystemArray, typename System>
    auto addSystem (SystemArray& systemArray, System system, SystemsQueues queue) {
        EntityId systemId = systemFactory.createEntity();
        auto newSystemArray = systemMapping.addFlag(systemId, systemArray, system, systemFactory);
        if (std::is_same<decltype(newSystemArray), decltype(systemArray)>::value)
            std::runtime_error("This system type is already added a system type can only be added once!");
        if (queue >= systemQueueIds.size())
            systemQueueIds.resize(queue+1);
        systemQueueIds[queue].push_back(systemId);
        return newSystemArray;
    }
    template <typename Component, typename EntityComponentArray>
    auto addEntityComponentFlag(EntityComponentArray& entityComponentArray) {
        return entityComponentMapping.template addFlag<Component>(entityComponentArray);
    }
    template <typename EntityComponentArray, typename Component, typename Factory>
    auto addEntityComponentFlag(EntityComponentArray& entityComponentArray, EntityId entityId, Component* component, Factory& factory) {
        auto newEntityComponentArray = entityComponentMapping.addFlag(entityId, entityComponentArray, component, factory);
        return newEntityComponentArray;
    }
    template <typename EntityComponentArray, typename Component, typename Factory>
    void addEntityComponentAgregate(EntityComponentArray& entityComponentArray, EntityId& entityId, Component component, Factory& factory) {
        entityComponentMapping.addAgregate(entityId, entityComponentArray, component, factory);
        auto newEntityComponentArray = entityComponentArray.add(component);
        if (!std::is_same<decltype(newEntityComponentArray), decltype(entityComponentArray)>::value) {
            std::runtime_error("Flag not found! You should call addEntityComponentFlag and get the returned array to add other components of the same type!");
        }
    }
    void addChild(EntityId rootId, EntityId parentId, EntityId childId, size_t treeLevel) {
        entityComponentMapping.addChild(rootId, parentId, childId, treeLevel);
    }
    template <typename SceneComponent, typename SceneArray>
    auto addSceneFlag(SceneArray& scenes) {
        return sceneMapping.addFlag<SceneComponent>(scenes);
    }
    template <typename SceneArray, typename SceneComponent, typename Factory>
    auto addSceneFlag(SceneArray& scenes,  EntityId& sceneId, SceneComponent scene, Factory& factory) {
        auto newScenes = scenes.add(scenes);
        sceneMapping.addFlag(sceneId, scenes, scene, factory);
        this->scenes.push_back(sceneId);
        return newScenes;
    }
    template <typename SceneArray, typename SceneComponent, typename Factory>
    void addSceneAgregate(SceneArray& scenes,  EntityId& sceneId, SceneComponent scene, Factory& factory) {
        auto newScenes = scenes.add(scene);
        if (!std::is_same<decltype(scene), decltype(newScenes)>::value) {
            std::runtime_error("Flag not found! You should call addSceneFlag and get the returned array to add other scenes of the same type!");
        }
        sceneMapping.addAgregate(sceneId, scenes, scene, factory);
    }
    template <typename RenderComponent, typename RendererArray>
    auto addRendererFlag(RendererArray& renderers) {
        return rendererMapping.addFlag<RenderComponent>(renderers);
    }
    template <typename RenderArray, typename RenderComponent, typename Factory>
    auto addRendererFlag(RenderArray& renderers, EntityId& rendererId, RenderComponent renderer, Factory& factory) {
        auto tuple = rendererMapping.addFlag(rendererId, renderers, renderer, factory);
        return tuple;
    }
    template <typename RenderArray, typename RenderComponent, typename Factory>
    void addRendererAgregate(RenderArray& renderers, EntityId& rendererId, RenderComponent renderer, Factory& factory) {
        auto newRenderers = renderers.add(renderer);
        if (!std::is_same<decltype(newRenderers), decltype(renderers)>::value) {
            std::runtime_error("Flag not found! You should call addRendererFlag and get the returned array to add other renderers of the same type!");
        }
        rendererId = factory.createEntity();
        rendererMapping.addAgregate(rendererId, renderers, renderer, factory);
        this->renderersIds.push_back(rendererId);
    }
    template <typename RenderArray, typename RenderComponent, typename Factory>
    auto addSubRendererFlag(RenderArray& renderers, EntityId rootId, EntityId parent, EntityId& child, size_t treeLevel, RenderComponent renderer, Factory& factory) {
        auto newRenderers = rendererMapping.addFlag(child, renderers, renderer, factory);
        rendererMapping.addChild(rootId, parent, child, treeLevel);
        return newRenderers;
    }
    template <typename RenderArray, typename RenderComponent, typename Factory>
    void addSubRenderAgregate(RenderArray& renderers, EntityId root, EntityId parent, EntityId& child, size_t treeLevel, RenderComponent renderer, Factory& factory) {
        rendererMapping.addAgregate(child, renderers, renderer, factory);
        rendererMapping.addChild(root, parent, child, treeLevel);
        auto newRenderers = renderers.add(renderer);
        if (!std::is_same<decltype(renderers), decltype(newRenderers)>::value) {
            std::runtime_error("Flag not found! You should call addSubRendererFlag and get the returned array to add other sub renderers of the same type!");
        }
    }
    template <typename SystemArray, typename RenderArray>
    void draw (SystemArray& systems, RenderArray& renderers) {
        draw_impl<typename SystemArray::types>(systems, renderers, std::make_index_sequence<SystemArray::nbTypes()>());
    }

    template <typename T, typename SystemArray, typename RenderArray, size_t... Ints>
    void draw_impl (SystemArray& systems, RenderArray& renderers, const std::index_sequence<Ints...>& seq) {
        auto params = std::make_tuple(renderers, renderersIds, rendererMapping);
        std::vector<EntityId> renderSystemId = systemQueueIds[RenderSystemQueueIndex];
        MainSystem main;
        systemMapping.apply<std::tuple_element_t<Ints, T>...>(systems, main, renderSystemId, params);
    }
    template <typename SystemArray, typename RenderArray, typename SceneArray>
    void toRender (SystemArray& systems, RenderArray& renderers, SceneArray& scenes) {
        toRender_impl<typename SystemArray::types>(systems, renderers, scenes, std::make_index_sequence<SystemArray::nbTypes()>());
    }

    template <typename T, typename SystemArray, typename RenderArray, typename SceneArray, size_t... Ints>
    void toRender_impl (SystemArray& systems, RenderArray& renderers, SceneArray& scenes, const std::index_sequence<Ints...>& seq) {
        std::vector<EntityId> scenesId;
        scenesId.push_back(currentSceneId);
        auto params = std::make_tuple(scenes, scenesId, sceneMapping, renderers, renderersIds, rendererMapping);
        std::vector<EntityId> loadSystemId = systemQueueIds[LoadSystemQueueIndex];
        MainSystem main;
        systemMapping.apply<std::tuple_element_t<Ints, T>...>(systems, main, loadSystemId, params);
    }
    ComponentMapping entityComponentMapping;
    ComponentMapping rendererMapping;
    ComponentMapping sceneMapping;
    ComponentMapping systemMapping;
    std::vector<EntityId> renderersIds;
    std::vector<EntityId> scenesIds;
    std::vector<std::vector<EntityId>> systemQueueIds;
    EntityId currentSceneId;
    ::EntityFactory systemFactory;
    std::map<SceneAlias, EntityId> sceneKeys;
};


int main(int argc, char* argv[]){
    ::World<std::string> world;
    DynamicTuple componentArray;
    ::EntityFactory factory;
    auto newComponentArray = world.addEntityComponentFlag<transform_component>(componentArray);

    std::vector<EntityId> entities;
    for (unsigned int i = 0; i < 1000; i++) {
        EntityId sphere = factory.createEntity();
        transform_component sphereTransform(vec3((i+1)*3, (i+1)*3, (i+1)*3));
        world.addEntityComponentAgregate(newComponentArray, sphere, sphereTransform, factory);

        transform_component rectTransform (vec3((i+1)*3+1, (i+1)*3+1, (i+1)*3+1));

        EntityId rectangle = factory.createEntity();
        world.addEntityComponentAgregate(newComponentArray, rectangle, rectTransform, factory);
        transform_component convexShapeTransform(vec3((i+1)*3+2, (i+1)*3+2, (i+1)*3+2));
        EntityId convexShape = factory.createEntity();
        world.addEntityComponentAgregate(newComponentArray, convexShape, convexShapeTransform, factory);
        world.addChild(sphere, sphere, rectangle, 0);
        world.addChild(sphere, sphere, convexShape, 0);
        entities.push_back(sphere);

    }

    MoveSystem mv;
    /*auto params =  std::make_tuple();
    mapping.template apply<transform_component>(newComponentArray, mv, entities, params);*/
    DynamicTuple systemsArray;

    auto systemsArray1 = world.initSystems(systemsArray);
    ::EntityFactory rendererFactory;
    RenderType1 render1;
    RenderType2 render2;
    DynamicTuple renderArray;
    auto renderArray1 = world.addRendererFlag<RenderType1>(renderArray);
    auto renderArray2 = world.addRendererFlag<RenderType2>(renderArray1);
    EntityId render1Id = rendererFactory.createEntity();
    EntityId render2Id = rendererFactory.createEntity();
    world.addRendererAgregate(renderArray2, render1Id, render1, rendererFactory);
    world.addRendererAgregate(renderArray2, render2Id, render2, rendererFactory);
    EntityId subRender = rendererFactory.createEntity();
    world.addSubRenderAgregate(renderArray2, render1Id, render1Id, subRender, 0, render2, rendererFactory);
    world.draw(systemsArray1, renderArray2);
    DynamicTuple sceneArray;
    auto sceneArray1 = world.addSceneFlag<SceneType1>(sceneArray);
    auto sceneArray2 = world.addSceneFlag<SceneType2>(sceneArray1);
    SceneType1 scene1;
    SceneType2 scene2;
    ::EntityFactory sceneFactory;
    EntityId sceneId1 = sceneFactory.createEntity();
    EntityId sceneId2 = sceneFactory.createEntity();
    world.addSceneAgregate(sceneArray2, sceneId1, scene1, sceneFactory);
    world.addSceneAgregate(sceneArray2, sceneId2, scene2, sceneFactory);
    world.setCurrentScene(sceneId1);
    world.toRender(systemsArray1, renderArray2, sceneArray2);
    DynamicTuple tp;
    auto tp1 = tp.addType<transform_component>();
    auto tp2 = tp1.addType<SceneType1>();
    auto tp3 = tp2.addType<SceneType2>();
    auto tp4 = tp3.addType<RenderType1>();
    auto tp5 = tp4.addType<RenderType2>();
    auto tp6 = tp5.removeType<2>();
    return 0;
    /*MyAppli app(sf::VideoMode(800, 600), "Test odfaeg");
    return app.exec();*/
}

