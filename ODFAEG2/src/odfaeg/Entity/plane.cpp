module;
#include <utility>
#include <string>
//import odfaeg.graphic.plane;
module odfaeg.entity.plane;
import odfaeg.entity.vertex;
import odfaeg.entity.vertexArray;
import odfaeg.entity.primitiveType;
namespace odfaeg {
    namespace entity {
        Plane::Plane(math::Vec3f position, math::Vec3f size) : GameObject(position, size, size*0.5f, "E_PLANE") {
            VertexArray va(Triangles);
            va.resize(4, 0);
            va[0] = Vertex(math::Vec3f(0, 0, size.z()));
            va[1] = Vertex(math::Vec3f(0, 0, 0));
            va[2] = Vertex(math::Vec3f(size.x(), 0, 0));
            va[3] = Vertex(math::Vec3f(size.x(), 0, size.z()));
            va[0].normal = math::Vec3f(0, 1, 0);
            va[1].normal = math::Vec3f(0, 1, 0);
            va[2].normal = math::Vec3f(0, 1, 0);
            va[3].normal = math::Vec3f(0, 1, 0);
            va.addIndex(0);
            va.addIndex(1);
            va.addIndex(2);
            va.addIndex(0);
            va.addIndex(2);
            va.addIndex(3);
            SubMesh subMesh;
            subMesh.setVertexArray(va);
            addSubMesh(subMesh);
        }
        void Plane::setTexCoords(FloatRect texCoords) {
            for (unsigned int i = 0; i < getSubMeshesCount(); i++) {
                VertexArray& va = getSubMeshes()[i].getVertexArray();
                va[0].texCoords = math::Vec2f(texCoords.left, texCoords.top);
                va[1].texCoords = math::Vec2f(texCoords.left, texCoords.top+texCoords.height);
                va[2].texCoords = math::Vec2f(texCoords.left + texCoords.width, texCoords.top+texCoords.height);
                va[3].texCoords = math::Vec2f(texCoords.left + texCoords.width, texCoords.top);
            }
        }
        void Plane::setTexture(std::string textureId) {
            this->textureId = textureId;
            for (unsigned int i = 0; i < getSubMeshesCount(); i++) {
                getSubMeshes()[i].setTextureId(SubMesh::DIFFUSE, textureId);
            }
        }
        GameObject* Plane::clone() {
            Plane* plane = new Plane(getPosition(), getSize());
            GameObject::copy(plane);
            return plane;
        }
    }
}
