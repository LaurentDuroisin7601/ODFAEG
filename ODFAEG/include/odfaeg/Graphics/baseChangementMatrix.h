#ifndef ODFAEG_BASE_CHANGEMENT_MATRIX
#define ODFAEG_BASE_CHANGEMENT_MATRIX
#include "../Math/matrix4.h"
#include "../Math/vec4.h"
#include "export.hpp"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace graphic {
        /**
          * \file baseChangementMatrix.h
          * \class BaseChangementMatrix
          * \brief Represent a matrix to pass from a landmark to another one.
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          *  Represent a matrix to pass from a landmark to another one.
          */
        class ODFAEG_GRAPHICS_API BaseChangementMatrix {
        public :
            BaseChangementMatrix();
            /** \fn void set2DIsoMatrix();
            *   \brief set the 2D isometric base changement matrix.
            */
            void set2DIsoMatrix();
            /**
            *  \fn Vec3f changeOfBase(Vec3f v);
            *  \brief convert the given point into the landmark.
            *  \param v : the given point.
            *  \return the coordinates of the point in the new landmark.
            */
            math::Vec3f changeOfBase(math::Vec3f v);
            /**
            *  \fn Vec3f unchangeOfBase(Vec3f v);
            *  \brief convert the given point of the landmark into the original landmark.
            *  \param v : the given point of the landmark.
            *  \return the coordinates of the point in the original landmark.
            */
            math::Vec3f unchangeOfBase(math::Vec3f v);
            /** \fn Matrix4f getMatrix()
            *   \brief get the base changement matrix.
            *    \return the base changement matrix.
            */
            math::Matrix4f getMatrix();
            bool isIso2DMatrix();
            template <typename Archive>
            void serialize(Archive &ar) {
                //std::cout<<"read matrix 2f"<<std::endl;
                ar(matrix4f);
                //std::cout<<"read inv matrix 2f"<<std::endl;
                ar(invMatrix4f);
                //std::cout<<"is 2D iso matrix"<<std::endl;
                ar(iso2DMatrix);
                //std::cout<<"end read"<<std::endl;
            }
        private :
            math::Matrix4f matrix4f; /** > the base changement matrix.*/
            math::Matrix4f invMatrix4f; /**> the inverse of the base changement matrix.*/
            bool iso2DMatrix;
        };
    }
}
#endif // ODFAEG_BASE_CHANGEMENT_MATRIX
