#ifndef ODFAEG_FACE_HPP
#define ODFAEG_FACE_HPP
#include "vertexArray.h"
#include "view.h"
#include <map>
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace graphic {
        /**
          * \file material.h
          * \class Material
          * \brief Represent a material of a face.
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          *  Represent a material of a face.
          */
        class ODFAEG_GRAPHICS_API Material {
             private :
             /** \struct TetureInfo
             *   \brief represent the informations about a texture used by the material.
             */
             struct TextureInfo {
                private :
                const Texture* texture; /**> A texture used by the material.*/
                sf::IntRect texRect;
                std::string texId; /**> An identifier to the texture of the material. */
                public :
                /**
                * \fn TextureInfo()
                * \brief constructor
                */
                TextureInfo();
                /**
                * \fn TextureInfo(const Texture* texture, sf::IntRect rect, std::string texId="")
                * \brief constructor
                */
                TextureInfo (const Texture* texture, sf::IntRect texRect = sf::IntRect(0, 0, 0, 0), std::string texId="");
                /**
                * \fn void setTexId(std::string texId)
                * \brief set the texture id.
                * \param texId : the texId.
                */
                void setTexRect(sf::IntRect texRect);
                sf::IntRect getTexRect();
                void setTexId(std::string texId);
                /**
                * \fn std::string getTexId() const
                * \brief return the id of the texture.
                * \return std::string a string representation of the id.
                */
                std::string getTexId() const;
                /**
                * \fn bool operator== (TextureInfo& info)
                * \brief compare a texture info with another one.
                * \param the other texture info.
                * \return if two textures infos are the same.
                */
                bool operator== (TextureInfo& info);
                /**
                * \fn bool operator!= (TextureInfo& info)
                * \param the other texture info.
                * \brief compare a texture info with another one.
                * \return if two textures infos are different.
                */
                bool operator!= (TextureInfo& info);
                /**
                * \fn const Texture* getTexture() const
                * \brief return the texture.
                * \return the texture.
                */
                const Texture* getTexture() const;
                template <typename Archive>
                void serialize(Archive &ar) {
                    //std::cout<<"mat rect height : "<<rect.height<<std::endl;
                    ar(texRect.left);
                    ar(texRect.top);
                    ar(texRect.width);
                    ar(texRect.height);
                    ar(texId);
                    //std::cout<<"mat tex id : "<<texId<<std::endl;
                }
            };
            public :
            enum Type {
                AIR, WATER, ICE, GLASS, DIAMOND
            };
            /**
            * \fn Material()
            * \brief constructor.
            */
            Material();
            Material (const Material& material);
            unsigned int getId();
            static unsigned int getNbMaterials();
            /**
            * \fn int getNbTextures ()
            * \brief return the number of textures.
            * \return the number of textures.
            */
            int getNbTextures ();
            /**
            * \fn void addTexture (const Texture* texture, sf::IntRect rect)
            * \brief add a texture to the material.
            * \param texture : the texture.
            * \param text : the texture coordinates.
            */
            void addTexture (const Texture* texture, sf::IntRect texRect = sf::IntRect(0, 0, 0, 0));
            void setTexRect(sf::IntRect texRect, int texUnit = 0);
            /**
            * \fn void clearTextures()
            * \brief clear every texture.
            */
            void clearTextures();
            /**
            * \fn const Texture* getTexture(int texUnit = 0)
            * \brief get the texture of the given unit. (0 = the first texture by default)
            * \param texUnit : the unit of the texture.
            * \return a pointer to the texture.
            */
            const Texture* getTexture(int texUnit = 0);
            sf::IntRect getTexRect(int texUnit = 0);
            /**
            * \fn std::string getTexId(int texUnit = 0)
            * \brief get the texture id of the given unit. (the first texture by default)
            * \param texUnit : the texture unit.
            * \return the id of the texture.
            */
            std::string getTexId(int texUnit = 0);
            /**
            * \fn void setTexId(std::string texId, int texUnit = 0)
            * \brief set the texId of the given texture unit.
            * \param texId : the texture id.
            * \param texUnit : the texture unit.
            */
            void setTexId(std::string texId, int texUnit = 0);
            /**
            * \fn bool useSameTextures (const Material& material)
            * \brief check if two material are using the same textures.
            * \param material : the other material.
            * \return if the materials are using the same texture.
            */
            bool useSameTextures (const Material& material);
            /**
            * \fn bool operator== (const Material& material)
            * \brief test of two material are equal.
            * \param material : the other material.
            * \return if the two material are equal.
            */
            bool operator== (const Material& material);
            /**
            * \fn bool operator!= (const Material& material)
            * \brief test of two material are different.
            * \param material : the other material.
            * \return if the two material are different.
            */
            bool operator!= (Material& material);
            /**
            * \fn void serialize(Archive & ar)
            * \brief write the material into the given archive.
            * \param ar : the archive.
            */
            float getSpecularIntensity();
            float getSpecularPower();
            void setSpecularIntensity(float specularIntensity);
            void setSpecularPower(float specularPower);
            void setBumpTexture(const Texture* texture);
            const Texture* getBumpTexture();
            void setRefractionFactor(float refractionFactor);
            float getRefractionFactor();
            void setReflectable(bool reflectable);
            void setRefractable(bool refractable);
            bool isReflectable();
            bool isRefractable();

            void updateIds();
            bool contains(Material& material);
            void countNbMaterials();
            Material& operator=(const Material& material);
            template <typename Archive>
            void serialize(Archive & ar) {
                ar(texInfos);
                //std::cout<<"color a : "<<color.a<<std::endl;
                ar(specularIntensity);
                //std::cout<<"specular intensity "<<specularIntensity<<std::endl;
                ar(specularPower);
                //std::cout<<"specular power : "<<specularPower<<std::endl;
                ar(reflectable);
                ar(refractable);
                ar(refractionFactor);
                if (ar.isInputArchive()) {
                    onLoad();
                }
            }
            void onLoad() {
                /*maxSpecularIntensity = (specularIntensity > maxSpecularIntensity) ? specularIntensity : maxSpecularIntensity;
                maxSpecularPower = (specularPower > maxSpecularPower) ? specularPower : maxSpecularPower;*/
            }
            void setType(Type type);
            Type getType();
            ~Material();
            private :
            std::vector<TextureInfo*> texInfos; /**> The informations about the textures. */
            float specularIntensity, specularPower, refractionFactor;
            static unsigned int nbMaterials;
            unsigned int id;
            const Texture* bumpTexture;
            static std::vector<Material*> materials;
            static std::vector<Material*> sameMaterials;
            bool reflectable, refractable;
            Type type;
        };
        /**
          * \file face.h
          * \class Face
          * \brief Represent a material of a face.
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          *  Represent a material of a face.
          */
        class ODFAEG_GRAPHICS_API Face {
        public :
            /**
            * \fn Face()
            * \brief constructor.
            */
            Face();
            /**
            * \fn Face(sf::PrimitiveType primType, TransformMatrix& tm)
            * \brief constructor.
            * \param primType : the primitive type.
            * \param tm : the transormation matrix.
            */
            Face(sf::PrimitiveType primType, TransformMatrix& tm);
            /**
            * \fn Face(VertexArray va, Material mat, TransformMatrix& tm)
            * \brief constructor.
            * \param va : the verte array.
            * \param tm : the material.
            * \param the transform matrix.
            */
            Face(VertexArray va, Material mat, TransformMatrix& tm);
            /**
            * \fn TransformMatrix& getTransformMatrix() const
            * \brief get the transform matrix of the face.
            * \return the transform matrix of the face.
            */
            TransformMatrix& getTransformMatrix() const;
            /**
            * \fn void append(Vertex vertex, unsigned int indice)
            * \brief add a vertex to the face.
            * \param vertex : the vertex to add.
            * \param indice : the indice of the vertex.
            */
            void append(Vertex vertex, unsigned int indice);
            /**
            * \fn Material& getMaterial()
            * \brief get the material of the face.
            * \return the material.
            */
            Material& getMaterial();
            void setMaterial(Material& material);
            void setTransformMatrix(TransformMatrix& tm);
            /**
            * \fn VertexArray& getVertexArray()
            * \brief get the vertex array.
            * \return the vertex array.
            */
            VertexArray& getVertexArray();
            void setVertexArray (VertexArray va);
            /**
            * \fn bool useSameMaterial(Face& other)
            * \brief check if two faces are using the same material.
            * \param other : the other face.
            */
            bool useSameMaterial(Face& other);
            /**
            * \fn bool useSamePrimType (Face &other)
            * \brief check if the face are using the same primitive type.
            * \param other : the other face.
            */
            bool useSamePrimType (Face &other);
            /**
            * \fn void serialize (Archive & ar)
            * \brief serialize the face into the archive.
            * \param ar : the archive.
            */
            template <typename Archive>
            void serialize (Archive & ar) {
                /*for (unsigned int j= 0; j< m_vertices.getVertexCount(); j++) {
                    std::cout<<m_vertices[j].position.x<<","<<m_vertices[j].position.y<<","<<m_vertices[j].position.z<<std::endl;
                }
                std::cout<<"prim type : "<<m_vertices.getPrimitiveType()<<std::endl;
                std::vector<math::Vec3f> locals = m_vertices.getLocals();
                std::cout<<"locals : ";
                for (unsigned int l = 0; l < locals.size(); l++) {
                    std::cout<<locals[l]<<std::endl;
                }
                std::cout<<"indexes : "<<std::endl;
                std::vector<unsigned int> indexes = m_vertices.getIndexes();
                for (unsigned int j = 0; j < indexes.size(); j++) {
                    std::cout<<"index  : "<<indexes[j]<<std::endl;
                } */
                ar(m_vertices);
                //std::cout<<"vertices"<<std::endl;
                ar(m_material);
                //std::cout<<"material"<<std::endl;
                ar(transform);
                //std::cout<<"transform"<<std::endl;
            }
            bool operator== (Face& other);
            bool operator!= (Face& other);
        private :
            VertexArray m_vertices; /**> the vertices.*/
            Material m_material; /**> the material.*/
            TransformMatrix* transform; /**> the transform.*/
        };
        /**
          * \file face.h
          * \class Instance
          * \brief Represent an instance.
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          *  Represent an instance, an instance used for instanced rendering.
          *  an instance if a set a contains several vertex arrays with the same material and the same
          */
        class ODFAEG_GRAPHICS_API Instance {
            public :
            Instance();
            /**
            *   \fn Instance (Material& material, sf::PrimitiveType pType)
            *   \brief constructor.
            *   \param material : the material.
            *   \param pType : the primitive type.
            */
            Instance (Material& material, sf::PrimitiveType pType);
            /**
            *  \fn void addVertexArray(VertexArray *va, TransformMatrix& tm, unsigned int baseVertex, unsigned int baseIndex)
            *  \brief add a vertex array to the instance.
            *
            */
            void addVertexArray(VertexArray& va, TransformMatrix& tm);
            void addVertexShadowArray(VertexArray& va, TransformMatrix& tm, ViewMatrix& viewMatrix, TransformMatrix shadowProjMatrix);
            void sortVertexArrays(View& view);
            /**
            * \fn std::vector<VertexArray*> getVertexArrays()
            * \brief get the vertex arrays of the instance.
            * \return the vertex arrays.
            */
            std::vector<VertexArray*> getVertexArrays();
            std::vector<unsigned int> getAllIndexes();
            std::vector<std::vector<unsigned int>> getIndexes();
            /**
            * \fn void clear()
            * \brief clear all the vertex arrays of the instances.
            */
            void clear();
            /**
            * \fn std::vector<std::reference_wrapper<TransformMatrix>> getTransforms()
            * \brief get the transformations of every vertex arrays of the instances.
            * \return the transforms.
            */
            std::vector<TransformMatrix*> getTransforms();
            std::vector<TransformMatrix> getShadowProjMatrix();
            /** \fn Material& getMaterial()
            * \brief get the material of the instance.
            * \return the material.
            */
            Material& getMaterial();
            /**
            * \fn sf::PrimitiveType getPrimitiveType()
            * \brief get the primitive type.
            * \return the pirmitive type.
            */
            sf::PrimitiveType getPrimitiveType();
            void setPrimitiveType (sf::PrimitiveType);
            void setMaterial(Material& material);
            /**
            * \fn unsigned int getNumInstances()
            * \brief get the number of instances.
            * \return the number of instances.
            */
            unsigned int getNumInstances();
            /**
            * \fn ~Instance()
            * \brief destructor.
            */
            VertexArray& getAllVertices();
            ~Instance();
        private:
            Material* material; /**> the material of the instance.*/
            std::vector<VertexArray*> m_vertexArrays; /**> the vertex arrays of the instance.*/
            std::vector<TransformMatrix*> m_transforms; /**> the transformations of the instance.*/
            std::vector<TransformMatrix> m_shadowProjMatrix;
            sf::PrimitiveType primType; /**>The primitive type of the instance.*/
            unsigned int numInstances; /**>The number of instances.*/
            VertexArray vertices;
            std::vector<unsigned int> allIndexes;
            std::vector<std::vector<unsigned int>> m_indexes;
        };
        /**
          * \file face.h
          * \class FaceGroup
          * \brief Put every vertices of the faces into instances for the instanced rendering.
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          *  Represent a set of instances used for instanced rendering, each vertices of the faces are grouped
          *  by primitive type and material (so each vertices with the same primitive type and the same material
          *  are grouped into the instances to minimise the calls to gl functions).
          */
        class Batcher {
            public :
            /**
            *  \fn FaceGroup()
            *  \brief constructor.
            */
            Batcher();
            /**
            * \fn void addFace(Face* face)
            * \brief add a face to the facegroup.
            * \param face : the face to add.
            */
            void addFace(Face* face);
            void addShadowFace(Face* face, ViewMatrix viewMatrix, TransformMatrix shadowProjMatrix);
            /**
            * \fn std::vector<Instance*> getInstances()
            * \brief return the instances.
            * \return the instances.
            */
            std::vector<Instance> getInstances();
            /**
            * \fn unsigned int getNumIndexes()
            * \brief get the number of indexes.
            * \return the number of index.
            */
            unsigned int getNumIndexes();
            /**
            * \fn unsigned int getNumVertices()
            * \brief get the number of vertices.
            * \return the number of vertices.
            */
            unsigned int getNumVertices();
            /**
            * \fn void clear()
            * \brief clean the instances.
            */
            void clear();
            unsigned int getNbLayers();
            static const unsigned int nbPrimitiveTypes = 7;
        private :
            unsigned int numVertices; /**> the number of vertices.*/
            unsigned int numIndexes; /**> the number of indexes.*/
            std::vector<Instance> instances; /**> the instances.*/
            unsigned int nbLayers;
            std::vector<float> tmpZPos;
        };
    }
}
#endif // FACE_HPP
