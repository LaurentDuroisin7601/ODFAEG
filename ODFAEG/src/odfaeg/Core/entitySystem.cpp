#include "../../../include/odfaeg/Core/entitySystem.h"
namespace odfaeg {
    namespace core {
        EntitySystem::EntitySystem() : m_needToUpdate(false), m_thread(EntitySystem::update, this) {
        }
        void EntitySystem::needToUpdate() {
            m_needToUpdate = true;
        }
        void EntitySystem::update() {
            running = true;
            while (running) {
                if (m_needToUpdate) {
                    onUpdate();
                    m_needToUpdate = false;
                }
            }
        }
        void EntitySystem::stop() {
            running = false;
            m_thread.join();
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
