module;
#include <vector>
export module odfaeg.entity.tile;
import odfaeg.entity.gameObject;
import odfaeg.math.vec;
import odfaeg.entity.rect;
import odfaeg.entity.color;
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace entity {

        /**
          * \file tile.h
          * \class Tile
          * \brief Respresent a tile.
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          * Represent a 2D tile.
          */
        export class Tile : public GameObject {
            public :
                /**
                * \fn Tile();
                * \brief constructor.
                */
                Tile();
                /**
                * \fn Tile (const Texture *image, math::Vec3f position, math::Vec3f size, IntRect subRect, Entity* parent = nullptr);
                * \brief constructor.
                * \param image : the texture of the tile.
                * \param position : the position of the tile.
                * \param size : the size of the tile.
                * \param subRect : the subrect of the texture's tile.
                * \param parent : the parent entity of the tile. (nullptr by default)
                */
                Tile (std::string textureId, math::Vec3f position, math::Vec3f size, FloatRect subRect,  Color color = Color::White, GameObject* parent = nullptr);
                GameObject* clone() override;
                void changeVerticesHeights (float h1, float h2, float h3, float h4);
                /**
                * \fn bool operator== (Entity &tile);
                * \brief check if two tiles are the same.
                * \param tile : the other tile.
                * \return if the two tile are the same.
                */
                bool operator== (GameObject &tile);
                /**
                * \fn bool operator!= (Entity &tile);
                * \brief check if two tiles are different.
                * \param tile : the other tile.
                * \return if the two tiles are different.
                */
                bool operator!= (GameObject &tile);
                /**
                * \fn void setColor(Color color);
                * \brief set the color of the tile.
                * \param color : the color.
                */
                void setColor(Color color);
                /**
                * \fn void vtserialize(Archive & ar)
                * \brief serialize the tile into the given archive.
                * \param ar : the archive.
                */
                template <typename Archive>
                void vtserialize(Archive & ar) {
                    GameObject::vtserialize(ar);
                }
                Color getColor();
                FloatRect getTexCoords();
                void setTexRect(FloatRect rect);
        private:
            FloatRect texRect;
        };
    }
}
