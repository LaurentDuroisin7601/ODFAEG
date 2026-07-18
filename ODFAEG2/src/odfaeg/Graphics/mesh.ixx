module;
#include <memory>
#include <vector>
#include <deque>
export module odfaeg.graphic.mesh;
import odfaeg.entity.gameObject;
import odfaeg.graphic.material;
import odfaeg.graphic.vertexBuffer;
namespace odfaeg {
    namespace graphic {
        export class Mesh {
            public :
            Mesh(entity::GameObject* gameObject);
            void addMaterial(Material* material);            
            std::vector<Material*> getMaterials();
            entity::GameObject* getGameObject();
            void buildChildren();            
            void populateVertexBuffers();
            std::vector<Mesh*> getChildren();
            std::deque<VertexBuffer>& getVertexBuffers();
            private :
            void buildChild(entity::GameObject* parent);
            void populateVertexBuffer(Mesh* parent);
            entity::GameObject* gameObject;
            std::vector<std::unique_ptr<Mesh>> children;
            std::deque<VertexBuffer> vbs;
            std::vector<std::unique_ptr<Material>> materials;
        };
    }
}