module;
export module odfaeg.graphic.viewMatrix;
import odfaeg.math.matrix;
import odfaeg.math.vec;
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
        export class ViewMatrix {
        private:
            math::Matrix4f matrix4f; /**< the 3D view matrix */
            math::Matrix4f invMat4f; /**< the 3D inverssed view matrix */            
            bool needToUpdate; /**< determine if the 3D matrix need to be update.*/
            bool inverseNeedToUpdate; /**< determine if the 3D inversed matrix need to be update.*/
            math::Vec3f xAxis, yAxis, zAxis; 
            math::Vec3f center;
        public:
            /** \fn ViewMatrix()
            *   \brief constructor. (defines the identity matrix)
            */
            ViewMatrix();
            /** \fn reset2D()
            *   \brief set the 2D identity matrix.
            */
            void update();
            void reset();            
            /** \fn Vec3f transform (const Vec3f vec3)
            *   \brief convert a 3D vector with world coordinates to a 3D vector with view coordinates
            *   \return the  vector of view coordinates.
            */
            math::Vec4f transform(math::Vec4f vec3);
            /** \fn Vec3f inverseTransform (const Vec3f vec3)
            *   \brief convert a 3D vector with view coordinates to a 3D vector with world coordinates
            *   \return the  vector of world coordinates.
            */
            math::Vec4f inverseTransform(math::Vec4f vec3);
            /** \fn Matrix3f get3DMatrix()
            *   \brief return the 3D view matrix
            *   \return the 3D view matrix.
            */
            math::Matrix4f getMatrix();
            /** \fn float* getGlMatrix()
            *   \brief return an array with the view matrix elements to pass it to opengl.
            *   \return an array with the matrix elements.
            */            
            void setAxis(math::Vec3f left, math::Vec3f up, math::Vec3f forward);
            void setCenter(math::Vec3f center);
        };
    }
}