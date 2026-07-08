module;
#include <iostream>
//import odfaeg.graphic.transformable;
module odfaeg.graphic.transformable;
import odfaeg.physic.boundingBox;
import odfaeg.math.vec;



namespace odfaeg {
	namespace graphic {
        Transformable::Transformable() : m_position(0.f, 0.f, 0.f), m_center(0.f, 0.f, 0.f), m_size(0.f, 0.f, 0.f), m_origin(0.f, 0.f, 0.f), m_scale(1.f, 1.f, 1.f),
			m_rotation(0), originRelative(false), m_relOrigin(0.f, 0.f, 0.f) {
            tm.reset();
		}
		Transformable::Transformable(math::Vec3f position, math::Vec3f size, math::Vec3f origin) : m_position(position), m_center(position + origin), m_size(size),
			m_origin(origin), m_scale(1.f, 1.f, 1.f), m_rotation(0), originRelative(false), m_relOrigin(0.f, 0.f, 0.f) {
            tm.setOrigin(m_origin);
            tm.setRotation(math::Vec3f(0.f, 0.f, 1.f), m_rotation);
            //std::cout<<"center : "<<m_center<<std::endl;
            tm.setTranslation(m_center);
            tm.setScale(m_scale);
            recomputeGlobalBounds();
            recomputeBounds();
		}
        physic::BoundingBox Transformable::getLocalBounds() const {
            return localBounds;
        }
        /**
        * \fn setCenter(Vec3f center)
        * \brief set the center of the transformable object.
        * \param the center of the transformable object.
        */
        void Transformable::setCenter(math::Vec3f center) {
            math::Vec3f t = center - m_center;            
            move(t);
        }
        /**
        * \fn Vec3f get the center of the transformable object.
        * \brief the center of the transformable object.
        * \return the center of the transformable object.
        */
        math::Vec3f Transformable::getCenter() {
            return m_center;
        }
        /**
        * \fn void setPosition(Vec3f position)
        * \brief set the position of the transformable object.
        * \param the position of the transformable object.
        */
        //Set the position of the entity, the position is always the top left corner of the entity.
        void Transformable::setPosition(math::Vec3f position) {
            ////////std::cout<<"positions : "<<position<<m_position<<std::endl;
            math::Vec3f t = position - m_position;
            /*if (getName() == "LFILE") {
                //////std::cout<<"set position : "<<position<<m_position<<t<<std::endl;
                //////std::cout<<position.x-m_position.x<<std::endl;
            }*/
            move(t);
        }
        /**
        * \fn Vec3f getSize() const
        * \brief get the size of the transformable object.
        * \return the size of the transformable object.
        */
        math::Vec3f Transformable::getSize() const {
            return m_size;
        }
        /**
        * \fn Vec3f getPosition() const
        * \brief get the position of the transformable object.
        * \return the position of the transformable object.
        */
        math::Vec3f Transformable::getPosition() const {
            return m_position;
        }
        /**
        * \fn set the rotation of the transformable object.
        * \brief rotate the transformation of the angle.
        * \param angle : the rotation angle.
        */
        void Transformable::setRotation(float angle, math::Vec3f axis) {            
            m_rotation = angle;
            m_rotationAxis = axis;
            tm.setRotation(axis, angle);
            recomputeGlobalBounds();
            recomputeBounds();
            //physic::BoundingBox bounds = getGlobalBounds();
            /*//////std::cout<<"get global bounds : "<<getGlobalBounds().getPosition()<<getGlobalBounds().getSize();
            //////std::cout<<"global bounds : "<<globalBounds.getPosition()<<globalBounds.getSize();*/
            /*if (getGlobalBounds().getPosition() != globalBounds.getPosition()
                || getGlobalBounds().getSize() != globalBounds.getSize())
                //////std::cout<<"different"<<std::endl;*/
            m_size = globalBounds.getSize();
            m_position = globalBounds.getPosition();
            onRotate(angle);
        }
        /**
        * \fn void setScale(Vec3f scale)
        * \brief set the scale of the transformable object.
        * \param the scale of the transformable object.
        */
        void Transformable::setScale(math::Vec3f scale) {
            math::Vec3f newSize = bounds.getSize() * scale;

            if (m_size.x() == 0)
                m_scale[0] = 0;
            else
                m_scale[0] = newSize.x() / getLocalBounds().getSize().x();
            if (m_size.y() == 0)
                m_scale[1] = 0;
            else
                m_scale[1] = newSize.y() / getLocalBounds().getSize().y();
            if (m_size.z() == 0)
                m_scale[2] = 0;
            else
                m_scale[2] = newSize.z() / getLocalBounds().getSize().z();
            //std::cout<<"transformable scale : "<<m_scale<<std::endl;
            tm.setScale(m_scale);
            recomputeGlobalBounds();
            recomputeBounds();
            /*//////std::cout<<"get global bounds : "<<getGlobalBounds().getPosition()<<getGlobalBounds().getSize();
            //////std::cout<<"global bounds : "<<globalBounds.getPosition()<<globalBounds.getSize();*/
            /*if (getGlobalBounds().getPosition() != globalBounds.getPosition()
                || getGlobalBounds().getSize() != globalBounds.getSize())
                //////std::cout<<"different"<<std::endl;*/
                //physic::BoundingBox bounds = getGlobalBounds();
            m_size = globalBounds.getSize();
            m_position = globalBounds.getPosition();
            if (originRelative) {
                m_origin = m_size * m_relOrigin;
                tm.setOrigin(m_origin);
            }
            /*if (name == "WALL") {
                //////std::cout<<"set scale new size : "<<localBounds.getSize()<<m_scale<<m_size<<std::endl;
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
        void Transformable::scale(math::Vec3f s) {
            setScale(m_scale * s);
        }
        /**
        * \fn void rotate (float angle)
        * \brief rotate the transformable object.
        * \param angle : the angle of a rotate object.
        */
        void Transformable::rotate(float angle, math::Vec3f axis) {
            setRotation(m_rotation + angle, axis);
        }
        /**
        * \fn void move (Vec3f t)
        * \brief move the transformable entity.
        * \param t : the translation vector.
        */
        void Transformable::move(math::Vec3f t) {
            m_center += t;
            m_position += t;
            /*if (getName() == "LFILE") {
                //////std::cout<<"position : "<<m_position<<"t : "<<t<<std::endl;
            }*/
            tm.setTranslation(m_center);
            recomputeGlobalBounds();
            recomputeBounds();
            onMove(t);
        }
        /**
        * \fn void setOrigin (Vec3f origin)
        * \brief change the origin of the transformable object.
        * \param Vec3f : the origin.
        */
        void Transformable::setOrigin(math::Vec3f origin) {
            m_origin = origin;
            tm.setOrigin(origin);
            m_center = m_position + m_origin;
            tm.setTranslation(m_center);
            recomputeBounds();           
        }
        void Transformable::setRelOrigin(math::Vec3f origin) {
            m_relOrigin = origin;
        }
        void Transformable::setOriginRelative(bool relative) {
            originRelative = relative;
        }
        math::Vec3f Transformable::getRelOrigin() {
            return m_relOrigin;
        }
        bool Transformable::isOriginRelative() {
            return originRelative;
        }
        /**
        * \fn Vec3f getScale() const
        * \brief get the scale of the transformable object.
        * \return the scale of the transformable object.
        */
        math::Vec3f Transformable::getScale() const {
            return m_scale;
        }
        /**
        * \fn Vec3f getRotation() const
        * \brief get the rotation of the transformable object.
        * \return the rotation of the transformable object.
        */
        float Transformable::getRotation() const {
            return m_rotation;
        }
        math::Vec3f Transformable::getRotationAxis() const {
            return m_rotationAxis;
        }
        /**
        * \fn Vec3f getTranslation() const
        * \brief get the translation of the transformable object.
        * \return the translation of the transformable object.
        */
        math::Vec3f Transformable::getTranslation() {
            return getGlobalBounds().getPosition() - getLocalPos();
        }
        /**
        * \fn Vec3f getOrigin() const
        * \brief get the origin of the transformable object.
        * \return the origin of the transformable object.
        */
        math::Vec3f Transformable::getOrigin() const {
            return m_origin;
        }

        /**
        * \fn void setSize (Vec3f size)
        * \brief set the size of the transformable object.
        * \param size : the size of the transformable object.
        */
        void Transformable::setSize(math::Vec3f size) {            
            math::Vec3f scale;
            if (m_size.x() == 0 && size.x() == 0) {
                scale[0] = 0;
            }
            else if (m_size.x() == 0) {
                scale[0] = 1;
                m_size[0] = size.x();

                localBounds.setSize(m_size.x(), localBounds.getHeight(), localBounds.getDepth());
            }
            else {
                scale[0] = size.x() / localBounds.getWidth();
            }
            if (m_size.y() == 0 && size.y() == 0) {
                scale[1] = 0;
            }
            else if (m_size.y() == 0) {
                /*if (name == "WALL") {
                    //////std::cout<<"change local bounds! "<<std::endl;
                }*/

                scale[1] = 1;
                m_size[1] = size.y();

                localBounds.setSize(localBounds.getWidth(), m_size.y(), localBounds.getDepth());
            }
            else {
                scale[1] = size.y() / localBounds.getHeight();
            }
            if (m_size.z() == 0 && size.z() == 0) {
                scale[2] = 0;
            }
            else if (m_size.z() == 0) {
                scale[2] = 1;
                m_size[2] = size.z();
                localBounds.setSize(localBounds.getWidth(), localBounds.getHeight(), m_size.z());
            }
            else {
                scale[2] = size.z() / localBounds.getDepth();
            }

            m_scale = scale;
            tm.setScale(scale);
            recomputeGlobalBounds();
            recomputeBounds();
            m_size = globalBounds.getSize();
            if (originRelative) {
                m_origin = m_size * m_relOrigin;
                tm.setOrigin(m_origin);
            }
            m_position = globalBounds.getPosition();
            onResize(size);
        }
        /**
        * \fn BoundingBox getGlobalBounds()
        * \brief get the local bounding box of the transformable object (without its transformation)
        */
        void Transformable::recomputeGlobalBounds() {            
            /*if (name == "TSCRIPTEDIT")
                globalBounds.setName("TSCRIPTEDIT");*/
                ////////std::cout<<"tranform : "<<getTransform().getMatrix()<<std::endl;
                ////////std::cout<<"local bounds : "<<localBounds.getPosition()<<std::endl<<localBounds.getSize()<<std::endl;
            globalBounds = localBounds.transform(getTransform());
            ////////std::cout<<"global bounds : "<<globalBounds.getPosition()<<std::endl<<globalBounds.getSize()<<std::endl;
            /*if (name == "E_HERO")
                //////std::cout<<"matrix : "<<getTransform().getMatrix()<<std::endl;*/
        }
        void Transformable::recomputeBounds() {
            entity::TransformMatrix tm2 = tm;
            tm2.setRotation(m_rotationAxis, 0);
            bounds = localBounds.transform(tm2);
        }
        physic::BoundingBox& Transformable::getGlobalBounds() {
            return globalBounds;
        }
        /**
        * \fn void setLocalBounds(BoundingBox localBounds)
        * \brief set the local bounds of the transformable object.
        * \param localBounds the local bounds of the transformable object.
        */
        void Transformable::setLocalBounds(physic::BoundingBox localBounds) {
            this->localBounds = localBounds;
            recomputeGlobalBounds();
            recomputeBounds();
        }
        /**
        * \fn const Matrix4f getMatrix ()
        * \brief get the matrix of the transformable object.
        * \return the matrix of the transformable object.
        */
        const math::Matrix4f Transformable::getMatrix() {
            return tm.getMatrix();
        }
        /**
        * \fn Vec3f getLocalPos ()
        * \brief get the local position of the object.
        * \return get the local position of the object.
        */
        math::Vec3f Transformable::getLocalPos() {
            return localBounds.getPosition();
        }
        /**
        * \fn Vec3f getLocalSize ()
        * \brief get the local size of the object.
        * \return get the local size of the object.
        */
        math::Vec3f Transformable::getLocalSize() {
            return math::Vec3f(localBounds.getWidth(), localBounds.getHeight(), localBounds.getDepth());
        }
        /**
        * \fn TransformMatrix& getTransform()
        * \brief get the transform matrix of the transformable object.
        * \return the transform matrix.
        */
        entity::TransformMatrix& Transformable::getTransform() {
            return tm;
        }
        /**
        * \fn TransformMatrix getInverseTransform()
        * \brief get the inverse of the matrix transformation.
        * \return the inverse of the transformation matrix.
        */
        entity::TransformMatrix Transformable::getInverseTransform() {
            entity::TransformMatrix invers = tm;
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
        void Transformable::combine(entity::TransformMatrix& tm) {
            this->tm.combine(tm.getMatrix());
        }
        void Transformable::setTransform(entity::TransformMatrix& tm) {
            this->tm.setMatrix(tm.getMatrix());
        }           
        void Transformable::onResize(math::Vec3f& s) {}
        /**
        * \fn virtual void onRotate(float angle)
        * \brief this function can be redefined in the sub-class if we need to do something when the object is rotating.
        */
        void Transformable::onRotate(float angle) {}
        /**
        * \fn virtual void onScale(Vec3f s)
        * \brief this function can be redefined in the sub-class if we need to do something when the object is rescaling.
        */
        void Transformable::onScale(math::Vec3f& s) {}
        /**
        * \fn virtual void onMove(Vec3f t)
        * \brief this function can be redefined in the sub-class if we need to do something when the object is moving.
        */
        void Transformable::onMove(math::Vec3f& t) {}
	    }
}
