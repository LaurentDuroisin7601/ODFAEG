module;
#include <vulkan/vulkan.hpp>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <deque>
#include <glm/glm.hpp>
export module odfaeg.entity.gameObject;
import odfaeg.entity.vertexArray;
import odfaeg.entity.entity;
import odfaeg.entity.transformable;
import odfaeg.math.vec;
import odfaeg.math.matrix;
namespace odfaeg {
	namespace entity {
        export class SubMesh {
            public:
            enum Type {
				AIR, WATER, ICE, GLASS, DIAMOND
			};
		    enum TexType {
		        DIFFUSE, NORMAL, METALNESS, ROUGHNESS, AO, EMISSIVE, SPECULAR, NBTEXTYPES, UNKNOWN
		    };   
            VertexArray& getVertexArray();
            void setVertexArray(VertexArray va);
            void setTextureId(TexType type, std::string id, unsigned int texUnit=0);
            std::string getTextureId(TexType type, unsigned int texUnit=0);
            /**
            * \fn void serialize (Archive & ar)
            * \brief serialize the face into the archive.
            * \param ar : the archive.
            */
            void detachChildren();
            template <typename Archive>
            void serialize(Archive& ar) {
                /*for (unsigned int j= 0; j< m_vertices.getVertexCount(); j++) {
                    //////std::cout<<m_vertices[j].position.x<<","<<m_vertices[j].position.y<<","<<m_vertices[j].position.z<<std::endl;
                }
                //////std::cout<<"prim type : "<<m_vertices.getPrimitiveType()<<std::endl;
                std::vector<math::Vec3f> locals = m_vertices.getLocals();
                //////std::cout<<"locals : ";
                for (unsigned int l = 0; l < locals.size(); l++) {
                    //////std::cout<<locals[l]<<std::endl;
                }
                //////std::cout<<"indexes : "<<std::endl;
                std::vector<unsigned int> indexes = m_vertices.getIndexes();
                for (unsigned int j = 0; j < indexes.size(); j++) {
                    //////std::cout<<"index  : "<<indexes[j]<<std::endl;
                } */
                ar(m_vertices);                                    
            }
            bool operator==(SubMesh& other);
            bool operator!=(SubMesh& other);        	
        	unsigned int verticesOffset;
        	unsigned int id;
            unsigned int materialId;            
        protected :
            void onResize(math::Vec3f& s);
            /**
            * \fn virtual void onRotate(float angle)
            * \brief this function can be redefined in the sub-class if we need to do something when the object is rotating.
            */
            void onRotate(float angle);
            /**
            * \fn virtual void onScale(Vec3f s)
            * \brief this function can be redefined in the sub-class if we need to do something when the object is rescaling.
            */
            void onScale(math::Vec3f& s);
            /**
            * \fn virtual void onMove(Vec3f t)
            * \brief this function can be redefined in the sub-class if we need to do something when the object is moving.
            */
            void onMove(math::Vec3f& t);
            void setType(Type type);
            Type getType();
        private:        	
            VertexArray m_vertices; /**> the vertices.*/
            std::array<std::vector<std::string>, NBTEXTYPES> texturesIds;
            Type type;                                   
        };		
		export class GameObject : public Entity, public Transformable {
		public:
			struct BoneInfo {
				/*id is index in finalBoneMatrices*/
				int id;
				/*offset matrix transforms vertex from model space to bone space*/
				glm::mat4 offset;
			};
			GameObject(std::string type);
			GameObject(math::Vec3f position, math::Vec3f size, math::Vec3f origin, std::string type, std::string name = "", GameObject* parent = nullptr);
            GameObject* getParent() const;
            //Set the parent's entity of the entity.
            /** \fn void setParent(Entity* parent)
            *   \brief set the parent of the entity.
            *   \param the parent of the entity.
            */
            void setParent(GameObject* parent);
            //Add a children to the entity.
            /** \fn void addChild(Entity* child)
            *   \brief add a child to the entity.
            *   \param Entity* the entity to add.
            */
            void addChild(GameObject* child);
            //Remove a children to the entity.
            /** \fn void removeChild(Entity* child)
            *   \brief remove a child from the entity.
            */
            void removeChild(GameObject* child);
            //Return the children of the entities.
            /** \fn std::vector<Entity*> getChildren() const;
            *   \brief get the list of the children of the entities.
            *   \return std::vector<Entity*> get the entities.
            */
            std::vector<GameObject*> getChildren() const;
            //Return the number of entity's children.
            /** \fn  unsigned int getNbChildren ();
            *   \brief get the number of children of the entity.
            *   \return the number of children of the entity.
            */
            void addSubMesh(SubMesh subMesh);
            /** \fn std::vector<Face*> getFaces()
            *   \brief get the faces of the entity.
            *   \return std::vector<Face*> the faces of the entity.
            */
            std::deque<SubMesh>& getSubMeshes();
            void detachChild(GameObject* child);
            GameObject* getChild(unsigned int index);
            GameObject* getRoot();
            void detachChildren();
            /** \fn void vtserialize(Archive & ar)
            *   \brief serialize the entity into an archive.
            *   \param Archive : the archive onwhich to serialize the entities.
            */
            template <typename Archive>
            void vtserialize(Archive& ar) {
                //if (!alreadySerialized) {
                //Entity::vtserialize(ar);
                //alreadySerialized = true;
                ////////std::cout<<"entity"<<std::endl;
                ar(parent);
                ////////std::cout<<"parent : "<<parent<<std::endl;
                ar(subMeshes);
                ////////std::cout<<"faces"<<std::endl;
                //ar(collisionVolume);
                ////////std::cout<<"collisions volume"<<std::endl;
                ar(shadowCenter);
                ////////std::cout<<"shadow center : "<<shadowCenter<<std::endl;
                ar(shadowScale);
                ////////std::cout<<"shadow scale : "<<shadowScale<<std::endl;
                ar(shadowOrigin);
                ////////std::cout<<"shadow origin : "<<shadowOrigin<<std::endl;
                ar(shadowRotationAngle);
                ////////std::cout<<"shadow rotation angle : "<<shadowRotationAngle;
                ar(shadowRotationAxis);
                ////////std::cout<<"shadow rotation axis : "<<shadowRotationAxis<<std::endl;
                ar(animIndex);
                ////////std::cout<<"bone index : "<<boneIndex<<std::endl;
                ar(externalObjectName);
            	////////std::cout<<"draw mode : "<<drawMode<<std::endl;

                ar(children);

                ////////std::cout<<"children"<<std::endl;
            /*} else {
                alreadySerialized = false;
            }*/
            ////////std::cout<<"children"<<std::endl;
        ////////std::cout<<"entity id : "<<getId()<<std::endl<<"Transform matrix : "<<getTransform().getMatrix()<<std::endl;
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
            void setShadowRotation(float angle, math::Vec3f axis = math::Vec3f(0.f, 0.f, 1.f));
            math::Vec3f getShadowRotationAxis();
            float getShadowRotationAngle();
            math::Vec3f getShadowScale();
            void setShadowOrigin(math::Vec3f origin);
            math::Vec3f getShadowOrigin();
            void setAnimIndex(unsigned int boneIndex);
            unsigned int getAnimIndex();
            void setSelected(bool selected) override;
            virtual GameObject* clone() = 0;
            void setExternal(bool external);
            bool isExternal();
            void setExternalObjectName(std::string externalObjectName);
            std::string getExternalObjectName();            
            void updateTransform();
            virtual bool operator==(GameObject& other);
            virtual bool operator!=(GameObject& other);
            unsigned int getSubMeshesCount();
			int& getBoneCount();
			std::map<std::string, BoneInfo>& getBoneInfoMap();
			unsigned int subMeshOffset;
            virtual ~GameObject();
		protected :
            void onResize(math::Vec3f& s);
            void onMove(math::Vec3f& t);
            void onScale(math::Vec3f& s);
            void onRotate(float angle);
			void copy(GameObject* other);
		private:
            math::Vec3f shadowCenter, shadowScale, shadowRotationAxis, shadowOrigin; /**> The center of the shadow of the entity.*/
            float shadowRotationAngle;
            std::deque<SubMesh> subMeshes; /**> the faces of the entity.*/
            std::vector<std::unique_ptr<GameObject>> children; /** the children of the entities. */
            GameObject* parent; /** the parent of the entity. */
            
            unsigned int animIndex;
            bool external, selected;
            std::string name, externalObjectName;
			std::map<std::string, BoneInfo> m_BoneInfoMap; //
			int m_BoneCounter = 0;
		};
	}
}