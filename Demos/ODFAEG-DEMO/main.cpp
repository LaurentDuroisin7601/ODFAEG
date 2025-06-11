#include "application.h"
#include "odfaeg/Graphics/graphics.hpp"
#include "odfaeg/Audio/audio.hpp"
#include "odfaeg/Math/math.hpp"
#include "hero.hpp"


using namespace odfaeg::core;
using namespace odfaeg::math;
using namespace odfaeg::physic;
using namespace odfaeg::graphic;
using namespace odfaeg::window;
using namespace odfaeg::audio;
using namespace sorrok;
#include "odfaeg/Graphics/renderWindow.h"
#include "odfaeg/Graphics/font.h"
#include "odfaeg/Graphics/text.h"
#include "odfaeg/Graphics/sprite.h"
#include "odfaeg/Window/iEvent.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
struct UniformBufferObject {
    Matrix4f model;
    Matrix4f view;
    Matrix4f proj;
};
void compileShaders(Shader& sLinkedList) {
    const std::string linkedListIndirectRenderingVertexShader = R"(#version 460
                                                               #extension GL_EXT_multiview : enable
                                                               #extension GL_EXT_debug_printf : enable
                                                               layout (location = 0) in vec3 position;
                                                               layout (location = 1) in vec4 color;
                                                               layout (location = 2) in vec2 texCoords;
                                                               layout (location = 3) in vec3 normals;
                                                               layout(binding = 0) uniform UniformBufferObject {
                                                                    mat4 model;
                                                                    mat4 view;
                                                                    mat4 proj;
                                                               } ubo;
                                                               layout (location = 0) out vec4 frontColor;
                                                               layout (location = 1) out vec2 fTexCoords;
                                                               layout (location = 2) out vec3 normal;
                                                               layout (location = 3) out flat int viewIndex;
                                                               void main() {
                                                                    gl_Position = (vec4(position, 1.f) * ubo.model * ubo.view * ubo.proj);
                                                                    gl_Position.z = 0;
                                                                    fTexCoords = texCoords;
                                                                    frontColor = color;
                                                                    normal = normals;
                                                                    viewIndex = gl_ViewIndex;
                                                               }
                                                               )";

    const std::string fragmentShader = R"(#version 460
                                          #extension GL_EXT_nonuniform_qualifier : enable
                                          #extension GL_EXT_debug_printf : enable
                                          layout(set = 0, binding = 1, r32ui) uniform coherent uimage3D headPointers;

                                          layout (location = 0) in vec4 frontColor;
                                          layout (location = 1) in vec2 fTexCoords;
                                          layout (location = 2) in vec3 normal;
                                          layout (location = 3) in flat int viewIndex;
                                          layout(location = 0) out vec4 fcolor;
                                          void main() {

                                               uint prevHead = imageLoad(headPointers, ivec3(gl_FragCoord.xy, viewIndex)).r;
                                               if (prevHead == 0)
                                                    debugPrintfEXT("prevHead: %i, view index : %i\n", prevHead, viewIndex);
                                               fcolor = frontColor;
                                          })";
    if (!sLinkedList.loadFromMemory(linkedListIndirectRenderingVertexShader, fragmentShader)) {
        throw Erreur(58, "Error, failed to load per pixel linked list shader", 3);
    }
}
void createDescriptorPoolLinkedList(Device& vkDevice, Shader& shader, RenderTarget& target) {
    std::vector<VkDescriptorPool>& descriptorPool = target.getDescriptorPool();
    descriptorPool.resize(Shader::getNbShaders()*RenderTarget::getNbRenderTargets());
    std::array<VkDescriptorPoolSize, 2> poolSizes;
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 1;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSizes[1].descriptorCount = 1;
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 1;
    unsigned int descriptorId = target.getId() * Shader::getNbShaders() + shader.getId();
    if (vkCreateDescriptorPool(vkDevice.getDevice(), &poolInfo, nullptr, &descriptorPool[descriptorId]) != VK_SUCCESS) {
        throw std::runtime_error("echec de la creation de la pool de descripteurs!");
    }
}
void createDescriptorLayoutLinkedList(Device& vkDevice,Shader& shader, RenderTarget& target) {
    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = target.getDescriptorSetLayout();
    descriptorSetLayout.resize(Shader::getNbShaders()*RenderTarget::getNbRenderTargets());
    VkDescriptorSetLayoutBinding uniformBufferLayoutBinding;
    uniformBufferLayoutBinding.binding = 0;
    uniformBufferLayoutBinding.descriptorCount = 1;
    uniformBufferLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformBufferLayoutBinding.pImmutableSamplers = nullptr;
    uniformBufferLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding headPtrImageLayoutBinding;
    headPtrImageLayoutBinding.binding = 1;
    headPtrImageLayoutBinding.descriptorCount = 1;
    headPtrImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    headPtrImageLayoutBinding.pImmutableSamplers = nullptr;
    headPtrImageLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uniformBufferLayoutBinding, headPtrImageLayoutBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    //layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());;
    layoutInfo.pBindings = bindings.data();
    unsigned int descriptorId = target.getId() * Shader::getNbShaders() + shader.getId();
    if (vkCreateDescriptorSetLayout(vkDevice.getDevice(), &layoutInfo, nullptr, &descriptorSetLayout[descriptorId]) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}
void allocateDescriptorSets(Device& vkDevice, Shader& shader, RenderTarget &target) {
    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = target.getDescriptorSet();
    std::vector<VkDescriptorSetLayout>& descriptorSetLayout = target.getDescriptorSetLayout();
    std::vector<VkDescriptorPool>& descriptorPool = target.getDescriptorPool();
    descriptorSets.resize(Shader::getNbShaders()*RenderTarget::getNbRenderTargets());
    unsigned int descriptorId = target.getId() * Shader::getNbShaders() + shader.getId();
    for (unsigned int i = 0; i < descriptorSets.size(); i++) {
        descriptorSets[i].resize(1);
    }
    std::vector<VkDescriptorSetLayout> layouts(1, descriptorSetLayout[descriptorId]);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool[descriptorId];
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = layouts.data();
    if (vkAllocateDescriptorSets(vkDevice.getDevice(), &allocInfo, descriptorSets[descriptorId].data()) != VK_SUCCESS) {
        throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
    }
}
void createDescriptorSets(Device& vkDevice, Shader& shader, RenderTarget& target, VkImage& headPtrTextureImage, VkImageView& headPtrTextureImageView, VkSampler& headPtrTextureSampler, VkBuffer ubo) {
    unsigned int descriptorId = target.getId() * Shader::getNbShaders() + shader.getId();
    std::vector<std::vector<VkDescriptorSet>>& descriptorSets = target.getDescriptorSet();
    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = ubo;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSets[descriptorId][0];
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;

    VkDescriptorImageInfo headPtrDescriptorImageInfo;
    headPtrDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    headPtrDescriptorImageInfo.imageView = headPtrTextureImageView;
    headPtrDescriptorImageInfo.sampler = headPtrTextureSampler;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = descriptorSets[descriptorId][0];
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &headPtrDescriptorImageInfo;
    vkUpdateDescriptorSets(vkDevice.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}
uint32_t findMemoryType(Device& vkDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(vkDevice.getPhysicalDevice(), &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("aucun type de memoire ne satisfait le buffer!");
}
void createBuffer(Device& vkDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
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
    allocInfo.memoryTypeIndex = findMemoryType(vkDevice, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(vkDevice.getDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(vkDevice.getDevice(), buffer, bufferMemory, 0);
}
int main(int argc, char *argv[]) {
    /*VkSettup instance;
    Device device(instance);
    RenderWindow window(sf::VideoMode(800, 600), "test vulkan", device);
    //window.getView().move(400, 300, 0);
    RenderTexture rtCubeMap(device);
    rtCubeMap.createCubeMap(800, 800);
    RectangleShape quad(Vec3f(window.getView().getSize().x, window.getView().getSize().y, window.getSize().y * 0.5f));
    quad.move(Vec3f(-window.getView().getSize().x * 0.5f, -window.getView().getSize().y * 0.5f, 0));
    VertexBuffer vb(device);
    vb.setPrimitiveType(sf::Triangles);
    Vertex v1 (sf::Vector3f(0, 0, quad.getSize().z), sf::Color::Red);
    Vertex v2 (sf::Vector3f(quad.getSize().x,0, quad.getSize().z), sf::Color::Red);
    Vertex v3 (sf::Vector3f(quad.getSize().x, quad.getSize().y, quad.getSize().z), sf::Color::Red);
    Vertex v4 (sf::Vector3f(0, quad.getSize().y, quad.getSize().z), sf::Color::Red);
    vb.append(v1);
    vb.append(v2);
    vb.append(v3);
    vb.append(v1);
    vb.append(v3);
    vb.append(v4);
    vb.update();


    VkImage headPtrTextureImage;
    VkImageCreateInfo imageInfo={};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_3D;
    imageInfo.extent.width = static_cast<uint32_t>(window.getView().getSize().x);
    imageInfo.extent.height = static_cast<uint32_t>(window.getView().getSize().y);
    imageInfo.extent.depth = 6;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R32_UINT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0; // Optionnel
    Shader linkedListShader(device);
    compileShaders(linkedListShader);

    if (vkCreateImage(device.getDevice(), &imageInfo, nullptr, &headPtrTextureImage) != VK_SUCCESS) {
        throw std::runtime_error("echec de la creation d'une image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(window.getDevice().getDevice(), headPtrTextureImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo={};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(device, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VkDeviceMemory headPtrTextureImageMemory;

    if (vkAllocateMemory(device.getDevice(), &allocInfo, nullptr, &headPtrTextureImageMemory) != VK_SUCCESS) {
        throw std::runtime_error("echec de l'allocation de la memoire d'une image!");
    }
    vkBindImageMemory(device.getDevice(), headPtrTextureImage, headPtrTextureImageMemory, 0);
    VkImageView headPtrTextureImageView;
    VkImageViewCreateInfo viewInfo={};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = headPtrTextureImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
    viewInfo.format = VK_FORMAT_R32_UINT;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    if (vkCreateImageView(device.getDevice(), &viewInfo, nullptr, &headPtrTextureImageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create head ptr texture image view!");
    }
    VkSampler headPtrTextureSampler;
    VkSamplerCreateInfo samplerInfo={};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(device.getPhysicalDevice(), &properties);
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    //const_cast<Texture&>(rtCubeMap.getTexture()).toShaderReadOnlyOptimal();
    rtCubeMap.beginRecordCommandBuffers();
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.image = headPtrTextureImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.layerCount = 1;
    vkCmdPipelineBarrier(rtCubeMap.getCommandBuffers()[0], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    //
    rtCubeMap.beginRenderPass();
    rtCubeMap.display();
    //const_cast<Texture&>(rtCubeMap.getTexture()).toColorAttachmentOptimal();
    //
    VkBuffer ubo;
    VkDeviceMemory uboMemory;
    createBuffer(device, sizeof(UniformBufferObject) , VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, ubo, uboMemory);
    UniformBufferObject uboData;
    uboData.view = window.getDefaultView().getViewMatrix().getMatrix();
    uboData.proj = window.getDefaultView().getProjMatrix().getMatrix();
    uboData.model = quad.getTransform().getMatrix();
    void* data;
    vkMapMemory(device.getDevice(), uboMemory, 0, sizeof(UniformBufferObject), 0, &data);
    memcpy(data, &uboData, sizeof(UniformBufferObject));
    vkUnmapMemory(device.getDevice(), uboMemory);
    createDescriptorPoolLinkedList(device, linkedListShader, rtCubeMap);
    createDescriptorLayoutLinkedList(device, linkedListShader, rtCubeMap);
    allocateDescriptorSets(device, linkedListShader, rtCubeMap);
    createDescriptorSets(device, linkedListShader, rtCubeMap, headPtrTextureImage, headPtrTextureImageView, headPtrTextureSampler, ubo);
    if (vkCreateSampler(device.getDevice(), &samplerInfo, nullptr, &headPtrTextureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
    RenderStates states;
    states.shader = &linkedListShader;
    rtCubeMap.createGraphicPipeline(sf::Triangles, states);


    while (window.isOpen()) {
        IEvent event;
        while (window.pollEvent(event)) {
            if (event.type == IEvent::WINDOW_EVENT && event.window.type == IEvent::WINDOW_EVENT_CLOSED) {
                window.close();
            }
        }
        rtCubeMap.clear(sf::Color::Transparent);
        rtCubeMap.display();
        rtCubeMap.beginRecordCommandBuffers();
        VkClearColorValue clearColor;
        clearColor.uint32[0] = 0xffffffff;
        VkImageSubresourceRange subresRange = {};
        subresRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresRange.levelCount = 1;
        subresRange.layerCount = 1;
        //transitionImageLayout(headPtrTextureImage, VK_FORMAT_R32_UINT, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        vkCmdClearColorImage(rtCubeMap.getCommandBuffers()[0], headPtrTextureImage, VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &subresRange);
        VkImageMemoryBarrier barrier2 = {};

        barrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier2.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier2.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        barrier2.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        barrier2.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        barrier2.image = headPtrTextureImage;
        barrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier2.subresourceRange.levelCount = 1;
        barrier2.subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(rtCubeMap.getCommandBuffers()[0], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier2);
        rtCubeMap.beginRenderPass();
        rtCubeMap.display(true, VK_PIPELINE_STAGE_2_CLEAR_BIT);
        unsigned int descriptorId = rtCubeMap.getId() * Shader::getNbShaders() + linkedListShader.getId();
        rtCubeMap.beginRecordCommandBuffers();
        rtCubeMap.beginRenderPass();
        states.shader = &linkedListShader;
        rtCubeMap.drawVertexBuffer(rtCubeMap.getCommandBuffers()[0], 0, vb, 0, states);
        rtCubeMap.display(false, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);
        window.clear();
        states.shader = nullptr;
        states.transform = quad.getTransform();
        window.draw(vb, states);
        window.display();
    }
    vkDestroyBuffer(device.getDevice(), ubo, nullptr);

    return 0;*/
    MyAppli app(sf::VideoMode(800, 600), "Test odfaeg");
    return app.exec();
}




