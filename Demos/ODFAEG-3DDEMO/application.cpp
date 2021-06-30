#include "application.h"
#include "odfaeg/Math/distributions.h"
using namespace odfaeg::math;
using namespace odfaeg::physic;
using namespace odfaeg::core;
using namespace odfaeg::graphic;
using namespace odfaeg::window;
MyAppli::MyAppli(Vec2f size, std::string title) :
    Application(sf::VideoMode(size.x, size.y), title, sf::Style::Default, ContextSettings(0, 0, 4, 3, 0)),
    view3D(size.x, size.y, 80, 0.1f, 1000) {
    //In perspective projection the x and y coordinates of the view are always between -1 and 1 with opengl.

    //Rotate the cube around a vector.

    //The default view have a perspective projection, but you can set another view with the setView function.
    view3D.move(0, 10, 0);
    ps = new ParticleSystem(Vec3f(0, 0, 0), Vec3f(100, 100, 100));
    billboard = new BillBoard(view3D, *ps);
    view3D.setConstrains(0, 10);
    //getRenderWindow().setView(view);
    //getView().setPerspective(-size.x * 0.5f, size.x * 0.5f, -size.y * 0.5f, size.y * 0.5f, -1000, 1000);

    speed = 10.f;
    sensivity = 0.1f;
    oldX = IMouse::getPosition(getRenderWindow()).x;
    oldY = IMouse::getPosition(getRenderWindow()).y;
    verticalMotionActive = false;
    verticalMotionDirection = 0;
    fpsCounter = 0;
    addClock(sf::Clock(), "FPS");
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
    theMap = new Map(&getRenderComponentManager(), "Map test", 100, 100, 100);
    BaseChangementMatrix bcm;
    //bcm.set3DMatrix();
    theMap->setBaseChangementMatrix(bcm);
    getWorld()->addEntityManager(theMap);
    getWorld()->setCurrentEntityManager("Map test");
    cube = new g3d::Cube(Vec3f(-400, -300, 0), 800, 600, 600, sf::Color::White);
    cube->setPosition(view3D.getPosition());

    /*cube->move(Vec3f(0.f, 0.f, 50));
    cube->rotate(45, Vec3f(0, 0, 1));
    cube->setOrigin(Vec3f(0, 0, 0));
    cube->scale(Vec3f(10, 10, 10));
    cube->setShadowCenter(Vec3f(0, 20, 0));*/
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
        const Texture* tex = tm.getResourceByAlias(static_cast<TEXTURES>(i+2));
        sf::IntRect texRect (0, 0, tex->getSize().x, tex->getSize().y);
        cube->getFace(i)->getMaterial().clearTextures();
        cube->getFace(i)->getMaterial().addTexture(tex, texRect);
        static_cast<g3d::Cube*>(cube)->setTexCoords(i, texRect);
    }
    getWorld()->addEntity(cube);
    std::vector<Tile*> tGround;
    std::vector<Tile*> tWall;
    Texture* text = const_cast<Texture*>(tm.getResourceByAlias(GRASS));
    text->setRepeated(true);
    text->setSmooth(true);
    tGround.push_back(new Tile(text, Vec3f(0, 0, 0), Vec3f(50, 50, 0),sf::IntRect(0, 0, 100*20, 100*20)));
    BoundingBox mapZone (-500, -500, 0, 1000, 1000, 50);
    getWorld()->generate_map(tGround, tWall, Vec2f(50, 50), mapZone, true);
    heightmap = static_cast<BigTile*>(getWorld()->getEntities("E_BIGTILE")[0]->getRootEntity());
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
    g2d::PonctualLight* light = new g2d::PonctualLight(Vec3f(0, 0, 10), 200, 200, 200, 255, sf::Color::Yellow, 16);
    getWorld()->addEntity(light);
    eu = new EntitiesUpdater();
    getWorld()->addWorker(eu);
    View view = billboard->getView();
    float z = heightmap->getHeight(Vec2f(view.getPosition().x, view.getPosition().y));
    loader = g3d::Model();
    Entity* model = loader.loadModel("tilesets/mesh_puddingmill/puddingmill.obj");
    model->move(Vec3f(0, 0, z+15));
    model->setRotation(90, Vec3f(1, 0, 0));
    model->setShadowCenter(Vec3f(0, 20, 0));
    //model->setScale(Vec3f(0.25, 0.25, 0.25));
    //std::cout<<"matrix : "<<model->getMatrix()<<std::endl;
    getWorld()->addEntity(model);

    PerPixelLinkedListRenderComponent* frc = new PerPixelLinkedListRenderComponent(getRenderWindow(), 0, "E_CUBE", ContextSettings(0, 0, 4, 4, 6));
    frc->setView(view3D);
    //frc->setVisible(false);
    /*ShadowRenderComponent* src = new ShadowRenderComponent(getRenderWindow(), 1, "E_CUBE+E_3DMODEL");
    src->setView(view);*/
    PerPixelLinkedListRenderComponent* frc2 = new PerPixelLinkedListRenderComponent(getRenderWindow(), 1, "E_BIGTILE+E_MESH",ContextSettings(0, 0, 4, 4, 6));
    frc2->setView(view3D);
    /*LightRenderComponent* lrc = new LightRenderComponent(getRenderWindow(), 3, "E_BIGTILE+E_CUBE+E_3DMODEL+E_PONCTUAL_LIGHT");
    lrc->setView(view);*/
    //getView().setPerspective(-size.x * 0.5f, size.x * 0.5f, -size.y * 0.5f, size.y * 0.5f,-1000, 1000);
    getRenderComponentManager().addComponent(frc);
    //getRenderComponentManager().addComponent(src);
    getRenderComponentManager().addComponent(frc2);
    //getRenderComponentManager().addComponent(lrc);

    for (unsigned int i = 0; i < getRenderComponentManager().getNbComponents(); i++) {
        View view = getRenderComponentManager().getRenderComponent(i)->getView();
        float z = heightmap->getHeight(Vec2f(view.getPosition().x, view.getPosition().y));
        view.setCenter(Vec3f(view.getPosition().x, view.getPosition().y, z+20));
        getRenderComponentManager().getRenderComponent(i)->setView(view);
    }

    view.setCenter(Vec3f(view.getPosition().x, view.getPosition().y, z+20));
    billboard->setView(view);
    billboard->setCenter(Vec3f(0, 0, z+20));
    g2d::AmbientLight::getAmbientLight().setColor(sf::Color::White);


    getWorld()->update();

}
void MyAppli::onRender(RenderComponentManager* frcm) {
    getWorld()->drawOnComponents("E_CUBE", 0);
    //World::drawOnComponents("E_CUBE+E_3DMODEL", 1);
    getWorld()->drawOnComponents("E_BIGTILE+E_MESH", 1);
    //World::drawOnComponents("E_BIGTILE+E_CUBE+E_3DMODEL+E_PONCTUAL_LIGHT", 3);
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
                int teta = view.getTeta() - relX;
                int phi = view.getPhi() - relY;
                view.rotate(teta, phi);
                getRenderComponentManager().getRenderComponent(i)->setView(view);
            }
            View view = billboard->getView();
            int teta = view.getTeta() - relX;
            int phi = view.getPhi() - relY;
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
            float z = heightmap->getHeight(Vec2f(view.getPosition().x, view.getPosition().y));
            view.setCenter(Vec3f(view.getPosition().x, view.getPosition().y, z+20));
            getRenderComponentManager().getRenderComponent(i)->setView(view);
        }
        View view = billboard->getView();
        view.move(view.getForward(), -speed * clock.getElapsedTime().asSeconds());
        float z = heightmap->getHeight(Vec2f(view.getPosition().x, view.getPosition().y));
        view.setCenter(Vec3f(view.getPosition().x, view.getPosition().y, z+20));
        billboard->setView(view);
        billboard->setCenter(Vec3f(0, 0, z+20));
        /*View view = getRenderWindow().getView();
        view.move(view.getForward(), -speed * clock.getElapsedTime().asSeconds());
        float z = heightmap->getHeight(Vec2f(view.getPosition().x, view.getPosition().y));
        view.setCenter(Vec3f(view.getPosition().x, view.getPosition().y, z+20));
        getRenderWindow().setView(view);*/
        getWorld()->update();
    }
    if (IKeyboard::isKeyPressed(IKeyboard::Down)) {
        for (unsigned int i = 0; i < getRenderComponentManager().getNbComponents(); i++) {
            View view = getRenderComponentManager().getRenderComponent(i)->getView();
            view.move(view.getForward(), speed * clock.getElapsedTime().asSeconds());
            float z = heightmap->getHeight(Vec2f(view.getPosition().x, view.getPosition().y));
            view.setCenter(Vec3f(view.getPosition().x, view.getPosition().y, z+20));
            getRenderComponentManager().getRenderComponent(i)->setView(view);
        }
        View view = billboard->getView();
        view.move(view.getForward(), -speed * clock.getElapsedTime().asSeconds());
        float z = heightmap->getHeight(Vec2f(view.getPosition().x, view.getPosition().y));
        view.setCenter(Vec3f(view.getPosition().x, view.getPosition().y, z+20));
        billboard->setView(view);
        billboard->setCenter(Vec3f(0, 0, z+20));
        /*View view = getRenderWindow().getView();
        view.move(view.getForward(), speed * clock.getElapsedTime().asSeconds());
        float z = heightmap->getHeight(Vec2f(view.getPosition().x, view.getPosition().y));
        view.setCenter(Vec3f(view.getPosition().x, view.getPosition().y, z+20));
        getRenderWindow().setView(view);*/
        getWorld()->update();
    }
    if (IKeyboard::isKeyPressed(IKeyboard::Right)) {
        for (unsigned int i = 0; i < getRenderComponentManager().getNbComponents(); i++) {
            View view = getRenderComponentManager().getRenderComponent(i)->getView();
            view.move(view.getLeft(), speed * clock.getElapsedTime().asSeconds());
            float z = heightmap->getHeight(Vec2f(view.getPosition().x, view.getPosition().y));
            view.setCenter(Vec3f(view.getPosition().x, view.getPosition().y, z+20));
            getRenderComponentManager().getRenderComponent(i)->setView(view);
        }
        View view = billboard->getView();
        view.move(view.getForward(), -speed * clock.getElapsedTime().asSeconds());
        float z = heightmap->getHeight(Vec2f(view.getPosition().x, view.getPosition().y));
        view.setCenter(Vec3f(view.getPosition().x, view.getPosition().y, z+20));
        billboard->setView(view);
        billboard->setCenter(Vec3f(0, 0, z+20));
        /*View view = getRenderWindow().getView();
        view.move(view.getLeft(), speed * clock.getElapsedTime().asSeconds());
        float z = heightmap->getHeight(Vec2f(view.getPosition().x, view.getPosition().y));
        view.setCenter(Vec3f(view.getPosition().x, view.getPosition().y, z+20));
        getRenderWindow().setView(view);*/
        getWorld()->update();
    }
    if (IKeyboard::isKeyPressed(IKeyboard::Left)) {
        for (unsigned int i = 0; i < getRenderComponentManager().getNbComponents(); i++) {
            View view = getRenderComponentManager().getRenderComponent(i)->getView();
            view.move(view.getLeft(), -speed * clock.getElapsedTime().asSeconds());
            float z = heightmap->getHeight(Vec2f(view.getPosition().x, view.getPosition().y));
            view.setCenter(Vec3f(view.getPosition().x, view.getPosition().y, z+20));
            getRenderComponentManager().getRenderComponent(i)->setView(view);
        }
        View view = billboard->getView();
        view.move(view.getForward(), -speed * clock.getElapsedTime().asSeconds());
        float z = heightmap->getHeight(Vec2f(view.getPosition().x, view.getPosition().y));
        view.setCenter(Vec3f(view.getPosition().x, view.getPosition().y, z+20));
        billboard->setView(view);
        billboard->setCenter(Vec3f(0, 0, z+20));
        /*View view = getRenderWindow().getView();
        view.move(view.getForward(), -speed * clock.getElapsedTime().asSeconds());
        float z = heightmap->getHeight(Vec2f(view.getPosition().x, view.getPosition().y));
        view.setCenter(Vec3f(view.getPosition().x, view.getPosition().y, z+20));
        getRenderWindow().setView(view);*/
        getWorld()->update();
    }
    ps->update(clock.getElapsedTime());
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
