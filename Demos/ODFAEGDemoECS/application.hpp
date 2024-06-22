#ifndef APPLICATION_DEMO_ECS_HPP
#define APPLICATION_DEMO_ECS_HPP
#include "odfaeg/Graphics/ECS/application.hpp"
#include "odfaeg/Graphics/ECS/scene.hpp"
#include "odfaeg/Graphics/ECS/perPixelLinkedList.hpp"
#include "odfaeg/Core/ecs.hpp"
#include "odfaeg/Graphics/ECS/modelFactory.hpp"
#include "odfaeg/Graphics/ECS/entitiesUpdater.hpp"
#include "odfaeg/Graphics/ECS/animationUpdater.hpp"
#include "odfaeg/Window/command.h"
#include "odfaeg/Window/iKeyboard.hpp"
#include "odfaeg/Window/iMouse.hpp"
#include "odfaeg/Core/resourceCache.h"
#include <fstream>
#include "odfaeg/Physics/ECS/particleSystem.hpp"
#include "odfaeg/Math/distributions.h"
namespace sorrok {
    class MyECSAppli : public odfaeg::core::ecs::Application {
        public:
        MyECSAppli(sf::VideoMode vm, std::string title);
        void onLoad();
        void onInit();
        void onRender(odfaeg::graphic::RenderComponentManager *cm);
        void onDisplay(odfaeg::graphic::RenderWindow* window);
        void onUpdate (odfaeg::graphic::RenderWindow* window, odfaeg::window::IEvent& event);
        void onExec ();
        odfaeg::physic::UniversalEmitter emitter;
        odfaeg::core::ResourceCache<> cache;
        unsigned int fpsCounter;
        odfaeg::graphic::ecs::EntityFactory factory;
        odfaeg::graphic::ecs::Scene* scene;
        odfaeg::graphic::ecs::EntitiesUpdater* eu;
    };
}
#endif // APPLICATION_DEMO_HPP
