#ifndef ODFAEG_ANIMATED_ENTITY_2D_HPP
#define ODFAEG_ANIMATED_ENTITY_2D_HPP
#include <vector>
#include "gameObject.hpp"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace graphic {
        /**
          * \file animatedEntity.h
          * \class AmbientLight
          * \brief Base class of all animated entities.
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          */
        class ODFAEG_GRAPHICS_API AnimatedEntity : public GameObject {
            protected :
            /** \fn AnimatedEntity(math::Vec3f position, math::Vec3f size,math::Vec3f origin, std::string type, Entity* parent = nullptr)
            *   \brief Create an animated entity which the given position, size, origin, type and parent.
            *   \param math::Vec3f position : the position of the animated entity
            *   \param math::Vec3f size : the size of the animated entity.
            *   \param math::Vec3f origin : the origin of the entity.
            *   \param std::string type : the type of the entity.
            *   \param Entity* entity : the parent entity.
            */
            AnimatedEntity(math::Vec3f position, math::Vec3f size,math::Vec3f origin, std::string type, EntityFactory& factory, std::string name = "", Entity* parent = nullptr);
            public :
            /**\fn bool isAnimated() const
            *  \brief abstract method to redefine if the entity is animated.
            *  \return true because the entity is animated.
            */
            bool isAnimated() const {
                return true;
            }
            /**\fn bool isLeaf() const
            *  \brief abstract method to redefine if the entity is a leaf.
            *  \return false because it's not a leaf.
            */
            bool isLeaf() const {
                return false;
            }
            /**\fn bool isModel() const
            *  \brief abstract method to redefine if the entity is a model.
            *  \return false because the entity is not a model.
            */
            bool isModel() const {
                 return false;
            }
            /**\fn bool isLight() const
            *  \brief abstract method to redefine if the entity is a light.
            *  \return false because the entity is not a light.
            */
            bool isLight() const {
                return false;
            }
            /**\fn bool isShadow() const
            *  \brief abstract method to redefine if the entity is a shadow.
            *  \return false because the entity is not a shadow.
            */
            bool isShadow() const {
                return false;
            }
            /**\fn virtual Entity* getCurrentEntity() const = 0
            *  \brief to redefined in the derived class, return the current frame of the animation.
            *  \return Entity* the current frame of the animation.
            */
            template <typename Archive>
            void vtserialize(Archive & ar) {
                GameObject::vtserialize(ar);
            }
        };
    }
}
#endif // ANIMATED_ENTITY
