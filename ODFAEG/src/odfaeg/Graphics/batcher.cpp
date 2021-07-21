#include "../../../include/odfaeg/Graphics/batcher.h"
#include "../../../include/odfaeg/Graphics/entity.h"
#include "../../../include/odfaeg/Graphics/application.h"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace graphic {
            unsigned int Material::nbMaterials = 0;
            std::vector<Material*> Material::materials = std::vector<Material*>();
            std::vector<Material*> Material::sameMaterials = std::vector<Material*>();
            Material::TextureInfo::TextureInfo() {
                texture = nullptr;
                texId = "";
            }
            Material::TextureInfo::TextureInfo (const Texture* texture, sf::IntRect texRect, std::string texId) {
                this->texture = texture;
                this->texRect = texRect;
                this->texId = texId;
            }
            void Material::TextureInfo::setTexRect(sf::IntRect texRect) {
                this->texRect = texRect;
            }
            sf::IntRect Material::TextureInfo::getTexRect() {
                return texRect;
            }
            void Material::TextureInfo::setTexId(std::string texId) {
                this->texId = texId;
            }
            std::string Material::TextureInfo::getTexId() const {
                return texId;
            }
            bool Material::TextureInfo::operator== (TextureInfo& info) {
                return texture == info.texture;
            }
            bool Material::TextureInfo::operator!= (TextureInfo& info) {
                return !(*this == info);
            }
            const Texture* Material::TextureInfo::getTexture() const {
                return texture;
            }
            Material::Material() {
                specularIntensity = 0;
                specularPower = 0;
                refractionFactor = 0;
                refractable = false;
                reflectable = false;
                bumpTexture = nullptr;
                id = 0;
                if (core::Application::app != nullptr)
                    core::Application::app->getMaterials().push_back(this);
                else
                    materials.push_back(this);
            }
            Material::Material(const Material& material) {
                texInfos = material.texInfos;
                id = material.id;
                specularIntensity = material.specularIntensity;
                specularPower = material.specularPower;
                refractionFactor = material.refractionFactor;
                refractable = material.refractable;
                reflectable = material.reflectable;
                bumpTexture = material.bumpTexture;
                //std::cout<<"copy material"<<std::endl;
                if (core::Application::app != nullptr)
                    core::Application::app->getMaterials().push_back(this);
                else
                    materials.push_back(this);
                //updateIds();
            }
            Material& Material::operator= (const Material& material) {
                texInfos = material.texInfos;
                id = material.id;
                specularIntensity = material.specularIntensity;
                specularPower = material.specularPower;
                refractionFactor = material.refractionFactor;
                refractable = material.refractable;
                reflectable = material.reflectable;
                bumpTexture = material.bumpTexture;
                if (core::Application::app != nullptr)
                    core::Application::app->getMaterials().push_back(this);
                else
                    materials.push_back(this);
                return *this;
            }
            bool Material::contains(Material& material) {
                if (core::Application::app != nullptr) {
                    std::vector<Material*>& sameMaterials = core::Application::app->getSameMaterials();
                    for (unsigned int i = 0; i < sameMaterials.size(); i++) {
                        if (*sameMaterials[i] == material) {
                            return true;
                        }
                    }
                } else {
                    for (unsigned int i = 0; i < sameMaterials.size(); i++) {
                        if (*sameMaterials[i] == material) {
                            return true;
                        }
                    }
                }
                return false;
            }
            unsigned int Material::getNbMaterials() {
                return nbMaterials;
            }
            unsigned int Material::getId() {
                return id;
            }
            float Material::getSpecularIntensity() {
                return specularIntensity;
            }
            float Material::getSpecularPower() {
                return specularPower;
            }
            void Material::setRefractable(bool refractable) {
                this->refractable = refractable;
            }
            void Material::setReflectable(bool reflectable) {
                this->reflectable = reflectable;
            }
            bool Material::isRefractable() {
                return refractable;
            }
            bool Material::isReflectable() {
                return reflectable;
            }
            void Material::setSpecularIntensity(float specularIntensity) {
                this->specularIntensity = specularIntensity;
                updateIds();
            }
            void Material::setSpecularPower(float specularPower) {
                this->specularPower = specularPower;
                updateIds();
            }
            void Material::setBumpTexture(const Texture* bumpTexture) {
                this->bumpTexture = bumpTexture;
                updateIds();
            }
            const Texture* Material::getBumpTexture() {
                return bumpTexture;
            }
            void Material::setRefractionFactor(float refractionFactor) {
                this->refractionFactor = refractionFactor;
                updateIds();
            }
            float Material::getRefractionFactor() {
                return refractionFactor;
            }
            int Material::getNbTextures () {
                return texInfos.size();
            }
            void Material::addTexture (const Texture* texture, sf::IntRect texRect) {
                texInfos.push_back(new TextureInfo(texture, texRect));
                updateIds();
            }
            void Material::clearTextures() {
                texInfos.clear();
                updateIds();
            }
            void Material::setTexRect(sf::IntRect texRect, int texUnit) {
                if (texUnit < texInfos.size())
                    texInfos[texUnit]->setTexRect(texRect);
            }
            const Texture* Material::getTexture(int texUnit) {
                return (texInfos.size() > 0) ? texInfos[texUnit]->getTexture() : nullptr;
            }
            sf::IntRect Material::getTexRect(int texUnit) {
                return (texInfos.size() > 0) ? texInfos[texUnit]->getTexRect() : sf::IntRect(0, 0, 0, 0);
            }
            std::string Material::getTexId(int texUnit) {
                return (texInfos.size() > 0) ? texInfos[texUnit]->getTexId() : "";
            }
            void Material::setTexId(std::string texId, int texUnit) {
                if (texInfos.size() > 0) {
                    texInfos[texUnit]->setTexId(texId);
                }
            }
            void Material::setType(Type type) {
                if (type == AIR) {
                    refractionFactor = 1;
                } else if (type == WATER) {
                    refractionFactor = 1.f / 1.33f;
                } else if (type == ICE) {
                    refractionFactor = 1.f / 1.309f;
                } else if (type == GLASS) {
                    refractionFactor = 1.f / 1.52f;
                } else {
                    refractionFactor = 1.f / 2.42f;
                }
            }
            Material::Type Material::getType() {
                return type;
            }
            bool Material::useSameTextures (const Material& material) {
                if (texInfos.size() != material.texInfos.size()) {
                    return false;
                }
                for (unsigned int i = 0; i < texInfos.size(); i++) {
                    if (*texInfos[i] != *material.texInfos[i]) {
                        return false;
                    }
                }
                return true;

            }
            bool Material::operator== (const Material& material) {
                return useSameTextures(material)
                       && specularIntensity == material.specularIntensity
                       && specularPower == material.specularPower
                       && bumpTexture == material.bumpTexture
                       && refractionFactor == material.refractionFactor
                       && refractable == material.refractable
                       && reflectable == material.reflectable;
            }
            bool Material::operator!= (Material& material) {
                return !(*this == material);
            }
            void Material::countNbMaterials() {
                if (core::Application::app != nullptr) {
                    unsigned int nbMaterials = 0;
                    std::vector<Material*>& sameMaterials = core::Application::app->getSameMaterials();
                    std::vector<Material*>& materials = core::Application::app->getMaterials();
                    sameMaterials.clear();
                    for (unsigned int i = 0; i < materials.size(); i++) {
                        if (materials[i] != nullptr && !contains(*materials[i])) {
                            nbMaterials++;
                            sameMaterials.push_back(materials[i]);
                        }
                    }
                    core::Application::app->setNbMaterials(nbMaterials);
                } else {
                    sameMaterials.clear();
                    for (unsigned int i = 0; i < materials.size(); i++) {
                        if (materials[i] != nullptr && !contains(*materials[i])) {
                            nbMaterials++;
                            sameMaterials.push_back(materials[i]);
                        }
                    }
                }
            }
            void Material::updateIds() {
               countNbMaterials();
               if (core::Application::app != nullptr) {
                   std::vector<Material*>& sameMaterials = core::Application::app->getSameMaterials();
                   std::vector<Material*>& materials = core::Application::app->getMaterials();
                   for (unsigned int i = 0; i < sameMaterials.size(); i++) {
                       for (unsigned int j = 0; j < materials.size(); j++) {
                            if (materials[j] != nullptr && *sameMaterials[i] == *materials[j]) {
                                materials[j]->id = i;
                            }
                       }
                   }
               } else {
                  for (unsigned int i = 0; i < sameMaterials.size(); i++) {
                       for (unsigned int j = 0; j < materials.size(); j++) {
                            if (materials[j] != nullptr && *sameMaterials[i] == *materials[j]) {
                                materials[j]->id = i;
                            }
                       }
                   }
               }
            }
            Material::~Material() {
                std::vector<Material*>::iterator it;
                if (core::Application::app != nullptr) {
                    std::vector<Material*>& materials = core::Application::app->getMaterials();
                    for (it = materials.begin(); it != materials.end();) {
                        if (*it == this)
                            it = materials.erase(it);
                        else
                            it++;
                    }
                } else {
                    for (it = materials.begin(); it != materials.end();) {
                        if (*it == this)
                            it = materials.erase(it);
                        else
                            it++;
                    }
                }
                //updateIds();
            }
            Face::Face() {
                transform = nullptr;
            }
            Face::Face(sf::PrimitiveType primType, TransformMatrix& tm) : transform(&tm) {
                m_vertices.setPrimitiveType(primType);
                m_material = Material();
                m_material.addTexture(nullptr);
            }
            Face::Face(VertexArray va, Material mat, TransformMatrix& tm) : transform(&tm) {
                //std::cout<<"create face"<<std::endl;
                m_vertices = va;
                m_material = mat;
                //std::cout<<"face created"<<std::endl;
            }
            TransformMatrix& Face::getTransformMatrix() const {
                return *transform;
            }
            void Face::append(Vertex vertex, unsigned int indice) {
                m_vertices.append(vertex);
            }
            Material& Face::getMaterial() {
                return m_material;
            }
            void Face::setMaterial (Material& material) {
                m_material = material;
            }
            void Face::setTransformMatrix(TransformMatrix& tm) {
                transform = &tm;
            }
            VertexArray& Face::getVertexArray() {
                return m_vertices;
            }
            void Face::setVertexArray(VertexArray va) {
                m_vertices = va;
            }
            bool Face::useSameMaterial(Face& other) {
                return m_material == other.m_material;

            }
            bool Face::useSamePrimType (Face &other) {
                return m_vertices.getPrimitiveType() == other.m_vertices.getPrimitiveType();
            }
            bool Face::operator==(Face& other) {
                if (m_vertices.getVertexCount() != other.getVertexArray().getVertexCount())
                    return false;
                for (unsigned int i = 0; i < m_vertices.getVertexCount(); i++)
                    if (m_vertices[i] != other.m_vertices[i])
                        return false;
                return m_material == other.m_material && transform == other.transform;
            }
            bool Face::operator!=(Face& other) {
                return !(*this == other);
            }
            Instance::Instance () {
                material = nullptr;
                numInstances = 0;
            }
            Instance::Instance (Material& material, sf::PrimitiveType pType) {
                primType = pType;
                numInstances = 0;
                vertices = VertexArray(pType);
                this->material = &material;
            }
            void Instance::setPrimitiveType (sf::PrimitiveType pType) {
                primType = pType;
                vertices.setPrimitiveType(pType);
            }
            void Instance::setMaterial (Material& mat) {
                material = &mat;
            }
            void Instance::addVertexArray(VertexArray& va, TransformMatrix& tm) {
                m_transforms.push_back(&tm);
                m_vertexArrays.push_back(&va);

                //m_indexes.push_back(std::vector<unsigned int>());
                for (unsigned int i = 0; i < va.getVertexCount(); i++) {
                    //std::cout<<"transform : "<<tm.getMatrix()<<std::endl;
                    math::Vec3f t = tm.transform(math::Vec3f(va[i].position.x, va[i].position.y, va[i].position.z));
                    Vertex v (sf::Vector3f(t.x, t.y, t.z), va[i].color, va[i].texCoords);
                    vertices.append(v);

                    //std::cout<<"nb indexes : "<<nbIndexes<<std::endl;
                    //if (i < va.m_indexes.size() /*&& *va.m_indexes[i] < nbIndexes*/) {
                        //std::cout<<"add index to batcher"<<va.m_indexes[i]<<std::endl;
                        /*m_indexes.back().push_back(va.m_indexes[i]);
                        allIndexes.push_back(va.m_indexes[i]);*/
                    //}
                }
            }
            void Instance::addVertexShadowArray (VertexArray& va, TransformMatrix& tm, ViewMatrix& viewMatrix, TransformMatrix shadowProjMatrix) {
                m_transforms.push_back(&tm);
                m_vertexArrays.push_back(&va);
                m_shadowProjMatrix.push_back(shadowProjMatrix);
                //shadowProjMatrix.combine(viewMatrix.getMatrix());
                shadowProjMatrix.combine(tm.getMatrix());
                for (unsigned int i = 0; i < va.getVertexCount(); i++) {
                    math::Vec3f t = shadowProjMatrix.transform(math::Vec3f(va[i].position.x, va[i].position.y, va[i].position.z));
                    Vertex v (sf::Vector3f(t.x, t.y, t.z), va[i].color, va[i].texCoords);
                    vertices.append(v);
                }
            }
            void Instance::sortVertexArrays(View& view) {
                vertices.clear();
                allIndexes.clear();
                m_indexes.clear();
                std::multimap<float, VertexArray*> sortedVA;
                std::multimap<float, TransformMatrix*> sortedTM;
                std::multimap<float, unsigned int> sortedIndexes;
                for (unsigned int i = 0; i < m_vertexArrays.size(); i++) {
                    odfaeg::math::Vec3f center;
                    for (unsigned int j = 0; j < m_vertexArrays[i]->getVertexCount(); j++) {
                        center += m_transforms[i]->transform(odfaeg::math::Vec3f((*m_vertexArrays[i])[j].position.x,(*m_vertexArrays[i])[j].position.y,(*m_vertexArrays[i])[j].position.z));
                    }
                    center = center / m_vertexArrays[i]->getVertexCount();
                    //center = view.getViewMatrix().transform(center);
                    sortedVA.insert(std::make_pair(center.z, m_vertexArrays[i]));
                    sortedTM.insert(std::make_pair(center.z, m_transforms[i]));
                }
                m_vertexArrays.clear();
                m_transforms.clear();
                std::multimap<float, VertexArray*>::iterator it;
                std::multimap<float, TransformMatrix*>::iterator it2;
                for (it = sortedVA.begin(), it2 = sortedTM.begin(); it != sortedVA.end(); it++, it2++) {
                    VertexArray* va = it->second;
                    TransformMatrix* tm = it2->second;
                    m_vertexArrays.push_back(va);
                    m_transforms.push_back(tm);
                    m_indexes.push_back(std::vector<unsigned int>());
                    for (unsigned int i = 0; i < va->getVertexCount(); i++) {
                        math::Vec3f t = tm->transform(math::Vec3f((*va)[i].position.x, (*va)[i].position.y, (*va)[i].position.z));
                        Vertex v (sf::Vector3f(t.x, t.y, t.z), (*va)[i].color, (*va)[i].texCoords);
                        vertices.append(v);
                        /*if (i < va->m_indexes.size()) {
                            m_indexes.back().push_back(va->m_indexes[i]);
                            allIndexes.push_back(va->m_indexes[i]);
                        }*/
                    }
                }
            }
            VertexArray& Instance::getAllVertices() {
                return vertices;
            }
            std::vector<VertexArray*> Instance::getVertexArrays() {
                return m_vertexArrays;
            }
            std::vector<unsigned int> Instance::getAllIndexes() {
                return allIndexes;
            }
            std::vector<std::vector<unsigned int>> Instance::getIndexes() {
                return m_indexes;
            }
            void Instance::clear() {
                m_transforms.clear();
                m_vertexArrays.clear();
                m_shadowProjMatrix.clear();
            }
            std::vector<TransformMatrix*> Instance::getTransforms() {
                 return m_transforms;
            }
            std::vector<TransformMatrix> Instance::getShadowProjMatrix() {
                return m_shadowProjMatrix;
            }
            Material& Instance::getMaterial() {
                return *material;
            }
            sf::PrimitiveType Instance::getPrimitiveType() {
                return primType;
            }
            unsigned int Instance::getNumInstances() {
                return numInstances;
            }
            Instance::~Instance() {
                m_transforms.clear();
                m_vertexArrays.clear();
                m_shadowProjMatrix.clear();
            }

            Batcher::Batcher() {
                numVertices = 0;
                numIndexes = 0;
                nbLayers = 0;
            }
            void Batcher::addFace(Face* face) {
                /*if (face->getVertexArray().getEntity() != nullptr && face->getVertexArray().getEntity()->getRootType() == "E_BIGTILE")
                        std::cout<<"add E_TILE : "<<face->getVertexArray().getPrimitiveType() * core::Application::app->getNbMaterials() + face->getMaterial().getId()<<std::endl;*/
                //std::cout<<"instance id : "<<face->getVertexArray().getPrimitiveType() * Material::getNbMaterials() + face->getMaterial().getId()<<std::endl;
                Instance& instance = instances[face->getVertexArray().getPrimitiveType() * core::Application::app->getNbMaterials() + face->getMaterial().getId()];
                instance.setPrimitiveType(face->getVertexArray().getPrimitiveType());
                instance.setMaterial(face->getMaterial());
                instance.addVertexArray(face->getVertexArray(),face->getTransformMatrix());
                /*if (face->getVertexArray().getEntity()->getType() == "E_MESH") {
                sf::Image img = instance.getMaterial().getTexture()->copyToImage();
                                for (unsigned int i = 0; i < img.getSize().x; i++) {
                                    for (unsigned int j = 0; j < img.getSize().y; j++) {
                                        if (img.getPixel(i, j).r > 0 || img.getPixel(i, j).g > 0 || img.getPixel(i, j).b > 0)
                                        std::cout<<"pixel : "<<(int) img.getPixel(i, j).r<<" , "<<(int) img.getPixel(i, j).g<<" , "<<(int) img.getPixel(i, j).b<<std::endl;
                                    }
                                    }
                                }*/
                /*bool added = false;
                for (unsigned int i = 0; i < instances.size() && !added; i++) {
                    if (instances[i].getMaterial() == face->getMaterial()
                        && instances[i].getPrimitiveType() == face->getVertexArray().getPrimitiveType()) {
                            added = true;
                            instances[i].addVertexArray(face->getVertexArray(),face->getTransformMatrix());
                    }
                }
                if (!added) {
                    Instance instance (face->getMaterial(), face->getVertexArray().getPrimitiveType());
                    instance.addVertexArray(face->getVertexArray(),face->getTransformMatrix());
                    instances.push_back(instance);
                }
                for (unsigned int i = 0; i < face->getVertexArray().getVertexCount(); i++) {
                    math::Vec3f tPos = face->getTransformMatrix().transform(math::Vec3f(face->getVertexArray()[i].position.x, face->getVertexArray()[i].position.y, face->getVertexArray()[i].position.z));
                    if (nbLayers == 0) {
                        nbLayers++;
                        tmpZPos.push_back(tPos.z);
                    } else {
                        bool found = false;
                        for (unsigned int j = 0; j < tmpZPos.size() && !found; j++) {
                            if (tmpZPos[j] == tPos.z) {
                                found = true;
                            }
                        }
                        if (!found) {
                            tmpZPos.push_back(tPos.z);
                            nbLayers++;
                        }
                    }
                }*/
            }
            void Batcher::addShadowFace(Face* face, ViewMatrix viewMatrix, TransformMatrix shadowProjMatrix) {

                Instance& instance = instances[face->getVertexArray().getPrimitiveType() * core::Application::app->getNbMaterials() + face->getMaterial().getId()];
                instance.setPrimitiveType(face->getVertexArray().getPrimitiveType());
                instance.setMaterial(face->getMaterial());
                instance.addVertexShadowArray(face->getVertexArray(),face->getTransformMatrix(), viewMatrix, shadowProjMatrix);
                /*bool added = false;
                for (unsigned int i = 0; i < instances.size() && !added; i++) {
                    if (instances[i].getMaterial() == face->getMaterial()
                        && instances[i].getPrimitiveType() == face->getVertexArray().getPrimitiveType()) {
                            added = true;
                            instances[i].addVertexShadowArray(face->getVertexArray(),face->getTransformMatrix(), viewMatrix, shadowProjMatrix);
                    }
                }
                if (!added) {
                    Instance instance (face->getMaterial(), face->getVertexArray().getPrimitiveType());
                    instance.addVertexShadowArray(face->getVertexArray(),face->getTransformMatrix(), viewMatrix,shadowProjMatrix);
                    instances.push_back(instance);
                }*/
            }
            std::vector<Instance> Batcher::getInstances() {
                /*std::vector<Instance> insts;
                for (unsigned int i = 0; i < instances.size(); i++) {
                    insts.push_back(instances[i]);
                }
                return insts;*/
                return instances;
            }
            unsigned int Batcher::getNumIndexes() {
                return numIndexes;
            }
            unsigned int Batcher::getNumVertices() {
                return numVertices;
            }
            unsigned int Batcher::getNbLayers() {
                return nbLayers;
            }
            void Batcher::clear() {
                instances.clear();
                //std::cout<<"nb instances : "<<nbPrimitiveTypes * Material::getNbMaterials()<<std::endl;
                instances.resize(nbPrimitiveTypes * core::Application::app->getNbMaterials());
                nbLayers = 0;
                //tmpZPos.clear();
            }
    }
}
