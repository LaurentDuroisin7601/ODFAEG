
// *** END ***
#ifndef MY_APPLI
#define MY_APPLI
#include "odfaeg/Graphics/perPixelLinkedListRenderComponent.hpp"
#include "odfaeg/Graphics/shadowRenderComponent.hpp"
#include "odfaeg/Graphics/lightRenderComponent.hpp"
#include "odfaeg/Graphics/fastRenderComponent.hpp"
#include "odfaeg/Graphics/reflectRefractRenderComponent.hpp"
#include "odfaeg/Graphics/application.h"
#include "odfaeg/Graphics/convexShape.h"
#include "odfaeg/Graphics/rectangleShape.h"
#include "odfaeg/Graphics/circleShape.h"
#include "odfaeg/Graphics/tile.h"
#include "odfaeg/Graphics/world.h"
#include "odfaeg/Graphics/map.h"
#include "odfaeg/Graphics/2D/decor.h"
#include "odfaeg/Graphics/anim.h"
#include "odfaeg/Graphics/2D/ambientLight.h"
#include "odfaeg/Graphics/2D/ponctualLight.h"
#include "odfaeg/Graphics/2D/wall.h"
#include "odfaeg/Graphics/tGround.h"
#include "odfaeg/Window/command.h"
#include "odfaeg/Graphics/entitiesUpdater.h"
#include "odfaeg/Graphics/animationUpdater.h"
#include "odfaeg/Graphics/particleSystemUpdater.hpp"
#include "hero.hpp"
#include "odfaeg/Graphics/billBoard.h"
#include "odfaeg/Physics/emmiters.h"
#include "odfaeg/Audio/player.h"
#include "odfaeg/Graphics/GUI/textArea.hpp"
#include "odfaeg/Graphics/GUI/optionPane.hpp"
#include "odfaeg/Window/iKeyboard.hpp"
#include "odfaeg/Window/iMouse.hpp"
#include <fstream>
namespace sorrok {
    class MyAppli : public odfaeg::core::Application, public odfaeg::graphic::gui::FocusListener {
    private :
        const float speed = 0.2f;
        odfaeg::graphic::EntitiesUpdater *eu;
        odfaeg::graphic::AnimUpdater *au;
        odfaeg::graphic::ParticleSystemUpdater* psu;
        bool running;
        odfaeg::graphic::g2d::Wall *w;
        Caracter* caracter;
        odfaeg::window::IKeyboard::Key actualKey, previousKey;
        std::vector<odfaeg::graphic::Tile*> tiles;
        std::vector<odfaeg::graphic::g2d::Wall*> walls;
        odfaeg::graphic::Scene* theMap;
        //odfaeg::graphic::g2d::PonctualLight* light2;
        odfaeg::graphic::Shader shader;
        odfaeg::core::ResourceCache<> cache;
        unsigned int fpsCounter;
        bool day;
        odfaeg::physic::UniversalEmitter emitter;
        odfaeg::physic::ParticleSystem* ps;
        odfaeg::audio::Player player;
        odfaeg::audio::Player pfire;
        odfaeg::graphic::gui::OptionPane* op;
        odfaeg::graphic::VertexArray point;
        odfaeg::graphic::EntityFactory entityFactory;
    public :
        MyAppli(sf::VideoMode wm, std::string title);
        void gaignedFocus(odfaeg::graphic::gui::TextArea* textArea);
        void keyHeldDown (odfaeg::window::IKeyboard::Key key);
        void leftMouseButtonPressed(sf::Vector2f mousePos);
        bool mouseInside (sf::Vector2f mousePos);
        void onMouseInside (sf::Vector2f mousePos);
        void onLoad();
        void onInit ();
        void onRender(odfaeg::graphic::RenderComponentManager *cm);
        void onDisplay(odfaeg::graphic::RenderWindow* window);
        void onUpdate (odfaeg::graphic::RenderWindow* window, odfaeg::window::IEvent& event);
        void onExec ();
    };
}
#endif // MY_APPLI
