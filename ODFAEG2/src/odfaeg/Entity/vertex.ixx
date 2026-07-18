module;
#include <array>
#include <iostream>
#include <odfaeg/config.hpp>
export module odfaeg.entity.vertex;
import odfaeg.entity.color;
import odfaeg.math.vec;
namespace odfaeg {
    namespace entity {
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
        };
    }
}