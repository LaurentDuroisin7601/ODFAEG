#ifndef ODFAEG_DECOR_2D_HPP
#define ODFAEG_DECOR_2D_HPP
#include <SFML/Graphics.hpp>
#include "../tile.h"
#include "../model.h"
#include "../../Graphics/world.h"
#include "../light.h"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace graphic {
        namespace g2d {
            /**
              * \file decor.h
              * \class Decor
              * \brief Represent a decor. (It can bee a tree, an house, ..., every objects except a wall.)
              * \author Duroisin.L
              * \version 1.0
              * \date 1/02/2014
              */
            class ODFAEG_GRAPHICS_API Decor : public Selectable, public Model {
            public :
                /**
                  * \fn Decor()
                  * \brief constructor.
                  */
                Decor(EntityFactory& factory) : Model(math::Vec3f(0, 0, 0), math::Vec3f(1, 1, 0), math::Vec3f(0.5f, 0.5f, 0.f),factory, "E_DECOR"){

                }
                Entity* clone();
                /**
                  * \fn Decor(Tile *t, Light* light, int height, Shadow::SHADOW_TYPE shadowType)
                  * \brief constructor.
                  * \param Tile tile : the tile of the decor.
                  * \param Light* light : the light, used to generate the shadow of the decor. (generally this is the ambient light)
                  * \param height : the height of the decor.
                  * \param Shadow::SHADOW_TYPE : the type of shadow to generate. (A shape, or a black sprite)
                  */
                Decor(Tile *t, Light *light, EntityFactory& factory);
                bool operator== (Entity &other);
                /**
                 *\fn void vtserialize(Archive & ar)
                 *\brief serialize the decor.
                 *\param the archive where to serialize the datas.
                */
                void createShadow(Light& light);
                template <typename Archive>
                void vtserialize(Archive & ar) {
                    Model::vtserialize(ar);
                }
                void onMove(math::Vec3f& t);
                /** \fn bool isAnimated
                *   \brief return false because the class is not a class of an animated entity.
                *   \return bool false. (unanimated entity)
                */
                bool isAnimated() const {
                    return false;
                }
                /** \fn bool haveShadow()
                *   \brief return false because the class is not a class of an animated entity.
                *   \return bool false. (unanimated entity)
                */
                bool haveShadow() const {
                     return true;
                }
                /** \fn bool selectable()
                *   \brief return true because the class is an entity that you can select.
                *   \return bool true. (selectable entity)
                */
                bool selectable () const {
                    return true;
                }
                /** \fn bool isLight()
                *   \brief return false because the class is a class of an entity that isn't a light.
                *   \return bool false. (unligthed entity)
                */
                bool isLight() const {
                    return false;
                }
                /** \fn bool isShadow
                *   \brief return false because the class is not a class of a shadow entity.
                *   \return bool false. (unshadowedentity)
                */
                bool isShadow() const {
                    return false;
                }
            };
        }
    }
}
#endif // DECOR_H
