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
            EntityId operator()(std::tuple<Components...> components, EntityId toCloneId, std::tuple<Params...>& params) {
                auto clonableComponent = std::get<ClonableComponent>(components);
                auto componentArray = std::get<0>(params);
                auto componentMapping = std::get<1>(params);
                auto factory = std::get<2>(params);
                bool& first = std::get<std::tuple_size<std::tuple<Params...>::value-1>();
                EntityId& tmpRootEntity = std::get<std::tuple_size<std::tuple<Params...>::value-2>();
                EntityId& tmpParentEntity = std::get<std::tuple_size<std::tuple<Params...>::value-3>();
                if (clonableComponent) {


                    EntityId clonedId = factory.createEntity();
                    clone_impl<decltype(componentArray)::types>(components, componentArray, componentMapping, toCloneId, clonedId, factory);
                    if (first) {
                        EntityId root = componentMapping.getRoot(toCloneId);
                        if (root != toCloneId) {
                            tmpClonedRootId = root;
                        }
                        EntityId parent = componentMapping.branchIds[toCloneId];
                        if (root != nullptr) {
                            if (parent == root) {
                                componentMapping.addChild(root, root, clonedId, componentMapping.treeLevels[*toCloneId]);
                            } else {
                                componentMapping.addChild(root, parent, clonedId, componentMapping.treeLevels[*toCloneId]);
                            }
                            componentMapping.addChild(root, parent, clonedId);
                            tmpClonedRootId = root;
                            tmpClonedParentId = parent;
                        } else {
                            tmpClonedRootId = clonedId;
                        }
                        first = false;
                    } else {
                        addChild(tmpClonedRootId, tmpClonedParentId, clonedId);
                        tmpClonedParentId = clonedId;
                    }
                    return cloneId;
                }
            }
            template <typename T, size_t I = 0, typename ComponentArray, typename Factory, class = typename std::enable_if_t<(sizeof...(Components) != 0 && I < sizeof...(Components)-1)>
            void clone_impl(ComponentArray& componentArray, EntityId tocloneId, EntityId clonedId, Factory factory) {
                if (componentMapping.getAgregate<std::tuple_element_t<I, T>(componentArray, toCloneId)) {
                    componentMapping.addAgregate(cloneId, componentArray, *componentMapping.getAgregate<std::tuple_element_t<I, T>(componentArray, toCloneId));
                }
                clone_impl<T, I+1>(componentArray, toCloneId, clonedId, factory);
            }
            template <typename T, size_t I = 0, typename ComponentArray, typename Factory, class... D, class = typename std::enable_if_t<(sizeof...(Components) != 0 && I == sizeof...(Components)-1)>
            void clone_impl(ComponentArray& componentArray, EntityId tocloneId, EntityId clonedId, Factory factory) {
                if (componentMapping.getAgregate<std::tuple_element_t<I, T>(componentArray, toCloneId)) {
                    componentMapping.addAgregate(cloneId, componentArray, *componentMapping.getAgregate<std::tuple_element_t<I, T>(componentArray, toCloneId));
                }
            }
            template <typename T, size_t I = 0, typename ComponentArray, typename Factory, class... D, class...E, class = typename std::enable_if_t<(sizeof...(Components) == 0)>
            void clone_impl(ComponentArray& componentArray, EntityId tocloneId, EntityId clonedId, Factory factory) {
            }
        };
        struct LabyrinthGeneratorSystem {
            template <typename... Components, typename... Params>
            void operator()(std::tuple<Components...> components, EntitydId entityId, std::tuple<Params...>& params) {
                auto sceneArray = std::get<0>(components);
                auto scene = std::get<1>(components);
                auto componentArray = std::get<2>(components);
                auto componentMapping = std::get<3>(components);
                EntityId bigTile = std::get<4>(components);
                std::vector<EntityId> walls = std::get<5>(components);
                auto factory = std::get<6>(components);
                unsigned int i, j;
                CloningSystem system;
                //Besoin de garder une référence sur les ids des entités parents et l'entité racine des clones pour ajouter les enfants clônés aux parents clônés.
                EntityId tmpClonedRootId, tmpClonedParentId;
                //Pour la racine qui est unique.
                bool isFirst = true;
                for (int y = startY, j = 0; y < endY; y+= tileSize.y, j++) {
                    for (int x = startX, i = 0; x < endX; x+= tileSize.x, i++) {
                        math::Vec3f projPos = scene->grid.getBaseChangementMatrix().changeOfBase(math::Vec3f (x - startX, y - startY, 0));
                        if (x == startX && y == startY) {
                            math::Vec2f pos (projPos.x + startX, projPos.y + startY);
                            std::vector<EntityId> wallId(1);
                            wallId[0] = walls[Wall::TOP_LEFT];
                            std::vector<EntityId> clonedWallIds;
                            std::vector<std::optional<size_t>> treeLevels;
                            std::vector<EntityId> branchs;
                            std::optional<size_t> nbLevels;
                            CloningSystem cloningSystem;
                            auto clparams = std::make_tuple(componentArray, componentMapping, factory);
                            componentMapping.apply(componentArray, cloningSystem, wallId, clparams, clonedWallsId);

                            MoveEntitySystem moveSystem;
                            auto transform = componentMapping.getAgregate<TransformComponent>(componentMapping, clonedWallsId[0]);
                            math::Vec3f position = math::Vec3f(pos.x, pos.y, pos.y + transform->size.y * 0.5f);
                            auto mvparams = std::make_tuple(componentArray, componentMapping, position);
                            wallId[0] = clonedWallsId[0];
                            componentMapping.apply(componentArray, moveSystem, wallId, mvparams);
                            scene->grid.getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, 0))->setPassable(false);
                        } else if (x == endX && y == startY) {
                            math::Vec2f pos (projPos.x + startX, projPos.y + startY);
                            std::vector<EntityId> wallId(1);
                            wallId[0] = walls[Wall::TOP_RIGHT];
                            std::vector<EntityId> clonedWallIds;
                            std::vector<std::optional<size_t>> treeLevels;
                            std::vector<EntityId> branchs;
                            std::optional<size_t> nbLevels;
                            CloningSystem cloningSystem;
                            auto clparams = std::make_tuple(componentArray, componentMapping, factory);
                            componentMapping.apply(componentArray, cloningSystem, wallId, clparams, clonedWallsId);

                            MoveEntitySystem moveSystem;
                            auto transform = componentMapping.getAgregate<TransformComponent>(componentMapping, clonedWallsId[0]);
                            math::Vec3f position = math::Vec3f(pos.x, pos.y, pos.y + transform->size.y * 0.5f);
                            auto mvparams = std::make_tuple(componentArray, componentMapping, position);
                            wallId[0] = clonedWallsId[0];
                            componentMapping.apply(componentArray, moveSystem, wallId, mvparams);
                            scene->grid.getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, 0))->setPassable(false);
                        }
                    }
                }
            }
        };
    }
}
