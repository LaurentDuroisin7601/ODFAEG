#ifndef ODFAEG_BLEND_MODE_HPP
#define ODFAEG_BLEND_MODE_HPP
#include "export.hpp"
#include <vector>
namespace odfaeg {
    namespace graphic {
        struct ODFAEG_GRAPHICS_API BlendMode
        {
            ////////////////////////////////////////////////////////
            /// \brief Enumeration of the blending factors
            ///
            /// The factors are mapped directly to their OpenGL equivalents,
            /// specified by glBlendFunc() or glBlendFuncSeparate().
            ////////////////////////////////////////////////////////
            enum Factor
            {
                Zero,             ///< (0, 0, 0, 0)
                One,              ///< (1, 1, 1, 1)
                SrcColor,         ///< (src.r, src.g, src.b, src.a)
                OneMinusSrcColor, ///< (1, 1, 1, 1) - (src.r, src.g, src.b, src.a)
                DstColor,         ///< (dst.r, dst.g, dst.b, dst.a)
                OneMinusDstColor, ///< (1, 1, 1, 1) - (dst.r, dst.g, dst.b, dst.a)
                SrcAlpha,         ///< (src.a, src.a, src.a, src.a)
                OneMinusSrcAlpha, ///< (1, 1, 1, 1) - (src.a, src.a, src.a, src.a)
                DstAlpha,         ///< (dst.a, dst.a, dst.a, dst.a)
                OneMinusDstAlpha  ///< (1, 1, 1, 1) - (dst.a, dst.a, dst.a, dst.a)
            };

            ////////////////////////////////////////////////////////
            /// \brief Enumeration of the blending equations
            ///
            /// The equations are mapped directly to their OpenGL equivalents,
            /// specified by glBlendEquation() or glBlendEquationSeparate().
            ////////////////////////////////////////////////////////
            enum Equation
            {
                Add,            ///< Pixel = Src * SrcFactor + Dst * DstFactor
                Subtract,       ///< Pixel = Src * SrcFactor - Dst * DstFactor
                ReverseSubtract ///< Pixel = Dst * DstFactor - Src * SrcFactor
            };

            ////////////////////////////////////////////////////////////
            /// \brief Default constructor
            ///
            /// Constructs a blending mode that does alpha blending.
            ///
            ////////////////////////////////////////////////////////////
            BlendMode();

            ////////////////////////////////////////////////////////////
            /// \brief Construct the blend mode given the factors and equation.
            ///
            /// This constructor uses the same factors and equation for both
            /// color and alpha components. It also defaults to the Add equation.
            ///
            /// \param sourceFactor      Specifies how to compute the source factor for the color and alpha channels.
            /// \param destinationFactor Specifies how to compute the destination factor for the color and alpha channels.
            /// \param blendEquation     Specifies how to combine the source and destination colors and alpha.
            ///
            ////////////////////////////////////////////////////////////
            BlendMode(Factor sourceFactor, Factor destinationFactor, Equation blendEquation = Add);

            ////////////////////////////////////////////////////////////
            /// \brief Construct the blend mode given the factors and equation.
            ///
            /// \param colorSourceFactor      Specifies how to compute the source factor for the color channels.
            /// \param colorDestinationFactor Specifies how to compute the destination factor for the color channels.
            /// \param colorBlendEquation     Specifies how to combine the source and destination colors.
            /// \param alphaSourceFactor      Specifies how to compute the source factor.
            /// \param alphaDestinationFactor Specifies how to compute the destination factor.
            /// \param alphaBlendEquation     Specifies how to combine the source and destination alphas.
            ///
            ////////////////////////////////////////////////////////////
            BlendMode(Factor colorSourceFactor, Factor colorDestinationFactor,
                      Equation colorBlendEquation, Factor alphaSourceFactor,
                      Factor alphaDestinationFactor, Equation alphaBlendEquation);
            BlendMode(const BlendMode& copy);
            ~BlendMode();
            void updateIds();

            ////////////////////////////////////////////////////////////
            // Member Data
            ////////////////////////////////////////////////////////////
            Factor   colorSrcFactor; ///< Source blending factor for the color channels
            Factor   colorDstFactor; ///< Destination blending factor for the color channels
            Equation colorEquation;  ///< Blending equation for the color channels
            Factor   alphaSrcFactor; ///< Source blending factor for the alpha channel
            Factor   alphaDstFactor; ///< Destination blending factor for the alpha channel
            Equation alphaEquation;  ///< Blending equation for the alpha channel
            unsigned int id;
            static unsigned int nbBlendModes;
        private :
            void countNbBlendMode();

            bool contains(BlendMode& blendMode);

            static std::vector<BlendMode*> blendModes;
            static std::vector<BlendMode*> sameBlendModes;
        };

        ////////////////////////////////////////////////////////////
        /// \relates BlendMode
        /// \brief Overload of the == operator
        ///
        /// \param left  Left operand
        /// \param right Right operand
        ///
        /// \return True if blending modes are equal, false if they are different
        ///
        ////////////////////////////////////////////////////////////
        ODFAEG_GRAPHICS_API bool operator ==(const BlendMode& left, const BlendMode& right);

        ////////////////////////////////////////////////////////////
        /// \relates BlendMode
        /// \brief Overload of the != operator
        ///
        /// \param left  Left operand
        /// \param right Right operand
        ///
        /// \return True if blending modes are different, false if they are equal
        ///
        ////////////////////////////////////////////////////////////
        ODFAEG_GRAPHICS_API bool operator !=(const BlendMode& left, const BlendMode& right);

        ////////////////////////////////////////////////////////////
        // Commonly used blending modes
        ////////////////////////////////////////////////////////////
        ODFAEG_GRAPHICS_API extern const BlendMode BlendAlpha;    ///< Blend source and dest according to dest alpha
        ODFAEG_GRAPHICS_API extern const BlendMode BlendAdd;      ///< Add source to dest
        ODFAEG_GRAPHICS_API extern const BlendMode BlendMultiply; ///< Multiply source and dest
        ODFAEG_GRAPHICS_API extern const BlendMode BlendNone;     ///< Overwrite dest with source
    }
}
#endif // ODFAEG_BLEND_MODE_HPP
