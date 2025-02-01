#ifndef ODFAEG_SKYBOX_HPP
#define ODFAEG_SKYBOX_HPP
#include "../gameObject.hpp"
namespace odfaeg {
    namespace graphic {
        namespace g3d {
            class Skybox : public GameObject {
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
        }
    }
}
#endif // SKYBOX_HPP
