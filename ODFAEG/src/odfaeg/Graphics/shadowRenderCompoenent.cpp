#include "../../../include/odfaeg/Graphics/shadowRenderComponent.hpp"
//#include "../../../include/odfaeg/Graphics/application.h"
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
            quad(math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, window.getSize().y * 0.5f)),
            expression(expression) {
            update = false;
            datasReady = false;
            quad.move(math::Vec3f(-window.getView().getSize().x * 0.5f, -window.getView().getSize().y * 0.5f, 0));
            sf::Vector3i resolution ((int) window.getSize().x, (int) window.getSize().y, window.getView().getSize().z);
            //settings.depthBits = 24;
            depthBuffer.create(resolution.x, resolution.y, settings);
            //settings.depthBits = 0;

            depthBufferTile = Sprite(depthBuffer.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));

            glCheck(glGenTextures(1, &depthTex));
            glCheck(glBindTexture(GL_TEXTURE_2D, depthTex));
            glCheck(glTexStorage2D(GL_TEXTURE_2D,1,GL_RGBA32F,window.getView().getSize().x, window.getView().getSize().y));
            glCheck(glBindImageTexture(0, depthTex,0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F));
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
            //Debug image.
            shadowMap.create(resolution.x, resolution.y,settings);
            shadowTile = Sprite(shadowMap.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));
            glCheck(glGenBuffers(1, &atomicBuffer));
            glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicBuffer));
            glCheck(glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW));
            glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0));
            glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicBuffer));
            glCheck(glGenTextures(1, &frameBufferTex));
            glCheck(glBindTexture(GL_TEXTURE_2D, frameBufferTex));
            glCheck(glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, window.getView().getSize().x, window.getView().getSize().y));
            glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
            glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            glCheck(glBindImageTexture(0, frameBufferTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F));
            glCheck(glBindTexture(GL_TEXTURE_2D, 0));
            std::vector<GLfloat> texClearBuf(window.getView().getSize().x*window.getView().getSize().y*4, 0);
            glCheck(glGenBuffers(1, &clearBuf4));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf4));
            glCheck(glBufferData(GL_PIXEL_UNPACK_BUFFER, texClearBuf.size() * sizeof(GLfloat),
            &texClearBuf[0], GL_STATIC_COPY));
            glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
            core::FastDelegate<bool> signal (&ShadowRenderComponent::needToUpdate, this);
            core::FastDelegate<void> slot (&ShadowRenderComponent::drawNextFrame, this);
            core::Command cmd(signal, slot);
            getListener().connect("UPDATE", cmd);
            glCheck(glGenBuffers(1, &vboIndirect));
            glGenBuffers(1, &modelDataBuffer);
            glGenBuffers(1, &materialDataBuffer);
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
            const std::string simpleFragmentShader = R"(#version 460
                                                        layout(origin_upper_left) in vec4 gl_FragCoord;
                                                        layout(rgba32f, binding = 0) uniform image2D img_output;
                                                        layout(location = 0) out vec4 fcolor;
                                                        void main() {
                                                            fcolor = imageLoad(img_output, ivec2(gl_FragCoord.xy));
                                                        })";
            const std::string indirectRenderingVertexShader = R"(#version 460
                                                                 layout (location = 0) in vec3 position;
                                                                 layout (location = 1) in vec4 color;
                                                                 layout (location = 2) in vec2 texCoords;
                                                                 layout (location = 3) in vec3 normals;
                                                                 uniform mat4 projectionMatrix;
                                                                 uniform mat4 viewMatrix;
                                                                 uniform mat4 textureMatrix[)"+core::conversionUIntString(Texture::getAllTextures().size())+R"(];
                                                                 struct ModelData {
                                                                    mat4 modelMatrix;
                                                                    mat4 shadowProjMatrix;
                                                                 };
                                                                 struct MaterialData {
                                                                     uint textureIndex;
                                                                     uint layer;
                                                                 };
                                                                 layout(binding = 0, std430) buffer modelData {
                                                                     ModelData modelDatas[];
                                                                 };
                                                                 layout(binding = 1, std430) buffer materialData {
                                                                     MaterialData materialDatas[];
                                                                 };
                                                                 out vec2 fTexCoords;
                                                                 out vec4 frontColor;
                                                                 out uint texIndex;
                                                                 out uint layer;
                                                                 void main() {
                                                                    ModelData model = modelDatas[gl_BaseInstance + gl_InstanceID];
                                                                    MaterialData material = materialDatas[gl_DrawID];
                                                                    uint textureIndex = material.textureIndex;
                                                                    uint l = material.layer;
                                                                    gl_Position = projectionMatrix * viewMatrix * model.modelMatrix * vec4(position, 1.f);
                                                                    fTexCoords = (textureIndex != 0) ? (textureMatrix[textureIndex-1] * vec4(texCoords, 1.f, 1.f)).xy : texCoords;
                                                                    frontColor = color;
                                                                    texIndex = textureIndex;
                                                                    layer = l;
                                                                 }
                                                                 )";
             const std::string buildDepthBufferFragmentShader = R"(#version 460
                                                                  #extension GL_ARB_bindless_texture : enable
                                                                  #extension GL_ARB_fragment_shader_interlock : require
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
                                                                      beginInvocationInterlockARB();
                                                                      vec4 depth = imageLoad(depthBuffer,ivec2(gl_FragCoord.xy));
                                                                      if (/*l > depth.y || l == depth.y &&*/ z > depth.z) {
                                                                        fColor = vec4(0, l, z, texel.a);
                                                                        imageStore(depthBuffer,ivec2(gl_FragCoord.xy),vec4(0,l,z,texel.a));
                                                                        memoryBarrier();
                                                                      } else {
                                                                        fColor = depth;
                                                                      }
                                                                      endInvocationInterlockARB();
                                                                  }
                                                                )";
             const std::string buildAlphaBufferFragmentShader = R"(#version 460
                                                              #extension GL_ARB_bindless_texture : enable
                                                              #extension GL_ARB_fragment_shader_interlock : require
                                                              layout(std140, binding=0) uniform ALL_TEXTURES {
                                                                sampler2D textures[200];
                                                              };
                                                              layout(binding = 0, rgba32f) coherent uniform image2D alphaBuffer;
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
                                                                  beginInvocationInterlockARB();
                                                                  vec4 alpha = imageLoad(alphaBuffer,ivec2(gl_FragCoord.xy));
                                                                  vec3 projCoords = shadowCoords.xyz / shadowCoords.w;
                                                                  projCoords = projCoords * 0.5 + 0.5;
                                                                  vec4 stencil = texture2D (stencilBuffer, projCoords.xy);
                                                                  float l = layer;
                                                                  float z = gl_FragCoord.z;
                                                                  if (/*l > stencil.y || l == stencil.y &&*/ stencil.z > projCoords.z && depth.z > z && current_alpha > alpha.a) {
                                                                      imageStore(alphaBuffer,ivec2(gl_FragCoord.xy),vec4(0, l, z, current_alpha));
                                                                      memoryBarrier();
                                                                      fColor = vec4(0, 1, z, current_alpha);
                                                                  } else {
                                                                      fColor = alpha;
                                                                  }
                                                                  endInvocationInterlockARB();
                                                              }
                                                              )";
            const std::string buildShadowMapFragmentShader = R"(#version 460
                                                                #extension GL_ARB_bindless_texture : enable
                                                                #extension GL_ARB_fragment_shader_interlock : require
                                                                in vec4 frontColor;
                                                                in vec2 fTexCoords;

                                                                layout (std140, binding = 0) uniform ALL_TEXTURES {
                                                                    sampler2D textures[200];
                                                                };
                                                                in flat uint texIndex;
                                                                in flat uint layer;
                                                                layout(binding = 0, rgba32f) coherent uniform image2D stencilBuffer;
                                                                layout (location = 0) out vec4 fColor;
                                                                void main() {
                                                                    vec4 texel = (texIndex != 0) ? frontColor * texture2D(textures[texIndex-1], fTexCoords) : frontColor;
                                                                    float current_alpha = texel.a;
                                                                    beginInvocationInterlockARB();
                                                                    vec4 alpha = imageLoad(stencilBuffer,ivec2(gl_FragCoord.xy));
                                                                    float l = layer;
                                                                    float z = gl_FragCoord.z;
                                                                    if (/*l > alpha.y || l == alpha.y &&*/ z > alpha.z) {
                                                                        imageStore(stencilBuffer,ivec2(gl_FragCoord.xy),vec4(0, l, z, current_alpha));
                                                                        memoryBarrier();
                                                                        fColor = vec4(0, l, z, current_alpha);
                                                                    } else {
                                                                        fColor = alpha;
                                                                    }
                                                                    endInvocationInterlockARB();
                                                                }
                                                            )";
                const std::string perPixShadowIndirectRenderingVertexShader = R"(#version 460
                                                                 layout (location = 0) in vec3 position;
                                                                 layout (location = 1) in vec4 color;
                                                                 layout (location = 2) in vec2 texCoords;
                                                                 layout (location = 3) in vec3 normals;
                                                                 uniform mat4 projectionMatrix;
                                                                 uniform mat4 viewMatrix;
                                                                 uniform mat4 lviewMatrix;
                                                                 uniform mat4 lprojectionMatrix;
                                                                 uniform mat4 textureMatrix[)"+core::conversionUIntString(Texture::getAllTextures().size())+R"(];
                                                                 struct ModelData {
                                                                    mat4 modelMatrix;
                                                                    mat4 shadowProjMatrix;
                                                                 };
                                                                 struct MaterialData {
                                                                     uint textureIndex;
                                                                     uint layer;
                                                                 };
                                                                 layout(binding = 0, std430) buffer modelData {
                                                                     ModelData modelDatas[];
                                                                 };
                                                                 layout(binding = 1, std430) buffer materialData {
                                                                     MaterialData materialDatas[];
                                                                 };
                                                                 out vec4 shadowCoords;
                                                                 out vec2 fTexCoords;
                                                                 out vec4 frontColor;
                                                                 out uint texIndex;
                                                                 out uint layer;
                                                                 void main() {
                                                                    ModelData model = modelDatas[gl_BaseInstance + gl_InstanceID];
                                                                    MaterialData material = materialDatas[gl_DrawID];
                                                                    uint textureIndex = material.textureIndex;
                                                                    uint l = material.layer;
                                                                    gl_Position = projectionMatrix * viewMatrix * model.shadowProjMatrix * model.modelMatrix * vec4(position, 1.f);
                                                                    shadowCoords = lprojectionMatrix * lviewMatrix * model.shadowProjMatrix * model.modelMatrix * vec4(position, 1);
                                                                    fTexCoords = (textureIndex != 0) ? (textureMatrix[textureIndex-1] * vec4(texCoords, 1.f, 1.f)).xy : texCoords;
                                                                    frontColor = color;
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
                                                                  uniform sampler2D stencilBuffer;
                                                                  uniform sampler2D depthBuffer;
                                                                  uniform sampler2D alphaBuffer;
                                                                  uniform float haveTexture;
                                                                  uniform vec3 resolution;
                                                                  layout (std140, binding = 0) uniform ALL_TEXTURES {
                                                                      sampler2D textures[200];
                                                                  };
                                                                  layout (location = 0) out vec4 fColor;
                                                                  layout(rgba32f, binding = 0) uniform image2D img_output;
                                                                  layout(binding = 0, offset = 0) uniform atomic_uint nextNodeCounter;

                                                                 /*Functions to debug, draw numbers to the image,
                                                                  draw a vertical ligne*/
                                                                  void drawVLine (ivec2 position, int width, int nbPixels, vec4 color) {
                                                                      int startY = position.y;
                                                                      int startX = position.x;
                                                                      while (position.y < startY + nbPixels) {
                                                                         while (position.x < startX + width) {
                                                                            imageStore(img_output, position, color);
                                                                            position.x++;
                                                                         }
                                                                         position.y++;
                                                                         position.x = startX;
                                                                      }
                                                                  }
                                                                  /*Draw an horizontal line*/
                                                                  void drawHLine (ivec2 position, int height, int nbPixels, vec4 color) {
                                                                      int startY = position.y;
                                                                      int startX = position.x;
                                                                      while (position.y > startY - height) {
                                                                         while (position.x < startX + nbPixels) {
                                                                            imageStore(img_output, position, color);
                                                                            position.x++;
                                                                         }
                                                                         position.y--;
                                                                         position.x = startX;
                                                                      }
                                                                  }
                                                                  /*Draw digits.*/
                                                                  void drawDigit (ivec2 position, int nbPixels, vec4 color, uint digit) {
                                                                      int digitSize = nbPixels * 10;
                                                                      if (digit == 0) {
                                                                          drawVLine(position, digitSize / 2, nbPixels, color);
                                                                          drawHLine(position, digitSize, nbPixels, color);
                                                                          drawHLine(ivec2(position.x + digitSize / 2 - nbPixels, position.y), digitSize, nbPixels, color);
                                                                          drawVLine(ivec2(position.x, position.y - digitSize + nbPixels), digitSize / 2, nbPixels, color);
                                                                      } else if (digit == 1) {
                                                                          drawHLine(ivec2(position.x + digitSize / 2 - nbPixels, position.y), digitSize, nbPixels, color);
                                                                      } else if (digit == 2) {
                                                                          drawVLine(position, digitSize / 2, nbPixels, color);
                                                                          drawHLine(ivec2(position.x, position.y), digitSize / 2 + nbPixels / 2, nbPixels, color);
                                                                          drawVLine(ivec2(position.x, position.y - digitSize / 2 + nbPixels / 2), digitSize / 2, nbPixels, color);
                                                                          drawHLine(ivec2(position.x + digitSize / 2 - nbPixels, position.y - digitSize / 2 + nbPixels / 2), digitSize / 2 + nbPixels / 2, nbPixels, color);
                                                                          drawVLine(ivec2(position.x, position.y - digitSize + nbPixels), digitSize / 2, nbPixels, color);
                                                                      } else if (digit == 3) {
                                                                          drawHLine(ivec2(position.x + digitSize / 2 - nbPixels, position.y), digitSize, nbPixels, color);
                                                                          drawVLine(position, digitSize / 2, nbPixels, color);
                                                                          drawVLine(ivec2(position.x, position.y - digitSize / 2 + nbPixels / 2), digitSize / 2, nbPixels, color);
                                                                          drawVLine(ivec2(position.x, position.y - digitSize + nbPixels), digitSize / 2, nbPixels, color);
                                                                      } else if (digit == 4) {
                                                                          drawHLine(ivec2(position.x, position.y - digitSize / 2), digitSize / 2 + nbPixels / 2, nbPixels, color);
                                                                          drawHLine(ivec2(position.x + digitSize / 2 - nbPixels, position.y), digitSize, nbPixels, color);
                                                                          drawVLine(ivec2(position.x, position.y - digitSize / 2 + nbPixels / 2), digitSize / 2, nbPixels, color);
                                                                      } else if (digit == 5) {
                                                                          drawVLine(position, digitSize / 2, nbPixels, color);
                                                                          drawHLine(ivec2(position.x, position.y - digitSize / 2 + nbPixels / 2), digitSize / 2 + nbPixels / 2, nbPixels, color);
                                                                          drawVLine(ivec2(position.x, position.y - digitSize / 2 + nbPixels / 2), digitSize / 2, nbPixels, color);
                                                                          drawHLine(ivec2(position.x + digitSize / 2 - nbPixels, position.y), digitSize / 2 + nbPixels / 2, nbPixels, color);
                                                                          drawVLine(ivec2(position.x, position.y - digitSize + nbPixels), digitSize / 2, nbPixels, color);
                                                                      } else if (digit == 6) {
                                                                          drawVLine(position, digitSize / 2, nbPixels, color);
                                                                          drawHLine(ivec2(position.x + digitSize / 2 - nbPixels, position.y), digitSize, nbPixels, color);
                                                                          drawVLine(ivec2(position.x, position.y - digitSize / 2 + nbPixels / 2), digitSize / 2, nbPixels, color);
                                                                          drawHLine(position, digitSize / 2 + nbPixels / 2, nbPixels, color);
                                                                          drawVLine(ivec2(position.x, position.y - digitSize + nbPixels), digitSize / 2, nbPixels, color);
                                                                      } else if (digit == 7) {
                                                                          drawVLine(ivec2(position.x, position.y - digitSize + nbPixels), digitSize / 2, nbPixels, color);
                                                                          drawHLine(ivec2(position.x + digitSize / 2 - nbPixels, position.y), digitSize, nbPixels, color);
                                                                      } else if (digit == 8) {
                                                                          drawHLine(position, digitSize, nbPixels, color);
                                                                          drawHLine(ivec2(position.x + digitSize / 2 - nbPixels, position.y), digitSize, nbPixels, color);
                                                                          drawVLine(position, digitSize / 2, nbPixels, color);
                                                                          drawVLine(ivec2(position.x, position.y - digitSize / 2 + nbPixels / 2), digitSize / 2, nbPixels, color);
                                                                          drawVLine(ivec2(position.x, position.y - digitSize + nbPixels), digitSize / 2, nbPixels, color);
                                                                      } else if (digit == 9) {
                                                                          drawVLine(position, digitSize / 2, nbPixels, color);
                                                                          drawHLine(ivec2(position.x + digitSize / 2 - nbPixels, position.y), digitSize, nbPixels, color);
                                                                          drawVLine(ivec2(position.x, position.y - digitSize / 2 + nbPixels / 2), digitSize / 2, nbPixels, color);
                                                                          drawHLine(ivec2(position.x, position.y - digitSize / 2 + nbPixels / 2), digitSize / 2 + nbPixels / 2, nbPixels, color);
                                                                          drawVLine(ivec2(position.x, position.y - digitSize + nbPixels), digitSize / 2, nbPixels, color);
                                                                      }
                                                                  }
                                                                  void drawSquare(ivec2 position, int size, vec4 color) {
                                                                      int startY = position.y;
                                                                      int startX = position.x;
                                                                      while (position.y > startY - size) {
                                                                         while (position.x < startX + size) {
                                                                            imageStore(img_output, position, color);
                                                                            position.x++;
                                                                         }
                                                                         position.y--;
                                                                         position.x = startX;
                                                                      }
                                                                  }
                                                                  void drawPunt(ivec2 position, int nbPixels, vec4 color) {
                                                                      int puntSize = nbPixels * 2;
                                                                      drawSquare(position, puntSize, color);
                                                                  })" \
                                                                  R"(ivec2 print (ivec2 position, int nbPixels, vec4 color, double number) {
                                                                      int digitSize = nbPixels * 10;
                                                                      int digitSpacing = nbPixels * 6;
                                                                      if (number < 0) {
                                                                         number = -number;
                                                                         drawVLine(ivec2(position.x, position.y - digitSize / 2 + nbPixels / 2), digitSize / 2, nbPixels, color);
                                                                         position.x += digitSpacing;
                                                                      }
                                                                      int pe = int(number);
                                                                      int n = 0;
                                                                      uint rpe[10];
                                                                      do {
                                                                         uint digit = pe % 10;
                                                                         pe /= 10;
                                                                         if (n < 10) {
                                                                            rpe[n] = digit;
                                                                         }
                                                                         n++;
                                                                      } while (pe != 0);
                                                                      if (n >= 10)
                                                                        n = 9;
                                                                      //drawDigit(position, nbPixels, color,0);
                                                                      for (int i = n-1; i >= 0; i--) {
                                                                         drawDigit(position, nbPixels, color, rpe[i]);
                                                                         //drawDigit(position, nbPixels, color,n-i-1);
                                                                         position.x += digitSpacing;
                                                                      }
                                                                      double rest = fract(number);
                                                                      if (rest > 0) {
                                                                          drawPunt(position, nbPixels, color);
                                                                          position.x += digitSpacing;
                                                                          do {
                                                                             rest *= 10;
                                                                             int digit = int(rest);
                                                                             rest -= digit;
                                                                             drawDigit(position, nbPixels, color, digit);
                                                                             position.x += digitSpacing;
                                                                          } while (rest != 0);
                                                                      }
                                                                      return position;
                                                                  }
                                                                  ivec2 print (ivec2 position, int nbPixels, vec4 color, mat4 matrix) {
                                                                      int numberSpacing = 10;
                                                                      for (uint i = 0; i < 4; i++) {
                                                                         for (uint j = 0; j < 4; j++) {
                                                                            position = print(position, nbPixels, color, matrix[i][j]);
                                                                            position.x += numberSpacing;
                                                                         }
                                                                      }
                                                                      return position;
                                                                  }
                                                                  ivec2 print (ivec2 position, int nbPixels, vec4 color, vec4 vector) {
                                                                      int numberSpacing = 10;
                                                                      for (uint i = 0; i < 4; i++) {
                                                                        position = print(position, nbPixels, color, vector[i]);
                                                                        position.x += numberSpacing;
                                                                      }
                                                                      return position;
                                                                  }
                                                                void main() {
                                                                    uint fragmentIdx = atomicCounterIncrement(nextNodeCounter);
                                                                    vec2 position = (gl_FragCoord.xy / resolution.xy);
                                                                    vec4 depth = texture(depthBuffer, position);
                                                                    vec4 alpha = texture(alphaBuffer, position);
                                                                    vec4 texel = (texIndex != 0) ? frontColor * texture2D(textures[texIndex-1], fTexCoords) : frontColor;

                                                                    float color = texel.a;
                                                                    vec3 projCoords = shadowCoords.xyz / shadowCoords.w;
                                                                    projCoords = projCoords * 0.5 + 0.5;
                                                                    vec4 stencil = texture (stencilBuffer, projCoords.xy);
                                                                    float z = gl_FragCoord.z;
                                                                    vec4 visibility;
                                                                    uint l = layer;
                                                                    if (/*l > stencil.y || l == stencil.y &&*/ stencil.z > projCoords.z) {
                                                                        if (depth.z > z) {
                                                                            visibility = vec4 (1, 1, 1, alpha.a);
                                                                        } else {
                                                                            visibility = vec4 (0.5, 0.5, 0.5, color);
                                                                        }
                                                                    } else {
                                                                        visibility = vec4 (1, 1, 1, 1);
                                                                    }
                                                                    /*if (fragmentIdx == 0)
                                                                        print(ivec2(200, 100), 1, vec4(1, 0, 0, 1), vec4(0, 0, depth.z, z));*/
                                                                    fColor = visibility /*vec4(0, 0, z*100, 1)*/;
                                                                  }
                                                                  )";
                if (!debugShader.loadFromMemory(simpleVertexShader, simpleFragmentShader)) {
                    throw core::Erreur(51, "Failed to load debug shader", 0);
                }
                if (!depthGenShader.loadFromMemory(indirectRenderingVertexShader, buildDepthBufferFragmentShader))  {
                    throw core::Erreur(52, "Error, failed to load build depth buffer shader", 3);
                }
                if (!buildShadowMapShader.loadFromMemory(indirectRenderingVertexShader, buildShadowMapFragmentShader)) {
                    throw core::Erreur(53, "Error, failed to load build shadow map shader", 3);
                }
                if (!perPixShadowShader.loadFromMemory(perPixShadowIndirectRenderingVertexShader, perPixShadowFragmentShader)) {
                    throw core::Erreur(54, "Error, failed to load per pix shadow map shader", 3);
                }
                if (!sBuildAlphaBufferShader.loadFromMemory(perPixShadowIndirectRenderingVertexShader,buildAlphaBufferFragmentShader)) {
                    throw core::Erreur(60, "Error, failed to load build alpha buffer shader", 3);
                }
                math::Matrix4f viewMatrix = window.getDefaultView().getViewMatrix().getMatrix().transpose();
                math::Matrix4f projMatrix = window.getDefaultView().getProjMatrix().getMatrix().transpose();
                debugShader.setParameter("projectionMatrix", projMatrix);
                debugShader.setParameter("viewMatrix", viewMatrix);
                depthGenShader.setParameter("texture", Shader::CurrentTexture);
                buildShadowMapShader.setParameter("texture", Shader::CurrentTexture);
                perPixShadowShader.setParameter("stencilBuffer", stencilBuffer.getTexture());
                perPixShadowShader.setParameter("depthBuffer", depthBuffer.getTexture());
                perPixShadowShader.setParameter("texture", Shader::CurrentTexture);
                perPixShadowShader.setParameter("resolution", resolution.x, resolution.y, resolution.z);
                perPixShadowShader.setParameter("alphaBuffer", alphaBuffer.getTexture());
                sBuildAlphaBufferShader.setParameter("depthBuffer", depthBuffer.getTexture());
                sBuildAlphaBufferShader.setParameter("stencilBuffer", stencilBuffer.getTexture());
                sBuildAlphaBufferShader.setParameter("texture", Shader::CurrentTexture);
                sBuildAlphaBufferShader.setParameter("resolution", resolution.x, resolution.y, resolution.z);

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
                buildShadowMapShader.setParameter("textureMatrix", textureMatrices);
                depthGenShader.setParameter("textureMatrix", textureMatrices);
                perPixShadowShader.setParameter("textureMatrix", textureMatrices);
                sBuildAlphaBufferShader.setParameter("textureMatrix", textureMatrices);

                glCheck(glGenBuffers(1, &ubo));
                glCheck(glBindBuffer(GL_UNIFORM_BUFFER, ubo));
                glCheck(glBufferData(GL_UNIFORM_BUFFER, sizeof(Samplers),allSamplers.tex, GL_STATIC_DRAW));
                glCheck(glBindBuffer(GL_UNIFORM_BUFFER, 0));
                glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, modelDataBuffer));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, materialDataBuffer));
                stencilBuffer.setActive();
                glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, modelDataBuffer));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, materialDataBuffer));
                depthBuffer.setActive();
                glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, modelDataBuffer));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, materialDataBuffer));
                alphaBuffer.setActive();
                glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, modelDataBuffer));
                glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, materialDataBuffer));
                alphaBuffer.setActive(false);
                //std::cout<<"size : "<<sizeof(Samplers)<<" "<<alignof (alignas(16) uint64_t[200])<<std::endl;

                /*glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo2));
                glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo3));*/


                for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                    vbBindlessTex[i].setPrimitiveType(static_cast<sf::PrimitiveType>(i));
                }
                //getListener().launchThread();
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
                        unsigned int p = m_normals[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_normals[i].getMaterial().getTexture() != nullptr) ? m_normals[i].getMaterial().getTexture()->getNativeHandle() : 0;
                        material.layer = m_normals[i].getMaterial().getLayer();
                        materials[p].push_back(material);
                        TransformMatrix tm;
                        ModelData model;
                        model.worldMat = tm.getMatrix().transpose();
                        model.shadowProjMat = tm.getMatrix().transpose();
                        matrices[p].push_back(model);
                        unsigned int vertexCount = 0;
                        for (unsigned int j = 0; j < m_normals[i].getAllVertices().getVertexCount(); j++) {
                            vbBindlessTex[p].append(m_normals[i].getAllVertices()[j]);
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
                for (unsigned int i = 0; i < m_instances.size(); i++) {
                    if (m_instances[i].getAllVertices().getVertexCount() > 0) {
                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        unsigned int p = m_instances[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_instances[i].getMaterial().getTexture() != nullptr) ? m_instances[i].getMaterial().getTexture()->getNativeHandle() : 0;
                        material.layer = m_instances[i].getMaterial().getLayer();
                        materials[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_instances[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            ModelData model;
                            model.worldMat = tm[j]->getMatrix().transpose();
                            TransformMatrix tm;
                            model.shadowProjMat = tm.getMatrix().transpose();
                            matrices[p].push_back(model);
                        }
                        unsigned int vertexCount = 0;
                        if (m_instances[i].getVertexArrays().size() > 0) {
                            Entity* entity = m_instances[i].getVertexArrays()[0]->getEntity();
                            for (unsigned int j = 0; j < m_instances[i].getVertexArrays().size(); j++) {
                                if (entity == m_instances[i].getVertexArrays()[j]->getEntity()) {
                                    unsigned int p = m_instances[i].getVertexArrays()[j]->getPrimitiveType();
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
                    }
                }
                RenderStates currentStates;
                currentStates.blendMode = sf::BlendNone;
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
                        currentStates.shader = &buildShadowMapShader;
                        stencilBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), currentStates, vboIndirect);
                        currentStates.shader = &depthGenShader;
                        depthBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), currentStates, vboIndirect);
                        vbBindlessTex[p].clear();
                    }
                }
                glCheck(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
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
                for (unsigned int i = 0; i < materials.size(); i++) {
                    materials[i].clear();
                }
                for (unsigned int i = 0; i < drawArraysIndirectCommands.size(); i++) {
                    drawArraysIndirectCommands[i].clear();
                }
                for (unsigned int i = 0; i < m_shadow_normals.size(); i++) {
                    if (m_shadow_normals[i].getAllVertices().getVertexCount() > 0) {
                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        unsigned int p = m_shadow_normals[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_shadow_normals[i].getMaterial().getTexture() != nullptr) ? m_shadow_normals[i].getMaterial().getTexture()->getNativeHandle() : 0;
                        material.layer = m_shadow_normals[i].getMaterial().getLayer();
                        materials[p].push_back(material);
                        TransformMatrix tm;
                        ModelData model;
                        model.worldMat = tm.getMatrix().transpose();
                        model.shadowProjMat = tm.getMatrix().transpose();
                        matrices[p].push_back(model);
                        unsigned int vertexCount = 0;
                        for (unsigned int j = 0; j < m_shadow_normals[i].getAllVertices().getVertexCount(); j++) {
                            vertexCount++;
                            vbBindlessTex[p].append(m_shadow_normals[i].getAllVertices()[j]);
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
                for (unsigned int i = 0; i < m_shadow_instances.size(); i++) {
                    if (m_shadow_instances[i].getAllVertices().getVertexCount() > 0) {
                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        unsigned int p = m_shadow_instances[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_shadow_normals[i].getMaterial().getTexture() != nullptr) ? m_shadow_normals[i].getMaterial().getTexture()->getNativeHandle() : 0;
                        material.layer = m_shadow_normals[i].getMaterial().getLayer();
                        materials[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_shadow_instances[i].getTransforms();
                        std::vector<TransformMatrix> tm2 = m_shadow_instances[i].getShadowProjMatrix();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            tm2[j].update();
                            ModelData model;
                            model.worldMat = tm[j]->getMatrix().transpose();
                            model.shadowProjMat = tm2[j].getMatrix().transpose();
                            matrices[p].push_back(model);
                        }
                        unsigned int vertexCount=0;
                        if (m_shadow_instances[i].getVertexArrays().size() > 0) {
                            Entity* entity = m_shadow_instances[i].getVertexArrays()[0]->getEntity();
                            for (unsigned int j = 0; j < m_shadow_instances[i].getVertexArrays().size(); j++) {
                                if (entity == m_shadow_instances[i].getVertexArrays()[j]->getEntity()) {
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
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelDataBuffer));
                        glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, matrices[p].size() * sizeof(ModelData), &matrices[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialDataBuffer));
                        glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, materials[p].size() * sizeof(MaterialData), &materials[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vboIndirect));
                        glCheck(glBufferData(GL_DRAW_INDIRECT_BUFFER, drawArraysIndirectCommands[p].size() * sizeof(DrawArraysIndirectCommand), &drawArraysIndirectCommands[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
                        vbBindlessTex[p].update();
                        alphaBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), currentStates, vboIndirect);
                    }
                }
                glCheck(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
                currentStates.shader = &perPixShadowShader;
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
                        shadowMap.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), currentStates, vboIndirect);
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
                for (unsigned int i = 0; i < m_normalsIndexed.size(); i++) {
                   if (m_normalsIndexed[i].getAllVertices().getVertexCount() > 0) {
                        DrawElementsIndirectCommand drawElementsIndirectCommand;
                        unsigned int p = m_normalsIndexed[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_normalsIndexed[i].getMaterial().getTexture() != nullptr) ? m_normalsIndexed[i].getMaterial().getTexture()->getNativeHandle() : 0;
                        material.layer = m_normalsIndexed[i].getMaterial().getLayer();
                        materials[p].push_back(material);
                        TransformMatrix tm;
                        ModelData model;
                        model.worldMat = tm.getMatrix().transpose();
                        model.shadowProjMat = tm.getMatrix().transpose();
                        matrices[p].push_back(model);
                        unsigned int indexCount = 0, vertexCount = 0;
                        for (unsigned int j = 0; j < m_normalsIndexed[i].getAllVertices().getVertexCount(); j++) {
                            vertexCount++;
                            vbBindlessTex[p].append(m_normalsIndexed[i].getAllVertices()[j]);
                        }
                        for (unsigned int j = 0; j < m_normalsIndexed[i].getAllVertices().getIndexes().size(); j++) {
                            indexCount++;
                            vbBindlessTex[p].addIndex(m_normalsIndexed[i].getAllVertices().getIndexes()[j]);
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
                        MaterialData material;
                        material.textureIndex = (m_normalsIndexed[i].getMaterial().getTexture() != nullptr) ? m_normalsIndexed[i].getMaterial().getTexture()->getNativeHandle() : 0;
                        material.layer = m_normalsIndexed[i].getMaterial().getLayer();
                        materials[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_instancesIndexed[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            ModelData model;
                            model.worldMat = tm[j]->getMatrix().transpose();
                            TransformMatrix tm;
                            model.shadowProjMat = tm.getMatrix().transpose();
                            matrices[p].push_back(model);
                        }
                        unsigned int vertexCount = 0, indexCount = 0;
                        if (m_instancesIndexed[i].getVertexArrays().size() > 0) {
                            Entity* entity = m_instancesIndexed[i].getVertexArrays()[0]->getEntity();
                            for (unsigned int j = 0; j < m_instancesIndexed[i].getVertexArrays().size(); j++) {
                                if (entity == m_instancesIndexed[i].getVertexArrays()[j]->getEntity()) {
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
                    }
                }
                RenderStates currentStates;
                currentStates.blendMode = sf::BlendNone;
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
                        currentStates.shader = &buildShadowMapShader;
                        stencilBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawElementsIndirectCommands[p].size(), currentStates, vboIndirect);
                        currentStates.shader = &depthGenShader;
                        depthBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawElementsIndirectCommands[p].size(), currentStates, vboIndirect);
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
                for (unsigned int i = 0; i < materials.size(); i++) {
                    materials[i].clear();
                }
                for (unsigned int i = 0; i < drawElementsIndirectCommands.size(); i++) {
                    drawElementsIndirectCommands[i].clear();
                }
                for (unsigned int i = 0; i < m_shadow_normalsIndexed.size(); i++) {
                    if (m_shadow_normalsIndexed[i].getAllVertices().getVertexCount() > 0) {
                        DrawElementsIndirectCommand drawElementsIndirectCommand;
                        unsigned int p = m_shadow_normalsIndexed[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_shadow_normalsIndexed[i].getMaterial().getTexture() != nullptr) ? m_shadow_normalsIndexed[i].getMaterial().getTexture()->getNativeHandle() : 0;
                        material.layer = m_shadow_normalsIndexed[i].getMaterial().getLayer();
                        materials[p].push_back(material);
                        TransformMatrix tm;
                        ModelData model;
                        model.worldMat = tm.getMatrix().transpose();
                        model.shadowProjMat = tm.getMatrix().transpose();
                        matrices[p].push_back(model);
                        unsigned int indexCount = 0, vertexCount = 0;
                        for (unsigned int j = 0; j < m_shadow_normalsIndexed[i].getAllVertices().getVertexCount(); j++) {
                            vertexCount++;
                            vbBindlessTex[p].append(m_shadow_normalsIndexed[i].getAllVertices()[j]);
                        }
                        for (unsigned int j = 0; j < m_shadow_normalsIndexed[i].getAllVertices().getIndexes().size(); j++) {
                            indexCount++;
                            vbBindlessTex[p].addIndex(m_shadow_normalsIndexed[i].getAllVertices().getIndexes()[j]);
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
                for (unsigned int i = 0; i < m_shadow_instances_indexed.size(); i++) {
                    DrawElementsIndirectCommand drawElementsIndirectCommand;
                    if (m_shadow_instances_indexed[i].getAllVertices().getVertexCount() > 0) {
                        unsigned int p = m_shadow_instances_indexed[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_shadow_instances_indexed[i].getMaterial().getTexture() != nullptr) ? m_shadow_instances_indexed[i].getMaterial().getTexture()->getNativeHandle() : 0;
                        material.layer = m_shadow_instances_indexed[i].getMaterial().getLayer();
                        materials[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_shadow_instances_indexed[i].getTransforms();
                        std::vector<TransformMatrix> tm2 = m_shadow_instances_indexed[i].getShadowProjMatrix();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            tm2[j].update();
                            ModelData model;
                            model.worldMat = tm[j]->getMatrix().transpose();
                            model.shadowProjMat = tm2[j].getMatrix().transpose();
                            matrices[p].push_back(model);
                        }
                        unsigned int vertexCount = 0, indexCount = 0;
                        if (m_shadow_instances_indexed[i].getVertexArrays().size() > 0) {
                            Entity* entity = m_shadow_instances_indexed[i].getVertexArrays()[0]->getEntity();
                            for (unsigned int j = 0; j < m_shadow_instances_indexed[i].getVertexArrays().size(); j++) {
                                if (entity == m_shadow_instances_indexed[i].getVertexArrays()[j]->getEntity()) {
                                    unsigned int p = m_shadow_instances_indexed[i].getVertexArrays()[j]->getPrimitiveType();
                                    for (unsigned int k = 0; k < m_shadow_instances_indexed[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                        vertexCount++;
                                        vbBindlessTex[p].append((*m_shadow_instances_indexed[i].getVertexArrays()[j])[k]);
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
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelDataBuffer));
                        glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, matrices[p].size() * sizeof(ModelData), &matrices[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialDataBuffer));
                        glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, materials[p].size() * sizeof(MaterialData), &materials[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vboIndirect));
                        glCheck(glBufferData(GL_DRAW_INDIRECT_BUFFER, drawElementsIndirectCommands[p].size() * sizeof(DrawElementsIndirectCommand), &drawElementsIndirectCommands[p][0], GL_DYNAMIC_DRAW));
                        glCheck(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
                        vbBindlessTex[p].update();
                        alphaBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawElementsIndirectCommands[p].size(), currentStates, vboIndirect);
                    }
                }
                currentStates.shader = &perPixShadowShader;
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
                        shadowMap.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawElementsIndirectCommands[p].size(), currentStates, vboIndirect);
                        vbBindlessTex[p].clear();
                    }
                }
                shadowMap.display();
            }
            void ShadowRenderComponent::drawNextFrame() {
                //glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));

                {
                    std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                    if (datasReady) {
                        datasReady = false;
                        m_instances = batcher.getInstances();
                        m_normals = normalBatcher.getInstances();
                        m_shadow_instances = shadowBatcher.getInstances();
                        m_shadow_normals = normalShadowBatcher.getInstances();
                        m_instancesIndexed = batcherIndexed.getInstances();
                        m_shadow_instances_indexed = shadowBatcherIndexed.getInstances();
                        m_normalsIndexed = normalBatcherIndexed.getInstances();
                        m_shadow_normalsIndexed = normalShadowBatcherIndexed.getInstances();
                        m_stencil_buffer = normalStencilBuffer.getInstances();
                    }
                }

                math::Vec3f centerLight = g2d::AmbientLight::getAmbientLight().getLightCenter();

                View lightView = View(view.getSize().x, view.getSize().y, 0, g2d::AmbientLight::getAmbientLight().getHeight());
                lightView.setCenter(centerLight);
                math::Vec3f forward = (view.getPosition() - lightView.getPosition()).normalize();
                math::Vec3f target = lightView.getPosition() + forward;
                lightView.lookAt(target.x, target.y, target.z);
                stencilBuffer.setView(lightView);
                depthBuffer.setView(view);
                math::Matrix4f lviewMatrix = lightView.getViewMatrix().getMatrix().transpose();
                math::Matrix4f lprojMatrix = lightView.getProjMatrix().getMatrix().transpose();
                buildShadowMapShader.setParameter("projectionMatrix", lprojMatrix);
                buildShadowMapShader.setParameter("viewMatrix", lviewMatrix);
                float zNear = view.getViewport().getPosition().z;
                if (!view.isOrtho())
                    view.setPerspective(80, view.getViewport().getSize().x / view.getViewport().getSize().y, zNear * 0.5f, view.getViewport().getSize().z);
                math::Matrix4f viewMatrix = view.getViewMatrix().getMatrix().transpose();
                math::Matrix4f projMatrix = view.getProjMatrix().getMatrix().transpose();
                depthGenShader.setParameter("projectionMatrix", projMatrix);
                depthGenShader.setParameter("viewMatrix", viewMatrix);
                if (!view.isOrtho())
                    view.setPerspective(80, view.getViewport().getSize().x / view.getViewport().getSize().y, zNear, view.getViewport().getSize().z);
                viewMatrix = view.getViewMatrix().getMatrix().transpose();
                projMatrix = view.getProjMatrix().getMatrix().transpose();
                perPixShadowShader.setParameter("projectionMatrix", projMatrix);
                perPixShadowShader.setParameter("viewMatrix", viewMatrix);
                perPixShadowShader.setParameter("lviewMatrix", lviewMatrix);
                perPixShadowShader.setParameter("lprojectionMatrix", lprojMatrix);
                sBuildAlphaBufferShader.setParameter("projectionMatrix", projMatrix);
                sBuildAlphaBufferShader.setParameter("viewMatrix", viewMatrix);
                sBuildAlphaBufferShader.setParameter("lviewMatrix", lviewMatrix);
                sBuildAlphaBufferShader.setParameter("lprojectionMatrix", lprojMatrix);
                drawInstanced();
                drawInstancedIndexed();


                /*glCheck(glFinish());
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
                debugShader.setParameter("worldMat", matrix);
                RenderStates currentStates;
                currentStates.blendMode = sf::BlendAlpha;
                currentStates.shader = &debugShader;
                currentStates.texture = nullptr;
                shadowMap.drawVertexBuffer(vb, currentStates);
                glCheck(glFinish());
                shadowMap.display();*/
                    //glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0));

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
                alphaBuffer.setView(this->view);
            }
            bool ShadowRenderComponent::loadEntitiesOnComponent(std::vector<Entity*> vEntities)
            {
                {
                    std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                    datasReady = false;
                    batcher.clear();
                    normalBatcher.clear();
                    shadowBatcher.clear();
                    normalShadowBatcher.clear();
                    batcherIndexed.clear();
                    shadowBatcherIndexed.clear();
                    normalBatcherIndexed.clear();
                    normalShadowBatcherIndexed.clear();
                    normalStencilBuffer.clear();

                }



                for (unsigned int i = 0; i < vEntities.size(); i++) {
                    if ( vEntities[i] != nullptr && vEntities[i]->isLeaf()) {
                        std::lock_guard<std::recursive_mutex> lock(rec_mutex);
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
                            /*if (entity->getType() == "E_WALL") {
                                std::cout<<"shadow center : "<<shadowCenter<<std::endl;
                                std::cout<<"shadow scale : "<<shadowScale<<std::endl;
                                std::cout<<"shadow rotation axis : "<<shadowRotationAxis<<std::endl;
                                std::cout<<"shadow rotation angle : "<<shadowRotationAngle<<std::endl;
                                std::cout<<"shadow origin : "<<shadowOrigin<<std::endl;
                                std::cout<<"shadow translation : "<<shadowTranslation<<std::endl;
                            }*/
                        //}
                        TransformMatrix tm;
                        tm.setOrigin(shadowOrigin);
                        tm.setScale(shadowScale);
                        tm.setRotation(shadowRotationAxis, shadowRotationAngle);
                        tm.setTranslation(shadowTranslation);

                        for (unsigned int j = 0; j <  vEntities[i]->getNbFaces(); j++) {

                             if(vEntities[i]->getDrawMode() == Entity::INSTANCED) {
                                if (vEntities[i]->getFace(j)->getVertexArray().getIndexes().size() == 0) {
                                    batcher.addFace( vEntities[i]->getFace(j));
                                    shadowBatcher.addShadowFace(vEntities[i]->getFace(j),  view.getViewMatrix(), tm);
                                } else {
                                    batcherIndexed.addFace( vEntities[i]->getFace(j));
                                    shadowBatcherIndexed.addShadowFace(vEntities[i]->getFace(j),  view.getViewMatrix(), tm);
                                }
                             } else {
                                 if (vEntities[i]->getFace(j)->getVertexArray().getIndexes().size() == 0) {
                                    normalBatcher.addFace( vEntities[i]->getFace(j));
                                    if (vEntities[i]->getRootEntity()->getType() != "E_BIGTILE") {
                                        normalShadowBatcher.addShadowFace(vEntities[i]->getFace(j), view.getViewMatrix(), tm);
                                        normalStencilBuffer.addFace(vEntities[i]->getFace(j));
                                    }
                                 } else {
                                    normalBatcherIndexed.addFace( vEntities[i]->getFace(j));
                                    normalShadowBatcherIndexed.addShadowFace(vEntities[i]->getFace(j), view.getViewMatrix(), tm);
                                 }
                             }
                        }
                    }
                }
                visibleEntities = vEntities;
                update = true;
                std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                datasReady = true;
                return true;
            }
            void ShadowRenderComponent::clear() {
                 shadowMap.clear(sf::Color::White);
                 depthBuffer.clear(sf::Color::Transparent);
                 stencilBuffer.clear(sf::Color::Transparent);
                 alphaBuffer.clear(sf::Color::Transparent);
                 glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf));
                 glCheck(glBindTexture(GL_TEXTURE_2D, depthTex));
                 glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, view.getSize().x, view.getSize().y, GL_RGBA,
                 GL_FLOAT, NULL));
                 glCheck(glBindTexture(GL_TEXTURE_2D, 0));
                 glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
                 glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf2));
                 glCheck(glBindTexture(GL_TEXTURE_2D, stencilTex));
                 glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, view.getSize().x, view.getSize().y, GL_RGBA,
                 GL_FLOAT, NULL));
                 glCheck(glBindTexture(GL_TEXTURE_2D, 0));
                 glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
                 glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf3));
                 glCheck(glBindTexture(GL_TEXTURE_2D, alphaTex));
                 glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, view.getSize().x, view.getSize().y, GL_RGBA,
                 GL_FLOAT, NULL));
                 glCheck(glBindTexture(GL_TEXTURE_2D, 0));
                 glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
                 glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf4));
                 glCheck(glBindTexture(GL_TEXTURE_2D, frameBufferTex));
                 glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, view.getSize().x, view.getSize().y, GL_RGBA,
                 GL_FLOAT, NULL));
                 glCheck(glBindTexture(GL_TEXTURE_2D, 0));
                 glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
                 GLuint zero = 0;
                 glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicBuffer));
                 glCheck(glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero));
                 glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0));

            }
            RenderTexture* ShadowRenderComponent::getFrameBuffer() {
                return &shadowMap;
            }
            ShadowRenderComponent::~ShadowRenderComponent() {
                glDeleteBuffers(1, &modelDataBuffer);
                glDeleteBuffers(1, &materialDataBuffer);
                glDeleteBuffers(1, &ubo);
            }
            #endif // VULKAN
        }
    }
