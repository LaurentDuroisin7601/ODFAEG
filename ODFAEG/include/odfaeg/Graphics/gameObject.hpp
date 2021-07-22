#ifndef ODFAEG_GAME_OBJECT_HPP
#define ODFAEG_GAME_OBJECT_HPP
#include "entity.h"
namespace odfaeg {
    namespace graphic {
        class GameObject : public Entity {
            public :
            GameObject (math::Vec3f position, math::Vec3f size, math::Vec3f origin, std::string type, EntityFactory& factory, std::string name = "", Entity* parent = nullptr);
            Entity* getRootEntity();
            /** \fn std::string getRootType.
            *   \param get the type of the root entity of the scene graph.
            */
            std::string getRootType();
            int getRootTypeInt();
            unsigned int getNbFaces();
            Face* getFace(unsigned int n);
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
            //This method have to be redifinied to determine when two entity's are equals.
            //This can avoid to have two equqls entity's at the same place.

            /** \fn
            *   \brief draw (RenderTarger& target, RenderStates states)
            *   \param RenderTarget target : the rendertarget onwhich to draw the entity.
            *   \param RenderStates states : the states used to render the entities.
            */
            void draw (RenderTarget& target, RenderStates states);
            /** \fn
            *   \brief void onDraw (RenderTarger& target, RenderStates states)
            *   \param RenderTarget target : the rendertarget onwhich to draw the entities.
            *   \param RenderStates states : the states used to render the entities.
            */
            virtual void onDraw(RenderTarget &target, RenderStates states) {}
            /** \fn void onMove (math::Vec3f& t)
            *   \brief virtual method which can be redefined if we need to do something when the entity is moving.
            *   \param math::Vec3f t : the translation of the entity.
            */
            virtual void onOriginChanged(math::Vec3f& origin);
            virtual void onResize(math::Vec3f& s);
            virtual void onMove(math::Vec3f &t);
            /** \fn void onScale (math::Vec3f& s)
            *   \brief virtual method which can be redefined if we need to do something when the entity is rescaling.
            *   \param math::Vec3f& s : the scale of the entity.
            */
            virtual void onScale(math::Vec3f &s);
            /** \fn void onRotate (float angle)
            *   \brief virtual method which can be redefined if we need to do something when the entity is rotating.
            *   \param math::Vec3f& s : the rotation angle of the entity.
            */
            virtual void onRotate(float angle);
            //Return the parent of the entity.
            /** \fn Entity* getParent()
            *   \brief Entity* getParent() const;
            *   \return the parent of the entity.
            */
            Entity* getParent() const;
            //Set the parent's entity of the entity.
            /** \fn void setParent(Entity* parent)
            *   \brief set the parent of the entity.
            *   \param the parent of the entity.
            */
            void setParent (Entity *parent);
            //Add a children to the entity.
            /** \fn void addChild(Entity* child)
            *   \brief add a child to the entity.
            *   \param Entity* the entity to add.
            */
            void addChild (Entity *child);
            //Remove a children to the entity.
            /** \fn void removeChild(Entity* child)
            *   \brief remove a child from the entity.
            */
            void removeChild (Entity *child);
            //Return the children of the entities.
            /** \fn std::vector<Entity*> getChildren() const;
            *   \brief get the list of the children of the entities.
            *   \return std::vector<Entity*> get the entities.
            */
            std::vector<Entity*> getChildren() const;
            //Return the number of entity's children.
            /** \fn  unsigned int getNbChildren ();
            *   \brief get the number of children of the entity.
            *   \return the number of children of the entity.
            */
            unsigned int getNbChildren ();
            Entity* getChild(unsigned int n);
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
            /** \fn void setCollisionVolumme (BoundingVolume* volume)
            *   \brief set the collision volume to the entity.
            *   \param BoundingVolume* volume : the collision volume to set.
            */
            void setCollisionVolume (physic::BoundingVolume* volume) {
                collisionVolume.reset(volume);
            }
            /** \fn BoundingVolume* getCollisionVolumme ()
            *   \brief get the collision volume of the entity.
            *   \return the collision volume of the entity.
            */
            physic::BoundingVolume* getCollisionVolume () {
               return collisionVolume.get();
            }
            /** \fn void addFace(Face* face)
            *   \brief add a face to the entity.
            */
            void addFace(Face face);
            /** \fn std::vector<Face*> getFaces()
            *   \brief get the faces of the entity.
            *   \return std::vector<Face*> the faces of the entity.
            */
            std::vector<Face>& getFaces();
            /** \fn void vtserialize(Archive & ar)
            *   \brief serialize the entity into an archive.
            *   \param Archive : the archive onwhich to serialize the entities.
            */
            template <typename Archive>
            void vtserialize(Archive & ar) {
                //if (!alreadySerialized) {
                    Entity::vtserialize(ar);
                    //alreadySerialized = true;
                    //std::cout<<"entity"<<std::endl;
                    ar(parent);
                    //std::cout<<"parent : "<<parent<<std::endl;
                    ar(faces);
                    //std::cout<<"faces"<<std::endl;
                    ar(collisionVolume);
                    //std::cout<<"collisions volume"<<std::endl;
                    ar(shadowCenter);
                    //std::cout<<"shadow center : "<<shadowCenter<<std::endl;
                    ar(shadowScale);
                    //std::cout<<"shadow scale : "<<shadowScale<<std::endl;
                    ar(shadowOrigin);
                    //std::cout<<"shadow origin : "<<shadowOrigin<<std::endl;
                    ar(shadowRotationAngle);
                    //std::cout<<"shadow rotation angle : "<<shadowRotationAngle;
                    ar(shadowRotationAxis);
                    //std::cout<<"shadow rotation axis : "<<shadowRotationAxis<<std::endl;
                    ar(boneIndex);
                    //std::cout<<"bone index : "<<boneIndex<<std::endl;
                    ar(externalObjectName);
                    //std::cout<<"external object name : "<<std::endl;
                    ar(layer);
                    //std::cout<<"water : "<<water<<std::endl;
                    ar(drawMode);
                    //std::cout<<"draw mode : "<<drawMode<<std::endl;

                    ar(children);
                    //std::cout<<"children"<<std::endl;
                /*} else {
                    alreadySerialized = false;
                }*/
                    //std::cout<<"children"<<std::endl;
                //std::cout<<"entity id : "<<getId()<<std::endl<<"Transform matrix : "<<getTransform().getMatrix()<<std::endl;
            }
            /** \fn void onLoad()
            *   \brief load the entities.
            */
            void getCombinedTransform(TransformMatrix& tm) {
                if (parent != nullptr)
                    parent->getCombinedTransform(tm);
                tm.combine(getTransform().getMatrix());
            }
             /**
              *\fn setShadowCenter(math::Vec3f shadowCenter)
              *\brief adjust the center of the generated shadow.
              *\param math::Vec3f shadowCenter : the center of the shadow.
            */
            void setShadowCenter(math::Vec3f shadowCenter);
            /**
              *\fn getShadowCenter()
              *\brief get the center of the shadow.
              *\return math::Vec3f : the center of the shadow.
            */
            math::Vec3f getShadowCenter();
            void setShadowScale(math::Vec3f shadowScale);
            void setShadowRotation(float angle, math::Vec3f axis = math::Vec3f::zAxis);
            math::Vec3f getShadowRotationAxis();
            float getShadowRotationAngle();
            math::Vec3f getShadowScale();
            void setShadowOrigin(math::Vec3f origin);
            math::Vec3f getShadowOrigin();
            void setBoneIndex (unsigned int boneIndex);
            unsigned int getBoneIndex();
            DrawMode getDrawMode();
            void setDrawMode (DrawMode);
            void setLayer(float layer);
            float getLayer();
            static float getNbLayers();
            void copy (GameObject* entity);
            virtual Entity* clone() = 0;
            void setExternal(bool external);
            bool isExternal();
            void setExternalObjectName(std::string externalObjectName);
            std::string getExternalObjectName();
            void detachChildren();
            void updateTransform();
            virtual bool operator==(Entity& other);
            virtual bool operator!=(Entity& other);
        private :
            void reset();
            math::Vec3f shadowCenter, shadowScale, shadowRotationAxis, shadowOrigin; /**> The center of the shadow of the entity.*/
            float shadowRotationAngle;
            std::vector<Face> faces; /**> the faces of the entity.*/
            std::vector<std::unique_ptr<Entity>> children; /** the children of the entities. */
            Entity* parent; /** the parent of the entity. */
            std::unique_ptr<physic::BoundingVolume> collisionVolume; /** the collision volume of the entity*/
            bool alreadySerialized;
            unsigned int boneIndex;
            float layer;
            static float nbLayers;
            DrawMode drawMode;
            bool external, selected;
            std::string externalObjectName;
        };
    }
}
#endif // ODFAEG_GAME_OBJECT_HPP
