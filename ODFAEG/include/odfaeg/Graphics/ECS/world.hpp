#ifndef ODFAEG_ECS_WORLD_HPP
#define ODFAEG_ECS_WORLD_HPP
#include "../../Core/ecs.hpp"
namespace odfaeg {
    namespace graphic {
        namespace ecs {
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
                return newSceneArray;
            }
            template <typename SceneArray, typename SceneComponent, typename Factory>
            void addSceneAgregate(SceneArray& scenes,  EntityId sceneId, SceneComponent* scene, Factory& factory) {
                scenes.add(scene);
                sceneMapping.addAgregate(sceneId, scenes, scene, factory);
                std::unique_ptr<IComponent> ptr;
                ptr.reset(scene);
                this->scenes.push_back(sceneId);
                components.push_back(std::move(ptr));
            }
            void setCurrentScene(EntityId scene) {
                currentScene = scene;
            }
            template <typename Render, typename RenderArray>
            auto addRenderFlag(RenderArray& renders) {
                auto newRenderArray = renderMapping.addFlag<Render>(renders);
                return newRenderArray;
            }
            template <typename RenderArray, typename RenderComponent, typename Factory>
            auto addRenderFlag(RenderArray& renders, EntityId renderId, RenderComponent* render, Factory& factory) {
                auto tuple = renderMapping.addFlag(renderId, renders, render, factory);
                renderMapping.addAgregate(renderId, renders, render, factory);
                std::unique_ptr<IComponent> ptr;
                ptr.reset(render);
                this->renders.push_back(renderId);
                components.push_back(std::move(ptr));
                return tuple;
            }
            template <typename RenderArray, typename RenderComponent, typename Factory>
            void addRenderAgregate(RenderArray& renders, EntityId renderId, RenderComponent* render, Factory& factory) {
                renderMapping.addAgregate(renderId, renders, render, factory);
                std::unique_ptr<IComponent> ptr;
                ptr.reset(render);
                this->renders.push_back(renderId);
                components.push_back(std::move(ptr));
            }
            template <typename RenderArray, typename RenderComponent, typename Factory>
            void addSubRenderAgregate(RenderArray& renders, EntityId parent, EntityId child, size_t treeLevel, RenderComponent* render, Factory& factory) {
                renderMapping.addAgregate(child, renders, render, factory);
                renderMapping.addChild(parent, child, treeLevel);
            }
            template <typename SystemArray, typename RenderArray>
            void draw (SystemArray& systems, RenderArray& renderers) {
                draw_impl<typename SystemArray::types>(systems, renderers, std::make_index_sequence<SystemArray::nbTypes()>());
            }

            template <typename T, typename SystemArray, typename RenderArray, size_t... Ints>
            void draw_impl (SystemArray& systems, RenderArray& renderers, const std::index_sequence<Ints...>& seq) {
                auto params = std::make_tuple(renderers, renders, renderMapping);
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
                scenesId.push_back(currentScene);
                auto params = std::make_tuple(scenes, scenesId, sceneMapping, renderers, renders/*[I-1]*/, renderMapping);
                std::vector<EntityId> loadSystemId = systemQueueIds[LoadSystemQueueIndex];
                systemMapping.apply<std::tuple_element_t<Ints, T>...>(systems, *static_cast<MainSystem*>(this->systems[MainSystemQueueIndex].get()), loadSystemId, params);
            }
            ComponentMapping renderMapping;
            ComponentMapping sceneMapping;
            ComponentMapping systemMapping;
            std::vector<std::unique_ptr<ISystem>> systems;
            std::vector<std::unique_ptr<IComponent>> components;
            std::vector<EntityId> renders;
            std::vector<EntityId> scenes;
            std::vector<std::vector<EntityId>> systemQueueIds;
            EntityId currentScene;
            EFactory systemFactory;

        };
    }
}
#endif
