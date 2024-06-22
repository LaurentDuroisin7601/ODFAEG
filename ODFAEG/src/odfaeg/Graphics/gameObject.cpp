#include "../../../include/odfaeg/Graphics/gameObject.hpp"
namespace odfaeg {
    namespace graphic {
        unsigned int GameObject::nbLayers = 1;
        GameObject::GameObject (math::Vec3f position, math::Vec3f size, math::Vec3f origin, std::string sType,  EntityFactory& factory, std::string name, Entity *parent) :
            Entity (position, size, origin, sType, factory, name) {
            this->parent = parent;
            alreadySerialized = false;
            collisionVolume = nullptr;
            shadowOrigin = math::Vec3f (0, 0, 0);
            shadowCenter = math::Vec3f (0, 0, 0);
            shadowScale = math::Vec3f(1.f, 1.f, 1.f);
            shadowRotationAngle = 0;
            shadowRotationAxis = math::Vec3f::zAxis;
            boneIndex = 0;
            drawMode = NORMAL;
            layer = 0;
            external = false;
            externalObjectName = "";
            /*if (sType == "E_HERO")
                setName("E_HERO");*/
        }
        void GameObject::copy(GameObject* gameObject) {
            //std::cout<<"copy entity : "<<getPosition()<<getType()<<std::endl;
            Entity::copy(gameObject);
            //gameObject->parent = parent;
            //std::cout<<"parent : "<<parent<<",this : "<<this<<std::endl;
            gameObject->alreadySerialized = false;
            gameObject->collisionVolume = (collisionVolume == nullptr) ? nullptr : getCollisionVolume()->clone();
            gameObject->shadowOrigin = getShadowOrigin();
            gameObject->shadowScale = getShadowScale();
            gameObject->shadowRotationAngle = getShadowRotationAngle();
            gameObject->shadowRotationAxis = getShadowRotationAxis();
            gameObject->shadowCenter = getShadowCenter();
            gameObject->boneIndex = boneIndex;
            gameObject->drawMode = drawMode;
            gameObject->layer = layer;
            gameObject->externalObjectName = externalObjectName;
            gameObject->external = external;
            gameObject->drawMode = drawMode;

            //std::cout<<"clone id : "<<entity->getId()<<std::endl;
            for (unsigned int i = 0; i < faces.size(); i++) {
                gameObject->addFace(Face(faces[i].getVertexArray(), faces[i].getMaterial(), gameObject->getTransform()));
                gameObject->getFace(i)->getVertexArray().setEntity(gameObject);
                /*if (entity->getRootType() == "E_WALL")
                    std::cout<<"alias : "<<entity->getFace(i)->getMaterial().getTexId();*/
            }
            for (unsigned int i = 0; i < children.size(); i++) {
                Entity* child = children[i]->clone();
                child->setParent(gameObject);
                gameObject->addChild(child);
            }
            //std::cout<<"nb entities type : "<<nbEntitiesTypes<<std::endl;
        }
        void GameObject::setExternalObjectName(std::string externalObjectName) {
            this->externalObjectName = externalObjectName;
        }
        std::string GameObject::getExternalObjectName() {
            return externalObjectName;
        }
        void GameObject::setExternal(bool external) {
            this->external = external;
        }
        bool GameObject::isExternal() {
            return external;
        }
        void GameObject::setLayer(unsigned int layer) {
            this->layer = layer;
            if (layer >= nbLayers) {
                nbLayers = layer+1;
            }
            for (unsigned int i = 0; i < faces.size(); i++) {
                faces[i].getMaterial().setLayer(layer);
            }
            for (unsigned int i = 0; i < children.size(); i++) {
                children[i]->setLayer(layer);
            }
        }
        unsigned int GameObject::getLayer() {
            return layer;
        }
        unsigned int GameObject::getNbLayers() {
            return nbLayers;
        }
        void GameObject::setDrawMode(DrawMode dm) {
            drawMode = dm;
        }
        Entity::DrawMode GameObject::getDrawMode() {
            return drawMode;
        }
        std::string GameObject::getRootType() {
            if (parent != nullptr) {
                return parent->getRootType();
            }
            return getType();
        }
        int GameObject::getRootTypeInt() {
            if (parent != nullptr) {
                /*if (parent->getType() == "E_WALL")
                    std::cout<<"parent type : "<<parent->getTypeInt()<<std::endl;*/
                return parent->getTypeInt();
            }
            /*if (getType() == "E_WALL")
                std::cout<<"type : "<<getType()<<" int of type :  "<<getTypeInt()<<std::endl;*/
            return getTypeInt();
        }
        Entity* GameObject::getRootEntity() {
            if (parent == nullptr)
                return this;
            return parent->getRootEntity();
        }
        void GameObject::draw (RenderTarget& target, RenderStates states) {
            states.transform = getTransform();
            onDraw(target, states);
            for (unsigned int i = 0; i < children.size(); i++) {
                children[i]->draw(target, states);
            }
        }
        void GameObject::setSelected(bool selected) {

            Entity::setSelected(selected);
            for (unsigned int i = 0; i < children.size(); i++) {
                children[i]->setSelected(selected);
            }
        }
        Entity* GameObject::getChild(unsigned int n) {
            if (n >= 0 && n < children.size())
                return children[n].get();
            return nullptr;
        }
        //Add a child and recompute the size of the parent.
        void GameObject::addChild (Entity* child) {
            std::vector<math::Vec3f> vecs;
            std::unique_ptr<Entity> ptr;
            ptr.reset(child);
            children.push_back(std::move(ptr));
            /*for (unsigned int i = 0; i < children.size(); i++) {
                vecs.push_back(children[i]->getPosition());
                vecs.push_back(children[i]->getPosition() + children[i]->getSize());


            }
            std::array<std::array<float, 2>, 3> minsMaxs;
            minsMaxs = math::Computer::getExtends(vecs);
            math::Vec3f pos((int) minsMaxs[0][0], (int) minsMaxs[1][0], (int) minsMaxs[2][0]);
            math::Vec3f size((int) minsMaxs[0][1] - (int) minsMaxs[0][0], (int) minsMaxs[1][1] - (int) minsMaxs[1][0], (int) minsMaxs[2][1] - (int) minsMaxs[2][0]);
            setLocalBounds(physic::BoundingBox(pos.x, pos.y, pos.z, size.x, size.y, size.z));

            vecs.clear();*/
        }
        void GameObject::removeChild (Entity *child) {
            std::vector<std::unique_ptr<Entity>>::iterator it;
            for (it = children.begin(); it != children.end();) {
                if (it->get() == child) {
                    delete it->release();
                    it = children.erase(it);
                } else
                    it++;
            }
        }
        void GameObject::updateTransform() {

            getTransform().update();
            for (unsigned int i = 0; i < children.size(); i++) {
                children[i]->updateTransform();
            }
        }
        //Return the children of the entities.
        std::vector<Entity*> GameObject::getChildren() const {
            std::vector<Entity*> childs;
            for (unsigned int i = 0; i < children.size(); i++)
                childs.push_back(children[i].get());
            return childs;
        }

