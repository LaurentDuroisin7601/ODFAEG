module;
export module odfaeg.graphic.primitiveType;
export namespace odfaeg {
	namespace graphic {
        enum PrimitiveType
        {
            Points,        ///< List of individual points
            Lines,         ///< List of individual lines
            LineStrip,     ///< List of connected lines, a point uses the previous point to form a line
            Triangles,     ///< List of individual triangles
            TriangleStrip, ///< List of connected triangles, a point uses the two previous points to form a triangle
            TriangleFan   ///< List of connected triangles, a point uses the common center and the previous point to form a triangle  
        };
	};
}
