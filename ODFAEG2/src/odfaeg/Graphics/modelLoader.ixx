module;
#include <cstdint>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <vector>
#include <deque>
#include <atomic>
export module odfaeg.graphic.modelLoader;
import odfaeg.graphic.model;
import odfaeg.graphic.device;
import odfaeg.core.resourceManager;
import odfaeg.math.vec;
import odfaeg.math.matrix;
import odfaeg.graphic.mesh;
import odfaeg.entity.vertex;
import odfaeg.graphic.texture;
import odfaeg.core.threadPool;
import odfaeg.graphic.commandPool;
import odfaeg.graphic.buffer;
import odfaeg.graphic.imageLoader;
import odfaeg.core.clock;
import odfaeg.graphic.material;
import odfaeg.entity.vertexArray;
import odfaeg.entity.gameObject;
namespace odfaeg {
    namespace graphic {
        export class ModelLoader {

        public :

            ModelLoader(Device& device, core::ResourceManager<Texture, std::string>& textureManager);
            Mesh* loadModel(std::string path, bool loadTextures=true);
        private :
            entity::SubMesh::TexType convertAssimpType(aiTextureType type);
            void setVertexBoneDataToDefault(entity::Vertex& vertex);
            void setVertexBoneData(entity::Vertex& vertex, int boneID, float weight);
            void extractBoneWeightForVertices(std::vector<entity::Vertex>& vertices, aiMesh* mesh, const aiScene* scene, entity::Model* mnode);
            void processNode(math::Matrix4f parentTransform, aiNode *node, const aiScene *scene, Mesh* mnode, entity::Model* mmodel, bool loadTextures);
            void processMesh(math::Matrix4f parentTransform, aiMesh *mesh, const aiScene *scene, Mesh* mnode, entity::Model* mmodel, bool loadTextures);
            std::vector<Texture*> loadMaterialTextures(const aiScene* scene, aiMaterial *mat, aiTextureType type);
            math::Vec3f max, min;
            std::string directory;
            Device& device;
            core::ResourceManager<Texture, std::string>& textureManager;
            CommandPool commandPool;
            core::ThreadPool threadPool;
            Buffer staggingBuffer;
            ImageLoader imageLoader;
            core::JobFence jobFence;
            core::Clock clk, clk2;
            bool isSkinned;
            unsigned int currentTexturesOffset;
        };
    }
}
