#ifndef ODFAEG_3D_MODEL_HPP
#define ODFAEG_3D_MODEL_HPP
#include "../mesh.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../../Window/vkDevice.hpp"
#include <map>
namespace odfaeg {
    namespace graphic {
        namespace g3d {
            #ifdef VULKAN

            #else
            class ODFAEG_GRAPHICS_API Model {

                public :
                    struct BoneInfo {
                        /*id is index in finalBoneMatrices*/
                        int id;
                        /*offset matrix transforms vertex from model space to bone space*/
                        math::Matrix4f offset;
                    };
                    Model ();
                    Entity* loadModel(std::string path, EntityFactory& factory);
                    std::map<std::string, BoneInfo>& getBoneInfoMap();
                    int& getBoneCount();
                private :
                    math::Matrix4f convertAssimpToODFAEGMatrix(aiMatrix4x4 aiMatrix);
                    void setVertexBoneData(Vertex& vertex, int boneID, float weight);
                    void extractBoneWeightForVertices(std::vector<Vertex*>& vertices, aiMesh* mesh, const aiScene* scene);
                    void processNode(aiNode *node, const aiScene *scene, Mesh* emesh);
                    void processMesh(aiMesh *mesh, const aiScene *scene, Mesh* emesh);
                    std::vector<const Texture*> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                             std::string typeName);
                    core::TextureManager<> tm;
                    math::Vec3f max, min;
                    std::string directory;
                    std::map<std::string, BoneInfo> m_BoneInfoMap; //
                    int m_BoneCounter = 0;
            };
            #endif
        }
    }
}
#endif
