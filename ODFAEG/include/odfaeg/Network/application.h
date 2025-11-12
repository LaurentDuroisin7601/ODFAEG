#ifndef ODFAEG_APPLICATION_HPP
#define ODFAEG_APPLICATION_HPP
#ifndef VULKAN
#include "GL/glew.h"
#else
#include "GLFW/glfw3.h"
#endif
#include "srkserveur.h"
#include "../Network/network.h"
#include "../Graphics/renderWindow.h"
#include "../Graphics/renderStates.h"
#include "../Graphics/renderComponentManager.h"
#include "../Graphics/world.h"
#include "../Graphics/color.hpp"
#include "export.hpp"

#include "../../../include/odfaeg/Window/iEvent.hpp"
#include "../Core/resourceCache.h"
#include <thread>


/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace core {
        /**
        * \file application.h
        * \class Application
        * \brief base class dedicated for all ODFAEG applications.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        #ifdef VULKAN
        template <typename A, typename T=std::string>
        class  Application {
            public :
             Application(window::VideoMode vm, std::string title, std::uint32_t style=window::Style::Default, window::ContextSettings settings = window::ContextSettings()) : vkDevice(vkSettup)

            {
                clearColor = graphic::Color::Black;
                graphic::RenderWindow* window = new graphic::RenderWindow (vm, title, vkDevice, style, settings);
                windows.push_back(std::make_pair(window, true));
                componentManager = std::make_unique<graphic::RenderComponentManager>(*window);
                //////std::cout<<"component manager : "<<componentManager.get()<<std::endl;
                app = this;
                running = false;
                core::Clock loopSpeed;
                addClock(loopSpeed, "LoopTime");
                core::Clock timeClock;
                addClock(timeClock, "TimeClock");
                listener = std::make_unique<Listener>();
                eventContextActivated = true;
                nbEntities = nbEntitiesTypes = nbComponents = nbMaterials = 0;
                world = nullptr;
            }
            /** \fn Application()
            *   \brief create a console odfaeg application.
            */
            Application () : vkDevice(vkSettup) {
                componentManager = nullptr;
                app = this;
                running = false;
                core::Clock loopTime;
                addClock(loopTime, "LoopTime");
                core::Clock timeClock;
                addClock(timeClock, "TimeClock");
                eventContextActivated = true;
                nbEntities = nbEntitiesTypes = nbComponents = nbMaterials = 0;
                world = nullptr;
            }
            void setCurrentWorld(graphic::World* world) {
                this->world = world;
            }
            unsigned int getNbMaterials() {
                return nbMaterials;
            }
            void setNbMaterials(unsigned int nbMaterials) {
                this->nbMaterials = nbMaterials;
            }
            std::vector<graphic::Material*>& getMaterials() {
                return materials;
            }
            std::vector<graphic::Material*>& getSameMaterials() {
                return sameMaterials;
            }
            int getIntOfType(std::string sType) {
                std::map<int, std::string>::iterator it;
                for (it = types.begin(); it != types.end(); ++it) {
                    if (it->second == sType)
                        return it->first;
                }
                return -1;
            }
            int getUniqueId() {
                nbEntities++;
                return nbEntities;
            }
            int getComponentId() {
                nbComponents++;
                return nbComponents;
            }
            std::pair<int, std::string> updateTypes(std::string sType) {
                int iType = getIntOfType(sType);
                if (iType == -1) {
                    std::pair<int, std::string> type = std::pair<int, std::string> (nbEntitiesTypes, sType);
                    types.insert(type);
                    nbEntitiesTypes++;
                    return type;
                } else {
                    std::map<int, std::string>::iterator it = types.find(iType);
                    return *it;
                }
            }
            unsigned int getNbEntities() {
                return nbEntities;
            }
            unsigned int getNbEntitiesTypes() {
                return nbEntitiesTypes;
            }
            void addWindow(graphic::RenderWindow* window, bool holdWindow) {
                if (windows.size() != 0) {
                    windows.push_back(std::make_pair(window, holdWindow));
                    componentManager->addWindow(*window);
                }
            }
            /** \fn int exec()
            *   \brief main loop of the odfaeg application.
            *   \return true if the application runs without errors, false otherwise.
            */
            int exec() {
                load();
                init();
                running = true;
                while (running) {

                    if (windows.size() != 0 && windows[0].first->isOpen()) {
                        //rendering_thread = std::thread(Application::render, this);
                        render();
                        update();
                    }
                    if (network::Network::getCliInstance().isRunning()) {
                        network::Network::getCliInstance().checkMessages();
                    }
                    if (network::Network::getSrvInstance().isRunning()) {
                        network::Network::getSrvInstance().checkMessages<A, T>();
                    }
                    static_cast<A*>(this)->onExec();
                    getClock("LoopTime").restart();
                }
                return EXIT_SUCCESS;
            }
            /** \fn void stop()
            *   \brief stop the odfaeg application and close the window if it's a graphic application.
            */
            void stop() {
                running = false;

                for(unsigned int i = 0; i < windows.size(); i++)
                    windows[i].first->close();
            }
            /** \fn void load()
            *   \brief call the onLoad function, this is where all resources used by the application are loaded.
            */
            void load() {
                static_cast<A*>(this)->onLoad();
            }
            /** \fn void init()
            *   \brief call the onInit function, this is where all the entities used by the application are initialized.
            */
            void init() {
                static_cast<A*>(this)->onInit();
            }
            /** \fn void render()
            *   \brief call the rendering functions used to render entities on components or on the window.
            */
            void render() {
                if (windows.size() != 0 && windows[0].first->isOpen()) {
                    for (unsigned int i = 0; i < windows.size(); i++) {
                        windows[i].first->clear(clearColor);
                    }
                    static_cast<A*>(this)->onRender(componentManager.get());
                    componentManager->clearComponents();
                    if (eventContextActivated) {
                       listener->processEvents();
                    }
                    componentManager->updateComponents();
                    componentManager->drawRenderComponents();
                    ////std::cout<<"render components drawn"<<std::endl;
                    static_cast<A*>(this)->onDisplay(windows[0].first);
                    componentManager->drawGuiComponents();
                    ////////std::cout<<"window size : "<<windows.size()<<std::endl;
                    for (unsigned int i = 0; i < windows.size(); i++) {
                        ////std::cout<<"submit window"<<std::endl;
                        windows[i].first->beginRecordCommandBuffers();
                        windows[i].first->submit(true);
                        windows[i].first->display();
                        ////std::cout<<"submitted window"<<std::endl;

                    }
                }
            }
            void setEventContextActivated(bool eventContextActivated) {
                this->eventContextActivated = eventContextActivated;
                if (!eventContextActivated)
                    listener->clearEventsStack();
            }
            bool isEventContextActivated() {
                return eventContextActivated;
            }
            /** \fn void update()
            *   \brief call the updates functions to update the entities
            *   filter the window::IEvents and pass window events which are generated to the listener.
            */
            void update() {
                if (windows.size() != 0) {
                    window::IEvent event;
                    events.clear();
                    for (unsigned int i = 0; i < windows.size(); i++) {
                        while (windows[i].first->pollEvent(event)) {
                            events.insert(std::make_pair(windows[i].first, event));
                        }
                    }
                    if (events.size() > 0) {
                        for (it = events.begin(); it != events.end(); it++) {

                            if (eventContextActivated) {
                                listener->pushEvent(it->second);
                            }
                            for (unsigned int i = 0; i < componentManager->getNbComponents(); i++) {
                                componentManager->getComponent(i)->onUpdate(it->first, it->second);
                                if (componentManager->getComponent(i)->isEventContextActivated()) {
                                    componentManager->getComponent(i)->pushEvent(it->second, *(it->first));
                                }
                            }
                            static_cast<A*>(this)->onUpdate(it->first, it->second);
                        }
                    }
                }
                //world.updateTimers();
            }
            /**
            * \fn void onLoad()
            * \brief function which can be redefined if the application have to load resources at the start.
            */
            void onLoad (){}
            /**
            * \fn void onLoad()
            * \brief function which can be redefined if the application have to init entities at the start.
            */
            void onInit() {}
            /**
            * \fn void onLoad()
            * \brief function which can be redefined if the application have to render entities on components.
            * \param RenderComponentManager : the manager of all render components.
            */
            void onRender (graphic::RenderComponentManager* cm){}
            /**
            * \fn void onLoad()
            * \brief function which can be redefined if the application have to render entities on the window.
            */
            void onDisplay(graphic::RenderWindow *rw){}
            /**
            * \fn void onUpdate()
            * \brief function which can be redefined if the application have to update entities when window's events are generated.
            * \param the generated event.
            */
            void onUpdate (graphic::RenderWindow* window, window::IEvent& event) {}
            /**
            * \fn void onExec()
            * \brief function which can be redefined if the application need to do something at each loop.
            * by example if the application need to do something when a networking message is arrived.
            */
            void onExec() {}
            /** \fn void addClock(core::Clock clock, std::string name)
            *   \brief add a clock to the application, the clock is so accessible everywhere in the source code.
            *   \param Clock : the clock to add.
            *   \param name : a name which identify the clock.
            */
            void addClock(core::Clock clock, std::string name)  {

                clocks.insert(std::pair<std::string, core::Clock>(name, clock));
            }
            /** \fn core::Clock& getClock(std::string name)
            *   \brief return a clock from its name.
            *   \param std::string : the name of the clock.
            *   \return the clock corresponding to the given name.
            */
            core::Clock& getClock(std::string name) {
                std::map<std::string, core::Clock>::iterator it = clocks.find(name);
                if (it == clocks.end())
                    throw Erreur (14, "Clock not found!", 0);
                return it->second;
            }
            /** \fn RenderWindow& getRenderWindow()
            *   \brief return a reference to the windows of the appliation
            *   \return the render window of the application.
            */
            graphic::RenderWindow& getRenderWindow(unsigned int i=0) {
                ////////std::cout<<windows[i].first<<std::endl;
                return *windows[i].first;
            }
            unsigned int getNbWindows() {
                return windows.size();
            }
            /** \fn RenderComponentManager& getRenderComponentManager()
            *   \brief return a reference to the render component manager.
            *   \return the render component manager of the application.
            */
            graphic::RenderComponentManager& getRenderComponentManager() {
                return *componentManager;
            }
            /** \fn View& getView()
            *   \brief return a reference to the current view of the window.
            *   \return a reference to the view.
            */
            graphic::View& getView() {
                return windows[0].first->getView();
            }
            /** \fn View& getDefaultView()
            *   \brief return the default view of the window.
            *   \return the default view.
            */
            graphic::View getDefaultView() {
                return windows[0].first->getDefaultView();
            }
            /** \fn setCleargraphic::Color (graphic::Color cleargraphic::Color)
            *   \brief define the clear color of the window.
            *   \param graphic::Color : the clear color.
            */
            void setClearColor(graphic::Color clearColor) {
                this->clearColor = clearColor;
            }
            Listener& getListener() {
                return *listener;
            }
            /** \fn ~Application()
            *   \brief destructor : stop the application and destroy the window if any.
            */
            void onDisconnected(network::User* user) {
            }
            graphic::World* getWorld() {
                return world;
            }
            window::Device& getDevice() {
                return vkDevice;
            }
            core::Clock getTimeClk() {
                return timeClk;
            }
            ResourceCache<T>& getResourceCache() {
                return resourceCache;
            }
            ~Application() {
                //////std::cout<<"destroy appli : "<<componentManager.get()<<std::endl;

                stop();
                for (unsigned int i = 0; i < windows.size(); i++) {
                    if (windows[i].second)
                        delete windows[i].first;
                }
            }

            static Application<A, T>* app;
        private :
            window::VkSettup vkSettup;
            window::Device vkDevice;
            std::map<std::string, core::Clock> clocks; /** > all the clocks used by the application to measure the time.*/
            bool running; /** > determine if the application running or not.*/
            graphic::Color clearColor; /** > keep the clear color of the window*/
            std::multimap<graphic::RenderWindow*, window::IEvent> events; /** > store the windows events generated by the application*/
            std::multimap<graphic::RenderWindow*, window::IEvent>::iterator it; /** > an iterator to the window::IEvents generated by the application*/
            std::unique_ptr<Listener> listener;
            bool eventContextActivated;
            static core::Clock timeClk;
            /*std::thread rendering_thread;
            std::recursive_rec_mutex rec_rec_mutex;*/
            graphic::World* world;
            unsigned int nbEntities, nbEntitiesTypes, nbComponents, nbMaterials;
            std::vector<graphic::Material*> materials;
            std::vector<graphic::Material*> sameMaterials;
            std::map<int, std::string> types;
            std::vector<std::pair<graphic::RenderWindow*, bool>> windows; /** > the render window*/
            std::unique_ptr<graphic::RenderComponentManager> componentManager; /** > the render component manager which draw components on the window*/
            ResourceCache<T> resourceCache;
        };
        template<typename A, typename T>
        Application<A, T>* Application<A, T>::app = nullptr;
        #else
        template <typename A, typename T=std::string>
        class Application {
        public :
            std::string name;
            /** \fn Application(sf::VideoMode, std::string title, int nbComponents, bool depthDepth, sf::Uint32 style, sf::ContetSettings settings)
            *   \brief create a graphic odfaeg application.
            *   \param sf::VideoMode : the video mode. (the size of the window)
            *   \param std::string : the title of the window.
            *   \param sf::Uint32 : the window's style.
            *   \param ContextSettings : advanced opengl settings.
            */
            Application(window::VideoMode vm, std::string title, std::uint32_t style = window::Style::Default, window::ContextSettings settings = window::ContextSettings())

            {
                clearColor = graphic::Color::Black;
                graphic::RenderWindow* window = new graphic::RenderWindow (vm, title, style, settings);
                windows.push_back(std::make_pair(window, true));
                componentManager = std::make_unique<graphic::RenderComponentManager>(*window);
                app = this;
                running = false;
                core::Clock loopSpeed;
                addClock(loopSpeed, "LoopTime");
                core::Clock timeClock;
                addClock(timeClock, "TimeClock");
                listener = std::make_unique<Listener>();
                eventContextActivated = true;
                nbEntities = nbEntitiesTypes = nbComponents = nbMaterials = 0;
                world = nullptr;
            }
            /** \fn Application()
            *   \brief create a console odfaeg application.
            */
            Application () {
                app = this;
                running = false;
                core::Clock loopTime;
                addClock(loopTime, "LoopTime");
                core::Clock timeClock;
                addClock(timeClock, "TimeClock");
                eventContextActivated = true;
                nbEntities = nbEntitiesTypes = nbComponents = nbMaterials = 0;
                world = nullptr;
            }
            unsigned int getNbMaterials() {
                return nbMaterials;
            }
            void setNbMaterials(unsigned int nbMaterials) {
                this->nbMaterials = nbMaterials;
            }
            std::vector<graphic::Material*>& getMaterials() {
                return materials;
            }
            std::vector<graphic::Material*>& getSameMaterials() {
                return sameMaterials;
            }
            int getIntOfType(std::string sType) {
                std::map<int, std::string>::iterator it;
                for (it = types.begin(); it != types.end(); ++it) {
                    if (it->second == sType)
                        return it->first;
                }
                return -1;
            }
            int getUniqueId() {
                nbEntities++;
                return nbEntities;
            }
            int getComponentId() {
                nbComponents++;
                return nbComponents;
            }
            std::pair<int, std::string> updateTypes(std::string sType) {
                int iType = getIntOfType(sType);
                if (iType == -1) {
                    std::pair<int, std::string> type = std::pair<int, std::string> (nbEntitiesTypes, sType);
                    types.insert(type);
                    nbEntitiesTypes++;
                    return type;
                } else {
                    std::map<int, std::string>::iterator it = types.find(iType);
                    return *it;
                }
            }
            unsigned int getNbEntities() {
                return nbEntities;
            }
            unsigned int getNbEntitiesTypes() {
                return nbEntitiesTypes;
            }
            void addWindow(graphic::RenderWindow* window, bool holdWindow = true) {
                if (windows.size() != 0) {
                    windows.push_back(std::make_pair(window, holdWindow));
                    componentManager->addWindow(*window);
                }
            }
            /** \fn int exec()
            *   \brief main loop of the odfaeg application.
            *   \return true if the application runs without errors, false otherwise.
            */
            int exec() {
                load();
                init();
                running = true;
                while (running) {
                    /*for (unsigned int i = 0; i < windows.size(); i++) {
                        windows[i].first->setActive(false);
                    }*/
                    if (windows.size() != 0 && windows[0].first->isOpen()) {
                        //rendering_thread = std::thread(Application::render, this);
                        render();
                        update();
                    }
                    if (network::Network::getCliInstance().isRunning()) {
                        network::Network::getCliInstance().checkMessages();
                    }
                    if (network::Network::getSrvInstance().isRunning()) {
                        network::Network::getSrvInstance().checkMessages<A, T>();
                    }
                    static_cast<A*>(this)->onExec();
                    getClock("LoopTime").restart();
                }
                return EXIT_SUCCESS;
            }
            /** \fn void stop()
            *   \brief stop the odfaeg application and close the window if it's a graphic application.
            */
            void stop() {
                running = false;

                //rendering_thread.join();
                for(unsigned int i = 0; i < windows.size(); i++)
                    windows[i].first->close();
            }
            /** \fn void load()
            *   \brief call the onLoad function, this is where all resources used by the application are loaded.
            */
            void load() {
                static_cast<A*>(this)->onLoad();
            }
            /** \fn void init()
            *   \brief call the onInit function, this is where all the entities used by the application are initialized.
            */
            void init() {
                static_cast<A*>(this)->onInit();
            }
            /** \fn void render()
            *   \brief call the rendering functions used to render entities on components or on the window.
            */
            void render() {
                if (windows.size() != 0 && windows[0].first->isOpen()) {
                    for (unsigned int i = 0; i < windows.size(); i++) {
                        windows[i].first->clear(clearColor);
                    }
                    static_cast<A*>(this)->onRender(componentManager.get());
                    componentManager->clearComponents();
                    //componentManager->clearECSComponents();
                    componentManager->updateComponents();
                    if (eventContextActivated) {
                       listener->processEvents();
                    }

                    componentManager->drawRenderComponents();
                    //componentManager->drawECSComponents();

                    static_cast<A*>(this)->onDisplay(windows[0].first);
                    componentManager->drawGuiComponents();
                    for (unsigned int i = 0; i < windows.size(); i++) {
                        //////std::cout<<"display"<<std::endl;
                        windows[i].first->display();
                    }
                }
            }
            void setEventContextActivated(bool eventContextActivated) {
                this->eventContextActivated = eventContextActivated;
                if (!eventContextActivated)
                    listener->clearEventsStack();
            }
            bool isEventContextActivated() {
                return eventContextActivated;
            }
            /** \fn void update()
            *   \brief call the updates functions to update the entities
            *   filter the window::IEvents and pass window events which are generated to the listener.
            */
            void update() {
                if (windows.size() != 0) {
                    window::IEvent event;
                    events.clear();
                    for (unsigned int i = 0; i < windows.size(); i++) {
                        while (windows[i].first->pollEvent(event)) {
                            events.insert(std::make_pair(windows[i].first, event));
                        }
                    }
                    if (events.size() > 0) {
                        for (it = events.begin(); it != events.end(); it++) {
                            static_cast<A*>(this)->onUpdate(it->first, it->second);
                            if (eventContextActivated) {
                                listener->pushEvent(it->second);
                            }
                            for (unsigned int i = 0; i < componentManager->getNbComponents(); i++) {
                                componentManager->getComponent(i)->onUpdate(it->first, it->second);
                                if (componentManager->getComponent(i)->isEventContextActivated()) {
                                    componentManager->getComponent(i)->pushEvent(it->second, *(it->first));
                                }
                            }
                        }
                    }
                }
                //world.updateTimers();
            }
            /**
            * \fn void onLoad()
            * \brief function which can be redefined if the application have to load resources at the start.
            */
            void onLoad (){}
            /**
            * \fn void onLoad()
            * \brief function which can be redefined if the application have to init entities at the start.
            */
            void onInit() {}
            /**
            * \fn void onLoad()
            * \brief function which can be redefined if the application have to render entities on components.
            * \param RenderComponentManager : the manager of all render components.
            */
            void onRender (graphic::RenderComponentManager* cm){}
            /**
            * \fn void onLoad()
            * \brief function which can be redefined if the application have to render entities on the window.
            */
            void onDisplay(graphic::RenderWindow *rw){}
            /**
            * \fn void onUpdate()
            * \brief function which can be redefined if the application have to update entities when window's events are generated.
            * \param the generated event.
            */
            void onUpdate (graphic::RenderWindow* window, window::IEvent& event) {}
            /**
            * \fn void onExec()
            * \brief function which can be redefined if the application need to do something at each loop.
            * by example if the application need to do something when a networking message is arrived.
            */
            void onExec() {}
            /** \fn void addClock(core::Clock clock, std::string name)
            *   \brief add a clock to the application, the clock is so accessible everywhere in the source code.
            *   \param Clock : the clock to add.
            *   \param name : a name which identify the clock.
            */
            void addClock(core::Clock clock, std::string name)  {

                clocks.insert(std::pair<std::string, core::Clock>(name, clock));
            }
            /** \fn core::Clock& getClock(std::string name)
            *   \brief return a clock from its name.
            *   \param std::string : the name of the clock.
            *   \return the clock corresponding to the given name.
            */
            core::Clock& getClock(std::string name) {
                std::map<std::string, core::Clock>::iterator it = clocks.find(name);
                if (it == clocks.end())
                    throw Erreur (14, "Clock not found!", 0);
                return it->second;
            }
            /** \fn RenderWindow& getRenderWindow()
            *   \brief return a reference to the windows of the appliation
            *   \return the render window of the application.
            */
            graphic::RenderWindow& getRenderWindow(unsigned int i = 0) {
                ////////std::cout<<windows[i].first<<std::endl;
                return *windows[i].first;
            }
            unsigned int getNbWindows() {
                return windows.size();
            }
            /** \fn RenderComponentManager& getRenderComponentManager()
            *   \brief return a reference to the render component manager.
            *   \return the render component manager of the application.
            */
            graphic::RenderComponentManager& getRenderComponentManager() {
                return *componentManager;
            }
            /** \fn View& getView()
            *   \brief return a reference to the current view of the window.
            *   \return a reference to the view.
            */
            graphic::View& getView() {
                return windows[0].first->getView();
            }
            /** \fn View& getDefaultView()
            *   \brief return the default view of the window.
            *   \return the default view.
            */
            graphic::View getDefaultView() {
                return windows[0].first->getDefaultView();
            }
            /** \fn setCleargraphic::Color (graphic::Color cleargraphic::Color)
            *   \brief define the clear color of the window.
            *   \param graphic::Color : the clear color.
            */
            void setClearColor(graphic::Color clearColor) {
                this->clearColor = clearColor;
            }
            Listener& getListener() {
                return *listener;
            }
            /** \fn ~Application()
            *   \brief destructor : stop the application and destroy the window if any.
            */
            virtual void onDisconnected(network::User* user) {
            }
            graphic::World* getWorld() {
                return world;
            }
            virtual ~Application() {
                stop();
                for (unsigned int i = 0; i < windows.size(); i++) {
                    if (windows[i].second)
                        delete windows[i].first;
                }
            }
            static core::Clock& getTimeClk() {
                return timeClk;
            }
            ResourceCache<T>& getResourceCache() {
                return resourceCache;
            }
            void setCurrentWorld(graphic::World* world) {
                this->world = world;
            }
            /** > a pointer to the current odfaeg application*/
            static Application<A, T>* app;
        private :

            std::unique_ptr<graphic::RenderComponentManager> componentManager; /** > the render component manager which draw components on the window*/
            std::vector<std::pair<graphic::RenderWindow*, bool>> windows; /** > the render window*/
            std::map<std::string, core::Clock> clocks; /** > all the clocks used by the application to measure the time.*/
            bool running; /** > determine if the application running or not.*/
            graphic::Color clearColor; /** > keep the clear color of the window*/
            std::multimap<graphic::RenderWindow*, window::IEvent> events; /** > store the windows events generated by the application*/
            std::multimap<graphic::RenderWindow*, window::IEvent>::iterator it; /** > an iterator to the window::IEvents generated by the application*/
            std::unique_ptr<Listener> listener;
            bool eventContextActivated;
            static core::Clock timeClk;
            /*std::thread rendering_thread;
            std::recursive_rec_mutex rec_rec_mutex;*/
            graphic::World* world;
            unsigned int nbEntities, nbEntitiesTypes, nbComponents, nbMaterials;
            std::vector<graphic::Material*> materials;
            std::vector<graphic::Material*> sameMaterials;
            std::map<int, std::string> types;
            ResourceCache<T> resourceCache;
        };
        template <typename A, typename T>
        Application<A, T>* Application<A, T>::app = nullptr;
        #endif
    }
}
#endif
