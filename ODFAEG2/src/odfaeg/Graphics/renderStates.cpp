module;
import odfaeg.graphic.renderStates;
module odfaeg.graphic.renderStates;
namespace odfaeg {
    namespace graphic {        
        RenderStates::RenderStates() :
            blendMode(BlendAlpha),
            transform(),
            texture(nullptr),
            shader(nullptr)
        {
        }



        ////////////////////////////////////////////////////////////
        RenderStates::RenderStates(const entity::TransformMatrix& theTransform) :
            blendMode(BlendAlpha),
            transform(theTransform),
            texture(nullptr),
            shader(nullptr)
        {
        }


        ////////////////////////////////////////////////////////////
        RenderStates::RenderStates(BlendMode theBlendMode) :
            blendMode(theBlendMode),
            transform(),
            texture(nullptr),
            shader(nullptr)
        {
        }


        ////////////////////////////////////////////////////////////
        RenderStates::RenderStates(const Texture* theTexture) :
            blendMode(BlendAlpha),
            transform(),
            texture(theTexture),
            shader(nullptr)
        {
        }


        ////////////////////////////////////////////////////////////
        RenderStates::RenderStates(Shader* theShader) :
            blendMode(BlendAlpha),
            transform(),
            texture(nullptr),
            shader(theShader)
        {
        }


        ////////////////////////////////////////////////////////////
        RenderStates::RenderStates(BlendMode theBlendMode, const entity::TransformMatrix& theTransform,
            const Texture* theTexture, Shader* theShader) :
            blendMode(theBlendMode),
            transform(theTransform),
            texture(theTexture),
            shader(theShader)
        {
        }
    }
}