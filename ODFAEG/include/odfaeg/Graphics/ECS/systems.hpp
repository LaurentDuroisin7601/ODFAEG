#ifndef ODFAEG_ECS_SYSTEM_HPP
#define ODFAEG_ECS_SYSTEM_HPP
namespace odfaeg {
    namespace graphic {
        //Charge les entités visible de la scène sur le render.
        struct LoaderSystem : ISystem {
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
        struct PerPixelLinkedListRenderSystem {
            template <typename... Components, typename... Params, class = typename = std::enable_if_t<contains<PerPixelLinkedListRenderer*, Components...>::value>
            void operator()(std::tuple<Components...> components, std::tuple<Params...> params) {
                PerPixelLinkedListRenderer* perPixelLinkedListRender = std::get<index<PerPixelLinkedListRenderer*, Components...>(components);
                if (perPixelLinkedListRender != nullptr) {

                }
            }
        };
        struct AnimationUpdaterSystem : IStystem {
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
    }
}
