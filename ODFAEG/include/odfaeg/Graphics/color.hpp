#ifndef ODFAEG_COLOR_HPP
#define ODFAEG_COLOR_HPP
#include "export.hpp"
namespace odfaeg {
    namespace graphic {
        class ODFAEG_GRAPHICS_API Color
        {
        public:

            ////////////////////////////////////////////////////////////
            /// \brief Default constructor
            ///
            /// Constructs an opaque black color. It is equivalent to
            /// Color(0, 0, 0, 255).
            ///
            ////////////////////////////////////////////////////////////
            Color();

            ////////////////////////////////////////////////////////////
            /// \brief Construct the color from its 4 RGBA components
            ///
            /// \param red   Red component (in the range [0, 255])
            /// \param green Green component (in the range [0, 255])
            /// \param blue  Blue component (in the range [0, 255])
            /// \param alpha Alpha (opacity) component (in the range [0, 255])
            ///
            ////////////////////////////////////////////////////////////
            Color(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha = 255);

            ////////////////////////////////////////////////////////////
            /// \brief Construct the color from 32-bit unsigned integer
            ///
            /// \param color Number containing the RGBA components (in that order)
            ///
            ////////////////////////////////////////////////////////////
            explicit Color(std::uint32_t color);

            ////////////////////////////////////////////////////////////
            /// \brief Retrieve the color as a 32-bit unsigned integer
            ///
            /// \return Color represented as a 32-bit unsigned integer
            ///
            ////////////////////////////////////////////////////////////
            std::uint32_t toInteger() const;

            ////////////////////////////////////////////////////////////
            // Static member data
            ////////////////////////////////////////////////////////////
            static const Color Black;       ///< Black predefined color
            static const Color White;       ///< White predefined color
            static const Color Red;         ///< Red predefined color
            static const Color Green;       ///< Green predefined color
            static const Color Blue;        ///< Blue predefined color
            static const Color Yellow;      ///< Yellow predefined color
            static const Color Magenta;     ///< Magenta predefined color
            static const Color Cyan;        ///< Cyan predefined color
            static const Color Transparent; ///< Transparent (black) predefined color

            ////////////////////////////////////////////////////////////
            // Member data
            ////////////////////////////////////////////////////////////
            std::uint8_t r; ///< Red component
            std::uint8_t g; ///< Green component
            std::uint8_t b; ///< Blue component
            std::uint8_t a; ///< Alpha (opacity) component
        };

        ////////////////////////////////////////////////////////////
        /// \relates Color
        /// \brief Overload of the == operator
        ///
        /// This operator compares two colors and check if they are equal.
        ///
        /// \param left  Left operand
        /// \param right Right operand
        ///
        /// \return True if colors are equal, false if they are different
        ///
        ////////////////////////////////////////////////////////////
        ODFAEG_GRAPHICS_API bool operator ==(const Color& left, const Color& right);

        ////////////////////////////////////////////////////////////
        /// \relates Color
        /// \brief Overload of the != operator
        ///
        /// This operator compares two colors and check if they are different.
        ///
        /// \param left  Left operand
        /// \param right Right operand
        ///
        /// \return True if colors are different, false if they are equal
        ///
        ////////////////////////////////////////////////////////////
        ODFAEG_GRAPHICS_API bool operator !=(const Color& left, const Color& right);

        ////////////////////////////////////////////////////////////
        /// \relates Color
        /// \brief Overload of the binary + operator
        ///
        /// This operator returns the component-wise sum of two colors.
        /// Components that exceed 255 are clamped to 255.
        ///
        /// \param left  Left operand
        /// \param right Right operand
        ///
        /// \return Result of \a left + \a right
        ///
        ////////////////////////////////////////////////////////////
        ODFAEG_GRAPHICS_API Color operator +(const Color& left, const Color& right);

        ////////////////////////////////////////////////////////////
        /// \relates Color
        /// \brief Overload of the binary - operator
        ///
        /// This operator returns the component-wise subtraction of two colors.
        /// Components below 0 are clamped to 0.
        ///
        /// \param left  Left operand
        /// \param right Right operand
        ///
        /// \return Result of \a left - \a right
        ///
        ////////////////////////////////////////////////////////////
        ODFAEG_GRAPHICS_API Color operator -(const Color& left, const Color& right);

        ////////////////////////////////////////////////////////////
        /// \relates Color
        /// \brief Overload of the binary * operator
        ///
        /// This operator returns the component-wise multiplication
        /// (also called "modulation") of two colors.
        /// Components are then divided by 255 so that the result is
        /// still in the range [0, 255].
        ///
        /// \param left  Left operand
        /// \param right Right operand
        ///
        /// \return Result of \a left * \a right
        ///
        ////////////////////////////////////////////////////////////
        ODFAEG_GRAPHICS_API Color operator *(const Color& left, const Color& right);

        ////////////////////////////////////////////////////////////
        /// \relates Color
        /// \brief Overload of the binary += operator
        ///
        /// This operator computes the component-wise sum of two colors,
        /// and assigns the result to the left operand.
        /// Components that exceed 255 are clamped to 255.
        ///
        /// \param left  Left operand
        /// \param right Right operand
        ///
        /// \return Reference to \a left
        ///
        ////////////////////////////////////////////////////////////
        ODFAEG_GRAPHICS_API Color& operator +=(Color& left, const Color& right);

        ////////////////////////////////////////////////////////////
        /// \relates Color
        /// \brief Overload of the binary -= operator
        ///
        /// This operator computes the component-wise subtraction of two colors,
        /// and assigns the result to the left operand.
        /// Components below 0 are clamped to 0.
        ///
        /// \param left  Left operand
        /// \param right Right operand
        ///
        /// \return Reference to \a left
        ///
        ////////////////////////////////////////////////////////////
        ODFAEG_GRAPHICS_API Color& operator -=(Color& left, const Color& right);

        ////////////////////////////////////////////////////////////
        /// \relates Color
        /// \brief Overload of the binary *= operator
        ///
        /// This operator returns the component-wise multiplication
        /// (also called "modulation") of two colors, and assigns
        /// the result to the left operand.
        /// Components are then divided by 255 so that the result is
        /// still in the range [0, 255].
        ///
        /// \param left  Left operand
        /// \param right Right operand
        ///
        /// \return Reference to \a left
        ///
        ////////////////////////////////////////////////////////////
        ODFAEG_GRAPHICS_API Color& operator *=(Color& left, const Color& right);
    }
}
#endif // ODFAEG_COLOR_HPP
