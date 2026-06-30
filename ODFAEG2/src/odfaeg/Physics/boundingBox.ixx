module;
#include <vector>
export module odfaeg.physic.boundingBox;
import odfaeg.math.vec;
import odfaeg.entity.transformMatrix;
namespace odfaeg {    
    namespace physic {
        /**
          * \file boundingBox.h
          * \class BoudingBox
          * \brief Manage a bounding box for collision detection
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          *
          * Manage a bounding box for collision detection.
          * The bounding box is aligned with the x and y axis.
          *
          */
        export  class BoundingBox {
        public:
            /** \fn BoundingBox()
            * \brief Default constructor (initialize a bounding retangle at position (0, 0) and with a size of (0, 0).
            */
            BoundingBox();
            /** \fn BoundingBox (int, int, int, int  int, int)
             * \brief Initialize a bounding box with the given position and the given size
             * \param the x position of the bounding box
             * \param the y position of the bounding box
             * \param the z position of the bounding box
             * \param the width of the bounding box
             * \param the height of the bounding box
             * \param the depth of the bounding box
             */
            BoundingBox(float x, float y, float z, float width, float height, float depth);
            bool intersects(BoundingBox& bx);
            bool isInside(BoundingBox& other);
            /** \fn bool isPointInside (Vec2f point)
            *   \brief test if a point is in the bounding box.
            *   \param the point to test in.
            *   \return the result of the collision test.
            */
            bool isPointInside(math::Vec3f point);
            /**\fn Vec2f getCenter()
            *  \brief gives the center of the bounding box.
            *  \return the center of the bounding box.
            */
            math::Vec3f getCenter();
            /**\fn float getWidth()
             *  \brief gives the width of the bounding box.
             *  \return the width of the bounding box.
             */
            float getWidth();
            /**\fn float getHeight()
            *  \brief gives the height of the bounding box.
            *  \return the height of the bounding box.
            */
            float getHeight();
            /**\fn float getDepth()
            *  \brief gives the depth of the bounding box.
            *  \return the depth of the bounding box.
            */
            float getDepth();
            /**\fn Vec3f getPosition()
            *  \brief gives the position of the bounding box.
            *  \return the position of the bounding box.
            */
            math::Vec3f getSize();
            math::Vec3f getPosition();
            /**\fn void setPosition(int x, int y, int z)
            *  \brief set the position of the bounding box.
            *  \param the x position of the bounding box.
            *  \param the y position of the bounding box.
            */
            void setPosition(int x, int y, int z);
            /**\fn void setSize(int width, int height, int depth)
            *  \brief set the size of the bounding box.
            *  \param the width of the bounding box.
            *  \param the height of the bounding box.
            *  \param the depth of the bounding box.
            */
            void setCenter(int x, int y, int z);
            void setSize(int width, int height, int depth);
            void move(math::Vec3f t);
            void scale(math::Vec3f s);
            /*const BoundingBox& operator= (const BoundingBox& other) {
                x = other.x;
                y = other.y;
                z = other.z;
                width = other.width;
                height = other.height;
                depth = other.depth;
                points = other.points;
                return *this;
            }*/
            BoundingBox transform(entity::TransformMatrix& tm);
            BoundingBox inverseTransform(entity::TransformMatrix& tm);
            std::vector<math::Vec3f> getVertices();
            std::vector<math::Vec3f> getFaceNormals();
            std::vector<math::Vec3f> getEdgeNormals();
            std::vector<math::Vec3f> getFaceBissectors();
            std::vector<math::Vec3f> getEdgeBissectors();
            bool isFlat();
        private:
            void computeVectors();
            std::vector<math::Vec3f> points;
            std::vector<math::Vec3f> faceNormals;
            std::vector<math::Vec3f> edgeNormals;
            std::vector<math::Vec3f> faceBissectors;
            std::vector<math::Vec3f> edgeBissectors;
            bool flat;
            float x, y, z, width, height, depth; /**< the x position of the bounding box */
            /**< the y position of the bounding box */
            /**< the z position of the bounding box */
            /**< the width of the bounding box */
            /**< the height of the bounding box */
            /**< the depth of the bounding box */
            math::Vec3f center;
            /**< the center of the bounding box */
        };
    }
}