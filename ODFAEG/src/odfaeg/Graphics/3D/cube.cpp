#include "../../../../include/odfaeg/Graphics/3D/cube.h"
namespace odfaeg {
    namespace graphic {
        namespace g3d {

            Cube::Cube (math::Vec3f position, float w, float h, float d, Color color, EntityFactory& factory) : GameObject(position,math::Vec3f(w, h, d),math::Vec3f(w*0.5f,h*0.5f,d*0.5f),"E_CUBE", factory) {
                m_color = color;
                //Droite.
                VertexArray va1(Triangles, 6, this);
                Vertex v1(math::Vec3f(w, 0, 0), color, math::Vec2f(0.f, 0.f));
                Vertex v2(math::Vec3f(w, 0, d), color, math::Vec2f(1.f, 0.f));
                Vertex v3(math::Vec3f(w, h, d), color, math::Vec2f(1.f, 1.f));
                Vertex v4(math::Vec3f(w, h, 0), color, math::Vec2f(0.f, 1.f));
                va1[0] = v1;
                va1[1] = v2;
                va1[2] = v3;
                va1[3] = v1;
                va1[4] = v3;
                va1[5] = v4;
                Material material1;
                material1.addTexture(nullptr, IntRect(0, 0, 0, 0));
                Face face1 (va1, material1, getTransform());
                //Gauche.
                VertexArray va2(Triangles, 6, this);
                Vertex v5(math::Vec3f(0, 0, d), color, math::Vec2f(0.f, 0.f));
                Vertex v6(math::Vec3f(0, 0, 0), color, math::Vec2f(1.f, 0.f));
                Vertex v7(math::Vec3f(0, h, 0), color, math::Vec2f(1.f, 1.f));
                Vertex v8(math::Vec3f(0, h, d), color, math::Vec2f(0.f, 1.f));
                va2[0] = v5;
                va2[1] = v6;
                va2[2] = v7;
                va2[3] = v5;
                va2[4] = v7;
                va2[5] = v8;
                Material material2;
                material2.addTexture(nullptr, IntRect(0, 0, 0, 0));
                Face face2 (va2, material2, getTransform());
                //Dessus
                VertexArray va3(Triangles, 6, this);
                Vertex v9(math::Vec3f(0, h, 0), color, math::Vec2f(0.f, 0.f));
                Vertex v10(math::Vec3f(w, h, 0), color, math::Vec2f(1.f, 0.f));
                Vertex v11(math::Vec3f(w, h, d), color, math::Vec2f(1.f, 1.f));
                Vertex v12(math::Vec3f(0, h, d), color, math::Vec2f(0.f, 1.f));
                va3[0] = v9;
                va3[1] = v10;
                va3[2] = v11;
                va3[3] = v9;
                va3[4] = v11;
                va3[5] = v12;
                Material material3;
                material3.addTexture(nullptr, IntRect(0, 0, 0, 0));
                Face face3 (va3, material3, getTransform());
                //Dessous.
                VertexArray va4(Triangles, 6, this);
                Vertex v13(math::Vec3f(0, 0, 0), color, math::Vec2f(0.f, 0.f));
                Vertex v14(math::Vec3f(0, 0, d), color, math::Vec2f(1.f, 0.f));
                Vertex v15(math::Vec3f(w, 0, d), color, math::Vec2f(1.f, 1.f));
                Vertex v16(math::Vec3f(w, 0, 0), color, math::Vec2f(0.f, 1.f));
                va4[0] = v13;
                va4[1] = v14;
                va4[2] = v15;
                va4[3] = v13;
                va4[4] = v15;
                va4[5] = v16;
                Material material4;
                material4.addTexture(nullptr, IntRect(0, 0, 0, 0));
                Face face4 (va4, material4, getTransform());
                //Devant
                VertexArray va5(Triangles, 6, this);
                Vertex v17(math::Vec3f(0, 0, d), color, math::Vec2f(0.f, 0.f));
                Vertex v18(math::Vec3f(0, h, d), color, math::Vec2f(1.f, 0.f));
                Vertex v19(math::Vec3f(w, h, d), color, math::Vec2f(1.f, 1.f));
                Vertex v20(math::Vec3f(w, 0, d), color, math::Vec2f(0.f, 1.f));
                va5[0] = v17;
                va5[1] = v18;
                va5[2] = v19;
                va5[3] = v17;
                va5[4] = v19;
                va5[5] = v20;
                Material material5;
                material5.addTexture(nullptr, IntRect(0, 0, 0, 0));
                Face face5 (va5, material5, getTransform());

                //Derrière.
                VertexArray va6(Triangles, 6, this);
                Vertex v21(math::Vec3f(0, 0, 0), color, math::Vec2f(0.f, 0.f));
                Vertex v22(math::Vec3f(w, 0, 0), color, math::Vec2f(1.f, 0.f));
                Vertex v23(math::Vec3f(w, h, 0), color, math::Vec2f(1.f, 1.f));
                Vertex v24(math::Vec3f(0, h, 0), color, math::Vec2f(0.f, 1.f));
                va6[0] = v21;
                va6[1] = v22;
                va6[2] = v23;
                va6[3] = v21;
                va6[4] = v23;
                va6[5] = v24;
                Material material6;
                material6.addTexture(nullptr, IntRect(0, 0, 0, 0));
                Face face6 (va6, material6, getTransform());

                addFace(face1);
                addFace(face2);
                addFace(face3);
                addFace(face4);
                addFace(face5);
                addFace(face6);
            }
            void Cube::setTexture(const Texture* texture) {
                for (unsigned int i = 0; i < getFaces().size(); i++) {
                    getFace(i)->getMaterial().clearTextures();
                    getFace(i)->getMaterial().addTexture(texture);
                }
            }
            void Cube::setTexCoords(IntRect texCoords) {
                for (unsigned int i = 0; i < getFaces().size(); i++) {
                    VertexArray& va = getFace(i)->getVertexArray();
                    va[0].texCoords = math::Vec2f(texCoords.left, texCoords.top);
                    va[1].texCoords = math::Vec2f(texCoords.left + texCoords.width, texCoords.top);
                    va[2].texCoords = math::Vec2f(texCoords.left + texCoords.width, texCoords.top + texCoords.height);
                    va[3].texCoords = math::Vec2f(texCoords.left, texCoords.top);
                    va[4].texCoords = math::Vec2f(texCoords.left + texCoords.width, texCoords.top + texCoords.height);
                    va[5].texCoords = math::Vec2f(texCoords.left, texCoords.top + texCoords.height);
                }
            }
            void Cube::onDraw (RenderTarget &target, RenderStates states) {
                for (unsigned int i = 0; i < getFaces().size(); i++) {
                    states.texture = const_cast<Cube*>(this)->getFace(i)->getMaterial().getTexture();
                    target.draw (getFace(i)->getVertexArray(),states);
                }
            }
            Entity* Cube::clone() {
                Cube* cube = factory.make_entity<Cube>(getPosition(), getSize().x(), getSize().y(), getSize().z(),m_color,factory);
                GameObject::copy(cube);
                cube->m_color = m_color;
                return cube;
            }
            Color Cube::getColor() {
                return m_color;
            }
        }
    }
}

