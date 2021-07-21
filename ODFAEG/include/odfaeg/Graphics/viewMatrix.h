#ifndef VIEW_MATRIX
#define VIEW_MATRIX
#include "../Math/vec4.h"
#include "../Math/vec2f.h"
#include "../Math/matrix4.h"
#include "../Math/matrix3.h"
#include "../Math/maths.h"
#include "../Physics/boundingBox.h"
#include "export.hpp"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace graphic {
        /**
          * \file viewMatrix.h
          * \class ViewMatrix
          * \brief Manage a view matrix
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          *
          * This class defines a view matrix, who's used to convert world coordinates into view coordinates.
          */
        class ODFAEG_GRAPHICS_API ViewMatrix {
            private :
                math::Matrix4f matrix4f; /**< the 3D view matrix */
                math::Matrix4f invMat4f; /**< the 3D inverssed view matrix */
                math::Vec3f s3d; /** < the 3D scale vector*/
                math::Vec3f o3d; /** < the 3D origin*/
                bool needToUpdate3D; /**< determine if the 3D matrix need to be update.*/
                bool inverseNeedToUpdate3D; /**< determine if the 3D inversed matrix need to be update.*/
                math::Vec3f xAxis, yAxis, zAxis;
                float angle;
            public :
            /** \fn ViewMatrix()
            *   \brief constructor. (defines the identity matrix)
            */
            ViewMatrix();
            /** \fn reset2D()
            *   \brief set the 2D identity matrix.
            */
            void update();
            void reset3D ();
            /** \fn setTranslation (const Vec3f trans)
            *   \brief set the 3D translation vector.
            */
            void setTranslation (const math::Vec3f trans);
            /** \fn setScale (const Vec3f scale)
            *   \brief set the 3D scale vector.
            */
            void setScale (const math::Vec3f scale);
            /** \fn void setOrigin (Vec3f)
            *   \brief set the origin of the 3D view matrix.
            *   \param the origin of the view matrix.
            */
            void setOrigin(math::Vec3f origin);
            /** \fn Vec3f transform (const Vec3f vec3)
            *   \brief convert a 3D vector with world coordinates to a 3D vector with view coordinates
            *   \return the  vector of view coordinates.
            */
            math::Vec3f transform (const math::Vec3f vec3);
            /** \fn Vec3f inverseTransform (const Vec3f vec3)
            *   \brief convert a 3D vector with view coordinates to a 3D vector with world coordinates
            *   \return the  vector of world coordinates.
            */
            math::Vec3f inverseTransform (const math::Vec3f vec3);
            /** \fn Matrix3f get3DMatrix()
            *   \brief return the 3D view matrix
            *   \return the 3D view matrix.
            */
            math::Matrix4f getMatrix ();
            /** \fn float* getGlMatrix()
            *   \brief return an array with the view matrix elements to pass it to opengl.
            *   \return an array with the matrix elements.
            */
            float* getGlMatrix();
            math::Vec3f getScale() {
                return s3d;
            }
            void setAxis(math::Vec3f left,  math::Vec3f up,  math::Vec3f forward);
            void setRotation(float angle);
            void combine(math::Matrix4f other);
        };
    }
}
#endif

