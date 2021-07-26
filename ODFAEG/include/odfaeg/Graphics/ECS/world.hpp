#ifndef ODFAEG_ECS_WORLD_HPP
#define ODFAEG_ECS_WORLD_HPP
#include "../../Core/ecs.hpp"
namespace odfaeg {
    namespace graphic {
        namespace ecs {
            class World {
                public :
                World() {

                }
                template <typename SystemArray>
                auto initSystems(SystemArray& systemsArray) {
                    MainSystem* mainSystem = new MainSystem();
                    LoadSystem* loadSystem = new LoadSystem();
                    RenderSystemType1* renderSystem1 = new RenderSystemType1();
                    RenderSystemType2* renderSystem2 = new RenderSystemType2();
                    auto systemsArray1 = systemMapping.addFlag<MainSystem>(systemsArray);
                    auto systemsArray2 = systemMapping.addFlag<LoadSystem>(systemsArray1);
                    auto systemsArray3 = systemMapping.addFlag<RenderSystemType1>(systemsArray2);
                    auto systemsArray4 = systemMapping.addFlag<RenderSystemType2>(systemsArray3);
                    std::unique_ptr<ISystem> ptr;
                    ptr.reset(mainSystem);
                    std::unique_ptr<ISystem> ptr1;
                    ptr1.reset(loadSystem);
                    std::unique_ptr<ISystem> ptr2;
                    ptr2.reset(renderSystem1);
                    std::unique_ptr<ISystem> ptr3;
                    ptr3.reset(renderSystem2);
                    systems.push_back(std::move(ptr));
                    systems.push_back(std::move(ptr1));
                    systems.push_back(std::move(ptr2));
                    systems.push_back(std::move(ptr3));
                    this->mainSystem = systemFactory.createEntity();
                    this->loadSystem = systemFactory.createEntity();
                    this->renderSystem1 = systemFactory.createEntity();
                    this->renderSystem2 = systemFactory.createEntity();
                    systemMapping.addAgregate(this->mainSystem, systemsArray4, mainSystem, systemFactory);
                    systemMapping.addAgregate(this->loadSystem, systemsArray4, loadSystem, systemFactory);
                    systemMapping.addAgregate(this->renderSystem1, systemsArray4, renderSystem1, systemFactory);
                    systemMapping.addAgregate(this->renderSystem2, systemsArray4, renderSystem2, systemFactory);


                    return systemsArray4;
                }
                template <typename SystemArray, typename System>
                auto addSubSystem (SystemArray& systemArray, System* system, EntityId parentSystem, size_t treeLevel, EntityId& subSystemId) {
                    subSystemId = systemFactory.createEntity();
                    auto newSystemArray = systemMapping.addFlag(subSystemId, systemArray, system, systemFactory);
                    systemMapping.addChild(parentSystem, subSystemId, 0);
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
                    this->renders[render->positionInTemplateParameterPack].resize(factory.getNbEntities());
                    this->renders[render->positionInTemplateParameterPack][renderId] = renderId;
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
                    std::vector<EntityId> renderSystemId;
                    renderSystemId.push_back(renderSystem1);
                    renderSystemId.push_back(renderSystem2);
                    systemMapping.apply<std::tuple_element_t<Ints, T>...>(systems, *static_cast<MainSystem*>(this->systems[MainSystemIndex].get()), renderSystemId, params);
                    draw_impl<T, I-1>(systems, renderers, seq);
                }
                template <typename T, size_t I, typename SystemArray, typename RenderArray, size_t... Ints, class... D, class = std::enable_if_t<I == 1>>
                void draw_impl (SystemArray& systems, RenderArray& renderers, const std::index_sequence<Ints...>& seq) {
                    auto params = std::make_tuple(renderers, renders[I-1], renderMapping);
                    std::vector<EntityId> renderSystemId;
                    renderSystemId.push_back(renderSystem1);
                    renderSystemId.push_back(renderSystem2);
                    systemMapping.apply<std::tuple_element_t<Ints, T>...>(systems, *static_cast<MainSystem*>(this->systems[MainSystemIndex].get()), renderSystemId, params);
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
                    std::vector<EntityId> loadSystemId;
                    loadSystemId.push_back(loadSystem);
                    systemMapping.apply<std::tuple_element_t<Ints, T>...>(systems, *static_cast<MainSystem*>(this->systems[MainSystemIndex].get()), loadSystemId, params);
                    toRender_impl<T, I-1>(systems, renderers, scenes, seq);
                }

                template <typename T, size_t I, typename SystemArray, typename RenderArray, typename SceneArray, size_t... Ints, class... D, class = std::enable_if_t<I == 1>>
                void toRender_impl (SystemArray& systems,RenderArray& renderers, SceneArray& scenes, const std::index_sequence<Ints...>& seq) {
                    std::vector<EntityId> scenesId;
                    scenesId.push_back(currentScene);
                    auto params = std::make_tuple(scenes, scenesId, sceneMapping, renderers, renders[I-1], renderMapping);
                    std::vector<EntityId> loadSystemId;
                    loadSystemId.push_back(loadSystem);
                    systemMapping.apply<std::tuple_element_t<Ints, T>...>(systems, *static_cast<MainSystem*>(this->systems[MainSystemIndex].get()), loadSystemId, params);
                }
                template <typename T, size_t I, typename SystemArray, typename RenderArray, typename SceneArray,  size_t... Ints, class... D, class... E, class = std::enable_if_t<I == 0>>
                void toRender_impl (SystemArray& systems, RenderArray& renderers, SceneArray& scenes, const std::index_sequence<Ints...>& seq) {

                }
                private :
                ComponentMapping entityComponentMapping;
                ComponentMapping renderMapping;
                ComponentMapping sceneMapping;
                ComponentMapping systemMapping;
                std::vector<std::unique_ptr<ISystem>> systems;
                std::vector<std::unique_ptr<IComponent>> components;
                std::vector<std::vector<EntityId>> renders;
                std::vector<std::vector<EntityId>> scenes;
                EntityId currentScene;

                EntityId mainSystem;
                EntityId loadSystem;
                EntityId renderSystem1;
                EntityId renderSystem2;

                EFactory systemFactory;
                enum Systems {
                    MainSystemIndex
                };
            };
        }
    }
}
#endif
