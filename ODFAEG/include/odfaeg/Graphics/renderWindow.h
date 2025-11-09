#ifndef ODFAEG_RENDER_WINDOW_HPP
#define ODFAEG_RENDER_WINDOW_HPP
////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////
//
// /!\ Important : this class is a modification of the circle shape class of the ODFAEG
// that I've adapted for odfaeg with 3D vertices.
// Here is the license and the author of the ODFAEG library.
//
// ODFAEG - Simple and Fast Multimedia Library
// Copyright (C) 2007-2013 Laurent Gomila (laurent.gom@gmail.com)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "export.hpp"
#include "renderTarget.h"
#include "image.hpp"
#include <string>
#include "../../../include/odfaeg/Window/window.hpp"
namespace odfaeg
{
    namespace graphic {
        #ifdef VULKAN

        class ODFAEG_GRAPHICS_API RenderWindow : public window::Window, public RenderTarget {
        public :


            RenderWindow(window::VideoMode mode, const core::String& title,   window::Device& vkDevice, std::uint32_t style = window::Style::Default, const window::ContextSettings& settings = window::ContextSettings());
            explicit RenderWindow(window::WindowHandle handle, window::Device& vkDevice, const window::ContextSettings& settings = window::ContextSettings());
            virtual math::Vector2u getSize() const;
            uint32_t getCurrentFrame();
            void recreateSwapchain();
            void cleanupSwapchain();
            void drawVulkanFrame();
            VkRenderPass getRenderPass(unsigned int renderPassId);
            window::Device& getDevice();
            void clear(const Color& color = Color(0, 0, 0, 255));
            const uint32_t& getImageIndex();
            virtual ~RenderWindow();
            void submit(bool lastSubmit = false, std::vector<VkSemaphore> signalSemaphores = std::vector<VkSemaphore>(),
                        std::vector<VkSemaphore> waitSemaphores = std::vector<VkSemaphore>(), std::vector<VkPipelineStageFlags> waitStages = std::vector<VkPipelineStageFlags>(),
                        std::vector<uint64_t> signalValues = std::vector<uint64_t>(),
                        std::vector<uint64_t> waitValues = std::vector<uint64_t>(), std::vector<VkFence>fences = std::vector<VkFence>());
            VkFormat getSwapchainImageFormat();
            std::vector<VkImage> getSwapchainImages();

        protected:
            VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
            VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
            VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
            ////////////////////////////////////////////////////////////
            /// \brief Function called after the window has been created
            ///
            /// This function is called so that derived classes can
            /// perform their own specific initialization as soon as
            /// the window is created.
            ///
            ////////////////////////////////////////////////////////////
            virtual void onCreate();

            ////////////////////////////////////////////////////////////
            /// \brief Function called after the window has been resized
            ///
            /// This function is called so that derived classes can
            /// perform custom actions when the size of the window changes.
            ///
            ////////////////////////////////////////////////////////////
            virtual void onResize();

            std::vector<VkImageView> getSwapChainImageViews();
            std::vector<VkFramebuffer> getSwapchainFrameBuffers(unsigned int frameBufferId);

            VkExtent2D getSwapchainExtents();

            VkSurfaceKHR getSurface();
            const int getMaxFramesInFlight();



        private :

            bool firstSubmit;
            uint32_t imageIndex;

            void createSurface();
            void createSwapChain();
            void createImageViews();
            VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
            void createFramebuffers();
            void createRenderPass();
            void createSyncObjects();
            void cleanup();
            VkSurfaceKHR surface;
            window::Device& vkDevice;
            VkSwapchainKHR swapChain;
            VkFormat swapChainImageFormat;
            VkExtent2D swapChainExtent;
            std::vector<VkImage> swapChainImages;
            std::vector<VkImageView> swapChainImageViews;
            std::vector<VkSemaphore> imageAvailableSemaphores;
            std::vector<VkSemaphore> renderFinishedSemaphores;
            std::vector<VkFence> inFlightFences;
            std::vector<VkFence> imagesInFlight;
            std::vector<VkRenderPass> renderPasses;
            std::vector<std::vector<VkFramebuffer>> swapChainFramebuffers;
            size_t currentFrame;
        };
        #else
          ////////////////////////////////////////////////////////////
        /// \brief Window that can serve as a target for 2D drawing
        ///
        ////////////////////////////////////////////////////////////
        class ODFAEG_GRAPHICS_API RenderWindow : public window::Window, public RenderTarget
        {
        public :

