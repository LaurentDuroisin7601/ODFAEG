#include "application.h"
using namespace odfaeg::core;
using namespace odfaeg::graphic;
using namespace odfaeg::math;
using namespace odfaeg::physic;
using namespace odfaeg::network;
namespace sorrok {
    int MyAppli::spotID = 0;
    MyAppli::MyAppli() : Application () {
        std::cout<<"application"<<std::endl;
        EXPORT_CLASS_GUID(BoundingVolumeBoundingBox, BoundingVolume, BoundingBox)
        EXPORT_CLASS_GUID_(EntityTile, Entity, Tile, VA_LIST(EntityFactory&), VA_LIST(std::ref(entityFactory)))
        EXPORT_CLASS_GUID_(EntityTile, Entity, BigTile, VA_LIST(EntityFactory&), VA_LIST(std::ref(entityFactory)))
        EXPORT_CLASS_GUID_(EntityWall, Entity, g2d::Wall, VA_LIST(EntityFactory&), VA_LIST(std::ref(entityFactory)))
        EXPORT_CLASS_GUID_(EntityDecor, Entity, g2d::Decor, VA_LIST(EntityFactory&), VA_LIST(std::ref(entityFactory)))
        EXPORT_CLASS_GUID_(EntityAnimation, Entity, Anim, VA_LIST(EntityFactory&), VA_LIST(std::ref(entityFactory)))
        EXPORT_CLASS_GUID_(EntityHero, Entity, Hero, VA_LIST(EntityFactory&), VA_LIST(std::ref(entityFactory)))
        EXPORT_CLASS_GUID_(EntityMonster, Entity, Monster, VA_LIST(EntityFactory&), VA_LIST(std::ref(entityFactory)))
        EXPORT_CLASS_GUID_(EntityPnj, Entity, Pnj, VA_LIST(EntityFactory&), VA_LIST(std::ref(entityFactory)))
        EXPORT_CLASS_GUID_(EntityMesh, Entity, Mesh, VA_LIST(EntityFactory&), VA_LIST(std::ref(entityFactory)))
        EXPORT_CLASS_GUID_(EntityPLIGHT, Entity, g2d::PonctualLight, VA_LIST(EntityFactory&), VA_LIST(std::ref(entityFactory)))
        EXPORT_CLASS_GUID_(EntityPSystem, Entity, ParticleSystem, VA_LIST(EntityFactory&), VA_LIST(std::ref(entityFactory)))
        running = false;
        Network::setCertifiateClientMess("SORROKCLIENT");
    }
    void MyAppli::onLoad() {

    }
    void MyAppli::onInit () {
        //std::cout<<"start srv"<<std::endl;
        Network::startSrv(10'000, 10'001);
        //std::cout<<"server started"<<std::endl;
        theMap = new Scene(nullptr, "Map test", 100, 50, 0);
        //std::cout<<"map created"<<std::endl;
        /*try {
            driver = get_driver_instance();
            connection = driver->connect("tcp://127.0.0.1:3306", "root", "Kirokofu457$");
            std::cout<<"We are connected!"<<std::endl;
        } catch (sql::SQLException &e) {
            // Gestion des execeptions pour déboggage
            std::cout << "# ERR: " << e.what();
            std::cout << " (code erreur MySQL: " << e.getErrorCode();
            std::cout << ", EtatSQL: " << e.getSQLState() << " )" << std::endl;
        }*/
        BaseChangementMatrix bcm;
        bcm.set2DIsoMatrix();
        theMap->setBaseChangementMatrix(bcm);
        std::cout<<"base changement matrix set"<<std::endl;
        getWorld()->addSceneManager(theMap);
        std::cout<<"entity manager added"<<std::endl;
        getWorld()->setCurrentSceneManager("Map test");
        std::cout<<"current em set"<<std::endl;
        tiles.push_back(entityFactory.make_entity<Tile>(nullptr, Vec3f(0, 0, 0), Vec3f(120, 60, 0),sf::IntRect(0, 0, 100, 50), entityFactory));
        std::cout<<"tiles created"<<std::endl;
        walls.resize(g2d::Wall::NB_WALL_TYPES, nullptr);
        std::cout<<"wall resized"<<std::endl;
        walls[g2d::Wall::TOP_BOTTOM] = entityFactory.make_entity<g2d::Wall>(entityFactory.make_entity<Tile>(nullptr, Vec3f(0, 0, 0), Vec3f(100, 100, 0), sf::IntRect(100, 0, 100, 100), entityFactory),g2d::Wall::TOP_BOTTOM,&g2d::AmbientLight::getAmbientLight(), entityFactory);
        std::cout<<"wall top bottom created"<<std::endl;
        walls[g2d::Wall::RIGHT_LEFT] = entityFactory.make_entity<g2d::Wall>(entityFactory.make_entity<Tile>(nullptr, Vec3f(0, 0, 0), Vec3f(100, 100, 0), sf::IntRect(100, 100, 100, 100), entityFactory),g2d::Wall::RIGHT_LEFT,&g2d::AmbientLight::getAmbientLight(), entityFactory);
        walls[g2d::Wall::BOTTOM_LEFT] = entityFactory.make_entity<g2d::Wall>(entityFactory.make_entity<Tile>(nullptr, Vec3f(0, 0, 0), Vec3f(100, 100, 0), sf::IntRect(100, 200, 100, 100), entityFactory),g2d::Wall::BOTTOM_LEFT,&g2d::AmbientLight::getAmbientLight(), entityFactory);
        walls[g2d::Wall::TOP_RIGHT] = entityFactory.make_entity<g2d::Wall>(entityFactory.make_entity<Tile>(nullptr, Vec3f(0, 0, 0), Vec3f(100, 100, 0), sf::IntRect(100, 300, 100, 100), entityFactory),g2d::Wall::TOP_RIGHT,&g2d::AmbientLight::getAmbientLight(), entityFactory);
        walls[g2d::Wall::TOP_LEFT] = entityFactory.make_entity<g2d::Wall>(entityFactory.make_entity<Tile>(nullptr, Vec3f(0, 0, 0), Vec3f(100, 100, 0), sf::IntRect(100, 400, 100, 100), entityFactory),g2d::Wall::TOP_LEFT,&g2d::AmbientLight::getAmbientLight(), entityFactory);
        walls[g2d::Wall::BOTTOM_RIGHT] = entityFactory.make_entity<g2d::Wall> (entityFactory.make_entity<Tile>(nullptr, Vec3f(0, 0, 0), Vec3f(100, 100, 0), sf::IntRect(100, 500, 100, 100), entityFactory), g2d::Wall::BOTTOM_RIGHT,&g2d::AmbientLight::getAmbientLight(), entityFactory);
        std::cout<<"walls created"<<std::endl;
        tiles[0]->getFaces()[0].getMaterial().setTexId("GRASS");
        walls[g2d::Wall::TOP_BOTTOM]->getChildren()[0]->getFace(0)->getMaterial().setTexId("WALLS");
        walls[g2d::Wall::RIGHT_LEFT]->getChildren()[0]->getFace(0)->getMaterial().setTexId("WALLS");
        walls[g2d::Wall::BOTTOM_LEFT]->getChildren()[0]->getFace(0)->getMaterial().setTexId("WALLS");
        walls[g2d::Wall::TOP_RIGHT]->getChildren()[0]->getFace(0)->getMaterial().setTexId("WALLS");
        walls[g2d::Wall::TOP_LEFT]->getChildren()[0]->getFace(0)->getMaterial().setTexId("WALLS");
        walls[g2d::Wall::BOTTOM_RIGHT]->getChildren()[0]->getFace(0)->getMaterial().setTexId("WALLS");
        std::cout<<"tex id set"<<std::endl;
        /*std::ifstream ifs("FichierDeSerialisation");
        if(ifs) {
            ITextArchive ia(ifs);
            std::vector<Entity*> entities;
            ia(entities);
            for (unsigned int i = 0; i < entities.size(); i++) {
                getWorld()->addEntity(entities[i]);
                if (entities[i]->getType() == "E_BIGTILE") {
                    for (unsigned int j = 0; j < entities[i]->getChildren().size(); j++) {
                        std::string texId =  entities[i]->getChildren()[j]->getFaces()[0]->getMaterial().getTexId();
                        entities[i]->getChildren()[j]->getFaces()[0]->getMaterial().setTexId(texId);
                    }

                } else if (entities[i]->getType() == "E_WALL") {
                    std::string texId =  entities[i]->getChildren()[0]->getFaces()[0]->getMaterial().getTexId();
                    entities[i]->getChildren()[0]->getFaces()[0]->getMaterial().setTexId(texId);
                    getWorld()->getGridCellAt(Vec3f(entities[i]->getCenter().x, entities[i]->getCenter().y, 0))->setPassable(false);
                } else if (entities[i]->getType() == "E_DECOR") {
                    std::string texId =  entities[i]->getChildren()[0]->getFaces()[0]->getMaterial().getTexId();
                    entities[i]->getChildren()[0]->getFaces()[0]->getMaterial().setTexId(texId);
                } else if (entities[i]->getType() == "E_ANIMATION") {
                    Anim* anim = static_cast<Anim*> (entities[i]);
                    for (unsigned int j = 0; j < anim->getChildren().size(); j++) {
                        std::string texId = entities[i]->getChildren()[j]->getChildren()[0]->getFaces()[0]->getMaterial().getTexId();
                        entities[i]->getChildren()[j]->getChildren()[0]->getFaces()[0]->getMaterial().setTexId(texId);
                    }
                }
            }
            ifs.close();
        } else {*/
            BoundingBox mapZone(0, 0, 0, 1500, 1000, 0);
            //std::cout<<"generate map"<<std::endl;

            getWorld()->generate_map(tiles, walls, Vec2f(100, 50), mapZone, false, entityFactory);

            //std::cout<<"map generated"<<std::endl;
            w = entityFactory.make_entity<g2d::Wall>(entityFactory.make_entity<Tile>(nullptr, Vec3f(0, 0, 0), Vec3f(100, 100, 0), sf::IntRect(100, 200, 100, 100), entityFactory),g2d::Wall::BOTTOM_LEFT, &g2d::AmbientLight::getAmbientLight(), entityFactory);
            w->setPosition(Vec3f(0, 130, 130 + w->getSize().y * 0.5f));
            w->getChildren()[0]->getFace(0)->getMaterial().setTexId("WALLS");
            getWorld()->addEntity(w);
            Tile* thouse = entityFactory.make_entity<Tile>(nullptr, Vec3f(0, 0, 0), Vec3f(250, 300, 0), sf::IntRect(0, 0, 250, 300), entityFactory);
            thouse->getFaces()[0].getMaterial().setTexId("HOUSE");
            g2d::Decor* decor = entityFactory.make_entity<g2d::Decor>(thouse, &g2d::AmbientLight::getAmbientLight(), entityFactory);
            decor->setPosition(Vec3f(-100, 250, 400));
            decor->setShadowCenter(Vec3f(0, 500, 0));
            BoundingVolume *bb = new BoundingBox(decor->getGlobalBounds().getPosition().x, decor->getGlobalBounds().getPosition().y + decor->getGlobalBounds().getSize().y * 0.4f, 0,
            decor->getGlobalBounds().getSize().x, decor->getGlobalBounds().getSize().y * 0.25f, 0);
            decor->setCollisionVolume(bb);
            getWorld()->addEntity(decor);
            Anim* fire = entityFactory.make_entity<Anim>(0.1f, Vec3f(0, 100, 150), Vec3f(100, 100, 0), entityFactory);
            Tile* tf1 =  entityFactory.make_entity<Tile>(nullptr, Vec3f(0, 100, 150), Vec3f(100, 100, 0), sf::IntRect(0, 0, 150, 200), entityFactory);
            tf1->getFaces()[0].getMaterial().setTexId("FIRE1");
            g2d::Decor *fire1 =  entityFactory.make_entity<g2d::Decor>(tf1, &g2d::AmbientLight::getAmbientLight(), entityFactory);
            fire->setShadowCenter(Vec3f(0, 200, 0));
            fire->setShadowScale(Vec3f(1, -1, 1));
            Tile* tf2 =  entityFactory.make_entity<Tile>(nullptr, Vec3f(0, 100, 150), Vec3f(100, 100, 0), sf::IntRect(0, 0, 150, 200), entityFactory);
            tf2->getFaces()[0].getMaterial().setTexId("FIRE2");
            g2d::Decor *fire2 =  entityFactory.make_entity<g2d::Decor>(tf2, &g2d::AmbientLight::getAmbientLight(), entityFactory);
            //fire2->setShadowCenter(Vec3f(0, 200, 0));
            Tile* tf3 =  entityFactory.make_entity<Tile>(nullptr, Vec3f(0, 100, 150), Vec3f(100, 100, 0), sf::IntRect(0, 0, 150, 200), entityFactory);
            tf3->getFaces()[0].getMaterial().setTexId("FIRE3");
            g2d::Decor *fire3 =  entityFactory.make_entity<g2d::Decor>(tf3, &g2d::AmbientLight::getAmbientLight(), entityFactory);
            //fire3->setShadowCenter(Vec3f(0, 200, 0));
            //decor->setShadowCenter(Vec2f(0, 60));
            //decor->changeGravityCenter(Vec3f(50, 50, 0));
            //fire1->setShadowCenter(Vec2f(80, 100));
            //fire2->setShadowCenter(Vec2f(80, 100));
            //fire3->setShadowCenter(Vec2f(80, 100));
            fire->addFrame(fire1);
            fire->addFrame(fire2);
            fire->addFrame(fire3);
            getWorld()->addEntity(fire);
                  //}

        //PonctualLight* light = new PonctualLight(Vec2f(50, 150),100,50,0,200,sf::Color(255,255,0),16,0);
        //getWorld()->addEntity(light);

        g2d::PonctualLight* light1 = entityFactory.make_entity<g2d::PonctualLight>(Vec3f(0, 420, 420), 100, 50, 50, 255, sf::Color::Yellow, 16, entityFactory);
        light2 =  entityFactory.make_entity<g2d::PonctualLight>(Vec3f(50, 160, 160), 100, 50, 50, 255, sf::Color::Yellow, 16, entityFactory);
        getWorld()->addEntity(light1);
        getWorld()->addEntity(light2);
        //std::cout<<"decor generated"<<std::endl;
        //getView().move(d.x * 0.5f, d.y * 0.5f, 0);
        //getWorld()->computeIntersectionsWithWalls();
        std::array<Vec3f, 4> pts;
        pts[0] = Vec3f(100, 200, 0);
        pts[1] = Vec3f(300, 200, 0);
        pts[2] = Vec3f(100, 400, 0);
        pts[3] = Vec3f(300, 400, 0);
        BoundingPolyhedron monsterZone(pts[0], pts[2], pts[3], true);
        monsterZone.addTriangle(pts[2], pts[0], pts[3]);
        Monster* monster = entityFactory.make_entity<Monster>("Ogro", "Orc","MapTest",1,monsterZone, entityFactory);
        //std::cout<<"monster id : "<<monster->getId()<<std::endl;
        Vec3f pos = monster->respawn();
        Item item("HP potion", Item::HP_POTION,"All");
        monster->addLootableItem(item, 1.f);
        tmpPosition = pos;
        monster->setCenter(pos);
        getWorld()->addEntity(monster);
        //std::cout<<"monter generated"<<std::endl;
        Quest quest("Quête du débutant", "Tuer 10 ogres", true);
        quest.addMonsterToKill(monster->getName(), 10);
        quest.addReward(5, item);
        Pnj* pnj = entityFactory.make_entity<Pnj>(entityFactory);
        pnj->addQuest(quest);
        pnj->setCenter(Vec3f (300, 300, 300));
        quest.setPnjToVisit("Questy");
        getWorld()->addEntity(pnj);
        std::cout<<"server is ready!"<<std::endl;
    }
    void MyAppli::onExec () {
        if (Network::hasRequest()) {
            User* user;
            std::string message = Network::getLastRequest(&user);
            sf::Int64 oldLoopCliTime = user->getClientTime();
            std::vector<std::string> infos = split(message, "*");
            std::string request = infos[0];
            if (request == "RETRACTMANA") {
                int id = conversionStringInt(infos[1]);
                Caracter* hero = static_cast<Caracter*>(getWorld()->getEntity(id));
                float manaAmount = conversionStringFloat(infos[2]);
                caracter->setMana(caracter->getMana() - manaAmount);
                std::string response = "RETRACTMANA"+conversionIntString(id)+"*"+conversionFloatString(manaAmount);
                SymEncPacket packet;
                packet<<response;
                user->sendTcpPacket(packet);
            }
            if (request == "ACCEPT") {
                unsigned int id = conversionStringInt(infos[1]);
                std::string name = infos[2];
                Pnj* pnj = static_cast<Pnj*>(getWorld()->getEntity(id));
                Quest quest;
                for (unsigned int i = 0; i < pnj->getQuests().size(); i++) {
                    if (pnj->getQuests()[i].getName() == name) {
                        quest = pnj->getQuests()[i];
                    }
                }
                Hero* hero = static_cast<Hero*>(getWorld()->getEntity(conversionStringInt(infos[3])));
                hero->addQuest(quest);
            }
            if (request == "TALKTOPNJ") {
                int x = conversionStringInt(infos[1]);
                int y = conversionStringInt(infos[2]);
                int z = conversionStringInt(infos[3]);
                int w = conversionStringInt(infos[4]);
                int h = conversionStringInt(infos[5]);
                int d = conversionStringInt(infos[6]);
                Vec3f position(x, y, 0);
                BoundingBox rect (x, y, z, w, h, d);
                std::vector<Entity*> pnjs = getWorld()->getEntitiesInRect(rect, "E_PNJ");
                int distMin = 100;
                int id = -1;
                for (unsigned int i = 0; i < pnjs.size(); i++) {

                    Vec3f center(pnjs[i]->getCenter().x, pnjs[i]->getCenter().y, 0);
                    if (center.computeDist(position) < distMin) {
                        distMin = center.computeDist(position);
                        id = pnjs[i]->getId();
                    }
                }
                if (id != -1) {
                    std::string message = "SHOWQUEST"+conversionIntString(id);
                    SymEncPacket packet;
                    packet<<message;
                    user->sendTcpPacket(packet);
                }
            }
            if (request == "GETMAPINFOS") {
                std::ostringstream oss;
                OTextArchive oa(oss);
                std::vector<Entity*> entities = getWorld()->getEntities("E_BIGTILE+E_WALL+E_DECOR+E_ANIMATION+E_PONCTUAL_LIGHT");
                oa(entities);
                SymEncPacket packet;
                packet<<"MAPINFOS"+oss.str();
                user->sendTcpPacket(packet);
            }
            if (request == "GETCARINFOS") {
                std::ostringstream oss;
                OTextArchive oa(oss);
                std::vector<Entity*> heroes = getWorld()->getEntities("E_HERO");
                oa(heroes);
                SymEncPacket packet;
                packet<<"CARINFOS"+oss.str();
                user->sendTcpPacket(packet);
                oss.str("");
                oa.clear();
                std::vector<Entity*> monsters = getWorld()->getEntities("E_MONSTER");
                oa(monsters);
                packet.clear();
                packet<<"MONSTERSINFOS"+oss.str();
                user->sendTcpPacket(packet);
                oss.str("");
                oa.clear();
                std::vector<Entity*> pnjs = getWorld()->getEntities("E_PNJ");
                oa(pnjs);
                packet.clear();
                packet<<"PNJINFOS"+oss.str();
                user->sendTcpPacket(packet);
            } else if (request == "MOVEFROMKEYBOARD") {
                int id = conversionStringInt(infos[1]);
                Hero* hero = static_cast<Hero*> (getWorld()->getEntity(id));
                Vec2f dir (conversionStringFloat(infos[2]), conversionStringFloat(infos[3]));
                sf::Int64 last_cli_time = conversionStringLong(infos[4]);
                sf::Int64 elapsedTime = user->getClientTime() - last_cli_time;
                sf::Int64 transferTime = user->getPingAvg();
                sf::Int64 cli_time = user->getClientTime();
                hero->setDir(Vec2f(dir.x, dir.y));
                hero->setMoving(true);
                hero->setIsMovingFromKeyboard(true);
                hero->setFightingMode(false);
                hero->setAttacking(false);
                Vec3f newPos = hero->getCenter();
                std::string response = "NEWPOS"+conversionIntString(hero->getId())+"*"+conversionLongString(transferTime)+"*"+conversionFloatString(newPos.x)+"*"+conversionFloatString(newPos.y)+"*"+conversionFloatString(newPos.z)+"*"+conversionLongString(cli_time)+"*"
                +conversionIntString(hero->isMoving())+"*"+conversionIntString(hero->isInFightingMode())+"*"+conversionIntString(hero->isAttacking())+"*"+conversionIntString(hero->isAlive())+"*"+conversionIntString(hero->getLife());
                SymEncPacket packet;
                packet<<response;
                user->sendTcpPacket(packet);
            //Update the caracter states for a mouse move and update it's position from the transfer time.
            } else if (request == "MOVEFROMPATH") {
                int id = conversionStringInt(infos[1]);
                Hero* hero = static_cast<Hero*> (getWorld()->getEntity(id));
                //std::cout<<"hero pos : "<<hero->getCenter()<<std::endl;
                Vec3f fPos (conversionStringInt(infos[2]), conversionStringInt(infos[3]), 0);
                std::vector<Vec2f> path = getWorld()->getPath(hero, fPos);
                hero->setPath(path);
                hero->setMoving(true);
                hero->setIsMovingFromKeyboard(false);
                hero->setFightingMode(false);
                hero->setAttacking(false);
                SymEncPacket packet;
                std::string response="";
                int size = path.size();
                sf::Int64 cli_time = user->getClientTime();
                //std::cout<<"cli time : "<<cli_time<<std::endl;
                response += "NEWPATH"+conversionIntString(size)+"*"+conversionIntString(hero->getId())+"*"+conversionFloatString(hero->getCenter().x)+"*"+conversionFloatString(hero->getCenter().y)+"*"+conversionLongString(cli_time)+"*"+conversionIntString(hero->isInFightingMode())+"*";
                for (int i = 0; i < size; i++) {
                    response += conversionFloatString(path[i].x)+"*"+conversionFloatString(path[i].y);
                    if (i != size - 1)
                        response += "*";
                }
                packet<<response;
                Network::sendTcpPacket(packet);
            //Update the path followed by the caracter. (For mouse movements)
            } else if (request == "GETCARPOS") {
                std::vector<Entity*> caracters = getWorld()->getEntities("E_MONSTER+E_HERO");
                for (unsigned int i = 0; i < caracters.size(); i++) {
                    Caracter* caracter = static_cast<Caracter*> (caracters[i]);
                    if (caracter->isMoving()) {
                        if (caracter->isMovingFromKeyboard()) {
                            Vec3f actualPos = Vec3f(caracter->getCenter().x, caracter->getCenter().y, 0);
                            sf::Int64 transferTime = user->getPingAvg();
                            sf::Int64 cli_time = user->getClientTime();
                            Vec3f newPos = actualPos + Vec3f(caracter->getDir().x, caracter->getDir().y, 0) * caracter->getSpeed() *  transferTime;
                            Ray ray(actualPos, newPos);
                            if (getWorld()->collide(caracter, ray)) {
                                newPos = actualPos;
                            }
                            std::string response = "NEWPOS"+conversionIntString(caracter->getId())+"*"+conversionLongString(transferTime)+"*"+conversionFloatString(newPos.x)+"*"+conversionFloatString(newPos.y)+"*"+conversionFloatString(newPos.z)+"*"+conversionLongString(cli_time)+"*"
                            +conversionIntString(caracter->isMoving())+"*"+conversionIntString(caracter->isInFightingMode())+"*"+conversionIntString(caracter->isAttacking())+"*"+conversionIntString(caracter->isAlive())+"*"+conversionIntString(caracter->getLife());
                            sf::Packet packet;
                            packet<<response;
                            user->sendUdpPacket(packet);
                        } else {
                            sf::Int64 transferTime = user->getPingAvg();
                            sf::Int64 cli_time = user->getClientTime();
                            Vec3f actualPos = Vec3f(caracter->getCenter().x, caracter->getCenter().y, 0);
                            Vec3f newPos = Computer::getPosOnPathFromTime(actualPos, caracter->getPath(), transferTime, caracter->getSpeed());
                            if (caracter->getPath().size() > 1 &&
                                newPos.computeDist(caracter->getPath()[caracter->getPath().size() - 1]) <= 1) {
                                newPos = caracter->getPath()[caracter->getPath().size() - 1];
                            }
                            std::string response = "NEWPOS"+conversionIntString(caracter->getId())+"*"+conversionLongString(transferTime)+"*"+conversionFloatString(newPos.x)+"*"+conversionFloatString(newPos.y)+"*"+conversionFloatString(newPos.z)+"*"+conversionLongString(cli_time)+"*"
                            +conversionIntString(caracter->isMoving())+"*"+conversionIntString(caracter->isInFightingMode())+"*"+conversionIntString(caracter->isAttacking())+"*"+conversionIntString(caracter->isAlive())+"*"+conversionIntString(caracter->getLife());
                            sf::Packet packet;
                            packet<<response;
                            user->sendUdpPacket(packet);
                        }
                    } else {
                        sf::Int64 transferTime = user->getPingAvg();
                        sf::Int64 cli_time = user->getClientTime();
                        Vec3f newPos = caracter->getCenter();
                        std::string response = "NEWPOS"+conversionIntString(caracter->getId())+"*"+conversionLongString(transferTime)+"*"+conversionFloatString(newPos.x)+"*"+conversionFloatString(newPos.y)+"*"+conversionFloatString(newPos.z)+"*"+conversionLongString(cli_time)+"*"
                        +conversionIntString(caracter->isMoving())+"*"+conversionIntString(caracter->isInFightingMode())+"*"+conversionIntString(caracter->isAttacking())+"*"+conversionIntString(caracter->isAlive())+"*"+conversionIntString(caracter->getLife());
                        sf::Packet packet;
                        packet<<response;
                        user->sendUdpPacket(packet);
                    }
                }
            } else if (request == "STOPCARMOVE") {
                sf::Int64 transferTime = user->getPingAvg();
                int id = conversionStringInt(infos[1]);
                Caracter* caracter = static_cast<Caracter*> (getWorld()->getEntity(id));
                sf::Int64 cli_time = user->getClientTime();
                if (caracter->isMoving()) {
                    sf::Int64 cli_time = conversionStringLong(infos[2]);
                    sf::Int64 delta_time = (user->getClientTime() - cli_time > 0) ? user->getClientTime() - cli_time : 0;
                    Vec3f newPos = caracter->getCenter();
                    if (caracter->isMovingFromKeyboard()) {
                        caracter->setMoving(false);
                        newPos = caracter->getCenter() - Vec3f(caracter->getDir().x, caracter->getDir().y, 0) * delta_time * caracter->getSpeed();
                        caracter->setCenter(newPos);
                    }
                    std::string response = "NEWPOS"+conversionIntString(caracter->getId())+"*"+conversionLongString(transferTime)+"*"+conversionFloatString(newPos.x)+"*"+conversionFloatString(newPos.y)+"*"+conversionFloatString(newPos.z)+"*"+conversionLongString(cli_time)+"*"
                        +conversionIntString(caracter->isMoving())+"*"+conversionIntString(caracter->isInFightingMode())+"*"+conversionIntString(caracter->isAttacking())+"*"+conversionIntString(caracter->isAlive())+"*"+conversionIntString(caracter->getLife());
                    SymEncPacket packet;
                    packet<<response;
                    user->sendTcpPacket(packet);
                }
            } else if (request == "ATTACK") {
                int id = conversionStringInt(infos[1]);
                Vec2f pos (conversionStringFloat(infos[2]), conversionStringFloat(infos[3]));
                sf::Int64 last_cli_time = conversionStringLong(infos[4]);
                sf::Int64 elapsedTime = user->getClientTime() - last_cli_time;
                std::vector<Entity*> monsters = getWorld()->getEntities("E_MONSTER");
                for (unsigned int i = 0; i < monsters.size(); i++) {
                    Monster* m = static_cast<Monster*>(monsters[i]);
                    if (m->isAlive()) {
                        sf::Int64 deltaTime;
                        bool updatePos = false;
                        if (m->isMoving()) {
                            updatePos = true;
                            deltaTime = elapsedTime;
                        } else if (!m->isMoving() && elapsedTime > m->getClkLastMove().getElapsedTime().asMicroseconds()) {
                            deltaTime = elapsedTime - (elapsedTime - m->getClkLastMove().getElapsedTime().asMicroseconds());
                            updatePos = true;
                        }
                        BoundingBox bx = m->getGlobalBounds();
                        if (updatePos) {
                            Vec2f pos = bx.getPosition();
                            Vec2f oldPos = Computer::getPosOnPathFromTime(m->getCenter(),m->getPath(),-elapsedTime,m->getSpeed());
                            Vec2f d = pos - oldPos;
                            bx.move(Vec3f(-d.x, -d.y, 0));
                        }
                        bx.setPosition(bx.getPosition().x, bx.getPosition().y, 0);
                        //std::cout<<"bx position : "<<bx.getPosition()<<"bs size : "<<bx.getSize()<<"pos : "<<pos<<std::endl;
                        if (bx.isPointInside(pos)) {
                            Caracter* hero = static_cast<Caracter*>(getWorld()->getEntity(id));
                            hero->setFightingMode(true);
                            hero->setFocusedCaracter(m);
                            hero->setMoving(false);
                            std::string response = "ATTACK"+conversionIntString(m->getId());
                            SymEncPacket packet;
                            packet<<response;
                            user->sendTcpPacket(packet);
                        }
                    }
                }
            } else if (request == "ALIVE") {
                int id = conversionStringInt(infos[1]);
                Caracter* hero = static_cast<Caracter*>(getWorld()->getEntity(id));
                hero->setAlive(true);
                hero->restartRegenHP();
                hero->setMoving(false);
                hero->setIsMovingFromKeyboard(false);
                hero->setAttacking(false);
                hero->setFightingMode(false);
                SymEncPacket packet;
                packet<<"ALIVE"+conversionIntString(hero->getId())+"*"+conversionFloatString(hero->getCenter().x)+"*"+conversionFloatString(hero->getCenter().y);
                user->sendTcpPacket(packet);
            } else if (request == "INV") {
                std::cout<<"connect"<<std::endl;
                caracter = entityFactory.make_entity<Hero>(user, "Sorrok", "Nagi", "M", "Map test", "Brain", "Green", "White","Normal","Novice", 1, entityFactory);
                BoundingVolume* bb2 = new BoundingBox(caracter->getGlobalBounds().getPosition().x, caracter->getGlobalBounds().getPosition().y + caracter->getGlobalBounds().getSize().y * 0.4f, 0,
                caracter->getGlobalBounds().getSize().x, caracter->getGlobalBounds().getSize().y * 0.25f, 0);
                caracter->setCollisionVolume(bb2);
                caracter->setCenter(Vec3f(0, 300, 300));
                Item noviceHead ("Novice Head", Item::HEAD, "All");
                Item noviceArmor ("Novice Armor", Item::ARMOR, "All");
                Item noviceGloves ("Novice Gloves", Item::GLOVES, "All");
                Item noviceBoots ("Novice Boots", Item::BOOTS, "ALL");
                Item noviceWeaponRight ("Novice Weapon Right", Item::WEAPON_RIGHT, "All");
                Item noviceWeaponLeft ("Novice Weapon Left", Item::WEAPON_LEFT, "ALL");
                static_cast<Hero*>(caracter)->addEquipment(noviceHead);
                static_cast<Hero*>(caracter)->addEquipment(noviceArmor);
                static_cast<Hero*>(caracter)->addEquipment(noviceGloves);
                static_cast<Hero*>(caracter)->addEquipment(noviceBoots);
                static_cast<Hero*>(caracter)->addEquipment(noviceWeaponRight);
                static_cast<Hero*>(caracter)->addEquipment(noviceWeaponLeft);
                Skill skill("LastHeal", 10, "All");
                static_cast<Hero*>(caracter)->addSkill(skill);
                SymEncPacket packet;
                packet<<"IDOK"+conversionIntString(caracter->getId());
                user->sendTcpPacket(packet);
                std::map<int, std::pair<std::vector<Item>, std::pair<sf::Time, sf::Time>>>::iterator it;
                for (it = itemsLooted.begin(); it != itemsLooted.end(); it++) {
                    int spotID = it->first;
                    std::vector<Item> items = it->second.first;
                    std::ostringstream oss;
                    OTextArchive oa(oss);
                    oa(items);
                    SymEncPacket packet;
                    std::string response = "ITEMS"+conversionIntString(spotID)+"*"+oss.str();
                    packet<<response;
                    Network::sendTcpPacket(packet);
                }
                getWorld()->addEntity(caracter);
            } else if (request == "CONNECT") {
                std::vector<Entity*> caracters = getWorld()->getEntities("E_HERO+E_MONSTER");
                for (unsigned int i = 0; i < caracters.size(); i++) {
                    if (dynamic_cast<Caracter*> (caracters[i])) {
                        Caracter* caracter = static_cast<Caracter*>(caracters[i]);
                        caracter->getRegen().clear();
                        caracter->getDamages().clear();
                    }
                }
                std::string pseudo = infos[1];
                std::string pswd = infos[2];
                /*sql::Statement* stmt = con->createStatement();
                sql::ResultSet* result = stmt->executeQuery("SELECT NAME, PASSWORD FROM HEROES WHERE NAME = "+pseudo+";");
                while(result->next()) {
                    if (result->getString("PASSWORD") == pswd) {
                        SymEncPacket packet;
                        packet<<"IDOK";
                        user->sendTcpPacket(packet);
                        std::cout<<"connect"<<std::endl;
                        caracter = new Hero(user, "Sorrok", "Nagi", "M", "Map test", "Brain", "Green", "White","Normal","Novice", 1);

                        BoundingVolume* bb2 = new BoundingBox(caracter->getGlobalBounds().getPosition().x, caracter->getGlobalBounds().getPosition().y + caracter->getGlobalBounds().getSize().y * 0.4f, 0,
                        caracter->getGlobalBounds().getSize().x, caracter->getGlobalBounds().getSize().y * 0.25f, 0);
                        caracter->setCollisionVolume(bb2);
                        caracter->setCenter(Vec3f(0, 300, 300));
                        getWorld()->addEntity(caracter);
                    }
                }
                delete stmt;
                delete result;*/
            } else if (request == "ADDLIFE")   {
                int id = conversionStringInt(infos[1]);
                Caracter* hero = static_cast<Caracter*>(getWorld()->getEntity(id));
                float potionAmount = conversionStringFloat(infos[2]);
                if (hero->getLife() + potionAmount > hero->getMaxLife()) {
                    hero->setLife(hero->getMaxLife());
                } else {
                    hero->setLife(hero->getLife() + potionAmount);
                }
                std::string response = "ADDLIFE"+conversionIntString(id)+"*"+conversionFloatString(potionAmount);
                SymEncPacket packet;
                packet<<response;
                user->sendTcpPacket(packet);
            } else if (request == "CANUSE") {
                int id = conversionStringInt(infos[1]);
                int type = conversionStringInt(infos[2]);
                Hero* caracter = static_cast<Hero*>(getWorld()->getEntity(id));
                std::map<Item::Type, std::vector<Item>> items = caracter->getInventory();
                std::map<Item::Type, std::vector<Item>>::iterator it;
                it = items.find(static_cast<Item::Type>(type));
                std::string response = "CANUSENO";
                if (it != items.end() && it->second.size() > 0) {
                    for (unsigned int i = 0; i < it->second.size(); i++) {
                        if (it->second[i].getType() == type && it->second[i].getRequiredClass().find(caracter->getClass()) != std::string::npos || it->second[i].getRequiredClass() == "All") {
                            response = "CANUSEOK";
                        }
                    }
                }
                //std::cout<<"can use : "<<response<<std::endl;
                SymEncPacket packet;
                packet<<response;
                user->sendTcpPacket(packet);
            } else if (request == "CANLAUNCH") {
                int id = conversionStringInt(infos[1]);
                std::string name = infos[2];
                Hero* caracter = static_cast<Hero*>(getWorld()->getEntity(id));
                std::vector<Skill> skills = caracter->getSkills();
                std::string response = "CANLAUNCHNO";
                for (unsigned int i = 0; i < skills.size(); i++) {
                    //std::cout<<"Ok ? "<<(skills[i].getRequiredClass().find(caracter->getClass()) != std::string::npos || skills[i].getRequiredClass() == "All")<<std::endl;
                    if (skills[i].getName() == name && caracter->getMana() >= skills[i].getManaCost() && skills[i].getRequiredClass().find(caracter->getClass()) != std::string::npos || skills[i].getRequiredClass() == "All") {
                        response = "CANLAUNCHOK";
                    }
                }
                SymEncPacket packet;
                packet<<response;
                user->sendTcpPacket(packet);
            } else if (request == "DROPITEM") {
                int id = conversionStringInt(infos[1]);
                std::istringstream iss(infos[2]);
                ITextArchive ia(iss);
                Item item;
                ia(item);
                Hero* hero = static_cast<Hero*>(getWorld()->getEntity(id));
                hero->addItem(item);
            }
        }
        if (getClock("LoopTime").getElapsedTime().asMilliseconds() < 100)
                sf::sleep(sf::milliseconds(100 - getClock("LoopTime").getElapsedTime().asMilliseconds()));
        std::vector<Entity*> caracters = getWorld()->getEntities("E_HERO+E_MONSTER");
        for (unsigned int i = 0; i < caracters.size(); i++) {
            Caracter* caracter = static_cast<Caracter*>(caracters[i]);
            /*if (caracter->getType() == "E_HERO") {
                std::cout<<"hero life : "<<caracter->getLife()<<std::endl;
            }*/
            if (caracter->isAlive()) {
                if (caracter->isMoving()) {
                    if (dynamic_cast<Hero*>(caracter) != nullptr && static_cast<Hero*>(caracter)->isMovingFromKeyboard()) {
                        sf::Int64 elapsedTime = getClock("LoopTime").getElapsedTime().asMicroseconds();
                        Vec3f actualPos = Vec3f(caracter->getCenter().x, caracter->getCenter().y, 0);
                        Vec3f newPos = actualPos + Vec3f(caracter->getDir().x, caracter->getDir().y, 0) * caracter->getSpeed() * elapsedTime;
                        Ray ray(actualPos, newPos);
                        if (getWorld()->collide(caracter, ray)) {
                            newPos = actualPos;
                        }
                        Vec3f d = newPos - actualPos;
                        Vec3f dir = d.normalize();
                        if (Vec2f(dir.x, dir.y) != caracter->getDir())
                            caracter->setDir(Vec2f(dir.x, dir.y));
                        getWorld()->moveEntity(caracter, d.x, d.y, d.y);
                    } else {
                        sf::Int64 elapsedTime = getClock("LoopTime").getElapsedTime().asMicroseconds();
                        Vec2f actualPos = caracter->getCenter();
                        Vec2f newPos = Computer::getPosOnPathFromTime(actualPos, caracter->getPath(), elapsedTime, caracter->getSpeed());
                        Vec2f d = newPos - actualPos;
                        Vec2f dir = d.normalize();
                        if (caracter->isInFightingMode() &&
                            Vec2f(caracter->getCenter().x, caracter->getCenter().y).computeDist(Vec2f(caracter->getFocusedCaracter()->getCenter().x, caracter->getFocusedCaracter()->getCenter().y)) <= caracter->getRange()) {
                            /*int delta = caracter->getRange() - Vec2f(caracter->getCenter().x, caracter->getCenter().y).computeDist(Vec2f(caracter->getFocusedCaracter()->getCenter().x, caracter->getFocusedCaracter()->getCenter().y));
                            newPos -= dir * delta;
                            d = newPos - actualPos;*/
                            caracter->setMoving(false);
                        }
                        if (caracter->isMoving()
                            && caracter->getPath().size() > 0
                            && newPos.computeDist(caracter->getPath()[caracter->getPath().size() - 1]) <= PATH_ERROR_MARGIN) {
                            caracter->setMoving(false);
                            newPos = caracter->getPath()[caracter->getPath().size() - 1];
                            d = newPos - actualPos;
                        }
                        if (dir != caracter->getDir())
                            caracter->setDir(dir);
                        getWorld()->moveEntity(caracter, d.x, d.y, d.y);
                    }
                    /*if (caracter->getType() == "E_HERO")
                        std::cout<<"caracter position : "<<caracter->getCenter()<<std::endl;*/
                }
                if (caracter->isInFightingMode() && !caracter->isMoving()) {
                    if (Vec2f(caracter->getCenter().x, caracter->getCenter().y).computeDist(Vec2f(caracter->getFocusedCaracter()->getCenter().x, caracter->getFocusedCaracter()->getCenter().y)) > caracter->getRange()) {
                        std::vector<Vec2f> path = getWorld()->getPath(caracter, caracter->getFocusedCaracter()->getCenter());
                        std::string response="";
                        int size = path.size();
                        caracter->setMoving(true);
                        caracter->setPath(path);
                        caracter->setIsMovingFromKeyboard(false);
                        caracter->setAttacking(false);
                        std::vector<User*> users = Network::getUsers();
                        if (caracter->getType() == "E_HERO")
                            std::cout<<"move from path"<<std::endl;
                        for (unsigned int i = 0; i < users.size(); i++) {
                            sf::Int64 cli_time = users[i]->getClientTime();
                            response = "NEWPATH"+conversionIntString(size)+"*"+conversionIntString(caracter->getId())+"*"+conversionFloatString(caracter->getCenter().x)+"*"+conversionFloatString(caracter->getCenter().y)+"*"+conversionLongString(cli_time)+"*"+conversionIntString(caracter->isInFightingMode())+"*";
                            for (int i = 0; i < size; i++) {
                                response += conversionFloatString(path[i].x)+"*"+conversionFloatString(path[i].y);
                                if (i != size - 1)
                                    response += "*";
                            }
                            SymEncPacket packet;
                            packet<<response;
                            users[i]->sendTcpPacket(packet);
                        }
                    } else {
                        if(dynamic_cast<Monster*>(caracter->getFocusedCaracter())) {
                            Monster* m = static_cast<Monster*>(caracter->getFocusedCaracter());
                            if (!m->isAgressif() && !m->isInFightingMode())  {
                                m->setFightingMode(true);
                                m->setFocusedCaracter(caracter);
                                m->setSavedPos(Vec3f(m->getCenter().x, m->getCenter().y, 0));
                                SymEncPacket packet;
                                packet<<"ENTERINFIGHTINGMODE"<<conversionIntString(m->getId());
                                Network::sendTcpPacket(packet);
                            }
                        }
                        if (!caracter->isAttacking()) {
                            caracter->setAttacking(true);
                            caracter->restartAttackSpeed();
                            SymEncPacket packet;
                            std::string response = "SETATTACKING"+conversionIntString(caracter->getId())+"*"+conversionIntString(caracter->getFocusedCaracter()->getId());
                            std::cout<<"response : "<<response<<std::endl;
                            packet<<response;
                            Network::sendTcpPacket(packet);
                        }
                        /*if (caracter->getDamages().empty()) {
                            std::vector<int> damages;
                            int nb = 10.f / caracter->getAttackSpeed();
                            for (unsigned int i = 0; i < nb; i++) {
                                int damage = 0;
                                int flee = Math::random(0, 100);
                                if (flee > caracter->getFocusedCaracter()->getFleeRate()) {
                                    int critical = Math::random(0, 100);
                                    int atk = 0;
                                    if (critical <= caracter->getCriticalChanceRate()) {
                                        atk = Math::random(caracter->getAttackMin(), caracter->getAttackMax());
                                        atk += atk / 100 * caracter->getCriticalAddDamagesRate();
                                    } else {
                                        atk = Math::random(caracter->getAttackMin(), caracter->getAttackMax());
                                    }
                                    int def = Math::random(caracter->getFocusedCaracter()->getDefMin(), caracter->getFocusedCaracter()->getDefMax());
                                    damage = atk - def;
                                }
                                damage = (damage < 0) ? 0 : damage;
                                damages.push_back(damage);
                            }
                            caracter->setDamages(damages);
                            int size = damages.size();
                            std::string response = "DMG"+conversionIntString(caracter->getId())+conversionIntString(size)+"*";
                            for(unsigned int i = 0; i < damages.size(); i++) {
                                response += conversionIntString(damages[i])+"*";
                            }
                            for (unsigned int i = 0; i < Network::getUsers().size(); i++) {
                                std::string userResponse = response;
                                userResponse+=conversionLongString(Network::getUsers()[i]->getClientTime())+"*"+conversionIntString(caracter->getLife());
                                SymEncPacket packet;
                                packet<<userResponse;
                                Network::getUsers()[i]->sendTcpPacket(packet);
                            }
                        }*/
                        if (caracter->getTimeOfLastAttack().asSeconds() >= caracter->getAttackSpeed()) {
                            caracter->restartAttackSpeed();
                            /*int dmg = caracter->getDamages().back();
                            caracter->getDamages().pop_back();
                            caracter->attackFocusedCaracter(dmg);
                            if (!caracter->getFocusedCaracter()->isAlive()) {
                                SymEncPacket packet;
                                packet<<"DEATH"+conversionIntString(caracter->getFocusedCaracter()->getId());
                                std::cout<<"caracter : "<<caracter->getType()<<" is death"<<std::endl;
                                Network::sendTcpPacket(packet);
                                if (caracter->getFocusedCaracter()->getType() == "E_MONSTER") {
                                    std::vector<Item> items = static_cast<Monster*>(caracter->getFocusedCaracter())->getLootedItems();
                                    std::ostringstream oss;
                                    OTextArchive ota(oss);
                                    ota(items);
                                    SymEncPacket packet;
                                    std::string response = "ITEMS"+oss.str();
                                    packet<<response;
                                    Network::sendTcpPacket(packet);
                                    static_cast<Hero*>(caracter)->up(static_cast<Monster*>(caracter->getFocusedCaracter())->getXp());

                                }
                            }*/
                            int damage = 0;
                            int flee = Math::random(0, 100);
                            if (flee > caracter->getFocusedCaracter()->getFleeRate()) {
                                int critical = Math::random(0, 100);
                                int atk = 0;
                                if (critical <= caracter->getCriticalChanceRate()) {
                                    atk = Math::random(caracter->getAttackMin(), caracter->getAttackMax());
                                    atk += atk / 100 * caracter->getCriticalAddDamagesRate();
                                } else {
                                    atk = Math::random(caracter->getAttackMin(), caracter->getAttackMax());
                                }
                                int def = Math::random(caracter->getFocusedCaracter()->getDefMin(), caracter->getFocusedCaracter()->getDefMax());
                                damage = atk - def;
                            }
                            damage = (damage < 0) ? 0 : damage;
                            //std::cout<<"damage : "<<damage<<std::endl;
                            caracter->attackFocusedCaracter(damage);
                            SymEncPacket packet;
                            packet<<"TAKE_DMG"+conversionIntString(caracter->getFocusedCaracter()->getId())+"*"+conversionIntString(damage);
                            Network::sendTcpPacket(packet);
                            if (!caracter->getFocusedCaracter()->isAlive()) {
                                SymEncPacket packet;
                                packet<<"DEATH"+conversionIntString(caracter->getFocusedCaracter()->getId());
                                //std::cout<<"caracter : "<<caracter->getFocusedCaracter()->getType()<<" is death"<<std::endl;
                                Network::sendTcpPacket(packet);
                                if (caracter->getFocusedCaracter()->getType() == "E_MONSTER") {
                                    std::vector<Item> items = static_cast<Monster*>(caracter->getFocusedCaracter())->getLootedItems();
                                    itemsLooted.insert(std::make_pair(spotID, std::make_pair(items, std::make_pair(Application::getTimeClk().getElapsedTime(), sf::seconds(120)))));
                                    std::ostringstream oss;
                                    OTextArchive ota(oss);
                                    ota(items);
                                    SymEncPacket packet;
                                    std::string response = "ITEMS"+conversionIntString(spotID)+"*"+oss.str();
                                    spotID++;
                                    packet<<response;
                                    Network::sendTcpPacket(packet);
                                    static_cast<Hero*>(caracter)->up(static_cast<Monster*>(caracter->getFocusedCaracter())->getXp());

                                }
                            }
                        }
                    }
                }
                if (!caracter->isInFightingMode() && !caracter->isMoving() && !caracter->isAttacked()) {
                    /*if (caracter->getRegen().empty()) {
                        std::vector<int> regen;
                        int nb = 10.f / caracter->getRegenHpSpeed();
                        std::string response = "RGN"+conversionIntString(caracter->getId())+conversionIntString(nb)+"*";
                        for (unsigned int i = 0; i < nb; i++) {
                            int rgn = Math::random(caracter->getRegenHpAmountMin(), caracter->getRegenHpAmountMax());
                            regen.push_back(rgn);
                            response += conversionIntString(rgn)+"*";
                        }
                        caracter->setRegen(regen);
                        for (unsigned int i = 0; i < Network::getUsers().size(); i++) {
                            std::string userResponse = response;
                            userResponse+=conversionLongString(Network::getUsers()[i]->getClientTime())+"*"+conversionIntString(caracter->getLife());
                            SymEncPacket packet;
                            packet<<userResponse;
                            Network::getUsers()[i]->sendTcpPacket(packet);
                        }
                    }*/
                    if (caracter->getTimeOfLastHpRegen().asSeconds() >= caracter->getRegenHpSpeed()) {
                        caracter->restartRegenHP();
                        /*int rgn = caracter->getRegen().back();
                        caracter->getRegen().pop_back();
                        if (caracter->getLife() + rgn >= caracter->getMaxLife()) {
                            caracter->setLife(caracter->getMaxLife());
                        } else {
                            caracter->setLife(caracter->getLife() + rgn);
                        }*/
                        int rgn = Math::random(caracter->getRegenHpAmountMin(), caracter->getRegenHpAmountMax());
                        if (caracter->getLife() + rgn >= caracter->getMaxLife()) {
                            caracter->setLife(caracter->getMaxLife());
                        } else {
                            caracter->setLife(caracter->getLife() + rgn);
                        }
                        SymEncPacket packet;
                        packet<<"RGN_LIFE"+conversionIntString(caracter->getId())+"*"+conversionIntString(rgn);
                        Network::sendTcpPacket(packet);
                    }
                    /*if (caracter->getRegenMana().empty()) {
                        std::vector<int> regen;
                        int nb = 10.f / caracter->getRegenManaSpeed();
                        std::string response = "RGNMANA"+conversionIntString(caracter->getId())+conversionIntString(nb)+"*";
                        for (unsigned int i = 0; i < nb; i++) {
                            int rgn = Math::random(caracter->getRegenManaAmountMin(), caracter->getRegenManaAmountMax());
                            regen.push_back(rgn);
                            response += conversionIntString(rgn)+"*";
                        }
                        caracter->setRegenMana(regen);
                        for (unsigned int i = 0; i < Network::getUsers().size(); i++) {
                            std::string userResponse = response;
                            userResponse+=conversionLongString(Network::getUsers()[i]->getClientTime())+"*"+conversionIntString(caracter->getMana());
                            SymEncPacket packet;
                            packet<<userResponse;
                            Network::getUsers()[i]->sendTcpPacket(packet);
                        }
                    }*/
                    if (caracter->getTimeOfLastManaRegen().asSeconds() >= caracter->getRegenManaSpeed()) {
                        caracter->restartRegenMana();
                        /*int rgn = caracter->getRegenMana().back();
                        caracter->getRegenMana().pop_back();
                        if (caracter->getMana() + rgn >= caracter->getManaMax()) {
                            caracter->setMana(caracter->getManaMax());
                        } else {
                            caracter->setMana(caracter->getMana() + rgn);
                        }*/
                        int rgn = Math::random(caracter->getRegenManaAmountMin(), caracter->getRegenManaAmountMax());
                        if (caracter->getMana() + rgn >= caracter->getManaMax()) {
                            caracter->setMana(caracter->getManaMax());
                        } else {
                            caracter->setMana(caracter->getMana() + rgn);
                        }
                        SymEncPacket packet;
                        packet<<"RGN_MANA"+conversionIntString(caracter->getId())+"*"+conversionIntString(rgn);
                        Network::sendTcpPacket(packet);
                    }
                }
            }
        }
        std::vector<Entity*> monsters = getWorld()->getEntities("E_MONSTER");
        for (unsigned int i = 0; i < monsters.size(); i++) {
            Monster* monster = static_cast<Monster*>(monsters[i]);

            if (monster->isAlive()) {
                if (!monster->isInFightingMode() && !monster->isMoving() && monster->getClkLastMove().getElapsedTime().asMicroseconds() >= monster->getTimeUntilNextMove().asMicroseconds()) {
                    Vec3f finalPos;
                    do {
                        finalPos = monster->respawn();
                    } while (getWorld()->collide(monster, finalPos));
                    std::vector<Vec2f> path = getWorld()->getPath(monster, finalPos);
                    monster->setPath(path);
                    monster->setMoving(true);
                    sf::Time time1 = monster->getMoveTimeInterval1();
                    sf::Time time2 = monster->getMoveTimeInterval2();
                    sf::Time time3 = sf::seconds(Math::random(time1.asSeconds(), time2.asSeconds()));
                    monster->setTimeUntilNextMove(time3);
                    monster->getClkLastMove().restart();
                    std::vector<User*> users = Network::getUsers();
                    for (unsigned int i = 0; i < users.size(); i++) {
                        std::string response="";
                        int size = path.size();
                        sf::Int64 cli_time = users[i]->getClientTime();
                        response = "NEWPATH"+conversionIntString(size)+"*"+conversionIntString(monster->getId())+"*"+conversionFloatString(monster->getCenter().x)+"*"+conversionFloatString(monster->getCenter().y)+"*"+conversionLongString(cli_time)+"*"+conversionIntString(monster->isInFightingMode())+"*";
                        for (int i = 0; i < size; i++) {
                            response += conversionFloatString(path[i].x)+"*"+conversionFloatString(path[i].y);
                            if (i != size - 1)
                                response += "*";
                        }
                        SymEncPacket packet;
                        packet<<response;
                        users[i]->sendTcpPacket(packet);
                    }
                } else if (monster->isInFightingMode() && monster->getSavedPos().computeDist(Vec2f(monster->getCenter().x, monster->getCenter().y)) >= 200) {
                    std::vector<Vec2f> path = getWorld()->getPath(monster, monster->getSavedPos());
                    monster->setPath(path);
                    monster->setMoving(true);
                    monster->setFightingMode(false);
                    monster->setAttacking(false);
                    std::vector<User*> users = Network::getUsers();
                    for (unsigned int i = 0; i < users.size(); i++) {
                        std::string response="";
                        int size = path.size();
                        sf::Int64 cli_time = users[i]->getClientTime();
                        response = "NEWPATH"+conversionIntString(size)+"*"+conversionIntString(monster->getId())+"*"+conversionFloatString(monster->getCenter().x)+"*"+conversionFloatString(monster->getCenter().y)+"*"+conversionLongString(cli_time)+"*"+conversionIntString(monster->isInFightingMode())+"*";
                        for (int i = 0; i < size; i++) {
                            response += conversionFloatString(path[i].x)+"*"+conversionFloatString(path[i].y);
                            if (i != size - 1)
                                response += "*";
                        }
                        SymEncPacket packet;
                        packet<<response;
                        users[i]->sendTcpPacket(packet);
                        packet.clear();
                        packet<<"LEAVEINFIGHTINGMODE"<<conversionIntString(monster->getId());
                        users[i]->sendTcpPacket(packet);
                    }
                }
            } else {
                if (monster->getTimeSinceLastRespawn() >= monster->getTimeBeforeLastRespawn()) {
                    std::string response="ALIVE"+conversionIntString(monster->getId())+"*"+conversionFloatString(monster->getCenter().x)+"*"+conversionFloatString(monster->getCenter().y);
                    SymEncPacket packet;
                    packet<<response;
                    std::vector<User*> users = Network::getUsers();
                    for (unsigned int i = 0; i < users.size(); i++) {
                        users[i]->sendTcpPacket(packet);
                    }
                    Vec3f pos = monster->respawn();
                    monster->setCenter(pos);
                    monster->getClkLastMove().restart();
                    monster->restartRespawn();
                    monster->setAlive(true);
                    monster->restartRegenHP();
                    monster->setFightingMode(false);
                    monster->setAttacking(false);
                    monster->setMoving(false);
                }
            }
        }
        std::map<int, std::pair<std::vector<Item>, std::pair<sf::Time, sf::Time>>>::iterator it;
        for (it = itemsLooted.begin(); it != itemsLooted.end();) {
            sf::Time elapsedTime = Application::getTimeClk().getElapsedTime() - it->second.second.first;
            if (elapsedTime >= it->second.second.second) {
                int spotID = it->first;
                it = itemsLooted.erase(it);
                SymEncPacket packet;
                std::string response = "REMOVEITEM"+conversionIntString(spotID);
                Network::sendTcpPacket(packet);
            } else {
                it++;
            }
        }
    }
    void MyAppli::onDisconnected(User* user) {
        std::vector<Entity*> heroes = getWorld()->getEntities("E_HERO");
        for (unsigned int i = 0; i < heroes.size(); i++) {
            if (user == static_cast<Hero*>(heroes[i])->getUser()) {
                if (static_cast<Hero*>(heroes[i])->getFocusedCaracter() != nullptr) {
                    static_cast<Hero*>(heroes[i])->getFocusedCaracter()->setFightingMode(false);
                    static_cast<Hero*>(heroes[i])->getFocusedCaracter()->setAttacking(false);
                }
                getWorld()->deleteEntity(heroes[i]);
            }
        }
        heroes = getWorld()->getEntities("E_HERO");
    }
    MyAppli::~MyAppli() {
        stop();
        for (unsigned int i = 0; i < getNbWindows(); i++) {
           RenderWindow* window = &getRenderWindow(i);
           delete window;
        }
        //delete con;
    }
}
