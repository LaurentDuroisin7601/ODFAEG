#ifndef ODFAEG_ECS_COMPONENTS_HPP
#define ODFAEG_ECS_COMPONENTS_HPP
#include "entity.h"
namespace odfaeg {
    namespace graphic {
        struct ODFAEG_GRAPHICS_API Relation {
            std::vector<EntityId> children;
            EntityId parent = entt::null;
        };
    }
}
#endif
