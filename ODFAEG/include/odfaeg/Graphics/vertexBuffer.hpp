
#ifndef ODFAEG_VERTEXBUFFER_HPP
#define ODFAEG_VERTEXBUFFER_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "vertex.h"
#include "primitiveType.hpp"
#include "drawable.h"
#include <vector>
#include "../config.hpp"
#ifdef VULKAN
#include "../Window/vkSettup.hpp"
#endif
#include "../Physics/boundingBox.h"

#include "../../../include/odfaeg/Window/iGlResource.hpp"
#include "vbo.h"
#include "vertexArray.h"
#include "drawable.h"

/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace graphic {
        #ifdef VULKAN

        class ODFAEG_GRAPHICS_API VertexBuffer : public Drawable {
        public :
            VertexBuffer(window::Device& vkDevice);

            VertexBuffer(VertexBuffer&& vb);

            VertexBuffer& operator= (VertexBuffer&& vb);
            void append(const Vertex& vertex);
            void addIndex(uint16_t index);
            size_t getIndicesSize();

            void clear();
            VkBuffer getVertexBuffer(unsigned int currentFrame);
            VkBuffer getIndexBuffer(unsigned int currentFrame);
            size_t getSize();
            size_t getVertexCount();
            void clearIndexes();
            void setPrimitiveType(PrimitiveType type);

            ////////////////////////////////////////////////////////////
            /// \brief Get the type of primitives drawn by the vertex array
            ///
            /// \return Primitive type
            ///
            ////////////////////////////////////////////////////////////
            PrimitiveType getPrimitiveType() const;
            void update();
            void update(unsigned int currentFrame, VkCommandBuffer cmd);
            void updateStagingBuffers(unsigned int currentFrame);
            Vertex& operator [](unsigned int index);
            void draw(RenderTarget& target, RenderStates states);
            void cleanup();
            ~VertexBuffer();
            std::string name;
        private :
            VertexBuffer(const VertexBuffer& vb);
            VertexBuffer& operator= (const VertexBuffer& vb);
            void createCommandPool();
            void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
            void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
            void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandBuffer cmd);
            void createIndexBuffer();
            uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
            std::vector<Vertex> m_vertices;
            window::Device& vkDevice;
            std::array<VkBuffer, MAX_FRAMES_IN_FLIGHT> vertexBuffer={}, indexBuffer={};
            std::array<VkBuffer, MAX_FRAMES_IN_FLIGHT> vertexStagingBuffer={};
            std::array<VkDeviceMemory, MAX_FRAMES_IN_FLIGHT> vertexBufferMemory={}, indexBufferMemory={};
            std::array<VkBuffer, MAX_FRAMES_IN_FLIGHT> indexStagingBuffer={};
            std::array<VkDeviceMemory, MAX_FRAMES_IN_FLIGHT> indexStagingBufferMemory={}, vertexStagingBufferMemory={};
            std::vector<uint16_t> indices;
            PrimitiveType       m_primitiveType; ///< Type of primitives to draw
            bool needToUpdateVertexBuffer, needToUpdateIndexBuffer, needToUpdateVertexStagingBuffer, needToUpdateIndexStagingBuffer;
            VkCommandPool commandPool;
            VkDeviceSize maxVerticesSize, maxIndexSize;
        };
        #else
        ////////////////////////////////////////////////////////////
        /// \brief Define a set of one or more 2D primitives
        ///
        ////////////////////////////////////////////////////////////
        class ODFAEG_GRAPHICS_API VertexBuffer : public Drawable, public window::IGLResource
        {
           public :
            struct Vector3f {
                float x, y, z;
            };
            struct Vector2f {
                float x, y;
            };
            struct GlVertex {
                Vector3f position;
                Color color;
                Vector2f texCoords;
                Vector3f normal;
                //bone indexes which will influence this vertex
                int m_BoneIDs[MAX_BONE_INFLUENCE];
                //weights from each bone
                float m_Weights[MAX_BONE_INFLUENCE];

            };
            struct LightInfos {
                math::Vec3f center;
                Color color;
            };
            ////////////////////////////////////////////////////////////
            /// \brief Default constructor
            ///
            /// Creates an empty vertex array.
            ///
            ////////////////////////////////////////////////////////////
            VertexBuffer();
            VertexBuffer(const VertexBuffer& va);
            VertexBuffer(const VertexBuffer&& va);
            VertexBuffer& operator= (const VertexBuffer& va);
            VertexBuffer& operator= (const VertexBuffer&& va);
            void addTransformId(unsigned int transformId, unsigned int vertexId);
            void addInstancedRenderingInfos(unsigned int numIndexes, unsigned int baseVertex, unsigned int baseIndice);
            void addIndex(unsigned int index);
            void update(VertexArray& va);
            void remove(VertexArray& va);
            std::vector<unsigned int> getIndexes();
            ////////////////////////////////////////////////////////////
            /// \brief Construct the vertex array with a type and an initial number of vertices
            ///
            /// \param type        Type of primitives
            /// \param vertexCount Initial number of vertices in the array
            ///
            ////////////////////////////////////////////////////////////
            explicit VertexBuffer(PrimitiveType type, unsigned int vertexCount = 0, Entity* entity = nullptr);
            Entity* getEntity();
            void setEntity(Entity* entity);
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
            void update();

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
            void append(const Vertex& vertex, unsigned int textureIndex = 0);
            void addMaterialType(unsigned int materialType);
            void addLayer(unsigned int layer);
            void addSpecular(float specularIntancity, float specularPower);
            void addLightInfos(math::Vec3f lightCenter, Color lightColor);
            void resizeVertices(unsigned int newSize);
            void resizeMaterialTypes(unsigned int newSize);
            void resizeLayers(unsigned int newSize);
            void resizeSpeculars(unsigned int newSize);
            void resizeLightInfos(unsigned int newSize);
            std::vector<Vertex>& getVertices();
            std::vector<unsigned int>& getTextureIndexes();
            std::vector<unsigned int>& getLayers();
            std::vector<unsigned int>& getMaterialTypes();
            std::vector<math::Vec2f>& getSpeculars();
            std::vector<LightInfos>& getLightInfos();
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
            bool operator== (const VertexBuffer &other);
            void updateVBOBuffer();
            void transform(TransformMatrix tm);
            template <typename Archive>
            void serialize (Archive & ar) {
                ar(m_indexes);
                ar(m_vertices);
                ar(m_primitiveType);
                ar(m_locals);
                if (ar.isInputArchive())
                    onLoad();
            }
            void onLoad() {
                updateVBOBuffer();
            }
            bool isLoop();
            void resetIndexes(std::vector<unsigned int> indexes);
             ////////////////////////////////////////////////////////////
            /// \brief Draw the vertex array to a render target
            ///
            /// \param target Render target to draw to
            /// \param states Current render states
            ///
            ////////////////////////////////////////////////////////////
            void draw(RenderTarget& target, RenderStates states);
            void computeNormals();
            void addNormal(math::Vec3f normals);
            std::string name;
            ~VertexBuffer();
        private :
            bool needToUpdateNormals;
            ////////////////////////////////////////////////////////////
            // Member data
            ////////////////////////////////////////////////////////////
            std::vector<math::Vec3f> m_normals;
            std::vector<math::Vec3f> m_locals;
            std::vector<Vertex> m_vertices;      ///< Vertices contained in the array


            PrimitiveType       m_primitiveType; ///< Type of primitives to draw
            unsigned int oldVerticesSize, oldIndexesSize, oldMaterialTypeSize, oldLayerSize, oldSpecularSize, oldLightInfosSize;
            bool needToUpdateVertexBuffer, needToUpdateIndexBuffer, needToUpdateMaterialTypeBuffer, needToUpdateLayersBuffer,
            needToUpdateSpecularBuffer, needToUpdateLightInfos;
            public :
            unsigned int vboVertexBuffer,vboNormalBuffer, vboIndexBuffer, vboTextureIndexesBuffer, vboMaterialType, vboLayer, vboSpecular, vboLightInfos;
            std::vector<unsigned int> m_numIndexes;
            std::vector<unsigned int> m_baseVertices;
            std::vector<unsigned int> m_baseIndexes;
            std::vector<unsigned int> m_indexes;
            std::vector<unsigned int> m_MaterialType;
            std::vector<unsigned int> m_layers;
            std::vector<LightInfos> m_lightInfos;
            std::vector<math::Vec2f> m_specular;

            //For bindless texturing.
            std::vector<unsigned int> m_texturesIndexes;
            std::vector<float> m_vPosX, m_vPosY, m_vPosZ, m_vPosW;
            std::vector<unsigned char> m_vcRed, m_vcBlue, m_vcGreen, m_vcAlpha;
            std::vector<unsigned int> m_ctX, m_ctY;
            unsigned int nbVerticesPerFace;
            bool loop;
            Entity* m_entity;
        };
        #endif
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

