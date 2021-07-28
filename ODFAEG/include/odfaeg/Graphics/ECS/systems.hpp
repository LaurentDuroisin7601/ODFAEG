#ifndef ODFAEG_ECS_SYSTEM_HPP
#define ODFAEG_ECS_SYSTEM_HPP
namespace odfaeg {
    namespace graphic {
        //Call the systems with the given system's IDs.
        struct MainSystem {
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
        //Load datas to renderders.
        struct LoaderSystem {
            template <typename... Components, typename... Params, typename SubSystem, class <typename = std::enable_if_t<contains<Scene, Components...>::value>
            void operator()(std::tuple<Components...> components, std::tuple<Params...> params) {
                Scene* scene = std::get<index<Scene*, Components...>(components);
                if (scene != nullptr) {

                }
            }
            //Type de composant inconnu, on le traite avec un système défini par le développeur si il existe.
            template <typename... Components, typename... Params, typename SubSystem, class <typename = std::enable_if_t<!contains<Scene, Components...>::value>
            void operator()(std::tuple<Components...> components, std::tuple<Params...> params, SubSystem& subSystem) {
                if (subSystem != nullptr) {
                    static_cast<SubSystem&>(*subSystem)(components, params);
                }
            }
        };
        //Render a per pixel linked list.
        struct PerPixelLinkedListRenderSystem {
            template <typename... Components, typename... Params, class = typename = std::enable_if_t<contains<PerPixelLinkedListRenderer*, Components...>::value>
            void operator()(std::tuple<Components...> components, std::tuple<Params...> params) {
                PerPixelLinkedListRenderer* perPixelLinkedListRender = std::get<index<PerPixelLinkedListRenderer*, Components...>(components);
                if (perPixelLinkedListRender != nullptr) {

                }
            }
        };
        //Update an animation.
        struct AnimationUpdaterSystem {
            template <typename... Components, typename... Params>
            void operator()(std::tuple<Components...> components, std::tuple<Params...> params) {
                //Récupération du composant de l'animation.
                AnimationComponent* ac = std::get<index<AnimationComponent*,Components...>()>(components);
                //si l'entité contient un composant d'animation, c'est que c'est une animation.
                if (ac != nullptr && ac->playing) {
                    size_t currentFrame = ac->currentFrame;
                    size_t nextFrame = ac->nexFrame;
                    if (ac->nbFrames >= 2) {
                        ac->interpPerc++;
                        if (ac->interpPerc >= ac->interpLevels) {
                            ac->previousFrame = currentFrame;
                            ac->currentFrame++;
                            if (ac->currentFrame >= ac->nbFrames) {
                                ac->currentFrame = 0;
                                if (!ac->loop) {
                                    ac->playing = false;
                                }
                            }
                            ac->interpolatedFrameFaces = ac->famesFaces[currentFrame];
                        }
                        ac->interpPerc = 0;
                        std::vector<Face> currentFrameFaces = ac->framesFaces[currentFrame];
                        std::vector<Face> nextFrameFaces = ac->framesFaces[nextFrame];
                        for (unsigned int i = 0; i < currentFrameFaces.size(); i++) {
                            const VertexArray& cva = currentFrameFaces[i].getVertexArray();
                            const VertexArray& nva = currentFrameFaces[i].getVertexArray();
                            VertexArray& iva = interpolatedFrameFaces.getVertexArray();
                            for (unsigned int j = 0; j < cva.getVerteexCount(); j++) {
                                iva[j].position.x = cva[j].position.x + (nva[j].position.x - cva[j].position.x) * (interpPerc / interpLevels);
                                iva[j].position.y = cva[j].position.y + (nva[j].position.y - cva[j].position.y) * (interpPerc / interpLevels);
                                iva[j].position.z = cva[j].position.z + (nva[j].position.z - cva[j].position.z) * (interpPerc / interpLevels);
                            }
                            ac->interpolatedFrameFaces[j].setVertexArray(iva);
                        }
                    }
                }
            }
        };
        struct CloningSystem {
            template <typename... Components, typename... Params>
            void operator()(std::tuple<Components...> components, std::tuple<Params...> params) {
                EntityId toClone = std::get<0>
            }
        };
        struct LabyrinthGeneratorSystem {
            template <typename... Components, typename... Params>
            void operator()(std::tuple<Components...> components, std::tuple<Params...> params) {
                auto sceneArray = std::get<0>(components);
                auto scene = std::get<1>(components);
                auto componentArray = std::get<2>(components);
                auto componentMapping = std::get<3>(components);
                EntityId bigTile = std::get<4>(components);
                std::vector<EntityId> walls = std::get<5>(components);
                auto factory = std::get<6>(components);
                unsigned int i, j;
                CloningSystem system;
                for (int y = startY, j = 0; y < endY; y+= tileSize.y, j++) {
                    for (int x = startX, i = 0; x < endX; x+= tileSize.x, i++) {
                        math::Vec3f projPos = scene->grid.getBaseChangementMatrix().changeOfBase(math::Vec3f (x - startX, y - startY, 0));
                        if (x == startX && y == startY) {
                            math::Vec2f pos (projPos.x + startX, projPos.y + startY);
                            std::vector<EntityId> wall;
                            componentMapping.apply(componentArray, system, walls[Wall::TOP_LEFT], walls[Wall::TOP_LEFT], wall, factory);
                            auto transform = componentMapping.getAgregate<TransformComponent>();
                            transform->position = math::Vec3f(pos.x, pos.y, pos.y + transform->size.y * 0.5f);
                        }
                    }
                }
            }
        };
    }
}
