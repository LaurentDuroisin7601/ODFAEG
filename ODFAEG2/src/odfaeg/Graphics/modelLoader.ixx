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
import odfaeg.graphic.gameObject;
import odfaeg.graphic.vertex;
import odfaeg.graphic.texture;
import odfaeg.core.threadPool;
import odfaeg.graphic.commandPool;
import odfaeg.graphic.buffer;
import odfaeg.graphic.imageLoader;
import odfaeg.core.clock;
import odfaeg.graphic.material;
import odfaeg.graphic.vertexBuffer;
namespace odfaeg {
    namespace graphic {
        export class ModelLoader {

        public :

            ModelLoader(Device& device, core::ResourceManager<Texture, std::string>& textureManager);
            GameObject* loadModel(std::string path, bool loadTextures=true);
        private :
            Material::TexType convertAssimpType(aiTextureType type);
            void setVertexBoneDataToDefault(Vertex& vertex);
            void setVertexBoneData(Vertex& vertex, int boneID, float weight);
            void extractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene, Model* mnode);
            void processNode(math::Matrix4f parentTransform, aiNode *node, const aiScene *scene, Model* mnode, bool loadTextures);
            void processMesh(math::Matrix4f parentTransform, aiMesh *mesh, const aiScene *scene, Model* mnode, bool loadTextures);
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
