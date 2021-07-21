#ifndef ODFAEG_FACE_HPP
#define ODFAEG_FACE_HPP
#include "vertexArray.h"
#include <map>
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
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
        sf::IntRect rect; /**> The coordinates of the texture.*/
        std::string texId; /**> An identifier to the texture of the material. */
        public :
        /**
        * \fn TextureInfo()
        * \brief constructor
        */
        TextureInfo() {
        }
        /**
        * \fn TextureInfo(const Texture* texture, sf::IntRect rect, std::string texId="")
        * \brief constructor
        */
        TextureInfo (const Texture* texture, sf::IntRect rect, std::string texId="") {
            this->texture = texture;
            this->rect = rect;
            this->texId = texId;
        }
        /**
        * \fn void setTexId(std::string texId)
        * \brief set the texture id.
        * \param texId : the texId.
        */
        void setTexId(std::string texId) {
            this->texId = texId;
        }
        /**
        * \fn std::string getTexId() const
        * \brief return the id of the texture.
        * \return std::string a string representation of the id.
        */
        std::string getTexId() const {
            return texId;
        }
        /**
        * \fn bool operator== (TextureInfo& info)
        * \brief compare a texture info with another one.
        * \param the other texture info.
        * \return if two textures infos are the same.
        */
        bool operator== (TextureInfo& info) {
            return texture == info.texture;
        }
        /**
        * \fn bool operator!= (TextureInfo& info)
        * \param the other texture info.
        * \brief compare a texture info with another one.
        * \return if two textures infos are different.
        */
        bool operator!= (TextureInfo& info) {
            return texture != info.texture;
        }
        /**
        * \fn const Texture* getTexture() const
        * \brief return the texture.
        * \return the texture.
        */
        const Texture* getTexture() const {
            return texture;
        }
        /**
        * \fn sf::IntRect getTexRect() const
        * \brief return the texture coordinates.
        * \return the textures coordinates.
        */
        sf::IntRect getTexRect() const {
            return rect;
        }
        /**
        * \fn void serialize(Archive &ar)
        * \brief serialize the textures information in a texture infos.
        * \param ar : the archive.
        */
        template <typename Archive>
        void serialize(Archive &ar) {
            ar(rect.left);
            ar(rect.top);
            ar(rect.width);
            ar(rect.height);
            ar(texId);
        }
    };
    std::vector<TextureInfo*> texInfos; /**> The informations about the textures. */
    sf::Color color; /**> the color of the material. */
    public :
    /**
    * \fn Material()
    * \brief constructor.
    */
    Material() {
    }
    /**
    * \fn int getNbTextures ()
    * \brief return the number of textures.
    * \return the number of textures.
    */
    int getNbTextures () {
        return texInfos.size();
    }
    /**
    * \fn void addTexture (const Texture* texture, sf::IntRect rect)
    * \brief add a texture to the material.
    * \param texture : the texture.
    * \param text : the texture coordinates.
    */
    void addTexture (const Texture* texture, sf::IntRect rect) {
        texInfos.push_back(new TextureInfo(texture, rect));
    }
    /**
    * \fn sf::IntRect getTexRect(int texUnit = 0) const {
    * \brief get the texture coordinates of a texture. (the first texture by default)
    * \param the number of the texture.
    * \return the coordinates of the textures.
    */
    sf::IntRect getTexRect(int texUnit = 0) const {
        return (texInfos.size() > 0) ? texInfos[texUnit]->getTexRect() : sf::IntRect(0, 0, 0, 0);
    }
    /**
    * \fn void clearTextures()
    * \brief clear every texture.
    */
    void clearTextures() {
        texInfos.clear();
    }
    /**
    * \fn const Texture* getTexture(int texUnit = 0)
    * \brief get the texture of the given unit. (0 = the first texture by default)
    * \param texUnit : the unit of the texture.
    * \return a pointer to the texture.
    */
    const Texture* getTexture(int texUnit = 0) {
        return (texInfos.size() > 0) ? texInfos[texUnit]->getTexture() : nullptr;
    }
    /**
    * \fn std::string getTexId(int texUnit = 0)
    * \brief get the texture id of the given unit. (the first texture by default)
    * \param texUnit : the texture unit.
    * \return the id of the texture.
    */
    std::string getTexId(int texUnit = 0) {
        return (texInfos.size() > 0) ? texInfos[texUnit]->getTexId() : "";
    }
    /**
    * \fn void setTexId(std::string texId, int texUnit = 0)
    * \brief set the texId of the given texture unit.
    * \param texId : the texture id.
    * \param texUnit : the texture unit.
    */
    void setTexId(std::string texId, int texUnit = 0) {
        if (texInfos.size() > 0) {
            texInfos[texUnit]->setTexId(texId);
        }
    }
    /**
    * \fn bool useSameTextures (const Material& material)
    * \brief check if two material are using the same textures.
    * \param material : the other material.
    * \return if the materials are using the same texture.
    */
    bool useSameTextures (const Material& material) {
        if (texInfos.size() != material.texInfos.size())
            return false;
        for (unsigned int i = 0; i < texInfos.size(); i++) {
            if (*texInfos[i] != *material.texInfos[i])
                return false;
        }
        return true;

    }
    /**
    * \fn  bool hasSameColor (const Material& material)
    * \brief test of two material have the same color.
    * \param material : the other material.
    */
    bool hasSameColor (const Material& material) {
        return color == material.color;
    }
    /**
    * \fn bool operator== (const Material& material)
    * \brief test of two material are equal.
    * \param material : the other material.
    * \return if the two material are equal.
    */
    bool operator== (const Material& material) {
        return useSameTextures(material) && hasSameColor(material);
    }
    /**
    * \fn bool operator!= (const Material& material)
    * \brief test of two material are different.
    * \param material : the other material.
    * \return if the two material are different.
    */
    bool operator!= (Material& material) {
        return !useSameTextures(material) || !hasSameColor(material);
    }
    /**
    * \fn void serialize(Archive & ar)
    * \brief write the material into the given archive.
    * \param ar : the archive.
    */
    template <typename Archive>
    void serialize(Archive & ar) {
        ar(texInfos);
    }
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
    Face() {
    }
    /**
    * \fn Face(sf::PrimitiveType primType, TransformMatrix& tm)
    * \brief constructor.
    * \param primType : the primitive type.
    * \param tm : the transormation matrix.
    */
    Face(sf::PrimitiveType primType, TransformMatrix& tm) : transform(&tm) {
        m_vertices.setPrimitiveType(primType);
    }
    /**
    * \fn Face(VertexArray va, Material mat, TransformMatrix& tm)
    * \brief constructor.
    * \param va : the verte array.
    * \param tm : the material.
    * \param the transform matrix.
    */
    Face(VertexArray va, Material mat, TransformMatrix& tm) : transform(&tm) {
        m_vertices = va;
        m_material = mat;
    }
    /**
    * \fn TransformMatrix& getTransformMatrix() const
    * \brief get the transform matrix of the face.
    * \return the transform matrix of the face.
    */
    TransformMatrix& getTransformMatrix() const {
        return *transform;
    }
    /**
    * \fn void append(Vertex vertex, unsigned int indice)
    * \brief add a vertex to the face.
    * \param vertex : the vertex to add.
    * \param indice : the indice of the vertex.
    */
    void append(Vertex vertex, unsigned int indice) {
        m_vertices.append(vertex);
        m_vertices.addIndex(indice);
    }
    /**
    * \fn Material& getMaterial()
    * \brief get the material of the face.
    * \return the material.
    */
    Material& getMaterial() {
        return m_material;
    }
    /**
    * \fn VertexArray& getVertexArray()
    * \brief get the vertex array.
    * \return the vertex array.
    */
    VertexArray& getVertexArray() {
        return m_vertices;
    }
    /**
    * \fn bool useSameMaterial(Face& other)
    * \brief check if two faces are using the same material.
    * \param other : the other face.
    */
    bool useSameMaterial(Face& other) {
        return m_material == other.m_material;

    }
    /**
    * \fn bool useSamePrimType (Face &other)
    * \brief check if the face are using the same primitive type.
    * \param other : the other face.
    */
    bool useSamePrimType (Face &other) {
        return m_vertices.getPrimitiveType() == other.m_vertices.getPrimitiveType();
    }
    /**
    * \fn void serialize (Archive & ar)
    * \brief serialize the face into the archive.
    * \param ar : the archive.
    */
    template <typename Archive>
    void serialize (Archive & ar) {
        ar(m_vertices);
        ar(m_material);
        ar(transform);
    }
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
    /**
    *   \fn Instance (Material& material, sf::PrimitiveType pType)
    *   \brief constructor.
    *   \param material : the material.
    *   \param pType : the primitive type.
    */
    Instance (Material& material, sf::PrimitiveType pType) : material(material) {
        primType = pType;
        numInstances = 0;
    }
    /**
    *  \fn void addVertexArray(VertexArray *va, TransformMatrix& tm, unsigned int baseVertex, unsigned int baseIndex)
    *  \brief add a vertex array to the instance.
    *
    */
    void addVertexArray(VertexArray *va, TransformMatrix& tm, unsigned int baseVertex, unsigned int baseIndex) {
        /*if (Shader::getShadingLanguageVersionMajor() >= 3 && Shader::getShadingLanguageVersionMinor() >= 3) {
            unsigned int numIndexes = va->getIndexes().size();
            for (unsigned int i = 0; i < va->getVertexCount(); i++) {
                m_vertices.append((*va)[i]);
                for (unsigned int i = 0; i < va->getIndexes().size(); i++) {
                    va->addIndex(va->getIndexes()[i]);
                }
            }
            m_vertices.addInstancedRenderingInfos(numIndexes,baseVertex,baseIndex);
            numInstances++;
        }*/
        m_transforms.push_back(std::ref(tm));
        m_vertexArrays.push_back(va);
    }
    /**
    * \fn std::vector<VertexArray*> getVertexArrays()
    * \brief get the vertex arrays of the instance.
    * \return the vertex arrays.
    */
    std::vector<VertexArray*> getVertexArrays() {
        return m_vertexArrays;
    }
    /**
    * \fn void clear()
    * \brief clear all the vertex arrays of the instances.
    */
    void clear() {
        m_transforms.clear();
        m_vertices.clear();
        m_vertexArrays.clear();
    }
    /**
    * \fn std::vector<std::reference_wrapper<TransformMatrix>> getTransforms()
    * \brief get the transformations of every vertex arrays of the instances.
    * \return the transforms.
    */
    std::vector<std::reference_wrapper<TransformMatrix>> getTransforms() {
         return m_transforms;
    }
    /** \fn VertexArray& getVertexArray()
    * \brief get the vertex array.
    * \return the vertex array.
    */
    VertexArray& getVertexArray() {
        return m_vertices;
    }
    /** \fn Material& getMaterial()
    * \brief get the material of the instance.
    * \return the material.
    */
    Material& getMaterial() {
        return material;
    }
    /**
    * \fn sf::PrimitiveType getPrimitiveType()
    * \brief get the primitive type.
    * \return the pirmitive type.
    */
    sf::PrimitiveType getPrimitiveType() {
        return primType;
    }
    /**
    * \fn unsigned int getNumInstances()
    * \brief get the number of instances.
    * \return the number of instances.
    */
    unsigned int getNumInstances() {
        return numInstances;
    }
    /**
    * \fn ~Instance()
    * \brief destructor.
    */
    ~Instance() {
        m_vertices.clear();
        m_transforms.clear();
        m_vertices.clear();
        m_vertexArrays.clear();
    }
