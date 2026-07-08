module;
#include <vulkan/vulkan.hpp>
#include <array>
#include <iostream>
#include <odfaeg/config.hpp>
export module odfaeg.graphic.vertex;
import odfaeg.graphic.color;
import odfaeg.math.vec;
namespace odfaeg {
    namespace graphic {
        ////////////////////////////////////////////////////////////
        /// \brief Define a point with color and texture coordinates
        ///
        ////////////////////////////////////////////////////////////
        /*export struct Vec2f {
            float x, y;
        };
        export struct Vec3f {
            float x, y, z;
        };
        export struct VulkanVertex {
            Vec3f position;
            Color color;
            Vec2f texCoords;
            Vec3f normal;
            int m_BoneIDs[MAX_BONES_INFLUENCE];
            //weights from each bone
            float m_Weights[MAX_BONES_INFLUENCE];

            //unsigned int paddings[3];
        };*/
        export class alignas(16) Vertex {
        public:

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
            Vertex(const math::Vec3f& thePosition);

            ////////////////////////////////////////////////////////////
            /// \brief Construct the vertex from its position and color
            ///
            /// The texture coordinates are (0, 0).
            ///
            /// \param thePosition Vertex position
            /// \param theColor    Vertex color
            ///
            ////////////////////////////////////////////////////////////
            Vertex(const math::Vec3f& thePosition, const Color& theColor);

            ////////////////////////////////////////////////////////////
            /// \brief Construct the vertex from its position and texture coordinates
            ///
            /// The vertex color is white.
            ///
            /// \param thePosition  Vertex position
            /// \param theTexCoords Vertex texture coordinates
            ///
            ////////////////////////////////////////////////////////////
            Vertex(const math::Vec3f& thePosition, const math::Vec2f& theTexCoords);

            ////////////////////////////////////////////////////////////
            /// \brief Construct the vertex from its position, color and texture coordinates
            ///
            /// \param thePosition  Vertex position
            /// \param theColor     Vertex color
            /// \param theTexCoords Vertex texture coordinates
            ///
            ////////////////////////////////////////////////////////////
            Vertex(const math::Vec3f& thePosition, const Color& theColor, const math::Vec2f& theTexCoords);
            /**
            * \fn bool operator== (Vertex& other) const
            * \brief compare a vertex with another one.
            * \param other : the other vertex.
            */
            bool operator== (const Vertex& other);
            bool operator!= (const Vertex& other);
            /**
            * \fn void serialize(Archive & ar)
            * \brief serialize the vertex into the given archive.
            * \param ar the archive to serialize.
            */
            template <typename Archive>
            void vtserialize(Archive& ar) {
                ////////std::cout<<"position : "<<std::endl;
                ar(position);
                ////////std::cout<<"position z "<<position.z<<std::endl;
                ar(color.r);
                ////////std::cout<<"color r "<<color.r<<std::endl;
                ar(color.g);
                ////////std::cout<<"color g "<<color.g<<std::endl;
                ar(color.b);
                ////////std::cout<<"color b "<<color.b<<std::endl;
                ar(color.a);
                ////////std::cout<<"color a "<<color.a<<std::endl;
                ar(texCoords);
                ////////std::cout<<"tex coord y "<<texCoords.y<<std::endl;
            }
            ////////////////////////////////////////////////////////////
            // Member data
            ////////////////////////////////////////////////////////////
            alignas (16) math::Vec3f position; ///< 3D position of the vertex
            Color color; ///< Color of the vertex
            math::Vec2f texCoords; ///< Coordinates of the texture's pixel to map to the vertex
            alignas(16) math::Vec3f normal;
            //bone indexes which will influence this vertex
            int m_BoneIDs[MAX_BONES_INFLUENCE];
            //weights from each bone
            float m_Weights[MAX_BONES_INFLUENCE];
            unsigned int drawableDataId;
            static VkVertexInputBindingDescription getBindingDescription() {
                VkVertexInputBindingDescription bindingDescription{};
                bindingDescription.binding = 0;
                bindingDescription.stride = sizeof(Vertex);
                bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                return bindingDescription;
            }
            static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions() {
                std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions{};
                attributeDescriptions[0].binding = 0;
                attributeDescriptions[0].location = 0;
                attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[0].offset = offsetof(Vertex, position);

                attributeDescriptions[1].binding = 0;
                attributeDescriptions[1].location = 1;
                attributeDescriptions[1].format = VK_FORMAT_R8G8B8A8_UNORM;
                attributeDescriptions[1].offset = offsetof(Vertex, color);

                attributeDescriptions[2].binding = 0;
                attributeDescriptions[2].location = 2;
                attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
                attributeDescriptions[2].offset = offsetof(Vertex, texCoords);
                /*std::cout<<"stride : "<<offsetof(Vertex, texCoords)<<std::endl;
                int pause;
                std::cin>>pause;*/

                attributeDescriptions[3].binding = 0;
                attributeDescriptions[3].location = 3;
                attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[3].offset = offsetof(Vertex, normal);

                attributeDescriptions[4].binding = 0;
                attributeDescriptions[4].location = 4;
                attributeDescriptions[4].format = VK_FORMAT_R32_UINT;
                attributeDescriptions[4].offset = offsetof(Vertex, drawableDataId);

                return attributeDescriptions;
            }
        };
    }
}