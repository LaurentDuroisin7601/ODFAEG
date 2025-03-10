#include "../../../include/odfaeg/Graphics/tGround.h"
namespace odfaeg {
    namespace graphic {
        using namespace sf;
        BigTile::BigTile (math::Vec3f pos, EntityFactory& factory, math::Vec2f tileSize, int nbTilesPerRow)
        : GameObject (pos, math::Vec3f (0, 0, 0), math::Vec3f (0, 0, 0), "E_BIGTILE", factory),
        tileSize(tileSize), nbTilesPerRow(nbTilesPerRow) {

        }
        Entity* BigTile::clone() {
            BigTile* bt = factory.make_entity<BigTile>(factory);
            GameObject::copy(bt);
            return bt;
        }
        void BigTile::addTile (Entity *tile, math::Vec2f tilePos, float* heights) {
            if (heights != nullptr) {
                tile->setDrawMode(Entity::NORMAL);
                math::Vec2f pos (0 - getPosition().x, 0 - getPosition().z);
                int xPosition = (tilePos.x + pos.x) / tileSize.x;
                int yPosition = (tilePos.y + pos.y) / tileSize.y;
                int position = yPosition * nbTilesPerRow + xPosition;
                int belowPos = (yPosition - 1) * nbTilesPerRow  + xPosition;

                if (xPosition - 1 >= 0) {
                    heights[0] = getChildren()[position-1]->getFace(0)->getVertexArray()[1].position.y;
                    heights[3] = getChildren()[position-1]->getFace(0)->getVertexArray()[2].position.y;
                }
                if (yPosition - 1  >= 0) {
                    heights[0] = getChildren()[belowPos]->getFace(0)->getVertexArray()[3].position.y;
                    heights[1] = getChildren()[belowPos]->getFace(0)->getVertexArray()[2].position.y;
                }
                tile->changeVerticesHeights(
                            heights[0],
                            heights[1],
                            heights[2],
                            heights[3]);
            }
            tile->setParent(this);
            addChild(tile);
        }
        bool BigTile::getHeight(math::Vec2f point, float& height) {
             if (getSize().z == 0) {
                if (point.x >= getGlobalBounds().getPosition().x && point.x < getGlobalBounds().getPosition().x + getGlobalBounds().getSize().x
                 && point.y >= getGlobalBounds().getPosition().y && point.y < getGlobalBounds().getPosition().y + getGlobalBounds().getSize().y) {
                     height = point.y;
                     return true;
                 } else {
                     return false;
                 }
             } else {
                 if (point.x >= getGlobalBounds().getPosition().x && point.x < getGlobalBounds().getPosition().x + getGlobalBounds().getSize().x
                     && point.y >= getGlobalBounds().getPosition().z && point.y < getGlobalBounds().getPosition().z + getGlobalBounds().getSize().z) {
                     math::Vec2f pos (0 - getPosition().x, 0 - getPosition().z);
                     int xPosition = (point.x + pos.x) / tileSize.x;
                     int yPosition = (point.y + pos.y) / tileSize.y;
                     int position = yPosition * nbTilesPerRow + xPosition;
                     float dx = (point.x - getChildren()[position]->getPosition().x) / tileSize.x;
                     float dy = (point.y - getChildren()[position]->getPosition().z) / tileSize.y;
                     int h1, h2, h3, h4;
                     h1 = getChildren()[position]->getFace(0)->getVertexArray()[0].position.y;
                     h2 = getChildren()[position]->getFace(0)->getVertexArray()[1].position.y;
                     h3 = getChildren()[position]->getFace(0)->getVertexArray()[2].position.y;
                     h4 = getChildren()[position]->getFace(0)->getVertexArray()[3].position.y;
                     height = ((1 - dx) * h1 + dx * h2) * (1 - dy) + ((1 - dx) * h4 + dx * h3) * dy;

                     return true;
                } else {
                     return false;
                }
             }
        }
        bool BigTile::operator== (Entity &other) {
            return GameObject::operator==(other);
        }
    }
}
