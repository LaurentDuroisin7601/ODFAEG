#ifndef ODFAEG_ECS_SYSTEM_HPP
#define ODFAEG_ECS_SYSTEM_HPP
#include "../../Core/tmp.hpp"
#include "../../Core/ecs.hpp"
#include <thread>
namespace odfaeg {
    namespace graphic {
        namespace ecs {
            template <size_t I, typename T1>
            concept TUPLENOTLASTELEM = requires {
                typename std::enable_if_t<std::tuple_size_v<T1> != 0 && I < std::tuple_size_v<T1>-1>;
            };
            template <size_t I, typename T1>
            concept TUPLELASTELEM = requires {
                typename std::enable_if_t<std::tuple_size_v<T1> != 0 && I == std::tuple_size_v<T1>-1>;
            };
            template <size_t I, typename T1>
            concept TUPLEEMPTY = requires  {
                typename std::enable_if_t<std::tuple_size_v<T1> == 0>;
            };
            //Call the systems with the given system's IDs.
            struct MainSystem {
                template <size_t I, typename... Components, typename T1, typename T2> requires TUPLENOTLASTELEM<I, T1>
                void operator()(EntityId entity, ComponentMapping& componentMapping, std::vector<EntityId> entities, T1& systems, T2& params) {

                    if (componentMapping.getComponent<std::tuple_element_t<I, T1>>(entity) != nullptr) {

                        call_system<Components...>(*componentMapping.getComponent<std::tuple_element_t<I, T1>>(entity), componentMapping, entities, params);
                    }
                    this->template operator()<I+1>(entity, componentMapping, entities, systems, params);
                }
                template <size_t I, typename... Components, typename T1, typename T2> requires TUPLELASTELEM<I, T1>
                void operator()(EntityId entity, ComponentMapping& componentMapping, std::vector<EntityId> entities, T1& systems, T2& params) {
                    if (componentMapping.getComponent<std::tuple_element_t<I, T1>>(entity) != nullptr) {

                        call_system<Components...>(*componentMapping.getComponent<std::tuple_element_t<I, T1>>(entity), componentMapping, entities, params);
                    }
                }
                template <size_t I, typename... Components, typename T1, typename T2> requires TUPLEEMPTY<I, T1>
                void operator()(EntityId entity, ComponentMapping& componentMapping, std::vector<EntityId> entities, T1& systems, T2& params) {
                }
                template <typename... Components, typename System, typename Mapping, typename T2>
                void call_system(System system, Mapping componentMapping, std::vector<EntityId> entities, T2& params) {
                    componentMapping.template apply<Components...>(system, entities, params);
                }
            };
            template <typename T, typename... Components>
            concept CONTAINS = requires {
                typename std::enable_if_t<core::contains<T, Components...>::value>;
            };
            template <typename T, typename... Components>
            concept NOTCONTAINS = requires {
                typename std::enable_if_t<!core::contains<T, Components...>::value>;
            };
            //Déplace des entités.
            struct MoveSystem {
                template <typename... Components, typename T> requires CONTAINS<TransformComponent, Components...>
                void operator()(EntityId entityId, ComponentMapping& cmapping, T& params) {
                    TransformComponent* tc = cmapping.getComponent<TransformComponent>(entityId);
                    math::Vec3f newPosition = std::get<0>(params);
                    if (tc != nullptr) {
                        math::Vec3f t = newPosition - tc->position;
                        tc->position += t;
                        tc->center += t;
                        tc->transformMatrix.setTranslation(tc->center);
                        tc->globalBounds = tc->localBounds.transform(tc->transformMatrix);
                    }
                    MeshComponent* mc = cmapping.getComponent<MeshComponent>(entityId);
                    if (mc != nullptr) {
                        tc->transformMatrix.update();
                        for (unsigned int i = 0; i < mc->faces.size(); i++) {
                            mc->faces[i].setTransformMatrix(tc->transformMatrix);
                        }
                    }
                    ColliderComponent* cc = cmapping.getComponent<ColliderComponent>(entityId);
                    if (cc != nullptr) {
                        cc->boundingVolume->move(newPosition - tc->position);
                    }
                }
                template <typename... Components, typename T> requires NOTCONTAINS<TransformComponent, Components...>
                void operator()(EntityId entityId, T& params) {

                }
            };
            struct ResizeSystem {
                template <typename... Components, typename T> requires CONTAINS<TransformComponent, Components...>
                void operator()(EntityId entityId, ComponentMapping& cmapping, T& params) {
                    TransformComponent* tc = cmapping.getComponent<TransformComponent>(entityId);
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
                    MeshComponent* mc = cmapping.getComponent<MeshComponent>(entityId);
                    if (mc != nullptr) {
                        tc->transformMatrix.update();
                        for (unsigned int i = 0; i < mc->faces.size(); i++) {
                            mc->faces[i].setTransformMatrix(tc->transformMatrix);
                        }
                    }
                }
                template <typename... Components, typename T> requires NOTCONTAINS<TransformComponent, Components...>
                void operator()(EntityId entityId, T& params) {
                }
            };
            struct RotationSystem {
                template <typename... Components, typename T> requires CONTAINS<TransformComponent, Components...>
                void operator()(EntityId entityId, ComponentMapping& cmapping, T& params) {
                    TransformComponent* tc = cmapping.getComponent<TransformComponent>(entityId);
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
                    MeshComponent* mc = cmapping.getComponent<MeshComponent>(entityId);
                    if (mc != nullptr) {
                        tc->transformMatrix.update();
                        for (unsigned int i = 0; i < mc->faces.size(); i++) {
                            mc->faces[i].setTransformMatrix(tc->transformMatrix);
                        }
                    }
                }
                template <typename... Components, typename T> requires NOTCONTAINS<TransformComponent, Components...>
                void operator()(EntityId entityId, T& params) {
                }
            };
            struct UpdateTransformSystem {
                template <typename... Components, typename T> requires CONTAINS<TransformComponent, Components...>
                void operator()(EntityId entityId, ComponentMapping& cmapping, T& params) {
                    TransformComponent* tc = cmapping.getComponent<TransformComponent>(entityId);
                    tc->transformMatrix.update();
                }
                template <typename... Components, typename T> requires NOTCONTAINS<TransformComponent, Components...>
                void operator()(EntityId entityId, T& params) {
                }
            };
        }
    }
}
#endif
