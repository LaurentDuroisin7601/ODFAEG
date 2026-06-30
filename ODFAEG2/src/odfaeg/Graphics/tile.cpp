module;
#include <odfaeg/config.hpp>
#include <iostream>
import odfaeg.graphic.tile;
module odfaeg.graphic.tile;
import odfaeg.entity.impl;
import odfaeg.graphic.material;
import odfaeg.graphic.gameObject;
import odfaeg.graphic.vertexBuffer;
import odfaeg.graphic.vertex;
import odfaeg.graphic.color;
namespace odfaeg {
    namespace graphic {
         /*Gère les différents tiles, cette classe hérite de Sprite.
        */
        //Crée un tile, avec une image passée en paramètre.
        Tile::Tile() : GameObject (math::Vec3f(0.f, 0.f, 0.f), math::Vec3f(0.f, 0.f, 0.f),math::Vec3f(0.f, 0.f, 0.f),"E_TILE") {

        }
        GameObject* Tile::clone() {
            Tile* t = new Tile();
            GameObject::copy(t);
            return t;
        }
        Tile::Tile (Device& device, const Texture *texture, math::Vec3f position, math::Vec3f size, FloatRect subRect, Color color, GameObject *parent)
        : GameObject (position, size, size * 0.5f, "E_TILE", "", parent) {
            //std::cout<<"color : "<<(int)color.r<<","<<(int)color.g<<","<<(int)color.b<<","<<(int)color.a<<std::endl;
            VertexBuffer vb(device, Triangles, MAX_FRAMES_IN_FLIGHT);
            Vertex v1(math::Vec3f(0.f, 0.f, 0.f), color);
            Vertex v2(math::Vec3f(0.f, size.y(), 0.f), color);
            Vertex v3(math::Vec3f(size.x(), size.y(), size.z()), color);
            Vertex v4(math::Vec3f(size.x(), 0.f, size.z()), color);
            v1.texCoords = math::Vec2f(static_cast<float>(subRect.left), static_cast<float>(subRect.top));
            v2.texCoords = math::Vec2f(static_cast<float>(subRect.left), static_cast<float>(subRect.top + subRect.height));
            v3.texCoords = math::Vec2f(static_cast<float>(subRect.left + subRect.width), static_cast<float>(subRect.top + subRect.height));
            v4.texCoords = math::Vec2f(static_cast<float>(subRect.left+subRect.width), static_cast<float>(subRect.top));
            v1.normal = math::Vec3f(0.0f, 0.0f, 1.0f);
            v2.normal = math::Vec3f(0.0f, 0.0f, 1.0f);
            v3.normal = math::Vec3f(0.0f, 0.0f, 1.0f);
            v4.normal = math::Vec3f(0.0f, 0.0f, 1.0f);
            vb.append(v1);
            vb.append(v2);
            vb.append(v3);
            vb.append(v4);
            vb.addIndex(0);
            vb.addIndex(1);
            vb.addIndex(2);
            vb.addIndex(0);
            vb.addIndex(2);
            vb.addIndex(3);

            SubMesh subMesh(device);
            subMesh.setVertexBuffer(vb);
            subMesh.getMaterial().setTexture(texture, Material::DIFFUSE);
            addSubMesh(std::move(subMesh));
            //std::cout<<getSubMeshes().back().getVertexBuffer().getVertexCount()<<std::endl;
            /*if (texture != nullptr)
                std::cout<<"texture : "<<texture->getId()<<std::endl;*/

            texRect = subRect;
            //std::cout<<"submesh added"<<std::endl;
        }
        void Tile::changeVerticesHeights(float h1, float h2, float h3, float h4) {
            getSubMeshes()[0].getVertexBuffer()[0].position[1] = h1;
            getSubMeshes()[0].getVertexBuffer()[1].position[1] = h2;
            getSubMeshes()[0].getVertexBuffer()[2].position[1] = h3;
            getSubMeshes()[0].getVertexBuffer()[3].position[1] = h4;
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
            setPosition(math::Vec3f(getPosition().x(), 0.f, getPosition().z()));
            setSize(math::Vec3f(getSize().x(), max-min, getSize().z()));
        }
        bool Tile::operator== (GameObject &other) {
            return GameObject::operator==(other);
        }

        bool Tile::operator!= (GameObject &tile) {
            return !(*this == tile);
        }
        void Tile::setColor (Color color) {
            for (unsigned int j = 0; j < getSubMeshes()[0].getVertexBuffer().getVertexCount(); j++)
                getSubMeshes()[0].getVertexBuffer()[j].color = color;
        }
        Color Tile::getColor() {
            return getSubMeshes()[0].getVertexBuffer()[0].color;
        }
        FloatRect Tile::getTexCoords() {
            return texRect;
        }
        void Tile::setTexRect(IntRect rect) {
            VertexBuffer& vb = getSubMeshes()[0].getVertexBuffer();
            vb[0].texCoords = math::Vec2f(static_cast<float>(rect.left), static_cast<float>(rect.top));
            vb[1].texCoords = math::Vec2f(static_cast<float>(rect.left + rect.width), static_cast<float>(rect.top));
            vb[2].texCoords = math::Vec2f(static_cast<float>(rect.left + rect.width), static_cast<float>(rect.top + rect.height));
            vb[3].texCoords = math::Vec2f(static_cast<float>(rect.left), static_cast<float>(rect.top + rect.height));
        }
    }
}
