#include "../../../include/odfaeg/Graphics/anim.h"
namespace odfaeg {
    namespace graphic {
        using namespace sf;
        using namespace std;
        Anim::Anim(EntityFactory& factory) : AnimatedEntity (math::Vec3f(0, 0, 0), math::Vec3f(0, 0, 0), math::Vec3f(0, 0, 0), "E_ANIMATION", factory, "", nullptr) {
            currentFrameIndex = 0;
            running = false;
            currentFrameChanged = false;
            currentFrame = nullptr;
            previousFrame = nullptr;
            nextFrame = nullptr;
            loop = false;
            interpLevels = 1;
            interpPerc = 0;
            interpolatedFrame = std::make_unique<Mesh>(math::Vec3f(0, 0, 0), math::Vec3f(0, 0, 0), "E_ANIMATION_FRAME", factory);
            interpolatedFrame->setParent(this);
            auname = "";
            fr = 0.1f;
            //interpolatedFrame->setName("E_ANIMATION_FRAME");
        }
        Anim::Anim (float fr, math::Vec3f position, math::Vec3f size, EntityFactory& factory, Entity *parent) : AnimatedEntity (position, size, size * 0.5f, "E_ANIMATION", factory, "", parent) {
            this->fr = fr;
            currentFrameIndex = 0;
            running = false;
            currentFrameChanged = false;
            currentFrame = nullptr;
            previousFrame = nullptr;
            nextFrame = nullptr;
            loop = false;
            interpLevels = 1;
            interpPerc = 0;
            interpolatedFrame = std::make_unique<Mesh>(position, size, "E_ANIMATION_FRAME", factory);
            interpolatedFrame->setParent(this);
            auname = "";
            //interpolatedFrame->setName("E_ANIMATION_FRAME");
        }
        bool Anim::isCurrentFrameChanged() {
            return currentFrameChanged;
        }
        void Anim::setCurrentFrameChanged(bool b) {
            currentFrameChanged = b;
        }
        void Anim::addFrame (Entity *entity) {
            entity->setParent(this);
            if (getChildren().size() == 0) {
                currentFrame = entity;
                interpolatedFrame->setName("IFRAME");
                createFirstInterpolatedFrame(entity);
            }
            else if (getChildren().size() == 1)
                nextFrame = entity;
            addChild(entity);
        }
        void Anim::removeFrame (Entity *entity) {
            removeChild(entity);
        }
        int Anim::getCurrentFrameIndex () {
            return currentFrameIndex;
        }
        void Anim::setCurrentFrame (int index) {
            if (getChildren().size() >= 2) {
                previousFrame = getChildren()[(index -1 < 0) ? 0 : index - 1];
                currentFrame = getChildren()[index];
                nextFrame = getChildren()[(index + 1 >= getChildren().size()) ? 0 : index + 1];
                currentFrameIndex = index;
                currentFrameChanged = true;
                changeInterpolatedFrame(currentFrame);
            }
        }
        Entity* Anim::getPreviousFrame() {
            return previousFrame;
        }
        void Anim::setFrameRate (float fr) {
            this->fr = fr;
        }
        bool Anim::isRunning () {

            return running;
        }
        void Anim::play (bool loop) {
            if (getChildren().size() > 1 && !running) {
                running = true;
                //std::cout<<"play : "<<running<<std::endl;
                this->loop = loop;
            }
        }
        void Anim::setSelected(bool selected) {
            //std::cout<<"set selected : "<<selected<<std::endl;
            Entity::updateSelected(selected);
            interpolatedFrame->setSelected(selected);
        }
        void Anim::setLayer(unsigned int layer) {
            interpolatedFrame->setLayer(layer);
        }
        void Anim::stop (){
            if (running) {

                running = false;
                loop = false;
            }
        }
        void Anim::computeNextFrame () {

            onFrameChanged();
        }
        void Anim::onFrameChanged() {
            if (getChildren().size() >= 2) {

                if (previousFrame == nullptr) {
                    previousFrame = getChildren()[(currentFrameIndex - 1 < 0) ? 0 : getChildren().size() - 1];
                }
                interpPerc++;
                if (interpPerc >= interpLevels) {
                    previousFrame = getChildren()[currentFrameIndex];
                    currentFrameChanged = true;
                    currentFrameIndex++;
                    if (currentFrameIndex >= getChildren().size()) {
                        currentFrameIndex = 0;
                        if (!loop) {
                            running = false;
                        }
                    }

                    currentFrame = getChildren()[currentFrameIndex];
                    nextFrame = getChildren()[(currentFrameIndex + 1 >= getChildren().size()) ? 0 : currentFrameIndex+1];
                    interpPerc = 0;
                    changeInterpolatedFrame(currentFrame);
                }
                interpolate(currentFrame, nextFrame);
            }
        }
        void Anim::createFirstInterpolatedFrame (Entity* currentFrame) {
            if (currentFrame->getChildren().size() > 0) {
                for (unsigned int i = 0; i < currentFrame->getChildren().size(); i++) {
                    createFirstInterpolatedFrame(currentFrame->getChildren()[i]);
                }
            }
            if (currentFrame->isLeaf()) {
                for (unsigned int i = 0; i < currentFrame->getFaces().size(); i++) {
                    VertexArray va = currentFrame->getFaces()[i].getVertexArray();
                    Face face (va,currentFrame->getFaces()[i].getMaterial(), currentFrame->getTransform());
                    interpolatedFrame->addFace(face);
                }
                /*interpolatedFrame->setSize(currentFrame->getSize());
                interpolatedFrame->setRotation(currentFrame->getRotation());
                interpolatedFrame->setPosition(currentFrame->getPosition());*/
                //addChild(interpolatedFrame);
                /*if (interpolatedFrame->getRootType() == "E_ANIMATION")
                    std::cout<<"interpolated frame type : "<<interpolatedFrame->getType()<<std::endl;*/
            }
        }
        void Anim::changeInterpolatedFrame(Entity* currentFrame) {

            if (currentFrame->getChildren().size() > 0) {
                for (unsigned int i = 0; i < currentFrame->getChildren().size(); i++) {
                    changeInterpolatedFrame(currentFrame->getChildren()[i]);
                }
            }
            if (currentFrame->isLeaf()) {

                std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                interpolatedFrame->getFaces().clear();
                for (unsigned int i = 0; i < currentFrame->getFaces().size(); i++) {

                    VertexArray va = currentFrame->getFaces()[i].getVertexArray();
                    Face face (va,currentFrame->getFaces()[i].getMaterial(), currentFrame->getTransform());
                    interpolatedFrame->addFace(face);
                }
                /*interpolatedFrame->setSize(currentFrame->getSize());
                interpolatedFrame->setRotation(currentFrame->getRotation());
                interpolatedFrame->setPosition(currentFrame->getPosition());*/
            }
        }
        void Anim::interpolate(Entity* currentFrame, Entity* nextFrame) {
            if (currentFrame->getNbChildren() == nextFrame->getNbChildren()) {
                if (currentFrame->getNbChildren() != 0 && nextFrame->getNbChildren() != 0) {
                    for (unsigned int i = 0; i < currentFrame->getNbChildren(); i++) {
                         interpolate(currentFrame->getChild(i), nextFrame->getChild(i));
                    }
                }
                if (currentFrame->getNbFaces() == nextFrame->getNbFaces()
                    && currentFrame->getNbFaces() == interpolatedFrame->getNbFaces()) {
                    for (unsigned int i = 0; i < currentFrame->getNbFaces(); i++) {
                        const VertexArray& cva = currentFrame->getFace(i)->getVertexArray();
                        VertexArray& iva = interpolatedFrame->getFace(i)->getVertexArray();
                        const VertexArray& nva = nextFrame->getFace(i)->getVertexArray();
                        if (cva.getVertexCount() == nva.getVertexCount()) {
                            for (unsigned int j = 0; j < cva.getVertexCount(); j++) {
                                iva[j].position.x = cva[j].position.x + (nva[j].position.x - cva[j].position.x) * (interpPerc / interpLevels);
                                iva[j].position.y = cva[j].position.y + (nva[j].position.y - cva[j].position.y) * (interpPerc / interpLevels);
                                iva[j].position.z = cva[j].position.z + (nva[j].position.z - cva[j].position.z) * (interpPerc / interpLevels);
                                iva[j].color = cva[j].color;
                                iva[j].texCoords = cva[j].texCoords;
                                //std::cout<<"position "<<iva[j].position.x<<","<<iva[j].position.y<<","<<iva[j].position.z<<std::endl;
                                /*if (currentFrame->getRootType() == "E_MONSTER")
                                    std::cout<<"interpolation tex coords : "<<interpolatedFrame->getFace(i)->getVertexArray()[j].texCoords.x<<","<<interpolatedFrame->getFace(i)->getVertexArray()[j].texCoords.y<<std::endl;*/
                            }
                            interpolatedFrame->getFace(i)->setMaterial(currentFrame->getFace(i)->getMaterial());
                            interpolatedFrame->getFace(i)->setTransformMatrix(currentFrame->getFace(i)->getTransformMatrix());
                        }
                    }
                }
            }
        }
        Time Anim::getElapsedTime () {
            return clock.getElapsedTime();
        }
        void Anim::resetClock () {
            clock.restart();
        }
        bool Anim::operator== (Entity &other) {

            if (other.getType() != "E_ANIMATION")
                return false;
            Anim &a = static_cast<Anim&> (other);
            if (getPosition().x != a.getPosition().x || getPosition().y != a.getPosition().y
                || getSize().x != a.getSize().x || getSize().y != a.getSize().y)
                return false;
            for (unsigned int i = 0; i < getChildren().size(); i++) {
                if (!(*getChildren()[i] == *a.getChildren()[i]))
                    return false;
            }
            return true;
        }

