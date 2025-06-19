
#ifndef ODFAEG_VERTEXARRAY_HPP
#define ODFAEG_VERTEXARRAY_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////

#include "vertex.h"
#include "primitiveType.hpp"
#include "drawable.h"
#include <vector>
#include "../Physics/boundingBox.h"
//#include "vbo.h"
#include "../Core/entityFactory.hpp"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace graphic {

        ////////////////////////////////////////////////////////////
        /// \brief Define a set of one or more 2D primitives
        ///
        ////////////////////////////////////////////////////////////
        class ODFAEG_GRAPHICS_API VertexArray : public Drawable
        {
           public :

            ////////////////////////////////////////////////////////////
            /// \brief Default constructor
            ///
            /// Creates an empty vertex array.
            ///
            ////////////////////////////////////////////////////////////
            VertexArray();
            VertexArray(const VertexArray& va);
            VertexArray(const VertexArray&& va);
            VertexArray& operator= (const VertexArray& va);
            VertexArray& operator= (const VertexArray&& va);
            void computeNormals();
            void addInstancedRenderingInfos(unsigned int numIndexes, unsigned int baseVertex, unsigned int baseIndice);
            void addIndex(unsigned int index);
            std::vector<unsigned int> getIndexes();
            std::vector<math::Vec3f> getLocals() {
                return m_locals;
            }
            ////////////////////////////////////////////////////////////
            /// \brief Construct the vertex array with a type and an initial number of vertices
            ///
            /// \param type        Type of primitives
            /// \param vertexCount Initial number of vertices in the array
            ///
            ////////////////////////////////////////////////////////////
            explicit VertexArray(PrimitiveType type, unsigned int vertexCount = 0, Entity* entity = nullptr, entt::entity entityId = entt::null);
            Entity* getEntity();
            entt::entity getEntityId();
            void setEntity(Entity* entity);
            void setEntityId(entt::entity id);
            ////////////////////////////////////////////////////////////
            /// \brief Return the vertex count
            ///
            /// \return Number of vertices in the array
            ///
            ////////////////////////////////////////////////////////////
            unsigned int getVertexCount() const;

            ////////////////////////////////////////////////////////////
            /// \brief Get a read-write access to a vertex by its index
            ///
            /// This function doesn't check \a index, it must be in range
            /// [0, getVertexCount() - 1]. The behaviour is undefined
            /// otherwise.
            ///
            /// \param index Index of the vertex to get
            ///
            /// \return Reference to the index-th vertex
            ///
            /// \see getVertexCount
            ///
            ////////////////////////////////////////////////////////////
            Vertex& operator [](unsigned int index);

            ////////////////////////////////////////////////////////////
            /// \brief Get a read-only access to a vertex by its index
            ///
            /// This function doesn't check \a index, it must be in range
            /// [0, getVertexCount() - 1]. The behaviour is undefined
            /// otherwise.
            ///
            /// \param index Index of the vertex to get
            ///
            /// \return Const reference to the index-th vertex
            ///
            /// \see getVertexCount
            ///
            ////////////////////////////////////////////////////////////
            const Vertex& operator [](unsigned int index) const;
            math::Vec3f getLocal(unsigned int index) const;
            void setLocal(unsigned int index, math::Vec3f v);
            std::vector<unsigned int> getBaseIndexes();
            void remove (unsigned int index);

            ////////////////////////////////////////////////////////////
            /// \brief Clear the vertex array
            ///
            /// This function removes all the vertices from the array.
            /// It doesn't deallocate the corresponding memory, so that
            /// adding new vertices after clearing doesn't involve
            /// reallocating all the memory.
            ///
            ////////////////////////////////////////////////////////////
            void clear();

            ////////////////////////////////////////////////////////////
            /// \brief Resize the vertex array
            ///
            /// If \a vertexCount is greater than the current size, the previous
            /// vertices are kept and new (default-constructed) vertices are
            /// added.
            /// If \a vertexCount is less than the current size, existing vertices
            /// are removed from the array.
            ///
            /// \param vertexCount New size of the array (number of vertices)
            ///
            ////////////////////////////////////////////////////////////
            void resize(unsigned int vertexCount);

            ////////////////////////////////////////////////////////////
            /// \brief Add a vertex to the array
            ///
            /// \param vertex Vertex to add
            ///
            ////////////////////////////////////////////////////////////
            void append(const Vertex& vertex);
            ////////////////////////////////////////////////////////////
            /// \brief Set the type of primitives to draw
            ///
            /// This function defines how the vertices must be interpreted
            /// when it's time to draw them:
            /// \li As points
            /// \li As lines
            /// \li As triangles
            /// \li As quads
            /// The default primitive type is sf::Points.
            ///
            /// \param type Type of primitive
            ///
            ////////////////////////////////////////////////////////////
            void setPrimitiveType(PrimitiveType type);

            ////////////////////////////////////////////////////////////
            /// \brief Get the type of primitives drawn by the vertex array
            ///
            /// \return Primitive type
            ///
            ////////////////////////////////////////////////////////////
            PrimitiveType getPrimitiveType() const;
            physic::BoundingBox getBounds();
            bool operator== (const VertexArray &other);
            void transform(TransformMatrix tm);
            template <typename Archive>
            void serialize (Archive & ar) {
                ////std::cout<<"read indexes"<<std::endl;
                ar(m_indexes);
                ////std::cout<<"indexes : "<<m_indexes.size()<<std::endl;
                ////std::cout<<"read vertices : "<<std::endl;
                ar(m_vertices);
                ////std::cout<<"vertices : "<<m_vertices.size()<<std::endl;
                ////std::cout<<"read primitives types"<<std::endl;
                ar(m_primitiveType);
                ////std::cout<<"primitive type : "<<m_primitiveType<<std::endl;
                ////std::cout<<"read locals"<<std::endl;
                ar(m_locals);
                ////std::cout<<"locals : "<<m_locals.size()<<std::endl;
                ////std::cout<<"read entity"<<std::endl;
                ar(m_entity);
                ////std::cout<<"entity : "<<m_entity<<std::endl;
                if (ar.isInputArchive())
                    onLoad();
            }
            void onLoad() {
                computeNormals();
            }
            bool isLoop();
            void updateNormals();
            ~VertexArray();
        private :

            ////////////////////////////////////////////////////////////
            /// \brief Draw the vertex array to a render target
            ///
            /// \param target Render target to draw to
            /// \param states Current render states
            ///
            ////////////////////////////////////////////////////////////
            void draw(RenderTarget& target, RenderStates states);


            ////////////////////////////////////////////////////////////
            // Member data
            ////////////////////////////////////////////////////////////
            std::vector<math::Vec3f> m_normals;
            std::vector<math::Vec3f> m_locals;
            std::vector<Vertex> m_vertices;      ///< Vertices contained in the array
            PrimitiveType       m_primitiveType; ///< Type of primitives to draw
            unsigned int vboVertexBuffer,vboNormalBuffer, vboIndexBuffer, oldVerticesSize;
            bool needToUpdateVBOBuffer, needToUpdateNormals;
            public :
            std::vector<unsigned int> m_numIndexes;
            std::vector<unsigned int> m_baseVertices;
            std::vector<unsigned int> m_baseIndexes;
            std::vector<unsigned int> m_indexes;
            std::vector<float> m_vPosX, m_vPosY, m_vPosZ, m_vPosW;
            std::vector<unsigned char> m_vcRed, m_vcBlue, m_vcGreen, m_vcAlpha;
            std::vector<unsigned int> m_ctX, m_ctY;
            unsigned int nbVerticesPerFace;
            bool loop;
            Entity* m_entity;
            entt::entity m_entityId;
        };
    }
} // namespace sf

#endif // ODFAEG_VERTEXARRAY_HPP


////////////////////////////////////////////////////////////
/// \class sf::VertexArray
/// \ingroup graphics
///
/// sf::VertexArray is a very simple wrapper around a dynamic
/// array of vertices and a primitives type.
///
/// It inherits sf::Drawable, but unlike other drawables it
/// is not transformable.
///
/// Example:
/// \code
/// sf::VertexArray lines(sf::LinesStrip, 4);
/// lines[0].position = math::Vec2f(10, 0);
/// lines[1].position = math::Vec2f(20, 0);
/// lines[2].position = math::Vec2f(30, 5);
/// lines[3].position = math::Vec2f(40, 2);
///
/// window.draw(lines);
/// \endcode
///
/// \see sf::Vertex
///
////////////////////////////////////////////////////////////
