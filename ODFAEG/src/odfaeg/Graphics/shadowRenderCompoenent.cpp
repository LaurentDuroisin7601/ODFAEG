#include "../../../include/odfaeg/Graphics/shadowRenderComponent.hpp"
#ifndef VULKAN
#include <SFML/OpenGL.hpp>
#include "glCheck.h"
#endif
#include <memory.h>
using namespace sf;
using namespace std;
namespace odfaeg {
    namespace graphic {
        #ifdef VULKAN
        #else
        ShadowRenderComponent::ShadowRenderComponent (RenderWindow& window, int layer, std::string expression,window::ContextSettings settings) :
            HeavyComponent(window, math::Vec3f(window.getView().getPosition().x, window.getView().getPosition().y, layer),
                          math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0),
                          math::Vec3f(window.getView().getSize().x + window.getView().getSize().x * 0.5f, window.getView().getPosition().y + window.getView().getSize().y * 0.5f, layer)),
            view(window.getView()),
            expression(expression) {
                update = false;
                sf::Vector3i resolution ((int) window.getSize().x, (int) window.getSize().y, window.getView().getSize().z);
                shadowMap.create(resolution.x, resolution.y,settings);
                settings.depthBits = 24;
                stencilBuffer.create(resolution.x, resolution.y,settings);
                depthBuffer.create(resolution.x, resolution.y, settings);
                stencilBufferTile = Sprite(stencilBuffer.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));
                shadowTile = Sprite(shadowMap.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));
                depthBufferTile = Sprite(depthBuffer.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));
                core::FastDelegate<bool> signal (&ShadowRenderComponent::needToUpdate, this);
                core::FastDelegate<void> slot (&ShadowRenderComponent::drawNextFrame, this);
                core::Command cmd(signal, slot);
                getListener().connect("UPDATE", cmd);
                if (settings.versionMajor >= 3 && settings.versionMinor >= 3) {
                    glGenBuffers(1, &vboWorldMatrices);
                    glGenBuffers(1, &vboShadowProjMatrices);
                    const std::string buildDepthBufferVertexShaderNormal = R"(#version 460
                                                                              layout (location = 0) in vec3 position;
                                                                              layout (location = 1) in vec4 color;
                                                                              layout (location = 2) in vec2 texCoords;
                                                                              layout (location = 3) in vec3 normals;
                                                                              layout (location = 4) in uint textureIndex;
                                                                              uniform mat4 projectionMatrix;
                                                                              uniform mat4 viewMatrix;
                                                                              uniform mat4 textureMatrix[)"+core::conversionUIntString(Texture::getAllTextures().size())+R"(];
                                                                              out vec2 fTexCoords;
                                                                              out vec4 frontColor;
                                                                              out uint texIndex;
                                                                              void main() {
                                                                                  gl_Position = projectionMatrix * viewMatrix * vec4(position, 1.f);
                                                                                  fTexCoords = (textureIndex != 0) ? (textureMatrix[textureIndex-1] * vec4(texCoords, 1.f, 1.f)).xy : texCoords;
                                                                                  frontColor = color;
                                                                                  texIndex = textureIndex;
                                                                              }
                                                                              )";
                    const std::string buildDepthBufferVertexShader = R"(#version 460
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
                    const std::string buildDepthBufferFragmentShader = R"(#version 460
                                                                          #extension GL_ARB_bindless_texture : enable
                                                                          in vec4 frontColor;
                                                                          in vec2 fTexCoords;
                                                                          uniform sampler2D texture;
                                                                          uniform float haveTexture;
                                                                          layout (std140, binding=0) uniform ALL_TEXTURES {
                                                                              sampler2D textures[200];
                                                                          };
                                                                          in flat uint texIndex;
                                                                          layout (location = 0) out vec4 fColor;
                                                                          void main () {
                                                                              vec4 texel = (texIndex != 0) ? frontColor * texture2D(textures[texIndex-1], fTexCoords) : frontColor;
                                                                              float current_alpha = texel.a;
                                                                              float z = gl_FragCoord.z;
                                                                              fColor = vec4(0, 0, z, current_alpha);
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
                                                                        uniform sampler2D texture;
                                                                        uniform float haveTexture;
                                                                        layout (std140, binding = 0) uniform ALL_TEXTURES {
                                                                            sampler2D textures[200];
                                                                        };
                                                                        in flat uint texIndex;
                                                                        layout (location = 0) out vec4 fColor;
                                                                        void main() {
                                                                            vec4 texel = (texIndex != 0) ? frontColor * texture2D(textures[texIndex-1], fTexCoords) : frontColor;
                                                                            float color = texel.a;
                                                                            float z = gl_FragCoord.z;
                                                                            fColor = vec4(0, 0, z, color);
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
                                                                   uniform mat4 projectionMatrix;
                                                                   uniform mat4 viewMatrix;
                                                                   uniform mat4 depthBiasMatrix;
                                                                   uniform mat4 textureMatrix[)"+core::conversionUIntString(Texture::getAllTextures().size())+R"(];
                                                                   out vec4 shadowCoords;
                                                                   out vec4 frontColor;
                                                                   out vec2 fTexCoords;
                                                                   out uint texIndex;
                                                                   void main() {
                                                                       gl_Position = projectionMatrix * viewMatrix * shadowProjMat * worldMat * vec4(position, 1.f);
                                                                       fTexCoords = (textureIndex != 0) ? (textureMatrix[textureIndex-1] * vec4(texCoords, 1.f, 1.f)).xy : texCoords;
                                                                       frontColor = color;
                                                                       shadowCoords = depthBiasMatrix * worldMat * viewMatrix * projectionMatrix * vec4(gl_Position.xyz, 1);
                                                                       texIndex = textureIndex;
                                                                   }
                                                                  )";
                        const std::string perPixShadowNormalVertexShader = R"(#version 460
                                                                   layout (location = 0) in vec3 position;
                                                                   layout (location = 1) in vec4 color;
                                                                   layout (location = 2) in vec2 texCoords;
                                                                   layout (location = 3) in vec3 normals;
                                                                   layout (location = 4) in uint textureIndex;
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
                                                                   void main() {
                                                                       gl_Position = projectionMatrix * viewMatrix * vec4(position, 1.f);
                                                                       fTexCoords = (textureIndex != 0) ? (textureMatrix[textureIndex-1] * vec4(texCoords, 1.f, 1.f)).xy : texCoords;
                                                                       frontColor = color;
                                                                       shadowCoords = depthBiasMatrix * lviewMatrix * lprojectionMatrix * vec4(gl_Position.xyz, 1);
                                                                       texIndex = textureIndex;
                                                                   }
                                                                  )";
                        const std::string perPixShadowFragmentShader = R"(#version 460
                                                                          #extension GL_ARB_bindless_texture : enable
                                                                          in vec4 shadowCoords;
                                                                          in vec4 frontColor;
                                                                          in vec2 fTexCoords;
                                                                          in flat uint texIndex;
                                                                          uniform sampler2D texture;
                                                                          uniform sampler2D stencilBuffer;
                                                                          uniform sampler2D depthBuffer;
                                                                          uniform float haveTexture;
                                                                          uniform vec3 resolution;
                                                                          layout (std140, binding = 0) uniform ALL_TEXTURES {
                                                                              sampler2D textures[200];
                                                                          };
                                                                          layout (location = 0) out vec4 fColor;
                                                                          void main() {
                                                                            vec2 position = (gl_FragCoord.xy / resolution.xy);
                                                                            vec4 depth = texture2D(depthBuffer, position);
                                                                            vec4 texel = (texIndex != 0) ? frontColor * texture2D(textures[texIndex-1], fTexCoords) : frontColor;

                                                                            float color = texel.a;
                                                                            vec4 stencil = texture2D (stencilBuffer, shadowCoords.xy);
                                                                            float z = gl_FragCoord.z;
                                                                            vec4 visibility;
                                                                            if (stencil.z < shadowCoords.z) {
                                                                                if (depth.z >= z) {
                                                                                    visibility = vec4 (1, 1, 1, depth.a);
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
                        depthGenShader.setParameter("texture", Shader::CurrentTexture);
                        buildShadowMapShader.setParameter("texture", Shader::CurrentTexture);
                        depthGenNormalShader.setParameter("texture", Shader::CurrentTexture);
                        buildShadowMapNormalShader.setParameter("texture", Shader::CurrentTexture);
                        perPixShadowShader.setParameter("stencilBuffer", stencilBuffer.getTexture());
                        perPixShadowShader.setParameter("depthBuffer", depthBuffer.getTexture());
                        perPixShadowShader.setParameter("texture", Shader::CurrentTexture);
                        perPixShadowShader.setParameter("resolution", resolution.x, resolution.y, resolution.z);
                        perPixShadowShaderNormal.setParameter("stencilBuffer", stencilBuffer.getTexture());
                        perPixShadowShaderNormal.setParameter("depthBuffer", depthBuffer.getTexture());
                        perPixShadowShaderNormal.setParameter("texture", Shader::CurrentTexture);
                        perPixShadowShaderNormal.setParameter("resolution", resolution.x, resolution.y, resolution.z);
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
                physic::BoundingBox viewArea = view.getViewVolume();
                math::Vec3f position (viewArea.getPosition().x,viewArea.getPosition().y, view.getPosition().z);
                math::Vec3f size (viewArea.getWidth(), viewArea.getHeight(), 0);
                depthBuffer.setView(view);
                for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                    vbBindlessTex[i].clear();
                }
                if (shadowMap.getSettings().versionMajor >= 3 && shadowMap.getSettings().versionMinor >= 3) {
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
                    RenderStates states;
                    for (unsigned int i = 0; i < m_instances.size(); i++) {
                        if (m_instances[i].getAllVertices().getVertexCount() > 0) {
                            states.texture = m_instances[i].getMaterial().getTexture();
                            if (m_instances[i].getMaterial().getTexture() != nullptr) {
                                math::Matrix4f texMatrix = m_instances[i].getMaterial().getTexture()->getTextureMatrix();
                                buildShadowMapShader.setParameter("textureMatrix", texMatrix);
                                buildShadowMapShader.setParameter("haveTexture", 1.f);
                                depthGenShader.setParameter("textureMatrix", texMatrix);
                                depthGenShader.setParameter("haveTexture", 1.f);
                            } else {
                                buildShadowMapShader.setParameter("haveTexture", 0.f);
                                depthGenShader.setParameter("haveTexture", 0.f);
                            }
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
                            states.shader = &buildShadowMapShader;
                            stencilBuffer.drawInstanced(vb, m_instances[i].getVertexArrays()[0]->getPrimitiveType(), 0, m_instances[i].getVertexArrays()[0]->getVertexCount(), tm.size(), states, vboWorldMatrices);
                            states.shader = &depthGenShader;
                            depthBuffer.drawInstanced(vb, m_instances[i].getVertexArrays()[0]->getPrimitiveType(), 0, m_instances[i].getVertexArrays()[0]->getVertexCount(), tm.size(), states, vboWorldMatrices);
                        }
                    }
                    for (unsigned int i = 0; i < m_normals.size(); i++) {
                       if (m_normals[i].getAllVertices().getVertexCount() > 0) {
                            if (m_normals[i].getMaterial().getTexture() != nullptr) {
                                math::Matrix4f texMatrix = m_normals[i].getMaterial().getTexture()->getTextureMatrix();
                                //buildShadowMapNormalShader.setParameter("textureMatrix", texMatrix);
                                buildShadowMapNormalShader.setParameter("haveTexture", 1.f);
                                //depthGenNormalShader.setParameter("textureMatrix", texMatrix);
                                depthGenNormalShader.setParameter("haveTexture", 1.f);
                            } else {
                                buildShadowMapNormalShader.setParameter("haveTexture", 0.f);
                                depthGenNormalShader.setParameter("haveTexture", 0.f);
                            }
                            states.blendMode = sf::BlendNone;
                            states.shader = &depthGenNormalShader;
                            states.texture = nullptr;
                            unsigned int p = m_normals[i].getAllVertices().getPrimitiveType();
                            for (unsigned int j = 0; j < m_normals[i].getAllVertices().getVertexCount(); j++) {
                                vbBindlessTex[p].append(m_normals[i].getAllVertices()[j],(m_normals[i].getMaterial().getTexture() != nullptr) ? m_normals[i].getMaterial().getTexture()->getNativeHandle() : 0);
                            }
                        }
                    }
                    states.blendMode = sf::BlendNone;
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
                    stencilBufferTile.setPosition(position);
                    depthBuffer.display();
                    depthBufferTile.setPosition(position);
                    shadowMap.setView(view);
                    math::Matrix4f biasMatrix(0.5f, 0.0f, 0.0f, 0.0f,
                                              0.0f, 0.5f, 0.0f, 0.0f,
                                              0.0f, 0.0f, 0.5f, 0.0f,
                                              0.5f, 0.5f, 0.5f, 1.f);
                    math::Matrix4f depthBiasMatrix = biasMatrix;
                    perPixShadowShaderNormal.setParameter("depthBiasMatrix", depthBiasMatrix.transpose());
                    perPixShadowShaderNormal.setParameter("projectionMatrix", projMatrix);
                    perPixShadowShaderNormal.setParameter("viewMatrix", viewMatrix);
                    perPixShadowShaderNormal.setParameter("lviewMatrix", lviewMatrix);
                     perPixShadowShaderNormal.setParameter("lprojectionMatrix", lprojMatrix);
                    states.shader = &perPixShadowShaderNormal;
                    for (unsigned int i = 0; i < m_shadow_normals.size(); i++) {
                        if (m_shadow_normals[i].getAllVertices().getVertexCount() > 0) {
                            states.texture = nullptr;
                            if (m_shadow_normals[i].getMaterial().getTexture() != nullptr) {
                                math::Matrix4f texMatrix = m_shadow_normals[i].getMaterial().getTexture()->getTextureMatrix();
                                //perPixShadowShaderNormal.setParameter("textureMatrix", texMatrix);
                                perPixShadowShaderNormal.setParameter("haveTexture", 1.f);
                            } else {
                                perPixShadowShaderNormal.setParameter("haveTexture", 0.f);
                            }

                            unsigned int p = m_shadow_normals[i].getAllVertices().getPrimitiveType();
                            for (unsigned int j = 0; j < m_shadow_normals[i].getAllVertices().getVertexCount(); j++) {
                                vbBindlessTex[p].append(m_shadow_normals[i].getAllVertices()[j],(m_shadow_normals[i].getMaterial().getTexture() != nullptr) ? m_shadow_normals[i].getMaterial().getTexture()->getNativeHandle() : 0);
                            }

                        }
                    }
                    for (unsigned int p = 0; p < Batcher::nbPrimitiveTypes; p++) {
                        if (vbBindlessTex[p].getVertexCount() > 0) {
                            vbBindlessTex[p].update();
                            shadowMap.drawVertexBuffer(vbBindlessTex[p], states);
                            vbBindlessTex[p].clear();
                        }
                    }
                    perPixShadowShader.setParameter("depthBiasMatrix", depthBiasMatrix.transpose());
                    perPixShadowShader.setParameter("projectionMatrix", projMatrix);
                    perPixShadowShader.setParameter("viewMatrix", viewMatrix);
                    states.shader = &perPixShadowShader;
                    for (unsigned int i = 0; i < m_shadow_instances.size(); i++) {
                        if (m_shadow_instances[i].getAllVertices().getVertexCount() > 0) {
                            states.texture = m_shadow_instances[i].getMaterial().getTexture();
                            if (m_shadow_instances[i].getMaterial().getTexture() != nullptr) {
                                math::Matrix4f texMatrix = m_shadow_instances[i].getMaterial().getTexture()->getTextureMatrix();
                                perPixShadowShader.setParameter("textureMatrix", texMatrix);
                                perPixShadowShader.setParameter("haveTexture", 1.f);
                            } else {
                                perPixShadowShader.setParameter("haveTexture", 0.f);
                            }
                            vb.clear();
                            vb.setPrimitiveType(m_shadow_instances[i].getVertexArrays()[0]->getPrimitiveType());
                            matrices.clear();
                            std::vector<TransformMatrix*> tm = m_shadow_instances[i].getTransforms();
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
                            matrices2.clear();
                            std::vector<TransformMatrix> tm2 = m_shadow_instances[i].getShadowProjMatrix();
                            for (unsigned int j = 0; j < tm2.size(); j++) {
                                tm2[j].update();
                                std::array<float, 16> matrix = tm2[j].getMatrix().transpose().toGlMatrix();
                                for (unsigned int n = 0; n < 16; n++) {
                                    matrices2.push_back(matrix[n]);
                                }
                            }
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, vboShadowProjMatrices));
                            glCheck(glBufferData(GL_ARRAY_BUFFER, matrices2.size() * sizeof(float), &matrices2[0], GL_DYNAMIC_DRAW));
                            glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
                            if (m_shadow_instances[i].getVertexArrays().size() > 0) {
                                Entity* entity = m_shadow_instances[i].getVertexArrays()[0]->getEntity();
                                for (unsigned int j = 0; j < m_shadow_instances[i].getVertexArrays().size(); j++) {
                                    if (entity == m_shadow_instances[i].getVertexArrays()[j]->getEntity()) {
                                        for (unsigned int k = 0; k < m_shadow_instances[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                            vb.append((*m_shadow_instances[i].getVertexArrays()[j])[k]);
                                        }
                                    }
                                }
                                vb.update();
                            }
                            shadowMap.drawInstanced(vb, m_shadow_instances[i].getVertexArrays()[0]->getPrimitiveType(), 0, m_shadow_instances[i].getVertexArrays()[0]->getVertexCount(), tm.size(), states, vboWorldMatrices, vboShadowProjMatrices);
                        }
                    }
                    shadowMap.display();
                    //glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0));
                } else {
                    RenderStates states;
                    states.shader = &buildShadowMapShader;
                    for (unsigned int i = 0; i < m_instances.size(); i++) {
                        if (m_instances[i].getAllVertices().getVertexCount() > 0) {
                            states.texture = m_instances[i].getMaterial().getTexture();
                            if (m_instances[i].getMaterial().getTexture() != nullptr) {
                                buildShadowMapShader.setParameter("haveTexture", 1.f);
                            } else {
                                buildShadowMapShader.setParameter("haveTexture", 0.f);
                            }
                            stencilBuffer.draw(m_instances[i].getAllVertices(), states);
                        }
                    }
                    stencilBuffer.display();
                    stencilBufferTile.setPosition(position);
                    shadowMap.setView(view);
                    math::Matrix4f biasMatrix(0.5f, 0.0f, 0.0f, 0.0f,
                                              0.0f, 0.5f, 0.0f, 0.0f,
                                              0.0f, 0.0f, 0.5f, 0.0f,
                                              0.5f, 0.5f, 0.5f, 1.f);
                    math::Matrix4f depthBiasMatrix = biasMatrix * view.getViewMatrix().getMatrix() * view.getProjMatrix().getMatrix();
                    perPixShadowShader.setParameter("depthBiasMatrix", depthBiasMatrix.transpose());
                    states.shader = &perPixShadowShader;
                    for (unsigned int i = 0; i < m_shadow_instances.size(); i++) {
                        if (m_shadow_instances[i].getAllVertices().getVertexCount() > 0) {
                            states.texture = m_shadow_instances[i].getMaterial().getTexture();
                            if (m_shadow_instances[i].getMaterial().getTexture() != nullptr) {
                                perPixShadowShader.setParameter("haveTexture", 1.f);
                            } else {
                                perPixShadowShader.setParameter("haveTexture", 0.f);
                            }

                            shadowMap.draw(m_shadow_instances[i].getAllVertices(), states);
                        }
                    }
                    shadowMap.display();

                }
            }
            std::vector<Entity*> ShadowRenderComponent::getEntities() {
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
            bool ShadowRenderComponent::loadEntitiesOnComponent(std::vector<Entity*> vEntities)
            {

                batcher.clear();
                normalBatcher.clear();
                shadowBatcher.clear();
                normalShadowBatcher.clear();
                for (unsigned int i = 0; i < vEntities.size(); i++) {
                    if ( vEntities[i]->isLeaf()) {
                        Entity* entity = vEntities[i]->getRootEntity();
                        math::Vec3f shadowOrigin, shadowCenter, shadowScale(1.f, 1.f, 1.f), shadowRotationAxis, shadowTranslation;
                        float shadowRotationAngle = 0;
                        //if (entity != nullptr && entity->isModel()) {
                            shadowCenter = entity->getShadowCenter();
                            shadowScale = entity->getShadowScale();
                            shadowRotationAxis = entity->getShadowRotationAxis();
                            shadowRotationAngle = entity->getShadowRotationAngle();
                            shadowOrigin = entity->getPosition();
                            shadowTranslation = entity->getPosition() + shadowCenter;
                        //}
                        TransformMatrix tm;
                        tm.setOrigin(shadowOrigin);
                        tm.setScale(shadowScale);
                        tm.setRotation(shadowRotationAxis, shadowRotationAngle);
                        tm.setTranslation(shadowTranslation);
                        for (unsigned int j = 0; j <  vEntities[i]->getNbFaces(); j++) {
                             if(vEntities[i]->getDrawMode() == Entity::INSTANCED) {
                                batcher.addFace( vEntities[i]->getFace(j));
                                shadowBatcher.addShadowFace(vEntities[i]->getFace(j),  view.getViewMatrix(), tm);
                             } else {
                                normalBatcher.addFace( vEntities[i]->getFace(j));
                                normalShadowBatcher.addShadowFace(vEntities[i]->getFace(j), view.getViewMatrix(), tm);
                             }
                        }
                    }
                }
                m_instances = batcher.getInstances();
                m_normals = normalBatcher.getInstances();
                m_shadow_instances = shadowBatcher.getInstances();
                m_shadow_normals = normalShadowBatcher.getInstances();
                visibleEntities = vEntities;
                update = true;
                return true;
            }
            void ShadowRenderComponent::clear() {
                 shadowMap.clear(sf::Color::White);
                 stencilBuffer.clear(sf::Color::Transparent);
                 depthBuffer.clear(sf::Color::Transparent);
            }
            ShadowRenderComponent::~ShadowRenderComponent() {
                glDeleteBuffers(1, &vboWorldMatrices);
                glDeleteBuffers(1, &vboShadowProjMatrices);
                glDeleteBuffers(1, &ubo);
            }
            #endif // VULKAN
        }
    }
