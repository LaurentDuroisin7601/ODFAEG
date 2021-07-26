#include "application.h"
#include "odfaeg/Core/mp.hpp"

using namespace odfaeg::core;
using namespace odfaeg::math;
using namespace odfaeg::physic;
using namespace odfaeg::graphic;
using namespace odfaeg::window;
using namespace odfaeg::audio;
using namespace sorrok;
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
            componentMapping[i].resize(newTuple.nbTypes());
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
        if (componentMapping[entityId][tuple.template getIndexOfTypeT<T>()].has_value()) {
            //std::cout<<"component : "<<tuple.template get<T>(componentMapping[entityId][tuple.template getIndexOfTypeT<T>()].value())<<std::endl;
            return tuple.template get<T>(componentMapping[entityId][tuple.template getIndexOfTypeT<T>()].value());
        }
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



struct EFactory {
    size_t nbEntities=0;
    EntityId createEntity() {
        nbEntities++;
        return nbEntities-1;
    }
    size_t getNbEntities() {
        return nbEntities;
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
        std::cout<<"position : "<<tc->position.x<<","<<tc->position.y<<","<<tc->position.z<<std::endl;
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
struct World {
    enum SystemsQueues {
        MainSystemQueueIndex, RenderSystemQueueIndex, LoadSystemQueueIndex
    };
    World() {

    }
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
        EntityId systemId = systemFactory.createEntity();
        auto newSystemArray = systemMapping.addFlag(systemId, systemArray, system, systemFactory);
        if (queue >= systemQueueIds.size())
            systemQueueIds.resize(queue+1);
        systemQueueIds[queue].push_back(systemId);
        std::unique_ptr<ISystem> ptr;
        ptr.reset(system);
        systems.push_back(std::move(ptr));
        return newSystemArray;
    }
    template <typename Scene, typename SceneArray>
    auto addSceneFlag(SceneArray& scenes) {
        auto newSceneArray = scenes.template addType<Scene>();
        this->scenes.resize(newSceneArray.nbTypes());
        return newSceneArray;
    }
    template <typename SceneArray, typename SceneComponent, typename Factory>
    void addSceneAgregate(SceneArray& scenes,  EntityId sceneId, SceneComponent* scene, Factory& factory) {
        scenes.add(scene);
        sceneMapping.addAgregate(sceneId, scenes, scene, factory);
        std::unique_ptr<IComponent> ptr;
        ptr.reset(scene);
        this->scenes[scene->positionInTemplateParameterPack].push_back(sceneId);
        components.push_back(std::move(ptr));
    }
    void setCurrentScene(EntityId scene) {
        currentScene = scene;
    }
    template <typename Render, typename RenderArray>
    auto addRenderFlag(RenderArray& renders) {
        auto newRenderArray = renderMapping.addFlag<Render>(renders);
        this->renders.resize(newRenderArray.nbTypes());
        return newRenderArray;
    }
    template <typename RenderArray, typename RenderComponent, typename Factory>
    auto addRenderFlag(RenderArray& renders, EntityId renderId, RenderComponent* render, Factory& factory) {
        auto tuple = renderMapping.addFlag(renderId, renders, render, factory);
        renderMapping.addAgregate(renderId, renders, render, factory);
        std::unique_ptr<IComponent> ptr;
        ptr.reset(render);
        this->renders[render->positionInTemplateParameterPack];
        this->renders[render->positionInTemplateParameterPack].push_back(renderId);
        components.push_back(std::move(ptr));
        return tuple;
    }
    template <typename RenderArray, typename RenderComponent, typename Factory>
    void addRenderAgregate(RenderArray& renders, EntityId renderId, RenderComponent* render, Factory& factory) {
        renderMapping.addAgregate(renderId, renders, render, factory);
        std::unique_ptr<IComponent> ptr;
        ptr.reset(render);
        this->renders[render->positionInTemplateParameterPack].push_back(renderId);
        components.push_back(std::move(ptr));
    }
    template <typename RenderArray, typename RenderComponent, typename Factory>
    void addSubRenderAgregate(RenderArray& renders, EntityId parent, EntityId child, size_t treeLevel, RenderComponent* render, Factory& factory) {
        renderMapping.addAgregate(child, renders, render, factory);
        renderMapping.addChild(parent, child, treeLevel);
    }
    template <typename SystemArray, typename RenderArray>
    void draw (SystemArray& systems, RenderArray& renderers) {
        draw_impl<typename SystemArray::types, RenderArray::nbTypes()>(systems, renderers, std::make_index_sequence<SystemArray::nbTypes()>());
    }

    template <typename T, size_t I, typename SystemArray, typename RenderArray, size_t... Ints, class = std::enable_if_t<(I > 1)>>
    void draw_impl (SystemArray& systems, RenderArray& renderers, const std::index_sequence<Ints...>& seq) {
        auto params = std::make_tuple(renderers, renders[I-1], renderMapping);
        std::vector<EntityId> renderSystemId = systemQueueIds[RenderSystemQueueIndex];
        systemMapping.apply<std::tuple_element_t<Ints, T>...>(systems, *static_cast<MainSystem*>(this->systems[MainSystemQueueIndex].get()), renderSystemId, params);
        draw_impl<T, I-1>(systems, renderers, seq);
    }
    template <typename T, size_t I, typename SystemArray, typename RenderArray, size_t... Ints, class... D, class = std::enable_if_t<I == 1>>
    void draw_impl (SystemArray& systems, RenderArray& renderers, const std::index_sequence<Ints...>& seq) {
        auto params = std::make_tuple(renderers, renders[I-1], renderMapping);
        std::vector<EntityId> renderSystemId = systemQueueIds[RenderSystemQueueIndex];
        systemMapping.apply<std::tuple_element_t<Ints, T>...>(systems, *static_cast<MainSystem*>(this->systems[MainSystemQueueIndex].get()), renderSystemId, params);
    }
    template <typename T, size_t I, typename SystemArray, typename RenderArray, size_t... Ints, class... D, class... E, class = std::enable_if_t<I == 0>>
    void draw_impl (SystemArray& systems, RenderArray& renderers, const std::index_sequence<Ints...>& seq) {

    }
    template <typename SystemArray, typename RenderArray, typename SceneArray>
    void toRender (SystemArray& systems, RenderArray& renderers, SceneArray& scenes) {
        toRender_impl<typename SystemArray::types, RenderArray::nbTypes()>(systems, renderers, scenes, std::make_index_sequence<SystemArray::nbTypes()>());
    }

    template <typename T, size_t I, typename SystemArray, typename RenderArray, typename SceneArray, size_t... Ints, class = std::enable_if_t<(I > 1)>>
    void toRender_impl (SystemArray& systems, RenderArray& renderers, SceneArray& scenes, const std::index_sequence<Ints...>& seq) {
        std::vector<EntityId> scenesId;
        scenesId.push_back(currentScene);
        auto params = std::make_tuple(scenes, scenesId, sceneMapping, renderers, renders[I-1], renderMapping);
        std::vector<EntityId> loadSystemId = systemQueueIds[LoadSystemQueueIndex];
        systemMapping.apply<std::tuple_element_t<Ints, T>...>(systems, *static_cast<MainSystem*>(this->systems[MainSystemQueueIndex].get()), loadSystemId, params);
        toRender_impl<T, I-1>(systems, renderers, scenes, seq);
    }

    template <typename T, size_t I, typename SystemArray, typename RenderArray, typename SceneArray, size_t... Ints, class... D, class = std::enable_if_t<I == 1>>
    void toRender_impl (SystemArray& systems,RenderArray& renderers, SceneArray& scenes, const std::index_sequence<Ints...>& seq) {
        std::vector<EntityId> scenesId;
        scenesId.push_back(currentScene);
        auto params = std::make_tuple(scenes, scenesId, sceneMapping, renderers, renders[I-1], renderMapping);
        std::vector<EntityId> loadSystemId = systemQueueIds[LoadSystemQueueIndex];
        systemMapping.apply<std::tuple_element_t<Ints, T>...>(systems, *static_cast<MainSystem*>(this->systems[MainSystemQueueIndex].get()), loadSystemId, params);
    }
    template <typename T, size_t I, typename SystemArray, typename RenderArray, typename SceneArray,  size_t... Ints, class... D, class... E, class = std::enable_if_t<I == 0>>
    void toRender_impl (SystemArray& systems, RenderArray& renderers, SceneArray& scenes, const std::index_sequence<Ints...>& seq) {

    }
    ComponentMapping renderMapping;
    ComponentMapping sceneMapping;
    ComponentMapping systemMapping;
    std::vector<std::unique_ptr<ISystem>> systems;
    std::vector<std::unique_ptr<IComponent>> components;
    std::vector<std::vector<EntityId>> renders;
    std::vector<std::vector<EntityId>> scenes;
    std::vector<std::vector<EntityId>> systemQueueIds;
    EntityId currentScene;
    EFactory systemFactory;

};


int main(int argc, char* argv[]){
    DynamicTuple componentArray;
    EFactory factory;
    EntityId sphere = factory.createEntity();
    ComponentMapping mapping;
    std::vector<IComponent*> components;
    transform_component* sphereTransform = new transform_component(vec3(0, 0, 0));
    auto newComponentArray = mapping.addFlag<transform_component>(componentArray);
    mapping.addAgregate(sphere, newComponentArray, sphereTransform, factory);
    EntityId rectangle = factory.createEntity();
    transform_component* rectTransform = new transform_component(vec3(1, 1, 1));
    mapping.addAgregate(rectangle, newComponentArray, rectTransform, factory);
    transform_component* convexShapeTransform = new transform_component(vec3(2, 2, 2));
    EntityId convexShape = factory.createEntity();
    mapping.addAgregate(convexShape, newComponentArray, convexShapeTransform, factory);
    mapping.addChild(sphere, rectangle, 0);
    mapping.addChild(sphere, convexShape, 0);
    components.push_back(sphereTransform);
    components.push_back(rectTransform);
    components.push_back(convexShapeTransform);
    std::vector<EntityId> entities;
    entities.push_back(sphere);
    for (unsigned int i = 1; i < 1000; i++) {
        EntityId rectangle = factory.createEntity();
        EntityId convexShape = factory.createEntity();
        mapping.addChild(sphere, rectangle, 0);
        mapping.addChild(sphere, convexShape, 0);
        sphere = factory.createEntity();
        sphereTransform = new transform_component(vec3((i+1)*3, (i+1)*3, (i+1)*3));
        mapping.addAgregate(sphere, newComponentArray, sphereTransform, factory);
        rectTransform = new transform_component(vec3((i+1)*3+1, (i+1)*3+1, (i+1)*3+1));
        mapping.addAgregate(rectangle, newComponentArray, rectTransform, factory);
        convexShapeTransform = new transform_component(vec3((i+1)*3+2, (i+1)*3+2, (i+1)*3+2));
        mapping.addAgregate(convexShape, newComponentArray, convexShapeTransform, factory);
        components.push_back(sphereTransform);
        components.push_back(rectTransform);
        components.push_back(convexShapeTransform);
        entities.push_back(sphere);
    }
    MoveSystem mv;
    auto params =  std::make_tuple();
    mapping.template apply<transform_component>(newComponentArray, mv, entities, params);
    DynamicTuple systemsArray;
    ::World world;
    auto systemsArray1 = world.initSystems(systemsArray);
    EFactory renderFactory;
    RenderType1* render1 = new RenderType1();
    RenderType2* render2 = new RenderType2();
    DynamicTuple renderArray;
    auto renderArray1 = world.addRenderFlag<RenderType1>(renderArray);
    auto renderArray2 = world.addRenderFlag<RenderType2>(renderArray1);
    EntityId render1Id = renderFactory.createEntity();
    EntityId render2Id = renderFactory.createEntity();
    world.addRenderAgregate(renderArray2, render1Id, render1, renderFactory);
    world.addRenderAgregate(renderArray2, render2Id, render2, renderFactory);
    EntityId subRender = renderFactory.createEntity();
    world.addSubRenderAgregate(renderArray2, render1Id, subRender, 0, render2, renderFactory);
    world.draw(systemsArray1, renderArray2);
    DynamicTuple sceneArray;
    auto sceneArray1 = world.addSceneFlag<SceneType1>(sceneArray);
    auto sceneArray2 = world.addSceneFlag<SceneType2>(sceneArray1);
    SceneType1* scene1 = new SceneType1();
    SceneType2* scene2 = new SceneType2();
    EFactory sceneFactory;
    EntityId sceneId1 = sceneFactory.createEntity();
    EntityId sceneId2 = sceneFactory.createEntity();
    world.addSceneAgregate(sceneArray2, sceneId1, scene1, sceneFactory);
    world.addSceneAgregate(sceneArray2, sceneId2, scene2, sceneFactory);
    world.setCurrentScene(sceneId1);
    world.toRender(systemsArray1, renderArray2, sceneArray2);
    for (unsigned int i = 0; i < components.size(); i++)
        delete components[i];
    components.clear();
    return 0;
    /*MyAppli app(sf::VideoMode(800, 600), "Test odfaeg");
    return app.exec();*/
}

