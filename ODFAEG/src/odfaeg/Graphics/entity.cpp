#include "../../../include/odfaeg/Graphics/entity.h"
#include "../../../include/odfaeg/Graphics/application.h"
namespace odfaeg {
    namespace graphic {
        //Initialization of static variables.
        int Entity::nbEntities = 0;
        int Entity::nbEntitiesTypes = 0;
        std::map<int, std::string> Entity::types = std::map<int, std::string>();
        //Construct an entity with the given position, size, origin, type and name.
        Entity::Entity (math::Vec3f position, math::Vec3f size, math::Vec3f origin, std::string sType,  EntityFactory& factory, std::string name) :
            Transformable (position, size, origin, name), Drawable(), entityState("Entity State", nullptr), factory(factory) {
            /*If this is an ODFAEG Application and if we call functions from a shared lib (like ODFAEGCreator), we prefer to avoid to use
            static variables which leads to problems because they haven't the expected values when there are used in an executable and in a shared lib.
            So global variables are stored as member variables of the application class which contains everything.
            But if this is a simple application which just draw things on a render window like SFML applications, we can use static variables rather than
            having a factory class to store global variables as member variables in this case.*/
            /*if (core::Application::app != nullptr) {
                id = core::Application::app->getUniqueId()-1;
                type = core::Application::app->updateTypes(sType);
            } else {
                id = nbEntities;
                int iType = getIntOfType(sType);
                if (iType == -1) {
                    type = std::pair<int, std::string> (nbEntitiesTypes, sType);
                    types.insert(type);
                    nbEntitiesTypes++;
                } else {
                    std::map<int, std::string>::iterator it = types.find(iType);
                    type = *it;
                }
                nbEntities++;
            }*/
            type = factory.updateTypes(sType);

            id = factory.getUniqueId();
            getTransform().setEntityId(id);
            selected = false;
        }
        //Setup the static variables when reading entities from an input stream.
        void Entity::onLoad() {
            /*if (core::Application::app != nullptr) {
                type = core::Application::app->updateTypes(getType());
            } else {
                int iType = getIntOfType(getType());
                if (iType == -1) {
                    type = std::pair<int, std::string> (nbEntitiesTypes, getType());
                    types.insert(type);
                    nbEntitiesTypes++;
                }
                nbEntities++;
            }*/

        }
        //Return the number of entities created.
        /*int Entity::getNbEntities () {
            //return nbEntities;
            return factory.getNbEntities();
        }*/
        //Change the type of the entity and updates global variables.
        void Entity::setType(std::string sType) {
            /*if (core::Application::app != nullptr) {
                type = core::Application::app->updateTypes(sType);
            } else {
                int iType = getIntOfType(sType);
                if (iType == -1) {
                    type = std::pair<int, std::string> (nbEntitiesTypes, sType);
                    types.insert(type);
                    nbEntitiesTypes++;
                } else {
                    std::map<int, std::string>::iterator it = types.find(iType);
                    type = *it;
                }
            }*/
            type = factory.updateTypes(sType);
        }
        //Get the type of the entity.
        std::string Entity::getType () const {
            return type.second;
        }
        //We may want to change ids of the entities when we remove ones.
        void Entity::setId (int id) {
            this->id = id;
            onIDUpdated(id);
        }
        int Entity::getId () {

            return id;
        }
        int Entity::getTypeInt () {
            return type.first;
        }
        //int Entity::getIntOfType(std::string sType) {