        //Return the number of entity's children.
        unsigned int GameObject::getNbChildren () {
            return children.size();
        }
        void GameObject::setParent(Entity *parent) {
            this->parent = parent;
        }
        Entity* GameObject::getParent() const {
            return parent;
        }
        void GameObject::onOriginChanged(math::Vec3f& o) {
            for (unsigned int i = 0; i < children.size(); i++) {
                children[i]->setOrigin(o);
            }
        }
        void GameObject::onResize(math::Vec3f& s) {
            for (unsigned int i = 0; i < faces.size(); i++) {
                faces[i].setTransformMatrix(getTransform());
            }
            for (unsigned int i = 0; i < children.size(); i++) {
                children[i]->setSize(s);
            }
        }
        void GameObject::onMove(math::Vec3f &t) {
            /*if (getRootType() == "E_HERO")
                std::cout<<"matrix : "<<getTransform().getMatrix()<<std::endl;*/
            //std::cout<<"move : "<<getType()<<"t : "<<t<<std::endl;
            for (unsigned int i = 0; i < faces.size(); i++) {
                getTransform().update();
                faces[i].setTransformMatrix(getTransform());
            }
            for (unsigned int i = 0; i < children.size(); i++) {
                children[i]->move(t);
            }


        }
        void GameObject::onScale(math::Vec3f &s) {
            //updateTransform();
            for (unsigned int i = 0; i < faces.size(); i++) {
                getTransform().update();
                faces[i].setTransformMatrix(getTransform());
            }
            for (unsigned int i = 0; i < children.size(); i++) {
                children[i]->scale(s);
            }

        }
        void GameObject::onRotate(float angle) {
            //updateTransform();
            for (unsigned int i = 0; i < faces.size(); i++) {
                getTransform().update();
                faces[i].setTransformMatrix(getTransform());
            }
            for (unsigned int i = 0; i < children.size(); i++) {
                children[i]->rotate(angle);
            }
        }
        void GameObject::addFace (Face face) {
            faces.push_back(face);
        }
        std::vector<Face>& GameObject::getFaces()  {
            return faces;
        }
        unsigned int GameObject::getNbFaces() {
            return faces.size();
        }
        Face* GameObject::getFace(unsigned int n) {
            if (n >= 0 && n < faces.size())
                return &faces[n];
            return nullptr;
        }
        void GameObject::setShadowCenter(math::Vec3f shadowCenter) {
            this->shadowCenter = shadowCenter;
            for (unsigned int i = 0; i < children.size(); i++) {
                children[i]->setShadowCenter(shadowCenter);
            }
        }
        math::Vec3f GameObject::getShadowCenter() {
            return shadowCenter;
        }
        void GameObject::setShadowScale(math::Vec3f shadowScale) {
            this->shadowScale = shadowScale;
            for (unsigned int i = 0; i < children.size(); i++) {
                children[i]->setShadowScale(shadowScale);
            }
        }
        math::Vec3f GameObject::getShadowScale() {
            return shadowScale;
        }
        void GameObject::setShadowRotation(float angle, math::Vec3f axis) {
            this->shadowRotationAngle = angle;
            this->shadowRotationAxis = axis;
            for (unsigned int i = 0; i < children.size(); i++) {
                children[i]->setShadowRotation(angle, axis);
            }
        }
        math::Vec3f GameObject::getShadowRotationAxis() {
            return shadowRotationAxis;
        }
        float GameObject::getShadowRotationAngle() {
            return shadowRotationAngle;
        }
        void GameObject::setShadowOrigin(math::Vec3f origin) {
            shadowOrigin = origin;
            for (unsigned int i = 0; i < children.size(); i++) {
                children[i]->setShadowOrigin(origin);
            }
        }
        math::Vec3f GameObject::getShadowOrigin() {
            return shadowOrigin;
        }
        void GameObject::setBoneIndex(unsigned int boneIndex) {
            this->boneIndex = boneIndex;
            for (unsigned int i = 0; i < children.size(); i++) {
                children[i]->setBoneIndex(boneIndex);
            }
        }
        unsigned int GameObject::getBoneIndex() {
            return boneIndex;
        }
        void GameObject::detachChildren() {
            for (unsigned int i = 0; i < children.size(); i++) {
                children[i].release();
            }
        }
        void GameObject::reset() {
            alreadySerialized = false;
        }
        bool GameObject::operator==(Entity& other) {
            if (!Entity::operator==(other))
                return false;
            if (children.size() != other.getChildren().size())
                return false;
            for (unsigned int i = 0; i < children.size(); i++) {
                if (*children[i] != *other.getChildren()[i])
                    return false;
            }
            if (faces.size() != getFaces().size())
                return false;
            for (unsigned int i = 0; i < faces.size(); i++) {
                if (faces[i] != getFaces()[i])
                    return false;
            }
            if (collisionVolume != nullptr && other.getCollisionVolume() == nullptr || other.getCollisionVolume() != nullptr && collisionVolume == nullptr)
                return false;
            if (collisionVolume == nullptr)
                return parent == other.getParent() && boneIndex == other.getBoneIndex() && layer == other.getLayer()
                    && shadowCenter == other.getShadowCenter() && shadowScale == other.getShadowScale() && shadowOrigin == other.getShadowOrigin()
                    && shadowRotationAxis == other.getShadowRotationAxis() && shadowRotationAngle == other.getShadowRotationAngle();
            return parent == other.getParent() && *collisionVolume == *other.getCollisionVolume() && boneIndex == other.getBoneIndex() && layer == other.getLayer()
                    && shadowCenter == other.getShadowCenter() && shadowScale == other.getShadowScale() && shadowOrigin == other.getShadowOrigin()
                    && shadowRotationAxis == other.getShadowRotationAxis() && shadowRotationAngle == other.getShadowRotationAngle();
        }
        bool GameObject::operator!=(Entity& other) {
            return !(*this == other);
        }
    }
}

