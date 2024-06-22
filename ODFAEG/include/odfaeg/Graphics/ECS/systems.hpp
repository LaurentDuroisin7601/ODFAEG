#ifndef ODFAEG_ECS_SYSTEM_HPP
#define ODFAEG_ECS_SYSTEM_HPP
#include "../../Core/tmp.hpp"
#include "../../Core/ecs.hpp"
#include <thread>
namespace odfaeg {
    namespace graphic {
        namespace ecs {
            //Call the systems with the given system's IDs.
            struct MainSystem {
                template <size_t I=0, typename... Components, typename T2>
                void operator()(EntityId entity, T2& params, typename std::enable_if_t<std::tuple_size_v<decltype(std::get<2>(params))> != 0 && I < std::tuple_size_v<decltype(std::get<2>(params))>-1>) {
                    auto& systems = std::get<2>(params);
                    if (getComponent<std::tuple_element<I, decltype(systems)>>(entity) != nullptr) {
                        std::vector<EntityId> entities = std::get<0>(params);
                        auto& componentMapping = std::get<1>(params);

                        //std::cout<<"call system : "<<std::get<I>(tp)->positionInTemplateParameterPack<<std::endl;
                        /*std::thread t(
                        &MainSystem::call_system<Components...,
                        std::remove_reference_t<decltype(params)>
                        entities, params);
                        t.detach();*/
                        call_system<Components...>(getComponent<std::tuple_element<I, decltype(systems)>>(entity), componentMapping, entities, params);
                    }
                    this->template operator()<I+1>(entity, params);
                }
                template <size_t I=0, typename... Components, typename T2, class... D>
                void operator()(EntityId entity, T2& params, typename std::enable_if_t<std::tuple_size_v<decltype(std::get<2>(params))> != 0 && I == std::tuple_size_v<decltype(std::get<2>(params))>-1>)  {
                    auto& systems = std::get<2>(params);
                    if (getComponent<std::tuple_element<I, decltype(systems)>>(entity) != nullptr) {
                        std::vector<EntityId> entities = std::get<0>(params);
                        auto& componentMapping = std::get<1>(params);
                        /*std::thread t(
                        &MainSystem::call_system<Components...,
                        std::remove_reference_t<decltype(params)>
                        (this, componentMapping, entities, params, std::make_index_sequence<array.nbTypes()>() );
                        t.detach();*/
                        call_system<Components...>(getComponent<std::tuple_element<I, decltype(systems)>>(entity), componentMapping, entities, params);
                    }
                }
                template <size_t I=0, typename... Components,typename T2, class... D, class... E, class... F>
                void operator()(EntityId entityId, T2& params, typename std::enable_if_t<std::tuple_size_v<decltype(std::get<2>(params))> == 0>) {
                }
                template <typename... Components, typename System, typename Mapping, typename T2, size_t... I>
                void call_system(System system, Mapping componentMapping, std::vector<EntityId> entities, T2& params) {
                    auto tp1 = core::remove_Nth_vals<0>(params);
                    auto tp2 = core::remove_Nth_vals<0>(tp1);
                    auto removed_params = core::remove_Nth_vals<0>(tp2);
                    componentMapping.template apply<Components...>(system, entities, removed_params);
                }
            };