            /*std::map<int, std::string>::iterator it;
            for (it = types.begin(); it != types.end(); ++it) {
                if (it->second == sType)
                    return it->first;
            }
            return -1;*/
            /*return factory.getIntOfType(sType);
        }*/
        //std::string Entity::getTypeOfInt (int type) {
            /*std::map<int, std::string>::iterator it = types.find(type);
            return it->second;*/
            /*return factory.getTypeOfInt(type);
        }*/
        /*int Entity::getNbEntitiesTypes () {
            //return nbEntitiesTypes;
            factory.getNbEntitiesTypes();
        }*/
        void Entity::setSelected(bool selected) {
            this->selected = selected;
        }
        bool Entity::isSelected() {
            return selected;
        }
        //Copy entities variables to another entity when cloning, so, we don't need to copy them on each clone method redefinition of derived classes.
        void Entity::copy(Entity* entity) {
            entity->m_position = m_position;
            entity->m_size = m_size;
            entity->m_scale = m_scale;
            entity->m_origin = m_origin;
            entity->m_rotation = m_rotation;
            entity->m_center = m_center;
            entity->localBounds = localBounds;
            entity->globalBounds = globalBounds;
            entity->tm = tm;
            entity->name = name;
            entity->type = type;
            entity->entityState = entityState;
            entity->selected = selected;
            entity->className = className;
        }
        //Do nothing by default.
        void Entity::setExternalObjectName(std::string externalObjectName) {

        }
        //Return empty string by default.
        std::string Entity::getExternalObjectName() {
            return "";
        }
        void Entity::setClassName(std::string className) {
            this->className = className;
        }
        std::string Entity::getClassName() {
            return className;
        }
        //Do nothing by default.
        void Entity::setExternal(bool external) {

        }
        //Return false by default.
        bool Entity::isExternal() {
            return false;
        }
        //Do nothing by default.
        void Entity::setLayer(unsigned int layer) {

        }
        //Returns 0 by default.
        unsigned int Entity::getLayer() {
            return 0;
        }
        //Do nothing by default.
        void Entity::setDrawMode(DrawMode dm) {

        }
        //Return normal by default.
        Entity::DrawMode Entity::getDrawMode() {
            return NORMAL;
        }
        //Return the type of this entity by default.
        std::string Entity::getRootType() {
            return type.second;
        }
        //Return the integer value of the type of this entity by default.
        int Entity::getRootTypeInt() {
            return getTypeInt();
        }
        //Return this by default.
        Entity* Entity::getRootEntity() {
            return this;
        }
        Entity* Entity::getParent() const {
            return nullptr;
        }
        //Set the parent's entity of the entity.
        /** \fn void setParent(Entity* parent)
        *   \brief set the parent of the entity.
        *   \param the parent of the entity.
        */
        void Entity::setParent (Entity *parent) {
        }
        //Add a children to the entity.
        /** \fn void addChild(Entity* child)
        *   \brief add a child to the entity.
        *   \param Entity* the entity to add.
        */
        void Entity::addChild (Entity *child) {
        }
        //Remove a children to the entity.
        /** \fn void removeChild(Entity* child)
        *   \brief remove a child from the entity.
        */
        void Entity::removeChild (Entity *child) {
        }
        //Return the children of the entities.
        /** \fn std::vector<Entity*> getChildren() const;
        *   \brief get the list of the children of the entities.
        *   \return std::vector<Entity*> get the entities.
        */
        std::vector<Entity*> Entity::getChildren() const {
            return std::vector<Entity*>();
        }
        //Return the number of entity's children.
        /** \fn  unsigned int getNbChildren ();
        *   \brief get the number of children of the entity.
        *   \return the number of children of the entity.
        */
        unsigned int Entity::getNbChildren () {
            return 0;
        }
        Entity* Entity::getChild(unsigned int n) {
            return nullptr;
        }
        /** \fn void setCollisionVolumme (BoundingVolume* volume)
        *   \brief set the collision volume to the entity.
        *   \param BoundingVolume* volume : the collision volume to set.
        */
        void Entity::setCollisionVolume (physic::BoundingVolume* volume) {
        }
        /** \fn BoundingVolume* getCollisionVolumme ()
        *   \brief get the collision volume of the entity.
        *   \return the collision volume of the entity.
        */
        physic::BoundingVolume* Entity::getCollisionVolume () {
            return nullptr;
        }
        /** \fn void addFace(Face* face)
        *   \brief add a face to the entity.
        */
        void Entity::addFace(Face face) {
        }
        /** \fn std::vector<Face*> getFaces()
        *   \brief get the faces of the entity.
        *   \return std::vector<Face*> the faces of the entity.
        */
        void Entity::updateTransform() {
            getTransform().update();
        }
        void Entity::getCombinedTransform(TransformMatrix& tm) {

        }
         /**
          *\fn setShadowCenter(math::Vec3f shadowCenter)
          *\brief adjust the center of the generated shadow.
          *\param math::Vec3f shadowCenter : the center of the shadow.
        */
        void Entity::setShadowCenter(math::Vec3f shadowCenter) {

        }
        /**
          *\fn getShadowCenter()
          *\brief get the center of the shadow.
          *\return math::Vec3f : the center of the shadow.
        */
        math::Vec3f Entity::getShadowCenter() {
            return math::Vec3f(0,0,0);
        }
        void Entity::setShadowScale(math::Vec3f shadowScale) {
        }
        void Entity::setShadowRotation(float angle, math::Vec3f axis) {
        }
        math::Vec3f Entity::getShadowRotationAxis() {
            return math::Vec3f(0,0,0);
        }
        float Entity::getShadowRotationAngle() {
            return 0;
        }
        math::Vec3f Entity::getShadowScale() {
            return math::Vec3f(0,0,0);
        }
        void Entity::setShadowOrigin(math::Vec3f origin) {
        }
        math::Vec3f Entity::getShadowOrigin() {
            return math::Vec3f(0,0,0);
        }
        void Entity::setBoneIndex (unsigned int boneIndex) {
        }
        unsigned int Entity::getBoneIndex() {
            return 0;
        }
        void Entity::setBoneAnimationIndex(unsigned int index) {
        }
        unsigned int Entity::getBoneAnimationIndex() {
            return 0;
        }
        void Entity::detachChildren() {
        }
        Entity* Entity::getCurrentFrame() const {
            return nullptr;
        }
        sf::Color Entity::getColor() {
            return sf::Color::Transparent;
        }
        void Entity::onFrameChanged() {
        }
        void Entity::changeVerticesHeights(float h1, float h2, float h3, float h4) {
        }
        float Entity::getHeight(math::Vec2f point) {
            return 0;
        }
        void Entity::reset() {
        }
        bool Entity::operator==(Entity& other) {
            return m_position == other.getPosition() && m_size == other.getSize() && type == other.type;
        }
        bool Entity::operator!=(Entity& other) {
            return !(*this == other);
        }
        int Entity::getIntensity() {
            return 0;
        }
        int Entity::getWallType() {
            return -1;
        }
        bool Entity::isRunning() {
            return false;
        }
        sf::Time Entity::getElapsedTime() {
            return sf::Time::Zero;
        }
        float Entity::getFrameRate() {
            return -1;
        }
        void Entity::computeNextFrame() {
        }
        bool Entity::isCurrentFrameChanged() {
            return false;
        }
        void Entity::setCurrentFrameChanged(bool currentFrameChanged) {
        }
        void Entity::resetClock() {
        }
        void Entity::setCurrentFrame(int index) {
        }
        void Entity::play(bool loop) {
        }
        void Entity::stop() {
        }
        void Entity::update(sf::Time dt) {
        }
        void Entity::update() {
        }
        void Entity::addFrame(Entity* frame) {
        }
    }
}
