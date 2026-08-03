#ifndef ODFAEG_MODELOADER_HPP
#define ODFAEG_MODELOADER_HPP
#include <cstdint>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <vector>
#include <deque>
#include <atomic>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <assimp/postprocess.h>
#include <odfaeg/config.hpp>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>
#include <glm/gtx/string_cast.hpp>
#include "../Entity/model.hpp"
#include "device.hpp"
#include "../Core/resourceManager.hpp"
#include "../Math/vec.hpp"
#include "../Math/matrix.hpp"
#include "mesh.hpp"
#include "../Entity/vertex.hpp"
#include "texture.hpp"
#include "../Core/threadPool.hpp"
#include "commandPool.hpp"
#include  "buffer.hpp"
#include "imageLoader.hpp"
#include "../Core/clock.hpp"
#include  "material.hpp"
#include "../Entity/vertexArray.hpp"
#include "../Entity/gameObject.hpp"
#include "../Entity/assimpHelper.hpp"
#include "../Math/transformMatrix.hpp"
#include "../Physics/boundingBox.hpp"
#include "../Entity/color.hpp"
#include "../Entity/primitiveType.hpp"
namespace odfaeg {
    namespace graphic {
        class ModelLoader {

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
#include "modelLoader.inl"
#endif