            //Déplace des entités.
            struct MoveSystem {
                template <typename... Components, typename T, class = typename std::enable_if_t<core::contains<TransformComponent, Components...>::value>>
                void operator()(EntityId entityId, T& params) {
                    TransformComponent* tc = getComponent<TransformComponent>(entityId);
                    math::Vec3f newPosition = std::get<0>(params);
                    if (tc != nullptr) {
                        math::Vec3f t = newPosition - tc->position;
                        tc->position += t;
                        tc->center += t;
                        tc->transformMatrix.setTranslation(tc->center);
                        tc->globalBounds = tc->localBounds.transform(tc->transformMatrix);
                    }
                    MeshComponent* mc = getComponent<MeshComponent>(entityId);
                    if (mc != nullptr) {
                        tc->transformMatrix.update();
                        for (unsigned int i = 0; i < mc->faces.size(); i++) {
                            mc->faces[i].setTransformMatrix(tc->transformMatrix);
                        }
                    }
                }
                template <typename... Components, typename T, class... D, class = typename std::enable_if_t<!core::contains<TransformComponent, Components...>::value>>
                void operator()(EntityId entityId, T& params) {

                }
            };
            struct ResizeSystem {
                template <typename... Components, typename T, class = typename std::enable_if_t<core::contains<TransformComponent, Components...>::value>>
                void operator()(EntityId entityId, T& params) {
                    TransformComponent* tc = getComponent<TransformComponent>(entityId);
                    math::Vec3f size = std::get<0>(params);
                    if (tc != nullptr) {
                        math::Vec3f scale;
                        if (tc->size.x == 0 && size.x == 0) {
                            scale.x = 0;
                        } else if (tc->size.x == 0) {
                            scale.x = 1;
                            tc->size.x = size.x;

                            tc->localBounds.setSize(tc->size.x, tc->localBounds.getHeight(), tc->localBounds.getDepth());
                        } else {
                            scale.x = size.x / tc->localBounds.getWidth();
                        }
                        if (tc->size.y == 0 && size.y == 0) {
                            scale.y = 0;
                        } else if (tc->size.y == 0) {
                            /*if (name == "WALL") {
                                std::cout<<"change local bounds! "<<std::endl;
                            }*/
                            scale.y = 1;
                            tc->size.y = size.y;
                            tc->localBounds.setSize(tc->localBounds.getWidth(), tc->size.y, tc->localBounds.getDepth());
                        } else {
                            scale.y = size.y / tc->localBounds.getHeight();
                        }
                        if (tc->size.z == 0 && size.z == 0) {
                            scale.z = 0;
                        } else if (tc->size.z == 0) {
                            scale.z = 1;
                            tc->size.z = size.z;
                            tc->localBounds.setSize(tc->localBounds.getWidth(), tc->localBounds.getHeight(), tc->size.z);
                        } else {
                            scale.z = size.z / tc->localBounds.getDepth();
                        }
                        tc->scale = scale;
                        tc->transformMatrix.setScale(math::Vec3f(scale.x, scale.y, scale.z));
                        tc->globalBounds = tc->localBounds.transform(tc->transformMatrix);
                        tc->size = tc->localBounds.getSize() * tc->scale;
                        tc->position = tc->globalBounds.getPosition();
                    }
                    MeshComponent* mc = getComponent<MeshComponent>(entityId);
                    if (mc != nullptr) {
                        tc->transformMatrix.update();
                        for (unsigned int i = 0; i < mc->faces.size(); i++) {
                            mc->faces[i].setTransformMatrix(tc->transformMatrix);
                        }
                    }
                }
                template <typename... Components, typename T, class... D, class = typename std::enable_if_t<!core::contains<TransformComponent, Components...>::value>>
                void operator()(EntityId entityId, T& params) {
                }
            };
            struct RotationSystem {
                template <typename... Components, typename T, class = typename std::enable_if_t<core::contains<TransformComponent, Components...>::value>>
                void operator()(EntityId entityId, T& params) {
                    TransformComponent* tc = getComponent<TransformComponent>(entityId);
                    if (tc != nullptr) {
                        auto angle = std::get<0>(params);
                        tc->rotation = angle;
                        tc->transformMatrix.setRotation(math::Vec3f(0, 0, 1), angle);
                        tc->localBounds.transform(tc->transformMatrix);
                        //physic::BoundingBox bounds = getGlobalBounds();
                        /*std::cout<<"get global bounds : "<<getGlobalBounds().getPosition()<<getGlobalBounds().getSize();
                        std::cout<<"global bounds : "<<globalBounds.getPosition()<<globalBounds.getSize();*/
                        /*if (getGlobalBounds().getPosition() != globalBounds.getPosition()
                            || getGlobalBounds().getSize() != globalBounds.getSize())
                            std::cout<<"different"<<std::endl;*/
                        tc->size = tc->globalBounds.getSize();
                        tc->position = tc->globalBounds.getPosition();
                    }
                    MeshComponent* mc = getComponent<MeshComponent>(entityId);
                    if (mc != nullptr) {
                        tc->transformMatrix.update();
                        for (unsigned int i = 0; i < mc->faces.size(); i++) {
                            mc->faces[i].setTransformMatrix(tc->transformMatrix);
                        }
                    }
                }
                template <typename... Components, typename T, class... D, class = typename std::enable_if_t<!core::contains<TransformComponent, Components...>::value>>
                void operator()(EntityId entityId, T& params) {
                }
            };
            struct UpdateTransformSystem {
                template <typename... Components, typename T, class = typename std::enable_if_t<core::contains<TransformComponent, Components...>::value>>
                void operator()(EntityId entityId, T& params) {
                    TransformComponent* tc = getComponent<TransformComponent>(entityId);
                    tc->transformMatrix.update();
                }
                template <typename... Components, typename T, class...D, class = typename std::enable_if_t<!core::contains<TransformComponent, Components...>::value>>
                void operator()(EntityId entityId, T& params) {
                }
            };
        }
    }
}
#endif
