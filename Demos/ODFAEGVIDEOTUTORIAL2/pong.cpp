#include "pong.hpp"
using namespace odfaeg::core;
using namespace odfaeg::math;
using namespace odfaeg::graphic;
using namespace odfaeg::physic;
using namespace odfaeg::network;
Pong::Pong() : Application (sf::VideoMode(800, 600), "Pong", false, true, sf::Style::Default, sf::ContextSettings(0, 0, 4, 3, 0)) {
    m_player1 = nullptr;
    m_player2 = nullptr;
    m_party = nullptr;
    top = nullptr;
    bottom = nullptr;
    sf::Clock clock1, clock2;
    addClock(clock1, "TimeSinceLastUpdate");
    addClock(clock2, "TransferTime");
    timeBtwnTwoUpdates = sf::seconds(1.f);
}
void Pong::onKeyPressed (sf::Keyboard::Key key) {
    if (key == sf::Keyboard::Up) {
        if (m_player1 != nullptr && !m_player1->isMoving()) {
            m_player1->setMoving(true);
            m_player1->setDir(Vec2f(0, -1));
            std::string message = "MOVE_UP*"+conversionIntString(m_player1->getId())+"*"+conversionLongString(Application::getTimeClk().getElapsedTime().asMicroseconds())+"*"+conversionIntString(m_party->id());
            SymEncPacket packet;
            packet<<message;
            Network::sendTcpPacket(packet);
            m_party->player1().m_interpolation.first = m_party->player1().getCenter();
            m_party->player1().m_interpolation.second = m_party->player1().m_interpolation.first + m_party->player1().dir() * m_party->player1().speed() * (ping + timeBtwnTwoUpdates.asMicroseconds());
        }
    }
    if (key == sf::Keyboard::Down) {
        if (m_player1 != nullptr && !m_player1->isMoving()) {
            m_player1->setMoving(true);
            m_player1->setDir(Vec2f(0, 1));
            std::string message = "MOVE_DOWN*"+conversionIntString(m_player1->getId())+"*"+conversionLongString(Application::getTimeClk().getElapsedTime().asMicroseconds())+"*"+conversionIntString(m_party->id());
            SymEncPacket packet;
            packet<<message;
            Network::sendTcpPacket(packet);
            m_party->player1().m_interpolation.first = m_party->player1().getCenter();
            m_party->player1().m_interpolation.second = m_party->player1().m_interpolation.first + m_party->player1().dir() * m_party->player1().speed() * (ping + timeBtwnTwoUpdates.asMicroseconds());
        }
    }
}
void Pong::onKeyReleased(sf::Keyboard::Key key) {
    if (key == sf::Keyboard::Up || key == sf::Keyboard::Down) {
        if (m_player1 != nullptr && m_player1->isMoving()) {
            m_player1->setMoving(false);
            m_player1->setDir(Vec2f(0, 0));
            std::string message = "STOP_MOVING*"+conversionIntString(m_player1->getId())+"*"+conversionLongString(Application::getTimeClk().getElapsedTime().asMicroseconds())+"*"+conversionIntString(m_party->id());
            SymEncPacket packet;
            packet<<message;
            Network::sendTcpPacket(packet);
        }
    }
}
void Pong::onLoad() {
    FontManager<> fm;
    fm.fromFileWithAlias("fonts/FreeSerif.ttf", "FreeSerif");
    cache.addResourceManager(fm, "FontManager");
}
void Pong::onInit() {
    Network::startCli(10000, 10001, sf::IpAddress::LocalHost);
    FontManager<>& fm = cache.resourceManager<Font, std::string>("FontManager");
    m_taPseudo = new gui::TextArea(Vec2f(300, 275), Vec2f(100, 50), fm.getResourceByAlias("FreeSerif"), "Pseudo",getRenderWindow());
    m_bPseudo = new gui::Button(Vec2f(420, 275), Vec2f(50, 50), fm.getResourceByAlias("FreeSerif"), "OK",getRenderWindow());
    m_bPseudo->addActionListener(this);
    m_pseudoUsed = new gui::OptionPane(Vec2f(200, 200), Vec2f(200, 100), fm.getResourceByAlias("FreeSerif"),"Pseudo already used!",gui::OptionPane::TYPE::MESSAGE_DIALOG);
    m_confirmInvitation = new gui::OptionPane(Vec2f(200, 200), Vec2f(300, 300), fm.getResourceByAlias("FreeSerif"),"",gui::OptionPane::TYPE::CONFIRMATION_DIALOG);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    m_pseudoUsed->setVisible(false);
    m_pseudoUsed->setEventContextActivated(false);
    m_confirmInvitation->setVisible(false);
    m_confirmInvitation->setEventContextActivated(false);
    getView().move(getRenderWindow().getSize().x * 0.5f, getRenderWindow().getSize().y * 0.5f, 0);
    getRenderComponentManager().addComponent(m_taPseudo);
    getRenderComponentManager().addComponent(m_bPseudo);
    getRenderComponentManager().addComponent(m_pseudoUsed);
    getRenderComponentManager().addComponent(m_confirmInvitation);
    Action a1 (Action::EVENT_TYPE::KEY_PRESSED_ONCE, sf::Keyboard::Key::Up);
    Action a2 (Action::EVENT_TYPE::KEY_PRESSED_ONCE, sf::Keyboard::Key::Down);
    Action a3 (Action::EVENT_TYPE::KEY_RELEASED, sf::Keyboard::Key::Up);
    Action a4 (Action::EVENT_TYPE::KEY_RELEASED, sf::Keyboard::Key::Down);
    Command cmd1 (a1, FastDelegate<void>(&Pong::onKeyPressed, this, sf::Keyboard::Key::Unknown));
    Command cmd2 (a2, FastDelegate<void>(&Pong::onKeyPressed, this, sf::Keyboard::Key::Unknown));
    Command cmd3 (a3, FastDelegate<void>(&Pong::onKeyReleased, this, sf::Keyboard::Key::Unknown));
    Command cmd4 (a4, FastDelegate<void>(&Pong::onKeyReleased, this, sf::Keyboard::Key::Unknown));
    getListener().connect("KEYPUP", cmd1);
    getListener().connect("KEYPDOWN", cmd2);
    getListener().connect("KEYRUP", cmd3);
    getListener().connect("KEYRDOWN", cmd4);
}
void Pong::onRender(FastRenderComponentManager* frcm) {

}
void Pong::onDisplay(RenderWindow* rw) {
    if (m_party != nullptr) {
        rw->draw(*m_player1);
        rw->draw(*m_player2);
        rw->draw(*m_ball);
        rw->draw(*top);
        rw->draw(*bottom);
    }
}
void Pong::onUpdate(sf::Event& event) {
    if (event.type == sf::Event::Closed) {
        stop();
    }
    if (event.type == sf::Event::KeyPressed) {
        getListener().setCommandSlotParams("KEYPUP", this, event.key.code);
        getListener().setCommandSlotParams("KEYPDOWN", this, event.key.code);
    }
    if (event.type == sf::Event::KeyReleased) {
        getListener().setCommandSlotParams("KEYRUP", this, event.key.code);
        getListener().setCommandSlotParams("KEYRDOWN", this, event.key.code);
    }
}
void Pong::onExec() {
    std::string response;
    if (Network::getResponse("CONNECTED", response)) {
        std::vector<std::string> infos = split(response, "*");
        if (infos[0] == "OK") {
            std::string pseudo = infos[2];
            FontManager<>& fm = cache.resourceManager<Font, std::string>("FontManager");
            gui::Button* button = new gui::Button(Vec2f(350, maxI), Vec2f(100, 50),fm.getResourceByAlias("FreeSerif"), pseudo, getRenderWindow());
            button->addActionListener(this);
            m_bPseudos.push_back(button);
            getRenderComponentManager().addComponent(button);
        }
    }
    if (Network::getResponse("INVITE", response)) {
        std::vector<std::string> infos = split(response, "*");
        int id = conversionStringInt(infos[0]);
        std::string pseudo = infos[1];
        m_player2 = new Player(pseudo, Vec2f (0, 0));
        m_player2->setId(id);
        std::string text = "The player "+pseudo+" wants to invite you to a party,\n do you accept it ?";
        m_confirmInvitation->setText(text);
        m_confirmInvitation->setVisible(true);
    }

    if (Network::getResponse("PARTYINFOS1", response)) {
        std::vector<std::string> infos = split(response, "*");
        int idParty = conversionStringInt(infos[0]);
        m_ball = new Ball();
        m_party = new Partie(*m_player1, *m_player2, *m_ball);
        m_party->setId(idParty);
        for (unsigned int i = 0; i < m_bPseudos.size(); i++) {
            m_bPseudos[i]->setVisible(false);
            m_bPseudos[i]->setEventContextActivated(false);
        }
        Vec2f p1Pos (conversionStringFloat(infos[1]), conversionStringFloat(infos[2]));
        Vec2f p2Pos (conversionStringFloat(infos[3]), conversionStringFloat(infos[4]));
        m_player2->setPosition(Vec3f(p1Pos.x, p1Pos.y, 0));
        m_player1->setPosition(Vec3f(p2Pos.x, p2Pos.y, 0));
        top = new Bar(Vec2f(0, 0), Vec2f(800, 10));
        bottom = new Bar(Vec2f(0, 590), Vec2f(800, 10));
        EntityManager* em = new Map(&getRenderComponentManager(), "PONG_MAP", 50, 50);
        World::addEntityManager(em);
        World::setCurrentEntityManager("PONG_MAP");
        World::addEntity(m_player1);
        World::addEntity(m_player2);
        World::addEntity(m_ball);
        World::addEntity(top);
        World::addEntity(bottom);
    }
    if (Network::getResponse("PARTYINFOS2", response)) {
        std::vector<std::string> infos = split(response, "*");
        int idParty = conversionStringInt(infos[0]);
        int idPlayer2 = conversionStringInt(infos[1]);
        std::string pseudo2 = infos[2];
        m_player2 = new Player(pseudo2, Vec2f(0, 0));
        m_player2->setId(idPlayer2);
        m_ball = new Ball();
        m_party = new Partie(*m_player1, *m_player2, *m_ball);
        m_party->setId(idParty);
        for (unsigned int i = 0; i < m_bPseudos.size(); i++) {
            m_bPseudos[i]->setVisible(false);
            m_bPseudos[i]->setEventContextActivated(false);
        }
        Vec2f p1Pos (conversionStringFloat(infos[3]), conversionStringFloat(infos[4]));
        Vec2f p2Pos (conversionStringFloat(infos[5]), conversionStringFloat(infos[6]));
        m_player2->setPosition(Vec3f(p1Pos.x, p1Pos.y, 0));
        m_player1->setPosition(Vec3f(p2Pos.x, p2Pos.y, 0));
        top = new Bar(Vec2f(0, 0), Vec2f(800, 10));
        bottom = new Bar(Vec2f(0, 590), Vec2f(800, 10));
        EntityManager* em = new Map(&getRenderComponentManager(), "PONG_MAP", 50, 50);
        World::addEntityManager(em);
        World::setCurrentEntityManager("PONG_MAP");
        World::addEntity(m_player1);
        World::addEntity(m_player2);
        World::addEntity(m_ball);
        World::addEntity(top);
        World::addEntity(bottom);
    }
    if (m_confirmInvitation->getOption() == gui::OptionPane::OPTION::YES_OPTION) {
        std::string message = "CREATE_PARTY*"+conversionIntString(m_player1->getId())+"*"+conversionIntString(m_player2->getId());
        SymEncPacket packet;
        packet<<message;
        Network::sendTcpPacket(packet);
    }
    if (m_confirmInvitation->getOption() == gui::OptionPane::OPTION::NO_OPTION) {
        delete m_player2;
    }
    if (m_party != nullptr && getClock("TimeSinceLastUpdate").getElapsedTime().asMicroseconds() >= timeBtwnTwoUpdates.asMicroseconds()) {
        std::string request = "UPDATE_POSITIONS*"+conversionIntString(m_party->id());
        sf::Packet packet;
        packet<<request;
        Network::sendUdpPacket(packet);
        getClock("TimeSinceLastUpdate").restart();
    }
    if (m_party != nullptr && (Network::getResponse("UPDATE_POSITIONS", response))) {
        std::vector<std::string> infos = split(response, "*");
        if (infos.size() == 15) {
            int id1 = conversionStringInt(infos[0]);
            Vec2f posP1 (conversionStringFloat(infos[1]), conversionStringFloat(infos[2]));
            int d1y = conversionStringInt(infos[3]);
            int id2 = conversionStringInt(infos[4]);
            Vec2f posP2 (conversionStringFloat(infos[5]), conversionStringFloat(infos[6]));
            int d2y = conversionStringInt(infos[7]);
            Vec2f ballPos (conversionStringFloat(infos[8]), conversionStringFloat(infos[9]));
            ping = conversionStringLong(infos[10]);
            bool isMoving1 = conversionStringInt(infos[11]);
            bool isMoving2 = conversionStringInt(infos[12]);
            Vec2f dir (conversionStringFloat(infos[13]), conversionStringFloat(infos[14]));
            m_ball->updateDir(dir);
            Player* p1 = static_cast<Player*>(World::getEntity(id1));
            Player* p2 = static_cast<Player*>(World::getEntity(id2));
            Vec2f d1 = posP1 - p1->getCenter();
            World::moveEntity(p1, d1.x, d1.y, 0);
            p1->setDir(Vec2f(0, d1y));
            p1->setMoving(isMoving1);
            Vec2f d2 = posP2 - p2->getCenter();
            World::moveEntity(p2, d2.x, d2.y, 0);
            p2->setDir(Vec2f(0, d2y));
            p2->setMoving(isMoving2);
            Vec2f d3 = ballPos - m_party->ball().getCenter();
            World::moveEntity(&m_party->ball(), d3.x, d3.y, 0);
        }
        m_party->player1().m_interpolation.first = m_party->player1().getCenter();
        m_party->player2().m_interpolation.first = m_party->player2().getCenter();
        m_party->ball().m_interpolation.first = m_party->ball().getCenter();
        m_party->player1().m_interpolation.second = m_party->player1().m_interpolation.first + m_party->player1().dir() * m_party->player1().speed() * (ping + timeBtwnTwoUpdates.asMicroseconds());
        m_party->player2().m_interpolation.second = m_party->player2().m_interpolation.first + m_party->player2().dir() * m_party->player2().speed() * (ping + timeBtwnTwoUpdates.asMicroseconds());
        m_party->ball().m_interpolation.second = m_party->ball().m_interpolation.first + m_party->ball().dir() * m_party->ball().speed() * (ping + timeBtwnTwoUpdates.asMicroseconds());
        getClock("TransferTime").restart();
    }
    if (m_party != nullptr && (Network::getResponse("STOP", response))) {
        std::vector<std::string> infos = split(response, "*");
        int id = conversionStringInt(infos[0]);
        Vec2f pPos (conversionStringFloat(infos[1]), conversionStringFloat(infos[2]));
        Player* p = static_cast<Player*>(World::getEntity(id));
        Vec2f d = pPos - p->getCenter();
        World::moveEntity(p, d.x, d.y, 0);
        p->setMoving(false);
        p->setDir(Vec2f(0, 0));
    }
    if (m_party != nullptr) {
        sf::Int64 elapsedTime = getClock("TransferTime").getElapsedTime().asMicroseconds();
        Vec2f newPosP1, newPosP2;
        if (m_party->player1().isMoving())
            newPosP1 = m_party->player1().m_interpolation.first + (m_party->player1().m_interpolation.second - m_party->player1().m_interpolation.first) * ((float) elapsedTime / (float) (ping + timeBtwnTwoUpdates.asMicroseconds()));
        else
            newPosP1 = m_party->player1().getCenter();
        if (m_party->player2().isMoving())
            newPosP2 = m_party->player2().m_interpolation.first + (m_party->player2().m_interpolation.second - m_party->player2().m_interpolation.first) * ((float) elapsedTime / (float) (ping + timeBtwnTwoUpdates.asMicroseconds()));
        else
            newPosP2 = m_party->player2().getCenter();
        Vec2f actualPosBall = m_party->ball().m_interpolation.first;
        Vec2f newPosBall = m_party->ball().m_interpolation.first + (m_party->ball().m_interpolation.second - m_party->ball().m_interpolation.first) * ((float) elapsedTime / (float) (ping + timeBtwnTwoUpdates.asMicroseconds()));
        Ray ray1 (m_player1->getCenter(), newPosP1);
        Ray ray2 (m_player2->getCenter(), newPosP2);
        if (!World::collide(&m_party->player1(), ray1)) {
            Vec2f actualPos = m_party->player1().getCenter();
            Vec2f d = newPosP1 - actualPos;
            if (!d.isNulVector())
                World::moveEntity(&m_party->player1(), d.x, d.y, 0);
        }
        if (!World::collide(&m_party->player2(), ray2)) {
            Vec2f actualPos = m_party->player2().getCenter();
            Vec2f d = newPosP2 - actualPos;
            if (!d.isNulVector())
                World::moveEntity(&m_party->player2(), d.x, d.y, 0);
        }
        Ray ballPath(actualPosBall, newPosBall);
        Vec2f tmpDir = m_party->ball().dir();
        while (actualPosBall != newPosBall && World::collide(&m_party->ball(), ballPath)) {
            CollisionResultSet::Info info = CollisionResultSet::popCollisionInfo();
            Entity* entity = info.entity;
            Vec3f near, far;
            std::unique_ptr<BoundingVolume> bv = m_party->ball().getCollisionVolume()->clone();
            bv->move(info.center - bv->getCenter());
            bv->intersectsWhere(ballPath, near, far, info);
            Vec2f tmpPosBall = far - tmpDir * (info.mtu.magnitude() + m_party->ball().getCollisionVolume()->getSize().x * 0.5f + EPSILON);
            Vec2f v1 = tmpPosBall - actualPosBall;
            Vec2f v2 = newPosBall - actualPosBall;
            float ratio = v1.magnSquared() / v2.magnSquared();
            sf::Int64 remainingTime = elapsedTime - elapsedTime * ratio;
            Vec2f n = static_cast<BoundingBox*>(entity->getCollisionVolume())->getEdgeNormals()[info.nearestEdgeIndex1];
            Vec2f invDir = -tmpDir;
            float dp = n.dot(invDir);
            n = n * dp;
            tmpDir = invDir + (n - invDir) * 2;
            newPosBall = tmpPosBall +  tmpDir * m_party->ball().speed() * remainingTime;
            Vec2f d = tmpPosBall - actualPosBall;
            World::moveEntity(&m_party->ball(), d.x, d.y, 0);
            actualPosBall = tmpPosBall;
            ballPath = Ray(actualPosBall, newPosBall);
            elapsedTime = remainingTime;
        }
        Vec2f d = newPosBall - m_party->ball().getCenter();
        World::moveEntity(&m_party->ball(), d.x, d.y, 0);
        if (newPosBall.x <= -100 || newPosBall.x >= 900) {
            m_party->ball().setCenter(Vec3f(400, 300, 0));
            m_party->ball().updateDir(Vec2f(0, 0));
        }
    }
}
void Pong::actionPerformed(gui::Button* button) {
    if (button == m_bPseudo) {
        std::string text = "CONNECT*"+m_taPseudo->getText();
        SymEncPacket packet;
        packet<<text;
        Network::sendTcpPacket(packet);
        std::string response = Network::waitForLastResponse("CONNECTED");
        std::vector<std::string> infos = split(response, "*");
        if (infos[0] == "NOTOK") {
            m_pseudoUsed->setVisible(true);
        } else {
            FontManager<>& fm = cache.resourceManager<Font, std::string>("FontManager");
            m_taPseudo->setVisible(false);
            m_bPseudo->setVisible(false);
            int id = conversionStringInt(infos[1]);
            std::string pseudo = infos[2];
            gui::Button* b = new gui::Button(Vec2f(350, 50), Vec2f(100, 50),fm.getResourceByAlias("FreeSerif"), pseudo, getRenderWindow());
            b->setEventContextActivated(false);
            m_bPseudos.push_back(b);
            getRenderComponentManager().addComponent(b);
            m_player1 = new Player(m_taPseudo->getText(), Vec2f(0, 0));
            m_player1->setId(id);
            int nbPlayers = conversionStringInt(infos[3]);
            maxI = 100;
            for (unsigned int i = 0; i < nbPlayers; i++) {
                std::string pseudo = infos[i + 4];
                gui::Button* bt = new gui::Button(Vec2f(350, (i + 1) * 100), Vec2f(100, 50),fm.getResourceByAlias("FreeSerif"), pseudo, getRenderWindow());
                bt->addActionListener(this);
                m_bPseudos.push_back(bt);
                getRenderComponentManager().addComponent(bt);
                maxI = (i + 2) * 100;
            }
        }
    } else {
        for (unsigned int i = 0; i < m_bPseudos.size(); i++) {
            if(m_bPseudos[i] == button) {
                std::string message = "INVITE*"+m_player1->pseudo()+"*"+conversionIntString(m_player1->getId())+"*"+button->getText();
                SymEncPacket packet;
                packet<<message;
                Network::sendTcpPacket(packet);
            }
        }
    }
}
