#ifndef ODFAEG_TILE_2D_HPP
#define ODFAEG_TILE_2D_HPP

#include "../Math/vec4.h"

#include "../Math/maths.h"
#include "../Math/computer.h"
#include <vector>
#include "gameObject.hpp"
#include "selectable.h"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace graphic {

        /**
          * \file tile.h
          * \class Tile
          * \brief Respresent a tile.
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          * Represent a 2D tile.
          */
        class ODFAEG_GRAPHICS_API Tile : public GameObject, public Selectable {
            public :
                /**
                * \fn Tile();
                * \brief constructor.
                */
                Tile(EntityFactory& factory);
                /**
                * \fn Tile (const Texture *image, math::Vec3f position, math::Vec3f size, sf::IntRect subRect, Entity* parent = nullptr);
                * \brief constructor.
                * \param image : the texture of the tile.
                * \param position : the position of the tile.
                * \param size : the size of the tile.
                * \param subRect : the subrect of the texture's tile.
                * \param parent : the parent entity of the tile. (nullptr by default)
                */
                Tile (const Texture *image, math::Vec3f position, math::Vec3f size, sf::IntRect subRect, EntityFactory& factory,  sf::Color color = sf::Color::White, Entity* parent = nullptr);
                Entity* clone();
                void changeVerticesHeights (float h1, float h2, float h3, float h4);
                /**
                * \fn void setParentType (std::string parentType);
                * \brief set the type of the parent entity.
                * \param parentType : the type of the parent's entity.
                */
                void setParentType (std::string parentType);
                /**
                * \fn bool operator== (Entity &tile);
                * \brief check if two tiles are the same.
                * \param tile : the other tile.
                * \return if the two tile are the same.
                */
                bool operator== (Entity &tile);
                /**
                * \fn bool operator!= (Entity &tile);
                * \brief check if two tiles are different.
                * \param tile : the other tile.
                * \return if the two tiles are different.
                */
                bool operator!= (Entity &tile);
                /**
                * \fn void onDraw (RenderTarget &target, RenderStates states) const;
                * \brief draw the tile on the given render target with the given render states.
                * \param target : the render target.
                * \param states : the render states.
                */
                void onDraw (RenderTarget &target, RenderStates states);
                /**
                * \fn void setColor(sf::Color color);
                * \brief set the color of the tile.
                * \param color : the color.
                */
                void setColor(sf::Color color);
                /**
                * \fn bool isAnimated() const
                * \brief redefinition of the method from the base class Entity.
                * \return false, a tile is not an animation.
                */
                bool isAnimated() const {
                    return false;
                }
                /**
                * \fn bool isModel() const
                * \brief redefinition of the method from the base class Entity.
                * \return false, a tile is not an model.
                */
                bool isModel() const {
                    return false;
                }
                /**
                * \fn bool selectable() const
                * \brief redefinition of the method from the base class Entity.
                * \return true, a tile is selectable.
                */
                bool selectable() const {
                    return true;
                }
                /**
                * \fn bool isLight() const
                * \brief redefinition of the method from the base class Entity.
                * \return false, a tile is not a light.
                */
                bool isLight() const {
                    return false;
                }
                /**
                * \fn bool isShadow() const
                * \brief redefinition of the method from the base class Entity.
                * \return false, a tile is not a shadow.
                */
                bool isShadow() const {
                    return false;
                }
                 /**
                * \fn bool isLeaf() const
                * \brief redefinition of the method from the base class Entity.
                * \return true, a tile can't have children.
                */
                bool isLeaf() const {
                    return true;
                }
                /**
                * \fn void vtserialize(Archive & ar)
                * \brief serialize the tile into the given archive.
                * \param ar : the archive.
                */
                template <typename Archive>
                void vtserialize(Archive & ar) {
                    GameObject::vtserialize(ar);
                }
                sf::Color getColor();
                sf::IntRect getTexCoords();
                void setTexRect(sf::IntRect rect);
        };
    }
}
#endif

