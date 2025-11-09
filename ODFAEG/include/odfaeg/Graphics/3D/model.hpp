#ifndef ODFAEG_3D_MODEL_HPP
#define ODFAEG_3D_MODEL_HPP
#include "../mesh.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../../Window/vkDevice.hpp"
#include "../assimp_helper.hpp"
#include <map>
namespace odfaeg {
    namespace graphic {
        namespace g3d {
            #ifdef VULKAN
            class ODFAEG_GRAPHICS_API Model {

                public :

                    Model (window::Device& vkDevice);
                    Entity* loadModel(std::string path, EntityFactory& factory);

                private :
                    void setVertexBoneData(Vertex& vertex, int boneID, float weight);
                    void extractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene, Mesh* eMesh);
                    void processNode(aiNode *node, const aiScene *scene, Mesh* emesh);
                    void processMesh(aiMesh *mesh, const aiScene *scene, Mesh* emesh);
                    std::vector<const Texture*> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                             std::string typeName);
                    core::TextureManager<> tm;
                    math::Vec3f max, min;
                    std::string directory;
                    window::Device& vkDevice;

            };
            #else
            class ODFAEG_GRAPHICS_API Model {

                public :

                    Model ();
                    Entity* loadModel(std::string path, EntityFactory& factory);

                private :
                    void setVertexBoneData(Vertex& vertex, int boneID, float weight);
                    void extractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene, Mesh* eMesh);
                    void processNode(aiNode *node, const aiScene *scene, Mesh* emesh);
                    void processMesh(aiMesh *mesh, const aiScene *scene, Mesh* emesh);
                    std::vector<const Texture*> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                             std::string typeName);
                    core::TextureManager<> tm;
                    math::Vec3f max, min;
                    std::string directory;

            };
            #endif
        }
    }
}
#endif
