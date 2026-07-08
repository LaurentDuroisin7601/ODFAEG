module;
export module odfaeg.graphic.shape;
import odfaeg.graphic.renderTarget;
import odfaeg.graphic.transformable;
import odfaeg.graphic.texture;
import odfaeg.graphic.rect;
import odfaeg.graphic.color;
import odfaeg.math.vec;
import odfaeg.physic.boundingBox;
import odfaeg.graphic.device;
import odfaeg.graphic.renderTarget;
import odfaeg.graphic.renderStates;
import odfaeg.graphic.vertexBuffer;
namespace odfaeg {
	namespace graphic {
		export class Shape : public Drawable, public Transformable {
		public :
            
            ////////////////////////////////////////////////////////////
            /// \brief Virtual destructor
            ///
            ////////////////////////////////////////////////////////////
            virtual ~Shape();

            ////////////////////////////////////////////////////////////
            /// \brief Change the source texture of the shape
            ///
            /// The \a texture argument refers to a texture that must
            /// exist as long as the shape uses it. Indeed, the shape
            /// doesn't store its own copy of the texture, but rather keeps
            /// a pointer to the one that you passed to this function.
            /// If the source texture is destroyed and the shape tries to
            /// use it, the behaviour is undefined.
            /// \a texture can be NULL to disable texturing.
            /// If \a resetRect is true, the TextureRect property of
            /// the shape is automatically adjusted to the size of the new
            /// texture. If it is false, the texture rect is left unchanged.
            ///
            /// \param texture   New texture
            /// \param resetRect Should the texture rect be reset to the size of the new texture?
            ///
            /// \see getTexture, setTextureRect
            ///
            ////////////////////////////////////////////////////////////
            void setTexture(const Texture* texture, bool resetRect = false);

            ////////////////////////////////////////////////////////////
            /// \brief Set the sub-rectangle of the texture that the shape will display
            ///
            /// The texture rect is useful when you don't want to display
            /// the whole texture, but rather a part of it.
            /// By default, the texture rect covers the entire texture.
            ///
            /// \param rect Rectangle defining the region of the texture to display
            ///
            /// \see getTextureRect, setTexture
            ///
            ////////////////////////////////////////////////////////////
            void setTextureRect(const IntRect& rect);

            ////////////////////////////////////////////////////////////
            /// \brief Set the fill color of the shape
            ///
            /// This color is modulated (multiplied) with the shape's
            /// texture if any. It can be used to colorize the shape,
            /// or change its global opacity.
            /// You can use Color::Transparent to make the inside of
            /// the shape transparent, and have the outline alone.
            /// By default, the shape's fill color is opaque white.
            ///
            /// \param color New color of the shape
            ///
            /// \see getFillColor, setOutlineColor
            ///
            ////////////////////////////////////////////////////////////
            void setFillColor(const Color& color);

            ////////////////////////////////////////////////////////////
            /// \brief Set the outline color of the shape
            ///
            /// By default, the shape's outline color is opaque white.
            ///
            /// \param color New outline color of the shape
            ///
            /// \see getOutlineColor, setFillColor
            ///
            ////////////////////////////////////////////////////////////
            void setOutlineColor(const Color& color);

            ////////////////////////////////////////////////////////////
            /// \brief Set the thickness of the shape's outline
            ///
            /// Note that negative values are allowed (so that the outline
            /// expands towards the center of the shape), and using zero
            /// disables the outline.
            /// By default, the outline thickness is 0.
            ///
            /// \param thickness New outline thickness
            ///
            /// \see getOutlineThickness
            ///
            ////////////////////////////////////////////////////////////
            void setOutlineThickness(float thickness);

            ////////////////////////////////////////////////////////////
            /// \brief Get the source texture of the shape
            ///
            /// If the shape has no source texture, a NULL pointer is returned.
            /// The returned pointer is const, which means that you can't
            /// modify the texture when you retrieve it with this function.
            ///
            /// \return Pointer to the shape's texture
            ///
            /// \see setTexture
            ///
            ////////////////////////////////////////////////////////////
            const Texture* getTexture() const;

