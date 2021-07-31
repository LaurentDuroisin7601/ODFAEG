#ifndef ODFAEG_ECS_SYSTEM_HPP
#define ODFAEG_ECS_SYSTEM_HPP
namespace odfaeg {
    namespace graphic {
        //Call the systems with the given system's IDs.
        struct MainSystem {
            template <size_t I=0, typename... Components, typename T2, class = typename std::enable_if_t<(sizeof...(Components) != 0 && I == 0)>>
            void operator()(std::tuple<Components...>& tp, EntityId entityId, T2& params) {
                auto& array = std::get<0>(params);
                this->template operator()<I+1>(tp, entityId, params, std::make_index_sequence<array.nbTypes()>());
            }
            template <typename ...Ts, std::enable_if_t<(std::is_function_v<Ts> && ...), bool> = true>
            void f(Ts &&... ts) {}

            template <size_t I=0, typename... Components, typename T2, class... D, size_t... Ints, class = typename std::enable_if_t<(sizeof...(Components) != 0 && I > 0 && I < sizeof...(Components)-1)>>
            void operator()(std::tuple<Components...>& tp, EntityId entityId, T2& params, std::index_sequence<Ints...> seq) {
                if (std::get<I>(tp) != nullptr) {
                    auto& array = std::get<0>(params);
                    std::vector<EntityId> entities = std::get<1>(params);
                    auto& componentMapping = std::get<2>(params);
                    //std::cout<<"call system : "<<std::get<I>(tp)->positionInTemplateParameterPack<<std::endl;
                    std::thread t(
                    &MainSystem::call_system<typename std::remove_reference_t<decltype(array)>::types,
                    std::remove_reference_t<decltype(array)>,
                    std::remove_reference_t<decltype(*std::get<I>(tp))>,
                    std::remove_reference_t<decltype(componentMapping)>,
                    std::remove_reference_t<decltype(params)>,
                    Ints...>, this, array, *std::get<I>(tp), componentMapping, entities, params, std::make_index_sequence<array.nbTypes()>() );
                    t.detach();
                    //call_system<typename std::remove_reference_t<decltype(array)>::types>(array, *std::get<I>(tp), componentMapping, entities, params, std::make_index_sequence<array.nbTypes()>());

                } else {
                    this->template operator()<I+1>(tp, entityId, params, seq);
                }
            }
            ///home/laurent/gitODFAEG/Demos/ODFAEG-DEMO/main.cpp|558|error: ‘class std::tuple<DynamicTuple<RenderType1, RenderType2>, std::vector<atomwrapper<long unsigned int*>, std::allocator<atomwrapper<long unsigned int*> > >, ComponentMapping>’ has no member named ‘apply’|
            template <size_t I=0, typename... Components, typename T2, size_t... Ints, class... D, class... E, class = typename std::enable_if_t<(sizeof...(Components) != 0 && I == sizeof...(Components)-1)>>
            void operator()(std::tuple<Components...>& tp, EntityId entityId, T2& params, std::index_sequence<Ints...>) {
                if (std::get<I>(tp) != nullptr) {
                    auto& array = std::get<0>(params);
                    std::vector<EntityId> entities = std::get<1>(params);
                    auto& componentMapping = std::get<2>(params);
                    std::thread t(
                    &MainSystem::call_system<typename std::remove_reference_t<decltype(array)>::types,
                    std::remove_reference_t<decltype(array)>,
                    std::remove_reference_t<decltype(*std::get<I>(tp))>,
                    std::remove_reference_t<decltype(componentMapping)>,
                    std::remove_reference_t<decltype(params)>,
                    Ints...>, this, array, *std::get<I>(tp), componentMapping, entities, params, std::make_index_sequence<array.nbTypes()>() );
                    t.detach();
                }
            }
            template <size_t I=0, typename... Components, typename T2, size_t... Ints, class... D, class... E, class... F, class = typename std::enable_if_t<sizeof...(Components) == 0>>
            void operator()(std::tuple<Components...>& tp, EntityId entityId, T2& params,  std::index_sequence<Ints...>) {
            }
            template <typename T, typename Array, typename System, typename Mapping, typename T2, size_t... I>
            void call_system(Array array, System system, Mapping componentMapping, std::vector<EntityId> entities, T2 params, std::index_sequence<I...>) {
                componentMapping.template apply<std::tuple_element_t<I, T>...>(array, system, entities, params);
            }

        };

