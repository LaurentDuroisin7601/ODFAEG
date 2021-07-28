#ifndef ODFAEG_ECS_WORLD_HPP
#define ODFAEG_ECS_WORLD_HPP
#include "../../Core/ecs.hpp"
namespace odfaeg {
    namespace graphic {
        namespace ecs {
            template <typename SceneAlias>
            class World {
                public :
                enum SystemsQueues {
                    MainSystemQueueIndex, RendererSystemQueueIndex, SubRendererSytemQueueIndex, LoadSystemQueueIndex
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
                template <typename SceneArray, typename SceneType>
                std::vector<GridCase*> getGridCases(SceneArray& sceneArray) {
                    return sceneMapping.getAgregate<SceneType>(sceneArray, currentSceneId)->gridMap.getGridCases();
                }
                template <typename EntityComponentArray, typename SceneType>
                bool addEntity(SceneArray& sceneArray, EntityComponentArray& entityComponentArray, EntityId entity) {
                    auto params = std::make_tuple(entityComponentArray, entityComponentMapping, entityId));
                    std::vector<bool> addeds;
                    std::vector<EntityID> scenesIDs(1);
                    scenesIDs[0] = currentSceneId;
                    AddEntityToSceneSystem system;
                    sceneMapping.apply<SceneType>(sceneArray, system, sceneIDs, params, addeds);
                    for(unsigned int i = 0; i < addeds.size(); i++)
                        if (!addeds[i])
                            return false;
                    return true;
                }
                template <typename SceneArray, typename EntityComponentArray, typename SceneType>
                bool removeEntity(EntityId entity, SceneArray& sceneArray, EntityComponentArray& entityComponentArray) {
                    auto params = std::make_tuple(sceneMapping.getAgregate<SceneType>(sceneArray, entityComponentMapping, entityId));
                    std::vector<bool> removeds;
                    std::vector<EntityID> scenesIDs(1);
                    scenesIDs[0] = currentSceneId;
                    RemoveEntityFromSceneSystem system;
                    sceneMapping.apply<SceneType>(sceneArray, system, entityIds, params, removeds);
                    for(unsigned int i = 0; i < removeds.size(); i++)
                        if (!removeds[i])
                            return false;
                    return true;
                }
                template <typename SceneArray, typename EntityComponentArray, typename SceneType, typename Factory>
                bool destroyEntity(EntityId entityId, SceneArray& sceneArray, EntityComponentArray& entityComponentArray, Factory& factory) {
                    auto params = std::make_tuple(sceneMapping.getAgregate<SceneType>(entityComponentArray, entityComponentMapping, entityId));
                    std::vector<bool> removeds;
                    std::vector<EntityID> scenesIDs(1);
                    scenesIDs[0] = currentSceneId;
                    RemoveEntityFromSceneSystem system;
                    sceneMapping.apply<SceneType>(sceneArray, system, scenesIds, params, removeds);
                    entityComponentMapping.removeMapping(entity);
                    return factory.destroyEntity(entityId);
                }
                template <typename SceneArray, typename EntityComponentArray, typename SceneType>
                std::vector<EntityId> getVisisibleEntities(SceneArray& sceneArray, EntityComponentArray& entityComponentArray, std::string expression) {
                    auto params = std::make_tuple(entityComponentArray, entityComponentMapping, expression);
                    GetVisibleEntitiesSystem system;
                    std::vector<EntityId> visibleEntities;
                    std::vector<EntityID> scenesIDs(1);
                    scenesIDs[0] = currentSceneId;
                    sceneMapping.apply<SceneType>(sceneArray, system, scenesIDs, params, visibleEntities);
                    return visibleEntities;
                }
                template <typename SceneArray, typename EntityComponentArray, typename SceneType>
                void moveEntity(SceneArray& sceneArray, EntityComponentArray& entityComponentArray, EntityId entity, float x, float y, float z) {
                    auto params = std::make_tuple(entityComponentArray, entityComponentMapping, x, y, z);
                    MoveSystem system;
                    std::vector<EntityID> scenesIDs(1);
                    scenesIDs[0] = currentSceneId;
                    sceneMapping.apply<SceneType>(sceneArray, system, scenesIds, params);
                }
                template <typename SceneArray, typename EntityComponentArray, typename SceneType>
                bool collide(SceneArray& sceneArray, EntityComponentArray& entityComponentArray, EntityId* entityId) {
                    auto params = std::make_tuple(entityComponentArray, entityComponentMapping, entityId);
                    CollideSystem system;
                    std::vector<EntityID> scenesIDs(1);
                    scenesIDs[0] = currentSceneId;
                    sceneMapping.apply<SceneType>(sceneArray, system, sceneIds, params);
                }
                template <typename SceneArray, typename EntityComponentArray, typename SceneType>
                bool collide(SceneArray& sceneArray, EntityComponentArray& entityComponentArray, EntityId* entityId, math::Vec3f position) {
                    auto params = std::make_tuple(entityComponentArray, entityComponentMapping, entityId, position);
                    CollideSystem system;
                    std::vector<EntityID> scenesIDs(1);
                    scenesIDs[0] = currentSceneId;
                    sceneMapping.apply<SceneType>(sceneArray, system, sceneIds, params);
                }
                template <typename SceneArray, typename EntityComponentArray, typename SceneType>
                bool collide(SceneArray& sceneArray, EntityComponentArray& entityComponentArray, EntityId* entityId, math::Ray ray) {
                    auto params = std::make_tuple(entityComponentArray, entityComponentMapping, entityId, ray);
                    CollideSystem system;
                    std::vector<EntityID> scenesIDs(1);
                    scenesIDs[0] = currentSceneId;
                    sceneMapping.apply<SceneType>(sceneArray, system, sceneIds, params);
                }
                template <typename SceneArray, typename EntityComponentArray, typename SceneType>
                void generate_map(SceneArray& sceneArray, EntityComponentArray& entityComponentArray, std::vector<EntityId> tGround, std::vector<EntityId> tWall, math::Vec2f tileSize, physic::BoundingBox zone) {
                    auto params = std::make_tuple(entityComponentArray, entityComponentMapping, entityId, tGround, tWall, tileSize, zone);
                    std::vector<EntityID> scenesIDs(1);
                    scenesIDs[0] = currentSceneId;
                    GenerateMapSystem system;
                    sceneMapping.apply<SceneType>(sceneArray, system, currentSceneId, params);
                }
                template <typename RenderArray, typename RenderType>
                void drawOnComponents(RendererArray& rendererArray, std::string expression, EntityId entityId) {
                    auto renderer = rendererMapping.getAgregate<RendererType>(rendererArray, entityId);
                    if (renderer) {
                        renderer->typesToDraw = expression;
                    }
                }
                template <typename SceneArray, typename SceneType, typename EntityComponentArray, typename SceneType>
                std::vector<EntityId> getRootEntities(SceneArray& sceneArray, std::string expression) {
                    auto params = std::make_tuple(entityComponentArray, entityComponentMapping, expression);
                    std::vector<EntityId> rootEntities;
                    std::vector<EntityId> scenesIDs(1);
                    scenesIDs[0] = currentSceneId;
                    GetRootEntitiesSystem system;
                    sceneMapping.apply(sceneArray, system, scenesIDs, params, rootEntities);
                    return rootEntities;
                }
                template <typename SceneArray, typename SceneType, typename EntityComponentArray, typename SceneType>
                std::vector<EntityId> getChildrenEntities(SceneArray& sceneArray, std::string expression) {
                    auto params = std::make_tuple(entityComponentArray, entityComponentMapping, expression);
                    std::vector<EntityId> childrenEntities;
                    std::vector<EntityId> scenesIDs(1);
                    scenesIDs[0] = currentSceneId;
                    GetChildrenEntitiesSystem system;
                    sceneMapping.apply(sceneArray, system, scenesIDs, params, rootEntities);
                    return childrenEntities;
                }
                template <typename SceneArray, typename SceneType, typename EntityComponentArray>
                std::vector<math::Vec3f> getPath(SceneArray& sceneArray, EntityComponentArray& componentArray, EntityId entity, math::Vec3f finalPos) {
                    return sceneMapping.getAgregate<SceneType>(sceneArray, currentSceneId)->grid.getPath(componentArray, componentMapping, entity, finalPos);
                }
                template <typename SceneType, typename EntityComponentArray>
                std::vector<math::Vec3f> getGridCaseAt(SceneArray& sceneArray, EntityComponentArray& componentArray, math::Vec3f position) {
                    return sceneMapping.getAgregate<SceneType>(sceneArray, currentSceneId)->grid.getGridCaseAt(componentArray, componentMapping, position);
                }
                template <typename SceneType, typename EntityComponentArray>
                BaseChangementMatrix getBaseChangementMatrix(SceneArray& sceneArray, EntityComponentArray& componentArray) {
                    return sceneMapping.getAgregate<SceneType>(sceneArray, currentSceneId)->grid.getBaseChangementMatrix();
                }
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
                    this->scenes.push_back(sceneId);
                    sceneMapping.addAgregate(sceneId, scenes, scene, factory);
                }
                template <typename RenderComponent, typename RendererArray>
                auto addRendererFlag(RendererArray& renderers) {
                    return rendererMapping.addFlag<RenderComponent>(renderers);
                }
                template <typename RenderArray, typename RenderComponent, typename Factory>
                auto addRendererFlag(RenderArray& renderers, EntityId& rendererId, RenderComponent renderer, Factory& factory) {
                    auto tuple = rendererMapping.addFlag(rendererId, renderers, renderer, factory);
                    renderersIds.push_back(rendererId);
                    return tuple;
                }
                template <typename RenderArray, typename RenderComponent, typename Factory>
                void addRendererAgregate(RenderArray& renderers, EntityId& rendererId, RenderComponent renderer, Factory& factory) {
                    rendererMapping.addAgregate(rendererId, renderers, renderer, factory);
                    renderersIds.push_back(rendererId);
                }
                template <typename RenderArray, typename RenderComponent, typename Factory>
                auto addSubRendererFlag(RenderArray& renderers, EntityId root, Entity parentId, Entity childId, size_t treeLevel, RenderComponent renderer, Factory& factory) {
                    auto newRenderers = rendererMapping.addFlag(child, renderers, renderer, factory);
                    rendererMapping.addChild(root, parent, child, treeLevel);
                    return newRenderers;
                }
                template <typename RenderArray, typename RenderComponent, typename Factory>
                void addSubRenderAgregate(RenderArray& renderers, EntityId root, EntityId parent, EntityId& child, size_t treeLevel, RenderComponent renderer, Factory& factory) {
                    rendererMapping.addAgregate(child, renderers, renderer, factory);
                    rendererMapping.addChild(root, parent, child, treeLevel);
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
                    //System which call each systems of a system queue.
                    MainSystem main;
                    systemMapping.apply<std::tuple_element_t<Ints, T>...>(systems, main, loadSystemId, params);
                }
                template <typename SceneArray, typename EntityComponentArray, typename SceneType>
                std::vector<EntityId> getEntitiesInRect(SceneArray& sceneArray, EntityComponentArray& entityComponentArray, physic::BoundingBox rect, std::string expression) {
                    std::vector<EntityId> entities;
                    auto params = std::make_tuple(entityComponentArray, entityComponentMapping, rect, expression);
                    std::vector<EntityId> scenesId;
                    scenesId.push_back(currentSceneId);
                    GetEntitiesInBoxSystem system;
                    sceneMapping.apply(sceneArray, system, scenesIds, params, entities);
                    return entities;
                }
                template <typename SceneArray, typename SceneType>
                math::Vec3f getCoordinatesAt(SceneArray& sceneArray, math::Vec3f point) {
                    return sceneMapping.getAgregate<SceneType>(sceneArray, currentSceneId)->grid.getCoordinatesAt(point);
                }
                ComponentMapping entityComponentMapping;
                ComponentMapping rendererMapping;
                ComponentMapping sceneMapping;
                ComponentMapping systemMapping;
                std::vector<EntityId> renderersIds;
                std::vector<EntityId> scenesIds;
                std::vector<std::vector<EntityId>> systemQueueIds;
                EntityId currentSceneId;
                EntityFactory systemFactory;
                std::map<SceneAlias, EntityId> sceneKeys;
            };
        }
    }
}
#endif
