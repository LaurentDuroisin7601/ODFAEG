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
        class ODFAEG_CORE_API AnimUpdater : public core::Timer {
        public :
            AnimUpdater(EntityManager* scene=nullptr) : Timer(), scene(scene) {}
            void setScene(EntityManager* scene) {
                this->scene = scene;
            }
            /**
            * \fn void addAnim(g2d::Anim *anim)
            * \brief add a 2D animation to the updater.
            * \param g2d::Anim : the 2D animation to add.
            */
            void addAnim (Entity *anim) {
                anims.push_back(anim);
            }
            /**
            * \fn onUpdate()
            * \brief update all the frames of the animations.
            */
            void onUpdate() {
                for (unsigned int i = 0; i < anims.size(); i++) {
                   /* if (anims[i]->getRootType() == "E_MONSTER")
                        std::cout<<"running ? "<<anims[i]->isRunning()<<std::endl;*/
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
               // std::cout<<"end animations"<<std::endl;
            }
            std::vector<Entity*> getAnims() {
                return anims;
            }
        private :
            /** < the animations of the updater. */
            std::vector<Entity*> anims;
            EntityManager* scene;
        };
    }
}

#endif // ANIM_UPDATER
