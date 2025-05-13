#include "../../../../include/odfaeg/Graphics/ECS/shadowRenderComponent.hpp"
#ifndef VULKAN
#include <SFML/OpenGL.hpp>
#include "../glCheck.h"
#endif
#include <memory.h>
using namespace sf;
using namespace std;
namespace odfaeg {
    namespace graphic {
        namespace ecs {
            #ifdef VULKAN
            #else
            ShadowRenderComponent::ShadowRenderComponent (RenderWindow& window, int layer, std::string expression,ComponentMapping& componentMapping, window::ContextSettings settings) :
            HeavyComponent(window, math::Vec3f(window.getView().getPosition().x, window.getView().getPosition().y, layer),
                          math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0),
                          math::Vec3f(window.getView().getSize().x + window.getView().getSize().x * 0.5f, window.getView().getPosition().y + window.getView().getSize().y * 0.5f, layer)),
            view(window.getView()),
            expression(expression),
            componentMapping(componentMapping) {
            update = false;
            sf::Vector3i resolution ((int) window.getSize().x, (int) window.getSize().y, window.getView().getSize().z);
            shadowMap.create(resolution.x, resolution.y,settings);

            depthBuffer.create(resolution.x, resolution.y, settings);

            shadowTile = Sprite(shadowMap.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));
            depthBufferTile = Sprite(depthBuffer.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));

