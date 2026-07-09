module;
#include <cassert>
#include <filesystem>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <odfaeg/config.hpp>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>
#include <glm/gtx/string_cast.hpp>
//import odfaeg.graphic.modelLoader;
module odfaeg.graphic.modelLoader;
import odfaeg.graphic.assimpHelper;
import odfaeg.entity.transformMatrix;
import odfaeg.physic.boundingBox;
import odfaeg.graphic.color;
namespace odfaeg {
    namespace graphic {
        ModelLoader::ModelLoader (Device& device, core::ResourceManager<Texture, std::string>& textureManager) : device(device), textureManager(textureManager),
        commandPool(device), staggingBuffer(device), threadPool(6) {
            float maxF = std::numeric_limits<float>::max();
            float minF = std::numeric_limits<float>::min();
            min = math::Vec3f(maxF, maxF, maxF);
            max = math::Vec3f(minF, minF, minF);
            Device::QueueFamilyIndices indices = device.findQueueFamilies(device.getPhysicalDevice());
            commandPool.create(indices.graphicsFamily.value());
            commandPool.createCommandBuffers(true, 1);
            isSkinned = false;
            currentTexturesOffset = 0;
        }
        GameObject* ModelLoader::loadModel(std::string path, bool loadTextures) {
            //std::cout<<"load model : "<<path<<std::endl;
            Model* model = new Model(math::Vec3f(0.f, 0.f, 0.f), math::Vec3f(0.f, 0.f, 0.f),math::Vec3f(0.f, 0.f, 0.f), "E_MODEL");
            Assimp::Importer importer;
            //std::cout<<"import"<<std::endl;
            /*uint32_t importFlags{ aiProcess_Triangulate
            | aiProcess_FixInfacingNormals
            | aiProcess_LimitBoneWeights
            | aiProcess_FindDegenerates };
            //if ( !parameters.get< bool >( cuT( "no_validation" ) ) )
                importFlags |= aiProcess_ValidateDataStructure
                    | aiProcess_FindInvalidData;
            //if ( !parameters.get< bool >( cuT( "no_optimisations" ) ) )
                importFlags |= aiProcess_JoinIdenticalVertices
                    | aiProcess_OptimizeMeshes
                    | aiProcess_OptimizeGraph
                    | aiProcess_ImproveCacheLocality
                    | aiProcess_RemoveRedundantMaterials;
            //if ( parameters.get< c3d::String >( cuT( "normals" ) ) == cuT( "smooth" ) )
                importFlags |= aiProcess_GenSmoothNormals;
            //if ( parameters.get< bool >( cuT( "tangent_space" ) ) )
                importFlags |= aiProcess_CalcTangentSpace;*/
            //std::cout<<"path : "<<path<<std::endl;
            const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs /*| aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace*/);

