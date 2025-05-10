#include "../../../include/odfaeg/Graphics/application.h"
namespace odfaeg {
    namespace core {
        using namespace sf;
        Application* Application::app = nullptr;
        Clock Application::timeClk = Clock();
        Application::Application(sf::VideoMode vm, std::string title, sf::Uint32 style, window::ContextSettings settings) : vkDevice(vkSettup)

        {
            clearColor = sf::Color::Black;
            graphic::RenderWindow* window = new graphic::RenderWindow (vm, title, vkDevice, style, settings);
            windows.push_back(std::make_pair(window, true));
            componentManager = std::make_unique<graphic::RenderComponentManager>(*window);
            std::cout<<"component manager : "<<componentManager.get()<<std::endl;
            app = this;
            running = false;
            sf::Clock loopSpeed;
            addClock(loopSpeed, "LoopTime");
            sf::Clock timeClock;
            addClock(timeClock, "TimeClock");
            listener = std::make_unique<Listener>();
            eventContextActivated = true;
            nbEntities = nbEntitiesTypes = nbComponents = nbMaterials = 0;
        }
        /** \fn Application()
        *   \brief create a console odfaeg application.
        */
        Application::Application () : vkDevice(vkSettup) {
            componentManager = nullptr;
            app = this;
            running = false;
            sf::Clock loopTime;
            addClock(loopTime, "LoopTime");
            sf::Clock timeClock;
            addClock(timeClock, "TimeClock");
            eventContextActivated = true;
            nbEntities = nbEntitiesTypes = nbComponents = nbMaterials = 0;
        }
        unsigned int Application::getNbMaterials() {
            return nbMaterials;
        }
        void Application::setNbMaterials(unsigned int nbMaterials) {
            this->nbMaterials = nbMaterials;
        }
        std::vector<graphic::Material*>& Application::getMaterials() {
            return materials;
        }
        std::vector<graphic::Material*>& Application::getSameMaterials() {
            return sameMaterials;
        }
        int Application::getIntOfType(std::string sType) {
            std::map<int, std::string>::iterator it;
            for (it = types.begin(); it != types.end(); ++it) {
                if (it->second == sType)
                    return it->first;
            }
            return -1;
        }
        int Application::getUniqueId() {
            nbEntities++;
            return nbEntities;
        }
        int Application::getComponentId() {
            nbComponents++;
            return nbComponents;
        }
        std::pair<int, std::string> Application::updateTypes(std::string sType) {
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
        unsigned int Application::getNbEntities() {
            return nbEntities;
        }
        unsigned int Application::getNbEntitiesTypes() {
            return nbEntitiesTypes;
        }
        void Application::addWindow(graphic::RenderWindow* window, bool holdWindow) {
            if (windows.size() != 0) {
                windows.push_back(std::make_pair(window, holdWindow));
                componentManager->addWindow(*window);
            }
        }
        /** \fn int exec()
        *   \brief main loop of the odfaeg application.
        *   \return true if the application runs without errors, false otherwise.
        */
        int Application::exec() {
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
                    network::Network::getSrvInstance().checkMessages();
                }
                onExec();
                getClock("LoopTime").restart();
            }
            return EXIT_SUCCESS;
        }
        /** \fn void stop()
        *   \brief stop the odfaeg application and close the window if it's a graphic application.
        */
        void Application::stop() {
            running = false;
            //rendering_thread.join();
            for(unsigned int i = 0; i < windows.size(); i++)
                windows[i].first->close();
        }
        /** \fn void load()
        *   \brief call the onLoad function, this is where all resources used by the application are loaded.
        */
        void Application::load() {
            onLoad();
        }
        /** \fn void init()
        *   \brief call the onInit function, this is where all the entities used by the application are initialized.
        */
        void Application::init() {
            onInit();
        }
        /** \fn void render()
        *   \brief call the rendering functions used to render entities on components or on the window.
        */
        void Application::render() {
            if (windows.size() != 0 && windows[0].first->isOpen()) {
                for (unsigned int i = 0; i < windows.size(); i++) {
                    windows[i].first->clear(clearColor);
                }
                onRender(componentManager.get());
                componentManager->clearComponents();
                if (eventContextActivated) {
                   listener->processEvents();
                }
                componentManager->updateComponents();
                componentManager->drawRenderComponents();
                onDisplay(windows[0].first);
                componentManager->drawGuiComponents();
                for (unsigned int i = 0; i < windows.size(); i++)
                    windows[i].first->display();
            }
        }
        void Application::setEventContextActivated(bool eventContextActivated) {
            this->eventContextActivated = eventContextActivated;
            if (!eventContextActivated)
                listener->clearEventsStack();
        }
        bool Application::isEventContextActivated() {
            return eventContextActivated;
        }
        /** \fn void update()
        *   \brief call the updates functions to update the entities
        *   filter the window::IEvents and pass window events which are generated to the listener.
        */
        void Application::update() {
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
                        onUpdate(it->first, it->second);
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
            world.updateTimers();
        }
        /**
        * \fn void onLoad()
        * \brief function which can be redefined if the application have to load resources at the start.
        */
        void Application::onLoad (){}
        /**
        * \fn void onLoad()
        * \brief function which can be redefined if the application have to init entities at the start.
        */
        void Application::onInit() {}
        /**
        * \fn void onLoad()
        * \brief function which can be redefined if the application have to render entities on components.
        * \param RenderComponentManager : the manager of all render components.
        */
        void Application::onRender (graphic::RenderComponentManager* cm){}
        /**
        * \fn void onLoad()
        * \brief function which can be redefined if the application have to render entities on the window.
        */
        void Application::onDisplay(graphic::RenderWindow *rw){}
        /**
        * \fn void onUpdate()
        * \brief function which can be redefined if the application have to update entities when window's events are generated.
        * \param the generated event.
        */
        void Application::onUpdate (graphic::RenderWindow* window, window::IEvent& event) {}
        /**
        * \fn void onExec()
        * \brief function which can be redefined if the application need to do something at each loop.
        * by example if the application need to do something when a networking message is arrived.
        */
        void Application::onExec() {}
        /** \fn void addClock(sf::Clock clock, std::string name)
        *   \brief add a clock to the application, the clock is so accessible everywhere in the source code.
        *   \param Clock : the clock to add.
        *   \param name : a name which identify the clock.
        */
        void Application::addClock(sf::Clock clock, std::string name)  {

            clocks.insert(std::pair<std::string, sf::Clock>(name, clock));
        }
        /** \fn sf::Clock& getClock(std::string name)
        *   \brief return a clock from its name.
        *   \param std::string : the name of the clock.
        *   \return the clock corresponding to the given name.
        */
        sf::Clock& Application::getClock(std::string name) {
            std::map<std::string, sf::Clock>::iterator it = clocks.find(name);
            if (it == clocks.end())
                throw Erreur (14, "Clock not found!", 0);
            return it->second;
        }
        /** \fn RenderWindow& getRenderWindow()
        *   \brief return a reference to the windows of the appliation
        *   \return the render window of the application.
        */
        graphic::RenderWindow& Application::getRenderWindow(unsigned int i) {
            //std::cout<<windows[i].first<<std::endl;
            return *windows[i].first;
        }
        unsigned int Application::getNbWindows() {
            return windows.size();
        }
        /** \fn RenderComponentManager& getRenderComponentManager()
        *   \brief return a reference to the render component manager.
        *   \return the render component manager of the application.
        */
        graphic::RenderComponentManager& Application::getRenderComponentManager() {
            return *componentManager;
        }
        /** \fn View& getView()
        *   \brief return a reference to the current view of the window.
        *   \return a reference to the view.
        */
        graphic::View& Application::getView() {
            return windows[0].first->getView();
        }
        /** \fn View& getDefaultView()
        *   \brief return the default view of the window.
        *   \return the default view.
        */
        graphic::View Application::getDefaultView() {
            return windows[0].first->getDefaultView();
        }
        /** \fn setClearColor (sf::Color clearColor)
        *   \brief define the clear color of the window.
        *   \param sf::Color : the clear color.
        */
        void Application::setClearColor(sf::Color clearColor) {
            this->clearColor = clearColor;
        }
        Listener& Application::getListener() {
            return *listener;
        }
        /** \fn ~Application()
        *   \brief destructor : stop the application and destroy the window if any.
        */
        void Application::onDisconnected(network::User* user) {
        }
        graphic::World* Application::getWorld() {
            return &world;
        }
        window::Device& Application::getDevice() {
            return vkDevice;
        }
        sf::Clock Application::getTimeClk() {
            return timeClk;
        }
        Application::~Application() {
            std::cout<<"rcm : "<<componentManager.get()<<std::endl;

            stop();
            for (unsigned int i = 0; i < windows.size(); i++) {
                if (windows[i].second)
                    delete windows[i].first;
            }
        }
    }
}
