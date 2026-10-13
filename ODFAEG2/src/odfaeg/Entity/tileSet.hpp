#ifndef ODFAEG_TILESET_HPP
#define ODFAEG_TILESET_HPP
#include "gameObject.hpp"
#include "../Physics/boundingBox.hpp"
#include "tile.hpp"
namespace odfaeg {
    namespace entity {
       class TileSet : public GameObject {
        public :
            TileSet(unsigned int squareSize, physic::BoundingBox rect);
            void addTile(GameObject* tile);
            bool getHeight(math::Vec2f point, float& height);            
            GameObject* clone();
            template <typename Archive>
            void vtserialize(Archive& ar) {
                GameObject::vtserialize(ar);
            }
        private :
           int nbQuadsPerRow;
           unsigned int squareSize;
       };
    }
}
#endif