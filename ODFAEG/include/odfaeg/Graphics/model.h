#ifndef ODFAEG_MODEL_2D_HPP
#define ODFAEG_MODEL_2D_HPP
#include "gameObject.hpp"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace graphic {
/**
* \file model.h
* \class Model
* \author Duroisin.L
* \version 1.0
* \date 1/02/2014
* \brief represent a 2D model, with a shadow and some other information to generate the shadows,
* and to check the intersections between the shadows and the lights.
*/
class ODFAEG_GRAPHICS_API Model : public GameObject {
protected :
    /**
    * \fn Model (math::Vec3f position, math::Vec3f size, math::Vec3f origin, std::string type = "E_MODEL", Entity *parent = nullptr)
    * \brief constructor
    * \param position : the position of the model.
    * \param size : the size of the model.
    * \param the type of the model. (E_MODEL by default)
    * \param the parent entity of the model. (null by default)
    */
    Model (math::Vec3f position, math::Vec3f size, math::Vec3f origin, EntityFactory& factory, std::string type = "E_MODEL", std::string name = "", Entity *parent = nullptr);
public :
    /**
    * \fn bool isModel() const;
    * \brief redefine the function isModel() from the base class entity.
    * \return if the entity is a model.
    */
    bool isModel() const;
    /**
    * \fn bool isAnimated() const;
    * \brief redefine the function isAnimated() from the base class entity.
    * \return if the entity is animated.
    */
    bool isAnimated() const;
    /**
    * \fn bool isLeaf() const;
    * \brief redefine the function isLeaf() from the base class entity.
    * \return if the entity is a leaf.
    */
    bool isLeaf() const;
    template <typename Archive>
    void vtserialize(Archive & ar) {
        GameObject::vtserialize(ar);
    }
};
}
}
#endif // ENTITY_WITH_SHADOW

