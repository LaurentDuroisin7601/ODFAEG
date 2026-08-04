#ifndef ODFAEG_MESH_HPP
#define ODFAEG_MESH_HPP
#include <memory>
#include <vector>
#include <deque>
#include "../Core/utilities.hpp"
#include "../Entity/gameObject.hpp"
#include "gpuContext.hpp"
#include "material.hpp"
#include "vertexBuffer.hpp"
#include "texture.hpp"
namespace odfaeg {
    namespace graphic {
        class Mesh {
            public :
            Mesh(entity::GameObject* gameObject);
            void addMaterial(Material* material);            
            std::vector<Material*> getMaterials();
            entity::GameObject* getGameObject();
            void buildChildren();            
            void populateVertexBuffers();
            std::vector<Mesh*> getChildren();
            std::deque<VertexBuffer>& getVertexBuffers();
            void buildMaterialsFromTexture(Texture* texture);
            template <typename I>
            void buildMaterialsFromTextureManager(core::TextureManager<I>& textureManager) {
                for(unsigned int i = 0; i < gameObject->getSubMeshes().size(); i++) {
                    materials.push_back(std::make_unique<Material>());
                    materials.back()->setTexture(textureManager.getResourceByAlias(static_cast<I>(core::conversionStringInt(gameObject->getSubMeshes()[i].getTextureId(entity::SubMesh::DIFFUSE)))), entity::SubMesh::DIFFUSE);
                }
                for (unsigned int i = 0; i < children.size(); i++) {
                    children[i]->buildMaterialsFromTextureManager(textureManager);
                }
            }    
            void buildMaterialsFromTextureManager(core::TextureManager<std::string>& textureManager) {
                for(unsigned int i = 0; i < gameObject->getSubMeshes().size(); i++) {
                    materials.push_back(std::make_unique<Material>());
                    materials.back()->setTexture(textureManager.getResourceByAlias(gameObject->getSubMeshes()[i].getTextureId(entity::SubMesh::DIFFUSE)), entity::SubMesh::DIFFUSE);
                }
                for (unsigned int i = 0; i < children.size(); i++) {
                    children[i]->buildMaterialsFromTextureManager(textureManager);
                }
            }
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
#endif