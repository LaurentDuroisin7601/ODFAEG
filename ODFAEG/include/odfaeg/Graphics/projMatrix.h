#ifndef PROJ_MATRIX
#define PROJ_MATRIX
#include "../Math/matrix2.h"
#include "../Math/matrix4.h"
#include "../Math/vec4.h"
#include "../Math/vec2f.h"
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
          * \file projMatrix4.h
          * \class ProjMatrix
          * \brief Manage a matrix.
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          *
          * Manage projections and base changement matrix who are used to perform 2D and 3D projections.
          */
        class ODFAEG_GRAPHICS_API ProjMatrix {
            public :
                /**
                * \fn ProjMatrix()
                * \brief constructor. (Construct the ProjMatrix matrix)
                */
                ProjMatrix ();
                /**
                * \fn void reset()
                * \brief reset the projection matrix.
                */
                void reset ();
                /**
                * \fn Vec2f project(Vec2f vec2)
                * \brief project the vector from the 2D projection matrix and return the resulting vector.
                * \param Vec2f the vector to be projected.
                * \return the resulting vector.
                */
                math::Vec2f project (math::Vec2f vec2);
                /**
                * \fn Vec2f unProject(Vec2f vec2)
                * \brief unproject a vector from the 2D projection matrix and return the resulting vector.
                * \param the vector to be projected.
                * \return the resulting vector.
                */
                math::Vec2f unProject(math::Vec2f vec2);
                /**
                * \fn Vec3f project(Vec3f vec3)
                * \brief project a 3D vector from the 3D projection matrix and return the resulting vector.
                * \param the vector to be projected.
                * \return the resulting vector.
                */
                math::Vec3f project (math::Vec3f vec3);
                /**
                * \fn Vec3f unProject(Vec3f vec3)
                * \brief unproject a 3D vector from the 3D projection matrix and return the resulting vector.
                * \param Vec3f the vector to be projected.
                * \return the resulting vector.
                */
                math::Vec3f unProject(math::Vec3f vec3);
                /**
                * \fn float* getGlMatrix();
                * \brief return an array of the 3D projection matrix to pass it to opengl
                * \return an array with all the elements of the projection matrix.
                */
                float* getGlMatrix();
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
                void setGlPerspectiveMatrix (double l, double r, double b, double t, double n, double f);
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
                void setGlOrthoMatrix (double l, double r, double b, double t, double n, double f);
                void setScale (float x, float y, float z);
                math::Vec3f getScale();
                physic::BoundingBox getFrustum();
                math::Matrix4f getMatrix();
            private :
                math::Matrix4f matrix4f; /**< the 3D projection matrix */
                math::Matrix4f invMatrix4f; /**< the inverse of the 3D projection matrix */
                double l, r, b, t, n, f;
        };
    }
}
#endif
