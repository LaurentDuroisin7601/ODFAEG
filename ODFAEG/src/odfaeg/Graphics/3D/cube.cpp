#include "../../../../include/odfaeg/Graphics/3D/cube.h"
namespace odfaeg {
    namespace graphic {
        namespace g3d {
            using namespace sf;
            Cube::Cube (math::Vec3f position, float w, float h, float d, Color color, EntityFactory& factory) : GameObject(position,math::Vec3f(w, h, d),math::Vec3f(w*0.5f,h*0.5f,d*0.5f),"E_CUBE", factory) {
                    m_color = color;
                    VertexArray va1(sf::Quads, 4, this);
                    //Devant
                    Vertex v1(Vector3f(position.x, position.y + d, position.z), color);
                    Vertex v2(Vector3f(position.x, position.y + d, position.z + h), color);
                    Vertex v3(Vector3f(position.x + w, position.y + d, position.z + h), color);
                    Vertex v4(Vector3f(position.x + w, position.y + d, position.z), color);
                    va1[0] = v1;
                    va1[1] = v2;
                    va1[2] = v3;
                    va1[3] = v4;
                    Material material1;
                    material1.addTexture(nullptr, sf::IntRect(0, 0, 0, 0));
                    Face face1 (va1, material1, getTransform());
                    //Dessus
                    VertexArray va2(sf::Quads, 4, this);
                    Vertex v5(Vector3f(position.x, position.y, position.z + h), color);
                    Vertex v6(Vector3f(position.x + w, position.y, position.z + h), color);
                    Vertex v7(Vector3f(position.x + w, position.y + d, position.z + h), color);
                    Vertex v8(Vector3f(position.x, position.y + d, position.z + h), color);
                    va2[0] = v5;
                    va2[1] = v6;
                    va2[2] = v7;
                    va2[3] = v8;
                    Material material2;
                    material2.addTexture(nullptr, sf::IntRect(0, 0, 0, 0));
                    Face face2 (va2, material2, getTransform());
                    //Derrière.
                    VertexArray va3(sf::Quads, 4, this);
                    Vertex v9(Vector3f(position.x, position.y, position.z), color);
                    Vertex v10(Vector3f(position.x, position.y, position.z + h), color);
                    Vertex v11(Vector3f(position.x + w, position.y, position.z + h), color);
                    Vertex v12(Vector3f(position.x + w, position.y, position.z), color);
                    va3[0] = v9;
                    va3[1] = v10;
                    va3[2] = v11;
                    va3[3] = v12;
                    Material material3;
                    material3.addTexture(nullptr, sf::IntRect(0, 0, 0, 0));
                    Face face3 (va3, material3, getTransform());
                    //Dessous.
                    VertexArray va4(sf::Quads, 4, this);
                    Vertex v13(Vector3f(position.x, position.y, position.z), color);
                    Vertex v14(Vector3f(position.x + w, position.y, position.z), color);
                    Vertex v15(Vector3f(position.x + w, position.y + d, position.z), color);
                    Vertex v16(Vector3f(position.x, position.y + d, position.z), color);
                    va3[0] = v13;
                    va3[1] = v14;
                    va3[2] = v15;
                    va3[3] = v16;
                    Material material4;
                    material4.addTexture(nullptr, sf::IntRect(0, 0, 0, 0));
                    Face face4 (va4, material4, getTransform());
                    //Gauche.
                    VertexArray va5(sf::Quads, 4, this);
                    Vertex v17(Vector3f(position.x, position.y, position.z), color);
                    Vertex v18(Vector3f(position.x, position.y, position.z + h), color);
                    Vertex v19(Vector3f(position.x, position.y + d, position.z + h), color);
                    Vertex v20(Vector3f(position.x, position.y + d, position.z), color);
                    va5[0] = v17;
                    va5[1] = v18;
                    va5[2] = v19;
                    va5[3] = v20;
                    Material material5;
                    material5.addTexture(nullptr, sf::IntRect(0, 0, 0, 0));
                    Face face5 (va5, material5, getTransform());
                    //Droite.
                    VertexArray va6(sf::Quads, 4, this);
                    Vertex v21(Vector3f(position.x + w, position.y, position.z), color);
                    Vertex v22(Vector3f(position.x + w, position.y, position.z + h), color);
                    Vertex v23(Vector3f(position.x + w, position.y + d, position.z + h), color);
                    Vertex v24(Vector3f(position.x + w, position.y + d, position.z), color);
                    va6[0] = v21;
                    va6[1] = v22;
                    va6[2] = v23;
                    va6[3] = v24;
                    Material material6;
                    material6.addTexture(nullptr, sf::IntRect(0, 0, 0, 0));
                    Face face6 (va6, material6, getTransform());
                    addFace(face1);
                    addFace(face2);
                    addFace(face3);
                    addFace(face4);
                    addFace(face5);
                    addFace(face6);
            }
            void Cube::setTexCoords(int face, sf::IntRect texCoords) {
                VertexArray& va = getFace(face)->getVertexArray();
                va[0].texCoords = sf::Vector2f(texCoords.left, texCoords.top);
                va[1].texCoords = sf::Vector2f(texCoords.left + texCoords.width, texCoords.top);
                va[2].texCoords = sf::Vector2f(texCoords.left + texCoords.width, texCoords.top + texCoords.height);
                va[3].texCoords = sf::Vector2f(texCoords.left, texCoords.top + texCoords.height);
            }
            void Cube::onDraw (RenderTarget &target, RenderStates states) {
                for (unsigned int i = 0; i < getFaces().size(); i++) {
                    states.texture = const_cast<Cube*>(this)->getFace(i)->getMaterial().getTexture();
                    target.draw (getFace(i)->getVertexArray(),states);
                }
            }
            Entity* Cube::clone() {
                Cube* cube = factory.make_entity<Cube>(getPosition(), getSize().x, getSize().y, getSize().z,m_color,factory);
                GameObject::copy(cube);
                cube->m_color = m_color;
                return cube;
            }
            sf::Color Cube::getColor() {
                return m_color;
            }
        }
    }
}

