
#include <iostream>
#include <string>
#include <vector>
// *** END ***
#ifndef MY_APPLI
#define MY_APPLI
#include "odfaeg/Graphics/application.h"
#include "odfaeg/Graphics/convexShape.h"
#include "odfaeg/Graphics/rectangleShape.h"
#include "odfaeg/Graphics/circleShape.h"
#include "odfaeg/Graphics/tile.h"
#include "odfaeg/Graphics/world.h"
#include "odfaeg/Graphics/map.h"
#include "odfaeg/Graphics/perPixelLinkedListRenderComponent.hpp"
#include "odfaeg/Graphics/2D/decor.h"
#include "odfaeg/Graphics/anim.h"
#include "odfaeg/Graphics/2D/ambientLight.h"
#include "odfaeg/Graphics/2D/ponctualLight.h"
#include "odfaeg/Graphics/2D/wall.h"
#include "odfaeg/Graphics/tGround.h"
#include "odfaeg/Graphics/entitiesUpdater.h"
#include "odfaeg/Graphics/animationUpdater.h"
#include "odfaeg/Graphics/particleSystemUpdater.hpp"
#include "hero.h"
#include "monster.h"
#include "odfaeg/Network/network.h"
#include "odfaeg/Graphics/sprite.h"

#include "odfaeg/Graphics/shadowRenderComponent.hpp"
#include "odfaeg/Graphics/lightRenderComponent.hpp"
#include "odfaeg/Graphics/GUI/label.hpp"
#include "odfaeg/Graphics/GUI/button.hpp"
#include "odfaeg/Graphics/GUI/textArea.hpp"
#include "odfaeg/Graphics/GUI/passwordField.hpp"
#include "odfaeg/Graphics/GUI/progressBar.hpp"
#include "odfaeg/Graphics/GUI/panel.hpp"
#include "odfaeg/Graphics/GUI/table.hpp"
#include "odfaeg/Graphics/GUI/icon.hpp"
#include "odfaeg/Math/distributions.h"
#include "item.hpp"
#include <fstream>
#include <unordered_map>
#include "gameAction.hpp"
#include "itemAction.hpp"
#include "skillAction.hpp"
#include "pnj.hpp"
#include "skill.hpp"
namespace sorrok {
    class MyAppli : public odfaeg::core::Application,
                    public odfaeg::graphic::gui::ActionListener {
    private :
        unsigned int fps;
        const float speed = 0.2f;
        odfaeg::graphic::EntitiesUpdater *eu;
        odfaeg::graphic::AnimUpdater *au;
        odfaeg::graphic::ParticleSystemUpdater *psu;
        bool running;
        odfaeg::graphic::g2d::Wall *w;
        Caracter* hero;
        odfaeg::window::IKeyboard::Key actualKey, previousKey;
        std::vector<odfaeg::graphic::Tile*> tiles;
        std::vector<odfaeg::graphic::Tile*> walls;
        odfaeg::graphic::Scene* theMap;
        odfaeg::graphic::g2d::PonctualLight* light2;
        odfaeg::core::ResourceCache<> cache;
        sf::Time timeBtwnTwoReq = sf::seconds(1.f);
        sf::Int64 ping;
        bool received = false;
        static const unsigned int PATH_ERROR_MARGIN = 5;
        odfaeg::graphic::RenderWindow* wResuHero, *wIdentification, *wPickupItems, *wInventory, *wDisplayQuests, *wDisplayQuest, *wDiary, *wSkills, *wEquipement;
        odfaeg::graphic::gui::Label* label, *labPseudo, *labMdp, *lQuestName, *lQuestTask;
        odfaeg::graphic::gui::TextArea* taPseudo;
        odfaeg::graphic::gui::PasswordField* taPassword;
        odfaeg::graphic::gui::Button* button, *idButton, *invButton, *bAccept, *bDeny, *bGiveUp;
        odfaeg::graphic::gui::ProgressBar* hpBar, *xpBar, *manaBar;
        odfaeg::graphic::gui::Panel* pItems, *pInventory, *pQuestList, *pQuestNames, *pQuestProgress, *pRewards, *pSkills, *pEquipement;
        bool isClientAuthentified;
        std::map<int, std::pair<odfaeg::graphic::Sprite*, std::vector<Item>>> cristals;
        std::pair<odfaeg::graphic::Sprite*, std::vector<Item>> selectedCristal;
        Quest* selectedQuest;
        Pnj* selectedPnj;
        std::array<odfaeg::graphic::gui::Button*, 9> shorcutsButtons;
        odfaeg::graphic::gui::Icon* floatingIcon;
        std::map<odfaeg::physic::ParticleSystem*, std::pair<sf::Time, sf::Time>> particles;
        std::vector<odfaeg::physic::ParticleSystem*> particles2;
        std::map<std::string, sf::Time> doubleClicks;
        odfaeg::physic::UniversalEmitter emitter, emitter2;
        std::vector<std::pair<odfaeg::core::Variant<Item, Skill>, Hero*>> gameActions;
        std::vector<std::pair<std::pair<Caracter*, odfaeg::graphic::Text>, std::pair<sf::Time, sf::Time>>> tmpTexts;
        std::vector<odfaeg::graphic::Entity*> monsters;
        std::array<odfaeg::core::Variant<Item, Skill>*, 9> shorcuts;
        odfaeg::physic::ParticleSystem* ps;
        odfaeg::graphic::EntityFactory factory;
    public :
        enum Fonts {
            Serif
        };
        MyAppli(sf::VideoMode wm, std::string title);
        void keyHeldDown (odfaeg::window::IKeyboard::Key key);
        void pickUpItems (odfaeg::window::IKeyboard::Key key);
        void leftMouseButtonPressed(sf::Vector2f mousePos);
        void rightMouseButtonPressed(sf::Vector2f mousePos);
        void showDiary();
        bool mouseInside (sf::Vector2f mousePos);
        void onMouseInside (sf::Vector2f mousePos);
        void onLoad();
        void onInit ();
        void onRender(odfaeg::graphic::RenderComponentManager *cm);
        void onDisplay(odfaeg::graphic::RenderWindow* window);
        void onUpdate (odfaeg::graphic::RenderWindow* window, odfaeg::window::IEvent& event);
        void onExec ();
        void actionPerformed(odfaeg::graphic::gui::Button* item);
        void dropItems (odfaeg::graphic::gui::Label* label);
        void showInventory();
        void onIconClicked(odfaeg::graphic::gui::Icon* icon);
        void talkToPnj(odfaeg::window::IKeyboard::Key key);
        void onLabQuestClicked(odfaeg::graphic::gui::Label* label);
        void onLabDiaryQuestName(odfaeg::graphic::gui::Label* label);
        void onLastHeal(odfaeg::graphic::gui::Label* label);
        void onShowSkillPressed();
        void launchSkillAnim(std::string name);
        void onIconMoved(odfaeg::graphic::gui::Icon* icon);
        void onIconMouseButtonReleased(odfaeg::graphic::gui::Icon* icon);
        void onIconPressed(odfaeg::graphic::gui::Icon* icon);
        void retractFromInventory(Item& item);
        void onF1Pressed();
        void showEquipement();
        ~MyAppli();
    };
}
#endif // MY_APPLI

