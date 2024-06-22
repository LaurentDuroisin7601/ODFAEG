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
#ifndef ODFAEG_VERTEX_HPP
#define ODFAEG_VERTEX_HPP
#include <SFML/Graphics/Color.hpp>
#include "../Math/vec4.h"
#include "../Math/vec2f.h"
#include "export.hpp"
#include "../config.hpp"
#ifdef VULKAN
#include <vulkan/vulkan.hpp>
#include <array>
#endif
namespace odfaeg {
    namespace graphic {
        ////////////////////////////////////////////////////////////
        /// \brief Define a point with color and texture coordinates
        ///
        ////////////////////////////////////////////////////////////
        class ODFAEG_GRAPHICS_API Vertex {
        public :

            ////////////////////////////////////////////////////////////
            /// \brief Default constructor
            ///
            ////////////////////////////////////////////////////////////
            Vertex();

            ////////////////////////////////////////////////////////////
            /// \brief Construct the vertex from its position
            ///
            /// The vertex color is white and texture coordinates are (0, 0).
            ///
            /// \param thePosition Vertex position
            ///
            ////////////////////////////////////////////////////////////
            Vertex(const sf::Vector3f& thePosition);

            ////////////////////////////////////////////////////////////
            /// \brief Construct the vertex from its position and color
            ///
            /// The texture coordinates are (0, 0).
            ///
            /// \param thePosition Vertex position
            /// \param theColor    Vertex color
            ///
            ////////////////////////////////////////////////////////////
            Vertex(const sf::Vector3f& thePosition, const sf::Color& theColor);

            ////////////////////////////////////////////////////////////
            /// \brief Construct the vertex from its position and texture coordinates
            ///
            /// The vertex color is white.
            ///
            /// \param thePosition  Vertex position
            /// \param theTexCoords Vertex texture coordinates
            ///
            ////////////////////////////////////////////////////////////
            Vertex(const sf::Vector3f& thePosition, const sf::Vector2f& theTexCoords);

            ////////////////////////////////////////////////////////////
            /// \brief Construct the vertex from its position, color and texture coordinates
            ///
            /// \param thePosition  Vertex position
            /// \param theColor     Vertex color
            /// \param theTexCoords Vertex texture coordinates
            ///
            ////////////////////////////////////////////////////////////
            Vertex(const sf::Vector3f& thePosition, const sf::Color& theColor, const sf::Vector2f& theTexCoords);
            /**
            * \fn bool operator== (Vertex& other) const
            * \brief compare a vertex with another one.
            * \param other : the other vertex.
            */
            bool operator== (const Vertex& other) const;
            bool operator!= (const Vertex& other) const;
            /**
            * \fn void serialize(Archive & ar)
            * \brief serialize the vertex into the given archive.
            * \param ar the archive to serialize.
            */
            template <typename Archive>
            void serialize(Archive & ar) {
                //std::cout<<"position : "<<std::endl;
                ar(position.x);
                //std::cout<<"position x "<<position.x<<std::endl;
                ar(position.y);
                //std::cout<<"position y "<<position.y<<std::endl;
                ar(position.z);
                //std::cout<<"position z "<<position.z<<std::endl;
                ar(color.r);
                //std::cout<<"color r "<<color.r<<std::endl;
                ar(color.g);
                //std::cout<<"color g "<<color.g<<std::endl;
                ar(color.b);
                //std::cout<<"color b "<<color.b<<std::endl;
                ar(color.a);
                //std::cout<<"color a "<<color.a<<std::endl;
                ar(texCoords.x);
                //std::cout<<"tex coord x "<<texCoords.x<<std::endl;
                ar(texCoords.y);
                //std::cout<<"tex coord y "<<texCoords.y<<std::endl;
            }
            ////////////////////////////////////////////////////////////
            // Member data
            ////////////////////////////////////////////////////////////
            sf::Vector3f position; ///< 3D position of the vertex
            sf::Color color; ///< Color of the vertex
            sf::Vector2f texCoords; ///< Coordinates of the texture's pixel to map to the vertex
            #ifdef VULKAN
            static VkVertexInputBindingDescription getBindingDescription() {
                VkVertexInputBindingDescription bindingDescription{};
                bindingDescription.binding = 0;
                bindingDescription.stride = sizeof(Vertex);
                bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                return bindingDescription;
            }
            static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
                std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
                attributeDescriptions[0].binding = 0;
                attributeDescriptions[0].location = 0;
                attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
                attributeDescriptions[0].offset = offsetof(Vertex, position);

                attributeDescriptions[1].binding = 0;
                attributeDescriptions[1].location = 1;
                attributeDescriptions[1].format = VK_FORMAT_R8G8B8A8_UNORM;
                attributeDescriptions[1].offset = offsetof(Vertex, color);
                return attributeDescriptions;
            }
            #endif
        };
    }
}
#endif // VERTEX_3D