        float Anim::getFrameRate () {
            return fr;
        }
        Entity* Anim::getCurrentFrame () const {
              return interpolatedFrame.get();
        }
        void Anim::onDraw(RenderTarget &target, RenderStates states) {
             target.draw(*interpolatedFrame, states);
        }
        void Anim::onMove(math::Vec3f& t) {
            GameObject::onMove(t);
            interpolatedFrame->move(t);
        }
        void Anim::onRotate(float angle) {
            GameObject::onRotate(angle);
            interpolatedFrame->rotate(angle);
        }
        void Anim::onScale(math::Vec3f& s) {
            GameObject::onScale(s);
            interpolatedFrame->scale(s);
        }
        Entity* Anim::clone() {
            Anim* anim = factory.make_entity<Anim>(factory);
            GameObject::copy(anim);
            anim->fr = fr;
            anim->currentFrameIndex = currentFrameIndex;
            anim->currentFrame = currentFrame;
            anim->previousFrame = previousFrame;
            anim->nextFrame = nextFrame;
            anim->loop = loop;
            anim->interpLevels = interpLevels;
            anim->interpPerc = interpPerc;
            anim->interpolatedFrame.reset(interpolatedFrame->clone());
            return anim;
        }
        void Anim::setAnimUpdater(std::string name) {
            auname = name;
        }
        std::string Anim::getAnimUpdater() {
            return auname;
        }
        Anim::~Anim () {

        }
    }
}
