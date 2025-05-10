#ifndef ODFAEG_SKYBOX_HPP
#define ODFAEG_SKYBOX_HPP
#include "../gameObject.hpp"
#include "../../config.hpp"
namespace odfaeg {
    namespace graphic {
        namespace g3d {
            #ifdef VULKAN
            #else
            class ODFAEG_GRAPHICS_API Skybox : public GameObject {
                public :
                Skybox  (std::vector<sf::Image> skyboxImages, EntityFactory& factory);
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
                std::vector<sf::Image> skyboxImgs;
            };
            #endif
        }
    }
}
#endif // SKYBOX_HPP
