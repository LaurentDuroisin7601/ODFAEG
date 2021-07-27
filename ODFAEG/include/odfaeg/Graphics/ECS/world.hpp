#ifndef ODFAEG_ECS_WORLD_HPP
#define ODFAEG_ECS_WORLD_HPP
#include "../../Core/ecs.hpp"
namespace odfaeg {
    namespace graphic {
        namespace ecs {
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
        }
    }
}
#endif