        //Update an animation.
        struct AnimationUpdaterSystem {
            template <typename... Components, typename... Params, class = typename = std::enable_if_t<!contains<AnimationComponent, Components...>::value>
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
        //Clone une entité ainsi que tout ses composants et ses enfants.
        struct CloningSystem {
            template <typename... Components, typename... Params, class = typename std::enable_if_t<contains<Clonable, Components...>::value>
            EntityId operator()(std::tuple<Components...> components, EntityId toCloneId, std::tuple<Params...>& params) {
                auto clonableComponent = std::get<ClonableComponent>(components);
                auto componentArray = std::get<0>(params);
                auto componentMapping = std::get<1>(params);
                auto factory = std::get<2>(params);
                bool& first = std::get<std::tuple_size<std::tuple<Params...>::value-1>();
                EntityId& tmpRootEntityId = std::get<std::tuple_size<std::tuple<Params...>::value-3>();
                EntityId& tmpParentEntityId = std::get<std::tuple_size<std::tuple<Params...>::value-2>();
                EntityId& tmpClonedParentEntityId = std::get<std::tuple_size<std::tuple<Params...>::value-4>();
                //Vérifie si l'entité est clonable.
                if (clonableComponent) {


                    EntityId clonedId = factory.createEntity();
                    clone_impl<decltype(componentArray)::types>(components, componentArray, componentMapping, toCloneId, clonedId, factory);
                    //Si c'est le premier noeud que l'on clône, il faut faire des vérifications supplémentaires.
                    if (first) {
                        //Récupération de la racine.
                        tmpRootEntityId = componentMapping.getRoot(toCloneId);
                        //Si l'entité à clôner n'est pas un noeud racine, il faut ajouter le parent de l'entité à clôner à l'entité clônée, sinon, on stocke les parents pour le niveau suivant.
                        if (tmpRootEntityId != toCloneId) {
                            tmpParentEntity = componentMapping.branchIds[*toCloneId];
                            componentMapping.addChild(tmpRootEntityId, tmpParentEntityId, clonedId, componentMapping.treeLevels[*toCloneId]);
                        } else {
                            tmpParentEntityId = tmpRootEntityId;
                            tmpClonedParentEntityId = clonedId;
                        }
                        first = false;
                    //Sinon on ajoute le noeud enfant au parent.
                    } else {
                        addChild(tmpClonedRootId, tmpClonedParentId, clonedId);
                        //Remettre à jour le parent, si ce n'est plus le même.
                        if (tmpParentEntityId != componentMapping.branchIds[*toCloneId]) {
                            tmpParentEntityId = componentMapping.branchIds[*toCloneId];
                            tmpClonedParentEntityId = clonedId;
                        }
                    }
                    return cloneId;
                }
                return nullptr;
            }
            template <typename... Components, typename... Params, class... D, class = typename std::enable_if_t<!contains<Clonable, Components...>::value>
            EntityId operator()(std::tuple<Components...> components, EntityId toCloneId, std::tuple<Params...>& params) {
            }
            //Clone tout les types de composants.
            template <typename T, size_t I = 0, typename ComponentArray, typename ComponentMapping, class = typename std::enable_if_t<(sizeof...(Components) != 0 && I < sizeof...(Components)-1)>
            void clone_impl(ComponentArray& componentArray, EntityId tocloneId, EntityId clonedId, ComponentMapping& componentMapping) {
                //si l'entité possède le composant en question on le clône.
                if (componentMapping.getAgregate<std::tuple_element_t<I, T>(componentArray, toCloneId)) {
                    componentMapping.addAgregate(cloneId, componentArray, *componentMapping.getAgregate<std::tuple_element_t<I, T>(componentArray, toCloneId));
                }
                clone_impl<T, I+1>(componentArray, toCloneId, clonedId, factory);
            }
            template <typename T, size_t I = 0, typename ComponentArray, typename ComponentMapping, class... D, class = typename std::enable_if_t<(sizeof...(Components) != 0 && I == sizeof...(Components)-1)>
            void clone_impl(ComponentArray& componentArray, EntityId tocloneId, EntityId clonedId, ComponentMapping& componentMapping) {
                if (componentMapping.getAgregate<std::tuple_element_t<I, T>(componentArray, toCloneId)) {
                    componentMapping.addAgregate(cloneId, componentArray, *componentMapping.getAgregate<std::tuple_element_t<I, T>(componentArray, toCloneId));
                }
            }
            template <typename T, size_t I = 0, typename ComponentArray, typename ComponentMapping, class... D, class...E, class = typename std::enable_if_t<(sizeof...(Components) == 0)>
            void clone_impl(ComponentArray& componentArray, EntityId tocloneId, EntityId clonedId, ComponentMapping& componentMapping) {
            }
        };
        /*Merge two trees into one. (This is possible that several entities have the same id in case when the composant are not creating with the same running instance,
        by example, with ODFAEGCreator the plugin'll create some specific components and the editor some common component), so we need to merge datas in the main script
        after they are serialized, another example is if we have to make a system like git with a distant repository and a local repository, we need to merge the repositories
        when pushing or pulling.*/
        struct MergingSystem {
            template <typename... Components, typename... Params, class = typename std::enable_if_t<contains<MergingComponent, Components...>::value>
            void operator()(std::tuple<Components...> components, EntityId entityId, std::tuple<Params...>& params) {
                auto componentArray = std::get<0>(params);
                auto componentMapping = std::get<1>(params);
                std::vector<EntityId> toMerge = std::get<2>(params);
                auto Factory = std::get<3>(params);

                for (unsigned int i = 0; i < toMerge.size(); i++) {
                    //On merge les composants, si le noeud est le même que sur la branche distante.
                    if (*entityId.get().load() == *toMergeIds[i].get.load()) {
                        merge_node(componentArray, entityId, toMergeIds[i], componentMapping)
                    //Si le noeud n'est pas le même il faut créer un nouveau noeud, récupérer le niveau du noeud sur l'arbre et l'ajouter à un parent du niveau inférieur.
                    } else {
                        EntityId newEntityId = factory.createEntity();
                        merge_node(componentArray, newEntityId, toMergeIds[i], componentMapping);
                        size_t treeLevel = componentMapping.treeLevels[*toMergeIds[i].get().load()];
                        EntityId root = componentMapping.getRoot(entityId);
                        //Il faut aussi vérifier si le niveau existe, si pas il faut le créer.
                        if (treeLevel <= componentMapping.nbLevels[*root.get().load()]) {
                            EntityId parent = branchs[*componentMapping.childrenMapping[*root.get().load()][treeLevel][0].get().load()];
                            componentMapping.addChild (root, parent, newEntityId, treeLevel);
                        } else {
                            EntityId parent = branchs[*root.get().load()][componentsMapping.nbLevels[*root.get().load()]][0];
                            componentMapping.addChild (root, parent, newEntityId, componentsMapping.nbLevels[*root.get().load()]+1);
                        }
                    }
                }
            }
            template <typename T, size_t I = 0, typename ComponentArray, typename ComponentMapping, class = typename std::enable_if_t<(sizeof...(Components) != 0 && I < sizeof...(Components)-1)>
            void merge_node(ComponentArray& componentArray, EntityId entityId, EntityId toMergeId, ComponentMapping& componentMapping) {
                //si l'entité possède le composant en question on le clône.
                if (componentMapping.getAgregate<std::tuple_element_t<I, T>(componentArray, toMergeId)) {
                    componentMapping.addAgregate(entityId, componentArray, *componentMapping.getAgregate<std::tuple_element_t<I, T>(componentArray, toMergeId));
                }
                merge_node<T, I+1>(componentArray, toCloneId, clonedId, componentMapping);
            }
            template <typename T, size_t I = 0, typename ComponentArray, typename ComponentMapping, class... D, class = typename std::enable_if_t<(sizeof...(Components) != 0 && I == sizeof...(Components)-1)>
            void merge_node(ComponentArray& componentArray, EntityId entityId, EntityId toMergeId, ComponentMapping& componentMapping) {
                if (componentMapping.getAgregate<std::tuple_element_t<I, T>(componentArray, toMergeId)) {
                    componentMapping.addAgregate(entityId, componentArray, *componentMapping.getAgregate<std::tuple_element_t<I, T>(componentArray, toMergeId));
                }
            }
            template <typename T, size_t I = 0, typename ComponentArray, typename ComponentMapping, class... D, class...E, class = typename std::enable_if_t<(sizeof...(Components) == 0)>
            void merge_node(ComponentArray& componentArray, EntityId tocloneId, EntityId clonedId, ComponentMapping& componentMapping) {
            }
        };
        //Génère un labyrinthe. (Non testé)
        struct LabyrinthGeneratorSystem {
            template <typename... Components, typename... Params, class = typename std::enable_if_t<contains<Scene>::value>
            void operator()(std::tuple<Components...> components, EntitydId entityId, std::tuple<Params...>& params) {
                auto scene = std::get<0>(components);
                auto componentArray = std::get<1>(params);
                auto componentMapping = std::get<2>(params);
                auto factory = std::get<3>(params);

                std::vector<EntityId> walls = std::get<4>(params);

                unsigned int i, j;
                EntityId bigTile;
                for (int y = startY, j = 0; y < endY; y+= tileSize.y, j++) {
                    for (int x = startX, i = 0; x < endX; x+= tileSize.x, i++) {
                        math::Vec3f projPos = scene->grid.getBaseChangementMatrix().changeOfBase(math::Vec3f (x - startX, y - startY, 0));
                        math::Vec2f pos (projPos.x + startX, projPos.y + startY);
                        if (x == startX && y == startY) {
                            std::vector<EntityId> wallId(1);
                            wallId[0] = walls[Wall::TOP_LEFT];
                            std::vector<EntityId> clonedWallIds;
                            CloningSystem cloningSystem;
                            auto clparams = std::make_tuple(componentArray, componentMapping, factory);
                            componentMapping.apply(componentArray, cloningSystem, wallId, clparams, clonedWallsId);
                            MoveEntitySystem moveSystem;
                            auto transform = componentMapping.getAgregate<TransformComponent>(componentMapping, clonedWallsId[0]);
                            math::Vec3f position = math::Vec3f(pos.x, pos.y, pos.y + transform->size.y * 0.5f);
                            auto mvparams = std::make_tuple(componentArray, componentMapping, position);
                            wallId[0] = clonedWallsId[0];
                            componentMapping.apply(componentArray, moveSystem, wallId, mvparams);
                            auto adparams = std::make_tuple(entityComponentArray, entityComponentMapping, entityId));
                            AddEntityToSceneSystem addSystem;
                            sceneMapping.apply(sceneArray, addSystem, wallId, adparams);
                            scene->grid.getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, 0))->setPassable(false);
                        } else if (x == endX && y == startY) {
                            std::vector<EntityId> wallId(1);
                            wallId[0] = walls[Wall::TOP_RIGHT];
                            std::vector<EntityId> clonedWallIds;
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
                        } else if (y == endY && x == endX) {
                            std::vector<EntityId> wallId(1);
                            wallId[0] = walls[Wall::BOTTOM_RIGHT];
                            std::vector<EntityId> clonedWallIds;
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
                        }   else if (x == startX && y == endY) {
                            std::vector<EntityId> wallId(1);
                            wallId[0] = walls[Wall::BOTTOM_LEFT];
                            std::vector<EntityId> clonedWallIds;
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
                        } else if (y == startY && j % 2 != 0) {
                            std::vector<EntityId> wallId(1);
                            wallId[0] = walls[Wall::TOP_BOTTOM];
                            std::vector<EntityId> clonedWallIds;
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
                        } else if (y == startY && j % 2 == 0) {
                            std::vector<EntityId> wallId(1);
                            wallId[0] = walls[Wall::T_TOP];
                            std::vector<EntityId> clonedWallIds;
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
                        } else if (x == endX && j % 2 != 0) {
                            std::vector<EntityId> wallId(1);
                            wallId[0] = walls[Wall::RIGHT_LEFT];
                            std::vector<EntityId> clonedWallIds;
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
                        } else if (x == endX && j % 2 == 0) {
                            std::vector<EntityId> wallId(1);
                            wallId[0] = walls[Wall::TOP_BOTTOM];
                            std::vector<EntityId> clonedWallIds;
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
                        } else if (y == endY && i % 2 == 0) {
                            std::vector<EntityId> wallId(1);
                            wallId[0] = walls[Wall::T_BOTTOM];
                            std::vector<EntityId> clonedWallIds;
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
                        } else if (x == startX && j % 2 != 0) {
                            std::vector<EntityId> wallId(1);
                            wallId[0] = walls[Wall::RIGHT_LEFT];
                            std::vector<EntityId> clonedWallIds;
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
                        } else if (x == startX && j % 2 == 0) {
                            std::vector<EntityId> wallId(1);
                            wallId[0] = walls[Wall::T_LEFT];
                            std::vector<EntityId> clonedWallIds;
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
                        } else if (j % 2 != 0 && i % 2 == 0) {
                            std::vector<EntityId> wallId(1);
                            wallId[0] = walls[Wall::RIGHT_LEFT];
                            std::vector<EntityId> clonedWallIds;
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
                        } else if (j % 2 == 0 && i % 2 != 0) {
                            std::vector<EntityId> wallId(1);
                            wallId[0] = walls[Wall::TOP_BOTTOM];
                            std::vector<EntityId> clonedWallIds;
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
                        } else if (j % 2 == 0 && i % 2 == 0) {
                            std::vector<EntityId> wallId(1);
                            wallId[0] = walls[Wall::X];
                            std::vector<EntityId> clonedWallIds;
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
                std::vector<CellMap*> visited;
                std::vector<math::Vec2f> dirs;
                int n = 0, cx = startX + tileSize.x, cy = startY + tileSize.y;
                while (n < i * j) {
                    for (unsigned int i = 0; i < 4; i++) {
                        int x = cx, y = cy;
                        math::Vec2f dir;
                        if (i == 0) {
                            dir = math::Vec2f (1, 0);
                        } else if (i == 1) {
                            dir = math::Vec2f (0, 1);
                        } else if (i == 2) {
                            dir = math::Vec2f (-1, 0);
                        } else {
                            dir = math::Vec2f (0, -1);
                        }
                        x += dir.x * tileSize.x * 2;
                        y += dir.y * tileSize.y * 2;
                        math::Vec3f projPos = gridMap->getBaseChangementMatrix().changeOfBase(math::Vec3f (x - startX, y - startY, 0));
                        math::Vec2f pos (projPos.x + startX, projPos.y + startY);
                        CellMap* cell = scene->grid.getGridCellAt(math::Vec2f(pos.x, pos.y));
                        if (cell != nullptr) {
                            dirs.push_back(dir);
                        }
                    }
                    math::Vec2f dir = dirs[math::Math::random(0, dirs.size())];
                    int x = cx + dir.x * tileSize.x;
                    int y = cy + dir.y * tileSize.y;
                    math::Vec3f projPos = gridMap->getBaseChangementMatrix().changeOfBase(math::Vec3f (x - startX, y - startY, 0));
                    math::Vec2f pos (projPos.x + startX, projPos.y + startY);
                    CellMap* cell = scene->grid.getGridCellAt(math::Vec2f(pos.x, pos.y));
                    cell->removeEntity("E_WALL");
                    cell->setPassable(true);
                    cx += dir.x * tileSize.x * 2;
                    cy += dir.y * tileSize.y * 2;
                    x = cx, y = cy;
                    projPos = scene->grid.getBaseChangementMatrix().changeOfBase(math::Vec3f (x - startX, y - startY, 0));
                    pos = math::Vec2f (projPos.x + startX, projPos.y + startY);
                    cell = gridMap->getGridCellAt(math::Vec2f(pos.x, pos.y));
                    bool contains = false;
                    for (unsigned int j = 0; j < visited.size() && !contains; j++) {
                        if (visited[j] == cell)
                            contains = true;
                    }
                    if (!contains) {
                        n++;
                        visited.push_back(cell);
                    }
                }
            }
            template <typename... Components, typename... Params, class = typename std::enable_if_t<!contains<Scene>::value>
            void operator()(std::tuple<Components...> components, EntitydId entityId, std::tuple<Params...>& params) {
            }
        };
            //Génère un terrain entouré des murs passés.
        struct TerrainGeneratorSystem {
            template <typename... Components, typename... Params, typename = class std::enable_if_t<contains<Scene, components...>::value>
            void operator()(std::tuple<Components...> components, EntitydId entityId, std::tuple<Params...>& params) {
                auto scene = std::get<0>(components);
                auto componentArray = std::get<0>(params);
                auto componentMapping = std::get<1>(params);
                auto factory = std::get<2>(params);
                auto tGround = std::get<3>(params);
                auto walls = std::get<4>(params);
                math::Vec3f tileSize = std::get<5>(params);
                physic::BoundingBox rect = std::get<6>(params);

                int startX = rect.getPosition().x / tileSize.x * tileSize.x;
                int startY = rect.getPosition().y / tileSize.y * tileSize.y;
                int endX = (rect.getPosition().x + rect.getWidth()) / tileSize.x * tileSize.x;
                int endY = (rect.getPosition().y + rect.getHeight()) / tileSize.y * tileSize.y;
                EntityId bt;
                if (rect.getSize().z != 0)
                    bt = createBigTileModel(math::Vec3f(startX, startY, startY + (endY - startY) * 0.5f), factory);
                else
                    bt = createBigTileModel(math::Vec3f(startX, startY, rect.getPosition().z),factory, tileSize,rect.getWidth() / tileSize.x);
                auto szparams = std::make_tuple(componentArray, componentMapping, rect.getsize());
                ResizeSystem resizeSystem;
                std::vector<EntityId> bigTileId;
                bigTileId.push_back(bt);
                componentMapping.apply(componentArray, resizeSystem, bigTileId, szparams);
                for (int y = startY; y < endY;  y+=tileSize.y) {
                    for (int x = startX; x < endX; x+=tileSize.x) {
                        //On projete les positions en fonction de la projection du jeux.
                        math::Vec3f projPos = gridMap->getBaseChangementMatrix().changeOfBase(math::Vec3f (x - startX, y - startY, 0));
                        math::Vec2f pos (projPos.x + startX, projPos.y + startY);
                        if (x == startX && y == startY && walls.size() >= 11) {
                            std::vector<EntityId> wallId(1);
                            wallId[0] = walls[Wall::TOP_LEFT];
                            std::vector<EntityId> clonedWallIds;
                            CloningSystem cloningSystem;
                            auto clparams = std::make_tuple(componentArray, componentMapping, factory);
                            componentMapping.apply(componentArray, cloningSystem, wallId, clparams, clonedWallsId);
                            MoveEntitySystem moveSystem;
                            auto transform = componentMapping.getAgregate<TransformComponent>(componentMapping, clonedWallsId[0]);
                            math::Vec3f position = math::Vec3f(pos.x, pos.y, pos.y + transform->size.y * 0.5f);
                            auto mvparams = std::make_tuple(componentArray, componentMapping, position);
                            wallId[0] = clonedWallsId[0];
                            componentMapping.apply(componentArray, moveSystem, wallId, mvparams);
                            auto adparams = std::make_tuple(entityComponentArray, entityComponentMapping, entityId));
                            AddEntityToSceneSystem addSystem;
                            sceneMapping.apply(sceneArray, addSystem, wallId, adparams);
                            scene->grid.getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, 0))->setPassable(false);
                        } else if (x == endX - tileSize.x && y == startY && walls.size() >= 11) {
                            std::vector<EntityId> wallId(1);
                            wallId[0] = walls[Wall::TOP_RIGHT];
                            std::vector<EntityId> clonedWallIds;
                            CloningSystem cloningSystem;
                            auto clparams = std::make_tuple(componentArray, componentMapping, factory);
                            componentMapping.apply(componentArray, cloningSystem, wallId, clparams, clonedWallsId);
                            MoveEntitySystem moveSystem;
                            auto transform = componentMapping.getAgregate<TransformComponent>(componentMapping, clonedWallsId[0]);
                            math::Vec3f position = math::Vec3f(pos.x, pos.y, pos.y + transform->size.y * 0.5f);
                            auto mvparams = std::make_tuple(componentArray, componentMapping, position);
                            wallId[0] = clonedWallsId[0];
                            componentMapping.apply(componentArray, moveSystem, wallId, mvparams);
                            auto adparams = std::make_tuple(entityComponentArray, entityComponentMapping, entityId));
                            AddEntityToSceneSystem addSystem;
                            sceneMapping.apply(sceneArray, addSystem, wallId, adparams);
                            scene->grid.getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, 0))->setPassable(false);
                            //Mur du coin en bas \E0 droite.
                        } else if (x == endX - tileSize.x && y == endY - tileSize.y && walls.size() >= 11) {
                            std::vector<EntityId> wallId(1);
                            wallId[0] = walls[Wall::TOP_RIGHT];
                            std::vector<EntityId> clonedWallIds;
                            CloningSystem cloningSystem;
                            auto clparams = std::make_tuple(componentArray, componentMapping, factory);
                            componentMapping.apply(componentArray, cloningSystem, wallId, clparams, clonedWallsId);
                            MoveEntitySystem moveSystem;
                            auto transform = componentMapping.getAgregate<TransformComponent>(componentMapping, clonedWallsId[0]);
                            math::Vec3f position = math::Vec3f(pos.x, pos.y, pos.y + transform->size.y * 0.5f);
                            auto mvparams = std::make_tuple(componentArray, componentMapping, position);
                            wallId[0] = clonedWallsId[0];
                            componentMapping.apply(componentArray, moveSystem, wallId, mvparams);
                            auto adparams = std::make_tuple(entityComponentArray, entityComponentMapping, entityId));
                            AddEntityToSceneSystem addSystem;
                            sceneMapping.apply(sceneArray, addSystem, wallId, adparams);
                            scene->grid.getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, 0))->setPassable(false);
                        } else if (x == startX && y == endY - tileSize.y && walls.size() >= 11) {
                            std::vector<EntityId> wallId(1);
                            wallId[0] = walls[Wall::BOTTOM_RIGHT];
                            std::vector<EntityId> clonedWallIds;
                            CloningSystem cloningSystem;
                            auto clparams = std::make_tuple(componentArray, componentMapping, factory);
                            componentMapping.apply(componentArray, cloningSystem, wallId, clparams, clonedWallsId);
                            MoveEntitySystem moveSystem;
                            auto transform = componentMapping.getAgregate<TransformComponent>(componentMapping, clonedWallsId[0]);
                            math::Vec3f position = math::Vec3f(pos.x, pos.y, pos.y + transform->size.y * 0.5f);
                            auto mvparams = std::make_tuple(componentArray, componentMapping, position);
                            wallId[0] = clonedWallsId[0];
                            componentMapping.apply(componentArray, moveSystem, wallId, mvparams);
                            auto adparams = std::make_tuple(entityComponentArray, entityComponentMapping, entityId));
                            AddEntityToSceneSystem addSystem;
                            sceneMapping.apply(sceneArray, addSystem, wallId, adparams);
                            scene->grid.getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, 0))->setPassable(false);
                        } else if ((y == startY || y == endY - tileSize.y) && walls.size() >= 11) {
                            std::vector<EntityId> wallId(1);
                            wallId[0] = walls[Wall::BOTTOM_RIGHT];
                            std::vector<EntityId> clonedWallIds;
                            CloningSystem cloningSystem;
                            auto clparams = std::make_tuple(componentArray, componentMapping, factory);
                            componentMapping.apply(componentArray, cloningSystem, wallId, clparams, clonedWallsId);
                            MoveEntitySystem moveSystem;
                            auto transform = componentMapping.getAgregate<TransformComponent>(componentMapping, clonedWallsId[0]);
                            math::Vec3f position = math::Vec3f(pos.x, pos.y, pos.y + transform->size.y * 0.5f);
                            auto mvparams = std::make_tuple(componentArray, componentMapping, position);
                            wallId[0] = clonedWallsId[0];
                            componentMapping.apply(componentArray, moveSystem, wallId, mvparams);
                            auto adparams = std::make_tuple(entityComponentArray, entityComponentMapping, entityId));
                            AddEntityToSceneSystem addSystem;
                            sceneMapping.apply(sceneArray, addSystem, wallId, adparams);
                            scene->grid.getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, 0))->setPassable(false);
                            if (y == endY - tileSize.y) {
                                int i = math::Math::random(tGround.size());
                                std::vector<EntityId> tileId {tGround[i]};
                                std::vector<EntityId> clonedTileIds;
                                componentMapping.apply(componentArray, cloningSystem, tGround[i], clparams, clonedTileIds);
                                auto mvparams = std::make_tuple(componentArray, componentMapping, position);
                                tileId[0] = clonedTileIds[0];
                                componentMapping.apply(componentArray, moveSystem, tileId, mvparams);
                                componentMapping.addChild(bt, bt, tile, 0);
                            }
                            gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, w->getPosition().z))->setPassable(false);
                        } else if ((x == startX || x == endX - tileSize.x) && walls.size() >= 11) {
                            std::vector<EntityId> wallId(1);
                            wallId[0] = walls[Wall::RIGHT_LEFT];
                            std::vector<EntityId> clonedWallIds;
                            CloningSystem cloningSystem;
                            auto clparams = std::make_tuple(componentArray, componentMapping, factory);
                            componentMapping.apply(componentArray, cloningSystem, wallId, clparams, clonedWallsId);
                            MoveEntitySystem moveSystem;
                            auto transform = componentMapping.getAgregate<TransformComponent>(componentMapping, clonedWallsId[0]);
                            math::Vec3f position = math::Vec3f(pos.x, pos.y, pos.y + transform->size.y * 0.5f);
                            auto mvparams = std::make_tuple(componentArray, componentMapping, position);
                            wallId[0] = clonedWallsId[0];
                            componentMapping.apply(componentArray, moveSystem, wallId, mvparams);
                            auto adparams = std::make_tuple(entityComponentArray, entityComponentMapping, entityId));
                            AddEntityToSceneSystem addSystem;
                            sceneMapping.apply(sceneArray, addSystem, wallId, adparams);
                            scene->grid.getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, 0))->setPassable(false);
                            if (y == endY - tileSize.y) {
                                int i = math::Math::random(tGround.size());
                                std::vector<EntityId> tileId {tGround[i]};
                                std::vector<EntityId> clonedTileIds;
                                componentMapping.apply(componentArray, cloningSystem, tGround[i], clparams, clonedTileIds);
                                auto mvparams = std::make_tuple(componentArray, componentMapping, position);
                                tileId[0] = clonedTileIds[0];
                                componentMapping.apply(componentArray, moveSystem, tileId, mvparams);
                                componentMapping.addChild(bt, bt, tile, 0);
                            }
                            gridMap->getGridCellAt(math::Vec3f(w->getPosition().x, w->getPosition().y, w->getPosition().z))->setPassable(false);
                        } else {
                            EntityId tile;
                            if (tGround.size() > 0)  {
                                int i = math::Math::random(tGround.size());
                                std::vector<EntityId> clonedTileIds;
                                componentMapping.apply(componentArray, cloningSystem, tGround[i], clparams, clonedTileIds);
                                auto mvparams = std::make_tuple(componentArray, componentMapping, position);
                                tileId[0] = clonedTileIds[0];
                                componentMapping.apply(componentArray, moveSystem, tileId, mvparams);
                            } else {
                                EntitId tile = createTileModel(componentMapping, entityComponentArray, nullptr, math::Vec3f(pos.x, pos.y, pos.y + tileSize.y * 0.5f), math::Vec3f(tileSize.x, tileSize.y, 0), sf::IntRect(0, 0, tileSize.x, tileSize.y), factory);
                            }
                            if (rect.getSize().z != 0) {
                                float heights[4];
                                for (unsigned int j = 0; j < sizeof(heights) / sizeof(float); j++) {
                                    heights[j] = math::Math::random(rect.getPosition().z, rect.getPosition().z + rect.getDepth());
                                }
                                HeightmapGeneratorSystem heightMapGeneratorSystem;
                                std::vector<EntityId> bigTileId {bt};
                                auto params = std::make_tuple(tile, math::Vec2f(pos.x, pos.y));
                                componentMapping.apply(componentArray, heightMapGeneratorSystem, bigTileId, params);
                            } else {
                                componentMapping.addChild(bt, bt, tile);
                            }
                        }
                    }
                }
            }
        };
        //Load datas to renderders.
        struct LoadVisbileDataToRenderersSystem {
            //Copie des données des entités visible de la scene.
            template <typename... Components, typename... Params, typename SubSystem, class <typename = std::enable_if_t<contains<Scene, Components...>::value>
            void operator()(std::tuple<Components...> components, std::tuple<Params...> params) {
                Scene* scene = std::get<index<Scene, Components...>(components);
                if (scene != nullptr) {

                }
            }
            //Type de composant inadéquat.
            template <typename... Components, typename... Params, typename SubSystem, class = typename = std::enable_if_t<!contains<Scene, Components...>::value>
            void operator()(std::tuple<Components...> components, std::tuple<Params...> params, SubSystem& subSystem) {

            }
        };
        struct CheckVisbleEntities {
            template <typename... Components, typename... Params, typename SubSystem, class <typename = std::enable_if_t<contains<Scene, Components...>::value>
            void operator()(std::tuple<Components...> components, std::tuple<Params...> params) {
                template <typename... Components, typename... Params, class = typename std::enable_if_t<contains<SceneType1*, Components...>::value>>
                void operator()(std::tuple<Components...>& tp, EntityId entityId, std::tuple<Params...>& params) {
                    Scene scene = std::get<index<SceneType1*,Components...>()>(tp);
                    if (scene != nullptr) {
                        auto renderers = std::get<3>(params);
                        std::vector<EntityId> rendersIds = std::get<4>(params);
                        ComponentMapping renderMapping = std::get<5>(params);
                        auto tp = std::make_tuple(scene);
                        LoadVisbileDataToRenderersSystem loader;
                        call_sub_system<typename std::remove_reference_t<decltype(renders)>::types>(renderers, loader, renderMapping, rendersIds, params, std::make_index_sequence<renders.nbTypes()>());
                    }
                }
                template <typename T, typename Array, typename System, typename Mapping, typename... Params, size_t... I>
                void call_sub_system(Array& array, System& system, Mapping& componentMapping, std::vector<EntityId> entities, std::tuple<Params...>& params, std::index_sequence<I...>) {
                    componentMapping.template apply<std::tuple_element_t<I, T>...>(array, system, entities, params);
                }
                template <typename... Components, typename... Params, class... D, class = typename std::enable_if_t<!contains<Scene, Components...>::value>>
                void operator()(std::tuple<Components...>& tp, EntityId entityId, std::tuple<Params...>& params) {

                }
            }
        };
    }
}