            //std::cout<<"imported"<<std::endl;
            if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
            {
                std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
                return model;
            }
            isSkinned = false;
            for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
                if (scene->mMeshes[i]->HasBones()) {
                    isSkinned = true;
                    break;
                }
            }
            /*std::cout << "ODFAEG root transform = "
            << glm::to_string(AssimpHelpers::convertMatrixToGLMFormat(scene->mRootNode->mTransformation))
            << std::endl;*/
            if(path.find("/") != std::string::npos)
                directory = path.substr(0, path.find_last_of('/'));
            else
                directory = "";
            math::Matrix4f transform;
            transform.identity();
            clk2.restart();
            processNode(transform, scene->mRootNode, scene, model, loadTextures);
            std::vector<ImageLoader> imageLoaders;
            size_t totalImagesSize = 0;
            for (unsigned int i = currentTexturesOffset; i < textureManager.getAliases().size(); i++) {
                std::string alias = textureManager.getAliases()[i];
                ImageLoader imageLoader;
                if (textureManager.getAliases()[i].length() > 0 && textureManager.getAliases()[i].at(0) == '*') {
                    //std::cout<<"load texture from memory!"<<std::endl;
                    int index = atoi(textureManager.getAliases()[i].data() + 1);
                    aiTexture* tex = scene->mTextures[index];
                    //std::cout<<"enqueue!"<<std::endl;
                    imageLoader.loadFromMemory(tex->pcData, tex->mWidth);
                } else {
                    imageLoader.loadFromFile(alias);
                }
                /*std::cout << "Texture " << i
                  <<" size=" << imageLoaders[i].getDataSize()
                  <<" w=" << imageLoaders[i].getSize().x()
                  <<" h=" << imageLoaders[i].getSize().y()
                  <<" offset=" << dataOffset
                  <<" offset + size : "<<dataOffset+imageLoaders[i].getDataSize()
                  <<" total image size : "<<totalImagesSize.load()
                  << std::endl;*/
                Texture* texture = textureManager.getResourceByAlias(alias);
                if (imageLoader.isCompressed()) {
                    texture->setFormat(imageLoader.getVkFormat());
                } else {
                    texture->setFormat(VK_FORMAT_R8G8B8A8_SRGB);
                }
                texture->setSize(imageLoader.getSize());
                texture->create(imageLoader.getSize().x(), imageLoader.getSize().y(),1, imageLoader.getMipLevels());
                for (unsigned int j = 0; j < imageLoader.getMipLevels(); j++) {
                    totalImagesSize = (totalImagesSize + 15) & ~15;
                    totalImagesSize += imageLoader.getDataSize(j);

                }
                imageLoaders.push_back(imageLoader);
            }
            commandPool.beginRecordCommandBuffer(0);
            staggingBuffer.create(totalImagesSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
            std::size_t dataOffset = 0;
            for (unsigned int i = 0; i < imageLoaders.size(); i++) {
                //std::cout<<"compressée ? : "<<imageLoaders[i].isCompressed()<<std::endl;
                for (unsigned int j = 0; j < imageLoaders[i].getMipLevels(); j++) {
                    // align offset to 16 bytes
                    /*for (unsigned int p = 0; p < imageLoaders[i].getDataSize(j); p++) {
                        std::cout<<"pixel : "<<(int)imageLoaders[i].getPixelsPtr(j)[p]<<std::endl;
                    }*/
                    dataOffset = (dataOffset + 15) & ~15;
                    staggingBuffer.update(imageLoaders[i].getPixelsPtr(j), imageLoaders[i].getDataSize(j), dataOffset);
                    textureManager.getResourceByAlias(textureManager.getAliases()[currentTexturesOffset+i])->update(commandPool, staggingBuffer, imageLoaders[i].getSize(j).x(), imageLoaders[i].getSize(j).y(), 0, 0, dataOffset, j);
                    //std::cout<<"id : "<<textureManager.getResourceByAlias(textureManager.getAliases()[currentTexturesOffset+i])->getId()<<std::endl;
                    /*std::cout << "mip " << j << " size = " << imageLoaders[i].getDataSize(j)
                    << "  offset : " << dataOffset << std::endl;*/

                    dataOffset += imageLoaders[i].getDataSize(j);


                }
            }
            commandPool.endRecordCommandBuffer(0);
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandPool.getHandle(0);
            Device::QueueFamilyIndices indices = device.findQueueFamilies(device.getPhysicalDevice());
            if (vkQueueSubmit(device.getQueue(indices.graphicsFamily.value(), 0), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
                throw std::runtime_error("Echec de l'envoi d'un command buffer!");
            }
            vkDeviceWaitIdle(device.getDevice());
            currentTexturesOffset += textureManager.getAliases().size();
            //std::cout<<"scene loading time : "<<clk2.getElapsedTime().asMilliseconds()<<"ms"<<std::endl;
            return model;
        }
        void ModelLoader::processNode(math::Matrix4f parentTransform, aiNode *node, const aiScene *scene, Model* mnode, bool loadTextures)
        {
            //std::cout<<"process node"<<std::endl;
            // process all the node's meshes (if any)
            math::Matrix4f nodeLocal = AssimpHelpers::convertAssimpToODFAEGMatrix(node->mTransformation);
            math::Matrix4f world = parentTransform * nodeLocal;
            //std::cout<<"parent : "<<parentTransform<<std::endl<<"node : "<<nodeLocal<<std::endl;
            //std::cout<<"nb meshes to load : "<<node->mNumMeshes<<std::endl;
            clk.restart();
            jobFence.reset(node->mNumMeshes);
            for(unsigned int i = 0; i < node->mNumMeshes; i++)
            {
                aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
                threadPool.enqueue([this, world, mesh, scene, mnode, loadTextures] {
                    processMesh(world, mesh, scene, mnode, loadTextures);
                    jobFence.jobDone();
                });
            }
            jobFence.wait();
            //std::cout<<"loading meshes time : "<<clk.getElapsedTime().asMilliseconds()<<"ms"<<std::endl;
            // then do the same for each of its children
            for(unsigned int i = 0; i < node->mNumChildren; i++)
            {
                //Model* mChildNode = new Model(math::Vec3f(0.f, 0.f, 0.f), math::Vec3f(0.f, 0.f, 0.f),math::Vec3f(0.f, 0.f, 0.f), "E_MODEL");
                processNode(world, node->mChildren[i], scene, mnode, loadTextures);
                /*math::Vec3f rootSize = mnode->getSize();
                if (mChildNode->getSize().x() > rootSize.x()) {
                    rootSize.x() = mChildNode->getSize().x();
                }
                if (mChildNode->getSize().y() > rootSize.y()) {
                    rootSize.y() = mChildNode->getSize().y();
                }
                if (mChildNode->getSize().z() > rootSize.z()) {
                    rootSize.z() = mChildNode->getSize().z();
                }
                mnode->setSize(rootSize);
                mnode->addChild(mChildNode);*/
            }
        }
        void ModelLoader::processMesh(math::Matrix4f world, aiMesh *mesh, const aiScene *scene, Model* mnode, bool loadTextures) {
            //world = world.transpose();
            //std::lock_guard<std::recursive_mutex> lock(getGlobalMutex());
            //std::cout<<"process mesh"<<std::endl;
            /*SubMesh subMesh(device);

            for (unsigned int i = 0; i < Material::NBTEXTYPES; i++)
                subMesh.getMaterial().setTexture(nullptr, static_cast<Material::TexType>(i));*/
           
            //Color diffuseColor;           
            std::vector<Texture*> diffuseMaps, specularMaps, normalMaps, metalnessMaps, roughnessMaps, aoMaps, emissiveMaps;
            if(mesh->mMaterialIndex >= 0) {
                //std::cout<<"load materials"<<std::endl;


                aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
                /* aiColor4D color;
                if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, color)) {
                    // color.r, color.g, color.b, color.a
                    diffuseColor.r = color.r;
                    diffuseColor.g = color.g;
                    diffuseColor.b = color.b;
                    diffuseColor.a = color.a;
                    //std::cout<<"color : "<<(int) diffuseColor.r<<std::endl;
                }*/
                if (loadTextures) {
                    //system("PAUSE");
                    diffuseMaps = loadMaterialTextures(scene, material,
                                                        aiTextureType_DIFFUSE);
                    for (unsigned int i = 0; i < diffuseMaps.size(); i++) {
                        diffuseMaps[i]->setTexType(Material::DIFFUSE);
                        //subMesh.getMaterial().setTexture(diffuseMaps[i], Material::DIFFUSE, i);
                    }
                    specularMaps = loadMaterialTextures(scene, material,
                                                        aiTextureType_SPECULAR);
                    for (unsigned int i = 0; i < specularMaps.size(); i++) {
                        specularMaps[i]->setTexType(Material::SPECULAR);
                        //subMesh.getMaterial().setTexture(specularMaps[i], Material::SPECULAR, i);
                    }
                    normalMaps = loadMaterialTextures(scene, material, aiTextureType_NORMALS);
                    for (unsigned int i = 0; i < normalMaps.size(); i++) {
                        //std::cout<<"normals"<<std::endl;
                        normalMaps[i]->setTexType(Material::NORMAL);
                        //subMesh.getMaterial().setTexture(normalMaps[i], Material::NORMAL,i);
                    }
                    metalnessMaps = loadMaterialTextures(scene, material, aiTextureType_METALNESS);
                    for (unsigned int i = 0; i < metalnessMaps.size(); i++) {
                        //std::cout<<"metalness"<<std::endl;
                        metalnessMaps[i]->setTexType(Material::METALNESS);
                        //subMesh.getMaterial().setTexture(metalnessMaps[i], Material::METALNESS, i);
                    }
                    roughnessMaps = loadMaterialTextures(scene, material, aiTextureType_DIFFUSE_ROUGHNESS);
                    for (unsigned int i = 0; i < roughnessMaps.size(); i++) {
                        //std::cout<<"roughness"<<std::endl;
                        roughnessMaps[i]->setTexType(Material::ROUGHNESS);
                        //subMesh.getMaterial().setTexture(roughnessMaps[i], Material::ROUGHNESS, i);
                    }
                    aoMaps = loadMaterialTextures(scene, material, aiTextureType_AMBIENT_OCCLUSION);
                    for (unsigned int i = 0; i < aoMaps.size(); i++) {
                        //std::cout<<"ao"<<std::endl;
                        aoMaps[i]->setTexType(Material::AO);
                        //subMesh.getMaterial().setTexture(aoMaps[i], Material::AO, i);
                    }
                    emissiveMaps = loadMaterialTextures(scene, material, aiTextureType_EMISSIVE);
                    for (unsigned int i = 0; i < emissiveMaps.size(); i++) {
                        //std::cout<<"emissive"<<std::endl;
                        emissiveMaps[i]->setTexType(Material::EMISSIVE);
                        //subMesh.getMaterial().setTexture(emissiveMaps[i], Material::EMISSIVE, i);
                    }
                }

            }

            int upAxis = 1;
            int upSign = 1;
            int frontAxis = 2;    // default Z-forward
            int frontSign = 1;
            int coordAxis = 0;    // default X-right
            int coordSign = 1;
            if (scene->mMetaData) {
                /*for (unsigned int i = 0; i < scene->mMetaData->mNumProperties; ++i) {
                   std::cout << scene->mMetaData->mKeys[i].C_Str() << std::endl;
                }*/
                scene->mMetaData->Get("UpAxis", upAxis);
                scene->mMetaData->Get("UpAxisSign", upSign);
                scene->mMetaData->Get("FrontAxis", frontAxis);
                scene->mMetaData->Get("FrontAxisSign", frontSign);
                scene->mMetaData->Get("CoordAxis", coordAxis);
                scene->mMetaData->Get("CoordAxisSign", coordSign);
            }
            enum Axis { X = 0, Y = 1, Z = 2 };
            Axis sceneUpAxis    = (Axis)upAxis;
            Axis sceneFrontAxis = (Axis)frontAxis;
            Axis sceneCoordAxis = (Axis)coordAxis;
            int sceneUpSign     = upSign;
            int sceneFrontSign  = frontSign;
            int sceneCoordSign  = coordSign;
            bool isLeftHandled = false;
            float det = world.getDet();
            bool isLeftHanded = false;
            if (det < 0.0f) {
                //std::cout<<"left handed!"<<std::endl;
                isLeftHanded = true;
            }
            entity::TransformMatrix axisCorrection, handednessCorrection, scaleCorrection;
            axisCorrection.reset();
            handednessCorrection.reset();
            scaleCorrection.reset();
            entity::TransformMatrix  zUpyUp;
            zUpyUp.reset();
            // 1. Correction UP (Z-up → Y-up)
            if (sceneUpAxis == Axis::Z) {
                std::cout<<"up axis rotation"<<std::endl;
                zUpyUp.setRotation(math::Vec3f(1.f,0.f,0.f), -90.f);
            }
            // 2. Correction FRONT (Y-forward → -Z-forward)
            if (sceneFrontAxis == Axis::Y) {
                std::cout<<"front axis rotation"<<std::endl;
                axisCorrection.setRotation(math::Vec3f(0.f,0.f,1.f), 180.f);
            }

            axisCorrection.combine(zUpyUp.getMatrix());

            // 3. Correction handedness
            if (isLeftHanded) {
                axisCorrection.setScale(math::Vec3f(1.f,1.f,-1.f));
            }
            //std::cout<<"materials loaded : "<<mat.getTexture()<<std::endl;
            std::vector<Vertex> vertices;
            vertices.resize(mesh->mNumVertices);
            /*unsigned int uvCount = 0;
            for (unsigned int i = 0; i < AI_MAX_NUMBER_OF_TEXTURECOORDS; i++) {
                if (mesh->mTextureCoords[i] != nullptr)
                    uvCount++;
            }
            std::cout<<"nb uvs : "<<uvCount<<std::endl;*/
            for (unsigned int i = 0; i < mesh->mNumVertices; i++)
            {
                setVertexBoneDataToDefault(vertices[i]);
                vertices[i].position[0] = mesh->mVertices[i].x;
                vertices[i].position[1] = mesh->mVertices[i].y;
                vertices[i].position[2] = mesh->mVertices[i].z;
                if (mesh->mTextureCoords[0])
                {
                    vertices[i].texCoords[0] = mesh->mTextureCoords[0][i].x/* * mat.getTexture()->getSize().x()*/;
                    vertices[i].texCoords[1] = mesh->mTextureCoords[0][i].y/* * mat.getTexture()->getSize().y()*/;
                    /*if (vertices[i].texCoords[0] == 0 && vertices[i].texCoords[1] == 0) {
                        std::cout<<"error!"<<std::endl;
                    }*/
                    /*std::cout<<"tex coords : "<<vertices[i].texCoords[0]<<std::endl;
                    std::cout<<"tex coords : "<<vertices[i].texCoords[1]<<std::endl;*/
                } else {
                    vertices[i].texCoords = math::Vec2f(0.f, 0.f);
                }
                vertices[i].normal[0] = mesh->mNormals[i].x;
                vertices[i].normal[1] = mesh->mNormals[i].y;
                vertices[i].normal[2] = mesh->mNormals[i].z;
                vertices[i].color = Color::White;
            }
            extractBoneWeightForVertices(vertices, mesh, scene, mnode);
            /*VertexBuffer vb(device, Triangles);
            for (unsigned int i = 0; i < vertices.size(); i++) {
                //std::cout<<"add vertex"<<std::endl;
                vb.append(vertices[i]);
            }
            physic::BoundingBox bounds = vb.getBounds();*/

            /*if (bounds.getSize().x() > 1000 && bounds.getSize().y() > 1000 && bounds.getSize().z() > 1000) {
                //std::cout<<"scale correction."<<std::endl;
                scaleCorrection.setScale(math::Vec3f(0.01f, 0.01f, 0.01f));
            }
            math::Matrix4f finalCorrection = scaleCorrection.getMatrix() *
                                            axisCorrection.getMatrix() *
                                            handednessCorrection.getMatrix();
            math::Matrix4f finalTransform = world * finalCorrection;
            //std::cout<<"final transform : "<<finalTransform<<std::endl;
            if (!isSkinned) {
                for (unsigned int i = 0; i < vb.getVertexCount(); i++) {
                    vb[i].position = finalTransform * vb[i].position;
                    vb[i].normal = finalCorrection * -vb[i].normal;
                    //std::cout<<"vertex position : "<<vb[i].position<<std::endl;
                }
            }*/
            std::vector<uint32_t> indexes;
            //unsigned int baseIndex = 0;
            for(unsigned int i = 0; i < mesh->mNumFaces; i++)
            {
                aiFace face = mesh->mFaces[i];
                //std::cout<<"add face : "<<face.mNumIndices<<std::endl;

                for(unsigned int j = 0; j < face.mNumIndices; j++) {
                    indexes.push_back(face.mIndices[j]);
                    //std::cout<<"index : "<<face.mIndices[j]<<std::endl;
                    //vb.append(vertices[face.mIndices[j]]);                    
                    //vb.addIndex(face.mIndices[j]);
                }
                //baseIndex += face.mNumIndices;
            }
            
            /*math::Vec3f center = vb.getBounds().getCenter();
            math::Vec3f worldPos = finalTransform * center;

            std::cout << "Mesh world pos = " << worldPos << std::endl;*/
            //std::cout<<"vertices loaded"<<std::endl;
            //model->setSize(vb.getBounds().getSize());



            //std::cout<<"bone loaded"<<std::endl;




            //std::cout<<"vertices : "<<vb.getVertexCount()<<std::endl;
            //std::cout<<"indexes : "<<vb.getIndexCount()<<std::endl;
            /*if (mat.getTexture() == nullptr) {
                std::cout<<"material id : "<<mat.getId()<<std::endl;
            } else {
                //std::cout<<"texture"<<std::endl;
            }*/
            
            math::Vec3f currentSize = mnode->getSize();
            const size_t MAX_VERTS = 63;
            const size_t MAX_PRIMS = 21;
            for (unsigned int v = 0; v < vertices.size(); v += MAX_VERTS) {
                //std::cout<<"vertices : "<<vertices.size()<<std::endl;
                SubMesh subMesh(device);
                for (unsigned int i = 0; i < Material::NBTEXTYPES; i++)
                    subMesh.getMaterial().setTexture(nullptr, static_cast<Material::TexType>(i));
                for (unsigned int i = 0; i < diffuseMaps.size(); i++) {
                    //diffuseMaps[i]->setTexType(Material::DIFFUSE);
                    //std::cout<<"diffuse : "<<diffuseMaps[i]->getId()<<std::endl;
                    subMesh.getMaterial().setTexture(diffuseMaps[i], Material::DIFFUSE, i);
                }
                for (unsigned int i = 0; i < specularMaps.size(); i++) {
                    //specularMaps[i]->setTexType(Material::SPECULAR);
                    subMesh.getMaterial().setTexture(specularMaps[i], Material::SPECULAR, i);
                }
                for (unsigned int i = 0; i < normalMaps.size(); i++) {
                    //std::cout<<"normals"<<std::endl;
                    //normalMaps[i]->setTexType(Material::NORMAL);
                    //std::cout<<"specular : "<<specularMaps[i]->getId()<<std::endl;
                    subMesh.getMaterial().setTexture(normalMaps[i], Material::NORMAL,i);
                }                
                for (unsigned int i = 0; i < metalnessMaps.size(); i++) {
                    //std::cout<<"metalness"<<std::endl;
                    //metalnessMaps[i]->setTexType(Material::METALNESS);
                    subMesh.getMaterial().setTexture(metalnessMaps[i], Material::METALNESS, i);
                }                
                for (unsigned int i = 0; i < roughnessMaps.size(); i++) {
                    //std::cout<<"roughness"<<std::endl;
                    //roughnessMaps[i]->setTexType(Material::ROUGHNESS);
                    subMesh.getMaterial().setTexture(roughnessMaps[i], Material::ROUGHNESS, i);
                }                
                for (unsigned int i = 0; i < aoMaps.size(); i++) {
                    //std::cout<<"ao"<<std::endl;
                    //aoMaps[i]->setTexType(Material::AO);
                    subMesh.getMaterial().setTexture(aoMaps[i], Material::AO, i);
                }                
                for (unsigned int i = 0; i < emissiveMaps.size(); i++) {
                    //std::cout<<"emissive"<<std::endl;
                    //emissiveMaps[i]->setTexType(Material::EMISSIVE);
                    subMesh.getMaterial().setTexture(emissiveMaps[i], Material::EMISSIVE, i);
                }
                size_t vertexCount = std::min(MAX_VERTS, vertices.size()  - v);
                //std::cout<<"vertex count : "<<vertexCount<<std::endl;                
                size_t indexOffset = (v / MAX_VERTS) * MAX_PRIMS * 3;               
                size_t indexCount  = std::min(MAX_PRIMS * 3, indexes.size() - indexOffset);
                //std::cout<<"index count : "<<indexOffset<<","<<indexCount<<std::endl;
                /*if (indexOffset > indexes.size()) {
                    std::cout<<"indexs offset : "<<indexOffset<<","<<indexes.size()<<std::endl;
                }*/
                VertexBuffer vb(device, Triangles);
                vb.resize(vertexCount, indexCount);                
                for (unsigned int i = 0; i < vertexCount; i++) {
                    //std::cout<<"vertex : "<<vertices[v+i].position<<std::endl;
                    vb[i] = vertices[v+i];
                }
                for (unsigned int i = 0; i < indexCount; i++) {
                    //std::cout<<"index : "<<indexes[indexOffset + i]<<std::endl;
                    vb.setIndex(i, indexes[indexOffset + i]-v);                    
                }
                physic::BoundingBox bounds = vb.getBounds();
                //std::cout<<"bounds : "<<bounds.getSize()<<std::endl;
                if (bounds.getSize().x() > 1000 && bounds.getSize().y() > 1000 && bounds.getSize().z() > 1000) {
                    //std::cout<<"scale correction."<<std::endl;
                    scaleCorrection.setScale(math::Vec3f(0.01f, 0.01f, 0.01f));
                }
                math::Matrix4f finalCorrection = scaleCorrection.getMatrix() *
                                                axisCorrection.getMatrix() *
                                                handednessCorrection.getMatrix();
                math::Matrix4f finalTransform = world * finalCorrection;
                //std::cout<<"final transform : "<<finalTransform<<std::endl;
                
                if (vb.getBounds().getSize().x() > currentSize.x())
                    currentSize.x() = vb.getBounds().getSize().x();
                if (vb.getBounds().getSize().y() > currentSize.y())
                    currentSize.y() = vb.getBounds().getSize().y();
                if (vb.getBounds().getSize().z() > currentSize.z())
                    currentSize.z() = vb.getBounds().getSize().z();
                if (!isSkinned) {
                    for (unsigned int i = 0; i < vb.getVertexCount(); i++) {
                        vb[i].position = finalTransform * vb[i].position;
                        vb[i].normal = finalCorrection * -vb[i].normal;
                        //std::cout<<"vertex position : "<<vb[i].position<<std::endl;
                    }
                }
                //std::cout<<"size : "<<currentSize<<std::endl;                
                subMesh.setVertexBuffer(vb);
                std::lock_guard<std::recursive_mutex>(getGlobalMutex());
                mnode->addSubMesh(std::move(subMesh));
                
            }     
            std::lock_guard<std::recursive_mutex>(getGlobalMutex());
            mnode->setSize(currentSize);       
            
            //std::cout<<"size : "<<vb.getBounds().getSize()<<std::endl;
            /*entity::TransformMatrix tm;
            tm.setMatrix(finalTransform);
            mnode->setTransform(tm);*/
          
        }
        void ModelLoader::setVertexBoneDataToDefault(Vertex& vertex)
        {
            for (int i = 0; i < MAX_BONES_INFLUENCE; i++)
            {
                vertex.m_BoneIDs[i] = -1;
                vertex.m_Weights[i] = 0.0f;
            }
        }
        void ModelLoader::setVertexBoneData(Vertex& vertex, int boneID, float weight)
        {
            for (int i = 0; i < MAX_BONES_INFLUENCE; ++i)
            {
                if (vertex.m_BoneIDs[i] < 0)
                {
                    vertex.m_Weights[i] = weight;
                    vertex.m_BoneIDs[i] = boneID;
                    //std::cout<<"set vertex bone id : "<<vertex.m_BoneIDs[i]<<std::endl<<",weight : "<<vertex.m_Weights[i]<<std::endl;
                    break;
                }
            }
        }
        void ModelLoader::extractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene, Model* model)
        {
            auto& boneInfoMap = model->getBoneInfoMap();
            int& boneCount = model->getBoneCount();
            for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
            {
                int boneID = -1;
                std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
                if (model->getBoneInfoMap().find(boneName) == model->getBoneInfoMap().end())
                {
                    GameObject::BoneInfo newBoneInfo;
                    newBoneInfo.id = boneCount;
                    newBoneInfo.offset = AssimpHelpers::convertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
                    //std::cout<<"offset : "<<newBoneInfo.offset.transpose()<<std::endl;
                    boneInfoMap[boneName] = newBoneInfo;
                    boneID = boneCount;;
                    boneCount++;
                }
                else
                {
                    boneID = boneInfoMap[boneName].id;
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
        Material::TexType ModelLoader::convertAssimpType(aiTextureType type) {
            switch(type) {
                case aiTextureType_DIFFUSE:  return Material::DIFFUSE;
                case aiTextureType_SPECULAR: return Material::SPECULAR;
                case aiTextureType_NORMALS:  return Material::NORMAL;
                case aiTextureType_METALNESS: return Material::METALNESS;
                case aiTextureType_DIFFUSE_ROUGHNESS: return Material::ROUGHNESS;
                case aiTextureType_AMBIENT_OCCLUSION: return Material::AO;
                case aiTextureType_EMISSIVE: return Material::EMISSIVE;
                default: return Material::UNKNOWN;
            }
        }
        std::vector<Texture*> ModelLoader::loadMaterialTextures(const aiScene* scene, aiMaterial *mat, aiTextureType type)
        {
            std::lock_guard<std::recursive_mutex> lock(getGlobalMutex());
            std::vector<Texture*> textures;
            //std::cout<<"nb textures to load : "<<mat->GetTextureCount(type)<<std::endl;
            for(unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
                aiString str;
                mat->GetTexture(type, i, &str);
                std::string path = std::string (str.C_Str());
                if (path.length() > 0 && path.at(0) != '*' && directory != "") {
                    while (path.find("..\\") != std::string::npos) {
                        path.erase(0, 3);
                    }
                    path = directory + "/" + path;
                }
                std::replace(path.begin(), path.end(), '\\', '/');
                bool skip = false;
                if (textureManager.existFromAlias(path)) {
                    //std::cout<<"exist!"<<std::endl;
                    Texture* texture = textureManager.getResourceByAlias(path);
                    textures.push_back(texture);
                    skip = true;
                }
                if (!skip) {
                    //std::cout<<"nb textures : "<<textureManager.getAliases().size()<<std::endl;
                    Texture* texture = new Texture(device);
                    aiTextureMapMode wrapU=aiTextureMapMode_Wrap, wrapV=aiTextureMapMode_Wrap;
                    mat->Get(AI_MATKEY_MAPPINGMODE_U(type, i), wrapU);
                    mat->Get(AI_MATKEY_MAPPINGMODE_V(type, i), wrapV);
                    VkSamplerAddressMode vkWrapU, vkWrapV;
                    switch (wrapU) {
                        case aiTextureMapMode_Wrap:   vkWrapU = VK_SAMPLER_ADDRESS_MODE_REPEAT;          break;
                        case aiTextureMapMode_Clamp:  vkWrapU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;   break;
                        case aiTextureMapMode_Decal:  vkWrapU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER; break;
                        case aiTextureMapMode_Mirror: vkWrapU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT; break;
                        default:                      vkWrapU = VK_SAMPLER_ADDRESS_MODE_REPEAT;          break;
                    }
                    switch (wrapV) {
                        case aiTextureMapMode_Wrap:   vkWrapV = VK_SAMPLER_ADDRESS_MODE_REPEAT;          break;
                        case aiTextureMapMode_Clamp:  vkWrapV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;   break;
                        case aiTextureMapMode_Decal:  vkWrapV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER; break;
                        case aiTextureMapMode_Mirror: vkWrapV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT; break;
                        default: vkWrapV = VK_SAMPLER_ADDRESS_MODE_REPEAT;          break;
                    }
                    texture->setSamplerAddressMode(vkWrapU, vkWrapV);
                    //texture->setTexType(convertAssimpType(type));
                    //texture->create(imageLoader.getSize().x(), imageLoader.getSize().y());
                    textures.push_back(texture);
                    //std::lock_guard<std::recursive_mutex> lock(getGlobalMutex());
                    textureManager.make_resource(texture, path);
                }
                //}
            }

            //std::cout<<"loading images for one mesh texture type time : "<<clk.getElapsedTime().asMilliseconds()<<"ms"<<std::endl;
            return textures;
        }
    }
}