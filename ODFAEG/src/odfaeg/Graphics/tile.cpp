#include "../../../include/odfaeg/Graphics/tile.h"
#include "../../../include/odfaeg/config.hpp"
namespace odfaeg {
    namespace graphic {

        /*G�re les diff�rents tiles, cette classe h�rite de Sprite.
        */
        //Cr�e un tile, avec une image pass�e en param�tre.

        Tile::Tile(EntityFactory& factory) : GameObject (math::Vec3f(0, 0, 0), math::Vec3f(0, 0, 0),math::Vec3f(0, 0, 0),"E_TILE", factory) {

        }
        Entity* Tile::clone() {
            Tile* t = factory.make_entity<Tile>(factory);
            GameObject::copy(t);
            return t;
        }
        Tile::Tile (const Texture *image, math::Vec3f position, math::Vec3f size, sf::IntRect subRect, EntityFactory& factory, sf::Color color, Entity *parent)
        : GameObject (position, size, size * 0.5f, "E_TILE", factory) {
            //std::cout<<"add vertex array"<<std::endl;
            #ifndef VULKAN
            VertexArray va(sf::Quads, 4, this);
            Vertex v1(sf::Vector3f(0, 0, 0), color);
            Vertex v2(sf::Vector3f(size.x, 0, 0), color);
            Vertex v3(sf::Vector3f(size.x, size.y, size.z), color);
            Vertex v4(sf::Vector3f(0, size.y, size.z), color);
            v1.texCoords = sf::Vector2f(subRect.left, subRect.top);
            v2.texCoords = sf::Vector2f(subRect.left + subRect.width, subRect.top);
            v3.texCoords = sf::Vector2f(subRect.left + subRect.width, subRect.top + subRect.height);
            v4.texCoords = sf::Vector2f(subRect.left, subRect.top + subRect.height);
            //std::cout<<"tex coords : "<<v2.texCoords.x<<" "<<v2.texCoords.y<<std::endl;
            //v1.color = v2.color = v3.color = v4.color = color;
            va[0] = v1;
            va[1] = v2;
            va[2] = v3;
            va[3] = v4;
            #else
            VertexArray va(sf::Triangles, 4, this);
            Vertex v1(sf::Vector3f(0, 0, 0), color);
            Vertex v2(sf::Vector3f(size.x, 0, 0), color);
            Vertex v3(sf::Vector3f(size.x, size.y, size.z), color);
            Vertex v4(sf::Vector3f(0, size.y, size.z), color);
            v1.texCoords = sf::Vector2f(subRect.left, subRect.top);
            v2.texCoords = sf::Vector2f(subRect.left + subRect.width, subRect.top);
            v3.texCoords = sf::Vector2f(subRect.left + subRect.width, subRect.top + subRect.height);
            v4.texCoords = sf::Vector2f(subRect.left, subRect.top + subRect.height);
            va[0] = v1;
            va[1] = v2;
            va[2] = v3;
            va[3] = v4;
            va.addIndex(0);
            va.addIndex(1);
            va.addIndex(2);
            va.addIndex(0);
            va.addIndex(2);
            va.addIndex(3);
            /*va[0] = v1;
            va[1] = v2;
            va[2] = v3;
            va[3] = v1;
            va[4] = v3;
            va[5] = v4;*/
            #endif
            Material material;
            material.addTexture(nullptr, sf::IntRect(0, 0, 0, 0));
            Face face(va, material,getTransform());
            //std::cout<<"add face"<<std::endl;
            addFace(face);
            getFace(0)->getMaterial().clearTextures();
            getFace(0)->getMaterial().addTexture(image, subRect);
            //std::cout<<"face added"<<std::endl;
        }
        void Tile::changeVerticesHeights(float h1, float h2, float h3, float h4) {
            getFace(0)->getVertexArray()[0].position.y = h1;
            getFace(0)->getVertexArray()[1].position.y = h2;
            getFace(0)->getVertexArray()[2].position.y = h3;
            getFace(0)->getVertexArray()[3].position.y = h4;
            float min, max;
            if (h1 < h2 && h1 < h3 && h1 < h4)
                min = h1;
            else if (h2 < h1 && h2 < h3 && h2 < h4)
                min = h2;
            else if (h3 < h1 && h3 < h2 && h3 < h4)
                min = h3;
            else
                min = h4;
            if (h1 > h2 && h1 > h3 && h1 > h4)
                max = h1;
            else if (h2 > h1 && h2 > h3 && h2 > h4)
                max = h2;
            else if (h3 > h1 && h3 > h2 && h3 > h4)
                max = h3;
            else
                max = h4;
            setPosition(math::Vec3f(getPosition().x, 0, getPosition().z));
            setSize(math::Vec3f(getSize().x, max-min, getSize().z));
        }
        bool Tile::operator== (Entity &other) {
            return GameObject::operator==(other);
        }

        bool Tile::operator!= (Entity &tile) {
            return !(*this == tile);
        }

        void Tile::onDraw(RenderTarget &target, RenderStates states) {
            states.texture = const_cast<Tile*>(this)->getFace(0)->getMaterial().getTexture();
            target.draw(const_cast<Tile*>(this)->getFace(0)->getVertexArray(), states);
        }

        void Tile::setColor (sf::Color color) {
            for (unsigned int j = 0; j < getFace(0)->getVertexArray().getVertexCount(); j++)
                getFace(0)->getVertexArray()[j].color = color;
        }
        sf::Color Tile::getColor() {
            return getFace(0)->getVertexArray()[0].color;
        }
        sf::IntRect Tile::getTexCoords() {
            return getFace(0)->getMaterial().getTexRect();
        }
        void Tile::setTexRect(sf::IntRect rect) {
            getFace(0)->getMaterial().setTexRect(rect);
            VertexArray& va = getFace(0)->getVertexArray();
            va[0].texCoords = sf::Vector2f(rect.left, rect.top);
            va[1].texCoords = sf::Vector2f(rect.left + rect.width, rect.top);
            va[2].texCoords = sf::Vector2f(rect.left + rect.width, rect.top + rect.height);
            va[3].texCoords = sf::Vector2f(rect.left, rect.top + rect.height);
        }
    }
}

