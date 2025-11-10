#include "../../../../include/odfaeg/Graphics/3D/model.hpp"
namespace odfaeg {
    namespace graphic {
        namespace g3d {
            #ifdef VULKAN
            Model::Model (window::Device& vkDevice) : vkDevice(vkDevice)  {
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
                #ifdef ODFAEG_SYSTEM_WINDOWS
                directory = path.substr(0, path.find_last_of('\\'));
                #else
                directory = path.substr(0, path.find_last_of('/'));
                #endif
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
                if(mesh->mMaterialIndex >= 0) {
                    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
                    std::vector<const Texture*> diffuseMaps = loadMaterialTextures(material,
                                                        aiTextureType_DIFFUSE, "texture_diffuse");
                    std::vector<const Texture*> specularMaps = loadMaterialTextures(material,
                                                        aiTextureType_SPECULAR, "texture_specular");
                    for (unsigned int i = 0; i < diffuseMaps.size(); i++) {
                        mat.addTexture(diffuseMaps[i], IntRect(0, 0, 0, 0));
                    }
                    for (unsigned int i = 0; i < specularMaps.size(); i++) {
                        mat.addTexture(specularMaps[i], IntRect(0, 0, 0, 0));
                    }
                }
                std::vector<Vertex> vertices;
                std::vector<math::Vec3f> verts;
                for (unsigned int i = 0; i < mesh->mNumVertices; i++)
                {
                    Vertex vertex;
                    vertex.position[0] = mesh->mVertices[i].x;
                    vertex.position[1] = mesh->mVertices[i].y;
                    vertex.position[2] = mesh->mVertices[i].z;
                    if (mesh->mTextureCoords[0])
                    {
                        vertex.texCoords[0] = mesh->mTextureCoords[0][i].x * mat.getTexture()->getSize().x();
                        vertex.texCoords[1] = mesh->mTextureCoords[0][i].y * mat.getTexture()->getSize().y();
                    } else {
                        vertex.texCoords = math::Vec2f(0, 0);
                    }
                    vertices.push_back(vertex);
                    verts.push_back(math::Vec3f(vertex.position.x(), vertex.position.y(), vertex.position.z()));
                }
                extractBoneWeightForVertices(vertices, mesh, scene, emesh);

                ////////std::cout<<"num bones : "<<mesh->mNumBones<<std::endl;

                for(unsigned int i = 0; i < mesh->mNumFaces; i++)
                {
                    VertexArray va(Triangles, 0, emesh);
                    //std::cout<<"EMESH ID : "<<emesh->getId()<<std::endl;
                    aiFace face = mesh->mFaces[i];
                    for(unsigned int j = 0; j < face.mNumIndices; j++) {
                        va.append(vertices[face.mIndices[j]]);
                    }
                    Face f(va,mat,emesh->getTransform());
                    emesh->addFace(f);
                }
                std::array<std::array<float, 2>, 3> exts = math::Computer::getExtends(verts);
                emesh->setSize(math::Vec3f(exts[0][1] - exts[0][0], exts[1][1] - exts[1][0], exts[2][1] - exts[2][0]));
                //emesh->setOrigin(math::Vec3f(emesh->getSize()*0.5));
            }
            void Model::setVertexBoneData(Vertex& vertex, int boneID, float weight)
            {
                for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
                {
                    if (vertex.m_BoneIDs[i] < 0)
                    {
                        vertex.m_Weights[i] = weight;
                        vertex.m_BoneIDs[i] = boneID;
                        ////////std::cout<<"set vertex bone id : "<<vertex.m_BoneIDs[i]<<std::endl;
                        break;
                    }
                }
            }
            void Model::extractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene, Mesh* emesh)
            {

                for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
                {
                    int boneID = -1;
                    std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
                    if (emesh->getBoneInfoMap().find(boneName) == emesh->getBoneInfoMap().end())
                    {
                        Entity::BoneInfo newBoneInfo;
                        newBoneInfo.id = emesh->getBoneCount();
                        newBoneInfo.offset = AssimpHelpers::convertAssimpToODFAEGOffsetMatrix(mesh->mBones[boneIndex]->mOffsetMatrix);
                        //std::cout<<"offset : "<<newBoneInfo.offset<<std::endl;
                        emesh->getBoneInfoMap()[boneName] = newBoneInfo;
                        boneID = emesh->getBoneCount();
                        emesh->getBoneCount()++;

                    }
                    else
                    {
                        boneID = emesh->getBoneInfoMap()[boneName].id;
                    }
                    ////////std::cout<<"bone id : "<<boneID<<std::endl;

                    assert(boneID != -1);
                    auto weights = mesh->mBones[boneIndex]->mWeights;
                    int numWeights = mesh->mBones[boneIndex]->mNumWeights;

                    for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
                    {
                        int vertexId = weights[weightIndex].mVertexId;
                        float weight = weights[weightIndex].mWeight;
                        assert(vertexId <= vertices.size());
                        setVertexBoneData(vertices[vertexId], boneID, weight);
                        /*//////std::cout<<"vertex bone id : "<<vertices[vertexId]->m_BoneIDs[boneID]<<std::endl;
                        system("PAUSE");*/
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
                    #ifdef ODFAEG_SYSTEM_WINDOWS
                    std::string path = directory + "\\" + std::string (str.C_Str());
                    #else
                    std::string path = directory + "/" + std::string (str.C_Str());
                    #endif
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
                        std::tuple<std::reference_wrapper<window::Device>> rArgs = std::make_tuple(std::ref(vkDevice));
                        tm.fromFileWithAlias(path, path, rArgs);
                        const Texture* texture = tm.getResourceByAlias(path);
                        textures.push_back(texture);
                    }
                }
                return textures;
            }
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
                #ifdef ODFAEG_SYSTEM_WINDOWS
                directory = path.substr(0, path.find_last_of('\\'));
                #else
                directory = path.substr(0, path.find_last_of('/'));
                #endif
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
                if(mesh->mMaterialIndex >= 0) {
                    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
                    std::vector<const Texture*> diffuseMaps = loadMaterialTextures(material,
                                                        aiTextureType_DIFFUSE, "texture_diffuse");
                    std::vector<const Texture*> specularMaps = loadMaterialTextures(material,
                                                        aiTextureType_SPECULAR, "texture_specular");
                    for (unsigned int i = 0; i < diffuseMaps.size(); i++) {
                        mat.addTexture(diffuseMaps[i], IntRect(0, 0, 0, 0));
                    }
                    for (unsigned int i = 0; i < specularMaps.size(); i++) {
                        mat.addTexture(specularMaps[i], IntRect(0, 0, 0, 0));
                    }
                }
                std::vector<Vertex> vertices;
                std::vector<math::Vec3f> verts;
                for (unsigned int i = 0; i < mesh->mNumVertices; i++)
                {
                    Vertex vertex;
                    vertex.position[0] = mesh->mVertices[i].x;
                    vertex.position[1] = mesh->mVertices[i].y;
                    vertex.position[2] = mesh->mVertices[i].z;
                    if (mesh->mTextureCoords[0])
                    {
                        vertex.texCoords[0] = mesh->mTextureCoords[0][i].x * mat.getTexture()->getSize().x();
                        vertex.texCoords[1] = mesh->mTextureCoords[0][i].y * mat.getTexture()->getSize().y();
                    } else {
                        vertex.texCoords = math::Vec2f(0, 0);
                    }
                    vertices.push_back(vertex);
                    verts.push_back(math::Vec3f(vertex.position.x(), vertex.position.y(), vertex.position.z()));
                }
                extractBoneWeightForVertices(vertices, mesh, scene, emesh);

