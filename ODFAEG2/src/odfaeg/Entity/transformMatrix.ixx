module;
export module odfaeg.entity.transformMatrix;
import odfaeg.math.vec;
import odfaeg.math.matrix;
namespace odfaeg {
  namespace entity {
        export class TransformMatrix {
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

        private:
            math::Matrix4f matrix4f; /**< the 3D transformation matrix*/
            math::Matrix4f invMat4f; /**< the 3D transformation's inverse matrix*/
            math::Vec3f t; /**< the 3D translation*/
            math::Vec3f s; /**< the 3D scale*/
            math::Vec3f o; /**< the 3D origin*/
            float rAngle; /**< the 3D rotation angle*/
            math::Vec3f rAxis; /**< the 3D rotation axis (the rotations'll be done around this axis)*/
            bool needToUpdate; /** < determine if the 3D transformation matrix have to be updated before the drawing*/
            bool inverseNeedToUpdate; /** < determine if the 3D transformation matrix have to be updated before the drawing*/
        public:
            /**
            * \fn TransformMatrix ()
            * \brief Default constructor : set the identity transformation's matrix.
            */
            TransformMatrix();
            void reset();
            /**
            * \fn void setTranslation (const Vec3f trans)
            * \brief set a 3D translation.
            * \param the 3D translation.
            */
            void setTranslation(const  math::Vec3f trans);
            /**
            * \fn void setScale (const Vec3f scale)
            * \brief set a 3D scale.
            * \param the 3D scale.
            */
            void setScale(const  math::Vec3f scale);
            /**
            * \fn setOrigin (Vec3f origin)
            * \brief set the coordinates of the 3D origin.
            * \param the 3D origin's position.
            */
            void setOrigin(math::Vec3f origin);
            void setRotation(const math::Vec3f axis, float teta);            
            /**
            * \fn Vec3f transform (const Vec3f vec3)
            * \brief transform a 3D point and give the resulting point
            * \param the point to be transformed.
            * \return the resulting point.
            */
            math::Vec4f transform(math::Vec4f vec4);
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
            math::Vec4f inverseTransform(math::Vec4f vec4);
            /**
            *   \fn Matrix4f get2DMatrix() const
            *   \brief return the 3D transformation matrix
            *   \return the 3D transformation matrix.
            */
            math::Matrix4f getMatrix();
            void setMatrix(math::Matrix4f matrix);
            /**
            *   \fn float* getGlMatrix()
            *   \brief return an array of the transformation matrix elements to pass it to opengl and
            *   construct the matrix again if it's necessary.
            */
            math::Vec3f getTranslation();
            math::Vec3f getOrigin();
            math::Vec3f getScale();
            float getRotation();
            bool operator== (const TransformMatrix& other) const;
        };
	}
}