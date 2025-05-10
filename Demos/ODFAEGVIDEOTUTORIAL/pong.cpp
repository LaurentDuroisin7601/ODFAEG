#include "pong.hpp"
using namespace odfaeg::network;
using namespace odfaeg::core;
using namespace odfaeg::graphic;
using namespace odfaeg::math;
using namespace odfaeg::physic;
void Pong::onInit() {
    Network::startSrv(10000, 10001);
}
void Pong::onExec() {
    std::map<int, Partie*>::iterator it;
    for (it = parties.begin(); it != parties.end();) {
        if (!it->second->isRunning()) {
            World::setCurrentEntityManager("PONG_MAP"+conversionIntString(it->second->id()));
            World::removeEntity(&it->second->player1());
            World::removeEntity(&it->second->player2());
            World::removeEntityManager("PONG_MAP"+conversionIntString(it->second->id()));
            it->second->player1().setPartyId(-1);
            it->second->player2().setPartyId(-1);
            delete it->second;
            it = parties.erase(it);
        } else {
            it++;
        }
    }
    if (Network::hasRequest()) {
        User* user;
        std::string request = Network::getLastRequest(&user);
        std::vector<std::string> infos = split(request, "*");
        if (infos[0] == "CONNECT") {
            std::string pseudo = infos[1];
            Player* player = new Player(*user, pseudo, Vec2f(0, 0));
            bool used = false;
            std::vector<std::string> pseudos;
            std::map<int, Player*>::iterator it;
            for (it = players.begin(); it != players.end(); it++) {
                if (pseudo == it->second->pseudo())
                   used = true;
                pseudos.push_back(it->second->pseudo());
            }
            std::string message = "CONNECTED";
            if (!used)  {
                players.insert(std::make_pair(player->getId(), player));
                message += "OK*";
            } else {
                delete player;
                message+="NOTOK";
            }
            if (!used) {
                message += conversionIntString(player->getId())+"*"+player->pseudo()+"*"+conversionIntString(pseudos.size());
                if (pseudos.size() > 0)
                    message += "*";
                for (unsigned int i = 0; i < pseudos.size(); i++) {
                    message += pseudos[i];
                    if (i != pseudos.size() - 1)
                        message += "*";
                }
            }
            SymEncPacket packet;
            packet<<message;
            Network::sendTcpPacket(packet);
        }
        if (infos[0] == "INVITE") {
            std::string pseudo1 = infos[1];
            int id = conversionStringInt(infos[2]);
            std::string pseudo2 = infos[3];
            std::map<int, Player*>::iterator it2;
            for (it2 = players.begin(); it2 != players.end(); it2++) {
                if (pseudo2 == it2->second->pseudo()) {
                    std::string message = "INVITE"+conversionIntString(id)+"*"+pseudo1;
                    SymEncPacket packet;
                    packet<<message;
                    it2->second->user().sendTcpPacket(packet);
                }
            }
        }
        if (infos[0] == "CREATE_PARTY") {
            int idPlayer1 = conversionStringInt(infos[1]);
            int idPlayer2 = conversionStringInt(infos[2]);
            Ball* ball = new Ball();
            Partie* partie = new Partie(*players[idPlayer1], *players[idPlayer2], *ball);
            Map* map = new Map(nullptr, "PONG_MAP"+conversionIntString(partie->id()), 50, 50);
            World::addEntityManager(map);
            World::setCurrentEntityManager("PONG_MAP"+conversionIntString(partie->id()));
            World::addEntity(ball);
            parties.insert(std::make_pair(partie->id(), partie));
            players[idPlayer1]->setPartyId(partie->id());
            players[idPlayer2]->setPartyId(partie->id());
            ball->setPartyId(partie->id());
            players[idPlayer1]->setPosition(Vec2f(0, 275));
            players[idPlayer2]->setPosition(Vec2f(790, 275));
            World::addEntity(players[idPlayer1]);
            World::addEntity(players[idPlayer2]);
            Bar* top = new Bar(Vec2f(0, 0), Vec2f(800, 10));
            Bar* bottom = new Bar(Vec2f(0, 590), Vec2f(800, 10));
            World::addEntity(top);
            World::addEntity(bottom);
            std::string message1 = "PARTYINFOS1"+conversionIntString(partie->id())+"*"+conversionFloatString(players[idPlayer1]->getPosition().x)+"*"+conversionFloatString(players[idPlayer1]->getPosition().y)
                                    +"*"+conversionFloatString(players[idPlayer2]->getPosition().x)+"*"+conversionFloatString(players[idPlayer2]->getPosition().y);
            std::string message2 = "PARTYINFOS2"+conversionIntString(partie->id())+"*"+conversionIntString(players[idPlayer1]->getId())+"*"+players[idPlayer1]->pseudo()
                                    +"*"+conversionFloatString(players[idPlayer1]->getPosition().x)+"*"+conversionFloatString(players[idPlayer1]->getPosition().y)
                                    +"*"+conversionFloatString(players[idPlayer2]->getPosition().x)+"*"+conversionFloatString(players[idPlayer2]->getPosition().y);
            SymEncPacket packet1, packet2;
            packet1<<message1;
            packet2<<message2;
            players[idPlayer1]->user().sendTcpPacket(packet1);
            players[idPlayer2]->user().sendTcpPacket(packet2);
        }
        if (infos[0] == "MOVE_DOWN") {
            int idPlayer = conversionStringInt(infos[1]);
            sf::Int64 lastCliTime = conversionStringLong(infos[2]);
            int idParty = conversionStringInt(infos[3]);
            players[idPlayer]->setDir(Vec2f(0, 1));
            players[idPlayer]->setMoving(true);
            sf::Int64 elapsedTime = user->getClientTime() - lastCliTime;
            movePlayer(players[idPlayer], elapsedTime);
            sf::Int64 lastSrvTime = Application().getTimeClk().getElapsedTime().asMicroseconds() - elapsedTime;
            Ball& ball = parties[idParty]->ball();
            sf::Int64 remainingTime = checkLastBallPos(&ball, lastSrvTime, elapsedTime);
            moveBall(&ball, remainingTime);
            int id1 = parties[idParty]->player1().getId();
            int id2 = parties[idParty]->player2().getId();
            Vec2f player1Pos = parties[idParty]->player1().getCenter();
            Vec2f player2Pos = parties[idParty]->player2().getCenter();
            Vec2f player1Dir = parties[idParty]->player1().dir();
            Vec2f player2Dir = parties[idParty]->player2().dir();
            Vec2f ballPos = parties[idParty]->ball().getCenter();
            std::string message = "UPDATE_POSITIONS"+conversionIntString(id1)+"*"+conversionFloatString(player1Pos.x)
                                    +"*"+conversionFloatString(player1Pos.y)+"*"+conversionIntString(player1Dir.y)
                                    +"*"+conversionIntString(id2)+"*"+conversionFloatString(player2Pos.x)+"*"+conversionFloatString(player2Pos.y)
                                    +"*"+conversionIntString(player2Dir.y)+"*"+conversionFloatString(ballPos.x)+"*"+conversionFloatString(ballPos.y)
                                    +"*"+conversionLongString(user->getPingAvg())+"*"+conversionIntString(parties[idParty]->player1().isMoving())
                                    +"*"+conversionIntString(parties[idParty]->player2().isMoving())+"*"+conversionFloatString(parties[idParty]->ball().dir().x)
                                    +"*"+conversionFloatString(parties[idParty]->ball().dir().y);
            SymEncPacket packet;
            packet<<message;
            Network::sendTcpPacket(packet);

        }
        if (infos[0] == "MOVE_UP") {
            int idPlayer = conversionStringInt(infos[1]);
            sf::Int64 lastCliTime = conversionStringLong(infos[2]);
            int idParty = conversionStringInt(infos[3]);
            players[idPlayer]->setDir(Vec2f(0, -1));
            players[idPlayer]->setMoving(true);
            sf::Int64 elapsedTime = user->getClientTime() - lastCliTime;
            movePlayer(players[idPlayer], elapsedTime);
            sf::Int64 lastSrvTime = Application().getTimeClk().getElapsedTime().asMicroseconds() - elapsedTime;
            Ball& ball = parties[idParty]->ball();
            sf::Int64 remainingTime = checkLastBallPos(&ball, lastSrvTime, elapsedTime);
            moveBall(&ball, remainingTime);
            int id1 = parties[idParty]->player1().getId();
            int id2 = parties[idParty]->player2().getId();
            Vec2f player1Pos = parties[idParty]->player1().getCenter();
            Vec2f player2Pos = parties[idParty]->player2().getCenter();
            Vec2f player1Dir = parties[idParty]->player1().dir();
            Vec2f player2Dir = parties[idParty]->player2().dir();
            Vec2f ballPos = parties[idParty]->ball().getCenter();
            std::string message = "UPDATE_POSITIONS"+conversionIntString(id1)+"*"+conversionFloatString(player1Pos.x)
                                    +"*"+conversionFloatString(player1Pos.y)+"*"+conversionIntString(player1Dir.y)
                                    +"*"+conversionIntString(id2)+"*"+conversionFloatString(player2Pos.x)+"*"+conversionFloatString(player2Pos.y)
                                    +"*"+conversionIntString(player2Dir.y)+"*"+conversionFloatString(ballPos.x)+"*"+conversionFloatString(ballPos.y)
                                    +"*"+conversionLongString(user->getPingAvg())+"*"+conversionIntString(parties[idParty]->player1().isMoving())
                                    +"*"+conversionIntString(parties[idParty]->player2().isMoving())+"*"+conversionFloatString(parties[idParty]->ball().dir().x)
                                    +"*"+conversionFloatString(parties[idParty]->ball().dir().y);
            SymEncPacket packet;
            packet<<message;
            Network::sendTcpPacket(packet);
        }
        if (infos[0] == "UPDATE_POSITIONS") {
            int idParty = conversionStringInt(infos[1]);
            int id1 = parties[idParty]->player1().getId();
            int id2 = parties[idParty]->player2().getId();
            Vec2f player1Pos = parties[idParty]->player1().getCenter();
            Vec2f player2Pos = parties[idParty]->player2().getCenter();
            Vec2f player1Dir = parties[idParty]->player1().dir();
            Vec2f player2Dir = parties[idParty]->player2().dir();
            Vec2f ballPos = parties[idParty]->ball().getCenter();
            std::string message = "UPDATE_POSITIONS"+conversionIntString(id1)+"*"+conversionFloatString(player1Pos.x)
                                    +"*"+conversionFloatString(player1Pos.y)+"*"+conversionIntString(player1Dir.y)
                                    +"*"+conversionIntString(id2)+"*"+conversionFloatString(player2Pos.x)+"*"+conversionFloatString(player2Pos.y)
                                    +"*"+conversionIntString(player2Dir.y)+"*"+conversionFloatString(ballPos.x)+"*"+conversionFloatString(ballPos.y)
                                    +"*"+conversionLongString(user->getPingAvg())+"*"+conversionIntString(parties[idParty]->player1().isMoving())
                                    +"*"+conversionIntString(parties[idParty]->player2().isMoving())+"*"+conversionFloatString(parties[idParty]->ball().dir().x)
                                    +"*"+conversionFloatString(parties[idParty]->ball().dir().y);
            sf::Packet packet;
            packet<<message;
            Network::sendUdpPacket(packet);
        }
        if (infos[0] == "STOP_MOVING") {
            int idPlayer = conversionStringInt(infos[1]);
            sf::Int64 lastCliTime = conversionStringLong(infos[2]);
            int idParty = conversionStringInt(infos[3]);
            sf::Int64 elapsedTime = user->getClientTime() - lastCliTime;
            movePlayer(players[idPlayer], -elapsedTime);
            players[idPlayer]->setDir(Vec2f(0, 0));
            players[idPlayer]->setMoving(false);
            sf::Int64 lastSrvTime = Application().getTimeClk().getElapsedTime().asMicroseconds() - elapsedTime;
            Ball& ball = parties[idParty]->ball();
            sf::Int64 remainingTime = checkLastBallPos(&ball, lastSrvTime, elapsedTime);
            if (idPlayer == parties[idParty]->player1().getId()) {
                std::string message = "STOP"+conversionIntString(idPlayer)
                                        +"*"+conversionFloatString(parties[idParty]->player1().getCenter().x)
                                        +"*"+conversionFloatString(parties[idParty]->player1().getCenter().y);
                SymEncPacket packet;
                packet<<message;
                parties[idParty]->player2().user().sendTcpPacket(packet);
            } else if (idPlayer == parties[idParty]->player2().getId()) {
                std::string message = "STOP"+conversionIntString(idPlayer)
                                        +"*"+conversionFloatString(parties[idParty]->player2().getCenter().x)
                                        +"*"+conversionFloatString(parties[idParty]->player2().getCenter().y);
                SymEncPacket packet;
                packet<<message;
                parties[idParty]->player1().user().sendTcpPacket(packet);
            }
            moveBall(&ball, remainingTime);
        }
    }
    for (it = parties.begin(); it != parties.end(); it++) {
        World::setCurrentEntityManager("PONG_MAP"+it->second->id());
        std::vector<Entity*> players = World::getEntities("E_PLAYER");
        for (unsigned int i = 0; i < players.size(); i++) {
            Player* player = static_cast<Player*>(players[i]);
            if (player->isMoving()) {
                sf::Int64 elapsedTime = getClock("LoopTime").getElapsedTime().asMicroseconds();
                movePlayer(player, elapsedTime);
            }
        }
        std::vector<Entity*> balls = World::getEntities("E_BALL");
        for (unsigned int i = 0; i < balls.size(); i++) {
            sf::Int64 elapsedTime = getClock("LoopTime").getElapsedTime().asMicroseconds();
            Ball* ball = static_cast<Ball*>(balls[i]);
            moveBall(ball, elapsedTime);
        }
    }
}
sf::Int64 Pong::checkLastBallPos(Ball* ball, sf::Int64 lastSrvTime, sf::Int64 elapsedTime) {
    Vec2f prevBallPos = ball->getCenter() - ball->dir() * ball->speed() * elapsedTime;
    Vec2f lastBallPos = (lastSrvTime < ball->getHistoric().time) ? ball->getHistoric().pos : prevBallPos;
    Vec2f v1 = ball->getCenter() - lastBallPos;
    Vec2f v2 = ball->getCenter() - prevBallPos;
    Vec2f d = ball->getCenter() - lastBallPos;
    World::moveEntity(ball, d.x, d.y, 0);
    float ratio = v1.magnSquared() / v2.magnSquared();
    return elapsedTime * ratio;
}
void Pong::movePlayer(Player* player, sf::Int64 elapsedTime) {
     Vec3f actualPos = player->getCenter();
     Vec3f newPos = actualPos + player->dir() * elapsedTime * player->speed();
     Ray ray (actualPos, newPos);
     if (!World::collide(player, ray)) {
         Vec3f d = newPos - actualPos;
         World::moveEntity(player, d.x, d.y, 0);
     }
}
void Pong::moveBall(Ball* ball, sf::Int64 elapsedTime) {
    Vec3f actualPos = ball->getCenter();
    Vec3f newPos = actualPos + ball->dir() * ball->speed() * elapsedTime;
    Ray ballPath (actualPos, newPos);
    if (World::collide(ball, ballPath)) {
        CollisionResultSet::Info info = CollisionResultSet::popCollisionInfo();
        Entity* entity = info.entity;
        Vec3f near, far;
        std::unique_ptr<BoundingVolume> bv = ball->getCollisionVolume()->clone();
        bv->move(info.center - bv->getCenter());
        bv->intersectsWhere(ballPath, near, far, info);
        Vec2f tmpPos = far - ball->dir() * (info.mtu.magnitude() + ball->getCollisionVolume()->getSize().x * 0.5f + EPSILON);
        Vec2f v1 = tmpPos - actualPos;
        Vec2f v2 = newPos - actualPos;
        float ratio = v1.magnSquared() / v2.magnSquared();
        sf::Int64 remainingTime = elapsedTime - elapsedTime * ratio;
       /* std::cout<<near<<info.mtu<<actualPos<<tmpPos<<newPos<<std::endl;
        std::cout<<elapsedTime<<" "<<remainingTime<<std::endl;*/
        Vec2f n = static_cast<BoundingBox*>(entity->getCollisionVolume())->getEdgeNormals()[info.nearestEdgeIndex1];
        Vec2f invDir = -ball->dir();
        float dp = n.dot(invDir);
        n = n * dp;
        Ball::Historic historic;
        historic.pos = ball->getCenter();
        historic.time = Application::getTimeClk().getElapsedTime().asMicroseconds();
        ball->registerBallStates(historic);
        Vec2f newDir = invDir + (n - invDir) * 2;
        ball->updateDir(newDir);
        Vec2f finalPos = tmpPos + ball->dir() * ball->speed() * remainingTime;
        Vec2f d = finalPos - actualPos;
        World::moveEntity(ball, d.x, d.y, 0);
    } else {
        Vec2f d = newPos - actualPos;
        World::moveEntity(ball, d.x, d.y, 0);
        //std::cout<<"new pos : "<<newPos;
    }
    if (newPos.x <= -100) {
        Partie* partie = parties[ball->partyId()];
        partie->player2().increaseScore();
        ball->setCenter(Vec2f(400, 300));
        ball->updateDir(Vec2f(-1, 1).normalize());
        if (partie->player2().score() >= 10) {
            std::string message = "player : "+ partie->player2().pseudo()+" win!";
            SymEncPacket packet;
            packet<<message;
            Network::sendTcpPacket(packet);
            partie->player1().resetScore();
            partie->player2().resetScore();
            partie->finished();
        }
    } else if (newPos.x >= 900) {
        Partie* partie = parties[ball->partyId()];
        partie->player1().increaseScore();
        ball->setCenter(Vec2f(400, 300));
        ball->updateDir(Vec2f(1, 1).normalize());
        if (partie->player1().score() >= 10) {
            std::string message = "player : "+ partie->player1().pseudo()+" win!";
            SymEncPacket packet;
            packet<<message;
            Network::sendTcpPacket(packet);
            partie->player1().resetScore();
            partie->player2().resetScore();
            partie->finished();
        }
    }
}
void Pong::onDisconnected(User* user) {
    std::map<int, Partie*>::iterator it;
    for (it = parties.begin(); it != parties.end();) {
        if (&it->second->player1().user() == user || &it->second->player2().user() == user) {
            World::setCurrentEntityManager("PONG_MAP"+conversionIntString(it->second->id()));
            World::removeEntity(&it->second->player1());
            World::removeEntity(&it->second->player2());
            World::removeEntityManager("PONG_MAP"+conversionIntString(it->second->id()));
            it->second->player1().setPartyId(-1);
            it->second->player2().setPartyId(-1);
            delete it->second;
            it = parties.erase(it);
        } else {
            it++;
        }
    }
    std::map<int, Player*>::iterator it2;
    for (it2 = players.begin(); it2 != players.end();) {
        if (&it2->second->user() == user) {
            delete it2->second;
            it2 = players.erase(it2);
        } else {
            it2++;
        }
    }
}
Pong::~Pong() {
    std::map<int, Partie*>::iterator it;
    for (it = parties.begin(); it != parties.end();) {
        delete it->second;
        it = parties.erase(it);
    }
    std::map<int, Player*>::iterator it2;
    for (it2 = players.begin(); it2 != players.end();) {
        if (it2->second->getId() == -1) {
            delete it2->second;
            it2 = players.erase(it2);
        } else {
            it2 = players.erase(it2);
        }
    }
}
