#include "../../../include/odfaeg/Graphics/reflectRefractRenderComponent.hpp"
#ifndef VULKAN
#include "glCheck.h"
#endif // VULKAN
namespace odfaeg {
    namespace graphic {
        #ifdef VULKAN
        #else

        ReflectRefractRenderComponent::ReflectRefractRenderComponent (RenderWindow& window, int layer, std::string expression, window::ContextSettings settings) :
            HeavyComponent(window, math::Vec3f(window.getView().getPosition().x, window.getView().getPosition().y, layer),
                          math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0),
                          math::Vec3f(window.getView().getSize().x + window.getView().getSize().x * 0.5f, window.getView().getPosition().y + window.getView().getSize().y * 0.5f, layer)),
            view(window.getView()),
            expression(expression)
            {
            if (window.getView().getSize().x > window.getView().getSize().y) {
                squareSize = window.getView().getSize().x;
            } else {
                squareSize = window.getView().getSize().y;
            }
            quad = RectangleShape(math::Vec3f(squareSize, squareSize, squareSize * 0.5f));
            quad.move(math::Vec3f(-squareSize * 0.5f, -squareSize * 0.5f, 0));
            dirs[0] = math::Vec3f(1, 0, 0);
            dirs[1] = math::Vec3f(-1, 0, 0);
            dirs[2] = math::Vec3f(0, 1, 0);
            dirs[3] = math::Vec3f(0, -1, 0);
            dirs[4] = math::Vec3f(0, 0, 1);
            dirs[5] = math::Vec3f(0, 0, -1);
            depthBuffer.create(window.getView().getSize().x, window.getView().getSize().y, settings);
            glCheck(glGenTextures(1, &depthTex));
            glCheck(glBindTexture(GL_TEXTURE_2D, depthTex));
            glCheck(glTexStorage2D(GL_TEXTURE_2D,1,GL_RGBA32F,window.getView().getSize().x, window.getView().getSize().y));
            glCheck(glBindImageTexture(0, depthTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F));
            glCheck(glBindTexture(GL_TEXTURE_2D, 0));
            std::vector<GLfloat> depthClearBuf(window.getView().getSize().x*window.getView().getSize().y*4, 0);
            glCheck(glGenBuffers(1, &clearBuf2));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf2));
            glCheck(glBufferData(GL_PIXEL_UNPACK_BUFFER, depthClearBuf.size() * sizeof(GLfloat),
            &depthClearBuf[0], GL_STATIC_COPY));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
            depthBufferSprite = Sprite(depthBuffer.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), sf::IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));
            alphaBuffer.create(window.getView().getSize().x, window.getView().getSize().y, settings);
            glCheck(glGenTextures(1, &alphaTex));
            glCheck(glBindTexture(GL_TEXTURE_2D, alphaTex));
            glCheck(glTexStorage2D(GL_TEXTURE_2D,1,GL_RGBA32F,window.getView().getSize().x, window.getView().getSize().y));
            glCheck(glBindImageTexture(0, alphaTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F));
            glCheck(glBindTexture(GL_TEXTURE_2D, 0));
            std::vector<GLfloat> alphaClearBuf(window.getView().getSize().x*window.getView().getSize().y*4, 0);
            glCheck(glGenBuffers(1, &clearBuf3));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf3));
            glCheck(glBufferData(GL_PIXEL_UNPACK_BUFFER, alphaClearBuf.size() * sizeof(GLfloat),
            &alphaClearBuf[0], GL_STATIC_COPY));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
            alphaBufferSprite = Sprite(alphaBuffer.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), sf::IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));
            sf::Vector3i resolution ((int) window.getSize().x, (int) window.getSize().y, window.getView().getSize().z);
            settings.depthBits = 0;
            reflectRefractTex.create(window.getView().getSize().x, window.getView().getSize().y, settings);
            reflectRefractTex.setEnableCubeMap(true);
            reflectRefractTexSprite = Sprite(reflectRefractTex.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), sf::IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));
            environmentMap.create(squareSize, squareSize, settings, GL_TEXTURE_CUBE_MAP);
            GLuint maxNodes = 20 * squareSize * squareSize;
            GLint nodeSize = 5 * sizeof(GLfloat) + sizeof(GLuint);
            glCheck(glGenBuffers(1, &atomicBuffer));
            glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicBuffer));
            glCheck(glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW));
            glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0));
            glCheck(glGenBuffers(1, &linkedListBuffer));
            glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, linkedListBuffer));
            glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, maxNodes * nodeSize, NULL, GL_DYNAMIC_DRAW));
            glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
            glCheck(glGenTextures(1, &headPtrTex));
            glCheck(glBindTexture(GL_TEXTURE_2D, headPtrTex));
            glCheck(glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, squareSize, squareSize));
            glCheck(glBindImageTexture(0, headPtrTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI));
            glCheck(glBindTexture(GL_TEXTURE_2D, 0));
            std::vector<GLuint> headPtrClearBuf(squareSize*squareSize, 0xffffffff);
            glCheck(glGenBuffers(1, &clearBuf));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf));
            glCheck(glBufferData(GL_PIXEL_UNPACK_BUFFER, headPtrClearBuf.size() * sizeof(GLuint),
            &headPtrClearBuf[0], GL_STATIC_COPY));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
            core::FastDelegate<bool> signal (&ReflectRefractRenderComponent::needToUpdate, this);
            core::FastDelegate<void> slot (&ReflectRefractRenderComponent::drawNextFrame, this);
            core::Command cmd(signal, slot);
            getListener().connect("UPDATE", cmd);
            if (settings.versionMajor >= 4 && settings.versionMinor >= 3) {
                glGenBuffers(1, &vboWorldMatrices);
                const std::string linkedListVertexShader = R"(#version 460
                                                    layout (location = 0) in vec3 position;
                                                    layout (location = 1) in vec4 color;
                                                    layout (location = 2) in vec2 texCoords;
                                                    layout (location = 3) in vec3 normals;
                                                    layout (location = 4) in mat4 worldMat;
                                                    layout (location = 12) in uint textureIndex;
                                                    uniform mat4 projectionMatrix;
                                                    uniform mat4 viewMatrix;
                                                    uniform mat4 textureMatrix[)"+core::conversionUIntString(Texture::getAllTextures().size())+R"(];
                                                    out vec4 vColor;
                                                    out vec2 vTexCoord;
                                                    out uint texIndex;
                                                    void main() {
                                                        gl_Position = projectionMatrix * viewMatrix * worldMat * vec4(position, 1.f);
                                                        vTexCoord = (textureIndex != 0) ? (textureMatrix[textureIndex-1] * vec4(texCoords, 1.f, 1.f)).xy : texCoords;
                                                        vColor = color;
                                                        texIndex = textureIndex;
                                                    }
                                                    )";
                const std::string  linkedListNormalVertexShader = R"(#version 460
                                                        layout (location = 0) in vec3 position;
                                                        layout (location = 1) in vec4 color;
                                                        layout (location = 2) in vec2 texCoords;
                                                        layout (location = 3) in vec3 normals;
                                                        layout (location = 4) in uint textureIndex;
                                                        uniform mat4 textureMatrix[)"+core::conversionUIntString(Texture::getAllTextures().size())+R"(];
                                                        uniform mat4 projectionMatrix;
                                                        uniform mat4 viewMatrix;
                                                        out vec4 vColor;
                                                        out vec2 vTexCoord;
                                                        out uint texIndex;
                                                        void main () {
                                                            gl_Position = projectionMatrix * viewMatrix * vec4(position, 1.f);
                                                            vTexCoord = (textureIndex != 0) ? (textureMatrix[textureIndex-1] * vec4(texCoords, 1.f, 1.f)).xy : texCoords;
                                                            vColor = color;
                                                            texIndex = textureIndex;
                                                        })";
                const std::string vertexShader = R"(#version 460
                                                    layout (location = 0) in vec3 position;
                                                    layout (location = 1) in vec4 color;
                                                    layout (location = 2) in vec2 texCoords;
                                                    layout (location = 3) in vec3 normals;
                                                    layout (location = 4) in mat4 worldMat;
                                                    layout (location = 12) in uint textureIndex;
                                                    uniform mat4 projectionMatrix;
                                                    uniform mat4 viewMatrix;
                                                    uniform mat4 textureMatrix[)"+core::conversionUIntString(Texture::getAllTextures().size())+R"(];
                                                    out vec2 fTexCoords;
                                                    out vec4 frontColor;
                                                    out uint texIndex;
                                                    void main() {
                                                        gl_Position = projectionMatrix * viewMatrix * worldMat * vec4(position, 1.f);
                                                        fTexCoords = (textureIndex != 0) ? (textureMatrix[textureIndex-1] * vec4(texCoords, 1.f, 1.f)).xy : texCoords;
                                                        frontColor = color;
                                                        texIndex = textureIndex;
                                                    }
                                                    )";
                const std::string  normalVertexShader = R"(#version 460
                                                        layout (location = 0) in vec3 position;
                                                        layout (location = 1) in vec4 color;
                                                        layout (location = 2) in vec2 texCoords;
                                                        layout (location = 3) in vec3 normals;
                                                        layout (location = 4) in uint textureIndex;
                                                        uniform mat4 textureMatrix[)"+core::conversionUIntString(Texture::getAllTextures().size())+R"(];
                                                        uniform mat4 projectionMatrix;
                                                        uniform mat4 viewMatrix;
                                                        out vec2 fTexCoords;
                                                        out vec4 frontColor;
                                                        out uint texIndex;
                                                        void main () {
                                                            gl_Position = projectionMatrix * viewMatrix * vec4(position, 1.f);
                                                            fTexCoords = (textureIndex != 0) ? (textureMatrix[textureIndex-1] * vec4(texCoords, 1.f, 1.f)).xy : texCoords;
                                                            frontColor = color;
                                                            texIndex = textureIndex;
                                                        })";
                const std::string  linkedListVertexShader2 = R"(#version 460
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
                const std::string geometryShader = R"(#version 460
                                                      #extension GL_EXT_geometry_shader4 : enable
                                                      layout (triangles) in;
                                                      layout (triangle_strip, max_vertices = 4) out;
                                                      uniform int layer;
                                                      in vec4 vColor[];
                                                      in vec2 vTexCoord[];
                                                      out vec4 frontColor;
                                                      out vec2 fTexCoords;
                                                      void main() {
                                                        /*gl_Layer = layer;*/
                                                        gl_Position = gl_in[0].gl_Position;
                                                        frontColor = vColor[0];
                                                        fTexCoords = vTexCoord[0];
                                                        EmitVertex();
                                                        gl_Position = gl_in[1].gl_Position;
                                                        frontColor = vColor[1];
                                                        fTexCoords = vTexCoord[1];
                                                        EmitVertex();
                                                        gl_Position = gl_in[2].gl_Position;
                                                        frontColor = vColor[2];
                                                        fTexCoords = vTexCoord[2];
                                                        EmitVertex();
                                                        EndPrimitive();
                                                      }
                                                      )";
                const std::string perPixReflectRefractVertexShader = R"(#version 460
                                                                   layout (location = 0) in vec3 position;
                                                                   layout (location = 1) in vec4 color;
                                                                   layout (location = 2) in vec2 texCoords;
                                                                   layout (location = 3) in vec3 normals;
                                                                   layout (location = 4) in mat4 worldMat;
                                                                   uniform mat4 projectionMatrix;
                                                                   uniform mat4 viewMatrix;
                                                                   out vec3 pos;
                                                                   out vec4 frontColor;
                                                                   out vec3 normal;
                                                                   void main() {
                                                                       pos = vec3(worldMat * vec4(position, 1.0));
                                                                       gl_Position = projectionMatrix * viewMatrix * worldMat * vec4(position, 1.f);
                                                                       frontColor = color;
                                                                       normal = mat3(transpose(inverse(worldMat))) * normals;
                                                                   }
                                                                  )";
                const std::string perPixReflectRefractVertexNormalShader = R"(#version 460
                                                                   layout (location = 0) in vec3 position;
                                                                   layout (location = 1) in vec4 color;
                                                                   layout (location = 2) in vec2 texCoords;
                                                                   layout (location = 3) in vec3 normals;
                                                                   uniform mat4 projectionMatrix;
                                                                   uniform mat4 viewMatrix;
                                                                   out vec4 frontColor;
                                                                   out vec2 fTexCoords;
                                                                   out vec3 pos;
                                                                   out vec3 normal;
                                                                   void main() {
                                                                       gl_Position = projectionMatrix * viewMatrix * vec4(position, 1.f);
                                                                       frontColor = color;
                                                                       pos = position;
                                                                       normal = normals;
                                                                   }
                                                                  )";
                const std::string buildDepthBufferFragmentShader = R"(#version 460
                                                                      #extension GL_ARB_bindless_texture : enable
                                                                          in vec4 frontColor;
                                                                          in vec2 fTexCoords;
                                                                          in flat uint texIndex;
                                                                          layout(std140, binding=0) uniform ALL_TEXTURES {
                                                                              sampler2D textures[200];
                                                                          };
                                                                          uniform sampler2D texture;
                                                                          uniform float haveTexture;
                                                                          uniform float layer;
                                                                          uniform float nbLayers;

                                                                          layout(binding = 0, rgba32f) uniform image2D depthBuffer;
                                                                          layout (location = 0) out vec4 fColor;

                                                                          void main () {
                                                                              vec4 texel = (texIndex != 0) ? frontColor * texture2D(textures[texIndex-1], fTexCoords.xy) : frontColor;
                                                                              float z = gl_FragCoord.z;
                                                                              float l = layer / nbLayers;
                                                                              vec4 depth = imageLoad(depthBuffer,ivec2(gl_FragCoord.xy));
                                                                              if (l > depth.y || l == depth.y && z > depth.z) {
                                                                                fColor = vec4(0, l, z, texel.a);
                                                                                imageStore(depthBuffer,ivec2(gl_FragCoord.xy),vec4(0,l,z,texel.a));
                                                                              } else {
                                                                                fColor = depth;
                                                                              }
                                                                          }
                                                                        )";
                const std::string buildAlphaBufferFragmentShader = R"(#version 460
                                                                      #extension GL_ARB_bindless_texture : enable
                                                                      layout(std140, binding=0) uniform ALL_TEXTURES {
                                                                        sampler2D textures[200];
                                                                      };
                                                                      layout(binding = 0, rgba32f) uniform image2D alphaBuffer;
                                                                      layout (location = 0) out vec4 fColor;
                                                                      uniform sampler2D texture;
                                                                      uniform sampler2D depthBuffer;
                                                                      uniform float haveTexture;
                                                                      uniform float layer;
                                                                      uniform float nbLayers;
                                                                      uniform vec3 resolution;
                                                                      in vec4 frontColor;
                                                                      in vec2 fTexCoords;
                                                                      in flat uint texIndex;
                                                                      void main() {
                                                                          vec4 texel = (texIndex != 0) ? frontColor * texture2D(textures[texIndex-1], fTexCoords.xy) : frontColor;
                                                                          float current_alpha = texel.a;
                                                                          vec2 position = (gl_FragCoord.xy / resolution.xy);
                                                                          vec4 depth = texture2D (depthBuffer, position);
                                                                          vec4 alpha = imageLoad(alphaBuffer,ivec2(gl_FragCoord.xy));
                                                                          float l = layer / nbLayers;
                                                                          float z = gl_FragCoord.z;
                                                                          if ((l > depth.y || l == depth.y && z > depth.z) && current_alpha > alpha.a) {
                                                                              fColor = vec4(0, l, z, current_alpha);
                                                                              imageStore(alphaBuffer,ivec2(gl_FragCoord.xy),vec4(0, l, z, current_alpha));
                                                                          } else {
                                                                              fColor = alpha;
                                                                          }
                                                                      }
                                                                      )";
                const std::string buildFramebufferShader = R"(#version 460
                                                                in vec4 frontColor;
                                                                in vec3 pos;
                                                                in flat vec3 normal;
                                                                uniform vec3 cameraPos;
                                                                uniform samplerCube sceneBox;
                                                                uniform sampler2D alphaBuffer;
                                                                uniform vec3 resolution;
                                                                layout (location = 0) out vec4 fColor;
                                                                void main () {
                                                                    vec2 position = (gl_FragCoord.xy / resolution.xy);
                                                                    vec4 alpha = texture2D(alphaBuffer, position);
                                                                    float ratio = 1.00 / 1.33;
                                                                    vec3 i = (pos - cameraPos);
                                                                    vec3 r = refract (i, normalize(normal), ratio);
                                                                    fColor = texture(sceneBox, r) * (1 - alpha.a);
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
                                                      layout(std140, binding=0) uniform ALL_TEXTURES {
                                                          sampler2D textures[200];
                                                      };
                                                      uniform uint maxNodes;
                                                      uniform float haveTexture;
                                                      uniform sampler2D texture;
                                                      in vec4 frontColor;
                                                      in vec2 fTexCoords;
                                                      in flat uint texIndex;
                                                      layout (location = 0) out vec4 fcolor;
                                                      void main() {
                                                           uint nodeIdx = atomicCounterIncrement(nextNodeCounter);
                                                           vec4 texel = (texIndex != 0) ? frontColor * texture2D(textures[texIndex-1], fTexCoords.xy) : frontColor;
                                                           if (nodeIdx < maxNodes) {
                                                                uint prevHead = imageAtomicExchange(headPointers, ivec2(gl_FragCoord.xy), nodeIdx);
                                                                nodes[nodeIdx].color = texel;
                                                                nodes[nodeIdx].depth = gl_FragCoord.z;
                                                                nodes[nodeIdx].next = prevHead;
                                                           }
                                                           fcolor = vec4(0, 0, 0, 0);
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
                      }
                      fcolor = color;
                   })";
                if (!sBuildDepthBuffer.loadFromMemory(vertexShader, buildDepthBufferFragmentShader)) {
                    throw core::Erreur(50, "Error, failed to load build depth buffer shader", 3);
                }
                if (!sBuildDepthBufferNormal.loadFromMemory(normalVertexShader, buildDepthBufferFragmentShader)) {
                    throw core::Erreur(51, "Error, failed to load build normal depth buffer shader", 3);
                }
                if (!sReflectRefract.loadFromMemory(perPixReflectRefractVertexShader, buildFramebufferShader)) {
                    throw core::Erreur(57, "Error, failed to load reflect refract shader", 3);
                }
                if (!sReflectRefractNormal.loadFromMemory(perPixReflectRefractVertexNormalShader, buildFramebufferShader)) {
                    throw core::Erreur(58, "Error, failed to load reflect refract normal shader", 3);
                }
                if (!sLinkedList.loadFromMemory(vertexShader, fragmentShader/*, geometryShader*/)) {
                    throw core::Erreur(58, "Error, failed to load per pixel linked list shader", 3);
                }
                if (!sLinkedListNormal.loadFromMemory(normalVertexShader, fragmentShader/*, geometryShader*/)) {
                    throw core::Erreur(58, "Error, failed to load per pixel linked list normal shader", 3);
                }
                if (!sLinkedList2.loadFromMemory(linkedListVertexShader2, fragmentShader2)) {
                    throw core::Erreur(59, "Error, failed to load per pixel linked list 2 shader", 3);
                }
                if (!sBuildAlphaBuffer.loadFromMemory(vertexShader,buildAlphaBufferFragmentShader)) {
                    throw core::Erreur(60, "Error, failed to load build alpha buffer shader", 3);
                }
                if (!sBuildAlphaBufferNormal.loadFromMemory(normalVertexShader,buildAlphaBufferFragmentShader)) {
                    throw core::Erreur(60, "Error, failed to load build alpha normal buffer shader", 3);
                }
                sBuildDepthBuffer.setParameter("texture", Shader::CurrentTexture);
                sBuildDepthBufferNormal.setParameter("texture", Shader::CurrentTexture);
                sBuildAlphaBuffer.setParameter("texture", Shader::CurrentTexture);
                sBuildAlphaBuffer.setParameter("depthBuffer", depthBuffer.getTexture());
                sBuildAlphaBuffer.setParameter("resolution", resolution.x, resolution.y, resolution.z);
                sBuildAlphaBufferNormal.setParameter("texture", Shader::CurrentTexture);
                sBuildAlphaBufferNormal.setParameter("depthBuffer", depthBuffer.getTexture());
                sBuildAlphaBufferNormal.setParameter("resolution", resolution.x, resolution.y, resolution.z);
                sReflectRefract.setParameter("resolution", resolution.x, resolution.y, resolution.z);
                sReflectRefract.setParameter("alphaBuffer", alphaBuffer.getTexture());
                sReflectRefract.setParameter("sceneBox", Shader::CurrentTexture);
                sReflectRefractNormal.setParameter("resolution", resolution.x, resolution.y, resolution.z);
                sReflectRefractNormal.setParameter("sceneBox", Shader::CurrentTexture);
                sReflectRefractNormal.setParameter("alphaBuffer", alphaBuffer.getTexture());
                sLinkedList.setParameter("maxNodes", maxNodes);
                sLinkedList.setParameter("texture", Shader::CurrentTexture);
                sLinkedListNormal.setParameter("maxNodes", maxNodes);
                sLinkedListNormal.setParameter("texture", Shader::CurrentTexture);
                View defaultView = window.getDefaultView();
                defaultView.setPerspective(-squareSize * 0.5f, squareSize * 0.5f, -squareSize * 0.5f, squareSize * 0.5f, 0, 1000);
                math::Matrix4f viewMatrix = defaultView.getViewMatrix().getMatrix().transpose();
                math::Matrix4f projMatrix = defaultView.getProjMatrix().getMatrix().transpose();
                sLinkedList2.setParameter("viewMatrix", viewMatrix);
                sLinkedList2.setParameter("projectionMatrix", projMatrix);
            }
            std::vector<Texture*> allTextures = Texture::getAllTextures();
            Samplers allSamplers{};
            std::vector<math::Matrix4f> textureMatrices;
            for (unsigned int i = 0; i < allTextures.size(); i++) {
                textureMatrices.push_back(allTextures[i]->getTextureMatrix());
                GLuint64 handle_texture = allTextures[i]->getTextureHandle();
                allTextures[i]->makeTextureResident(handle_texture);
                allSamplers.tex[i].handle = handle_texture;
                //std::cout<<"add texture i : "<<i<<" id : "<<allTextures[i]->getNativeHandle()<<std::endl;
            }
            sBuildDepthBuffer.setParameter("textureMatrix", textureMatrices);
            sBuildDepthBufferNormal.setParameter("textureMatrix", textureMatrices);
            sBuildAlphaBuffer.setParameter("textureMatrix", textureMatrices);
            sBuildAlphaBufferNormal.setParameter("textureMatrix", textureMatrices);
            sLinkedList.setParameter("textureMatrix", textureMatrices);
            sLinkedListNormal.setParameter("textureMatrix", textureMatrices);

            unsigned int ubid;
            glCheck(ubid = glGetUniformBlockIndex(sLinkedList.getHandle(), "ALL_TEXTURES"));
            glCheck(glUniformBlockBinding(sLinkedList.getHandle(),    ubid, 0));
            glCheck(ubid = glGetUniformBlockIndex(sLinkedListNormal.getHandle(), "ALL_TEXTURES"));
            //std::cout<<"shader handle : "<<sLinkedListNormal.getHandle()<<std::endl;
            glCheck(glUniformBlockBinding(sLinkedListNormal.getHandle(),    ubid, 0));
            glCheck(ubid = glGetUniformBlockIndex(sBuildDepthBuffer.getHandle(), "ALL_TEXTURES"));
            glCheck(glUniformBlockBinding(sBuildDepthBuffer.getHandle(),    ubid, 0));
            glCheck(ubid = glGetUniformBlockIndex(sBuildDepthBufferNormal.getHandle(), "ALL_TEXTURES"));
            glCheck(glUniformBlockBinding(sBuildDepthBufferNormal.getHandle(),    ubid, 0));
            glCheck(ubid = glGetUniformBlockIndex(sBuildAlphaBuffer.getHandle(), "ALL_TEXTURES"));
            glCheck(glUniformBlockBinding(sBuildAlphaBuffer.getHandle(),    ubid, 0));
            //std::cout<<"shader handle : "<<sBuildAlphaBufferNormal.getHandle()<<std::endl;
            glCheck(ubid = glGetUniformBlockIndex(sBuildAlphaBufferNormal.getHandle(), "ALL_TEXTURES"));
            glCheck(glUniformBlockBinding(sBuildAlphaBufferNormal.getHandle(),    ubid, 0));

            //std::cout<<"ubid : "<<ubid<<std::endl;
            backgroundColor = sf::Color::Transparent;
            glCheck(glGenBuffers(1, &ubo));
            glCheck(glBindBuffer(GL_UNIFORM_BUFFER, ubo));
            glCheck(glBufferData(GL_UNIFORM_BUFFER, sizeof(Samplers),allSamplers.tex, GL_STATIC_DRAW));
            glCheck(glBindBuffer(GL_UNIFORM_BUFFER, 0));
            glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
            depthBuffer.setActive();
            glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
            alphaBuffer.setActive();
            glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
            environmentMap.setActive();
            //std::cout<<"size : "<<sizeof(Samplers)<<" "<<alignof (alignas(16) uint64_t[200])<<std::endl;

            /*glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo2));
            glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo3));*/
            glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicBuffer));
            glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, linkedListBuffer));

            for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                vbBindlessTex[i].setPrimitiveType(static_cast<sf::PrimitiveType>(i));
            }
        }
        void ReflectRefractRenderComponent::loadTextureIndexes() {
            std::vector<Texture*> allTextures = Texture::getAllTextures();
            Samplers allSamplers{};
            std::vector<math::Matrix4f> textureMatrices;
            for (unsigned int i = 0; i < allTextures.size(); i++) {
                textureMatrices.push_back(allTextures[i]->getTextureMatrix());
                GLuint64 handle_texture = allTextures[i]->getTextureHandle();
                allTextures[i]->makeTextureResident(handle_texture);
                allSamplers.tex[i].handle = handle_texture;
                //std::cout<<"add texture i : "<<i<<" id : "<<allTextures[i]->getNativeHandle()<<std::endl;
            }
            glCheck(glBindBuffer(GL_UNIFORM_BUFFER, ubo));
            glCheck(glBufferData(GL_UNIFORM_BUFFER, sizeof(Samplers),allSamplers.tex, GL_STATIC_DRAW));
            glCheck(glBindBuffer(GL_UNIFORM_BUFFER, 0));
        }
        void ReflectRefractRenderComponent::onVisibilityChanged(bool visible) {
            if (visible) {
                glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicBuffer));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, linkedListBuffer));
            } else {
                glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicBuffer));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, linkedListBuffer));
            }
        }
        void ReflectRefractRenderComponent::drawNextFrame() {
            if (reflectRefractTex.getSettings().versionMajor >= 4 && reflectRefractTex.getSettings().versionMinor >= 3) {
                /*glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicBuffer));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, linkedListBuffer));*/
                RenderStates currentStates;
                math::Matrix4f viewMatrix = view.getViewMatrix().getMatrix().transpose();
                math::Matrix4f projMatrix = view.getProjMatrix().getMatrix().transpose();
                sBuildDepthBuffer.setParameter("viewMatrix", viewMatrix);
                sBuildDepthBuffer.setParameter("projectionMatrix", projMatrix);
                sBuildDepthBufferNormal.setParameter("viewMatrix", viewMatrix);
                sBuildDepthBufferNormal.setParameter("projectionMatrix", projMatrix);
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    vbBindlessTex[p].clear();
                }
                std::vector<TransformMatrix*> tm;
                for (unsigned int i = 0; i < m_reflInstances.size(); i++) {
                    if (m_reflInstances[i].getAllVertices().getVertexCount() > 0) {
                        if (m_reflInstances[i].getMaterial().getTexture() != nullptr) {
                            math::Matrix4f texMatrix = m_reflInstances[i].getMaterial().getTexture()->getTextureMatrix();
                            //sBuildDepthBuffer.setParameter("textureMatrix", texMatrix);
                            sBuildDepthBuffer.setParameter("haveTexture", 1.f);
                        } else {
                            sBuildDepthBuffer.setParameter("haveTexture", 0.f);
                        }
                        float layer = m_reflInstances[i].getVertexArrays()[0]->getEntity()->getLayer();
                        sBuildDepthBuffer.setParameter("layer", layer);

                        sBuildDepthBuffer.setParameter("nbLayers",GameObject::getNbLayers());
                        matrices.clear();
                        tm = m_reflInstances[i].getTransforms();
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
                        if (m_reflInstances[i].getVertexArrays().size() > 0) {
                            Entity* entity = m_reflInstances[i].getVertexArrays()[0]->getEntity();
                            for (unsigned int j = 0; j < m_reflInstances[i].getVertexArrays().size(); j++) {
                                if (entity == m_reflInstances[i].getVertexArrays()[j]->getEntity()) {

                                    unsigned int p = m_reflInstances[i].getVertexArrays()[j]->getPrimitiveType();
                                    for (unsigned int k = 0; k < m_reflInstances[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                        //std::cout<<"add vertices : "<<" i : "<<i<<" j : "<<"k : "<<k<<" size  : "<<vbBindlessTex[p].getVertexCount()<<std::endl;
                                        vbBindlessTex[p].append((*m_reflInstances[i].getVertexArrays()[j])[k], (m_reflInstances[i].getMaterial().getTexture() != nullptr) ? m_reflInstances[i].getMaterial().getTexture()->getNativeHandle() : 0);
                                    }
                                }
                            }
                        }
                    }
                }
                currentStates.blendMode = sf::BlendNone;
                currentStates.shader = &sBuildDepthBuffer;
                currentStates.texture = nullptr;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        vbBindlessTex[p].update();
                        depthBuffer.drawInstanced(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), 0, vbBindlessTex[p].getVertexCount(), tm.size(), currentStates, vboWorldMatrices);
                        vbBindlessTex[p].clear();
                    }
                }

                for (unsigned int i = 0; i < m_reflNormals.size(); i++) {
                    if (m_reflNormals[i].getAllVertices().getVertexCount() > 0) {
                        if (m_reflNormals[i].getMaterial().getTexture() != nullptr) {
                            math::Matrix4f texMatrix = m_reflNormals[i].getMaterial().getTexture()->getTextureMatrix();
                            sBuildDepthBufferNormal.setParameter("textureMatrix", texMatrix);
                            sBuildDepthBufferNormal.setParameter("haveTexture", 1.f);
                        } else {
                            sBuildDepthBufferNormal.setParameter("haveTexture", 0.f);
                        }
                        float layer = m_reflNormals[i].getVertexArrays()[0]->getEntity()->getLayer();
                        sBuildDepthBuffer.setParameter("layer", layer);
                        sBuildDepthBuffer.setParameter("nbLayers",GameObject::getNbLayers());
                        vb.clear();
                        vb.setPrimitiveType(m_reflNormals[i].getAllVertices().getPrimitiveType());
                        for (unsigned int j = 0; j < m_reflNormals[i].getAllVertices().getVertexCount(); j++) {
                            vb.append(m_reflNormals[i].getAllVertices()[j]);
                        }
                        vb.update();
                        currentStates.blendMode = sf::BlendNone;
                        currentStates.shader = &sBuildDepthBufferNormal;
                        currentStates.texture = m_reflNormals[i].getMaterial().getTexture();
                        depthBuffer.drawVertexBuffer(vb, currentStates);
                    }
                }
                glCheck(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
                depthBuffer.display();
                sBuildAlphaBuffer.setParameter("viewMatrix", viewMatrix);
                sBuildAlphaBuffer.setParameter("projectionMatrix", projMatrix);
                sBuildAlphaBufferNormal.setParameter("viewMatrix", viewMatrix);
                sBuildAlphaBufferNormal.setParameter("projectionMatrix", projMatrix);
                for (unsigned int i = 0; i < m_instances.size(); i++) {
                    if (m_instances[i].getAllVertices().getVertexCount() > 0) {
                        if (m_instances[i].getMaterial().getTexture() != nullptr) {
                            math::Matrix4f texMatrix = m_instances[i].getMaterial().getTexture()->getTextureMatrix();
                            sBuildAlphaBuffer.setParameter("textureMatrix", texMatrix);
                            sBuildAlphaBuffer.setParameter("haveTexture", 1.f);
                        } else {
                            sBuildAlphaBuffer.setParameter("haveTexture", 0.f);
                        }
                        float layer = m_instances[i].getVertexArrays()[0]->getEntity()->getLayer();
                        sBuildAlphaBuffer.setParameter("layer", layer);
                        sBuildAlphaBuffer.setParameter("nbLayers",GameObject::getNbLayers());
                        vb.clear();
                        vb.setPrimitiveType(m_instances[i].getVertexArrays()[0]->getPrimitiveType());
                        matrices.clear();
                        std::vector<TransformMatrix*> tm = m_instances[i].getTransforms();
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
                                    for (unsigned int k = 0; k < m_instances[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                        vb.append((*m_instances[i].getVertexArrays()[j])[k]);
                                    }
                                }
                            }
                            vb.update();
                        }
                        currentStates.blendMode = sf::BlendNone;
                        currentStates.shader = &sBuildAlphaBuffer;
                        currentStates.texture = m_instances[i].getMaterial().getTexture();
                        alphaBuffer.drawInstanced(vb, m_instances[i].getVertexArrays()[0]->getPrimitiveType(), 0, m_instances[i].getVertexArrays()[0]->getVertexCount(), tm.size(), currentStates, vboWorldMatrices);
                    }
                }
                for (unsigned int i = 0; i < m_normals.size(); i++) {
                    if (m_normals[i].getAllVertices().getVertexCount() > 0) {
                        if (m_normals[i].getMaterial().getTexture() != nullptr) {
                            math::Matrix4f texMatrix = m_normals[i].getMaterial().getTexture()->getTextureMatrix();
                            //sBuildAlphaBufferNormal.setParameter("textureMatrix", texMatrix);
                            sBuildAlphaBufferNormal.setParameter("haveTexture", 1.f);
                        } else {
                            sBuildAlphaBufferNormal.setParameter("haveTexture", 0.f);
                        }
                        float layer = m_normals[i].getVertexArrays()[0]->getEntity()->getLayer();
                        sBuildAlphaBufferNormal.setParameter("layer", layer);
                        sBuildAlphaBufferNormal.setParameter("nbLayers",GameObject::getNbLayers());
                        //std::cout<<"layer : "<<layer<<" nb layers : "<<Entity::getNbLayers()<<std::endl;
                        unsigned int p = m_normals[i].getAllVertices().getPrimitiveType();
                        for (unsigned int j = 0; j < m_normals[i].getAllVertices().getVertexCount(); j++) {
                            vbBindlessTex[p].append(m_normals[i].getAllVertices()[j],(m_normals[i].getMaterial().getTexture() != nullptr) ? m_normals[i].getMaterial().getTexture()->getNativeHandle() : 0);
                        }
                    }
                }
                currentStates.blendMode = sf::BlendNone;
                currentStates.shader = &sBuildAlphaBufferNormal;
                currentStates.texture = nullptr;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        vbBindlessTex[p].update();
                        alphaBuffer.drawVertexBuffer(vbBindlessTex[p], currentStates);
                        vbBindlessTex[p].clear();
                    }
                }
                glCheck(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
                alphaBuffer.display();
                View reflectView;
                if (view.isOrtho()) {
                    reflectView = View (squareSize, squareSize, view.getViewport().getPosition().z, view.getViewport().getSize().z);
                } else {
                    reflectView = View (squareSize, squareSize, 90, view.getViewport().getPosition().z, view.getViewport().getSize().z);
                }
                for (unsigned int t = 0; t < 2; t++) {
                    std::vector<Instance> instances = (t == 0) ? m_reflInstances : m_reflNormals;
                    for (unsigned int n = 0; n < instances.size(); n++) {
                        if (instances[n].getAllVertices().getVertexCount() > 0) {
                            Entity* entity = instances[n].getVertexArrays()[0]->getEntity()->getRootEntity();
                            math::Vec3f scale(1, 1, 1);
                            if (entity->getSize().x > squareSize) {
                                scale.x = entity->getSize().x / squareSize;
                            }
                            if (entity->getSize().y > squareSize) {
                                scale.y = entity->getSize().y / squareSize;
                            }
                            reflectView.setScale(scale.x, scale.y, scale.z);
                            reflectView.setCenter(entity->getPosition()+entity->getSize()*0.5f);
                            for (unsigned int m = 0; m < 6; m++) {
                                math::Vec3f target = reflectView.getPosition() + dirs[m];
                                reflectView.lookAt(target.x, target.y, target.z);
                                /*std::vector<Entity*> visibleReflEntities = World::getEntitiesInRect(reflectView.getViewVolume(), expression);
                                std::vector<Entity*> vEntities;
                                for (unsigned int j = 0; j < visibleEntities.size(); j++)  {
                                    if (!visibleEntities[j]->isReflectable()) {
                                        vEntities.push_back(visibleEntities[j]);
                                    }
                                }
                                rvBatcher.clear();
                                normalRvBatcher.clear();
                                for (unsigned int j = 0; j < vEntities.size(); j++) {
                                    for (unsigned int n = 0; n < vEntities[j]->getNbFaces(); n++) {
                                        if (vEntities[j]->getDrawMode() == Entity::INSTANCED) {
                                            rvBatcher.addFace(vEntities[j]->getFace(n));
                                        } else {
                                            normalRvBatcher.addFace(vEntities[j]->getFace(n));
                                        }
                                    }
                                }
                                std::vector<Instance> rvInstances = rvBatcher.getInstances();
                                std::vector<Instance> rvNormals = normalRvBatcher.getInstances();*/
                                environmentMap.setView(reflectView);
                                environmentMap.setActive();
                                GLuint zero = 0;
                                glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicBuffer));
                                glCheck(glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero));
                                glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0));
                                glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf));
                                glCheck(glBindTexture(GL_TEXTURE_2D, headPtrTex));
                                glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, squareSize, squareSize, GL_RED_INTEGER,
                                GL_UNSIGNED_INT, NULL));
                                glCheck(glBindTexture(GL_TEXTURE_2D, 0));
                                glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
                                environmentMap.resetGLStates();
                                environmentMap.selectCubemapFace(m);
                                viewMatrix = reflectView.getViewMatrix().getMatrix().transpose();
                                projMatrix = reflectView.getProjMatrix().getMatrix().transpose();
                                sLinkedList.setParameter("viewMatrix", viewMatrix);
                                sLinkedList.setParameter("projectionMatrix", projMatrix);
                                for (unsigned int i = 0; i < m_instances.size(); i++) {
                                    if (m_instances[i].getAllVertices().getVertexCount() > 0) {
                                        matrices.clear();
                                        std::vector<TransformMatrix*> tm = m_instances[i].getTransforms();
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
                                        vb.clear();
                                        vb.setPrimitiveType(m_instances[i].getVertexArrays()[0]->getPrimitiveType());
                                        if (m_instances[i].getVertexArrays().size() > 0) {
                                            Entity* entity = m_instances[i].getVertexArrays()[0]->getEntity();
                                            for (unsigned int j = 0; j < m_instances[i].getVertexArrays().size(); j++) {
                                                if (entity == m_instances[i].getVertexArrays()[j]->getEntity()) {
                                                    for (unsigned int k = 0; k < m_instances[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                                        vb.append((*m_instances[i].getVertexArrays()[j])[k]);
                                                    }
                                                }
                                            }
                                            vb.update();
                                        }
                                        currentStates.blendMode = sf::BlendNone;
                                        currentStates.shader = &sLinkedList;
                                        currentStates.texture =m_instances[i].getMaterial().getTexture();
                                        if (m_instances[i].getMaterial().getTexture() != nullptr) {
                                            math::Matrix4f texMatrix = m_instances[i].getMaterial().getTexture()->getTextureMatrix();
                                            sLinkedList.setParameter("textureMatrix", texMatrix);
                                            sLinkedList.setParameter("haveTexture", 1.f);
                                        } else {
                                            sLinkedList.setParameter("haveTexture", 0.f);
                                        }
                                        environmentMap.drawInstanced(vb, m_instances[i].getVertexArrays()[0]->getPrimitiveType(), 0, m_instances[i].getVertexArrays()[0]->getVertexCount(), tm.size(), currentStates, vboWorldMatrices);
                                    }
                                }
                                viewMatrix = reflectView.getViewMatrix().getMatrix().transpose();
                                projMatrix = reflectView.getProjMatrix().getMatrix().transpose();
                                sLinkedListNormal.setParameter("viewMatrix", viewMatrix);
                                sLinkedListNormal.setParameter("projectionMatrix", projMatrix);
                                for (unsigned int i = 0; i < m_normals.size(); i++) {
                                   if (m_normals[i].getAllVertices().getVertexCount() > 0) {
                                        if (m_normals[i].getMaterial().getTexture() == nullptr) {
                                            sLinkedListNormal.setParameter("haveTexture", 0.f);
                                        } else {
                                            math::Matrix4f texMatrix = m_normals[i].getMaterial().getTexture()->getTextureMatrix();
                                            //sLinkedListNormal.setParameter("textureMatrix", texMatrix);
                                            sLinkedListNormal.setParameter("haveTexture", 1.f);
                                        }
                                        unsigned int p = m_normals[i].getAllVertices().getPrimitiveType();
                                        for (unsigned int j = 0; j < m_normals[i].getAllVertices().getVertexCount(); j++) {
                                            vbBindlessTex[p].append(m_normals[i].getAllVertices()[j],(m_normals[i].getMaterial().getTexture() != nullptr) ? m_normals[i].getMaterial().getTexture()->getNativeHandle() : 0);
                                        }

                                    }
                                }
                                currentStates.blendMode = sf::BlendNone;
                                currentStates.shader = &sLinkedListNormal;
                                currentStates.texture = nullptr;
                                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                                    if (vbBindlessTex[p].getVertexCount() > 0) {
                                        vbBindlessTex[p].update();
                                        environmentMap.drawVertexBuffer(vbBindlessTex[p], currentStates);
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
                                sLinkedList2.setParameter("worldMat", matrix);
                                currentStates.shader = &sLinkedList2;
                                currentStates.texture = nullptr;
                                environmentMap.drawVertexBuffer(vb, currentStates);
                                glCheck(glFinish());
                                glCheck(glMemoryBarrier(GL_ALL_BARRIER_BITS));
                            }
                            environmentMap.display();
                            viewMatrix = view.getViewMatrix().getMatrix().transpose();
                            projMatrix = view.getProjMatrix().getMatrix().transpose();
                            sReflectRefract.setParameter("viewMatrix", viewMatrix);
                            sReflectRefract.setParameter("projectionMatrix", projMatrix);
                            sReflectRefract.setParameter("cameraPos", view.getPosition().x, view.getPosition().y, view.getPosition().z);
                            for (unsigned int i = 0; i < m_reflInstances.size(); i++) {
                                if (m_reflInstances[i].getAllVertices().getVertexCount() > 0) {
                                    vb.clear();
                                    vb.setPrimitiveType(m_reflInstances[i].getVertexArrays()[0]->getPrimitiveType());
                                    matrices.clear();
                                    std::vector<TransformMatrix*> tm = m_reflInstances[i].getTransforms();
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
                                    if (m_reflInstances[i].getVertexArrays().size() > 0) {
                                        Entity* entity = m_reflInstances[i].getVertexArrays()[0]->getEntity();
                                        for (unsigned int j = 0; j < m_reflInstances[i].getVertexArrays().size(); j++) {
                                            if (entity == m_reflInstances[i].getVertexArrays()[j]->getEntity()) {
                                                for (unsigned int k = 0; k < m_reflInstances[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                                    vb.append((*m_reflInstances[i].getVertexArrays()[j])[k]);
                                                }
                                            }
                                        }
                                        vb.update();
                                    }
                                    currentStates.blendMode = sf::BlendNone;
                                    currentStates.shader = &sReflectRefract;
                                    currentStates.texture = &environmentMap.getTexture();
                                    reflectRefractTex.drawInstanced(vb, m_reflInstances[i].getVertexArrays()[0]->getPrimitiveType(), 0, m_reflInstances[i].getVertexArrays()[0]->getVertexCount(), tm.size(), currentStates, vboWorldMatrices);
                                }
                            }
                            viewMatrix = view.getViewMatrix().getMatrix().transpose();
                            projMatrix = view.getProjMatrix().getMatrix().transpose();
                            sReflectRefractNormal.setParameter("viewMatrix", viewMatrix);
                            sReflectRefractNormal.setParameter("projectionMatrix", projMatrix);
                            sReflectRefractNormal.setParameter("cameraPos", view.getPosition().x, view.getPosition().y, view.getPosition().z);
                            for (unsigned int i = 0; i < m_reflNormals.size(); i++) {
                                if (m_reflNormals[i].getAllVertices().getVertexCount() > 0) {
                                    vb.clear();
                                    vb.setPrimitiveType(m_reflNormals[i].getAllVertices().getPrimitiveType());
                                    for (unsigned int j = 0; j < m_reflNormals[i].getAllVertices().getVertexCount(); j++) {
                                        vb.append(m_reflNormals[i].getAllVertices()[j]);
                                    }
                                    vb.update();
                                    currentStates.blendMode = sf::BlendNone;
                                    currentStates.shader = &sReflectRefractNormal;
                                    //currentStates.texture = &environmentMap.getTexture();
                                    reflectRefractTex.drawVertexBuffer(vb, currentStates);
                                }
                            }
                        }
                    }
                }
                reflectRefractTex.display();
                /*glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0));
                glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, 0));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0));*/
            }
        }
        std::vector<Entity*> ReflectRefractRenderComponent::getEntities() {
            return visibleEntities;
        }
        bool ReflectRefractRenderComponent::loadEntitiesOnComponent(std::vector<Entity*> vEntities) {
            batcher.clear();
            normalBatcher.clear();
            reflBatcher.clear();
            reflNormalBatcher.clear();
            for (unsigned int i = 0; i < vEntities.size(); i++) {
                if ( vEntities[i]->isLeaf()) {
                    for (unsigned int j = 0; j <  vEntities[i]->getNbFaces(); j++) {
                        if (vEntities[i]->getFace(j)->getMaterial().isReflectable() || vEntities[i]->getFace(j)->getMaterial().isRefractable()) {
                            if (vEntities[i]->getDrawMode() == Entity::INSTANCED) {
                                reflBatcher.addFace( vEntities[i]->getFace(j));
                            } else {
                                reflNormalBatcher.addFace(vEntities[i]->getFace(j));
                            }
                        } else {
                            if (vEntities[i]->getDrawMode() == Entity::INSTANCED) {
                                batcher.addFace( vEntities[i]->getFace(j));
                            } else {
                                normalBatcher.addFace(vEntities[i]->getFace(j));
                            }
                        }
                    }
                }
            }
            m_instances = batcher.getInstances();
            m_normals = normalBatcher.getInstances();
            m_reflInstances = reflBatcher.getInstances();
            m_reflNormals = reflNormalBatcher.getInstances();
            visibleEntities = vEntities;
            update = true;
            return true;
        }
        bool ReflectRefractRenderComponent::needToUpdate() {
            return update;
        }
        void ReflectRefractRenderComponent::clear() {
            depthBuffer.clear(sf::Color::Transparent);
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf2));
            glCheck(glBindTexture(GL_TEXTURE_2D, depthTex));
            glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, view.getSize().x, view.getSize().y, GL_RGBA,
            GL_FLOAT, NULL));
            glCheck(glBindTexture(GL_TEXTURE_2D, 0));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
            alphaBuffer.clear(sf::Color::Transparent);
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf3));
            glCheck(glBindTexture(GL_TEXTURE_2D, alphaTex));
            glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, view.getSize().x, view.getSize().y, GL_RGBA,
            GL_FLOAT, NULL));
            glCheck(glBindTexture(GL_TEXTURE_2D, 0));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
            environmentMap.clear(sf::Color::Transparent);
            reflectRefractTex.clear(sf::Color::Transparent);
        }
        void ReflectRefractRenderComponent::setBackgroundColor (sf::Color color) {
            this->backgroundColor = color;
        }
        void ReflectRefractRenderComponent::setExpression (std::string expression) {
            this->expression = expression;
        }
        void ReflectRefractRenderComponent::draw (Drawable& drawable, RenderStates states) {

        }
        void ReflectRefractRenderComponent::draw (RenderTarget& target, RenderStates states) {
            reflectRefractTexSprite.setCenter(target.getView().getPosition());
            target.draw(reflectRefractTexSprite, states);
        }
        std::string ReflectRefractRenderComponent::getExpression() {
            return expression;
        }
        int ReflectRefractRenderComponent::getLayer() {
            return getPosition().z;
        }
        void ReflectRefractRenderComponent::pushEvent(window::IEvent event, RenderWindow& rw) {
            if (event.type == window::IEvent::WINDOW_EVENT && event.window.type == window::IEvent::WINDOW_EVENT_RESIZED && &getWindow() == &rw && isAutoResized()) {
                std::cout<<"recompute size"<<std::endl;
                recomputeSize();
                getListener().pushEvent(event);
                getView().reset(physic::BoundingBox(getView().getViewport().getPosition().x, getView().getViewport().getPosition().y, getView().getViewport().getPosition().z, event.window.data1, event.window.data2, getView().getViewport().getDepth()));
            }
        }
        void ReflectRefractRenderComponent::setView (View view) {
            depthBuffer.setView(view);
            alphaBuffer.setView(view);
            reflectRefractTex.setView(view);
            this->view = view;
        }
        View& ReflectRefractRenderComponent::getView() {
            return view;
        }
        ReflectRefractRenderComponent::~ReflectRefractRenderComponent() {
            glDeleteBuffers(1, &vboWorldMatrices);
            glDeleteBuffers(1, &atomicBuffer);
            glDeleteBuffers(1, &linkedListBuffer);
            glDeleteBuffers(1, &clearBuf);
            glDeleteTextures(1, &headPtrTex);
            glDeleteTextures(1, &depthTex);
            glDeleteTextures(1, &alphaTex);
            glDeleteBuffers(1, &clearBuf2);
            glDeleteBuffers(1, &clearBuf3);
            glDeleteBuffers(1, &ubo);
        }
        #endif // VULKAN
        #ifdef BIND
        ReflectRefractRenderComponent::ReflectRefractRenderComponent (RenderWindow& window, int layer, std::string expression, window::ContextSettings settings) :
            HeavyComponent(window, math::Vec3f(window.getView().getPosition().x, window.getView().getPosition().y, layer),
                          math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0),
                          math::Vec3f(window.getView().getSize().x + window.getView().getSize().x * 0.5f, window.getView().getPosition().y + window.getView().getSize().y * 0.5f, layer)),
            view(window.getView()),
            expression(expression)
            {
            if (window.getView().getSize().x > window.getView().getSize().y) {
                squareSize = window.getView().getSize().x;
            } else {
                squareSize = window.getView().getSize().y;
            }
            quad = RectangleShape(math::Vec3f(squareSize, squareSize, squareSize * 0.5f));
            quad.move(math::Vec3f(-squareSize * 0.5f, -squareSize * 0.5f, 0));
            dirs[0] = math::Vec3f(1, 0, 0);
            dirs[1] = math::Vec3f(-1, 0, 0);
            dirs[2] = math::Vec3f(0, 1, 0);
            dirs[3] = math::Vec3f(0, -1, 0);
            dirs[4] = math::Vec3f(0, 0, 1);
            dirs[5] = math::Vec3f(0, 0, -1);
            depthBuffer.create(window.getView().getSize().x, window.getView().getSize().y, settings);
            glCheck(glGenTextures(1, &depthTex));
            glCheck(glBindTexture(GL_TEXTURE_2D, depthTex));
            glCheck(glTexStorage2D(GL_TEXTURE_2D,1,GL_RGBA32F,window.getView().getSize().x, window.getView().getSize().y));
            glCheck(glBindImageTexture(0, depthTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F));
            glCheck(glBindTexture(GL_TEXTURE_2D, 0));
            std::vector<GLfloat> depthClearBuf(window.getView().getSize().x*window.getView().getSize().y*4, 0);
            glCheck(glGenBuffers(1, &clearBuf2));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf2));
            glCheck(glBufferData(GL_PIXEL_UNPACK_BUFFER, depthClearBuf.size() * sizeof(GLfloat),
            &depthClearBuf[0], GL_STATIC_COPY));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
            depthBufferSprite = Sprite(depthBuffer.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), sf::IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));
            alphaBuffer.create(window.getView().getSize().x, window.getView().getSize().y, settings);
            glCheck(glGenTextures(1, &alphaTex));
            glCheck(glBindTexture(GL_TEXTURE_2D, alphaTex));
            glCheck(glTexStorage2D(GL_TEXTURE_2D,1,GL_RGBA32F,window.getView().getSize().x, window.getView().getSize().y));
            glCheck(glBindImageTexture(0, alphaTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F));
            glCheck(glBindTexture(GL_TEXTURE_2D, 0));
            std::vector<GLfloat> alphaClearBuf(window.getView().getSize().x*window.getView().getSize().y*4, 0);
            glCheck(glGenBuffers(1, &clearBuf3));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf3));
            glCheck(glBufferData(GL_PIXEL_UNPACK_BUFFER, alphaClearBuf.size() * sizeof(GLfloat),
            &alphaClearBuf[0], GL_STATIC_COPY));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
            alphaBufferSprite = Sprite(alphaBuffer.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), sf::IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));
            sf::Vector3i resolution ((int) window.getSize().x, (int) window.getSize().y, window.getView().getSize().z);
            settings.depthBits = 0;
            reflectRefractTex.create(window.getView().getSize().x, window.getView().getSize().y, settings);
            reflectRefractTex.setEnableCubeMap(true);
            reflectRefractTexSprite = Sprite(reflectRefractTex.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), sf::IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));
            environmentMap.create(squareSize, squareSize, settings, GL_TEXTURE_CUBE_MAP);
            GLuint maxNodes = 20 * squareSize * squareSize;
            GLint nodeSize = 5 * sizeof(GLfloat) + sizeof(GLuint);
            glCheck(glGenBuffers(1, &atomicBuffer));
            glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicBuffer));
            glCheck(glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW));
            glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0));
            glCheck(glGenBuffers(1, &linkedListBuffer));
            glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, linkedListBuffer));
            glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, maxNodes * nodeSize, NULL, GL_DYNAMIC_DRAW));
            glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
            glCheck(glGenTextures(1, &headPtrTex));
            glCheck(glBindTexture(GL_TEXTURE_2D, headPtrTex));
            glCheck(glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, squareSize, squareSize));
            glCheck(glBindImageTexture(0, headPtrTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI));
            glCheck(glBindTexture(GL_TEXTURE_2D, 0));
            std::vector<GLuint> headPtrClearBuf(squareSize*squareSize, 0xffffffff);
            glCheck(glGenBuffers(1, &clearBuf));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf));
            glCheck(glBufferData(GL_PIXEL_UNPACK_BUFFER, headPtrClearBuf.size() * sizeof(GLuint),
            &headPtrClearBuf[0], GL_STATIC_COPY));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
            core::FastDelegate<bool> signal (&ReflectRefractRenderComponent::needToUpdate, this);
            core::FastDelegate<void> slot (&ReflectRefractRenderComponent::drawNextFrame, this);
            core::Command cmd(signal, slot);
            getListener().connect("UPDATE", cmd);
            if (settings.versionMajor >= 4 && settings.versionMinor >= 3) {
                glGenBuffers(1, &vboWorldMatrices);
                const std::string linkedListVertexShader = R"(#version 460
                                                    layout (location = 0) in vec3 position;
                                                    layout (location = 1) in vec4 color;
                                                    layout (location = 2) in vec2 texCoords;
                                                    layout (location = 3) in vec3 normals;
                                                    layout (location = 4) in mat4 worldMat;
                                                    uniform mat4 projectionMatrix;
                                                    uniform mat4 viewMatrix;
                                                    uniform mat4 textureMatrix;
                                                    out vec4 vColor;
                                                    out vec2 vTexCoord;
                                                    void main() {
                                                        gl_Position = projectionMatrix * viewMatrix * worldMat * vec4(position, 1.f);
                                                        vTexCoord = (textureMatrix * vec4(texCoords, 1.f, 1.f)).xy;
                                                        vColor = color;
                                                    }
                                                    )";
                const std::string  linkedListNormalVertexShader = R"(#version 460
                                                        layout (location = 0) in vec3 position;
                                                        layout (location = 1) in vec4 color;
                                                        layout (location = 2) in vec2 texCoords;
                                                        layout (location = 3) in vec3 normals;
                                                        uniform mat4 textureMatrix;
                                                        uniform mat4 projectionMatrix;
                                                        uniform mat4 viewMatrix;
                                                        out vec4 vColor;
                                                        out vec2 vTexCoord;
                                                        void main () {
                                                            gl_Position = projectionMatrix * viewMatrix * vec4(position, 1.f);
                                                            vTexCoord = (textureMatrix * vec4(texCoords, 1.f, 1.f)).xy;
                                                            vColor = color;
                                                        })";
                const std::string vertexShader = R"(#version 460
                                                    layout (location = 0) in vec3 position;
                                                    layout (location = 1) in vec4 color;
                                                    layout (location = 2) in vec2 texCoords;
                                                    layout (location = 3) in vec3 normals;
                                                    layout (location = 4) in mat4 worldMat;
                                                    uniform mat4 projectionMatrix;
                                                    uniform mat4 viewMatrix;
                                                    uniform mat4 textureMatrix;
                                                    out vec2 fTexCoords;
                                                    out vec4 frontColor;
                                                    void main() {
                                                        gl_Position = projectionMatrix * viewMatrix * worldMat * vec4(position, 1.f);
                                                        fTexCoords = (textureMatrix * vec4(texCoords, 1.f, 1.f)).xy;
                                                        frontColor = color;
                                                    }
                                                    )";
                const std::string  normalVertexShader = R"(#version 460
                                                        layout (location = 0) in vec3 position;
                                                        layout (location = 1) in vec4 color;
                                                        layout (location = 2) in vec2 texCoords;
                                                        layout (location = 3) in vec3 normals;
                                                        uniform mat4 textureMatrix;
                                                        uniform mat4 projectionMatrix;
                                                        uniform mat4 viewMatrix;
                                                        out vec2 fTexCoords;
                                                        out vec4 frontColor;
                                                        void main () {
                                                            gl_Position = projectionMatrix * viewMatrix * vec4(position, 1.f);
                                                            fTexCoords = (textureMatrix * vec4(texCoords, 1.f, 1.f)).xy;
                                                            frontColor = color;
                                                        })";
                const std::string  linkedListVertexShader2 = R"(#version 460
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
                const std::string geometryShader = R"(#version 460
                                                      #extension GL_EXT_geometry_shader4 : enable
                                                      layout (triangles) in;
                                                      layout (triangle_strip, max_vertices = 4) out;
                                                      uniform int layer;
                                                      in vec4 vColor[];
                                                      in vec2 vTexCoord[];
                                                      out vec4 frontColor;
                                                      out vec2 fTexCoords;
                                                      void main() {
                                                        /*gl_Layer = layer;*/
                                                        gl_Position = gl_in[0].gl_Position;
                                                        frontColor = vColor[0];
                                                        fTexCoords = vTexCoord[0];
                                                        EmitVertex();
                                                        gl_Position = gl_in[1].gl_Position;
                                                        frontColor = vColor[1];
                                                        fTexCoords = vTexCoord[1];
                                                        EmitVertex();
                                                        gl_Position = gl_in[2].gl_Position;
                                                        frontColor = vColor[2];
                                                        fTexCoords = vTexCoord[2];
                                                        EmitVertex();
                                                        EndPrimitive();
                                                      }
                                                      )";
                const std::string perPixReflectRefractVertexShader = R"(#version 460
                                                                   layout (location = 0) in vec3 position;
                                                                   layout (location = 1) in vec4 color;
                                                                   layout (location = 2) in vec2 texCoords;
                                                                   layout (location = 3) in vec3 normals;
                                                                   layout (location = 4) in mat4 worldMat;
                                                                   uniform mat4 projectionMatrix;
                                                                   uniform mat4 viewMatrix;
                                                                   out vec3 pos;
                                                                   out vec4 frontColor;
                                                                   out vec3 normal;
                                                                   void main() {
                                                                       pos = vec3(worldMat * vec4(position, 1.0));
                                                                       gl_Position = projectionMatrix * viewMatrix * worldMat * vec4(position, 1.f);
                                                                       frontColor = color;
                                                                       normal = mat3(transpose(inverse(worldMat))) * normals;
                                                                   }
                                                                  )";
                const std::string perPixReflectRefractVertexNormalShader = R"(#version 460
                                                                   layout (location = 0) in vec3 position;
                                                                   layout (location = 1) in vec4 color;
                                                                   layout (location = 2) in vec2 texCoords;
                                                                   layout (location = 3) in vec3 normals;
                                                                   uniform mat4 projectionMatrix;
                                                                   uniform mat4 viewMatrix;
                                                                   out vec4 frontColor;
                                                                   out vec2 fTexCoords;
                                                                   out vec3 pos;
                                                                   out vec3 normal;
                                                                   void main() {
                                                                       gl_Position = projectionMatrix * viewMatrix * vec4(position, 1.f);
                                                                       frontColor = color;
                                                                       pos = position;
                                                                       normal = normals;
                                                                   }
                                                                  )";
                const std::string buildDepthBufferFragmentShader = R"(#version 460
                                                                          in vec4 frontColor;
                                                                          in vec2 fTexCoords;
                                                                          uniform sampler2D texture;
                                                                          uniform float haveTexture;
                                                                          uniform float layer;
                                                                          uniform float nbLayers;
                                                                          layout(binding = 0, rgba32f) uniform image2D depthBuffer;
                                                                          layout (location = 0) out vec4 fColor;
                                                                          void main () {
                                                                              vec4 texel = texture2D(texture, fTexCoords);
                                                                              vec4 colors[2];
                                                                              colors[1] = texel * frontColor;
                                                                              colors[0] = frontColor;
                                                                              bool b = (haveTexture > 0.9);
                                                                              float current_alpha = colors[int(b)].a;
                                                                              float z = gl_FragCoord.z;
                                                                              float l = layer / nbLayers;
                                                                              vec4 depth = imageLoad(depthBuffer,ivec2(gl_FragCoord.xy));
                                                                              if (l > depth.y || l == depth.y && z > depth.z) {
                                                                                fColor = vec4(0, l, z, current_alpha);
                                                                                imageStore(depthBuffer,ivec2(gl_FragCoord.xy),vec4(0,l,z,current_alpha));
                                                                              } else {
                                                                                fColor = depth;
                                                                              }
                                                                          }
                                                                        )";
                const std::string buildAlphaBufferFragmentShader = R"(#version 460
                                                                      in vec4 frontColor;
                                                                      in vec2 fTexCoords;
                                                                      uniform sampler2D texture;
                                                                      uniform sampler2D depthBuffer;
                                                                      uniform float haveTexture;
                                                                      uniform float layer;
                                                                      uniform float nbLayers;
                                                                      uniform vec3 resolution;
                                                                      layout(binding = 0, rgba32f) uniform image2D alphaBuffer;
                                                                      layout (location = 0) out vec4 fColor;
                                                                      void main() {
                                                                          vec4 texel = texture2D(texture, fTexCoords);
                                                                          vec4 colors[2];
                                                                          colors[1] = texel * frontColor;
                                                                          colors[0] = frontColor;
                                                                          bool b = (haveTexture > 0.9);
                                                                          float current_alpha = colors[int(b)].a;
                                                                          vec2 position = (gl_FragCoord.xy / resolution.xy);
                                                                          vec4 depth = texture2D (depthBuffer, position);
                                                                          vec4 alpha = imageLoad(alphaBuffer,ivec2(gl_FragCoord.xy));
                                                                          float l = layer / nbLayers;
                                                                          float z = gl_FragCoord.z;
                                                                          if ((l > depth.y || l == depth.y && z > depth.z) && current_alpha > alpha.a) {
                                                                              fColor = vec4(0, l, z, current_alpha);
                                                                              imageStore(alphaBuffer,ivec2(gl_FragCoord.xy),vec4(0, l, z, current_alpha));
                                                                          } else {
                                                                              fColor = alpha;
                                                                          }
                                                                      }
                                                                      )";
                const std::string buildFramebufferShader = R"(#version 460
                                                                in vec4 frontColor;
                                                                in vec3 pos;
                                                                in vec3 normal;
                                                                uniform vec3 cameraPos;
                                                                uniform samplerCube sceneBox;
                                                                uniform sampler2D alphaBuffer;
                                                                uniform vec3 resolution;
                                                                layout (location = 0) out vec4 fColor;
                                                                void main () {
                                                                    vec2 position = (gl_FragCoord.xy / resolution.xy);
                                                                    vec4 alpha = texture2D(alphaBuffer, position);
                                                                    float ratio = 1.00 / 1.33;
                                                                    vec3 i = (pos - cameraPos);
                                                                    vec3 r = refract (i, normalize(normal), ratio);
                                                                    fColor = texture(sceneBox, r) * (1 - alpha.a);
                                                                }
                                                              )";
                const std::string fragmentShader = R"(#version 460
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
                                                      in vec4 frontColor;
                                                      in vec2 fTexCoords;
                                                      layout (location = 0) out vec4 fcolor;
                                                      void main() {
                                                           uint nodeIdx = atomicCounterIncrement(nextNodeCounter);
                                                           vec4 texel = texture2D(texture, fTexCoords.xy);
                                                           vec4 color = (haveTexture > 0.9) ? frontColor * texel : frontColor;
                                                           if (nodeIdx < maxNodes) {
                                                                uint prevHead = imageAtomicExchange(headPointers, ivec2(gl_FragCoord.xy), nodeIdx);
                                                                nodes[nodeIdx].color = color;
                                                                nodes[nodeIdx].depth = gl_FragCoord.z;
                                                                nodes[nodeIdx].next = prevHead;
                                                           }
                                                           fcolor = vec4(0, 0, 0, 0);
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
                      }
                      fcolor = color;
                   })";
                if (!sBuildDepthBuffer.loadFromMemory(vertexShader, buildDepthBufferFragmentShader)) {
                    throw core::Erreur(50, "Error, failed to load build depth buffer shader", 3);
                }
                if (!sBuildDepthBufferNormal.loadFromMemory(normalVertexShader, buildDepthBufferFragmentShader)) {
                    throw core::Erreur(51, "Error, failed to load build normal depth buffer shader", 3);
                }
                if (!sReflectRefract.loadFromMemory(perPixReflectRefractVertexShader, buildFramebufferShader)) {
                    throw core::Erreur(57, "Error, failed to load reflect refract shader", 3);
                }
                if (!sReflectRefractNormal.loadFromMemory(perPixReflectRefractVertexNormalShader, buildFramebufferShader)) {
                    throw core::Erreur(58, "Error, failed to load reflect refract normal shader", 3);
                }
                if (!sLinkedList.loadFromMemory(vertexShader, fragmentShader/*, geometryShader*/)) {
                    throw core::Erreur(58, "Error, failed to load per pixel linked list shader", 3);
                }
                if (!sLinkedListNormal.loadFromMemory(normalVertexShader, fragmentShader/*, geometryShader*/)) {
                    throw core::Erreur(58, "Error, failed to load per pixel linked list normal shader", 3);
                }
                if (!sLinkedList2.loadFromMemory(linkedListVertexShader2, fragmentShader2)) {
                    throw core::Erreur(59, "Error, failed to load per pixel linked list 2 shader", 3);
                }
                if (!sBuildAlphaBuffer.loadFromMemory(vertexShader,buildAlphaBufferFragmentShader)) {
                    throw core::Erreur(60, "Error, failed to load build alpha buffer shader", 3);
                }
                if (!sBuildAlphaBufferNormal.loadFromMemory(normalVertexShader,buildAlphaBufferFragmentShader)) {
                    throw core::Erreur(60, "Error, failed to load build alpha buffer shader", 3);
                }
                sBuildDepthBuffer.setParameter("texture", Shader::CurrentTexture);
                sBuildDepthBufferNormal.setParameter("texture", Shader::CurrentTexture);
                sBuildAlphaBuffer.setParameter("texture", Shader::CurrentTexture);
                sBuildAlphaBuffer.setParameter("depthBuffer", depthBuffer.getTexture());
                sBuildAlphaBuffer.setParameter("resolution", resolution.x, resolution.y, resolution.z);
                sBuildAlphaBufferNormal.setParameter("texture", Shader::CurrentTexture);
                sBuildAlphaBufferNormal.setParameter("depthBuffer", depthBuffer.getTexture());
                sBuildAlphaBufferNormal.setParameter("resolution", resolution.x, resolution.y, resolution.z);
                sReflectRefract.setParameter("resolution", resolution.x, resolution.y, resolution.z);
                sReflectRefract.setParameter("alphaBuffer", alphaBuffer.getTexture());
                sReflectRefract.setParameter("sceneBox", Shader::CurrentTexture);
                sReflectRefractNormal.setParameter("resolution", resolution.x, resolution.y, resolution.z);
                sReflectRefractNormal.setParameter("sceneBox", Shader::CurrentTexture);
                sReflectRefractNormal.setParameter("alphaBuffer", alphaBuffer.getTexture());
                sLinkedList.setParameter("maxNodes", maxNodes);
                sLinkedList.setParameter("texture", Shader::CurrentTexture);
                sLinkedListNormal.setParameter("maxNodes", maxNodes);
                sLinkedListNormal.setParameter("texture", Shader::CurrentTexture);
                View defaultView = window.getDefaultView();
                defaultView.setPerspective(-squareSize * 0.5f, squareSize * 0.5f, -squareSize * 0.5f, squareSize * 0.5f, 0, 1000);
                math::Matrix4f viewMatrix = defaultView.getViewMatrix().getMatrix().transpose();
                math::Matrix4f projMatrix = defaultView.getProjMatrix().getMatrix().transpose();
                sLinkedList2.setParameter("viewMatrix", viewMatrix);
                sLinkedList2.setParameter("projectionMatrix", projMatrix);
            }
            backgroundColor = sf::Color::Transparent;
            /*glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicBuffer));
            glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, linkedListBuffer));*/
        }
        void ReflectRefractRenderComponent::onVisibilityChanged(bool visible) {
        }
        void ReflectRefractRenderComponent::drawNextFrame() {
            if (reflectRefractTex.getSettings().versionMajor >= 4 && reflectRefractTex.getSettings().versionMinor >= 3) {
                /*glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicBuffer));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, linkedListBuffer));*/
                RenderStates currentStates;
                math::Matrix4f viewMatrix = view.getViewMatrix().getMatrix().transpose();
                math::Matrix4f projMatrix = view.getProjMatrix().getMatrix().transpose();
                sBuildDepthBuffer.setParameter("viewMatrix", viewMatrix);
                sBuildDepthBuffer.setParameter("projectionMatrix", projMatrix);
                sBuildDepthBufferNormal.setParameter("viewMatrix", viewMatrix);
                sBuildDepthBufferNormal.setParameter("projectionMatrix", projMatrix);
                for (unsigned int i = 0; i < m_reflInstances.size(); i++) {
                    if (m_reflInstances[i].getAllVertices().getVertexCount() > 0) {
                        if (m_reflInstances[i].getMaterial().getTexture() != nullptr) {
                            math::Matrix4f texMatrix = m_reflInstances[i].getMaterial().getTexture()->getTextureMatrix();
                            sBuildDepthBuffer.setParameter("textureMatrix", texMatrix);
                            sBuildDepthBuffer.setParameter("haveTexture", 1.f);
                        } else {
                            sBuildDepthBuffer.setParameter("haveTexture", 0.f);
                        }
                        float layer = m_reflInstances[i].getVertexArrays()[0]->getEntity()->getLayer();
                        sBuildDepthBuffer.setParameter("layer", layer);
                        sBuildDepthBuffer.setParameter("nbLayers",Entity::getNbLayers());
                        vb.clear();
                        vb.setPrimitiveType(m_reflInstances[i].getVertexArrays()[0]->getPrimitiveType());
                        matrices.clear();
                        std::vector<TransformMatrix*> tm = m_reflInstances[i].getTransforms();
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
                        if (m_reflInstances[i].getVertexArrays().size() > 0) {
                            Entity* entity = m_reflInstances[i].getVertexArrays()[0]->getEntity();
                            for (unsigned int j = 0; j < m_reflInstances[i].getVertexArrays().size(); j++) {
                                if (entity == m_reflInstances[i].getVertexArrays()[j]->getEntity()) {
                                    for (unsigned int k = 0; k < m_reflInstances[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                        vb.append((*m_reflInstances[i].getVertexArrays()[j])[k]);
                                    }
                                }
                            }
                            vb.update();
                        }
                        currentStates.blendMode = sf::BlendNone;
                        currentStates.shader = &sBuildDepthBuffer;
                        currentStates.texture = m_reflInstances[i].getMaterial().getTexture();
                        depthBuffer.drawInstanced(vb, m_reflInstances[i].getVertexArrays()[0]->getPrimitiveType(), 0, m_reflInstances[i].getVertexArrays()[0]->getVertexCount(), tm.size(), currentStates, vboWorldMatrices);
                    }
                }
                for (unsigned int i = 0; i < m_reflNormals.size(); i++) {
                    if (m_reflNormals[i].getAllVertices().getVertexCount() > 0) {
                        if (m_reflNormals[i].getMaterial().getTexture() != nullptr) {
                            math::Matrix4f texMatrix = m_reflNormals[i].getMaterial().getTexture()->getTextureMatrix();
                            sBuildDepthBufferNormal.setParameter("textureMatrix", texMatrix);
                            sBuildDepthBufferNormal.setParameter("haveTexture", 1.f);
                        } else {
                            sBuildDepthBufferNormal.setParameter("haveTexture", 0.f);
                        }
                        float layer = m_reflNormals[i].getVertexArrays()[0]->getEntity()->getLayer();
                        sBuildDepthBuffer.setParameter("layer", layer);
                        sBuildDepthBuffer.setParameter("nbLayers",Entity::getNbLayers());
                        vb.clear();
                        vb.setPrimitiveType(m_reflNormals[i].getAllVertices().getPrimitiveType());
                        for (unsigned int j = 0; j < m_reflNormals[i].getAllVertices().getVertexCount(); j++) {
                            vb.append(m_reflNormals[i].getAllVertices()[j]);
                        }
                        vb.update();
                        currentStates.blendMode = sf::BlendNone;
                        currentStates.shader = &sBuildDepthBufferNormal;
                        currentStates.texture = m_reflNormals[i].getMaterial().getTexture();
                        depthBuffer.drawVertexBuffer(vb, currentStates);
                    }
                }
                depthBuffer.display();
                sBuildAlphaBuffer.setParameter("viewMatrix", viewMatrix);
                sBuildAlphaBuffer.setParameter("projectionMatrix", projMatrix);
                sBuildAlphaBufferNormal.setParameter("viewMatrix", viewMatrix);
                sBuildAlphaBufferNormal.setParameter("projectionMatrix", projMatrix);
                for (unsigned int i = 0; i < m_instances.size(); i++) {
                    if (m_instances[i].getAllVertices().getVertexCount() > 0) {
                        if (m_instances[i].getMaterial().getTexture() != nullptr) {
                            math::Matrix4f texMatrix = m_instances[i].getMaterial().getTexture()->getTextureMatrix();
                            sBuildAlphaBuffer.setParameter("textureMatrix", texMatrix);
                            sBuildAlphaBuffer.setParameter("haveTexture", 1.f);
                        } else {
                            sBuildAlphaBuffer.setParameter("haveTexture", 0.f);
                        }
                        float layer = m_instances[i].getVertexArrays()[0]->getEntity()->getLayer();
                        sBuildAlphaBuffer.setParameter("layer", layer);
                        sBuildAlphaBuffer.setParameter("nbLayers",Entity::getNbLayers());
                        vb.clear();
                        vb.setPrimitiveType(m_instances[i].getVertexArrays()[0]->getPrimitiveType());
                        matrices.clear();
                        std::vector<TransformMatrix*> tm = m_instances[i].getTransforms();
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
                                    for (unsigned int k = 0; k < m_instances[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                        vb.append((*m_instances[i].getVertexArrays()[j])[k]);
                                    }
                                }
                            }
                            vb.update();
                        }
                        currentStates.blendMode = sf::BlendNone;
                        currentStates.shader = &sBuildAlphaBuffer;
                        currentStates.texture = m_instances[i].getMaterial().getTexture();
                        alphaBuffer.drawInstanced(vb, m_instances[i].getVertexArrays()[0]->getPrimitiveType(), 0, m_instances[i].getVertexArrays()[0]->getVertexCount(), tm.size(), currentStates, vboWorldMatrices);
                    }
                }
                for (unsigned int i = 0; i < m_normals.size(); i++) {
                    if (m_normals[i].getAllVertices().getVertexCount() > 0) {
                        if (m_normals[i].getMaterial().getTexture() != nullptr) {
                            math::Matrix4f texMatrix = m_normals[i].getMaterial().getTexture()->getTextureMatrix();
                            sBuildAlphaBufferNormal.setParameter("textureMatrix", texMatrix);
                            sBuildAlphaBufferNormal.setParameter("haveTexture", 1.f);
                        } else {
                            sBuildAlphaBufferNormal.setParameter("haveTexture", 0.f);
                        }
                        float layer = m_normals[i].getVertexArrays()[0]->getEntity()->getLayer();
                        sBuildAlphaBufferNormal.setParameter("layer", layer);
                        sBuildAlphaBufferNormal.setParameter("nbLayers",Entity::getNbLayers());
                        vb.clear();
                        vb.setPrimitiveType(m_normals[i].getAllVertices().getPrimitiveType());
                        for (unsigned int j = 0; j < m_normals[i].getAllVertices().getVertexCount(); j++) {
                            vb.append(m_normals[i].getAllVertices()[j]);
                        }
                        vb.update();
                        currentStates.blendMode = sf::BlendNone;
                        currentStates.shader = &sBuildAlphaBufferNormal;
                        currentStates.texture = m_normals[i].getMaterial().getTexture();
                        alphaBuffer.drawVertexBuffer(vb, currentStates);
                    }
                }
                alphaBuffer.display();
                View reflectView;
                if (view.isOrtho()) {
                    reflectView = View (squareSize, squareSize, view.getViewport().getPosition().z, view.getViewport().getSize().z);
                } else {
                    reflectView = View (squareSize, squareSize, 90, view.getViewport().getPosition().z, view.getViewport().getSize().z);
                }
                for (unsigned int t = 0; t < 2; t++) {
                    std::vector<Instance> instances = (t == 0) ? m_reflInstances : m_reflNormals;
                    for (unsigned int n = 0; n < instances.size(); n++) {
                        if (instances[n].getAllVertices().getVertexCount() > 0) {
                            Entity* entity = instances[n].getVertexArrays()[0]->getEntity()->getRootEntity();
                            math::Vec3f scale(1, 1, 1);
                            if (entity->getSize().x > squareSize) {
                                scale.x = entity->getSize().x / squareSize;
                            }
                            if (entity->getSize().y > squareSize) {
                                scale.y = entity->getSize().y / squareSize;
                            }
                            reflectView.setScale(scale.x, scale.y, scale.z);
                            reflectView.setCenter(entity->getPosition()+entity->getSize()*0.5f);
                            for (unsigned int m = 0; m < 6; m++) {
                                math::Vec3f target = reflectView.getPosition() + dirs[m];
                                reflectView.lookAt(target.x, target.y, target.z);
                                /*std::vector<Entity*> visibleReflEntities = World::getEntitiesInRect(reflectView.getViewVolume(), expression);
                                std::vector<Entity*> vEntities;
                                for (unsigned int j = 0; j < visibleEntities.size(); j++)  {
                                    if (!visibleEntities[j]->isReflectable()) {
                                        vEntities.push_back(visibleEntities[j]);
                                    }
                                }
                                rvBatcher.clear();
                                normalRvBatcher.clear();
                                for (unsigned int j = 0; j < vEntities.size(); j++) {
                                    for (unsigned int n = 0; n < vEntities[j]->getNbFaces(); n++) {
                                        if (vEntities[j]->getDrawMode() == Entity::INSTANCED) {
                                            rvBatcher.addFace(vEntities[j]->getFace(n));
                                        } else {
                                            normalRvBatcher.addFace(vEntities[j]->getFace(n));
                                        }
                                    }
                                }
                                std::vector<Instance> rvInstances = rvBatcher.getInstances();
                                std::vector<Instance> rvNormals = normalRvBatcher.getInstances();*/
                                environmentMap.setView(reflectView);
                                environmentMap.setActive();
                                GLuint zero = 0;
                                glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicBuffer));
                                glCheck(glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero));
                                glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0));
                                glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf));
                                glCheck(glBindTexture(GL_TEXTURE_2D, headPtrTex));
                                glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, squareSize, squareSize, GL_RED_INTEGER,
                                GL_UNSIGNED_INT, NULL));
                                glCheck(glBindTexture(GL_TEXTURE_2D, 0));
                                glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
                                environmentMap.resetGLStates();
                                environmentMap.selectCubemapFace(m);
                                viewMatrix = reflectView.getViewMatrix().getMatrix().transpose();
                                projMatrix = reflectView.getProjMatrix().getMatrix().transpose();
                                sLinkedList.setParameter("viewMatrix", viewMatrix);
                                sLinkedList.setParameter("projectionMatrix", projMatrix);
                                for (unsigned int i = 0; i < m_instances.size(); i++) {
                                    if (m_instances[i].getAllVertices().getVertexCount() > 0) {
                                        matrices.clear();
                                        std::vector<TransformMatrix*> tm = m_instances[i].getTransforms();
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
                                        vb.clear();
                                        vb.setPrimitiveType(m_instances[i].getVertexArrays()[0]->getPrimitiveType());
                                        if (m_instances[i].getVertexArrays().size() > 0) {
                                            Entity* entity = m_instances[i].getVertexArrays()[0]->getEntity();
                                            for (unsigned int j = 0; j < m_instances[i].getVertexArrays().size(); j++) {
                                                if (entity == m_instances[i].getVertexArrays()[j]->getEntity()) {
                                                    for (unsigned int k = 0; k < m_instances[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                                        vb.append((*m_instances[i].getVertexArrays()[j])[k]);
                                                    }
                                                }
                                            }
                                            vb.update();
                                        }
                                        currentStates.blendMode = sf::BlendNone;
                                        currentStates.shader = &sLinkedList;
                                        currentStates.texture =m_instances[i].getMaterial().getTexture();
                                        if (m_instances[i].getMaterial().getTexture() != nullptr) {
                                            math::Matrix4f texMatrix = m_instances[i].getMaterial().getTexture()->getTextureMatrix();
                                            sLinkedList.setParameter("textureMatrix", texMatrix);
                                            sLinkedList.setParameter("haveTexture", 1.f);
                                        } else {
                                            sLinkedList.setParameter("haveTexture", 0.f);
                                        }
                                        environmentMap.drawInstanced(vb, m_instances[i].getVertexArrays()[0]->getPrimitiveType(), 0, m_instances[i].getVertexArrays()[0]->getVertexCount(), tm.size(), currentStates, vboWorldMatrices);
                                    }
                                }
                                viewMatrix = reflectView.getViewMatrix().getMatrix().transpose();
                                projMatrix = reflectView.getProjMatrix().getMatrix().transpose();
                                sLinkedListNormal.setParameter("viewMatrix", viewMatrix);
                                sLinkedListNormal.setParameter("projectionMatrix", projMatrix);
                                for (unsigned int i = 0; i < m_normals.size(); i++) {
                                   if (m_normals[i].getAllVertices().getVertexCount() > 0) {
                                        if (m_normals[i].getMaterial().getTexture() == nullptr) {
                                            sLinkedListNormal.setParameter("haveTexture", 0.f);
                                        } else {
                                            math::Matrix4f texMatrix = m_normals[i].getMaterial().getTexture()->getTextureMatrix();
                                            sLinkedListNormal.setParameter("textureMatrix", texMatrix);
                                            sLinkedListNormal.setParameter("haveTexture", 1.f);
                                        }
                                        currentStates.blendMode = sf::BlendNone;
                                        currentStates.shader = &sLinkedListNormal;
                                        currentStates.texture = m_normals[i].getMaterial().getTexture();
                                        vb.clear();
                                        vb.setPrimitiveType(m_normals[i].getAllVertices().getPrimitiveType());
                                        for (unsigned int j = 0; j < m_normals[i].getAllVertices().getVertexCount(); j++) {
                                            vb.append(m_normals[i].getAllVertices()[j]);
                                        }
                                        vb.update();
                                        environmentMap.drawVertexBuffer(vb, currentStates);
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
                                sLinkedList2.setParameter("worldMat", matrix);
                                currentStates.shader = &sLinkedList2;
                                currentStates.texture = nullptr;
                                environmentMap.drawVertexBuffer(vb, currentStates);
                                glCheck(glFinish());
                                glCheck(glMemoryBarrier(GL_ALL_BARRIER_BITS));
                            }
                            environmentMap.display();
                            viewMatrix = view.getViewMatrix().getMatrix().transpose();
                            projMatrix = view.getProjMatrix().getMatrix().transpose();
                            sReflectRefract.setParameter("viewMatrix", viewMatrix);
                            sReflectRefract.setParameter("projectionMatrix", projMatrix);
                            sReflectRefract.setParameter("cameraPos", view.getPosition().x, view.getPosition().y, view.getPosition().z);
                            for (unsigned int i = 0; i < m_reflInstances.size(); i++) {
                                if (m_reflInstances[i].getAllVertices().getVertexCount() > 0) {
                                    vb.clear();
                                    vb.setPrimitiveType(m_reflInstances[i].getVertexArrays()[0]->getPrimitiveType());
                                    matrices.clear();
                                    std::vector<TransformMatrix*> tm = m_reflInstances[i].getTransforms();
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
                                    if (m_reflInstances[i].getVertexArrays().size() > 0) {
                                        Entity* entity = m_reflInstances[i].getVertexArrays()[0]->getEntity();
                                        for (unsigned int j = 0; j < m_reflInstances[i].getVertexArrays().size(); j++) {
                                            if (entity == m_reflInstances[i].getVertexArrays()[j]->getEntity()) {
                                                for (unsigned int k = 0; k < m_reflInstances[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                                    vb.append((*m_reflInstances[i].getVertexArrays()[j])[k]);
                                                }
                                            }
                                        }
                                        vb.update();
                                    }
                                    currentStates.blendMode = sf::BlendNone;
                                    currentStates.shader = &sReflectRefract;
                                    currentStates.texture = &environmentMap.getTexture();
                                    reflectRefractTex.drawInstanced(vb, m_reflInstances[i].getVertexArrays()[0]->getPrimitiveType(), 0, m_reflInstances[i].getVertexArrays()[0]->getVertexCount(), tm.size(), currentStates, vboWorldMatrices);
                                }
                            }
                            viewMatrix = view.getViewMatrix().getMatrix().transpose();
                            projMatrix = view.getProjMatrix().getMatrix().transpose();
                            sReflectRefractNormal.setParameter("viewMatrix", viewMatrix);
                            sReflectRefractNormal.setParameter("projectionMatrix", projMatrix);
                            sReflectRefractNormal.setParameter("cameraPos", view.getPosition().x, view.getPosition().y, view.getPosition().z);
                            for (unsigned int i = 0; i < m_reflNormals.size(); i++) {
                                if (m_reflNormals[i].getAllVertices().getVertexCount() > 0) {
                                    vb.clear();
                                    vb.setPrimitiveType(m_reflNormals[i].getAllVertices().getPrimitiveType());
                                    for (unsigned int j = 0; j < m_reflNormals[i].getAllVertices().getVertexCount(); j++) {
                                        vb.append(m_reflNormals[i].getAllVertices()[j]);
                                    }
                                    vb.update();
                                    currentStates.blendMode = sf::BlendNone;
                                    currentStates.shader = &sReflectRefractNormal;
                                    currentStates.texture = &environmentMap.getTexture();
                                    reflectRefractTex.drawVertexBuffer(vb, currentStates);
                                }
                            }
                        }
                    }
                }
                reflectRefractTex.display();
                /*glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, 0));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0));*/
            }
        }
        std::vector<Entity*> ReflectRefractRenderComponent::getEntities() {
            return visibleEntities;
        }
        bool ReflectRefractRenderComponent::loadEntitiesOnComponent(std::vector<Entity*> vEntities) {
            batcher.clear();
            normalBatcher.clear();
            reflBatcher.clear();
            reflNormalBatcher.clear();
            for (unsigned int i = 0; i < vEntities.size(); i++) {
                if ( vEntities[i]->isLeaf()) {
                    for (unsigned int j = 0; j <  vEntities[i]->getNbFaces(); j++) {
                        if (vEntities[i]->isReflectable()) {
                            if (vEntities[i]->getDrawMode() == Entity::INSTANCED) {
                                reflBatcher.addFace( vEntities[i]->getFace(j));
                            } else {
                                reflNormalBatcher.addFace(vEntities[i]->getFace(j));
                            }
                        } else {
                            if (vEntities[i]->getDrawMode() == Entity::INSTANCED) {
                                batcher.addFace( vEntities[i]->getFace(j));
                            } else {
                                normalBatcher.addFace(vEntities[i]->getFace(j));
                            }
                        }
                    }
                }
            }
            m_instances = batcher.getInstances();
            m_normals = normalBatcher.getInstances();
            m_reflInstances = reflBatcher.getInstances();
            m_reflNormals = reflNormalBatcher.getInstances();
            visibleEntities = vEntities;
            update = true;
            return true;
        }
        bool ReflectRefractRenderComponent::needToUpdate() {
            return update;
        }
        void ReflectRefractRenderComponent::clear() {
            depthBuffer.clear(sf::Color::Transparent);
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf2));
            glCheck(glBindTexture(GL_TEXTURE_2D, depthTex));
            glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, view.getSize().x, view.getSize().y, GL_RGBA,
            GL_FLOAT, NULL));
            glCheck(glBindTexture(GL_TEXTURE_2D, 0));
            alphaBuffer.clear(sf::Color::Transparent);
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf3));
            glCheck(glBindTexture(GL_TEXTURE_2D, alphaTex));
            glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, view.getSize().x, view.getSize().y, GL_RGBA,
            GL_FLOAT, NULL));
            glCheck(glBindTexture(GL_TEXTURE_2D, 0));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
            environmentMap.clear(sf::Color::Transparent);
            reflectRefractTex.clear(sf::Color::Transparent);
        }
        void ReflectRefractRenderComponent::setBackgroundColor (sf::Color color) {
            this->backgroundColor = color;
        }
        void ReflectRefractRenderComponent::setExpression (std::string expression) {
            this->expression = expression;
        }
        void ReflectRefractRenderComponent::draw (Drawable& drawable, RenderStates states) {

        }
        void ReflectRefractRenderComponent::draw (RenderTarget& target, RenderStates states) {
            reflectRefractTexSprite.setCenter(target.getView().getPosition());
            target.draw(reflectRefractTexSprite, states);
        }
        std::string ReflectRefractRenderComponent::getExpression() {
            return expression;
        }
        int ReflectRefractRenderComponent::getLayer() {
            return getPosition().z;
        }
        void ReflectRefractRenderComponent::changeVisibleEntities(Entity* toRemove, Entity* toAdd, EntityManager* em) {
        }
        void ReflectRefractRenderComponent::pushEvent(window::IEvent event, RenderWindow& rw) {
            if (event.type == window::IEvent::WINDOW_EVENT && event.window.type == window::IEvent::WINDOW_EVENT_RESIZED && &getWindow() == &rw && isAutoResized()) {
                std::cout<<"recompute size"<<std::endl;
                recomputeSize();
                getListener().pushEvent(event);
                getView().reset(physic::BoundingBox(getView().getViewport().getPosition().x, getView().getViewport().getPosition().y, getView().getViewport().getPosition().z, event.window.data1, event.window.data2, getView().getViewport().getDepth()));
            }
        }
        void ReflectRefractRenderComponent::updateParticleSystems() {

        }
        void ReflectRefractRenderComponent::setView (View view) {
            depthBuffer.setView(view);
            reflectRefractTex.setView(view);
            this->view = view;
        }
        View& ReflectRefractRenderComponent::getView() {
            return view;
        }
        ReflectRefractRenderComponent::~ReflectRefractRenderComponent() {
            glDeleteBuffers(1, &vboWorldMatrices);
            glDeleteBuffers(1, &atomicBuffer);
            glDeleteBuffers(1, &linkedListBuffer);
            glDeleteBuffers(1, &clearBuf);
            glDeleteTextures(1, &headPtrTex);
        }
        #endif // VULKAN

    }
}
