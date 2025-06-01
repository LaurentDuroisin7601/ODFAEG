#include "application.h"
#include "odfaeg/Math/distributions.h"
using namespace odfaeg::math;
using namespace odfaeg::physic;
using namespace odfaeg::core;
using namespace odfaeg::graphic;
using namespace odfaeg::window;
using namespace odfaeg::network;
MyAppli::MyAppli(Vec2f size, std::string title) :
    Application(sf::VideoMode(size.x, size.y), title, sf::Style::Default, ContextSettings(24, 0, 4, 3, 0)),
    view3D(size.x, size.y, 80, 1, 1000) {
    //In perspective projection the x and y coordinates of the view are always between -1 and 1 with opengl.

    //Rotate the cube around a vector.

    //The default view have a perspective projection, but you can set another view with the setView function.
    view3D.move(0, 0, -10);
    ps = new ParticleSystem(Vec3f(0, 0, 0), Vec3f(100, 100, 100), factory);
    billboard = new BillBoard(view3D, *ps);
    view3D.setConstrains(10, 0);
    //getRenderWindow().setView(view);
    //getView().setPerspective(-size.x * 0.5f, size.x * 0.5f, -size.y * 0.5f, size.y * 0.5f, -1000, 1000);

    speed = 10.f;
    sensivity = 0.5f;

    verticalMotionActive = false;
    verticalMotionDirection = 0;
    fpsCounter = 0;
    addClock(sf::Clock(), "FPS");
    std::vector<std::string> faces
    {
        "tilesets/skybox/right.jpg",
        "tilesets/skybox/left.jpg",
        "tilesets/skybox/top.jpg",
        "tilesets/skybox/bottom.jpg",
        "tilesets/skybox/front.jpg",
        "tilesets/skybox/back.jpg"
    };
    std::vector<sf::Image> skyboxImgs;
    for (unsigned int i = 0; i < 6; i++) {
        sf::Image img;
        img.loadFromFile(faces[i]);
        skyboxImgs.push_back(img);
    }
    skybox = std::make_unique<g3d::Skybox>(skyboxImgs, factory);
}
void MyAppli::onLoad() {
    TextureManager<TEXTURES> tm;
    tm.fromFileWithAlias("tilesets/Terre2.jpg", GRASS);
    tm.fromFileWithAlias("tilesets/particule.png", PARTICLE);
    std::vector<std::string> faces
    {
        "tilesets/skybox/right.jpg",
        "tilesets/skybox/left.jpg",
        "tilesets/skybox/top.jpg",
        "tilesets/skybox/bottom.jpg",
        "tilesets/skybox/front.jpg",
        "tilesets/skybox/back.jpg"
    };
    for (unsigned int i = 0; i < 6; i++) {
        tm.fromFileWithAlias(faces[i], static_cast<TEXTURES>(i+2));
    }
    cache.addResourceManager(tm, "TextureManager");
}
void MyAppli::onInit() {

    TextureManager<TEXTURES> &tm = cache.resourceManager<Texture, TEXTURES>("TextureManager");
    theMap = new Scene(&getRenderComponentManager(), "Map test", 100, 100, 100);
    BaseChangementMatrix bcm;
    //bcm.set3DMatrix();
    theMap->setBaseChangementMatrix(bcm);
    getWorld()->addSceneManager(theMap);
    getWorld()->setCurrentSceneManager("Map test");

    std::vector<Tile*> tGround;
    std::vector<g3d::Wall*> tWall;
    Texture* text = const_cast<Texture*>(tm.getResourceByAlias(GRASS));
    text->setRepeated(true);
    text->setSmooth(true);
    tGround.push_back(new Tile(text, Vec3f(0, 0, 0), Vec3f(50, 0, 50),sf::IntRect(0, 0, 100*20, 100*20), factory));
    BoundingBox mapZone (-500, 0, -500, 1000, 50, 1000);
    std::cout<<"generate map"<<std::endl;
    getWorld()->generate_3d_map(tGround, tWall, Vec2f(50, 50), mapZone, factory);
    std::cout<<"map generated"<<std::endl;
    heightmap = static_cast<BigTile*>(getWorld()->getEntities("E_BIGTILE")[0]->getRootEntity());
    heightmap->setShadowScale(Vec3f(1, -1, 1));
    float cy;
    bool isOnHeightMap = heightmap->getHeight(Vec2f(10, 10), cy);
    cube = new g3d::Cube(Vec3f(10, cy, 10), 20, 20, 20, sf::Color::Transparent, factory);
    for (unsigned int i = 0; i < cube->getFaces().size(); i++) {
        cube->getFace(i)->getMaterial().setRefractable(true);
        cube->getFace(i)->getMaterial().setType(Material::GLASS);
    }
    cube->setShadowScale(Vec3f(1, -1, 1));
    getWorld()->addEntity(cube);
    ps->setTexture(*tm.getResourceByAlias(PARTICLE));
    for (unsigned int i = 0; i < 10; i++) {
        ps->addTextureRect(sf::IntRect(i*10, 0, 10, 10));
    }
    UniversalEmitter emitter;
    emitter.setEmissionRate(30);
    emitter.setParticleLifetime(Distributions::uniform(sf::seconds(5), sf::seconds(7)));
    emitter.setParticlePosition(Distributions::rect(Vec3f(0, 0, 50), Vec3f(10, 0, 100)));   // Emit particles in given circle
    emitter.setParticleVelocity(Distributions::deflect(Vec3f(0, 0, 10),  0)); // Emit towards direction with deviation of 15°
    emitter.setParticleRotation(Distributions::uniform(0.f, 0.f));
    emitter.setParticleTextureIndex(Distributions::uniformui(0, 9));
    emitter.setParticleScale(Distributions::rect(Vec3f(2.1f, 2.1f, 2.f), Vec3f(2.f, 2.f, 2.f)));
    ps->addEmitter(emitter);
    g3d::PonctualLight* light = new g3d::PonctualLight(Vec3f(0, 25, 10), 200, 200, 200, 255, sf::Color::Yellow, 16, factory);
    getWorld()->addEntity(light);
    eu = new EntitiesUpdater(factory, *getWorld());
    getWorld()->addWorker(eu);

    loader = g3d::Model();
    model = loader.loadModel("tilesets/mesh_puddingmill/puddingmill.obj", factory);

    animatedModel = loader.loadModel("tilesets/vampire/dancing_vampire.dae", factory);
    g3d::Animation* danceAnimation = new g3d::Animation("tilesets/vampire/dancing_vampire.dae", animatedModel);
    Entity* animator = factory.make_entity<g3d::Animator>(danceAnimation, factory);
    animator->move(Vec3f(-100, -85, 0));
    animator->setScale(Vec3f(0.1f, 0.1f, 0.1f));
    //animator->setRotation(90);
    animator->setDrawMode(Entity::INSTANCED);

    float y, z;
    model->move(Vec3f(0, 0, 10));
    //model->setScale(Vec3f(2.f, 2.f, 2.f));

    //animator->scale(Vec3f(0.1f, 0.1f, 0.1f));
    isOnHeightMap = heightmap->getHeight(Vec2f(model->getPosition().x, model->getPosition().z), y);
    float y2;
    isOnHeightMap = heightmap->getHeight(Vec2f(model->getPosition().x, model->getPosition().z - 5), y2);
    float y3;
    isOnHeightMap = heightmap->getHeight(Vec2f(model->getPosition().x + 5, model->getPosition().z), y3);
    Vec3f v1(0, y, 1);
    Vec3f v2(0, y2, 1);
    float angle = Math::toDegrees(v1.getAngleBetween(v2, v2.cross(v1)));
    Vec3f v3(1, y, 0);
    Vec3f v4(1, y3, 0);
    float angle2 = Math::toDegrees(v3.getAngleBetween(v3, v4.cross(v3)));
    angle = (y2 > y) ? angle : -angle;
    angle2 = (y3 > y) ? angle2 : -angle2;
    model->move(Vec3f(0, y, 0));
    model->setShadowScale(Vec3f(1, -1, 1));
    model->setShadowRotation(90 + angle * 100, Vec3f(1, 0, 0));
    //model->setShadowRotation(angle2 * 150, Vec3f(0, 0, 1));
    model->setShadowCenter(Vec3f(0, 0, -5));
    isOnHeightMap = heightmap->getHeight(Vec2f(animator->getPosition().x, animator->getPosition().z), z);
    //std::cout<<"animator size : "<<animator->getSize()<<std::endl;
    animator->move(Vec3f(0, z, 0));
    getWorld()->addEntity(animator);

    //std::cout<<model->getPosition()<<model->getCenter()<<std::endl;

    //model->setScale(Vec3f(0.25, 0.25, 0.25));
    //std::cout<<"matrix : "<<model->getMatrix()<<std::endl;
    getWorld()->addEntity(model);

    /*PerPixelLinkedListRenderComponent* frc = new PerPixelLinkedListRenderComponent(getRenderWindow(), 0, "E_CUBE", ContextSettings(0, 0, 4, 4, 6));
    frc->setView(view3D);*/

    //frc->setVisible(false);
    ShadowRenderComponent* src = new ShadowRenderComponent(getRenderWindow(), 1, "E_MESH+E_BIGTILE",ContextSettings(0, 0, 4, 4, 6));
    src->setView(view3D);
    PerPixelLinkedListRenderComponent* frc2 = new PerPixelLinkedListRenderComponent(getRenderWindow(), 0, "E_BIGTILE+E_MESH+E_CUBE+E_BONE_ANIMATION",ContextSettings(0, 0, 4, 4, 6));
    //frc2->preloadEntitiesOnComponent(getWorld()->getEntities("*"), factory);
    frc2->setView(view3D);
    frc2->loadSkybox(skybox.get());
    ReflectRefractRenderComponent* rrrc = new ReflectRefractRenderComponent(getRenderWindow(), 2, "E_CUBE+E_BIGTILE+E_MESH", ContextSettings(0, 0, 4, 4, 6));
    rrrc->setView(view3D);
    rrrc->loadSkybox(skybox.get());
    /*std::vector<Entity*> tiles = heightmap->getChildren();
    for (unsigned int i = 0; i < tiles.size(); i++)
        std::cout<<"window coords : "<<frc2->getFrameBuffer()->mapCoordsToPixel(tiles[i]->getPosition())<<std::endl;*/
    LightRenderComponent* lrc = new LightRenderComponent(getRenderWindow(), 3, "E_PONCTUAL_LIGHT+E_BIGTILE",ContextSettings(0, 0, 4, 4, 6));
    lrc->setView(view3D);
    //getView().setPerspective(-size.x * 0.5f, size.x * 0.5f, -size.y * 0.5f, size.y * 0.5f,-1000, 1000);
    //getRenderComponentManager().addComponent(frc);

    getRenderComponentManager().addComponent(frc2);
    getRenderComponentManager().addComponent(src);
    getRenderComponentManager().addComponent(rrrc);
    getRenderComponentManager().addComponent(lrc);
    //std::cout<<"model size : "<<model->getSize().y<<std::endl;
    for (unsigned int i = 0; i < getRenderComponentManager().getNbComponents(); i++) {
        View view = getRenderComponentManager().getRenderComponent(i)->getView();
        float y;
        bool isOnHeightMap = heightmap->getHeight(Vec2f(view.getPosition().x, view.getPosition().z), y);
        //std::cout<<"height : "<<y<<std::endl;
        view.setCenter(Vec3f(view.getPosition().x, y + model->getSize().y, view.getPosition().z));
        /*Vec3f target = view.getPosition() + Vec3f(0, -1, 0);
        view.lookAt(target.x, target.y, target.z);*/
        getRenderComponentManager().getRenderComponent(i)->setView(view);
    }

    isOnHeightMap = heightmap->getHeight(Vec2f(view3D.getPosition().x, view3D.getPosition().z), y);
    view3D.setCenter(Vec3f(view3D.getPosition().x, y + model->getSize().y, view3D.getPosition().z));

    billboard->setView(view3D);
    //std::cout<<"screen coords : "<<getRenderWindow().mapCoordsToPixel(model->getPosition(), view3D);
    g2d::AmbientLight::getAmbientLight().setColor(sf::Color::White);
    animUpdater = new AnimUpdater();
    animUpdater->addBoneAnim(animator);
    getWorld()->addTimer(animUpdater);
    getWorld()->update();
}
void MyAppli::onRender(RenderComponentManager* frcm) {
    //getWorld()->drawOnComponents("E_CUBE", 0);
    getWorld()->drawOnComponents("E_MESH+E_BIGTILE", 1);
    getWorld()->drawOnComponents("E_BIGTILE+E_MESH+E_CUBE+E_BONE_ANIMATION", 0);
    getWorld()->drawOnComponents("E_CUBE+E_BIGTILE+E_MESH", 2);
    getWorld()->drawOnComponents("E_PONCTUAL_LIGHT+E_BIGTILE", 3);
    fpsCounter++;
    if (getClock("FPS").getElapsedTime() >= sf::seconds(1.f)) {
        std::cout<<"FPS : "<<fpsCounter<<std::endl;
        fpsCounter = 0;
        getClock("FPS").restart();
    }
    /*Entity& lightMap = World::getLightMap("E_PONCTUAL_LIGHT", 1, 0);
    World::drawOnComponents(lightMap, 0, sf::BlendMultiply);*/
    //World::drawOnComponents(*billboard, 0);
    /*std::vector<Entity*> entities = World::getVisibleEntities("E_BIGTILE+E_CUBE");
    frcm->getRenderComponent(0)->loadEntitiesOnComponent(entities);
    frcm->getRenderComponent(0)->drawNextFrame();
    frcm->getRenderComponent(0)->getFrameBufferTile().setCenter(getRenderWindow().getView().getPosition());*/
}
void MyAppli::onDisplay(RenderWindow* window) {
    /*View view = window->getView();
    window->getView().setPerspective(-1, 1, -1, 1, 0, 1);
    window->draw(*skybox);
    window->setView(view);*/
    /*std::vector<Entity*> entities = World::getVisibleEntities("E_BIGTILE+E_CUBE");
    for (unsigned int i = 0; i < entities.size(); i++) {
        window->draw(*entities[i]);
    }*/
    /*Entity* lightMap = World::getLightMap("E_PONCTUAL_LIGHT", 1, 0);
    window->draw(*lightMap, sf::BlendMultiply);*/
    //std::cout<<()<<std::endl;

}
void MyAppli::onUpdate (RenderWindow* window, IEvent& event) {

        if (event.type == IEvent::WINDOW_EVENT && event.window.type == IEvent::WINDOW_EVENT_CLOSED)
        {
            stop();
        }
        if (event.type == IEvent::WINDOW_EVENT && event.window.type == IEvent::WINDOW_EVENT_RESIZED)
        {
            // Ajust the viewport size when the window is resized.
            getView().reset(BoundingBox(0, 0, getView().getViewport().getPosition().z,event.window.data1, event.window.data2, getView().getViewport().getDepth()));
        } else if (event.type == IEvent::MOUSE_MOTION_EVENT && IMouse::isButtonPressed(IMouse::Right)) {
            int relX = (event.mouseMotion.x - oldX) * sensivity;
            int relY = (event.mouseMotion.y - oldY) * sensivity;
            //Rotate the view, (Polar coordinates) but you can also use the lookAt function to look at a point.
            for (unsigned int i = 0; i < getRenderComponentManager().getNbComponents(); i++) {
                View view = getRenderComponentManager().getRenderComponent(i)->getView();
                float teta = view.getTeta() - relY;
                float phi = view.getPhi() - relX;
                view.rotate(teta, phi);
                getRenderComponentManager().getRenderComponent(i)->setView(view);
            }
            View view = billboard->getView();
            float teta = view.getTeta() - relY;
            float phi = view.getPhi() - relX;
            view.rotate(teta, phi);
            billboard->setView(view);
            getWorld()->update();
        } /*else if (event.type == sf::Event::MouseWheelMoved) {
            if (event.mouseWheel.delta > 0) {
                verticalMotionActive = true;
                verticalMotionDirection = 1;
                timeBeforeStoppingVerticalMotion = sf::milliseconds(250);
                clock2.restart();
                World::update();
            } else if (event.mouseWheel.delta < 0) {
                verticalMotionActive = true;
                verticalMotionDirection = -1;
                timeBeforeStoppingVerticalMotion = sf::milliseconds(250);
                clock2.restart();
                World::update();
            }

        }*/
}
void MyAppli::onExec() {
    if (IKeyboard::isKeyPressed(IKeyboard::Up)) {
        //Move the view along a vector, but you case also move the view at a point.
        for (unsigned int i = 0; i < getRenderComponentManager().getNbComponents(); i++) {
            View view = getRenderComponentManager().getRenderComponent(i)->getView();
            view.move(view.getForward(), -speed * clock.getElapsedTime().asSeconds());
            float y;
            bool isOnHeightMap = heightmap->getHeight(Vec2f(view.getPosition().x, view.getPosition().z), y);
            view.setCenter(Vec3f(view.getPosition().x, y + model->getSize().y,view.getPosition().z));
            getRenderComponentManager().getRenderComponent(i)->setView(view);
        }
        View view = billboard->getView();
        view.move(view.getForward(), -speed * clock.getElapsedTime().asSeconds());
        float y;
        bool isOnHeightMap = heightmap->getHeight(Vec2f(view.getPosition().x, view.getPosition().z), y);
        view.setCenter(Vec3f(view.getPosition().x, y + model->getSize().y,view.getPosition().z));
        billboard->setView(view);
        /*View view = getRenderWindow().getView();
        view.move(view.getForward(), -speed * clock.getElapsedTime().asSeconds());
        float z = heightmap->getHeight(Vec2f(view.getPosition().x, view.getPosition().y));
        view.setCenter(Vec3f(view.getPosition().x, view.getPosition().y, z+20));
        getRenderWindow().setView(view);*/

    }
    if (IKeyboard::isKeyPressed(IKeyboard::Down)) {
        for (unsigned int i = 0; i < getRenderComponentManager().getNbComponents(); i++) {
            View view = getRenderComponentManager().getRenderComponent(i)->getView();
            view.move(view.getForward(), speed * clock.getElapsedTime().asSeconds());
            float y;
            bool isOnHeightMap = heightmap->getHeight(Vec2f(view.getPosition().x, view.getPosition().z), y);
            view.setCenter(Vec3f(view.getPosition().x, y + model->getSize().y,view.getPosition().z));
            getRenderComponentManager().getRenderComponent(i)->setView(view);
        }
        View view = billboard->getView();
        view.move(view.getForward(), speed * clock.getElapsedTime().asSeconds());
        float y;
        bool isOnHeightMap = heightmap->getHeight(Vec2f(view.getPosition().x, view.getPosition().z), y);
        view.setCenter(Vec3f(view.getPosition().x, y + model->getSize().y,view.getPosition().z));
        billboard->setView(view);
        /*View view = getRenderWindow().getView();
        view.move(view.getForward(), speed * clock.getElapsedTime().asSeconds());
        float z = heightmap->getHeight(Vec2f(view.getPosition().x, view.getPosition().y));
        view.setCenter(Vec3f(view.getPosition().x, view.getPosition().y, z+20));
        getRenderWindow().setView(view);*/

    }
    if (IKeyboard::isKeyPressed(IKeyboard::Right)) {
        for (unsigned int i = 0; i < getRenderComponentManager().getNbComponents(); i++) {
            View view = getRenderComponentManager().getRenderComponent(i)->getView();
            view.move(view.getLeft(), speed * clock.getElapsedTime().asSeconds());
            float y;
            bool isOnHeightMap = heightmap->getHeight(Vec2f(view.getPosition().x, view.getPosition().z), y);
            view.setCenter(Vec3f(view.getPosition().x, y + model->getSize().y,view.getPosition().z));
            getRenderComponentManager().getRenderComponent(i)->setView(view);
        }
        View view = billboard->getView();
        view.move(view.getLeft(), speed * clock.getElapsedTime().asSeconds());
        float y;
        bool isOnHeightMap = heightmap->getHeight(Vec2f(view.getPosition().x, view.getPosition().z), y);
        view.setCenter(Vec3f(view.getPosition().x, y + model->getSize().y,view.getPosition().z));
        billboard->setView(view);
        /*View view = getRenderWindow().getView();
        view.move(view.getLeft(), speed * clock.getElapsedTime().asSeconds());
        float z = heightmap->getHeight(Vec2f(view.getPosition().x, view.getPosition().y));
        view.setCenter(Vec3f(view.getPosition().x, view.getPosition().y, z+20));
        getRenderWindow().setView(view);*/

    }
    if (IKeyboard::isKeyPressed(IKeyboard::Left)) {
        for (unsigned int i = 0; i < getRenderComponentManager().getNbComponents(); i++) {
            View view = getRenderComponentManager().getRenderComponent(i)->getView();
            view.move(view.getLeft(), -speed * clock.getElapsedTime().asSeconds());
            float y;
            bool isOnHeightMap = heightmap->getHeight(Vec2f(view.getPosition().x, view.getPosition().z), y);
            view.setCenter(Vec3f(view.getPosition().x, y + model->getSize().y,view.getPosition().z));
            getRenderComponentManager().getRenderComponent(i)->setView(view);
        }
        View view = billboard->getView();
        view.move(view.getLeft(), -speed * clock.getElapsedTime().asSeconds());
        float y;
        bool isOnHeightMap = heightmap->getHeight(Vec2f(view.getPosition().x, view.getPosition().z), y);
        view.setCenter(Vec3f(view.getPosition().x, y + model->getSize().y,view.getPosition().z));
        billboard->setView(view);
        /*View view = getRenderWindow().getView();
        view.move(view.getForward(), -speed * clock.getElapsedTime().asSeconds());
        float z = heightmap->getHeight(Vec2f(view.getPosition().x, view.getPosition().y));
        view.setCenter(Vec3f(view.getPosition().x, view.getPosition().y, z+20));
        getRenderWindow().setView(view);*/

    }
    getWorld()->update();
    /*ps->update(clock.getElapsedTime());
    animUpdater->update();*/
    /*if (clock2.getElapsedTime() > timeBeforeStoppingVerticalMotion) {
        verticalMotionActive = false;
        verticalMotionDirection = 0;
    } else {
        timeBeforeStoppingVerticalMotion -= clock2.getElapsedTime();
    }
    for (unsigned int i = 0; i < getRenderComponentManager().getNbComponents(); i++) {
        View view = getRenderComponentManager().getRenderComponent(i)->getView();
        view.move(0, 0, -verticalMotionDirection * speed * clock2.getElapsedTime().asSeconds());
        getRenderComponentManager().getRenderComponent(i)->setView(view);
    }*/
    oldX = IMouse::getPosition(getRenderWindow()).x;
    oldY = IMouse::getPosition(getRenderWindow()).y;
    clock.restart();
}
