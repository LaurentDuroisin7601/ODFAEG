module;
#include <iostream>
#include <vulkan/vulkan.hpp>
#include <mutex>
#include <gli.hpp>
#include <vk_mem_alloc.h>
//import odfaeg.graphic.texture;
module odfaeg.graphic.texture;
import odfaeg.graphic.buffer;
import odfaeg.graphic.gpuContext;
namespace
{
    std::mutex idMutex;
    // Thread-safe unique identifier generator,
    // is used for states cache (see RenderTarget)
    std::uint64_t getUniqueId()
    {
        std::lock_guard<std::mutex> lock(idMutex);

        static std::uint64_t id = 1; // start at 1, zero is "no texture"

        return id++;
    }
}
namespace odfaeg {
	namespace graphic {
        Texture::Texture(Device& device, unsigned int nbBuffers) : device(device), texType(0), nbBuffers(nbBuffers), m_Smooth(false), m_Repeated(false), m_size(0u, 0u), commandPool(device), id(0), unormalized(false), isFBOTexture(false) {
            images.reserve(nbBuffers);            
            for (unsigned int i = 0; i < nbBuffers; i++) {
                images.emplace_back(device);
            }
            id = 0;
            wrapU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            wrapV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            mipLevels = 1;
            layerCount = 1;
        }
	    void Texture::setSize(math::Vector2u size) {
            m_size = size;
        }
        void Texture::setTexType(unsigned int texType) {
            this->texType = texType;
        }
        void Texture::createCommandBuffers() {
            Device::QueueFamilyIndices indices = device.findQueueFamilies(device.getPhysicalDevice());
            commandPool.create(indices.graphicsFamily.value());
            commandPool.createCommandBuffers(true, nbBuffers);
        }
	    void Texture::copyFrom(Texture& texture) {
            nbBuffers = texture.nbBuffers;
            m_format = texture.m_format;
            id = texture.id;
            unormalized = texture.unormalized;
            m_size = texture.m_size;
            imageAspectMask = texture.imageAspectMask;
            wrapU = texture.wrapU;
            wrapV = texture.wrapV;
            texType = texture.texType;
            mipLevels = texture.mipLevels;
            mipsInfos = texture.mipsInfos;
            layerCount = texture.layerCount;
            create(texture.m_size.x(), texture.m_size.y(), 1, texture.mipLevels);
            update(texture, 0, 0);
        }
	    void Texture::copyFrom(CommandPool& commandPool, Texture& texture) {
            nbBuffers = texture.nbBuffers;
            m_format = texture.m_format;
            id = texture.id;
            unormalized = texture.unormalized;
            m_size = texture.m_size;
            imageAspectMask = texture.imageAspectMask;
            wrapU = texture.wrapU;
            wrapV = texture.wrapV;
            texType = texture.texType;
            mipLevels = texture.mipLevels;
            layerCount = texture.layerCount;
            mipsInfos = texture.mipsInfos;
            create(texture.m_size.x(), texture.m_size.y(), 1, texture.mipLevels);
            for (unsigned int i = 0; i < mipLevels; i++) {
                update(commandPool, texture, 0, 0, i);
            }
        }
        Texture::Texture(Texture&& other) noexcept : device(other.device), commandPool(other.device) {
            nbBuffers = other.nbBuffers;
            images = std::move(other.images);
            m_format = other.m_format;
            id = other.id;
            unormalized = other.unormalized;
            m_size = other.m_size;
            imageAspectMask = other.imageAspectMask;
            wrapU = other.wrapU;
            wrapV = other.wrapV;
            texType = other.texType;
            mipLevels = other.mipLevels;
            layerCount = other.layerCount;
            mipsInfos = other.mipsInfos;
        }
        Texture& Texture::operator= (Texture&& other) noexcept {
            if (this != &other) {
                nbBuffers = other.nbBuffers;
                images = std::move(other.images);
                id = other.id;
                m_format = other.m_format;
                unormalized = other.unormalized;
                m_size = other.m_size;
                imageAspectMask = other.imageAspectMask;
                wrapU = other.wrapU;
                wrapV = other.wrapV;
                texType = other.texType;
                mipLevels = other.mipLevels;
                layerCount = other.layerCount;
                mipsInfos = other.mipsInfos;
            }
            return *this;
        }
        unsigned int Texture::getNbBuffers() const {
            return nbBuffers;
        }
	    void Texture::setFormat(VkFormat format) {
            m_format = format;
        }
	    void Texture::setSamplerAddressMode(VkSamplerAddressMode wrapU, VkSamplerAddressMode wrapV) {
            this->wrapU = wrapU;
            this->wrapV = wrapV;
            if (GPUContext::instance().getSharedTextures(0).size() > 0) {
                for (unsigned int i = 0; i < GPUContext::instance().getSharedTextures(0).size(); i++) {
                    for (unsigned int j = 0; j < GPUContext::instance().getSharedTextures(0)[i].images.size(); j++) {
                        GPUContext::instance().getSharedTextures(0)[i].images[j].createSampler(wrapU, wrapV, GPUContext::instance().getSharedTextures(0)[i].mipLevels, GPUContext::instance().getSharedTextures(0)[i].m_Smooth, GPUContext::instance().getSharedTextures(0)[i].unormalized);
                    }
                }
            }                      
        }
	    unsigned int Texture::getLayerCount() {
            return layerCount;
        }
        bool Texture::create(uint32_t texWidth, uint32_t texHeight, uint32_t texDepth, unsigned int mipLevels, bool layered, bool FBOAttachment) {
            //std::cout<<"fbo attachment ?"<<FBOAttachment<<std::endl;
            m_size = math::Vector2u(texWidth, texHeight);
            this->mipLevels = mipLevels;
            layerCount = (layered) ? texDepth : 1;
            //std::cout<<"create format : "<<m_format<<std::endl;
            id = getUniqueId();
            bool isCompressed =
                m_format == VK_FORMAT_BC1_RGBA_UNORM_BLOCK ||
                m_format == VK_FORMAT_BC1_RGBA_SRGB_BLOCK ||
                m_format == VK_FORMAT_BC3_UNORM_BLOCK ||
                m_format == VK_FORMAT_BC3_SRGB_BLOCK ||
                m_format == VK_FORMAT_BC4_UNORM_BLOCK ||
                m_format == VK_FORMAT_BC4_SNORM_BLOCK ||
                m_format == VK_FORMAT_BC5_UNORM_BLOCK ||
                m_format == VK_FORMAT_BC5_SNORM_BLOCK ||
                m_format == VK_FORMAT_BC6H_UFLOAT_BLOCK ||
                m_format == VK_FORMAT_BC6H_SFLOAT_BLOCK ||
                m_format == VK_FORMAT_BC7_UNORM_BLOCK ||
                m_format == VK_FORMAT_BC7_SRGB_BLOCK;
            VkImageType imageType;
            VkImageViewType viewType;
            //std::cout<<"tex height : "<<texHeight<<std::endl;
            if (isCompressed) {
                imageType = VK_IMAGE_TYPE_2D;
                viewType  = layered ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
            }
            else {
                if (texHeight > 1) {
                    if (!layered) {
                        if (texDepth > 1) {
                            imageType = VK_IMAGE_TYPE_3D;
                        } else {
                            //std::cout<<"image 2D"<<std::endl;
                            imageType = VK_IMAGE_TYPE_2D;
                        }
                    } else {
                        if (texDepth > 1) {
                            imageType = VK_IMAGE_TYPE_2D;
                        } else {
                            imageType = VK_IMAGE_TYPE_1D;
                        }
                    }
                } else {
                    //std::cout<<"image 1d"<<std::endl;
                    imageType = VK_IMAGE_TYPE_1D;
                }
                if (texHeight > 1) {
                    if (!layered) {
                        if (texDepth > 1) {
                            viewType = VK_IMAGE_VIEW_TYPE_3D;
                        }
                        else {
                            //std::cout<<"view 2d"<<std::endl;
                            viewType = VK_IMAGE_VIEW_TYPE_2D;
                        }
                    } else {
                        if (texDepth > 1) {
                            viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                        }
                        else {
                            viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
                        }
                    }
                }  else {
                    //std::cout<<"view 1d"<<std::endl;
                    viewType = VK_IMAGE_VIEW_TYPE_1D;
                }
            }
            createCommandBuffers();
            imageAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            for (unsigned int i = 0; i < nbBuffers; i++) {
                images[i].create(texWidth, texHeight, (layered) ? 1 : texDepth, imageType, m_format, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VMA_MEMORY_USAGE_GPU_ONLY, mipLevels, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL);
                images[i].createImageView(viewType, m_format, VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, mipLevels, (layered) ? texDepth : 1);
                images[i].createSampler(wrapU, wrapV, mipLevels, m_Smooth, unormalized);
                if (FBOAttachment) {
                    m_format = VK_FORMAT_R8G8B8A8_UNORM;
                    commandPool.beginRecordCommandBuffer(i);
                    transitionImageLayout(images[i], commandPool.getHandle(i), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
                    commandPool.beginRecordCommandBuffer(i);                   
                }
                
            }            
            if (FBOAttachment) {
                VkSubmitInfo submitInfo{};
                submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submitInfo.commandBufferCount = commandPool.getHandles().size();
                submitInfo.pCommandBuffers = commandPool.getHandles().data();
                Device::QueueFamilyIndices indices = device.findQueueFamilies(device.getPhysicalDevice(), VK_NULL_HANDLE);
                if (vkQueueSubmit(device.getQueue(indices.graphicsFamily.value(), 0), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
                    throw std::runtime_error("�chec de l'envoi d'un command buffer!");
                }
                vkDeviceWaitIdle(device.getDevice());
                GPUContext::instance().getSharedTextures(0).push_back(std::move(*this));
            }
            //std::cout<<"image created!"<<std::endl;
            isFBOTexture = FBOAttachment;
            return true;
        }
        bool Texture::createDepthTexture(uint32_t texWidth, uint32_t texHeight, uint32_t texDepth, bool layered) {
            
            VkImageType imageType;
            VkImageViewType viewType;
            layerCount = (layered) ? texDepth : 1;
            if (texHeight > 1) {
                if (!layered) {
                    if (texDepth > 1) {
                        imageType = VK_IMAGE_TYPE_3D;
                    } else {
                        //std::cout<<"image 2D"<<std::endl;
                        imageType = VK_IMAGE_TYPE_2D;
                    }
                } else {
                    if (texDepth > 1) {
                        imageType = VK_IMAGE_TYPE_2D;
                    } else {
                        imageType = VK_IMAGE_TYPE_1D;
                    }
                }
            } else {
                //std::cout<<"image 1d"<<std::endl;
                imageType = VK_IMAGE_TYPE_1D;
            }
            if (texHeight > 1) {
                if (!layered) {
                    if (texDepth > 1) {
                        viewType = VK_IMAGE_VIEW_TYPE_3D;
                    }
                    else {
                        //std::cout<<"view 2d"<<std::endl;
                        viewType = VK_IMAGE_VIEW_TYPE_2D;
                    }
                } else {
                    if (texDepth > 1) {
                        viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                    }
                    else {
                        viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
                    }
                }
            }  else {
                //std::cout<<"view 1d"<<std::endl;
                viewType = VK_IMAGE_VIEW_TYPE_1D;
            }
            m_size = math::Vector2u(texWidth, texHeight);
            m_format = findDepthFormat();
            imageAspectMask = VK_IMAGE_ASPECT_DEPTH_BIT /*| VK_IMAGE_ASPECT_STENCIL_BIT*/;
            createCommandBuffers(); 
            /*if (layered) {
                std::cout<<"layered ? "<<layered<<","<<texDepth<<std::endl;
                int i;
                std::cin>>i;    
            }  */  
            //std::cout<<"nb buffers : "<<nbBuffers<<std::endl;         
            for (unsigned int i = 0; i < nbBuffers; i++) {  
                images[i].create(texWidth, texHeight, 1, imageType, m_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VMA_MEMORY_USAGE_GPU_ONLY, 1, (layered) ? texDepth : 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL);
                images[i].createImageView(viewType, m_format, VK_IMAGE_ASPECT_DEPTH_BIT /*| VK_IMAGE_ASPECT_STENCIL_BIT*/, 0, 0, 1, (layered) ? texDepth : 1);
                images[i].createSampler(wrapU, wrapV, mipLevels, m_Smooth, unormalized);
                commandPool.beginRecordCommandBuffer(i);
                transitionImageLayout(images[i], commandPool.getHandle(i), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 0, 0, 1, (layered) ? texDepth : 1);                
                commandPool.endRecordCommandBuffer(i);                
            }             
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = commandPool.getHandles().size();
            submitInfo.pCommandBuffers = commandPool.getHandles().data();
            Device::QueueFamilyIndices indices = device.findQueueFamilies(device.getPhysicalDevice(), VK_NULL_HANDLE);
            /*std::cout << "submitInfo.sType = " << submitInfo.sType << std::endl;
            std::cout << "submitInfo.pNext = " << submitInfo.pNext << std::endl;
            std::cout << "submitInfo.waitSemaphoreCount = " << submitInfo.waitSemaphoreCount << std::endl;
            std::cout << "submitInfo.pWaitSemaphores = " << submitInfo.pWaitSemaphores << std::endl;
            std::cout << "submitInfo.pWaitDstStageMask = " << submitInfo.pWaitDstStageMask << std::endl;
            std::cout << "submitInfo.commandBufferCount = " << submitInfo.commandBufferCount << std::endl;
            std::cout << "submitInfo.pCommandBuffers = " << submitInfo.pCommandBuffers << std::endl;
            std::cout << "submitInfo.signalSemaphoreCount = " << submitInfo.signalSemaphoreCount << std::endl;
            std::cout << "submitInfo.pSignalSemaphores = " << submitInfo.pSignalSemaphores << std::endl;
            std::cout<<"device : "<<device.getDevice()<<std::endl;
            std::cout<<"command : "<< commandPool.getHandle(0)<<std::endl;
            std::cout<<"queue family = "<<indices.graphicsFamily.value()<<std::endl;*/
            if (vkQueueSubmit(device.getQueue(indices.graphicsFamily.value(), 0), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
                throw std::runtime_error("�chec de l'envoi d'un command buffer!");
            }
            vkDeviceWaitIdle(device.getDevice());
            /*if (layered)
                std::cout<<"transition done : "<<std::endl;*/
            return true;
        }
        bool Texture::createCubeMap(uint32_t size, bool layered, bool FBOAttachment) {
            id = getUniqueId();
            m_size = math::Vector2u(size, size);
            layerCount = 6;
            VkImageType imageType = VK_IMAGE_TYPE_2D; 
            VkImageViewType viewType;            
            if (!layered) {
                viewType = VK_IMAGE_VIEW_TYPE_CUBE;
            }
            else {
                viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
            }  
            imageAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createCommandBuffers();
            for (unsigned int i = 0; i < nbBuffers; i++) {
                images[i].create(size, size, 1, imageType, m_format, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                    VMA_MEMORY_USAGE_GPU_ONLY, 1, 6, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
                images[i].createImageView(viewType, m_format, VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1, 6);
                
                images[i].createSampler(wrapU, wrapV, mipLevels, m_Smooth, unormalized);
                if (FBOAttachment) {
                    m_format = VK_FORMAT_R8G8B8A8_UNORM;
                    commandPool.beginRecordCommandBuffer(i);
                    transitionImageLayout(images[i], commandPool.getHandle(i), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0, 0, 1, 6);
                    commandPool.endRecordCommandBuffer(i);
                }                
            }
            if (FBOAttachment) {
                VkSubmitInfo submitInfo{};
                submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submitInfo.commandBufferCount = commandPool.getHandles().size();
                submitInfo.pCommandBuffers = commandPool.getHandles().data();
                Device::QueueFamilyIndices indices = device.findQueueFamilies(device.getPhysicalDevice(), VK_NULL_HANDLE);
                if (vkQueueSubmit(device.getQueue(indices.graphicsFamily.value(), 0), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
                    throw std::runtime_error("�chec de l'envoi d'un command buffer!");
                }
                vkDeviceWaitIdle(device.getDevice());
                //id = getUniqueId();
                GPUContext::instance().getSharedTextures(0).push_back(std::move(*this));
            }
            isFBOTexture = FBOAttachment;
            return true;
        }
        bool Texture::createDepthCubeMap(uint32_t size, bool layered) {
            m_size = math::Vector2u(size, size);
            VkFormat depthFormat = findDepthFormat();
            VkImageType imageType = VK_IMAGE_TYPE_2D; 
            m_format = depthFormat;
            imageAspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            VkImageViewType viewType; 
            layerCount = 6;           
            if (!layered) {
                viewType = VK_IMAGE_VIEW_TYPE_CUBE;
            }
            else {
                viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
            }  
            imageAspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            createCommandBuffers();
            for (unsigned int i = 0; i < nbBuffers; i++) {
                images[i].create(size, size, 1, imageType, m_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT| VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VMA_MEMORY_USAGE_GPU_ONLY, 1, 6, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
                images[i].createImageView(viewType, m_format, VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1, 6);
                images[i].createSampler(wrapU, wrapV, mipLevels, m_Smooth, unormalized);
                commandPool.beginRecordCommandBuffer(i);
                transitionImageLayout(images[i], commandPool.getHandle(i), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 0, 0, 1, 6);                
                commandPool.endRecordCommandBuffer(i);
            }            
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = commandPool.getHandles().size();
            submitInfo.pCommandBuffers = commandPool.getHandles().data();
            Device::QueueFamilyIndices indices = device.findQueueFamilies(device.getPhysicalDevice(), VK_NULL_HANDLE);
            if (vkQueueSubmit(device.getQueue(indices.graphicsFamily.value(), 0), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
                throw std::runtime_error("�chec de l'envoi d'un command buffer!");
            }
            vkDeviceWaitIdle(device.getDevice());
            return true;
        }
        bool Texture::loadCubeMapFromFile(std::vector<std::string> filenames, const IntRect& area) {

            createCubeMap(imageLoader.getSize().x());
            for (unsigned int i = 0; i < nbBuffers; i++) {
                commandPool.beginRecordCommandBuffer(i);
                transitionImageLayout(images[i], commandPool.getHandle(i), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 6);
            }            
            for (unsigned int i = 0; i < 6; i++) {
                ImageLoader imageLoader;
                if (!imageLoader.loadFromFile(filenames[i]) || !loadCubeMapFromImage(imageLoader, area, i))
                    return false;
            }
            for (unsigned int i = 0; i < nbBuffers; i++) {
                transitionImageLayout(images[i], commandPool.getHandle(i), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 6);
                commandPool.endRecordCommandBuffer(i);
            }
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = commandPool.getHandles().size();
            submitInfo.pCommandBuffers = commandPool.getHandles().data();
            Device::QueueFamilyIndices indices = device.findQueueFamilies(device.getPhysicalDevice(), VK_NULL_HANDLE);
            if (vkQueueSubmit(device.getQueue(indices.graphicsFamily.value(), 0), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {                
                throw std::runtime_error("�chec de l'envoi d'un command buffer!");
            }
            vkDeviceWaitIdle(device.getDevice());
            //id = getUniqueId();
            GPUContext::instance().getSharedTextures(0).push_back(std::move(*this));

            return true;
        }
        bool Texture::loadCubeMapFromImage(const ImageLoader& image, const IntRect& area, uint32_t face) {
            if (imageLoader.isCompressed()) {
                m_format = image.getVkFormat();
            }
            else {
                m_format = VK_FORMAT_R8G8B8A8_SRGB;
            }
            const std::uint8_t* pixels = image.getPixelsPtr();
            int texWidth = image.getSize().x();
            int texHeight = image.getSize().y();
            updateCubeMap(pixels, texWidth, texHeight, 0, 0, face);            
            return true;
        }
        bool Texture::loadFromFile(const std::string& filename, const IntRect& area) {
            ImageLoader imageLoader;
            return  imageLoader.loadFromFile(filename) && loadFromImage(imageLoader, area);
        }
	    bool Texture::loadFromMemory(const void* data, std::size_t size, const IntRect& area) {
            ImageLoader imageLoader;
            return imageLoader.loadFromMemory(data, size) && loadFromImage(imageLoader, area);
        }
        bool Texture::loadFromImage(const ImageLoader& imageLoader, const IntRect& area) {
            if (imageLoader.isCompressed()) {

                m_format = imageLoader.getVkFormat();
                //std::cout<<"loaded from gli, vulkan format : "<<m_format<<std::endl;
            }
            else {
                //std::cout<<"load from image format : "<<m_format<<std::endl;
                m_format = VK_FORMAT_R8G8B8A8_SRGB;
            }
            m_DataSize = imageLoader.getDataSize();
            create(imageLoader.getSize().x(), imageLoader.getSize().y());

            //std::cout<<"data size : "<<m_DataSize<<std::endl;

            update(imageLoader.getPixelsPtr(), imageLoader.getSize().x(), imageLoader.getSize().y(), 0, 0);
            //std::cout<<"texture : "<<this<<std::endl;


            //std::cout<<"id  : "<<id<<std::endl;
            
            return true;
        }
        void Texture::updateCubeMap(const std::uint8_t* pixels, unsigned int texWidth, unsigned int texHeight, unsigned int x, unsigned int y, uint32_t face) {
            VkDeviceSize imageSize = texWidth * texHeight * 4;
            if (!pixels) {
                throw std::runtime_error("�chec du chargement d'une image!");
            }
            Buffer staggingBuffer(device);

            staggingBuffer.create(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
            staggingBuffer.update(pixels, imageSize);
            for (unsigned int i = 0; i < nbBuffers; i++) {
                images[i].copyBufferToImage(commandPool.getHandle(i), staggingBuffer, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), static_cast<uint32_t>(x), static_cast<uint32_t>(y), face);
            }  
        }
        void Texture::update(const std::uint8_t* pixels, unsigned int texWidth, unsigned int texHeight, unsigned int x, unsigned int y) {
            VkDeviceSize imageSize = m_DataSize;
            if (!pixels) {
                throw std::runtime_error("Echec du chargement d'une image!");
            }
            /*std::cout<<"data size : "<<imageSize<<",image size : "<<texWidth<<","<<texHeight<<",pixels : "<<pixels<<std::endl;
            for (size_t i = 0; i < 16; ++i)
                std::cout << +pixels[i] << " ";
            std::cout << std::endl;*/
            Buffer staggingBuffer(device);
            staggingBuffer.create(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
            staggingBuffer.update(pixels, imageSize);
            for (unsigned int i = 0; i < nbBuffers; i++) {
                commandPool.beginRecordCommandBuffer(i);
                transitionImageLayout(images[i], commandPool.getHandle(i),VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
                images[i].copyBufferToImage(commandPool.getHandle(i), staggingBuffer, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), static_cast<uint32_t>(x), static_cast<uint32_t>(y));
                transitionImageLayout(images[i], commandPool.getHandle(i), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                commandPool.endRecordCommandBuffer(i);
            }         
            
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = commandPool.getHandles().size();
            submitInfo.pCommandBuffers = commandPool.getHandles().data();
            Device::QueueFamilyIndices indices = device.findQueueFamilies(device.getPhysicalDevice(), VK_NULL_HANDLE);
            if (vkQueueSubmit(device.getQueue(indices.graphicsFamily.value(), 0), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
                throw std::runtime_error("Echec de l'envoi d'un command buffer!");
            }
            vkDeviceWaitIdle(device.getDevice());
            auto& vec = GPUContext::instance().getSharedTextures(texType);
            id = vec.size()+1;
            //vec.push_back(std::move(*this));
            vec.emplace_back(device);
            vec.back().copyFrom(*this);
        }
	    void Texture::update(CommandPool& commandPool, Buffer& staggingBuffer, unsigned int texWidth, unsigned int texHeight, unsigned int x, unsigned int y, size_t srcStart, size_t mipLevel) {
            //std::cout<<"data size : "<<imageSize<<"image size : "<<texWidth<<","<<texHeight<<std::endl;
            //create(texWidth, texHeight, 1, mipLevels, false, false);
            for (unsigned int i = 0; i < nbBuffers; i++) {
                transitionImageLayout(images[i], commandPool.getHandle(i),VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevel, 0, 1, 1);
                images[i].copyBufferToImage(commandPool.getHandle(i), staggingBuffer, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), static_cast<uint32_t>(x), static_cast<uint32_t>(y), srcStart, mipLevel);
                transitionImageLayout(images[i], commandPool.getHandle(i), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevel, 0, 1, 1);
            }
            mipsInfos.push_back({texWidth, texHeight});
            if (mipLevel == mipLevels-1) {
                auto& vec = GPUContext::instance().getSharedTextures(texType);
                id = vec.size()+1;
                vec.emplace_back(device);
                //vec.push_back(std::move(*this));
                vec.back().copyFrom(commandPool, *this);
            }
        }
        VkFormat Texture::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
            for (VkFormat format : candidates) {
                VkFormatProperties props;
                vkGetPhysicalDeviceFormatProperties(GPUContext::instance().getDevice().getPhysicalDevice(), format, &props);
                if (hasStencilComponent(format) && tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                    return format;
                }
                else if (hasStencilComponent(format) && tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                    return format;
                }
            }
            throw std::runtime_error("failed to find supported format!");
        }
        VkFormat Texture::findDepthFormat() {
            return findSupportedFormat(
                { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
            );
        }
        bool Texture::hasStencilComponent(VkFormat format) {
            return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
        } 
        void Texture::toShaderReadOnlyOptimal(unsigned int currentFrame, VkCommandBuffer cmd) {
            ////std::cout<<"is on color attachment optimal : "<<isOnColorAttachmentOptimal<<std::endl;
            if (isFBOTexture) {
                transitionImageLayout(images[currentFrame], cmd, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            }
        }
        void Texture::toColorAttachmentOptimal(unsigned int currentFrame, VkCommandBuffer cmd) {
            ////std::cout<<"is on color attachment optimal : "<<isOnColorAttachmentOptimal<<std::endl;
            if (isFBOTexture) {
                transitionImageLayout(images[currentFrame], cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            }
        }
        void Texture::transitionImageLayout(Image& image, VkCommandBuffer cmd, VkImageLayout oldLayout, VkImageLayout newLayout, unsigned int mipLevel, unsigned int baseLayer, unsigned int nbLevels, unsigned int nbLayers) {
            image.setLayout(newLayout);
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = oldLayout;
            barrier.newLayout = newLayout;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = image.getHandle();
            VkImageAspectFlags aspectMask = image.getImageAspectFlags(); 
            if (hasStencilComponent(image.getFormat())) {
                aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
            barrier.subresourceRange.aspectMask = aspectMask;           
            barrier.subresourceRange.baseMipLevel = mipLevel;
            barrier.subresourceRange.levelCount = nbLevels;
            barrier.subresourceRange.baseArrayLayer = baseLayer;
            barrier.subresourceRange.layerCount = nbLayers;
            VkPipelineStageFlags sourceStage;
            VkPipelineStageFlags destinationStage;
            /*if (nbLayers > 1) {
                std::cout<<"layered : "<<nbLayers<<std::endl;
                int i;
                std::cin>>i;
            }*/            
            if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            } else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

            }
            else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

            }
            else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            } 
            else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) {
                barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
                barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                destinationStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {

                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            }
            else {
                throw std::invalid_argument("unsupported layout transition!");
            }
            vkCmdPipelineBarrier(
                cmd,
                sourceStage, destinationStage,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );

        }
        math::Vector2u Texture::getSize() const {
            return m_size;
        }
        void Texture::update(Texture& texture) {
            update(texture, 0, 0);

        }
        void Texture::updateCubeMap(Texture& texture) {
            updateCubeMap(texture, 0, 0);
        }
        void Texture::update(Texture& texture, unsigned int x, unsigned int y) {

            /*VkImageBlit2 blitRegion{ .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2, .pNext = nullptr };

            blitRegion.srcOffsets[0].x = 0;
            blitRegion.srcOffsets[0].y = 0;
            blitRegion.srcOffsets[0].z = 0;
            blitRegion.srcOffsets[1].x = texture.m_size.x();
            blitRegion.srcOffsets[1].y = texture.m_size.y();
            blitRegion.srcOffsets[1].z = 1;

            blitRegion.dstOffsets[0].x = x;
            blitRegion.dstOffsets[0].y = y;
            blitRegion.dstOffsets[0].z = 0;
            blitRegion.dstOffsets[1].x = x + texture.m_size.x();
            blitRegion.dstOffsets[1].y = y + texture.m_size.y();
            blitRegion.dstOffsets[1].z = 1;

            blitRegion.srcSubresource.aspectMask = imageAspectMask;
            blitRegion.srcSubresource.baseArrayLayer = 0;
            blitRegion.srcSubresource.layerCount = 1;
            blitRegion.srcSubresource.mipLevel = 0;

            blitRegion.dstSubresource.aspectMask = imageAspectMask;
            blitRegion.dstSubresource.baseArrayLayer = 0;
            blitRegion.dstSubresource.layerCount = 1;
            blitRegion.dstSubresource.mipLevel = 0;
            for (unsigned int i = 0; i < nbBuffers; i++) {
                VkBlitImageInfo2 blitInfo{ .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2, .pNext = nullptr };
                blitInfo.dstImage = images[i].getHandle();
                blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                blitInfo.srcImage = texture.images[i].getHandle();
                blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                blitInfo.filter = VK_FILTER_LINEAR;
                blitInfo.regionCount = 1;
                blitInfo.pRegions = &blitRegion;
                commandPool.beginRecordCommandBuffer(i);
                texture.transitionImageLayout(i, commandPool.getHandle(i), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
                transitionImageLayout(i, commandPool.getHandle(i), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
                vkCmdBlitImage2(commandPool.getHandle(i), &blitInfo);
                texture.transitionImageLayout(i, commandPool.getHandle(i), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                transitionImageLayout(i, commandPool.getHandle(i), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                commandPool.endRecordCommandBuffer(i);
            }            
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = commandPool.getHandles().size();
            submitInfo.pCommandBuffers = commandPool.getHandles().data();
            Device::QueueFamilyIndices indices = device.findQueueFamilies(device.getPhysicalDevice(), VK_NULL_HANDLE);
            if (vkQueueSubmit(device.getQueue(indices.graphicsFamily.value(), 0), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
                throw std::runtime_error("�chec de l'envoi d'un command buffer!");
            }
            vkDeviceWaitIdle(device.getDevice());*/
            VkImageCopy copyRegion{};

            copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyRegion.srcSubresource.mipLevel = 0;
            copyRegion.srcSubresource.baseArrayLayer = 0;
            copyRegion.srcSubresource.layerCount = 1;
            copyRegion.srcOffset = {0, 0, 0};

            copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyRegion.dstSubresource.mipLevel = 0;
            copyRegion.dstSubresource.baseArrayLayer = 0;
            copyRegion.dstSubresource.layerCount = 1;
            copyRegion.dstOffset = {0, 0, 0};

            copyRegion.extent.width  = texture.m_size.x();
            copyRegion.extent.height = texture.m_size.y();
            copyRegion.extent.depth  = 1;
            for (unsigned int i = 0; i < nbBuffers; i++) {
                commandPool.beginRecordCommandBuffer(i);
                texture.transitionImageLayout(texture.images[i], commandPool.getHandle(i), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
                transitionImageLayout(images[i], commandPool.getHandle(i), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
                vkCmdCopyImage(
                    commandPool.getHandle(i),
                    texture.images[i].getHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    images[i].getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1,
                    &copyRegion
                );
                texture.transitionImageLayout(texture.images[i], commandPool.getHandle(i), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                transitionImageLayout(images[i], commandPool.getHandle(i), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                commandPool.endRecordCommandBuffer(i);
            }
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = commandPool.getHandles().size();
            submitInfo.pCommandBuffers = commandPool.getHandles().data();
            Device::QueueFamilyIndices indices = device.findQueueFamilies(device.getPhysicalDevice(), VK_NULL_HANDLE);
            if (vkQueueSubmit(device.getQueue(indices.graphicsFamily.value(), 0), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
                throw std::runtime_error("�chec de l'envoi d'un command buffer!");
            }
            vkDeviceWaitIdle(device.getDevice());
        }
	    void Texture::update(CommandPool& commandPool, Texture& texture, unsigned int x, unsigned int y, size_t mipLevel) {

            VkImageCopy copyRegion{};

            copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyRegion.srcSubresource.mipLevel = mipLevel;
            copyRegion.srcSubresource.baseArrayLayer = 0;
            copyRegion.srcSubresource.layerCount = 1;
            copyRegion.srcOffset = {0, 0, 0};

            copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyRegion.dstSubresource.mipLevel = mipLevel;
            copyRegion.dstSubresource.baseArrayLayer = 0;
            copyRegion.dstSubresource.layerCount = 1;
            copyRegion.dstOffset = {0, 0, 0};

            copyRegion.extent.width  = texture.mipsInfos[mipLevel].width;
            copyRegion.extent.height = texture.mipsInfos[mipLevel].height;
            copyRegion.extent.depth  = 1;
            for (unsigned int i = 0; i < nbBuffers; i++) {
                texture.transitionImageLayout(texture.images[i], commandPool.getHandle(i), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, mipLevel, 0, 1, 1);
                transitionImageLayout(images[i], commandPool.getHandle(i), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevel, 0, 1, 1);
                vkCmdCopyImage(
                    commandPool.getHandle(i),
                    texture.images[i].getHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    images[i].getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1,
                    &copyRegion
                );
                texture.transitionImageLayout(texture.images[i], commandPool.getHandle(i), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevel, 0, 1, 1);
                transitionImageLayout(images[i], commandPool.getHandle(i), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevel, 0, 1, 1);
            }
        }
        void Texture::updateCubeMap(Texture& texture, unsigned int x, unsigned int y) {
            VkImageSubresourceLayers subResourceLayers = {
                .aspectMask = imageAspectMask,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 6
            };
            VkImageBlit2 blitRegion{ .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2, .pNext = nullptr };

            blitRegion.srcOffsets[0].x = 0;
            blitRegion.srcOffsets[0].y = 0;
            blitRegion.srcOffsets[0].z = 0;
            blitRegion.srcOffsets[1].x = texture.m_size.x();
            blitRegion.srcOffsets[1].y = texture.m_size.y();
            blitRegion.srcOffsets[1].z = 1;

            blitRegion.dstOffsets[0].x = x;
            blitRegion.dstOffsets[0].y = y;
            blitRegion.dstOffsets[0].z = 0;
            blitRegion.dstOffsets[1].x = x + texture.m_size.x();
            blitRegion.dstOffsets[1].y = y + texture.m_size.y();
            blitRegion.dstOffsets[1].z = 1;

            blitRegion.srcSubresource.aspectMask = texture.imageAspectMask;
            blitRegion.srcSubresource.baseArrayLayer = 0;
            blitRegion.srcSubresource.layerCount = 6;
            blitRegion.srcSubresource.mipLevel = 0;

            blitRegion.dstSubresource.aspectMask = imageAspectMask;
            blitRegion.dstSubresource.baseArrayLayer = 0;
            blitRegion.dstSubresource.layerCount = 6;
            blitRegion.dstSubresource.mipLevel = 0;
            for (unsigned int i = 0; i < nbBuffers; i++) {
                VkBlitImageInfo2 blitInfo{ .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2, .pNext = nullptr };
                blitInfo.dstImage = images[i].getHandle();
                blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                blitInfo.srcImage = texture.images[i].getHandle();
                blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                blitInfo.filter = VK_FILTER_LINEAR;
                blitInfo.regionCount = 1;
                blitInfo.pRegions = &blitRegion;              
                commandPool.beginRecordCommandBuffer(i);
                transitionImageLayout(texture.images[i], commandPool.getHandle(i), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
                texture.transitionImageLayout(images[i], commandPool.getHandle(i), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
                vkCmdBlitImage2(commandPool.getHandle(i), &blitInfo);
                transitionImageLayout(texture.images[i], commandPool.getHandle(i), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                texture.transitionImageLayout(images[i], commandPool.getHandle(i), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                commandPool.endRecordCommandBuffer(i);
            }
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = commandPool.getHandles().size();
            submitInfo.pCommandBuffers = commandPool.getHandles().data();
            Device::QueueFamilyIndices indices = device.findQueueFamilies(device.getPhysicalDevice(), VK_NULL_HANDLE);
            if (vkQueueSubmit(device.getQueue(indices.graphicsFamily.value(), 0), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
                throw std::runtime_error("�chec de l'envoi d'un command buffer!");
            }
            vkDeviceWaitIdle(device.getDevice());
        }
        void Texture::setSmooth(bool smooth) {
            m_Smooth = smooth;
            for (unsigned int i = 0; i < nbBuffers; i++)
                images[i].createSampler(wrapU, wrapV, mipLevels, smooth, unormalized);
        }
        void Texture::swap(Texture& right) {            
            std::swap(m_size, right.m_size);            
            std::swap(images, right.images);
            std::swap(m_format, right.m_format);            
            std::swap(m_Smooth, right.m_Smooth);            
            /*id = getUniqueId();
            right.id = getUniqueId();*/            
        }        
        unsigned int Texture::getMaximumSize() {
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(device.getPhysicalDevice(), &props);

            // Taille max des images 2D
            uint32_t maxTexSize = props.limits.maxImageDimension2D;
            return maxTexSize;
        }        
        bool Texture::isSmooth() const {
            return m_Smooth;
        }
        void Texture::setRepeated(bool repeated) {
            m_Repeated = repeated;
        }
        bool Texture::isRepeated() const {
            return m_Repeated;
        } 
        unsigned int Texture::getId() const {
            return id;
        }        
        Image& Texture::getImage(unsigned int currentFrame) {
            return images[currentFrame];
        }        
        VkFormat Texture::getFormat() {
            return m_format;
        }
        math::Vector2u Texture::getSize() {
            /*if (m_isRepeated) {
                return math::Vector2u(1, 1);
            }*/
            return m_size;
        }
	    std::vector<Image>& Texture::getImages() {
            return images;
        }
	}
}