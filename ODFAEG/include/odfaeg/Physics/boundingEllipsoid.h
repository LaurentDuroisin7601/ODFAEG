#ifndef ODFAEG_BOUNDING_ELLIPSOID_HPP
#define ODFAEG_BOUNDING_ELLIPSOID_HPP
#include "../Math/vec2f.h"
#include "../Math/ray.h"
#include "boundingVolume.h"
#include "../Math/matrix4.h"
#include "../Math/triangle.h"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace physic {
        /**
         *\file boundingEllipsoid.h
         *\class BoundingEllipsoid
         *\brief manage a bounding  ellipsoid for collision detection.
         *\author Duroisin.L
         *\version 1.0
         *\date 1/02/2014
         *
         * Manage a bounding  ellipsoid for collision detection.
         *
         */
        class BoundingSphere;
        class BoundingBox;
        class OrientedBoundingBox;
        class BoundingPolyhedron;
        class BoundingEllipsoid : public BoundingVolume {
            public :
                BoundingEllipsoid() {}
                /**
                *\fn BoudingEllipsoid (Vec2f center, int a, int b, int c)
                *\brief Constructor of the class BoundingEllipsoid.
                *\param the center of the bounding ellipsoid.
                *\param the x radius of the bounding ellipsoid.
                *\param the y radius of the bounding ellipsoid.
                *\param the z radius of the bounding ellipsoid.
                */
                BoundingEllipsoid (math::Vec3f center, int a, int b, int c);
                /**
                *\fn bool intersects (BoundingSphere &bc)
                *\brief check if the bounding ellipsoid is in collision with a bounding sphere.
                *\param the bounding sphere to test with.
                */
                bool intersects (BoundingSphere &bs, CollisionResultSet::Info& info);
                /**
                *\fn bool intersects (BoundingEllipsoid &be)
                *\brief check if the bounding ellipsoid is in collision with another.
                *\param the other bounding  ellispsoid to test with.
                */
                bool intersects (BoundingEllipsoid &be, CollisionResultSet::Info& info);
                /**
                *\fn bool intersects (BoundingBox &bx)
                *\brief check if the bounding ellipsoid is in collision with a bounding box.
                *\param the bounding box to test with.
                */
                bool intersects (BoundingBox &bx, CollisionResultSet::Info& info);
                /**
                *\fn bool intersects (OrientedBoundingBox &bx)
                *\brief check if the bounding ellipsoid is in collision with an oriented bounding box.
                *\param the oriented bounding box to test with.
                */
                bool intersects (OrientedBoundingBox &obx, CollisionResultSet::Info& info);
                /**
                *\fn bool intersects (BoundingPolyhedron &bp)
                *\brief check if the oriented bounding ellipsoid is in collision with a bounding polyhedron.
                *\param the bounding polyhedron to test with.
                */
                bool intersects (BoundingPolyhedron &bp, CollisionResultSet::Info& info);
                /**
                *\fn bool intersects (math::Ray &ray)
                *\brief check if the ray is in collision with another.
                *\param the ray to test with.
                */
                bool intersects (math::Ray& ray, bool segment, CollisionResultSet::Info& info);
                /**
                *\fn bool intersects (math::Ray &ray)
                *\brief check if the ray is in collision with another and give the intersection points.
                *\param the ray to test with.
                */
                bool intersectsWhere (math::Ray& ray, math::Vec3f& near, math::Vec3f &far, CollisionResultSet::Info& info);
                /**
                *\fn bool intersects (math::Vec3f p)
                *\brief check if a point is in the bounding ellipsoid.
                *\param the point to test in.
                */
                bool isPointInside(math::Vec3f p);
                /**
                *\fn math::Vec3f getCenter ()
                *\brief give the center of the bounding ellispoid.
                *\return math::Vec3f the center of the bounding ellipsoid.
                **/
                math::Vec3f getPosition();
                math::Vec3f getCenter();
                /**
                *\fn math::Vec3f getRadius ()
                *\brief give the x, y and z radius of the bounding ellispoid.
                *\return math::Vec3f the radius of the bounding ellipsoid.
                **/
                math::Vec3f getRadius();
                math::Vec3f getSize();
                template <typename Archive>
                void vtserialize (Archive & ar) {
                    BoundingVolume::vtserialize(ar);
                    ar(radius);
                    ar(center);
                }
                const BoundingEllipsoid& operator= (const BoundingEllipsoid& other) {
                    *this = other;
                    return *this;
                }
                bool onIntersects(BaseInterface& _interface, CollisionResultSet::Info& info) {
                    return _interface.intersects(*this, info);
                }
                bool onIntersects(BaseInterface& _interface, math::Ray& ray, bool segment, CollisionResultSet::Info& info) {
                    return _interface.intersects(ray, segment, info);
                }
                bool onIntersects(BaseInterface& _interface, math::Ray& ray, math::Vec3f& _near, math::Vec3f& _far, CollisionResultSet::Info& info) {
                    return _interface.intersectsWhere(ray, _near, _far, info);
                }
                std::unique_ptr<BoundingVolume> clone() {
                    return std::make_unique<BoundingEllipsoid>(*this);
                }
                void move (math::Vec3f t);
                void setScale (math::Vec3f s);
                void rotate (float angle, math::Vec3f axis);
            private :
                math::Vec3f radius; /**< The x, y and z radius of the bounding ellipsoid.*/
                math::Vec3f center; /**< The center of the bounding ellipsoid.*/
                math::Matrix4f rotation;
                math::Matrix4f translation;
                math::Matrix4f scale;
        };
    }
}
#endif
