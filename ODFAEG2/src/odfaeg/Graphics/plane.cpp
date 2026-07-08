module;
#include <utility>
//import odfaeg.graphic.plane;
module odfaeg.graphic.plane;
import odfaeg.graphic.vertex;
import odfaeg.graphic.vertexBuffer;
import odfaeg.graphic.gpuContext;
import odfaeg.graphic.primitiveType;
import odfaeg.graphic.material;
namespace odfaeg {
    namespace graphic {
        Plane::Plane(math::Vec3f position, math::Vec3f size) : GameObject(position, size, size*0.5f, "E_PLANE") {
            VertexBuffer va(GPUContext::instance().getDevice(), Triangles);
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
            SubMesh subMesh(GPUContext::instance().getDevice());
            subMesh.setVertexBuffer(va);
            addSubMesh(std::move(subMesh));
        }
        void Plane::setTexCoords(FloatRect texCoords) {
            for (unsigned int i = 0; i < getSubMeshesCount(); i++) {
                VertexBuffer& va = getSubMeshes()[i].getVertexBuffer();
                va[0].texCoords = math::Vec2f(texCoords.left, texCoords.top);
                va[1].texCoords = math::Vec2f(texCoords.left, texCoords.top+texCoords.height);
                va[2].texCoords = math::Vec2f(texCoords.left + texCoords.width, texCoords.top+texCoords.height);
                va[3].texCoords = math::Vec2f(texCoords.left + texCoords.width, texCoords.top);
            }
        }
        void Plane::setTexture(Texture* texture) {
            this->texture = texture;
            for (unsigned int i = 0; i < getSubMeshesCount(); i++) {
                getSubMeshes()[i].getMaterial().setTexture(texture, Material::DIFFUSE);
            }
        }
        GameObject* Plane::clone() {
            Plane* plane = new Plane(getPosition(), getSize());
            GameObject::copy(plane);
            return plane;
        }
    }
}
