#ifndef ODFAEG_ECS_COMPONENTS_HPP
#define ODFAEG_ECS_COMPONENTS_HPP
#include "entityFactory.hpp"
namespace odfaeg {
    namespace graphic {
        //Each serializable component must inherits from this.
        struct ODFAEG_GRAPHICS_API ISerializable {
        };
        struct ODFAEG_GRAPHICS_API Relation {
            std::vector<EntityId> children;
            EntityId parent = entt::null;
        };
    }
}
#endif
