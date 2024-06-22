#ifndef ODFAEG_ORIENTED_BOUNDING_BOX_HPP
#define ODFAEG_ORIENTED_BOUNDING_BOX_HPP
#include "../Math/vec2f.h"
#include "../Math/ray.h"
#include "boundingVolume.h"
#include "../Math/computer.h"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace physic {
        class BoundingCircle;
        class BoundingPolygon;
        class BoundingEllipse;
        class BoundingRectangle;
        /**
          * \file orientedBoundingBox.h
          * \class OrientedBoundingBox
          * \brief Manage an oriented bounding box for collision detection
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          *
          * Manage an oriented bounding box for collision detection.
          *
          */
        class OrientedBoundingBox : public BoundingVolume {
            public :
                OrientedBoundingBox() {}
                /**\fn OrientedBoundingBox (math::Vec3f p1, math::Vec3f p2, math::Vec3f p3, math::Vec3f p4, math::Vec3f p5, math::Vec3f p6, math::Vec3f p7, math::Vec3f p8)
                 * \brief Constructor (instanciate an oriented bounding box with 8 points)
                 * \param The first point of the oriented bounding box.
                 * \param The second point of the oriented bounding box.
                 * \param The third point of the oriented bounding box.
                 * \param The fourth point of the oriented bounding box.
                 * \param The fifth point of the oriented bounding box.
                 * \param The sixth point of the oriented bounding box.
                 * \param The seventh point of the oriented bounding box.
                 * \param The heigthth point of the oriented bounding box.
                 */
                OrientedBoundingBox (math::Vec3f p1, math::Vec3f p2, math::Vec3f p3, math::Vec3f p4, math::Vec3f p5, math::Vec3f p6, math::Vec3f p7, math::Vec3f p8);
                /**\fn bool intersects (BoundingSphere &bs)
                *  \brief check if the oriented bounding box collides with a bounding sphere.
                *  \param the bounding sphere to test with.
                *  \return the result of the collision test.
                */
                OrientedBoundingBox (math::Vec3f p1, math::Vec3f p2, math::Vec3f p3, math::Vec3f p4);
                bool intersects (BoundingSphere &bs, CollisionResultSet::Info& info);
                /**\fn bool intersects (BoundingEllipsoid &be)
                *  \brief check if the oriented bounding box collides with a bounding ellipsoid.
                *  \param the bounding ellips to test with.
                *  \return the result of the collision test.
                */
                bool intersects (BoundingEllipsoid &be, CollisionResultSet::Info& info);
                /**\fn bool intersects (BoundingBox &br)
                *  \brief check if the oriented bounding box collides with a bounding box.
                *  \param the bounding box to test with.
                *  \return the result of the collision test.
                */
                bool intersects (BoundingBox &bx, CollisionResultSet::Info& info);
                /**\fn bool intersects (OrientedBoundingBox &other)
                *  \brief check if the oriented bounding box collides with another.
                *  \param the other oriented bounding box to test with.
                *  \return the result of the collision test.
                */
                bool intersects (OrientedBoundingBox &other, CollisionResultSet::Info& info);
                /**\fn bool intersects (BoundingPolyhedron &bp)
                *  \brief check if the oriented bounding box collides with a bounding polyhedrin.
                *  \param the bounding polyhedron to test with.
                *  \return the result of the collision test.
                */
                bool intersects (BoundingPolyhedron &bp, CollisionResultSet::Info& info);
                /**\fn bool intersects (Ray &ray)
                *  \brief check if the oriented bounding box collides with a segment.
                *  \param the bounding polygon to test with.
                *  \return the result of the collision test.
                */
                bool intersects (math::Ray &ray, bool segment, CollisionResultSet::Info& info);
                bool intersectsWhere(math::Ray& ray, math::Vec3f& i1, math::Vec3f& i2, CollisionResultSet::Info& info);
                /**\fn bool isPointInside (math::Vec3f point)
                *  \brief check if the point is inside the oriented bounding box.
                *  \param the point to test in.
                *  \return the result of the collision test.
                */
                bool isPointInside (math::Vec3f point);
                /**\fn void setCorners (math::Vec3f p1, math::Vec3f p2, math::Vec3f p3, math::Vec3f p4, math::Vec3f p5, math::Vec3f p6, math::Vec3f p7, math::Vec3f p8)
                * \brief set the 8 points of the oriented bounding box.
                * \param The first point of the oriented bounding box.
                * \param The second point of the oriented bounding box.
                * \param The third point of the oriented bounding box.
                * \param The fourth point of the oriented bounding box.
                * \param The fifth point of the oriented bounding box.
                * \param The sixth point of the oriented bounding box.
                * \param The seventh point of the oriented bounding box.
                * \param The heigthth point of the oriented bounding box.
                */
                void setCorners (math::Vec3f p1, math::Vec3f p2, math::Vec3f p3, math::Vec3f p4, math::Vec3f p5, math::Vec3f p6, math::Vec3f p7, math::Vec3f p8);
                /**\fn Vec2f getCenter()
                *  \return the center of the oriented bounding box.
                **/
                math::Vec3f getCenter();
                /**\fn int getWidth()
                *  \brief return the width of the oriented bounding box.
                *  \return the width of the oriented bounding box.
                */
                int getWidth();
                /**\fn int getHeight()
                *  \brief return the width of the oriented bounding box.
                *  \return the width of the oriented bounding box.
                */
                int getHeight();
                /**\fn int getDepth()
                *  \brief return the depth of the oriented bounding box.
                *  \return the depth of the oriented bounding box.
                */
                int getDepth();
                /**\fn math::Vec3f getBX()
                *  \brief return the first edge of the oriented bounding box.
                *  \return the first edge of the oriented bounding box.
                */
                math::Vec3f getSize();                
                math::Vec3f getPosition();
                /**\fn math::Vec3f getCenter()
                *  \brief return the center of the oriented bounding box
                *  \return the center of the oriented bounding box.
                */
                std::string getType() {
                    return "OrientedBoundingBox";
                }
                template <typename Archive>
                void vtserialize (Archive & ar) {
                    BoundingVolume::vtserialize(ar);
                    ar(x);
                    ar(y);
                    ar(z);
                    ar(width);
                    ar(height);
                    ar(depth);
                    ar(points);
                    ar(edgeNormals);
                    ar(edgeBissectors);
                    ar(faceNormals);
                    ar(faceBissectors);
                }
                const OrientedBoundingBox& operator= (const OrientedBoundingBox& other) {
                    *this = other;
                    return *this;
                }
                bool onIntersects(BaseInterface &_interface, CollisionResultSet::Info& info) {
                    return _interface.intersects(*this, info);
                }
                bool onIntersects(BaseInterface &_interface, math::Ray& ray, bool segment, CollisionResultSet::Info& info) {
                    return _interface.intersects(ray, segment, info);
                }
                bool onIntersects(BaseInterface& _interface, math::Ray& ray, math::Vec3f &_near, math::Vec3f& _far, CollisionResultSet::Info& info) {
                    return _interface.intersectsWhere(ray, _near, _far, info);
                }
                std::unique_ptr<BoundingVolume> clone() {
                    return std::make_unique<OrientedBoundingBox>(*this);
                }
                std::vector<math::Vec3f> getVertices() {
                    return points;
                }
                std::vector<math::Vec3f> getEdgeNormals();
                std::vector<math::Vec3f> getEdgeBissectors();
                std::vector<math::Vec3f> getFaceBissectors();
                std::vector<math::Vec3f> getFaceNormals();
                void move(math::Vec3f t);
                void scale(math::Vec3f s);
                void rotate(float angle, math::Vec3f r);
                bool isFlat();
            private :
                void computeVectors();
                int x, y, z, width, height, depth; /**< the x position of the bounding box*/
                /**< the y position of the bounding box*/
                /**< the z position of the bounding box*/
                /**< the width of the bounding box*/
                /**< the height of the bounding box*/
                /**< the depth position of the bounding box*/
                /**< the second edge of the bounding box*/
                /**< the third edge of the bounding box*/
                math::Vec3f center; /**< the center of the bounding box*/
                std::vector<math::Vec3f> points;
                std::vector<math::Vec3f> edgeNormals;
                std::vector<math::Vec3f> faceNormals;
                std::vector<math::Vec3f> edgeBissectors;
                std::vector<math::Vec3f> faceBissectors;
                bool flat;
        };
    }
}

#endif // ORIENTED_BOUNDING_RECT
