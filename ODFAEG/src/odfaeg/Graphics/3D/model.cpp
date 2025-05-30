#include "../../../../include/odfaeg/Graphics/3D/model.hpp"
namespace odfaeg {
    namespace graphic {
        namespace g3d {
            #ifdef VULKAN
            #else
            Model::Model ()  {
                float maxF = std::numeric_limits<float>::max();
                float minF = std::numeric_limits<float>::min();
                min = math::Vec3f(maxF, maxF, maxF);
                max = math::Vec3f(minF, minF, minF);
            }
            Entity* Model::loadModel(std::string path, EntityFactory& factory) {
                Mesh* emesh = factory.make_entity<Mesh>(math::Vec3f(0, 0, 0), math::Vec3f(0, 0, 0),"E_MESH", factory);
                Assimp::Importer importer;
                const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
                if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
                {
                    std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
                    return emesh;
                }
                directory = path.substr(0, path.find_last_of('/'));
                processNode(scene->mRootNode, scene, emesh);
                return emesh;
            }
            void Model::processNode(aiNode *node, const aiScene *scene, Mesh* emesh)
            {

                // process all the node's meshes (if any)
                for(unsigned int i = 0; i < node->mNumMeshes; i++)
                {
                    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
                    processMesh(mesh, scene, emesh);
                }
                // then do the same for each of its children
                for(unsigned int i = 0; i < node->mNumChildren; i++)
                {
                    processNode(node->mChildren[i], scene, emesh);
                }
            }
            void Model::processMesh(aiMesh *mesh, const aiScene *scene, Mesh* emesh) {

                Material mat;
                mat.clearTextures();
                std::vector<Vertex*> ptrVerts;
                if(mesh->mMaterialIndex >= 0) {
                    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
                    std::vector<const Texture*> diffuseMaps = loadMaterialTextures(material,
                                                        aiTextureType_DIFFUSE, "texture_diffuse");
                    std::vector<const Texture*> specularMaps = loadMaterialTextures(material,
                                                        aiTextureType_SPECULAR, "texture_specular");
                    for (unsigned int i = 0; i < diffuseMaps.size(); i++) {
                        mat.addTexture(diffuseMaps[i], sf::IntRect(0, 0, 0, 0));
                    }
                    for (unsigned int i = 0; i < specularMaps.size(); i++) {
                        mat.addTexture(specularMaps[i], sf::IntRect(0, 0, 0, 0));
                    }
                }
                std::vector<math::Vec3f> verts;
                for(unsigned int i = 0; i < mesh->mNumFaces; i++)
                {
                    VertexArray va(sf::Triangles, 0, emesh);
                    aiFace face = mesh->mFaces[i];
                    for(unsigned int j = 0; j < face.mNumIndices; j++) {
                        Vertex vertex;
                        vertex.position.x = mesh->mVertices[face.mIndices[j]].x;
                        vertex.position.y = mesh->mVertices[face.mIndices[j]].y;
                        vertex.position.z = mesh->mVertices[face.mIndices[j]].z;
                        vertex.texCoords.x = mesh->mTextureCoords[0][face.mIndices[j]].x * mat.getTexture()->getSize().x;
                        vertex.texCoords.y = mesh->mTextureCoords[0][face.mIndices[j]].y * mat.getTexture()->getSize().y;
                        va.append(vertex);
                        verts.push_back(math::Vec3f(vertex.position.x, vertex.position.y, vertex.position.z));
                    }
                    Face f(va,mat,emesh->getTransform());
                    emesh->addFace(f);
                }
                for (unsigned int i = 0; i < emesh->getFaces().size(); i++) {
                    for (unsigned int v = 0; v < emesh->getFaces()[i].getVertexArray().getVertexCount(); v++) {
                        ptrVerts.push_back(&emesh->getFaces()[i].getVertexArray()[v]);
                    }
                }
                extractBoneWeightForVertices(ptrVerts, mesh, scene);
                std::array<std::array<float, 2>, 3> exts = math::Computer::getExtends(verts);
                emesh->setSize(math::Vec3f(exts[0][1] - exts[0][0], exts[1][1] - exts[1][0], exts[2][1] - exts[2][0]));
                emesh->setOrigin(math::Vec3f(emesh->getSize()*0.5));
            }
            void Model::setVertexBoneData(Vertex& vertex, int boneID, float weight)
            {
                for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
                {
                    if (vertex.m_BoneIDs[i] < 0)
                    {
                        vertex.m_Weights[i] = weight;
                        vertex.m_BoneIDs[i] = boneID;
                        break;
                    }
                }
            }
            math::Matrix4f Model::convertAssimpToODFAEGMatrix(aiMatrix4x4 aiMatrix) {
                math::Matrix4f mat;
                mat.m11 = aiMatrix.a1;
                mat.m12 = aiMatrix.a2;
                mat.m13 = aiMatrix.a3;
                mat.m14 = aiMatrix.a4;

                mat.m21 = aiMatrix.b1;
                mat.m22 = aiMatrix.b2;
                mat.m23 = aiMatrix.b3;
                mat.m24 = aiMatrix.b4;

                mat.m31 = aiMatrix.c1;
                mat.m32 = aiMatrix.c2;
                mat.m33 = aiMatrix.c3;
                mat.m34 = aiMatrix.c4;

                mat.m41 = aiMatrix.d1;
                mat.m42 = aiMatrix.d2;
                mat.m43 = aiMatrix.d3;
                mat.m44 = aiMatrix.d4;
                return mat;
            }
            void Model::extractBoneWeightForVertices(std::vector<Vertex*>& vertices, aiMesh* mesh, const aiScene* scene)
            {
                for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
                {
                    int boneID = -1;
                    std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
                    if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end())
                    {
                        BoneInfo newBoneInfo;
                        newBoneInfo.id = m_BoneCounter;
                        newBoneInfo.offset = convertAssimpToODFAEGMatrix(mesh->mBones[boneIndex]->mOffsetMatrix);
                        m_BoneInfoMap[boneName] = newBoneInfo;
                        boneID = m_BoneCounter;
                        m_BoneCounter++;
                    }
                    else
                    {
                        boneID = m_BoneInfoMap[boneName].id;
                    }
                    assert(boneID != -1);
                    auto weights = mesh->mBones[boneIndex]->mWeights;
                    int numWeights = mesh->mBones[boneIndex]->mNumWeights;

                    for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
                    {
                        int vertexId = weights[weightIndex].mVertexId;
                        float weight = weights[weightIndex].mWeight;
                        assert(vertexId <= vertices.size());
                        setVertexBoneData(*vertices[vertexId], boneID, weight);
                    }
                }
            }
            std::vector<const Texture*> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
            {
                std::vector<const Texture*> textures;
                for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
                {
                    aiString str;
                    mat->GetTexture(type, i, &str);
                    bool skip = false;
                    std::string path = directory + "/" + std::string (str.C_Str());
                    for(unsigned int j = 0; j < tm.getPaths().size(); j++)
                    {
                        if(tm.getPaths()[j] == path)
                        {
                            textures.push_back(tm.getResourceByAlias(path));
                            skip = true;
                            break;
                        }
                    }
                    if(!skip)
                    {   // if texture hasn't been loaded already, load it
                        tm.fromFileWithAlias(path, path);
                        const Texture* texture = tm.getResourceByAlias(path);
                        textures.push_back(texture);
                    }
                }
                return textures;
            }
            std::map<std::string, Model::BoneInfo>& Model::getBoneInfoMap() { return m_BoneInfoMap; }
            int& Model::getBoneCount() { return m_BoneCounter; }
            #endif
        }
    }
}
