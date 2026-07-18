module;
#include <vector>
#include <iostream>
//import odfaeg.graphic.morphAnim;
module odfaeg.entity.morphAnim;
namespace odfaeg {
    namespace entity {
        MorphAnim::MorphAnim(math::Vec3f position, math::Vec3f size, unsigned int interpLevels) : GameObject(position, size, size * 0.5f, "E_MORPH_ANIMATION") {
            this->interpLevels = interpLevels;
        }
        void MorphAnim::addFrame(GameObject* frame) {            
            frames.push_back(frame);
        }
        unsigned int MorphAnim::getIntLevels() {
            return interpLevels;
        }
        std::vector<GameObject*> MorphAnim::getFrames() {
            return frames;
        }
        GameObject* MorphAnim::clone() {
            MorphAnim* ma = new MorphAnim(getPosition(), getSize(), getIntLevels());
            ma->frames = frames;
            ma->interpLevels = interpLevels;
            GameObject::copy(ma);
            return ma;
        }
    }
}
