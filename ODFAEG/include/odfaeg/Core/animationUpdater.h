#ifndef ODFAEG_ANIM_UPDATER_HPP
#define ODFAEG_ANIM_UPDATER_HPP
#include "../Graphics/2D/anim.h"
#include "world.h"
#include "timer.h"
#include "export.hpp"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
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
class ODFAEG_CORE_API AnimUpdater : public Timer {
public :
    AnimUpdater(bool useThread) : Timer(useThread) {}
    /**
    * \fn void addAnim(g2d::Anim *anim)
    * \brief add a 2D animation to the updater.
    * \param g2d::Anim : the 2D animation to add.
    */
    void addAnim (g2d::Anim *anim) {
        anims2D.push_back(anim);
    }
    /**
    * \fn onUpdate()
    * \brief update all the frames of the animations.
    */
    void onUpdate() {
        for (unsigned int i = 0; i < anims2D.size(); i++) {

            if (anims2D[i]->isRunning() &&
                anims2D[i]->getElapsedTime().asSeconds() > anims2D[i]->getFrameRate()) {
                anims2D[i]->nextImage();
                if (World::containsVisibleEntity(anims2D[i])) {
                    World::changeVisibleEntity(anims2D[i]->getPreviousCurrentEntity(), anims2D[i]->getCurrentEntity());
                }
                anims2D[i]->setCurrentEntityChanged(false);
                anims2D[i]->resetClock();
            }
        }
    }
private :
    /** < the animations of the updater. */
    std::vector<g2d::Anim*> anims2D;
};
}

#endif // ANIM_UPDATER
