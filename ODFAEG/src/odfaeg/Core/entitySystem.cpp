#include "../../../include/odfaeg/Core/entitySystem.h"
#include <iostream>
namespace odfaeg {
    namespace core {
        EntitySystem::EntitySystem(bool usingThread) : isUsingThread(usingThread), m_needToUpdate(false) {
            if (usingThread) {
                m_thread = std::thread(&EntitySystem::tUpdate, this);
            }
        }
        void EntitySystem::needToUpdate() {
            m_needToUpdate = true;
        }
        void EntitySystem::update() {
            onUpdate();
        }
        void EntitySystem::tUpdate() {
            running = true;
            while (running) {
                if (m_needToUpdate) {
                    onUpdate();
                    m_needToUpdate = false;
                }
            }
        }
        void EntitySystem::stop() {
            if (isUsingThread && running) {
                running = false;
                m_thread.join();
            }
        }
        void EntitySystem::setName(std::string name) {
            this->name = name;
        }
        std::string EntitySystem::getName() {
            return name;
        }
        EntitySystem::~EntitySystem() {
            stop();
        }
    }
}
