#ifndef ODFAEG_PRIMITIVE_TYPE_HPP
#define ODFAEG_PRIMITIVE_TYPE_HPP
namespace odfaeg {
    namespace graphic {
        ////////////////////////////////////////////////////////////
        /// \ingroup graphics
        /// \brief Types of primitives that a sf::VertexArray can render
        ///
        /// Points and lines have no area, therefore their thickness
        /// will always be 1 pixel, regardless the current transform
        /// and view.
        ///
        ////////////////////////////////////////////////////////////
        enum PrimitiveType
        {
            Points,        ///< List of individual points
            Lines,         ///< List of individual lines
            LineStrip,     ///< List of connected lines, a point uses the previous point to form a line
            Triangles,     ///< List of individual triangles
            TriangleStrip, ///< List of connected triangles, a point uses the two previous points to form a triangle
            TriangleFan,   ///< List of connected triangles, a point uses the common center and the previous point to form a triangle
            Quads,         ///< List of individual quads (deprecated, don't work with OpenGL ES)

            // Deprecated names
            LinesStrip     = LineStrip,     ///< \deprecated Use LineStrip instead
            TrianglesStrip = TriangleStrip, ///< \deprecated Use TriangleStrip instead
            TrianglesFan   = TriangleFan    ///< \deprecated Use TriangleFan instead
        };
    }
}
#endif // ODFAEG_PRIMITIVE_TYPE_HPP
