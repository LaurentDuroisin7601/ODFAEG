#include "../../../include/odfaeg/Graphics/renderComponentManager.h"
/*#include "../../../include/odfaeg/Graphics/GUI/menu.hpp"
#include "../../../include/odfaeg/Window/command.h"*/
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
            eventComponents.insert(std::make_pair(component->getEventPriority(), component));
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
        bool RenderComponentManager::removeComponent(Component* component) {
            std::multimap<int, Component*, std::greater<int>>::iterator it;
            for (it = eventComponents.begin(); it != eventComponents.end();) {
                if (it->second == component)
                    it = eventComponents.erase(it);
                else
                    it++;
            }
            std::multimap<int, std::unique_ptr<Component>, std::greater<int>>::iterator it2;
            for (it2 = components.begin(); it2 != components.end();) {
                if (it2->second.get() == component) {
                    it2 = components.erase(it2);
                    return true;
                } else {
                    it2++;
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
        void RenderComponentManager::drawRenderComponents() {
            std::multimap<int, std::unique_ptr<Component>, std::greater<int>>::reverse_iterator it;
            unsigned int i = 0;
            for (it = components.rbegin(); it != components.rend(); it++) {
                if (it->second->getComponentType() == 0 && it->second->isVisible()) {
                    for (unsigned int i = 0; i < windows.size(); i++) {
                        if (windows[i] == &it->second->getWindow()) {
                            it->second->getWindow().enableDepthTest(false);
                            //std::cout<<"draw on window : "<<it->second->getPosition().z()<<std::endl;
                            it->second->getWindow().draw(*it->second.get());
                            //std::cout<<"drawed on window : "<<i<<std::endl;
                            /*#ifdef VULKAN
                            //it->second->getWindow().submit();
                            it->second->getWindow().beginRecordCommandBuffers();
                            #endif
                            i++;*/
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
                            it->second->getWindow().enableDepthTest(true);
                            it->second->getWindow().clearDepth();
                            View view = it->second->getWindow().getView();
                            View defaultView = it->second->getWindow().getDefaultView();
                            defaultView.setCenter(math::Vec3f(it->second->getWindow().getSize().x() * 0.5f, it->second->getWindow().getSize().y() * 0.5f, 0));
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
        bool RenderComponentManager::isComponentAdded(unsigned int layer) {
           std::multimap<int, std::unique_ptr<Component>, std::greater<int>>::iterator it;
           for (it = components.begin(); it != components.end(); it++) {
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
           return nullptr;
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
        void RenderComponentManager::clearComponents() {
            std::multimap<int, std::unique_ptr<Component>, std::greater<int>>::iterator it;
            for (it = components.begin(); it != components.end(); it++) {
                if (it->second->isVisible()) {
                    it->second->clear();
                }
            }
        }
        void RenderComponentManager::updateComponents() {
           std::multimap<int, Component*, std::greater<int>>::iterator it;
           //std::cout<<"process events"<<std::endl;
           for (it = eventComponents.begin(); it != eventComponents.end(); it++) {
               if (it->second->isEventContextActivated() && it->second->isVisible()) {
                   if (!it->second->getListener().isUsingThread()) {
                       /*if (it->second->getName() != "")
                           std::cout<<"process events of : "<<it->second->getName()<<std::endl;*/
                       it->second->processEvents();
                   }
                   //it->second->recomputeSize();
               }
           }
           //core::Command::clearEventsStack();
        }
        void RenderComponentManager::recreateDescriptorsAndPipelines() {
            std::multimap<int, std::unique_ptr<Component>, std::greater<int>>::iterator it;
            for (it = components.begin(); it != components.end(); it++) {
                if (it->second->getListener().isRunning()) {
                    it->second->stopRenderer();
                }
            }
            for (it = components.begin(); it != components.end(); it++) {
                it->second->createDescriptorsAndPipelines();
                it->second->launchRenderer();
            }
            /*for (unsigned int i = 0; i < windows.size(); i++) {
                windows[i]->createDescriptorsAndPipelines();
            }*/
        }
        RenderComponentManager::~RenderComponentManager() {
            //////std::cout<<"rcm desrtructor"<<std::endl;

        }
    }
}
