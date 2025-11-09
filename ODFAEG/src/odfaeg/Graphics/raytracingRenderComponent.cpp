#include "../../../include/odfaeg/Graphics/raytracingRenderComponent.hpp"
#ifndef VULKAN
#include "glCheck.h"
#endif
#include "../../../include/odfaeg/Math/triangle.h"

namespace odfaeg {
    namespace graphic {
        #ifdef VULKAN
        RaytracingRenderComponent::RaytracingRenderComponent (RenderWindow& window, int layer, std::string expression, window::ContextSettings settings) : HeavyComponent(window, math::Vec3f(window.getView().getPosition().x(), window.getView().getPosition().y(), layer),
                          math::Vec3f(window.getView().getSize().x(), window.getView().getSize().y(), 0),
                          math::Vec3f(window.getView().getSize().x() + window.getView().getSize().x() * 0.5f, window.getView().getPosition().y() + window.getView().getSize().y() * 0.5f, layer)),
            view(window.getView()),
            expression(expression),
            vkDevice(window.getDevice()),
            layer(layer),
            window(window),
            raytracingShader(window.getDevice()) {

            window::Device::QueueFamilyIndices queueFamilyIndices = vkDevice.findQueueFamilies(vkDevice.getPhysicalDevice(), VK_NULL_HANDLE);

            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
            poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optionel
            if (vkCreateCommandPool(vkDevice.getDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
                throw core::Erreur(0, "échec de la création d'une command pool!", 1);
            }
            rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
            VkPhysicalDeviceProperties2 deviceProperties2{};
            deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
            deviceProperties2.pNext = &rayTracingPipelineProperties;
            vkGetPhysicalDeviceProperties2(vkDevice.getPhysicalDevice(), &deviceProperties2);

            // Get acceleration structure properties, which will be used later on in the sample
            accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
            VkPhysicalDeviceFeatures2 deviceFeatures2{};
            deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            deviceFeatures2.pNext = &accelerationStructureFeatures;
            vkGetPhysicalDeviceFeatures2(vkDevice.getPhysicalDevice(), &deviceFeatures2);
            core::FastDelegate<bool> signal (&RaytracingRenderComponent::needToUpdate, this);
            core::FastDelegate<void> slot (&RaytracingRenderComponent::drawNextFrame, this);
            core::Command cmd(signal, slot);
            getListener().connect("UPDATE", cmd);
            compileShaders();
            createStorageImage();
            createUniformBuffers();
            createRayTracingPipeline();
            createShaderBindingTable();
            createDescriptorPool();
            createDescriptorSetLayout();
            allocateDescriptorSets();
            update = true;
            needToUpdateDS = false;
            datasReady = false;
        }
        void RaytracingRenderComponent::compileShaders() {
            const std::string raygen = R"(#version 460
                                          #extension GL_EXT_ray_tracing : enable
                                          layout(binding = 0, set = 0) uniform accelerationStructureEXT topLevelAS;
                                          layout(binding = 1, set = 0, rgba8) uniform image2D image;
                                          layout(binding = 2, set = 0) uniform CameraProperties
                                          {
                                              mat4 viewInverse;
                                              mat4 projInverse;
                                          } cam;
                                          // Dans le shader commun
                                          struct RayPayload {
                                              vec4 color;
                                          };
                                          layout(location = 0) rayPayloadEXT RayPayload payload; // ray generation
                                          void main() {
                                                const vec2 pixelCenter = vec2(gl_LaunchIDEXT.xy) + vec2(0.5);
                                                const vec2 inUV = pixelCenter/vec2(gl_LaunchSizeEXT.xy);
                                                vec2 d = inUV * 2.0 - 1.0;

                                                vec4 origin = cam.viewInverse * vec4(0,0,0,1);
                                                vec4 target = cam.projInverse * vec4(d.x, d.y, 1, 1) ;
                                                vec4 direction = cam.viewInverse*vec4(normalize(target.xyz), 0) ;

                                                float tmin = 0.001;
                                                float tmax = 10000.0;

                                                traceRayEXT(topLevelAS, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, origin.xyz, tmin, direction.xyz, tmax, 0);

                                                imageStore(image, ivec2(gl_LaunchIDEXT.xy), payload.color);
                                          }
                                          )";
            const std::string rayhit = R"(#version 460
                                          #version 460
                                          #extension GL_EXT_ray_tracing : enable
                                          #extension GL_EXT_nonuniform_qualifier : enable
                                          struct Vertex {
                                              vec3 position;
                                              vec4 color;
                                              vec2 texCoords;
                                              vec3 normal;
                                          };
                                          struct GeometryOffset {
                                              uint vertexOffset;
                                              uint indexOffset;
                                          };
                                          struct MaterialData {
                                              mat4 textureMatrix;
                                              uint textureIndex;
                                          };
                                          layout (location = 3) uniform sampler2D textures[];
                                          layout (location = 4) uniform buffer vertexBuffer {
                                              Vertex vertices[];
                                          };
                                          layout (location = 5) uniform buffer geomBuffer {
                                              GeometryOffset geomOffsets[];
                                          };
                                          layout (location = 6) uniform buffer indexBuffer {
                                              uint indexes[];
                                          };
                                          layout (location = 7) uniform buffer materialBuffer {
                                              uint materials[];
                                          };
                                          struct RayPayload {
                                              vec4 color;
                                          };
                                          layout(location = 0) rayPayloadInEXT RayPayload payload;
                                          hitAttributeEXT vec2 baryCoords;
                                          void main() {
                                              GeometryOffset geomOffs = geomOffsets[gl_InstanceCustomIndexEXT];
                                              int primID = gl_PrimitiveID;
                                              uint i1 = indexes[geomOffs.indexOffset + primId * 3 + 0];
                                              uint i2 = indexes[geomOffs.indexOffset + primId * 3 + 1];
                                              uint i3 = indexes[geomOffs.indexOffset + primId * 3 + 2];
                                              vec4 c1 = vertices[geomOffs.vertexOffset + i1].color;
                                              vec4 c2 = vertices[geomOffs.vertexOffset + i2].color;
                                              vec4 c3 = vertices[geomOffs.vertexOffset + i3].color;
                                              mat4 textureMatrix = materials[gl_InstanceCustomIndexEXT].textureMatrix;
                                              vec2 ct1 = (vec4(vertices[geomOffs.vertexOffset + i1].texCoords.xy, 0, 1) * textureMatrix).xy;
                                              vec2 ct2 = (vec4(vertices[geomOffs.vertexOffset + i2].texCoords.xy, 0, 1) * textureMatrix).xy;
                                              vec2 ct3 = (vec4(vertices[geomOffs.vertexOffset + i3].texCoords.xy, 0, 1) * textureMatrix).xy;
                                              float u = baryCoord.x;
                                              float v = baryCoord.y;
                                              float w = 1.0 - u - v;
                                              vec4 color = w * c1 + u * c2 + v * c3;
                                              vec2 tc = w * ct1 + u * ct2 + v * ct3;
                                              uint textureIndex = materials[gl_InstanceCustomIndexEXT].textureIndex;
                                              payload.color = (textureIndex > 0) ? color * texture(textures[textureIndex-1], ct) : color;
                                          }
                                          )";
                                          const std::string raymiss = R"(
                                                                         #version 460
                                                                         #extension GL_EXT_ray_tracing : enable
                                                                         struct RayPayload {
                                                                             vec4 color;
                                                                         };
                                                                         layout(location = 0) rayPayloadInEXT RayPayload payload;
                                                                         void main()
                                                                         {
                                                                             payload.color = vec4(0.0, 0.0, 0.0, 0.0);
                                                                         }
                                                                         )";
            if (!raytracingShader.loadRaytracingFromMemory(raygen, raymiss, rayhit)) {
                throw core::Erreur(52, "Error, failed to load ray tracing shader", 3);
            }
        }
        RaytracingRenderComponent::RayTracingScratchBuffer RaytracingRenderComponent::createScratchBuffer(VkDeviceSize size) {
            RayTracingScratchBuffer scratchBuffer{};

            VkBufferCreateInfo bufferCreateInfo{};
            bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferCreateInfo.size = size;
            bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
            if (vkCreateBuffer(vkDevice.getDevice(), &bufferCreateInfo, nullptr, &scratchBuffer.handle) != VK_SUCCESS) {
                throw std::runtime_error("failed to create scratch buffer");
            }

            VkMemoryRequirements memoryRequirements{};
            vkGetBufferMemoryRequirements(vkDevice.getDevice(), scratchBuffer.handle, &memoryRequirements);

            VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
            memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
            memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

            VkMemoryAllocateInfo memoryAllocateInfo = {};
            memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
            memoryAllocateInfo.allocationSize = memoryRequirements.size;
            memoryAllocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            if(vkAllocateMemory(vkDevice.getDevice(), &memoryAllocateInfo, nullptr, &scratchBuffer.memory) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate scratch buffer memory");
            }
            if(vkBindBufferMemory(vkDevice.getDevice(), scratchBuffer.handle, scratchBuffer.memory, 0)) {
                throw std::runtime_error("failed to bind scratch buffer memory");
            }

            VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{};
            bufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
            bufferDeviceAddressInfo.buffer = scratchBuffer.handle;
            scratchBuffer.deviceAddress = vkGetBufferDeviceAddressKHR(vkDevice.getDevice(), &bufferDeviceAddressInfo);

            return scratchBuffer;
        }
        uint32_t RaytracingRenderComponent::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
            VkPhysicalDeviceMemoryProperties memProperties;
            vkGetPhysicalDeviceMemoryProperties(vkDevice.getPhysicalDevice(), &memProperties);
            for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
                if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                    return i;
                }
            }
            throw std::runtime_error("aucun type de memoire ne satisfait le buffer!");
        }
        void RaytracingRenderComponent::deleteScratchBuffer(RayTracingScratchBuffer& scratchBuffer) {
            if (scratchBuffer.memory != VK_NULL_HANDLE) {
			vkFreeMemory(vkDevice.getDevice(), scratchBuffer.memory, nullptr);
            }
            if (scratchBuffer.handle != VK_NULL_HANDLE) {
                vkDestroyBuffer(vkDevice.getDevice(), scratchBuffer.handle, nullptr);
            }
        }
        void RaytracingRenderComponent::createAccelerationStructureBuffer(AccelerationStructure &accelerationStructure, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo)
        {
            VkBufferCreateInfo bufferCreateInfo{};
            bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferCreateInfo.size = buildSizeInfo.accelerationStructureSize;
            bufferCreateInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
            if(vkCreateBuffer(vkDevice.getDevice(), &bufferCreateInfo, nullptr, &accelerationStructure.buffer) != VK_SUCCESS) {
                throw std::runtime_error("failed to create acceleration structure buffer!");
            }
            VkMemoryRequirements memoryRequirements{};
            vkGetBufferMemoryRequirements(vkDevice.getDevice(), accelerationStructure.buffer, &memoryRequirements);
            VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
            memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
            memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
            VkMemoryAllocateInfo memoryAllocateInfo{};
            memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
            memoryAllocateInfo.allocationSize = memoryRequirements.size;
            memoryAllocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            if(vkAllocateMemory(vkDevice.getDevice(), &memoryAllocateInfo, nullptr, &accelerationStructure.memory) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate memory for acceleration structure buffer!");
            }
            if(vkBindBufferMemory(vkDevice.getDevice(), accelerationStructure.buffer, accelerationStructure.memory, 0) != VK_SUCCESS) {
                throw std::runtime_error("failed to bind memory for acceleration structure buffer!");
            }
        }
        void RaytracingRenderComponent::createStorageImage()
        {
            VkImageCreateInfo image = {};
            image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image.imageType = VK_IMAGE_TYPE_2D;
            image.format = window.getSwapchainImageFormat();
            image.extent.width = window.getSize().x();
            image.extent.height = window.getSize().y();
            image.extent.depth = 1;
            image.mipLevels = 1;
            image.arrayLayers = 1;
            image.samples = VK_SAMPLE_COUNT_1_BIT;
            image.tiling = VK_IMAGE_TILING_OPTIMAL;
            image.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
            image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            if(vkCreateImage(vkDevice.getDevice(), &image, nullptr, &storageImage.image) != VK_SUCCESS) {
                throw std::runtime_error("failed to create storage image!");
            }
            VkMemoryRequirements memReqs;
            vkGetImageMemoryRequirements(vkDevice.getDevice(), storageImage.image, &memReqs);
            VkMemoryAllocateInfo memoryAllocateInfo = {};
            memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            memoryAllocateInfo.allocationSize = memReqs.size;
            memoryAllocateInfo.memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            if(vkAllocateMemory(vkDevice.getDevice(), &memoryAllocateInfo, nullptr, &storageImage.memory) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate memory for storage image!");
            }
            if(vkBindImageMemory(vkDevice.getDevice(), storageImage.image, storageImage.memory, 0) != VK_SUCCESS) {
                throw std::runtime_error("failed to bind memory for storage image!");
            }

            VkImageViewCreateInfo colorImageView = {};
            colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
            colorImageView.format = window.getSwapchainImageFormat();
            colorImageView.subresourceRange = {};
            colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            colorImageView.subresourceRange.baseMipLevel = 0;
            colorImageView.subresourceRange.levelCount = 1;
            colorImageView.subresourceRange.baseArrayLayer = 0;
            colorImageView.subresourceRange.layerCount = 1;
            colorImageView.image = storageImage.image;
            if(vkCreateImageView(vkDevice.getDevice(), &colorImageView, nullptr, &storageImage.view) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image view for storage image!");
            }

            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

            allocInfo.commandPool = commandPool;
            allocInfo.commandBufferCount = 1;

            VkCommandBuffer commandBuffer;
            vkAllocateCommandBuffers(vkDevice.getDevice(), &allocInfo, &commandBuffer);

            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            vkBeginCommandBuffer(commandBuffer, &beginInfo);
            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            barrier.image = storageImage.image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.layerCount = 1;
            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
            vkEndCommandBuffer(commandBuffer);

            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;

            vkQueueSubmit(vkDevice.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(vkDevice.getGraphicsQueue());

            vkFreeCommandBuffers(vkDevice.getDevice(), commandPool, 1, &commandBuffer);
        }
        uint64_t RaytracingRenderComponent::getBufferDeviceAddress(VkBuffer buffer)
        {
            VkBufferDeviceAddressInfoKHR bufferDeviceAI{};
            bufferDeviceAI.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
            bufferDeviceAI.buffer = buffer;
            return vkGetBufferDeviceAddressKHR(vkDevice.getDevice(), &bufferDeviceAI);
        }
        void RaytracingRenderComponent::createTrianglesBuffers() {
            VkDeviceSize bufferSize = vertices.size() * sizeof(Vertex);
            if (bufferSize > maxTriangleBufferSize) {
                if (triangleStagingBuffer != nullptr) {
                    vkDestroyBuffer(vkDevice.getDevice(), triangleStagingBuffer, nullptr);
                    vkFreeMemory(vkDevice.getDevice(), triangleStagingBufferMemory, nullptr);
                }
                createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, triangleStagingBuffer, triangleStagingBufferMemory);

                vkDestroyBuffer(vkDevice.getDevice(), triangleBuffer, nullptr);
                vkFreeMemory(vkDevice.getDevice(), triangleBufferMemory, nullptr);
                createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, triangleBuffer, triangleBufferMemory);
                maxTriangleBufferSize = bufferSize;
                needToUpdateDS  = true;
            }
            void* data;
            vkMapMemory(vkDevice.getDevice(), triangleBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, vertices.data(), (size_t)bufferSize);
            vkUnmapMemory(vkDevice.getDevice(), triangleBufferMemory);
            copyBuffer(triangleStagingBuffer, triangleBuffer, bufferSize);
            bufferSize = indexes.size() * sizeof(uint32_t);
            if (bufferSize > maxIndexTriangleBufferSize) {
                if (indexTriangleStagingBuffer != nullptr) {
                    vkDestroyBuffer(vkDevice.getDevice(), indexTriangleStagingBuffer, nullptr);
                    vkFreeMemory(vkDevice.getDevice(), indexTriangleStagingBufferMemory, nullptr);
                }
                createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, indexTriangleStagingBuffer, indexTriangleStagingBufferMemory);

                vkDestroyBuffer(vkDevice.getDevice(), indexTriangleBuffer, nullptr);
                vkFreeMemory(vkDevice.getDevice(), indexTriangleBufferMemory, nullptr);
                createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexTriangleBuffer, indexTriangleBufferMemory);
                maxIndexTriangleBufferSize = bufferSize;
                needToUpdateDS  = true;
            }
            vkMapMemory(vkDevice.getDevice(), indexTriangleBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, indexes.data(), (size_t)bufferSize);
            vkUnmapMemory(vkDevice.getDevice(), indexTriangleBufferMemory);
            copyBuffer(indexTriangleStagingBuffer, indexTriangleBuffer, bufferSize);
        }
        void RaytracingRenderComponent::createBottomLevelAccelerationStructure()
        {
            geometryOffsets.clear();
            bottomLevelASs.clear();
            geometryOffsets.clear();
            uint32_t offset = 0;
            uint32_t nbVertices = 0;
            for (unsigned int i = 0; i < m_normals.size(); i++) {
                if (m_normals[i].getAllVertices().getVertexCount() > 0) {
                    GeometryOffset geometryOffset;
                    geometryOffset.vertexOffset = offset;
                    geometryOffset.indexOffset = offset;
                    geometryOffsets.push_back(geometryOffset);
                    std::vector<math::Vec3f> vertices;
                    std::vector<uint32_t> indexes;
                    VertexArray va = m_normals[i].getAllVertices();
                    unsigned int size = 0;
                    if (va.getPrimitiveType() == PrimitiveType::Quads) {
                        size = va.getVertexCount() / 4;
                    } else if (va.getPrimitiveType() == PrimitiveType::Triangles) {
                        size = va.getVertexCount() / 3;
                    } else if (va.getPrimitiveType() == PrimitiveType::TriangleStrip || va.getPrimitiveType() == PrimitiveType::TriangleFan) {
                        size = va.getVertexCount() - 2;
                    }
                    for (unsigned int i = 0; i < size; i++) {
                        if (va.getPrimitiveType() == PrimitiveType::Quads) {
                            for (unsigned int n = 0; n < 2; n++) {
                                if (n == 0) {
                                    math::Vec3f v1, v2, v3;
                                    v1 = va[i*4].position;
                                    v2 = va[i*4+1].position;
                                    v3 = va[i*4+2].position;
                                    indexes.push_back(nbVertices);
                                    nbVertices++;
                                    indexes.push_back(nbVertices);
                                    nbVertices++;
                                    indexes.push_back(nbVertices);
                                    nbVertices++;
                                    vertices.push_back(v1);
                                    vertices.push_back(v2);
                                    vertices.push_back(v3);
                                    offset+=3;
                                } else {
                                    math::Vec3f v1, v2, v3;
                                    v1 = va[i*4].position;
                                    v2 = va[i*4+2].position;
                                    v3 = va[i*4+3].position;
                                    indexes.push_back(nbVertices);
                                    nbVertices++;
                                    indexes.push_back(nbVertices);
                                    nbVertices++;
                                    indexes.push_back(nbVertices);
                                    nbVertices++;
                                    vertices.push_back(v1);
                                    vertices.push_back(v2);
                                    vertices.push_back(v3);
                                    offset+=3;
                                }
                            }
                        } else if (va.getPrimitiveType() == PrimitiveType::Triangles) {
                            math::Vec3f v1, v2, v3;
                            v1 = va[i*3].position;
                            v2 = va[i*3+1].position;
                            v3 = va[i*3+2].position;
                            indexes.push_back(nbVertices);
                            nbVertices++;
                            indexes.push_back(nbVertices);
                            nbVertices++;
                            indexes.push_back(nbVertices);
                            nbVertices++;
                            vertices.push_back(v1);
                            vertices.push_back(v2);
                            vertices.push_back(v3);
                            offset+=3;
                        } /*else if (va.getPrimitiveType() == PrimitiveType::TriangleStrip) {
                            if (i == 0) {
                                Vertex triangle;
                                triangle.positions[0] = math::Vec3f(va[i*3].position.x,va[i*3].position.y,va[i*3].position.z);
                                triangle.positions[1] = math::Vec3f(va[i*3+1].position.x,va[i*3+1].position.y,va[i*3+1].position.z);
                                triangle.positions[2] = math::Vec3f(va[i*3+2].position.x,va[i*3+2].position.y,va[i*3+2].position.z);
                                triangles.push_back(triangle);
                                indexes.push_back(i*3);
                                indexes.push_back(i*3+1);
                                indexes.push_back(i*3+2);
                                offset++;
                            } else {
                                Vertex triangle;
                                triangle.positions[0] = math::Vec3f(va[i].position.x,va[i].position.y,va[i].position.z);;
                                triangle.positions[1] = math::Vec3f(va[i+1].position.x,va[i+1].position.y,va[i+1].position.z);
                                triangle.positions[2] = math::Vec3f(va[i+2].position.x,va[i+2].position.y,va[i+2].position.z);
                                triangles.push_back(triangle);
                                indexes.push_back(i);
                                indexes.push_back(i+1);
                                indexes.push_back(i+2);
                                offset++;
                            }
                        } else if (va.getPrimitiveType() == TriangleFan) {
                            if (i == 0) {
                                Vertex triangle;
                                triangle.positions[0] = math::Vec3f(va[i*3].position.x,va[i*3].position.y,va[i*3].position.z);;
                                triangle.positions[1] = math::Vec3f(va[i*3+1].position.x,va[i*3+1].position.y,va[i*3+1].position.z);
                                triangle.positions[2] = math::Vec3f(va[i*3+2].position.x,va[i*3+2].position.y,va[i*3+2].position.z);
                                triangles.push_back(triangle);
                                indexes.push_back(i*3);
                                indexes.push_back(i*3+1);
                                indexes.push_back(i*3+2);
                                offset++;
                            } else {
                                Vertex triangle;
                                triangle.positions[0] = math::Vec3f(va[0].position.x,va[0].position.y,va[0].position.z);
                                triangle.positions[1] = math::Vec3f(va[i+1].position.x,va[i+1].position.y,va[i+1].position.z);
                                triangle.positions[2] = math::Vec3f(va[i+2].position.x,va[i+2].position.y,va[i+2].position.z);
                                triangles.push_back(triangle);
                                indexes.push_back(0);
                                indexes.push_back(i+1);
                                indexes.push_back(i+2);
                                offset++;
                            }
                        }*/
                    }
                }

                VkTransformMatrixKHR transformMatrix = {
                    1.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f
                };
                VkDeviceSize bufferSize = vertices.size() * sizeof(math::Vec3f);
                if (bufferSize > maxVertexBufferSize) {
                    if (vertexStagingBuffer != nullptr) {
                        vkDestroyBuffer(vkDevice.getDevice(), vertexStagingBuffer, nullptr);
                        vkFreeMemory(vkDevice.getDevice(), vertexStagingBufferMemory, nullptr);
                    }
                    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexStagingBuffer, vertexStagingBufferMemory);

                    vkDestroyBuffer(vkDevice.getDevice(), vertexBuffer, nullptr);
                    vkFreeMemory(vkDevice.getDevice(), vertexBufferMemory, nullptr);
                    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
                    maxVertexBufferSize = bufferSize;
                }
                void* data;
                vkMapMemory(vkDevice.getDevice(), vertexBufferMemory, 0, bufferSize, 0, &data);
                memcpy(data, vertices.data(), (size_t)bufferSize);
                vkUnmapMemory(vkDevice.getDevice(), vertexBufferMemory);
                copyBuffer(vertexStagingBuffer, vertexBuffer, bufferSize);
                bufferSize = indexes.size() * sizeof(uint32_t);
                if (bufferSize > maxIndexBufferSize) {
                    if (indexStagingBuffer != nullptr) {
                        vkDestroyBuffer(vkDevice.getDevice(), indexStagingBuffer, nullptr);
                        vkFreeMemory(vkDevice.getDevice(), indexStagingBufferMemory, nullptr);
                    }
                    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, indexStagingBuffer, indexStagingBufferMemory);

                    vkDestroyBuffer(vkDevice.getDevice(), indexBuffer, nullptr);
                    vkFreeMemory(vkDevice.getDevice(), indexBufferMemory, nullptr);
                    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);
                    maxVertexBufferSize = bufferSize;
                }
                vkMapMemory(vkDevice.getDevice(), indexBufferMemory, 0, bufferSize, 0, &data);
                memcpy(data, indexes.data(), (size_t)bufferSize);
                vkUnmapMemory(vkDevice.getDevice(), indexBufferMemory);
                copyBuffer(vertexStagingBuffer, indexBuffer, bufferSize);
                bufferSize = sizeof(VkTransformMatrixKHR);
                if (bufferSize > maxTransformBufferSize) {
                    if (transformStagingBuffer != nullptr) {
                        vkDestroyBuffer(vkDevice.getDevice(), transformStagingBuffer, nullptr);
                        vkFreeMemory(vkDevice.getDevice(), transformStagingBufferMemory, nullptr);
                    }
                    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, transformStagingBuffer, transformStagingBufferMemory);

                    vkDestroyBuffer(vkDevice.getDevice(), transformBuffer, nullptr);
                    vkFreeMemory(vkDevice.getDevice(), transformBufferMemory, nullptr);
                    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, transformBuffer, transformBufferMemory);
                    maxTransformBufferSize = bufferSize;
                }
                vkMapMemory(vkDevice.getDevice(), transformBufferMemory, 0, bufferSize, 0, &data);
                memcpy(data, &transformMatrix, (size_t)bufferSize);
                vkUnmapMemory(vkDevice.getDevice(), transformBufferMemory);
                copyBuffer(transformStagingBuffer, transformBuffer, bufferSize);
                VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
                VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};
                VkDeviceOrHostAddressConstKHR transformBufferDeviceAddress{};

                vertexBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(vertexBuffer);
                indexBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(indexBuffer);
                transformBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(transformBuffer);

                // Build
                VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
                accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
                accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
                accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
                accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
                accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
                accelerationStructureGeometry.geometry.triangles.vertexData = vertexBufferDeviceAddress;
                accelerationStructureGeometry.geometry.triangles.maxVertex = 2;
                accelerationStructureGeometry.geometry.triangles.vertexStride = sizeof(Vertex);
                accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
                accelerationStructureGeometry.geometry.triangles.indexData = indexBufferDeviceAddress;
                accelerationStructureGeometry.geometry.triangles.transformData.deviceAddress = 0;
                accelerationStructureGeometry.geometry.triangles.transformData.hostAddress = nullptr;
                accelerationStructureGeometry.geometry.triangles.transformData = transformBufferDeviceAddress;

                // Get size info
                VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
                accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
                accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
                accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
                accelerationStructureBuildGeometryInfo.geometryCount = 1;
                accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;

                const uint32_t numTriangles = vertices.size() / 3;
                VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
                accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
                vkGetAccelerationStructureBuildSizesKHR(
                    vkDevice.getDevice(),
                    VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                    &accelerationStructureBuildGeometryInfo,
                    &numTriangles,
                    &accelerationStructureBuildSizesInfo);
                AccelerationStructure bottomLevelAS;
                createAccelerationStructureBuffer(bottomLevelAS, accelerationStructureBuildSizesInfo);

                VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
                accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
                accelerationStructureCreateInfo.buffer = bottomLevelAS.buffer;
                accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
                accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
                vkCreateAccelerationStructureKHR(vkDevice.getDevice(), &accelerationStructureCreateInfo, nullptr, &bottomLevelAS.handle);

                // Create a small scratch buffer used during build of the bottom level acceleration structure
                RayTracingScratchBuffer scratchBuffer = createScratchBuffer(accelerationStructureBuildSizesInfo.buildScratchSize);

                VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
                accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
                accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
                accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
                accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
                accelerationBuildGeometryInfo.dstAccelerationStructure = bottomLevelAS.handle;
                accelerationBuildGeometryInfo.geometryCount = 1;
                accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
                accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

                VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
                accelerationStructureBuildRangeInfo.primitiveCount = numTriangles;
                accelerationStructureBuildRangeInfo.primitiveOffset = 0;
                accelerationStructureBuildRangeInfo.firstVertex = 0;
                accelerationStructureBuildRangeInfo.transformOffset = 0;
                std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

                // Build the acceleration structure on the device via a one-time command buffer submission
                // Some implementations may support acceleration structure building on the host (VkPhysicalDeviceAccelerationStructureFeaturesKHR->accelerationStructureHostCommands), but we prefer device builds
                VkCommandBufferAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

                allocInfo.commandPool = commandPool;
                allocInfo.commandBufferCount = 1;

                VkCommandBuffer commandBuffer;
                vkAllocateCommandBuffers(vkDevice.getDevice(), &allocInfo, &commandBuffer);

                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

                vkBeginCommandBuffer(commandBuffer, &beginInfo);
                vkCmdBuildAccelerationStructuresKHR(
                        commandBuffer,
                        1,
                        &accelerationBuildGeometryInfo,
                        accelerationBuildStructureRangeInfos.data());
                vkEndCommandBuffer(commandBuffer);

                VkSubmitInfo submitInfo{};
                submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &commandBuffer;

                vkQueueSubmit(vkDevice.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
                vkQueueWaitIdle(vkDevice.getGraphicsQueue());

                vkFreeCommandBuffers(vkDevice.getDevice(), commandPool, 1, &commandBuffer);



                VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
                accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
                accelerationDeviceAddressInfo.accelerationStructure = bottomLevelAS.handle;
                bottomLevelAS.deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(vkDevice.getDevice(), &accelerationDeviceAddressInfo);

                deleteScratchBuffer(scratchBuffer);
                bottomLevelASs.push_back(bottomLevelAS);
            }
            for (unsigned int i = 0; i < m_instances.size(); i++) {
                if (m_instances[i].getAllVertices().getVertexCount() > 0) {
                    VertexArray firstInstanceVertices;
                    if (m_instances[i].getVertexArrays().size() > 0) {
                        Entity* entity = m_instances[i].getVertexArrays()[0]->getEntity();
                        PrimitiveType p = m_instances[i].getVertexArrays()[0]->getPrimitiveType();
                        firstInstanceVertices.setPrimitiveType(p);
                        for (unsigned int j = 0; j < m_instances[i].getVertexArrays().size(); j++) {
                            if (entity == m_instances[i].getVertexArrays()[j]->getEntity()) {
                                for (unsigned int k = 0; k < m_instances[i].getVertexArrays()[j]->getVertexCount(); k++) {
                                    firstInstanceVertices.append((*m_instances[i].getVertexArrays()[j])[k]);
                                }
                            }
                        }
                    }

                    std::vector<math::Vec3f> vertices;
                    std::vector<uint32_t> indexes;
                    unsigned int size = 0;
                    if (firstInstanceVertices.getPrimitiveType() == PrimitiveType::Quads) {
                        size = firstInstanceVertices.getVertexCount() / 4;
                    } else if (firstInstanceVertices.getPrimitiveType() == PrimitiveType::Triangles) {
                        size = firstInstanceVertices.getVertexCount() / 3;
                    } else if (firstInstanceVertices.getPrimitiveType() == PrimitiveType::TriangleStrip || firstInstanceVertices.getPrimitiveType() == PrimitiveType::TriangleFan) {
                        size = firstInstanceVertices.getVertexCount() - 2;
                    }
                    for (unsigned int i = 0; i < size; i++) {
                        if (firstInstanceVertices.getPrimitiveType() == PrimitiveType::Quads) {
                            for (unsigned int n = 0; n < 2; n++) {
                                if (n == 0) {
                                    math::Vec3f v1, v2, v3;
                                    v1 = firstInstanceVertices[i*4].position;
                                    v2 = firstInstanceVertices[i*4+1].position;
                                    v3 = firstInstanceVertices[i*4+2].position;
                                    indexes.push_back(nbVertices);
                                    nbVertices++;
                                    indexes.push_back(nbVertices);
                                    nbVertices++;
                                    indexes.push_back(nbVertices);
                                    nbVertices++;
                                    vertices.push_back(v1);
                                    vertices.push_back(v2);
                                    vertices.push_back(v3);
                                } else {
                                    math::Vec3f v1, v2, v3;
                                    v1 = firstInstanceVertices[i*4].position;
                                    v2 = firstInstanceVertices[i*4+2].position;
                                    v3 = firstInstanceVertices[i*4+3].position;
                                    indexes.push_back(nbVertices);
                                    nbVertices++;
                                    indexes.push_back(nbVertices);
                                    nbVertices++;
                                    indexes.push_back(nbVertices);
                                    nbVertices++;
                                    vertices.push_back(v1);
                                    vertices.push_back(v2);
                                    vertices.push_back(v3);
                                }
                            }
                        } else if (firstInstanceVertices.getPrimitiveType() == PrimitiveType::Triangles) {
                            math::Vec3f v1, v2, v3;
                            v1 = firstInstanceVertices[i*3].position;
                            v2 = firstInstanceVertices[i*3+1].position;
                            v3 = firstInstanceVertices[i*3+2].position;
                            indexes.push_back(nbVertices);
                            nbVertices++;
                            indexes.push_back(nbVertices);
                            nbVertices++;
                            indexes.push_back(nbVertices);
                            nbVertices++;
                            vertices.push_back(v1);
                            vertices.push_back(v2);
                            vertices.push_back(v3);
                        }
                    }
                    for (unsigned int t = 0; t < m_instances[i].getTransforms().size(); t++) {
                        GeometryOffset geometryOffset;
                        geometryOffset.vertexOffset = offset;
                        geometryOffset.indexOffset = offset;
                        geometryOffsets.push_back(geometryOffset);
                        offset += vertices.size();
                    }
                    VkTransformMatrixKHR transformMatrix = {
                    1.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f
                    };
                    VkDeviceSize bufferSize = vertices.size() * sizeof(math::Vec3f);
                    if (bufferSize > maxVertexBufferSize) {
                        if (vertexStagingBuffer != nullptr) {
                            vkDestroyBuffer(vkDevice.getDevice(), vertexStagingBuffer, nullptr);
                            vkFreeMemory(vkDevice.getDevice(), vertexStagingBufferMemory, nullptr);
                        }
                        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexStagingBuffer, vertexStagingBufferMemory);

                        vkDestroyBuffer(vkDevice.getDevice(), vertexBuffer, nullptr);
                        vkFreeMemory(vkDevice.getDevice(), vertexBufferMemory, nullptr);
                        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
                        maxVertexBufferSize = bufferSize;
                    }
                    void* data;
                    vkMapMemory(vkDevice.getDevice(), vertexBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, vertices.data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), vertexBufferMemory);
                    copyBuffer(vertexStagingBuffer, vertexBuffer, bufferSize);
                    bufferSize = indexes.size() * sizeof(uint32_t);
                    if (bufferSize > maxIndexBufferSize) {
                        if (indexStagingBuffer != nullptr) {
                            vkDestroyBuffer(vkDevice.getDevice(), indexStagingBuffer, nullptr);
                            vkFreeMemory(vkDevice.getDevice(), indexStagingBufferMemory, nullptr);
                        }
                        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, indexStagingBuffer, indexStagingBufferMemory);

                        vkDestroyBuffer(vkDevice.getDevice(), indexBuffer, nullptr);
                        vkFreeMemory(vkDevice.getDevice(), indexBufferMemory, nullptr);
                        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);
                        maxVertexBufferSize = bufferSize;
                    }
                    vkMapMemory(vkDevice.getDevice(), indexBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, indexes.data(), (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), indexBufferMemory);
                    copyBuffer(vertexStagingBuffer, indexBuffer, bufferSize);
                    bufferSize = sizeof(VkTransformMatrixKHR);
                    if (bufferSize > maxTransformBufferSize) {
                        if (transformStagingBuffer != nullptr) {
                            vkDestroyBuffer(vkDevice.getDevice(), transformStagingBuffer, nullptr);
                            vkFreeMemory(vkDevice.getDevice(), transformStagingBufferMemory, nullptr);
                        }
                        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, transformStagingBuffer, transformStagingBufferMemory);

                        vkDestroyBuffer(vkDevice.getDevice(), transformBuffer, nullptr);
                        vkFreeMemory(vkDevice.getDevice(), transformBufferMemory, nullptr);
                        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, transformBuffer, transformBufferMemory);
                        maxTransformBufferSize = bufferSize;
                    }
                    vkMapMemory(vkDevice.getDevice(), transformBufferMemory, 0, bufferSize, 0, &data);
                    memcpy(data, &transformMatrix, (size_t)bufferSize);
                    vkUnmapMemory(vkDevice.getDevice(), transformBufferMemory);
                    copyBuffer(transformStagingBuffer, transformBuffer, bufferSize);
                    VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
                    VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};
                    VkDeviceOrHostAddressConstKHR transformBufferDeviceAddress{};

                    vertexBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(vertexBuffer);
                    indexBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(indexBuffer);
                    transformBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(transformBuffer);

                    // Build
                    VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
                    accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
                    accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
                    accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
                    accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
                    accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
                    accelerationStructureGeometry.geometry.triangles.vertexData = vertexBufferDeviceAddress;
                    accelerationStructureGeometry.geometry.triangles.maxVertex = 2;
                    accelerationStructureGeometry.geometry.triangles.vertexStride = sizeof(Vertex);
                    accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
                    accelerationStructureGeometry.geometry.triangles.indexData = indexBufferDeviceAddress;
                    accelerationStructureGeometry.geometry.triangles.transformData.deviceAddress = 0;
                    accelerationStructureGeometry.geometry.triangles.transformData.hostAddress = nullptr;
                    accelerationStructureGeometry.geometry.triangles.transformData = transformBufferDeviceAddress;

                    // Get size info
                    VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
                    accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
                    accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
                    accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
                    accelerationStructureBuildGeometryInfo.geometryCount = 1;
                    accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;

                    const uint32_t numTriangles = vertices.size() / 3;
                    VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
                    accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
                    vkGetAccelerationStructureBuildSizesKHR(
                        vkDevice.getDevice(),
                        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                        &accelerationStructureBuildGeometryInfo,
                        &numTriangles,
                        &accelerationStructureBuildSizesInfo);
                    AccelerationStructure bottomLevelAS;
                    createAccelerationStructureBuffer(bottomLevelAS, accelerationStructureBuildSizesInfo);

                    VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
                    accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
                    accelerationStructureCreateInfo.buffer = bottomLevelAS.buffer;
                    accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
                    accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
                    vkCreateAccelerationStructureKHR(vkDevice.getDevice(), &accelerationStructureCreateInfo, nullptr, &bottomLevelAS.handle);

                    // Create a small scratch buffer used during build of the bottom level acceleration structure
                    RayTracingScratchBuffer scratchBuffer = createScratchBuffer(accelerationStructureBuildSizesInfo.buildScratchSize);

                    VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
                    accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
                    accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
                    accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
                    accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
                    accelerationBuildGeometryInfo.dstAccelerationStructure = bottomLevelAS.handle;
                    accelerationBuildGeometryInfo.geometryCount = 1;
                    accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
                    accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

                    VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
                    accelerationStructureBuildRangeInfo.primitiveCount = numTriangles;
                    accelerationStructureBuildRangeInfo.primitiveOffset = 0;
                    accelerationStructureBuildRangeInfo.firstVertex = 0;
                    accelerationStructureBuildRangeInfo.transformOffset = 0;
                    std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

                    // Build the acceleration structure on the device via a one-time command buffer submission
                    // Some implementations may support acceleration structure building on the host (VkPhysicalDeviceAccelerationStructureFeaturesKHR->accelerationStructureHostCommands), but we prefer device builds
                    VkCommandBufferAllocateInfo allocInfo{};
                    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

                    allocInfo.commandPool = commandPool;
                    allocInfo.commandBufferCount = 1;

                    VkCommandBuffer commandBuffer;
                    vkAllocateCommandBuffers(vkDevice.getDevice(), &allocInfo, &commandBuffer);

                    VkCommandBufferBeginInfo beginInfo{};
                    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

                    vkBeginCommandBuffer(commandBuffer, &beginInfo);
                    vkCmdBuildAccelerationStructuresKHR(
                            commandBuffer,
                            1,
                            &accelerationBuildGeometryInfo,
                            accelerationBuildStructureRangeInfos.data());
                    vkEndCommandBuffer(commandBuffer);

                    VkSubmitInfo submitInfo{};
                    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                    submitInfo.commandBufferCount = 1;
                    submitInfo.pCommandBuffers = &commandBuffer;

                    vkQueueSubmit(vkDevice.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
                    vkQueueWaitIdle(vkDevice.getGraphicsQueue());

                    vkFreeCommandBuffers(vkDevice.getDevice(), commandPool, 1, &commandBuffer);



                    VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
                    accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
                    accelerationDeviceAddressInfo.accelerationStructure = bottomLevelAS.handle;
                    bottomLevelAS.deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(vkDevice.getDevice(), &accelerationDeviceAddressInfo);

                    deleteScratchBuffer(scratchBuffer);
                    bottomLevelASs.push_back(bottomLevelAS);
                }
            }
            VkDeviceSize bufferSize = geometryOffsets.size() * sizeof(GeometryOffset);
            if (bufferSize > maxTriangleOffsetBufferSize) {
                if (triangleOffsetStagingBuffer != nullptr) {
                    vkDestroyBuffer(vkDevice.getDevice(), triangleOffsetStagingBuffer, nullptr);
                    vkFreeMemory(vkDevice.getDevice(), triangleOffsetStagingBufferMemory, nullptr);
                }
                createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, triangleOffsetStagingBuffer, triangleOffsetStagingBufferMemory);

                vkDestroyBuffer(vkDevice.getDevice(), triangleOffsetBuffer, nullptr);
                vkFreeMemory(vkDevice.getDevice(), triangleOffsetBufferMemory, nullptr);
                createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, triangleOffsetBuffer, triangleOffsetBufferMemory);
                maxTriangleOffsetBufferSize = bufferSize;
                needToUpdateDS  = true;
            }
            void* data;
            vkMapMemory(vkDevice.getDevice(), triangleOffsetBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, geometryOffsets.data(), (size_t)bufferSize);
            vkUnmapMemory(vkDevice.getDevice(), triangleOffsetBufferMemory);
            copyBuffer(triangleOffsetStagingBuffer, triangleOffsetBuffer, bufferSize);
        }
        VkTransformMatrixKHR RaytracingRenderComponent::toVkTransformMatrixKHR (math::Matrix4f matrix) {
            VkTransformMatrixKHR transformMatrix = {
                    matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3],
                    matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3],
                    matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3] };
            return transformMatrix;
        }
        void RaytracingRenderComponent::createTopLevelAccelerationStructure() {
            uint32_t blasIndex = 0, instanceId = 0;
            std::vector<VkAccelerationStructureInstanceKHR> instances;
            materialDatas.clear();
            for (unsigned int i = 0; i < m_normals.size(); i++) {
                if (m_normals[i].getAllVertices().getVertexCount() > 0) {
                    VkTransformMatrixKHR transformMatrix = {
                    1.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f };

                    VkAccelerationStructureInstanceKHR instance{};
                    instance.transform = transformMatrix;
                    instance.instanceCustomIndex = 0;
                    instance.mask = 0xFF;
                    instance.instanceShaderBindingTableRecordOffset = 0;
                    instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
                    instance.accelerationStructureReference = bottomLevelASs[blasIndex].deviceAddress;
                    instance.instanceCustomIndex = instanceId;
                    instances.push_back(instance);
                    blasIndex++;
                    instanceId++;
                    MaterialData materialData;
                    materialData.materialIndex = (m_normals[i].getMaterial().getTexture() != nullptr) ? m_normals[i].getMaterial().getTexture()->getId() : 0;
                    materialData.textureMatrix = (m_normals[i].getMaterial().getTexture() != nullptr) ? m_normals[i].getMaterial().getTexture()->getTextureMatrix() : math::Matrix4f();
                    materialDatas.push_back(materialData);
                }
            }
            for (unsigned int i = 0; i < m_instances.size(); i++) {
                if (m_instances[i].getAllVertices().getVertexCount() > 0) {
                    std::vector<TransformMatrix*> tm = m_instances[i].getTransforms();
                    for (unsigned int t = 0; t < tm.size(); t++) {
                        VkTransformMatrixKHR transformMatrix = toVkTransformMatrixKHR(tm[t]->getMatrix().transpose());

                        VkAccelerationStructureInstanceKHR instance{};
                        instance.transform = transformMatrix;
                        instance.instanceCustomIndex = 0;
                        instance.mask = 0xFF;
                        instance.instanceShaderBindingTableRecordOffset = 0;
                        instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
                        instance.accelerationStructureReference = bottomLevelASs[blasIndex].deviceAddress;
                        instance.instanceCustomIndex = instanceId;
                        instances.push_back(instance);
                        instanceId++;
                        MaterialData materialData;
                        materialData.materialIndex = (m_instances[i].getMaterial().getTexture() != nullptr) ? m_instances[i].getMaterial().getTexture()->getId() : 0;
                        materialData.textureMatrix = (m_instances[i].getMaterial().getTexture() != nullptr) ? m_instances[i].getMaterial().getTexture()->getTextureMatrix() : math::Matrix4f();
                        materialDatas.push_back(materialData);
                    }
                    blasIndex++;
                }
            }

            VkDeviceSize bufferSize = instances.size() * sizeof(VkAccelerationStructureInstanceKHR);
            if (bufferSize > maxInstanceBufferSize) {
                if (instanceStagingBuffer != nullptr) {
                    vkDestroyBuffer(vkDevice.getDevice(), instanceStagingBuffer, nullptr);
                    vkFreeMemory(vkDevice.getDevice(), instanceStagingBufferMemory, nullptr);
                }
                createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, instanceStagingBuffer, instanceStagingBufferMemory);

                vkDestroyBuffer(vkDevice.getDevice(), instanceBuffer, nullptr);
                vkFreeMemory(vkDevice.getDevice(), instanceBufferMemory, nullptr);
                createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, instanceBuffer, instanceBufferMemory);
                maxInstanceBufferSize = bufferSize;
                needToUpdateDS = true;
            }
            void* data;
            vkMapMemory(vkDevice.getDevice(), instanceBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, instances.data(), (size_t)bufferSize);
            vkUnmapMemory(vkDevice.getDevice(), instanceBufferMemory);
            copyBuffer(instanceStagingBuffer, instanceBuffer, bufferSize);
            VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
            instanceDataDeviceAddress.deviceAddress = getBufferDeviceAddress(instanceBuffer);

            VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
            accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
            accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
            accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
            accelerationStructureGeometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
            accelerationStructureGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
            accelerationStructureGeometry.geometry.instances.data = instanceDataDeviceAddress;

            // Get size info
            /*
            The pSrcAccelerationStructure, dstAccelerationStructure, and mode members of pBuildInfo are ignored. Any VkDeviceOrHostAddressKHR members of pBuildInfo are ignored by this command, except that the hostAddress member of VkAccelerationStructureGeometryTrianglesDataKHR::transformData will be examined to check if it is NULL.*
            */
            VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
            accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
            accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
            accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
            accelerationStructureBuildGeometryInfo.geometryCount = 1;
            accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;

            uint32_t primitive_count = 1;

            VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
            accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
            vkGetAccelerationStructureBuildSizesKHR(
                vkDevice.getDevice(),
                VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                &accelerationStructureBuildGeometryInfo,
                &primitive_count,
                &accelerationStructureBuildSizesInfo);

            createAccelerationStructureBuffer(topLevelAS, accelerationStructureBuildSizesInfo);

            VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
            accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
            accelerationStructureCreateInfo.buffer = topLevelAS.buffer;
            accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
            accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
            vkCreateAccelerationStructureKHR(vkDevice.getDevice(), &accelerationStructureCreateInfo, nullptr, &topLevelAS.handle);

            // Create a small scratch buffer used during build of the top level acceleration structure
            RayTracingScratchBuffer scratchBuffer = createScratchBuffer(accelerationStructureBuildSizesInfo.buildScratchSize);

            VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
            accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
            accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
            accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
            accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
            accelerationBuildGeometryInfo.dstAccelerationStructure = topLevelAS.handle;
            accelerationBuildGeometryInfo.geometryCount = 1;
            accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
            accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

            VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
            accelerationStructureBuildRangeInfo.primitiveCount = 1;
            accelerationStructureBuildRangeInfo.primitiveOffset = 0;
            accelerationStructureBuildRangeInfo.firstVertex = 0;
            accelerationStructureBuildRangeInfo.transformOffset = 0;
            std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

            // Build the acceleration structure on the device via a one-time command buffer submission
            // Some implementations may support acceleration structure building on the host (VkPhysicalDeviceAccelerationStructureFeaturesKHR->accelerationStructureHostCommands), but we prefer device builds
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

            allocInfo.commandPool = commandPool;
            allocInfo.commandBufferCount = 1;

            VkCommandBuffer commandBuffer;
            vkAllocateCommandBuffers(vkDevice.getDevice(), &allocInfo, &commandBuffer);

            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            vkBeginCommandBuffer(commandBuffer, &beginInfo);
            vkCmdBuildAccelerationStructuresKHR(
                commandBuffer,
                1,
                &accelerationBuildGeometryInfo,
                accelerationBuildStructureRangeInfos.data());

            vkEndCommandBuffer(commandBuffer);

            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;

            vkQueueSubmit(vkDevice.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(vkDevice.getGraphicsQueue());

            vkFreeCommandBuffers(vkDevice.getDevice(), commandPool, 1, &commandBuffer);

            VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
            accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
            accelerationDeviceAddressInfo.accelerationStructure = topLevelAS.handle;
            topLevelAS.deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(vkDevice.getDevice(), &accelerationDeviceAddressInfo);

            deleteScratchBuffer(scratchBuffer);
            bufferSize = materialDatas.size() * sizeof(MaterialData);
            if (bufferSize > maxMaterialBufferSize) {
                if (materialStagingBuffer != nullptr) {
                    vkDestroyBuffer(vkDevice.getDevice(), materialStagingBuffer, nullptr);
                    vkFreeMemory(vkDevice.getDevice(), materialStagingBufferMemory, nullptr);
                }
                createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, materialStagingBuffer, materialStagingBufferMemory);

                vkDestroyBuffer(vkDevice.getDevice(), materialBuffer, nullptr);
                vkFreeMemory(vkDevice.getDevice(), materialBufferMemory, nullptr);
                createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, materialBuffer, materialBufferMemory);
                maxMaterialBufferSize = bufferSize;
                needToUpdateDS  = true;
            }
            vkMapMemory(vkDevice.getDevice(), materialBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, materialDatas.data(), (size_t)bufferSize);
            vkUnmapMemory(vkDevice.getDevice(), materialBufferMemory);
            copyBuffer(materialStagingBuffer, materialBuffer, bufferSize);
        }
        void RaytracingRenderComponent::createShaderBindingTable() {
            const uint32_t handleSize = rayTracingPipelineProperties.shaderGroupHandleSize;
            const uint32_t handleSizeAligned = (rayTracingPipelineProperties.shaderGroupHandleSize + rayTracingPipelineProperties.shaderGroupHandleAlignment - 1) & ~(rayTracingPipelineProperties.shaderGroupHandleAlignment - 1);
            const uint32_t groupCount = static_cast<uint32_t>(shaderGroups.size());
            const uint32_t sbtSize = groupCount * handleSizeAligned;

            std::vector<uint8_t> shaderHandleStorage(sbtSize);
            if(vkGetRayTracingShaderGroupHandlesKHR(vkDevice.getDevice(), pipeline, 0, groupCount, sbtSize, shaderHandleStorage.data()) != VK_SUCCESS) {
                throw std::runtime_error("failed to raytracing shader group handle!");
            }

            const VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
            const VkMemoryPropertyFlags memoryUsageFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            createBuffer(handleSize, bufferUsageFlags, memoryUsageFlags, raygenShaderBindingTable, raygenShaderBindingTableMemory);
            createBuffer(handleSize, bufferUsageFlags, memoryUsageFlags, missShaderBindingTable, missShaderBindingTableMemory);
            createBuffer(handleSize, bufferUsageFlags, memoryUsageFlags, hitShaderBindingTable, hitShaderBindingTableMemory);

            // Copy handles
            void* data;
            vkMapMemory(vkDevice.getDevice(), raygenShaderBindingTableMemory, 0, handleSize, 0, &data);
            memcpy(data, shaderHandleStorage.data(), handleSize);
            vkUnmapMemory(vkDevice.getDevice(), instanceBufferMemory);

            vkMapMemory(vkDevice.getDevice(), missShaderBindingTableMemory, 0, handleSize, 0, &data);
            memcpy(data, shaderHandleStorage.data() + handleSizeAligned, handleSize);
            vkUnmapMemory(vkDevice.getDevice(), missShaderBindingTableMemory);

            vkMapMemory(vkDevice.getDevice(), hitShaderBindingTableMemory, 0, handleSize, 0, &data);
            memcpy(data, shaderHandleStorage.data() + handleSizeAligned*2, handleSize);
            vkUnmapMemory(vkDevice.getDevice(), hitShaderBindingTableMemory);
        }
        void RaytracingRenderComponent::createDescriptorPool() {
            std::vector<Texture*> allTextures = Texture::getAllTextures();
            std::vector<VkDescriptorPoolSize> poolSizes = {
                { VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1 },
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, allTextures.size()},
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1},
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1},
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1},
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1}
            };
            VkDescriptorPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            poolInfo.pPoolSizes = poolSizes.data();
            poolInfo.maxSets = 1;
            if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
                throw std::runtime_error("echec de la creation de la pool de descripteurs!");
            }
        }
        void RaytracingRenderComponent::createDescriptorSetLayout() {
            std::vector<Texture*> allTextures = Texture::getAllTextures();
            VkDescriptorSetLayoutBinding accelerationStructureLayoutBinding{};
            accelerationStructureLayoutBinding.binding = 0;
            accelerationStructureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
            accelerationStructureLayoutBinding.descriptorCount = 1;
            accelerationStructureLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

            VkDescriptorSetLayoutBinding resultImageLayoutBinding{};
            resultImageLayoutBinding.binding = 1;
            resultImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            resultImageLayoutBinding.descriptorCount = 1;
            resultImageLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

            VkDescriptorSetLayoutBinding uniformBufferBinding{};
            uniformBufferBinding.binding = 2;
            uniformBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uniformBufferBinding.descriptorCount = 1;
            uniformBufferBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

            VkDescriptorSetLayoutBinding samplerLayoutBinding{};
            samplerLayoutBinding.binding = 3;
            samplerLayoutBinding.descriptorCount = allTextures.size();
            samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            samplerLayoutBinding.pImmutableSamplers = nullptr;
            samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

            VkDescriptorSetLayoutBinding triangleDataLayoutBinding{};
            triangleDataLayoutBinding.binding = 4;
            triangleDataLayoutBinding.descriptorCount = 1;
            triangleDataLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            triangleDataLayoutBinding.pImmutableSamplers = nullptr;
            triangleDataLayoutBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

            VkDescriptorSetLayoutBinding offsetDataLayoutBinding{};
            offsetDataLayoutBinding.binding = 5;
            offsetDataLayoutBinding.descriptorCount = 1;
            offsetDataLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            offsetDataLayoutBinding.pImmutableSamplers = nullptr;
            offsetDataLayoutBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

            VkDescriptorSetLayoutBinding indexDataLayoutBinding{};
            offsetDataLayoutBinding.binding = 6;
            offsetDataLayoutBinding.descriptorCount = 1;
            offsetDataLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            offsetDataLayoutBinding.pImmutableSamplers = nullptr;
            offsetDataLayoutBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

            VkDescriptorSetLayoutBinding materialDataLayoutBinding{};
            materialDataLayoutBinding.binding = 7;
            materialDataLayoutBinding.descriptorCount = 1;
            materialDataLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            materialDataLayoutBinding.pImmutableSamplers = nullptr;
            materialDataLayoutBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

            std::array<VkDescriptorSetLayoutBinding, 8> bindings = {accelerationStructureLayoutBinding, resultImageLayoutBinding, uniformBufferBinding, samplerLayoutBinding, triangleDataLayoutBinding, offsetDataLayoutBinding, indexDataLayoutBinding, materialDataLayoutBinding};

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            //layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
            layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());;
            layoutInfo.pBindings = bindings.data();
            if (vkCreateDescriptorSetLayout(vkDevice.getDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor set layout!");
            }
        }
        void RaytracingRenderComponent::allocateDescriptorSets() {
            std::vector<VkDescriptorSetLayout> layouts(1, descriptorSetLayout);
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = layouts.data();
            if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS) {
                throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
            }
        }
        void RaytracingRenderComponent::createDescriptorSets() {
            std::array<VkWriteDescriptorSet, 8> descriptorWrites{};
            VkWriteDescriptorSetAccelerationStructureKHR descriptorAccelerationStructureInfo{};
            descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
            descriptorAccelerationStructureInfo.accelerationStructureCount = 1;
            descriptorAccelerationStructureInfo.pAccelerationStructures = &topLevelAS.handle;

            VkWriteDescriptorSet accelerationStructureWrite{};
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            // The specialized acceleration structure descriptor has to be chained
            descriptorWrites[0].pNext = &descriptorAccelerationStructureInfo;
            descriptorWrites[0].dstSet = descriptorSet;
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

            VkDescriptorImageInfo storageImageDescriptor{};
            storageImageDescriptor.imageView = storageImage.view;
            storageImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSet;
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &storageImageDescriptor;

            VkDescriptorBufferInfo uboDescriptor{};
            uboDescriptor.buffer = ubo;
            uboDescriptor.offset = 0;
            uboDescriptor.range = sizeof(UniformData);

            descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[2].dstSet = descriptorSet;
            descriptorWrites[2].dstBinding = 2;
            descriptorWrites[2].dstArrayElement = 0;
            descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[2].descriptorCount = 1;
            descriptorWrites[2].pBufferInfo = &uboDescriptor;

            std::vector<Texture*> allTextures = Texture::getAllTextures();
            std::vector<VkDescriptorImageInfo>	descriptorImageInfos;
            descriptorImageInfos.resize(allTextures.size());
            for (unsigned int j = 0; j < allTextures.size(); j++) {
                descriptorImageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                descriptorImageInfos[j].imageView = allTextures[j]->getImageView();
                descriptorImageInfos[j].sampler = allTextures[j]->getSampler();
            }
            descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[3].dstSet = descriptorSet;
            descriptorWrites[3].dstBinding = 3;
            descriptorWrites[3].dstArrayElement = 0;
            descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[3].descriptorCount = allTextures.size();
            descriptorWrites[3].pImageInfo = descriptorImageInfos.data();

            VkDescriptorBufferInfo triangleDataStorageBufferDescriptor{};
            triangleDataStorageBufferDescriptor.buffer = triangleBuffer;
            triangleDataStorageBufferDescriptor.offset = 0;
            triangleDataStorageBufferDescriptor.range = maxTriangleBufferSize;

            descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[4].dstSet = descriptorSet;
            descriptorWrites[4].dstBinding = 4;
            descriptorWrites[4].dstArrayElement = 0;
            descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            descriptorWrites[4].descriptorCount = 1;
            descriptorWrites[4].pBufferInfo = &triangleDataStorageBufferDescriptor;

            VkDescriptorBufferInfo offsetDataStorageBufferDescriptor{};
            offsetDataStorageBufferDescriptor.buffer = triangleOffsetBuffer;
            offsetDataStorageBufferDescriptor.offset = 0;
            offsetDataStorageBufferDescriptor.range = maxTriangleOffsetBufferSize;

            descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[5].dstSet = descriptorSet;
            descriptorWrites[5].dstBinding = 5;
            descriptorWrites[5].dstArrayElement = 0;
            descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            descriptorWrites[5].descriptorCount = 1;
            descriptorWrites[5].pBufferInfo = &offsetDataStorageBufferDescriptor;

            VkDescriptorBufferInfo indexDataStorageBufferDescriptor{};
            offsetDataStorageBufferDescriptor.buffer = indexTriangleBuffer;
            offsetDataStorageBufferDescriptor.offset = 0;
            offsetDataStorageBufferDescriptor.range = maxIndexTriangleBufferSize;

            descriptorWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[6].dstSet = descriptorSet;
            descriptorWrites[6].dstBinding = 6;
            descriptorWrites[6].dstArrayElement = 0;
            descriptorWrites[6].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            descriptorWrites[6].descriptorCount = 1;
            descriptorWrites[6].pBufferInfo = &indexDataStorageBufferDescriptor;

            VkDescriptorBufferInfo materialDataStorageBufferDescriptor{};
            materialDataStorageBufferDescriptor.buffer = materialBuffer;
            materialDataStorageBufferDescriptor.offset = 0;
            materialDataStorageBufferDescriptor.range = maxMaterialBufferSize;

            descriptorWrites[7].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[7].dstSet = descriptorSet;
            descriptorWrites[7].dstBinding = 7;
            descriptorWrites[7].dstArrayElement = 0;
            descriptorWrites[7].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            descriptorWrites[7].descriptorCount = 1;
            descriptorWrites[7].pBufferInfo = &materialDataStorageBufferDescriptor;

            vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
        void RaytracingRenderComponent::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = size;
            bufferInfo.usage = usage;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (vkCreateBuffer(vkDevice.getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
                throw std::runtime_error("failed to create buffer!");
            }

            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(vkDevice.getDevice(), buffer, &memRequirements);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

            if (vkAllocateMemory(vkDevice.getDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate buffer memory!");
            }

            vkBindBufferMemory(vkDevice.getDevice(), buffer, bufferMemory, 0);
        }
        void RaytracingRenderComponent::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool = commandPool;
            allocInfo.commandBufferCount = 1;

            VkCommandBuffer commandBuffer;
            vkAllocateCommandBuffers(vkDevice.getDevice(), &allocInfo, &commandBuffer);

            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            vkBeginCommandBuffer(commandBuffer, &beginInfo);

                VkBufferCopy copyRegion{};
                copyRegion.size = size;
                vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

            vkEndCommandBuffer(commandBuffer);

            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;

            vkQueueSubmit(vkDevice.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(vkDevice.getGraphicsQueue());

            vkFreeCommandBuffers(vkDevice.getDevice(), commandPool, 1, &commandBuffer);
        }
        void RaytracingRenderComponent::createRayTracingPipeline() {
            VkPipelineLayoutCreateInfo pipelineLayoutCI{};
            pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutCI.setLayoutCount = 1;
            pipelineLayoutCI.pSetLayouts = &descriptorSetLayout;
            if (vkCreatePipelineLayout(vkDevice.getDevice(), &pipelineLayoutCI, nullptr, &pipelineLayout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create raytracing pipeline layout!");
            }
            raytracingShader.createRaytracingShaderModules();
            std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
            VkPipelineShaderStageCreateInfo raygenShaderStageInfo{};
            raygenShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            raygenShaderStageInfo.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
            raygenShaderStageInfo.module = raytracingShader.getRaygenShaderModule();
            raygenShaderStageInfo.pName = "main";

            VkPipelineShaderStageCreateInfo raymissShaderStageInfo{};
            raymissShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            raymissShaderStageInfo.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
            raymissShaderStageInfo.module = raytracingShader.getRaymissShaderModule();
            raymissShaderStageInfo.pName = "main";

            VkPipelineShaderStageCreateInfo rayhitShaderStageInfo{};
            rayhitShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            rayhitShaderStageInfo.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
            rayhitShaderStageInfo.module = raytracingShader.getRayhitShaderModule();
            rayhitShaderStageInfo.pName = "main";

            shaderStages = {raygenShaderStageInfo, raymissShaderStageInfo, rayhitShaderStageInfo};

            {
                VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
                shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
                shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
                shaderGroup.generalShader = static_cast<uint32_t>(shaderStages.size()) - 1;
                shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
                shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
                shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
                shaderGroups.push_back(shaderGroup);
            }

            {
                VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
                shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
                shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
                shaderGroup.generalShader = static_cast<uint32_t>(shaderStages.size()) - 1;
                shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
                shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
                shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
                shaderGroups.push_back(shaderGroup);
            }

            {
                VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
                shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
                shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
                shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
                shaderGroup.closestHitShader = static_cast<uint32_t>(shaderStages.size()) - 1;
                shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
                shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
                shaderGroups.push_back(shaderGroup);
            }

			VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCI{};
            rayTracingPipelineCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
            rayTracingPipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
            rayTracingPipelineCI.pStages = shaderStages.data();
            rayTracingPipelineCI.groupCount = static_cast<uint32_t>(shaderGroups.size());
            rayTracingPipelineCI.pGroups = shaderGroups.data();
            rayTracingPipelineCI.maxPipelineRayRecursionDepth = 1;
            rayTracingPipelineCI.layout = pipelineLayout;
            if (vkCreateRayTracingPipelinesKHR(vkDevice.getDevice(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineCI, nullptr, &pipeline) != VK_SUCCESS) {
                throw std::runtime_error("failed to create raytracing pipeline!");
            }
            raytracingShader.cleanupRaytracingShaderModules();
        }
        void RaytracingRenderComponent::createUniformBuffers() {
            VkDeviceSize bufferSize = sizeof(UniformData);
            createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, ubo, uboMemory);
        }
        void RaytracingRenderComponent::updateUniformBuffers() {
            uniformData.projInverse = view.getProjMatrix().getMatrix().inverse().transpose();
		    uniformData.viewInverse = view.getViewMatrix().getMatrix().inverse().transpose();
            void* data;
            vkMapMemory(vkDevice.getDevice(), uboMemory, 0, sizeof(ubo), 0, &data);
            memcpy(data, &ubo, sizeof(ubo));
            vkUnmapMemory(vkDevice.getDevice(), uboMemory);
        }
        void RaytracingRenderComponent::drawNextFrame() {
            {
                std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                if (datasReady) {
                    datasReady = false;
                    m_instances = instanceBatcher.getInstances();
                    m_normals = normalBatcher.getInstances();
                }
            }
            if (needToUpdateDS) {
                createDescriptorSets();
            }
            updateUniformBuffers();
            createTrianglesBuffers();
            createBottomLevelAccelerationStructure();
            createTopLevelAccelerationStructure();
            std::vector<VkCommandBuffer> commandBuffers = window.getCommandBuffers();
            unsigned int currentFrame = window.getCurrentFrame();
            const uint32_t handleSizeAligned = (rayTracingPipelineProperties.shaderGroupHandleSize + rayTracingPipelineProperties.shaderGroupHandleAlignment - 1) & ~(rayTracingPipelineProperties.shaderGroupHandleAlignment - 1);
            VkStridedDeviceAddressRegionKHR raygenShaderSbtEntry{};
			raygenShaderSbtEntry.deviceAddress = getBufferDeviceAddress(raygenShaderBindingTable);
			raygenShaderSbtEntry.stride = handleSizeAligned;
			raygenShaderSbtEntry.size = handleSizeAligned;

			VkStridedDeviceAddressRegionKHR missShaderSbtEntry{};
			missShaderSbtEntry.deviceAddress = getBufferDeviceAddress(missShaderBindingTable);
			missShaderSbtEntry.stride = handleSizeAligned;
			missShaderSbtEntry.size = handleSizeAligned;

			VkStridedDeviceAddressRegionKHR hitShaderSbtEntry{};
			hitShaderSbtEntry.deviceAddress = getBufferDeviceAddress(hitShaderBindingTable);
			hitShaderSbtEntry.stride = handleSizeAligned;
			hitShaderSbtEntry.size = handleSizeAligned;

			VkStridedDeviceAddressRegionKHR callableShaderSbtEntry{};

			/*
				Dispatch the ray tracing commands
			*/
			vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
			vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipelineLayout, 0, 1, &descriptorSet, 0, 0);

			vkCmdTraceRaysKHR(
				commandBuffers[currentFrame],
				&raygenShaderSbtEntry,
				&missShaderSbtEntry,
				&hitShaderSbtEntry,
				&callableShaderSbtEntry,
				view.getSize().x(),
				view.getSize().y(),
				1);
        }
        void RaytracingRenderComponent::draw(RenderTarget& target, RenderStates states) {
            std::vector<VkCommandBuffer> commandBuffers = window.getCommandBuffers();
            unsigned int currentFrame = window.getCurrentFrame();

            {
                VkImageMemoryBarrier barrier = {};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.image = window.getSwapchainImages()[window.getImageIndex()];
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.layerCount = 1;
                vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
            }

            {
                VkImageMemoryBarrier barrier = {};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.image = storageImage.image;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.layerCount = 1;
                vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
            }

            VkImageCopy copyRegion{};
			copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
			copyRegion.srcOffset = { 0, 0, 0 };
			copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
			copyRegion.dstOffset = { 0, 0, 0 };
			copyRegion.extent = { view.getSize().x(), view.getSize().y(), 1 };
			vkCmdCopyImage(commandBuffers[currentFrame], storageImage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, window.getSwapchainImages()[window.getImageIndex()], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

			{
                VkImageMemoryBarrier barrier = {};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                barrier.image = window.getSwapchainImages()[window.getImageIndex()];
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.layerCount = 1;
                vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
			}

            {
                VkImageMemoryBarrier barrier = {};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
                barrier.image = storageImage.image;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.layerCount = 1;
                vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
            }

        }
        void RaytracingRenderComponent::clear () {
            std::vector<VkCommandBuffer> commandBuffers = window.getCommandBuffers();
            VkClearColorValue clearColor = {0.f, 0.f, 0.f, 0.f};
            VkImageSubresourceRange subresRange = {};
            subresRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subresRange.levelCount = 1;
            subresRange.layerCount = 1;
            for (unsigned int i = 0; i < commandBuffers.size(); i++) {
                vkCmdClearColorImage(commandBuffers[i], storageImage.image, VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &subresRange);
                VkMemoryBarrier memoryBarrier;
                memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                memoryBarrier.pNext = VK_NULL_HANDLE;
                memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                vkCmdPipelineBarrier(commandBuffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
            }
        }
        bool RaytracingRenderComponent::loadEntitiesOnComponent(std::vector<Entity*> vEntities) {
            vertices.clear();
            indexes.clear();
            {
                std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                datasReady = false;
                normalBatcher.clear();
                instanceBatcher.clear();
            }
            /*lights.clear();
            Light ambientLight;
            g2d::AmbientLight al = g2d::AmbientLight::getAmbientLight();
            ambientLight.center = math::Vec3f(al.getLightCenter().x, al.getLightCenter().y, al.getLightCenter().z);
            ambientLight.radius = 10000;
            ambientLight.color = math::Vec3f(al.getColor().r/255.f, al.getColor().g/255.f, al.getColor().b/255.f, al.getColor().a/255.f);
            lights.push_back(ambientLight);*/
            unsigned int nbVertices =0;
            for (unsigned int e = 0; e < vEntities.size(); e++) {
                if (vEntities[e] != nullptr && vEntities[e]->isLeaf()) {
                    if (!vEntities[e]->isLight()) {
                        for (unsigned int j = 0; j <  vEntities[e]->getNbFaces(); j++) {
                            if(vEntities[e]->getDrawMode() == Entity::INSTANCED) {
                                std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                                instanceBatcher.addFace( vEntities[e]->getFace(j));
                            } else {
                                std::lock_guard<std::recursive_mutex> lock(rec_mutex);
                                normalBatcher.addFace( vEntities[e]->getFace(j));
                            }
                            VertexArray va = vEntities[e]->getFace(j)->getVertexArray();
                            unsigned int size = 0;
                            if (va.getPrimitiveType() == PrimitiveType::Quads) {
                                size = va.getVertexCount() / 4;
                            } else if (va.getPrimitiveType() == PrimitiveType::Triangles) {
                                size = va.getVertexCount() / 3;
                            } else if (va.getPrimitiveType() == PrimitiveType::TriangleStrip || va.getPrimitiveType() == PrimitiveType::TriangleFan) {
                                size = va.getVertexCount() - 2;
                            }
                            for (unsigned int i = 0; i < size; i++) {
                                if (va.getPrimitiveType() == PrimitiveType::Quads) {
                                    for (unsigned int n = 0; n < 2; n++) {
                                        if (n == 0) {
                                            Vertex v1, v2, v3;
                                            v1.position = math::Vec3f(va[i*4].position.x(),va[i*4].position.y(),va[i*4].position.z());
                                            v2.position = math::Vec3f(va[i*4+1].position.x(),va[i*4+1].position.y(),va[i*4+1].position.z());
                                            v3.position = math::Vec3f(va[i*4+2].position.x(),va[i*4+2].position.y(),va[i*4+2].position.z());
                                            v1.colour = math::Vec4f(va[i*4].color.r / 255.f,va[i*4].color.g / 255.f,va[i*4].color.b / 255.f, va[i*4].color.a / 255.f);
                                            v2.colour = math::Vec4f(va[i*4+1].color.r / 255.f,va[i*4+1].color.g / 255.f,va[i*4+1].color.b / 255.f, va[i*4+1].color.a / 255.f);
                                            v3.colour = math::Vec4f(va[i*4+2].color.r / 255.f,va[i*4+2].color.g / 255.f,va[i*4+2].color.b / 255.f, va[i*4+2].color.a / 255.f);
                                            v1.texCoords = math::Vec2f(va[i*4].texCoords.x(), va[i*4].texCoords.y());
                                            v2.texCoords = math::Vec2f(va[i*4+1].texCoords.x(), va[i*4+1].texCoords.y());
                                            v3.texCoords = math::Vec2f(va[i*4+2].texCoords.x(), va[i*4+2].texCoords.y());


                                            math::Vec3f vt1(va[i*4].position.x(), va[i*4].position.y(), va[i*4].position.z());
                                            math::Vec3f vt2(va[i*4+1].position.x(), va[i*4+1].position.y(), va[i*4+1].position.z());
                                            math::Vec3f vt3(va[i*4+2].position.x(), va[i*4+2].position.y(), va[i*4+2].position.z());
                                            math::Vec3f d1 = vt2 - vt1;
                                            math::Vec3f d2 = vt3 - vt1;
                                            math::Vec3f n = d1.cross(d2).normalize();
                                            v1.normal = math::Vec3f(n.x(), n.y(), n.z());
                                            v2.normal = math::Vec3f(n.x(), n.y(), n.z());
                                            v3.normal = math::Vec3f(n.x(), n.y(), n.z());


                                            vertices.push_back(v1);
                                            vertices.push_back(v2);
                                            vertices.push_back(v3);
                                            indexes.push_back(nbVertices);
                                            nbVertices++;
                                            indexes.push_back(nbVertices);
                                            nbVertices++;
                                            indexes.push_back(nbVertices);
                                            nbVertices++;
                                        } else {
                                            Vertex v1, v2, v3;
                                            v1.position = math::Vec3f(va[i*4].position.x(),va[i*4].position.y(),va[i*4].position.z());
                                            v2.position = math::Vec3f(va[i*4+2].position.x(),va[i*4+2].position.y(),va[i*4+2].position.z());
                                            v3.position = math::Vec3f(va[i*4+3].position.x(),va[i*4+3].position.y(),va[i*4+3].position.z());
                                            v1.colour = math::Vec4f(va[i*4].color.r / 255.f,va[i*4].color.g / 255.f,va[i*4].color.b / 255.f, va[i*4].color.a / 255.f);
                                            v2.colour = math::Vec4f(va[i*4+2].color.r / 255.f,va[i*4+2].color.g / 255.f,va[i*4+2].color.b / 255.f, va[i*4+2].color.a / 255.f);
                                            v3.colour = math::Vec4f(va[i*4+3].color.r / 255.f,va[i*4+3].color.g / 255.f,va[i*4+3].color.b / 255.f, va[i*4+3].color.a / 255.f);
                                            v1.texCoords = math::Vec2f(va[i*4].texCoords.x(), va[i*4].texCoords.y());
                                            v2.texCoords = math::Vec2f(va[i*4+2].texCoords.x(), va[i*4+2].texCoords.y());
                                            v3.texCoords = math::Vec2f(va[i*4+3].texCoords.x(), va[i*4+3].texCoords.y());


                                            math::Vec3f vt1(va[i*4].position.x(), va[i*4].position.y(), va[i*4].position.z());
                                            math::Vec3f vt2(va[i*4+2].position.x(), va[i*4+2].position.y(), va[i*4+2].position.z());
                                            math::Vec3f vt3(va[i*4+3].position.x(), va[i*4+3].position.y(), va[i*4+3].position.z());
                                            math::Vec3f d1 = vt2 - vt1;
                                            math::Vec3f d2 = vt3 - vt1;
                                            math::Vec3f n = d1.cross(d2).normalize();
                                            v1.normal = math::Vec3f(n.x(), n.y(), n.z());
                                            v2.normal = math::Vec3f(n.x(), n.y(), n.z());
                                            v3.normal = math::Vec3f(n.x(), n.y(), n.z());


                                            vertices.push_back(v1);
                                            vertices.push_back(v2);
                                            vertices.push_back(v3);
                                            indexes.push_back(nbVertices);
                                            nbVertices++;
                                            indexes.push_back(nbVertices);
                                            nbVertices++;
                                            indexes.push_back(nbVertices);
                                            nbVertices++;
                                        }
                                    }
                                } else if (va.getPrimitiveType() == PrimitiveType::Triangles) {
                                    Vertex v1, v2, v3;
                                    v1.position = math::Vec3f(va[i*3].position.x(),va[i*3].position.y(),va[i*3].position.z());
                                    v2.position = math::Vec3f(va[i*3+1].position.x(),va[i*3+1].position.y(),va[i*3+1].position.z());
                                    v3.position = math::Vec3f(va[i*3+2].position.x(),va[i*3+2].position.y(),va[i*3+2].position.z());
                                    v1.colour = math::Vec4f(va[i*3].color.r / 255.f,va[i*3].color.g / 255.f,va[i*3].color.b / 255.f, va[i*3].color.a / 255.f);
                                    v2.colour = math::Vec4f(va[i*3+1].color.r / 255.f,va[i*3+1].color.g / 255.f,va[i*3+1].color.b / 255.f, va[i*3+1].color.a / 255.f);
                                    v3.colour = math::Vec4f(va[i*3+2].color.r / 255.f,va[i*3+2].color.g / 255.f,va[i*3+2].color.b / 255.f, va[i*3+2].color.a / 255.f);
                                    v1.texCoords = math::Vec2f(va[i*3].texCoords.x(), va[i*3].texCoords.y());
                                    v2.texCoords = math::Vec2f(va[i*3+1].texCoords.x(), va[i*3+1].texCoords.y());
                                    v3.texCoords = math::Vec2f(va[i*3+2].texCoords.x(), va[i*3+2].texCoords.y());


                                    math::Vec3f vt1(va[i*3].position.x(), va[i*3].position.y(), va[i*3].position.z());
                                    math::Vec3f vt2(va[i*3+1].position.x(), va[i*3+1].position.y(), va[i*3+1].position.z());
                                    math::Vec3f vt3(va[i*3+2].position.x(), va[i*3+2].position.y(), va[i*3+2].position.z());
                                    math::Vec3f d1 = vt2 - vt1;
                                    math::Vec3f d2 = vt3 - vt1;
                                    math::Vec3f n = d1.cross(d2).normalize();
                                    v1.normal = math::Vec3f(n.x(), n.y(), n.z());
                                    v2.normal = math::Vec3f(n.x(), n.y(), n.z());
                                    v3.normal = math::Vec3f(n.x(), n.y(), n.z());


                                    vertices.push_back(v1);
                                    vertices.push_back(v2);
                                    vertices.push_back(v3);
                                    indexes.push_back(nbVertices);
                                    nbVertices++;
                                    indexes.push_back(nbVertices);
                                    nbVertices++;
                                    indexes.push_back(nbVertices);
                                    nbVertices++;
                                } /*else if (va.getPrimitiveType() == sf::PrimitiveType::TriangleStrip) {
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
                                    }*/
                                /*} else {
                                    Light light;
                                    g2d::PonctualLight* pl = static_cast<g2d::PonctualLight*>(vEntities[e]);
                                    light.center = math::Vec3f(pl->getCenter().x, pl->getCenter().y, pl->getCenter().z);
                                    light.radius = pl->getSize().y * 0.5f;
                                    light.color = math::Vec3f(pl->getColor().r / 255.f,pl->getColor().g/255.f,pl->getColor().b/255.f,pl->getColor().a/255.f);
                                    lights.push_back(light);
                                }*/
                            }
                        }
                    }
                }
            }
            visibleEntities = vEntities;
            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
            datasReady = true;
            return true;
        }
        bool RaytracingRenderComponent::needToUpdate() {
            return update;
        }
        void RaytracingRenderComponent::setBackgroundColor(Color color) {
            backgroundColor = color;
        }
        void RaytracingRenderComponent::setExpression(std::string expression) {
            this->expression = expression;
        }
        std::string RaytracingRenderComponent::getExpression() {
            return expression;
        }
        int RaytracingRenderComponent::getLayer() {
            return layer;
        }
        void RaytracingRenderComponent::setView(View view) {
            this->view = view;
        }
        View& RaytracingRenderComponent::getView() {
            return view;
        }
        void RaytracingRenderComponent::pushEvent(window::IEvent event, RenderWindow& window) {
        }
        RenderTexture* RaytracingRenderComponent::getFrameBuffer() {
            return nullptr;
        }
        std::vector<Entity*> RaytracingRenderComponent::getEntities() {
            return visibleEntities;
        }
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
                frameBufferSprite = Sprite(frameBuffer.getTexture(), math::Vec3f(0, 0, 0), math::Vec3f(window.getView().getSize().x, window.getView().getSize().y, 0), IntRect(0, 0, window.getView().getSize().x, window.getView().getSize().y));
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
                    backgroundColor = Color::Transparent;
                    glCheck(glBindBuffer(GL_UNIFORM_BUFFER, ubo));
                    glCheck(glBufferData(GL_UNIFORM_BUFFER, sizeof(allSamplers),allSamplers.tex, GL_STATIC_DRAW));
                    glCheck(glBindBuffer(GL_UNIFORM_BUFFER, 0));
                    glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo));
                    glCheck(glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicBuffer));
                    glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, linkedListBuffer));
                    glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, trianglesSSBO));
                    glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, lightsSSBO));
            }
            void RaytracingRenderComponent::setBackgroundColor(Color color) {
                backgroundColor = color;
            }
            void RaytracingRenderComponent::clear() {
                frameBuffer.setActive();
                frameBuffer.clear(Color::Transparent);
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
                                ////////std::cout<<"triangle : "<<triangles[t].positions[0]<<triangles[t].positions[1]<<triangles[t].positions[2]<<std::endl;
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
                    Vertex v1 (math::Vec3f(0, 0, quad.getSize().z), Color::White,math::Vec2f(0, 0));
                    Vertex v2 (math::Vec3f(quad.getSize().x,0, quad.getSize().z), Color::White,math::Vec2f(quad.getSize().x, 0));
                    Vertex v3 (math::Vec3f(quad.getSize().x, quad.getSize().y, quad.getSize().z),Color::White,math::Vec2f(quad.getSize().x, quad.getSize().y));
                    Vertex v4 (math::Vec3f(0, quad.getSize().y, quad.getSize().z), Color::White,math::Vec2f(0, quad.getSize().y));
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
                ////////std::cout<<"load entities on component"<<std::endl;
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
                                                ////////std::cout<<"texture index : "<<triangle.textureIndex<<std::endl;
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
                                                    //////std::cout<<va[i*4+v+1].position.x<<","<<va[i*4+v+1].position.y<<","<<va[i*4+v+1].position.z<<std::endl;
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
                    //////std::cout<<"recompute size"<<std::endl;
                    recomputeSize();
                    getListener().pushEvent(event);
                    getView().reset(physic::BoundingBox(getView().getViewport().getPosition().x, getView().getViewport().getPosition().y, getView().getViewport().getPosition().z, event.window.data1, event.window.data2, getView().getViewport().getDepth()));
                }
            }
            const Texture& RaytracingRenderComponent::getFrameBufferTexture() {
                return frameBuffer.getTexture();
            }
            RenderTexture* RaytracingRenderComponent::getFrameBuffer() {
                return &frameBuffer;
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
