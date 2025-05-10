#include "../../../include/odfaeg/Graphics/sprite.h"
using namespace sf;
namespace odfaeg {
    namespace graphic {
        ////////////////////////////////////////////////////////////
        Sprite::Sprite() : Transformable(math::Vec3f(0, 0, 0), math::Vec3f(0, 0, 0), math::Vec3f(0, 0, 0)),
        m_texture    (NULL),
        m_textureRect(),
        m_vertices(Quads)
        {
        }


        ////////////////////////////////////////////////////////////
        Sprite::Sprite(const Texture& texture, math::Vec3f position, math::Vec3f size) : Transformable(position, size, size * 0.5f),
        m_texture    (NULL),
        m_textureRect(),
        m_vertices(Quads)
        {
            setTexture(texture);
            Vertex v1 (Vector3f(0, 0, 0));
            Vertex v2 (Vector3f(size.x, 0, 0));
            Vertex v3 (Vector3f(size.x, size.y, 0));
            Vertex v4 (Vector3f(0, size.y, 0));
            v1.texCoords = Vector2f(0, 0);
            v2.texCoords = Vector2f(size.x, 0);
            v3.texCoords = Vector2f(size.x, size.y);
            v4.texCoords = Vector2f(0, size.y);
            m_vertices.append(v1);
            m_vertices.append(v2);
            m_vertices.append(v3);
            m_vertices.append(v4);
        }


        ////////////////////////////////////////////////////////////
        Sprite::Sprite(const Texture& texture, math::Vec3f position, math::Vec3f size, const IntRect& rectangle) : Transformable(position, size, size * 0.5f),
        m_texture    (NULL),
        m_textureRect(),
        m_vertices(Quads)
        {
           setTexture(texture);
           Vertex v1 (Vector3f(0, 0, 0));
           Vertex v2 (Vector3f(size.x, 0, 0));
           Vertex v3 (Vector3f(size.x, size.y, 0));
           Vertex v4 (Vector3f(0, size.y, 0));
           v1.texCoords = Vector2f(rectangle.left, rectangle.top);
           v2.texCoords = Vector2f(rectangle.left + rectangle.width, rectangle.top);
           v3.texCoords = Vector2f(rectangle.left + rectangle.width, rectangle.top + rectangle.height);
           v4.texCoords = Vector2f(rectangle.left, rectangle.top + rectangle.height);
           m_vertices.append(v1);
           m_vertices.append(v2);
           m_vertices.append(v3);
           m_vertices.append(v4);
        }
        bool Sprite::operator== (const Sprite& other) const {
            return const_cast<Sprite*>(this)->getTransform() == const_cast<Sprite&>(other).getTransform() && m_vertices == other.m_vertices && m_texture == other.m_texture;
        }
        ////////////////////////////////////////////////////////////
        void Sprite::setTexture(const Texture& texture, bool resetRect)
        {
            // Recompute the texture area if requested, or if there was no valid texture & rect before
            if ((resetRect || m_texture != nullptr) && (!m_texture && (m_textureRect == sf::IntRect())))
                setTextureRect(IntRect(0, 0, texture.getSize().x, texture.getSize().y));

            // Assign the new texture
            m_texture = &texture;
        }


        ////////////////////////////////////////////////////////////
        void Sprite::setTextureRect(const IntRect& rectangle)
        {
            if (rectangle != m_textureRect)
            {
                m_textureRect = rectangle;
                updateTexCoords();
            }
        }


        ////////////////////////////////////////////////////////////
        void Sprite::setColor(const Color& color)
        {
            // Update the vertices' color
            m_vertices[0].color = color;
            m_vertices[1].color = color;
            m_vertices[2].color = color;
            m_vertices[3].color = color;
            for (unsigned int i = 0; i < m_vertices.getVertexCount(); i++) {
                m_vertices[i].color = color;
            }
        }


        ////////////////////////////////////////////////////////////
        const Texture* Sprite::getTexture() const
        {
            return m_texture;
        }


        ////////////////////////////////////////////////////////////
        const IntRect& Sprite::getTextureRect() const
        {
            return m_textureRect;
        }


        ////////////////////////////////////////////////////////////
        const Color& Sprite::getColor() const
        {
            return m_vertices[0].color;
        }
        ////////////////////////////////////////////////////////////
        void Sprite::draw(RenderTarget& target, RenderStates states)
        {
            if (m_texture)
            {

                states.transform.combine(getTransform().getMatrix());
                //m_vertices.transform(getTransform());
                states.texture = m_texture;
                target.draw(m_vertices, states);
            }
        }
        ////////////////////////////////////////////////////////////
        void Sprite::updateTexCoords()
        {
            float left   = static_cast<float>(m_textureRect.left);
            float right  = left + m_textureRect.width;
            float top    = static_cast<float>(m_textureRect.top);
            float bottom = top + m_textureRect.height;

            m_vertices[0].texCoords = Vector2f(left, top);
            m_vertices[1].texCoords = Vector2f(left, bottom);
            m_vertices[2].texCoords = Vector2f(right, bottom);
            m_vertices[3].texCoords = Vector2f(right, top);
        }
        VertexArray Sprite::getVertexArray() {
            return m_vertices;
        }
    }
}
