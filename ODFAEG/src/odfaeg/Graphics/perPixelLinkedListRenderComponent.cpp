#include "../../../include/odfaeg/Graphics/perPixelLinkedListRenderComponent.hpp"
#include "../../../include/odfaeg/Physics/particuleSystem.h"
#include "../../../include/odfaeg/Graphics/application.h"
#ifndef VULKAN
#include "glCheck.h"
#endif
namespace odfaeg {
    namespace graphic {
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

            if (settings.versionMajor >= 4 && settings.versionMinor >= 3) {
                glGenBuffers(1, &vboWorldMatrices);
                compileShaders();
            } else {
                const std::string  simpleVertexShader =
                R"(#version 140
                void main () {
                    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
                    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
                    gl_FrontColor = gl_Color;
                })";
                const std::string fragmentShader =
                R"(#version 140
                   #extension GL_ARB_shader_atomic_counters : require
                   #extension GL_ARB_shading_language_420pack : require
                   #extension GL_ARB_shader_image_load_store : require
                   #extension GL_ARB_shader_storage_buffer_object : require
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
                   uniform uint maxNodes;
                   uniform float haveTexture;
                   uniform sampler2D texture;
                   void main() {
                       uint nodeIdx = atomicCounterIncrement(nextNodeCounter);
                       vec4 texel = texture2D(texture, gl_TexCoord[0].xy);
                       vec4 color = (haveTexture > 0.9) ? gl_Color * texel : gl_Color;
                       if (nodeIdx < maxNodes) {
                            uint prevHead = imageAtomicExchange(headPointers, ivec2(gl_FragCoord.xy), nodeIdx);
                            nodes[nodeIdx].color = color;
                            nodes[nodeIdx].depth = gl_FragCoord.z;
                            nodes[nodeIdx].next = prevHead;
                       }
                       gl_FragColor = vec4(0, 0, 0, 0);
                   })";
                   const std::string fragmentShader2 =
                   R"(
                   #version 140
                   #extension GL_ARB_shader_atomic_counters : require
                   #extension GL_ARB_shading_language_420pack : require
                   #extension GL_ARB_shader_image_load_store : require
                   #extension GL_ARB_shader_storage_buffer_object : require
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
                      int i, j1, j2, k;
                      int a, b, c;
                      int step = 1;
                      NodeType leftArray[MAX_FRAGMENTS/2]; //for merge sort
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
                              }
                              ////////////////////////////////////////////////////////////////////////
                              i += 2 * step;
                          }
                          step *= 2;
                      }
                      vec4 color = vec4(0, 0, 0, 0);
                      for( int i = count - 1; i >= 0; i--)
                      {
                        color.rgb = frags[i].color.rgb * frags[i].color.a + color.rgb * (1 - frags[i].color.a);
                        color.a = frags[i].color.a + color.a * (1 - frags[i].color.a);
                      }
                      gl_FragColor = color;
                   })";
                   if (!perPixelLinkedList.loadFromMemory(simpleVertexShader, fragmentShader)) {
                        throw core::Erreur(54, "Failed to load per pixel linked list shader");
                   }
                   if (!perPixelLinkedListP2.loadFromMemory(simpleVertexShader, fragmentShader2)) {
                        throw core::Erreur(54, "Failed to load per pixel linked list pass 2 shader");
                   }
                   std::cout<<"shaders compilated"<<std::endl;
                   perPixelLinkedList.setParameter("maxNodes", maxNodes);
                   perPixelLinkedList.setParameter("texture", Shader::CurrentTexture);
                }
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
                perPixelLinkedList2.setParameter("textureMatrix", textureMatrices);
                glCheck(glGenBuffers(1, &ubo));
                unsigned int ubid;
                glCheck(ubid = glGetUniformBlockIndex(perPixelLinkedList2.getHandle(), "ALL_TEXTURES"));
                glCheck(glUniformBlockBinding(perPixelLinkedList2.getHandle(),    ubid, 0));
                glCheck(ubid = glGetUniformBlockIndex(perPixelLinkedList.getHandle(), "ALL_TEXTURES"));
                glCheck(glUniformBlockBinding(perPixelLinkedList.getHandle(),    ubid, 0));
                backgroundColor = sf::Color::Transparent;
                glCheck(glBindBuffer(GL_UNIFORM_BUFFER, ubo));
                glCheck(glBufferData(GL_UNIFORM_BUFFER, sizeof(Samplers),allSamplers.tex, GL_STATIC_DRAW));
                glCheck(glBindBuffer(GL_UNIFORM_BUFFER, 0));
                //std::cout<<"size : "<<sizeof(Samplers)<<" "<<alignof (alignas(16) uint64_t[200])<<std::endl;
                backgroundColor = sf::Color::Transparent;
                glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicBuffer));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, linkedListBuffer));

               for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                    vbBindlessTex[i].setPrimitiveType(static_cast<sf::PrimitiveType>(i));
               }
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
            perPixelLinkedList.setParameter("textureMatrix", textureMatrices);
            perPixelLinkedList2.setParameter("textureMatrix", textureMatrices);
            glCheck(glBindBuffer(GL_UNIFORM_BUFFER, ubo));
            glCheck(glBufferData(GL_UNIFORM_BUFFER, sizeof(Samplers),allSamplers.tex, GL_STATIC_DRAW));
            glCheck(glBindBuffer(GL_UNIFORM_BUFFER, 0));
        }
        void PerPixelLinkedListRenderComponent::compileShaders() {
            const std::string vertexShader = R"(#version 460
                                                    #define M_PI 3.1415926535897932384626433832795
                                                    #define FPI M_PI/4
                                                    layout (location = 0) in vec3 position;
                                                    layout (location = 1) in vec4 color;
                                                    layout (location = 2) in vec2 texCoords;
                                                    layout (location = 3) in vec3 normals;
                                                    layout (location = 4) in mat4 worldMat;
                                                    layout (location = 12) in uint textureIndex;
                                                    uniform mat4 projectionMatrix;
                                                    uniform mat4 viewMatrix;
                                                    uniform mat4 textureMatrix[)"+core::conversionUIntString(Texture::getAllTextures().size())+R"(];
                                                    uniform float water;
                                                    uniform float time;
                                                    uniform vec3 resolution;
                                                    out vec2 fTexCoords;
                                                    out vec4 frontColor;
                                                    out uint texIndex;
                                                    void main() {
                                                        float xOff = 0;
                                                        float yOff = 0;
                                                        if (water > 0.9f) {
                                                            yOff = 0.05*sin(position.x*12+time*FPI)*resolution.y;
                                                            xOff = 0.025*cos(position.x*12+time*FPI)*resolution.x;
                                                        }
                                                        gl_Position = projectionMatrix * viewMatrix * worldMat * vec4((position.x - xOff), (position.y + yOff), position.z, 1.f);
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
                const std::string  simpleVertexShader2 = R"(#version 460
                                                            #define M_PI 3.1415926535897932384626433832795
                                                            #define FPI M_PI/4
                                                            layout (location = 0) in vec3 position;
                                                            layout (location = 1) in vec4 color;
                                                            layout (location = 2) in vec2 texCoords;
                                                            layout (location = 3) in vec3 normals;
                                                            layout (location = 4) in uint textureIndex;
                                                            uniform mat4 textureMatrix[)"+core::conversionUIntString(Texture::getAllTextures().size())+R"(];
                                                            uniform mat4 projectionMatrix;
                                                            uniform mat4 viewMatrix;
                                                            uniform float water;
                                                            uniform float time;
                                                            uniform vec3 resolution;
                                                            out vec2 fTexCoords;
                                                            out vec4 frontColor;
                                                            out uint texIndex;
                                                            void main () {
                                                                float xOff = 0;
                                                                float yOff = 0;
                                                                if (water > 0.9f) {
                                                                    yOff = 0.05*sin(position.x*12+time*FPI)*resolution.y;
                                                                    xOff = 0.025*cos(position.x*12+time*FPI)*resolution.x;
                                                                }
                                                                gl_Position = projectionMatrix * viewMatrix * vec4((position.x - xOff), (position.y + yOff), position.z, 1.f);
                                                                fTexCoords = (textureIndex != 0) ? (textureMatrix[textureIndex-1] * vec4(texCoords, 1.f, 1.f)).xy : texCoords;
                                                                frontColor = color;
                                                                texIndex = textureIndex;
                                                            })";
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
                                                      uniform float haveTexture;
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
                   if (!perPixelLinkedList.loadFromMemory(vertexShader, fragmentShader)) {
                        throw core::Erreur(54, "Failed to load per pixel linked list shader");
                   }
                   if (!perPixelLinkedListP2.loadFromMemory(simpleVertexShader, fragmentShader2)) {
                        throw core::Erreur(55, "Failed to load per pixel linked list pass 2 shader");
                   }
                   if (!perPixelLinkedList2.loadFromMemory(simpleVertexShader2, fragmentShader)) {
                       throw core::Erreur(56, "Failed to load per pixel linked list 2 shader");
                   }
                   perPixelLinkedList.setParameter("maxNodes", maxNodes);
                   perPixelLinkedList.setParameter("currentTex", Shader::CurrentTexture);
                   perPixelLinkedList.setParameter("resolution", resolution.x, resolution.y, resolution.z);
                   perPixelLinkedList2.setParameter("maxNodes", maxNodes);
                   perPixelLinkedList2.setParameter("currentTex", Shader::CurrentTexture);
                   perPixelLinkedList2.setParameter("resolution", resolution.x, resolution.y, resolution.z);
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
        void PerPixelLinkedListRenderComponent::drawNextFrame() {
            if (frameBuffer.getSettings().versionMajor >= 4 && frameBuffer.getSettings().versionMinor >= 3) {
                /*glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicBuffer));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, linkedListBuffer));*/
                //std::cout<<"draw nex frame"<<std::endl;
                math::Matrix4f viewMatrix = view.getViewMatrix().getMatrix().transpose();
                math::Matrix4f projMatrix = view.getProjMatrix().getMatrix().transpose();
                perPixelLinkedList.setParameter("projectionMatrix", projMatrix);
                perPixelLinkedList.setParameter("viewMatrix", viewMatrix);
                perPixelLinkedList2.setParameter("projectionMatrix", projMatrix);
                perPixelLinkedList2.setParameter("viewMatrix", viewMatrix);
                for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                    vbBindlessTex[i].clear();
                }
                std::vector<TransformMatrix*> tm;
                for (unsigned int i = 0; i < m_instances.size(); i++) {
                    if (m_instances[i].getAllVertices().getVertexCount() > 0) {
                        if (m_instances[i].getMaterial().getType() == Material::WATER) {
                            perPixelLinkedList.setParameter("water", 1.0f);
                        } else {
                            perPixelLinkedList.setParameter("water", 0.0f);
                        }
                        if (core::Application::app != nullptr) {
                            float time = core::Application::getTimeClk().getElapsedTime().asSeconds();
                            perPixelLinkedList.setParameter("time", time);
                        }
                        matrices.clear();
                        tm = m_instances[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            std::array<float, 16> matrix = tm[j]->getMatrix().transpose().toGlMatrix();
                            for (unsigned int n = 0; n < 16; n++) {
                                matrices.push_back(matrix[n]);
                            }
                        }
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboWorldMatrices));
                        glCheck(glBufferData(GL_ARRAY_BUFFER, matrices.size() * sizeof(float), &matrices[0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        if (m_instances[i].getVertexArrays().size() > 0) {
                            Entity* entity = m_instances[i].getVertexArrays()[0]->getEntity();
                            for (unsigned int j = 0; j < m_instances[i].getVertexArrays().size(); j++) {
                                if (entity == m_instances[i].getVertexArrays()[j]->getEntity()) {
                                    unsigned int p = m_instances[i].getVertexArrays()[j]->getPrimitiveType();
                                    for (unsigned int k = 0; k < m_instances[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                        vbBindlessTex[p].append((*m_instances[i].getVertexArrays()[j])[k], (m_instances[i].getMaterial().getTexture() != nullptr) ? m_instances[i].getMaterial().getTexture()->getId() : 0);
                                    }
                                }
                            }
                        }
                        //std::cout<<"texture : "<<m_instances[i].getMaterial().getTexture()<<std::endl;
                        //std::cout<<"entity : "<<m_instances[i].getVertexArrays()[0]->getEntity()->getRootEntity()->getType()<<std::endl;
                        if (m_instances[i].getMaterial().getTexture() != nullptr) {



                            //std::cout<<"texture"<<std::endl;
                            math::Matrix4f texMatrix = m_instances[i].getMaterial().getTexture()->getTextureMatrix();
                            //perPixelLinkedList.setParameter("textureMatrix", texMatrix);
                            perPixelLinkedList.setParameter("haveTexture", 1.f);
                        } else {
                            //std::cout<<"no texture"<<std::endl;
                            perPixelLinkedList.setParameter("haveTexture", 0.f);
                        }
                    }
                }
                currentStates.blendMode = sf::BlendNone;
                currentStates.shader = &perPixelLinkedList;
                currentStates.texture = nullptr;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        vbBindlessTex[p].update();
                        frameBuffer.drawInstanced(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), 0, vbBindlessTex[p].getVertexCount(), tm.size(), currentStates, vboWorldMatrices);
                        vbBindlessTex[p].clear();
                    }
                }
                //std::cout<<"nb instances : "<<m_normals.size()<<std::endl;
                for (unsigned int i = 0; i < m_normals.size(); i++) {

                   if (m_normals[i].getAllVertices().getVertexCount() > 0) {
                        //std::cout<<"next frame draw normal"<<std::endl;
                        if (m_normals[i].getMaterial().getTexture() == nullptr) {
                            perPixelLinkedList2.setParameter("haveTexture", 0.f);
                        } else {
                            math::Matrix4f texMatrix = m_normals[i].getMaterial().getTexture()->getTextureMatrix();
                            //perPixelLinkedList2.setParameter("textureMatrix", texMatrix);
                            perPixelLinkedList2.setParameter("haveTexture", 1.f);
                        }
                        if (m_normals[i].getMaterial().getType() == Material::WATER) {
                            perPixelLinkedList2.setParameter("water", 1.0f);
                        } else {
                            perPixelLinkedList2.setParameter("water", 0.0f);
                        }
                        if (core::Application::app != nullptr) {
                            float time = core::Application::getTimeClk().getElapsedTime().asSeconds();
                            perPixelLinkedList2.setParameter("time", time);
                        }
                        unsigned int p = m_normals[i].getAllVertices().getPrimitiveType();
                        for (unsigned int j = 0; j < m_normals[i].getAllVertices().getVertexCount(); j++) {
                            //std::cout<<"position : "<<m_normals[i].getAllVertices()[j].position.x<<std::endl;
                            vbBindlessTex[p].append(m_normals[i].getAllVertices()[j],(m_normals[i].getMaterial().getTexture() != nullptr) ? m_normals[i].getMaterial().getTexture()->getId() : 0);
                        }

                    }
                }
                currentStates.blendMode = sf::BlendNone;
                currentStates.shader = &perPixelLinkedList2;
                currentStates.texture = nullptr;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vbBindlessTex[p].getVertexCount() > 0) {

                        vbBindlessTex[p].update();
                        frameBuffer.drawVertexBuffer(vbBindlessTex[p], currentStates);
                    }
                }

                glCheck(glFinish());
                glCheck(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
                vb.clear();
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
            } else {
                /*currentStates.blendMode = sf::BlendNone;
                currentStates.shader=&perPixelLinkedList;

                for (unsigned int i = 0; i < m_instances.size(); i++) {
                   if (m_instances[i].getAllVertices().getVertexCount() > 0) {
                        if (m_instances[i].getMaterial().getTexture() == nullptr) {
                            perPixelLinkedList.setParameter("haveTexture", 0.f);
                        } else {
                            perPixelLinkedList.setParameter("haveTexture", 1.f);
                        }
                        currentStates.texture = m_instances[i].getMaterial().getTexture();
                        frameBuffer.draw(m_instances[i].getAllVertices(), currentStates);
                    }
                }
                glCheck(glFinish());
                glCheck(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));

                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );


                currentStates.shader = &perPixelLinkedListP2;
                /*for (unsigned int i = 0; i < m_instances.size(); i++) {
                   if (m_instances[i].getAllVertices().getVertexCount() > 0) {
                        frameBuffer.draw(m_instances[i].getAllVertices(), currentStates);
                   }
                //glCheck(glDepthMask(GL_TRUE));

                //quad.setCenter(frameBuffer.getView().getPosition());
                frameBuffer.draw(quad, currentStates);
                glCheck(glFinish());*/
                frameBuffer.display();
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
        std::vector<Entity*> PerPixelLinkedListRenderComponent::getEntities() {
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
        bool PerPixelLinkedListRenderComponent::loadEntitiesOnComponent(std::vector<Entity*> vEntities) {
            batcher.clear();
            normalBatcher.clear();


            //std::cout<<"load tile"<<std::endl;
            for (unsigned int i = 0; i < vEntities.size(); i++) {
                if ( vEntities[i]->isLeaf()) {

                    for (unsigned int j = 0; j <  vEntities[i]->getNbFaces(); j++) {
                         if (vEntities[i]->getDrawMode() == Entity::INSTANCED) {
                            batcher.addFace( vEntities[i]->getFace(j));
                         } else {
                            //std::cout<<"add face to batch : "<<vEntities[i]->getFace(j)->getTransformMatrix().getMatrix()<<std::endl;
                            /*if (vEntities[i]->getRootType() == "E_WALL")
                                std::cout<<"size  : "<<vEntities[i]->getSize();/*<<"transform  : "<<std::endl<<vEntities[i]->getFace(0)->getTransformMatrix().getMatrix()<<std::endl;*/
                            normalBatcher.addFace(vEntities[i]->getFace(j));
                         }
                    }
                }
            }
            m_instances = batcher.getInstances();
            m_normals = normalBatcher.getInstances();

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
            glDeleteBuffers(1, &vboWorldMatrices);
            glDeleteBuffers(1, &ubo);
        }
        #endif // VULKAN
    }
}
