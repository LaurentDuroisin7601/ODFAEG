#include "application.h"
#include "odfaeg/Math/distributions.h"
#include "odfaeg/Graphics/GUI/menubar.hpp"

using namespace odfaeg;
using namespace odfaeg::graphic;
using namespace odfaeg::math;
using namespace odfaeg::physic;
using namespace odfaeg::core;
using namespace odfaeg::audio;
using namespace odfaeg::window;
namespace sorrok {
    MyAppli::MyAppli(sf::VideoMode wm, std::string title) : Application (wm, title, sf::Style::Default, ContextSettings(0, 0, 4, 4, 6)) {
        std::cout<<"construct"<<std::endl;
        running = false;
        actualKey = IKeyboard::Key::Unknown;
        previousKey = IKeyboard::Key::Unknown;
        getView().move(0, 400, 400);
        fpsCounter = 0;
        addClock(sf::Clock(), "FPS");
        day = false;
        sf::Listener::setUpVector(0.f, 0.f, 1.f);
        ps = entityFactory.make_entity<ParticleSystem>(Vec3f(0, 0, 150),Vec3f(100, 100, 0), entityFactory);
        Tile* tile;
        //FastDelegate<void> fd(&Allocator<Entity>::allocate<Tile, EntityFactory&>,tile, std::ref(entityFactory));
        EXPORT_CLASS_GUID(BoundingVolumeBoundingBox, BoundingVolume, BoundingBox)
        EXPORT_CLASS_GUID_(EntityTile, Entity, Tile, VA_LIST(EntityFactory&), VA_LIST(std::ref(entityFactory)))
        EXPORT_CLASS_GUID_(EntityTile, Entity, BigTile, VA_LIST(EntityFactory&), VA_LIST(std::ref(entityFactory)))
        EXPORT_CLASS_GUID_(EntityWall, Entity, g2d::Wall, VA_LIST(EntityFactory&), VA_LIST(std::ref(entityFactory)))
        EXPORT_CLASS_GUID_(EntityDecor, Entity, g2d::Decor, VA_LIST(EntityFactory&), VA_LIST(std::ref(entityFactory)))
        EXPORT_CLASS_GUID_(EntityAnimation, Entity, Anim, VA_LIST(EntityFactory&), VA_LIST(std::ref(entityFactory)))
        EXPORT_CLASS_GUID_(EntityHero, Entity, Hero, VA_LIST(EntityFactory&), VA_LIST(std::ref(entityFactory)))
        EXPORT_CLASS_GUID_(EntityMesh, Entity, Mesh, VA_LIST(EntityFactory&), VA_LIST(std::ref(entityFactory)))

    }
    void MyAppli::gaignedFocus(gui::TextArea* textArea) {
        std::cout<<"gaigned focus"<<std::endl;
    }
    void MyAppli::keyHeldDown (IKeyboard::Key key) {
        //BoundingRectangle rect (pos.x, pos.y, getView().getSize().x, getView().getSize().y);
        if (actualKey != IKeyboard::Key::Unknown && key == IKeyboard::Key::Z) {
            if (!caracter->isMoving()) {
                if (actualKey != previousKey) {
                    Vec2f dir(0, -1);
                    caracter->setDir(dir);
                    sf::Listener::setDirection(dir.x, dir.y, 0);
                }
                caracter->setMoving(true);
                caracter->setIsMovingFromKeyboard(true);
            }
        } else if (actualKey != IKeyboard::Key::Unknown && key == IKeyboard::Key::Q) {
            if (!caracter->isMoving()) {
                if (actualKey != previousKey) {
                    Vec2f dir(-1, 0);
                    caracter->setDir(dir);
                    sf::Listener::setDirection(dir.x, dir.y, 0);
                }
                caracter->setMoving(true);
                caracter->setIsMovingFromKeyboard(true);
            }
        } else if (actualKey != IKeyboard::Key::Unknown && actualKey == IKeyboard::Key::S) {
            if (!caracter->isMoving()) {
                if (actualKey != previousKey) {
                    Vec2f dir(0, 1);
                    caracter->setDir(dir);
                    sf::Listener::setDirection(dir.x, dir.y, 0);
                }
                caracter->setMoving(true);
                caracter->setIsMovingFromKeyboard(true);
            }
        } else if (actualKey != IKeyboard::Key::Unknown && key == IKeyboard::Key::D) {
            if (!caracter->isMoving()) {
                if (actualKey != previousKey) {
                    Vec2f dir(1, 0);
                    caracter->setDir(dir);
                    sf::Listener::setDirection(dir.x, dir.y, 0);
                }
                caracter->setIsMovingFromKeyboard(true);
                caracter->setMoving(true);
            }
        }
    }
    void MyAppli::leftMouseButtonPressed(sf::Vector2f mousePos) {
        Vec2f mouse(mousePos.x, mousePos.y);
        Vec3f finalPos = getRenderWindow().mapPixelToCoords(Vec3f(mouse.x, getRenderWindow().getSize().y-mouse.y, 0));
        //std::cout<<mouse.x<<" "<<mouse.y<<std::endl;
        point.clear();
        point.append(Vertex(sf::Vector3f(finalPos.x, finalPos.y, finalPos.y), sf::Color(255, 0, 0)));
        std::vector<Vec2f> path = getWorld()->getPath(caracter, finalPos);
        if (path.size() > 0) {
            caracter->setPath(path);
            caracter->setIsMovingFromKeyboard(false);
            caracter->setMoving(true);
        }
    }
    bool MyAppli::mouseInside (sf::Vector2f mousePos) {
        BoundingBox bx (0, 0, 0, 100, 100, 0);
        if (bx.isPointInside(Vec3f(mousePos.x, mousePos.y, 0))) {
            return true;
        }
        return false;
    }
    void MyAppli::onMouseInside (sf::Vector2f mousePos) {
        std::cout<<"Mouse inside : "<<mousePos.x<<" "<<mousePos.y<<std::endl;
    }
    void MyAppli::onLoad() {

        TextureManager<> tm;
        tm.fromFileWithAlias("tilesets/eau.png", "WATER");
        tm.fromFileWithAlias("tilesets/herbe.png", "GRASS");
        tm.fromFileWithAlias("tilesets/murs.png", "WALLS");
        tm.fromFileWithAlias("tilesets/maison.png", "HOUSE");
        tm.fromFileWithAlias("tilesets/flemmes1.png", "FIRE1");
        tm.fromFileWithAlias("tilesets/flemmes2.png", "FIRE2");
        tm.fromFileWithAlias("tilesets/flemmes3.png", "FIRE3");
        tm.fromFileWithAlias("tilesets/particule.png", "PARTICLE");
        std::string path = "tilesets/vlad_sword.png";
        tm.fromFileWithAlias(path, "VLADSWORD");
        SoundBufferManager<> sm;
        sm.fromFileWithAlias("sounds/walk_crop.wav", "step foot");
        sm.fromFileWithAlias("sounds/fire.wav", "fire");
        Stream* stream = new Stream();
        stream->load(*sm.getResourceByAlias("step foot"));
        Stream* stream2 = new Stream();
        stream2->load(*sm.getResourceByAlias("fire"));
        player.setAudioStream(stream);
        pfire.setAudioStream(stream2);
        cache.addResourceManager(tm, "TextureManager");
        cache.addResourceManager(sm, "SoundManager");
        FontManager<> fm;
        fm.fromFileWithAlias("fonts/FreeSerif.ttf", "FreeSerif");
        cache.addResourceManager(fm, "FontManager");
    }
    void MyAppli::onInit () {
        std::cout<<"init"<<std::endl;
        if (day)
            g2d::AmbientLight::getAmbientLight().setColor(sf::Color::White);
        TextureManager<> &tm = cache.resourceManager<Texture, std::string>("TextureManager");
        FontManager<> &fm = cache.resourceManager<Font, std::string>("FontManager");
        Vec2f pos (getView().getPosition().x - getView().getSize().x * 0.5f, getView().getPosition().y - getView().getSize().y * 0.5f);
        BoundingBox bx (pos.x, pos.y, 0, getView().getSize().x, getView().getSize().y, 0);
        theMap = new Scene(&getRenderComponentManager(), "Map test", 100, 50, 0);
        BaseChangementMatrix bm;
        bm.set2DIsoMatrix();
        theMap->setBaseChangementMatrix(bm);
        getWorld()->addSceneManager(theMap);
        getWorld()->setCurrentSceneManager("Map test");
        eu = new EntitiesUpdater(entityFactory, *getWorld());
        eu->setName("EntitiesUpdater");
        getWorld()->addWorker(eu);
        au = new AnimUpdater();
        au->setInterval(sf::seconds(0.01f));
        getWorld()->addTimer(au);
        psu = new ParticleSystemUpdater();
        getWorld()->addWorker(psu);
        tiles.push_back(entityFactory.make_entity<Tile>(tm.getResourceByAlias("WATER"), Vec3f(0, 0, 0), Vec3f(120, 60, 0),sf::IntRect(0, 0, 100, 50), entityFactory));
        walls.resize(g2d::Wall::NB_WALL_TYPES, nullptr);
        //tiles.push_back(new Tile(tm.getResourceByAlias("GRASS"), Vec3f(0, 0, 0), Vec3f(120, 60, 0),sf::IntRect(0, 0, 100, 50)));
        walls[g2d::Wall::TOP_BOTTOM] = entityFactory.make_entity<g2d::Wall>(entityFactory.make_entity<Tile>(tm.getResourceByAlias("WALLS"), Vec3f(0, 0, 0), Vec3f(100, 100, 0), sf::IntRect(100, 0, 100, 100), entityFactory),g2d::Wall::TOP_BOTTOM,&g2d::AmbientLight::getAmbientLight(), entityFactory);
        //std::cout<<"global bounds : "<<walls[g2d::Wall::TOP_BOTTOM]->getGlobalBounds().getPosition()<<std::endl;
        walls[g2d::Wall::RIGHT_LEFT] = entityFactory.make_entity<g2d::Wall>(entityFactory.make_entity<Tile>(tm.getResourceByAlias("WALLS"), Vec3f(0, 0, 0), Vec3f(100, 100, 0), sf::IntRect(100, 100, 100, 100), entityFactory),g2d::Wall::RIGHT_LEFT,&g2d::AmbientLight::getAmbientLight(), entityFactory);
        walls[g2d::Wall::BOTTOM_RIGHT] = entityFactory.make_entity<g2d::Wall>(entityFactory.make_entity<Tile>(tm.getResourceByAlias("WALLS"), Vec3f(0, 0, 0), Vec3f(100, 100, 0), sf::IntRect(100, 200, 100, 100), entityFactory),g2d::Wall::BOTTOM_RIGHT,&g2d::AmbientLight::getAmbientLight(), entityFactory);
        walls[g2d::Wall::TOP_LEFT] = entityFactory.make_entity<g2d::Wall>(entityFactory.make_entity<Tile>(tm.getResourceByAlias("WALLS"), Vec3f(0, 0, 0), Vec3f(100, 100, 0), sf::IntRect(100, 300, 100, 100), entityFactory),g2d::Wall::TOP_LEFT,&g2d::AmbientLight::getAmbientLight(), entityFactory);
        walls[g2d::Wall::TOP_RIGHT] = entityFactory.make_entity<g2d::Wall>(entityFactory.make_entity<Tile>(tm.getResourceByAlias("WALLS"), Vec3f(0, 0, 0), Vec3f(100, 100, 0), sf::IntRect(100, 400, 100, 100), entityFactory),g2d::Wall::TOP_RIGHT,&g2d::AmbientLight::getAmbientLight(), entityFactory);
        walls[g2d::Wall::BOTTOM_LEFT] = entityFactory.make_entity<g2d::Wall> (entityFactory.make_entity<Tile>(tm.getResourceByAlias("WALLS"), Vec3f(0, 0, 0), Vec3f(100, 100, 0), sf::IntRect(100, 500, 100, 100), entityFactory), g2d::Wall::BOTTOM_LEFT,&g2d::AmbientLight::getAmbientLight(), entityFactory);
        tiles[0]->getFace(0)->getMaterial().setTexId("WATER");
        tiles[0]->getFace(0)->getMaterial().setReflectable(true);
        tiles[0]->getFace(0)->getMaterial().setType(Material::WATER);
        tiles[0]->setDrawMode(Entity::INSTANCED);
        walls[g2d::Wall::TOP_BOTTOM]->getChildren()[0]->getFace(0)->getMaterial().setTexId("WALLS");
        walls[g2d::Wall::RIGHT_LEFT]->getChildren()[0]->getFace(0)->getMaterial().setTexId("WALLS");
        walls[g2d::Wall::BOTTOM_LEFT]->getChildren()[0]->getFace(0)->getMaterial().setTexId("WALLS");
        walls[g2d::Wall::TOP_RIGHT]->getChildren()[0]->getFace(0)->getMaterial().setTexId("WALLS");
        walls[g2d::Wall::TOP_LEFT]->getChildren()[0]->getFace(0)->getMaterial().setTexId("WALLS");
        walls[g2d::Wall::BOTTOM_RIGHT]->getChildren()[0]->getFace(0)->getMaterial().setTexId("WALLS");
        walls[g2d::Wall::TOP_BOTTOM]->getChildren()[0]->setLayer(1);
        walls[g2d::Wall::RIGHT_LEFT]->getChildren()[0]->setLayer(1);
        walls[g2d::Wall::BOTTOM_LEFT]->getChildren()[0]->setLayer(1);
        walls[g2d::Wall::TOP_RIGHT]->getChildren()[0]->setLayer(1);
        walls[g2d::Wall::TOP_LEFT]->getChildren()[0]->setLayer(1);
        walls[g2d::Wall::BOTTOM_RIGHT]->getChildren()[0]->setLayer(1);
        std::ifstream ifs("FichierDeSerialisation");
        if(ifs) {
            std::cout<<"read serialisation file"<<std::endl;
            ITextArchive ia(ifs);
            std::vector<Entity*> entities;
            ia(entities);
            for (unsigned int i = 0; i < entities.size(); i++) {
                getWorld()->addEntity(entities[i]);
                if (entities[i]->getType() == "E_BIGTILE") {
                    for (unsigned int j = 0; j < entities[i]->getChildren().size(); j++) {
                        std::string texId =  entities[i]->getChildren()[j]->getFace(0)->getMaterial().getTexId();

                        sf::IntRect texRect = entities[i]->getChildren()[j]->getFace(0)->getMaterial().getTexRect();
                        entities[i]->getChildren()[j]->getFace(0)->getMaterial().clearTextures();
                        entities[i]->getChildren()[j]->getFace(0)->getMaterial().addTexture(tm.getResourceByAlias(texId), texRect);
                        entities[i]->getChildren()[j]->getFace(0)->getMaterial().setTexId(texId);
                    }

                } else if (entities[i]->getType() == "E_WALL") {
                    std::string texId =  entities[i]->getChildren()[0]->getFace(0)->getMaterial().getTexId();
                    std::cout<<static_cast<g2d::Wall*>(entities[i])->getType()<<"texId : "<<texId<<std::endl;
                    sf::IntRect texRect = entities[i]->getChildren()[0]->getFace(0)->getMaterial().getTexRect();
                    entities[i]->getChildren()[0]->getFace(0)->getMaterial().clearTextures();
                    entities[i]->getChildren()[0]->getFace(0)->getMaterial().addTexture(tm.getResourceByAlias(texId), texRect);
                    entities[i]->getChildren()[0]->getFace(0)->getMaterial().setTexId(texId);
                    getWorld()->getGridCellAt(Vec3f(entities[i]->getCenter().x, entities[i]->getCenter().y, 0))->setPassable(false);
                } else if (entities[i]->getType() == "E_DECOR") {
                    std::string texId =  entities[i]->getChildren()[0]->getFace(0)->getMaterial().getTexId();
                    sf::IntRect texRect = entities[i]->getChildren()[0]->getFace(0)->getMaterial().getTexRect();
                    entities[i]->getChildren()[0]->getFace(0)->getMaterial().clearTextures();
                    entities[i]->getChildren()[0]->getFace(0)->getMaterial().addTexture(tm.getResourceByAlias(texId), texRect);
                    entities[i]->getChildren()[0]->getFace(0)->getMaterial().setTexId(texId);
                } else if (entities[i]->getType() == "E_ANIMATION") {
                    Anim* anim = static_cast<Anim*> (entities[i]);
                    for (unsigned int j = 0; j < anim->getChildren().size(); j++) {
                        std::string texId = entities[i]->getChildren()[j]->getChildren()[0]->getFace(0)->getMaterial().getTexId();
                        sf::IntRect texRect = entities[i]->getChildren()[j]->getChildren()[0]->getFace(0)->getMaterial().getTexRect();
                        entities[i]->getChildren()[j]->getChildren()[0]->getFace(0)->getMaterial().clearTextures();
                        entities[i]->getChildren()[j]->getChildren()[0]->getFace(0)->getMaterial().addTexture(tm.getResourceByAlias(texId), texRect);
                        entities[i]->getChildren()[j]->getChildren()[0]->getFace(0)->getMaterial().setTexId(texId);
                    }
                    anim->play(true);
                    au->addAnim(anim);
                }
            }
            ifs.close();
        } else {
            std::cout<<"not read serialisation file"<<std::endl;
            BoundingBox mapZone(0, 0, 0, 1500, 1000, 0);
            getWorld()->generate_map(tiles, walls, Vec2f(100, 50), mapZone, false, entityFactory);
            Tile* thouse = entityFactory.make_entity<Tile>(tm.getResourceByAlias("HOUSE"), Vec3f(-100, 250, 400), Vec3f(250, 300, 0), sf::IntRect(0, 0, 250, 300), entityFactory);
            thouse->setLayer(1);
            thouse->getFace(0)->getMaterial().setTexId("HOUSE");
            g2d::Decor* decor = entityFactory.make_entity<g2d::Decor>(thouse, &g2d::AmbientLight::getAmbientLight(), entityFactory);

            BoundingVolume *bb = new BoundingBox(decor->getGlobalBounds().getPosition().x, decor->getGlobalBounds().getPosition().y + decor->getGlobalBounds().getSize().y * 0.4f, 0,
            decor->getGlobalBounds().getSize().x, decor->getGlobalBounds().getSize().y * 0.25f, 0);
            //std::cout<<bb->getPosition()<<" "<<bb->getSize()<<std::endl;
            decor->setCollisionVolume(bb);
            decor->setShadowCenter(Vec3f(0, 400, -400));
            thouse->getFace(0)->getMaterial().setSpecularPower(10);
            thouse->getFace(0)->getMaterial().setSpecularIntensity(100);
            getWorld()->addEntity(decor);
            Anim* fire = entityFactory.make_entity<Anim>(0.1f, Vec3f(0, 0, 0), Vec3f(100, 100, 0), entityFactory);
            Tile* tf1 = entityFactory.make_entity<Tile>(tm.getResourceByAlias("FIRE1"), Vec3f(0, 100, 150), Vec3f(100, 100, 0), sf::IntRect(0, 0, 150, 200), entityFactory);
            tf1->setLayer(1);
            tf1->getFace(0)->getMaterial().setTexId("FIRE1");
            g2d::Decor *fire1 = entityFactory.make_entity<g2d::Decor>(tf1, &g2d::AmbientLight::getAmbientLight(), entityFactory);
            fire1->setShadowCenter(Vec3f(0, 200, 0));
            //decor->changeGravityCenter(Vec3f(50, 50, 0));
            Tile* tf2 = entityFactory.make_entity<Tile>(tm.getResourceByAlias("FIRE2"), Vec3f(0, 100, 150), Vec3f(100, 100, 0), sf::IntRect(0, 0, 150, 200), entityFactory);
            tf2->setLayer(1);
            tf2->getFace(0)->getMaterial().setTexId("FIRE2");
            g2d::Decor *fire2 = entityFactory.make_entity<g2d::Decor>(tf2, &g2d::AmbientLight::getAmbientLight(), entityFactory);
            fire2->setShadowCenter(Vec3f(0, 200, 0));
            //decor->changeGravityCenter(Vec3f(50, 50, 0));
            Tile* tf3 = entityFactory.make_entity<Tile>(tm.getResourceByAlias("FIRE3"), Vec3f(0, 100, 150), Vec3f(100, 100, 0), sf::IntRect(0, 0, 150, 200), entityFactory);
            tf3->setLayer(1);
            tf3->getFace(0)->getMaterial().setTexId("FIRE3");
            g2d::Decor *fire3 = entityFactory.make_entity<g2d::Decor>(tf3, &g2d::AmbientLight::getAmbientLight(), entityFactory);
            fire3->setShadowCenter(Vec3f(0, 200, 0));
            //decor->changeGravityCenter(Vec3f(50, 50, 0));
            //fire1->setShadowCenter(Vec2f(80, 100));
            //fire2->setShadowCenter(Vec2f(80, 100));
            //fire3->setShadowCenter(Vec2f(80, 100));
            fire->addFrame(fire1);
            fire->addFrame(fire2);
            fire->addFrame(fire3);
            fire->play(true);
            fire->setShadowScale(Vec3f(1, -1, 1));
            fire->setShadowCenter(Vec3f(0, 350, -150));

            getWorld()->addEntity(fire);
            au->addAnim(fire);
            w = entityFactory.make_entity<g2d::Wall>(entityFactory.make_entity<Tile>(tm.getResourceByAlias("WALLS"), Vec3f(0, 0, 0), Vec3f(100, 100, 0), sf::IntRect(100, 200, 100, 100), entityFactory),g2d::Wall::TOP_LEFT, &g2d::AmbientLight::getAmbientLight(), entityFactory);
            w->getChildren()[0]->getFace(0)->getMaterial().setTexId("WALLS");
            w->setPosition(Vec3f(0, 130, 130 + w->getSize().y * 0.5f));
            w->setLayer(1);
            getWorld()->addEntity(w);
        }
        ps->setTexture(*tm.getResourceByAlias("PARTICLE"));
        for (unsigned int i = 0; i < 10; i++) {
            ps->addTextureRect(sf::IntRect(i*10, 0, 10, 10));
        }
        emitter.setEmissionRate(30);
        emitter.setParticleLifetime(Distributions::uniform(sf::seconds(5), sf::seconds(7)));
        emitter.setParticlePosition(Distributions::rect(Vec3f(50, 90, 0), Vec3f(25, 5, 0)));   // Emit particles in given circle
        emitter.setParticleVelocity(Distributions::deflect(Vec3f(0, -10, 0),  0)); // Emit towards direction with deviation of 15°
        emitter.setParticleRotation(Distributions::uniform(0.f, 0.f));
        emitter.setParticleTextureIndex(Distributions::uniformui(0, 9));
        emitter.setParticleScale(Distributions::rect(Vec3f(2.1f, 2.1f, 1.f), Vec3f(2.f, 2.f, 1.f)));
        //emitter.setParticleColor(Distributions::color(Vec3f(0, 0, 0, 255), Vec3f(0, 0, 0, 255)));
       /* sf::Vector3f acceleration(0, -1, 0);
        ForceAffector affector(acceleration);
        billboard->getParticleSystem().addAffector(affector);*/
        ps->addEmitter(refEmitter(emitter));
        ps->setLayer(1);
        psu->addParticleSystem(ps);
        psu->setName("ParticlesSystemUpdater");
        getWorld()->addEntity(ps);
        View view = getView();
        //view.rotate(0, 0, 20);
        PerPixelLinkedListRenderComponent *frc1 = new PerPixelLinkedListRenderComponent(getRenderWindow(),0, "E_BIGTILE", ContextSettings(0, 0, 4, 4, 6));
        PerPixelLinkedListRenderComponent *frc2 = new PerPixelLinkedListRenderComponent(getRenderWindow(), 1, "E_WALL+E_DECOR+E_ANIMATION+E_HERO+E_PARTICLES", ContextSettings(0, 0, 4, 4, 6));

        ReflectRefractRenderComponent *rrrc = new ReflectRefractRenderComponent(getRenderWindow(), 2, "E_BIGTILE+E_WALL+E_DECOR+E_ANIMATION+E_HERO", ContextSettings(0, 0, 4, 4, 6));
        ShadowRenderComponent *src = new ShadowRenderComponent(getRenderWindow(), 3, "E_WALL+E_DECOR+E_ANIMATION+E_HERO", ContextSettings(0, 0, 4, 4, 6));
        LightRenderComponent *lrc = new LightRenderComponent(getRenderWindow(), 4, "E_WALL+E_DECOR+E_ANIMATION+E_HERO+E_PONCTUAL_LIGHT", ContextSettings(0, 0, 4, 4, 6));
        //std::cout<<"component created"<<std::endl;
        /*frc1->setVisible(false);
        frc2->setVisible(false);
        rrrc->setVisible(false);
        src->setVisible(false);
        lrc->setVisible(false);*/
        /*gui::TextArea* textArea = new gui::TextArea(Vec3f(350, 275, 0),Vec3f(100, 50, 0),fm.getResourceByAlias("FreeSerif"), "Test",getRenderWindow());
        textArea->addFocusListener(this);
        textArea->setVisible(false);
        textArea->setEventContextActivated(false);*/
        /*op = new gui::OptionPane(Vec2f(200, 175), Vec2f(400, 200), fm.getResourceByAlias("FreeSerif"), "Test",gui::OptionPane::TYPE::CONFIRMATION_DIALOG);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        op->setVisible(false);
        op->setEventContextActivated(false);*/
        /*frc1->setVisible(false);
        frc2->setVisible(false);
        rrrc->setVisible(false);
        src->setVisible(false);
        lrc->setVisible(false);*/
        getRenderComponentManager().addComponent(frc1);
        getRenderComponentManager().addComponent(frc2);
        getRenderComponentManager().addComponent(rrrc);
        getRenderComponentManager().addComponent(src);
        getRenderComponentManager().addComponent(lrc);
        /*getRenderComponentManager().addComponent(textArea);
        getRenderComponentManager().addComponent(op);*/

        caracter = entityFactory.make_entity<Hero>("Sorrok", "Nagi", "M", "Map test", "Brain", "Green", "White","Normal","Novice", 1, entityFactory);

        const Texture *text = cache.resourceManager<Texture, std::string>("TextureManager").getResourceByAlias("VLADSWORD");
        int textRectX = 0, textRectY = 0, textRectWidth = 50, textRectHeight = 100;
        int textWidth = text->getSize().x;
        caracter->setCenter(math::Vec3f(-25, -50, 0));
        for (unsigned int i = 0; i < 8; i++) {
            Anim* animation = entityFactory.make_entity<Anim>(0.1f, Vec3f(-25, -50, 0), Vec3f(50, 100, 0), entityFactory);
            for (unsigned int j = 0; j < 8; j++) {
                sf::IntRect textRect (textRectX, textRectY, textRectWidth, textRectHeight);
                Tile *tile = entityFactory.make_entity<Tile>(text, Vec3f(-25, -50, 0), Vec3f(textRectWidth, textRectHeight, 0), textRect, entityFactory);
                tile->setLayer(1);
                tile->getFace(0)->getMaterial().setTexId("VLADSWORD");
                g2d::Decor *frame = entityFactory.make_entity<g2d::Decor>(tile, &g2d::AmbientLight::getAmbientLight(), entityFactory);
                frame->setShadowCenter(Vec3f(0, 200, 200));
                textRectX += textRectWidth;
                if (textRectX >= textWidth) {
                    textRectX = 0;
                    textRectY += textRectHeight;
                }
                animation->addFrame(frame);
            }
            animation->getCurrentFrame()->setBoneIndex(i);
            caracter->addAnimation(animation);
            au->addAnim(animation);
        }
        BoundingVolume* bb2 = new BoundingBox(caracter->getGlobalBounds().getPosition().x, caracter->getGlobalBounds().getPosition().y + caracter->getGlobalBounds().getSize().y * 0.4f, 0,
        caracter->getGlobalBounds().getSize().x, caracter->getGlobalBounds().getSize().y * 0.25f, 0);
        caracter->setCollisionVolume(bb2);
        caracter->setCenter(Vec3f(getView().getPosition().x, getView().getPosition().y, getView().getPosition().y));
        caracter->setShadowScale(Vec3f(1, -1, 1));
        caracter->setShadowCenter(Vec3f(0, 280, -140));
        caracter->setSelected(true);
        //std::cout<<bb2->getPosition()<<" "<<bb2->getSize()<<std::endl;
        g2d::PonctualLight* light1 = entityFactory.make_entity<g2d::PonctualLight>(Vec3f(-50, 420, 420), 100, 50, 0, 255, sf::Color::Yellow, 16, entityFactory);
        light1->setLayer(1);
        g2d::PonctualLight* light2 = entityFactory.make_entity<g2d::PonctualLight>(Vec3f(50, 160, 160), 100, 50, 0, 255, sf::Color::Yellow, 16, entityFactory);
        light2->setLayer(1);
        getWorld()->addEntity(light1);
        getWorld()->addEntity(light2);
        //getView().move(d.x * 0.5f, d.y * 0.5f, 0);
        getWorld()->addEntity(caracter);

        //World::computeIntersectionsWithWalls();
        getWorld()->update();
        Action a1 (Action::EVENT_TYPE::KEY_PRESSED_ONCE, IKeyboard::Key::Z);
        Action a2 (Action::EVENT_TYPE::KEY_PRESSED_ONCE, IKeyboard::Key::Q);
        Action a3 (Action::EVENT_TYPE::KEY_PRESSED_ONCE, IKeyboard::Key::S);
        Action a4 (Action::EVENT_TYPE::KEY_PRESSED_ONCE, IKeyboard::Key::D);
        Action a5 (Action::EVENT_TYPE::MOUSE_BUTTON_PRESSED_ONCE, IMouse::Left);
        Action combined  = a1 || a2 || a3 || a4;
        Command moveAction(combined, FastDelegate<void>(&MyAppli::keyHeldDown, this, IKeyboard::Unknown));
        getListener().connect("MoveAction", moveAction);
        if (!day)
            g2d::AmbientLight::getAmbientLight().setColor(sf::Color(0, 0, 255));
        Command mouseInsideAction(a5, FastDelegate<bool>(&MyAppli::mouseInside,this, sf::Vector2f(-1, -1)), FastDelegate<void>(&MyAppli::onMouseInside, this, sf::Vector2f(-1,-1)));
        getListener().connect("MouseInside",mouseInsideAction);
        Command leftMouseButtonPressedAction (a5, FastDelegate<void>(&MyAppli::leftMouseButtonPressed, this, sf::Vector2f(-1, -1)));
        getListener().connect("LeftMouseButtonPressedAction", leftMouseButtonPressedAction);
        player.setRelativeToListener(true);
        player.setPosition(0.f, 0.f, 0.f);
        player.setMinDistance(5.f);
        player.setAttenuation(10.f);
        pfire.setPosition(0, 100, 0);
        pfire.setMinDistance(200);
        pfire.setAttenuation(10.f);
        pfire.play(true);
        sf::Listener::setPosition(caracter->getCenter().x, caracter->getCenter().y, 0);
        std::cout<<"appli initialised"<<std::endl;
    }
    void MyAppli::onRender(RenderComponentManager *cm) {
        // draw everything here...
        getWorld()->drawOnComponents("E_BIGTILE", 0);
        getWorld()->drawOnComponents("E_WALL+E_DECOR+E_ANIMATION+E_HERO+E_PARTICLES", 1);
        getWorld()->drawOnComponents("E_BIGTILE+E_WALL+E_DECOR+E_ANIMATION+E_HERO", 2);
        getWorld()->drawOnComponents("E_WALL+E_DECOR+E_ANIMATION+E_HERO", 3);
        getWorld()->drawOnComponents("E_WALL+E_DECOR+E_ANIMATION+E_HERO+E_PONCTUAL_LIGHT", 4);

    }
    void MyAppli::onDisplay(RenderWindow* window) {
        /*Entity* shadowMap = World::getShadowMap("E_WALL+E_DECOR+E_ANIMATION+E_CARACTER", 2, 1);
        window->draw(*shadowMap, sf::BlendMultiply);
        //getView().rotate(0, 0, 20);
        window->draw(*ps);
        //getView().rotate(0, 0, 0);
        Entity* lightMap = World::getLightMap("E_PONCTUAL_LIGHT", 2, 1);
        window->draw(*lightMap, sf::BlendMultiply);*/
        //window->draw(point);
        /*View view = getView();
        Entity& normalMap = theMap->getNormalMapTile();
        window->draw(normalMap);*/
        /*std::vector<Vec2f> segments = caracter->getPath();
        VertexArray va(sf::LinesStrip);
        for (unsigned int i = 0; i < segments.size(); i++) {
            Vertex vertex1(sf::Vector3f(segments[i].x, segments[i].y, segments[i].y), sf::Color::Red);

            va.append(vertex1);

        }
        window->draw(va);*/
        //window->draw(light2->getIntPoints());
        /*RectangleShape shape(Vec3f(100, 100, 0));
        shape.setFillColor(Color::Red);
        shape.setOutlineThickness(10);
        shape.setOutlineColor(Color(0, 0, 0));
        shape.setPosition(Vec3f(0, 300, 300));
        window->draw(shape);*/
        //window->draw(World::getLightMap<Entity>());
        /*std::vector<Entity*> entities = World::getVisibleEntities("E_BIGTILE");
        //std::cout<<entities.size()<<std::endl;
        for (unsigned int i = 0; i < entities.size(); i++) {
            window->draw(*entities[i]);
        }
        entities = World::getVisibleEntities("E_WALL+E_DECOR+E_ANIMATION+E_CARACTER");
        for (unsigned int i = 0; i < entities.size(); i++) {
        if(entities[i]->getType() != "E_SHADOW_WALL" && entities[i]->getType() != "E_SHADOW_TILE")
            window->draw(*entities[i]);
        }*/
        /*g2d::Entity* decor = World::getEntities<g2d::Entity>("E_DECOR")[0];
        g2d::Entity* caracter = World::getEntities<g2d::Entity>("E_CARACTER")[0];
        RectangleShape rect(Vec3f(decor->getCollisionVolume()->getSize().x, decor->getCollisionVolume()->getSize().y, 0));
        rect.setPosition(Vec3f(Vec3f(decor->getCollisionVolume()->getPosition().x, decor->getCollisionVolume()->getPosition().y, decor->getPosition().z)));
        window->draw(rect);
        RectangleShape rect2(Vec3f(caracter->getCollisionVolume()->getSize().x, caracter->getCollisionVolume()->getSize().y, 0));
        rect2.setPosition(Vec3f(Vec3f(caracter->getCollisionVolume()->getPosition().x, caracter->getCollisionVolume()->getPosition().y, caracter->getPosition().z)));
        window->draw(rect2);*/
    }
    void MyAppli::onUpdate (RenderWindow* rw, IEvent& event) {
        // check all the window's events that were triggered since the last iteration of the loop
        if (rw == &getRenderWindow() && event.type == IEvent::WINDOW_EVENT && event.window.type == IEvent::WINDOW_EVENT_CLOSED) {
            std::cout<<"close"<<std::endl;
            stop();
            /*eu->stop();
            au->stop();*/
            pfire.stop();
            std::vector<Entity*> entities = getWorld()->getRootEntities("E_BIGTILE+E_WALL+E_DECOR+E_ANIMATION");
            //std::cout<<"size : "<<entities.size()<<std::endl;
            std::ofstream ofs("FichierDeSerialisation");
            OTextArchive oa(ofs);
            oa(entities);
            ofs.close();
            //std::cout<<"serialized"<<std::endl;
        }
        if (event.type == IEvent::KEYBOARD_EVENT && event.keyboard.type == IEvent::KEY_EVENT_PRESSED) {
            previousKey = actualKey;
            actualKey = static_cast<IKeyboard::Key>(event.keyboard.code);
            getListener().setCommandSlotParams("MoveAction", this, static_cast<IKeyboard::Key>(event.keyboard.code));
        }
        if (event.type == IEvent::KEYBOARD_EVENT && event.keyboard.type == IEvent::KEY_EVENT_RELEASED && caracter->isMovingFromKeyboard()) {
            if (player.isPlaying())
                player.stop();

            caracter->setMoving(false);
            caracter->setIsMovingFromKeyboard(false);
            previousKey = static_cast<IKeyboard::Key>(event.keyboard.code);
            actualKey = IKeyboard::Key::Unknown;
        }
        if (event.type == IEvent::MOUSE_MOTION_EVENT) {
            sf::Vector2f mousePos = sf::Vector2f(event.mouseMotion.x, event.mouseMotion.y);
            getListener().setCommandSigParams("MouseInside", this, mousePos);
            getListener().setCommandSlotParams("MouseInside", this, mousePos);
        }
        if (event.type == IEvent::MOUSE_BUTTON_EVENT && event.mouseButton.type == IEvent::BUTTON_EVENT_PRESSED) {
            sf::Vector2f mousePos (event.mouseButton.x, event.mouseButton.y);

            getListener().setCommandSlotParams("LeftMouseButtonPressedAction", this, mousePos);
        }
    }
    void MyAppli::onExec () {
        sf::Int64 t = getClock("LoopTime").getElapsedTime().asMicroseconds();
        if (caracter->isMoving()) {
            if (!player.isPlaying()) {
                player.play(true);
            }
            if (caracter->isMovingFromKeyboard()) {
                Vec3f actualPos = Vec3f(caracter->getCenter().x, caracter->getCenter().y, 0);
                Vec3f newPos = actualPos +  Vec3f(caracter->getDir().x, caracter->getDir().y, 0) * caracter->getSpeed() * t;
                Ray r (actualPos, newPos);
                if (getWorld()->collide(caracter, r)) {
                    newPos = actualPos;
                }
                for (unsigned int i = 0; i < getRenderComponentManager().getNbComponents(); i++) {
                    if (getRenderComponentManager().getRenderComponent(i) != nullptr) {
                        View view = getRenderComponentManager().getRenderComponent(i)->getView();
                        Vec3f d = newPos - view.getPosition();
                        view.move(d.x, d.y, d.y);
                        getRenderComponentManager().getRenderComponent(i)->setView(view);
                    }
                }
                Vec3f d = newPos - getView().getPosition();
                getView().move(d.x, d.y, d.y);
                getWorld()->moveEntity(caracter, d.x, d.y, d.y);
                sf::Listener::setPosition(newPos.x, newPos.y, 0);
                //World::update("EntitiesUpdater");
            } else {

                Vec3f actualPos = caracter->getCenter();
                Vec2f pos = Computer::getPosOnPathFromTime(actualPos, caracter->getPath(),t,caracter->getSpeed());
                Vec2f d = pos - actualPos;
                Vec2f dir = d.normalize();
                //std::cout<<"position : "<<caracter->getPosition().z<<" dy : "<<d.y<<std::endl;
                getWorld()->moveEntity(caracter, d.x, d.y, d.y);
                //std::cout<<"position : "<<caracter->getPosition().z<<std::endl;
                if (dir != caracter->getDir())
                    caracter->setDir(dir);
                for (unsigned int i = 0; i < getRenderComponentManager().getNbComponents(); i++) {
                    if (getRenderComponentManager().getRenderComponent(i) != nullptr) {
                        View view = getRenderComponentManager().getRenderComponent(i)->getView();
                        view.move(d.x, d.y, d.y);
                        getRenderComponentManager().getRenderComponent(i)->setView(view);
                    }
                }
                getView().move(d.x, d.y, d.y);
                actualPos = Vec2f(caracter->getCenter().x, caracter->getCenter().y);
                sf::Listener::setDirection(dir.x, dir.y, 0);
                sf::Listener::setPosition(actualPos.x, actualPos.y, 0);
                if (actualPos.computeDist(caracter->getPath()[caracter->getPath().size() - 1]) <= 1.f) {
                    caracter->setMoving(false);
                    if (player.isPlaying())
                        player.stop();
                }
                //World::update("EntitiesUpdater");
            }
        }
        getWorld()->update();
        fpsCounter++;
        if (getClock("FPS").getElapsedTime() >= sf::seconds(1.f)) {
            std::cout<<"FPS : "<<fpsCounter<<std::endl;
            fpsCounter = 0;
            getClock("FPS").restart();
        }
        //ps->update(getClock("LoopTime").getElapsedTime());
    }
}

