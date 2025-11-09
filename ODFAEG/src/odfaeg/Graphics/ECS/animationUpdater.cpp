#include "../../../../include/odfaeg/Graphics/ECS/animationUpdater.hpp"
namespace odfaeg {
    namespace graphic {
        namespace ecs {
            AnimationUpdater::AnimationUpdater(World& world) : world (world) {
            }
            void AnimationUpdater::addAnim(EntityId anim) {
                AnimationComponent* ac = world.getComponentMapping().getComponent<AnimationComponent>(anim);
                if (ac->interpolatedFrame == entt::null && ac->currentFrameIndex < world.getComponentMapping().getChildren(anim).size()) {
                    ac->interpolatedFrame = world.getComponentMapping().clone<EntityInfoComponent, TransformComponent, MeshComponent, ColliderComponent, ShadowInfoComponent>(world.getComponentMapping().getChildren(anim)[ac->currentFrameIndex], anim);

                }
                anims.push_back(anim);
            }
            void AnimationUpdater::onUpdate() {
                for (unsigned int i = 0; i < anims.size(); i++) {
                    AnimationComponent* ac = world.getComponentMapping().getComponent<AnimationComponent>(anims[i]);
                    if (ac->playing && ac->clock.getElapsedTime().asSeconds() > ac->fr) {
                        if (world.getComponentMapping().getChildren(anims[i]).size() > 2) {
                            ac->interpPerc++;
                            if (ac->interpPerc >= ac->interpLevels) {
                                ac->previousFrame = world.getComponentMapping().getChildren(anims[i])[ac->currentFrameIndex];
                                ac->currentFrameIndex++;
                                if (ac->currentFrameIndex >= world.getComponentMapping().getChildren(anims[i]).size()-1) {
                                    ac->currentFrameIndex = 0;
                                    if (!ac->loop) {
                                        ac->playing = false;
                                    }
                                }
                                ac->currentFrame = world.getComponentMapping().getChildren(anims[i])[ac->currentFrameIndex];
                                ac->nextFrame = world.getComponentMapping().getChildren(anims[i])[(ac->currentFrameIndex + 1 >= world.getComponentMapping().getChildren(anims[i]).size()) ? 0 : ac->currentFrameIndex+1];
                                ac->interpPerc = 0;
                            }
                            interpolate(ac->currentFrame, ac->nextFrame, ac->interpolatedFrame, ac->interpPerc, ac->interpLevels);
                        }
                        ac->clock.restart();
                    }
                }
            }
            void AnimationUpdater::interpolate(EntityId currentFrame, EntityId nextFrame, EntityId interpolatedFrame, std::size_t interpPerc, std::size_t interpLevels) {

                if (world.getComponentMapping().getChildren(currentFrame).size() == world.getComponentMapping().getChildren(nextFrame).size()) {

                    for (unsigned int i = 0; i < world.getComponentMapping().getChildren(currentFrame).size(); i++) {
                        interpolate(world.getComponentMapping().getChildren(currentFrame)[i], world.getComponentMapping().getChildren(nextFrame)[i], world.getComponentMapping().getChildren(interpolatedFrame)[i], interpPerc, interpLevels);
                    }
                }


                if (world.getComponentMapping().getComponent<MeshComponent>(currentFrame) != nullptr && world.getComponentMapping().getComponent<MeshComponent>(nextFrame) != nullptr
                    && world.getComponentMapping().getComponent<MeshComponent>(currentFrame)->faces.size() == world.getComponentMapping().getComponent<MeshComponent>(nextFrame)->faces.size()) {

                    for (unsigned int i = 0; i < world.getComponentMapping().getComponent<MeshComponent>(currentFrame)->faces.size(); i++) {
                        const VertexArray& cva = world.getComponentMapping().getComponent<MeshComponent>(currentFrame)->faces[i].getVertexArray();
                        VertexArray& iva = world.getComponentMapping().getComponent<MeshComponent>(interpolatedFrame)->faces[i].getVertexArray();
                        const VertexArray& nva = world.getComponentMapping().getComponent<MeshComponent>(nextFrame)->faces[i].getVertexArray();

                        if (cva.getVertexCount() == nva.getVertexCount()) {
                            for (unsigned int j = 0; j < cva.getVertexCount(); j++) {

                                iva[j].position[0] = cva[j].position.x() + (nva[j].position.x() - cva[j].position.x()) * (interpPerc / interpLevels);
                                iva[j].position[1] = cva[j].position.y() + (nva[j].position.y() - cva[j].position.y()) * (interpPerc / interpLevels);
                                iva[j].position[2] = cva[j].position.z() + (nva[j].position.z() - cva[j].position.z()) * (interpPerc / interpLevels);
                                iva[j].color = cva[j].color;
                                iva[j].texCoords = cva[j].texCoords;
                                ////////std::cout<<"pos : "<<iva[j].position.x <<std::endl;
                            }
                        }

                        world.getComponentMapping().getComponent<MeshComponent>(interpolatedFrame)->faces[i].setMaterial(world.getComponentMapping().getComponent<MeshComponent>(currentFrame)->faces[i].getMaterial());
                        world.getComponentMapping().getComponent<MeshComponent>(interpolatedFrame)->faces[i].setTransformMatrix(world.getComponentMapping().getComponent<MeshComponent>(currentFrame)->faces[i].getTransformMatrix());
                    }
                }
            }
        }
    }
}
