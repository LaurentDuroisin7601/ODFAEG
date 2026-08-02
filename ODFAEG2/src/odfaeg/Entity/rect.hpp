#ifndef ODFAEG_RECT_HPP
#define ODFAEG_RECT_HPP
#include <algorithm>
namespace odfaeg {
    namespace entity {
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
    }
}
#include "rect.inl"
#endif