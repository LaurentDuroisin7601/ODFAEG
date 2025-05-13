#include "../../../../include/odfaeg/Graphics/ECS/perPixelLinkedList.hpp"
//#include "../../../../include/odfaeg/Graphics/ECS/application.hpp"
#ifndef VULKAN
#include "../glCheck.h"
#endif
namespace odfaeg {
    namespace graphic {
        namespace ecs {
                #ifdef VULKAN
                #else
                PerPixelLinkedListRenderComponent::PerPixelLinkedListRenderComponent(RenderWindow& window, int layer, std::string expression, window::ContextSettings settings) :
                HeavyComponent(window, math::Vec3f(window.getView().getPosition().x, window.getView().getPosition().y, layer),
                              math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0),
                              math::Vec3f(window.getView().getSize().x + window.getView().getSize().x * 0.5f, window.getView().getPosition().y + window.getView().getSize().y * 0.5f, layer)),
                view(window.getView()),
                expression(expression),
                quad(math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, window.getSize().y * 0.5f)),
                layer(layer) {
                if (!(settings.versionMajor >= 4 && settings.versionMinor >= 6))
                    throw core::Erreur(53, "opengl version not supported for this renderer type");
                //std::cout<<"move quad"<<std::endl;
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
                core::FastDelegate<bool> signal (&PerPixelLinkedListRenderComponent::needToUpdate, this);
                core::FastDelegate<void> slot (&PerPixelLinkedListRenderComponent::drawNextFrame, this);
                core::Command cmd(signal, slot);
                getListener().connect("UPDATE", cmd);


                glCheck(glGenBuffers(1, &modelDataBuffer));
                glCheck(glGenBuffers(1, &materialDataBuffer));
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
                indirectRenderingShader.setParameter("textureMatrix", textureMatrices);
                glCheck(glGenBuffers(1, &ubo));

                backgroundColor = sf::Color::Transparent;
                glCheck(glBindBuffer(GL_UNIFORM_BUFFER, ubo));
                glCheck(glBufferData(GL_UNIFORM_BUFFER, sizeof(Samplers),allSamplers.tex, GL_STATIC_DRAW));
                glCheck(glBindBuffer(GL_UNIFORM_BUFFER, 0));
                //std::cout<<"size : "<<sizeof(Samplers)<<" "<<alignof (alignas(16) uint64_t[200])<<std::endl;
                backgroundColor = sf::Color::Transparent;
                glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicBuffer));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, linkedListBuffer));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, modelDataBuffer));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, materialDataBuffer));

                for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                    vbBindlessTex[i].setPrimitiveType(static_cast<sf::PrimitiveType>(i));
                }
                skybox = entt::null;
            }
            void PerPixelLinkedListRenderComponent::loadTextureIndexes() {
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
                indirectRenderingShader.setParameter("textureMatrix", textureMatrices);
                glCheck(glBindBuffer(GL_UNIFORM_BUFFER, ubo));
                glCheck(glBufferData(GL_UNIFORM_BUFFER, sizeof(Samplers),allSamplers.tex, GL_STATIC_DRAW));
                glCheck(glBindBuffer(GL_UNIFORM_BUFFER, 0));
            }
            void PerPixelLinkedListRenderComponent::compileShaders() {
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
                   const std::string indirectDrawVertexShader = R"(#version 460
                                                                #define M_PI 3.1415926535897932384626433832795
                                                                #define FPI M_PI/4
                                                                layout (location = 0) in vec3 position;
                                                                layout (location = 1) in vec4 color;
                                                                layout (location = 2) in vec2 texCoords;
                                                                layout (location = 3) in vec3 normals;
                                                                uniform mat4 projectionMatrix;
                                                                uniform mat4 viewMatrix;
                                                                uniform mat4 textureMatrix[)"+core::conversionUIntString(Texture::getAllTextures().size())+R"(];
                                                                uniform float time;
                                                                uniform vec3 resolution;
                                                                struct ModelData {
                                                                    mat4 modelMatrix;
                                                                };
                                                                struct MaterialData {
                                                                    uint textureIndex;
                                                                    uint materialType;
                                                                };
                                                                layout(binding = 1, std430) buffer modelData {
                                                                    ModelData modelDatas[];
                                                                };
                                                                layout(binding = 2, std430) buffer materialData {
                                                                    MaterialData materialDatas[];
                                                                };
                                                                out vec2 fTexCoords;
                                                                out vec4 frontColor;
                                                                out uint texIndex;
                                                                void main() {
                                                                    MaterialData materialData = materialDatas[gl_DrawID];
                                                                    ModelData modelData = modelDatas[gl_BaseInstance + gl_InstanceID];
                                                                    float xOff = 0;
                                                                    float yOff = 0;
                                                                    if (materialData.materialType == 1) {
                                                                        yOff = 0.05*sin(position.x*12+time*FPI)*resolution.y;
                                                                        xOff = 0.025*cos(position.x*12+time*FPI)*resolution.x;
                                                                    }
                                                                    uint textureIndex =  materialData.textureIndex;
                                                                    gl_Position = projectionMatrix * viewMatrix * modelData.modelMatrix * vec4((position.x - xOff), (position.y + yOff), position.z, 1.f);
                                                                    fTexCoords = (textureIndex != 0) ? (textureMatrix[textureIndex-1] * vec4(texCoords, 1.f, 1.f)).xy : texCoords;
                                                                    frontColor = color;
                                                                    texIndex = textureIndex;
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
                       if (!perPixelLinkedListP2.loadFromMemory(simpleVertexShader, fragmentShader2)) {
                            throw core::Erreur(55, "Failed to load per pixel linked list pass 2 shader");
                       }
                       if (!indirectRenderingShader.loadFromMemory(indirectDrawVertexShader, fragmentShader)) {
                           throw core::Erreur(57, "Failed to load indirect rendering shader");
                       }
                       skyboxShader.setParameter("skybox", Shader::CurrentTexture);
                       indirectRenderingShader.setParameter("maxNodes", maxNodes);
                       indirectRenderingShader.setParameter("currentTex", Shader::CurrentTexture);
                       indirectRenderingShader.setParameter("resolution", resolution.x, resolution.y, resolution.z);
                       math::Matrix4f viewMatrix = getWindow().getDefaultView().getViewMatrix().getMatrix().transpose();
                       math::Matrix4f projMatrix = getWindow().getDefaultView().getProjMatrix().getMatrix().transpose();
                       perPixelLinkedListP2.setParameter("viewMatrix", viewMatrix);
                       perPixelLinkedListP2.setParameter("projectionMatrix", projMatrix);
            }
            void PerPixelLinkedListRenderComponent::setBackgroundColor(sf::Color color) {
                backgroundColor = color;
            }
            void PerPixelLinkedListRenderComponent::clear() {
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
            void PerPixelLinkedListRenderComponent::drawSelectedInstances() {
                for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                    vbBindlessTex[i].clear();
                }
                std::array<std::vector<DrawArraysIndirectCommand>, Batcher::nbPrimitiveTypes> drawArraysIndirectCommands;
                std::array<std::vector<ModelData>, Batcher::nbPrimitiveTypes> matrices;
                std::array<std::vector<MaterialData>, Batcher::nbPrimitiveTypes> materials;
                std::array<unsigned int, Batcher::nbPrimitiveTypes> firstIndex, baseInstance;
                for (unsigned int i = 0; i < firstIndex.size(); i++) {
                    firstIndex[i] = 0;
                }
                for (unsigned int i = 0; i < baseInstance.size(); i++) {
                    baseInstance[i] = 0;
                }
                for (unsigned int i = 0; i < m_selected.size(); i++) {
                    if (m_selected[i].getAllVertices().getVertexCount() > 0) {
                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        //std::cout<<"next frame draw normal"<<std::endl;

                        float time = timeClock.getElapsedTime().asSeconds();
                        indirectRenderingShader.setParameter("time", time);

                        unsigned int p = m_selected[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_selected[i].getMaterial().getTexture() != nullptr) ? m_selected[i].getMaterial().getTexture()->getId() : 0;
                        material.materialType = m_selected[i].getMaterial().getType();
                        materials[p].push_back(material);

                        TransformMatrix tm;
                        ModelData model;
                        model.worldMat = tm.getMatrix().transpose();
                        matrices[p].push_back(model);
                        unsigned int vertexCount = 0;
                        for (unsigned int j = 0; j < m_selected[i].getAllVertices().getVertexCount(); j++) {
                            vbBindlessTex[p].append(m_selected[i].getAllVertices()[j]);
                            vertexCount++;
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
                for (unsigned int i = 0; i < m_selectedInstance.size(); i++) {
                    if (m_selectedInstance[i].getAllVertices().getVertexCount() > 0) {
                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        unsigned int p = m_selectedInstance[i].getAllVertices().getPrimitiveType();

                        float time = timeClock.getTimeClk().getElapsedTime().asSeconds();
                        indirectRenderingShader.setParameter("time", time);

                        MaterialData material;
                        material.textureIndex = (m_selectedInstance[i].getMaterial().getTexture() != nullptr) ? m_selectedInstance[i].getMaterial().getTexture()->getId() : 0;
                        material.materialType = m_selectedInstance[i].getMaterial().getType();
                        materials[p].push_back(material);

                        std::vector<TransformMatrix*> tm = m_selectedInstance[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            ModelData model;
                            model.worldMat = tm[j]->getMatrix().transpose();
                            matrices[p].push_back(model);
                        }
                        unsigned int vertexCount = 0;
                        if (m_selectedInstance[i].getVertexArrays().size() > 0) {
                            EntityId entity = m_selectedInstance[i].getVertexArrays()[0]->getEntityId();
                            for (unsigned int j = 0; j < m_selectedInstance[i].getVertexArrays().size(); j++) {
                                if (entity == m_selectedInstance[i].getVertexArrays()[j]->getEntityId()) {
                                    for (unsigned int k = 0; k < m_selectedInstance[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                        vertexCount++;
                                        vbBindlessTex[p].append((*m_selectedInstance[i].getVertexArrays()[j])[k]);
                                    }
                                }
                            }
                        }
                        drawArraysIndirectCommand.count = vertexCount;
                        drawArraysIndirectCommand.firstIndex = firstIndex[p];
                        drawArraysIndirectCommand.baseInstance = baseInstance[p];
                        drawArraysIndirectCommand.instanceCount = tm.size();
                        drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                        firstIndex[p] += vertexCount;
                        baseInstance[p] += tm.size();
                        //std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                        //std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;

                    }
                }
                currentStates.blendMode = sf::BlendNone;
                currentStates.shader = &indirectRenderingShader;
                currentStates.texture = nullptr;
                glCheck(glEnable(GL_STENCIL_TEST));
                glCheck(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE));
                glStencilFunc(GL_ALWAYS, 1, 0xFF);
                glStencilMask(0xFF);
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelDataBuffer));
                        glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, matrices[p].size() * sizeof(ModelData), &matrices[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialDataBuffer));
                        glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, materials[p].size() * sizeof(MaterialData), &materials[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vboIndirect));
                        glCheck(glBufferData(GL_DRAW_INDIRECT_BUFFER, drawArraysIndirectCommands[p].size() * sizeof(DrawArraysIndirectCommand), &drawArraysIndirectCommands[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
                        vbBindlessTex[p].update();
                        frameBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), currentStates, vboIndirect);
                        vbBindlessTex[p].clear();
                    }
                }
                for (unsigned int i = 0; i < firstIndex.size(); i++) {
                    firstIndex[i] = 0;
                }
                for (unsigned int i = 0; i < baseInstance.size(); i++) {
                    baseInstance[i] = 0;
                }
                for (unsigned int i = 0; i < matrices.size(); i++) {
                    matrices[i].clear();
                }
                for (unsigned int i = 0; i < materials.size(); i++) {
                    materials[i].clear();
                }
                for (unsigned int i = 0; i < drawArraysIndirectCommands.size(); i++) {
                    drawArraysIndirectCommands[i].clear();
                }
                for (unsigned int i = 0; i < m_selectedScale.size(); i++) {
                    if (m_selectedScale[i].getAllVertices().getVertexCount() > 0) {
                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        //std::cout<<"next frame draw normal"<<std::endl;
                        /*if (core::Application::app != nullptr) {
                            float time = core::Application::getTimeClk().getElapsedTime().asSeconds();
                            perPixelLinkedList2.setParameter("time", time);
                        }*/
                        unsigned int p = m_selectedScale[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = 0;
                        material.materialType = m_selectedScale[i].getMaterial().getType();
                        materials[p].push_back(material);
                        TransformMatrix tm;
                        ModelData model;
                        model.worldMat = tm.getMatrix().transpose();
                        matrices[p].push_back(model);

                        unsigned int vertexCount = 0;
                        for (unsigned int j = 0; j < m_selectedScale[i].getAllVertices().getVertexCount(); j++) {
                            vertexCount++;
                            vbBindlessTex[p].append(m_selectedScale[i].getAllVertices()[j], 0);
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
                for (unsigned int i = 0; i < m_selectedScaleInstance.size(); i++) {
                    unsigned int p = m_selectedScaleInstance[i].getAllVertices().getPrimitiveType();
                    if (m_selectedScaleInstance[i].getAllVertices().getVertexCount() > 0) {
                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        /*if (core::Application::app != nullptr) {
                            float time = core::Application::getTimeClk().getElapsedTime().asSeconds();
                            perPixelLinkedList.setParameter("time", time);
                        }*/
                        MaterialData material;
                        material.textureIndex = 0;
                        material.materialType = m_selectedScaleInstance[i].getMaterial().getType();
                        materials[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_selectedScaleInstance[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            ModelData model;
                            model.worldMat = tm[j]->getMatrix().transpose();
                            matrices[p].push_back(model);
                        }
                        unsigned int vertexCount = 0;
                        if (m_selectedScaleInstance[i].getVertexArrays().size() > 0) {
                            EntityId entity = m_selectedScaleInstance[i].getVertexArrays()[0]->getEntityId();
                            for (unsigned int j = 0; j < m_selectedScaleInstance[i].getVertexArrays().size(); j++) {
                                if (entity == m_selectedScaleInstance[i].getVertexArrays()[j]->getEntityId()) {
                                    for (unsigned int k = 0; k < m_selectedScaleInstance[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                        vertexCount++;
                                        vbBindlessTex[p].append((*m_selectedScaleInstance[i].getVertexArrays()[j])[k], 0);
                                    }
                                }
                            }
                        }
                        drawArraysIndirectCommand.count = vertexCount;
                        drawArraysIndirectCommand.firstIndex = firstIndex[p];
                        drawArraysIndirectCommand.baseInstance = baseInstance[p];
                        drawArraysIndirectCommand.instanceCount = tm.size();
                        drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                        firstIndex[p] += vertexCount;
                        baseInstance[p] += tm.size();
                        //std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                        //std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;

                    }
                }
                currentStates.blendMode = sf::BlendNone;
                currentStates.shader = &indirectRenderingShader;
                currentStates.texture = nullptr;
                glCheck(glStencilFunc(GL_NOTEQUAL, 1, 0xFF));
                glCheck(glStencilMask(0x00));
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelDataBuffer));
                        glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, matrices[p].size() * sizeof(ModelData), &matrices[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialDataBuffer));
                        glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, materials[p].size() * sizeof(MaterialData), &materials[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vboIndirect));
                        glCheck(glBufferData(GL_DRAW_INDIRECT_BUFFER, drawArraysIndirectCommands[p].size() * sizeof(DrawArraysIndirectCommand), &drawArraysIndirectCommands[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
                        vbBindlessTex[p].update();
                        frameBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), currentStates, vboIndirect);
                        vbBindlessTex[p].clear();
                    }
                }
                glCheck(glDisable(GL_STENCIL_TEST));
            }
            void PerPixelLinkedListRenderComponent::drawSelectedInstancesIndexed() {
                for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                    vbBindlessTex[i].clear();
                }
                std::array<std::vector<DrawElementsIndirectCommand>, Batcher::nbPrimitiveTypes> drawElementsIndirectCommands;
                std::array<std::vector<ModelData>, Batcher::nbPrimitiveTypes> matrices;
                std::array<std::vector<MaterialData>, Batcher::nbPrimitiveTypes> materials;
                std::array<unsigned int, Batcher::nbPrimitiveTypes> firstIndex, baseInstance, baseVertex;
                for (unsigned int i = 0; i < firstIndex.size(); i++) {
                    firstIndex[i] = 0;
                }
                for (unsigned int i = 0; i < baseVertex.size(); i++) {
                    baseVertex[i] = 0;
                }
                for (unsigned int i = 0; i < baseInstance.size(); i++) {
                    baseInstance[i] = 0;
                }
                for (unsigned int i = 0; i < m_selected.size(); i++) {
                    if (m_selected[i].getAllVertices().getVertexCount() > 0) {
                        DrawElementsIndirectCommand drawElementsIndirectCommand;
                        //std::cout<<"next frame draw normal"<<std::endl;

                        float time = timeClock.getTimeClk().getElapsedTime().asSeconds();
                        indirectRenderingShader.setParameter("time", time);

                        unsigned int p = m_selectedIndexed[i].getAllVertices().getPrimitiveType();

                        MaterialData material;
                        material.textureIndex = (m_selectedIndexed[i].getMaterial().getTexture() != nullptr) ? m_selectedIndexed[i].getMaterial().getTexture()->getId() : 0;
                        material.materialType = m_selectedIndexed[i].getMaterial().getType();
                        materials[p].push_back(material);

                        TransformMatrix tm;
                        ModelData model;
                        model.worldMat = tm.getMatrix().transpose();
                        matrices[p].push_back(model);
                        unsigned int vertexCount = 0, indexCount = 0;
                        for (unsigned int j = 0; j < m_selectedIndexed[i].getAllVertices().getVertexCount(); j++) {
                            vertexCount++;
                            vbBindlessTex[p].append(m_selectedIndexed[i].getAllVertices()[j]);
                        }
                        for (unsigned int j = 0; j < m_selectedIndexed[i].getAllVertices().getIndexes().size(); j++) {
                            indexCount++;
                            vbBindlessTex[p].addIndex(m_selectedIndexed[i].getAllVertices().getIndexes()[j]);
                        }
                        drawElementsIndirectCommand.index_count = indexCount;
                        drawElementsIndirectCommand.first_index = firstIndex[p];
                        drawElementsIndirectCommand.instance_base = baseInstance[p];
                        drawElementsIndirectCommand.vertex_base = baseVertex[p];
                        drawElementsIndirectCommand.instance_count = 1;
                        drawElementsIndirectCommands[p].push_back(drawElementsIndirectCommand);
                        firstIndex[p] += indexCount;
                        baseVertex[p] += vertexCount;
                        baseInstance[p] += 1;
                    }
                }
                for (unsigned int i = 0; i < m_selectedInstanceIndexed.size(); i++) {

                    if (m_selectedInstanceIndexed[i].getAllVertices().getVertexCount() > 0) {
                        DrawElementsIndirectCommand drawElementsIndirectCommand;
                        unsigned int p = m_selectedInstanceIndexed[i].getAllVertices().getPrimitiveType();
                        /*if (core::Application::app != nullptr) {
                            float time = core::Application::getTimeClk().getElapsedTime().asSeconds();
                            perPixelLinkedList.setParameter("time", time);
                        }*/
                        MaterialData material;
                        material.textureIndex = (m_selectedInstanceIndexed[i].getMaterial().getTexture() != nullptr) ? m_selectedInstanceIndexed[i].getMaterial().getTexture()->getId() : 0;
                        material.materialType = m_selectedInstanceIndexed[i].getMaterial().getType();
                        materials[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_selectedInstanceIndexed[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            ModelData model;
                            model.worldMat = tm[j]->getMatrix().transpose();
                            matrices[p].push_back(model);
                        }
                        unsigned int indexCount = 0, vertexCount = 0;
                        if (m_selectedInstanceIndexed[i].getVertexArrays().size() > 0) {
                            Entity* entity = m_selectedInstanceIndexed[i].getVertexArrays()[0]->getEntity();
                            for (unsigned int j = 0; j < m_selectedInstanceIndexed[i].getVertexArrays().size(); j++) {
                                if (entity == m_selectedInstanceIndexed[i].getVertexArrays()[j]->getEntity()) {

                                    for (unsigned int k = 0; k < m_selectedInstanceIndexed[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                        vertexCount++;
                                        vbBindlessTex[p].append((*m_selectedInstanceIndexed[i].getVertexArrays()[j])[k]);
                                    }
                                    for (unsigned int k = 0; k < m_selectedInstanceIndexed[i].getVertexArrays()[j]->getIndexes().size(); k++) {
                                        indexCount++;
                                        vbBindlessTex[p].addIndex(m_selectedInstanceIndexed[i].getVertexArrays()[j]->getIndexes()[k]);
                                    }
                                }
                            }
                        }
                        drawElementsIndirectCommand.index_count = indexCount;
                        drawElementsIndirectCommand.first_index = firstIndex[p];
                        drawElementsIndirectCommand.instance_base = baseInstance[p];
                        drawElementsIndirectCommand.vertex_base = baseVertex[p];
                        drawElementsIndirectCommand.instance_count = tm.size();
                        drawElementsIndirectCommands[p].push_back(drawElementsIndirectCommand);
                        firstIndex[p] += indexCount;
                        baseVertex[p] += vertexCount;
                        baseInstance[p] += tm.size();
                        //std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                        //std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;
                    }
                }
                currentStates.blendMode = sf::BlendNone;
                currentStates.shader = &indirectRenderingShader;
                currentStates.texture = nullptr;
                glCheck(glEnable(GL_STENCIL_TEST));
                glCheck(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE));
                glStencilFunc(GL_ALWAYS, 1, 0xFF);
                glStencilMask(0xFF);
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelDataBuffer));
                        glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, matrices[p].size() * sizeof(ModelData), &matrices[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialDataBuffer));
                        glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, materials[p].size() * sizeof(MaterialData), &materials[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vboIndirect));
                        glCheck(glBufferData(GL_DRAW_INDIRECT_BUFFER, drawElementsIndirectCommands[p].size() * sizeof(DrawElementsIndirectCommand), &drawElementsIndirectCommands[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
                        vbBindlessTex[p].update();
                        frameBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawElementsIndirectCommands[p].size(), currentStates, vboIndirect);
                        vbBindlessTex[p].clear();
                    }
                }
                for (unsigned int i = 0; i < firstIndex.size(); i++) {
                    firstIndex[i] = 0;
                }
                for (unsigned int i = 0; i < baseVertex.size(); i++) {
                    baseVertex[i] = 0;
                }
                for (unsigned int i = 0; i < baseInstance.size(); i++) {
                    baseInstance[i] = 0;
                }
                for (unsigned int i = 0; i < matrices.size(); i++) {
                    matrices[i].clear();
                }
                for (unsigned int i = 0; i < materials.size(); i++) {
                    materials[i].clear();
                }
                for (unsigned int i = 0; i < drawElementsIndirectCommands.size(); i++) {
                    drawElementsIndirectCommands[i].clear();
                }
                for (unsigned int i = 0; i < m_selectedScaleIndexed.size(); i++) {
                    if (m_selectedScaleIndexed[i].getAllVertices().getVertexCount() > 0) {
                        DrawElementsIndirectCommand drawElementsIndirectCommand;
                        //std::cout<<"next frame draw normal"<<std::endl;
                        unsigned int p = m_selectedScaleIndexed[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = 0;
                        material.materialType = m_selectedScaleIndexed[i].getMaterial().getType();
                        materials[p].push_back(material);

                        TransformMatrix tm;
                        ModelData model;
                        model.worldMat = tm.getMatrix().transpose();
                        matrices[p].push_back(model);

                        unsigned int indexCount = 0, vertexCount = 0;
                        for (unsigned int j = 0; j < m_selectedScaleIndexed[i].getAllVertices().getVertexCount(); j++) {
                            vertexCount++;
                            vbBindlessTex[p].append(m_selectedScaleIndexed[i].getAllVertices()[j]);
                        }
                        for (unsigned int j = 0; j < m_selectedScaleIndexed[i].getAllVertices().getIndexes().size(); j++) {
                            indexCount++;
                            vbBindlessTex[p].addIndex(m_selectedScaleIndexed[i].getAllVertices().getIndexes()[j]);
                        }
                        drawElementsIndirectCommand.index_count = indexCount;
                        drawElementsIndirectCommand.first_index = firstIndex[p];
                        drawElementsIndirectCommand.instance_base = baseInstance[p];
                        drawElementsIndirectCommand.vertex_base = baseVertex[p];
                        drawElementsIndirectCommand.instance_count = 1;
                        drawElementsIndirectCommands[p].push_back(drawElementsIndirectCommand);
                        firstIndex[p] += indexCount;
                        baseVertex[p] += vertexCount;
                        baseInstance[p] += 1;
                    }
                }
                for (unsigned int i = 0; i < m_selectedScaleInstanceIndexed.size(); i++) {

                    if (m_selectedScaleInstanceIndexed[i].getAllVertices().getVertexCount() > 0) {
                        DrawElementsIndirectCommand drawElementsIndirectCommand;
                        unsigned int p = m_selectedScaleInstanceIndexed[i].getAllVertices().getPrimitiveType();
                        /*if (core::Application::app != nullptr) {
                            float time = core::Application::getTimeClk().getElapsedTime().asSeconds();
                            perPixelLinkedList.setParameter("time", time);
                        }*/
                        MaterialData material;
                        material.textureIndex = 0;
                        material.materialType = m_selectedScaleInstanceIndexed[i].getMaterial().getType();
                        materials[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_selectedScaleInstanceIndexed[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            ModelData model;
                            model.worldMat = tm[j]->getMatrix().transpose();
                            matrices[p].push_back(model);
                        }
                        unsigned int indexCount = 0, vertexCount = 0;
                        if (m_selectedScaleInstanceIndexed[i].getVertexArrays().size() > 0) {
                            Entity* entity = m_selectedScaleInstanceIndexed[i].getVertexArrays()[0]->getEntity();
                            for (unsigned int j = 0; j < m_selectedScaleInstanceIndexed[i].getVertexArrays().size(); j++) {
                                if (entity == m_selectedScaleInstanceIndexed[i].getVertexArrays()[j]->getEntity()) {
                                    unsigned int p = m_selectedScaleInstanceIndexed[i].getVertexArrays()[j]->getPrimitiveType();
                                    for (unsigned int k = 0; k < m_selectedScaleInstanceIndexed[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                        vertexCount++;
                                        vbBindlessTex[p].append((*m_selectedScaleInstanceIndexed[i].getVertexArrays()[j])[k]);
                                    }
                                    for (unsigned int k = 0; k < m_selectedScaleInstanceIndexed[i].getVertexArrays()[j]->getIndexes().size(); k++) {
                                        indexCount++;
                                        vbBindlessTex[p].addIndex(m_selectedScaleInstanceIndexed[i].getVertexArrays()[j]->getIndexes()[k]);
                                    }
                                }
                            }
                        }
                        drawElementsIndirectCommand.index_count = indexCount;
                        drawElementsIndirectCommand.first_index = firstIndex[p];
                        drawElementsIndirectCommand.instance_base = baseInstance[p];
                        drawElementsIndirectCommand.vertex_base = baseVertex[p];
                        drawElementsIndirectCommand.instance_count = tm.size();
                        drawElementsIndirectCommands[p].push_back(drawElementsIndirectCommand);
                        firstIndex[p] += indexCount;
                        baseVertex[p] += vertexCount;
                        baseInstance[p] += tm.size();
                        //std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                        //std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;
                    }
                }
                currentStates.blendMode = sf::BlendNone;
                currentStates.shader = &indirectRenderingShader;
                currentStates.texture = nullptr;
                glCheck(glStencilFunc(GL_NOTEQUAL, 1, 0xFF));
                glCheck(glStencilMask(0x00));
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelDataBuffer));
                        glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, matrices[p].size() * sizeof(ModelData), &matrices[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialDataBuffer));
                        glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, materials[p].size() * sizeof(MaterialData), &materials[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vboIndirect));
                        glCheck(glBufferData(GL_DRAW_INDIRECT_BUFFER, drawElementsIndirectCommands[p].size() * sizeof(DrawElementsIndirectCommand), &drawElementsIndirectCommands[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
                        vbBindlessTex[p].update();
                        frameBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawElementsIndirectCommands[p].size(), currentStates, vboIndirect);
                        vbBindlessTex[p].clear();
                    }
                }
            }
            void PerPixelLinkedListRenderComponent::drawNextFrame() {


                /*glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicBuffer));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, linkedListBuffer));*/
                //std::cout<<"draw nex frame"<<std::endl;
                //basicView.setPerspective(-1, 1, -1, 1, 0, 1);
                float zNear = view.getViewport().getPosition().z;
                if (!view.isOrtho())
                    view.setPerspective(80, view.getViewport().getSize().x / view.getViewport().getSize().y, 0.1f, view.getViewport().getSize().z);
                math::Matrix4f viewMatrix = view.getViewMatrix().getMatrix().transpose();
                math::Matrix4f projMatrix = view.getProjMatrix().getMatrix().transpose();
                viewMatrix = math::Matrix4f(math::Matrix3f(viewMatrix));

                skyboxShader.setParameter("projection", projMatrix);
                skyboxShader.setParameter("view", viewMatrix);
                vb.clear();
                //vb.name = "SKYBOXVB";
                for (unsigned int i = 0; i < m_skyboxInstance.size(); i++) {
                    if (m_skyboxInstance[i].getAllVertices().getVertexCount() > 0) {
                        vb.setPrimitiveType(m_skyboxInstance[i].getAllVertices().getPrimitiveType());
                        for (unsigned int j = 0; j < m_skyboxInstance[i].getAllVertices().getVertexCount(); j++) {
                            //if (m_skyboxInstance[i].getAllVertices()[j].position.x != 0 && m_skyboxInstance[i].getAllVertices()[j].position.y != 0 && m_skyboxInstance[i].getAllVertices()[j].position.z != 0);
                            vb.append(m_skyboxInstance[i].getAllVertices()[j]);
                        }
                    }
                }
                currentStates.blendMode = sf::BlendAlpha;
                currentStates.shader = &skyboxShader;
                currentStates.texture = (skybox == entt::null ) ? nullptr : core::ecs::Application::app->getWorld()->getComponentMapping().getComponent<MeshComponent>(skybox)->faces[0].getMaterial().getTexture();
                vb.update();
                frameBuffer.drawVertexBuffer(vb, currentStates);
                vb.clear();
                if (!view.isOrtho())
                    view.setPerspective(80, view.getViewport().getSize().x / view.getViewport().getSize().y, zNear, view.getViewport().getSize().z);
                projMatrix = view.getProjMatrix().getMatrix().transpose();
                viewMatrix = view.getViewMatrix().getMatrix().transpose();
                indirectRenderingShader.setParameter("projectionMatrix", projMatrix);
                indirectRenderingShader.setParameter("viewMatrix", viewMatrix);

                drawInstances();
                drawInstancesIndexed();
                drawSelectedInstances();
                drawSelectedInstancesIndexed();
                //std::cout<<"nb instances : "<<m_normals.size()<<std::endl;


                glCheck(glFinish());
                glCheck(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));
                //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
                vb.clear();
                //vb.name = "";
                vb.setPrimitiveType(sf::Quads);
                Vertex v1 (sf::Vector3f(0, 0, quad.getSize().z));
                Vertex v2 (sf::Vector3f(quad.getSize().x,0, quad.getSize().z));
                Vertex v3 (sf::Vector3f(quad.getSize().x, quad.getSize().y, quad.getSize().z));
                Vertex v4 (sf::Vector3f(0, quad.getSize().y, quad.getSize().z));
                vb.append(v1);
                vb.append(v2);
                vb.append(v3);
                vb.append(v4);
                vb.update();
                math::Matrix4f matrix = quad.getTransform().getMatrix().transpose();
                perPixelLinkedListP2.setParameter("worldMat", matrix);
                currentStates.shader = &perPixelLinkedListP2;
                frameBuffer.drawVertexBuffer(vb, currentStates);
                glCheck(glFinish());
                frameBuffer.display();
                //glCheck(glMemoryBarrier(GL_ALL_BARRIER_BITS));
                /*glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0));
                glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, 0));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0));*/
            }
            void PerPixelLinkedListRenderComponent::drawInstances() {
                for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                    vbBindlessTex[i].clear();
                }

                std::array<std::vector<DrawArraysIndirectCommand>, Batcher::nbPrimitiveTypes> drawArraysIndirectCommands;
                std::array<std::vector<ModelData>, Batcher::nbPrimitiveTypes> matrices;
                std::array<std::vector<MaterialData>, Batcher::nbPrimitiveTypes> materials;
                std::array<unsigned int, Batcher::nbPrimitiveTypes> firstIndex, baseInstance;
                for (unsigned int i = 0; i < firstIndex.size(); i++) {
                    firstIndex[i] = 0;
                }
                for (unsigned int i = 0; i < baseInstance.size(); i++) {
                    baseInstance[i] = 0;
                }
                for (unsigned int i = 0; i < m_normals.size(); i++) {
                    if (m_normals[i].getAllVertices().getVertexCount() > 0) {
                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        //std::cout<<"next frame draw normal"<<std::endl;

                        float time = timeClock.getTimeClk().getElapsedTime().asSeconds();
                        indirectRenderingShader.setParameter("time", time);

                        unsigned int p = m_normals[i].getAllVertices().getPrimitiveType();
                        /*if (m_normals[i].getVertexArrays()[0]->getEntity()->getRootType() == "E_MONSTER") {
                                std::cout<<"tex coords : "<<(*m_normals[i].getVertexArrays()[0])[0].texCoords.x<<","<<(*m_normals[i].getVertexArrays()[0])[0].texCoords.y<<std::endl;
                            }*/
                        unsigned int vertexCount = 0;
                        MaterialData material;
                        material.textureIndex = (m_normals[i].getMaterial().getTexture() != nullptr) ? m_normals[i].getMaterial().getTexture()->getId() : 0;
                        material.materialType = m_normals[i].getMaterial().getType();
                        materials[p].push_back(material);
                        for (unsigned int j = 0; j < m_normals[i].getAllVertices().getVertexCount(); j++) {
                            vbBindlessTex[p].append(m_normals[i].getAllVertices()[j]);
                            vertexCount++;
                        }
                        TransformMatrix tm;
                        ModelData modelData;
                        modelData.worldMat = tm.getMatrix().transpose();
                        matrices[p].push_back(modelData);

                        drawArraysIndirectCommand.count = vertexCount;
                        drawArraysIndirectCommand.firstIndex = firstIndex[p];
                        drawArraysIndirectCommand.baseInstance = baseInstance[p];
                        drawArraysIndirectCommand.instanceCount = 1;
                        drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                        firstIndex[p] += vertexCount;
                        baseInstance[p] += 1;
                        /*for (unsigned int j = 0; j < m_normals[i].getVertexArrays().size(); j++) {
                            if (m_normals[i].getVertexArrays()[j]->getEntity() != nullptr && m_normals[i].getVertexArrays()[j]->getEntity()->getRootType() == "E_HERO") {
                                for (unsigned int n = 0; n < m_normals[i].getVertexArrays()[j]->getVertexCount(); n++)
                                    std::cout<<"position hero : "<<(*m_normals[i].getVertexArrays()[j])[n].position.x<<","<<(*m_normals[i].getVertexArrays()[j])[n].position.y<<","<<(*m_normals[i].getVertexArrays()[j])[n].position.z<<std::endl;
                            }
                        }*/
                    }
                }
                for (unsigned int i = 0; i < m_instances.size(); i++) {
                    if (m_instances[i].getAllVertices().getVertexCount() > 0) {
                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        unsigned int p = m_instances[i].getAllVertices().getPrimitiveType();

                        float time = timeClock.getTimeClk().getElapsedTime().asSeconds();
                        indirectRenderingShader.setParameter("time", time);

                        std::vector<TransformMatrix*> tm = m_instances[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            ModelData modelData;
                            modelData.worldMat = tm[j]->getMatrix().transpose();
                            matrices[p].push_back(modelData);
                        }
                        MaterialData materialData;
                        materialData.textureIndex = (m_instances[i].getMaterial().getTexture() != nullptr) ? m_instances[i].getMaterial().getTexture()->getId() : 0;
                        materialData.materialType = m_instances[i].getMaterial().getType();
                        materials[p].push_back(materialData);
                        unsigned int vertexCount = 0;
                        if (m_instances[i].getVertexArrays().size() > 0) {
                            EntityId entity = m_instances[i].getVertexArrays()[0]->getEntityId();
                            for (unsigned int j = 0; j < m_instances[i].getVertexArrays().size(); j++) {
                                if (entity == m_instances[i].getVertexArrays()[j]->getEntityId()) {
                                    for (unsigned int k = 0; k < m_instances[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                        vertexCount++;
                                        vbBindlessTex[p].append((*m_instances[i].getVertexArrays()[j])[k]);
                                    }
                                }
                            }
                        }
                        drawArraysIndirectCommand.count = vertexCount;
                        drawArraysIndirectCommand.firstIndex = firstIndex[p];
                        drawArraysIndirectCommand.baseInstance = baseInstance[p];
                        drawArraysIndirectCommand.instanceCount = tm.size();
                        drawArraysIndirectCommands[p].push_back(drawArraysIndirectCommand);
                        firstIndex[p] += vertexCount;
                        baseInstance[p] += tm.size();
                        //std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                        //std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;

                    }
                }
                RenderStates currentStates;
                currentStates.blendMode = sf::BlendNone;
                currentStates.shader = &indirectRenderingShader;
                currentStates.texture = nullptr;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelDataBuffer));
                        glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, matrices[p].size() * sizeof(ModelData), &matrices[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialDataBuffer));
                        glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, materials[p].size() * sizeof(MaterialData), &materials[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vboIndirect));
                        glCheck(glBufferData(GL_DRAW_INDIRECT_BUFFER, drawArraysIndirectCommands[p].size() * sizeof(DrawArraysIndirectCommand), &drawArraysIndirectCommands[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
                        vbBindlessTex[p].update();
                        frameBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), currentStates, vboIndirect);
                        vbBindlessTex[p].clear();
                    }
                }
            }
            void PerPixelLinkedListRenderComponent::drawInstancesIndexed() {
                for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                    vbBindlessTex[i].clear();
                }
                std::array<std::vector<DrawElementsIndirectCommand>, Batcher::nbPrimitiveTypes> drawElementsIndirectCommands;
                std::array<std::vector<ModelData>, Batcher::nbPrimitiveTypes> matrices;
                std::array<std::vector<MaterialData>, Batcher::nbPrimitiveTypes> materials;
                std::array<unsigned int, Batcher::nbPrimitiveTypes> firstIndex, baseInstance, baseVertex;
                for (unsigned int i = 0; i < firstIndex.size(); i++) {
                    firstIndex[i] = 0;
                }
                for (unsigned int i = 0; i < baseVertex.size(); i++) {
                    baseVertex[i] = 0;
                }
                for (unsigned int i = 0; i < baseInstance.size(); i++) {
                    baseInstance[i] = 0;
                }
                for (unsigned int i = 0; i < m_normals.size(); i++) {

                   if (m_normalsIndexed[i].getAllVertices().getVertexCount() > 0) {
                        DrawElementsIndirectCommand drawElementsIndirectCommand;

                        float time = timeClock.getTimeClk().getElapsedTime().asSeconds();
                        indirectRenderingShader.setParameter("time", time);

                        unsigned int p = m_normalsIndexed[i].getAllVertices().getPrimitiveType();
                        /*if (m_normals[i].getVertexArrays()[0]->getEntity()->getRootType() == "E_MONSTER") {
                                std::cout<<"tex coords : "<<(*m_normals[i].getVertexArrays()[0])[0].texCoords.x<<","<<(*m_normals[i].getVertexArrays()[0])[0].texCoords.y<<std::endl;
                            }*/
                        MaterialData material;
                        material.textureIndex = (m_normalsIndexed[i].getMaterial().getTexture() != nullptr) ? m_normalsIndexed[i].getMaterial().getTexture()->getId() : 0;
                        material.materialType = m_normalsIndexed[i].getMaterial().getType();
                        materials[p].push_back(material);

                        TransformMatrix tm;
                        ModelData modelData;
                        modelData.worldMat = tm.getMatrix().transpose();
                        matrices[p].push_back(modelData);
                        unsigned int indexCount = 0, vertexCount = 0;
                        for (unsigned int j = 0; j < m_normalsIndexed[i].getAllVertices().getVertexCount(); j++) {
                            vbBindlessTex[p].append(m_normalsIndexed[i].getAllVertices()[j]);
                            vertexCount++;
                        }
                        for (unsigned int j = 0; j < m_normalsIndexed[i].getAllVertices().getIndexes().size(); j++) {
                            vbBindlessTex[p].addIndex(m_normalsIndexed[i].getAllVertices().getIndexes()[j]);
                            indexCount++;
                        }
                        drawElementsIndirectCommand.index_count = indexCount;
                        drawElementsIndirectCommand.first_index = firstIndex[p];
                        drawElementsIndirectCommand.instance_base = baseInstance[p];
                        drawElementsIndirectCommand.vertex_base = baseVertex[p];
                        drawElementsIndirectCommand.instance_count = 1;
                        drawElementsIndirectCommands[p].push_back(drawElementsIndirectCommand);
                        firstIndex[p] += indexCount;
                        baseVertex[p] += vertexCount;
                        baseInstance[p] += 1;
                    }
                }
                for (unsigned int i = 0; i < m_instancesIndexed.size(); i++) {
                    if (m_instancesIndexed[i].getAllVertices().getVertexCount() > 0) {
                        DrawElementsIndirectCommand drawElementsIndirectCommand;
                        unsigned int p = m_instancesIndexed[i].getAllVertices().getPrimitiveType();
                        /*if (core::Application::app != nullptr) {
                            float time = core::Application::getTimeClk().getElapsedTime().asSeconds();
                            perPixelLinkedList.setParameter("time", time);
                        }*/
                        MaterialData material;
                        material.textureIndex = (m_instancesIndexed[i].getMaterial().getTexture() != nullptr) ? m_instancesIndexed[i].getMaterial().getTexture()->getId() : 0;
                        material.materialType = m_instancesIndexed[i].getMaterial().getType();
                        materials[p].push_back(material);

                        std::vector<TransformMatrix*> tm = m_instancesIndexed[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            ModelData model;
                            model.worldMat = tm[j]->getMatrix().transpose();
                            matrices[p].push_back(model);
                        }

                        unsigned int indexCount = 0, vertexCount = 0;
                        if (m_instancesIndexed[i].getVertexArrays().size() > 0) {
                            EntityId entity = m_instancesIndexed[i].getVertexArrays()[0]->getEntityId();
                            for (unsigned int j = 0; j < m_instancesIndexed[i].getVertexArrays().size(); j++) {
                                if (entity == m_instancesIndexed[i].getVertexArrays()[j]->getEntityId()) {
                                    unsigned int p = m_instancesIndexed[i].getVertexArrays()[j]->getPrimitiveType();
                                    for (unsigned int k = 0; k < m_instancesIndexed[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                        vertexCount++;
                                        vbBindlessTex[p].append((*m_instancesIndexed[i].getVertexArrays()[j])[k]);
                                    }
                                    for (unsigned int k = 0; k < m_instancesIndexed[i].getVertexArrays()[j]->getIndexes().size(); k++) {
                                        indexCount++;
                                        vbBindlessTex[p].addIndex(m_instancesIndexed[i].getVertexArrays()[j]->getIndexes()[k]);
                                    }
                                }
                            }
                        }
                        drawElementsIndirectCommand.index_count = indexCount;
                        drawElementsIndirectCommand.first_index = firstIndex[p];
                        drawElementsIndirectCommand.instance_base = baseInstance[p];
                        drawElementsIndirectCommand.vertex_base = baseVertex[p];
                        drawElementsIndirectCommand.instance_count = tm.size();
                        drawElementsIndirectCommands[p].push_back(drawElementsIndirectCommand);
                        firstIndex[p] += indexCount;
                        baseVertex[p] += vertexCount;
                        baseInstance[p] += tm.size();
                        //std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                        //std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;
                    }
                }
                currentStates.blendMode = sf::BlendNone;
                currentStates.shader = &indirectRenderingShader;
                currentStates.texture = nullptr;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelDataBuffer));
                        glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, matrices[p].size() * sizeof(ModelData), &matrices[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialDataBuffer));
                        glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, materials[p].size() * sizeof(MaterialData), &materials[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vboIndirect));
                        glCheck(glBufferData(GL_DRAW_INDIRECT_BUFFER, drawElementsIndirectCommands[p].size() * sizeof(DrawElementsIndirectCommand), &drawElementsIndirectCommands[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
                        vbBindlessTex[p].update();
                        frameBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawElementsIndirectCommands[p].size(), currentStates, vboIndirect);
                        vbBindlessTex[p].clear();
                    }
                }
            }
            void PerPixelLinkedListRenderComponent::draw(RenderTarget& target, RenderStates states) {
                /*states.shader=&perPixelLinkedList;
                for (unsigned int i = 0; i < m_instances.size(); i++) {
                   if (m_instances[i].getAllVertices().getVertexCount() > 0) {
                        if (m_instances[i].getMaterial().getTexture() == nullptr) {
                            perPixelLinkedList.setParameter("haveTexture", 0.f);
                            filterNotOpaque.setParameter("haveTexture", 0.f);
                        } else {
                            perPixelLinkedList.setParameter("haveTexture", 1.f);
                            filterNotOpaque.setParameter("haveTexture", 1.f);
                        }
                        states.texture = m_instances[i].getMaterial().getTexture();
                        target.draw(m_instances[i].getAllVertices(), states);
                    }
                }
                glCheck(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));
                glCheck(glFinish());
                //glCheck(glTextureBarrier());
                states.shader = &perPixelLinkedListP2;
                //glCheck(glDepthMask(GL_FALSE));
                /*for (unsigned int i = 0; i < m_instances.size(); i++) {
                   if (m_instances[i].getAllVertices().getVertexCount() > 0) {
                        target.draw(m_instances[i].getAllVertices(), states);
                   }
                }*/

                /*target.draw(quad, states);
                glCheck(glFinish());*/
                frameBufferSprite.setCenter(target.getView().getPosition());
                //std::cout<<"view position : "<<view.getPosition()<<std::endl;
                //std::cout<<"sprite position : "<<frameBufferSprite.getCenter()<<std::endl;
                target.draw(frameBufferSprite, states);


            }
            int  PerPixelLinkedListRenderComponent::getLayer() {
                return layer;
            }
            void PerPixelLinkedListRenderComponent::draw(Drawable& drawable, RenderStates states) {
                //drawables.insert(std::make_pair(drawable, states));
            }
            void PerPixelLinkedListRenderComponent::setView(View view) {
                frameBuffer.setView(view);
                this->view = view;
            }
            std::vector<EntityId> PerPixelLinkedListRenderComponent::getEntities() {
                return visibleEntities;
            }
            std::string PerPixelLinkedListRenderComponent::getExpression() {
                return expression;
            }
            View& PerPixelLinkedListRenderComponent::getView() {
                return view;
            }
            bool PerPixelLinkedListRenderComponent::needToUpdate() {
                return update;
            }
            void PerPixelLinkedListRenderComponent::setExpression (std::string expression) {
                this->expression = expression;
            }
            void PerPixelLinkedListRenderComponent::loadSkybox(EntityId skybox) {
                this->skybox = skybox;
            }
            bool PerPixelLinkedListRenderComponent::loadEntitiesOnComponent(ComponentMapping& componentMapping, std::vector<EntityId> vEntities) {
                batcher.clear();
                normalBatcher.clear();
                batcherIndexed.clear();
                normalBatcherIndexed.clear();
                selectedBatcher.clear();
                selectedScaleBatcher.clear();
                selectedIndexBatcher.clear();
                selectedIndexScaleBatcher.clear();
                selectedInstanceBatcher.clear();
                selectedInstanceScaleBatcher.clear();
                selectedInstanceIndexBatcher.clear();
                selectedInstanceIndexScaleBatcher.clear();
                skyboxBatcher.clear();
                if (skybox != entt::null) {
                    MeshComponent* mc = componentMapping.getComponent<MeshComponent>(skybox);
                    for (unsigned int i = 0; i <  mc->faces.size(); i++) {
                        skyboxBatcher.addFace(&mc->faces[i]);
                    }
                }
                //std::cout<<"load tile"<<std::endl;
                for (unsigned int i = 0; i < vEntities.size(); i++) {
                    /*if (componentMapping.getComponent<EntityInfoComponent>(componentMapping.getRoot(vEntities[i]))->groupName == "E_DECOR")
                        std::cout<<"add E_DECOR"<<std::endl;*/
                    if (componentMapping.getComponent<MeshComponent>(vEntities[i]) != nullptr) {
                        EntityInfoComponent* eci = componentMapping.getComponent<EntityInfoComponent>(vEntities[i]);
                        MeshComponent* mc = componentMapping.getComponent<MeshComponent>(vEntities[i]);
                        EntityId border;
                        if (eci->isSelected)
                            border = componentMapping.clone(vEntities[i]);
                        for (unsigned int j = 0; j <  mc->faces.size(); j++) {
                             if (eci->drawMode == DrawMode::INSTANCED && !eci->isSelected) {
                                if (mc->faces[j].getVertexArray().getIndexes().size() == 0)
                                    batcher.addFace(& mc->faces[j]);
                                else
                                    batcherIndexed.addFace( &mc->faces[j]);
                             } else if (eci->drawMode == DrawMode::NORMAL && !eci->isSelected) {
                                 if (mc->faces[j].getVertexArray().getIndexes().size() == 0)
                                    normalBatcher.addFace( &mc->faces[j]);
                                 else
                                    normalBatcherIndexed.addFace( &mc->faces[j]);
                            } else if (eci->drawMode == DrawMode::INSTANCED && eci->isSelected) {
                               // std::cout<<"selected add face"<<std::endl;
                                if (mc->faces[j].getVertexArray().getIndexes().size() == 0) {
                                    selectedInstanceBatcher.addFace(&mc->faces[j]);
                               // std::cout<<"remove texture"<<std::endl;
                                    MeshComponent* cmc = componentMapping.getComponent<MeshComponent>(border);
                                    TransformComponent* tc = componentMapping.getComponent<TransformComponent>(border);
                                //std::cout<<"get va"<<std::endl;
                                    VertexArray& va = cmc->faces[j].getVertexArray();
                                    //std::cout<<"change color"<<std::endl;
                                    for (unsigned int j = 0; j < va.getVertexCount(); j++) {

                                        va[j].color = sf::Color::Cyan;
                                    }

                                    tc->origin = tc->size * 0.5f;
                                    tc->scale = math::Vec3f(1.1f, 1.1f, 1.1f);
                                   // std::cout<<"add to batcher"<<std::endl;
                                    selectedInstanceScaleBatcher.addFace(&cmc->faces[j]);
                               // std::cout<<"face added"<<std::endl;
                                 } else {
                                    selectedInstanceIndexBatcher.addFace(&mc->faces[j]);
                                   // std::cout<<"remove texture"<<std::endl;

                                    MeshComponent* cmc = componentMapping.getComponent<MeshComponent>(border);
                                    TransformComponent* tc = componentMapping.getComponent<TransformComponent>(border);
                                //std::cout<<"get va"<<std::endl;
                                    VertexArray& va = cmc->faces[j].getVertexArray();
                                    //std::cout<<"change color"<<std::endl;
                                    for (unsigned int j = 0; j < va.getVertexCount(); j++) {

                                        va[j].color = sf::Color::Cyan;
                                    }

                                    tc->origin = tc->size * 0.5f;
                                    tc->scale = math::Vec3f(1.1f, 1.1f, 1.1f);
                                   // std::cout<<"add to batcher"<<std::endl;
                                    selectedInstanceIndexScaleBatcher.addFace(&cmc->faces[j]);
                                 }
                            } else {
                                if (mc->faces[j].getVertexArray().getIndexes().size() == 0 == 0) {
                                    selectedBatcher.addFace(&mc->faces[j]);
                               // std::cout<<"remove texture"<<std::endl;

                                //std::cout<<"get va"<<std::endl;
                                   MeshComponent* cmc = componentMapping.getComponent<MeshComponent>(border);
                                   TransformComponent* tc = componentMapping.getComponent<TransformComponent>(border);
                                //std::cout<<"get va"<<std::endl;
                                   VertexArray& va = cmc->faces[j].getVertexArray();
                                    //std::cout<<"change color"<<std::endl;
                                   for (unsigned int j = 0; j < va.getVertexCount(); j++) {

                                       va[j].color = sf::Color::Cyan;
                                   }

                                   tc->origin = tc->size * 0.5f;
                                   tc->scale = math::Vec3f(1.1f, 1.1f, 1.1f);
                                   // std::cout<<"add to batcher"<<std::endl;
                                   selectedScaleBatcher.addFace(&cmc->faces[j]);

                                   // std::cout<<"face added"<<std::endl;
                                 } else {
                                     selectedIndexBatcher.addFace(&mc->faces[j]);
                                   // std::cout<<"remove texture"<<std::endl;

                                //std::cout<<"get va"<<std::endl;
                                    MeshComponent* cmc = componentMapping.getComponent<MeshComponent>(border);
                                    TransformComponent* tc = componentMapping.getComponent<TransformComponent>(border);
                                //std::cout<<"get va"<<std::endl;
                                    VertexArray& va = cmc->faces[j].getVertexArray();
                                    //std::cout<<"change color"<<std::endl;
                                    for (unsigned int j = 0; j < va.getVertexCount(); j++) {

                                        va[j].color = sf::Color::Cyan;
                                    }

                                    tc->origin = tc->size * 0.5f;
                                    tc->scale = math::Vec3f(1.1f, 1.1f, 1.1f);
                                   // std::cout<<"add to batcher"<<std::endl;
                                    selectedIndexScaleBatcher.addFace(&cmc->faces[j]);
                                 }
                            }
                        }
                    }

                }
                m_instances = batcher.getInstances();
                m_normals = normalBatcher.getInstances();
                m_instancesIndexed = batcherIndexed.getInstances();
                m_normalsIndexed = normalBatcherIndexed.getInstances();
                m_selected = selectedBatcher.getInstances();
                m_selectedScale = selectedScaleBatcher.getInstances();
                m_selectedIndexed = selectedIndexBatcher.getInstances();
                m_selectedScaleIndexed = selectedIndexScaleBatcher.getInstances();
                m_selectedInstance = selectedInstanceBatcher.getInstances();
                m_selectedScaleInstance = selectedInstanceScaleBatcher.getInstances();
                m_selectedInstanceIndexed = selectedInstanceIndexBatcher.getInstances();
                m_selectedScaleInstanceIndexed = selectedInstanceIndexScaleBatcher.getInstances();
                //std::cout<<"instances added"<<std::endl;
                visibleEntities = vEntities;
                update = true;
                return true;
            }
            void PerPixelLinkedListRenderComponent::pushEvent(window::IEvent event, RenderWindow& rw) {
                if (event.type == window::IEvent::WINDOW_EVENT && event.window.type == window::IEvent::WINDOW_EVENT_RESIZED && &getWindow() == &rw && isAutoResized()) {
                    std::cout<<"recompute size"<<std::endl;
                    recomputeSize();
                    getListener().pushEvent(event);
                    getView().reset(physic::BoundingBox(getView().getViewport().getPosition().x, getView().getViewport().getPosition().y, getView().getViewport().getPosition().z, event.window.data1, event.window.data2, getView().getViewport().getDepth()));
                }
            }
            const Texture& PerPixelLinkedListRenderComponent::getFrameBufferTexture() {
                return frameBuffer.getTexture();
            }
            void PerPixelLinkedListRenderComponent::onVisibilityChanged(bool visible) {
                if (visible) {
                    glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                    glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicBuffer));
                    glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, linkedListBuffer));
                } else {
                    glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0));
                    glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, 0));
                    glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0));
                }
            }
            PerPixelLinkedListRenderComponent::~PerPixelLinkedListRenderComponent() {
                glDeleteBuffers(1, &atomicBuffer);
                glDeleteBuffers(1, &linkedListBuffer);
                glDeleteBuffers(1, &clearBuf);
                glDeleteTextures(1, &headPtrTex);
                glDeleteBuffers(1, &modelDataBuffer);
                glDeleteBuffers(1, &materialDataBuffer);
                glDeleteBuffers(1, &ubo);
            }
            #endif // VULKAN
        }
    }
}
