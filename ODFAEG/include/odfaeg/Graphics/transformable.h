#ifndef ODFAEG_TRANSFORMABLE_HPP
#define ODFAEG_TRANSFORMABLE_HPP

#include "../Math/transformMatrix.h"
#include "../Physics/boundingBox.h"
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
        class Transformable {
        public :
            /**
            * \fn BoundingBox getLocalBounds() const
            * \brief get the bounding box of the transformable object.
            * \return the bounding box of the global object.
            */
            void setName (std::string name) {
                this->name = name;
            }
            std::string getName() {
                return name;
            }
            virtual physic::BoundingBox getLocalBounds() const {
                return localBounds;
            }
            /**
            * \fn setCenter(Vec3f center)
            * \brief set the center of the transformable object.
            * \param the center of the transformable object.
            */
            void setCenter(math::Vec3f center) {
                math::Vec3f t = center - m_center;
                move(t);
            }
            /**
            * \fn Vec3f get the center of the transformable object.
            * \brief the center of the transformable object.
            * \return the center of the transformable object.
            */
            math::Vec3f getCenter() {
                return m_center;
            }
            /**
            * \fn void setPosition(Vec3f position)
            * \brief set the position of the transformable object.
            * \param the position of the transformable object.
            */
            //Set the position of the entity, the position is always the top left corner of the entity.
            void setPosition(math::Vec3f position) {
                //std::cout<<"positions : "<<position<<m_position<<std::endl;
                math::Vec3f t = position - m_position;
                /*if (getName() == "LFILE") {
                    std::cout<<"set position : "<<position<<m_position<<t<<std::endl;
                    std::cout<<position.x-m_position.x<<std::endl;
                }*/
                move(t);
            }
            /**
            * \fn Vec3f getSize() const
            * \brief get the size of the transformable object.
            * \return the size of the transformable object.
            */
            math::Vec3f getSize() const {
                return m_size;
            }
            /**
            * \fn Vec3f getPosition() const
            * \brief get the position of the transformable object.
            * \return the position of the transformable object.
            */
            math::Vec3f getPosition() const {
                return m_position;
            }
            /**
            * \fn set the rotation of the transformable object.
            * \brief rotate the transformation of the angle.
            * \param angle : the rotation angle.
            */
            void setRotation (float angle, math::Vec3f axis = math::Vec3f(0, 0, 1)) {
                tm.setEntityId(1);
                m_rotation = angle;
                tm.setRotation(axis, angle);
                recomputeBounds();
                //physic::BoundingBox bounds = getGlobalBounds();
                /*std::cout<<"get global bounds : "<<getGlobalBounds().getPosition()<<getGlobalBounds().getSize();
                std::cout<<"global bounds : "<<globalBounds.getPosition()<<globalBounds.getSize();*/
                /*if (getGlobalBounds().getPosition() != globalBounds.getPosition()
                    || getGlobalBounds().getSize() != globalBounds.getSize())
                    std::cout<<"different"<<std::endl;*/
                m_size = globalBounds.getSize();
                m_position = globalBounds.getPosition();
                onRotate(angle);
            }
            /**
            * \fn void setScale(Vec3f scale)
            * \brief set the scale of the transformable object.
            * \param the scale of the transformable object.
            */
            void setScale(math::Vec3f scale) {
                m_scale = scale;
                tm.setScale(math::Vec3f(scale.x, scale.y, scale.z));
                recomputeBounds();
                /*std::cout<<"get global bounds : "<<getGlobalBounds().getPosition()<<getGlobalBounds().getSize();
                std::cout<<"global bounds : "<<globalBounds.getPosition()<<globalBounds.getSize();*/
                /*if (getGlobalBounds().getPosition() != globalBounds.getPosition()
                    || getGlobalBounds().getSize() != globalBounds.getSize())
                    std::cout<<"different"<<std::endl;*/
                //physic::BoundingBox bounds = getGlobalBounds();
                m_size = localBounds.getSize() * m_scale;
                m_position = globalBounds.getPosition();
                /*if (name == "WALL") {
                    std::cout<<"set scale new size : "<<localBounds.getSize()<<m_scale<<m_size<<std::endl;
                }*/
                /*m_center = m_position + m_origin;
                tm.setTranslation(m_center);*/
                onScale(scale);
            }
            /**
            * \fn void scale(Vec3f s)
            * \brief scale the transformable object.
            * \param s the scale.
            */
            void scale(math::Vec3f s) {
                setScale(m_scale * s);
            }
            /**
            * \fn void rotate (float angle)
            * \brief rotate the transformable object.
            * \param angle : the angle of a rotate object.
            */
            void rotate (float angle, math::Vec3f axis = math::Vec3f(0, 0, 1)) {
                setRotation(m_rotation + angle, axis);
            }
            /**
            * \fn void move (Vec3f t)
            * \brief move the transformable entity.
            * \param t : the translation vector.
            */
            void move (math::Vec3f t) {
                m_center += t;
                m_position += t;
                /*if (getName() == "LFILE") {
                    std::cout<<"position : "<<m_position<<"t : "<<t<<std::endl;
                }*/
                tm.setTranslation(m_center);
                recomputeBounds();
                onMove(t);
            }
            /**
            * \fn void setOrigin (Vec3f origin)
            * \brief change the origin of the transformable object.
            * \param Vec3f : the origin.
            */
            void setOrigin(math::Vec3f origin) {

               m_origin = origin;
               tm.setOrigin(origin);
               m_center = m_position + m_origin;
               tm.setTranslation(m_center);
               recomputeBounds();
               onOriginChanged(origin);
            }
            /**
            * \fn Vec3f getScale() const
            * \brief get the scale of the transformable object.
            * \return the scale of the transformable object.
            */
            math::Vec3f getScale() const {
                return m_scale;
            }
            /**
            * \fn Vec3f getRotation() const
            * \brief get the rotation of the transformable object.
            * \return the rotation of the transformable object.
            */
            float getRotation() const {
                return m_rotation;
            }
            /**
            * \fn Vec3f getTranslation() const
            * \brief get the translation of the transformable object.
            * \return the translation of the transformable object.
            */
            math::Vec3f getTranslation() {
                return getGlobalBounds().getPosition() - getLocalPos();
            }
            /**
            * \fn Vec3f getOrigin() const
            * \brief get the origin of the transformable object.
            * \return the origin of the transformable object.
            */
            math::Vec3f getOrigin() const {
                return m_origin;
            }
            /**
            * \fn void setSize (Vec3f size)
            * \brief set the size of the transformable object.
            * \param size : the size of the transformable object.
            */
            virtual void setSize (math::Vec3f size) {
                math::Vec3f scale;
                if (m_size.x == 0 && size.x == 0) {
                    scale.x = 0;
                } else if (m_size.x == 0) {
                    scale.x = 1;
                    m_size.x = size.x;

                    localBounds.setSize(m_size.x, localBounds.getHeight(), localBounds.getDepth());
                } else {
                    scale.x = size.x / localBounds.getWidth();
                }
                if (m_size.y == 0 && size.y == 0) {
                    scale.y = 0;
                } else if (m_size.y == 0) {
                    /*if (name == "WALL") {
                        std::cout<<"change local bounds! "<<std::endl;
                    }*/
                    scale.y = 1;
                    m_size.y = size.y;
                    localBounds.setSize(localBounds.getWidth(), m_size.y, localBounds.getDepth());
                } else {
                    scale.y = size.y / localBounds.getHeight();
                }
                if (m_size.z == 0 && size.z == 0) {
                    scale.z = 0;
                } else if (m_size.z == 0) {
                    scale.z = 1;
                    m_size.z = size.z;
                    localBounds.setSize(localBounds.getWidth(), localBounds.getHeight(), m_size.z);
                } else {
                    scale.z = size.z / localBounds.getDepth();
                }

                m_scale = scale;
                tm.setScale(math::Vec3f(scale.x, scale.y, scale.z));
                recomputeBounds();
                m_size = localBounds.getSize() * m_scale;

                m_position = globalBounds.getPosition();
                onResize(size);
            }
            /**
            * \fn BoundingBox getGlobalBounds()
            * \brief get the local bounding box of the transformable object (without its transformation)
            */
            void recomputeBounds() {
                globalBounds = localBounds.transform(getTransform());
                /*if (name == "E_HERO")
                    std::cout<<"matrix : "<<getTransform().getMatrix()<<std::endl;*/
            }
            virtual physic::BoundingBox& getGlobalBounds() {
                return globalBounds;
            }
            /**
            * \fn void setLocalBounds(BoundingBox localBounds)
            * \brief set the local bounds of the transformable object.
            * \param localBounds the local bounds of the transformable object.
            */
            void setLocalBounds(physic::BoundingBox localBounds) {
                this->localBounds = localBounds;
                recomputeBounds();
            }
            /**
            * \fn const Matrix4f getMatrix ()
            * \brief get the matrix of the transformable object.
            * \return the matrix of the transformable object.
            */
            const math::Matrix4f getMatrix () {
                return tm.getMatrix();
            }
            /**
            * \fn Vec3f getLocalPos ()
            * \brief get the local position of the object.
            * \return get the local position of the object.
            */
            math::Vec3f getLocalPos () {
                return localBounds.getPosition();
            }
            /**
            * \fn Vec3f getLocalSize ()
            * \brief get the local size of the object.
            * \return get the local size of the object.
            */
            math::Vec3f getLocalSize() {
                return math::Vec3f(localBounds.getWidth(), localBounds.getHeight(), localBounds.getDepth());
            }
            /**
            * \fn TransformMatrix& getTransform()
            * \brief get the transform matrix of the transformable object.
            * \return the transform matrix.
            */
            TransformMatrix& getTransform() {
                return tm;
            }
            /**
            * \fn TransformMatrix getInverseTransform()
            * \brief get the inverse of the matrix transformation.
            * \return the inverse of the transformation matrix.
            */
            TransformMatrix getInverseTransform() {
                TransformMatrix invers = tm;
                invers.getMatrix().inverse();
                return invers;
            }
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
            void combine (TransformMatrix &tm){
                tm.combine(tm.getMatrix());
            }
            void setTransform (math::Matrix4f matrix) {
                tm.setMatrix(matrix);
            }
            template <typename Archive>
            void serialize (Archive & ar) {
                //std::cout<<"serialize name"<<std::endl;
                ar(name);
                //std::cout<<"serialize position"<<std::endl;
                ar(m_position);
                //std::cout<<"serialize center"<<std::endl;
                ar(m_center);
                //std::cout<<"serialize size"<<std::endl;
                ar(m_size);
                //std::cout<<"serialize origin"<<std::endl;
                ar(m_origin);
                //std::cout<<"serialize scale"<<std::endl;
                ar(m_scale);
                //std::cout<<"serialize rotation"<<std::endl;
                ar(m_rotation);
                //std::cout<<"serialize local bounds"<<std::endl;
                ar(localBounds);
                //std::cout<<"serialize global bounds"<<std::endl;
                ar(globalBounds);
                //std::cout<<"transform matrices"<<std::endl;
                ar(std::ref(tm));
            }
        protected :
            /**
            * \fn Transformable()
            * \brief constructor.
            */
            Transformable() {
                m_position = math::Vec3f(0, 0, 0);
                m_size = math::Vec3f(0, 0, 0);
                localBounds = physic::BoundingBox(m_position.x, m_position.y, m_position.z, m_size.x, m_size.y, m_size.z);
                m_origin = math::Vec3f(0, 0, 0);
                m_scale = math::Vec3f (1.f, 1.f, 1.f);
                m_rotation = 0;
                tm.setOrigin(m_origin);
                tm.setRotation(math::Vec3f::zAxis, 0);

                //std::cout<<"default constructor size : "<<localBounds.getSize()<<std::endl;
                globalBounds = localBounds.transform(getTransform());
                name = "";
            }
            /**
            * \fn Transformable(Vec3f position, Vec3f size, Vec3f origin)
            * \brief construct a transformable object.
            * \param position : the position of the transformable object.
            * \param size = the size of the transformable object.
            * \param origin = the origin of the transformable object.
            */
            Transformable(math::Vec3f position, math::Vec3f size, math::Vec3f origin, std::string name="") {
                localBounds = physic::BoundingBox(0, 0, 0, size.x, size.y, size.z);
                m_position = position;
                /*if (name == "WALL")
                    std::cout<<"constructor size : "<<localBounds.getSize()<<std::endl;*/
                //std::cout<<"create transform"<<std::endl;
                m_size = size;
                m_center = math::Vec3f (m_position.x + origin.x, m_position.y + origin.y, m_position.z + origin.z);
                m_origin = origin;
                m_scale = math::Vec3f (1.f, 1.f, 1.f);
                m_rotation = 0;
                tm.setOrigin(m_origin);
                tm.setRotation(math::Vec3f::zAxis, 0);
                tm.setTranslation(m_center);
                tm.setScale(m_scale);
                globalBounds = localBounds.transform(getTransform());
                this->name = name;
            }
            virtual void onOriginChanged(math::Vec3f& origin) {}
            virtual void onResize(math::Vec3f& s) {}
            /**
            * \fn virtual void onRotate(float angle)
            * \brief this function can be redefined in the sub-class if we need to do something when the object is rotating.
            */
            virtual void onRotate(float angle) {}
            /**
            * \fn virtual void onScale(Vec3f s)
            * \brief this function can be redefined in the sub-class if we need to do something when the object is rescaling.
            */
            virtual void onScale(math::Vec3f& s) {}
            /**
            * \fn virtual void onMove(Vec3f t)
            * \brief this function can be redefined in the sub-class if we need to do something when the object is moving.
            */
            virtual void onMove(math::Vec3f& t) {}
        protected :
            /**
             * m_position : the position of the top left corner of the entity
             * m_center : the center of the entity.
             * m_size : the size of the entity.
             * m_origin : the origin of the rotation and scale of the entity, the position of the origin is locel to entity's position.
             * m_rotation : the rotation of the antity with is given as degrees.
             * localBounds : the initial bounding rectangle of the entity. (without his transformations)
             * tm : the matrix for the transformation of the entity.
             */
            math::Vec3f m_position, m_center, m_size, m_origin, m_scale;
            float m_rotation;
            physic::BoundingBox localBounds, globalBounds;
            TransformMatrix tm;
            std::string name;
        };
    }
}
#endif // TRANSF_ENTITY