private:
    Material& material; /**> the material of the instance.*/
    VertexArray m_vertices; /**> the vertices of the instance.*/
    std::vector<VertexArray*> m_vertexArrays; /**> the vertex arrays of the instance.*/
    std::vector<std::reference_wrapper<TransformMatrix>> m_transforms; /**> the transformations of the instance.*/
    sf::PrimitiveType primType; /**>The primitive type of the instance.*/
    unsigned int numInstances; /**>The number of instances.*/
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
class FaceGroup {
    public :
    /**
    *  \fn FaceGroup()
    *  \brief constructor.
    */
    FaceGroup() {
        numVertices = 0;
        numIndexes = 0;
    }
    /**
    * \fn void addFace(Face* face)
    * \brief add a face to the facegroup.
    * \param face : the face to add.
    */
    void addFace(Face* face) {
        bool added = false;
        for (unsigned int i = 0; i < instances.size() && !added; i++) {
            if (instances[i]->getMaterial() == face->getMaterial()
                && instances[i]->getPrimitiveType() == face->getVertexArray().getPrimitiveType()) {
                    added = true;
                    instances[i]->addVertexArray(&face->getVertexArray(),face->getTransformMatrix(), numVertices,numIndexes);
            }
        }
        if (!added) {
            Instance* instance = new Instance(face->getMaterial(), face->getVertexArray().getPrimitiveType());
            instance->addVertexArray(&face->getVertexArray(),face->getTransformMatrix(), numVertices,numIndexes);
            instances.push_back(instance);
            /*if (Shader::getShadingLanguageVersionMajor() >= 3 && Shader::getShadingLanguageVersionMinor() >= 3)
                numIndexes += instance->getVertexArray().getBaseIndexes().back();*/
        }
        if (Shader::getShadingLanguageVersionMajor() >= 3 && Shader::getShadingLanguageVersionMinor() >= 3)
            numVertices += face->getVertexArray().getVertexCount();
    }
    /**
    * \fn std::vector<Instance*> getInstances()
    * \brief return the instances.
    * \return the instances.
    */
    std::vector<Instance*> getInstances() {
        return instances;
    }
    /**
    * \fn unsigned int getNumIndexes()
    * \brief get the number of indexes.
    * \return the number of index.
    */
    unsigned int getNumIndexes() {
        return numIndexes;
    }
    /**
    * \fn unsigned int getNumVertices()
    * \brief get the number of vertices.
    * \return the number of vertices.
    */
    unsigned int getNumVertices() {
        return numVertices;
    }
    /**
    * \fn void clear()
    * \brief clean the instances.
    */
    void clear() {
        numVertices = 0;
        numIndexes = 0;
        for (unsigned int i = 0; i < instances.size(); i++) {
            delete instances[i];
        }
        instances.clear();
    }
private :
    unsigned int numVertices; /**> the number of vertices.*/
    unsigned int numIndexes; /**> the number of indexes.*/
    std::vector<Instance*> instances; /**> the instances.*/
};
}
#endif // FACE_HPP
