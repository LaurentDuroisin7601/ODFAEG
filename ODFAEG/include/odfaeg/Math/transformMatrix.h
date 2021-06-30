#ifndef TRANSFORM_MATRIX
#define TRANSFORM_MATRIX

#include "../Math/vec4.h"
#include "../Math/vec2f.h"
#include "../Math/matrix4.h"
#include "../Math/matrix3.h"
#include "../Math/maths.h"
#include <SFML/Graphics.hpp>
#include "export.hpp"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg
{
    namespace graphic {
        /**
          * \file transformMatrix.h
          * \class TransformMatrix
          * \brief Manage a transformation matrix.
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          *
          * Manage transformations matrix who are used to perform 2D and 3D transformations.
          * The construction of the transformations matrix depend on the translation, scale, rotation and origin provided.
          * The matrix is constructed only when it's necessary.
          */
        class ODFAEG_MATH_API TransformMatrix
        {
        private :
            math::Matrix4f matrix4f; /**< the 3D transformation matrix*/
            math::Matrix4f invMat4f; /**< the 3D transformation's inverse matrix*/
            math::Vec3f t3d; /**< the 3D translation*/
            math::Vec3f s3d; /**< the 3D scale*/
            math::Vec3f o3d; /**< the 3D origin*/
            float r3d; /**< the 3D rotation angle*/
            math::Vec3f r3dAxis; /**< the 3D rotation axis (the rotations'll be done around this axis)*/
            bool needToUpdate3D; /** < determine if the 3D transformation matrix have to be updated before the drawing*/
            bool inverseNeedToUpdate3D; /** < determine if the 3D transformation matrix have to be updated before the drawing*/
            unsigned int id;
            unsigned int entityId;
        public :
            /**
            * \fn TransformMatrix ()
            * \brief Default constructor : set the identity transformation's matrix.
            */
            TransformMatrix();
            void setTransformId(unsigned int id);
            unsigned int getTransformId();
            void setEntityId(unsigned int id);
            unsigned int getEntityId();
            float* getGlMatrix();
            /**
            * \fn void reset3D()
            * \brief Reset the 3D transformation matrix to the identity transformation's matrix.
            */
            void reset3D ();
            /**
            * \fn sf::Transform getSFMLTransform()
            * \brief return an sf::Transform object to pass the 2D transformation matrix to the SFML transformation matrix.
            * \return an sf::Transform object.
            */
            sf::Transform getSFMLTransform();
            /**
            * \fn void setTranslation (const Vec3f trans)
            * \brief set a 3D translation.
            * \param the 3D translation.
            */
            void setTranslation (const  math::Vec3f trans);
            /**
            * \fn void setScale (const Vec3f scale)
            * \brief set a 3D scale.
            * \param the 3D scale.
            */
            void setScale (const  math::Vec3f scale);
            /**
            * \fn void setRotation (const Vec3f axis, float teta)
            * \brief set the 3D rotation axis.
            *
            *        / <- 3D rotation axis.
            *      (/) <- rotation angle.
            *      /  . .
            *     /  . .  <- entity's vertices.
            *    /   . .
            *   O <- origin.
            * \param axis : the 3D rotation axis.
            * \param teta : the rotation angle.
            */
            void setRotation (const  math::Vec3f axis, float teta);
            /**
            * \fn setOrigin (float ox, float oy)
            * \brief set the coordinates of the 2D origin.
            * \param ox : the x coordinate of the origin
            * \param oy : the y coordinate of the origin
            */
            void setOrigin(float ox, float oy);
            /**
            * \fn setOrigin (Vec3f origin)
            * \brief set the coordinates of the 3D origin.
            * \param the 3D origin's position.
            */
            void setOrigin(math::Vec3f origin);
            float getRotation();
            /**
            * \fn Vec3f transform (const Vec3f vec3)
            * \brief transform a 3D point and give the resulting point
            * \param the point to be transformed.
            * \return the resulting point.
            */
            math::Vec3f transform (const  math::Vec3f vec3);
            void update();
            /**
            * \fn void combine (Matrix3f matrix)
            * \brief combine the transformation matrix with another transformatin on matrix.
            * \param the transformation matrix to be combined with.
            */
            void combine(math::Matrix4f matrix4f);
            /**
            * \fn Vec3f inverseTransform (const Vec3f)
            * \brief untransform a point and give the resulting point.
            * \param the point to be transformed.
            * \return the resulting point.
            */
            math::Vec3f inverseTransform (const  math::Vec3f vec3);
            /**
            *   \fn Matrix4f get2DMatrix() const
            *   \brief return the 3D transformation matrix
            *   \return the 3D transformation matrix.
            */
            math::Matrix4f getMatrix ();
            void setMatrix (math::Matrix4f matrix);
            /**
            *   \fn float* getGlMatrix()
            *   \brief return an array of the transformation matrix elements to pass it to opengl and
            *   construct the matrix again if it's necessary.
            */
            math::Vec3f getTranslation();
            math::Vec3f getOrigin();
            math::Vec3f getScale();
            bool operator== (const TransformMatrix& other);
            template <typename Archive>
            void serialize (Archive & ar) {
                ar(matrix4f);
                ar(t3d);
                ar(s3d);
                ar(r3d);
                ar(o3d);
            }
        };
    }
}
#endif


