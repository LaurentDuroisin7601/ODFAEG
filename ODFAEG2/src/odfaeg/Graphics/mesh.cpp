module;
#include <memory>
#include <deque>
module odfaeg.graphic.mesh;
import odfaeg.graphic.gpuContext;
namespace odfaeg {
    namespace graphic {
        Mesh::Mesh(entity::GameObject* gameObject) : gameObject(gameObject) {
            
        }
        void Mesh::buildChildren() {
            buildChild(gameObject);
        }
        void Mesh::buildChild(entity::GameObject* parent) {
            for (unsigned int i = 0; i < parent->getChildren().size(); i++) {                
                children.push_back(std::make_unique<Mesh>(parent->getChildren()[i]));
                children.back()->buildChild(parent->getChildren()[i]);
            }
        }
        void Mesh::populateVertexBuffers() {
            populateVertexBuffer(this);
        }
        void Mesh::populateVertexBuffer(Mesh* parent) {
            for (unsigned int i = 0; i < children.size(); i++) {
                populateVertexBuffer(children[i].get());
            }
            for (unsigned int i = 0; i < parent->getGameObject()->getSubMeshes().size(); i++) {
                vbs.emplace_back(GPUContext::instance().getDevice());
                for(unsigned int v = 0; v < parent->getGameObject()->getSubMeshes()[i].getVertexArray().getVertexCount(); v++) {
                    vbs.back().append(parent->getGameObject()->getSubMeshes()[i].getVertexArray()[v]);                    
                }
                for(unsigned int v = 0; v < parent->getGameObject()->getSubMeshes()[i].getVertexArray().getIndexCount(); v++) {
                    vbs.back().addIndex(parent->getGameObject()->getSubMeshes()[i].getVertexArray().getIndex(v));                    
                }
            }
        }
        std::deque<VertexBuffer>& Mesh::getVertexBuffers() {
            return vbs;
        }
        void Mesh::addMaterial(Material* material) {
            std::unique_ptr<Material> ptr;
            ptr.reset(material);
            materials.push_back(std::move(ptr));
        }           
        std::vector<Material*> Mesh::getMaterials() {
            std::vector<Material*> mats;
            for (unsigned int i = 0; i < materials.size(); i++) {
                mats.push_back(materials[i].get());
            }
            return mats;
        }
        entity::GameObject* Mesh::getGameObject() {
            return gameObject;
        } 
        std::vector<Mesh*> Mesh::getChildren() {
            std::vector<Mesh*> chldr;
            for (unsigned int i = 0; i < children.size(); i++) {
                chldr.push_back(children[i].get());
            }
            return chldr;
        }         
    }
}