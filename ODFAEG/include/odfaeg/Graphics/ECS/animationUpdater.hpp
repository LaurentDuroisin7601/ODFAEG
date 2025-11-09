#ifndef ECS_ANIMATION_UPDATER_CPP
#define ECS_ANIMATION_UPDATER_CPP
#include "world.hpp"
#include "../../Core/timer.h"
namespace odfaeg {
    namespace graphic {
        namespace ecs {
            class ODFAEG_GRAPHICS_API AnimationUpdater : public core::Timer {
            public :
                AnimationUpdater(World& world);
                void addAnim(EntityId Anim);
                void onUpdate();
            private :
                void interpolate(EntityId currentFrame, EntityId nextFrame, EntityId interpolatedFrame, std::size_t interpPerc, std::size_t interpLevels);
                World& world;
                std::vector<EntityId> anims;
            };
        }
    }
}
#endif // ECS_ANIMATION_UPDATER_CPP
