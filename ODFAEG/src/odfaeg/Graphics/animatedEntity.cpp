#include "../../../include/odfaeg/Graphics/animatedEntity.h"
namespace odfaeg {
    namespace graphic {
        AnimatedEntity::AnimatedEntity(math::Vec3f position, math::Vec3f size,math::Vec3f origin, std::string type, EntityFactory& factory, std::string name, Entity* parent) :
        GameObject(position, size, origin, type, factory, name, parent) {

        }
    }
}
