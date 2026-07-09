module;
//import odfaeg.graphic.renderStates;
module odfaeg.graphic.renderStates;
import odfaeg.graphic.blendMode;
namespace odfaeg {
    namespace graphic { 
        RenderStates::RenderStates() :
            blendMode(),
            transform(),
            texture(nullptr),
            shader(nullptr)
        {
        }



        ////////////////////////////////////////////////////////////
        RenderStates::RenderStates(const entity::TransformMatrix& theTransform) :
            blendMode(),
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
            blendMode(),
            transform(),
            texture(theTexture),
            shader(nullptr)
        {
        }


        ////////////////////////////////////////////////////////////
        RenderStates::RenderStates(Shader* theShader) :
            blendMode(),
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