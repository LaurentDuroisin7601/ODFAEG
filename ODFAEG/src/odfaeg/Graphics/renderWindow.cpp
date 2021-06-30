#include <SFML/OpenGL.hpp>
#include "../../../include/odfaeg/Graphics/renderWindow.h"
#ifndef VULKAN
#include "glCheck.h"
#endif


namespace odfaeg {
    namespace graphic {
        using namespace sf;
        #ifdef VULKAN
        ////////////////////////////////////////////////////////////
        RenderWindow::RenderWindow() : RenderTarget(getVkSettup())
        {
            // Nothing to do
        }
        ////////////////////////////////////////////////////////////
        RenderWindow::RenderWindow(VideoMode mode, const String& title, Uint32 style, const window::ContextSettings& settings) : RenderTarget(getVkSettup())
        {
            // Don't call the base class constructor because it contains virtual function calls
            create(mode, title, style, settings);
        }
         ////////////////////////////////////////////////////////////
        RenderWindow::RenderWindow(WindowHandle handle, const window::ContextSettings& settings) : RenderTarget(getVkSettup())
        {
            // Don't call the base class constructor because it contains virtual function calls
            create(handle, settings);
        }
        void RenderWindow::cleanup() {
            for (auto framebuffer : swapChainFramebuffers) {
                vkDestroyFramebuffer(vkSettup.getDevice(), framebuffer, nullptr);
            }
        }
        ////////////////////////////////////////////////////////////
        RenderWindow::~RenderWindow()
        {
            cleanup();
            RenderTarget::cleanup();
            vkSettup.cleanup();
        }
        ////////////////////////////////////////////////////////////
        Vector2u RenderWindow::getSize() const
        {
            return Window::getSize();
        }
        ////////////////////////////////////////////////////////////
        void RenderWindow::onCreate()
        {
            // Just initialize the render target part
            RenderTarget::initialize();
            createRenderPass();
            createFramebuffers();
        }


        ////////////////////////////////////////////////////////////
        void RenderWindow::onResize()
        {
            // Update the current view (recompute the viewport, which is stored in relative coordinates)
            setView(getView());
            vkSettup.recreateSwapchain();
            cleanup();
            vkDestroyRenderPass(vkSettup.getDevice(), renderPass, nullptr);
            createRenderPass();
            createFramebuffers();
        }
        void RenderWindow::createFramebuffers() {
            std::vector<VkImageView>& swapChainImageViews = vkSettup.getSwapChainImageViews();
            swapChainFramebuffers.resize(swapChainImageViews.size());

            for (size_t i = 0; i < swapChainImageViews.size(); i++) {
                VkImageView attachments[] = {
                    swapChainImageViews[i]
                };

                VkFramebufferCreateInfo framebufferInfo{};
                framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferInfo.renderPass = renderPass;
                framebufferInfo.attachmentCount = 1;
                framebufferInfo.pAttachments = attachments;
                framebufferInfo.width = vkSettup.getSwapchainExtends().width;
                framebufferInfo.height = vkSettup.getSwapchainExtends().height;
                framebufferInfo.layers = 1;

                if (vkCreateFramebuffer(vkSettup.getDevice(), &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
                    throw core::Erreur(0, "failed to create framebuffer!", 1);
                }
            }
        }
        #else
        ////////////////////////////////////////////////////////////
        RenderWindow::RenderWindow()
        {
            // Nothing to do
        }

        ////////////////////////////////////////////////////////////
        RenderWindow::RenderWindow(VideoMode mode, const String& title, Uint32 style, const window::ContextSettings& settings)
        {
            // Don't call the base class constructor because it contains virtual function calls
            create(mode, title, style, settings);
        }




        ////////////////////////////////////////////////////////////
        RenderWindow::RenderWindow(WindowHandle handle, const window::ContextSettings& settings)
        {
            // Don't call the base class constructor because it contains virtual function calls
            create(handle, settings);
        }


        ////////////////////////////////////////////////////////////
        RenderWindow::~RenderWindow()
        {

        }


        ////////////////////////////////////////////////////////////
        bool RenderWindow::activate(bool active)
        {
            return setActive(active);
        }


        ////////////////////////////////////////////////////////////
        Vector2u RenderWindow::getSize() const
        {
            return Window::getSize();
        }

        ////////////////////////////////////////////////////////////
        Image RenderWindow::capture()
        {
            Image image;
            if (setActive())
            {
                int width = static_cast<int>(getSize().x);
                int height = static_cast<int>(getSize().y);

                // copy rows one by one and flip them (OpenGL's origin is bottom while SFML's origin is top)
                std::vector<Uint8> pixels(width * height * 4);
                for (int i = 0; i < height; ++i)
                {
                    Uint8* ptr = &pixels[i * width * 4];
                    glCheck(glReadPixels(0, height - i - 1, width, 1, GL_RGBA, GL_UNSIGNED_BYTE, ptr));
                }

                image.create(width, height, &pixels[0]);
            }

            return image;
        }
        ////////////////////////////////////////////////////////////
        void RenderWindow::onCreate()
        {
            priv::ensureGlewInit();
            RenderTarget::setVersionMajor(getSettings().versionMajor);
            RenderTarget::setVersionMinor(getSettings().versionMinor);
            // Just initialize the render target part
            RenderTarget::initialize(0);
        }


        ////////////////////////////////////////////////////////////
        void RenderWindow::onResize()
        {
            // Update the current view (recompute the viewport, which is stored in relative coordinates)
            setView(getView());
        }
        #endif // VULKAN
    }

} // namespace sf


