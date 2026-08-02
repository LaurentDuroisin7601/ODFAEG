#ifndef ODFAEG_MORPHANIM_HPP
#define ODFAEG_MORPHANIM_HPP
#include <vector>
#include <iostream>
namespace odfaeg {
    namespace entity {
        export class MorphAnim : public GameObject {
            public :
            MorphAnim(math::Vec3f position, math::Vec3f size, unsigned int interpLevels=1);
            void addFrame(GameObject* frame);
            std::vector<GameObject*> getFrames();
            unsigned int getIntLevels();
            GameObject* clone();
            private :
            std::vector<GameObject*> frames;
            unsigned int currentFrameIndex, interpLevels;
        };
    }
}
#include "morphAnim.inl"
#endif
