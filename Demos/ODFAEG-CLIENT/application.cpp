#include "application.h"
using namespace odfaeg::core;
using namespace odfaeg::graphic;
using namespace odfaeg::graphic::gui;
using namespace odfaeg::physic;
using namespace odfaeg::math;
using namespace odfaeg::network;
using namespace odfaeg::window;
namespace sorrok {
    MyAppli::MyAppli(sf::VideoMode wm, std::string title) : Application (wm, title, sf::Style::Default, ContextSettings(0, 0, 4, 3, 0)) {
        EXPORT_CLASS_GUID(BoundingVolumeBoundingBox, BoundingVolume, BoundingBox)
        EXPORT_CLASS_GUID_(EntityTile, Entity, Tile, VA_LIST(EntityFactory&), VA_LIST(std::ref(factory)))
        EXPORT_CLASS_GUID_(EntityTile, Entity, BigTile, VA_LIST(EntityFactory&), VA_LIST(std::ref(factory)))
        EXPORT_CLASS_GUID_(EntityWall, Entity, g2d::Wall, VA_LIST(EntityFactory&), VA_LIST(std::ref(factory)))
        EXPORT_CLASS_GUID_(EntityDecor, Entity, g2d::Decor, VA_LIST(EntityFactory&), VA_LIST(std::ref(factory)))
        EXPORT_CLASS_GUID_(EntityAnimation, Entity, Anim, VA_LIST(EntityFactory&), VA_LIST(std::ref(factory)))
        EXPORT_CLASS_GUID_(EntityHero, Entity, Hero, VA_LIST(EntityFactory&), VA_LIST(std::ref(factory)))
        EXPORT_CLASS_GUID_(EntityMonster, Entity, Monster, VA_LIST(EntityFactory&), VA_LIST(std::ref(factory)))
        EXPORT_CLASS_GUID_(EntityPnj, Entity, Pnj, VA_LIST(EntityFactory&), VA_LIST(std::ref(factory)))
        EXPORT_CLASS_GUID_(EntityMesh, Entity, Mesh, VA_LIST(EntityFactory&), VA_LIST(std::ref(factory)))
        EXPORT_CLASS_GUID_(EntityPLIGHT, Entity, g2d::PonctualLight, VA_LIST(EntityFactory&), VA_LIST(std::ref(factory)))
        EXPORT_CLASS_GUID_(EntityPSystem, Entity, ParticleSystem, VA_LIST(EntityFactory&), VA_LIST(std::ref(factory)))
        running = false;
        actualKey = IKeyboard::Key::Unknown;
        previousKey = IKeyboard::Key::Unknown;
        getView().setScale(1, -1, 1);
        sf::Clock clock1;
        addClock(clock1, "RequestTime");
        //std::cout<<"add clock"<<std::endl;
        sf::Clock clock2;
        addClock(clock2, "FPS");
        //std::cout<<"add clock 2"<<std::endl;
        fps = 0;
        hero = nullptr;
        selectedPnj = nullptr;
        selectedQuest = nullptr;
        received = false;
        //std::cout<<"set client message"<<std::endl;
        Network::setCertifiateClientMess("SORROKCLIENT");
        //std::cout<<"client message set"<<std::endl;
        isClientAuthentified = false;
        //std::cout<<"create ps"<<std::endl;
        ps = new ParticleSystem(Vec3f(0, 0, 150),Vec3f(100, 100, 0), factory);
        //std::cout<<"ps created"<<std::endl;
        doubleClicks.insert(std::make_pair("useItem", getClock("TimeClock").getElapsedTime()));
        doubleClicks.insert(std::make_pair("useSkill", getClock("TimeClock").getElapsedTime()));
        //std::cout<<"appli created"<<std::endl;
    }
    void MyAppli::onF1Pressed() {
        if (shorcuts[0] != nullptr) {
            gameActions.push_back(std::make_pair(*shorcuts[0], static_cast<Hero*>(hero)));
        }
    }
    void MyAppli::onIconMoved(Icon* icon) {
        std::cout<<"move icon : "<<IMouse::getPosition(getRenderWindow()).x<<","<<IMouse::getPosition(getRenderWindow()).y<<std::endl;
        floatingIcon->setPosition(Vec3f(IMouse::getPosition(getRenderWindow()).x, IMouse::getPosition(getRenderWindow()).y, 0));

    }
    void MyAppli::onIconPressed(Icon* icon) {
        //std::cout<<"icon pressed"<<std::endl;
        floatingIcon->getSprite().setTexture(*icon->getSprite().getTexture());
        floatingIcon->setVisible(true);
    }
    void MyAppli::onIconMouseButtonReleased(Icon* icon) {
        //std::cout<<"release icon"<<std::endl;
        unsigned int distMin = std::numeric_limits<unsigned int>::max();
        int id = -1;
        for (unsigned int i = 0; i < 9; i++) {
            if (shorcutsButtons[i]->getGlobalBounds().intersects(floatingIcon->getGlobalBounds())) {
                if (floatingIcon->getCenter().computeDist(shorcutsButtons[i]->getCenter()) < distMin) {
                    distMin = floatingIcon->getCenter().computeDist(shorcutsButtons[i]->getCenter());
                    id = i;
                }
            }
        }
        if (id != -1) {
            shorcutsButtons[id]->setIcon(floatingIcon->getSprite().getTexture());
            for (unsigned int i = 0; i < static_cast<Hero*>(hero)->getSkills().size(); i++) {
                if (icon->getName() == static_cast<Hero*>(hero)->getSkills()[i].getName()) {
                    Skill skill = static_cast<Hero*>(hero)->getSkills()[i];
                    SkillAction* skillAction = new SkillAction();
                    skill.setSkillBehaviour(new FastDelegate<void>(&SkillAction::launchLastHeal, skillAction, static_cast<Hero*>(hero), skill));
                    shorcuts[id] = new Variant<Item, Skill>(skill);
                }
            }
        }
        floatingIcon->setVisible(false);
    }
    void MyAppli::onLastHeal (Label* label) {
        std::map<std::string, sf::Time>::iterator it;
        it = doubleClicks.find("useSkill");
        sf::Time elapsedTime = getClock("TimeClock").getElapsedTime() - it->second;
        if (elapsedTime.asSeconds() <= 1.f) {
            Skill skill;
            for (unsigned int i = 0; i < static_cast<Hero*>(hero)->getSkills().size(); i++) {
                if (label->getText() == static_cast<Hero*>(hero)->getSkills()[i].getName()) {
                    skill = static_cast<Hero*>(hero)->getSkills()[i];
                }
            }
            SkillAction* skillAction = new SkillAction();
            skill.setSkillBehaviour(new FastDelegate<void>(&SkillAction::launchLastHeal, skillAction, static_cast<Hero*>(hero), skill));
            gameActions.push_back(std::make_pair(skill, static_cast<Hero*>(hero)));
        } else {
            it->second = getClock("TimeClock").getElapsedTime();
        }
    }
    void MyAppli::launchSkillAnim(std::string name) {
        if (name == "LastHeal") {
            ParticleSystem* ps = new ParticleSystem(Vec3f(hero->getCenter().x, hero->getCenter().y, hero->getPosition().z), Vec3f(300, 300, 0), factory);
            TextureManager<> &tm = cache.resourceManager<Texture, std::string>("TextureManager");
            ps->setTexture(*tm.getResourceByAlias("HEAL_PARTICLE"));
            for (unsigned int i = 0; i < 10; i++) {
                ps->addTextureRect(sf::IntRect(i*10, 148, 10, 10));
            }
            emitter.setEmissionRate(30);
            emitter.setParticleLifetime(Distributions::uniform(sf::seconds(5), sf::seconds(7)));
            emitter.setParticlePosition(Distributions::circle(Vec3f(0, 0, 0), 10));   // Emit particles in given circle
            emitter.setParticleVelocity(Distributions::deflect(Vec3f(0, 0, 0),  360)); // Emit towards direction with deviation of 15°
            emitter.setParticleRotation(Distributions::uniform(0.f, 0.f));
            emitter.setParticleTextureIndex(Distributions::uniformui(0, 9));
            emitter.setParticleScale(Distributions::rect(Vec3f(0.5f, 0.5f, 0.5f), Vec3f(0.5f, 0.5f, 0.5f)));
            ps->addEmitter(refEmitter(emitter));
            psu->addParticleSystem(ps);
            particles.insert(std::make_pair(ps, std::make_pair(Application::getClock("TimeClock").getElapsedTime(), sf::seconds(1))));
            getWorld()->addEntity(ps);
        }
    }
    void MyAppli::onShowSkillPressed () {
        pSkills->removeAll();
        FontManager<Fonts>& fm = cache.resourceManager<Font, Fonts>("FontManager");
        TextureManager<Skill::Type>& tm3 = cache.resourceManager<Texture, Skill::Type>("TextureManager3");
        for (unsigned int i = 0; i < static_cast<Hero*>(hero)->getSkills().size(); i++) {
            Sprite sprite(*tm3.getResourceByAlias(Skill::LAST_HEAL), Vec3f(0, 100*i, 0), Vec3f(50, 50, 0), sf::IntRect(0, 0, 50, 50));
            Icon* icon = new Icon(*wSkills, Vec3f(0, 100*i, 0),Vec3f(50, 50, 0), sprite);
            std::cout<<"skill name : "<<static_cast<Hero*>(hero)->getSkills()[i].getName()<<std::endl;
            icon->setName(static_cast<Hero*>(hero)->getSkills()[i].getName());
            Action aSkillButtonPressed(Action::MOUSE_BUTTON_PRESSED_ONCE, IMouse::Left);
            Action aSkillButtonHeldDown(Action::MOUSE_BUTTON_HELD_DOWN, IMouse::Left);
            aSkillButtonHeldDown.name="SkillButtonHeldDown";
            Action aSkillMouseMoved(Action::MOUSE_MOVED_);
            aSkillMouseMoved.name="SkillMouseMoved";
            Action aSkillCombined = aSkillButtonHeldDown && aSkillMouseMoved;
            aSkillCombined.name ="SkillCombined";
            Command cmd (aSkillCombined, FastDelegate<void>(&MyAppli::onIconMoved, this, icon));
            icon->getListener().connect("IconMoved", cmd);
            Action aSkillMouseButtonReleased(Action::MOUSE_BUTTON_RELEASED, IMouse::Left);
            Command cmd2 (aSkillMouseButtonReleased, FastDelegate<void>(&MyAppli::onIconMouseButtonReleased, this, icon));
            icon->getListener().connect("IconReleased", cmd2);
            Command cmd3 (aSkillButtonPressed, FastDelegate<bool> (&Icon::isMouseInside, icon), FastDelegate<void>(&MyAppli::onIconPressed, this, icon));
            icon->getListener().connect("IconPressed", cmd3);
            pSkills->addChild(icon);
            icon->setParent(pSkills);
            Label* lSkill = new Label(*wSkills, Vec3f(50, 100 * i, 0), Vec3f(400, 100, 0),fm.getResourceByAlias(Fonts::Serif),static_cast<Hero*>(hero)->getSkills()[i].getName(), 15);
            Action aSkillSelected (Action::MOUSE_BUTTON_PRESSED_ONCE, IMouse::Left);
            Command cmdSkillSelected(aSkillSelected, FastDelegate<bool>(&Label::isMouseInside, lSkill), FastDelegate<void>(&MyAppli::onLastHeal, this, lSkill));
            lSkill->getListener().connect("SkillSelected", cmdSkillSelected);
            pSkills->addChild(lSkill);
            lSkill->setParent(pSkills);

        }
        wSkills->setVisible(true);
        getRenderComponentManager().setEventContextActivated(true, *wSkills);
        setEventContextActivated(false);
        getRenderComponentManager().setEventContextActivated(false, getRenderWindow());
    }
    void MyAppli::onLabDiaryQuestName(Label* label)  {
        Quest* quest;
        for (unsigned int i = 0; i < static_cast<Hero*>(hero)->getDiary().size(); i++) {
            if (label->getText() == static_cast<Hero*>(hero)->getDiary()[i].getName()) {
                quest = &static_cast<Hero*>(hero)->getDiary()[i];
            }
        }
        FontManager<Fonts>& fm = cache.resourceManager<Font, Fonts>("FontManager");
        std::map<std::string, std::pair<unsigned int, unsigned int>>::iterator it;
        std::map<std::string, std::pair<unsigned int, unsigned int>> monstersToKill = quest->getMonsterToKill();
        unsigned int i = 0;
        for (it = monstersToKill.begin(); it != monstersToKill.end(); it++) {
            Label* lab = new Label(*wDiary, Vec3f(300, i*100, 0), Vec3f(300, 100, 0), fm.getResourceByAlias(Fonts::Serif),conversionIntString(it->second.second)+"/"+conversionIntString(it->second.first)+" "+it->first, 15);
            pQuestProgress->addChild(lab);
            lab->setParent(pQuestProgress);
            i++;
        }
        std::map<std::string, std::pair<unsigned int, unsigned int>> itemsToCollect = quest->getItemsToCollect();
        for (it = itemsToCollect.begin(); it != itemsToCollect.end(); it++) {
            Label* lab = new Label(*wDiary, Vec3f(300, i*100, 0), Vec3f(300, 100, 0), fm.getResourceByAlias(Fonts::Serif),conversionIntString(it->second.first)+"/"+conversionIntString(it->second.second)+" "+it->first, 15);
            pQuestProgress->addChild(lab);
            lab->setParent(pQuestProgress);
            i++;
        }
    }
    void MyAppli::showDiary() {
        FontManager<Fonts>& fm = cache.resourceManager<Font, Fonts>("FontManager");
        pQuestNames->removeAll();
        pQuestProgress->removeAll();
        for(unsigned int i = 0; i < static_cast<Hero*>(hero)->getDiary().size(); i++) {
            Label* label = new Label(*wDiary, Vec3f(0, 100*i, 0), Vec3f(300, 100, 0),fm.getResourceByAlias(Fonts::Serif),static_cast<Hero*>(hero)->getDiary()[i].getName(), 15);
            pQuestNames->addChild(label);
            label->setParent(pQuestNames);
            Action aLabDiaryQuestName (Action::MOUSE_BUTTON_PRESSED_ONCE, IMouse::Left);
            Command cmdLabDiaryQuestName (aLabDiaryQuestName, FastDelegate<bool>(&Label::isMouseInside, label), FastDelegate<void>(&MyAppli::onLabDiaryQuestName, this, label));
            label->getListener().connect("LabDiaryQuestName", cmdLabDiaryQuestName);
        }
        wDiary->setVisible(true);
        getRenderComponentManager().setEventContextActivated(true, *wDiary);
        setEventContextActivated(false);
        getRenderComponentManager().setEventContextActivated(false, getRenderWindow());
    }
    void MyAppli::onLabQuestClicked(Label* label) {
        label->setEventContextActivated(false);
        wDisplayQuests->setVisible(false);
        if (static_cast<Hero*>(hero)->containsQuest(label->getText().toAnsiString())) {
            selectedQuest = static_cast<Hero*>(hero)->getQuest(label->getText());
        } else {
            for (unsigned int i = 0; i < selectedPnj->getQuests().size(); i++) {
                if (selectedPnj->getQuests()[i].getName() == label->getText()) {
                    selectedQuest = &selectedPnj->getQuests()[i];
                }
            }
        }
        lQuestName->setText(selectedQuest->getName());
        lQuestTask->setText(selectedQuest->getTask());
        pRewards->removeAll();
        FontManager<Fonts> &fm = cache.resourceManager<Font, Fonts>("FontManager");
        Label* lxp = new Label(*wDisplayQuest, pRewards->getPosition(), Vec3f(50, 100, 0),fm.getResourceByAlias(Fonts::Serif),"xp : "+conversionIntString(selectedQuest->getXp()), 10);
        pRewards->addChild(lxp);
        lxp->setParent(pRewards);
        std::map<unsigned int, Item> rewards = selectedQuest->getRewards();
        std::map<unsigned int, Item>::iterator it;
        TextureManager<Item::Type> &tm2 = cache.resourceManager<Texture, Item::Type>("TextureManager2");
        unsigned int i = 0;
        for (it = rewards.begin(); it != rewards.end(); it++) {
            Sprite sprite (*tm2.getResourceByAlias(it->second.getType()),Vec3f(pRewards->getPosition().x + 50 * (i+1), pRewards->getPosition().y, 0),Vec3f(50, 50, 0), sf::IntRect(0, 0, 50, 50));
            Icon* icon = new Icon(*wDisplayQuest,Vec3f(pRewards->getPosition().x + 50 * (i+1), pRewards->getPosition().y, 0),Vec3f(50, 50, 0),sprite);
            pRewards->addChild(icon);
            icon->setParent(pRewards);
            Label* lNbItems = new Label(*wDisplayQuest, Vec3f(pRewards->getPosition().x + 50 * (i+1), pRewards->getPosition().y, 0), Vec3f(50, 50, 0),fm.getResourceByAlias(Fonts::Serif),conversionIntString(it->first), 10);
            lNbItems->setBackgroundColor(sf::Color::Transparent);
            pRewards->addChild(lNbItems);
            lNbItems->setParent(pRewards);
            i++;
        }
        if (selectedQuest->getStatus() == Quest::NEW) {
            std::cout<<"new"<<std::endl;
            bAccept->setText("Accept");
        } else if (selectedQuest->getStatus() == Quest::IN_PROGRESS)  {
            std::cout<<"in progress"<<std::endl;
            bAccept->setText("Continue");
        } else if (selectedQuest->getStatus() == Quest::COMPLETE) {
            std::cout<<"finished"<<std::endl;
            bAccept->setText("Get rewards");
        }
        bAccept->setEventContextActivated(true);
        bDeny->setEventContextActivated(true);
        setEventContextActivated(false);
        wDisplayQuest->setVisible(true);
    }
    void MyAppli::retractFromInventory(Item& item) {
        std::map<Item::Type, std::vector<Item>>& items = static_cast<Hero*>(hero)->getInventory();
        std::map<Item::Type, std::vector<Item>>::iterator it = items.find(item.getType());
        if (item.getType() == Item::HP_POTION) {
            it->second.pop_back();
            Label* label;
            bool found = false;
            for (unsigned int i = 0; i < pInventory->getChildren().size() && !found; i+=2) {
                Icon* icn = dynamic_cast<Icon*> (pInventory->getChildren()[i]);
                label = dynamic_cast<Label*> (pInventory->getChildren()[i+1]);
                if (item.getIcon() == icn) {
                    label->setText(conversionIntString(it->second.size()));
                    found = true;
                }
            }
            if (it->second.empty()) {
                for (unsigned int i = 0; i < pInventory->getChildren().size(); i++) {
                    if (pInventory->getChildren()[i] == item.getIcon()) {
                        pInventory->removeChild(*item.getIcon());
                        pInventory->removeChild(*label);
                    }
                }
                items.erase(it);
            }
        }
    }
    void MyAppli::onIconClicked(Icon* icon) {
        std::map<std::string, sf::Time>::iterator it;
        it = doubleClicks.find("useItem");
        sf::Time elapsedTime = getClock("TimeClock").getElapsedTime() - it->second;
        if (elapsedTime.asSeconds() <= 1.f) {
            std::map<Item::Type, std::vector<Item>> items = static_cast<Hero*>(hero)->getInventory();
            std::map<Item::Type, std::vector<Item>>::iterator it;
            Item item;
            for (it = items.begin(); it != items.end(); it++) {
                for (unsigned int i = 0; i < it->second.size(); i++) {
                    if (it->second[i].getName() == icon->getName()) {
                        item = it->second[i];
                    }
                }
            }
            if (item.getType() == Item::HP_POTION) {
                ItemAction* ia = new ItemAction();
                item.setItemBehaviour(new FastDelegate<void>(&ItemAction::useHpPotion,ia,static_cast<Hero*>(hero),item));
            }
            gameActions.push_back(std::make_pair(item, static_cast<Hero*>(hero)));
        } else {
            it->second = getClock("TimeClock").getElapsedTime();
        }
    }
    void MyAppli::showInventory() {
        wInventory->setVisible(true);
        getRenderComponentManager().setEventContextActivated(true, *wInventory);
        setEventContextActivated(false);
        getRenderComponentManager().setEventContextActivated(false, getRenderWindow());
        std::map<Item::Type, std::vector<Item>>& items = static_cast<Hero*>(hero)->getInventory();
        TextureManager<Item::Type> &tm2 = cache.resourceManager<Texture, Item::Type>("TextureManager2");
        FontManager<Fonts> &fm = cache.resourceManager<Font, Fonts>("FontManager");
        const Font* font = fm.getResourceByAlias(Fonts::Serif);
        unsigned int nbRows = 1 + items.size() / 10;
        unsigned int rowIndex = 0, colIndex = 0;
        Table table(4, 10);
        std::map<Item::Type, std::vector<Item>>::iterator it;
        for(it = items.begin(); it != items.end(); it++) {
            Sprite sprite (*tm2.getResourceByAlias(it->first),pItems->getPosition(),Vec3f(50, 50, 0), sf::IntRect(0, 0, 50, 50));
            Icon* icon = new Icon(*wInventory,Vec3f(0, 0, 0),Vec3f(0, 0, 0),sprite);
            for (unsigned int i = 0; i < it->second.size(); i++) {
                it->second[i].setIcon(icon);
                icon->setName(it->second[i].getName());
            }
            Action a (Action::MOUSE_BUTTON_PRESSED_ONCE, IMouse::Left);
            Command cmd(a, FastDelegate<bool>(&Icon::isMouseInside, icon), FastDelegate<void>(&MyAppli::onIconClicked, this, icon));
            icon->getListener().connect("UseItem"+conversionIntString(it->first), cmd);
            Label* label = new Label(*wInventory,Vec3f(0, 0, 0),Vec3f(0, 0 ,0),font,conversionUIntString(it->second.size()),10);
            label->setBackgroundColor(sf::Color::Transparent);
            table.addElement(icon, rowIndex, colIndex);
            table.addElement(label, rowIndex, colIndex);
            colIndex++;
            if (colIndex == 10) {
                rowIndex++;
                colIndex = 0;
            }
            pInventory->addChild(icon);
            pInventory->addChild(label);
            icon->setParent(pInventory);
            label->setParent(pInventory);
        }
    }
    void MyAppli::showEquipement() {
        std::vector<Item> stuff = static_cast<Hero*>(hero)->getEquipment();
        pEquipement->clearDrawables();
        TextureManager<std::string>& tm = cache.resourceManager<Texture, std::string>("TextureManager");
        Sprite sequipement (*tm.getResourceByAlias("EQUIPEMENT"), Vec3f(0, 0, 0), Vec3f(400, 600, 0), sf::IntRect(0, 0, 300, 500));
        pEquipement->addSprite(sequipement);
        for (unsigned int i = 0; i < stuff.size(); i++) {
            Sprite sprite;
            TextureManager<Item::Type>& tm2 = cache.resourceManager<Texture, Item::Type>("TextureManager2");
            if (stuff[i].getName() == "Novice Head") {
                sprite = Sprite (*tm2.getResourceByAlias(Item::HEAD), Vec3f(0, 0, 0), Vec3f(50, 50, 0),sf::IntRect(0, 0, 50, 50));
            } if (stuff[i].getName() == "Novice Weapon Right")
                sprite = Sprite (*tm2.getResourceByAlias(Item::WEAPON_RIGHT), Vec3f(0, 200, 0), Vec3f(50, 50, 0),sf::IntRect(0, 0, 50, 50));
            if (stuff[i].getName() == "Novice Boots")
                sprite = Sprite (*tm2.getResourceByAlias(Item::BOOTS), Vec3f(0, 400, 0), Vec3f(50, 50, 0),sf::IntRect(0, 0, 50, 50));
            pEquipement->addSprite(sprite);
        }
        wEquipement->setVisible(true);
    }
    void MyAppli::dropItems(Label* label) {
        std::vector<Item>::iterator it;
        for (it = selectedCristal.second.begin(); it != selectedCristal.second.end(); ) {
            if (it->getName() == label->getText()) {
                static_cast<Hero*>(hero)->addItem(*it);
                it = selectedCristal.second.erase(it);
                SymEncPacket packet;
                std::ostringstream oss;
                OTextArchive oa(oss);
                oa(*it);
                std::string response= "DROPITEM*"+conversionIntString(hero->getId())+"*"+oss.str();
                packet<<response;
                Network::sendTcpPacket(packet);
            } else {
                it++;
            }
        }
        std::vector<Item> itemsToDisplay = selectedCristal.second;
        pItems->removeAll();
        if (itemsToDisplay.size() > 0) {
            FontManager<Fonts> &fm = cache.resourceManager<Font,Fonts>("FontManager");
            Table table (4,2);
            TextureManager<Item::Type> &tm2 = cache.resourceManager<Texture, Item::Type>("TextureManager2");
            for (unsigned int i = 0; i < itemsToDisplay.size(); i++) {
                Label* itemName = new Label(*wPickupItems, Vec3f(0, 0, 0), Vec3f(0, 0, 0), fm.getResourceByAlias(Fonts::Serif), itemsToDisplay[i].getName(),15);
                Action a (Action::EVENT_TYPE::MOUSE_BUTTON_PRESSED_ONCE, IMouse::Left);
                Command cmd (a, FastDelegate<bool>(&Label::isMouseInside, itemName), FastDelegate<void>(&MyAppli::dropItems, this, itemName));
                itemName->getListener().connect("PICKUPITEM", cmd);
                itemName->getListener().setRemoveListener(true);
                itemName->setParent(pItems);
                table.addElement(itemName,i, 1);
                Sprite sprite (*tm2.getResourceByAlias(Item::HP_POTION),pItems->getPosition(),Vec3f(50, 50, 0), sf::IntRect(0, 0, 50, 50));
                Icon* icon = new Icon(*wPickupItems,Vec3f(0, 0, 0),Vec3f(0, 0, 0),sprite);
                icon->setParent(pItems);
                table.addElement(icon,i,0);
                pItems->addChild(itemName);
                pItems->addChild(icon);
            }
        } else {
            std::map<int, std::pair<Sprite*, std::vector<Item>>>::iterator it;
            for (it = cristals.begin(); it != cristals.end();) {
                if (it->second.first == selectedCristal.first) {
                    delete it->second.first;
                    //std::cout<<"erase it"<<std::endl;
                    it = cristals.erase(it);
                    //std::cout<<"it erased"<<std::endl;
                } else {
                    it++;
                }
            }
        }
        //std::cout<<"items droped"<<std::endl;
    }
    void MyAppli::pickUpItems (IKeyboard::Key key) {
        if (key != IKeyboard::Key::Unknown && key == IKeyboard::Key::A) {
            std::map<int, std::pair<Sprite*, std::vector<Item>>>::iterator it, itf;
            std::vector<Item> itemsToDisplay;
            if (cristals.size() > 0) {
                itf = cristals.begin();
                float minDist = itf->second.first->getPosition().computeDist(hero->getPosition());
                if (minDist < 100) {
                    itemsToDisplay = itf->second.second;
                    selectedCristal = itf->second;
                }
                itf++;
                for (it = itf; it != cristals.end(); it++) {
                    float dist = it->second.first->getPosition().computeDist(hero->getPosition());
                    if (dist < 100
                        && dist < minDist) {
                        itemsToDisplay = it->second.second;
                        minDist = dist;
                        selectedCristal = it->second;
                    }
                }
                wPickupItems->setPosition(sf::Vector2i(selectedCristal.first->getPosition().x, selectedCristal.first->getPosition().y));
            }
            if (itemsToDisplay.size() > 0) {
                FontManager<Fonts> &fm = cache.resourceManager<Font,Fonts>("FontManager");
                TextureManager<Item::Type> &tm2 = cache.resourceManager<Texture, Item::Type>("TextureManager2");
                Table table (4,2);
                for (unsigned int i = 0; i < itemsToDisplay.size(); i++) {
                    Label* itemName = new Label(*wPickupItems, Vec3f(0, 0, 0), Vec3f(0, 0, 0), fm.getResourceByAlias(Fonts::Serif), itemsToDisplay[i].getName(),15);
                    Action a (Action::EVENT_TYPE::MOUSE_BUTTON_PRESSED_ONCE, IMouse::Left);
                    Command cmd (a, FastDelegate<bool>(&Label::isMouseInside, itemName), FastDelegate<void>(&MyAppli::dropItems, this, itemName));
                    itemName->getListener().connect("PICKUPITEM", cmd);
                    itemName->setParent(pItems);
                    table.addElement(itemName,i, 1);
                    Sprite sprite (*tm2.getResourceByAlias(Item::HP_POTION),pItems->getPosition(),Vec3f(50, 50, 0), sf::IntRect(0, 0, 50, 50));
                    Icon* icon = new Icon(*wPickupItems,Vec3f(0, 0, 0),Vec3f(0, 0, 0),sprite);
                    icon->setParent(pItems);
                    table.addElement(icon,i,0);
                    pItems->addChild(itemName);
                    pItems->addChild(icon);
                }
                wPickupItems->setVisible(true);
                getRenderComponentManager().setEventContextActivated(true, *wPickupItems);
                setEventContextActivated(false);
                getRenderComponentManager().setEventContextActivated(false, getRenderWindow());
            }
        }
    }
    void MyAppli::talkToPnj(IKeyboard::Key key) {
        std::cout<<"talk to pnj"<<std::endl;
        int x = getView().getPosition().x;
        int y = getView().getPosition().y;
        int z = getView().getPosition().z;
        int w = getView().getSize().x;
        int h = getView().getSize().y;
        int d = getView().getSize().z;
        std::string message = "TALKTOPNJ*"+conversionIntString(x)+"*"+conversionIntString(y)+"*"+conversionIntString(z)+"*"+conversionIntString(w)+"*"+conversionIntString(h)+"*"+conversionIntString(d);
        SymEncPacket packet;
        packet<<message;
        Network::sendTcpPacket(packet);
    }
    void MyAppli::keyHeldDown (IKeyboard::Key key) {
        //BoundingRectangle rect (pos.x, pos.y, getView().getSize().x, getView().getSize().y);
        if (actualKey != IKeyboard::Key::Unknown && key == IKeyboard::Key::Up) {
            if (!hero->isMoving()) {
                //if (actualKey != previousKey) {
                    Vec2f dir(0, -1);
                    hero->setDir(dir);
                    sf::Int64 cli_time = Application::getTimeClk().getElapsedTime().asMicroseconds();
                    hero->setIsMovingFromKeyboard(true);
                    std::string message = "MOVEFROMKEYBOARD*"+conversionIntString(hero->getId())+"*"+conversionFloatString(dir.x)+"*"+conversionFloatString(dir.y)+"*"+conversionLongString(cli_time);
                    SymEncPacket packet;
                    packet<<message;
                    Network::sendTcpPacket(packet);
                //}

            }
        } else if (actualKey != IKeyboard::Key::Unknown && key == IKeyboard::Key::Left) {
            if (!hero->isMoving()) {
                //if (actualKey != previousKey) {
                    Vec2f dir(-1, 0);
                    hero->setDir(dir);
                    sf::Int64 cli_time = Application::getTimeClk().getElapsedTime().asMicroseconds();
                    hero->setIsMovingFromKeyboard(true);
                    std::string message = "MOVEFROMKEYBOARD*"+conversionIntString(hero->getId())+"*"+conversionFloatString(dir.x)+"*"+conversionFloatString(dir.y)+"*"+conversionLongString(cli_time);
                    SymEncPacket packet;
                    packet<<message;
                    Network::sendTcpPacket(packet);
                //}
            }
        } else if (actualKey != IKeyboard::Key::Unknown && actualKey == IKeyboard::Key::Down) {
            if (!hero->isMoving()) {
                //if (actualKey != previousKey) {
                    Vec2f dir(0, 1);
                    hero->setDir(dir);
                    sf::Int64 cli_time = Application::getTimeClk().getElapsedTime().asMicroseconds();
                    hero->setIsMovingFromKeyboard(true);
                    std::string message = "MOVEFROMKEYBOARD*"+conversionIntString(hero->getId())+"*"+conversionFloatString(dir.x)+"*"+conversionFloatString(dir.y)+"*"+conversionLongString(cli_time);
                    SymEncPacket packet;
                    packet<<message;
                    Network::sendTcpPacket(packet);
                //}
            }
        } else if (actualKey != IKeyboard::Key::Unknown && key == IKeyboard::Key::Right) {
            if (!hero->isMoving()) {
                //if (actualKey != previousKey) {
                    Vec2f dir(1, 0);
                    hero->setDir(dir);
                    sf::Int64 cli_time = Application::getTimeClk().getElapsedTime().asMicroseconds();
                    hero->setIsMovingFromKeyboard(true);
                    std::string message = "MOVEFROMKEYBOARD*"+conversionIntString(hero->getId())+"*"+conversionFloatString(dir.x)+"*"+conversionFloatString(dir.y)+"*"+conversionLongString(cli_time);
                    SymEncPacket packet;
                    packet<<message;
                    Network::sendTcpPacket(packet);
                //}
            }
        }
    }
    void MyAppli::leftMouseButtonPressed(sf::Vector2f mousePos) {
        Vec3f finalPos(mousePos.x, getRenderWindow().getSize().y - mousePos.y, 0);
        finalPos = getRenderWindow().mapPixelToCoords(finalPos);
        finalPos = Vec3f(finalPos.x, finalPos.y, 0);
        std::string message = "MOVEFROMPATH*"+conversionIntString(hero->getId())+"*"+conversionFloatString(finalPos.x)+"*"+conversionFloatString(finalPos.y);
        SymEncPacket packet;
        packet<<message;
        Network::sendTcpPacket(packet);
    }
    void MyAppli::rightMouseButtonPressed(sf::Vector2f mousePos) {
        Vec3f finalPos (mousePos.x, getRenderWindow().getSize().y - mousePos.y, 0);
        finalPos = getRenderWindow().mapPixelToCoords(finalPos);
        finalPos = Vec3f(finalPos.x, finalPos.y, 0);
        int id = hero->getId();
        std::string message = "ATTACK*"+conversionIntString(id)+"*"+conversionFloatString(finalPos.x)+"*"+conversionFloatString(finalPos.y)+"*"+conversionLongString(Application::getTimeClk().getElapsedTime().asMicroseconds());
        SymEncPacket packet;
        packet<<message;
        Network::sendTcpPacket(packet);
    }
    bool MyAppli::mouseInside (sf::Vector2f mousePos) {
        BoundingBox bx (0, 0, 0, 100, 100, 0);
        if (bx.isPointInside(Vec3f(mousePos.x, mousePos.y, 0))) {
            return true;
        }
        return false;
    }
    void MyAppli::onLoad() {
        //std::cout<<"load"<<std::endl;
        TextureManager<> tm;
        tm.fromFileWithAlias("tilesets/herbe.png", "GRASS");
        tm.fromFileWithAlias("tilesets/murs.png", "WALLS");
        tm.fromFileWithAlias("tilesets/maison.png", "HOUSE");
        tm.fromFileWithAlias("tilesets/flemmes1.png", "FIRE1");
        tm.fromFileWithAlias("tilesets/flemmes2.png", "FIRE2");
        tm.fromFileWithAlias("tilesets/flemmes3.png", "FIRE3");
        tm.fromFileWithAlias("tilesets/cristal.png", "CRISTAL");
        tm.fromFileWithAlias("tilesets/particles.jpeg", "HEAL_PARTICLE");
        tm.fromFileWithAlias("tilesets/particule.png", "PARTICLE");
        tm.fromFileWithAlias("tilesets/equipement.png", "EQUIPEMENT");
        TextureManager<Item::Type> tm2;
        tm2.fromFileWithAlias("tilesets/hppotion-icon.png", Item::HP_POTION);
        tm2.fromFileWithAlias("tilesets/head.png", Item::HEAD);
        tm2.fromFileWithAlias("tilesets/armor.png", Item::ARMOR);
        tm2.fromFileWithAlias("tilesets/gloves.png", Item::GLOVES);
        tm2.fromFileWithAlias("tilesets/boots.png", Item::BOOTS);
        tm2.fromFileWithAlias("tilesets/weaponright.png", Item::WEAPON_RIGHT);
        tm2.fromFileWithAlias("tilesets/weaponleft.png", Item::WEAPON_LEFT);

        TextureManager<Skill::Type> tm3;
        tm3.fromFileWithAlias("tilesets/lastheal-icon.png", Skill::LAST_HEAL);
        std::string path = "tilesets/vlad_sword.png";
        tm.fromFileWithAlias(path, "VLADSWORD");
        std::string path2 = "tilesets/luigi.png";
        tm.fromFileWithAlias(path2, "LUIGI");
        std::string path3 = "tilesets/ogro.png";
        tm.fromFileWithAlias(path3, "OGRO");
        FontManager<Fonts> fm;
        fm.fromFileWithAlias("fonts/Arial.ttf", Serif);
        cache.addResourceManager(fm, "FontManager");
        cache.addResourceManager(tm, "TextureManager");
        cache.addResourceManager(tm2, "TextureManager2");
        cache.addResourceManager(tm3, "TextureManager3");
        //std::cout<<"resources loaded"<<std::endl;
        //shader.loadFromFile("Shaders/SimpleVertexShader.vertexshader", "Shaders/SimpleFragmentShader.fragmentshader");
    }
    void MyAppli::onInit () {

        setEventContextActivated(false);
        FontManager<Fonts>& fm = cache.resourceManager<Font, Fonts>("FontManager");
        //std::cout<<"init"<<std::endl;
        Network::startCli(10'000, 10'001,sf::IpAddress::LocalHost);
        //std::cout<<"client connected"<<std::endl;
        TextureManager<> &tm = cache.resourceManager<Texture, std::string>("TextureManager");
        Vec2f pos (getView().getPosition().x - getView().getSize().x * 0.5f, getView().getPosition().y - getView().getSize().y * 0.5f);
        BoundingBox bx (pos.x, pos.y, 0, getView().getSize().x, getView().getSize().y, 0);
        theMap = new Scene(&getRenderComponentManager(), "Map test", 100, 50, 0);
        BaseChangementMatrix bcm;
        bcm.set2DIsoMatrix();
        theMap->setBaseChangementMatrix(bcm);
        getWorld()->addSceneManager(theMap);
        getWorld()->setCurrentSceneManager("Map test");
        eu = new EntitiesUpdater(factory, *getWorld());
        eu->setName("Entity system updater");
        getWorld()->addWorker(eu);
        au = new AnimUpdater();
        au->setInterval(sf::seconds(0.01f));
        getWorld()->addTimer(au);
        psu = new ParticleSystemUpdater();
        psu->setName("Particle system updater");
        getWorld()->addWorker(psu);
        tiles.push_back(new Tile(tm.getResourceByAlias("GRASS"), Vec3f(0, 0, 0), Vec3f(120, 60, 0),sf::IntRect(0, 0, 100, 50), factory));
        walls.push_back(new Tile(tm.getResourceByAlias("WALLS"), Vec3f(0, 0, 0), Vec3f(100, 100, 0), sf::IntRect(100, 0, 100, 100), factory));
        walls.push_back(new Tile(tm.getResourceByAlias("WALLS"), Vec3f(0, 0, 0), Vec3f(100, 100, 0), sf::IntRect(100, 100, 100, 100), factory));
        walls.push_back(new Tile(tm.getResourceByAlias("WALLS"), Vec3f(0, 0, 0), Vec3f(100, 100, 0), sf::IntRect(100, 200, 100, 100), factory));
        walls.push_back(new Tile(tm.getResourceByAlias("WALLS"), Vec3f(0, 0, 0), Vec3f(100, 100, 0), sf::IntRect(100, 300, 100, 100), factory));
        walls.push_back(new Tile(tm.getResourceByAlias("WALLS"), Vec3f(0, 0, 0), Vec3f(100, 100, 0), sf::IntRect(100, 400, 100, 100), factory));
        walls.push_back(new Tile(tm.getResourceByAlias("WALLS"), Vec3f(0, 0, 0), Vec3f(100, 100, 0), sf::IntRect(100, 500, 100, 100), factory));
        tiles[0]->getFaces()[0].getMaterial().setTexId("GRASS");
        walls[0]->getFaces()[0].getMaterial().setTexId("WALLS");
        walls[1]->getFaces()[0].getMaterial().setTexId("WALLS");
        walls[2]->getFaces()[0].getMaterial().setTexId("WALLS");
        walls[3]->getFaces()[0].getMaterial().setTexId("WALLS");
        walls[4]->getFaces()[0].getMaterial().setTexId("WALLS");
        walls[5]->getFaces()[0].getMaterial().setTexId("WALLS");

        //}

        //caracter->setCenter(Vec3f(getView().getPosition().x, getView().getPosition().y, 300));

        PerPixelLinkedListRenderComponent *frc1 = new PerPixelLinkedListRenderComponent(getRenderWindow(),0, "",ContextSettings(0, 0, 0, 4, 6));
        PerPixelLinkedListRenderComponent *frc2 = new PerPixelLinkedListRenderComponent(getRenderWindow(), 1, "", ContextSettings(0, 0, 4, 4, 6));
        ShadowRenderComponent *frc3 = new ShadowRenderComponent(getRenderWindow(),2, "",ContextSettings(0, 0, 0, 4, 6));
        LightRenderComponent* frc4 = new LightRenderComponent(getRenderWindow(),3,"",ContextSettings(0, 0, 0, 4, 6));
        /*View view = getView();
        frc1->setView(view);*/
        /*frc2->setView(view);
        frc3->setView(view);
        frc4->setView(view);*/
        /*frc1->setRelPosition(0.f, 0.f);
        frc1->setRelSize(1.f, 1.f);
        /*frc2->setRelPosition(0.f, 0.f);
        frc2->setRelSize(1.f, 1.f);
        frc3->setRelPosition(0.f, 0.f);
        frc3->setRelSize(1.f, 1.f);
        frc4->setRelPosition(0.f, 0.f);
        frc4->setRelSize(1.f, 1.f);*/
        getRenderComponentManager().addComponent(frc1);
        getRenderComponentManager().addComponent(frc2);
        getRenderComponentManager().addComponent(frc3);
        getRenderComponentManager().addComponent(frc4);
        //getView().move(d.x * 0.5f, d.y * 0.5f, 0);
        //getWorld()->computeIntersectionsWithWalls();
        //getWorld()->update();
        //getWorld()->computeIntersectionsWithWalls();
        Action a1 (Action::EVENT_TYPE::KEY_HELD_DOWN, IKeyboard::Key::Up);
        Action a2 (Action::EVENT_TYPE::KEY_HELD_DOWN, IKeyboard::Key::Left);
        Action a3 (Action::EVENT_TYPE::KEY_HELD_DOWN, IKeyboard::Key::Down);
        Action a4 (Action::EVENT_TYPE::KEY_HELD_DOWN, IKeyboard::Key::Right);
        Action a5 (Action::EVENT_TYPE::MOUSE_BUTTON_PRESSED_ONCE, IMouse::Left);
        Action a6 (Action::EVENT_TYPE::MOUSE_BUTTON_PRESSED_ONCE, IMouse::Right);
        Action combined  = a1 || a2 || a3 || a4;
        Command moveAction(combined, FastDelegate<void>(&MyAppli::keyHeldDown, this, IKeyboard::Key::Unknown));
        getListener().connect("MoveAction", moveAction);
        g2d::AmbientLight::getAmbientLight().setColor(sf::Color(255, 255, 255));
        Command leftMouseButtonPressedCommand (a5, FastDelegate<void>(&MyAppli::leftMouseButtonPressed, this, sf::Vector2f(-1, -1)));
        Command rightMouseButtonPressedCommand (a6, FastDelegate<void>(&MyAppli::rightMouseButtonPressed, this, sf::Vector2f(-1, -1)));
        getListener().connect("LeftMouseButtonPressedAction", leftMouseButtonPressedCommand);
        getListener().connect("RightMouseButtonPressedAction", rightMouseButtonPressedCommand);
        Action aPickUpItems (Action::EVENT_TYPE::KEY_PRESSED_ONCE, IKeyboard::Key::A);
        Command cmdPickUpItems(aPickUpItems,FastDelegate<void>(&MyAppli::pickUpItems, this, IKeyboard::Key::Unknown));
        getListener().connect("PickUpItems", cmdPickUpItems);
        Action aShowInventory(Action::KEY_PRESSED_ONCE, IKeyboard::Key::I);
        Command cmdShowInventory(aShowInventory, FastDelegate<void>(&MyAppli::showInventory, this));
        getListener().connect("ShowInventory", cmdShowInventory);
        Action aTalk (Action::EVENT_TYPE::KEY_PRESSED_ONCE, IKeyboard::Key::T);
        Command cmdTalk (aTalk, FastDelegate<void>(&MyAppli::talkToPnj, this, IKeyboard::Key::Unknown));
        getListener().connect("TalkToPnj", cmdTalk);
        Action aShowDiary(Action::KEY_PRESSED_ONCE, IKeyboard::Key::D);
        Command cmd(aShowDiary, FastDelegate<void>(&MyAppli::showDiary, this));
        getListener().connect("ShowDiary", cmd);
        Action aShowSkill (Action::KEY_PRESSED_ONCE, IKeyboard::Key::S);
        Command cmdShowSkill(aShowSkill, FastDelegate<void>(&MyAppli::onShowSkillPressed, this));
        getListener().connect("ShowSkill", cmdShowSkill);
        Action aShorcutF1(Action::KEY_PRESSED_ONCE, IKeyboard::Key::F1);
        Command cmdShorcutF1(aShorcutF1, FastDelegate<void>(&MyAppli::onF1Pressed, this));
        getListener().connect("ShorcutF1", cmdShorcutF1);
        Action aEquipement(Action::KEY_PRESSED_ONCE, IKeyboard::E);
        Command cmdEquipement(aEquipement, FastDelegate<void>(&MyAppli::showEquipement, this));
        getListener().connect("ShowEquipement", cmdEquipement);

        wResuHero = new RenderWindow (sf::VideoMode(400, 300), "Create ODFAEG Application", sf::Style::Titlebar, ContextSettings(0, 0, 4, 3, 0));
        label = new gui::Label(*wResuHero, Vec3f(0, 0, 0), Vec3f(200, 50, 0),fm.getResourceByAlias(Fonts::Serif),"5", 15);
        getRenderComponentManager().addComponent(label);
        button = new gui::Button(Vec3f(0, 200, 0), Vec3f(400, 100, 0), fm.getResourceByAlias(Fonts::Serif),"Respawn", 15, *wResuHero);
        getRenderComponentManager().addComponent(button);
        button->addActionListener(this);
        wResuHero->setVisible(false);
        wResuHero->setPosition(sf::Vector2i(500, 400));
        getRenderComponentManager().setEventContextActivated(false, *wResuHero);
        wPickupItems = new RenderWindow(sf::VideoMode(100, 200), "PickupItems", sf::Style::Close, ContextSettings(0, 0, 4, 3, 0));
        addWindow(wPickupItems);
        wPickupItems->setVisible(false);
        getRenderComponentManager().setEventContextActivated(false, *wPickupItems);
        getWorld()->update();
        addWindow(wResuHero);
        pItems = new Panel(*wPickupItems,Vec3f(0, 0, 0),Vec3f(100, 200, 0));
        getRenderComponentManager().addComponent(pItems);
        wInventory = new RenderWindow(sf::VideoMode(500, 300), "Inventory", sf::Style::Close, ContextSettings(0, 0, 4, 3, 0));
        addWindow(wInventory);
        wInventory->setVisible(false);
        getRenderComponentManager().setEventContextActivated(false, *wInventory);
        pInventory = new Panel(*wInventory, Vec3f(0, 0, 0), Vec3f(500, 300, 0));
        getRenderComponentManager().addComponent(pInventory);
        //sf::sleep(sf::seconds(1.f));
        wIdentification = new RenderWindow(sf::VideoMode(400, 300), "Identification", sf::Style::Titlebar, ContextSettings(0, 0, 4, 3, 0));
        View iView = wIdentification->getDefaultView();
        iView.setCenter(Vec3f(wIdentification->getSize().x * 0.5f, wIdentification->getSize().y * 0.5f, 0));
        wIdentification->setView(iView);
        addWindow(wIdentification);
        labPseudo = new gui::Label(*wIdentification, Vec3f(0, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Pseudo : ", 15);
        getRenderComponentManager().addComponent(labPseudo);
        labMdp = new gui::Label(*wIdentification, Vec3f(0, 60, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Password : ", 15);
        getRenderComponentManager().addComponent(labMdp);
        taPseudo = new gui::TextArea(Vec3f(200, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif),"Pseudo", *wIdentification);
        getRenderComponentManager().addComponent(taPseudo);
        taPassword = new gui::PasswordField(Vec3f(200, 60, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif),"Password", *wIdentification);
        getRenderComponentManager().addComponent(taPassword);
        idButton = new gui::Button(Vec3f(0, 120, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Connect", 15, *wIdentification);
        getRenderComponentManager().addComponent(idButton);
        invButton = new gui::Button(Vec3f(200, 120, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Invite", 15, *wIdentification);
        getRenderComponentManager().addComponent(invButton);
        invButton->addActionListener(this);
        idButton->addActionListener(this);
        wDisplayQuests = new RenderWindow(sf::VideoMode(400, 300), "Quest list", sf::Style::Default, ContextSettings(0, 0, 4, 3, 0));
        addWindow(wDisplayQuests);
        pQuestList = new Panel(*wDisplayQuests,Vec3f(0, 0, 0),Vec3f(400, 300, 0));
        getRenderComponentManager().addComponent(pQuestList);
        wDisplayQuests->setVisible(false);
        getRenderComponentManager().setEventContextActivated(false, *wDisplayQuests);

        wDisplayQuest = new RenderWindow(sf::VideoMode(400, 600), "Quest details", sf::Style::Default, ContextSettings(0, 0, 4, 3, 0));
        addWindow(wDisplayQuest);
        lQuestName = new Label(*wDisplayQuest, Vec3f(0, 0, 0),Vec3f(400, 100, 0),fm.getResourceByAlias(Fonts::Serif), "", 15);
        getRenderComponentManager().addComponent(lQuestName);
        lQuestTask = new Label(*wDisplayQuest, Vec3f(0, 100, 0),Vec3f(400, 400, 0),fm.getResourceByAlias(Fonts::Serif), "", 15);
        getRenderComponentManager().addComponent(lQuestTask);
        bAccept = new Button(Vec3f(0, 500, 0), Vec3f(200, 100, 0),fm.getResourceByAlias(Fonts::Serif), "Accept", 15, *wDisplayQuest);
        getRenderComponentManager().addComponent(bAccept);
        bAccept->addActionListener(this);
        bDeny = new Button(Vec3f(200, 500, 0), Vec3f(200, 100, 0),fm.getResourceByAlias(Fonts::Serif), "Give up", 15, *wDisplayQuest);
        getRenderComponentManager().addComponent(bDeny);
        pRewards = new Panel(*wDisplayQuest, Vec3f(0, 400, 0), Vec3f(400, 100, 0));
        getRenderComponentManager().addComponent(pRewards);
        bDeny->addActionListener(this);
        bDeny->setName("DENY");
        wDisplayQuest->setVisible(false);
        getRenderComponentManager().setEventContextActivated(false, *wDisplayQuest);

        wDiary = new RenderWindow(sf::VideoMode(600, 600), "Diary", sf::Style::Default, ContextSettings(0, 0, 4, 3, 0));
        addWindow(wDiary);
        pQuestNames = new Panel(*wDiary, Vec3f(0, 0, 0), Vec3f(300, 500, 0));
        getRenderComponentManager().addComponent(pQuestNames);
        pQuestProgress = new Panel(*wDiary, Vec3f(300, 0, 0), Vec3f(300, 500, 0));
        getRenderComponentManager().addComponent(pQuestProgress);
        bGiveUp = new Button(Vec3f(0, 500, 0), Vec3f(600, 100, 0),fm.getResourceByAlias(Fonts::Serif),"Give up",15,*wDiary);
        bGiveUp->addActionListener(this);
        getRenderComponentManager().addComponent(bGiveUp);
        wDiary->setVisible(false);
        getRenderComponentManager().setEventContextActivated(false, *wDiary);

        wSkills = new RenderWindow(sf::VideoMode(400, 600), "Skills", sf::Style::Default, ContextSettings(0, 0, 4, 3, 0));
        addWindow(wSkills);
        pSkills = new Panel (*wSkills, Vec3f(0, 0, 0), Vec3f(400, 600, 0));
        getRenderComponentManager().addComponent(pSkills);
        wSkills->setVisible(false);
        getRenderComponentManager().setEventContextActivated(false, *wSkills);
        TextureManager<Skill::Type>& tm3 = cache.resourceManager<Texture,Skill::Type>("TextureManager3");
        Sprite sprite (*tm3.getResourceByAlias(Skill::LAST_HEAL),Vec3f(0, 0, 0),Vec3f(50, 50, 0), sf::IntRect(0, 0, 50, 50));
        floatingIcon = new Icon(getRenderWindow(), Vec3f(0, 0, -1), Vec3f(50, 50, 0), sprite);
        floatingIcon->setVisible(false);
        getRenderComponentManager().addComponent(floatingIcon);

        wEquipement = new RenderWindow(sf::VideoMode(400, 600),"Equipment",sf::Style::Default,ContextSettings(0, 0, 4, 3, 0));
        addWindow(wEquipement);
        wEquipement->setVisible(false);
        pEquipement = new Panel(*wEquipement, Vec3f(0, 0, 0), Vec3f(400, 600, 0));
        getRenderComponentManager().addComponent(pEquipement);
        Sprite sEquipement(*tm.getResourceByAlias("EQUIPEMENT"),Vec3f(0, 0, 0),Vec3f(400, 600, 0), sf::IntRect(0, 0, 300, 500));
        pEquipement->addSprite(sEquipement);



        ps->setTexture(*tm.getResourceByAlias("PARTICLE"));
        for (unsigned int i = 0; i < 10; i++) {
            ps->addTextureRect(sf::IntRect(i*10, 0, 10, 10));
        }
        emitter2.setEmissionRate(30);
        emitter2.setParticleLifetime(Distributions::uniform(sf::seconds(5), sf::seconds(7)));
        emitter2.setParticlePosition(Distributions::rect(Vec3f(50, 90, 0), Vec3f(25, 5, 0)));   // Emit particles in given circle
        emitter2.setParticleVelocity(Distributions::deflect(Vec3f(0, -10, 0),  0)); // Emit towards direction with deviation of 15°
        emitter2.setParticleRotation(Distributions::uniform(0.f, 0.f));
        emitter2.setParticleTextureIndex(Distributions::uniformui(0, 9));
        emitter2.setParticleScale(Distributions::rect(Vec3f(2.1f, 2.1f, 1.f), Vec3f(2.f, 2.f, 1.f)));
        //emitter.setParticleColor(Distributions::color(Vec3f(0, 0, 0, 255), Vec3f(0, 0, 0, 255)));

        ps->addEmitter(refEmitter(emitter2));
        psu->addParticleSystem(ps);
        getWorld()->addEntity(ps);
        setEventContextActivated(false);
        getRenderComponentManager().setEventContextActivated(false, getRenderWindow());
    }
    void MyAppli::onRender(RenderComponentManager *cm) {
        // draw everything here...
        if (isClientAuthentified) {
            getWorld()->drawOnComponents("E_BIGTILE", 0);
            getWorld()->drawOnComponents("E_WALL+E_DECOR+E_ANIMATION+E_HERO+E_MONSTER+E_PNJ+E_PARTICLES", 1);
            /*getWorld()->drawOnComponents("E_WALL+E_DECOR+E_ANIMATION+E_HERO+E_MONSTER+E_PNJ", 2);
            getWorld()->drawOnComponents("E_WALL+E_DECOR+E_ANIMATION+E_HERO+E_MONSTER+E_PNJ+E_PONCTUAL_LIGHT", 3);*/
        } else {
            getWorld()->drawOnComponents("", 0);
            getWorld()->drawOnComponents("", 1);
            getWorld()->drawOnComponents("", 2);
            getWorld()->drawOnComponents("", 3);
        }
    }
    void MyAppli::onDisplay(RenderWindow* window) {
        if (window == &getRenderWindow()) {
            std::map<int, std::pair<Sprite*, std::vector<Item>>>::iterator it;
            for (it = cristals.begin(); it != cristals.end(); it++) {
                Sprite sprite = *(it->second.first);
                window->draw(sprite);
            }
            std::vector<std::pair<std::pair<Caracter*, Text>, std::pair<sf::Time, sf::Time>>>::iterator it2;
            for (it2 = tmpTexts.begin(); it2 != tmpTexts.end();) {
                sf::Time elapsedTime = Application::getTimeClk().getElapsedTime() - it2->second.first;
                it2->second.first = Application::getTimeClk().getElapsedTime();
                it2->second.second -= elapsedTime;
                if (it2->second.second <= sf::seconds(0)) {
                    it2 = tmpTexts.erase(it2);
                } else {
                    if (it2->first.second.getColor() == sf::Color::Red || it2->first.second.getColor() == sf::Color::Green)
                        it2->first.second.setPosition(Vec3f(it2->first.first->getPosition().x, it2->first.first->getPosition().y-10, 0));
                    else
                        it2->first.second.setPosition(Vec3f(it2->first.first->getPosition().x, it2->first.first->getPosition().y-20, 0));
                    window->draw(it2->first.second);
                    it2++;
                }
            }
        }
    }
    void MyAppli::onUpdate (RenderWindow* window, IEvent& event) {
        // check all the window's events that were triggered since the last iteration of the loop
        if (event.type == IEvent::WINDOW_EVENT && event.window.type == IEvent::WINDOW_EVENT_CLOSED && window == &getRenderWindow()) {
            Network::stopCli();
            stop();
        }
        if (event.type == IEvent::WINDOW_EVENT && event.window.type == IEvent::WINDOW_EVENT_CLOSED && window == wPickupItems) {
            pItems->removeAll();
            wPickupItems->setVisible(false);
            getRenderComponentManager().setEventContextActivated(false, *wPickupItems);
            setEventContextActivated(true);
            getRenderComponentManager().setEventContextActivated(true, getRenderWindow());
        }
        if (event.type == IEvent::WINDOW_EVENT && event.window.type == IEvent::WINDOW_EVENT_CLOSED && window == wInventory) {
            pInventory->removeAll();
            wInventory->setVisible(false);
            getRenderComponentManager().setEventContextActivated(false, *wInventory);
            setEventContextActivated(true);
            getRenderComponentManager().setEventContextActivated(true, getRenderWindow());
        }
        if (event.type == IEvent::WINDOW_EVENT && event.window.type == IEvent::WINDOW_EVENT_CLOSED && window == wDisplayQuests) {
            wDisplayQuests->setVisible(false);
            getRenderComponentManager().setEventContextActivated(false, *wDisplayQuests);
            setEventContextActivated(true);
            getRenderComponentManager().setEventContextActivated(true, getRenderWindow());
        }
        if (event.type == IEvent::WINDOW_EVENT && event.window.type == IEvent::WINDOW_EVENT_CLOSED && window == wDiary) {
            wDiary->setVisible(false);
            getRenderComponentManager().setEventContextActivated(false, *wDiary);
            setEventContextActivated(true);
            getRenderComponentManager().setEventContextActivated(true, getRenderWindow());
        }
        if (event.type == IEvent::WINDOW_EVENT && event.window.type == IEvent::WINDOW_EVENT_CLOSED && window == wSkills) {
            wSkills->setVisible(false);
            /*for (unsigned int i = 0; i < pSkills->getChildren().size(); i++)
                pSkills->getChildren()[i]->setEventContextActivated(false);*/
            getRenderComponentManager().setEventContextActivated(false, *wSkills);
            setEventContextActivated(true);
            getRenderComponentManager().setEventContextActivated(true, getRenderWindow());
        }
        if (event.type == IEvent::KEYBOARD_EVENT && event.keyboard.type == IEvent::KEY_EVENT_PRESSED && window == &getRenderWindow()) {
            previousKey = actualKey;
            actualKey = static_cast<IKeyboard::Key>(event.keyboard.code);
            getListener().setCommandSlotParams("MoveAction", this, static_cast<IKeyboard::Key>(event.keyboard.code));
            getListener().setCommandSlotParams("PickUpItems", this, static_cast<IKeyboard::Key>(event.keyboard.code));
        }
        if (event.type == IEvent::KEYBOARD_EVENT && event.keyboard.type == IEvent::KEY_EVENT_RELEASED
            && hero != nullptr && hero->isMovingFromKeyboard() && window == &getRenderWindow()) {
            previousKey = static_cast<IKeyboard::Key>(event.keyboard.code);
            actualKey = IKeyboard::Key::Unknown;
            sf::Int64 cli_time = Application::getTimeClk().getElapsedTime().asMicroseconds();
            std::string message = "STOPCARMOVE*"+conversionIntString(hero->getId())+"*"+conversionLongString(cli_time);
            SymEncPacket packet;
            packet<<message;
            Network::sendTcpPacket(packet);
        }
        if (event.type == IEvent::MOUSE_BUTTON_EVENT && event.mouseButton.type == IEvent::BUTTON_EVENT_PRESSED && window == &getRenderWindow()) {
            sf::Vector2f mousePos (event.mouseButton.x, event.mouseButton.y);
            getListener().setCommandSlotParams("LeftMouseButtonPressedAction", this, mousePos);
            getListener().setCommandSlotParams("RightMouseButtonPressedAction", this, mousePos);
        }
    }
    void MyAppli::onExec () {
        if (getClock("RequestTime").getElapsedTime().asSeconds() >= timeBtwnTwoReq.asSeconds() && isClientAuthentified) {
            std::string request = "GETCARPOS";
            sf::Packet packet;
            packet<<request;
            Network::sendUdpPacket(packet);
            getClock("RequestTime").restart();
        }
        std::string response;
        if (Network::getResponse("ADDLIFE", response)) {
            std::vector<std::string> infos = split(response, "*");
            int id = conversionStringInt(infos[0]);
            float amount = conversionStringFloat(infos[1]);
            Caracter* caracter = static_cast<Caracter*>(getWorld()->getEntity(id));
            caracter->setLife(caracter->getLife() + amount);
        }
        if (Network::getResponse("RETRACTMANA", response)) {
            std::vector<std::string> infos = split(response, "*");
            int id = conversionStringInt(infos[0]);
            float amount = conversionStringFloat(infos[1]);
            Caracter* caracter = static_cast<Caracter*>(getWorld()->getEntity(id));
            caracter->setMana(caracter->getMana() - amount);
        }
        if (Network::getResponse("SHOWQUEST", response)) {
            std::cout<<"show quests"<<std::endl;
            Pnj* pnj = static_cast<Pnj*>(getWorld()->getEntity(conversionStringInt(response)));
            selectedPnj = pnj;
            std::vector<Quest> quests = pnj->getQuests();
            std::vector<Quest> diary = static_cast<Hero*>(hero)->getDiary();
            pQuestList->removeAll();
            FontManager<Fonts>& fm = cache.resourceManager<Font, Fonts>("FontManager");
            for (unsigned int i = 0; i < quests.size(); i++) {
                if (static_cast<Hero*>(hero)->containsQuest(quests[i].getName())) {
                    Quest* quest = static_cast<Hero*>(hero)->getQuest(quests[i].getName());
                    if (quest != nullptr && quest->getStatus() != Quest::FINISHED) {
                        Label* label = new Label(*wDisplayQuests,Vec3f(0, i * 100, 0),Vec3f(400, 100, 0),fm.getResourceByAlias(Fonts::Serif), quests[i].getName(), 15);
                        pQuestList->addChild(label);
                        label->setParent(pQuestList);
                        Action aLabClicked(Action::MOUSE_BUTTON_PRESSED_ONCE, IMouse::Left);
                        Command cmdLabClicked(aLabClicked, FastDelegate<bool>(&Label::isMouseInside, label), FastDelegate<void>(&MyAppli::onLabQuestClicked, this, label));
                        label->getListener().connect("ALABCLICKED", cmdLabClicked);
                    }
                } else {
                    Label* label = new Label(*wDisplayQuests,Vec3f(0, i * 100, 0),Vec3f(400, 100, 0),fm.getResourceByAlias(Fonts::Serif), quests[i].getName(), 15);
                    pQuestList->addChild(label);
                    label->setParent(pQuestList);
                    Action aLabClicked(Action::MOUSE_BUTTON_PRESSED_ONCE, IMouse::Left);
                    Command cmdLabClicked(aLabClicked, FastDelegate<bool>(&Label::isMouseInside, label), FastDelegate<void>(&MyAppli::onLabQuestClicked, this, label));
                    label->getListener().connect("ALABCLICKED", cmdLabClicked);
                }
            }
            wDisplayQuests->setVisible(true);
            getRenderComponentManager().setEventContextActivated(true, *wDisplayQuests);
            setEventContextActivated(false);
            getRenderComponentManager().setEventContextActivated(false, getRenderWindow());
        }
        if (Network::getResponse("NEWPATH", response) && isClientAuthentified) {
            std::vector<std::string> infos = split(response, "*");
            std::vector<Vec2f> path;
            int size = conversionStringInt(infos[0]);
            int id = conversionStringInt(infos[1]);
            Caracter* caracter = static_cast<Caracter*>(getWorld()->getEntity(id));
            /*if (caracter->getType() == "E_HERO") {
                std::cout<<"id : "<<caracter->getId()<<std::endl;
            }*/
            if (static_cast<Hero*>(caracter) && static_cast<Hero*>(caracter)->isMovingFromKeyboard()) {
                caracter->setIsMovingFromKeyboard(false);
            }
            Vec2f actualPos (conversionStringFloat(infos[2]), conversionStringFloat(infos[3]));
            sf::Int64 last_cli_time = conversionStringLong(infos[4]);
            sf::Int64 elapsedTime = Application::getTimeClk().getElapsedTime().asMicroseconds() - last_cli_time;
            bool isInFightingMode = conversionStringInt(infos[5]);
            caracter->setFightingMode(isInFightingMode);
            if (caracter->isAttacking())
                caracter->setAttacking(false);
            caracter->setFightingMode(false);
            Vec2f newPos = Computer::getPosOnPathFromTime(actualPos, caracter->getPath(),elapsedTime,caracter->getSpeed());
            for (int i = 0; i < size; i++) {
                path.push_back(Vec2f(conversionStringFloat(infos[i*2+6]), conversionStringFloat(infos[i*2+7])));
            }
            if (elapsedTime > 0) {
                Vec2f d = newPos - actualPos;
                if (id == hero->getId()) {
                    for (unsigned int i = 0; i < getRenderComponentManager().getNbComponents(); i++) {
                        if (getRenderComponentManager().getRenderComponent(i) != nullptr) {
                            View view = getRenderComponentManager().getRenderComponent(i)->getView();
                            view.move(d.x, d.y, d.y);
                            getRenderComponentManager().getRenderComponent(i)->setView(view);
                        }
                    }
                    getView().move (d.x, d.y, d.y);
                }
                Vec2f dir = d.normalize();
                if (dir != caracter->getDir())
                    caracter->setDir(dir);
                getWorld()->moveEntity(caracter, d.x, d.y, d.y);
            }
            caracter->setPath(path);
            caracter->setMoving(true);
            caracter->interpolation.first = caracter->getCenter();
            caracter->interpolation.second = Computer::getPosOnPathFromTime(caracter->interpolation.first, caracter->getPath(),ping + timeBtwnTwoReq.asMicroseconds(),caracter->getSpeed());
            caracter->getClkTransfertTime().restart();
            std::string request = "GETCARPOS";
            sf::Packet packet;
            packet<<request;
            Network::sendUdpPacket(packet);
            getClock("RequestTime").restart();
        }
        if (Network::getResponse("NEWPOS", response)) {
            std::vector<std::string> infos = split(response, "*");
            int id = conversionStringInt(infos[0]);
            ping = conversionStringLong(infos[1]);
            Caracter* caracter = static_cast<Caracter*>(getWorld()->getEntity(id));
            Vec3f actualPos = Vec3f(caracter->getCenter().x, caracter->getCenter().y, 0);
            Vec3f newPos (conversionStringFloat(infos[2]), conversionStringFloat(infos[3]), conversionStringFloat(infos[4]));

            sf::Int64 last_cli_time = conversionStringLong(infos[5]);
            sf::Int64 elapsedTime = Application::getTimeClk().getElapsedTime().asMicroseconds() - last_cli_time;
            bool isMoving = conversionStringInt(infos[6]);
            bool isInFightingMode = conversionStringInt(infos[7]);
            bool isAttacking = conversionStringInt(infos[8]);
            bool isAlive = conversionStringInt(infos[9]);
            int life = conversionStringInt(infos[10]);
            if (elapsedTime > 0 && last_cli_time > caracter->getAttribute("isMoving"+conversionIntString(id)).getValue<sf::Int64>()) {
                caracter->setMoving(isMoving);
            }
            if (elapsedTime > 0 && last_cli_time > caracter->getAttribute("isInFightingMode"+conversionIntString(id)).getValue<sf::Int64>()) {
                caracter->setFightingMode(isInFightingMode);
            }
            if (elapsedTime > 0 && last_cli_time > caracter->getAttribute("isAttacking"+conversionIntString(id)).getValue<sf::Int64>()) {
                caracter->setAttacking(isAttacking);
            }
            if (elapsedTime > 0 && last_cli_time > caracter->getAttribute("isAlive"+conversionIntString(id)).getValue<sf::Int64>()) {
                caracter->setAlive(isAlive);
            }
            if (!caracter->isMoving() && static_cast<Hero*>(caracter)->isMovingFromKeyboard()) {
                static_cast<Hero*>(caracter)->setIsMovingFromKeyboard(false);
            }
            if (static_cast<Hero*> (caracter) && static_cast<Hero*>(caracter)->isMovingFromKeyboard() && caracter->isMoving()) {
                newPos = newPos + Vec3f(caracter->getDir().x, caracter->getDir().y, 0) * caracter->getSpeed() * elapsedTime;
            } else if (caracter->isMoving()) {
                newPos = Computer::getPosOnPathFromTime(newPos, caracter->getPath(),elapsedTime, caracter->getSpeed());
            }
            if (elapsedTime > 0 && last_cli_time > caracter->getAttribute("position"+conversionIntString(id)).getValue<sf::Int64>()) {
                Vec3f d = newPos - actualPos;

                if (id == hero->getId()) {
                    for (unsigned int i = 0; i < getRenderComponentManager().getNbComponents(); i++) {
                        if (getRenderComponentManager().getRenderComponent(i) != nullptr) {
                            View view = getRenderComponentManager().getRenderComponent(i)->getView();
                            view.move(d.x, d.y, d.y);
                            getRenderComponentManager().getRenderComponent(i)->setView(view);
                        }
                    }
                    getView().move (d.x, d.y, d.y);
                }
                getWorld()->moveEntity(caracter, d.x, d.y, d.y);
                //getWorld()->update();
            }
            caracter->interpolation.first = caracter->getCenter();
            if (caracter->isMoving()) {
                if (caracter->isMovingFromKeyboard()) {
                    caracter->interpolation.second = caracter->interpolation.first + Vec3f(caracter->getDir().x,caracter->getDir().y,0)  * caracter->getSpeed() * (ping + timeBtwnTwoReq.asMicroseconds());
                } else {
                    caracter->interpolation.second = Computer::getPosOnPathFromTime(caracter->interpolation.first, caracter->getPath(),ping + timeBtwnTwoReq.asMicroseconds(),caracter->getSpeed());
                }
            } else {
                caracter->interpolation.second = caracter->interpolation.first;
            }
            caracter->getClkTransfertTime().restart();
       }
       if (Network::getResponse("DEATH", response)) {
           int id = conversionStringInt(response);
           Caracter* caracter = static_cast<Caracter*>(getWorld()->getEntity(id));
           std::cout<<caracter->getType()<<" is death"<<std::endl;
           caracter->setLife(0);
           caracter->setAttacking(false);
           caracter->setFightingMode(false);
           caracter->setAlive(false);
           if (caracter->isMonster()) {
                static_cast<Hero*>(caracter->getFocusedCaracter())->up(static_cast<Monster*>(caracter)->getXp());
                Hero* hero = static_cast<Hero*>(caracter->getFocusedCaracter());
                for (unsigned int i = 0; i < hero->getDiary().size(); i++) {
                    hero->getDiary()[i].addMonsterToKillProgress(caracter->getName());
                }
           }
       }
       if (Network::getResponse("ENTERINFIGHTINGMODE", response)) {
            int id = conversionStringInt(response);
            Entity* entity = getWorld()->getEntity(id);
            if (static_cast<Caracter*> (entity))
                static_cast<Caracter*> (entity)->setFightingMode(true);
       }
       if (Network::getResponse("LEAVEINFIGHTINGMODE", response)) {
            int id = conversionStringInt(response);
            Entity* entity = getWorld()->getEntity(id);
            if (static_cast<Caracter*> (entity))
                static_cast<Caracter*> (entity)->setFightingMode(false);
       }
       if (Network::getResponse("SETATTACKING", response)) {
            //std::cout<<"response : "<<response<<std::endl;
            std::vector<std::string> parts = split(response, "*");
            //std::cout<<"size : "<<parts.size()<<std::endl;
            int id = conversionStringInt(parts[0]);
            Entity* entity = getWorld()->getEntity(id);
            int idFocused = conversionStringInt(parts[1]);
            std::cout<<"ids : "<<parts[0]<<std::endl<<parts[1]<<std::endl;
            Entity* focused = getWorld()->getEntity(idFocused);
            if (static_cast<Caracter*> (entity)) {
                static_cast<Caracter*> (entity)->setAttacking(true);
                static_cast<Caracter*> (entity)->setFocusedCaracter(static_cast<Caracter*>(focused));
                static_cast<Caracter*> (entity)->restartAttackSpeed();
                Vec2f dir = Vec2f(static_cast<Caracter*> (entity)->getFocusedCaracter()->getCenter().x, static_cast<Caracter*> (entity)->getFocusedCaracter()->getCenter().y) - Vec2f(static_cast<Caracter*> (entity)->getCenter().x, static_cast<Caracter*> (entity)->getCenter().y);
                dir = dir.normalize();
                if (static_cast<Caracter*> (entity)->getDir() != dir)
                    static_cast<Caracter*> (entity)->setDir(dir);
            }
       }
       if (Network::getResponse("ALIVE", response)) {
            std::vector<std::string> infos = split(response, "*");
            int id = conversionStringInt(infos[0]);
            Vec3f center(conversionStringFloat(infos[1]), conversionStringFloat(infos[2]), conversionStringFloat(infos[2]));
            Caracter* caracter = static_cast<Caracter*>(getWorld()->getEntity(id));
            if (caracter->getType() == "E_HERO") {
                wResuHero->setVisible(false);
                button->setEventContextActivated(false);
                setEventContextActivated(true);
            }
            caracter->setCenter(center);
            caracter->setMoving(false);
            caracter->setFightingMode(false);
            caracter->setAttacking(false);
            caracter->setAlive(true);
            caracter->restartRegenHP();
       }
       if (Network::getResponse("ATTACK", response)) {
            Caracter* monster = static_cast<Caracter*>(getWorld()->getEntity(conversionStringInt(response)));
            hero->setFocusedCaracter(monster);
            hero->setFightingMode(true);
            hero->setAttacking(false);
            hero->setMoving(false);
        }
        if (Network::getResponse("IDOK", response)) {
            isClientAuthentified = true;
            setEventContextActivated(true);
            getRenderComponentManager().setEventContextActivated(true, getRenderWindow());
            wIdentification->setVisible(false);
            getRenderComponentManager().setEventContextActivated(false, *wIdentification);
            int heroId = conversionStringInt(response);
            SymEncPacket packet;
            packet<<"GETMAPINFOS";
            Network::sendTcpPacket(packet);
            std::string response = Network::waitForLastResponse("MAPINFOS", sf::seconds(20.f));
            std::istringstream iss(response);
            ITextArchive ia(iss);
            std::vector<Entity*> entities;
            ia(entities);
            TextureManager<> &tm = cache.resourceManager<Texture, std::string>("TextureManager");
            std::cout<<"load map"<<std::endl;
            for (unsigned int i = 0; i < entities.size(); i++) {
                getWorld()->addEntity(entities[i]);
                if (entities[i]->getType() == "E_BIGTILE") {
                    for (unsigned int j = 0; j < entities[i]->getChildren().size(); j++) {
                        std::string texId =  entities[i]->getChildren()[j]->getFaces()[0].getMaterial().getTexId();

                        sf::IntRect texRect = entities[i]->getChildren()[j]->getFaces()[0].getMaterial().getTexRect();
                        entities[i]->getChildren()[j]->getFaces()[0].getMaterial().clearTextures();
                        entities[i]->getChildren()[j]->getFaces()[0].getMaterial().addTexture(tm.getResourceByAlias(texId), texRect);
                        entities[i]->getChildren()[j]->getFaces()[0].getMaterial().setTexId(texId);
                    }

                } else if (entities[i]->getType() == "E_WALL") {
                    std::string texId =  entities[i]->getChildren()[0]->getFaces()[0].getMaterial().getTexId();

                    sf::IntRect texRect = entities[i]->getChildren()[0]->getFaces()[0].getMaterial().getTexRect();
                    entities[i]->getChildren()[0]->getFaces()[0].getMaterial().clearTextures();
                    entities[i]->getChildren()[0]->getFaces()[0].getMaterial().addTexture(tm.getResourceByAlias(texId), texRect);
                    entities[i]->getChildren()[0]->getFaces()[0].getMaterial().setTexId(texId);
                    getWorld()->getGridCellAt(Vec3f(entities[i]->getCenter().x, entities[i]->getCenter().y, 0))->setPassable(false);
                } else if (entities[i]->getType() == "E_DECOR") {
                    std::string texId =  entities[i]->getChildren()[0]->getFaces()[0].getMaterial().getTexId();
                    sf::IntRect texRect = entities[i]->getChildren()[0]->getFaces()[0].getMaterial().getTexRect();
                    entities[i]->getChildren()[0]->getFaces()[0].getMaterial().clearTextures();
                    entities[i]->getChildren()[0]->getFaces()[0].getMaterial().addTexture(tm.getResourceByAlias(texId), texRect);
                    entities[i]->getChildren()[0]->getFaces()[0].getMaterial().setTexId(texId);
                } else if (entities[i]->getType() == "E_ANIMATION") {
                    Anim* anim = static_cast<Anim*> (entities[i]);
                    for (unsigned int j = 0; j < anim->getChildren().size(); j++) {
                        if (entities[i]->getChildren()[j]->getChildren().size() > 0) {
                            std::string texId = entities[i]->getChildren()[j]->getChildren()[0]->getFaces()[0].getMaterial().getTexId();
                            sf::IntRect texRect = entities[i]->getChildren()[j]->getChildren()[0]->getFaces()[0].getMaterial().getTexRect();
                            entities[i]->getChildren()[j]->getChildren()[0]->getFaces()[0].getMaterial().clearTextures();
                            entities[i]->getChildren()[j]->getChildren()[0]->getFaces()[0].getMaterial().addTexture(tm.getResourceByAlias(texId), texRect);
                            entities[i]->getChildren()[j]->getChildren()[0]->getFaces()[0].getMaterial().setTexId(texId);
                        }
                    }
                    anim->play(true);
                    au->addAnim(anim);
                }
            }

            FontManager<Fonts>& fm = cache.resourceManager<Font, Fonts>("FontManager");
            packet.clear();
            packet<<"GETCARINFOS";
            Network::sendTcpPacket(packet);
            response = Network::waitForLastResponse("CARINFOS", sf::seconds(5.f));
            iss.str("");
            ia.clear();
            iss.str(response);
            std::vector<Entity*> heroes;
            ia(heroes);
            for (unsigned int i = 0; i < heroes.size(); i++) {
                Hero* player = static_cast<Hero*>(heroes[i]);
                std::string path = "tilesets/vlad_sword.png";
                const Texture *text = cache.resourceManager<Texture, std::string>("TextureManager").getResourceByPath(path);
                int textRectX = 0, textRectY = 0, textRectWidth = 50, textRectHeight = 100;
                int textWidth = text->getSize().x;
                Vec3f tmpCenter = player->getCenter();
                player->setCenter(Vec3f(0, 0, 0));
                player->setShadowCenter(Vec3f(0, 200, 0));
                player->setShadowScale(Vec3f(1, -1, 1));
                for (unsigned int i = 0; i < 8; i++) {
                    Anim* animation = factory.make_entity<Anim>(0.1f, Vec3f(-25, -50, 0), Vec3f(50, 100, 0), factory);
                    for (unsigned int j = 0; j < 8; j++) {
                        sf::IntRect textRect (textRectX, textRectY, textRectWidth, textRectHeight);
                        Tile *tile = factory.make_entity<Tile>(text, Vec3f(-25, -50, 0), Vec3f(50, 100, 0), textRect, factory);
                        tile->getFaces()[0].getMaterial().setTexId("VLADSWORD");
                        g2d::Decor *frame = factory.make_entity<g2d::Decor>(tile, &g2d::AmbientLight::getAmbientLight(), factory);
                        //frame->setShadowCenter(Vec3f(0, 200, 0));
                        if (textRectX + textRectWidth >= textWidth) {
                            textRectX = 0;
                            textRectY += textRectHeight;
                        } else {
                            textRectX += textRectWidth;
                        }
                        animation->getCurrentFrame()->setBoneIndex(i);
                        animation->addFrame(frame);
                    }
                    player->addAnimation(animation);
                    au->addAnim(animation);
                }
                for (unsigned int i = 0; i < 8; i++) {
                    Anim* animation = factory.make_entity<Anim>(0.1f, Vec3f(-25, -50, 0), Vec3f(50, 100, 0), factory);
                    for (unsigned int j = 0; j < 12; j++) {
                        sf::IntRect textRect (textRectX, textRectY, textRectWidth, textRectHeight);
                        Tile *tile = factory.make_entity<Tile>(text, Vec3f(-25, -50, 0), Vec3f(50, 100, 0), textRect, factory);
                        tile->getFaces()[0].getMaterial().setTexId("VLADSWORD");
                        g2d::Decor *frame = factory.make_entity<g2d::Decor>(tile, &g2d::AmbientLight::getAmbientLight(), factory);
                        //frame->setShadowCenter(Vec3f(0, 200, 0));
                        if (textRectX + textRectWidth >= textWidth) {
                            textRectX = 0;
                            textRectY += textRectHeight;
                        } else {
                            textRectX += textRectWidth;
                        }
                        animation->addFrame(frame);
                    }
                    animation->getCurrentFrame()->setBoneIndex(i+8);
                    player->addAnimation(animation);
                    au->addAnim(animation);
                }
                textRectWidth = 100;
                for (unsigned int i = 0; i < 8; i++) {
                    Anim* animation = factory.make_entity<Anim>(0.1f, Vec3f(-50, -50, 0), Vec3f(100, 100, 0), factory);
                    for (unsigned int j = 0; j < 12; j++) {
                        sf::IntRect textRect (textRectX, textRectY, textRectWidth, textRectHeight);
                        Tile *tile = factory.make_entity<Tile>(text, Vec3f(-50, -50, 0), Vec3f(100, 100, 0), textRect, factory);
                        tile->getFaces()[0].getMaterial().setTexId("VLADSWORD");
                        g2d::Decor *frame = factory.make_entity<g2d::Decor>(tile, &g2d::AmbientLight::getAmbientLight(), factory);
                        //frame->setShadowCenter(Vec3f(0, 200, 0));
                        if (textRectX + textRectWidth >= textWidth) {
                            textRectX = 0;
                            textRectY += textRectHeight;
                        } else {
                            textRectX += textRectWidth;
                        }
                        animation->addFrame(frame);
                    }
                    animation->getCurrentFrame()->setBoneIndex(i+16);
                    player->addAnimation(animation);
                    au->addAnim(animation);
                }
                player->setCenter(tmpCenter);
                if (heroId == player->getId()) {
                    //std::cout<<"hero id : "<<player->getId()<<std::endl;
                    hero = player;
                    getView().move(hero->getCenter().x, hero->getCenter().y, hero->getCenter().z - 300);
                    for (unsigned int i = 0; i < getRenderComponentManager().getNbComponents(); i++) {
                        if (getRenderComponentManager().getRenderComponent(i) != nullptr) {
                            View view = getView();
                            getRenderComponentManager().getRenderComponent(i)->setView(view);
                        }
                    }
                    hpBar = new gui::ProgressBar(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(100, 10, 0),*fm.getResourceByAlias(Fonts::Serif),15);
                    hpBar->setMaximum(100);
                    hpBar->setMinimum(0);
                    hpBar->setValue(100);
                    xpBar = new gui::ProgressBar(getRenderWindow(), Vec3f(0, 590, 0), Vec3f(800, 10, 0),*fm.getResourceByAlias(Fonts::Serif),15);
                    xpBar->setMaximum(1500);
                    xpBar->setMinimum(0);
                    xpBar->setValue(0);

                    manaBar = new gui::ProgressBar(getRenderWindow(), Vec3f(0, 20, 0), Vec3f(100, 10, 0),*fm.getResourceByAlias(Fonts::Serif),15);
                    manaBar->setMaximum(100);
                    manaBar->setMinimum(0);
                    manaBar->setValue(100);
                    manaBar->setColor(sf::Color::Blue);
                    getRenderComponentManager().addComponent(hpBar);
                    getRenderComponentManager().addComponent(xpBar);
                    getRenderComponentManager().addComponent(manaBar);
                    hero->setXpHpBar(xpBar, hpBar, manaBar);
                }
                for (unsigned int i = 0; i < shorcutsButtons.size(); i++) {
                    shorcutsButtons[i] = new Button(Vec3f(i * 50, 540, 0),Vec3f(45, 45, 0), fm.getResourceByAlias(Fonts::Serif), "F"+conversionIntString(i+1), 10,getRenderWindow());
                    getRenderComponentManager().addComponent(shorcutsButtons[i]);
                }
                getWorld()->addEntity(player);
            }
            //Network::sendTcpPacket(packet);
            response = Network::waitForLastResponse("MONSTERSINFOS", sf::seconds(5.f));
            iss.str("");
            ia.clear();
            iss.str(response);
            std::vector<Entity*> monsters;
            ia(monsters);

            for (unsigned int i = 0; i < monsters.size(); i++) {
                Monster* monster = dynamic_cast<Monster*>(monsters[i]);
                std::string path = "tilesets/ogro.png";
                //for (unsigned int n = 0; n < monsters.size(); n++) {
                Vec3f tmpCenter = monster->getCenter();
                monster->setCenter(Vec3f(0, 0, 0));
                monster->setShadowCenter(Vec3f(0, 200, 0));
                monster->setShadowScale(Vec3f(1, -1, 1));
                const Texture* text = cache.resourceManager<Texture, std::string>("TextureManager").getResourceByPath(path);
                int textRectX = 0, textRectY = 0, textRectWidth = 50, textRectHeight = 100;
                int textWidth = text->getSize().x;
                for (unsigned int i = 0; i < 8; i++) {
                    Anim* animation = factory.make_entity<Anim>(0.1f, Vec3f(-25, -50, 0), Vec3f(50, 100, 0), factory);
                    for (unsigned int j = 0; j < 8; j++) {
                        sf::IntRect textRect (textRectX, textRectY, textRectWidth, textRectHeight);
                        Tile *tile = factory.make_entity<Tile>(text, Vec3f(-25, -50, 0), Vec3f(50, 100, 0), textRect, factory);
                        tile->getFaces()[0].getMaterial().setTexId("OGRO");
                        g2d::Decor *frame = factory.make_entity<g2d::Decor>(tile, &g2d::AmbientLight::getAmbientLight(), factory);
                        //frame->setShadowCenter(Vec3f(0, 200, 0));
                        if (textRectX + textRectWidth >= textWidth) {
                            textRectX = 0;
                            textRectY += textRectHeight;
                        } else {
                            textRectX += textRectWidth;
                        }
                        animation->addFrame(frame);
                    }
                    animation->getCurrentFrame()->setBoneIndex(i);
                    monster->addAnimation(animation);
                    au->addAnim(animation);
                }
                for (unsigned int i = 0; i < 8; i++) {
                    Anim* animation = factory.make_entity<Anim>(0.1f, Vec3f(-25, -50, 0), Vec3f(50, 100, 0), factory);
                    for (unsigned int j = 0; j < 11; j++) {
                        sf::IntRect textRect (textRectX, textRectY, textRectWidth, textRectHeight);
                        Tile *tile = factory.make_entity<Tile>(text, Vec3f(-25, -50, 0), Vec3f(50, 100, 0), textRect, factory);
                        tile->getFaces()[0].getMaterial().setTexId("OGRO");
                        g2d::Decor *frame = factory.make_entity<g2d::Decor>(tile, &g2d::AmbientLight::getAmbientLight(), factory);
                        //frame->setShadowCenter(Vec3f(0, 200, 0));
                        //decor->changeGravityCenter(Vec3f(50, 50, 0));
                        if (textRectX + textRectWidth >= textWidth) {
                            textRectX = 0;
                            textRectY += textRectHeight;
                        } else {
                            textRectX += textRectWidth;
                        }
                        animation->addFrame(frame);
                    }
                    animation->getCurrentFrame()->setBoneIndex(i+8);
                    monster->addAnimation(animation);
                    au->addAnim(animation);
                }
                textRectWidth = 100;
                for (unsigned int i = 0; i < 8; i++) {
                    Anim* animation = factory.make_entity<Anim>(0.1f, Vec3f(-50, -50, 0), Vec3f(100, 100, 0), factory);
                    for (unsigned int j = 0; j < 11; j++) {
                        sf::IntRect textRect(textRectX, textRectY, textRectWidth, textRectHeight);
                        Tile *tile = factory.make_entity<Tile>(text, Vec3f(-50, -50, 0), Vec3f(100, 100, 0), textRect, factory);
                        tile->getFaces()[0].getMaterial().setTexId("OGRO");
                        g2d::Decor *frame = factory.make_entity<g2d::Decor>(tile, &g2d::AmbientLight::getAmbientLight(), factory);
                        //frame->setShadowCenter(Vec3f(0, 200, 0));
                        //decor->changeGravityCenter(Vec3f(50, 50, 0));
                        if (textRectX + textRectWidth >= textWidth) {
                            textRectX = 0;
                            textRectY += textRectHeight;
                        } else {
                            textRectX += textRectWidth;
                        }
                        animation->addFrame(frame);
                    }
                    animation->getCurrentFrame()->setBoneIndex(i+16);
                    monster->addAnimation(animation);
                    au->addAnim(animation);
                }
                monster->setCenter(tmpCenter);
                ProgressBar* fcHpBar = new gui::ProgressBar(getRenderWindow(), Vec3f(700, 0, 0), Vec3f(100, 10, 0),*fm.getResourceByAlias(Fonts::Serif),15);
                fcHpBar->setMinimum(0);
                fcHpBar->setMaximum(100);
                fcHpBar->setValue(100);
                fcHpBar->setVisible(false);
                monster->setXpHpBar(nullptr, fcHpBar, nullptr);
                getRenderComponentManager().addComponent(fcHpBar);
                getWorld()->addEntity(monster);
            }
            response = Network::waitForLastResponse("PNJINFOS");
            iss.str("");
            ia.clear();
            iss.str(response);
            std::vector<Entity*> pnjs;
            ia(pnjs);
            for (unsigned int i = 0; i < pnjs.size(); i++) {
                Pnj* pnj = static_cast<Pnj*>(pnjs[i]);
                pnj->setShadowCenter(Vec3f(0, 200, 0));
                pnj->setShadowScale(Vec3f(1, -1, 1));
                std::string path = "tilesets/luigi.png";
                const Texture* text = cache.resourceManager<Texture, std::string>("TextureManager").getResourceByPath(path);
                int textRectX = 0, textRectY = 0, textRectWidth = 50, textRectHeight = 100;
                int textWidth = text->getSize().x;
                for (unsigned int i = 0; i < 1; i++) {
                    Anim* animation = factory.make_entity<Anim>(0.1f, Vec3f(275, 250, 0), Vec3f(50, 100, 0), factory);
                    for (unsigned int j = 0; j < 8; j++) {
                        sf::IntRect textRect (textRectX, textRectY, textRectWidth, textRectHeight);
                        Tile *tile = factory.make_entity<Tile>(text, Vec3f(275, 250, 0), Vec3f(50, 100, 0), textRect, factory);
                        tile->setName("PNJ");
                        tile->getFaces()[0].getMaterial().setTexId("LUIGI");
                        g2d::Decor *frame = factory.make_entity<g2d::Decor>(tile, &g2d::AmbientLight::getAmbientLight(), factory);
                        //frame->setShadowCenter(Vec3f(0, 200, 0));
                        if (textRectX + textRectWidth >= textWidth) {
                            textRectX = 0;
                            textRectY += textRectHeight;
                        } else {
                            textRectX += textRectWidth;
                        }
                        animation->addFrame(frame);
                    }
                    animation->getCurrentFrame()->setBoneIndex(i);
                    pnj->addAnimation(animation);
                    au->addAnim(animation);
                }
                getWorld()->addEntity(pnj);
            }

            //monster->setXpHpBar(nullptr, fcHpBar);
            packet.clear();
            packet<<"GETCARPOS";
            hero->getClkTransfertTime().restart();
            getClock("RequestTime").restart();
            Network::sendTcpPacket(packet);
            received = false;
            getWorld()->update();
            setEventContextActivated(true);
       }
       if (Network::getResponse("ITEMS", response)) {
            std::vector<std::string> parts = split(response,"*");
            int spotID = conversionStringInt(parts[0]);
            std::istringstream iss(parts[1]);
            ITextArchive ita(iss);
            std::vector<Item> items;
            ita(items);
            if (items.size() > 0) {
                TextureManager<> &tm = cache.resourceManager<Texture, std::string>("TextureManager");
                Sprite* cristal = new Sprite(*tm.getResourceByAlias("CRISTAL"),hero->getFocusedCaracter()->getPosition(),Vec3f(50, 100, 0),sf::IntRect(0, 0, 50, 100));
                cristals.insert(std::make_pair(spotID, std::make_pair(cristal, items)));
            }
       }
       if (Network::getResponse("REMOVEITEM", response)) {
           std::map<int, std::pair<Sprite*, std::vector<Item>>>::iterator it;
           int spotID = conversionStringInt(response);
           for (it = cristals.begin(); it != cristals.end();) {
               if (spotID == it->first) {
                    it = cristals.erase(it);
               } else {
                   it++;
               }
           }
       }
       if (Network::getResponse("TAKE_DMG", response)) {
            std::vector<std::string> parts = split(response, "*");
            int id = conversionStringInt(parts[0]);
            int dmg = conversionStringInt(parts[1]);
            Caracter* caracter = static_cast<Caracter*>(getWorld()->getEntity(id));
            caracter->getFocusedCaracter()->attackFocusedCaracter(dmg);
            FontManager<Fonts>& fm = cache.resourceManager<Font, Fonts>("FontManager");
            Text text;
            text.setFont(*fm.getResourceByAlias(Fonts::Serif));
            text.setString("-"+conversionIntString(dmg));
            text.setColor(sf::Color::Red);
            text.setCharacterSize(10);
            text.setPosition(Vec3f(caracter->getPosition().x,caracter->getPosition().y-10, 0));
            text.setSize(Vec3f(10, 10, 0));
            tmpTexts.push_back(std::make_pair(std::make_pair(caracter, text), std::make_pair(Application::getTimeClk().getElapsedTime(), sf::seconds(0.5))));
       }
       if (Network::getResponse("RGN_LIFE", response) && isClientAuthentified) {
            std::vector<std::string> parts = split(response, "*");
            int id = conversionStringInt(parts[0]);
            int rgn = conversionStringInt(parts[1]);
            Caracter* caracter = static_cast<Caracter*>(getWorld()->getEntity(id));
            if (caracter->getLife() + rgn >= caracter->getMaxLife()) {
                caracter->setLife(caracter->getMaxLife());
            } else {
                caracter->setLife(caracter->getLife() + rgn);
            }
            FontManager<Fonts>& fm = cache.resourceManager<Font, Fonts>("FontManager");
            Text text;
            text.setFont(*fm.getResourceByAlias(Fonts::Serif));
            text.setString("+"+conversionIntString(rgn));
            text.setColor(sf::Color::Green);
            text.setCharacterSize(10);
            text.setPosition(Vec3f(caracter->getPosition().x,caracter->getPosition().y-10, 0));
            text.setSize(Vec3f(10, 10, 0));
            tmpTexts.push_back(std::make_pair(std::make_pair(caracter, text), std::make_pair(Application::getTimeClk().getElapsedTime(), sf::seconds(0.5))));
       }
       if (Network::getResponse("RGN_MANA", response) && isClientAuthentified) {
            std::vector<std::string> parts = split(response, "*");
            int id = conversionStringInt(parts[0]);
            int rgn = conversionStringInt(parts[1]);
            Caracter* caracter = static_cast<Caracter*>(getWorld()->getEntity(id));
            if (caracter->getMana() + rgn >= caracter->getManaMax()) {
                caracter->setMana(caracter->getManaMax());
            } else {
                caracter->setMana(caracter->getMana() + rgn);
            }
            FontManager<Fonts>& fm = cache.resourceManager<Font, Fonts>("FontManager");
            Text text;
            text.setFont(*fm.getResourceByAlias(Fonts::Serif));
            text.setString("+"+conversionIntString(rgn));
            text.setColor(sf::Color::Blue);
            text.setCharacterSize(10);
            text.setPosition(Vec3f(caracter->getPosition().x,caracter->getPosition().y-20, 0));
            text.setSize(Vec3f(10, 10, 0));
            tmpTexts.push_back(std::make_pair(std::make_pair(caracter, text), std::make_pair(Application::getTimeClk().getElapsedTime(), sf::seconds(0.5))));
       }
       std::vector<Entity*> caracters = getWorld()->getEntities("E_MONSTER+E_HERO");
       for (unsigned int i = 0; i < caracters.size(); i++) {
            Caracter* caracter = static_cast<Caracter*>(caracters[i]);

            if (caracter->isAlive()) {
                if (caracter->isMoving()) {
                    if (dynamic_cast<Hero*>(caracter) && dynamic_cast<Hero*>(caracter)->isMovingFromKeyboard()) {
                        Vec3f actualPos = Vec3f(caracter->getCenter().x, caracter->getCenter().y, 0);
                        sf::Int64 elapsedTime = caracter->getClkTransfertTime().getElapsedTime().asMicroseconds();
                        Vec3f newPos = caracter->interpolation.first + (caracter->interpolation.second - caracter->interpolation.first) * ((float) elapsedTime / (float) (ping + timeBtwnTwoReq.asMicroseconds()));
                        Ray ray(actualPos, newPos);
                        if (getWorld()->collide(caracter, ray)) {
                            newPos = actualPos;
                        }
                        for (unsigned int j = 0; j < getRenderComponentManager().getNbComponents(); j++) {
                            if (getRenderComponentManager().getRenderComponent(j) != nullptr) {
                                View view = getRenderComponentManager().getRenderComponent(j)->getView();
                                Vec3f d = newPos - view.getPosition();
                                view.move(d.x, d.y, d.y);
                                getRenderComponentManager().getRenderComponent(j)->setView(view);
                            }
                        }
                        Vec3f d = newPos - actualPos;
                        getWorld()->moveEntity(caracter, d.x, d.y, d.y);
                        getView().move(d.x, d.y, d.y);
                    } else {
                        Vec2f actualPos = Vec2f(caracter->getCenter().x, caracter->getCenter().y);
                        sf::Int64 elapsedTime = caracter->getClkTransfertTime().getElapsedTime().asMicroseconds();

                        Vec2f newPos = Computer::getPosOnPathFromTime(caracter->interpolation.first, caracter->getPath(),elapsedTime,caracter->getSpeed());
                        /*if (caracter->getType() == "E_HERO")
                            std::cout<<"move : "<<actualPos<<caracter->getPath()[0]<<std::endl;*/
                        Vec3f d = newPos - actualPos;
                        Vec2f dir = d.normalize();
                        if (caracter->getFocusedCaracter() != nullptr && caracter->isInFightingMode() &&
                            Vec2f(caracter->getCenter().x, caracter->getCenter().y).computeDist(Vec2f(caracter->getFocusedCaracter()->getCenter().x, caracter->getFocusedCaracter()->getCenter().y)) <= caracter->getRange()) {
                            /*int delta = caracter->getRange() - Vec2f(caracter->getCenter().x, caracter->getCenter().y).computeDist(Vec2f(caracter->getFocusedCaracter()->getCenter().x, caracter->getFocusedCaracter()->getCenter().y));
                            newPos -= dir * delta;
                            d = newPos - actualPos;*/
                            caracter->setMoving(false);
                        }
                        if (caracter->isMoving() &&
                            caracter->getPath().size() > 0 &&
                            newPos.computeDist(caracter->getPath()[caracter->getPath().size() - 1]) <= PATH_ERROR_MARGIN) {
                            caracter->setMoving(false);
                            newPos = caracter->getPath()[caracter->getPath().size() - 1];
                            d = newPos - actualPos;
                        }

                        if (dir != caracter->getDir())
                            caracter->setDir(dir);
                        if (caracter->getId() == hero->getId()) {
                            for (unsigned int j = 0; j < getRenderComponentManager().getNbComponents(); j++) {
                                if (getRenderComponentManager().getRenderComponent(j) != nullptr) {
                                    View view = getRenderComponentManager().getRenderComponent(j)->getView();
                                    view.move(d.x, d.y, d.y);
                                    getRenderComponentManager().getRenderComponent(j)->setView(view);
                                }
                            }
                            getView().move(d.x, d.y, d.y);
                        }
                        getWorld()->moveEntity(caracter, d.x, d.y, d.y);
                    }
                    /*if (caracter->getType() == "E_HERO")
                        std::cout<<"caracter position : "<<caracter->getCenter()<<std::endl;*/
                }
                /*if (caracter->isInFightingMode()
                && caracter->getFocusedCaracter() != nullptr
                && Vec2f(caracter->getCenter().x, caracter->getCenter().y).computeDist(Vec2f(caracter->getFocusedCaracter()->getCenter().x,caracter->getFocusedCaracter()->getCenter().y)) <= caracter->getRange()) {
                    if (dynamic_cast<Monster*>(caracter->getFocusedCaracter())
                        && !dynamic_cast<Monster*>(caracter->getFocusedCaracter())->isInFightingMode()) {
                        caracter->getFocusedCaracter()->setFightingMode(true);
                        caracter->getFocusedCaracter()->setFocusedCaracter(caracter);
                    }
                    Vec2f dir = Vec2f(caracter->getFocusedCaracter()->getCenter().x, caracter->getFocusedCaracter()->getCenter().y) - Vec2f(caracter->getCenter().x, caracter->getCenter().y);
                    dir = dir.normalize();
                    if (caracter->getDir() != dir)
                        caracter->setDir(dir);
                    caracter->setAttacking(true);
                    if (caracter->getDamages().empty()) {
                        //std::cout<<"wait for receiving damages"<<std::endl;
                        std::vector<int> damages;
                        std::string response;
                        if (Network::getResponse("DMG"+conversionIntString(caracter->getId()), response)) {
                            std::vector<std::string> infos = split(response, "*");
                            int nb = conversionStringInt(infos[0]);
                            for (unsigned int i = 0; i < nb; i++) {
                                damages.push_back(conversionStringInt(infos[i+1]));
                            }
                            caracter->setDamages(damages);
                            sf::Int64 time = Application::getTimeClk().getElapsedTime().asMicroseconds() - conversionStringLong(infos[nb+1]);
                            if (time < 0) {
                                time = 0;
                            }
                            caracter->setDmgTransferTime(time);
                            if (conversionStringLong(infos[nb+1]) > caracter->getAttribute("life"+conversionIntString(caracter->getId())).getValue<sf::Int64>())
                                caracter->setLife(conversionStringInt(infos[nb+2]));
                        }
                    }
                    sf::Int64 time = caracter->getDmgTransferTime();
                    if (!caracter->getDamages().empty() && caracter->getTimeOfLastAttack().asSeconds() >= caracter->getAttackSpeed() - time / 1e+6f) {
                        if (time  > 0) {
                            time -= caracter->getAttackSpeed() * 1e+6f;
                            if (time < 0)
                                time = 0;
                            caracter->setDmgTransferTime(time);
                        }
                        caracter->restartAttackSpeed();
                        int dmg = caracter->getDamages().back();
                        caracter->getDamages().pop_back();
                        FontManager<Fonts>& fm = cache.resourceManager<Font, Fonts>("FontManager");
                        Text text;
                        text.setFont(*fm.getResourceByAlias(Fonts::Serif));
                        text.setString("-"+conversionIntString(dmg));
                        text.setColor(sf::Color::Red);
                        text.setCharacterSize(10);
                        text.setPosition(Vec3f(caracter->getFocusedCaracter()->getPosition().x,caracter->getFocusedCaracter()->getPosition().y-10, 0));
                        text.setSize(Vec3f(10, 10, 0));
                        tmpTexts.push_back(std::make_pair(std::make_pair(caracter->getFocusedCaracter(), text), std::make_pair(Application::getTimeClk().getElapsedTime(), sf::seconds(0.5))));
                        caracter->attackFocusedCaracter(dmg);
                        if (!caracter->getFocusedCaracter()->isAlive()) {
                            caracter->setAttacking(false);
                            caracter->setFightingMode(false);
                        }
                    }
                } else {
                    if (caracter->isAttacking())
                        caracter->setAttacking(false);
                }
                if (!caracter->isInFightingMode() && !caracter->isMoving() && !caracter->isAttacked()) {
                    //std::cout<<"rgn size : "<<caracter->getRegen().size()<<std::endl;
                    if (caracter->getRegen().empty()) {
                        //std::cout<<"wait for received regen"<<std::endl;
                        std::string response;

                        if (Network::getResponse("RGN"+conversionIntString(caracter->getId()), response)) {
                            std::vector<int> regen;
                            std::vector<std::string> infos = split(response, "*");
                            int nb = conversionStringInt(infos[0]);
                            for (unsigned int i = 0; i < nb; i++) {
                                regen.push_back(conversionStringInt(infos[i+1]));
                            }
                            caracter->setRegen(regen);
                            sf::Int64 time = Application::getTimeClk().getElapsedTime().asMicroseconds() - conversionStringLong(infos[nb+1]);
                            if (time < 0) {
                                time = 0;
                            }
                            caracter->setRgnTransferTime(time);
                            if (conversionStringLong(infos[nb+1]) > caracter->getAttribute("life"+conversionIntString(caracter->getId())).getValue<sf::Int64>())
                                caracter->setLife(conversionStringInt(infos[nb+2]));
                        }
                    }
                    sf::Int64 time = caracter->getRgnTransferTime();
                    if (!caracter->getRegen().empty() && caracter->getTimeOfLastHpRegen().asSeconds() >= caracter->getRegenHpSpeed() - time / 1e+6f) {
                        if (time > 0) {
                            time -= caracter->getRegenHpSpeed() * 1e+6f;
                            if (time < 0)
                                time = 0;
                            caracter->setRgnTransferTime(time);
                        }
                        caracter->restartRegenHP();
                        int rgn = caracter->getRegen().back();
                        caracter->getRegen().pop_back();
                        FontManager<Fonts>& fm = cache.resourceManager<Font, Fonts>("FontManager");
                        Text text;
                        text.setFont(*fm.getResourceByAlias(Fonts::Serif));
                        text.setString(conversionIntString(rgn));
                        text.setColor(sf::Color::Green);
                        text.setCharacterSize(10);
                        text.setPosition(Vec3f(caracter->getPosition().x,caracter->getPosition().y-10, 0));
                        text.setSize(Vec3f(10, 10, 0));
                        tmpTexts.push_back(std::make_pair(std::make_pair(caracter, text), std::make_pair(Application::getTimeClk().getElapsedTime(), sf::seconds(0.5))));
                        if (caracter->getLife() + rgn >= caracter->getMaxLife()) {
                            caracter->setLife(caracter->getMaxLife());
                        } else {
                            caracter->setLife(caracter->getLife() + rgn);
                        }
                    }
                    if (caracter->getManaRegen().empty()) {
                        //std::cout<<"wait for received regen"<<std::endl;
                        std::string response;

                        if (Network::getResponse("RGNMANA"+conversionIntString(caracter->getId()), response)) {
                            std::vector<int> regen;
                            std::vector<std::string> infos = split(response, "*");
                            int nb = conversionStringInt(infos[0]);
                            for (unsigned int i = 0; i < nb; i++) {
                                regen.push_back(conversionStringInt(infos[i+1]));
                            }
                            caracter->setManaRegen(regen);
                            sf::Int64 time = Application::getTimeClk().getElapsedTime().asMicroseconds() - conversionStringLong(infos[nb+1]);
                            if (time < 0) {
                                time = 0;
                            }
                            caracter->setRgnManaTransferTime(time);
                            if (conversionStringLong(infos[nb+1]) > caracter->getAttribute("mana"+conversionIntString(caracter->getId())).getValue<sf::Int64>())
                                caracter->setMana(conversionStringInt(infos[nb+2]));
                        }
                    }
                    sf::Int64 time2 = caracter->getRgnManaTransferTime();
                    if (!caracter->getManaRegen().empty() && caracter->getTimeOfLastManaRegen().asSeconds() >= caracter->getRegenManaSpeed() - time2 / 1e+6f) {
                        if (time2 > 0) {
                            time2 -= caracter->getRegenManaSpeed() * 1e+6f;
                            if (time2 < 0)
                                time2 = 0;
                            caracter->setRgnManaTransferTime(time2);
                        }
                        caracter->restartRegenMana();

                        int rgn = caracter->getManaRegen().back();
                        caracter->getManaRegen().pop_back();
                        FontManager<Fonts>& fm = cache.resourceManager<Font, Fonts>("FontManager");
                        Text text;
                        text.setFont(*fm.getResourceByAlias(Fonts::Serif));
                        text.setString(conversionIntString(rgn));
                        text.setColor(sf::Color::Blue);
                        text.setCharacterSize(10);
                        text.setPosition(Vec3f(caracter->getPosition().x,caracter->getPosition().y-10, 0));
                        text.setSize(Vec3f(10, 10, 0));
                        tmpTexts.push_back(std::make_pair(std::make_pair(caracter, text), std::make_pair(Application::getTimeClk().getElapsedTime(), sf::seconds(0.5))));
                        if (caracter->getMana() + rgn >= caracter->getManaMax()) {
                            caracter->setMana(caracter->getManaMax());
                        } else {
                            caracter->setMana(caracter->getMana() + rgn);
                        }
                    }
                }*/
            } else {
                if (caracter == hero) {
                    setEventContextActivated(false);
                    wResuHero->setVisible(true);
                    int time = Math::roundToInt(hero->getTimeBeforeLastRespawn().asSeconds() - hero->getTimeSinceLastRespawn().asSeconds());
                    if (time < 0) {
                        time = 0;
                    }
                    label->setText(conversionIntString(time));
                    if (time <= 0) {
                        button->setEventContextActivated(true);
                    }
                }
            }
        }
        while(!gameActions.empty()) {
            std::cout<<"game action!"<<std::endl;
            GameAction ga;
            apply_visitor(ga, gameActions.back().first, gameActions.back().second);
            gameActions.pop_back();
        }
        std::map<ParticleSystem*, std::pair<sf::Time, sf::Time>>::iterator it;
        for (it = particles.begin(); it != particles.end(); ) {
            sf::Time elapsedTime = Application::getTimeClk().getElapsedTime() - it->second.first;
            it->second.first = Application::getTimeClk().getElapsedTime();
            it->second.second -= elapsedTime;
            if (it->second.second <= sf::seconds(0)) {
                psu->removeParticleSystem(it->first);
                getWorld()->deleteEntity(it->first);
                it = particles.erase(it);
                //std::cout<<"it erased"<<std::endl;
            } else
                it++;
        }
        /*ps->update(getClock("LoopTime").getElapsedTime());
        for (it = particles.begin(); it != particles.end(); it++) {
            it->first->update(getClock("LoopTime").getElapsedTime());
        }
        for (unsigned int i=0; i < particles2.size(); i++) {
            particles2[i]->update(getClock("LoopTime").getElapsedTime());
        }*/
        getWorld()->update();
        fps++;
        if (getClock("FPS").getElapsedTime().asSeconds() >= 1.f) {
            //std::cout<<"fps : "<<fps<<std::endl;
            getClock("FPS").restart();
            fps = 0;
        }
    }
    void MyAppli::actionPerformed(gui::Button* item) {
        //std::cout<<"text : "<<item->getText()<<std::endl;
        if (item->getText() == "Accept") {
            selectedQuest->setStatus(Quest::IN_PROGRESS);
            if (!static_cast<Hero*>(hero)->containsQuest(selectedQuest->getName())) {
                static_cast<Hero*>(hero)->addQuest(*selectedQuest);
            }
            wDisplayQuest->setVisible(false);
            std::string request = "ACCEPT*"+conversionIntString(selectedPnj->getId())+"*"+selectedQuest->getName()+"*"+conversionIntString(hero->getId());
            SymEncPacket packet;
            packet<<request;
            Network::sendTcpPacket(packet);
            getRenderComponentManager().setEventContextActivated(false, *wDisplayQuest);
            setEventContextActivated(true);
            getRenderComponentManager().setEventContextActivated(true, getRenderWindow());
        }
        if (item->getText() == "Give up") {
            if (static_cast<Hero*>(hero)->containsQuest(selectedQuest->getName())) {
                static_cast<Hero*>(hero)->removeQuest(*selectedQuest);
            }
            wDisplayQuest->setVisible(false);
            getRenderComponentManager().setEventContextActivated(false, *wDisplayQuest);
            setEventContextActivated(true);
            getRenderComponentManager().setEventContextActivated(true, getRenderWindow());
        }
        if (item->getText() == "Continue") {
            wDisplayQuest->setVisible(false);
            getRenderComponentManager().setEventContextActivated(false, *wDisplayQuest);
            setEventContextActivated(true);
            getRenderComponentManager().setEventContextActivated(true, getRenderWindow());
        }
        if (item->getText() == "Get rewards") {
            static_cast<Hero*>(hero)->up(selectedQuest->getXp());
            std::map<unsigned int, Item> rewards = selectedQuest->getRewards();
            std::map<unsigned int, Item>::iterator it;
            for (it = rewards.begin(); it != rewards.end(); it++) {
                for (unsigned int i = 0; i < it->first; i++) {
                    static_cast<Hero*>(hero)->addItem(it->second);
                }
            }
            if (selectedQuest->isRepeat()) {
                if (static_cast<Hero*>(hero)->containsQuest(selectedQuest->getName())) {
                    static_cast<Hero*>(hero)->removeQuest(*selectedQuest);
                }
            } else {
                selectedQuest->setStatus(Quest::FINISHED);
            }
            wDisplayQuest->setVisible(false);
            getRenderComponentManager().setEventContextActivated(false, *wDisplayQuest);
            setEventContextActivated(true);
            getRenderComponentManager().setEventContextActivated(true, getRenderWindow());
        }
        if (item->getText() == "Respawn") {
            SymEncPacket packet;
            std::string message = "ALIVE*"+conversionIntString(hero->getId());
            packet<<message;
            Network::sendTcpPacket(packet);
            getRenderComponentManager().setEventContextActivated(false, *wResuHero);
            setEventContextActivated(true);
            getRenderComponentManager().setEventContextActivated(true, getRenderWindow());
        }
        if (item->getText() == "Invite") {
            std::cout<<"client authentified"<<std::endl;
            isClientAuthentified = true;
            wIdentification->setVisible(false);
            idButton->setEventContextActivated(false);
            invButton->setEventContextActivated(false);
            setEventContextActivated(true);
            SymEncPacket packet;
            packet<<"INV";
            Network::sendTcpPacket(packet);
        }
        if (item->getText() == "Connect") {
            std::cout<<"try to connect"<<std::endl;
            std::string pseudo = labPseudo->getText();
            std::string pswd = labMdp->getText();
            SymEncPacket packet;
            packet<<"CONNECT*"+pseudo+"*"+pswd;
            Network::sendTcpPacket(packet);
        }
    }
    MyAppli::~MyAppli() {

    }
}
