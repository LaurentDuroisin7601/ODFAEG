////////////////////////////////////////////////////////////
//
// /!\ Important : this class is a modification of the circle shape class of the SFML
// that I've adapted for odfaeg with 3D vertices.
// Here is the license and the author of the SFML library.
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2013 Laurent Gomila (laurent.gom@gmail.com)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//
// /!\ Important : this class is a modification of the circle shape class of the SFML
// that I've adapted for odfaeg with 3D vertices.
// Here is the license and the author of the SFML library.
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2013 Laurent Gomila (laurent.gom@gmail.com)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////
#ifndef ODFAEG_RENDER_STATES_HPP
#define ODFAEG_RENDER_STATES_HPP
#include "../Math/transformMatrix.h"
#include <SFML/Graphics/BlendMode.hpp>
#include "texture.h"
#include "shader.h"
namespace odfaeg {
    namespace graphic {
        class  ODFAEG_API_EXPORT RenderStates {
        public :

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
            RenderStates(sf::BlendMode theBlendMode);

             ////////////////////////////////////////////////////////////
            /// \brief Construct a default set of render states with a custom transform
            ///
            /// \param theTransform Transform to use
            ///
            ////////////////////////////////////////////////////////////
            RenderStates(const TransformMatrix& theTransform);

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
            RenderStates(const Shader* theShader);

            ////////////////////////////////////////////////////////////
            /// \brief Construct a set of render states with all its attributes
            ///
            /// \param theBlendMode Blend mode to use
            /// \param theTransform Transform to use
            /// \param theTexture   Texture to use
            /// \param theShader    Shader to use
            ///
            ////////////////////////////////////////////////////////////
            RenderStates(sf::BlendMode theBlendMode, const TransformMatrix& theTransform,
                         const Texture* theTexture, const Shader* theShader);
            ////////////////////////////////////////////////////////////
            // Static member data
            ////////////////////////////////////////////////////////////
            static RenderStates Default;  ///< Special instance holding the default render

            ////////////////////////////////////////////////////////////
            // Member data
            ////////////////////////////////////////////////////////////
            sf::BlendMode      blendMode; ///< Blending mode
            TransformMatrix      transform; ///< Transform
            const Texture* texture; ///< Texture
            const Shader*  shader; ///< Shader
            unsigned int vboVertexID;
        };
    }
}
#endif
