module;
export module odfaeg.graphic.transformable;
import odfaeg.physic.boundingBox;
import odfaeg.entity.transformMatrix;
import odfaeg.entity.transformMatrix;
import odfaeg.math.vec;
import odfaeg.math.matrix;
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace graphic {
        /**
          * \file transformable.h
          * \class Transformable
          * \brief Represent each transform of a transformable object.
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          *
          * each object inherits from this class.
          */
        export class  Transformable {
        public:            
            virtual physic::BoundingBox getLocalBounds() const;
            /**
            * \fn setCenter(Vec3f center)
            * \brief set the center of the transformable object.
            * \param the center of the transformable object.
            */
            void setCenter(math::Vec3f center);
            /**
            * \fn Vec3f get the center of the transformable object.
            * \brief the center of the transformable object.
            * \return the center of the transformable object.
            */
            math::Vec3f getCenter();
            /**
            * \fn void setPosition(Vec3f position)
            * \brief set the position of the transformable object.
            * \param the position of the transformable object.
            */
            //Set the position of the entity, the position is always the top left corner of the entity.
            void setPosition(math::Vec3f position);
            /**
            * \fn Vec3f getSize() const
            * \brief get the size of the transformable object.
            * \return the size of the transformable object.
            */
            math::Vec3f getSize() const;
            /**
            * \fn Vec3f getPosition() const
            * \brief get the position of the transformable object.
            * \return the position of the transformable object.
            */
            math::Vec3f getPosition() const;
            /**
            * \fn set the rotation of the transformable object.
            * \brief rotate the transformation of the angle.
            * \param angle : the rotation angle.
            */
            void setRotation(float angle, math::Vec3f axis = math::Vec3f(0.f, 0.f, 1.f));
            /**
            * \fn void setScale(Vec3f scale)
            * \brief set the scale of the transformable object.
            * \param the scale of the transformable object.
            */
            void setScale(math::Vec3f scale);
            /**
            * \fn void scale(Vec3f s)
            * \brief scale the transformable object.
            * \param s the scale.
            */
            void scale(math::Vec3f s);
            /**
            * \fn void rotate (float angle)
            * \brief rotate the transformable object.
            * \param angle : the angle of a rotate object.
            */
            void rotate(float angle, math::Vec3f axis = math::Vec3f(0, 0, 1));
            /**
            * \fn void move (Vec3f t)
            * \brief move the transformable entity.
            * \param t : the translation vector.
            */
            void move(math::Vec3f t);
            /**
            * \fn void setOrigin (Vec3f origin)
            * \brief change the origin of the transformable object.
            * \param Vec3f : the origin.
            */
            void setOrigin(math::Vec3f origin);
			void setRelOrigin(math::Vec3f origin);
            /**
            * \fn Vec3f getScale() const
            * \brief get the scale of the transformable object.
            * \return the scale of the transformable object.
            */
            math::Vec3f getScale() const;
            /**
            * \fn Vec3f getRotation() const
            * \brief get the rotation of the transformable object.
            * \return the rotation of the transformable object.
            */
            float getRotation() const;
            math::Vec3f getRotationAxis() const;
            /**
            * \fn Vec3f getTranslation() const
            * \brief get the translation of the transformable object.
            * \return the translation of the transformable object.
            */
            math::Vec3f getTranslation();
            /**
            * \fn Vec3f getOrigin() const
            * \brief get the origin of the transformable object.
            * \return the origin of the transformable object.
            */
            math::Vec3f getOrigin() const;
            math::Vec3f getRelOrigin();
            /**
            * \fn void setSize (Vec3f size)
            * \brief set the size of the transformable object.
            * \param size : the size of the transformable object.
            */
            virtual void setSize(math::Vec3f size);
            /**
            * \fn BoundingBox getGlobalBounds()
            * \brief get the local bounding box of the transformable object (without its transformation)
            */
            void recomputeGlobalBounds();
            void recomputeBounds();
            virtual physic::BoundingBox& getGlobalBounds();
            /**
            * \fn void setLocalBounds(BoundingBox localBounds)
            * \brief set the local bounds of the transformable object.
            * \param localBounds the local bounds of the transformable object.
            */
            void setLocalBounds(physic::BoundingBox localBounds);
            /**
            * \fn const Matrix4f getMatrix ()
            * \brief get the matrix of the transformable object.
            * \return the matrix of the transformable object.
            */
            const math::Matrix4f getMatrix();
            /**
            * \fn Vec3f getLocalPos ()
            * \brief get the local position of the object.
            * \return get the local position of the object.
            */
            math::Vec3f getLocalPos();
            /**
            * \fn Vec3f getLocalSize ()
            * \brief get the local size of the object.
            * \return get the local size of the object.
            */
            math::Vec3f getLocalSize();
            /**
            * \fn TransformMatrix& getTransform()
            * \brief get the transform matrix of the transformable object.
            * \return the transform matrix.
            */
            entity::TransformMatrix& getTransform();
            /**
            * \fn TransformMatrix getInverseTransform()
            * \brief get the inverse of the matrix transformation.
            * \return the inverse of the transformation matrix.
            */
            entity::TransformMatrix getInverseTransform();
            /**
            * \fn void serialize (Archive & ar)
            * \brief serialize the object into the archive.
            * \param ar : the archive.
            */
            /**
           * \fn void combine (TransformMatrix &tm)
           * \brief combine the transform with another transform.
           * \param tm : the transform matrix.
           */
            void combine(entity::TransformMatrix& tm);
            void setTransform(entity::TransformMatrix& tm);
			void setOriginRelative(bool originRelative);
			bool isOriginRelative();
            template <typename Archive>
            void vtserialize(Archive& ar) {                
                ar(m_position);
                ////////std::cout<<"serialize center"<<std::endl;
                ar(m_center);
                ////////std::cout<<"serialize size"<<std::endl;
                ar(m_size);
                ////////std::cout<<"serialize origin"<<std::endl;
                ar(m_origin);
                ////////std::cout<<"serialize scale"<<std::endl;
                ar(m_scale);
                ////////std::cout<<"serialize rotation"<<std::endl;
                ar(m_rotation);
                ////////std::cout<<"serialize local bounds"<<std::endl;
                ar(localBounds);
                ////////std::cout<<"serialize global bounds"<<std::endl;
                ar(globalBounds);

                ////////std::cout<<"transform matrices"<<std::endl;
                ar(tm);
                if (ar.isInputArchive()) {
                    recomputeGlobalBounds();
                    recomputeBounds();
                }
            }
        protected:
            /**
            * \fn Transformable()
            * \brief constructor.
            */
            Transformable();
            /**
            * \fn Transformable(Vec3f position, Vec3f size, Vec3f origin)
            * \brief construct a transformable object.
            * \param position : the position of the transformable object.
            * \param size = the size of the transformable object.
            * \param origin = the origin of the transformable object.
            */
            Transformable(math::Vec3f position, math::Vec3f size, math::Vec3f origin);
            virtual void onResize(math::Vec3f& s);
            /**
            * \fn virtual void onRotate(float angle)
            * \brief this function can be redefined in the sub-class if we need to do something when the object is rotating.
            */
            virtual void onRotate(float angle);
            /**
            * \fn virtual void onScale(Vec3f s)
            * \brief this function can be redefined in the sub-class if we need to do something when the object is rescaling.
            */
            virtual void onScale(math::Vec3f& s);
            /**
            * \fn virtual void onMove(Vec3f t)
            * \brief this function can be redefined in the sub-class if we need to do something when the object is moving.
            */
            virtual void onMove(math::Vec3f& t);
            /**
             * m_position : the position of the top left corner of the entity
             * m_center : the center of the entity.
             * m_size : the size of the entity.
             * m_origin : the origin of the rotation and scale of the entity, the position of the origin is locel to entity's position.
             * m_rotation : the rotation of the antity with is given as degrees.
             * localBounds : the initial bounding rectangle of the entity. (without his transformations)
             * tm : the matrix for the transformation of the entity.
             */
            math::Vec3f m_position, m_center, m_size, m_origin,  m_scale, m_rotationAxis, m_relOrigin;
            float m_rotation;
            physic::BoundingBox localBounds, globalBounds, bounds;
            entity::TransformMatrix tm;      
            bool originRelative;
        };
    }
}

