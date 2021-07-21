#ifndef ODFEAG_AMBIENT_LIGHT_2D_HPP
#define ODFEAG_AMBIENT_LIGHT_2D_HPP
#include "../light.h"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace graphic {
        namespace g2d {
            /**
              * \file ambientLight.h
              * \class AmbientLight
              * \brief The ambient light of the scene.
              * \author Duroisin.L
              * \version 1.0
              * \date 1/02/2014
              * Set up the ambient light for the scene.
              */
            class ODFAEG_API_EXPORT AmbientLight : public Light {
            private :
                  /**  \fn AmbientLight(math::Vec3f center, int height, sf::Color color)
                  *    \brief create an ambient light with the given center, height and color.
                  *    \param math::Vec3f center : the center of the ambient light.
                  *    \param int height : the height of the ambient light.
                  *    \param sf::Color color : the color of the ambient light.
                  */
                  AmbientLight (math::Vec3f center, int height, sf::Color color) :
                      Light (center, height, color) {

                  }
                  /**  \fn int getLightId
                  *    \return int& : the id of the light.
                  */
                  int getLightId() {
                      return id;
                  }
                  static AmbientLight ambientLight; /**> the current ambient light.*/
            public :
                  /**
                  *  \fn void setAmbientLight(math::Vec3f center, int height, sf::Color)
                  *  \brief set the AmbientLight of the scene.
                  *  \param math::Vec3f center : the center of the ambient light.
                  *  \param int height : the height of the ambient light.
                  *  \param sf::Color color : the color of the ambient light.
                  */
                  static void setAmbientLight(math::Vec3f center, int height, sf::Color color);
                  /**
                  *  \fn AmbientLight* getAmbientLight()
                  *  \brief get the ambient light of the scene.
                  */
                  static AmbientLight& getAmbientLight();
                  static int id; /**> the id of the ambient light.*/
            };
        }
    }
}
#endif // AMBIENT_LIGHT