            ////////////////////////////////////////////////////////////
            /// \brief Get the sub-rectangle of the texture displayed by the shape
            ///
            /// \return Texture rectangle of the shape
            ///
            /// \see setTextureRect
            ///
            ////////////////////////////////////////////////////////////
            const IntRect& getTextureRect() const;

            ////////////////////////////////////////////////////////////
            /// \brief Get the fill color of the shape
            ///
            /// \return Fill color of the shape
            ///
            /// \see setFillColor
            ///
            ////////////////////////////////////////////////////////////
            const Color& getFillColor() const;

            ////////////////////////////////////////////////////////////
            /// \brief Get the outline color of the shape
            ///
            /// \return Outline color of the shape
            ///
            /// \see setOutlineColor
            ///
            ////////////////////////////////////////////////////////////
            const Color& getOutlineColor() const;

            ////////////////////////////////////////////////////////////
            /// \brief Get the outline thickness of the shape
            ///
            /// \return Outline thickness of the shape
            ///
            /// \see setOutlineThickness
            ///
            ////////////////////////////////////////////////////////////
            float getOutlineThickness() const;

            ////////////////////////////////////////////////////////////
            /// \brief Get the total number of points of the shape
            ///
            /// \return Number of points of the shape
            ///
            /// \see getPoint
            ///
            ////////////////////////////////////////////////////////////
            virtual unsigned int getPointCount() const = 0;

            ////////////////////////////////////////////////////////////
            /// \brief Get a point of the shape
            ///
            /// The result is undefined if \a index is out of the valid range.
            ///
            /// \param index Index of the point to get, in range [0 .. getPointCount() - 1]
            ///
            /// \return Index-th point of the shape
            ///
            /// \see getPointCount
            ///
            ////////////////////////////////////////////////////////////
            virtual math::Vec3f getPoint(unsigned int index) const = 0;

            
            const unsigned int& getId();
            physic::BoundingBox getLocalBounds() const;
            physic::BoundingBox& getGlobalBounds();
            void draw(RenderTarget& target, RenderStates states);
        protected:

            ////////////////////////////////////////////////////////////
            /// \brief Default constructor
            ///
            ////////////////////////////////////////////////////////////
            Shape(Device& device);

            ////////////////////////////////////////////////////////////
            /// \brief Recompute the internal geometry of the shape
            ///
            /// This function must be called by the derived class everytime
            /// the shape's points change (ie. the result of either
            /// getPointCount or getPoint is different).
            ///
            ////////////////////////////////////////////////////////////
            void update();

        private:
            ////////////////////////////////////////////////////////////
            /// \brief Update the fill vertices' color
            ///
            ////////////////////////////////////////////////////////////
            void updateFillColors();

            ////////////////////////////////////////////////////////////
            /// \brief Update the fill vertices' texture coordinates
            ///
            ////////////////////////////////////////////////////////////
            void updateTexCoords();

            ////////////////////////////////////////////////////////////
            /// \brief Update the outline vertices' position
            ///
            ////////////////////////////////////////////////////////////
            void updateOutline();

            ////////////////////////////////////////////////////////////
            /// \brief Update the outline vertices' color
            ///
            ////////////////////////////////////////////////////////////
            void updateOutlineColors();
            ////////////////////////////////////////////////////////////
            // Member data
            ////////////////////////////////////////////////////////////
            const Texture* m_texture;          ///< Texture of the shape
            IntRect    m_textureRect;      ///< Rectangle defining the area of the source texture to display
            Color          m_fillColor;        ///< Fill color
            Color          m_outlineColor;     ///< Outline color
            float          m_outlineThickness; ///< Thickness of the shape's outline
            VertexBuffer   m_vertices;         ///< Vertex array containing the fill geometry
            VertexBuffer   m_outlineVertices;  ///< Vertex array containing the outline geometry
            physic::BoundingBox           m_insideBounds;     ///< Bounding rectangle of the inside (fill)
            physic::BoundingBox           m_bounds, m_globalBounds;           ///< Bounding rectangle of the whole shape (outline + fill)
            unsigned int id;
            static unsigned int nbShapes;
		};
	}
}
