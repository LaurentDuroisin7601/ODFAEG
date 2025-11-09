
#include "odfaeg/Network/application.h"
#include "odfaeg/Graphics/3D/cube.h"
#include "odfaeg/Graphics/3D/model.hpp"
#include "odfaeg/Graphics/map.h"
#include "odfaeg/Graphics/tGround.h"
#include "odfaeg/Graphics/entitiesUpdater.h"
#include "odfaeg/Graphics/billBoard.h"
#include "odfaeg/Graphics/world.h"
#include "odfaeg/Graphics/shadowRenderComponent.hpp"
#include "odfaeg/Graphics/lightRenderComponent.hpp"
#include "odfaeg/Graphics/perPixelLinkedListRenderComponent.hpp"
#include "odfaeg/Graphics/perPixelLinkedListRenderComponent2.hpp"
#include "odfaeg/Graphics/raytracingRenderComponent.hpp"
#include "odfaeg/Graphics/reflectRefractRenderComponent.hpp"
#include "odfaeg/Graphics/3D/ponctualLight.hpp"
#include "odfaeg/Graphics/particuleSystem.h"
#include "odfaeg/Graphics/animationUpdater.h"
#include "odfaeg/Graphics/3D/animator.hpp"
class MyAppli : public odfaeg::core::Application<MyAppli> {
public :
    enum TEXTURES {
        GRASS, PARTICLE, SKYBOX0, SKYBOX1, SKYBOX2, SKYBOX3, SKYBOX4, SKYBOX5
    };
    MyAppli(odfaeg::math::Vec2f size, std::string title);
    void onLoad();
    void onInit();
    void onRender(odfaeg::graphic::RenderComponentManager* frcm);
    void onDisplay(odfaeg::graphic::RenderWindow* window);
    void onUpdate (odfaeg::graphic::RenderWindow*, odfaeg::window::IEvent& event);
    void onExec();
private :
    odfaeg::graphic::Entity* cube;
    odfaeg::graphic::Scene* theMap;
    odfaeg::graphic::BillBoard* billboard;
    odfaeg::physic::ParticleSystem* ps;
    float speed, sensivity;
    int oldX, oldY;
    bool verticalMotionActive;
    int verticalMotionDirection;
    odfaeg::core::Clock clock, clock2;
    odfaeg::core::Time timeBeforeStoppingVerticalMotion;
    odfaeg::graphic::EntitiesUpdater *eu;
    odfaeg::core::ResourceCache<> cache;
    odfaeg::graphic::Entity* heightmap;
    unsigned int fpsCounter;
    odfaeg::graphic::View view3D;
    odfaeg::graphic::g3d::Model loader;
    odfaeg::graphic::Entity *model, *animatedModel;
    odfaeg::graphic::EntityFactory factory;
    std::unique_ptr<odfaeg::graphic::g3d::Skybox> skybox;
    odfaeg::graphic::AnimUpdater* animUpdater;
    std::unique_ptr<odfaeg::graphic::World> m_world;
};
