module;
#include <deque>
#include <array>
#include <string>
#include <vector>
export module odfaeg.graphic.material;
import odfaeg.graphic.texture;
import odfaeg.entity.color;
import odfaeg.math.vec;
import odfaeg.entity.gameObject;
namespace odfaeg {
	namespace graphic {
		export class Material {
            class TextureInfo {
            private:
                const Texture* texture; /**> A texture used by the material.*/                
            public:
                /**
                * \fn TextureInfo()
                * \brief constructor
                */
                TextureInfo();
                /**
                * \fn TextureInfo(const Texture* texture, IntRect rect, std::string texId="")
                * \brief constructor
                */
                TextureInfo(const Texture* texture);               
                /**
                * \fn bool operator== (TextureInfo& info)
                * \brief compare a texture info with another one.
                * \param the other texture info.
                * \return if two textures infos are the same.
                */
                bool operator== (const TextureInfo& info);
                /**
                * \fn bool operator!= (TextureInfo& info)
                * \param the other texture info.
                * \brief compare a texture info with another one.
                * \return if two textures infos are different.
                */
                bool operator!= (const TextureInfo& info);
                /**
                * \fn const Texture* getTexture() const
                * \brief return the texture.
                * \return the texture.
                */
                const Texture* getTexture() const;
                template <typename Archive>
                void vtserialize(Archive& ar) {
                    ////////std::cout<<"mat rect height : "<<rect.height<<std::endl;                    
                    
                    ////////std::cout<<"mat tex id : "<<texId<<std::endl;

                }
            };
        public:			
            Material();
            unsigned int getId();
            static unsigned int getNbMaterials();
            static std::deque<Material*> getSameMaterials();
            /**
            * \fn int getNbTextures ()
            * \brief return the number of textures.
            * \return the number of textures.
            */
            int getNbTextures();
            /**
            * \fn void addTexture (const Texture* texture, IntRect rect)
            * \brief add a texture to the material.
            * \param texture : the texture.
            * \param text : the texture coordinates.
            */
            void setTexture(const Texture* texture,entity::SubMesh::TexType texType, unsigned int texUnit=0, std::string texId="");
            void setLightInfos(math::Vec4f lightCenter, entity::Color lightColor);
            math::Vec4f getLightCenter();
            entity::Color getLightColor();  
            void setType(entity::SubMesh::Type type);
            entity::SubMesh::Type getType();
            /**
            * \fn const Texture* getTexture(int texUnit = 0)
            * \brief get the texture of the given unit. (0 = the first texture by default)
            * \param texUnit : the unit of the texture.
            * \return a pointer to the texture.
            */
            const Texture* getTexture(entity::SubMesh::TexType texType, int texUnit = 0);            
            /* \fn bool useSameTextures (const Material& material)
            * \brief check if two material are using the same textures.
            * \param material : the other material.
            * \return if the materials are using the same texture.
            */
            bool useSameTextures(const Material& material);
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
            bool operator!= (const Material& material);
            /**
            * \fn void serialize(Archive & ar)
            * \brief write the material into the given archive.
            * \param ar : the archive.
            */
            float getSpecularIntensity();
            float getSpecularPower();
            void setSpecularIntensity(float specularIntensity);
            void setSpecularPower(float specularPower);            
            void setRefractionFactor(float refractionFactor);
            float getRefractionFactor();
            void setReflectable(bool reflectable);
            void setRefractable(bool refractable);
            bool isReflectable();
            bool isRefractable();

            static void updateIds();
            static bool contains(Material& material);
            static void countNbMaterials();
            void setInstanceGroup(unsigned int instanceGroup);
            void setLayer(unsigned int layer);
            unsigned int getLayer();
            template <typename Archive>
            void serialize(Archive& ar) {
                ar(texInfos);
                ////////std::cout<<"color a : "<<color.a<<std::endl;
                ar(specularIntensity);
                ////////std::cout<<"specular intensity "<<specularIntensity<<std::endl;
                ar(specularPower);
                ////////std::cout<<"specular power : "<<specularPower<<std::endl;
                ar(reflectable);
                ar(refractable);
                ar(refractionFactor);
                ar(type);
                ar(instanceGroup);
                ar(lightCenter);
                ar(lightColor.r);
                ar(lightColor.g);
                ar(lightColor.b);
                ar(lightColor.a);
                if (ar.isInputArchive()) {
                    maxSpecularIntensity = (specularIntensity > maxSpecularIntensity) ? specularIntensity : maxSpecularIntensity;
                    maxSpecularPower = (specularPower > maxSpecularPower) ? specularPower : maxSpecularPower;
                }
            }     
            static std::deque<Material*> getAllMaterials();
		    int getInstanceGroup();
            ~Material();
            std::string name;
            static unsigned int getMaxSpecularIntensity();
            static unsigned int getMaxSpecularPower();
            static unsigned int getNbLayers();
        private:
            Material(const Material&) = delete;
            Material& operator=(const Material&) = delete;
            Material(const Material&&) = delete;
            Material& operator=(const Material&&) = delete;
            std::array<std::deque<TextureInfo>, entity::SubMesh::NBTEXTYPES> texInfos={}; /**> The informations about the textures. */
            entity::SubMesh::Type type;
            float specularIntensity, specularPower, refractionFactor;
            inline static unsigned int nbMaterials = 0;
            unsigned int id, layer;
		    int instanceGroup;
            inline static std::deque<Material*> materials = std::deque<Material*>();
            inline static std::deque<Material*> sameMaterials = std::deque<Material*>();
            bool reflectable, refractable;
            math::Vec4f lightCenter;
            entity::Color lightColor;            
            inline static unsigned int maxSpecularIntensity = 0;
            inline static unsigned int maxSpecularPower = 0;
            inline static unsigned int nbLayers = 0;
		};
	}
}