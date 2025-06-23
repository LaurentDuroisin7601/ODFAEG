#include "application.h"
#include "odfaeg/Graphics/graphics.hpp"
#include "odfaeg/Audio/audio.hpp"
#include "odfaeg/Math/math.hpp"
#include "hero.hpp"



#include "odfaeg/Graphics/renderWindow.h"
#include "odfaeg/Graphics/font.h"
#include "odfaeg/Graphics/text.h"
#include "odfaeg/Graphics/sprite.h"
#include "odfaeg/Graphics/rectangleShape.h"
#include "odfaeg/Window/iEvent.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <array>
#include <iostream>
#include <math.h>
#include <functional>
using namespace odfaeg::core;
using namespace odfaeg::math;
//using namespace odfaeg::physic;
using namespace odfaeg::graphic;
using namespace odfaeg::window;
using namespace odfaeg::audio;
using namespace sorrok;
const std::string  simpleVertexShader = R"(#version 460
                                            layout (location = 0) in vec3 position;
                                            layout (location = 1) in vec4 color;
                                            layout (location = 2) in vec2 texCoords;
                                            layout (location = 3) in vec3 normals;
                                            uniform mat4 projectionMatrix;
                                            uniform mat4 viewMatrix;
                                            uniform mat4 worldMat;
                                            void main () {
                                                gl_Position = projectionMatrix * viewMatrix * worldMat * vec4(position, 1.f);
                                            })";

const std::string  simpleFragmentShader = R"(
                   #version 460
                   layout(location = 0) out vec4 fcolor;
                   void main() {
                      fcolor = vec4(1, 0, 0, 1);
                   })";
