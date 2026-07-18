module;
//#include <bits/move.h>
#include <string>
//import odfaeg.graphic.cube;
module odfaeg.entity.cube;
import odfaeg.entity.vertexArray;
import odfaeg.entity.vertex;
import odfaeg.entity.gameObject;
namespace odfaeg {
    namespace entity {
        Cube::Cube (math::Vec3f position, float w, float h, float d, Color color) : GameObject(position,math::Vec3f(w, h, d),math::Vec3f(w*0.5f,h*0.5f,d*0.5f),"E_CUBE") {
            m_color = color;
            m_textureId = "";
            //Gauche.
            VertexArray va1(Triangles);
            va1.resize(4, 0);
            Vertex v1(math::Vec3f(w, 0, d), color, math::Vec2f(0.f, 0.f));
            Vertex v2(math::Vec3f(w, h, d), color, math::Vec2f(1.f, 0.f));
            Vertex v3(math::Vec3f(w, h, 0), color, math::Vec2f(1.f, 1.f));
            Vertex v4(math::Vec3f(w, 0, 0), color, math::Vec2f(0.f, 1.f));
            v1.normal = math::Vec3f(-1, 0, 0);
            v2.normal = math::Vec3f(-1, 0, 0);
            v3.normal = math::Vec3f(-1, 0, 0);
            v4.normal = math::Vec3f(-1, 0, 0);
            va1[0] = v1;
            va1[1] = v2;
            va1[2] = v3;
            va1[3] = v4;
            va1.addIndex(0);
            va1.addIndex(1);
            va1.addIndex(2);
            va1.addIndex(0);
            va1.addIndex(2);
            va1.addIndex(3);
            SubMesh face1;
            face1.setVertexArray(va1);
            //Droite.
            VertexArray va2(Triangles);
            va2.resize(4, 0);
            Vertex v5(math::Vec3f(0, 0, d), color, math::Vec2f(0.f, 0.f));
            Vertex v6(math::Vec3f(0, 0, 0), color, math::Vec2f(1.f, 0.f));
            Vertex v7(math::Vec3f(0, h, 0), color, math::Vec2f(1.f, 1.f));
            Vertex v8(math::Vec3f(0, h, d), color, math::Vec2f(0.f, 1.f));
            v5.normal = math::Vec3f(1, 0, 0);
            v6.normal = math::Vec3f(1, 0, 0);
            v7.normal = math::Vec3f(1, 0, 0);
            v8.normal = math::Vec3f(1, 0, 0);
            va2[0] = v5;
            va2[1] = v6;
            va2[2] = v7;
            va2[3] = v8;
            va2.addIndex(0);
            va2.addIndex(1);
            va2.addIndex(2);
            va2.addIndex(0);
            va2.addIndex(2);
            va2.addIndex(3);
            SubMesh face2;
            face2.setVertexArray(va2);
            //Dessus
            VertexArray va3(Triangles);
            va3.resize(4, 0);
            Vertex v9(math::Vec3f(0, h, d), color, math::Vec2f(0.f, 0.f));
            Vertex v10(math::Vec3f(0, h, 0), color, math::Vec2f(1.f, 0.f));
            Vertex v11(math::Vec3f(w, h, 0), color, math::Vec2f(1.f, 1.f));
            Vertex v12(math::Vec3f(w, h, d), color, math::Vec2f(0.f, 1.f));
            v9.normal = math::Vec3f(0, -1, 0);
            v10.normal = math::Vec3f(0, -1, 0);
            v11.normal = math::Vec3f(0, -1, 0);
            v12.normal = math::Vec3f(0, -1, 0);
            va3[0] = v9;
            va3[1] = v10;
            va3[2] = v11;
            va3[3] = v12;
            va3.addIndex(0);
            va3.addIndex(1);
            va3.addIndex(2);
            va3.addIndex(0);
            va3.addIndex(2);
            va3.addIndex(3);
            SubMesh face3;
            face3.setVertexArray(va3);
            //Dessous.
            VertexArray va4(Triangles);
            va4.resize(4, 0);
            Vertex v13(math::Vec3f(w, 0, d), color, math::Vec2f(0.f, 0.f));
            Vertex v14(math::Vec3f(w, 0, 0), color, math::Vec2f(1.f, 0.f));
            Vertex v15(math::Vec3f(0, 0, 0), color, math::Vec2f(1.f, 1.f));
            Vertex v16(math::Vec3f(0, 0, d), color, math::Vec2f(0.f, 1.f));
            v13.normal = math::Vec3f(0, 1, 0);
            v14.normal = math::Vec3f(0, 1, 0);
            v15.normal = math::Vec3f(0, 1, 0);
            v16.normal = math::Vec3f(0, 1, 0);
            va4[0] = v13;
            va4[1] = v14;
            va4[2] = v15;
            va4[3] = v16;
            va4.addIndex(0);
            va4.addIndex(1);
            va4.addIndex(2);
            va4.addIndex(0);
            va4.addIndex(2);
            va4.addIndex(3);
            SubMesh face4;
            face4.setVertexArray(va4);
            //Derrière
            VertexArray va5(Triangles);
            va5.resize(4, 0);
            Vertex v17(math::Vec3f(w, 0, 0), color, math::Vec2f(0.f, 0.f));
            Vertex v18(math::Vec3f(w, h, 0), color, math::Vec2f(1.f, 0.f));
            Vertex v19(math::Vec3f(0, h, 0), color, math::Vec2f(1.f, 1.f));
            Vertex v20(math::Vec3f(0, 0, 0), color, math::Vec2f(0.f, 1.f));
            v17.normal = math::Vec3f(0, 0, 1);
            v18.normal = math::Vec3f(0, 0, 1);
            v19.normal = math::Vec3f(0, 0, 1);
            v20.normal = math::Vec3f(0, 0, 1);
            va5[0] = v17;
            va5[1] = v18;
            va5[2] = v19;
            va5[3] = v20;
            va5.addIndex(0);
            va5.addIndex(1);
            va5.addIndex(2);
            va5.addIndex(0);
            va5.addIndex(2);
            va5.addIndex(3);
            SubMesh face5;
            face5.setVertexArray(va5);
            //Devant.
            VertexArray va6(Triangles);
            va6.resize(4, 0);
            Vertex v21(math::Vec3f(0, 0, d), color, math::Vec2f(0.f, 0.f));
            Vertex v22(math::Vec3f(0, h, d), color, math::Vec2f(1.f, 0.f));
            Vertex v23(math::Vec3f(w, h, d), color, math::Vec2f(1.f, 1.f));
            Vertex v24(math::Vec3f(w, 0, d), color, math::Vec2f(0.f, 1.f));
            v21.normal = math::Vec3f(0, 0, -1);
            v22.normal = math::Vec3f(0, 0, -1);
            v23.normal = math::Vec3f(0, 0, -1);
            v24.normal = math::Vec3f(0, 0, -1);
            va6[0] = v21;
            va6[1] = v22;
            va6[2] = v23;
            va6[3] = v24;
            va6.addIndex(0);
            va6.addIndex(1);
            va6.addIndex(2);
            va6.addIndex(0);
            va6.addIndex(2);
            va6.addIndex(3);
            SubMesh face6;
            face6.setVertexArray(va6);
            addSubMesh(face1);
            addSubMesh(face2);
            addSubMesh(face3);
            addSubMesh(face4);
            addSubMesh(face5);
            addSubMesh(face6);
        }
        void Cube::setTexture(std::string textureId) {
            for (unsigned int i = 0; i < getSubMeshesCount(); i++) {
                getSubMeshes()[i].setTextureId(SubMesh::DIFFUSE, textureId);
            }    
            m_textureId = textureId;        
        }
        void Cube::setTexCoords(FloatRect texCoords) {
            for (unsigned int i = 0; i < getSubMeshesCount(); i++) {
                VertexArray& va = getSubMeshes()[i].getVertexArray();
                //std::cout<<"submesh : "<<i<<std::endl;
                va[0].texCoords = math::Vec2f(texCoords.left, texCoords.top);
                va[1].texCoords = math::Vec2f(texCoords.left + texCoords.width, texCoords.top);
                va[2].texCoords = math::Vec2f(texCoords.left + texCoords.width, texCoords.top + texCoords.height);                
                va[3].texCoords = math::Vec2f(texCoords.left, texCoords.top + texCoords.height);
            }
            m_textRect = texCoords;
        }
        GameObject* Cube::clone() {
            Cube* cube = new Cube(getPosition(), getSize().x(), getSize().y(), getSize().z(), getColor());
            GameObject::copy(cube);
            cube->m_color = m_color;
            cube->m_textRect = m_textRect;
            cube->setTexCoords(m_textRect);
            cube->setTexture(m_textureId);
            return cube;
        }
        Color Cube::getColor() {
            return m_color;
        }
    }
}


