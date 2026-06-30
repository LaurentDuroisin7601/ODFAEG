module;
export module odfaeg.graphic.projMatrix;
import odfaeg.math.matrix;
import odfaeg.math.vec;
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace graphic {
        /**
          * \file projMatrix4.h
          * \class ProjMatrix
          * \brief Manage a matrix.
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          *
          * Manage projections and base changement matrix who are used to perform 2D and 3D projections.
          */
        export class ProjMatrix {
        public:
            /**
            * \fn ProjMatrix()
            * \brief constructor. (Construct the ProjMatrix matrix)
            */
            ProjMatrix();
            /**
            * \fn void reset()
            * \brief reset the projection matrix.
            */
            void reset();
            /**
            * \fn Vec3f project(Vec3f vec3)
            * \brief project a 3D vector from the 3D projection matrix and return the resulting vector.
            * \param the vector to be projected.
            * \return the resulting vector.
            */
            math::Vec4f project(math::Vec4f vec3);
            /**
            * \fn Vec3f unProject(Vec3f vec3)
            * \brief unproject a 3D vector from the 3D projection matrix and return the resulting vector.
            * \param Vec3f the vector to be projected.
            * \return the resulting vector.
            */
            math::Vec4f unProject(math::Vec4f vec3);            
            /**
            * \fn void setGlPerspectiveMatrix (double l, double r, double b, double t, double n, double f)
            * \brief set a 3D perspective projection matrix. (right - left = width, top - bottom = height and
            * f - n = depth)
            * The frustum is a cube within the 3D visibles entities are clipped.
            * This function construct the perspective projection matrix with the given frustum and build an opengl
            * perspective projection matrix.
            * With opengl the vertex coordinates with a perspective projection must always be between -1 and 1.
            * \param l the left value of the frustum.
            * \param r the right value of the frustum.
            * \param b the bottom value of the furstum.
            * \param t the top value of the frustum.
            * \param n the near value of the frustum.
            * \param f the far value of the frustum.
            */
            //void setGlPerspectiveMatrix (double aspect, double tanHalFovy, double n, double f);
            void setPerspectiveMatrix(double l, double r, double b, double t, double n, double f);
            /**
           * \fn void setGlOrthoMatrix (double l, double r, double b, double t, double n, double f)
           * \brief set a 3D perspective projection matrix. (right - left = width, top - bottom = height and
           * f - n = depth)
           * The frustum is a cube within the 3D visibles entities are clipped.
           * This function construct the orthographic projection matrix with the given frustum and build an opengl
           * orthographic projection matrix.
           * \param l the left value of the frustum.
           * \param r the right value of the frustum.
           * \param b the bottom value of the furstum.
           * \param t the top value of the frustum.
           * \param n the near value of the frustum.
           * \param f the far value of the frustum.
           */
            void setOrthoMatrix(double l, double r, double b, double t, double n, double f);                     
            math::Matrix4f getMatrix();
        private:
            math::Matrix4f matrix4f; /**< the 3D projection matrix */
            math::Matrix4f invMatrix4f; /**< the inverse of the 3D projection matrix */            
        };
    }
}