class RCM {
        public :
        RCM(RenderWindow& window) {
            windows.push_back(&window);
        }
        void addWindow(RenderWindow& window) {
            windows.push_back(&window);
        }
        void addComponent(Component* component) {
            std::unique_ptr<Component> ptr;
            ptr.reset(component);
            components.insert(std::make_pair(component->getPriority(), std::move(ptr)));
            eventComponents.insert(std::make_pair(component->getEventPriority(), component));
        }
        void addECSComponent(ecs::Component* component) {
            std::unique_ptr<ecs::Component> ptr;
            ptr.reset(component);
            ecs_components.insert(std::make_pair(component->getPriority(), std::move(ptr)));
        }
        void setEventContextActivated(bool activated, RenderWindow& window) {
            std::multimap<int, std::unique_ptr<Component>, std::greater<int>>::iterator it;
            for (it = components.begin();it != components.end(); it++) {
                if (&it->second->getWindow() == &window && it->second->getComponentType() == 1) {
                    it->second->setEventContextActivated(activated);
                }
            }
        }
        bool removeComponent(unsigned int layer) {
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
        bool removeECSComponent(unsigned int layer) {
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
        RenderWindow& getWindow() {
            return *windows[0];
        }
        unsigned int getNbComponents() {
            return components.size();
        }
        unsigned int getNbECSComponents() {
            return ecs_components.size();
        }
        void drawRenderComponents() {
            std::multimap<int, std::unique_ptr<Component>, std::greater<int>>::reverse_iterator it;
            for (it = components.rbegin(); it != components.rend(); it++) {
                if (it->second->getComponentType() == 0 && it->second->isVisible()) {
                    for (unsigned int i = 0; i < windows.size(); i++) {
                        if (windows[i] == &it->second->getWindow()) {
                            std::cout<<"draw on window"<<std::endl;
                            it->second->getWindow().draw(*it->second.get());
                            #ifdef VULKAN
                            it->second->getWindow().submit();
                            it->second->getWindow().beginRecordCommandBuffers();
                            #endif
                        }
                    }
                }
            }
        }
        void drawECSComponents() {
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
        void drawGuiComponents() {
            std::multimap<int, std::unique_ptr<Component>, std::greater<int>>::iterator it;
            for (it = components.begin(); it != components.end(); it++) {
                if (it->second->getComponentType() == 1 && it->second->isVisible()) {
                    for (unsigned int i = 0; i < windows.size(); i++) {
                        if (windows[i] == &it->second->getWindow()) {
                            windows[i]->clearDepth();
                            View view = it->second->getWindow().getView();
                            View defaultView = it->second->getWindow().getDefaultView();
                            defaultView.setCenter(Vec3f(it->second->getWindow().getSize().x() * 0.5f, it->second->getWindow().getSize().y() * 0.5f, 0));
                            it->second->getWindow().setView(defaultView);
                            it->second->getWindow().draw(*it->second.get());
                            it->second->getWindow().setView(view);
                        }
                    }
                }
            }
        }
        Component* getRenderComponent(unsigned int layer) {
            std::multimap<int, std::unique_ptr<Component>, std::greater<int>>::reverse_iterator it;
            for (it = components.rbegin(); it != components.rend(); it++) {
               if (it->second->getComponentType() ==  0 && it->second->getPriority() == layer) {
                   return it->second.get();
               }
            }
            return nullptr;
        }
        Component* getGuiComponent(unsigned int layer) {
            std::multimap<int, std::unique_ptr<Component>, std::greater<int>>::iterator it;
            for (it = components.begin(); it != components.end(); it++) {
               if (it->second->getComponentType() && it->second->getPriority() == layer) {
                   return it->second.get();
               }
            }
            return nullptr;
        }
        ecs::Component* getECSComponent(unsigned int layer) {
            std::multimap<int, std::unique_ptr<ecs::Component>, std::greater<int>>::iterator it;
            for (it = ecs_components.begin(); it != ecs_components.end(); it++) {
               if (it->second->getComponentType() == 0 && it->second->getPriority() == layer) {
                   return it->second.get();
               }
            }
            return nullptr;
        }
        bool isComponentAdded(unsigned int layer) {
           std::multimap<int, std::unique_ptr<Component>, std::greater<int>>::iterator it;
           for (it = components.begin(); it != components.end(); it++) {
               if (it->second->getPriority() == layer) {
                   return true;
               }
           }
           return false;
        }
        bool isECSComponentAdded(unsigned int layer) {
           std::multimap<int, std::unique_ptr<ecs::Component>, std::greater<int>>::iterator it;
           for (it = ecs_components.begin(); it != ecs_components.end(); it++) {
               if (it->second->getPriority() == layer) {
                   return true;
               }
           }
           return false;
        }
        Component* getComponent(unsigned int layer) {
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
        std::vector<Component*> getComponents() {
            std::multimap<int, std::unique_ptr<Component>, std::greater<int>>::iterator it;
            std::vector<Component*> cpnts;
            for (it = components.begin(); it != components.end(); it++) {
                cpnts.push_back(it->second.get());
            }
            return cpnts;
        }
        std::vector<Component*> getRenderComponents() {
            std::multimap<int, std::unique_ptr<Component>, std::greater<int>>::iterator it;
            std::vector<Component*> cpnts;
            for (it = components.begin(); it != components.end(); it++) {
                if (it->second->getComponentType() == 0)
                    cpnts.push_back(it->second.get());
            }
            return cpnts;
        }
        std::vector<ecs::Component*> getECSComponents() {
            std::multimap<int, std::unique_ptr<ecs::Component>, std::greater<int>>::iterator it;
            std::vector<ecs::Component*> cpnts;
            for (it = ecs_components.begin(); it != ecs_components.end(); it++) {
                cpnts.push_back(it->second.get());
            }
            return cpnts;
        }
        void clearComponents() {
            std::multimap<int, std::unique_ptr<Component>, std::greater<int>>::iterator it;
            for (it = components.begin(); it != components.end(); it++) {
                if (it->second->isVisible()) {
                    it->second->clear();
                }
            }
        }
        void clearECSComponents() {
            std::multimap<int, std::unique_ptr<ecs::Component>, std::greater<int>>::iterator it;
            for (it = ecs_components.begin(); it != ecs_components.end(); it++) {
                if (it->second->isVisible()) {
                    it->second->clear();
                }
            }
        }
        void updateComponents() {
           std::multimap<int, Component*, std::greater<int>>::iterator it;
           for (it = eventComponents.begin(); it != eventComponents.end(); it++) {
               if (it->second->isEventContextActivated() && it->second->isVisible()) {
                   if (!it->second->getListener().isUsingThread()) {

                       it->second->processEvents();
                   }
                   //it->second->recomputeSize();
               }
           }
           //core::Command::clearEventsStack();
        }
        void updateECSComponents() {
           std::multimap<int, std::unique_ptr<ecs::Component>, std::greater<int>>::iterator it;
           for (it = ecs_components.begin(); it != ecs_components.end(); it++) {
               if (it->second->isEventContextActivated() && it->second->isVisible()) {
                   it->second->processEvents();
                   it->second->recomputeSize();
               }
           }
           //core::Command::clearEventsStack();
        }
        ~RCM() {
            //std::cout<<"rcm desrtructor"<<std::endl;

        }


        private:
        std::multimap<int, std::unique_ptr<Component>, std::greater<int>> components; /**> the components.*/
        std::multimap<int, std::unique_ptr<ecs::Component>, std::greater<int>> ecs_components;
            std::multimap<int, Component*, std::greater<int>> eventComponents;
            std::vector<RenderWindow*> windows; /**> the window.*/
};
int main(int argc, char *argv[]) {

    /*VkSettup instance;
    Device device(instance);*/

    /*RenderWindow window(VideoMode(800, 600), "test", Style::Default, ContextSettings(0, 0, 4, 4, 6));
    RenderComponentManager rcm(window);

    //window.getView().move(400, 300, 0);
    Component* component = new PerPixelLinkedListRenderComponent(window, 0, "", ContextSettings(0, 0, 4, 4, 6));
    std::vector<Entity*> entities;
    component->loadEntitiesOnComponent(entities);
    rcm.addComponent(component);
    /*std::multimap<int, std::unique_ptr<Component>, std::greater<int>> components;
    std::multimap<int, Component*, std::greater<int>> eventComponents;
    std::unique_ptr<Component> ptr;
    ptr.reset(component);
    components.insert(std::make_pair(component->getPriority(), std::move(ptr)));
    eventComponents.insert(std::make_pair(component->getEventPriority(), component));*/

    /*std::vector<RenderWindow*> windows;
    windows.push_back(&window);*/
    /*while (window.isOpen()) {
        window.clear();
        /*std::multimap<int, std::unique_ptr<Component>, std::greater<int>>::reverse_iterator rit;
        for (rit = components.rbegin(); rit != components.rend(); rit++) {
               if (rit->second->getComponentType() == 0 && rit->second->isVisible()) {
                   rit->second->clear();
                   //it->second->recomputeSize();
               }
           }
        std::multimap<int, Component*, std::greater<int>>::iterator it;
        for (it = eventComponents.begin(); it != eventComponents.end(); it++) {
               if (it->second->isEventContextActivated() && it->second->isVisible()) {
                   if (!it->second->getListener().isUsingThread()) {

                       it->second->processEvents();
                   }
                   //it->second->recomputeSize();
               }
           }

           for (rit = components.rbegin(); rit != components.rend(); rit++) {
               if (rit->second->getComponentType() == 0 && rit->second->isVisible()) {
                   windows[0]->draw(*rit->second.get());
                   //it->second->recomputeSize();
               }
           }*/

        //window.beginRenderPass();

        //window.endRenderPass();
        //window.submit(true);
       /* rcm.clearComponents();
        rcm.updateComponents();
        rcm.drawRenderComponents();
        window.display();
    }*/



    MyAppli app(VideoMode(800, 600), "Test odfaeg");
    return app.exec();
}




