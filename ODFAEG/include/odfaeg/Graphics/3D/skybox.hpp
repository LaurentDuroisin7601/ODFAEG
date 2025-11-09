#ifndef ODFAEG_SKYBOX_HPP
#define ODFAEG_SKYBOX_HPP
#include "../gameObject.hpp"
#include "../../config.hpp"
namespace odfaeg {
    namespace graphic {
        namespace g3d {
            #ifdef VULKAN
            class ODFAEG_GRAPHICS_API Skybox : public GameObject {
                public :
                Skybox  (float size, std::vector<std::string> filepaths, EntityFactory& factory, window::Device& vkDevice);
                void onDraw (RenderTarget& target, RenderStates states);
                Texture& getTexture();
                bool isAnimated() const {
                    return false;
                }
                bool isModel() const {
                    return false;
                }
                bool selectable() const {
                    return false;
                }
                bool isLight() const {
                    return false;
                }
                bool isShadow() const {
                    return false;
                }
                bool isLeaf() const {
                    return true;
                }
                Entity* clone();
                private :
                Texture skyboxCM;
                std::vector<std::string> filepaths;
                window::Device& vkDevice;
                float m_size;
            };
            #else
            class ODFAEG_GRAPHICS_API Skybox : public GameObject {
                public :
                Skybox  (std::vector<Image> skyboxImages, EntityFactory& factory);
                void onDraw (RenderTarget& target, RenderStates states);
                Texture& getTexture();
                bool isAnimated() const {
                    return false;
                }
                bool isModel() const {
                    return false;
                }
                bool selectable() const {
                    return false;
                }
                bool isLight() const {
                    return false;
                }
                bool isShadow() const {
                    return false;
                }
                bool isLeaf() const {
                    return true;
                }
                Entity* clone();
                private :
                Texture skyboxCM;
                std::vector<Image> skyboxImgs;
            };
            #endif
        }
    }
}
#endif // SKYBOX_HPP
