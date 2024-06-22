#ifndef T_GROUND
#define T_GROUND
#include "../../../include/odfaeg/Graphics/tile.h"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace graphic {
        /**
        * \file tGround.h
        * \class BigTile
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        * \brief represent a sets of several tiles.
        */
        class BigTile : public GameObject, public Selectable {
        public :
            /**
            * \fn BigTile()
            * \brief constructor.
            */
            BigTile(EntityFactory& factory) : GameObject (math::Vec3f(0, 0, 0), math::Vec3f (0, 0, 0), math::Vec3f (0, 0, 0), "E_BIGTILE", factory) {}
            /**
            * \fn BigTile(math::Vec3f pos)
            * \brief constructor.
            * \param pos : the position of the big tile.
            */
            BigTile  (math::Vec3f pos, EntityFactory& factory, math::Vec2f tileSize=math::Vec2f(0, 0), int nbTilesPerRow = 0);
            /**
            * \fn bool operator== (Entity &entity);
            * \brief compare two entities.
            * \param entity : the other entity.
            */
            Entity* clone();
            bool operator== (Entity &entity);
            /**
            * \fn void addTile (Tile *tile);
            * \brief add a tile.
            * \param tile : the tile to add.
            */
            void addTile (Entity *tile, math::Vec2f tilePos=math::Vec2f(0, 0), float* height=nullptr);
            /**
            * \fn bool isAnimated() const
            * \brief redefinition of the method from the base class Entity.
            * \return false, a bigtile is not an animation.
            */
            float getHeight(math::Vec2f point);
            bool isAnimated() const {
                return false;
            }
            /**
            * \fn bool isModel() const
            * \brief redefinition of the method from the base class Entity.
            * \return false, a bigtile is not an model.
            */
            bool isModel() const {
                return false;
            }
            /**
            * \fn bool selectable() const
            * \brief redefinition of the method from the base class Entity.
            * \return true, a bigtile is selectable.
            */
            bool selectable() const {
                return true;
            }
            /**
            * \fn bool isLight() const
            * \brief redefinition of the method from the base class Entity.
            * \return false, a bigtile is not a light.
            */
            bool isLight() const {
                return false;
            }
            /**
            * \fn bool isShadow() const
            * \brief redefinition of the method from the base class Entity.
            * \return false, a bigtile is not a shadow.
            */
            bool isShadow() const {
                return false;
            }
            /**
            * \fn bool isLeaf() const
            * \brief redefinition of the method from the base class Entity.
            * \return false, a bigtile can have children.
            */
            bool isLeaf() const {
                return false;
            }
            /**
            * \fn void vtserialize(Archive & ar)
            * \brief serialize an entity into another one.
            */
            template <typename Archive>
            void vtserialize(Archive & ar) {
                GameObject::vtserialize(ar);
            }
        private :
            /** \fn
            *   \brief recompute the size of the bigtile when we add tiles.
            */
            void recomputeSize();
            math::Vec2f tileSize;
            int nbTilesPerRow;
            std::vector<Tile*> m_tiles;
        };
    }
}
#endif // T_GROUND

