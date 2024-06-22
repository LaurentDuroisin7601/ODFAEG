#ifndef ODFAEG_3D_MODEL_HPP
#define ODFAEG_3D_MODEL_HPP
#include "../mesh.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
namespace odfaeg {
    namespace graphic {
        namespace g3d {
            class Model {
                public :
                    Model ();
                    Entity* loadModel(std::string path, EntityFactory& factory);
                private :
                    void processNode(aiNode *node, const aiScene *scene, Mesh* emesh);
                    void processMesh(aiMesh *mesh, const aiScene *scene, Mesh* emesh);
                    std::vector<const Texture*> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                             std::string typeName);
                    core::TextureManager<> tm;
                    math::Vec3f max, min;
                    std::string directory;
            };
        }
    }
}
#endif
