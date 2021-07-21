#ifndef ODFAEG_BOUNDING_SPHERE_HPP
#define ODFAEG_BOUNDING_SPHERE_HPP
#include "../Math/vec2f.h"
#include "../Math/ray.h"
#include "boundingVolume.h"
#include "collisionResultSet.hpp"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace physic {
        /**
          * \file boundingSphere.h
          * \class BoudingSphere
          * \brief Manage a bounding sphere for collision detection
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          *
          * Manage a bounding sphere for collision detection.
          *
          */
        class BoundingEllipsoid;
        class OrientedBoundingBox;
        class BoundingPolyhedron;
        class BoundingBox;
        class BoundingSphere : public BoundingVolume {
            public :
                BoundingSphere();
                /**
                * \fn BoundingSphere (Vec2f center, float radius)
                * \brief Constructor : create an instance of a bounding sphere.
                * \param the center of the bounding sphere.
                * \param the radius of the bounding sphere.
                */
                BoundingSphere (math::Vec3f center, float radius);
                /**\fn bool intersects (BoundingSphere &other)
                 * \brief check if the bounding sphere is in collision with an other.
                 * \param the other bounding sphere to test with.
                 * \return the result of the collision test.
                 */
                bool onIntersects (BaseInterface& _interface, CollisionResultSet::Info& info) {
                    return _interface.intersects(*this, info);
                }
                bool onIntersects (BaseInterface& _interface, math::Ray& ray, bool segment, CollisionResultSet::Info& info) {
                    return _interface.intersects(ray, segment, info);
                }
                bool onIntersects (BaseInterface& _interface, math::Ray& ray, math::Vec3f& _near, math::Vec3f& _far, CollisionResultSet::Info& info) {
                    return _interface.intersectsWhere(ray, _near, _far, info);
                }
                bool intersects (BoundingSphere &other, CollisionResultSet::Info& info);
                /** \fn bool intersects (BoundingEllipsoid &be)
                 *  \brief check if the bounding sphere is in collision with a bounding ellipsoid.
                 *  \param the bouding ellipsoid to test with.
                 *  \return the result of the collision test.
                 */
                bool intersects (BoundingEllipsoid &be, CollisionResultSet::Info& info);
                /** \fn bool intersects (BoundingBox &br)
                 *  \brief check if the bounding sphere is in collision with a bounding box.
                 *  \param the bounding box to test with.
                 *  \return the result of the collision test.
                 */
                bool intersects (BoundingBox &obr, CollisionResultSet::Info& info);
                /**\fn bool intersects (OrientedBoundingRectangle &obr)
                *  \brief check if the bounding sphere is in collision with an oriented bounding box.
                *  \param the oriented bounding box to test with.
                *  \return the result of the collision test.
                */
                bool intersects (OrientedBoundingBox &obr, CollisionResultSet::Info& info);
                /**\fn bool intersects (BoundingPolygon &bp)
                *  \brief check if the bounding sphere is in collision with a bounding polyhedron.
                *  \param the bounding polyhedron to test with.
                *  \return the result of the collision test.
                */
                bool intersects (BoundingPolyhedron &bp, CollisionResultSet::Info& info);
                /** \fn bool isPointInside (Vec2f &point)
                 *  \brief check if a point is inside the sphere.
                 *  \param the point to test in.
                 *  \return the result of the collision test.
                 */
                bool isPointInside (math::Vec3f &point);
                /** \fn bool intersects (Segment &ray)
                 *  \brief check if a ray interects the sphere.
                 *  \param the ray to test with.
                 *  \return the result of the collision test.
                 */
                bool intersects (math::Ray& ray, bool segment, CollisionResultSet::Info& info);
                /** \fn bool intersects (Segment &ray, Vec2f &near, Vec2f &far)
                 *  \brief check if a ray interects the sphere and get the points of the intersection.
                 *  \param the ray to test with.
                 *  \param the vector to store the nearest point of the intersection. (from the ray's origin)
                 *  \param the vector to store the farest point of the intersection. (from the ray"s origin)
                 *  \return the result of the collision test.
                 */
                bool intersectsWhere (math::Ray &ray, math::Vec3f &near, math::Vec3f &far, CollisionResultSet::Info& info);
                /** \fn Vec2f getCenter()
                *   \return the center of the bounding sphere.
                */
                math::Vec3f getPosition();
                math::Vec3f getCenter();
                /** \fn Vec2f getRadius()
                *   \return the radius of the bounding sphere.
                */
                math::Vec3f getSize();
                float getRadius();
                template <typename Archive>
                void vtserialize (Archive & ar) {
                    BoundingVolume::vtserialize(ar);
                    ar(center);
                    ar(radius);
                }
                void move() {
                }
                const BoundingSphere& operator= (const BoundingSphere& other) {
                    *this = other;
                    return *this;
                }
                std::unique_ptr<BoundingVolume> clone() {
                    return std::make_unique<BoundingSphere>(*this);
                }
                void move (math::Vec3f t);
                void scale (float s);
            private :
                math::Vec3f center; /** < the center of the bouding sphere */
                float radius; /** < the radius of the bouding sphere */
        };
    }
}
#endif
