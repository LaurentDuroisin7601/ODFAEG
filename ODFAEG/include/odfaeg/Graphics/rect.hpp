#ifndef ODFAEG_RECT_HPP
#define ODFAEG_RECT_HPP
#include <algorithm>
#include "../Math/vec4.h"
namespace odfaeg {
    namespace graphic {
        ////////////////////////////////////////////////////////////
        /// \brief Utility class for manipulating 2D axis aligned rectangles
        ///
        ////////////////////////////////////////////////////////////
        template <typename T>
        class Rect
        {
        public:

            ////////////////////////////////////////////////////////////
            /// \brief Default constructor
            ///
            /// Creates an empty rectangle (it is equivalent to calling
            /// Rect(0, 0, 0, 0)).
            ///
            ////////////////////////////////////////////////////////////
            Rect();

            ////////////////////////////////////////////////////////////
            /// \brief Construct the rectangle from its coordinates
            ///
            /// Be careful, the last two parameters are the width
            /// and height, not the right and bottom coordinates!
            ///
            /// \param rectLeft   Left coordinate of the rectangle
            /// \param rectTop    Top coordinate of the rectangle
            /// \param rectWidth  Width of the rectangle
            /// \param rectHeight Height of the rectangle
            ///
            ////////////////////////////////////////////////////////////
            Rect(T rectLeft, T rectTop, T rectWidth, T rectHeight);

            ////////////////////////////////////////////////////////////
            /// \brief Construct the rectangle from position and size
            ///
            /// Be careful, the last parameter is the size,
            /// not the bottom-right corner!
            ///
            /// \param position Position of the top-left corner of the rectangle
            /// \param size     Size of the rectangle
            ///
            ////////////////////////////////////////////////////////////
            Rect(const math::VecN<T, 2>& position, const math::VecN<T, 2>& size);

            ////////////////////////////////////////////////////////////
            /// \brief Construct the rectangle from another type of rectangle
            ///
            /// This constructor doesn't replace the copy constructor,
            /// it's called only when U != T.
            /// A call to this constructor will fail to compile if U
            /// is not convertible to T.
            ///
            /// \param rectangle Rectangle to convert
            ///
            ////////////////////////////////////////////////////////////
            template <typename U>
            explicit Rect(const Rect<U>& rectangle);

            ////////////////////////////////////////////////////////////
            /// \brief Check if a point is inside the rectangle's area
            ///
            /// This check is non-inclusive. If the point lies on the
            /// edge of the rectangle, this function will return false.
            ///
            /// \param x X coordinate of the point to test
            /// \param y Y coordinate of the point to test
            ///
            /// \return True if the point is inside, false otherwise
            ///
            /// \see intersects
            ///
            ////////////////////////////////////////////////////////////
            bool contains(T x, T y) const;

            ////////////////////////////////////////////////////////////
            /// \brief Check if a point is inside the rectangle's area
            ///
            /// This check is non-inclusive. If the point lies on the
            /// edge of the rectangle, this function will return false.
            ///
            /// \param point Point to test
            ///
            /// \return True if the point is inside, false otherwise
            ///
            /// \see intersects
            ///
            ////////////////////////////////////////////////////////////
            bool contains(const math::VecN<T, 2>& point) const;

            ////////////////////////////////////////////////////////////
            /// \brief Check the intersection between two rectangles
            ///
            /// \param rectangle Rectangle to test
            ///
            /// \return True if rectangles overlap, false otherwise
            ///
            /// \see contains
            ///
            ////////////////////////////////////////////////////////////
            bool intersects(const Rect<T>& rectangle) const;

            ////////////////////////////////////////////////////////////
            /// \brief Check the intersection between two rectangles
            ///
            /// This overload returns the overlapped rectangle in the
            /// \a intersection parameter.
            ///
            /// \param rectangle    Rectangle to test
            /// \param intersection Rectangle to be filled with the intersection
            ///
            /// \return True if rectangles overlap, false otherwise
            ///
            /// \see contains
            ///
            ////////////////////////////////////////////////////////////
            bool intersects(const Rect<T>& rectangle, Rect<T>& intersection) const;

            ////////////////////////////////////////////////////////////
            // Member data
            ////////////////////////////////////////////////////////////
            T left;   ///< Left coordinate of the rectangle
            T top;    ///< Top coordinate of the rectangle
            T width;  ///< Width of the rectangle
            T height; ///< Height of the rectangle
        };
        ////////////////////////////////////////////////////////////
        template <typename T>
        Rect<T>::Rect() :
        left  (0),
        top   (0),
        width (0),
        height(0)
        {

        }


