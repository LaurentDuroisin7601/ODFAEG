#include "../../../include/odfaeg/Graphics/perPixelLinkedListRenderComponent2.hpp"
#include "../../../include/odfaeg/Physics/particuleSystem.h"
#include "../../../include/odfaeg/Graphics/application.h"
#ifndef VULKAN
#include "glCheck.h"
#endif
namespace odfaeg {
    namespace graphic {
        #ifdef VULKAN
        #else
            PerPixelLinkedListRenderComponent2::PerPixelLinkedListRenderComponent2(RenderWindow& window, int layer, std::string expression, window::ContextSettings settings) :
            HeavyComponent(window, math::Vec3f(window.getView().getPosition().x, window.getView().getPosition().y, layer),
                          math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0),
                          math::Vec3f(window.getView().getSize().x + window.getView().getSize().x * 0.5f, window.getView().getPosition().y + window.getView().getSize().y * 0.5f, layer)),
            view(window.getView()),
            expression(expression),
            quad(math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, window.getSize().y * 0.5f)),
            layer(layer) {
                if (!(settings.versionMajor >= 4 && settings.versionMinor >= 6))
                    throw core::Erreur(53, "opengl version not supported for this renderer type");
                quad.move(math::Vec3f(-window.getView().getSize().x * 0.5f, -window.getView().getSize().y * 0.5f, 0));
                maxNodes = 20 * window.getView().getSize().x * window.getView().getSize().y;
                GLint nodeSize = 5 * sizeof(GLfloat) + sizeof(GLuint);
                frameBuffer.create(window.getView().getSize().x, window.getView().getSize().y, settings);
                frameBufferSprite = Sprite(frameBuffer.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), sf::IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));
                frameBuffer.setView(view);
                resolution = sf::Vector3i((int) window.getSize().x, (int) window.getSize().y, window.getView().getSize().z);
                //window.setActive();
                glCheck(glGenBuffers(1, &atomicBuffer));
                glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicBuffer));
                glCheck(glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW));
                glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0));
                glCheck(glGenBuffers(1, &linkedListBuffer));
                glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, linkedListBuffer));
                glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, maxNodes * nodeSize, nullptr, GL_DYNAMIC_DRAW));
                glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
                glCheck(glGenTextures(1, &headPtrTex));
                glCheck(glBindTexture(GL_TEXTURE_2D, headPtrTex));
                glCheck(glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, window.getView().getSize().x, window.getView().getSize().y));
                glCheck(glBindImageTexture(0, headPtrTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI));
                glCheck(glBindTexture(GL_TEXTURE_2D, 0));
                std::vector<GLuint> headPtrClearBuf(window.getView().getSize().x*window.getView().getSize().y, 0xffffffff);
                glCheck(glGenBuffers(1, &clearBuf));
                glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf));
                glCheck(glBufferData(GL_PIXEL_UNPACK_BUFFER, headPtrClearBuf.size() * sizeof(GLuint),
                &headPtrClearBuf[0], GL_STATIC_COPY));
                glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
                //std::cout<<"buffers : "<<atomicBuffer<<" "<<linkedListBuffer<<" "<<headPtrTex<<" "<<clearBuf<<std::endl;
                core::FastDelegate<bool> signal (&PerPixelLinkedListRenderComponent2::needToUpdate, this);
                core::FastDelegate<void> slot (&PerPixelLinkedListRenderComponent2::drawNextFrame, this);
                core::Command cmd(signal, slot);
                getListener().connect("UPDATE", cmd);
                glCheck(glGenBuffers(1, &vboIndirect));
                compileShaders();
                std::vector<Texture*> allTextures = Texture::getAllTextures();
                Samplers allSamplers{};
                std::vector<math::Matrix4f> textureMatrices;
                for (unsigned int i = 0; i < allTextures.size(); i++) {
                    textureMatrices.push_back(allTextures[i]->getTextureMatrix());
                    GLuint64 handle_texture = allTextures[i]->getTextureHandle();
                    allTextures[i]->makeTextureResident(handle_texture);
                    allSamplers.tex[i].handle = handle_texture;
                    //std::cout<<"add texture i : "<<i<<" id : "<<allTextures[i]->getId()<<std::endl;
                }
                perPixelLinkedList.setParameter("textureMatrix", textureMatrices);
                glCheck(glGenBuffers(1, &ubo));
                unsigned int ubid;
                glCheck(ubid = glGetUniformBlockIndex(perPixelLinkedList.getHandle(), "ALL_TEXTURES"));
                glCheck(glUniformBlockBinding(perPixelLinkedList.getHandle(),    ubid, 0));
                backgroundColor = sf::Color::Transparent;
                glCheck(glBindBuffer(GL_UNIFORM_BUFFER, ubo));
                glCheck(glBufferData(GL_UNIFORM_BUFFER, sizeof(Samplers),allSamplers.tex, GL_STATIC_DRAW));
                glCheck(glBindBuffer(GL_UNIFORM_BUFFER, 0));
                //std::cout<<"size : "<<sizeof(Samplers)<<" "<<alignof (alignas(16) uint64_t[200])<<std::endl;
                backgroundColor = sf::Color::Transparent;
                glCheck(glGenBuffers(1, &entityData));
                glCheck(glGenBuffers(1, &vertexData));
                glCheck(glGenBuffers(1, &materialData));
                glCheck(glGenBuffers(1, &entityIdData));
                std::vector<MaterialData> materialsDatas;
                materialsDatas.resize(Material::getNbMaterials());
                for (unsigned int i = 0; i < Material::getNbMaterials(); i++) {
                    Material *material = Material::getSameMaterials()[i];
                    MaterialData md;
                    md.textureId = (material->getTexture() != nullptr) ? material->getTexture()->getId() : 0;
                    materialsDatas[material->getId()] = md;
                }
                glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialData));
                glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, materialsDatas.size() * sizeof(MaterialData), &materialsDatas[0], GL_STATIC_DRAW));
                glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
                glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicBuffer));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, linkedListBuffer));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, materialData));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, entityData));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, vertexData));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, entityIdData));
                quadVBO.setPrimitiveType(sf::Quads);
                Vertex v1 (sf::Vector3f(0, 0, quad.getSize().z));
                Vertex v2 (sf::Vector3f(quad.getSize().x,0, quad.getSize().z));
                Vertex v3 (sf::Vector3f(quad.getSize().x, quad.getSize().y, quad.getSize().z));
                Vertex v4 (sf::Vector3f(0, quad.getSize().y, quad.getSize().z));
                quadVBO.append(v1);
                quadVBO.append(v2);
                quadVBO.append(v3);
                quadVBO.append(v4);
                quadVBO.update();
                update  = false;
            }
            void PerPixelLinkedListRenderComponent2::preloadEntitiesOnComponent(std::vector<Entity*> entities, EntityFactory& factory) {
                std::vector<VertexData> verticesDatas;
                std::vector<EntityData> entitiesDatas;
                entitiesDatas.resize(factory.getNbEntities());
                unsigned int vertexOffset = 0;
                for (unsigned int e = 0; e < entities.size(); e++) {
                    if (entities[e]->getFaces().size() > 0) {
                        EntityData entityData;
                        entities[e]->getTransform().update();
                        entityData.transformMatrix = entities[e]->getTransform().getMatrix().transpose();
                        entityData.materialID = entities[e]->getFaces()[0].getMaterial().getId();
                        entityData.vertexOffset = vertexOffset;
                        entitiesDatas[entities[e]->getId()] = entityData;
                        for (unsigned int f = 0; f < entities[e]->getNbFaces(); f++) {
                            Face face = entities[e]->getFaces()[f];
                            for (unsigned int v = 0; v < face.getVertexArray().getVertexCount(); v++) {
                                Vertex vertex = face.getVertexArray()[v];
                                VertexData vertexData;
                                vertexData.position = math::Vec3f(vertex.position.x, vertex.position.y, vertex.position.z);
                                vertexData.color = math::Vec3f(1.f / 255.f * vertex.color.r, 1.f / 255.f * vertex.color.g, 1.f / 255.f * vertex.color.b, 1.f / 255.f * vertex.color.a);
                                vertexData.texCoords = math::Vec3f(vertex.texCoords.x, vertex.texCoords.y, 0);
                                verticesDatas.push_back(vertexData);
                                vertexOffset++;
                            }
                        }
                    }
                }
                glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, entityData));
                glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, entitiesDatas.size() * sizeof(EntityData), &entitiesDatas[0], GL_DYNAMIC_DRAW));
                glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexData));
                glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, verticesDatas.size() * sizeof(VertexData), &verticesDatas[0], GL_DYNAMIC_DRAW));
                glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
            }
            void PerPixelLinkedListRenderComponent2::compileShaders() {
                const std::string skyboxVertexShader = R"(#version 460
                                                         layout (location = 0) in vec3 aPos;
                                                         out vec3 texCoords;
                                                         uniform mat4 projection;
                                                         uniform mat4 view;
                                                         void main()
                                                         {
                                                             texCoords = aPos;
                                                             gl_Position = projection * view * vec4(aPos, 1.0);
                                                         }
                                                         )";
                const std::string  simpleVertexShader = R"(#version 460
                                                        layout (location = 0) in vec3 position;
                                                        layout (location = 1) in vec4 color;
                                                        layout (location = 2) in vec2 texCoords;
                                                        layout (location = 3) in vec3 normals;
                                                        uniform mat4 projectionMatrix;
                                                        uniform mat4 viewMatrix;
                                                        uniform mat4 worldMat;
                                                        void main () {
                                                            gl_Position = projectionMatrix * viewMatrix * worldMat * vec4(position, 1.f);
                                                        })";
                const std::string vertexShader = R"(#version 460
                                                    uniform mat4 projectionMatrix;
                                                    uniform mat4 viewMatrix;
                                                    uniform mat4 textureMatrix[)"+core::conversionUIntString(Texture::getAllTextures().size())+R"(];
                                                    out vec2 fTexCoords;
                                                    out vec4 frontColor;
                                                    out uint texIndex;
                                                    struct EntityData {
                                                       mat4 transformMatrix;
                                                       uint materialId;
                                                       uint vertexOffset;
                                                    };
                                                    struct VertexData {
                                                        vec4 position;
                                                        vec4 color;
                                                        vec4 texCoords;
                                                    };
                                                    struct MaterialData {
                                                        uint textureId;
                                                    };
                                                    struct EntityIdData {
                                                        uint entityId;
                                                    };
                                                    layout(binding = 1, std430) buffer materialLists {
                                                          MaterialData materialsDatas[];
                                                    };
                                                    layout(binding = 2, std430) buffer entityLists {
                                                          EntityData entitiesDatas[];
                                                    };
                                                    layout(binding = 3, std430) buffer vertexLists {
                                                          VertexData verticesDatas[];
                                                    };
                                                    layout(binding = 4, std430) buffer entityIdLists {
                                                          EntityIdData entitiesIdsDatas[];
                                                    };
                                                    void main() {
                                                        uint vertexOffset = entitiesDatas[entitiesIdsDatas[gl_DrawID].entityId].vertexOffset;
                                                        mat4 modelMatrix = entitiesDatas[entitiesIdsDatas[gl_DrawID].entityId].transformMatrix;
                                                        uint materialId = entitiesDatas[entitiesIdsDatas[gl_DrawID].entityId].materialId;
                                                        uint textureIndex = materialsDatas[materialId].textureId;
                                                        VertexData vertex = verticesDatas[vertexOffset + gl_VertexID];
                                                        gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertex.position;
                                                        frontColor = vertex.color;
                                                        fTexCoords = (textureIndex != 0) ? (textureMatrix[textureIndex-1] * vertex.texCoords).xy : vec2(0, 0);
                                                        texIndex = textureIndex;
                                                    }
                                                    )";
            const std::string skyboxFragmentShader = R"(#version 460
                                                    layout (location = 0) out vec4 fcolor;
                                                    in vec3 texCoords;
                                                    uniform samplerCube skybox;
                                                    void main() {
                                                        fcolor = texture(skybox, texCoords);
                                                    }
                                                    )";
                                                    const std::string fragmentShader = R"(#version 460
                                                      #extension GL_ARB_bindless_texture : enable
                                                      struct NodeType {
                                                          vec4 color;
                                                          float depth;
                                                          uint next;
                                                      };
                                                      layout(binding = 0, offset = 0) uniform atomic_uint nextNodeCounter;
                                                      layout(binding = 0, r32ui) uniform uimage2D headPointers;
                                                      layout(binding = 0, std430) buffer linkedLists {
                                                          NodeType nodes[];
                                                      };
                                                      layout(std140, binding = 0) uniform ALL_TEXTURES {
                                                          sampler2D textures[200];
                                                      };
                                                      uniform uint maxNodes;
                                                      uniform sampler2D currentTex;
                                                      uniform float water;
                                                      in vec4 frontColor;
                                                      in vec2 fTexCoords;
                                                      in flat uint texIndex;
                                                      layout (location = 0) out vec4 fcolor;
                                                      /* fix: because of layout std140 16byte alignment, get uvec2 from array of uvec4 */
                                                      /*uvec2 GetTexture(uint index)
                                                      {
                                                          uint index_corrected = index / 2;
                                                          if (index % 2 == 0)
                                                              return maps[index_corrected].xy;
                                                          return maps[index_corrected].zw;
                                                      }*/
                                                      void main() {
                                                           uint nodeIdx = atomicCounterIncrement(nextNodeCounter);
                                                           //sampler2D tex = sampler2D(GetTexture(texIndex-1));
                                                           vec4 color = (texIndex != 0) ? frontColor * texture(textures[texIndex-1], fTexCoords.xy) : frontColor;
                                                           if (nodeIdx < maxNodes) {
                                                                uint prevHead = imageAtomicExchange(headPointers, ivec2(gl_FragCoord.xy), nodeIdx);
                                                                nodes[nodeIdx].color = color;
                                                                nodes[nodeIdx].depth = gl_FragCoord.z;
                                                                nodes[nodeIdx].next = prevHead;
                                                           }
                                                           //fcolor = vec4(0, 0, 0, 0);
                                                      })";
                 const std::string fragmentShader2 =
                   R"(
                   #version 460
                   #define MAX_FRAGMENTS 20
                   struct NodeType {
                      vec4 color;
                      float depth;
                      uint next;
                   };
                   layout(binding = 0, r32ui) uniform uimage2D headPointers;
                   layout(binding = 0, std430) buffer linkedLists {
                       NodeType nodes[];
                   };
                   layout(location = 0) out vec4 fcolor;
                   void main() {
                      NodeType frags[MAX_FRAGMENTS];
                      int count = 0;
                      uint n = imageLoad(headPointers, ivec2(gl_FragCoord.xy)).r;
                      while( n != 0xffffffffu && count < MAX_FRAGMENTS) {
                           frags[count] = nodes[n];
                           n = frags[count].next;
                           count++;
                      }
                      //merge sort
                      /*int i, j1, j2, k;
                      int a, b, c;
                      int step = 1;
                      NodeType leftArray[MAX_FRAGMENTS/2]; //for merge sort
                      //NodeType fgs[2];
                      while (step <= count)
                      {
                          i = 0;
                          while (i < count - step)
                          {
                              ////////////////////////////////////////////////////////////////////////
                              //merge(step, i, i + step, min(i + step + step, count));
                              a = i;
                              b = i + step;
                              c = (i + step + step) >= count ? count : (i + step + step);
                              for (k = 0; k < step; k++)
                                  leftArray[k] = frags[a + k];
                              j1 = 0;
                              j2 = 0;
                              for (k = a; k < c; k++)
                              {

                                  if (b + j1 >= c || (j2 < step && leftArray[j2].depth > frags[b + j1].depth))
                                      frags[k] = leftArray[j2++];
                                  else
                                      frags[k] = frags[b + j1++];
                                  //bool idx = (b + j1 >= c || (j2 < step && leftArray[j2].depth > frags[b + j1].depth));
                                  //fgs[1] = leftArray[j2++];
                                  //fgs[0] = frags[b + j1++];
                                  //frags[k] = fgs[int(idx)];
                              }
                              ////////////////////////////////////////////////////////////////////////
                              i += 2 * step;
                          }
                          step *= 2;
                      }*/
                      //Insertion sort.
                      for (int i = 0; i < count - 1; i++) {
                        for (int j = i + 1; j > 0; j--) {
                            if (frags[j - 1].depth > frags[j].depth) {
                                NodeType tmp = frags[j - 1];
                                frags[j - 1] = frags[j];
                                frags[j] = tmp;
                            }
                        }
                      }
                      vec4 color = vec4(0, 0, 0, 0);
                      for( int i = 0; i < count; i++)
                      {
                        color.rgb = frags[i].color.rgb * frags[i].color.a + color.rgb * (1 - frags[i].color.a);
                        color.a = frags[i].color.a + color.a * (1 - frags[i].color.a);
                        //color = mix (color, frags[i].color, frags[i].color.a);
                      }
                      fcolor = color;
                   })";
                   if (!skyboxShader.loadFromMemory(skyboxVertexShader, skyboxFragmentShader)) {
                        throw core::Erreur(53, "Failed to load skybox shader");
                   }
                   if (!perPixelLinkedList.loadFromMemory(vertexShader, fragmentShader)) {
                        throw core::Erreur(54, "Failed to load per pixel linked list shader");
                   }
                   if (!perPixelLinkedListP2.loadFromMemory(simpleVertexShader, fragmentShader2)) {
                        throw core::Erreur(55, "Failed to load per pixel linked list pass 2 shader");
                   }
                   skyboxShader.setParameter("skybox", Shader::CurrentTexture);
                   perPixelLinkedList.setParameter("maxNodes", maxNodes);
                   perPixelLinkedList.setParameter("currentTex", Shader::CurrentTexture);
                   perPixelLinkedList.setParameter("resolution", resolution.x, resolution.y, resolution.z);
                   math::Matrix4f viewMatrix = getWindow().getDefaultView().getViewMatrix().getMatrix().transpose();
                   math::Matrix4f projMatrix = getWindow().getDefaultView().getProjMatrix().getMatrix().transpose();
                   perPixelLinkedListP2.setParameter("viewMatrix", viewMatrix);
                   perPixelLinkedListP2.setParameter("projectionMatrix", projMatrix);
            }
            void PerPixelLinkedListRenderComponent2::loadTextureIndexes() {
                compileShaders();
                std::vector<Texture*> allTextures = Texture::getAllTextures();
                Samplers allSamplers{};
                std::vector<math::Matrix4f> textureMatrices;
                for (unsigned int i = 0; i < allTextures.size(); i++) {
                    textureMatrices.push_back(allTextures[i]->getTextureMatrix());
                    GLuint64 handle_texture = allTextures[i]->getTextureHandle();
                    allTextures[i]->makeTextureResident(handle_texture);
                    allSamplers.tex[i].handle = handle_texture;
                    //std::cout<<"add texture i : "<<i<<" id : "<<allTextures[i]->getId()<<std::endl;
                }
                perPixelLinkedList.setParameter("textureMatrix", textureMatrices);
                glCheck(glBindBuffer(GL_UNIFORM_BUFFER, ubo));
                glCheck(glBufferData(GL_UNIFORM_BUFFER, sizeof(Samplers),allSamplers.tex, GL_STATIC_DRAW));
                glCheck(glBindBuffer(GL_UNIFORM_BUFFER, 0));
            }
            void PerPixelLinkedListRenderComponent2::setBackgroundColor(sf::Color color) {
                backgroundColor = color;
            }
            bool PerPixelLinkedListRenderComponent2::loadEntitiesOnComponent(std::vector<Entity*> visibleEntities) {

                sf::PrimitiveType p;
                for (unsigned int e = 0; e < visibleEntities.size(); e++) {
                    if (visibleEntities[e] != nullptr && visibleEntities[e]->getNbFaces() > 0) {
                        p = visibleEntities[e]->getFaces()[0].getVertexArray().getPrimitiveType();
                        pVisibleEntities[p].push_back(visibleEntities[e]);
                    }
                }
                std::vector<EntityIdData> entitiesIdDatas;
                for (unsigned int p = 0; p < pVisibleEntities.size(); p++) {
                    std::vector<Entity*> vEntities = pVisibleEntities[p];
                    for (unsigned int e = 0; e < vEntities.size(); e++) {
                        EntityIdData entityIdData;
                        entityIdData.entityId = vEntities[e]->getId();
                        entitiesIdDatas.push_back(entityIdData);
                    }
                }
                glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, entityIdData));
                glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, entitiesIdDatas.size() * sizeof(EntityIdData), &entitiesIdDatas[0], GL_DYNAMIC_DRAW));
                glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
                update = true;
                return true;
            }
            void PerPixelLinkedListRenderComponent2::clear() {
                frameBuffer.setActive();
                frameBuffer.clear(backgroundColor);
                //getWindow().setActive();
                GLuint zero = 0;
                glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicBuffer));
                glCheck(glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero));
                glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0));
                glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf));
                glCheck(glBindTexture(GL_TEXTURE_2D, headPtrTex));
                glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, view.getSize().x, view.getSize().y, GL_RED_INTEGER,
                GL_UNSIGNED_INT, NULL));
                glCheck(glBindTexture(GL_TEXTURE_2D, 0));
                glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));

                frameBuffer.resetGLStates();

                //getWindow().resetGLStates();

            }
            void PerPixelLinkedListRenderComponent2::loadSkybox(Entity* skybox) {
                this->skybox = skybox;
                for (unsigned int f = 0; f < skybox->getFaces().size(); f++) {
                    for (unsigned int v = 0; v < skybox->getFaces()[f].getVertexArray().getVertexCount(); v++) {
                        skyboxVBO.append(skybox->getFaces()[f].getVertexArray()[v]);
                    }
                }
                skyboxVBO.update();
            }
            void PerPixelLinkedListRenderComponent2::drawNextFrame() {
                float zNear = view.getViewport().getPosition().z;
                if (!view.isOrtho())
                    view.setPerspective(80, view.getViewport().getSize().x / view.getViewport().getSize().y, 0.1f, view.getViewport().getSize().z);
                math::Matrix4f viewMatrix = view.getViewMatrix().getMatrix().transpose();
                math::Matrix4f projMatrix = view.getProjMatrix().getMatrix().transpose();
                viewMatrix = math::Matrix4f(math::Matrix3f(viewMatrix));
                skyboxShader.setParameter("projection", projMatrix);
                skyboxShader.setParameter("view", viewMatrix);
                currentStates.blendMode = sf::BlendAlpha;
                currentStates.shader = &skyboxShader;
                currentStates.texture = (skybox == nullptr ) ? nullptr : &static_cast<g3d::Skybox*>(skybox)->getTexture();
                frameBuffer.drawVertexBuffer(skyboxVBO, currentStates);
                std::array<std::vector<DrawArraysIndirectCommand>, Batcher::nbPrimitiveTypes> drawArraysIndirectCommands;
                std::array<unsigned int, Batcher::nbPrimitiveTypes> firstIndex, baseInstance;
                for (unsigned int i = 0; i < firstIndex.size(); i++) {
                    firstIndex[i] = 0;
                }
                for (unsigned int i = 0; i < baseInstance.size(); i++) {
                    baseInstance[i] = 0;
                }
                for (unsigned int p = 0; p < pVisibleEntities.size(); p++) {
                    std::vector<Entity*> vEntities = pVisibleEntities[p];
                    for (unsigned int e = 0; e < vEntities.size(); e++) {
                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        unsigned int vertexCount = 0;
                        for (unsigned int f = 0; f < vEntities[e]->getNbFaces(); f++) {
                            for (unsigned int v = 0; v < vEntities[e]->getFaces()[f].getVertexArray().getVertexCount(); v++) {
                                vertexCount++;
                            }
                        }
                        drawArraysIndirectCommand.count = vertexCount;
                        drawArraysIndirectCommand.firstIndex = firstIndex[p];
                        drawArraysIndirectCommand.baseInstance = baseInstance[p];
                        drawArraysIndirectCommand.instanceCount = 1;
                        drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                        firstIndex[p] += vertexCount;
                        baseInstance[p] += 1;
                    }
                }
                projMatrix = view.getProjMatrix().getMatrix().transpose();
                viewMatrix = view.getViewMatrix().getMatrix().transpose();
                perPixelLinkedList.setParameter("projectionMatrix", projMatrix);
                perPixelLinkedList.setParameter("viewMatrix", viewMatrix);
                RenderStates currentStates;
                currentStates.blendMode = sf::BlendNone;
                currentStates.shader = &perPixelLinkedList;
                currentStates.texture = nullptr;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vboIndirect));
                    glCheck(glBufferData(GL_DRAW_INDIRECT_BUFFER, drawArraysIndirectCommands[p].size() * sizeof(DrawArraysIndirectCommand), &drawArraysIndirectCommands[p][0], GL_DYNAMIC_DRAW));
                    glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
                    frameBuffer.drawVBOBindlessIndirect(static_cast<sf::PrimitiveType>(p), drawArraysIndirectCommands[p].size(), currentStates, vboIndirect);
                }
                glCheck(glFinish());
                glCheck(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));
                math::Matrix4f matrix = quad.getTransform().getMatrix().transpose();
                perPixelLinkedListP2.setParameter("worldMat", matrix);
                currentStates.shader = &perPixelLinkedListP2;
                frameBuffer.drawVertexBuffer(quadVBO, currentStates);
                glCheck(glFinish());
                frameBuffer.display();
            }
            void PerPixelLinkedListRenderComponent2::draw(RenderTarget& target, RenderStates states) {
                frameBufferSprite.setCenter(target.getView().getPosition());
                target.draw(frameBufferSprite, states);
            }
            int  PerPixelLinkedListRenderComponent2::getLayer() {
                return layer;
            }
            void PerPixelLinkedListRenderComponent2::draw(Drawable& drawable, RenderStates states) {
                //drawables.insert(std::make_pair(drawable, states));
            }
            void PerPixelLinkedListRenderComponent2::setView(View view) {
                frameBuffer.setView(view);
                this->view = view;
            }
            std::string PerPixelLinkedListRenderComponent2::getExpression() {
                return expression;
            }
            View& PerPixelLinkedListRenderComponent2::getView() {
                return view;
            }
            bool PerPixelLinkedListRenderComponent2::needToUpdate() {
                return update;
            }
            void PerPixelLinkedListRenderComponent2::setExpression (std::string expression) {
                this->expression = expression;
            }
            void PerPixelLinkedListRenderComponent2::pushEvent(window::IEvent event, RenderWindow& rw) {
                if (event.type == window::IEvent::WINDOW_EVENT && event.window.type == window::IEvent::WINDOW_EVENT_RESIZED && &getWindow() == &rw && isAutoResized()) {
                    std::cout<<"recompute size"<<std::endl;
                    recomputeSize();
                    getListener().pushEvent(event);
                    getView().reset(physic::BoundingBox(getView().getViewport().getPosition().x, getView().getViewport().getPosition().y, getView().getViewport().getPosition().z, event.window.data1, event.window.data2, getView().getViewport().getDepth()));
                }
            }
            const Texture& PerPixelLinkedListRenderComponent2::getFrameBufferTexture() {
                return frameBuffer.getTexture();
            }
            RenderTexture* PerPixelLinkedListRenderComponent2::getFrameBuffer() {
                return &frameBuffer;
            }
            std::vector<Entity*> PerPixelLinkedListRenderComponent2::getEntities() {
                std::vector<Entity*> visibleEntities;
                for (unsigned int p = 0; p < pVisibleEntities.size(); p++) {
                    for (unsigned int e = 0; e < pVisibleEntities[p].size(); e++) {
                        visibleEntities.push_back(pVisibleEntities[p][e]);
                    }
                }
                return visibleEntities;
            }
            PerPixelLinkedListRenderComponent2::~PerPixelLinkedListRenderComponent2() {
                glDeleteBuffers(1, &atomicBuffer);
                glDeleteBuffers(1, &linkedListBuffer);
                glDeleteBuffers(1, &clearBuf);
                glDeleteTextures(1, &headPtrTex);
                glDeleteBuffers(1, &vboIndirect);
                glDeleteBuffers(1, &ubo);
            }
        #endif
    }
}
