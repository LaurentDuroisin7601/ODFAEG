#include "../../../include/odfaeg/Graphics/boneAnimation.hpp"
namespace odfaeg {
    namespace graphic {
        BoneAnimation::BoneAnimation(math::Vec3f position, math::Vec3f  size, std::string type, EntityFactory& factory) : GameObject(position, size, size * 0.5f, type, factory) {
            boneIndex = 0;
        }
        void BoneAnimation::setBoneAnimationIndex(unsigned int index) {
            boneIndex = index;
        }
        unsigned int BoneAnimation::getBoneAnimationIndex() {
            return boneIndex;
        }
    }
}