            glCheck(glGenTextures(1, &depthTex));
            glCheck(glBindTexture(GL_TEXTURE_2D, depthTex));
            glCheck(glTexStorage2D(GL_TEXTURE_2D,1,GL_RGBA32F,window.getView().getSize().x, window.getView().getSize().y));
            glCheck(glBindImageTexture(0, depthTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F));
            glCheck(glBindTexture(GL_TEXTURE_2D, 0));
            std::vector<GLfloat> depthClearBuf(window.getView().getSize().x*window.getView().getSize().y*4, 0);
            glCheck(glGenBuffers(1, &clearBuf));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf));
            glCheck(glBufferData(GL_PIXEL_UNPACK_BUFFER, depthClearBuf.size() * sizeof(GLfloat),
            &depthClearBuf[0], GL_STATIC_COPY));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));

            stencilBuffer.create(resolution.x, resolution.y,settings);
            stencilBufferTile = Sprite(stencilBuffer.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));
            glCheck(glGenTextures(1, &stencilTex));
            glCheck(glBindTexture(GL_TEXTURE_2D, stencilTex));
            glCheck(glTexStorage2D(GL_TEXTURE_2D,1,GL_RGBA32F,window.getView().getSize().x, window.getView().getSize().y));
            glCheck(glBindImageTexture(0, stencilTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F));
            glCheck(glBindTexture(GL_TEXTURE_2D, 0));
            std::vector<GLfloat> stencilClearBuf(window.getView().getSize().x*window.getView().getSize().y*4, 0);
            glCheck(glGenBuffers(1, &clearBuf2));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf2));
            glCheck(glBufferData(GL_PIXEL_UNPACK_BUFFER, stencilClearBuf.size() * sizeof(GLfloat),
            &stencilClearBuf[0], GL_STATIC_COPY));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));

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
                core::FastDelegate<bool> signal (&ShadowRenderComponent::needToUpdate, this);
                core::FastDelegate<void> slot (&ShadowRenderComponent::drawNextFrame, this);
                core::Command cmd(signal, slot);
                getListener().connect("UPDATE", cmd);
                glCheck(glGenBuffers(1, &vboWorldMatrices));
                glCheck(glGenBuffers(1, &vboIndirect));
                if (settings.versionMajor >= 3 && settings.versionMinor >= 3) {
                    glGenBuffers(1, &vboWorldMatrices);
                    glGenBuffers(1, &vboShadowProjMatrices);
                    const std::string buildDepthBufferVertexShaderNormal = R"(#version 460
                                                                              layout (location = 0) in vec3 position;
                                                                              layout (location = 1) in vec4 color;
                                                                              layout (location = 2) in vec2 texCoords;
                                                                              layout (location = 3) in vec3 normals;
                                                                              layout (location = 4) in uint textureIndex;
                                                                              layout (location = 6) in uint l;
                                                                              uniform mat4 projectionMatrix;
                                                                              uniform mat4 viewMatrix;
                                                                              uniform mat4 textureMatrix[)"+core::conversionUIntString(Texture::getAllTextures().size())+R"(];
                                                                              out vec2 fTexCoords;
                                                                              out vec4 frontColor;
                                                                              out uint texIndex;
                                                                              out uint layer;
                                                                              void main() {
                                                                                  gl_Position = projectionMatrix * viewMatrix * vec4(position, 1.f);
                                                                                  fTexCoords = (textureIndex != 0) ? (textureMatrix[textureIndex-1] * vec4(texCoords, 1.f, 1.f)).xy : texCoords;
                                                                                  frontColor = color;
                                                                                  texIndex = textureIndex;
                                                                                  layer = l;
                                                                              }
                                                                              )";
                    const std::string buildDepthBufferVertexShader = R"(#version 460
                                                                        layout (location = 0) in vec3 position;
                                                                        layout (location = 1) in vec4 color;
                                                                        layout (location = 2) in vec2 texCoords;
                                                                        layout (location = 3) in vec3 normals;
                                                                        layout (location = 4) in mat4 worldMat;
                                                                        layout (location = 12) in uint textureIndex;
                                                                        layout (location = 14) in uint l;
                                                                        uniform mat4 projectionMatrix;
                                                                        uniform mat4 viewMatrix;
                                                                        uniform mat4 textureMatrix[)"+core::conversionUIntString(Texture::getAllTextures().size())+R"(];
                                                                        out vec2 fTexCoords;
                                                                        out vec4 frontColor;
                                                                        out uint texIndex;
                                                                        out uint layer;
                                                                        void main() {
                                                                            gl_Position = projectionMatrix * viewMatrix * worldMat * vec4(position, 1.f);
                                                                            fTexCoords = (textureIndex != 0) ? (textureMatrix[textureIndex-1] * vec4(texCoords, 1.f, 1.f)).xy : texCoords;
                                                                            frontColor = color;
                                                                            texIndex = textureIndex;
                                                                            layer = l;
                                                                        }
                                                                     )";
                     const std::string buildDepthBufferFragmentShader = R"(#version 460
                                                                          #extension GL_ARB_bindless_texture : enable
                                                                          in vec4 frontColor;
                                                                          in vec2 fTexCoords;
                                                                          in flat uint texIndex;
                                                                          in flat uint layer;
                                                                          layout(std140, binding=0) uniform ALL_TEXTURES {
                                                                              sampler2D textures[200];
                                                                          };

                                                                          layout(binding = 0, rgba32f) uniform image2D depthBuffer;
                                                                          layout (location = 0) out vec4 fColor;

                                                                          void main () {
                                                                              vec4 texel = (texIndex != 0) ? frontColor * texture2D(textures[texIndex-1], fTexCoords.xy) : frontColor;
                                                                              float z = gl_FragCoord.z;
                                                                              float l = layer;
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
                                                                      uniform sampler2D depthBuffer;
                                                                      uniform sampler2D stencilBuffer;
                                                                      uniform vec3 resolution;
                                                                      uniform mat4 lviewMatrix;
                                                                      uniform mat4 lprojectionMatrix;
                                                                      in vec4 frontColor;
                                                                      in vec2 fTexCoords;
                                                                      in flat uint texIndex;
                                                                      in flat uint layer;
                                                                      in vec4 shadowCoords;
                                                                      void main() {
                                                                          vec4 texel = (texIndex != 0) ? frontColor * texture2D(textures[texIndex-1], fTexCoords.xy) : frontColor;
                                                                          float current_alpha = texel.a;
                                                                          vec2 position = (gl_FragCoord.xy / resolution.xy);
                                                                          vec4 depth = texture2D (depthBuffer, position);
                                                                          vec4 alpha = imageLoad(alphaBuffer,ivec2(gl_FragCoord.xy));
                                                                          vec4 stencil = texture2D (stencilBuffer, shadowCoords.xy);
                                                                          float l = layer;
                                                                          float z = gl_FragCoord.z;
                                                                          if (l < stencil.y || l == stencil.z && stencil.z < shadowCoords.z && depth.z >= z && current_alpha > alpha.a) {
                                                                              imageStore(alphaBuffer,ivec2(gl_FragCoord.xy),vec4(0, l, z, current_alpha));
                                                                          } else {
                                                                              fColor = alpha;
                                                                          }
                                                                      }
                                                                      )";
                    /*const std::string buildShadowMapVertexShaderNormal = R"(#version 460
                                                                            layout (location = 0) in vec3 position;
                                                                            layout (location = 1) in vec4 color;
                                                                            layout (location = 2) in vec2 texCoords;
                                                                            layout (location = 3) in vec3 normals;
                                                                            uniform mat4 projectionMatrix;
                                                                            uniform mat4 viewMatrix;
                                                                            uniform mat4 textureMatrix;
                                                                            out vec2 fTexCoords;
                                                                            out vec4 frontColor;
                                                                            void main() {
                                                                                gl_Position = projectionMatrix * viewMatrix  * vec4(position, 1.f);
                                                                                fTexCoords = (textureMatrix * vec4(texCoords, 1.f, 1.f)).xy;
                                                                                frontColor = color;
                                                                            }
                                                                        )";
                    const std::string buildShadowMapVertexShader = R"(#version 460
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
                                                                          gl_Position = projectionMatrix * viewMatrix  * worldMat * vec4(position, 1.f);
                                                                          fTexCoords = (textureMatrix * vec4(texCoords, 1.f, 1.f)).xy;
                                                                          frontColor = color;
                                                                      }
                                                                    )";*/
                    const std::string buildShadowMapFragmentShader = R"(#version 460
                                                                        #extension GL_ARB_bindless_texture : enable
                                                                        in vec4 frontColor;
                                                                        in vec2 fTexCoords;

                                                                        layout (std140, binding = 0) uniform ALL_TEXTURES {
                                                                            sampler2D textures[200];
                                                                        };
                                                                        in flat uint texIndex;
                                                                        in flat uint layer;
                                                                        layout(binding = 0, rgba32f) uniform image2D stencilBuffer;
                                                                        layout (location = 0) out vec4 fColor;
                                                                        void main() {
                                                                            vec4 texel = (texIndex != 0) ? frontColor * texture2D(textures[texIndex-1], fTexCoords) : frontColor;
                                                                            float current_alpha = texel.a;
                                                                            vec4 alpha = imageLoad(stencilBuffer,ivec2(gl_FragCoord.xy));
                                                                            float l = layer;
                                                                            float z = gl_FragCoord.z;
                                                                            if (/*l > alpha.y || l == alpha.y &&*/ z > alpha.z) {
                                                                              fColor = vec4(0, l, z, current_alpha);
                                                                              imageStore(stencilBuffer,ivec2(gl_FragCoord.xy),vec4(0, l, z, current_alpha));
                                                                          } else {
                                                                              fColor = alpha;
                                                                          }
                                                                        }
                                                                    )";
                        const std::string perPixShadowVertexShader = R"(#version 460
                                                                   layout (location = 0) in vec3 position;
                                                                   layout (location = 1) in vec4 color;
                                                                   layout (location = 2) in vec2 texCoords;
                                                                   layout (location = 3) in vec3 normals;
                                                                   layout (location = 4) in mat4 worldMat;
                                                                   layout (location = 8) in mat4 shadowProjMat;
                                                                   layout (location = 12) in uint textureIndex;
                                                                   layout (location = 14) in uint l;
                                                                   uniform mat4 projectionMatrix;
                                                                   uniform mat4 viewMatrix;
                                                                   uniform mat4 lviewMatrix;
                                                                   uniform mat4 lprojectionMatrix;
                                                                   uniform mat4 depthBiasMatrix;
                                                                   uniform mat4 textureMatrix[)"+core::conversionUIntString(Texture::getAllTextures().size())+R"(];
                                                                   out vec4 shadowCoords;
                                                                   out vec4 frontColor;
                                                                   out vec2 fTexCoords;
                                                                   out uint texIndex;
                                                                   out uint layer;
                                                                   void main() {
                                                                       gl_Position = projectionMatrix * viewMatrix * shadowProjMat * worldMat * vec4(position, 1.f);
                                                                       fTexCoords = (textureIndex != 0) ? (textureMatrix[textureIndex-1] * vec4(texCoords, 1.f, 1.f)).xy : texCoords;
                                                                       frontColor = color;
                                                                       shadowCoords = depthBiasMatrix * lviewMatrix * lprojectionMatrix *  vec4(gl_Position.xyz, 1);
                                                                       texIndex = textureIndex;
                                                                       layer = l;
                                                                   }
                                                                  )";
                        const std::string perPixShadowNormalVertexShader = R"(#version 460
                                                                   layout (location = 0) in vec3 position;
                                                                   layout (location = 1) in vec4 color;
                                                                   layout (location = 2) in vec2 texCoords;
                                                                   layout (location = 3) in vec3 normals;
                                                                   layout (location = 4) in uint textureIndex;
                                                                   layout (location = 6) in uint l;
                                                                   uniform mat4 projectionMatrix;
                                                                   uniform mat4 viewMatrix;
                                                                   uniform mat4 lviewMatrix;
                                                                   uniform mat4 lprojectionMatrix;
                                                                   uniform mat4 depthBiasMatrix;
                                                                   uniform mat4 textureMatrix[)"+core::conversionUIntString(Texture::getAllTextures().size())+R"(];
                                                                   out vec4 shadowCoords;
                                                                   out vec4 frontColor;
                                                                   out vec2 fTexCoords;
                                                                   out uint texIndex;
                                                                   out uint layer;
                                                                   void main() {
                                                                       gl_Position = projectionMatrix * viewMatrix * vec4(position, 1.f);
                                                                       fTexCoords = (textureIndex != 0) ? (textureMatrix[textureIndex-1] * vec4(texCoords, 1.f, 1.f)).xy : texCoords;
                                                                       frontColor = color;
                                                                       shadowCoords = depthBiasMatrix * lviewMatrix * lprojectionMatrix * vec4(gl_Position.xyz, 1);
                                                                       texIndex = textureIndex;
                                                                       layer = l;
                                                                   }
                                                                  )";
                        const std::string perPixShadowFragmentShader = R"(#version 460
                                                                          #extension GL_ARB_bindless_texture : enable
                                                                          in vec4 shadowCoords;
                                                                          in vec4 frontColor;
                                                                          in vec2 fTexCoords;
                                                                          in flat uint texIndex;
                                                                          in flat uint layer;
                                                                          uniform sampler2D texture;
                                                                          uniform sampler2D stencilBuffer;
                                                                          uniform sampler2D depthBuffer;
                                                                          uniform sampler2D alphaBuffer;
                                                                          uniform float haveTexture;
                                                                          uniform vec3 resolution;
                                                                          layout (std140, binding = 0) uniform ALL_TEXTURES {
                                                                              sampler2D textures[200];
                                                                          };
                                                                          layout (location = 0) out vec4 fColor;
                                                                          void main() {
                                                                            vec2 position = (gl_FragCoord.xy / resolution.xy);
                                                                            vec4 depth = texture2D(depthBuffer, position);
                                                                            vec4 alpha = texture2D(alphaBuffer, position);
                                                                            vec4 texel = (texIndex != 0) ? frontColor * texture2D(textures[texIndex-1], fTexCoords) : frontColor;

                                                                            float color = texel.a;
                                                                            vec4 stencil = texture2D (stencilBuffer, shadowCoords.xy);
                                                                            float z = gl_FragCoord.z;
                                                                            vec4 visibility;
                                                                            uint l = layer;
                                                                            if (l < stencil.y || l == stencil.y && stencil.z < shadowCoords.z) {
                                                                                if (depth.z >= z) {
                                                                                    visibility = vec4 (1, 1, 1, alpha.a);
                                                                                } else {
                                                                                    visibility = vec4 (0.5, 0.5, 0.5, color);
                                                                                }
                                                                            } else {
                                                                                visibility = vec4 (1, 1, 1, 1);
                                                                            }
                                                                            fColor = visibility;
                                                                          }
                                                                          )";
                        if (!depthGenShader.loadFromMemory(buildDepthBufferVertexShader, buildDepthBufferFragmentShader))  {
                            throw core::Erreur(52, "Error, failed to load build depth buffer shader", 3);
                        }
                        if (!depthGenNormalShader.loadFromMemory(buildDepthBufferVertexShaderNormal, buildDepthBufferFragmentShader)) {
                            throw core::Erreur(51, "Error, failed to load build depth buffer normal shader", 3);
                        }
                        if (!buildShadowMapShader.loadFromMemory(buildDepthBufferVertexShader, buildShadowMapFragmentShader)) {
                            throw core::Erreur(53, "Error, failed to load build shadow map shader", 3);
                        }
                        if (!buildShadowMapNormalShader.loadFromMemory(buildDepthBufferVertexShaderNormal, buildShadowMapFragmentShader)) {
                            throw core::Erreur(50, "Error, failed to load build shadow map normal shader", 3);
                        }
                        if (!perPixShadowShader.loadFromMemory(perPixShadowVertexShader, perPixShadowFragmentShader)) {
                            throw core::Erreur(54, "Error, failed to load per pix shadow map shader", 3);
                        }
                        if (!perPixShadowShaderNormal.loadFromMemory(perPixShadowNormalVertexShader, perPixShadowFragmentShader)) {
                            throw core::Erreur(54, "Error, failed to load per pix normal shadow map shader", 3);
                        }
                        if (!sBuildAlphaBufferShader.loadFromMemory(perPixShadowVertexShader,buildAlphaBufferFragmentShader)) {
                            throw core::Erreur(60, "Error, failed to load build alpha buffer shader", 3);
                        }
                        if (!sBuildAlphaBufferNormalShader.loadFromMemory(perPixShadowNormalVertexShader,buildAlphaBufferFragmentShader)) {
                            throw core::Erreur(60, "Error, failed to load build alpha normal buffer shader", 3);
                        }
                        depthGenShader.setParameter("texture", Shader::CurrentTexture);
                        buildShadowMapShader.setParameter("texture", Shader::CurrentTexture);
                        depthGenNormalShader.setParameter("texture", Shader::CurrentTexture);
                        buildShadowMapNormalShader.setParameter("texture", Shader::CurrentTexture);
                        perPixShadowShader.setParameter("stencilBuffer", stencilBuffer.getTexture());
                        perPixShadowShader.setParameter("depthBuffer", depthBuffer.getTexture());
                        perPixShadowShader.setParameter("texture", Shader::CurrentTexture);
                        perPixShadowShader.setParameter("resolution", resolution.x, resolution.y, resolution.z);
                        perPixShadowShader.setParameter("alphaBuffer", alphaBuffer.getTexture());
                        perPixShadowShaderNormal.setParameter("stencilBuffer", stencilBuffer.getTexture());
                        perPixShadowShaderNormal.setParameter("depthBuffer", depthBuffer.getTexture());
                        perPixShadowShaderNormal.setParameter("texture", Shader::CurrentTexture);
                        perPixShadowShaderNormal.setParameter("resolution", resolution.x, resolution.y, resolution.z);
                        perPixShadowShaderNormal.setParameter("alphaBuffer", alphaBuffer.getTexture());
                        sBuildAlphaBufferShader.setParameter("depthBuffer", depthBuffer.getTexture());
                        sBuildAlphaBufferShader.setParameter("stencilBuffer", stencilBuffer.getTexture());
                        sBuildAlphaBufferShader.setParameter("texture", Shader::CurrentTexture);
                        sBuildAlphaBufferShader.setParameter("resolution", resolution.x, resolution.y, resolution.z);
                        sBuildAlphaBufferNormalShader.setParameter("depthBuffer", depthBuffer.getTexture());
                        sBuildAlphaBufferNormalShader.setParameter("stencilBuffer", stencilBuffer.getTexture());
                        sBuildAlphaBufferNormalShader.setParameter("texture", Shader::CurrentTexture);
                        sBuildAlphaBufferNormalShader.setParameter("resolution", resolution.x, resolution.y, resolution.z);
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
                        buildShadowMapNormalShader.setParameter("textureMatrix", textureMatrices);
                        buildShadowMapShader.setParameter("textureMatrix", textureMatrices);
                        depthGenShader.setParameter("textureMatrix", textureMatrices);
                        depthGenNormalShader.setParameter("textureMatrix", textureMatrices);
                        perPixShadowShader.setParameter("textureMatrix", textureMatrices);
                        perPixShadowShaderNormal.setParameter("textureMatrix", textureMatrices);


                        unsigned int ubid;
                        glCheck(ubid = glGetUniformBlockIndex(buildShadowMapShader.getHandle(), "ALL_TEXTURES"));
                        glCheck(glUniformBlockBinding(buildShadowMapShader.getHandle(),    ubid, 0));
                        glCheck(ubid = glGetUniformBlockIndex(buildShadowMapNormalShader.getHandle(), "ALL_TEXTURES"));
                        glCheck(glUniformBlockBinding(buildShadowMapNormalShader.getHandle(),    ubid, 0));
                        glCheck(ubid = glGetUniformBlockIndex(depthGenShader.getHandle(), "ALL_TEXTURES"));
                        glCheck(glUniformBlockBinding(depthGenShader.getHandle(),    ubid, 0));
                        glCheck(ubid = glGetUniformBlockIndex(depthGenNormalShader.getHandle(), "ALL_TEXTURES"));
                        glCheck(glUniformBlockBinding(depthGenNormalShader.getHandle(),    ubid, 0));
                        glCheck(ubid = glGetUniformBlockIndex(perPixShadowShader.getHandle(), "ALL_TEXTURES"));
                        glCheck(glUniformBlockBinding(perPixShadowShader.getHandle(),    ubid, 0));
                        glCheck(ubid = glGetUniformBlockIndex(perPixShadowShaderNormal.getHandle(), "ALL_TEXTURES"));
                        glCheck(glUniformBlockBinding(perPixShadowShaderNormal.getHandle(),    ubid, 0));

                        glCheck(glGenBuffers(1, &ubo));
                        glCheck(glBindBuffer(GL_UNIFORM_BUFFER, ubo));
                        glCheck(glBufferData(GL_UNIFORM_BUFFER, sizeof(Samplers),allSamplers.tex, GL_STATIC_DRAW));
                        glCheck(glBindBuffer(GL_UNIFORM_BUFFER, 0));
                        glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                        stencilBuffer.setActive();
                        glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                        shadowMap.setActive();
                        glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                        depthBuffer.setActive();
                        glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                        //std::cout<<"size : "<<sizeof(Samplers)<<" "<<alignof (alignas(16) uint64_t[200])<<std::endl;

                        /*glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo2));
                        glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo3));*/


                        for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                            vbBindlessTex[i].setPrimitiveType(static_cast<sf::PrimitiveType>(i));
                        }

                } else {
                    if (Shader::isAvailable()) {
                        const std::string buildShadowMapVertexShader =
                            "#version 130 \n"
                            "out mat4 projMat;"
                            "void main () {"
                                "gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
                                "gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;"
                                "gl_FrontColor = gl_Color;"
                                "projMat = gl_ProjectionMatrix;"
                            "}";
                        const std::string buildShadowMapFragmentShader =
                            "#version 130 \n"
                            "uniform sampler2D texture;"
                            "uniform float haveTexture;"
                            "in mat4 projMat;"
                            "mat4 inverse(mat4 mat) {"
                            "   mat4 inv;"
                            "   return inv;"
                            "}"
                            "void main() {"
                            "   vec4 texel = texture2D(texture, gl_TexCoord[0].xy);"
                            "   vec4 colors[2];"
                            "   colors[1] = texel * gl_Color;"
                            "   colors[0] = gl_Color;"
                            "   bool b = (haveTexture == 1);"
                            "   float color = colors[int(b)].a;"
                            "   float z = (gl_FragCoord.w != 1.f) ? (inverse(projMat) * vec4(0, 0, 0, gl_FragCoord.w)).w : gl_FragCoord.z;"
                            "   gl_FragColor = vec4(0, 0, z, color);"
                            "}";
                        const std::string perPixShadowVertexShader =
                            "#version 130 \n"
                            "uniform mat4 depthBiasMatrix;"
                            "out vec4 shadowCoords;"
                            "out mat4 projMat;"
                            "void main () {"
                                "gl_Position = gl_ProjectionMatrix * gl_Vertex;"
                                "gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;"
                                "gl_FrontColor = gl_Color;"
                                "projMat = gl_ProjectionMatrix;"
                                "shadowCoords = depthBiasMatrix * vec4(gl_Position.xyz, 1);"
                            "}";
                        const std::string perPixShadowFragmentShader =
                            "#version 130 \n"
                            "uniform sampler2D texture;"
                            "uniform sampler2D stencilBuffer;"
                            "uniform float haveTexture;"
                            "in vec4 shadowCoords;"
                            "in mat4 projMat;"
                            "mat4 inverse(mat4 mat) {"
                            "   mat4 inv;"
                            "   return inv;"
                            "}"
                            "void main() {"
                            "   vec4 texel = texture2D(texture, gl_TexCoord[0].xy);"
                            "   vec4 colors[2];"
                            "   colors[1] = texel * gl_Color;"
                            "   colors[0] = gl_Color;"
                            "   bool b = (haveTexture == 1);"
                            "   float color = colors[int(b)].a;"
                            "   vec4 stencil = texture2D (stencilBuffer, shadowCoords.xy);"
                            "   float z = (gl_FragCoord.w != 1.f) ? (inverse(projMat) * vec4(0, 0, 0, gl_FragCoord.w)).w : gl_FragCoord.z;"
                            "   colors[1] = vec4 (0, 0, 0, color);"
                            "   colors[0] = vec4 (0.5, 0.5, 0.5, 0.5);"
                            "   b = (stencil.z < z);"
                            "   vec4 visibility = colors[int(b)];"
                            "   gl_FragColor = visibility;"
                            "}";
                        if (!buildShadowMapShader.loadFromMemory(buildShadowMapVertexShader, buildShadowMapFragmentShader)) {
                            throw core::Erreur(53, "Error, failed to load build shadow map shader", 3);
                        }
                        if (!perPixShadowShader.loadFromMemory(perPixShadowVertexShader, perPixShadowFragmentShader)) {
                            throw core::Erreur(54, "Error, failed to load per pix shadow map shader", 3);
                        }
                        buildShadowMapShader.setParameter("texture", Shader::CurrentTexture);
                        perPixShadowShader.setParameter("stencilBuffer", stencilBuffer.getTexture());
                        perPixShadowShader.setParameter("texture", Shader::CurrentTexture);


                    }   else {
                        throw core::Erreur(55, "Shader not supported!", 0);
                    }
                }
            }
            void ShadowRenderComponent::loadTextureIndexes() {
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
            void ShadowRenderComponent::onVisibilityChanged(bool visible) {
                if (visible) {
                    glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                } else {
                    glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0));
                }
            }
            void ShadowRenderComponent::drawInstanced() {
                for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                    vbBindlessTex[i].clear();
                }
                std::array<std::vector<DrawArraysIndirectCommand>, Batcher::nbPrimitiveTypes> drawArraysIndirectCommands;
                std::array<std::vector<float>, Batcher::nbPrimitiveTypes> matrices, matrices2;
                std::array<unsigned int, Batcher::nbPrimitiveTypes> firstIndex, baseInstance;
                for (unsigned int i = 0; i < firstIndex.size(); i++) {
                    firstIndex[i] = 0;
                }
                for (unsigned int i = 0; i < baseInstance.size(); i++) {
                    baseInstance[i] = 0;
                }
                for (unsigned int i = 0; i < m_instances.size(); i++) {
                    if (m_instances[i].getAllVertices().getVertexCount() > 0) {
                        unsigned int p = m_instances[i].getAllVertices().getPrimitiveType();
                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        std::vector<TransformMatrix*> tm = m_instances[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            std::array<float, 16> matrix = tm[j]->getMatrix().transpose().toGlMatrix();
                            for (unsigned int n = 0; n < 16; n++) {
                                matrices[p].push_back(matrix[n]);
                            }
                        }
                        unsigned int vertexCount = 0;
                        if (m_instances[i].getVertexArrays().size() > 0) {
                            entt::entity entity = m_instances[i].getVertexArrays()[0]->getEntityId();
                            for (unsigned int j = 0; j < m_instances[i].getVertexArrays().size(); j++) {
                                if (entity == m_instances[i].getVertexArrays()[j]->getEntityId()) {
                                    unsigned int p = m_instances[i].getVertexArrays()[j]->getPrimitiveType();
                                    for (unsigned int k = 0; k < m_instances[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                        vertexCount++;
                                        vbBindlessTex[p].append((*m_instances[i].getVertexArrays()[j])[k], (m_instances[i].getMaterial().getTexture() != nullptr) ? m_instances[i].getMaterial().getTexture()->getId() : 0);
                                        vbBindlessTex[p].addLayer(m_instances[i].getMaterial().getLayer());
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
                    }
                }
                glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboWorldMatrices));
                glCheck(glBufferData(GL_ARRAY_BUFFER, matrices.size() * sizeof(float), &matrices[0], GL_DYNAMIC_DRAW));
                glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vboIndirect));
                glCheck(glBufferData(GL_DRAW_INDIRECT_BUFFER, drawArraysIndirectCommands.size() * sizeof(DrawArraysIndirectCommand), &drawArraysIndirectCommands[0], GL_DYNAMIC_DRAW));
                glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
                RenderStates currentStates;
                currentStates.blendMode = sf::BlendNone;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboWorldMatrices));
                        glCheck(glBufferData(GL_ARRAY_BUFFER, matrices[p].size() * sizeof(float), &matrices[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vboIndirect));
                        glCheck(glBufferData(GL_DRAW_INDIRECT_BUFFER, drawArraysIndirectCommands[p].size() * sizeof(DrawArraysIndirectCommand), &drawArraysIndirectCommands[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
                        vbBindlessTex[p].update();
                        currentStates.shader = &buildShadowMapShader;
                        stencilBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), currentStates, vboIndirect, vboWorldMatrices);
                        currentStates.shader = &depthGenShader;
                        depthBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), currentStates, vboIndirect, vboWorldMatrices);
                        vbBindlessTex[p].clear();
                    }
                }
                physic::BoundingBox viewArea = view.getViewVolume();
                math::Vec3f position (viewArea.getPosition().x,viewArea.getPosition().y, view.getPosition().z);
                math::Vec3f size (viewArea.getWidth(), viewArea.getHeight(), 0);
                stencilBuffer.display();
                stencilBufferTile.setPosition(position);
                depthBuffer.display();
                depthBufferTile.setPosition(position);
                shadowMap.setView(view);
                for (unsigned int i = 0; i < firstIndex.size(); i++) {
                    firstIndex[i] = 0;
                }
                for (unsigned int i = 0; i < baseInstance.size(); i++) {
                    baseInstance[i] = 0;
                }
                for (unsigned int i = 0; i < matrices.size(); i++) {
                    matrices[i].clear();
                }
                for (unsigned int i = 0; i < drawArraysIndirectCommands.size(); i++) {
                    drawArraysIndirectCommands[i].clear();
                }
                for (unsigned int i = 0; i < m_shadow_instances.size(); i++) {
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    unsigned int p = m_shadow_instances[i].getAllVertices().getPrimitiveType();
                    if (m_shadow_instances[i].getAllVertices().getVertexCount() > 0) {
                        std::vector<TransformMatrix*> tm = m_shadow_instances[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            std::array<float, 16> matrix = tm[j]->getMatrix().transpose().toGlMatrix();
                            for (unsigned int n = 0; n < 16; n++) {
                                matrices[p].push_back(matrix[n]);
                            }
                        }

                        std::vector<TransformMatrix> tm2 = m_shadow_instances[i].getShadowProjMatrix();
                        for (unsigned int j = 0; j < tm2.size(); j++) {
                            tm2[j].update();
                            std::array<float, 16> matrix = tm2[j].getMatrix().transpose().toGlMatrix();
                            for (unsigned int n = 0; n < 16; n++) {
                                matrices2[p].push_back(matrix[n]);
                            }
                        }
                        unsigned int vertexCount=0;
                        if (m_shadow_instances[i].getVertexArrays().size() > 0) {
                            entt::entity entity = m_shadow_instances[i].getVertexArrays()[0]->getEntityId();
                            for (unsigned int j = 0; j < m_shadow_instances[i].getVertexArrays().size(); j++) {
                                if (entity == m_shadow_instances[i].getVertexArrays()[j]->getEntityId()) {
                                    unsigned int p = m_shadow_instances[i].getVertexArrays()[j]->getPrimitiveType();
                                    for (unsigned int k = 0; k < m_shadow_instances[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                        vertexCount++;
                                        vbBindlessTex[p].append((*m_shadow_instances[i].getVertexArrays()[j])[k], (m_shadow_instances[i].getMaterial().getTexture() != nullptr) ? m_shadow_instances[i].getMaterial().getTexture()->getId() : 0);
                                        vbBindlessTex[p].addLayer(m_shadow_instances[i].getMaterial().getLayer());
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
                    }
                }
                currentStates.shader = &sBuildAlphaBufferShader;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboWorldMatrices));
                        glCheck(glBufferData(GL_ARRAY_BUFFER, matrices[p].size() * sizeof(float), &matrices[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboShadowProjMatrices));
                        glCheck(glBufferData(GL_ARRAY_BUFFER, matrices2[p].size() * sizeof(float), &matrices2[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vboIndirect));
                        glCheck(glBufferData(GL_DRAW_INDIRECT_BUFFER, drawArraysIndirectCommands[p].size() * sizeof(DrawArraysIndirectCommand), &drawArraysIndirectCommands[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
                        vbBindlessTex[p].update();
                        alphaBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), currentStates, vboIndirect, vboWorldMatrices, vboShadowProjMatrices);
                    }
                }
                currentStates.shader = &perPixShadowShader;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboWorldMatrices));
                        glCheck(glBufferData(GL_ARRAY_BUFFER, matrices[p].size() * sizeof(float), &matrices[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboShadowProjMatrices));
                        glCheck(glBufferData(GL_ARRAY_BUFFER, matrices2[p].size() * sizeof(float), &matrices2[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vboIndirect));
                        glCheck(glBufferData(GL_DRAW_INDIRECT_BUFFER, drawArraysIndirectCommands[p].size() * sizeof(DrawArraysIndirectCommand), &drawArraysIndirectCommands[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
                        vbBindlessTex[p].update();
                        shadowMap.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), currentStates, vboIndirect, vboWorldMatrices, vboShadowProjMatrices);
                        vbBindlessTex[p].clear();
                    }
                }
                shadowMap.display();
            }
            void ShadowRenderComponent::drawInstancedIndexed() {
                for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                    vbBindlessTex[i].clear();
                }
                std::array<std::vector<DrawElementsIndirectCommand>, Batcher::nbPrimitiveTypes> drawElementsIndirectCommands;
                std::array<std::vector<float>, Batcher::nbPrimitiveTypes> matrices, matrices2;
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
                for (unsigned int i = 0; i < m_instancesIndexed.size(); i++) {
                    DrawElementsIndirectCommand drawElementsIndirectCommand;
                    if (m_instancesIndexed[i].getAllVertices().getVertexCount() > 0) {
                        unsigned int p = m_instancesIndexed[i].getAllVertices().getPrimitiveType();
                        std::vector<TransformMatrix*> tm = m_instancesIndexed[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            std::array<float, 16> matrix = tm[j]->getMatrix().transpose().toGlMatrix();
                            for (unsigned int n = 0; n < 16; n++) {
                                matrices[p].push_back(matrix[n]);
                            }
                        }
                        unsigned int vertexCount = 0, indexCount = 0;
                        if (m_instancesIndexed[i].getVertexArrays().size() > 0) {
                            entt::entity entity = m_instancesIndexed[i].getVertexArrays()[0]->getEntityId();
                            for (unsigned int j = 0; j < m_instancesIndexed[i].getVertexArrays().size(); j++) {
                                if (entity == m_instancesIndexed[i].getVertexArrays()[j]->getEntityId()) {
                                    unsigned int p = m_instancesIndexed[i].getVertexArrays()[j]->getPrimitiveType();
                                    for (unsigned int k = 0; k < m_instancesIndexed[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                        vertexCount++;
                                        vbBindlessTex[p].append((*m_instancesIndexed[i].getVertexArrays()[j])[k], (m_instancesIndexed[i].getMaterial().getTexture() != nullptr) ? m_instancesIndexed[i].getMaterial().getTexture()->getId() : 0);
                                        vbBindlessTex[p].addLayer(m_instancesIndexed[i].getMaterial().getLayer());
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
                    }
                }
                RenderStates currentStates;
                currentStates.blendMode = sf::BlendNone;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboWorldMatrices));
                        glCheck(glBufferData(GL_ARRAY_BUFFER, matrices[p].size() * sizeof(float), &matrices[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vboIndirect));
                        glCheck(glBufferData(GL_DRAW_INDIRECT_BUFFER, drawElementsIndirectCommands[p].size() * sizeof(DrawElementsIndirectCommand), &drawElementsIndirectCommands[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
                        vbBindlessTex[p].update();
                        currentStates.shader = &buildShadowMapShader;
                        stencilBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawElementsIndirectCommands[p].size(), currentStates, vboIndirect, vboWorldMatrices);
                        currentStates.shader = &depthGenShader;
                        depthBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawElementsIndirectCommands[p].size(), currentStates, vboIndirect, vboWorldMatrices);
                        vbBindlessTex[p].clear();
                    }
                }
                physic::BoundingBox viewArea = view.getViewVolume();
                math::Vec3f position (viewArea.getPosition().x,viewArea.getPosition().y, view.getPosition().z);
                math::Vec3f size (viewArea.getWidth(), viewArea.getHeight(), 0);
                stencilBuffer.display();
                stencilBufferTile.setPosition(position);
                depthBuffer.display();
                depthBufferTile.setPosition(position);
                shadowMap.setView(view);
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
                for (unsigned int i = 0; i < drawElementsIndirectCommands.size(); i++) {
                    drawElementsIndirectCommands[i].clear();
                }
                for (unsigned int i = 0; i < m_shadow_instances_indexed.size(); i++) {
                    DrawElementsIndirectCommand drawElementsIndirectCommand;
                    if (m_shadow_instances_indexed[i].getAllVertices().getVertexCount() > 0) {
                        unsigned int p = m_shadow_instances_indexed[i].getAllVertices().getPrimitiveType();
                        std::vector<TransformMatrix*> tm = m_shadow_instances_indexed[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            std::array<float, 16> matrix = tm[j]->getMatrix().transpose().toGlMatrix();
                            for (unsigned int n = 0; n < 16; n++) {
                                matrices[p].push_back(matrix[n]);
                            }
                        }

                        std::vector<TransformMatrix> tm2 = m_shadow_instances_indexed[i].getShadowProjMatrix();
                        for (unsigned int j = 0; j < tm2.size(); j++) {
                            tm2[j].update();
                            std::array<float, 16> matrix = tm2[j].getMatrix().transpose().toGlMatrix();
                            for (unsigned int n = 0; n < 16; n++) {
                                matrices2[p].push_back(matrix[n]);
                            }
                        }
                        unsigned int vertexCount = 0, indexCount = 0;
                        if (m_shadow_instances_indexed[i].getVertexArrays().size() > 0) {
                            entt::entity entity = m_shadow_instances_indexed[i].getVertexArrays()[0]->getEntityId();
                            for (unsigned int j = 0; j < m_shadow_instances_indexed[i].getVertexArrays().size(); j++) {
                                if (entity == m_shadow_instances_indexed[i].getVertexArrays()[j]->getEntityId()) {
                                    unsigned int p = m_shadow_instances_indexed[i].getVertexArrays()[j]->getPrimitiveType();
                                    for (unsigned int k = 0; k < m_shadow_instances_indexed[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                        vertexCount++;
                                        vbBindlessTex[p].append((*m_shadow_instances_indexed[i].getVertexArrays()[j])[k], (m_shadow_instances_indexed[i].getMaterial().getTexture() != nullptr) ? m_shadow_instances_indexed[i].getMaterial().getTexture()->getId() : 0);
                                        vbBindlessTex[p].addLayer(m_shadow_instances_indexed[i].getMaterial().getLayer());
                                    }
                                        for (unsigned int k = 0; k < m_shadow_instances_indexed[i].getVertexArrays()[j]->getIndexes().size(); k++) {
                                        indexCount++;
                                        vbBindlessTex[p].addIndex(m_shadow_instances_indexed[i].getVertexArrays()[j]->getIndexes()[k]);
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
                    }
                }
                currentStates.shader = &sBuildAlphaBufferShader;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboWorldMatrices));
                        glCheck(glBufferData(GL_ARRAY_BUFFER, matrices[p].size() * sizeof(float), &matrices[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboShadowProjMatrices));
                        glCheck(glBufferData(GL_ARRAY_BUFFER, matrices2[p].size() * sizeof(float), &matrices2[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vboIndirect));
                        glCheck(glBufferData(GL_DRAW_INDIRECT_BUFFER, drawElementsIndirectCommands[p].size() * sizeof(DrawElementsIndirectCommand), &drawElementsIndirectCommands[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
                        vbBindlessTex[p].update();
                        alphaBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawElementsIndirectCommands[p].size(), currentStates, vboIndirect, vboWorldMatrices, vboShadowProjMatrices);
                    }
                }
                currentStates.shader = &perPixShadowShader;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboWorldMatrices));
                        glCheck(glBufferData(GL_ARRAY_BUFFER, matrices[p].size() * sizeof(float), &matrices[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboShadowProjMatrices));
                        glCheck(glBufferData(GL_ARRAY_BUFFER, matrices2[p].size() * sizeof(float), &matrices2[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vboIndirect));
                        glCheck(glBufferData(GL_DRAW_INDIRECT_BUFFER, drawElementsIndirectCommands[p].size() * sizeof(DrawElementsIndirectCommand), &drawElementsIndirectCommands[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
                        vbBindlessTex[p].update();
                        shadowMap.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawElementsIndirectCommands[p].size(), currentStates, vboIndirect, vboWorldMatrices, vboShadowProjMatrices);
                        vbBindlessTex[p].clear();
                    }
                }
                shadowMap.display();
            }
            void ShadowRenderComponent::drawNormal() {
                for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                    vbBindlessTex[i].clear();
                }
                for (unsigned int i = 0; i < m_normals.size(); i++) {
                   if (m_normals[i].getAllVertices().getVertexCount() > 0) {
                        unsigned int p = m_normals[i].getAllVertices().getPrimitiveType();
                        for (unsigned int j = 0; j < m_normals[i].getAllVertices().getVertexCount(); j++) {
                            vbBindlessTex[p].append(m_normals[i].getAllVertices()[j],(m_normals[i].getMaterial().getTexture() != nullptr) ? m_normals[i].getMaterial().getTexture()->getNativeHandle() : 0);
                            vbBindlessTex[p].addLayer(m_normals[i].getMaterial().getLayer());
                        }
                    }
                }
                RenderStates states;
                states.blendMode = sf::BlendNone;
                states.shader = &depthGenNormalShader;
                states.texture = nullptr;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        states.shader = &depthGenNormalShader;
                        vbBindlessTex[p].update();
                        depthBuffer.drawVertexBuffer(vbBindlessTex[p], states);
                        states.shader = &buildShadowMapNormalShader;
                        stencilBuffer.drawVertexBuffer(vbBindlessTex[p], states);
                        vbBindlessTex[p].clear();
                    }
                }
                stencilBuffer.display();
                depthBuffer.display();
                for (unsigned int i = 0; i < m_shadow_normals.size(); i++) {
                    if (m_shadow_normals[i].getAllVertices().getVertexCount() > 0) {



                        unsigned int p = m_shadow_normals[i].getAllVertices().getPrimitiveType();
                        for (unsigned int j = 0; j < m_shadow_normals[i].getAllVertices().getVertexCount(); j++) {
                            vbBindlessTex[p].append(m_shadow_normals[i].getAllVertices()[j],(m_shadow_normals[i].getMaterial().getTexture() != nullptr) ? m_shadow_normals[i].getMaterial().getTexture()->getNativeHandle() : 0);
                            vbBindlessTex[p].addLayer(m_shadow_normals[i].getMaterial().getLayer());
                        }

                    }
                }
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    states.shader=&sBuildAlphaBufferNormalShader;
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        vbBindlessTex[p].update();
                        alphaBuffer.drawVertexBuffer(vbBindlessTex[p], states);
                    }
                }
                alphaBuffer.display();
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    states.shader=&perPixShadowShaderNormal;
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        vbBindlessTex[p].update();
                        shadowMap.drawVertexBuffer(vbBindlessTex[p], states);
                        vbBindlessTex[p].clear();
                    }
                }
                shadowMap.display();
            }
            void ShadowRenderComponent::drawNormalIndexed() {
                for (unsigned int i = 0; i < m_normalsIndexed.size(); i++) {
                   if (m_normalsIndexed[i].getAllVertices().getVertexCount() > 0) {
                        unsigned int p = m_normalsIndexed[i].getAllVertices().getPrimitiveType();
                        for (unsigned int j = 0; j < m_normalsIndexed[i].getAllVertices().getVertexCount(); j++) {
                            vbBindlessTex[p].append(m_normalsIndexed[i].getAllVertices()[j],(m_normalsIndexed[i].getMaterial().getTexture() != nullptr) ? m_normalsIndexed[i].getMaterial().getTexture()->getNativeHandle() : 0);
                            vbBindlessTex[p].addLayer(m_normalsIndexed[i].getMaterial().getLayer());
                        }
                        for (unsigned int j = 0; j < m_normalsIndexed[i].getAllVertices().getIndexes().size(); j++) {
                            vbBindlessTex[p].addIndex(m_normalsIndexed[i].getAllVertices().getIndexes()[j]);
                        }
                    }
                }
                RenderStates states;
                states.blendMode = sf::BlendNone;
                states.shader = &depthGenNormalShader;
                states.texture = nullptr;
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        states.shader = &depthGenNormalShader;
                        vbBindlessTex[p].update();
                        depthBuffer.drawVertexBuffer(vbBindlessTex[p], states);
                        states.shader = &buildShadowMapNormalShader;
                        stencilBuffer.drawVertexBuffer(vbBindlessTex[p], states);
                        vbBindlessTex[p].clear();
                    }
                }
                stencilBuffer.display();
                depthBuffer.display();
                for (unsigned int i = 0; i < m_shadow_normalsIndexed.size(); i++) {
                    if (m_shadow_normalsIndexed[i].getAllVertices().getVertexCount() > 0) {



                        unsigned int p = m_shadow_normalsIndexed[i].getAllVertices().getPrimitiveType();
                        for (unsigned int j = 0; j < m_shadow_normalsIndexed[i].getAllVertices().getVertexCount(); j++) {
                            vbBindlessTex[p].append(m_shadow_normalsIndexed[i].getAllVertices()[j],(m_shadow_normalsIndexed[i].getMaterial().getTexture() != nullptr) ? m_shadow_normalsIndexed[i].getMaterial().getTexture()->getNativeHandle() : 0);
                            vbBindlessTex[p].addLayer(m_shadow_normalsIndexed[i].getMaterial().getLayer());
                        }
                        for (unsigned int j = 0; j < m_shadow_normalsIndexed[i].getAllVertices().getIndexes().size(); j++) {
                            vbBindlessTex[p].addIndex(m_shadow_normalsIndexed[i].getAllVertices().getIndexes()[j]);
                        }
                    }
                }
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    states.shader=&sBuildAlphaBufferNormalShader;
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        vbBindlessTex[p].update();
                        alphaBuffer.drawVertexBuffer(vbBindlessTex[p], states);
                    }
                }
                alphaBuffer.display();
                for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                    states.shader=&perPixShadowShaderNormal;
                    if (vbBindlessTex[p].getVertexCount() > 0) {
                        vbBindlessTex[p].update();
                        shadowMap.drawVertexBuffer(vbBindlessTex[p], states);
                        vbBindlessTex[p].clear();
                    }
                }
                shadowMap.display();
            }
            void ShadowRenderComponent::drawNextFrame() {
                //glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                math::Vec3f centerLight = g2d::AmbientLight::getAmbientLight().getLightCenter();
                View lightView = View(view.getSize().x, view.getSize().y, -g2d::AmbientLight::getAmbientLight().getHeight(), g2d::AmbientLight::getAmbientLight().getHeight());
                lightView.setCenter(centerLight);
                math::Vec3f forward = view.getPosition() - lightView.getPosition();
                math::Vec3f target = lightView.getPosition() + forward;
                lightView.lookAt(target.x, target.y, target.z);
                stencilBuffer.setView(lightView);
                math::Vec3f v = lightView.getPosition() - view.getPosition();
                depthBuffer.setView(view);
                math::Matrix4f lviewMatrix = lightView.getViewMatrix().getMatrix().transpose();
                math::Matrix4f lprojMatrix = lightView.getProjMatrix().getMatrix().transpose();
                buildShadowMapShader.setParameter("projectionMatrix", lprojMatrix);
                buildShadowMapShader.setParameter("viewMatrix", lviewMatrix);
                buildShadowMapNormalShader.setParameter("projectionMatrix", lprojMatrix);
                buildShadowMapNormalShader.setParameter("viewMatrix", lviewMatrix);
                math::Matrix4f viewMatrix = view.getViewMatrix().getMatrix().transpose();
                math::Matrix4f projMatrix = view.getProjMatrix().getMatrix().transpose();
                depthGenShader.setParameter("projectionMatrix", projMatrix);
                depthGenShader.setParameter("viewMatrix", viewMatrix);
                depthGenNormalShader.setParameter("projectionMatrix", projMatrix);
                depthGenNormalShader.setParameter("viewMatrix", viewMatrix);
                sBuildAlphaBufferShader.setParameter("projectionMatrix", projMatrix);
                sBuildAlphaBufferShader.setParameter("viewMatrix", viewMatrix);
                sBuildAlphaBufferNormalShader.setParameter("projectionMatrix", projMatrix);
                sBuildAlphaBufferNormalShader.setParameter("viewMatrix", viewMatrix);
                math::Matrix4f biasMatrix(0.5f, 0.0f, 0.0f, 0.0f,
                                          0.0f, 0.5f, 0.0f, 0.0f,
                                          0.0f, 0.0f, 0.5f, 0.0f,
                                          0.5f, 0.5f, 0.5f, 1.f);
                math::Matrix4f depthBiasMatrix = biasMatrix;
                perPixShadowShader.setParameter("depthBiasMatrix", depthBiasMatrix.transpose());
                perPixShadowShader.setParameter("projectionMatrix", projMatrix);
                perPixShadowShader.setParameter("viewMatrix", viewMatrix);
                perPixShadowShader.setParameter("lviewMatrix", lviewMatrix);
                perPixShadowShader.setParameter("lprojectionMatrix", lprojMatrix);
                perPixShadowShaderNormal.setParameter("depthBiasMatrix", depthBiasMatrix.transpose());
                perPixShadowShaderNormal.setParameter("projectionMatrix", projMatrix);
                perPixShadowShaderNormal.setParameter("viewMatrix", viewMatrix);
                perPixShadowShaderNormal.setParameter("lviewMatrix", lviewMatrix);
                perPixShadowShaderNormal.setParameter("lprojectionMatrix", lprojMatrix);
                sBuildAlphaBufferShader.setParameter("lviewMatrix", lviewMatrix);
                sBuildAlphaBufferShader.setParameter("lprojectionMatrix", lviewMatrix);
                sBuildAlphaBufferShader.setParameter("depthBiasMatrix", depthBiasMatrix.transpose());
                sBuildAlphaBufferNormalShader.setParameter("lviewMatrix", lviewMatrix);
                sBuildAlphaBufferNormalShader.setParameter("lprojectionMatrix", lviewMatrix);
                sBuildAlphaBufferNormalShader.setParameter("depthBiasMatrix", depthBiasMatrix.transpose());
                drawInstanced();
                drawInstancedIndexed();
                drawNormal();
                drawNormalIndexed();
                    //glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0));

            }
            std::vector<EntityId> ShadowRenderComponent::getEntities() {
                return visibleEntities;
            }
            void ShadowRenderComponent::draw(RenderTarget& target, RenderStates states) {
                shadowTile.setCenter(target.getView().getPosition());
                states.blendMode = sf::BlendMultiply;
                target.draw(shadowTile, states);
            }
            void ShadowRenderComponent::pushEvent(window::IEvent event, RenderWindow& rw) {
                if (event.type == window::IEvent::WINDOW_EVENT && event.window.type == window::IEvent::WINDOW_EVENT_RESIZED && &getWindow() == &rw && isAutoResized()) {
                    recomputeSize();
                    getListener().pushEvent(event);
                    getView().reset(physic::BoundingBox(getView().getViewport().getPosition().x, getView().getViewport().getPosition().y, getView().getViewport().getPosition().z, event.window.data1, event.window.data2, getView().getViewport().getDepth()));
                }
            }
            bool ShadowRenderComponent::needToUpdate() {
                return update;
            }
            View& ShadowRenderComponent::getView() {
                return view;
            }
            int ShadowRenderComponent::getLayer() {
                return getPosition().z;
            }
            const Texture& ShadowRenderComponent::getStencilBufferTexture() {
                return stencilBuffer.getTexture();
            }
            const Texture& ShadowRenderComponent::getShadowMapTexture() {
                return shadowMap.getTexture();
            }
            Sprite& ShadowRenderComponent::getFrameBufferTile () {
                return stencilBufferTile;
            }
            Sprite& ShadowRenderComponent::getDepthBufferTile() {
                return shadowTile;
            }
            void ShadowRenderComponent::setExpression(std::string expression) {
                update = true;
                this->expression = expression;
            }
            std::string ShadowRenderComponent::getExpression() {
                return expression;
            }
            void ShadowRenderComponent::setView(View view) {
                this->view = view;/*View(view.getSize().x, view.getSize().y, view.getPosition().z, view.getDepth());
                this->view.setCenter(view.getPosition());*/
                shadowMap.setView(this->view);
                depthBuffer.setView(this->view);
            }
            bool ShadowRenderComponent::loadEntitiesOnComponent(ComponentMapping& componentMapping, std::vector<EntityId> vEntities)
            {

                batcher.clear();
                normalBatcher.clear();
                shadowBatcher.clear();
                normalShadowBatcher.clear();
                batcherIndexed.clear();
                shadowBatcherIndexed.clear();
                normalBatcherIndexed.clear();
                normalShadowBatcherIndexed.clear();

                for (unsigned int i = 0; i < vEntities.size(); i++) {
                    if (componentMapping.getComponent<MeshComponent>(vEntities[i]) != nullptr) {
                        EntityInfoComponent* eic = componentMapping.getComponent<EntityInfoComponent>(vEntities[i]);
                        MeshComponent* mc = componentMapping.getComponent<MeshComponent>(vEntities[i]);
                        TransformComponent *tc = componentMapping.getComponent<TransformComponent>(vEntities[i]);
                        ShadowInfoComponent* sic = componentMapping.getComponent<ShadowInfoComponent>(componentMapping.getRoot(vEntities[i]));
                        math::Vec3f shadowOrigin, shadowCenter, shadowScale(1.f, 1.f, 1.f), shadowRotationAxis, shadowTranslation;
                        float shadowRotationAngle = 0;

                        if (sic != nullptr) {
                            shadowCenter = sic->shadowCenter;
                            shadowScale = sic->shadowScale;
                            shadowRotationAxis = sic->shadowRotationAxis;
                            shadowRotationAngle = sic->shadowRotationAngle;
                            shadowOrigin = tc->position;
                            shadowTranslation = tc->position + shadowCenter;
                            /*if (componentMapping.getComponent<EntityInfoComponent>(componentMapping.getRoot(vEntities[i]))->groupName == "E_WALL") {
                                std::cout<<"shadow center : "<<shadowCenter<<std::endl;
                                std::cout<<"shadow scale : "<<shadowScale<<std::endl;
                                std::cout<<"shadow rotation axis : "<<shadowRotationAxis<<std::endl;
                                std::cout<<"shadow rotation angle : "<<shadowRotationAngle<<std::endl;
                                std::cout<<"shadow origin : "<<shadowOrigin<<std::endl;
                                std::cout<<"shadow translation : "<<shadowTranslation<<std::endl;
                            }*/
                        }
                        TransformMatrix tm;
                        tm.setOrigin(shadowOrigin);
                        tm.setScale(shadowScale);
                        tm.setRotation(shadowRotationAxis, shadowRotationAngle);
                        tm.setTranslation(shadowTranslation);

                        for (unsigned int j = 0; j <  mc->faces.size(); j++) {

                             if(eic->drawMode == DrawMode::INSTANCED) {
                                if (mc->faces[j].getVertexArray().getIndexes().size() == 0) {
                                    batcher.addFace(&mc->faces[j]);
                                    shadowBatcher.addShadowFace(&mc->faces[j],  view.getViewMatrix(), tm);
                                } else {
                                    batcherIndexed.addFace(&mc->faces[j]);
                                    shadowBatcherIndexed.addShadowFace(&mc->faces[j],  view.getViewMatrix(), tm);
                                }
                             } else {
                                 if (mc->faces[j].getVertexArray().getIndexes().size() == 0) {
                                    normalBatcher.addFace( &mc->faces[j]);
                                    normalShadowBatcher.addShadowFace(&mc->faces[j], view.getViewMatrix(), tm);
                                 } else {
                                    normalBatcherIndexed.addFace( &mc->faces[j]);
                                    normalShadowBatcherIndexed.addShadowFace(&mc->faces[j], view.getViewMatrix(), tm);
                                 }
                             }
                        }
                    }
                }

                m_instances = batcher.getInstances();
                m_normals = normalBatcher.getInstances();
                m_shadow_instances = shadowBatcher.getInstances();
                m_shadow_normals = normalShadowBatcher.getInstances();
                m_instancesIndexed = batcherIndexed.getInstances();
                m_shadow_instances_indexed = shadowBatcherIndexed.getInstances();
                m_normalsIndexed = normalBatcherIndexed.getInstances();
                m_shadow_normalsIndexed = normalShadowBatcherIndexed.getInstances();
                visibleEntities = vEntities;
                update = true;
                return true;
            }
            void ShadowRenderComponent::clear() {
                 shadowMap.clear(sf::Color::White);
                 depthBuffer.clear(sf::Color::Transparent);
                 glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf));
                 glCheck(glBindTexture(GL_TEXTURE_2D, depthTex));
                 glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, view.getSize().x, view.getSize().y, GL_RGBA,
                 GL_FLOAT, NULL));
                 glCheck(glBindTexture(GL_TEXTURE_2D, 0));
                 glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
                 stencilBuffer.clear(sf::Color::Transparent);
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
            }
            RenderTexture* ShadowRenderComponent::getFrameBuffer() {
                return &shadowMap;
            }
            ShadowRenderComponent::~ShadowRenderComponent() {
                glDeleteBuffers(1, &vboWorldMatrices);
                glDeleteBuffers(1, &vboShadowProjMatrices);
                glDeleteBuffers(1, &ubo);
            }
            #endif // VULKAN
        }
    }
}
