#ifndef ODFAEG_ENTITY_2D_HPP
#define ODFAEG_ENTITY_2D_HPP
#include "transformable.h"
#include "drawable.h"
#include "../Core/state.h"
#include "../Core/stateExecutor.h"
#include "renderTarget.h"
#include "batcher.h"
#include "../Math/computer.h"
#include "../Core/serialization.h"
#include <vector>
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace graphic {
        class EntityManager;
        /**
          * \file entity.h
          * \class Entity
          * \brief Represent a 2D entity.
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          * \brief Abstract class which can represent any type of entities.
          *  Entities can have 0, 1 or more children and 0 or one parent.
          *  Each entity of the framework inherits from this class.
          */
        class EntityFactory {
            public :
                EntityFactory() {
                    nbEntities = 0;
                    nbEntitiesTypes = 0;
                }
                int getIntOfType(std::string sType) {
                    std::map<int, std::string>::iterator it;
                    for (it = types.begin(); it != types.end(); ++it) {
                        if (it->second == sType) {
                            //std::cout<<"s type : "<<sType<<" int of type :"<<it->first<<std::endl;
                            return it->first;
                        }
                    }
                    return -1;
                }
                std::pair<int, std::string> updateTypes(std::string sType) {
                    int iType = getIntOfType(sType);
                    /*if (sType == "E_WALL")
                        std::cout<<"i type : "<<iType<<std::endl;*/
                    if (iType == -1) {
                        /*if (sType == "E_WALL")
                            std::cout<<"i type  : "<<nbEntitiesTypes<<std::endl;*/
                        std::pair<int, std::string> type = std::pair<int, std::string> (nbEntitiesTypes, sType);
                        types.insert(type);
                        nbEntitiesTypes++;
                        return type;
                    } else {
                        std::map<int, std::string>::iterator it = types.find(iType);
                        /*if (it->second == "E_WALL")
                            std::cout<<"i type : "<<it->first<<std::endl;*/
                        return *it;
                    }
                }
                std::string getTypeOfInt (int type) {
                    std::map<int, std::string>::iterator it = types.find(type);
                    return it != types.end() ? it->second : "";
                }
                int getNbEntitiesTypes () {
                    return nbEntitiesTypes;
                }
                int getNbEntities () {
                    return nbEntities;
                }
                template <typename D, typename... Args>
                D* make_entity(Args&&... args) {
                    return new D(std::forward<Args>(args)...);
                }
                int getUniqueId() {
                    nbEntities++;
                    return nbEntities-1;
                }
            private :
                //EntityFactory(const EntityFactory& entity) = delete; /**> an entity if not copiable.*/
                //EntityFactory& operator=(const EntityFactory& entity) = delete; /**> an entity is not affectable*/
                unsigned int nbEntities, nbEntitiesTypes;
                std::map<int, std::string> types;

        };
        class ODFAEG_API_EXPORT Entity : public Transformable, public Drawable, public core::Registered<Entity> {
            friend class EntityFactory;
            public :
                enum DrawMode {
                    NORMAL, INSTANCED, BASE_INSTANCED
                };

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
                std::string getType() const;
                virtual Entity* getRootEntity();
                //Get the type's id of the entity.
                /**
                *  \fn int getTypeInt()
                *  \brief get the type/group id of the entity
                *  \return the type/group id of the entity.
                */
                int getTypeInt ();
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
                virtual std::string getRootType();
                virtual int getRootTypeInt();
                virtual unsigned int getNbFaces() = 0;
                virtual Face* getFace(unsigned int n) = 0;
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
                int getId();
                //Set an id to the entity.
                 /** \fn void setId(int id)
                *   \brief set the id of the entity.
                *   \param the id of the entity.
                */
                void setId (int id);
                virtual void onIDUpdated(int id) {}
                //This method have to be redifinied to determine when two entity's are equals.
                //This can avoid to have two equqls entity's at the same place.
                /** \fn
                *   \brief abstract method to redefine to compare two entities.
                *   \return if two entities are equals.
                */
                virtual bool operator== (Entity& other);
                virtual bool operator!= (Entity& other);
                void copy (Entity* entity);
                /** \fn
                *   \brief draw (RenderTarger& target, RenderStates states)
                *   \param RenderTarget target : the rendertarget onwhich to draw the entity.
                *   \param RenderStates states : the states used to render the entities.
                */
                /** \fn void onMove (math::Vec3f& t)
                *   \brief virtual method which can be redefined if we need to do something when the entity is moving.
                *   \param math::Vec3f t : the translation of the entity.
                */
                //Return the parent of the entity.
                /** \fn Entity* getParent()
                *   \brief Entity* getParent() const;
                *   \return the parent of the entity.
                */
                virtual Entity* getParent() const;
                //Set the parent's entity of the entity.
                /** \fn void setParent(Entity* parent)
                *   \brief set the parent of the entity.
                *   \param the parent of the entity.
                */
                virtual void setParent (Entity *parent);
                //Add a children to the entity.
                /** \fn void addChild(Entity* child)
                *   \brief add a child to the entity.
                *   \param Entity* the entity to add.
                */
                virtual void addChild (Entity *child);
                //Remove a children to the entity.
                /** \fn void removeChild(Entity* child)
                *   \brief remove a child from the entity.
                */
                virtual void removeChild (Entity *child);
                virtual sf::Color getColor();
                //Return the children of the entities.
                /** \fn std::vector<Entity*> getChildren() const;
                *   \brief get the list of the children of the entities.
                *   \return std::vector<Entity*> get the entities.
                */
                virtual std::vector<Entity*> getChildren() const;
                //Return the number of entity's children.
                /** \fn  unsigned int getNbChildren ();
                *   \brief get the number of children of the entity.
                *   \return the number of children of the entity.
                */
                virtual unsigned int getNbChildren ();
                virtual Entity* getChild(unsigned int n);
                //Determine if the entity is animated.
                /** \fn  bool isAnimated ();
                *   \brief virtual method to redefine if an entity is animated.
                *   \return if the entity is animated.
                */
                virtual bool isAnimated() const = 0;
                //Determine if the entity have a shadow.
                /**
                * \fn bool isModel() const;
                * \brief virtual method to redefine if an entity is a model.
                * \return if the entity is a model.
                */
                virtual bool isModel() const = 0;
                /**
                * \fn bool isSelectable() const;
                * \brief virtual method to redefine if an entity is selectable.
                * \return if the entity is selectable.
                */
                virtual bool selectable() const = 0;
                /**
                * \fn bool isLight() const;
                * \brief virtual method to redefine if an entity is a light.
                * \return if the entity is a light.
                */
                virtual bool isLight() const = 0;
                /**
                * \fn bool isShadow() const;
                * \brief virtual method to redefine if an entity is a shadow.
                * \return if the entity is a shadow.
                */
                virtual bool isShadow() const = 0;
                /**
                * \fn bool isLeaf() const;
                * \brief virtual method to redefine if an entity is a leaf. (the entity have no children)
                * \return if the entity is a leaf.
                */
                virtual bool isLeaf() const = 0;
                //Set the type of the entity.
                /** \fn void setType (std::string type)
                *   \brief set the type's name of the entity.
                *   \param type : the type's name of the entity.
                */
                void setType (std::string type);
                void setClassName(std::string className);
                std::string getClassName();
                //Destructor.
                /** \fn void addAttribute (std::string name, T value)
                *   \brief add an attribute to the entity.
                *   \param std::string the name of the attribute.
                *   \param T the value of the attribute.
                *   \return if the attribute has been correctly added.
                */
                template <typename T> bool addAttribute (std::string name, T value) {
                    return entityState.addParameter(name, value);
                }
                /** \fn const StateParameter& getAttribute (const std::string name)
                *   \brief get an attribute of the entity.
                *   \param std::string name : the name of the attribute of the entity.
                *   \return StateParameter& : the attribute of the entity.
                */
                const core::StateParameter& getAttribute (const std::string name) {
                       return entityState.getParameter(name);
                }
                /** \fn void changeAttribute (const std::string name)
                *   \brief change the value of the attribute of the entity.
                *   \param std::string name : the name of the attribute of the entity.
                *   \param T value : the value of the attribute.
                */
                template <typename T> void changeAttribute (const std::string name, T value) {
                    entityState.changeParameter(name, value);
                }
                /** \fn void removeAttribute (const std::string name)
                *   \brief remove an attribute from the entity.
                *   \return bool if the entity has been correctly removed.
                */
                bool removeAttribute (std::string name) {
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
                bool uninteract (core::StateExecutor* se) {
                    entityState.setExecutor(se);
                    return entityState.undoActionState();
                }
                /** \fn void setCollisionVolumme (BoundingVolume* volume)
                *   \brief set the collision volume to the entity.
                *   \param BoundingVolume* volume : the collision volume to set.
                */
                virtual void setCollisionVolume (physic::BoundingVolume* volume);
                /** \fn BoundingVolume* getCollisionVolumme ()
                *   \brief get the collision volume of the entity.
                *   \return the collision volume of the entity.
                */
                virtual physic::BoundingVolume* getCollisionVolume ();
                /** \fn void addFace(Face* face)
                *   \brief add a face to the entity.
                */
                virtual void addFace(Face face);
                /** \fn std::vector<Face*> getFaces()
                *   \brief get the faces of the entity.
                *   \return std::vector<Face*> the faces of the entity.
                */
                virtual std::vector<Face>& getFaces() = 0;
                static int getNbEntityTypes();
                /** \fn void vtserialize(Archive & ar)
                *   \brief serialize the entity into an archive.
                *   \param Archive : the archive onwhich to serialize the entities.
                */
                virtual void updateTransform();
                //void setTypeInt(int iType);
                template <typename Archive>
                void vtserialize(Archive & ar) {
                    //std::cout<<"already serialized : "<<alreadySerialized<<std::endl;
                    Transformable::serialize(ar);
                    ar(id);
                    ar(type.first);
                    //std::cout<<"type first : "<<type.first<<std::endl;
                    ar(type.second);
                    /*if (type.second == "E_WALL")
                        std::cout<<"i type : "<<type.first<<std::endl;*/
                    //std::cout<<"entity id : "<<getId()<<std::endl<<"Transform matrix : "<<getTransform().getMatrix()<<std::endl;
                    if (ar.isInputArchive())
                        type = factory.updateTypes(getType());
                }
                /** \fn void onLoad()
                *   \brief load the entities.
                */
                void onLoad();
                virtual void getCombinedTransform(TransformMatrix& tm);
                 /**
                  *\fn setShadowCenter(math::Vec3f shadowCenter)
                  *\brief adjust the center of the generated shadow.
                  *\param math::Vec3f shadowCenter : the center of the shadow.
                */
                virtual void setShadowCenter(math::Vec3f shadowCenter);
                /**
                  *\fn getShadowCenter()
                  *\brief get the center of the shadow.
                  *\return math::Vec3f : the center of the shadow.
                */
                virtual math::Vec3f getShadowCenter();
                virtual void setShadowScale(math::Vec3f shadowScale);
                virtual void setShadowRotation(float angle, math::Vec3f axis = math::Vec3f::zAxis);
                virtual math::Vec3f getShadowRotationAxis();
                virtual float getShadowRotationAngle();
                virtual math::Vec3f getShadowScale();
                virtual void setShadowOrigin(math::Vec3f origin);
                virtual math::Vec3f getShadowOrigin();
                virtual void setBoneIndex (unsigned int boneIndex);
                virtual unsigned int getBoneIndex();
                virtual void setBoneAnimationIndex(unsigned int index);
                virtual unsigned int getBoneAnimationIndex();
                virtual DrawMode getDrawMode();
                virtual void setDrawMode (DrawMode);
                virtual void setLayer(unsigned int layer);
                virtual void changeVerticesHeights(float h1, float h2, float h3, float h4);
                virtual float getHeight(math::Vec2f point);
                virtual unsigned int getLayer();
                virtual Entity* clone() = 0;
                virtual void setExternal(bool external);
                virtual bool isExternal();
                virtual void setExternalObjectName(std::string externalObjectName);
                virtual std::string getExternalObjectName();
                virtual void detachChildren();
                virtual Entity* getCurrentFrame() const;

                virtual void setSelected(bool selected);
                bool isSelected();
                virtual int getIntensity();
                virtual void onFrameChanged();
                virtual void reset();
                virtual void draw(RenderTarget& target, RenderStates states) = 0;
                virtual int getWallType();
                virtual void addFrame(Entity* frame);
                virtual bool isRunning();
                virtual sf::Time getElapsedTime();
                virtual float getFrameRate();
                virtual void computeNextFrame();
                virtual bool isCurrentFrameChanged();
                virtual void setCurrentFrameChanged(bool currentFrameChanged);
                virtual void setCurrentFrame(int index);
                virtual void resetClock();
                virtual void play(bool loop);
                virtual void stop();
                virtual void update(sf::Time dt);
                virtual void update();
            protected :
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
                Entity (math::Vec3f position, math::Vec3f size, math::Vec3f origin, std::string type, EntityFactory& factory, std::string name = "");
                EntityFactory& factory;
            private :
                std::string className;
                static std::map<int, std::string> types; /** A list of the type's id and name's of the entities. */
                std::pair<int, std::string> type; /** The type's id and the type's name of the entity.*/
                int id; /** the id of the entity.*/
                bool selected;
                static int nbEntities, nbEntitiesTypes; /** the number of entities and the number of entities types.*/
                core::State entityState; /** the states of the entity.*/
                Entity(const Entity& entity) = delete; /**> an entity if not copiable.*/
                Entity& operator=(const Entity& entity) = delete; /**> an entity is not affectable*/
        };
    }
}
#endif
