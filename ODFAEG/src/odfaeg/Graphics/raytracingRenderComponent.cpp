#include "../../../include/odfaeg/Graphics/raytracingRenderComponent.hpp"
#ifndef VULKAN
#include "glCheck.h"
#endif
#include "../../../include/odfaeg/Math/triangle.h"

namespace odfaeg {
    namespace graphic {
        #ifdef VULKAN
        #else
        RaytracingRenderComponent::RaytracingRenderComponent(RenderWindow& window, int layer, std::string expression, window::ContextSettings settings) : HeavyComponent(window, math::Vec3f(window.getView().getPosition().x, window.getView().getPosition().y, layer),
                          math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0),
                          math::Vec3f(window.getView().getSize().x + window.getView().getSize().x * 0.5f, window.getView().getPosition().y + window.getView().getSize().y * 0.5f, layer)),
            view(window.getView()),
            expression(expression),
            quad(math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, window.getSize().y * 0.5f)),
            layer(layer) {
                quad.move(math::Vec3f(-window.getView().getSize().x * 0.5f, -window.getView().getSize().y * 0.5f, 0));
                frameBuffer.create(window.getView().getSize().x, window.getView().getSize().y, settings);
                frameBufferSprite = Sprite(frameBuffer.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), sf::IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));
                frameBuffer.setView(view);
                sf::Vector3i resolution ((int) window.getSize().x, (int) window.getSize().y, window.getView().getSize().z);
                GLuint maxNodes = 20 * window.getView().getSize().x * window.getView().getSize().y;
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
                glCheck(glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, window.getView().getSize().x, window.getView().getSize().y));
                glCheck(glBindImageTexture(0, headPtrTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI));
                glCheck(glBindTexture(GL_TEXTURE_2D, 0));
                std::vector<GLuint> headPtrClearBuf(window.getView().getSize().x*window.getView().getSize().y, 0xffffffff);
                glCheck(glGenBuffers(1, &clearBuf2));
                glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf2));
                glCheck(glBufferData(GL_PIXEL_UNPACK_BUFFER, headPtrClearBuf.size() * sizeof(GLuint),
                &headPtrClearBuf[0], GL_STATIC_COPY));
                glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
                glCheck(glGenTextures(1, &frameBufferTex));
                glCheck(glBindTexture(GL_TEXTURE_2D, frameBufferTex));
                glCheck(glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, window.getView().getSize().x, window.getView().getSize().y));
                glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
                glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
                glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
                glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
                glCheck(glBindImageTexture(1, frameBufferTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F));
                glCheck(glBindTexture(GL_TEXTURE_2D, 0));
                std::vector<GLfloat> texClearBuf(window.getView().getSize().x*window.getView().getSize().y*4, 0);
                glCheck(glGenBuffers(1, &clearBuf));
                glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf));
                glCheck(glBufferData(GL_PIXEL_UNPACK_BUFFER, texClearBuf.size() * sizeof(GLfloat),
                &texClearBuf[0], GL_STATIC_COPY));
                glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
                glCheck(glGenBuffers(1, &trianglesSSBO));
                glCheck(glGenBuffers(1, &lightsSSBO));
                external.setNativeHandle(frameBufferTex, window.getView().getSize().x, window.getView().getSize().y);
                core::FastDelegate<bool> signal (&RaytracingRenderComponent::needToUpdate, this);
                core::FastDelegate<void> slot (&RaytracingRenderComponent::drawNextFrame, this);
                core::Command cmd(signal, slot);
                getListener().connect("UPDATE", cmd);
                std::string quadVertexShader = R"(#version 460
                                                        layout (location = 0) in vec3 position;
                                                        layout (location = 1) in vec4 color;
                                                        layout (location = 2) in vec2 texCoords;
                                                        layout (location = 3) in vec3 normals;
                                                        uniform mat4 projectionMatrix;
                                                        uniform mat4 viewMatrix;
                                                        uniform mat4 worldMat;
                                                        uniform mat4 textureMatrix;
                                                        out vec2 fTexCoords;
                                                        out vec4 frontColor;
                                                        void main () {
                                                            gl_Position = projectionMatrix * viewMatrix * worldMat * vec4(position, 1.f);
                                                            fTexCoords = (textureMatrix * vec4(texCoords, 1.f, 1.f)).xy;
                                                            frontColor = color;
                                                        })";
                std::string quadFragmentShader = R"(#version 460
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
                                                        uniform sampler2D texture;
                                                        in vec4 frontColor;
                                                        in vec2 fTexCoords;
                                                        //layout(origin_upper_left) in vec4 gl_FragCoord;
                                                        layout (location = 0) out vec4 fcolor;
                                                        void main() {
                                                            /*vec4 texel = texture2D(texture, fTexCoords.xy) * frontColor;
                                                            fcolor = texel;*/
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
                                                              fcolor = color;
                                                        }
                                                    )";
                std::string raytracingShaderCode = R"(#version 460
                                                      #define MAX_FRAGMENTS 20
                                                      #extension GL_ARB_bindless_texture : enable
                                                      struct NodeType {
                                                          vec4 color;
                                                          float depth;
                                                          uint next;
                                                      };
                                                      struct Triangle {
                                                          mat4 transform;
                                                          mat4 textureMatrix;
                                                          vec4 position[3];
                                                          vec4 color[3];
                                                          vec4 texCoords[3];
                                                          vec4 normal;
                                                          uint textureIndex;
                                                          uint refractReflect;
                                                          float ratio;
                                                      };
                                                      struct Light {
                                                          vec4 center;
                                                          vec4 color;
                                                          float radius;
                                                      };
                                                      struct Pixel {
                                                          vec3 position;
                                                          vec4 color;
                                                          vec2 texCoords;
                                                      };
                                                      struct Ray {
                                                          vec3 origin;
                                                          vec3 dir;
                                                          vec3 ext;
                                                      };
                                                      layout(local_size_x = 1, local_size_y = 1) in;
                                                      layout(rgba32f, binding = 1) uniform image2D img_output;
                                                      layout(std140, binding = 0) uniform ALL_TEXTURES {
                                                          sampler2D textures[200];
                                                      };
                                                      layout(std430, binding = 1) buffer trianglesBuffer {
                                                          Triangle triangles[];
                                                      };
                                                      layout(std430, binding = 2) buffer lightsBuffer {
                                                          Light lights[];
                                                      };
                                                      layout(binding = 0, offset = 0) uniform atomic_uint nextNodeCounter;
                                                      layout(binding = 0, r32ui) uniform uimage2D headPointers;
                                                      layout(binding = 0, std430) buffer linkedLists {
                                                          NodeType nodes[];
                                                      };
                                                      uniform uint maxNodes;
                                                      uniform uint nbLights;
                                                      uniform uint nbTriangles;
                                                      uniform vec3 cameraPos;
                                                      uniform vec3 resolution;
                                                      uniform mat4 viewMatrix;
                                                      uniform mat4 projMatrix;
                                                      uniform mat4 viewportMatrix;
                                                      uniform uint tindex;
                                                      uniform uint x;
                                                      uniform uint y;
                                                      vec4 castReflectionRay(Pixel currentPixel, uint triangle);
                                                      vec4 castRefractionRay(Pixel currentPixel, uint triangle);
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
                                                      bool intersects(Ray ray, vec4[3] positions, inout vec4 intersection, inout float u, inout float v) {
                                                         /*vec3 r = ray.origin - ray.ext;
                                                         vec3 u = (positions[1] - positions[0]).xyz;
                                                         vec3 v = (positions[2] - positions[0]).xyz;
                                                         vec3 n = cross(u, v);
                                                         vec3 p1 = positions[0].xyz;
                                                         vec3 otr = ray.origin - p1;
                                                         vec3 point;
                                                         if (dot(n, r) == 0)
                                                            return false;
                                                         point.x = dot(n, otr) / dot(n, r);
                                                         point.y = dot(cross(-otr, u), r) / dot(n, r);
                                                         point.z = dot(cross(-v, otr), r) / dot(n, r);

                                                         if (0 <= point.x
                                                               && 0 <= point.y && point.y <= 1
                                                               &&  0 <= point.z && point.z <= 1
                                                               && point.y + point.z <= 1) {
                                                               intersection.x = p1.x + u.x * point.z + v.x * point.y;
                                                               intersection.y = p1.y + u.y * point.z + v.y * point.y;
                                                               intersection.z = p1.z + u.z * point.z + v.z * point.y;


                                                               return true;
                                                         }

                                                         intersection = vec4(0, 0, 0);



                                                         return false;*/
                                                         /*intersection = vec4(0, 0, 0, 0);
                                                         vec3 v0v1 = (positions[1] - positions[0]).xyz;
                                                         vec3 v0v2 = (positions[2] - positions[0]).xyz;
                                                         vec3 v0 = positions[0].xyz;
                                                         float wv0 = positions[0].w;
                                                         vec3 v1 = positions[1].xyz;
                                                         float wv1 = positions[1].w;
                                                         vec3 v2 = positions[2].xyz;
                                                         float wv2 = positions[2].w;
                                                         vec3 n = cross(v0v1, v0v2);
                                                         float denom = dot(n, n);
                                                         float ndotRayDirection = dot(n, ray.dir);
                                                         if (abs(ndotRayDirection) < 0.0000001)
                                                            return false;
                                                         float d = dot(n, v0);
                                                         float t = (dot(n, ray.origin) + d) / ndotRayDirection;
                                                         if (t < 0)
                                                            return false;
                                                         vec3 p = ray.origin + t * ray.dir;


                                                         vec3 c;
                                                         vec3 edge0 = v1 - v0;
                                                         vec3 vp0 = p - v0;
                                                         c = cross(edge0, vp0);



                                                         if (dot(n, c) < 0)
                                                            return false;
                                                         vec3 edge1 = v2 - v1;
                                                         vec3 vp1 = p - v1;
                                                         c = cross(edge1, vp1);

                                                         if ((u = dot(n, c)) < 0)
                                                            return false;
                                                         vec3 edge2 = v0 - v2;
                                                         vec3 vp2 = p - v2;
                                                         c = cross(edge2, vp2);
                                                         if ((v = dot(n, c)) < 0)
                                                            return false;
                                                        u /= denom;
                                                        v /= denom;

                                                        intersection = vec4(p, 1);
                                                        intersection.w = u * wv0 + v * wv1 + (1-u-v) * wv2;
                                                        return true;*/
                                                        //Möller Trumbore.
                                                        vec3 v0v1 = (positions[1] - positions[0]).xyz;
                                                        vec3 v0v2 = (positions[2] - positions[0]).xyz;
                                                        vec3 v0 = positions[0].xyz;
                                                        float wv0 = positions[0].w;
                                                        vec3 v1 = positions[1].xyz;
                                                        float wv1 = positions[1].w;
                                                        vec3 v2 = positions[2].xyz;
                                                        float wv2 = positions[2].w;
                                                        vec3 pvec = cross(ray.dir, v0v2);
                                                        float det = dot(v0v1, pvec);
                                                        if (abs(det) < 0.000001) return false;
                                                        float invDet = 1 / det;

                                                        vec3 tvec = ray.origin - v0;
                                                        u = dot(tvec, pvec) * invDet;
                                                        if (u < 0 || u > 1) return false;

                                                        vec3 qvec = cross(tvec, v0v1);
                                                        v = dot(ray.dir, qvec) * invDet;
                                                        if (v < 0 || u + v > 1) return false;

                                                        float t = dot(v0v2, qvec) * invDet;

                                                        intersection = vec4(ray.origin + t * ray.dir, u * wv0 + v * wv1 + (1-u-v) * wv2);
                                                        return true;
                                                      }
                                                      bool intersects (Ray ray, Light light) {
                                                          vec3 omc = ray.origin - light.center.xyz;
                                                          float b = dot(ray.dir, omc);
                                                          float c = dot(omc, omc) - light.radius * light.radius;
                                                          float bsqmc = b * b - c;
                                                          if (bsqmc >= 0)
                                                            return true;
                                                          return false;
                                                      }
                                                      /* fix: because of layout std140 16byte alignment, get uvec2 from array of uvec4 */
                                                      /*uvec2 GetTexture(uint index)
                                                      {
                                                          uint index_corrected = index / 2;
                                                          if (index % 2 == 0)
                                                              return maps[index_corrected].xy;
                                                          return maps[index_corrected].zw;
                                                      }*/
                                                      Pixel interpolate(Triangle triangle, vec3 p, float u, float v) {
                                                           Pixel pixel;
                                                           /*vec3 p1 = triangle.position[0].xyz;
                                                           vec3 p2 = triangle.position[1].xyz;
                                                           vec3 p3 = triangle.position[2].xyz;
                                                           float d = (p2.y - p3.y) * (p.x - p3.x) + (p3.x - p2.x) * (p.y - p3.y);
                                                           float u = ((p2.y - p3.y) * (p.x - p3.x) + (p3.x - p2.x) * (p.y - p3.y)) / d;
                                                           float v = ((p3.y - p1.y) * (p.x - p3.x) + (p1.x - p3.x) * (p.y - p3.y)) / d;*/
                                                           float w = 1 - u - v;
                                                           pixel.position = p;
                                                           vec3 r = vec3(triangle.color[0].r, triangle.color[1].r, triangle.color[2].r);
                                                           vec3 g = vec3(triangle.color[0].g, triangle.color[1].g, triangle.color[2].g);
                                                           vec3 b = vec3(triangle.color[0].b, triangle.color[1].b, triangle.color[2].b);
                                                           vec3 a = vec3(triangle.color[0].a, triangle.color[1].a, triangle.color[2].a);
                                                           vec4 color = vec4 (r.x * u + r.y * v + r.z * w,
                                                                            g.x * u + g.y * v + g.z * w,
                                                                            b.x * u + b.y * v + b.z * w,
                                                                            a.x * u + a.y * v + a.z * w);
                                                           vec2 tc1 = (triangle.textureMatrix * triangle.texCoords[0]).xy;
                                                           vec2 tc2 = (triangle.textureMatrix * triangle.texCoords[1]).xy;
                                                           vec2 tc3 = (triangle.textureMatrix * triangle.texCoords[2]).xy;)" R"(pixel.texCoords = vec2(tc1.x * u + tc2.x * v + tc3.x * w,
                                                                                  tc1.y * u + tc2.y * v + tc3.y * w);
                                                           if (pixel.texCoords.x < 0)
                                                               pixel.texCoords.x = 0;
                                                           else if (pixel.texCoords.x > 1)
                                                               pixel.texCoords.x = fract(pixel.texCoords.x);
                                                           if (pixel.texCoords.y < 0)
                                                               pixel.texCoords.y = 0;
                                                           else if (pixel.texCoords.y > 1)
                                                               pixel.texCoords.y = fract(pixel.texCoords.y);
                                                           //sampler2D tex = sampler2D(GetTexture(triangle.textureIndex-1));
                                                           pixel.color = (triangle.textureIndex > 0) ? color * texture(textures[triangle.textureIndex-1], pixel.texCoords) : color;
                                                           return pixel;
                                                      }
                                                      vec4 computeLightAndShadow(Pixel currentPixel, vec3 n, uint triangle) {
                                                           vec4 lightColor = vec4(1, 1, 1, 1);
                                                           int count = 0;
                                                           bool isInShadow=false;
                                                           float maxAlpha = 0; )"\
                                                           R"(Pixel pixels[MAX_FRAGMENTS];
                                                           vec3 i = currentPixel.position - cameraPos;
                                                           vec3 dir = i + dot (i, n) * 2;
                                                           Ray toLight;
                                                           toLight.origin = currentPixel.position;
                                                           toLight.dir = dir;
                                                           vec4 visibility=vec4(1, 1, 1, 1);
                                                           float u, v;
                                                           for (uint l = 0; l < nbLights; l++) {
                                                                if (intersects(toLight, lights[l])) {
                                                                    vec3 vertexToLight = lights[l].center.xyz - currentPixel.position;
                                                                    float attenuation = 1.f - length(vertexToLight) / lights[l].radius;
                                                                    vec3 dirToLight = normalize(vertexToLight.xyz);
                                                                    float nDotl = dot (dirToLight, n);
                                                                    attenuation *= nDotl;
                                                                    lightColor *= lights[l].color * max(0.0, attenuation);
                                                                    Ray shadowRay;
                                                                    shadowRay.origin = lights[l].center.xyz;
                                                                    shadowRay.dir = -toLight.dir;
                                                                    vec4 shadowRayIntersection;
                                                                    for (uint i = 0; i < nbTriangles; i++) {
                                                                        if (i != triangle) {
                                                                            /*for (uint j = 0; j < 3; j++) {
                                                                                triangles[i].position[j] = (viewportMatrix * projMatrix * viewMatrix * triangles[i].transform * vec4(triangles[i].position[j], 1)).xyz;
                                                                            }*/
                                                                            if (intersects(shadowRay, triangles[i].position, shadowRayIntersection, u, v)) {
                                                                                pixels[count] = interpolate(triangles[i], shadowRayIntersection.xyz, u, v);
                                                                                //count++;
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                                for (uint p = 0; p < count; p++) {
                                                                    if (pixels[p].position.z > currentPixel.position.z) {
                                                                        isInShadow = true;
                                                                        if (pixels[p].color.a > maxAlpha);
                                                                            maxAlpha = pixels[p].color.a;

                                                                    }
                                                                }
                                                           }
                                                           if (isInShadow) {
                                                               visibility = vec4(0.5, 0.5, 0.5, 1-maxAlpha);
                                                           }
                                                           return visibility * lightColor;
                                                      }
                                                      vec4 blendAlpha(Pixel depthPixels[MAX_FRAGMENTS], int count) {
                                                          //merge sort
                                                          int i, j1, j2, k;
                                                          int a, b, c;
                                                          int step = 1;
                                                          Pixel leftArray[MAX_FRAGMENTS/2]; //for merge sort
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
                                                                      leftArray[k] = depthPixels[a + k];
                                                                  j1 = 0;
                                                                  j2 = 0;
                                                                  for (k = a; k < c; k++)
                                                                  {
                                                                      if (b + j1 >= c || (j2 < step && leftArray[j2].position.z > depthPixels[b + j1].position.z))
                                                                          depthPixels[k] = leftArray[j2++];
                                                                      else
                                                                          depthPixels[k] = depthPixels[b + j1++];
                                                                  }
                                                                  ////////////////////////////////////////////////////////////////////////
                                                                  i += 2 * step;
                                                              }
                                                              step *= 2;
                                                          }
                                                          vec4 color = vec4(0, 0, 0, 0);
                                                          for( int i = count - 1; i >= 0; i--)
                                                          {
                                                            color.rgb = depthPixels[i].color.rgb * depthPixels[i].color.a + color.rgb * (1 - depthPixels[i].color.a);
                                                            color.a = depthPixels[i].color.a + color.a * (1 - depthPixels[i].color.a);
                                                          }
                                                          return color;
                                                      }
                                                      vec4 castRefractionRay(Pixel currentPixel, uint triangle) {
                                                          vec4 color;
                                                          vec3 I = currentPixel.position - cameraPos;
                                                          int count = 0;
                                                          Pixel depthPixels[MAX_FRAGMENTS];
                                                          float u, v;
                                                          for (unsigned int i = 0; i < nbTriangles; i++) {
                                                                if (i != triangle) {
                                                                    /*for (uint j = 0; j < 3; j++) {
                                                                        triangles[i].position[j] = (viewportMatrix * projMatrix * viewMatrix * triangles[i].transform * vec4(triangles[i].position[j], 1)).xyz;
                                                                    }*/
                                                                    vec3 n = triangles[i].normal.xyz;
                                                                    float k = 1.0 - triangles[i].ratio * triangles[i].ratio * (1.0 - dot(n, I) * dot(n, I));
                                                                    vec3 r;
                                                                    if (k < 0.0) {
                                                                        r = vec3(0, 0, 0);
                                                                    } else {
                                                                        r = triangles[i].ratio * I - (triangles[i].ratio * dot(n, I) + sqrt(k)) * n;
                                                                    }
                                                                    Ray ray;
                                                                    ray.origin = currentPixel.position;
                                                                    ray.dir = r;
                                                                    vec4 intersection;
                                                                    if (intersects(ray, triangles[i].position,intersection, u, v)) {
                                                                        vec4 intersection;
                                                                        vec4 reflectRefractColor = vec4(1, 1, 1, 1);
                                                                        vec4 lightShadowColor = vec4(1, 1, 1, 1);
                                                                        Pixel p;
                                                                        if (intersects(ray, triangles[i].position,intersection, u , v)) {
                                                                            p = interpolate(triangles[i], intersection.xyz, u, v);
                                                                            lightShadowColor = computeLightAndShadow(p, triangles[i].normal.xyz, i);
                                                                            /*if (triangles[i].refractReflect == 1) {
                                                                                reflectRefractColor = castReflectionRay(p, i);
                                                                            }
                                                                            if (triangles[i].refractReflect == 2) {
                                                                                reflectRefractColor = castRefractionRay(p, i);
                                                                            }
                                                                            if (triangles[i].refractReflect == 3) {
                                                                                vec4 reflectColor = castReflectionRay(p, i);
                                                                                vec4 refractColor = castRefractionRay(p, i);
                                                                                color = reflectColor * refractColor;
                                                                            }*/
                                                                        }
                                                                        p.color *= lightShadowColor;
                                                                        depthPixels[count] = p;
                                                                        count++;
                                                                    }
                                                               }
                                                          }
                                                          color = blendAlpha(depthPixels, count);
                                                          return color;
                                                      }
                                                      vec4 castReflectionRay(Pixel currentPixel, uint triangle) {
                                                          vec4 color;
                                                          vec3 I = currentPixel.position - cameraPos;
                                                          int count = 0;
                                                          Pixel depthPixels[MAX_FRAGMENTS];
                                                          float u, v;

                                                          for (unsigned int i = 0; i < nbTriangles; i++) {
                                                                if (i != triangle) {
                                                                    /*for (uint j = 0; j < 3; j++) {
                                                                        triangles[i].position[j] = (viewportMatrix * projMatrix * viewMatrix * triangles[i].transform * vec4(triangles[i].position[j], 1)).xyz;
                                                                    }*/
                                                                    vec3 n = triangles[i].normal.xyz;
                                                                    vec3 r = I - 2.0 * dot(I, n) * n;
                                                                    Ray ray;
                                                                    ray.origin = currentPixel.position;
                                                                    ray.dir = r;
                                                                    vec4 intersection;
                                                                    vec4 lightShadowColor = vec4(1, 1, 1, 1);
                                                                    vec4 reflectRefractColor = vec4(1, 1, 1, 1);
                                                                    Pixel p;
                                                                    if (intersects(ray, triangles[i].position,intersection, u, v)) {
                                                                        p = interpolate(triangles[i], intersection.xyz, u, v);
                                                                        //lightShadowColor = computeLightAndShadow(p, triangles[i].normal, i);
                                                                        /*if (triangles[i].refractReflect == 1) {
                                                                            reflectRefractColor = castReflectionRay(p, i);
                                                                        }
                                                                        if (triangles[i].refractReflect == 2) {
                                                                            reflectRefractColor = castRefractionRay(p, i);
                                                                        }
                                                                        if (triangles[i].refractReflect == 3) {
                                                                            vec4 reflectColor = castReflectionRay(p, i);)"\
                                                                            R"(vec4 refractColor = castRefractionRay(p, i);"
                                                                            reflectRefractColor = reflectColor * refractColor;
                                                                        }*/
                                                                    }
                                                                    p.color *= lightShadowColor;
                                                                    //depthPixels[count] = p;
                                                                    count++;
                                                                }
                                                          }
                                                          color = blendAlpha(depthPixels, count);
                                                          return color;
                                                      }
                                                      void main() {
                                                          vec4 pixel = vec4(1.0, 0.0, 0.0, 1.0);
                                                          ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
                                                          Ray ray;
                                                          ray.origin = vec3(pixel_coords.xy, 0);
                                                          ray.ext = vec3 (pixel_coords.xy, 1);
                                                          ray.dir = ray.ext - ray.origin;
                                                          Pixel depthPixels[MAX_FRAGMENTS];
                                                          Pixel currentPixel;
                                                          int count = 0;

                                                           Triangle tri = triangles[gl_GlobalInvocationID.z];
                                                           for (uint j = 0; j < 3; j++) {
                                                                vec4 t = projMatrix * viewMatrix * triangles[gl_GlobalInvocationID.z].transform * triangles[gl_GlobalInvocationID.z].position[j];
                                                                float tmp = t.w;
                                                                if (t.w != 0) {
                                                                    t /= t.w;
                                                                }
                                                                tri.position[j] = viewportMatrix * t;
                                                                tri.position[j].w = tmp;
                                                           }

                                                           vec4 intersection;
                                                           float u, v;
                                                           if (intersects(ray, tri.position, intersection, u, v)
                                                               && intersection.x >= 0 && intersection.x < resolution.x
                                                               && intersection.y >= 0 && intersection.y < resolution.y
                                                               && intersection.w >= 50) {

                                                               currentPixel = interpolate(tri, intersection.xyz, u, v);
                                                               /*vec4 shadowLightColor = computeLightAndShadow (currentPixel, triangles[i].normal, i);
                                                               vec4 reflectRefractColor=vec4(1, 1, 1, 1);
                                                               if (triangles[i].refractReflect == 1) {
                                                                   reflectRefractColor = castReflectionRay(currentPixel, i);
                                                               }
                                                               if (triangles[i].refractReflect == 2) {
                                                                   reflectRefractColor = castRefractionRay(currentPixel, i);
                                                               }
                                                               if (triangles[i].refractReflect == 3) {
                                                                   vec4 reflectColor = castReflectionRay(currentPixel, i);
                                                                   vec4 refractColor = castRefractionRay(currentPixel, i);
                                                                   reflectRefractColor = reflectColor * refractColor;
                                                               }
                                                               currentPixel.color = currentPixel.color * reflectRefractColor * shadowLightColor;
                                                               depthPixels[count] = currentPixel;
                                                               if (count < MAX_FRAGMENTS)
                                                                   count++;
                                                               vec4 color = blendAlpha (depthPixels, count);*/
                                                               uint nodeIdx = atomicCounterIncrement(nextNodeCounter);
                                                               if (nodeIdx < maxNodes) {
                                                                   uint prevHead = imageAtomicExchange(headPointers, pixel_coords, nodeIdx);
                                                                   nodes[nodeIdx].color = currentPixel.color;
                                                                   nodes[nodeIdx].depth = -intersection.w;
                                                                   nodes[nodeIdx].next = prevHead;
                                                               }
                                                               //imageStore(img_output, pixel_coords, currentPixel.color);
                                                           }
                                                      }
                                                      )";
                    if (!rayComputeShader.loadFromMemory(raytracingShaderCode, Shader::Compute)) {
                        throw core::Erreur(60, "failed to load raytracing compute shader!", 3);
                    }
                    if (!quadShader.loadFromMemory(quadVertexShader, quadFragmentShader)) {
                        throw core::Erreur(61, "failed to load quad shader!", 3);
                    }
                    rayComputeShader.setParameter("maxNodes", maxNodes);
                    rayComputeShader.setParameter("resolution", resolution.x, resolution.y, resolution.z);
                    math::Matrix4f viewMatrix = window.getDefaultView().getViewMatrix().getMatrix().transpose();
                    math::Matrix4f projMatrix = window.getDefaultView().getProjMatrix().getMatrix().transpose();
                    quadShader.setParameter("viewMatrix", viewMatrix);
                    quadShader.setParameter("projectionMatrix", projMatrix);
                    quadShader.setParameter("worldMat", quad.getTransform().getMatrix().transpose());
                    quadShader.setParameter("texture", Shader::CurrentTexture);
                    quadShader.setParameter("textureMatrix", external.getTextureMatrix());

                    std::vector<Texture*> allTextures = Texture::getAllTextures();
                    Samplers allSamplers{};
                    for (unsigned int i = 0; i < allTextures.size(); i++) {
                        GLuint64 handle_texture = glGetTextureHandleARB(allTextures[i]->getNativeHandle());
                        glMakeTextureHandleResidentARB(handle_texture);
                        allSamplers.tex[i].handle = handle_texture;
                    }
                    glCheck(glGenBuffers(1, &ubo));
                    unsigned int ubid;
                    glCheck(ubid = glGetUniformBlockIndex(rayComputeShader.getHandle(), "ALL_TEXTURES"));
                    glCheck(glUniformBlockBinding(rayComputeShader.getHandle(),    ubid, 0));
                    backgroundColor = sf::Color::Transparent;
                    glCheck(glBindBuffer(GL_UNIFORM_BUFFER, ubo));
                    glCheck(glBufferData(GL_UNIFORM_BUFFER, sizeof(allSamplers),allSamplers.tex, GL_STATIC_DRAW));
                    glCheck(glBindBuffer(GL_UNIFORM_BUFFER, 0));
                    glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                    glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicBuffer));
                    glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, linkedListBuffer));
                    glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, trianglesSSBO));
                    glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, lightsSSBO));
            }
            void RaytracingRenderComponent::setBackgroundColor(sf::Color color) {
                backgroundColor = color;
            }
            void RaytracingRenderComponent::clear() {
                frameBuffer.setActive();
                frameBuffer.clear(sf::Color::Transparent);
                glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf));
                glCheck(glBindTexture(GL_TEXTURE_2D, frameBufferTex));
                glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, view.getSize().x, view.getSize().y, GL_RGBA,
                GL_FLOAT, NULL));
                glCheck(glBindTexture(GL_TEXTURE_2D, 0));
                glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
                GLuint zero = 0;
                glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicBuffer));
                glCheck(glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero));
                glCheck(glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0));
                glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf2));
                glCheck(glBindTexture(GL_TEXTURE_2D, headPtrTex));
                glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, view.getSize().x, view.getSize().y, GL_RED_INTEGER,
                GL_UNSIGNED_INT, NULL));
                glCheck(glBindTexture(GL_TEXTURE_2D, 0));
                glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
                frameBuffer.resetGLStates();
            }
            void RaytracingRenderComponent::drawNextFrame() {
                if (frameBuffer.getSettings().versionMajor >= 4 && frameBuffer.getSettings().versionMinor >= 6) {
                    /*float max_x = view.getSize().x*0.5;
                    float max_y = view.getSize().y*0.5;
                    math::Vec3f dims = view.getSize();
                    bool intersects = false;
                    for (unsigned int i = 0; i < dims.x; i++) {
                        for (unsigned int j = 0; j < dims.y; j++) {

                            math::Vec3f origin (i, j, 0);
                            math::Vec3f ext(i, j, dims.z*0.5);
                            math::Ray ray(origin, ext);

                            for (unsigned int t = 0; t < triangles.size(); t++) {
                                math::Vec3f p1 (triangles[t].positions[0].x, triangles[t].positions[0].y,triangles[t].positions[0].z);
                                math::Vec3f p2 (triangles[t].positions[1].x, triangles[t].positions[1].y,triangles[t].positions[1].z);
                                math::Vec3f p3 (triangles[t].positions[2].x, triangles[t].positions[2].y,triangles[t].positions[2].z);
                                math::Vec3f tp1, tp2, tp3;

                                p1 = triangles[t].transform.transpose() * p1;
                                p2 = triangles[t].transform.transpose() * p2;
                                p3 = triangles[t].transform.transpose() * p3;

                                tp1 = p1;
                                tp2 = p2;
                                tp3 = p3;
                                //std::cout<<"triangle : "<<triangles[t].positions[0]<<triangles[t].positions[1]<<triangles[t].positions[2]<<std::endl;
                                p1 = frameBuffer.mapCoordsToPixel(math::Vec3f(p1.x, p1.y, p1.z));
                                p2 = frameBuffer.mapCoordsToPixel(math::Vec3f(p2.x, p2.y, p2.z));
                                p3 = frameBuffer.mapCoordsToPixel(math::Vec3f(p3.x, p3.y, p3.z));


                                tp1 = view.getViewMatrix().transform(tp1);
                                tp2 = view.getViewMatrix().transform(tp2);
                                tp3 = view.getViewMatrix().transform(tp3);

                                tp1 = view.getProjMatrix().project(tp1);
                                tp2 = view.getProjMatrix().project(tp2);
                                tp3 = view.getProjMatrix().project(tp3);

                                if (tp1.w == 0)
                                    tp1.w = dims.z * 0.5;
                                if (tp2.w == 0)
                                    tp2.w = dims.z * 0.5;
                                if (tp3.w == 0)
                                    tp3.w = dims.z * 0.5;
                                float tmp1 = tp1.w;
                                tp1 = tp1.normalizeToVec3();
                                float tmp2 = tp2.w;
                                tp2 = tp2.normalizeToVec3();
                                float tmp3 = tp3.w;
                                tp3 = tp3.normalizeToVec3();


                                p1.w = tmp1;
                                p2.w = tmp2;
                                p3.w = tmp3;

                                math::Triangle tri(p1, p2, p3);
                                math::Vec3f i1, i2;
                                if(tri.intersectsWhere(ray, i1, i2)
                                   && i1.x >= 0 && i1.x < dims.x
                                   && i1.y >= 0 && i1.y < dims.y
                                   && i1.w > 50) {

                                    intersects = true;
                                    rayComputeShader.setParameter("x", i);
                                    rayComputeShader.setParameter("y", j);
                                    rayComputeShader.setParameter("tindex", t);
                                    break;
                                }



                            }
                            if (intersects)
                                break;

                        }
                        if (intersects)
                            break;
                    }*/
                    glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, trianglesSSBO));
                    glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, triangles.size() * sizeof(Triangle), NULL, GL_DYNAMIC_COPY));
                    GLvoid* p = nullptr;
                    glCheck(p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY));
                    memcpy(p, triangles.data(), triangles.size() * sizeof(Triangle));



                    glCheck(glUnmapBuffer(GL_SHADER_STORAGE_BUFFER));
                    glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightsSSBO));
                    glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, lights.size() * sizeof(Light), NULL, GL_DYNAMIC_DRAW));
                    GLvoid* p2 = nullptr;
                    glCheck(p2 = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY));
                    memcpy(p2, lights.data(), lights.size() * sizeof(Light));
                    glCheck(glUnmapBuffer(GL_SHADER_STORAGE_BUFFER));
                    glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
                    math::Matrix4f viewMatrix = view.getViewMatrix().getMatrix().transpose();
                    math::Matrix4f projMatrix = view.getProjMatrix().getMatrix().transpose();
                    ViewportMatrix vpm;
                    vpm.setViewport(math::Vec3f(view.getViewport().getPosition().x, view.getViewport().getPosition().y, 0),
                    math::Vec3f(view.getViewport().getWidth(), view.getViewport().getHeight(), 1));
                    math::Matrix4f viewportMatrix = vpm.getMatrix().transpose();
                    rayComputeShader.setParameter("viewMatrix", viewMatrix);
                    rayComputeShader.setParameter("projMatrix", projMatrix);
                    rayComputeShader.setParameter("viewportMatrix", viewportMatrix);
                    unsigned int nbTriangles = triangles.size();
                    unsigned int nbLights = lights.size();
                    rayComputeShader.setParameter("nbTriangles", nbTriangles);
                    rayComputeShader.setParameter("nbLights", nbLights);
                    Shader::bind(&rayComputeShader);
                    glCheck(glDispatchCompute(view.getSize().x, view.getSize().y, triangles.size()));
                    glCheck(glFinish());
                    // make sure writing to image has finished before read
                    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

                    RenderStates states;
                    states.shader = &quadShader;
                    states.transform = quad.getTransform();
                    states.texture = &external;
                    vb.clear();
                    vb.setPrimitiveType(sf::Quads);
                    Vertex v1 (sf::Vector3f(0, 0, quad.getSize().z), sf::Color::White,sf::Vector2f(0, 0));
                    Vertex v2 (sf::Vector3f(quad.getSize().x,0, quad.getSize().z), sf::Color::White,sf::Vector2f(quad.getSize().x, 0));
                    Vertex v3 (sf::Vector3f(quad.getSize().x, quad.getSize().y, quad.getSize().z),sf::Color::White,sf::Vector2f(quad.getSize().x, quad.getSize().y));
                    Vertex v4 (sf::Vector3f(0, quad.getSize().y, quad.getSize().z), sf::Color::White,sf::Vector2f(0, quad.getSize().y));
                    vb.append(v1);
                    vb.append(v2);
                    vb.append(v3);
                    vb.append(v4);
                    vb.update();
                    frameBuffer.drawVertexBuffer(vb, states);
                    glCheck(glFinish());
                    frameBuffer.display();
                }
            }
            bool RaytracingRenderComponent::loadEntitiesOnComponent(std::vector<Entity*> vEntities) {
                //std::cout<<"load entities on component"<<std::endl;
                triangles.clear();
                lights.clear();
                Light ambientLight;
                g2d::AmbientLight al = g2d::AmbientLight::getAmbientLight();
                ambientLight.center = math::Vec3f(al.getLightCenter().x, al.getLightCenter().y, al.getLightCenter().z);
                ambientLight.radius = 10000;
                ambientLight.color = math::Vec3f(al.getColor().r/255.f, al.getColor().g/255.f, al.getColor().b/255.f, al.getColor().a/255.f);
                lights.push_back(ambientLight);
                unsigned int nbVertices =0;
                for (unsigned int e = 0; e < vEntities.size(); e++) {
                    if ( vEntities[e]->isLeaf()) {
                        if (!vEntities[e]->isLight()) {
                            for (unsigned int j = 0; j <  vEntities[e]->getNbFaces(); j++) {
                                Material material = vEntities[e]->getFace(j)->getMaterial();
                                VertexArray va = vEntities[e]->getFace(j)->getVertexArray();
                                unsigned int size = 0;
                                if (va.getPrimitiveType() == sf::PrimitiveType::Quads) {
                                    size = va.getVertexCount() / 4;
                                } else if (va.getPrimitiveType() == sf::PrimitiveType::Triangles) {
                                    size = va.getVertexCount() / 3;
                                } else if (va.getPrimitiveType() == sf::PrimitiveType::TriangleStrip || va.getPrimitiveType() == sf::PrimitiveType::TriangleFan) {
                                    size = va.getVertexCount() - 2;
                                }
                                for (unsigned int i = 0; i < size; i++) {
                                    if (va.getPrimitiveType() == sf::PrimitiveType::Quads) {
                                        for (unsigned int n = 0; n < 2; n++) {
                                            if (n == 0) {
                                                Triangle triangle;
                                                triangle.positions[0] = math::Vec3f(va[i*4].position.x,va[i*4].position.y,va[i*4].position.z);
                                                triangle.positions[1] = math::Vec3f(va[i*4+1].position.x,va[i*4+1].position.y,va[i*4+1].position.z);
                                                triangle.positions[2] = math::Vec3f(va[i*4+2].position.x,va[i*4+2].position.y,va[i*4+2].position.z);
                                                triangle.colours[0] = math::Vec3f(va[i*4].color.r / 255.f,va[i*4].color.g / 255.f,va[i*4].color.b / 255.f, va[i*4].color.a / 255.f);
                                                triangle.colours[1] = math::Vec3f(va[i*4+1].color.r / 255.f,va[i*4+1].color.g / 255.f,va[i*4+1].color.b / 255.f, va[i*4+1].color.a / 255.f);
                                                triangle.colours[2] = math::Vec3f(va[i*4+2].color.r / 255.f,va[i*4+2].color.g / 255.f,va[i*4+2].color.b / 255.f, va[i*4+2].color.a / 255.f);
                                                triangle.texCoords[0] = math::Vec3f(va[i*4].texCoords.x, va[i*4].texCoords.y, 0, 0);
                                                triangle.texCoords[1] = math::Vec3f(va[i*4+1].texCoords.x, va[i*4+1].texCoords.y, 0, 0);
                                                triangle.texCoords[2] = math::Vec3f(va[i*4+2].texCoords.x, va[i*4+2].texCoords.y, 0, 0);


                                                math::Vec3f v1(va[i*4].position.x, va[i*4].position.y, va[i*4].position.z);
                                                math::Vec3f v2(va[i*4+1].position.x, va[i*4+1].position.y, va[i*4+1].position.z);
                                                math::Vec3f v3(va[i*4+2].position.x, va[i*4+2].position.y, va[i*4+2].position.z);
                                                math::Vec3f d1 = v2 - v1;
                                                math::Vec3f d2 = v3 - v1;
                                                math::Vec3f n = d1.cross(d2).normalize();
                                                triangle.normal = math::Vec3f(n.x, n.y, n.z);
                                                triangle.ratio = material.getRefractionFactor();
                                                math::Matrix4f m;
                                                triangle.textureIndex = (material.getTexture() == nullptr) ? 0 : material.getTexture()->getNativeHandle();
                                                //std::cout<<"texture index : "<<triangle.textureIndex<<std::endl;
                                                triangle.textureMatrix = (material.getTexture() == nullptr) ? m : material.getTexture()->getTextureMatrix();
                                                triangle.transform = vEntities[e]->getTransform().getMatrix().transpose();
                                                if (!material.isReflectable() && !material.isRefractable())
                                                    triangle.refractReflect = 0;
                                                else if (material.isReflectable() && !material.isRefractable())
                                                    triangle.refractReflect = 1;
                                                else if (!material.isReflectable() && material.isRefractable())
                                                    triangle.refractReflect = 2;
                                                else
                                                    triangle.refractReflect = 3;
                                                triangles.push_back(triangle);
                                            } else {
                                                Triangle triangle;
                                                triangle.positions[0] = math::Vec3f(va[i*4].position.x,va[i*4].position.y,va[i*4].position.z);
                                                triangle.positions[1] = math::Vec3f(va[i*4+2].position.x,va[i*4+2].position.y,va[i*4+2].position.z);
                                                triangle.positions[2] =  math::Vec3f(va[i*4+3].position.x,va[i*4+3].position.y,va[i*4+3].position.z);
                                                triangle.colours[0] = math::Vec3f(va[i*4].color.r / 255.f,va[i*4].color.g / 255.f,va[i*4].color.b / 255.f, va[i*4].color.a / 255.f);
                                                triangle.colours[1] = math::Vec3f(va[i*4+2].color.r / 255.f,va[i*4+2].color.g / 255.f,va[i*4+2].color.b / 255.f, va[i*4+2].color.a / 255.f);
                                                triangle.colours[2] = math::Vec3f(va[i*4+3].color.r / 255.f,va[i*4+3].color.g / 255.f,va[i*4+3].color.b / 255.f, va[i*4+3].color.a / 255.f);
                                                triangle.texCoords[0] = math::Vec3f(va[i*4].texCoords.x, va[i*4].texCoords.y, 0, 0);
                                                triangle.texCoords[1] = math::Vec3f(va[i*4+2].texCoords.x, va[i*4+2].texCoords.y, 0, 0);
                                                triangle.texCoords[2] = math::Vec3f(va[i*4+3].texCoords.x, va[i*4+3].texCoords.y, 0, 0);
                                                /*for (unsigned int v = 0; v < 3; v++) {
                                                    std::cout<<va[i*4+v+1].position.x<<","<<va[i*4+v+1].position.y<<","<<va[i*4+v+1].position.z<<std::endl;
                                                }*/
                                                math::Vec3f v1(va[i*4].position.x, va[i*4].position.y, va[i*4].position.z);
                                                math::Vec3f v2(va[i*4+2].position.x, va[i*4+2].position.y, va[i*4+2].position.z);
                                                math::Vec3f v3(va[i*4+3].position.x, va[i*4+3].position.y, va[i*4+3].position.z);
                                                math::Vec3f d1 = v2 - v1;
                                                math::Vec3f d2 = v3 - v1;
                                                math::Vec3f n = d1.cross(d2).normalize();
                                                triangle.normal = math::Vec3f(n.x, n.y, n.z);
                                                triangle.ratio = material.getRefractionFactor();
                                                math::Matrix4f m;
                                                triangle.textureIndex = (material.getTexture() == nullptr) ? 0 : material.getTexture()->getNativeHandle();
                                                triangle.textureMatrix = (material.getTexture() == nullptr) ? m : material.getTexture()->getTextureMatrix();
                                                triangle.transform = vEntities[e]->getTransform().getMatrix().transpose();
                                                if (!material.isReflectable() && !material.isRefractable())
                                                    triangle.refractReflect = 0;
                                                else if (material.isReflectable() && !material.isRefractable())
                                                    triangle.refractReflect = 1;
                                                else if (!material.isReflectable() && material.isRefractable())
                                                    triangle.refractReflect = 2;
                                                else
                                                    triangle.refractReflect = 3;
                                                triangles.push_back(triangle);
                                            }
                                        }
                                    } else if (va.getPrimitiveType() == sf::PrimitiveType::Triangles) {
                                        Triangle triangle;
                                        triangle.positions[0] = math::Vec3f(va[i*3].position.x,va[i*3].position.y,va[i*3].position.z);
                                        triangle.positions[1] = math::Vec3f(va[i*3+1].position.x,va[i*3+1].position.y,va[i*3+1].position.z);
                                        triangle.positions[2] = math::Vec3f(va[i*3+2].position.x,va[i*3+2].position.y,va[i*3+2].position.z);
                                        triangle.colours[0] = math::Vec3f(va[i*3].color.r / 255.f,va[i*3].color.g / 255.f,va[i*3].color.b / 255.f, va[i*3].color.a / 255.f);
                                        triangle.colours[1] = math::Vec3f(va[i*3+1].color.r / 255.f,va[i*3+1].color.g / 255.f,va[i*3+1].color.b / 255.f, va[i*3+1].color.a / 255.f);
                                        triangle.colours[2] = math::Vec3f(va[i*3+2].color.r / 255.f,va[i*3+2].color.g / 255.f,va[i*3+2].color.b / 255.f, va[i*3+2].color.a / 255.f);
                                        triangle.texCoords[0] = math::Vec3f(va[i*3].texCoords.x, va[i*3].texCoords.y, 0, 0);
                                        triangle.texCoords[1] = math::Vec3f(va[i*3+1].texCoords.x, va[i*3+2].texCoords.y, 0, 0);
                                        triangle.texCoords[2] = math::Vec3f(va[i*3+2].texCoords.x, va[i*3+2].texCoords.y, 0, 0);

                                        math::Vec3f v1(va[i*3].position.x, va[i*3].position.y, va[i*3].position.z);
                                        math::Vec3f v2(va[i*3+1].position.x, va[i*3+1].position.y, va[i*3+1].position.z);
                                        math::Vec3f v3(va[i*3+2].position.x, va[i*3+2].position.y, va[i*3+2].position.z);
                                        math::Vec3f d1 = v2 - v1;
                                        math::Vec3f d2 = v3 - v1;
                                        math::Vec3f n = d1.cross(d2).normalize();
                                        triangle.normal = math::Vec3f(n.x, n.y, n.z);
                                        triangle.ratio = material.getRefractionFactor();
                                        math::Matrix4f m;
                                        triangle.textureIndex = (material.getTexture() == nullptr) ? 0 : material.getTexture()->getNativeHandle();
                                        triangle.textureMatrix = (material.getTexture() == nullptr) ? m : material.getTexture()->getTextureMatrix();
                                        triangle.transform = vEntities[e]->getTransform().getMatrix().transpose();
                                        if (!material.isReflectable() && !material.isRefractable())
                                            triangle.refractReflect = 0;
                                        else if (material.isReflectable() && !material.isRefractable())
                                            triangle.refractReflect = 1;
                                        else if (!material.isReflectable() && material.isRefractable())
                                            triangle.refractReflect = 2;
                                        else
                                            triangle.refractReflect = 3;
                                        triangles.push_back(triangle);
                                    } else if (va.getPrimitiveType() == sf::PrimitiveType::TriangleStrip) {
                                        if (i == 0) {
                                            Triangle triangle;
                                            triangle.positions[0] = math::Vec3f(va[i*3].position.x,va[i*3].position.y,va[i*3].position.z);
                                            triangle.positions[1] = math::Vec3f(va[i*3+1].position.x,va[i*3+1].position.y,va[i*3+1].position.z);
                                            triangle.positions[2] = math::Vec3f(va[i*3+2].position.x,va[i*3+2].position.y,va[i*3+2].position.z);
                                            triangle.colours[0] = math::Vec3f(va[i*3].color.r / 255.f,va[i*3].color.g / 255.f,va[i*3].color.b / 255.f, va[i*3].color.a / 255.f);
                                            triangle.colours[1] = math::Vec3f(va[i*3+1].color.r / 255.f,va[i*3+1].color.g / 255.f,va[i*3+1].color.b / 255.f, va[i*3+1].color.a / 255.f);
                                            triangle.colours[2] = math::Vec3f(va[i*3+2].color.r / 255.f,va[i*3+2].color.g / 255.f,va[i*3+2].color.b / 255.f, va[i*3+2].color.a / 255.f);
                                            triangle.texCoords[0] = math::Vec3f(va[i*3].texCoords.x, va[i*3].texCoords.y, 0, 0);
                                            triangle.texCoords[1] = math::Vec3f(va[i*3+1].texCoords.x, va[i*3+1].texCoords.y, 0, 0);
                                            triangle.texCoords[2] = math::Vec3f(va[i*3+2].texCoords.x, va[i*3+2].texCoords.y, 0, 0);

                                            math::Vec3f v1(va[i*3].position.x, va[i*3].position.y, va[i*3].position.z);
                                            math::Vec3f v2(va[i*3+1].position.x, va[i*3+1].position.y, va[i*3+1].position.z);
                                            math::Vec3f v3(va[i*3+2].position.x, va[i*3+2].position.y, va[i*3+2].position.z);
                                            math::Vec3f d1 = v2 - v1;
                                            math::Vec3f d2 = v3 - v1;
                                            math::Vec3f n = d1.cross(d2).normalize();
                                            triangle.normal = math::Vec3f(n.x, n.y, n.z);
                                            triangle.ratio = material.getRefractionFactor();
                                            math::Matrix4f m;
                                            triangle.textureIndex = (material.getTexture() == nullptr) ? 0 : material.getTexture()->getNativeHandle();
                                            triangle.textureMatrix = (material.getTexture() == nullptr) ? m : material.getTexture()->getTextureMatrix();
                                            triangle.transform = vEntities[e]->getTransform().getMatrix().transpose();
                                            if (!material.isReflectable() && !material.isRefractable())
                                                triangle.refractReflect = 0;
                                            else if (material.isReflectable() && !material.isRefractable())
                                                triangle.refractReflect = 1;
                                            else if (!material.isReflectable() && material.isRefractable())
                                                triangle.refractReflect = 2;
                                            else
                                                triangle.refractReflect = 3;
                                            triangles.push_back(triangle);
                                        } else {
                                            Triangle triangle;
                                            triangle.positions[0] = math::Vec3f(va[i].position.x,va[i].position.y,va[i].position.z);;
                                            triangle.positions[1] = math::Vec3f(va[i+1].position.x,va[i+1].position.y,va[i+1].position.z);
                                            triangle.positions[2] = math::Vec3f(va[i+2].position.x,va[i+2].position.y,va[i+2].position.z);
                                            triangle.colours[0] = math::Vec3f(va[i].color.r / 255.f,va[i].color.g / 255.f,va[i].color.b / 255.f, va[i].color.a / 255.f);
                                            triangle.colours[1] = math::Vec3f(va[i+1].color.r / 255.f,va[i+1].color.g / 255.f,va[i+1].color.b / 255.f, va[i+1].color.a / 255.f);
                                            triangle.colours[2] = math::Vec3f(va[i+2].color.r / 255.f,va[i+2].color.g / 255.f,va[i+2].color.b / 255.f, va[i+2].color.a / 255.f);
                                            triangle.texCoords[0] = math::Vec3f(va[i].texCoords.x, va[i].texCoords.y, 0, 0);
                                            triangle.texCoords[1] = math::Vec3f(va[i+1].texCoords.x, va[i+1].texCoords.y, 0, 0);
                                            triangle.texCoords[2] = math::Vec3f(va[i+2].texCoords.x, va[i+2].texCoords.y, 0, 0);

                                            math::Vec3f v1(va[i].position.x, va[i].position.y, va[i].position.z);
                                            math::Vec3f v2(va[i+1].position.x, va[i+1].position.y, va[i+1].position.z);
                                            math::Vec3f v3(va[i+2].position.x, va[i+2].position.y, va[i+2].position.z);
                                            math::Vec3f d1 = v2 - v1;
                                            math::Vec3f d2 = v3 - v1;
                                            math::Vec3f n = d1.cross(d2).normalize();
                                            triangle.normal = math::Vec3f(n.x, n.y, n.z);
                                            triangle.ratio = material.getRefractionFactor();
                                            math::Matrix4f m;
                                            triangle.textureIndex = (material.getTexture() == nullptr) ? 0 : material.getTexture()->getNativeHandle();
                                            triangle.textureMatrix = (material.getTexture() == nullptr) ? m : material.getTexture()->getTextureMatrix();
                                            triangle.transform = vEntities[e]->getTransform().getMatrix().transpose();
                                            if (!material.isReflectable() && !material.isRefractable())
                                                triangle.refractReflect = 0;
                                            else if (material.isReflectable() && !material.isRefractable())
                                                triangle.refractReflect = 1;
                                            else if (!material.isReflectable() && material.isRefractable())
                                                triangle.refractReflect = 2;
                                            else
                                                triangle.refractReflect = 3;
                                            triangles.push_back(triangle);
                                        }
                                    } else if (va.getPrimitiveType() == sf::TriangleFan) {
                                        if (i == 0) {
                                            Triangle triangle;
                                            triangle.positions[0] = math::Vec3f(va[i*3].position.x,va[i*3].position.y,va[i*3].position.z);;
                                            triangle.positions[1] = math::Vec3f(va[i*3+1].position.x,va[i*3+1].position.y,va[i*3+1].position.z);;
                                            triangle.positions[2] = math::Vec3f(va[i*3+2].position.x,va[i*3+2].position.y,va[i*3+2].position.z);;
                                            triangle.colours[0] = math::Vec3f(va[i*3].color.r / 255.f,va[i*3].color.g / 255.f,va[i*3].color.b / 255.f, va[i*3].color.a / 255.f);
                                            triangle.colours[1] = math::Vec3f(va[i*3+1].color.r / 255.f,va[i*3+1].color.g / 255.f,va[i*3+1].color.b / 255.f, va[i*3+1].color.a / 255.f);
                                            triangle.colours[2] = math::Vec3f(va[i*3+2].color.r / 255.f,va[i*3+2].color.g / 255.f,va[i*3+2].color.b / 255.f, va[i*3+2].color.a / 255.f);
                                            triangle.texCoords[0] = math::Vec3f(va[i*3].texCoords.x, va[i*3].texCoords.y, 0, 0);
                                            triangle.texCoords[1] = math::Vec3f(va[i*3+1].texCoords.x, va[i*3+1].texCoords.y, 0, 0);
                                            triangle.texCoords[2] = math::Vec3f(va[i*3+2].texCoords.x, va[i*3+2].texCoords.y, 0, 0);

                                            math::Vec3f v1(va[i*3].position.x, va[i*3].position.y, va[i*3].position.z);
                                            math::Vec3f v2(va[i*3+1].position.x, va[i*3+1].position.y, va[i*3+1].position.z);
                                            math::Vec3f v3(va[i*3+2].position.x, va[i*3+2].position.y, va[i*3+2].position.z);
                                            math::Vec3f d1 = v2 - v1;
                                            math::Vec3f d2 = v3 - v1;
                                            math::Vec3f n = d1.cross(d2).normalize();
                                            triangle.normal = math::Vec3f(n.x, n.y, n.z);
                                            triangle.ratio = material.getRefractionFactor();
                                            math::Matrix4f m;
                                            triangle.textureIndex = (material.getTexture() == nullptr) ? 0 : material.getTexture()->getNativeHandle();
                                            triangle.textureMatrix = (material.getTexture() == nullptr) ? m : material.getTexture()->getTextureMatrix();
                                            triangle.transform = vEntities[e]->getTransform().getMatrix().transpose();
                                            if (!material.isReflectable() && !material.isRefractable())
                                                triangle.refractReflect = 0;
                                            else if (material.isReflectable() && !material.isRefractable())
                                                triangle.refractReflect = 1;
                                            else if (!material.isReflectable() && material.isRefractable())
                                                triangle.refractReflect = 2;
                                            else
                                                triangle.refractReflect = 3;
                                            triangles.push_back(triangle);
                                        } else {
                                            Triangle triangle;
                                            triangle.positions[0] = math::Vec3f(va[0].position.x,va[0].position.y,va[0].position.z);;
                                            triangle.positions[1] = math::Vec3f(va[i+1].position.x,va[i+1].position.y,va[i+1].position.z);;
                                            triangle.positions[2] = math::Vec3f(va[i+2].position.x,va[i+2].position.y,va[i+2].position.z);
                                            triangle.colours[0] = math::Vec3f(va[0].color.r / 255.f,va[0].color.g / 255.f,va[0].color.b / 255.f, va[0].color.a / 255.f);
                                            triangle.colours[1] = math::Vec3f(va[i+1].color.r / 255.f,va[i+1].color.g / 255.f,va[i+1].color.b / 255.f, va[i+1].color.a / 255.f);
                                            triangle.colours[2] = math::Vec3f(va[i+2].color.r / 255.f,va[i+2].color.g / 255.f,va[i+2].color.b / 255.f, va[i+2].color.a / 255.f);
                                            triangle.texCoords[0] = math::Vec3f(va[0].texCoords.x, va[0].texCoords.y, 0, 0);
                                            triangle.texCoords[1] = math::Vec3f(va[i+1].texCoords.x, va[i+1].texCoords.y, 0, 0);
                                            triangle.texCoords[2] = math::Vec3f(va[i+2].texCoords.x, va[i+2].texCoords.y, 0, 0);

                                            math::Vec3f v1(va[0].position.x, va[0].position.y, va[0].position.z);
                                            math::Vec3f v2(va[i+1].position.x, va[i+1].position.y, va[i+1].position.z);
                                            math::Vec3f v3(va[i+2].position.x, va[i+2].position.y, va[i+2].position.z);
                                            math::Vec3f d1 = v2 - v1;
                                            math::Vec3f d2 = v3 - v1;
                                            math::Vec3f n = d1.cross(d2).normalize();
                                            triangle.normal = math::Vec3f(n.x, n.y, n.z);
                                            triangle.ratio = material.getRefractionFactor();
                                            math::Matrix4f m;
                                            triangle.textureIndex = (material.getTexture() == nullptr) ? 0 : material.getTexture()->getNativeHandle();
                                            triangle.textureMatrix = (material.getTexture() == nullptr) ? m : material.getTexture()->getTextureMatrix();
                                            triangle.transform = vEntities[e]->getTransform().getMatrix().transpose();
                                            if (!material.isReflectable() && !material.isRefractable())
                                                triangle.refractReflect = 0;
                                            else if (material.isReflectable() && !material.isRefractable())
                                                triangle.refractReflect = 1;
                                            else if (!material.isReflectable() && material.isRefractable())
                                                triangle.refractReflect = 2;
                                            else
                                                triangle.refractReflect = 3;
                                            triangles.push_back(triangle);
                                        }
                                    } else {
                                        Light light;
                                        g2d::PonctualLight* pl = static_cast<g2d::PonctualLight*>(vEntities[e]);
                                        light.center = math::Vec3f(pl->getCenter().x, pl->getCenter().y, pl->getCenter().z);
                                        light.radius = pl->getSize().y * 0.5f;
                                        light.color = math::Vec3f(pl->getColor().r / 255.f,pl->getColor().g/255.f,pl->getColor().b/255.f,pl->getColor().a/255.f);
                                        lights.push_back(light);
                                    }
                                }
                            }
                        }
                    }
                }
                update = true;
                return true;
            }
            void RaytracingRenderComponent::draw(RenderTarget& target, RenderStates states) {
                frameBufferSprite.setCenter(target.getView().getPosition());
                /*states.texture = &external;
                states.transform = quad.getTransform();
                View view = target.getView();
                target.setView(target.getDefaultView());*/
                target.draw(frameBufferSprite, states);
                //target.setView(view);
            }
            bool RaytracingRenderComponent::needToUpdate() {
                return update;
            }
            void RaytracingRenderComponent::setExpression (std::string expression) {
                this->expression = expression;
            }
            void RaytracingRenderComponent::draw(Drawable& drawable, RenderStates states) {
                //drawables.insert(std::make_pair(drawable, states));
            }
            void RaytracingRenderComponent::setView(View view) {
                frameBuffer.setView(view);
                sf::Vector3i resolution ((int) view.getSize().x, (int) view.getSize().y, view.getSize().z);
                rayComputeShader.setParameter("resolution", resolution.x, resolution.y, resolution.z);
                this->view = view;
            }
            std::vector<Entity*> RaytracingRenderComponent::getEntities() {
                return visibleEntities;
            }
            std::string RaytracingRenderComponent::getExpression() {
                return expression;
            }
            View& RaytracingRenderComponent::getView() {
                return view;
            }
            void RaytracingRenderComponent::pushEvent(window::IEvent event, RenderWindow& rw) {
                if (event.type == window::IEvent::WINDOW_EVENT && event.window.type == window::IEvent::WINDOW_EVENT_RESIZED && &getWindow() == &rw && isAutoResized()) {
                    std::cout<<"recompute size"<<std::endl;
                    recomputeSize();
                    getListener().pushEvent(event);
                    getView().reset(physic::BoundingBox(getView().getViewport().getPosition().x, getView().getViewport().getPosition().y, getView().getViewport().getPosition().z, event.window.data1, event.window.data2, getView().getViewport().getDepth()));
                }
            }
            const Texture& RaytracingRenderComponent::getFrameBufferTexture() {
                return frameBuffer.getTexture();
            }
            RaytracingRenderComponent::~RaytracingRenderComponent() {
                glDeleteBuffers(1, &trianglesSSBO);
                glDeleteBuffers(1, &lightsSSBO);
                glDeleteBuffers(1, &clearBuf);
                glDeleteTextures(1, &frameBufferTex);
                glDeleteBuffers(1, &atomicBuffer);
                glDeleteBuffers(1, &linkedListBuffer);
                glDeleteBuffers(1, &clearBuf2);
                glDeleteTextures(1, &headPtrTex);
            }
            #endif
    }
}
