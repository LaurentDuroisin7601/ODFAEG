module;
export module odfaeg.graphic.renderStates;
import odfaeg.graphic.blendMode;
import odfaeg.math.transformMatrix;
import odfaeg.graphic.texture;
import odfaeg.graphic.shader;
namespace odfaeg {
	namespace graphic {
        export class RenderStates {
        public:

            ////////////////////////////////////////////////////////////
            /// \brief Default constructor
            ///
            /// Constructing a default set of render states is equivalent
            /// to using sf::RenderStates::Default.
            /// The default set defines:
            /// \li the BlendAlpha blend mode
            /// \li the identity transform
            /// \li a null texture
            /// \li a null shader
            ///
            ////////////////////////////////////////////////////////////
            RenderStates();

            ////////////////////////////////////////////////////////////
           /// \brief Construct a default set of render states with a custom blend mode
           ///
           /// \param theBlendMode Blend mode to use
           ///
           ////////////////////////////////////////////////////////////
            RenderStates(BlendMode theBlendMode);

            ////////////////////////////////////////////////////////////
           /// \brief Construct a default set of render states with a custom transform
           ///
           /// \param theTransform Transform to use
           ///
           ////////////////////////////////////////////////////////////
            RenderStates(const math::TransformMatrix& theTransform);

            ////////////////////////////////////////////////////////////
            /// \brief Construct a default set of render states with a custom texture
            ///
            /// \param theTexture Texture to use
            ///
            ////////////////////////////////////////////////////////////
            RenderStates(const Texture* theTexture);

            ////////////////////////////////////////////////////////////
           /// \brief Construct a default set of render states with a custom shader
           ///
           /// \param theShader Shader to use
           ///
           ////////////////////////////////////////////////////////////
            RenderStates(Shader* theShader);

            ////////////////////////////////////////////////////////////
            /// \brief Construct a set of render states with all its attributes
            ///
            /// \param theBlendMode Blend mode to use
            /// \param theTransform Transform to use
            /// \param theTexture   Texture to use
            /// \param theShader    Shader to use
            ///
            ////////////////////////////////////////////////////////////
            RenderStates(BlendMode theBlendMode, const math::TransformMatrix& theTransform,
                const Texture* theTexture, Shader* theShader);
            ////////////////////////////////////////////////////////////
            // Static member data
            ////////////////////////////////////////////////////////////
            static RenderStates Default;  ///< Special instance holding the default render

            ////////////////////////////////////////////////////////////
            // Member data
            ////////////////////////////////////////////////////////////
            BlendMode      blendMode; ///< Blending mode
            math::TransformMatrix      transform; ///< Transform
            const Texture* texture; ///< Texture
            Shader* shader; ///< Shader
        };     
        RenderStates RenderStates::Default = RenderStates();     
	}
}