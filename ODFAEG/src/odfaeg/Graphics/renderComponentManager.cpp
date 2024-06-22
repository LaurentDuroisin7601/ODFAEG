#include "../../../include/odfaeg/Graphics/renderComponentManager.h"
#include "../../../include/odfaeg/Graphics/GUI/menu.hpp"
#include "../../../include/odfaeg/Window/command.h"
namespace odfaeg {
    namespace graphic {
        RenderComponentManager::RenderComponentManager(RenderWindow& window) {
            windows.push_back(&window);
        }
        void RenderComponentManager::addWindow(RenderWindow& window) {
            windows.push_back(&window);
        }
        void RenderComponentManager::addComponent(Component* component) {
            std::unique_ptr<Component> ptr;
            ptr.reset(component);
            components.insert(std::make_pair(component->getPriority(), std::move(ptr)));
        }
        void RenderComponentManager::addECSComponent(ecs::Component* component) {
            std::unique_ptr<ecs::Component> ptr;
            ptr.reset(component);
            ecs_components.insert(std::make_pair(component->getPriority(), std::move(ptr)));
        }
        void RenderComponentManager::setEventContextActivated(bool activated, RenderWindow& window) {
            std::multimap<int, std::unique_ptr<Component>, std::greater<int>>::iterator it;
            for (it = components.begin();it != components.end(); it++) {
                if (&it->second->getWindow() == &window && it->second->getComponentType() == 1) {
                    it->second->setEventContextActivated(activated);
                }
            }
        }
        bool RenderComponentManager::removeComponent(unsigned int layer) {
            std::multimap<int, std::unique_ptr<Component>, std::greater<int>>::iterator it;
            for (it = components.begin(); it != components.end();) {
                if (it->second->getId() == layer) {
                    it = components.erase(it);
                    return true;
                } else {
                    it++;
                }
            }
            return false;
        }
        bool RenderComponentManager::removeECSComponent(unsigned int layer) {
            std::multimap<int, std::unique_ptr<ecs::Component>, std::greater<int>>::iterator it;
            for (it = ecs_components.begin(); it != ecs_components.end();) {
                if (it->second->getId() == layer) {
                    it = ecs_components.erase(it);
                    return true;
                } else {
                    it++;
                }
            }
            return false;
        }
        RenderWindow& RenderComponentManager::getWindow() {
            return *windows[0];
        }
        unsigned int RenderComponentManager::getNbComponents() {
            return components.size();
        }
        unsigned int RenderComponentManager::getNbECSComponents() {
            return ecs_components.size();
        }
        void RenderComponentManager::drawRenderComponents() {
            std::multimap<int, std::unique_ptr<Component>, std::greater<int>>::reverse_iterator it;
            for (it = components.rbegin(); it != components.rend(); it++) {
                if (it->second->getComponentType() == 0 && it->second->isVisible()) {
                    for (unsigned int i = 0; i < windows.size(); i++) {
                        if (windows[i] == &it->second->getWindow()) {
                            windows[i]->clearDepth();
                            it->second->getWindow().draw(*it->second.get());
                        }
                    }
                }
            }
        }
        void RenderComponentManager::drawECSComponents() {
            std::multimap<int, std::unique_ptr<ecs::Component>, std::greater<int>>::reverse_iterator it;
            for (it = ecs_components.rbegin(); it != ecs_components.rend(); it++) {
                if (it->second->getComponentType() == 0 && it->second->isVisible()) {
                    for (unsigned int i = 0; i < windows.size(); i++) {
                        if (windows[i] == &it->second->getWindow()) {
                            windows[i]->clearDepth();
                            it->second->getWindow().draw(*it->second.get());
                        }
                    }
                }
            }
        }
        void RenderComponentManager::drawGuiComponents() {
            std::multimap<int, std::unique_ptr<Component>, std::greater<int>>::iterator it;
            for (it = components.begin(); it != components.end(); it++) {
                if (it->second->getComponentType() == 1 && it->second->isVisible()) {
                    for (unsigned int i = 0; i < windows.size(); i++) {
                        if (windows[i] == &it->second->getWindow()) {
                            windows[i]->clearDepth();
                            View view = it->second->getWindow().getView();
                            View defaultView = it->second->getWindow().getDefaultView();
                            defaultView.setCenter(math::Vec3f(it->second->getWindow().getSize().x * 0.5f, it->second->getWindow().getSize().y * 0.5f, 0));
                            it->second->getWindow().setView(defaultView);
                            it->second->getWindow().draw(*it->second.get());
                            it->second->getWindow().setView(view);
                        }
                    }
                }
            }
        }
        Component* RenderComponentManager::getRenderComponent(unsigned int layer) {
            std::multimap<int, std::unique_ptr<Component>, std::greater<int>>::reverse_iterator it;
            for (it = components.rbegin(); it != components.rend(); it++) {
               if (it->second->getComponentType() ==  0 && it->second->getPriority() == layer) {
                   return it->second.get();
               }
            }
            return nullptr;
        }
        Component* RenderComponentManager::getGuiComponent(unsigned int layer) {
            std::multimap<int, std::unique_ptr<Component>, std::greater<int>>::iterator it;
            for (it = components.begin(); it != components.end(); it++) {
               if (it->second->getComponentType() && it->second->getPriority() == layer) {
                   return it->second.get();
               }
            }
            return nullptr;
        }
        ecs::Component* RenderComponentManager::getECSComponent(unsigned int layer) {
            std::multimap<int, std::unique_ptr<ecs::Component>, std::greater<int>>::iterator it;
            for (it = ecs_components.begin(); it != ecs_components.end(); it++) {
               if (it->second->getComponentType() == 0 && it->second->getPriority() == layer) {
                   return it->second.get();
               }
            }
            return nullptr;
        }
        bool RenderComponentManager::isComponentAdded(unsigned int layer) {
           std::multimap<int, std::unique_ptr<Component>, std::greater<int>>::iterator it;
           for (it = components.begin(); it != components.end(); it++) {
               if (it->second->getPriority() == layer) {
                   return true;
               }
           }
           return false;
        }
        bool RenderComponentManager::isECSComponentAdded(unsigned int layer) {
           std::multimap<int, std::unique_ptr<ecs::Component>, std::greater<int>>::iterator it;
           for (it = ecs_components.begin(); it != ecs_components.end(); it++) {
               if (it->second->getPriority() == layer) {
                   return true;
               }
           }
           return false;
        }
        Component* RenderComponentManager::getComponent(unsigned int layer) {
           std::multimap<int, std::unique_ptr<Component>, std::greater<int>>::iterator it;
           unsigned int i = 0;
           for (it = components.begin(); it != components.end(); it++) {
               if (i == layer) {
                    return it->second.get();
               }
               i++;
           }
        }
        std::vector<Component*> RenderComponentManager::getComponents() {
            std::multimap<int, std::unique_ptr<Component>, std::greater<int>>::iterator it;
            std::vector<Component*> cpnts;
            for (it = components.begin(); it != components.end(); it++) {
                cpnts.push_back(it->second.get());
            }
            return cpnts;
        }
        std::vector<Component*> RenderComponentManager::getRenderComponents() {
            std::multimap<int, std::unique_ptr<Component>, std::greater<int>>::iterator it;
            std::vector<Component*> cpnts;
            for (it = components.begin(); it != components.end(); it++) {
                if (it->second->getComponentType() == 0)
                    cpnts.push_back(it->second.get());
            }
            return cpnts;
        }
        std::vector<ecs::Component*> RenderComponentManager::getECSComponents() {
            std::multimap<int, std::unique_ptr<ecs::Component>, std::greater<int>>::iterator it;
            std::vector<ecs::Component*> cpnts;
            for (it = ecs_components.begin(); it != ecs_components.end(); it++) {
                cpnts.push_back(it->second.get());
            }
            return cpnts;
        }
        void RenderComponentManager::clearComponents() {
            std::multimap<int, std::unique_ptr<Component>, std::greater<int>>::iterator it;
            for (it = components.begin(); it != components.end(); it++) {
                if (it->second->isVisible()) {
                    it->second->clear();
                }
            }
        }
        void RenderComponentManager::clearECSComponents() {
            std::multimap<int, std::unique_ptr<ecs::Component>, std::greater<int>>::iterator it;
            for (it = ecs_components.begin(); it != ecs_components.end(); it++) {
                if (it->second->isVisible()) {
                    it->second->clear();
                }
            }
        }
        void RenderComponentManager::updateComponents() {
           std::multimap<int, std::unique_ptr<Component>, std::greater<int>>::iterator it;
           for (it = components.begin(); it != components.end(); it++) {
               if (it->second->isEventContextActivated() && it->second->isVisible()) {
                   it->second->processEvents();
                   it->second->recomputeSize();
               }
           }
           //core::Command::clearEventsStack();
        }
        void RenderComponentManager::updateECSComponents() {
           std::multimap<int, std::unique_ptr<ecs::Component>, std::greater<int>>::iterator it;
           for (it = ecs_components.begin(); it != ecs_components.end(); it++) {
               if (it->second->isEventContextActivated() && it->second->isVisible()) {
                   it->second->processEvents();
                   it->second->recomputeSize();
               }
           }
           //core::Command::clearEventsStack();
        }
        RenderComponentManager::~RenderComponentManager() {
           /*std::multimap<int, Component*, std::greater<int>>::iterator it;
           for (it = components.begin(); it != components.end(); it++) {
                delete it->second;
           }*/
        }
    }
}
