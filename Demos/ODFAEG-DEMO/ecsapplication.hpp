#ifndef ECS_APPLI
#define ECS_APPLI
#include "../../ODFAEG/include/odfaeg/Graphics/ECS/application.hpp"
#include "../../ODFAEG/include/odfaeg/Core/ecs.hpp"
#include "../../ODFAEG/include/odfaeg/Graphics/ECS/modelFactory.hpp"
#include "odfaeg/Window/command.h"
#include "odfaeg/Window/iKeyboard.hpp"
#include "odfaeg/Window/iMouse.hpp"
#include "../../ODFAEG/include/odfaeg/Core/resourceCache.h"
#include <fstream>
#include "../../ODFAEG/include/odfaeg/Core/dynamicTuple.hpp"
namespace sorrok {
    class MyECSAppli : public odfaeg::graphic::ecs::Application<MyECSAppli> {
        public:
        MyECSAppli(sf::VideoMode vm, std::string title);
        void onLoad();
        template <typename SystemArray, typename SceneArray, typename RendererArray, typename EntityComponentArray>
        void onInit (SystemArray& sya, SceneArray& sa, RendererArray& ra, EntityComponentArray& eca) {
            odfaeg::core::TextureManager<> &tm = cache.resourceManager<odfaeg::graphic::Texture, std::string>("TextureManager");
            odfaeg::graphic::ecs::EntityId sceneId = factory.createEntity();
            auto sa2 = odfaeg::graphic::ecs::ModelFactory::createGridSceneModel(*getWorld(), sa, factory, sceneId, "Map Test", 100, 50, 0);
            world.setCurrentScene(sceneId);
            odfaeg::graphic::ecs::EntityId tileId = factory.createEntity();
            odfaeg::graphic::ecs::ModelFactory::createTileModel(*getWorld(), eca, factory, tileId, tm.getResourceByAlias("GRASS"), odfaeg::math::Vec3f(0, 0, 0), odfaeg::math::Vec3f(120, 60, 0),sf::IntRect(0, 0, 100, 50));
            odfaeg::physic::BoundingBox mapZone(0, 0, 0, 1500, 1000, 0);
            std::vector<odfaeg::graphic::ecs::EntityId> tGround = {tileId};
            std::vector<odfaeg::graphic::ecs::EntityId> tWall;
            getWorld()->generate_map(sa, eca, tGround, tWall, odfaeg::math::Vec2f(100, 50), mapZone);
            odfaeg::graphic::ecs::EntityId perPixelLinkedListRendererId = factory.createEntity();
            odfaeg::graphic::ecs::ModelFactory::createPerPixelLinkedListBindlessRenderComponent(*getWorld(), factory, perPixelLinkedListRendererId, ra, getRenderWindow(), 0, "E_TILE");
            exec(sya, sa2, ra, eca);
        }
        template <typename SystemArray, typename SceneArray, typename RendererArray, typename EntityComponentArray>
        void onRender(SystemArray& sya, SceneArray& sa, RendererArray& ra, EntityComponentArray& eca, odfaeg::graphic::RenderComponentManager *cm) {
        }
        template <typename SystemArray, typename SceneArray, typename RendererArray, typename EntityComponentArray>
        void onDisplay(SystemArray& sya, SceneArray& sa, RendererArray& ra, EntityComponentArray& eca, odfaeg::graphic::RenderWindow* window){
        }
        template <typename SystemArray, typename SceneArray, typename RendererArray, typename EntityComponentArray>
        void onUpdate (SystemArray& sya, SceneArray& sa, RendererArray& ra, EntityComponentArray& eca, odfaeg::graphic::RenderWindow* window, odfaeg::window::IEvent& event) {
            if (window == &getRenderWindow() && event.type == odfaeg::window::IEvent::WINDOW_EVENT && event.window.type == odfaeg::window::IEvent::WINDOW_EVENT_CLOSED) {
                stop();
            }
        }
        template <typename SystemArray, typename SceneArray, typename RendererArray, typename EntityComponentArray>
        void onExec (SystemArray& sya, SceneArray& sa, RendererArray& ra, EntityComponentArray& eca) {
        }
        private :
        odfaeg::core::ResourceCache<> cache;
        unsigned int fpsCounter;
        odfaeg::graphic::ecs::EntityFactory factory;
    };
}
#endif
