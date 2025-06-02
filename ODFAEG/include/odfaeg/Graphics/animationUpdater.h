#ifndef ODFAEG_ANIM_UPDATER_HPP
#define ODFAEG_ANIM_UPDATER_HPP
#include "anim.h"
#include "../Graphics/world.h"
#include "../Core/timer.h"
#include "export.hpp"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace graphic {
        /**
        * \file animationUpdater.h
        * \class AnimUpdater
        * \brief Store and updates the frames of an animation.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        * Store one or more animations.
        * Update the frames of all the animations.
        */
        class ODFAEG_GRAPHICS_API AnimUpdater : public core::Timer {
        public :
            AnimUpdater(bool useThread = true, EntityManager* scene=nullptr) : Timer(useThread), scene(scene) {}
            void setScene(EntityManager* scene) {
                this->scene = scene;
            }
            /**
            * \fn void addAnim(g2d::Anim *anim)
            * \brief add a 2D animation to the updater.
            * \param g2d::Anim : the 2D animation to add.
            */
            void addBoneAnim (Entity *anim) {
                boneAnims.push_back(anim);
            }
            void addAnim (Entity *anim) {
                anim->setAnimUpdater(getName());
                anims.push_back(anim);
            }
            void removeAnim(Entity* anim) {
                std::vector<Entity*>::iterator it;
                for (it = anims.begin(); it != anims.end();) {
                    if (anim == *it)
                        it = anims.erase(it);
                    else
                        it++;
                }
            }
            /**
            * \fn onUpdate()
            * \brief update all the frames of the animations.
            */
            void onUpdate() {

                for (unsigned int i = 0; i < anims.size(); i++) {

                    if (anims[i]->isRunning() &&
                        anims[i]->getElapsedTime().asSeconds() > anims[i]->getFrameRate()) {
                       /* if (anims[i]->getRootType() == "E_HERO")
                            std::cout<<"update anim hero"<<std::endl;*/

                        anims[i]->computeNextFrame();
                        if (anims[i]->isCurrentFrameChanged() /*&& graphic::World::containsVisibleParentEntity(anims[i]->getRootEntity())*/) {
                            //graphic::World::changeVisibleEntity(anims[i]->getPreviousFrame(), anims[i]->getCurrentFrame());
                        }
                        anims[i]->setCurrentFrameChanged(false);
                        anims[i]->resetClock();
                    }
                }
                for (unsigned int i = 0; i < boneAnims.size(); i++) {
                    boneAnims[i]->updateAnimation(clock.getElapsedTime().asSeconds());
                }
                clock.restart();
               // std::cout<<"end animations"<<std::endl;
            }
            std::vector<Entity*> getAnims() {
                return anims;
            }
        private :
            /** < the animations of the updater. */
            std::vector<Entity*>  boneAnims;
            std::vector<Entity*> anims;
            EntityManager* scene;
            sf::Clock clock;
        };
    }
}

#endif // ANIM_UPDATER