            ////////////////////////////////////////////////////////////
            /// \brief Default constructor
            ///
            /// This constructor doesn't actually create the window,
            /// use the other constructors or call "create" to do so.
            ///
            ////////////////////////////////////////////////////////////
            RenderWindow();

            ////////////////////////////////////////////////////////////
            /// \brief Construct a new window
            ///
            /// This constructor creates the window with the size and pixel
            /// depth defined in \a mode. An optional style can be passed to
            /// customize the look and behaviour of the window (borders,
            /// title bar, resizable, closable, ...).
            ///
            /// The fourth parameter is an optional structure specifying
            /// advanced OpenGL context settings such as antialiasing,
            /// depth-buffer bits, etc. You shouldn't care about these
            /// parameters for a regular usage of the graphics module.
            ///
            /// \param mode     Video mode to use (defines the width, height and depth of the rendering area of the window)
            /// \param title    Title of the window
            /// \param style    Window style
            /// \param settings Additional settings for the underlying OpenGL context
            ///
            ////////////////////////////////////////////////////////////
            RenderWindow(window::VideoMode mode, const core::String& title, std::uint32_t style = window::Style::Default, const window::ContextSettings& settings = window::ContextSettings());

            ////////////////////////////////////////////////////////////
            /// \brief Construct the window from an existing control
            ///
            /// Use this constructor if you want to create an ODFAEG
            /// rendering area into an already existing control.
            ///
            /// The fourth parameter is an optional structure specifying
            /// advanced OpenGL context settings such as antialiasing,
            /// depth-buffer bits, etc. You shouldn't care about these
            /// parameters for a regular usage of the graphics module.
            ///
            /// \param handle   Platform-specific handle of the control
            /// \param settings Additional settings for the underlying OpenGL context
            ///
            ////////////////////////////////////////////////////////////
            explicit RenderWindow(window::WindowHandle handle, const window::ContextSettings& settings = window::ContextSettings());

            ////////////////////////////////////////////////////////////
            /// \brief Destructor
            ///
            /// Closes the window and free all the resources attached to it.
            ///
            ////////////////////////////////////////////////////////////
            virtual ~RenderWindow();

            ////////////////////////////////////////////////////////////
            /// \brief Get the size of the rendering region of the window
            ///
            /// The size doesn't include the titlebar and borders
            /// of the window.
            ///
            /// \return Size in pixels
            ///
            ////////////////////////////////////////////////////////////
            virtual math::Vector2u getSize() const;

            ////////////////////////////////////////////////////////////
            /// \brief Copy the current contents of the window to an image
            ///
            /// This is a slow operation, whose main purpose is to make
            /// screenshots of the application. If you want to update an
            /// image with the contents of the window and then use it for
            /// drawing, you should rather use a sf::Texture and its
            /// update(Window&) function.
            /// You can also draw things directly to a texture with the
            /// sf::RenderTexture class.
            ///
            /// \return Image containing the captured contents
            ///
            ////////////////////////////////////////////////////////////
            Image capture();
        protected:

            ////////////////////////////////////////////////////////////
            /// \brief Function called after the window has been created
            ///
            /// This function is called so that derived classes can
            /// perform their own specific initialization as soon as
            /// the window is created.
            ///
            ////////////////////////////////////////////////////////////
            virtual void onCreate();

            ////////////////////////////////////////////////////////////
            /// \brief Function called after the window has been resized
            ///
            /// This function is called so that derived classes can
            /// perform custom actions when the size of the window changes.
            ///
            ////////////////////////////////////////////////////////////
            virtual void onResize();

        private :
            ////////////////////////////////////////////////////////////
            /// \brief Activate the target for rendering
            ///
            /// \param active True to make the target active, false to deactivate it
            ///
            /// \return True if the function succeeded
            ///
            ////////////////////////////////////////////////////////////
            bool activate(bool active);
        };
        #endif
    }

} // namespace odfaeg


#endif // ODFAEG_RENDERWINDOW_HPP

