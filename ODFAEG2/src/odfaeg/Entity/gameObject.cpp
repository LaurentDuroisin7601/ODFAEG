module;
#include <vulkan/vulkan.hpp>
#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <deque>
//import odfaeg.graphic.gameObject;
module odfaeg.entity.gameObject;
import odfaeg.entity.impl;
namespace odfaeg {
	namespace entity { 
        std::string SubMesh::getTextureId(TexType texType, unsigned int texUnit) {
            return (texturesIds[texType].size() > 0) ? texturesIds[texType][texUnit] : "";
        }
        void SubMesh::setTextureId(TexType texType, std::string texId, unsigned int texUnit) {
            for (unsigned int i = 0; i < NBTEXTYPES; i++) {
                if (texUnit >= texturesIds[i].size())
                    texturesIds[i].resize(texUnit+1);
            }            
            texturesIds[texType][texUnit] = texId;            
        }
        void SubMesh::setType(Type type) {            
            this->type = type;
        }
        SubMesh::Type SubMesh::getType() {
            return type;
        }      
        VertexArray& SubMesh::getVertexArray() {
            /*if (m_vertices.getEntity() != nullptr && m_vertices.getEntity()->getRootType() == "E_MONSTER")
                //////std::cout<<"face tex coords : "<<m_vertices[0].texCoords.x<<","<<m_vertices[0].texCoords.y<<std::endl;*/
            return m_vertices;
        }
        void SubMesh::setVertexArray(VertexArray va) {
            m_vertices = va;
            //std::cout<<"swap submesh : "<<m_vertices.getVertexCount()<<std::endl;
            /*for (unsigned int i = 0; i < vb.getNbBuffers(); i++) {
                std::cout<<"buffer : "<<vb.getVertexBuffer(i).getHandle();
                std::cout<<"index buffer : "<<vb.getIndexBuffer(i).getHandle();
            }*/
            /*if (m_vertices.getEntity()->getRootType() == "E_MONSTER")
                //////std::cout<<"face tex coords : "<<m_vertices[0].texCoords.x<<","<<m_vertices[0].texCoords.y<<std::endl;*/
        }        
        bool SubMesh::operator==(SubMesh& other) {
            if (m_vertices.getVertexCount() != other.m_vertices.getVertexCount())
                return false;
            for (unsigned int i = 0; i < m_vertices.getVertexCount(); i++)
                if (m_vertices[i] != other.m_vertices[i])
                    return false;
            return true;
        }
        bool SubMesh::operator!=(SubMesh& other) {
            return !(*this == other);
        }	    
	    GameObject::GameObject(std::string type) : entity::Entity(type) {
            entity::EnttEntity::initEntity(*this);
        }
        GameObject::GameObject(math::Vec3f position, math::Vec3f size, math::Vec3f origin, std::string sType, std::string name, GameObject* parent) :
            entity::Entity(sType, name), Transformable (position, size, origin) {
            entity::EnttEntity::initEntity(*this);
            this->parent = parent;            
            shadowOrigin = math::Vec3f(0.f, 0.f, 0.f);
            shadowCenter = math::Vec3f(0.f, 0.f, 0.f);
            shadowScale = math::Vec3f(1.f, 1.f, 1.f);
            shadowRotationAngle = 0;
            shadowRotationAxis = math::Vec3f(0.f, 0.f, 1.f);
            animIndex = 0;
            external = false;
            this->name = name;
            externalObjectName = "";

            /*if (sType == "E_HERO")
                setName("E_HERO");*/
        }
        void GameObject::copy(GameObject* gameObject) {
            ////////std::cout<<"copy entity : "<<getPosition()<<getType()<<std::endl;
            Entity::copy(gameObject);
            entity::EnttEntity::initEntity(*gameObject);
            //gameObject->parent = parent;
            ////////std::cout<<"parent : "<<parent<<",this : "<<this<<std::endl;
            //gameObject->collisionVolume = (collisionVolume == nullptr) ? nullptr : getCollisionVolume()->clone();
            gameObject->shadowOrigin = getShadowOrigin();
            gameObject->shadowScale = getShadowScale();
            gameObject->shadowRotationAngle = getShadowRotationAngle();
            gameObject->shadowRotationAxis = getShadowRotationAxis();
            gameObject->shadowCenter = getShadowCenter();
            gameObject->animIndex = animIndex;
            gameObject->externalObjectName = externalObjectName;
            gameObject->external = external;
            gameObject->m_position = m_position;
            gameObject->m_size = m_size;
            gameObject->m_scale = m_scale;
            gameObject->m_origin = m_origin;
            gameObject->m_rotation = m_rotation;
            gameObject->m_center = m_center;
            gameObject->localBounds = localBounds;
            gameObject->globalBounds = globalBounds;
            gameObject->originRelative = originRelative;
            gameObject->m_relOrigin = m_relOrigin;
            gameObject->tm = tm;
            for (unsigned int i = 0; i < getChildren().size(); i++) {
                GameObject* child = getChildren()[i]->clone();
                child->setParent(gameObject);
                gameObject->addChild(child);
            }
            ////////std::cout<<"nb entities type : "<<nbEntitiesTypes<<std::endl;
        }
        unsigned int GameObject::getSubMeshesCount() {
            return subMeshes.size();
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
        GameObject* GameObject::getRoot() {
            if (parent == nullptr)
                return this;
            return parent->getRoot();
        }        
        void GameObject::setSelected(bool selected) {

            Entity::setSelected(selected);
            for (unsigned int i = 0; i < getChildren().size(); i++) {
                getChildren()[i]->setSelected(selected);
            }
        }
        GameObject* GameObject::getChild(unsigned int n) {
            if (n >= 0 && n < children.size())
                return children[n].get();
            return nullptr;
        }
        //Add a child and recompute the size of the parent.
        void GameObject::addChild(GameObject* child) {
            std::vector<math::Vec3f> vecs;
            std::unique_ptr<GameObject> ptr;
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
        void GameObject::detachChild(GameObject* child) {
            std::vector<std::unique_ptr<GameObject>>::iterator it;
            for (it = children.begin(); it != children.end();) {
                if (it->get() == child) {
                    it->release();
                    it = children.erase(it);
                }
                else
                    it++;
            }
        }
        void GameObject::removeChild(GameObject* child) {
            std::vector<std::unique_ptr<GameObject>>::iterator it;
            for (it = children.begin(); it != children.end();) {
                if (it->get() == child) {
                    delete it->release();
                    it = children.erase(it);
                }
                else
                    it++;
            }
        }
        void GameObject::updateTransform() {

            getTransform().update();
            for (unsigned int i = 0; i < getChildren().size(); i++) {
                getChildren()[i]->updateTransform();
            }
        }
        //Return the children of the entities.
        std::vector<GameObject*> GameObject::getChildren() const {
            std::vector<GameObject*> childs;
            for (unsigned int i = 0; i < children.size(); i++)
                childs.push_back(children[i].get());
            return childs;
        }
        void GameObject::setParent(GameObject* parent) {
            this->parent = parent;
        }
        GameObject* GameObject::getParent() const {
            return parent;
        }        
        void GameObject::onResize(math::Vec3f& s) {            
            for (unsigned int i = 0; i < getChildren().size(); i++) {
                getChildren()[i]->setSize(s);
            }
        }        
        void GameObject::onMove(math::Vec3f& t) {
            /*if (getRootType() == "E_HERO")
                //////std::cout<<"matrix : "<<getTransform().getMatrix()<<std::endl;*/
                ////////std::cout<<"move : "<<getType()<<"t : "<<t<<std::endl;            
            for (unsigned int i = 0; i < getChildren().size(); i++) {
                getChildren()[i]->move(t);
            }

        }
        void GameObject::onScale(math::Vec3f& s) {
            //updateTransform();            
            for (unsigned int i = 0; i < children.size(); i++) {
                children[i]->setScale(s);
            }

        }
        void GameObject::onRotate(float angle) {
            //updateTransform();            
            for (unsigned int i = 0; i < children.size(); i++) {
                children[i]->setRotation(angle);
            }
        }
        void GameObject::addSubMesh(SubMesh subMesh) {
            subMeshes.push_back(std::move(subMesh));
        }
        std::deque<SubMesh>& GameObject::getSubMeshes() {
            return subMeshes;
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
            for (unsigned int i = 0; i < getChildren().size(); i++) {
                getChildren()[i]->setShadowScale(shadowScale);
            }
        }
        math::Vec3f GameObject::getShadowScale() {
            return shadowScale;
        }
        void GameObject::setShadowRotation(float angle, math::Vec3f axis) {
            this->shadowRotationAngle = angle;
            this->shadowRotationAxis = axis;
            for (unsigned int i = 0; i < getChildren().size(); i++) {
                getChildren()[i]->setShadowRotation(angle, axis);
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
            for (unsigned int i = 0; i < getChildren().size(); i++) {
                getChildren()[i]->setShadowOrigin(origin);
            }
        }
        math::Vec3f GameObject::getShadowOrigin() {
            return shadowOrigin;
        }
        void GameObject::setAnimIndex(unsigned int animIndex) {
            this->animIndex = animIndex;
            for (unsigned int i = 0; i < children.size(); i++) {
                getChildren()[i]->setAnimIndex(animIndex);
            }
        }
        unsigned int GameObject::getAnimIndex() {
            return animIndex;
        }
        void GameObject::detachChildren() {
            std::vector<std::unique_ptr<GameObject>>::iterator it;
            for (it = children.begin(); it != children.end();) {
                it->release();
                it = children.erase(it);
            }
            children.clear();
        }        
        bool GameObject::operator==(GameObject& other) {
            if (!entity::Entity::operator==(other))
                return false;
            if (getChildren().size() != other.getChildren().size())
                return false;
            for (unsigned int i = 0; i < children.size(); i++) {
                if (*getChildren()[i] != *other.getChildren()[i])
                    return false;
            }
            if (subMeshes.size() != getSubMeshes().size())
                return false;
            for (unsigned int i = 0; i < subMeshes.size(); i++) {
                if (subMeshes[i] != getSubMeshes()[i])
                    return false;
            }
            /*if (collisionVolume != nullptr && other.getCollisionVolume() == nullptr || other.getCollisionVolume() != nullptr && collisionVolume == nullptr)
                return false;
            if (collisionVolume == nullptr)*/
                return parent == other.getParent() && animIndex == other.getAnimIndex() 
                && shadowCenter == other.getShadowCenter() && shadowScale == other.getShadowScale() && shadowOrigin == other.getShadowOrigin()
                && shadowRotationAxis == other.getShadowRotationAxis() && shadowRotationAngle == other.getShadowRotationAngle();
            /*return parent == other.getParent() && *collisionVolume == *other.getCollisionVolume() && boneIndex == other.getBoneIndex() && layer == other.getLayer()
                && shadowCenter == other.getShadowCenter() && shadowScale == other.getShadowScale() && shadowOrigin == other.getShadowOrigin()
                && shadowRotationAxis == other.getShadowRotationAxis() && shadowRotationAngle == other.getShadowRotationAngle();*/
        }
        bool GameObject::operator!=(GameObject& other) {
            return !(*this == other);
        }
	    int& GameObject::getBoneCount() {
            return m_BoneCounter;
        }
	    std::map<std::string, GameObject::BoneInfo>& GameObject::getBoneInfoMap() {
            return m_BoneInfoMap;
        }
        GameObject::~GameObject() {

        }
	}
}