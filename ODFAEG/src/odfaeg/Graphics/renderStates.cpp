
#include "../../../include/odfaeg/Graphics/renderStates.h"
#include <cstddef>
namespace odfaeg {
    namespace graphic {
        using namespace sf;
        RenderStates RenderStates::Default;
        RenderStates::RenderStates() :
        blendMode(BlendAlpha),
        transform(),
        texture  (nullptr),
        shader   (nullptr)
        {
        }



        ////////////////////////////////////////////////////////////
        RenderStates::RenderStates(const TransformMatrix& theTransform) :
        blendMode(BlendAlpha),
        transform(theTransform),
        texture  (nullptr),
        shader   (nullptr)
        {
        }


        ////////////////////////////////////////////////////////////
        RenderStates::RenderStates(BlendMode theBlendMode) :
        blendMode(theBlendMode),
        transform(),
        texture  (nullptr),
        shader   (nullptr)
        {
        }


        ////////////////////////////////////////////////////////////
        RenderStates::RenderStates(const Texture* theTexture) :
        blendMode(BlendAlpha),
        transform(),
        texture  (theTexture),
        shader   (nullptr)
        {
        }


        ////////////////////////////////////////////////////////////
        RenderStates::RenderStates(const Shader* theShader) :
        blendMode(BlendAlpha),
        transform(),
        texture  (nullptr),
        shader   (theShader)
        {
        }


        ////////////////////////////////////////////////////////////
        RenderStates::RenderStates(BlendMode theBlendMode, const TransformMatrix& theTransform,
                                   const Texture* theTexture, const Shader* theShader) :
        blendMode(theBlendMode),
        transform(theTransform),
        texture  (theTexture),
        shader   (theShader)
        {
        }
    }
}

