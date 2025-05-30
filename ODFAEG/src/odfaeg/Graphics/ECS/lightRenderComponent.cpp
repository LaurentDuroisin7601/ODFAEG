#include "../../../../include/odfaeg/Graphics/ECS/lightRenderComponent.hpp"
#ifndef VULKAN







#include <memory.h>
using namespace sf;
using namespace std;
namespace odfaeg {
    namespace graphic {
        namespace ecs {

            LightRenderComponent::LightRenderComponent (RenderWindow& window, int layer, std::string expression, ComponentMapping& componentMapping, window::ContextSettings settings) :
                        HeavyComponent(window, math::Vec3f(window.getView().getPosition().x, window.getView().getPosition().y, layer),
                                      math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0),
                                      math::Vec3f(window.getView().getSize().x + window.getView().getSize().x * 0.5f, window.getView().getPosition().y + window.getView().getSize().y * 0.5f, layer)),
                        view(window.getView()),
                        expression(expression),
                        quad(math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, window.getSize().y * 0.5f)),
                        componentMapping(componentMapping) {
                        update = false;
                        quad.move(math::Vec3f(-window.getView().getSize().x * 0.5f, -window.getView().getSize().y * 0.5f, 0));
                        sf::Vector3i resolution ((int) window.getSize().x, (int) window.getSize().y, window.getView().getSize().z);
                        settings.depthBits = 24;
                        depthBuffer.create(resolution.x, resolution.y,settings);
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

                        lightDepthBuffer.create(resolution.x, resolution.y,settings);
                        glCheck(glGenTextures(1, &lightDepthTex));
                        glCheck(glBindTexture(GL_TEXTURE_2D, lightDepthTex));
                        glCheck(glTexStorage2D(GL_TEXTURE_2D,1,GL_RGBA32F,window.getView().getSize().x, window.getView().getSize().y));
                        glCheck(glBindImageTexture(0, lightDepthTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F));
                        glCheck(glBindTexture(GL_TEXTURE_2D, 0));
                        std::vector<GLfloat> lDepthClearBuf(window.getView().getSize().x*window.getView().getSize().y*4, 0);
                        glCheck(glGenBuffers(1, &clearBuf2));
                        glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf2));
                        glCheck(glBufferData(GL_PIXEL_UNPACK_BUFFER, lDepthClearBuf.size() * sizeof(GLfloat),
                        &lDepthClearBuf[0], GL_STATIC_COPY));
                        glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
                        settings.depthBits = 0;
                        alphaBuffer.create(resolution.x, resolution.y,settings);
                        glCheck(glGenTextures(1, &alphaTex));
                        glCheck(glBindTexture(GL_TEXTURE_2D, alphaTex));
                        glCheck(glTexStorage2D(GL_TEXTURE_2D,1,GL_RGBA32F,window.getView().getSize().x, window.getView().getSize().y));
                        glCheck(glBindImageTexture(0, alphaTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F));
                        glCheck(glBindTexture(GL_TEXTURE_2D, 0));
                        std::vector<GLfloat> lAlphaClearBuf(window.getView().getSize().x*window.getView().getSize().y*4, 0);
                        glCheck(glGenBuffers(1, &clearBuf3));
                        glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf3));
                        glCheck(glBufferData(GL_PIXEL_UNPACK_BUFFER, lAlphaClearBuf.size() * sizeof(GLfloat),
                        &lAlphaClearBuf[0], GL_STATIC_COPY));
                        glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));


                        specularTexture.create(resolution.x, resolution.y,settings);
                        bumpTexture.create(resolution.x, resolution.y,settings);
                        lightMap.create(resolution.x, resolution.y,settings);
                        normalMap.create(resolution.x, resolution.y,settings);
                        normalMap.setView(window.getView());
                        depthBuffer.setView(window.getView());
                        specularTexture.setView(window.getView());
                        bumpTexture.setView(window.getView());
                        lightMap.setView(window.getView());
                        lightMapTile = Sprite(lightMap.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));
                        depthBufferTile = Sprite(depthBuffer.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));
                        normalMapTile = Sprite(normalMap.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));
                        specularBufferTile = Sprite(specularTexture.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));
                        bumpMapTile = Sprite(bumpTexture.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));


                        core::FastDelegate<bool> signal (&LightRenderComponent::needToUpdate, this);
                        core::FastDelegate<void> slot (&LightRenderComponent::drawNextFrame, this);
                        core::Command cmd(signal, slot);
                        getListener().connect("UPDATE", cmd);
                        glCheck(glGenBuffers(1, &vboWorldMatrices));
                        glCheck(glGenBuffers(1, &vboIndirect));
                        //To debug.
                        lightMap.setActive(true);
                        glCheck(glGenTextures(1, &frameBufferTex));
                        glCheck(glBindTexture(GL_TEXTURE_2D, frameBufferTex));
                        glCheck(glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, window.getView().getSize().x, window.getView().getSize().y));
                        glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
                        glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
                        glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
                        glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
                        glCheck(glBindImageTexture(0, frameBufferTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F));
                        glCheck(glBindTexture(GL_TEXTURE_2D, 0));
                        std::vector<GLfloat> texClearBuf(window.getView().getSize().x*window.getView().getSize().y*4, 0);
                        glCheck(glGenBuffers(1, &clearBuf4));
                        glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf4));
                        glCheck(glBufferData(GL_PIXEL_UNPACK_BUFFER, texClearBuf.size() * sizeof(GLfloat),
                        &texClearBuf[0], GL_STATIC_COPY));
                        glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
                        if (settings.versionMajor >= 3 && settings.versionMinor >= 3) {
                            glGenBuffers(1, &vboWorldMatrices);
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
                                                                             };
                                                                             struct MaterialData {
                                                                                uint textureIndex;
                                                                                uint layer;
                                                                                float specularIntensity;
                                                                                float specularPower;
                                                                                vec4 lightCenter;
                                                                                vec4 lightColor;
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
                        const std::string specularIndirectRenderingVertexShader = R"(#version 460
                                                                                     layout (location = 0) in vec3 position;
                                                                                     layout (location = 1) in vec4 color;
                                                                                     layout (location = 2) in vec2 texCoords;
                                                                                     layout (location = 3) in vec3 normals;
                                                                                     uniform mat4 projectionMatrix;
                                                                                     uniform mat4 viewMatrix;
                                                                                     uniform mat4 textureMatrix[)"+core::conversionUIntString(Texture::getAllTextures().size())+R"(];
                                                                                     struct ModelData {
                                                                                        mat4 modelMatrix;
                                                                                     };
                                                                                     struct MaterialData {
                                                                                        uint textureIndex;
                                                                                        uint layer;
                                                                                        float specularIntensity;
                                                                                        float specularPower;
                                                                                        vec4 lightCenter;
                                                                                        vec4 lightColor;
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
                                                                                     out vec2 specular;
                                                                                     void main() {
                                                                                         ModelData model = modelDatas[gl_BaseInstance + gl_InstanceID];
                                                                                         MaterialData material = materialDatas[gl_DrawID];
                                                                                         uint textureIndex = material.textureIndex;
                                                                                         uint l = material.layer;
                                                                                         gl_Position = projectionMatrix * viewMatrix * model.modelMatrix * vec4(position, 1.f);
                                                                                         frontColor = color;
                                                                                         texIndex = textureIndex;
                                                                                         layer = l;
                                                                                         specular = vec2(material.specularIntensity, material.specularPower);
                                                                                     }
                                                                                     )";
                        const std::string perPixLightingIndirectRenderingVertexShader = R"(#version 460
                                                                                      layout (location = 0) in vec3 position;
                                                                                      layout (location = 1) in vec4 color;
                                                                                      layout (location = 2) in vec2 texCoords;
                                                                                      layout (location = 3) in vec3 normals;
                                                                                      uniform mat4 projectionMatrix;
                                                                                      uniform mat4 viewMatrix;
                                                                                      uniform mat4 viewportMatrix;
                                                                                      uniform mat4 textureMatrix;
                                                                                      uniform vec3 resolution;
                                                                                      struct ModelData {
                                                                                         mat4 modelMatrix;
                                                                                      };
                                                                                      struct MaterialData {
                                                                                         uint textureIndex;
                                                                                         uint layer;
                                                                                         float specularIntensity;
                                                                                         float specularPower;
                                                                                         vec4 lightCenter;
                                                                                         vec4 lightColor;
                                                                                      };
                                                                                      layout(binding = 0, std430) buffer modelData {
                                                                                          ModelData modelDatas[];
                                                                                      };
                                                                                      layout(binding = 1, std430) buffer materialData {
                                                                                          MaterialData materialDatas[];
                                                                                      };
                                                                                      out vec2 fTexCoords;
                                                                                      out vec4 frontColor;
                                                                                      out uint layer;
                                                                                      out vec4 lightPos;
                                                                                      out vec4 lightColor;
                                                                                      void main() {
                                                                                         ModelData model = modelDatas[gl_BaseInstance + gl_InstanceID];
                                                                                         MaterialData material = materialDatas[gl_DrawID];
                                                                                         uint l = material.layer;
                                                                                         gl_Position = projectionMatrix * viewMatrix * model.modelMatrix * vec4(position, 1.f);
                                                                                         fTexCoords = (textureMatrix * vec4(texCoords, 1.f, 1.f)).xy;
                                                                                         frontColor = color;
                                                                                         layer = l;
                                                                                         vec4 coords = vec4(material.lightCenter.xyz, 1);
                                                                                         coords = projectionMatrix * viewMatrix * model.modelMatrix * coords;
                                                                                         if (coords.w == 0)
                                                                                             coords.w = resolution.z * 0.5;
                                                                                         coords = coords / coords.w;
                                                                                         coords = viewportMatrix * coords;
                                                                                         coords.w = material.lightCenter.w;
                                                                                         lightPos = coords;
                                                                                         lightColor = material.lightColor;
                                                                                      }
                                                                                      )";

                        const std::string depthGenFragShader = R"(#version 460
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
                                                                                imageStore(depthBuffer,ivec2(gl_FragCoord.xy),vec4(0,l,z,texel.a));
                                                                                memoryBarrier();
                                                                                fColor = vec4(0, l, z, texel.a);
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
                                                                      layout(binding = 0, rgba32f) uniform image2D alphaBuffer;
                                                                      layout (location = 0) out vec4 fColor;
                                                                      uniform sampler2D lightDepthBuffer;
                                                                      uniform vec3 resolution;
                                                                      uniform mat4 lviewMatrix;
                                                                      uniform mat4 lprojectionMatrix;
                                                                      in vec4 frontColor;
                                                                      in vec2 fTexCoords;
                                                                      in flat uint texIndex;
                                                                      in flat uint layer;
                                                                      void main() {
                                                                          vec4 texel = (texIndex != 0) ? frontColor * texture2D(textures[texIndex-1], fTexCoords.xy) : frontColor;
                                                                          float current_alpha = texel.a;
                                                                          vec2 position = (gl_FragCoord.xy / resolution.xy);
                                                                          vec4 depth = texture2D (lightDepthBuffer, position);
                                                                          beginInvocationInterlockARB();
                                                                          vec4 alpha = imageLoad(alphaBuffer,ivec2(gl_FragCoord.xy));
                                                                          float l = layer;
                                                                          float z = gl_FragCoord.z;
                                                                          if (/*l > depth.y || l == depth.y &&*/ depth.z >= z && current_alpha > alpha.a) {
                                                                              imageStore(alphaBuffer,ivec2(gl_FragCoord.xy),vec4(0, l, z, current_alpha));
                                                                              memoryBarrier();
                                                                              fColor = vec4(0, l, z, current_alpha);
                                                                          } else {
                                                                              fColor = alpha;
                                                                          }
                                                                          endInvocationInterlockARB();
                                                                      }
                                                                      )";
                        const std::string specularGenFragShader = R"(#version 460
                                                                     #extension GL_ARB_bindless_texture : enable
                                                                     layout(std140, binding=0) uniform ALL_TEXTURES {
                                                                        sampler2D textures[200];
                                                                     };
                                                                     in vec4 frontColor;
                                                                     in vec2 fTexCoords;
                                                                     in flat uint texIndex;
                                                                     in flat uint layer;
                                                                     in flat vec2 specular;
                                                                     uniform float maxM;
                                                                     uniform float maxP;
                                                                     uniform sampler2D depthBuffer;
                                                                     uniform sampler2D specularBuffer;
                                                                     uniform vec3 resolution;
                                                                     layout (location = 0) out vec4 fColor;
                                                                     void main() {
                                                                        vec4 texel = (texIndex != 0) ? frontColor * texture2D(textures[texIndex-1], fTexCoords.xy) : frontColor;
                                                                        vec4 depth = texture2D(depthBuffer, (gl_FragCoord.xy / resolution.xy));
                                                                        vec4 specular = texture2D(specularBuffer,(gl_FragCoord.xy / resolution.xy));
                                                                        vec4 colors[2];
                                                                        colors[1] = texel * frontColor;
                                                                        colors[0] = frontColor;
                                                                        bool b = (texIndex != 0);
                                                                        vec4 color = colors[int(b)];
                                                                        float z = gl_FragCoord.z;
                                                                        float intensity = (maxM != 0.f) ? specular.x / maxM : 0.f;
                                                                        float power = (maxP != 0.f) ? specular.y / maxP : 0.f;
                                                                        if (/*layer > depth.y || layer == depth.y &&*/ z > depth.z)
                                                                            fColor = vec4(intensity, power, z, color.a);
                                                                        else
                                                                            fColor = specular;
                                                                     }
                                                                  )";
                        const std::string bumpGenFragShader =    R"(#version 460
                                                                 #extension GL_ARB_bindless_texture : enable
                                                                 layout(std140, binding=0) uniform ALL_TEXTURES {
                                                                    sampler2D textures[200];
                                                                 };
                                                                 in vec4 frontColor;
                                                                 in vec2 fTexCoords;
                                                                 in flat uint texIndex;
                                                                 in flat uint layer;
                                                                 uniform sampler2D depthBuffer;
                                                                 uniform sampler2D bumpMap;
                                                                 uniform vec3 resolution;

                                                                 layout (location = 0) out vec4 fColor;
                                                                 void main() {
                                                                     vec4 color = (texIndex != 0) ? texture2D(textures[texIndex-1], fTexCoords.xy) : vec4(0, 0, 0, 0);
                                                                     vec2 position = gl_FragCoord.xy / resolution.xy;
                                                                     vec4 depth = texture2D(depthBuffer, position);
                                                                     vec4 bump = texture2D(bumpMap, position);
                                                                     if (/*layer > depth.y || layer == depth.y &&*/ gl_FragCoord.z > depth.z) {
                                                                        fColor = color;
                                                                     } else {
                                                                        fColor = bump;
                                                                     }
                                                                 }
                                                                 )";
                        const std::string perPixLightingFragmentShader =  R"(#version 460
                                                                 in vec4 frontColor;
                                                                 in vec2 fTexCoords;
                                                                 in flat uint layer;
                                                                 in flat vec4 lightColor;
                                                                 in flat vec4 lightPos;
                                                                 const vec2 size = vec2(2.0,0.0);
                                                                 const ivec3 off = ivec3(-1,0,1);
                                                                 uniform sampler2D depthTexture;
                                                                 uniform sampler2D lightMap;
                                                                 uniform sampler2D specularTexture;
                                                                 uniform sampler2D bumpMap;
                                                                 uniform sampler2D alphaMap;
                                                                 uniform vec3 resolution;
                                                                 layout (location = 0) out vec4 fColor;
                                                                 layout(rgba32f, binding = 0) uniform image2D img_output;

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

                                                                 void main () {
                                                                     vec2 position = (gl_FragCoord.xy / resolution.xy);
                                                                     vec2 invPosition = vec2(position.x, 1 - position.y);
                                                                     vec4 depth = texture2D(depthTexture, position);
                                                                     vec4 invDepth = texture2D (depthTexture, invPosition);
                                                                     vec4 alpha = texture2D(alphaMap, position);
                                                                     float s01 = textureOffset(depthTexture, position, off.xy).z;
                                                                     float s21 = textureOffset(depthTexture, position, off.zy).z;
                                                                     float s10 = textureOffset(depthTexture, position, off.yx).z;
                                                                     float s12 = textureOffset(depthTexture, position, off.yz).z;
                                                                     vec3 va = normalize (vec3(size.xy, s21 - s01));
                                                                     vec3 vb = normalize (vec3(size.yx, s12 - s10));
                                                                     vec3 normal = vec3(cross(va, vb));
                                                                     vec4 bump = texture2D(bumpMap, position);
                                                                     vec4 specularInfos = texture2D(specularTexture, position);
                                                                     vec3 sLightPos = vec3 (lightPos.x, lightPos.y, -lightPos.z * (gl_DepthRange.far - gl_DepthRange.near));
                                                                     float radius = lightPos.w;
                                                                     vec3 pixPos = vec3 (gl_FragCoord.x, gl_FragCoord.y, -depth.z * (gl_DepthRange.far - gl_DepthRange.near));
                                                                     vec4 lightMapColor = texture2D(lightMap, position);
                                                                     vec3 viewPos = vec3(resolution.x * 0.5f, resolution.y * 0.5f, 0);
                                                                     float z = gl_FragCoord.z;
                                                                     vec3 vertexToLight = sLightPos - pixPos;
                                                                     if (bump.x != 0 || bump.y != 0 || bump.z != 0) {
                                                                         vec3 tmpNormal = (normal.xyz);
                                                                         vec3 tangeant = normalize (vec3(size.xy, s21 - s01));
                                                                         vec3 binomial = normalize (vec3(size.yx, s12 - s10));
                                                                         normal.x = dot(bump.xyz, tangeant);
                                                                         normal.y = dot(bump.xyz, binomial);
                                                                         normal.z = dot(bump.xyz, tmpNormal);
                                                                     }
                                                                     if (/*layer > depth.y || layer == depth.y &&*/ z > depth.z) {
                                                                         vec4 specularColor = vec4(0, 0, 0, 0);
                                                                         float attenuation = 1.f - length(vertexToLight) / radius;
                                                                         vec3 pixToView = pixPos - viewPos;
                                                                         float normalLength = dot(normal.xyz, vertexToLight);
                                                                         vec3 lightReflect = vertexToLight + 2 * (normal.xyz * normalLength - vertexToLight);
                                                                         float m = specularInfos.r;
                                                                         float p = specularInfos.g;
                                                                         float specularFactor = dot(normalize(pixToView), normalize(lightReflect));
                                                                         specularFactor = pow (specularFactor, p);
                                                                         if (specularFactor > 0) {
                                                                             specularColor = vec4(lightColor.rgb, 1) * m * specularFactor;
                                                                         }
                                                                         if (normal.x != 0 || normal.y != 0 || normal.z != 0 && vertexToLight.z > 0.f) {
                                                                             vec3 dirToLight = normalize(vertexToLight.xyz);
                                                                             float nDotl = max(dot (dirToLight, normal.xyz), 0.0);
                                                                             attenuation *= nDotl;

                                                                         }
                                                                         fColor = vec4(lightColor.rgb, 1) * max(0.0f, attenuation) + specularColor * (1 - alpha.a);
                                                                     } else {
                                                                         fColor = lightMapColor;
                                                                     }
                                                                 }
                                                                 )";
                        if (!debugShader.loadFromMemory(simpleVertexShader, simpleFragmentShader)) {
                            throw core::Erreur(54, "Failed to load debug shader", 0);
                        }
                        if (!depthBufferGenerator.loadFromMemory(indirectRenderingVertexShader, depthGenFragShader))
                            throw core::Erreur(50, "Failed to load depth buffer generator shader", 0);


                        if (!specularTextureGenerator.loadFromMemory(specularIndirectRenderingVertexShader, specularGenFragShader))
                            throw core::Erreur(52, "Failed to load specular texture generator shader", 0);

                        if (!bumpTextureGenerator.loadFromMemory(indirectRenderingVertexShader, bumpGenFragShader))
                            throw core::Erreur(53, "Failed to load bump texture generator shader", 0);
                        if (!buildAlphaBufferGenerator.loadFromMemory(indirectRenderingVertexShader, buildAlphaBufferFragmentShader))
                            throw core::Erreur(53, "Failed to load build alpha buffer generator shader", 0);

                        if (!lightMapGenerator.loadFromMemory(perPixLightingIndirectRenderingVertexShader, perPixLightingFragmentShader))
                            throw core::Erreur(54, "Failed to load light map generator shader", 0);
                        bumpTextureGenerator.setParameter("resolution", resolution.x, resolution.y, resolution.z);
                        bumpTextureGenerator.setParameter("depthBuffer", depthBuffer.getTexture());
                        bumpTextureGenerator.setParameter("bumpMap", bumpTexture.getTexture());

                        specularTextureGenerator.setParameter("resolution", resolution.x, resolution.y, resolution.z);
                        specularTextureGenerator.setParameter("depthBuffer", depthBuffer.getTexture());
                        specularTextureGenerator.setParameter("specularBuffer", specularTexture.getTexture());

                        buildAlphaBufferGenerator.setParameter("lightDepthBuffer", lightDepthBuffer.getTexture());

                        lightMapGenerator.setParameter("resolution", resolution.x, resolution.y, resolution.z);
                        lightMapGenerator.setParameter("depthTexture", depthBuffer.getTexture());
                        lightMapGenerator.setParameter("alphaMap", alphaBuffer.getTexture());
                        lightMapGenerator.setParameter("specularTexture",specularTexture.getTexture());
                        lightMapGenerator.setParameter("bumpMap",bumpTexture.getTexture());
                        lightMapGenerator.setParameter("lightMap",lightMap.getTexture());
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
                            depthBufferGenerator.setParameter("textureMatrix", textureMatrices);
                            depthBufferNormalGenerator.setParameter("textureMatrix", textureMatrices);
                            specularTextureGenerator.setParameter("textureMatrix", textureMatrices);
                            specularTextureNormalGenerator.setParameter("textureMatrix", textureMatrices);
                            bumpTextureGenerator.setParameter("textureMatrix", textureMatrices);
                            bumpTextureNormalGenerator.setParameter("textureMatrix", textureMatrices);
                            depthBuffer.setActive();
                            glCheck(glGenBuffers(1, &ubo));
                            unsigned int ubid;
                            glCheck(ubid = glGetUniformBlockIndex(depthBufferGenerator.getHandle(), "ALL_TEXTURES"));
                            glCheck(glUniformBlockBinding(depthBufferGenerator.getHandle(),    ubid, 0));
                            glCheck(ubid = glGetUniformBlockIndex(depthBufferNormalGenerator.getHandle(), "ALL_TEXTURES"));
                            glCheck(glUniformBlockBinding(depthBufferNormalGenerator.getHandle(),    ubid, 0));
                            glCheck(glBindBuffer(GL_UNIFORM_BUFFER, ubo));
                            glCheck(glBufferData(GL_UNIFORM_BUFFER, sizeof(Samplers),allSamplers.tex, GL_STATIC_DRAW));
                            //std::cout<<"size : "<<sizeof(Samplers)<<" "<<alignof (alignas(16) uint64_t[200])<<std::endl;

                            glCheck(glBindBuffer(GL_UNIFORM_BUFFER, 0));
                            glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                            lightDepthBuffer.setActive(true);
                            glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                            alphaBuffer.setActive(true);
                            glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                            specularTexture.setActive(true);
                            glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                            bumpTexture.setActive(true);
                            glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));


                            for (unsigned int i = 0; i < Batcher::nbPrimitiveTypes; i++) {
                                vbBindlessTex[i].setPrimitiveType(static_cast<sf::PrimitiveType>(i));
                            }
                        } else {
                            if (Shader::isAvailable()) {
                                const std::string  vertexShader =
                                "#version 130 \n"
                                "out mat4 projMat;"
                                "void main () {"
                                    "gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
                                    "gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;"
                                    "gl_FrontColor = gl_Color;"
                                    "projMat = gl_ProjectionMatrix;"
                                "}";
                                const std::string  buildNormalMapVertexShader =
                                "#version 130 \n"
                                "void main () {"
                                    "gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
                                    "gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;"
                                    "gl_FrontColor = gl_Color;"
                                "}";
                                const std::string buildNormalMapFragmentShader =
                                "#version 130 \n"
                                "const vec2 size = vec2(2.0,0.0);"
                                "const ivec3 off = ivec3(-1,0,1);"
                                "uniform sampler2D texture;"
                                "void main() {"
                                    "vec4 depth = texture2D(texture, gl_TexCoord[0].xy);"
                                    "float s01 = textureOffset(texture, gl_TexCoord[0].xy, off.xy).z;"
                                    "float s21 = textureOffset(texture, gl_TexCoord[0].xy, off.zy).z;"
                                    "float s10 = textureOffset(texture, gl_TexCoord[0].xy, off.yx).z;"
                                    "float s12 = textureOffset(texture, gl_TexCoord[0].xy, off.yz).z;"
                                    "vec3 va = normalize (vec3(size.xy, s21 - s01));"
                                    "vec3 vb = normalize (vec3(size.yx, s12 - s10));"
                                    "gl_FragColor = vec4(cross(va, vb), depth.z);"
                                "}";
                                const std::string depthGenFragShader =
                                "#version 130 \n"
                                "uniform sampler2D texture;"
                                "uniform float haveTexture;"
                                "in mat4 projMat;"
                                "mat4 inverse(mat4 m) {"
                                "     float"
                                "     a00 = m[0][0], a01 = m[0][1], a02 = m[0][2], a03 = m[0][3],"
                                "     a10 = m[1][0], a11 = m[1][1], a12 = m[1][2], a13 = m[1][3],"
                                "     a20 = m[2][0], a21 = m[2][1], a22 = m[2][2], a23 = m[2][3],"
                                "     a30 = m[3][0], a31 = m[3][1], a32 = m[3][2], a33 = m[3][3],"
                                "     b00 = a00 * a11 - a01 * a10,"
                                "     b01 = a00 * a12 - a02 * a10,"
                                "     b02 = a00 * a13 - a03 * a10,"
                                "     b03 = a01 * a12 - a02 * a11,"
                                "     b04 = a01 * a13 - a03 * a11,"
                                "     b05 = a02 * a13 - a03 * a12,"
                                "     b06 = a20 * a31 - a21 * a30,"
                                "     b07 = a20 * a32 - a22 * a30,"
                                "     b08 = a20 * a33 - a23 * a30,"
                                "     b09 = a21 * a32 - a22 * a31,"
                                "     b10 = a21 * a33 - a23 * a31,"
                                "     b11 = a22 * a33 - a23 * a32,"
                                "     det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;"
                                "     return mat4("
                                "                 a11 * b11 - a12 * b10 + a13 * b09,"
                                "                 a02 * b10 - a01 * b11 - a03 * b09,"
                                "                 a31 * b05 - a32 * b04 + a33 * b03,"
                                "                 a22 * b04 - a21 * b05 - a23 * b03,"
                                "                 a12 * b08 - a10 * b11 - a13 * b07,"
                                "                 a00 * b11 - a02 * b08 + a03 * b07,"
                                "                 a32 * b02 - a30 * b05 - a33 * b01,"
                                "                 a20 * b05 - a22 * b02 + a23 * b01,"
                                "                 a10 * b10 - a11 * b08 + a13 * b06,"
                                "                 a01 * b08 - a00 * b10 - a03 * b06,"
                                "                 a30 * b04 - a31 * b02 + a33 * b00,"
                                "                 a21 * b02 - a20 * b04 - a23 * b00,"
                                "                 a11 * b07 - a10 * b09 - a12 * b06,"
                                "                 a00 * b09 - a01 * b07 + a02 * b06,"
                                "                 a31 * b01 - a30 * b03 - a32 * b00,"
                                "                 a20 * b03 - a21 * b01 + a22 * b00) / det;"
                            "}"
                                "void main () {"
                                    "vec4 texel = texture2D(texture, gl_TexCoord[0].xy);"
                                    "vec4 colors[2];"
                                    "colors[1] = texel * gl_Color;"
                                    "colors[0] = gl_Color;"
                                    "bool b = (haveTexture > 0.9);"
                                    "float current_alpha = colors[int(b)].a;"
                                    "float z = (gl_FragCoord.w != 1.f) ? (inverse(projMat) * vec4(0, 0, 0, gl_FragCoord.w)).w : gl_FragCoord.z;"
                                    "gl_FragColor = vec4(0, 0, z, current_alpha);"
                                "}";
                                const std::string specularGenFragShader =
                                "#version 130 \n"
                                "uniform sampler2D texture;"
                                "uniform float maxM;"
                                "uniform float maxP;"
                                "uniform float m;"
                                "uniform float p;"
                                "uniform float haveTexture;"
                                "in mat4 projMat;"
                                "mat4 inverse(mat4 m) {"
                                    "     float"
                                    "     a00 = m[0][0], a01 = m[0][1], a02 = m[0][2], a03 = m[0][3],"
                                    "     a10 = m[1][0], a11 = m[1][1], a12 = m[1][2], a13 = m[1][3],"
                                    "     a20 = m[2][0], a21 = m[2][1], a22 = m[2][2], a23 = m[2][3],"
                                    "     a30 = m[3][0], a31 = m[3][1], a32 = m[3][2], a33 = m[3][3],"
                                    "     b00 = a00 * a11 - a01 * a10,"
                                    "     b01 = a00 * a12 - a02 * a10,"
                                    "     b02 = a00 * a13 - a03 * a10,"
                                    "     b03 = a01 * a12 - a02 * a11,"
                                    "     b04 = a01 * a13 - a03 * a11,"
                                    "     b05 = a02 * a13 - a03 * a12,"
                                    "     b06 = a20 * a31 - a21 * a30,"
                                    "     b07 = a20 * a32 - a22 * a30,"
                                    "     b08 = a20 * a33 - a23 * a30,"
                                    "     b09 = a21 * a32 - a22 * a31,"
                                    "     b10 = a21 * a33 - a23 * a31,"
                                    "     b11 = a22 * a33 - a23 * a32,"
                                    "     det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;"
                                    "     return mat4("
                                    "                 a11 * b11 - a12 * b10 + a13 * b09,"
                                    "                 a02 * b10 - a01 * b11 - a03 * b09,"
                                    "                 a31 * b05 - a32 * b04 + a33 * b03,"
                                    "                 a22 * b04 - a21 * b05 - a23 * b03,"
                                    "                 a12 * b08 - a10 * b11 - a13 * b07,"
                                    "                 a00 * b11 - a02 * b08 + a03 * b07,"
                                    "                 a32 * b02 - a30 * b05 - a33 * b01,"
                                    "                 a20 * b05 - a22 * b02 + a23 * b01,"
                                    "                 a10 * b10 - a11 * b08 + a13 * b06,"
                                    "                 a01 * b08 - a00 * b10 - a03 * b06,"
                                    "                 a30 * b04 - a31 * b02 + a33 * b00,"
                                    "                 a21 * b02 - a20 * b04 - a23 * b00,"
                                    "                 a11 * b07 - a10 * b09 - a12 * b06,"
                                    "                 a00 * b09 - a01 * b07 + a02 * b06,"
                                    "                 a31 * b01 - a30 * b03 - a32 * b00,"
                                    "                 a20 * b03 - a21 * b01 + a22 * b00) / det;"
                                "}"
                                "void main() {"
                                    "vec4 texel = texture2D(texture, gl_TexCoord[0].xy);"
                                    "vec4 colors[2];"
                                    "colors[1] = texel * gl_Color;"
                                    "colors[0] = gl_Color;"
                                    "bool b = (haveTexture > 0.9);"
                                    "vec4 color = colors[int(b)];"
                                    "float z = (gl_FragCoord.w != 1.f) ? (inverse(projMat) * vec4(0, 0, 0, gl_FragCoord.w)).w : gl_FragCoord.z;"
                                    "float intensity = (maxM != 0.f) ? m / maxM : 0.f;"
                                    "float power = (maxP != 0.f) ? p / maxP : 0.f;"
                                    "gl_FragColor = vec4(intensity, power, z, color.a);"
                                "}";
                                 const std::string bumpGenFragShader =
                                 "#version 130 \n"
                                 "uniform sampler2D texture;"
                                 "uniform float haveTexture;"
                                 "void main() {"
                                     "vec4 texel = texture2D(texture, gl_TexCoord[0].xy);"
                                     "vec4 colors[2];"
                                     "colors[1] = texel * gl_Color;"
                                     "colors[0] = gl_Color;"
                                     "bool b = (haveTexture > 0.9);"
                                     "vec4 color = colors[int(b)];"
                                     "gl_FragColor = color;"
                                 "}";
                                 const std::string perPixLightingFragmentShader =
                                 "#version 130 \n"
                                 "const vec2 size = vec2(2.0,0.0);"
                                 "const ivec3 off = ivec3(-1,0,1);"
                                 "uniform sampler2D normalMap;"
                                 "uniform sampler2D lightMap;"
                                 "uniform sampler2D specularTexture;"
                                 "uniform sampler2D bumpMap;"
                                 "uniform vec3 resolution;"
                                 "uniform vec4 lightColor;"
                                 "uniform vec4 lightPos;"
                                 "in mat4 projMat;"
                                 "mat4 inverse(mat4 m) {"
                                "     float"
                                "     a00 = m[0][0], a01 = m[0][1], a02 = m[0][2], a03 = m[0][3],"
                                "     a10 = m[1][0], a11 = m[1][1], a12 = m[1][2], a13 = m[1][3],"
                                "     a20 = m[2][0], a21 = m[2][1], a22 = m[2][2], a23 = m[2][3],"
                                "     a30 = m[3][0], a31 = m[3][1], a32 = m[3][2], a33 = m[3][3],"
                                "     b00 = a00 * a11 - a01 * a10,"
                                "     b01 = a00 * a12 - a02 * a10,"
                                "     b02 = a00 * a13 - a03 * a10,"
                                "     b03 = a01 * a12 - a02 * a11,"
                                "     b04 = a01 * a13 - a03 * a11,"
                                "     b05 = a02 * a13 - a03 * a12,"
                                "     b06 = a20 * a31 - a21 * a30,"
                                "     b07 = a20 * a32 - a22 * a30,"
                                "     b08 = a20 * a33 - a23 * a30,"
                                "     b09 = a21 * a32 - a22 * a31,"
                                "     b10 = a21 * a33 - a23 * a31,"
                                "     b11 = a22 * a33 - a23 * a32,"
                                "     det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;"
                                "     return mat4("
                                "                 a11 * b11 - a12 * b10 + a13 * b09,"
                                "                 a02 * b10 - a01 * b11 - a03 * b09,"
                                "                 a31 * b05 - a32 * b04 + a33 * b03,"
                                "                 a22 * b04 - a21 * b05 - a23 * b03,"
                                "                 a12 * b08 - a10 * b11 - a13 * b07,"
                                "                 a00 * b11 - a02 * b08 + a03 * b07,"
                                "                 a32 * b02 - a30 * b05 - a33 * b01,"
                                "                 a20 * b05 - a22 * b02 + a23 * b01,"
                                "                 a10 * b10 - a11 * b08 + a13 * b06,"
                                "                 a01 * b08 - a00 * b10 - a03 * b06,"
                                "                 a30 * b04 - a31 * b02 + a33 * b00,"
                                "                 a21 * b02 - a20 * b04 - a23 * b00,"
                                "                 a11 * b07 - a10 * b09 - a12 * b06,"
                                "                 a00 * b09 - a01 * b07 + a02 * b06,"
                                "                 a31 * b01 - a30 * b03 - a32 * b00,"
                                "                 a20 * b03 - a21 * b01 + a22 * b00) / det;"
                            "}"
                                 "void main () { "
                                     "vec2 position = (gl_FragCoord.xy / resolution.xy);"
                                     "vec4 normal = texture2D(normalMap, position);"
                                     "vec4 bump = texture2D(bumpMap, position);"
                                     "vec4 specularInfos = texture2D(specularTexture, position);"
                                     "vec3 sLightPos = vec3 (lightPos.x, lightPos.y, -lightPos.z * (gl_DepthRange.far - gl_DepthRange.near));"
                                     "float radius = lightPos.w;"
                                     "vec3 pixPos = vec3 (gl_FragCoord.x, gl_FragCoord.y, -normal.w * (gl_DepthRange.far - gl_DepthRange.near));"
                                     "vec4 lightMapColor = texture2D(lightMap, position);"
                                     "vec3 viewPos = vec3(resolution.x * 0.5f, resolution.y * 0.5f, 0);"
                                     "float z = (gl_FragCoord.w != 1.f) ? (inverse(projMat) * vec4(0, 0, 0, gl_FragCoord.w)).w : gl_FragCoord.z;"
                                     "vec3 vertexToLight = sLightPos - pixPos;"
                                     "if (bump.x != 0 || bump.y != 0 || bump.z != 0) {"
                                         "float s01 = textureOffset(normalMap, position, off.xy).z;"
                                         "float s21 = textureOffset(normalMap, position, off.zy).z;"
                                         "float s10 = textureOffset(normalMap, position, off.yx).z;"
                                         "float s12 = textureOffset(normalMap, position, off.yz).z;"
                                         "vec3 tmpNormal = (normal.xyz);"
                                         "vec3 tangeant = normalize (vec3(size.xy, s21 - s01));"
                                         "vec3 binomial = normalize (vec3(size.yx, s12 - s10));"
                                         "normal.x = dot(vertexToLight, tangeant);"
                                         "normal.y = dot(vertexToLight, binomial);"
                                         "normal.z = dot(vertexToLight, tmpNormal);"
                                         "normal.w = bump.w;"
                                     "}"
                                     "if (z >= normal.w) {"
                                         "vec4 specularColor = vec4(0, 0, 0, 0);"
                                         "float attenuation = 1.f - length(vertexToLight) / radius;"
                                         "vec3 pixToView = pixPos - viewPos;"
                                         "float normalLength = dot(normal.xyz, vertexToLight);"
                                         "vec3 lightReflect = vertexToLight + 2 * (normal.xyz * normalLength - vertexToLight);"
                                         "float m = specularInfos.r;"
                                         "float p = specularInfos.g;"
                                         "float specularFactor = dot(normalize(pixToView), normalize(lightReflect));"
                                         "specularFactor = pow (specularFactor, p);"
                                         "if (specularFactor > 0) {"
                                             "specularColor = vec4(lightColor.rgb, 1) * m * specularFactor;"
                                         "}"
                                         "if (normal.x != 0 || normal.y != 0 || normal.z != 0 && vertexToLight.z > 0.f) {"
                                             "vec3 dirToLight = normalize(vertexToLight.xyz);"
                                             "float nDotl = dot (dirToLight, normal.xyz);"
                                             "attenuation *= nDotl;"
                                         "}"
                                         "gl_FragColor = vec4(lightColor.xyz, 1) * max(0.0f, attenuation) + specularColor;"
                                     "} else {"
                                         "gl_FragColor = lightMapColor;"
                                     "}"
                                "}";
                                if (!depthBufferGenerator.loadFromMemory(vertexShader, depthGenFragShader))
                                    throw core::Erreur(50, "Failed to load depth buffer generator shader", 0);
                                if (!normalMapGenerator.loadFromMemory(buildNormalMapVertexShader, buildNormalMapFragmentShader))
                                    throw core::Erreur(51, "Failed to load normal generator shader", 0);
                                if (!specularTextureGenerator.loadFromMemory(vertexShader, specularGenFragShader))
                                    throw core::Erreur(52, "Failed to load specular texture generator shader", 0);
                                if (!bumpTextureGenerator.loadFromMemory(buildNormalMapVertexShader, bumpGenFragShader))
                                    throw core::Erreur(53, "Failed to load bump texture generator shader", 0);
                                if (!lightMapGenerator.loadFromMemory(vertexShader, perPixLightingFragmentShader))
                                    throw core::Erreur(54, "Failed to load light map generator shader", 0);
                                normalMapGenerator.setParameter("texture", Shader::CurrentTexture);
                                depthBufferGenerator.setParameter("texture", Shader::CurrentTexture);
                                specularTextureGenerator.setParameter("texture",Shader::CurrentTexture);
                                /*specularTextureGenerator.setParameter("maxM", Material::getMaxSpecularIntensity());
                                specularTextureGenerator.setParameter("maxP", Material::getMaxSpecularPower());*/
                                bumpTextureGenerator.setParameter("texture",Shader::CurrentTexture);
                                lightMapGenerator.setParameter("resolution", resolution.x, resolution.y, resolution.z);
                                lightMapGenerator.setParameter("normalMap", normalMap.getTexture());
                                lightMapGenerator.setParameter("specularTexture",specularTexture.getTexture());
                                lightMapGenerator.setParameter("bumpMap",bumpTexture.getTexture());
                                lightMapGenerator.setParameter("lightMap",lightMap.getTexture());
                            } else {
                                throw core::Erreur(55, "Shader not supported!", 0);
                            }
                        }
                }
                void LightRenderComponent::pushEvent(window::IEvent event, RenderWindow& rw) {
                    if (event.type == window::IEvent::WINDOW_EVENT && event.window.type == window::IEvent::WINDOW_EVENT_RESIZED && &getWindow() == &rw && isAutoResized()) {
                        std::cout<<"recompute size"<<std::endl;
                        recomputeSize();
                        getListener().pushEvent(event);
                        getView().reset(physic::BoundingBox(getView().getViewport().getPosition().x, getView().getViewport().getPosition().y, getView().getViewport().getPosition().z, event.window.data1, event.window.data2, getView().getViewport().getDepth()));
                    }
                }
                bool LightRenderComponent::needToUpdate() {
                return update;
            }
            void LightRenderComponent::loadTextureIndexes () {
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
            std::string LightRenderComponent::getExpression() {
                return expression;
            }
            void LightRenderComponent::clear() {
                 depthBuffer.clear(sf::Color::Transparent);
                 glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf));
                 glCheck(glBindTexture(GL_TEXTURE_2D, depthTex));
                 glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, view.getSize().x, view.getSize().y, GL_RGBA,
                 GL_FLOAT, NULL));
                 glCheck(glBindTexture(GL_TEXTURE_2D, 0));
                 glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
                 lightDepthBuffer.clear(sf::Color::Transparent);
                 glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf2));
                 glCheck(glBindTexture(GL_TEXTURE_2D, lightDepthTex));
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
                 glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf4));
                 glCheck(glBindTexture(GL_TEXTURE_2D, frameBufferTex));
                 glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, view.getSize().x, view.getSize().y, GL_RGBA,
                 GL_FLOAT, NULL));
                 glCheck(glBindTexture(GL_TEXTURE_2D, 0));
                 glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
                 normalMap.clear(sf::Color::Transparent);
                 specularTexture.clear(sf::Color::Transparent);
                 bumpTexture.clear(sf::Color::Transparent);
                 sf::Color ambientColor = g2d::AmbientLight::getAmbientLight().getColor();
                 lightMap.clear(ambientColor);
            }
            Sprite& LightRenderComponent::getNormalMapTile () {
                return normalMapTile;
            }
            Sprite& LightRenderComponent::getDepthBufferTile() {
                return depthBufferTile;
            }
            Sprite& LightRenderComponent::getspecularTile () {
                return specularBufferTile;
            }
            Sprite& LightRenderComponent::getBumpTile() {
                return bumpMapTile;
            }
            Sprite& LightRenderComponent::getLightTile() {
                return lightMapTile;
            }
            const Texture& LightRenderComponent::getDepthBufferTexture() {
                return depthBuffer.getTexture();
            }
            const Texture& LightRenderComponent::getnormalMapTexture() {
                return normalMap.getTexture();
            }
            const Texture& LightRenderComponent::getSpecularTexture() {
                return specularTexture.getTexture();
            }
            const Texture& LightRenderComponent::getbumpTexture() {
                return bumpTexture.getTexture();
            }
            const Texture& LightRenderComponent::getLightMapTexture() {
                return lightMap.getTexture();
            }
            bool LightRenderComponent::loadEntitiesOnComponent(ComponentMapping& componentMapping, std::vector<EntityId> vEntities)
            {
                batcher.clear();
                normalBatcher.clear();
                lightBatcher.clear();
                for (unsigned int i = 0; i < vEntities.size(); i++) {

                    if (componentMapping.getComponent<MeshComponent>(vEntities[i]) != nullptr) {
                        MeshComponent* mc = componentMapping.getComponent<MeshComponent>(vEntities[i]);
                        if (componentMapping.getComponent<EntityInfoComponent>(vEntities[i])->isLight) {
                            for (unsigned int j = 0; j <  mc->faces.size(); j++) {
                                lightBatcher.addFace(&mc->faces[j]);
                            }
                        } else {
                            for (unsigned int j = 0; j <  mc->faces.size(); j++) {
                                if (componentMapping.getComponent<EntityInfoComponent>(vEntities[i])->drawMode == DrawMode::INSTANCED)
                                    batcher.addFace(&mc->faces[j]);
                                else
                                    normalBatcher.addFace(&mc->faces[j]);
                            }
                        }
                    }
                }
                m_instances = batcher.getInstances();
                m_normals = normalBatcher.getInstances();
                m_light_instances = lightBatcher.getInstances();
                visibleEntities = vEntities;
                update = true;
                return true;
            }
            void LightRenderComponent::setView(View view){
                this->view = view;
                depthBuffer.setView(view);
                normalMap.setView(view);
                specularTexture.setView(view);
                bumpTexture.setView(view);
                lightMap.setView(view);
                lightDepthBuffer.setView(view);
            }
            void LightRenderComponent::setExpression(std::string expression) {
                update = true;
                this->expression = expression;
            }
            void LightRenderComponent::onVisibilityChanged (bool visible) {
                if (visible) {
                    glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                } else {
                    glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0));
                }
            }
            void LightRenderComponent::drawDepthLightInstances() {
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
                for (unsigned int i = 0; i < m_light_instances.size(); i++) {
                    if (m_light_instances[i].getAllVertices().getVertexCount() > 0) {
                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        //std::cout<<"instance : "<<i<<std::endl;
                        unsigned int p = m_light_instances[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = 0;
                        material.layer = m_light_instances[i].getMaterial().getLayer();
                        materials[p].push_back(material);
                        ModelData model;
                        TransformMatrix tm;
                        model.worldMat = tm.getMatrix().transpose();
                        matrices[p].push_back(model);
                        unsigned int vertexCount = 0;
                        for (unsigned int j = 0; j < m_light_instances[i].getAllVertices().getVertexCount(); j++) {
                            vertexCount++;
                            vbBindlessTex[p].append(m_light_instances[i].getAllVertices()[j]);
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
                RenderStates states;
                states.blendMode = sf::BlendNone;
                states.shader = &depthBufferGenerator;
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
                        lightDepthBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), states, vboIndirect);
                    }
                }
            }
            void LightRenderComponent::drawLightInstances() {
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
                for (unsigned int i = 0; i < m_light_instances.size(); i++) {
                    if (m_light_instances[i].getAllVertices().getVertexCount() > 0) {
                        //std::cout<<"instance : "<<i<<std::endl;
                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        unsigned int p = m_light_instances[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = 0;
                        material.layer = m_light_instances[i].getMaterial().getLayer();
                        material.lightCenter = m_light_instances[i].getMaterial().getLightCenter();
                        sf::Color c = m_light_instances[i].getMaterial().getLightColor();
                        material.lightColor = math::Vec3f(1.f / 255.f * c.r, 1.f / 255.f * c.g, 1.f / 255.f * c.b, 1.f / 255.f * c.a);
                        materials[p].push_back(material);
                        ModelData model;
                        TransformMatrix tm;
                        model.worldMat = tm.getMatrix().transpose();
                        matrices[p].push_back(model);
                        unsigned int vertexCount = 0;
                        for (unsigned int j = 0; j < m_light_instances[i].getAllVertices().getVertexCount(); j++) {
                            vertexCount++;
                            vbBindlessTex[p].append(m_light_instances[i].getAllVertices()[j]);
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
                RenderStates states;
                states.blendMode = sf::BlendAdd;
                states.shader = &lightMapGenerator;
                states.texture = nullptr;
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
                        lightMap.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), states, vboIndirect);
                    }
                }
                /*math::Matrix4f viewMatrix = view.getViewMatrix().getMatrix().transpose();
                math::Matrix4f projMatrix = view.getProjMatrix().getMatrix().transpose();
                debugShader.setParameter("projectionMatrix", projMatrix);
                debugShader.setParameter("viewMatrix", viewMatrix);
                glCheck(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
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
                debugShader.setParameter("worldMat", matrix);
                states.blendMode = sf::BlendNone;
                states.shader = &debugShader;
                lightMap.drawVertexBuffer(vb, states);
                glCheck(glFinish());*/
                lightMap.display();
            }
            void LightRenderComponent::drawInstances() {
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
                        ModelData model;
                        TransformMatrix tm;
                        model.worldMat = tm.getMatrix().transpose();
                        matrices[p].push_back(model);
                        unsigned int vertexCount = 0;
                        for (unsigned int j = 0; j < m_normals[i].getAllVertices().getVertexCount(); j++) {
                            vertexCount++;
                            vbBindlessTex[p].append(m_normals[i].getAllVertices()[j]);
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
                            matrices[p].push_back(model);
                        }
                        unsigned int vertexCount = 0;
                        if (m_instances[i].getVertexArrays().size() > 0) {
                            Entity* entity = m_instances[i].getVertexArrays()[0]->getEntity();
                            for (unsigned int j = 0; j < m_instances[i].getVertexArrays().size(); j++) {
                                if (entity == m_instances[i].getVertexArrays()[j]->getEntity()) {
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
                RenderStates states;
                states.blendMode = sf::BlendNone;
                states.shader = &depthBufferGenerator;
                states.texture = nullptr;
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
                        depthBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), states, vboIndirect);
                    }
                }
                states.shader = &buildAlphaBufferGenerator;
                glCheck(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
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
                        alphaBuffer.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), states, vboIndirect);
                        vbBindlessTex[p].clear();
                    }
                }
                glCheck(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));

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
                for (unsigned int i = 0; i < m_normals.size(); i++) {
                   if (m_normals[i].getAllVertices().getVertexCount() > 0) {
                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        unsigned int p = m_normals[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_normals[i].getMaterial().getBumpTexture() != nullptr) ? m_normals[i].getMaterial().getBumpTexture()->getNativeHandle() : 0;
                        material.layer = m_normals[i].getMaterial().getLayer();
                        materials[p].push_back(material);
                        ModelData model;
                        TransformMatrix tm;
                        model.worldMat = tm.getMatrix().transpose();
                        matrices[p].push_back(model);
                        unsigned int vertexCount = 0;
                        for (unsigned int j = 0; j < m_normals[i].getAllVertices().getVertexCount(); j++) {
                            vertexCount++;
                            vbBindlessTex[p].append(m_normals[i].getAllVertices()[j]);
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
                        material.textureIndex = (m_instances[i].getMaterial().getBumpTexture() != nullptr) ? m_instances[i].getMaterial().getBumpTexture()->getNativeHandle() : 0;
                        material.layer = m_instances[i].getMaterial().getLayer();
                        materials[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_instances[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            ModelData model;
                            model.worldMat = tm[j]->getMatrix().transpose();
                            matrices[p].push_back(model);
                        }
                        unsigned int vertexCount = 0;
                        if (m_instances[i].getVertexArrays().size() > 0) {
                            Entity* entity = m_instances[i].getVertexArrays()[0]->getEntity();
                            for (unsigned int j = 0; j < m_instances[i].getVertexArrays().size(); j++) {
                                if (entity == m_instances[i].getVertexArrays()[j]->getEntity()) {
                                    for (unsigned int k = 0; k < m_instances[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                        vertexCount++;
                                        vbBindlessTex[p].append((*m_instances[i].getVertexArrays()[j])[k], (m_instances[i].getMaterial().getBumpTexture() != nullptr) ? m_instances[i].getMaterial().getBumpTexture()->getId() : 0);
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
                states.blendMode = sf::BlendNone;
                states.shader = &bumpTextureGenerator;
                states.texture = nullptr;
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
                        bumpTexture.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), states, vboIndirect);
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
                for (unsigned int i = 0; i < m_normals.size(); i++) {
                   if (m_normals[i].getAllVertices().getVertexCount() > 0) {
                        DrawArraysIndirectCommand drawArraysIndirectCommand;
                        unsigned int p = m_normals[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_normals[i].getMaterial().getTexture() != nullptr) ? m_normals[i].getMaterial().getTexture()->getNativeHandle() : 0;
                        material.layer = m_normals[i].getMaterial().getLayer();
                        material.specularIntensity = m_normals[i].getMaterial().getSpecularIntensity();
                        material.specularPower = m_normals[i].getMaterial().getSpecularPower();
                        materials[p].push_back(material);
                        TransformMatrix tm;
                        ModelData model;
                        model.worldMat = tm.getMatrix().transpose();
                        matrices[p].push_back(model);
                        unsigned int vertexCount = 0;
                        for (unsigned int j = 0; j < m_normals[i].getAllVertices().getVertexCount(); j++) {
                            vertexCount++;
                            vbBindlessTex[p].append(m_normals[i].getAllVertices()[j]);
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
                    DrawArraysIndirectCommand drawArraysIndirectCommand;
                    if (m_instances[i].getAllVertices().getVertexCount() > 0) {
                        unsigned int p = m_instances[i].getAllVertices().getPrimitiveType();
                        MaterialData material;
                        material.textureIndex = (m_instances[i].getMaterial().getBumpTexture() != nullptr) ? m_instances[i].getMaterial().getBumpTexture()->getNativeHandle() : 0;
                        material.layer = m_instances[i].getMaterial().getLayer();
                        material.specularIntensity = m_instances[i].getMaterial().getSpecularIntensity();
                        material.specularPower = m_instances[i].getMaterial().getSpecularPower();
                        materials[p].push_back(material);
                        std::vector<TransformMatrix*> tm = m_instances[i].getTransforms();
                        for (unsigned int j = 0; j < tm.size(); j++) {
                            tm[j]->update();
                            ModelData model;
                            model.worldMat = tm[j]->getMatrix().transpose();
                            matrices[p].push_back(model);
                        }
                        unsigned int vertexCount = 0;
                        if (m_instances[i].getVertexArrays().size() > 0) {
                            Entity* entity = m_instances[i].getVertexArrays()[0]->getEntity();
                            for (unsigned int j = 0; j < m_instances[i].getVertexArrays().size(); j++) {
                                if (entity == m_instances[i].getVertexArrays()[j]->getEntity()) {
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
                states.blendMode = sf::BlendNone;
                states.shader = &specularTextureGenerator;
                states.texture = nullptr;
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
                        specularTexture.drawIndirect(vbBindlessTex[p], vbBindlessTex[p].getPrimitiveType(), drawArraysIndirectCommands[p].size(), states, vboIndirect);
                        vbBindlessTex[p].clear();
                    }
                }
                specularTexture.display();
                bumpTexture.display();
                depthBuffer.display();
            }
            void LightRenderComponent::drawNextFrame() {
                update = false;
                physic::BoundingBox viewArea = view.getViewVolume();
                math::Vec3f position (viewArea.getPosition().x,viewArea.getPosition().y, view.getPosition().z);
                math::Vec3f size (viewArea.getWidth(), viewArea.getHeight(), 0);

                if (lightMap.getSettings().versionMajor >= 3 && lightMap.getSettings().versionMinor >= 3) {
                        //glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                        if (!view.isOrtho())
                            view.setPerspective(80, view.getViewport().getSize().x / view.getViewport().getSize().y, 0.1, view.getViewport().getSize().z);
                        math::Matrix4f viewMatrix = view.getViewMatrix().getMatrix().transpose();
                        math::Matrix4f projMatrix = view.getProjMatrix().getMatrix().transpose();
                        depthBufferGenerator.setParameter("projectionMatrix", projMatrix);
                        depthBufferGenerator.setParameter("viewMatrix", viewMatrix);
                        specularTextureGenerator.setParameter("projectionMatrix", projMatrix);
                        specularTextureGenerator.setParameter("viewMatrix", viewMatrix);
                        bumpTextureGenerator.setParameter("projectionMatrix", projMatrix);
                        bumpTextureGenerator.setParameter("viewMatrix", viewMatrix);
                        buildAlphaBufferGenerator.setParameter("projectionMatrix", projMatrix);
                        buildAlphaBufferGenerator.setParameter("viewMatrix", viewMatrix);
                        /*for (unsigned int i = 0; i < m_light_instances.size(); i++) {
                            if (m_light_instances[i].getAllVertices().getVertexCount() > 0) {
                                vb.clear();
                                vb.setPrimitiveType( m_light_instances[i].getAllVertices().getPrimitiveType());
                                for (unsigned int n = 0; n < m_light_instances[i].getAllVertices().getVertexCount(); n++) {
                                    vb.append(m_light_instances[i].getAllVertices()[n]);
                                    vb.addLayer(m_light_instances[i].getMaterial().getLayer());
                                }
                                vb.update();
                                RenderStates states;
                                states.blendMode = sf::BlendNone;
                                states.shader = &depthBufferNormalGenerator;
                                lightDepthBuffer.drawVertexBuffer(vb, states);
                            }
                        }
                        lightDepthBuffer.display();*/
                        drawDepthLightInstances();
                        //drawNormals();
                        drawInstances();
                        lightMapGenerator.setParameter("projectionMatrix", projMatrix);
                        lightMapGenerator.setParameter("viewMatrix", viewMatrix);
                        lightMapGenerator.setParameter("viewportMatrix", lightMap.getViewportMatrix(&lightMap.getView()).getMatrix().transpose());


                        drawLightInstances();
                        /*states.shader = &normalMapGenerator;
                        VertexArray va = depthBufferTile.getVertexArray();
                        depthBufferTile.setCenter(view.getPosition());
                        vb.clear();
                        vb.setPrimitiveType(va.getPrimitiveType());
                        for (unsigned int n = 0; n < va.getVertexCount(); n++) {
                            vb.append(va[n]);
                        }
                        vb.update();
                        math::Matrix4f worldMatrix = depthBufferTile.getTransform().getMatrix().transpose();
                        math::Matrix4f texMatrix = depthBufferTile.getTexture()->getTextureMatrix();
                        normalMapGenerator.setParameter("projectionMatrix", projMatrix);
                        normalMapGenerator.setParameter("viewMatrix", viewMatrix);
                        normalMapGenerator.setParameter("textureMatrix", texMatrix);
                        normalMapGenerator.setParameter("worldMatrix", worldMatrix);
                        states.texture = depthBufferTile.getTexture();
                        normalMap.drawVertexBuffer(vb, states);
                        normalMap.display();*/
                        /*RenderStates states;
                        states.shader = &lightMapGenerator;
                        states.blendMode = sf::BlendAdd;
                        lightMapGenerator.setParameter("projectionMatrix", projMatrix);
                        lightMapGenerator.setParameter("viewMatrix", viewMatrix);
                        lightMapGenerator.setParameter("viewportMatrix", lightMap.getViewportMatrix(&lightMap.getView()).getMatrix().transpose());
                        for (unsigned int i = 0; i < m_light_instances.size(); i++) {
                            if (m_light_instances[i].getAllVertices().getVertexCount() > 0) {
                                for (unsigned int j = 0; j < m_light_instances[i].getVertexArrays().size(); j++) {
                                    vb.clear();
                                    vb.setPrimitiveType( m_light_instances[i].getVertexArrays()[j]->getPrimitiveType());
                                    for (unsigned int n = 0; n < m_light_instances[i].getVertexArrays()[j]->getVertexCount(); n++) {
                                        vb.append((*m_light_instances[i].getVertexArrays()[j])[n]);
                                        vb.addLayer(m_light_instances[i].getMaterial().getLayer());
                                    }
                                    vb.update();
                                    math::Matrix4f m = m_light_instances[i].getPerVaTransforms()[j]->getMatrix().transpose();
                                    lightMapGenerator.setParameter("worldMat", m);
                                    Entity* el = m_light_instances[i].getVertexArrays()[j]->getEntity();
                                    //std::cout<<"add light : "<<el<<std::endl;
                                    math::Vec3f center = getWindow().mapCoordsToPixel(el->getCenter() - el->getSize()*0.5f, view);
                                    std::cout<<"light center : "<<center<<std::endl;
                                    center.w = el->getSize().x * 0.5f;
                                    //std::cout<<"center : "<<center<<std::endl;
                                    /*lightMapGenerator.setParameter("lightPos", center.x, center.y, center.z, center.w);
                                    lightMapGenerator.setParameter("lightColor", el->getColor().r, el->getColor().g,el->getColor().b,el->getColor().a);
                                    lightMap.drawVertexBuffer(vb, states);
                                }
                            }
                        }
                        lightMap.display();*/
                        //glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0));
                }
            }
            std::vector<EntityId> LightRenderComponent::getEntities() {
                return visibleEntities;
            }
            void LightRenderComponent::draw(RenderTarget& target, RenderStates states) {
                lightMapTile.setCenter(target.getView().getPosition());
                states.blendMode = sf::BlendMultiply;
                target.draw(lightMapTile, states);
            }
            View& LightRenderComponent::getView() {
                return view;
            }
            int LightRenderComponent::getLayer() {
                return getPosition().z;
            }
            RenderTexture* LightRenderComponent::getFrameBuffer() {
                return &lightMap;
            }
            LightRenderComponent::~LightRenderComponent() {
                glDeleteBuffers(1, &vboWorldMatrices);
                glDeleteBuffers(1, &ubo);
            }
        }

    }
}
#endif // VULKAN