                ////////std::cout<<"num bones : "<<mesh->mNumBones<<std::endl;

                for(unsigned int i = 0; i < mesh->mNumFaces; i++)
                {
                    VertexArray va(Triangles, 0, emesh);
                    aiFace face = mesh->mFaces[i];
                    for(unsigned int j = 0; j < face.mNumIndices; j++) {
                        va.append(vertices[face.mIndices[j]]);
                    }
                    Face f(va,mat,emesh->getTransform());
                    emesh->addFace(f);
                }
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
                        ////////std::cout<<"set vertex bone id : "<<vertex.m_BoneIDs[i]<<std::endl;
                        break;
                    }
                }
            }
            void Model::extractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene, Mesh* emesh)
            {

                for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
                {
                    int boneID = -1;
                    std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
                    if (emesh->getBoneInfoMap().find(boneName) == emesh->getBoneInfoMap().end())
                    {
                        Entity::BoneInfo newBoneInfo;
                        newBoneInfo.id = emesh->getBoneCount();
                        newBoneInfo.offset = AssimpHelpers::convertAssimpToODFAEGMatrix(mesh->mBones[boneIndex]->mOffsetMatrix);
                        emesh->getBoneInfoMap()[boneName] = newBoneInfo;
                        boneID = emesh->getBoneCount();
                        emesh->getBoneCount()++;

                    }
                    else
                    {
                        boneID = emesh->getBoneInfoMap()[boneName].id;
                    }
                    ////////std::cout<<"bone id : "<<boneID<<std::endl;

                    assert(boneID != -1);
                    auto weights = mesh->mBones[boneIndex]->mWeights;
                    int numWeights = mesh->mBones[boneIndex]->mNumWeights;

                    for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
                    {
                        int vertexId = weights[weightIndex].mVertexId;
                        float weight = weights[weightIndex].mWeight;
                        assert(vertexId <= vertices.size());
                        setVertexBoneData(vertices[vertexId], boneID, weight);
                        /*//////std::cout<<"vertex bone id : "<<vertices[vertexId]->m_BoneIDs[boneID]<<std::endl;
                        system("PAUSE");*/
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
                    #ifdef ODFAEG_SYSTEM_WINDOWS
                    std::string path = directory + "\\" + std::string (str.C_Str());
                    #else
                    std::string path = directory + "/" + std::string (str.C_Str());
                    #endif
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
            #endif
        }
    }
}
