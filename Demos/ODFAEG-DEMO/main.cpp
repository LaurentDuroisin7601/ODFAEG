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

template < typename Tp >
struct contains<Tp> : std::false_type {};


template <typename T, typename U=void, typename... Types>
constexpr size_t index() {
    return std::is_same<T, U>::value ? 0 : 1 + index<T, Types...>();
}
struct IDynamicTupleElement {
    unsigned int positionInVector, positionInTemplateParameterPack;
    template <size_t I=0>
    static constexpr size_t get(size_t position) {
        if (I == position) {
            return I;
        } else {
            return get<I+1>(position);
        }
    }
};
template <template <size_t , class...> class T, size_t I, class Head, class... Tail>
Head* getElement(T<I, Head, Tail...>& tuple, unsigned int index) {
    if (index < tuple.T<I, Head>::elements.size())
        return static_cast<Head*>(tuple.T<I, Head>::elements[index]);
    return nullptr;
}
template <size_t I, class D>
struct DynamicTupleLeaf {
    std::vector<IDynamicTupleElement*> elements;
    void add(IDynamicTupleElement* element) {
        element->positionInVector = elements.size();
        element->positionInTemplateParameterPack = I;
        elements.push_back(element);
    }
};
template <size_t I, class... D>
struct DynamicTupleHolder {

};
template <size_t I, class DH, class... DT>
struct DynamicTupleHolder<I, DH, DT...> : DynamicTupleLeaf<I, DH>, DynamicTupleHolder<I+1, DT...> {
    using BaseLeaf = DynamicTupleLeaf<I, DH>;
    using BaseDT = DynamicTupleHolder<I+1, DT...>;
    void add(IDynamicTupleElement* head, size_t N) {
        if (I == N) {
            BaseLeaf::add(head);
        } else {
            BaseDT::add(head, N);
        }
    }
    template <class H, class... T>
    void copy(DynamicTupleHolder<I, H, T...>& holder) {
        BaseLeaf::elements = holder.DynamicTupleLeaf<I, H>::elements;
        BaseDT::copy(holder);
    }
};
template <size_t I, class DH>
struct DynamicTupleHolder<I, DH> : DynamicTupleLeaf<I, DH>, DynamicTupleHolder<I+1> {
    using BaseLeaf = DynamicTupleLeaf<I, DH>;
    using BaseDT = DynamicTupleHolder<I+1>;
    void add(IDynamicTupleElement* head, size_t N) {
        if (I == N) {
            BaseLeaf::add(head);
        }
    }
    template <class... T>
    void copy(DynamicTupleHolder<I>& holder) {

    }
};
template <size_t I>
struct DynamicTupleHolder<I> : DynamicTupleLeaf<I, IDynamicTupleElement> {
    void add(DH* head, size_t N) {

    }
};
template <typename... TupleTypes>
struct DynamicTuple {
    DynamicTupleHolder<0, TupleTypes...> contents;
    using types = typename std::tuple<TupleTypes...>;
    template <typename H, class = typename std::enable_if_t<contains<H, TupleTypes...>::value>>
    DynamicTuple add (H* head) {
        contents.add(head, index<H, TupleTypes...>());
        return *this;
    }
    template <typename H, class = typename std::enable_if_t<!contains<H, TupleTypes...>::value>>
    DynamicTuple <TupleTypes..., H> add (H* head) {
        DynamicTuple<TupleTypes..., H> tuple;
        tuple.contents.template copy<TupleTypes...>(contents);
        return tuple.add(head);
    }
    template <typename H, class = typename std::enable_if_t<!contains<H, TupleTypes...>::value>>
    DynamicTuple <TupleTypes..., H> addType () {
        DynamicTuple<TupleTypes..., H> tuple;
        tuple.contents.template copy<TupleTypes...>(contents);
        return tuple;
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
        return getElement<DynamicTupleLeaf, I>(contents, containerIdx);
    }
    template <size_t I>
    auto get(unsigned int containerIdx) {
        return getElement<DynamicTupleLeaf, I>(contents, containerIdx);
    }
    auto get(unsigned int positionInTemplateParameterPack, unsigned int containerIdx) {
        return get<IDynamicTupleElement::get(positionInTemplateParameterPack)>(containerIdx);
    }
};
template <size_t I=0, class... Types>
struct elements {
};
template <size_t I, class H, class... T>
struct elements <I, H, T...> : elements<I+1, T...> {
    using type = H;
};
template <size_t I, class H>
struct elements <I, H> {
    using type = H;
};
template <size_t I, class... Types>
struct element_type {
    using type = typename elements<I, Types...>::type;
};
struct IComponent : IDynamicTupleElement {
};
using EntityId = std::size_t*;
class ComponentMapping {
    template <class>
    friend class World;
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
        branchIds.resize(factory.getNbEntities());
        treeLevels.resize(factory.getNbEntities());
        for (unsigned int i = 0; i < componentMapping.size(); i++) {
            componentMapping[i].resize(newTuple.nbTypes());
        }
        componentMapping[*entity][component->positionInTemplateParameterPack] = component->positionInVector;
        return newTuple;
    }
    template <typename DynamicTuple, typename Component, typename Factory>
    void addAgregate(EntityId entityId, DynamicTuple& tuple, Component* component, Factory& factory) {
        tuple.add(component);
        componentMapping.resize(factory.getNbEntities());
        childrenMapping.resize(factory.getNbEntities());
        nbLevels.resize(factory.getNbEntities());
        treeLevels.resize(factory.getNbEntities());
        branchIds.resize(factory.getNbEntities());
        for (unsigned int i = 0; i < componentMapping.size(); i++) {
            componentMapping[i].resize(tuple.nbTypes());
        }
        componentMapping[*entityId][component->positionInTemplateParameterPack] = component->positionInVector;
    }
    void addChild(EntityId parentId, EntityId child, size_t treeLevel) {
        //std::cout<<"id : "<<parentId<<std::endl;
        if (treeLevel >= nbLevels[*parentId]) {
            nbLevels[*parentId]++;
            for (unsigned int i = 0; i < childrenMapping.size(); i++) {
                childrenMapping[*parentId].resize(nbLevels[*parentId]);
            }
        }
        childrenMapping[*parentId][treeLevel].push_back(child);
        //std::cout<<"add child : "<<*child<<","<<treeLevels.size()<<","<<branchIds.size()<<std::endl;
        treeLevels[*child] = treeLevel;
        branchIds[*child] = parentId;
    }
    void remove(size_t i) {

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
    void removeMapping(EntityId entityId) {

        bool found = false;
        std::vector<std::vector<std::optional<size_t>>>::iterator itToFind;
        unsigned int i;
        for (itToFind = componentMapping.begin(), i = 0; itToFind != componentMapping.end() && !found; itToFind++, i++) {

            if (*entityId == i) {
                std::cout<<"found id : "<<*entityId<<"i : "<<i<<std::endl;
                found = true;
            }
        }
        if (found) {
            i--;
            std::optional<size_t> treeLevel = treeLevels[*entityId];

            //Met à jour les informations sur la branche si l'entité possèdes des enfants.
            if (treeLevel.has_value()) {
                EntityId branch = branchIds[*entityId];
                size_t level = treeLevel.value();
                unsigned int j;
                std::vector<std::vector<std::optional<size_t>>>::iterator it;
                for (it = componentMapping.begin(), j = 0; it != componentMapping.end(); j++) {
                    //si l'enfant est située à un niveau en dessous sur la même branche, on le supprime!

                    if (branch == branchIds[j] && treeLevels[j].has_value() && treeLevels[j].value() > level) {

                        nbLevels[*branch]--;
                        std::vector<std::vector<EntityId>>::iterator it2;
                        unsigned int k;
                        for (it2 = childrenMapping[*branch].begin(), k = 0; it2 != childrenMapping[*branch].end(); k++) {
                            std::vector<EntityId>::iterator it3;
                            unsigned int c;
                            for (it3 = childrenMapping[*branch][k].begin(), c = 0; it3 != childrenMapping[*branch][k].end(); c++) {
                                if (branch == childrenMapping[*branch][k][c] && nbLevels[*childrenMapping[*branch][k][c]] > level) {
                                    childrenMapping[*branch][k].erase(it3);
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
            }
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
        system(tp, params);
    }
    private :
    std::vector<std::vector<std::optional<size_t>>> componentMapping;
    std::vector<std::vector<std::vector<EntityId>>> childrenMapping;
    std::vector<size_t> nbLevels;
    std::vector<std::optional<size_t>> treeLevels;
    std::vector<EntityId> branchIds;
    };
class EntityFactory {
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
struct transform_component : IComponent {
    transform_component(vec3 position) : position(position) {}
    vec3 position;
};
struct ISystem : IDynamicTupleElement {

};
struct MoveSystem : ISystem {
    template <typename... Components, typename... Params, class = typename std::enable_if_t<contains<transform_component*, Components...>::value>>
    void operator()(std::tuple<Components...>& tp, std::tuple<Params...>& params) {
        transform_component* tc = std::get<index<transform_component*,Components...>()>(tp);
        //std::cout<<"position : "<<tc->position.x<<","<<tc->position.y<<","<<tc->position.z<<std::endl;
    }
};
struct RenderType1 : IComponent {

};
struct RenderType2 : IComponent {

};
struct RenderSystemType1 : ISystem {
    template <typename... Components, typename... Params, class = typename std::enable_if_t<contains<RenderType1*, Components...>::value>>
    void operator()(std::tuple<Components...>& tp, std::tuple<Params...>& params) {
        RenderType1* renderType1 = std::get<index<RenderType1*,Components...>()>(tp);
        if (renderType1 != nullptr) {
            std::cout<<"draw render type 1 "<<std::endl;
        }
    }
    template <typename... Components, typename... Params, class... D, class = typename std::enable_if_t<!contains<RenderType1*, Components...>::value>>
    void operator()(std::tuple<Components...>& tp, std::tuple<Params...>& params) {
    }
};
struct RenderSystemType2 : ISystem {
    template <typename... Components, typename... Params, class = typename std::enable_if_t<contains<RenderType2*, Components...>::value>>
    void operator()(std::tuple<Components...>& tp, std::tuple<Params...>& params) {
        RenderType2* renderType2 = std::get<index<RenderType2*,Components...>()>(tp);
        if (renderType2 != nullptr) {
            std::cout<<"draw render type 2 "<<std::endl;
        }
    }
    template <typename... Components, typename... Params, class... D, class = typename std::enable_if_t<!contains<RenderType2*, Components...>::value>>
    void operator()(std::tuple<Components...>& tp, std::tuple<Params...>& params) {
    }
};

struct SceneType1 : IComponent {

};
struct SceneType2 : IComponent {

};
struct LoadToRender : ISystem {
    template <typename... Components, typename... Params>
    void operator()(std::tuple<Components...>& tp, std::tuple<Params...>& params) {
        std::cout<<"load entities from scene to render"<<std::endl;
    }
};
struct LoadSystem : ISystem {
    template <typename... Components, typename... Params, class = typename std::enable_if_t<contains<SceneType1*, Components...>::value>>
    void operator()(std::tuple<Components...>& tp, std::tuple<Params...>& params) {
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
    void operator()(std::tuple<Components...>& tp, std::tuple<Params...>& params) {

    }
};
struct MainSystem : ISystem {
    template <size_t I=0, typename... Components, typename... Params, class = typename std::enable_if_t<(sizeof...(Components) != 0 && I == 0)>>
    void operator()(std::tuple<Components...>& tp, std::tuple<Params...>& params) {
        this->template operator()<I+1>(tp, params);
    }
    template <size_t I=0, typename... Components, typename... Params, class... D, class = typename std::enable_if_t<(sizeof...(Components) != 0 && I > 0 && I < sizeof...(Components)-1)>>
    void operator()(std::tuple<Components...>& tp, std::tuple<Params...>& params) {
        if (std::get<I>(tp) != nullptr) {
            auto& array = std::get<0>(params);
            std::vector<EntityId> entities = std::get<1>(params);
            auto& componentMapping = std::get<2>(params);
            //std::cout<<"call system : "<<std::get<I>(tp)->positionInTemplateParameterPack<<std::endl;
            call_system<typename std::remove_reference_t<decltype(array)>::types>(array, *std::get<I>(tp), componentMapping, entities, params, std::make_index_sequence<array.nbTypes()>());
        } else {
            this->template operator()<I+1>(tp, params);
        }
    }

    template <size_t I=0, typename... Components, typename... Params, class... D, class... E, class = typename std::enable_if_t<(sizeof...(Components) != 0 && I == sizeof...(Components)-1)>>
    void operator()(std::tuple<Components...>& tp, std::tuple<Params...>& params) {
        if (std::get<I>(tp) != nullptr) {
            auto& array = std::get<0>(params);
            std::vector<EntityId> entities = std::get<1>(params);
            auto& componentMapping = std::get<2>(params);
            //std::cout<<"call system : "<<std::get<I>(tp)->positionInTemplateParameterPack<<std::endl;
            call_system<typename std::remove_reference_t<decltype(array)>::types>(array, *std::get<I>(tp), componentMapping, entities, params, std::make_index_sequence<array.nbTypes()>());
        }
    }
    template <size_t I=0, typename... Components, typename... Params, class... D, class... E, class... F, class = typename std::enable_if_t<sizeof...(Components) == 0>>
    void operator()(std::tuple<Components...>& tp, std::tuple<Params...>& params) {
    }
    template <typename T, typename Array, typename System, typename Mapping, typename... Params, size_t... I>
    void call_system(Array& array, System& system, Mapping& componentMapping, std::vector<EntityId> entities, std::tuple<Params...>& params, std::index_sequence<I...>) {
        componentMapping.template apply<std::tuple_element_t<I, T>...>(array, system, entities, params);
    }

};
template <typename SceneAlias>
class World {
    template <typename T, typename P>
    bool containsUniquePtr (std::vector<std::unique_ptr<T>>& upointers, P* rpointer) {
        const auto itToFind =
            std::find_if(upointers.begin(), upointers.end(),
                         [&](auto& p) { return p.get() == rpointer; });
        const bool found = (itToFind != upointers.end());
        return found;
    }
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
        MainSystem* mainSystem = new MainSystem();
        LoadSystem* loadSystem = new LoadSystem();
        RenderSystemType1* renderSystem1 = new RenderSystemType1();
        RenderSystemType2* renderSystem2 = new RenderSystemType2();
        auto systemsArray1 = addSystem(systemsArray, mainSystem, MainSystemQueueIndex);
        auto systemsArray2 = addSystem(systemsArray1, loadSystem, LoadSystemQueueIndex);
        auto systemsArray3 = addSystem(systemsArray2, renderSystem1, RenderSystemQueueIndex);
        auto systemsArray4 = addSystem(systemsArray3, renderSystem2, RenderSystemQueueIndex);
        return systemsArray4;
    }
    template <typename SystemArray, typename System>
    auto addSystem (SystemArray& systemArray, System* system, SystemsQueues queue) {
        if (containsUniquePtr(systems, system))
            std::runtime_error("This system is already added, a system can only be added once");
        EntityId systemId = systemFactory.createEntity();
        auto newSystemArray = systemMapping.addFlag(systemId, systemArray, system, systemFactory);
        if (std::is_same<decltype(newSystemArray), decltype(systemArray)>::value)
            std::runtime_error("This system type is already added a system type can only be added once!");
        if (queue >= systemQueueIds.size())
            systemQueueIds.resize(queue+1);
        systemQueueIds[queue].push_back(systemId);
        std::unique_ptr<ISystem> ptr;
        ptr.reset(system);
        systems.push_back(std::move(ptr));
        return newSystemArray;
    }
    template <typename Component, typename EntityComponentArray>
    auto addEntityComponentFlag(EntityComponentArray& entityComponentArray) {
        return entityComponentMapping.template addFlag<Component>(entityComponentArray);
    }
    template <typename EntityComponentArray, typename Component, typename Factory>
    auto addEntityComponentFlag(EntityComponentArray& entityComponentArray, EntityId entityId, Component* component, Factory& factory) {
        entityId = factory.createEntity();
        auto newEntityComponentArray = entityComponentMapping.addFlag(entityId, entityComponentArray, component, factory);
        if (!containsUniquePtr(components, component)) {
            std::unique_ptr<IComponent> ptr;
            ptr.reset(component);
            components.push_back(std::move(ptr));
        }
        return newEntityComponentArray;
    }
    template <typename EntityComponentArray, typename Component, typename Factory>
    void addEntityComponentAgregate(EntityComponentArray& entityComponentArray, EntityId& entityId, Component* component, Factory& factory) {
        entityId = factory.createEntity();
        entityComponentMapping.addAgregate(entityId, entityComponentArray, component, factory);
        auto newEntityComponentArray = entityComponentArray.add(component);
        if (!std::is_same<decltype(newEntityComponentArray), decltype(entityComponentArray)>::value) {
            std::runtime_error("Flag not found! You should call addEntityComponentFlag and get the returned array to add other components of the same type!");
        }
        if (!containsUniquePtr(components, component)) {
            std::unique_ptr<IComponent> ptr;
            ptr.reset(component);
            components.push_back(std::move(ptr));
        }
    }
    void addChild(EntityId parentId, EntityId childId, size_t treeLevel) {
        entityComponentMapping.addChild(parentId, childId, treeLevel);
    }
    template <typename SceneComponent, typename SceneArray>
    auto addSceneFlag(SceneArray& scenes) {
        return sceneMapping.addFlag<SceneComponent>(scenes);
    }
    template <typename SceneArray, typename SceneComponent, typename Factory>
    auto addSceneFlag(SceneArray& scenes,  EntityId& sceneId, SceneComponent* scene, Factory& factory) {
        if (containsUniquePtr(components, scene))
            std::runtime_error("This scene is already added, a scene can only be added once");
        sceneId = factory.createEntity();
        auto newScenes = scenes.add(scenes);
        sceneMapping.addFlag(sceneId, scenes, scene, factory);
        std::unique_ptr<IComponent> ptr;
        ptr.reset(scene);
        this->scenes.push_back(sceneId);
        components.push_back(std::move(ptr));
        return newScenes;
    }
    template <typename SceneArray, typename SceneComponent, typename Factory>
    void addSceneAgregate(SceneArray& scenes,  EntityId& sceneId, SceneComponent* scene, Factory& factory) {
        if (containsUniquePtr(components, scene))
            std::runtime_error("This scene is already added, a scene can only be added once");
        sceneId = factory.createEntity();
        auto newScenes = scenes.add(scene);
        if (!std::is_same<decltype(scene), decltype(newScenes)>::value) {
            std::runtime_error("Flag not found! You should call addSceneFlag and get the returned array to add other scenes of the same type!");
        }
        sceneMapping.addAgregate(sceneId, scenes, scene, factory);
        std::unique_ptr<IComponent> ptr;
        ptr.reset(scene);
        this->scenesIds.push_back(sceneId);
        components.push_back(std::move(ptr));
    }
    template <typename RenderComponent, typename RendererArray>
    auto addRendererFlag(RendererArray& renderers) {
        return rendererMapping.addFlag<RenderComponent>(renderers);
    }
    template <typename RenderArray, typename RenderComponent, typename Factory>
    auto addRendererFlag(RenderArray& renderers, EntityId& rendererId, RenderComponent* renderer, Factory& factory) {
        if (containsUniquePtr(components, renderer))
            std::runtime_error("This renderer is already added, a renderer can only be added once, if you want to make a sub pass call addSubRenderer");
        rendererId = factory.createEntity();
        auto tuple = rendererMapping.addFlag(rendererId, renderers, renderer, factory);
        rendererMapping.addAgregate(rendererId, renderers, renderer, factory);
        std::unique_ptr<IComponent> ptr;
        ptr.reset(renderer);
        renderersIds.push_back(rendererId);
        components.push_back(std::move(ptr));
        return tuple;
    }
    template <typename RenderArray, typename RenderComponent, typename Factory>
    void addRendererAgregate(RenderArray& renderers, EntityId& rendererId, RenderComponent* renderer, Factory& factory) {
        if (containsUniquePtr(components, renderer))
            std::runtime_error("This renderer is already added, a renderer can only be added once");
        auto newRenderers = renderers.add(renderer);
        if (!std::is_same<decltype(newRenderers), decltype(renderers)>::value) {
            std::runtime_error("Flag not found! You should call addRendererFlag and get the returned array to add other renderers of the same type!");
        }
        rendererId = factory.createEntity();
        rendererMapping.addAgregate(rendererId, renderers, renderer, factory);
        this->renderersIds.push_back(rendererId);
        std::unique_ptr<IComponent> ptr;
        ptr.reset(renderer);
        components.push_back(std::move(ptr));
    }
    template <typename RenderArray, typename RenderComponent, typename Factory>
    auto addSubRendererFlag(RenderArray& renderers, EntityId parent, EntityId& child, size_t treeLevel, RenderComponent* renderer, Factory& factory) {
        child = factory.createEntity();
        auto newRenderers = rendererMapping.addFlag(child, renderers, renderer, factory);
        rendererMapping.addChild(parent, child, treeLevel);

        if (!containsUniquePtr(components, renderer)) {
            std::unique_ptr<IComponent> ptr;
            ptr.reset(renderer);
            components.push_back(std::move(ptr));
        }
        return newRenderers;
    }
    template <typename RenderArray, typename RenderComponent, typename Factory>
    void addSubRenderAgregate(RenderArray& renderers, EntityId parent, EntityId& child, size_t treeLevel, RenderComponent* renderer, Factory& factory) {
        child = factory.createEntity();
        rendererMapping.addAgregate(child, renderers, renderer, factory);
        rendererMapping.addChild(parent, child, treeLevel);
        auto newRenderers = renderers.add(renderer);
        if (!std::is_same<decltype(renderers), decltype(newRenderers)>::value) {
            std::runtime_error("Flag not found! You should call addSubRendererFlag and get the returned array to add other sub renderers of the same type!");
        }
        if (!containsUniquePtr(components, renderer)) {
            std::unique_ptr<IComponent> ptr;
            ptr.reset(renderer);
            components.push_back(std::move(ptr));
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
        systemMapping.apply<std::tuple_element_t<Ints, T>...>(systems, *static_cast<MainSystem*>(this->systems[MainSystemQueueIndex].get()), renderSystemId, params);
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
        systemMapping.apply<std::tuple_element_t<Ints, T>...>(systems, *static_cast<MainSystem*>(this->systems[MainSystemQueueIndex].get()), loadSystemId, params);
    }
    ComponentMapping entityComponentMapping;
    ComponentMapping rendererMapping;
    ComponentMapping sceneMapping;
    ComponentMapping systemMapping;
    std::vector<std::unique_ptr<ISystem>> systems;
    std::vector<std::unique_ptr<IComponent>> components;
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
    std::vector<IComponent*> components;
    transform_component* sphereTransform = new transform_component(vec3(0, 0, 0));
    auto newComponentArray = world.addEntityComponentFlag<transform_component>(componentArray);

    std::vector<EntityId> entities;
    for (unsigned int i = 0; i < 1000; i++) {
        EntityId sphere;
        transform_component* sphereTransform = new transform_component(vec3((i+1)*3, (i+1)*3, (i+1)*3));
        world.addEntityComponentAgregate(newComponentArray, sphere, sphereTransform, factory);

        transform_component* rectTransform = new transform_component(vec3((i+1)*3+1, (i+1)*3+1, (i+1)*3+1));
        EntityId rectangle;
        world.addEntityComponentAgregate(newComponentArray, rectangle, rectTransform, factory);
        transform_component* convexShapeTransform = new transform_component(vec3((i+1)*3+2, (i+1)*3+2, (i+1)*3+2));
        EntityId convexShape;
        world.addEntityComponentAgregate(newComponentArray, convexShape, convexShapeTransform, factory);
        world.addChild(sphere, rectangle, 0);
        world.addChild(sphere, convexShape, 0);
        entities.push_back(sphere);
    }
    std::cout<<"sphere created"<<std::endl;
    MoveSystem mv;
    /*auto params =  std::make_tuple();
    mapping.template apply<transform_component>(newComponentArray, mv, entities, params);*/
    DynamicTuple systemsArray;

    auto systemsArray1 = world.initSystems(systemsArray);
    ::EntityFactory rendererFactory;
    RenderType1* render1 = new RenderType1();
    RenderType2* render2 = new RenderType2();
    DynamicTuple renderArray;
    auto renderArray1 = world.addRendererFlag<RenderType1>(renderArray);
    auto renderArray2 = world.addRendererFlag<RenderType2>(renderArray1);
    EntityId render1Id;
    EntityId render2Id;
    world.addRendererAgregate(renderArray2, render1Id, render1, rendererFactory);
    world.addRendererAgregate(renderArray2, render2Id, render2, rendererFactory);
    EntityId subRender;
    world.addSubRenderAgregate(renderArray2, render1Id, subRender, 0, render2, rendererFactory);
    world.draw(systemsArray1, renderArray2);
    DynamicTuple sceneArray;
    auto sceneArray1 = world.addSceneFlag<SceneType1>(sceneArray);
    auto sceneArray2 = world.addSceneFlag<SceneType2>(sceneArray1);
    SceneType1* scene1 = new SceneType1();
    SceneType2* scene2 = new SceneType2();
    ::EntityFactory sceneFactory;
    EntityId sceneId1;
    EntityId sceneId2;
    world.addSceneAgregate(sceneArray2, sceneId1, scene1, sceneFactory);
    world.addSceneAgregate(sceneArray2, sceneId2, scene2, sceneFactory);
    world.setCurrentScene(sceneId1);
    world.toRender(systemsArray1, renderArray2, sceneArray2);
    return 0;
    /*MyAppli app(sf::VideoMode(800, 600), "Test odfaeg");
    return app.exec();*/
}

