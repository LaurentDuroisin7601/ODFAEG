#include "../../../include/odfaeg/Graphics/model.h"
namespace odfaeg {
    namespace graphic {
        Model::Model (math::Vec3f position, math::Vec3f size, math::Vec3f origin, EntityFactory& factory, std::string type, std::string name, Entity *parent)
            : GameObject(position, size, origin, type, factory, name, parent) {
        }
        bool Model::isModel() const {
            return true;
        }
        bool Model::isAnimated() const {
            return false;
        }
        bool Model::isLeaf() const {
            return false;
        }
    }
}

