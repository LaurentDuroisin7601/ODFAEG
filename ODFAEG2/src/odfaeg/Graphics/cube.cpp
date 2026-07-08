module;
//#include <bits/move.h>
#include <iostream>
//import odfaeg.graphic.cube;
module odfaeg.graphic.cube;
import odfaeg.graphic.vertexBuffer;
import odfaeg.graphic.gpuContext;
import odfaeg.graphic.vertex;
import odfaeg.graphic.gameObject;
import odfaeg.graphic.material;
namespace odfaeg {
    namespace graphic {
        Cube::Cube (math::Vec3f position, float w, float h, float d, Color color) : GameObject(position,math::Vec3f(w, h, d),math::Vec3f(w*0.5f,h*0.5f,d*0.5f),"E_CUBE") {
            m_color = color;
            m_texture = nullptr;
            //Gauche.
            VertexBuffer vb1(GPUContext::instance().getDevice(), Triangles);
            vb1.resize(4, 0);
            Vertex v1(math::Vec3f(w, 0, d), color, math::Vec2f(0.f, 0.f));
            Vertex v2(math::Vec3f(w, h, d), color, math::Vec2f(1.f, 0.f));
            Vertex v3(math::Vec3f(w, h, 0), color, math::Vec2f(1.f, 1.f));
            Vertex v4(math::Vec3f(w, 0, 0), color, math::Vec2f(0.f, 1.f));
            v1.normal = math::Vec3f(-1, 0, 0);
            v2.normal = math::Vec3f(-1, 0, 0);
            v3.normal = math::Vec3f(-1, 0, 0);
            v4.normal = math::Vec3f(-1, 0, 0);
            vb1[0] = v1;
            vb1[1] = v2;
            vb1[2] = v3;
            vb1[3] = v4;
            vb1.addIndex(0);
            vb1.addIndex(1);
            vb1.addIndex(2);
            vb1.addIndex(0);
            vb1.addIndex(2);
            vb1.addIndex(3);
            SubMesh face1 (GPUContext::instance().getDevice());
            face1.setVertexBuffer(vb1);
            //Droite.
            VertexBuffer vb2(GPUContext::instance().getDevice(), Triangles);
            vb2.resize(4, 0);
            Vertex v5(math::Vec3f(0, 0, d), color, math::Vec2f(0.f, 0.f));
            Vertex v6(math::Vec3f(0, 0, 0), color, math::Vec2f(1.f, 0.f));
            Vertex v7(math::Vec3f(0, h, 0), color, math::Vec2f(1.f, 1.f));
            Vertex v8(math::Vec3f(0, h, d), color, math::Vec2f(0.f, 1.f));
            v5.normal = math::Vec3f(1, 0, 0);
            v6.normal = math::Vec3f(1, 0, 0);
            v7.normal = math::Vec3f(1, 0, 0);
            v8.normal = math::Vec3f(1, 0, 0);
            vb2[0] = v5;
            vb2[1] = v6;
            vb2[2] = v7;
            vb2[3] = v8;
            vb2.addIndex(0);
            vb2.addIndex(1);
            vb2.addIndex(2);
            vb2.addIndex(0);
            vb2.addIndex(2);
            vb2.addIndex(3);
            SubMesh face2 (GPUContext::instance().getDevice());
            face2.setVertexBuffer(vb2);
            //Dessus
            VertexBuffer vb3(GPUContext::instance().getDevice(), Triangles);
            vb3.resize(4, 0);
            Vertex v9(math::Vec3f(0, h, d), color, math::Vec2f(0.f, 0.f));
            Vertex v10(math::Vec3f(0, h, 0), color, math::Vec2f(1.f, 0.f));
            Vertex v11(math::Vec3f(w, h, 0), color, math::Vec2f(1.f, 1.f));
            Vertex v12(math::Vec3f(w, h, d), color, math::Vec2f(0.f, 1.f));
            v9.normal = math::Vec3f(0, -1, 0);
            v10.normal = math::Vec3f(0, -1, 0);
            v11.normal = math::Vec3f(0, -1, 0);
            v12.normal = math::Vec3f(0, -1, 0);
            vb3[0] = v9;
            vb3[1] = v10;
            vb3[2] = v11;
            vb3[3] = v12;
            vb3.addIndex(0);
            vb3.addIndex(1);
            vb3.addIndex(2);
            vb3.addIndex(0);
            vb3.addIndex(2);
            vb3.addIndex(3);
            SubMesh face3 (GPUContext::instance().getDevice());
            face3.setVertexBuffer(vb3);
            //Dessous.
            VertexBuffer vb4(GPUContext::instance().getDevice(), Triangles);
            vb4.resize(4, 0);
            Vertex v13(math::Vec3f(w, 0, d), color, math::Vec2f(0.f, 0.f));
            Vertex v14(math::Vec3f(w, 0, 0), color, math::Vec2f(1.f, 0.f));
            Vertex v15(math::Vec3f(0, 0, 0), color, math::Vec2f(1.f, 1.f));
            Vertex v16(math::Vec3f(0, 0, d), color, math::Vec2f(0.f, 1.f));
            v13.normal = math::Vec3f(0, 1, 0);
            v14.normal = math::Vec3f(0, 1, 0);
            v15.normal = math::Vec3f(0, 1, 0);
            v16.normal = math::Vec3f(0, 1, 0);
            vb4[0] = v13;
            vb4[1] = v14;
            vb4[2] = v15;
            vb4[3] = v16;
            vb4.addIndex(0);
            vb4.addIndex(1);
            vb4.addIndex(2);
            vb4.addIndex(0);
            vb4.addIndex(2);
            vb4.addIndex(3);
            SubMesh face4 (GPUContext::instance().getDevice());
            face4.setVertexBuffer(vb4);
            //Derrière
            VertexBuffer vb5(GPUContext::instance().getDevice(), Triangles);
            vb5.resize(4, 0);
            Vertex v17(math::Vec3f(w, 0, 0), color, math::Vec2f(0.f, 0.f));
            Vertex v18(math::Vec3f(w, h, 0), color, math::Vec2f(1.f, 0.f));
            Vertex v19(math::Vec3f(0, h, 0), color, math::Vec2f(1.f, 1.f));
            Vertex v20(math::Vec3f(0, 0, 0), color, math::Vec2f(0.f, 1.f));
            v17.normal = math::Vec3f(0, 0, 1);
            v18.normal = math::Vec3f(0, 0, 1);
            v19.normal = math::Vec3f(0, 0, 1);
            v20.normal = math::Vec3f(0, 0, 1);
            vb5[0] = v17;
            vb5[1] = v18;
            vb5[2] = v19;
            vb5[3] = v20;
            vb5.addIndex(0);
            vb5.addIndex(1);
            vb5.addIndex(2);
            vb5.addIndex(0);
            vb5.addIndex(2);
            vb5.addIndex(3);
            SubMesh face5 (GPUContext::instance().getDevice());
            face5.setVertexBuffer(vb5);
            //Devant.
            VertexBuffer vb6(GPUContext::instance().getDevice(), Triangles);
            vb6.resize(4, 0);
            Vertex v21(math::Vec3f(0, 0, d), color, math::Vec2f(0.f, 0.f));
            Vertex v22(math::Vec3f(0, h, d), color, math::Vec2f(1.f, 0.f));
            Vertex v23(math::Vec3f(w, h, d), color, math::Vec2f(1.f, 1.f));
            Vertex v24(math::Vec3f(w, 0, d), color, math::Vec2f(0.f, 1.f));
            v21.normal = math::Vec3f(0, 0, -1);
            v22.normal = math::Vec3f(0, 0, -1);
            v23.normal = math::Vec3f(0, 0, -1);
            v24.normal = math::Vec3f(0, 0, -1);
            vb6[0] = v21;
            vb6[1] = v22;
            vb6[2] = v23;
            vb6[3] = v24;
            vb6.addIndex(0);
            vb6.addIndex(1);
            vb6.addIndex(2);
            vb6.addIndex(0);
            vb6.addIndex(2);
            vb6.addIndex(3);
            SubMesh face6 (GPUContext::instance().getDevice());
            face6.setVertexBuffer(vb6);
            addSubMesh(std::move(face1));
            addSubMesh(std::move(face2));
            addSubMesh(std::move(face3));
            addSubMesh(std::move(face4));
            addSubMesh(std::move(face5));
            addSubMesh(std::move(face6));
        }
        void Cube::setTexture(const Texture* texture) {
            for (unsigned int i = 0; i < getSubMeshesCount(); i++) {
                getSubMeshes()[i].getMaterial().setTexture(texture, Material::DIFFUSE);
            }
            m_texture = const_cast<Texture*>(texture);
        }
        void Cube::setTexCoords(FloatRect texCoords) {
            for (unsigned int i = 0; i < getSubMeshesCount(); i++) {
                VertexBuffer& vb = getSubMeshes()[i].getVertexBuffer();
                //std::cout<<"submesh : "<<i<<std::endl;
                vb[0].texCoords = math::Vec2f(texCoords.left, texCoords.top);
                vb[1].texCoords = math::Vec2f(texCoords.left + texCoords.width, texCoords.top);
                vb[2].texCoords = math::Vec2f(texCoords.left + texCoords.width, texCoords.top + texCoords.height);                
                vb[3].texCoords = math::Vec2f(texCoords.left, texCoords.top + texCoords.height);
            }
            m_textRect = texCoords;
        }
        GameObject* Cube::clone() {
            Cube* cube = new Cube(getPosition(), getSize().x(), getSize().y(), getSize().z(), getColor());
            GameObject::copy(cube);
            cube->m_color = m_color;
            cube->m_textRect = m_textRect;
            cube->m_texture = m_texture;
            if (m_texture != nullptr) {
                cube->setTexCoords(m_textRect);
                cube->setTexture(m_texture);
            }
            return cube;
        }
        Color Cube::getColor() {
            return m_color;
        }
    }
}


