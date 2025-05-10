#include "../../../include/odfaeg/Core/entitySystem.h"
namespace odfaeg {
    namespace core {
        void EntitySystem::update() {
            onUpdate();
        }
        void EntitySystem::setName(std::string name) {
            this->name = name;
        }
        std::string EntitySystem::getName() {
            return name;
        }
        EntitySystem::~EntitySystem() {}
    }
}
