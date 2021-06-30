#include "odfaegCreatorStateExecutor.hpp"
#include "application.hpp"

using namespace odfaeg::core;
using namespace odfaeg::math;
using namespace odfaeg::graphic;
ODFAEGCreatorStateExecutor::ODFAEGCreatorStateExecutor(ODFAEGCreator* app) : app(app) {
}
bool ODFAEGCreatorStateExecutor::doState(State& state) {
    if (state.getName() == "ADDREMOVESHAPE") {
        std::string str = state.getParameter("ADDREMOVESHAPE").getValue<std::string>();
        std::istringstream iss(str);
        ITextArchive ia(iss);
        unsigned int id;
        ia(id);
        Shape* shape;
        ia(shape);
        //app->addShape(shape);
    }
    if (state.getName() == "SCHANGEXPOS") {
        float newXPos = state.getParameter("NEWVALUE").getValue<float>();
        Transformable* shape = state.getParameter("OBJECT").getValue<Transformable*>();
        shape->setPosition(Vec3f(newXPos,shape->getPosition().y, shape->getPosition().z));
        //app->updateScriptPos(shape);
    }
    if (state.getName() == "SCHANGEYPOS") {
        float newYPos = state.getParameter("NEWVALUE").getValue<float>();
        Transformable* shape = state.getParameter("OBJECT").getValue<Transformable*>();
        shape->setPosition(Vec3f(shape->getPosition().x, newYPos, shape->getPosition().z));
        //app->updateScriptPos(shape);
    }
    if (state.getName() == "SCHANGEZPOS") {
        float newZPos = state.getParameter("NEWVALUE").getValue<float>();
        Transformable* shape = state.getParameter("OBJECT").getValue<Transformable*>();
        shape->setPosition(Vec3f(shape->getPosition().x, shape->getPosition().y, newZPos));
        //app->updateScriptPos(shape);
    }
    if (state.getName() == "SCHANGERCOLOR") {
        unsigned int color = state.getParameter("NEWVALUE").getValue<unsigned int>();
        Transformable* shape = state.getParameter("OBJECT").getValue<Transformable*>();
        if (dynamic_cast<Shape*>(shape)) {
            static_cast<Shape*>(shape)->setFillColor(sf::Color(color, static_cast<Shape*>(shape)->getFillColor().g, static_cast<Shape*>(shape)->getFillColor().b, static_cast<Shape*>(shape)->getFillColor().a));
        }
    }
    if (state.getName() == "SCHANGEGCOLOR") {
        unsigned int color = state.getParameter("NEWVALUE").getValue<unsigned int>();
        Transformable* shape = state.getParameter("OBJECT").getValue<Transformable*>();
        if (dynamic_cast<Shape*>(shape)) {
            static_cast<Shape*>(shape)->setFillColor(sf::Color(static_cast<Shape*>(shape)->getFillColor().r, color, static_cast<Shape*>(shape)->getFillColor().b, static_cast<Shape*>(shape)->getFillColor().a));
        }
    }
    if (state.getName() == "SCHANGEBCOLOR") {
        unsigned int color = state.getParameter("NEWVALUE").getValue<unsigned int>();
        Transformable* shape = state.getParameter("OBJECT").getValue<Transformable*>();
        if (dynamic_cast<Shape*>(shape)) {
            static_cast<Shape*>(shape)->setFillColor(sf::Color(static_cast<Shape*>(shape)->getFillColor().r, static_cast<Shape*>(shape)->getFillColor().g, color, static_cast<Shape*>(shape)->getFillColor().a));
        }
    }
    if (state.getName() == "SCHANGEACOLOR") {
        unsigned int color = state.getParameter("NEWVALUE").getValue<unsigned int>();
        Transformable* shape = state.getParameter("OBJECT").getValue<Transformable*>();
        if (dynamic_cast<Shape*>(shape)) {
            static_cast<Shape*>(shape)->setFillColor(sf::Color(static_cast<Shape*>(shape)->getFillColor().r, static_cast<Shape*>(shape)->getFillColor().g, static_cast<Shape*>(shape)->getFillColor().b, color));
        }
    }
    if (state.getName() == "SCHANGETEXTURE") {
        const Texture* texture = state.getParameter("NEWVALUE").getValue<const Texture*>();
        Transformable* shape = state.getParameter("OBJECT").getValue<Transformable*>();
        if (dynamic_cast<Shape*>(shape)) {
            static_cast<Shape*>(shape)->setTexture(texture);
        }
    }
    if (state.getName() == "SCHANGEXTEXCOORD") {
        int texCoordX = state.getParameter("NEWVALUE").getValue<int>();
        Transformable* shape = state.getParameter("OBJECT").getValue<Transformable*>();
        if (dynamic_cast<Shape*>(shape)) {
            static_cast<Shape*>(shape)->setTextureRect(sf::IntRect(texCoordX,static_cast<Shape*>(shape)->getTextureRect().top,static_cast<Shape*>(shape)->getTextureRect().width,static_cast<Shape*>(shape)->getTextureRect().height));
        }
    }
    if (state.getName() == "SCHANGEYTEXCOORD") {
        int texCoordY = state.getParameter("NEWVALUE").getValue<int>();
        Transformable* shape = state.getParameter("OBJECT").getValue<Transformable*>();
        if (dynamic_cast<Shape*>(shape)) {
            static_cast<Shape*>(shape)->setTextureRect(sf::IntRect(static_cast<Shape*>(shape)->getTextureRect().left,texCoordY,static_cast<Shape*>(shape)->getTextureRect().width,static_cast<Shape*>(shape)->getTextureRect().height));
        }
    }
    if (state.getName() == "SCHANGEWTEXCOORD") {
        int texCoordW = state.getParameter("NEWALUE").getValue<int>();
        Transformable* shape = state.getParameter("OBJECT").getValue<Transformable*>();
        if (dynamic_cast<Shape*>(shape)) {
            static_cast<Shape*>(shape)->setTextureRect(sf::IntRect(static_cast<Shape*>(shape)->getTextureRect().left,static_cast<Shape*>(shape)->getTextureRect().top,texCoordW,static_cast<Shape*>(shape)->getTextureRect().height));
        }
    }
    if (state.getName() == "SCHANGEXTEHCOORD") {
        int texCoordH = state.getParameter("NEWVALUE").getValue<int>();
        Transformable* shape = state.getParameter("OBJECT").getValue<Transformable*>();
        if (dynamic_cast<Shape*>(shape)) {
            static_cast<Shape*>(shape)->setTextureRect(sf::IntRect(static_cast<Shape*>(shape)->getTextureRect().left,static_cast<Shape*>(shape)->getTextureRect().top,static_cast<Shape*>(shape)->getTextureRect().width,texCoordH));
        }
    }
    return true;
}
bool ODFAEGCreatorStateExecutor::undoState(State& state) {
    if (state.getName() == "ADDREMOVESHAPE") {
        std::string str = state.getParameter("ADDREMOVESHAPE").getValue<std::string>();
        std::istringstream iss(str);
        ITextArchive ia(iss);
        unsigned int id;
        ia(id);
        app->removeShape(id);
    }
    if (state.getName() == "SCHANGEXPOS") {
        float newXPos = state.getParameter("OLDVALUE").getValue<float>();
        Transformable* shape = state.getParameter("OBJECT").getValue<Transformable*>();
        shape->setPosition(Vec3f(newXPos,shape->getPosition().y, shape->getPosition().z));
        //app->updateScriptPos(shape);
    }
    if (state.getName() == "SCHANGEYPOS") {
        float newYPos = state.getParameter("OLDVALUE").getValue<float>();
        Transformable* shape = state.getParameter("OBJECT").getValue<Transformable*>();
        shape->setPosition(Vec3f(shape->getPosition().x, newYPos, shape->getPosition().z));
        //app->updateScriptPos(shape);
    }
    if (state.getName() == "SCHANGEZPOS") {
        float newZPos = state.getParameter("OLDVALUE").getValue<float>();
        Transformable* shape = state.getParameter("OBJECT").getValue<Transformable*>();
        shape->setPosition(Vec3f(shape->getPosition().x, shape->getPosition().y, newZPos));
        //app->updateScriptPos(shape);
    }
    if (state.getName() == "SCHANGERCOLOR") {
        unsigned int color = state.getParameter("OLDVALUE").getValue<unsigned int>();
        Transformable* shape = state.getParameter("OBJECT").getValue<Transformable*>();
        if (dynamic_cast<Shape*>(shape)) {
            static_cast<Shape*>(shape)->setFillColor(sf::Color(color, static_cast<Shape*>(shape)->getFillColor().g, static_cast<Shape*>(shape)->getFillColor().b, static_cast<Shape*>(shape)->getFillColor().a));
        }
    }
    if (state.getName() == "SCHANGEGCOLOR") {
        unsigned int color = state.getParameter("OLDVALUE").getValue<unsigned int>();
        Transformable* shape = state.getParameter("OBJECT").getValue<Transformable*>();
        if (dynamic_cast<Shape*>(shape)) {
            static_cast<Shape*>(shape)->setFillColor(sf::Color(static_cast<Shape*>(shape)->getFillColor().r, color, static_cast<Shape*>(shape)->getFillColor().b, static_cast<Shape*>(shape)->getFillColor().a));
        }
    }
    if (state.getName() == "SCHANGEBCOLOR") {
        unsigned int color = state.getParameter("OLDVALUE").getValue<unsigned int>();
        Transformable* shape = state.getParameter("OBJECT").getValue<Transformable*>();
        if (dynamic_cast<Shape*>(shape)) {
            static_cast<Shape*>(shape)->setFillColor(sf::Color(static_cast<Shape*>(shape)->getFillColor().r, static_cast<Shape*>(shape)->getFillColor().g, color, static_cast<Shape*>(shape)->getFillColor().a));
        }
    }
    if (state.getName() == "SCHANGEACOLOR") {
        unsigned int color = state.getParameter("OLDVALUE").getValue<unsigned int>();
        Transformable* shape = state.getParameter("OBJECT").getValue<Transformable*>();
        if (dynamic_cast<Shape*>(shape)) {
            static_cast<Shape*>(shape)->setFillColor(sf::Color(static_cast<Shape*>(shape)->getFillColor().r, static_cast<Shape*>(shape)->getFillColor().g, static_cast<Shape*>(shape)->getFillColor().b, color));
        }
    }
    if (state.getName() == "SCHANGETEXTURE") {
        const Texture* texture = state.getParameter("OLDVALUE").getValue<const Texture*>();
        Transformable* shape = state.getParameter("OBJECT").getValue<Transformable*>();
        if (dynamic_cast<Shape*>(shape)) {
            static_cast<Shape*>(shape)->setTexture(texture);
        }
    }
    if (state.getName() == "SCHANGEXTEXCOORD") {
        int texCoordX = state.getParameter("OLDVALUE").getValue<int>();
        Transformable* shape = state.getParameter("OBJECT").getValue<Transformable*>();
        if (dynamic_cast<Shape*>(shape)) {
            static_cast<Shape*>(shape)->setTextureRect(sf::IntRect(texCoordX,static_cast<Shape*>(shape)->getTextureRect().top,static_cast<Shape*>(shape)->getTextureRect().width,static_cast<Shape*>(shape)->getTextureRect().height));
        }
    }
    if (state.getName() == "SCHANGEYTEXCOORD") {
        int texCoordY = state.getParameter("OLDVALUE").getValue<int>();
        Transformable* shape = state.getParameter("OBJECT").getValue<Transformable*>();
        if (dynamic_cast<Shape*>(shape)) {
            static_cast<Shape*>(shape)->setTextureRect(sf::IntRect(static_cast<Shape*>(shape)->getTextureRect().left,texCoordY,static_cast<Shape*>(shape)->getTextureRect().width,static_cast<Shape*>(shape)->getTextureRect().height));
        }
    }
    if (state.getName() == "SCHANGEWTEXCOORD") {
        int texCoordW = state.getParameter("NEWALUE").getValue<int>();
        Transformable* shape = state.getParameter("OBJECT").getValue<Transformable*>();
        if (dynamic_cast<Shape*>(shape)) {
            static_cast<Shape*>(shape)->setTextureRect(sf::IntRect(static_cast<Shape*>(shape)->getTextureRect().left,static_cast<Shape*>(shape)->getTextureRect().top,texCoordW,static_cast<Shape*>(shape)->getTextureRect().height));
        }
    }
    if (state.getName() == "SCHANGEXTEHCOORD") {
        int texCoordH = state.getParameter("OLDVALUE").getValue<int>();
        Transformable* shape = state.getParameter("OBJECT").getValue<Transformable*>();
        if (dynamic_cast<Shape*>(shape)) {
            static_cast<Shape*>(shape)->setTextureRect(sf::IntRect(static_cast<Shape*>(shape)->getTextureRect().left,static_cast<Shape*>(shape)->getTextureRect().top,static_cast<Shape*>(shape)->getTextureRect().width,texCoordH));
        }
    }
    return true;
}
