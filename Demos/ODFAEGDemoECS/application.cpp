#include "application.hpp"
using namespace odfaeg::window;
using namespace odfaeg::core;
using namespace odfaeg::graphic;
using namespace odfaeg::graphic::ecs;
using namespace odfaeg::math;
using namespace odfaeg::physic;
namespace sorrok {
    MyECSAppli::MyECSAppli(sf::VideoMode vm, std::string title, ComponentMapping& componentMapping) : Application(vm, componentMapping, title, sf::Style::Default, ContextSettings(0, 0, 4, 4, 6)) {
        fpsCounter = 0;
        speed = 40.f;
        getView().move(0, 400, 400);
        addClock(sf::Clock(), "FPS");
    }
    void MyECSAppli::onLoad() {
        TextureManager<> tm;
        tm.fromFileWithAlias("tilesets/eau.png", "WATER");
        tm.fromFileWithAlias("tilesets/herbe.png", "GRASS");
        tm.fromFileWithAlias("tilesets/murs.png", "WALLS");
        tm.fromFileWithAlias("tilesets/maison.png", "HOUSE");
        tm.fromFileWithAlias("tilesets/flemmes1.png", "FIRE1");
        tm.fromFileWithAlias("tilesets/flemmes2.png", "FIRE2");
        tm.fromFileWithAlias("tilesets/flemmes3.png", "FIRE3");
        tm.fromFileWithAlias("tilesets/particule.png", "PARTICLE");
        cache.addResourceManager(tm, "TextureManager");
    }
    void MyECSAppli::onInit () {
        TextureManager<> &tm = cache.resourceManager<Texture, std::string>("TextureManager");
        ComponentMapping& componentMapping = getWorld()->getComponentMapping();
        scene = new Scene(&getRenderComponentManager(), componentMapping, "Map test", 100, 50, 0);
        BaseChangementMatrix bm;
        bm.set2DIsoMatrix();
        scene->setBaseChangementMatrix(bm);
        getWorld()->addSceneManager(scene);
        getWorld()->setCurrentSceneManager("Map test");
        eu = new EntitiesUpdater(*getWorld());
        eu->setName("EntitiesUpdater");
        getWorld()->addWorker(eu);
        AnimationUpdater* animUpdater = new AnimationUpdater(*getWorld());
        getWorld()->addTimer(animUpdater);
        std::ifstream ifs("myScene.oc");

        if (ifs) {
            ITextArchive ita(ifs);
            std::vector<EntityId> entities;

            componentMapping.readEntities<ITextArchive, EntityInfoComponent, TransformComponent, MeshComponent, WallTypeComponent, AnimationComponent, ColliderComponent, ShadowInfoComponent>(ita, entities);
            for (unsigned int i = 0; i < entities.size(); i++) {
                EntityInfoComponent* eic = getWorld()->getComponentMapping().getComponent<EntityInfoComponent>(entities[i]);

                if (eic != nullptr) {
                    if (eic->groupName == "E_BIGTILE") {

                        for (unsigned int j = 0; j < componentMapping.getChildren(entities[i]).size(); j++) {
                            std::string texId =  componentMapping.getComponent<MeshComponent>(componentMapping.getChildren(entities[i])[j])->faces[0].getMaterial().getTexId();
                            sf::IntRect texRect = componentMapping.getComponent<MeshComponent>(componentMapping.getChildren(entities[i])[j])->faces[0].getMaterial().getTexRect();
                            componentMapping.getComponent<MeshComponent>(componentMapping.getChildren(entities[i])[j])->faces[0].getMaterial().clearTextures();
                            componentMapping.getComponent<MeshComponent>(componentMapping.getChildren(entities[i])[j])->faces[0].getMaterial().addTexture(tm.getResourceByAlias(texId), texRect);
                            componentMapping.getComponent<MeshComponent>(componentMapping.getChildren(entities[i])[j])->faces[0].getMaterial().setTexId(texId);
                        }
                    }
                    if (eic->groupName == "E_WALL") {
                        std::string texId =  componentMapping.getComponent<MeshComponent>(componentMapping.getChildren(entities[i])[0])->faces[0].getMaterial().getTexId();
                        sf::IntRect texRect = componentMapping.getComponent<MeshComponent>(componentMapping.getChildren(entities[i])[0])->faces[0].getMaterial().getTexRect();
                        componentMapping.getComponent<MeshComponent>(componentMapping.getChildren(entities[i])[0])->faces[0].getMaterial().clearTextures();
                        componentMapping.getComponent<MeshComponent>(componentMapping.getChildren(entities[i])[0])->faces[0].getMaterial().addTexture(tm.getResourceByAlias(texId), texRect);
                        componentMapping.getComponent<MeshComponent>(componentMapping.getChildren(entities[i])[0])->faces[0].getMaterial().setTexId(texId);
                        Vec3f center = componentMapping.getComponent<TransformComponent>(entities[i])->center;
                        getWorld()->getGridCellAt(center)->setPassable(false);
                    }
                    if (eic->groupName == "E_DECOR") {
                        std::string texId =  componentMapping.getComponent<MeshComponent>(componentMapping.getChildren(entities[i])[0])->faces[0].getMaterial().getTexId();
                        sf::IntRect texRect = componentMapping.getComponent<MeshComponent>(componentMapping.getChildren(entities[i])[0])->faces[0].getMaterial().getTexRect();
                        componentMapping.getComponent<MeshComponent>(componentMapping.getChildren(entities[i])[0])->faces[0].getMaterial().clearTextures();
                        componentMapping.getComponent<MeshComponent>(componentMapping.getChildren(entities[i])[0])->faces[0].getMaterial().addTexture(tm.getResourceByAlias(texId), texRect);
                        componentMapping.getComponent<MeshComponent>(componentMapping.getChildren(entities[i])[0])->faces[0].getMaterial().setTexId(texId);
                    }
                    if (eic->groupName == "E_ANIMATION") {
                        for (unsigned int j = 0; j < componentMapping.getChildren(entities[i]).size(); j++) {
                            std::string texId = componentMapping.getComponent<MeshComponent>(componentMapping.getChildren(componentMapping.getChildren(entities[i])[j])[0])->faces[0].getMaterial().getTexId();
                            sf::IntRect texRect = componentMapping.getComponent<MeshComponent>(componentMapping.getChildren(componentMapping.getChildren(entities[i])[j])[0])->faces[0].getMaterial().getTexRect();
                            componentMapping.getComponent<MeshComponent>(componentMapping.getChildren(componentMapping.getChildren(entities[i])[j])[0])->faces[0].getMaterial().clearTextures();
                            componentMapping.getComponent<MeshComponent>(componentMapping.getChildren(componentMapping.getChildren(entities[i])[j])[0])->faces[0].getMaterial().addTexture(tm.getResourceByAlias(texId), texRect);
                            componentMapping.getComponent<MeshComponent>(componentMapping.getChildren(componentMapping.getChildren(entities[i])[j])[0])->faces[0].getMaterial().setTexId(texId);
                        }
                        animUpdater->addAnim(entities[i]);
                    }
                }
                getWorld()->addEntity(entities[i]);
            }
        } else {


            EntityId tile = ModelFactory::createTileModel(componentMapping, tm.getResourceByAlias("WATER"), Vec3f(0, 0, 0), Vec3f(120, 60, 0),sf::IntRect(0, 0, 100, 50));
            componentMapping.getComponent<MeshComponent>(tile)->faces[0].getMaterial().setTexId("WATER");
            componentMapping.getComponent<MeshComponent>(tile)->faces[0].getMaterial().setReflectable(true);
            componentMapping.getComponent<MeshComponent>(tile)->faces[0].getMaterial().setType(Material::WATER);
            componentMapping.getComponent<EntityInfoComponent>(tile)->drawMode = DrawMode::INSTANCED;
            std::vector<EntityId> tiles;
            tiles.push_back(tile);
            std::vector<EntityId> walls;
            walls.resize(WallType::NB_WALL_TYPES);
            EntityId tileWTopBot = ModelFactory::createTileModel(componentMapping, tm.getResourceByAlias("WALLS"), Vec3f(0, 0, 0), Vec3f(100, 100, 0), sf::IntRect(100, 0, 100, 100));
            componentMapping.getComponent<MeshComponent>(tileWTopBot)->faces[0].getMaterial().setTexId("WALLS");
            walls[WallType::TOP_BOTTOM] = ModelFactory::createWallModel(componentMapping, WallType::TOP_BOTTOM, tileWTopBot, *getWorld());

            EntityId tileWRightLeft = ModelFactory::createTileModel(componentMapping, tm.getResourceByAlias("WALLS"), Vec3f(0, 0, 0), Vec3f(100, 100, 0), sf::IntRect(100, 100, 100, 100));
            componentMapping.getComponent<MeshComponent>(tileWRightLeft)->faces[0].getMaterial().setTexId("WALLS");
            walls[WallType::RIGHT_LEFT] = ModelFactory::createWallModel(componentMapping, WallType::RIGHT_LEFT, tileWRightLeft, *getWorld());

            EntityId tileWBotRight = ModelFactory::createTileModel(componentMapping, tm.getResourceByAlias("WALLS"), Vec3f(0, 0, 0), Vec3f(100, 100, 0), sf::IntRect(100, 200, 100, 100));
            componentMapping.getComponent<MeshComponent>(tileWBotRight)->faces[0].getMaterial().setTexId("WALLS");
            walls[WallType::BOTTOM_RIGHT] = ModelFactory::createWallModel(componentMapping, WallType::BOTTOM_RIGHT, tileWBotRight, *getWorld());

            EntityId tileWTopRight = ModelFactory::createTileModel(componentMapping, tm.getResourceByAlias("WALLS"), Vec3f(0, 0, 0), Vec3f(100, 100, 0), sf::IntRect(100, 300, 100, 100));
            componentMapping.getComponent<MeshComponent>(tileWTopRight)->faces[0].getMaterial().setTexId("WALLS");
            walls[WallType::TOP_LEFT] = ModelFactory::createWallModel(componentMapping, WallType::TOP_LEFT, tileWTopRight, *getWorld());

            EntityId tileWTopLeft = ModelFactory::createTileModel(componentMapping, tm.getResourceByAlias("WALLS"), Vec3f(0, 0, 0), Vec3f(100, 100, 0), sf::IntRect(100, 400, 100, 100));
            componentMapping.getComponent<MeshComponent>(tileWTopLeft)->faces[0].getMaterial().setTexId("WALLS");
            walls[WallType::TOP_RIGHT] = ModelFactory::createWallModel(componentMapping, WallType::TOP_RIGHT, tileWTopLeft, *getWorld());

            EntityId tileWBotLeft = ModelFactory::createTileModel(componentMapping, tm.getResourceByAlias("WALLS"), Vec3f(0, 0, 0), Vec3f(100, 100, 0), sf::IntRect(100, 500, 100, 100));
            componentMapping.getComponent<MeshComponent>(tileWBotLeft)->faces[0].getMaterial().setTexId("WALLS");
            walls[WallType::BOTTOM_LEFT] = ModelFactory::createWallModel(componentMapping, WallType::BOTTOM_LEFT, tileWBotRight, *getWorld());


            BoundingBox mapZone(0, 0, 0, 1500, 1000, 0);
            getWorld()->generate_map(tiles, walls, Vec2f(100, 50), mapZone);
            EntityId tHouse = ModelFactory::createTileModel(componentMapping, tm.getResourceByAlias("HOUSE"),Vec3f(-100, 250, 400), Vec3f(250, 300, 0), sf::IntRect(0, 0, 250, 300));
            componentMapping.getComponent<MeshComponent>(tHouse)->faces[0].getMaterial().setTexId("HOUSE");
            EntityId dhouse = ModelFactory::createDecorModel(componentMapping, tHouse, *getWorld());
            getWorld()->addEntity(dhouse);

            std::vector<EntityId> frames;
            EntityId tf1 = ModelFactory::createTileModel(componentMapping, tm.getResourceByAlias("FIRE1"),  Vec3f(0, 100, 150), Vec3f(100, 100, 0),sf::IntRect(0, 0, 150 , 200));
            componentMapping.getComponent<MeshComponent>(tf1)->faces[0].getMaterial().setTexId("FIRE1");
            EntityId fire1 = ModelFactory::createDecorModel(componentMapping, tf1, *getWorld());
            frames.push_back(fire1);

            EntityId tf2 = ModelFactory::createTileModel(componentMapping, tm.getResourceByAlias("FIRE2"),  Vec3f(0, 100, 150), Vec3f(100, 100, 0),sf::IntRect(0, 0, 150 , 200));
            componentMapping.getComponent<MeshComponent>(tf2)->faces[0].getMaterial().setTexId("FIRE2");
            EntityId fire2 = ModelFactory::createDecorModel(componentMapping, tf2, *getWorld());
            frames.push_back(fire2);

            EntityId tf3 = ModelFactory::createTileModel(componentMapping, tm.getResourceByAlias("FIRE3"),  Vec3f(0, 100, 150), Vec3f(100, 100, 0),sf::IntRect(0, 0, 150 , 200));
            componentMapping.getComponent<MeshComponent>(tf3)->faces[0].getMaterial().setTexId("FIRE3");
            EntityId fire3 = ModelFactory::createDecorModel(componentMapping, tf3, *getWorld());
            frames.push_back(fire3);
            EntityId animation = ModelFactory::createAnimationModel(componentMapping, 0.1f, Vec3f(0, 0, 0), Vec3f(100, 100, 0), true, frames, *getWorld());
            animUpdater->addAnim(animation);
            getWorld()->addEntity(animation);

            EntityId wTile = ModelFactory::createTileModel(componentMapping, tm.getResourceByAlias("WALLS"), Vec3f(0, 0, 0), Vec3f(100, 100, 0), sf::IntRect(100, 200, 100, 100));
            EntityId w = ModelFactory::createWallModel(componentMapping, WallType::BOTTOM_RIGHT, wTile, *getWorld());
            componentMapping.getComponent<MeshComponent>(wTile)->faces[0].getMaterial().setTexId("WALLS");
            getWorld()->moveEntity<MoveSystem>(w, 0, 130, 130 + componentMapping.getComponent<TransformComponent>(w)->size.y * 0.5f);
            //w->setLayer(1);
            getWorld()->addEntity(w);
        }



        odfaeg::physic::ecs::ParticleSystem* ps = new odfaeg::physic::ecs::ParticleSystem(Vec3f(0, 0, 150), Vec3f(100, 100, 0), componentMapping);
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
        ps->addEmitter(refEmitter(emitter));

        getWorld()->addTimer(ps);
        getWorld()->addEntity(ps->getEntity());
        PerPixelLinkedListRenderComponent *frc1 = new PerPixelLinkedListRenderComponent(getRenderWindow(),0, "E_BIGTILE", ContextSettings(0, 0, 4, 4, 6));
        PerPixelLinkedListRenderComponent *frc2 = new PerPixelLinkedListRenderComponent(getRenderWindow(),1, "E_WALL+E_DECOR+E_PARTICLES+E_ANIMATION", ContextSettings(0, 0, 4, 4, 6));
        ReflectRefractRenderComponent *rrrc = new ReflectRefractRenderComponent(getRenderWindow(), 2, "E_BIGTILE+E_WALL+E_DECOR+E_ANIMATION", ContextSettings(0, 0, 4, 4, 6));
        ShadowRenderComponent *src = new ShadowRenderComponent(getRenderWindow(), 3, "E_WALL+E_DECOR+E_ANIMATION", ContextSettings(0, 0, 4, 4, 6));
        getRenderComponentManager().addECSComponent(frc1);
        getRenderComponentManager().addECSComponent(frc2);
        getRenderComponentManager().addECSComponent(rrrc);
        getRenderComponentManager().addECSComponent(src);

    }
    void MyECSAppli::onRender(odfaeg::graphic::RenderComponentManager *cm) {
        getWorld()->drawOnComponents("E_BIGTILE", 0);
        getWorld()->drawOnComponents("E_WALL+E_DECOR+E_PARTICLES+E_ANIMATION", 1);
        getWorld()->drawOnComponents("E_BIGTILE+E_WALL+E_DECOR+E_ANIMATION", 2);
        getWorld()->drawOnComponents("E_WALL+E_DECOR+E_ANIMATION", 3);
    }
    void MyECSAppli::onDisplay(odfaeg::graphic::RenderWindow* window){
    }
    void MyECSAppli::onUpdate(RenderWindow* window, IEvent& event) {
        if (window == &getRenderWindow() && event.type == odfaeg::window::IEvent::WINDOW_EVENT && event.window.type == odfaeg::window::IEvent::WINDOW_EVENT_CLOSED) {
            std::vector<EntityId> entities = getWorld()->getRootEntities("E_BIGTILE+E_WALL+E_DECOR+E_ANIMATION");
            std::ofstream ofs("myScene.oc");
            OTextArchive ota(ofs);
            getWorld()->getComponentMapping().writeEntities<OTextArchive, EntityInfoComponent, TransformComponent, MeshComponent, WallTypeComponent, AnimationComponent, ColliderComponent, ShadowInfoComponent>(ota, entities);
            ofs.close();
            stop();
        }
    }
    void MyECSAppli::onExec () {
        if (IKeyboard::isKeyPressed(IKeyboard::Right)) {
            View view = getRenderWindow().getView();
            view.move(speed * getClock("LoopTime").getElapsedTime().asSeconds(), 0, 0);
            getRenderWindow().setView(view);
            //std::cout<<"move : "<<view.getPosition()<<std::endl;
            for (unsigned int i = 0; i < getRenderComponentManager().getECSComponents().size(); i++) {
                if (getRenderComponentManager().getECSComponent(i) != nullptr) {
                    View cpntView = getRenderComponentManager().getECSComponent(i)->getView();
                    if (cpntView.isOrtho()) {

                        getRenderComponentManager().getECSComponent(i)->setView(view);
                    } else {
                        cpntView.move(cpntView.getLeft(), speed * getClock("LoopTime").getElapsedTime().asSeconds());
                        getRenderComponentManager().getECSComponent(i)->setView(cpntView);
                    }
                }
            }
        }
        if (IKeyboard::isKeyPressed(IKeyboard::Left)) {
            View view = getRenderWindow().getView();
            view.move(-speed * getClock("LoopTime").getElapsedTime().asSeconds(), 0, 0);
            getRenderWindow().setView(view);
            for (unsigned int i = 0; i < getRenderComponentManager().getECSComponents().size(); i++) {
                if (getRenderComponentManager().getECSComponent(i) != nullptr) {
                    View cpntView = getRenderComponentManager().getECSComponent(i)->getView();
                    if (cpntView.isOrtho()) {
                        getRenderComponentManager().getECSComponent(i)->setView(view);
                    } else {
                        cpntView.move(cpntView.getLeft(), -speed * getClock("LoopTime").getElapsedTime().asSeconds());
                        getRenderComponentManager().getECSComponent(i)->setView(cpntView);
                    }
                }
            }
        }
        if (IKeyboard::isKeyPressed(IKeyboard::Up)) {
            View view = getRenderWindow().getView();
            view.move(0, -speed * getClock("LoopTime").getElapsedTime().asSeconds(), 0);
            getRenderWindow().setView(view);
            for (unsigned int i = 0; i < getRenderComponentManager().getECSComponents().size(); i++) {
                if (getRenderComponentManager().getECSComponent(i) != nullptr) {
                    View cpntView = getRenderComponentManager().getECSComponent(i)->getView();
                    if (cpntView.isOrtho()) {
                        getRenderComponentManager().getECSComponent(i)->setView(view);
                    } else {
                        cpntView.move(cpntView.getForward(), -speed * getClock("LoopTime").getElapsedTime().asSeconds());
                        getRenderComponentManager().getECSComponent(i)->setView(cpntView);
                    }
                }
            }
        }
        if (IKeyboard::isKeyPressed(IKeyboard::Down)) {
            View view = getRenderWindow().getView();
            view.move(0, speed * getClock("LoopTime").getElapsedTime().asSeconds(), 0);
            getRenderWindow().setView(view);
            for (unsigned int i = 0; i < getRenderComponentManager().getECSComponents().size(); i++) {
                if (getRenderComponentManager().getECSComponent(i) != nullptr) {
                    View cpntView = getRenderComponentManager().getECSComponent(i)->getView();
                    if (cpntView.isOrtho()) {
                        getRenderComponentManager().getECSComponent(i)->setView(view);
                    } else {
                        cpntView.move(cpntView.getForward(), speed * getClock("LoopTime").getElapsedTime().asSeconds());
                        getRenderComponentManager().getECSComponent(i)->setView(cpntView);
                    }
                }
            }
        }
        getWorld()->update();
        fpsCounter++;
        if (getClock("FPS").getElapsedTime() >= sf::seconds(1.f)) {
            std::cout<<"FPS : "<<fpsCounter<<std::endl;
            fpsCounter = 0;
            getClock("FPS").restart();
        }
    }
}
