module;
#include <iostream>
#include <mutex>
#include <vulkan/vulkan.hpp>
#include <odfaeg/config.hpp>
#include <deque>
//import odfaeg.graphic.material;
module odfaeg.graphic.material;
namespace odfaeg {
	namespace graphic {
        Material::TextureInfo::TextureInfo() {
            texture = nullptr;   
        }
        Material::TextureInfo::TextureInfo(const Texture* texture) {
            //std::cout<<"texture : "<<texture;
            this->texture = texture;
        }
        std::deque<Material*> Material::getSameMaterials() {
            return sameMaterials;
        } 
        bool Material::TextureInfo::operator== (const TextureInfo& info) {
            //std::lock_guard<std::recursive_mutex> lock(getGlobalMutex());
            return texture == info.texture;
        }
        bool Material::TextureInfo::operator!= (const TextureInfo& info) {
            return !(*this == info);
        }
        const Texture* Material::TextureInfo::getTexture() const {
            //std::cout<<"texture : "<<texture;
            return texture;
        }
        Material::Material() {
            specularIntensity = 0;
            specularPower = 0;
            refractionFactor = 0;
            refractable = false;
            reflectable = false;            
            id = 0;            
            instanceGroup = -1;
            layer = 0;
            lightCenter = math::Vec3f(0.f, 0.f, 0.f);
            lightColor = entity::Color::White;
            for (unsigned int i = 0; i < entity::SubMesh::NBTEXTYPES; i++) {
                setTexture(nullptr, static_cast<entity::SubMesh::TexType>(i));
            }
            std::lock_guard<std::recursive_mutex> lock(getGlobalMutex());
            materials.push_back(this);
        }
        /*Material::Material(const Material&& material) : noexcept {
            texInfos = material.texInfos;
            id = material.id;
            specularIntensity = material.specularIntensity;
            specularPower = material.specularPower;
            refractionFactor = material.refractionFactor;
            refractable = material.refractable;
            reflectable = material.reflectable;            
            type = material.type;
            instanceGroup = material.instanceGroup;
            layer = material.layer;
            lightCenter = material.lightCenter;
            lightColor = material.lightColor;
            //std::lock_guard<std::recursive_mutex> lock(getGlobalMutex());
            //materials.push_back(this);
            //updateIds();
        }*/
        /*Material& Material::operator= (const Material& material) {
            texInfos = material.texInfos;
            id = material.id;
            specularIntensity = material.specularIntensity;
            specularPower = material.specularPower;
            refractionFactor = material.refractionFactor;
            refractable = material.refractable;
            reflectable = material.reflectable;            
            type = material.type;
            instanceGroup = material.instanceGroup;
            layer = material.layer;
            lightCenter = material.lightCenter;
            lightColor = material.lightColor;

            //materials.push_back(this);
            return *this;
        }*/
        void Material::setType(entity::SubMesh::Type type) {
            if (type == entity::SubMesh::AIR) {
                refractionFactor = 1;
            }
            else if (type == entity::SubMesh::WATER) {
                refractionFactor = 1.f / 1.33f;
            }
            else if (type == entity::SubMesh::ICE) {
                refractionFactor = 1.f / 1.309f;
            }
            else if (type == entity::SubMesh::GLASS) {
                refractionFactor = 1.f / 1.52f;
            }
            else {
                refractionFactor = 1.f / 2.42f;
            }
            this->type = type;
        }
        entity::SubMesh::Type Material::getType() {
            return type;
        }      
        void Material::setLightInfos(math::Vec4f center, entity::Color color) {
            lightCenter = center;
            lightColor = color;
            updateIds();
        }
        math::Vec4f Material::getLightCenter() {
            return lightCenter;
        }
        entity::Color Material::getLightColor() {
            return lightColor;
        }
        void Material::setInstanceGroup(unsigned int instanceGroup) {
            this->instanceGroup = instanceGroup;
        }
        void Material::setLayer(unsigned int layer) {
            nbLayers = (layer > nbLayers) ? layer : nbLayers;
            this->layer = layer;
        }
        unsigned int Material::getLayer() {
            return layer;
        }
        bool Material::contains(Material& material) {
            for (unsigned int i = 0; i < sameMaterials.size(); i++) {
                if (*sameMaterials[i] == material) {
                    return true;
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
            maxSpecularIntensity = (specularIntensity > maxSpecularIntensity) ? specularIntensity : maxSpecularIntensity;
            updateIds();
        }
        void Material::setSpecularPower(float specularPower) {
            this->specularPower = specularPower;
            maxSpecularIntensity = (specularPower > maxSpecularIntensity) ? specularIntensity : maxSpecularPower;
            updateIds();
        }        
        void Material::setRefractionFactor(float refractionFactor) {
            this->refractionFactor = refractionFactor;
            updateIds();
        }
        float Material::getRefractionFactor() {
            return refractionFactor;
        }
        int Material::getNbTextures() {
            return texInfos[0].size();
        }
        void Material::setTexture(const Texture* texture, entity::SubMesh::TexType texType, unsigned int texUnit, std::string texId) {
            //std::lock_guard<std::recursive_mutex> lock(getGlobalMutex());
            for (unsigned int i = 0; i < entity::SubMesh::NBTEXTYPES; i++) {
                if (texUnit >= texInfos[i].size())
                    texInfos[i].resize(texUnit+1);
            }
            //std::cout<<"size : "<<texInfos.size()<<","<<texInfos[texType].size()<<","<<texType<<","<<texUnit<<std::endl;
            TextureInfo texInfo(texture);
            texInfos[texType][texUnit] = texInfo;
            //updateIds();
        }        
        const Texture* Material::getTexture(entity::SubMesh::TexType texType, int texUnit) {
            std::lock_guard<std::recursive_mutex> lock(getGlobalMutex());
            return (texInfos.size() > 0) ? texInfos[texType][texUnit].getTexture() : nullptr;
        }  
        bool Material::useSameTextures(const Material& material) {
            for (unsigned int i = 0; i < entity::SubMesh::NBTEXTYPES; i++) {
                if (texInfos[i].size() != material.texInfos[i].size()) {
                    return false;
                }
            }
            for (unsigned int i = 0; i < entity::SubMesh::NBTEXTYPES; i++) {
                for (unsigned int j = 0; j < texInfos[i].size(); j++) {
                    if (texInfos[i][j] != material.texInfos[i][j]) {
                        return false;
                    }
                }
            }
            return true;
        }
        bool Material::operator== (const Material& material) {
            return useSameTextures(material)
                && specularIntensity == material.specularIntensity
                && specularPower == material.specularPower
                && refractionFactor == material.refractionFactor
                && refractable == material.refractable
                && reflectable == material.reflectable
                && instanceGroup == material.instanceGroup
                && layer == material.layer
                && lightCenter == material.lightCenter
                && lightColor == material.lightColor;
        }
        bool Material::operator!= (const Material& material) {
            return !(*this == material);
        }
        void Material::countNbMaterials() {
            nbMaterials = 0;
            sameMaterials.clear();
            for (unsigned int i = 0; i < materials.size(); i++) {
                if (materials[i] != nullptr && !contains(*materials[i])) {
                    nbMaterials++;
                    sameMaterials.push_back(materials[i]);
                }
            }
            //std::cout<<"nb materials : "<<sameMaterials.size()<<std::endl;
        }
        void Material::updateIds() {
            //std::lock_guard<std::recursive_mutex> lock(getGlobalMutex());
            countNbMaterials();
            for (unsigned int i = 0; i < sameMaterials.size(); i++) {
                for (unsigned int j = 0; j < materials.size(); j++) {
                    if (materials[j] != nullptr && *sameMaterials[i] == *materials[j]) {
                        materials[j]->id = i;
                    }
                }
            }
        }
	    int Material::getInstanceGroup() {
            return instanceGroup;
        }
        Material::~Material() {
            /*std::lock_guard<std::recursive_mutex> lock(getGlobalMutex());
            std::vector<Material*>::iterator it;
            for (it = materials.begin(); it != materials.end();) {
                if (*it == this)
                    it = materials.erase(it);
                else
                    it++;
            }*/
            //updateIds();
        }
        unsigned int Material::getMaxSpecularIntensity() {
            return maxSpecularIntensity;
        }
        unsigned int Material::getMaxSpecularPower() {
            return maxSpecularPower;
        }
        unsigned int Material::getNbLayers() {
            return nbLayers;
        }
        std::deque<Material*> Material::getAllMaterials() {
            return sameMaterials;
        }
	}
}