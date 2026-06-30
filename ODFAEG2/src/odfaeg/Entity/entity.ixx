module;
#include <optional>
#include <array>
#include <vector>
#include <string>
#include <map>
#include <cstdint>
export module odfaeg.entity.entity;
import odfaeg.core.serialization;
import odfaeg.core.stateManager;
namespace odfaeg {
    namespace entity {       
        export class Entity : core::Registered<Entity> {
        public:
            //Get the number of entities created with the application.
            /**
            *  \fn int getNbEntities ()
            *  \brief get the number of entities which exist.
            *  \return the number of entities which exist.
            */
            /*static int getNbEntities ();*/
            /*static int getNbEntitiesTypes();-*
            //Get teh type of the entity.
            /**
            *  \fn std::string getType() const;
            *  \brief return the type's/group name of the entity.
            *  \return the type/group name of the entity.
            */
            std::string getType() {
                return type.second;
            }

            //Get the type's id of the entity.
            /**
            *  \fn int getTypeInt()
            *  \brief get the type/group id of the entity
            *  \return the type/group id of the entity.
            */
            int getTypeInt() {
                return type.first;
            }
            //Get the type corresponding to the given id.
            /**
            *  \fn std::string getTypeOfInt(int type)
            *  \brief get the type's name of the given type's id.
            *  \param the type's id.
            *  \return  int type : the type's name of the given type's id.
            */
            /*static std::string getTypeOfInt (int type);*/
            /**
            *  \fn int getIntOfType (std::string type)
            *  \brief get the type's id of the given type's name.
            *  \param get the type's id of the given type's name.
            *  \param the type's name.
            *  \return the type's id.
            */
            //Get the id corresponding to the given type.
            /*static int getIntOfType (std::string type);*/
            /**
            *  \fn int getRadius()
            *  \brief get the radius of the entity.
            *  \return int the radius of the entity.
            */
            /** \fn std::string getRootType.
            *   \param get the type of the root entity of the scene graph.
            */

            //Init the map who is a correspondance between a type of an entity and the id of it's type.
            /** \fn std::string std::map<int, std::string>
            *   \param get the list of the id's and associated type's names.
            */
            /*static std::map<int, std::string>* initTypes () {
                if (types == nullptr) {
                    static std::map<int, std::string> *t = new std::map<int, std::string> ();
                    return t;
                }
                return types;
            }*/
            //Return the id of the entity.
            /** \fn int getId()
            *   \brief get the id of the entity.
            *   \return the id of the entity.
            */
            int getId() {
                return id;
            }            
            template <typename T> bool addAttribute(std::string name, T value) {
                return entityState.addParameter(name, value);
            }
            /** \fn const StateParameter& getAttribute (const std::string name)
            *   \brief get an attribute of the entity.
            *   \param std::string name : the name of the attribute of the entity.
            *   \return StateParameter& : the attribute of the entity.
            */
            const core::StateParameter& getAttribute(const std::string name) {
                return entityState.getParameter(name);
            }
            /** \fn void changeAttribute (const std::string name)
            *   \brief change the value of the attribute of the entity.
            *   \param std::string name : the name of the attribute of the entity.
            *   \param T value : the value of the attribute.
            */
            template <typename T> void changeAttribute(const std::string name, T value) {
                entityState.changeParameter(name, value);
            }
            /** \fn void removeAttribute (const std::string name)
            *   \brief remove an attribute from the entity.
            *   \return bool if the entity has been correctly removed.
            */
            bool removeAttribute(std::string name) {
                entityState.removeParameter(name);
                return true;
            }
            /** \fn bool interact (StateExecutor *se)
            *   \brief apply a state to the entity.
            *   \return bool if the state has been successfully applied.
            */
            bool interact(core::StateExecutor* se) {
                entityState.setExecutor(se);
                return entityState.doActionState();
            }
            /** \fn bool uninteract (StateExecutor *se)
            *   \brief unapply a state to the entity.
            *   \return bool if the state has been successfully applied.
            */
            bool uninteract(core::StateExecutor* se) {
                entityState.setExecutor(se);
                return entityState.undoActionState();
            }            
            template <typename Archive>
            void vtserialize(Archive& ar) {
                ar(type.first);
                ar(type.second);
                
            }
           
            virtual void setSelected(bool selected) {
                this->selected = selected;
            }            
            bool isSelected() {
                return selected;
            }
            void setVisible(bool visible) {
                this->visible = visible;
            }
            void setId(unsigned int id) {
                this->id = id;
            }
            bool isVisible() {
                return visible;
            }
            uint32_t getEnttID() {
                return enttID;
            }            
            void setEnttID(uint32_t enttID) {
                this->enttID = enttID;
            }   
            void setTypes(std::pair<int, std::string> type) {
                this->type = type;
            }
            bool operator==(Entity& other) {
                return type.second == other.type.second;
            }
            static void setNbEntitiesTypes(unsigned int nbEntitiesTypes) {
                Entity::nbEntitiesTypes = nbEntitiesTypes;
            }
            static unsigned int getNbEntitiesTypes() {
                return nbEntitiesTypes;
            }

        protected:
            /** \fn Entity(math::Vec3f position, math::Vec3f size, math::Vec3f origin, std::string type, Entity* parent)
                * \brief constructor.
                * \param position : the position of the top left corner of the entity
                * \param size : the size of the entity
                * \param origin : the origin of the entity's position wich is local to the entity's position.
                * \param zOrder : the layer's position of the entity
                * \param type : the type of the entity. (the type can be anything (model, floor, light, shadow, etc...))
                * \param the type should describe the kind of the entity.
                * \param parent : the parent of the entity, the value is null if the entity doesn't have a parent.
                */
            Entity(std::string sType, std::string name = "") : entityState("Entity State", nullptr) {
                
                selected = false;
            }
            void copy(Entity* entity) {
                entity->selected = selected;
                entity->visible = visible;
                entity->className = className;
                entity->entityState = entityState;
            }
        private:            
            bool visible, selected;
            std::string className;
            inline static std::map<int, std::string> types = std::map<int, std::string>(); /** A list of the type's id and name's of the entities. */
            std::pair<int, std::string> type; /** The type's id and the type's name of the entity.*/
            int id;
            core::State entityState;                                         /** the states of the entity.*/
            Entity(const Entity& entity) = delete; /**> an entity if not copiable.*/
            Entity& operator=(const Entity& entity) = delete; /**> an entity is not affectable*/            
            uint32_t enttID;   
            inline static unsigned int nbEntitiesTypes = 0;
        };   
    }
}