        ////////////////////////////////////////////////////////////
        template <typename T>
        Rect<T>::Rect(T rectLeft, T rectTop, T rectWidth, T rectHeight) :
        left  (rectLeft),
        top   (rectTop),
        width (rectWidth),
        height(rectHeight)
        {

        }


        ////////////////////////////////////////////////////////////
        template <typename T>
        Rect<T>::Rect(const math::VecN<T, 2>& position, const math::VecN<T, 2>& size) :
        left  (position.x),
        top   (position.y),
        width (size.x),
        height(size.y)
        {

        }


        ////////////////////////////////////////////////////////////
        template <typename T>
        template <typename U>
        Rect<T>::Rect(const Rect<U>& rectangle) :
        left  (static_cast<T>(rectangle.left)),
        top   (static_cast<T>(rectangle.top)),
        width (static_cast<T>(rectangle.width)),
        height(static_cast<T>(rectangle.height))
        {
        }


        ////////////////////////////////////////////////////////////
        template <typename T>
        bool Rect<T>::contains(T x, T y) const
        {
            // Rectangles with negative dimensions are allowed, so we must handle them correctly

            // Compute the real min and max of the rectangle on both axes
            T minX = std::min(left, static_cast<T>(left + width));
            T maxX = std::max(left, static_cast<T>(left + width));
            T minY = std::min(top, static_cast<T>(top + height));
            T maxY = std::max(top, static_cast<T>(top + height));

            return (x >= minX) && (x < maxX) && (y >= minY) && (y < maxY);
        }


        ////////////////////////////////////////////////////////////
        template <typename T>
        bool Rect<T>::contains(const math::VecN<T, 2>& point) const
        {
            return contains(point.x, point.y);
        }


        ////////////////////////////////////////////////////////////
        template <typename T>
        bool Rect<T>::intersects(const Rect<T>& rectangle) const
        {
            Rect<T> intersection;
            return intersects(rectangle, intersection);
        }


        ////////////////////////////////////////////////////////////
        template <typename T>
        bool Rect<T>::intersects(const Rect<T>& rectangle, Rect<T>& intersection) const
        {
            // Rectangles with negative dimensions are allowed, so we must handle them correctly

            // Compute the min and max of the first rectangle on both axes
            T r1MinX = std::min(left, static_cast<T>(left + width));
            T r1MaxX = std::max(left, static_cast<T>(left + width));
            T r1MinY = std::min(top, static_cast<T>(top + height));
            T r1MaxY = std::max(top, static_cast<T>(top + height));

            // Compute the min and max of the second rectangle on both axes
            T r2MinX = std::min(rectangle.left, static_cast<T>(rectangle.left + rectangle.width));
            T r2MaxX = std::max(rectangle.left, static_cast<T>(rectangle.left + rectangle.width));
            T r2MinY = std::min(rectangle.top, static_cast<T>(rectangle.top + rectangle.height));
            T r2MaxY = std::max(rectangle.top, static_cast<T>(rectangle.top + rectangle.height));

            // Compute the intersection boundaries
            T interLeft   = std::max(r1MinX, r2MinX);
            T interTop    = std::max(r1MinY, r2MinY);
            T interRight  = std::min(r1MaxX, r2MaxX);
            T interBottom = std::min(r1MaxY, r2MaxY);

            // If the intersection is valid (positive non zero area), then there is an intersection
            if ((interLeft < interRight) && (interTop < interBottom))
            {
                intersection = Rect<T>(interLeft, interTop, interRight - interLeft, interBottom - interTop);
                return true;
            }
            else
            {
                intersection = Rect<T>(0, 0, 0, 0);
                return false;
            }
        }


        ////////////////////////////////////////////////////////////
        template <typename T>
        inline bool operator ==(const Rect<T>& left, const Rect<T>& right)
        {
            return (left.left == right.left) && (left.width == right.width) &&
                   (left.top == right.top) && (left.height == right.height);
        }


        ////////////////////////////////////////////////////////////
        template <typename T>
        inline bool operator !=(const Rect<T>& left, const Rect<T>& right)
        {
            return !(left == right);
        }
        // Create typedefs for the most common types
        typedef Rect<int>   IntRect;
        typedef Rect<float> FloatRect;
    }
}
#endif // ODFAEG_RECT_HPP
