#ifndef ODFAEG_RENDER_TARGET_HPP
#define ODFAEG_RENDER_TARGET_HPP

#ifdef VULKAN
#include "../Window/vkDevice.hpp"
#endif // VULKAN
#include "shader.h"
#include "view.h"
#include "vertex.h"
#include "vertexBuffer.hpp"

#include "renderStates.h"
#include <SFML/Graphics/PrimitiveType.hpp>

#include <cstdarg>
#include "../config.hpp"


////////////////////////////////////////////////////////////
//
// /!\ Important : this class is a modification of the circle shape class of the SFML
// that I've adapted for odfaeg with 3D vertices.
// Here is the license and the author of the SFML library.
//
// SFML - Simple and Fast Multimedia Library
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

namespace odfaeg {


    namespace graphic {
        class Drawable;

        #ifdef VULKAN
        class ODFAEG_GRAPHICS_API RenderTarget {
            public :
            RenderTarget(window::Device& vkDevice);
            virtual ~RenderTarget ();
            virtual void clear(const sf::Color& color = sf::Color(0, 0, 0, 255)) = 0;
            void clearDepth();
            void setView(View view);

             ////////////////////////////////////////////////////////////
            /// \brief Get the view currently in use in the render target
            ///
            /// \return The view object that is currently used
            ///
            /// \see setView, getDefaultView
            ///
            ////////////////////////////////////////////////////////////
            View& getView();

            ////////////////////////////////////////////////////////////
            /// \brief Get the default view of the render target
            ///
            /// The default view has the initial size of the render target,
            /// and never changes after the target has been created.
            ///
            /// \return The default view of the render target
            ///
            /// \see setView, getView
            ///
            ////////////////////////////////////////////////////////////
            View& getDefaultView();

             ////////////////////////////////////////////////////////////
            /// \brief Convert a point from target coordinates to world
            ///        coordinates, using the current view
            ///
            /// This function is an overload of the mapPixelToCoords
            /// function that implicitely uses the current view.
            /// It is equivalent to:
            /// \code
            /// target.mapPixelToCoords(point, target.getView());
            /// \endcode
            ///
            /// \param point Pixel to convert
            ///
            /// \return The converted point, in "world" coordinates
            ///
            /// \see mapCoordsToPixel
            ///
            ////////////////////////////////////////////////////////////
            math::Vec3f mapPixelToCoords(const math::Vec3f& point);

            ////////////////////////////////////////////////////////////
            /// \brief Convert a point from target coordinates to world coordinates
            ///
            /// This function finds the 2D position that matches the
            /// given pixel of the render-target. In other words, it does
            /// the inverse of what the graphics card does, to find the
            /// initial position of a rendered pixel.
            ///
            /// Initially, both coordinate systems (world units and target pixels)
            /// match perfectly. But if you define a custom view or resize your
            /// render-target, this assertion is not true anymore, ie. a point
            /// located at (10, 50) in your render-target may map to the point
            /// (150, 75) in your 2D world -- if the view is translated by (140, 25).
            ///
            /// For render-windows, this function is typically used to find
            /// which point (or object) is located below the mouse cursor.
            ///
            /// This version uses a custom view for calculations, see the other
            /// overload of the function if you want to use the current view of the
            /// render-target.
            ///
            /// \param point Pixel to convert
            /// \param view The view to use for converting the point
            ///
            /// \return The converted point, in "world" units
            ///
            /// \see mapCoordsToPixel
            ///
            ////////////////////////////////////////////////////////////
            math::Vec3f mapPixelToCoords(const math::Vec3f& point, View& view);
            ////////////////////////////////////////////////////////////
            /// \brief Convert a point from world coordinates to target
            ///        coordinates, using the current view
            ///
            /// This function is an overload of the mapCoordsToPixel
            /// function that implicitely uses the current view.
            /// It is equivalent to:
            /// \code
            /// target.mapCoordsToPixel(point, target.getView());
            /// \endcode
            ///
            /// \param point Point to convert
            ///
            /// \return The converted point, in target coordinates (pixels)
            ///
            /// \see mapPixelToCoords
            ///
            ////////////////////////////////////////////////////////////
            math::Vec3f mapCoordsToPixel(const math::Vec3f& point);

            ////////////////////////////////////////////////////////////
            /// \brief Convert a point from world coordinates to target coordinates
            ///
            /// This function finds the pixel of the render-target that matches
            /// the given 2D point. In other words, it goes through the same process
            /// as the graphics card, to compute the final position of a rendered point.
            ///
            /// Initially, both coordinate systems (world units and target pixels)
            /// match perfectly. But if you define a custom view or resize your
            /// render-target, this assertion is not true anymore, ie. a point
            /// located at (150, 75) in your 2D world may map to the pixel
            /// (10, 50) of your render-target -- if the view is translated by (140, 25).
            ///
            /// This version uses a custom view for calculations, see the other
            /// overload of the function if you want to use the current view of the
            /// render-target.
            ///
            /// \param point Point to convert
            /// \param view The view to use for converting the point
            ///
            /// \return The converted point, in target coordinates (pixels)
            ///
            /// \see mapPixelToCoords
            ///
            ////////////////////////////////////////////////////////////
            math::Vec3f mapCoordsToPixel(const math::Vec3f& point, View& view);
            ////////////////////////////////////////////////////////////
            /// \brief Draw a drawable object to the render-target
            ///
            /// \param drawable Object to draw
            /// \param states   Render states to use for drawing
            ///
            ////////////////////////////////////////////////////////////
            void draw(Drawable& drawable, RenderStates states = RenderStates::Default);

            ////////////////////////////////////////////////////////////
            /// \brief Draw primitives defined by an array of vertices
            ///
            /// \param vertices    Pointer to the vertices
            /// \param vertexCount Number of vertices in the array
            /// \param type        Type of primitives to draw
            /// \param states      Render states to use for drawing
            ///
            ////////////////////////////////////////////////////////////
            void draw(const Vertex* vertices, unsigned int vertexCount, sf::PrimitiveType type,
                      RenderStates states = RenderStates::Default);
            void drawIndirect(VkCommandBuffer& cmd, unsigned int i, unsigned int nbIndirectCommands, unsigned int stride, VertexBuffer& vertexBuffer, VkBuffer vboIndirect,unsigned int depthStencilId = 0, RenderStates states = RenderStates::Default);
            void drawVertexBuffer(VkCommandBuffer& cmd, unsigned int i,VertexBuffer& vertexBuffer,  unsigned int depthStencilId = 0, RenderStates states = RenderStates::Default);
            /// \brief Return the size of the rendering region of the target
            ///
            /// \return Size in pixels
            ///
            ////////////////////////////////////////////////////////////
            virtual sf::Vector2u getSize() const = 0;
            virtual uint32_t getImageIndex() = 0;
            void cleanup();
            void createGraphicPipeline(sf::PrimitiveType type,
                      RenderStates states = RenderStates::Default, unsigned int depthStencilId=0, unsigned int nbDepthStencil=1);
            virtual std::vector<VkFramebuffer> getSwapchainFrameBuffers(unsigned int frameBufferId) = 0;
            virtual VkRenderPass getRenderPass(unsigned int renderPassId) = 0;
            virtual uint32_t getCurrentFrame() = 0;
            static std::vector<std::vector<std::vector<VkPipelineLayoutCreateInfo>>>& getPipelineLayoutCreateInfo();
            static std::vector<std::vector<std::vector<VkPipelineDepthStencilStateCreateInfo>>>& getDepthStencilCreateInfo();
            static std::vector<std::vector<std::vector<VkPipelineLayout>>>& getPipelineLayout();
            static std::vector<std::vector<std::vector<VkPipeline>>>& getGraphicPipeline();
            static std::vector<VkDescriptorPool>& getDescriptorPool();
            static std::vector<VkDescriptorSetLayout>& getDescriptorSetLayout();
            static std::vector<std::vector<VkDescriptorSet>>& getDescriptorSet();


            void updateCommandBuffers(VkCommandPool commandPool, std::vector<VkCommandBuffer> commandBuffers);
            void createDescriptorSetLayout(RenderStates states);
            void createDescriptorPool(RenderStates states);
            void createDescriptorSets(RenderStates states);
            unsigned int getId();
            static unsigned int getNbRenderTargets();
            void enableStencilTest(bool enabled);
            void enableDepthTest(bool enable);
            void beginRecordCommandBuffers();
            void beginRenderPass();
            std::vector<VkCommandBuffer>& getCommandBuffers();
            std::string m_name;

        protected :

            void initialize();

            window::Device& vkDevice;
            virtual VkSurfaceKHR getSurface() = 0;
            virtual VkExtent2D getSwapchainExtents() = 0;
            virtual VkFormat getSwapchainImageFormat() = 0;
            virtual std::vector<VkImage> getSwapchainImages() = 0;
            virtual const int getMaxFramesInFlight() = 0;
            Texture& getDepthTexture();

            sf::Color clearColor;
            bool depthTestEnabled, stencilTestEnabled;

        private :
            void applyViewportAndScissor();
            PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR{ VK_NULL_HANDLE };
            void createUniformBuffers();
            void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
            uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
            struct UniformBufferObject {
                alignas(16) math::Matrix4f model;
                alignas(16) math::Matrix4f view;
                alignas(16) math::Matrix4f proj;
            };
            void updateUniformBuffer(uint32_t imageIndex, UniformBufferObject ubo);

            void createCommandPool();
            void createCommandBuffers();
            void recordCommandBuffers(sf::PrimitiveType type, RenderStates states);
            View        m_defaultView; ///< Default view
            View        m_view;  ///< Current view
            Shader defaultShader, defaultShader2;
            static std::vector<std::vector<std::vector<VkPipelineLayoutCreateInfo>>> pipelineLayoutInfo;
            static std::vector<std::vector<std::vector<VkPipelineDepthStencilStateCreateInfo>>> depthStencil;
            static std::vector<std::vector<std::vector<VkPipelineLayout>>> pipelineLayout;
            static std::vector<std::vector<std::vector<VkPipeline>>> graphicsPipeline;
            VkCommandPool commandPool;
            std::vector<VkCommandBuffer> commandBuffers;
            static std::vector<VertexBuffer*> vertexBuffers;
            static std::vector<std::vector<VkBuffer>> uniformBuffers;
            static std::vector<std::vector<VkDeviceMemory>> uniformBuffersMemory;
            static std::vector<VkDescriptorPool> descriptorPool;
            static std::vector<VkDescriptorSetLayout> descriptorSetLayout;
            static std::vector<std::vector<VkDescriptorSet>> descriptorSets;
            unsigned int selectedBuffer;
            static unsigned int nbBuffers;
            unsigned int id;
            static unsigned int nbRenderTargets;
            Texture* depthTexture;
        };
        #else
        ////////////////////////////////////////////////////////////
        /// \brief Base class for all render targets (window, texture, ...)
        ///
        ////////////////////////////////////////////////////////////
        class ODFAEG_GRAPHICS_API RenderTarget {

        public :

            ////////////////////////////////////////////////////////////
            /// \brief Destructor
            ///
            ////////////////////////////////////////////////////////////
            virtual ~RenderTarget();

             ////////////////////////////////////////////////////////////
            /// \brief Clear the entire target with a single color
            ///
            /// This function is usually called once every frame,
            /// to clear the previous contents of the target.
            ///
            /// \param color Fill color to use to clear the render target
            ///
            ////////////////////////////////////////////////////////////
            void clear(const sf::Color& color = sf::Color(0, 0, 0, 255));
            void clearDepth();

             ////////////////////////////////////////////////////////////
            /// \brief Get the viewport of a view, applied to this render target
            ///
            /// The viewport is defined in the view as a ratio, this function
            /// simply applies this ratio to the current dimensions of the
            /// render target to calculate the pixels rectangle that the viewport
            /// actually covers in the target.
            ///
            /// \param view The view for which we want to compute the viewport
            ///
            /// \return Viewport rectangle, expressed in pixels
            ///
            ////////////////////////////////////////////////////////////
            sf::IntRect getViewport(const View* view) const;
            ViewportMatrix getViewportMatrix(View* view);

            ////////////////////////////////////////////////////////////
            /// \brief Change the current active view
            ///
            /// The view is like a 2D camera, it controls which part of
            /// the 2D scene is visible, and how it is viewed in the
            /// render-target.
            /// The new view will affect everything that is drawn, until
            /// another view is set.
            /// The render target keeps its own copy of the view object,
            /// so it is not necessary to keep the original one alive
            /// after calling this function.
            /// To restore the original view of the target, you can pass
            /// the result of getDefaultView() to this function.
            ///
            /// \param view New view to use
            ///
            /// \see getView, getDefaultView
            ///
            ////////////////////////////////////////////////////////////
            void setView(View view);

             ////////////////////////////////////////////////////////////
            /// \brief Get the view currently in use in the render target
            ///
            /// \return The view object that is currently used
            ///
            /// \see setView, getDefaultView
            ///
            ////////////////////////////////////////////////////////////
            View& getView();

            ////////////////////////////////////////////////////////////
            /// \brief Get the default view of the render target
            ///
            /// The default view has the initial size of the render target,
            /// and never changes after the target has been created.
            ///
            /// \return The default view of the render target
            ///
            /// \see setView, getView
            ///
            ////////////////////////////////////////////////////////////
            View& getDefaultView();

             ////////////////////////////////////////////////////////////
            /// \brief Convert a point from target coordinates to world
            ///        coordinates, using the current view
            ///
            /// This function is an overload of the mapPixelToCoords
            /// function that implicitely uses the current view.
            /// It is equivalent to:
            /// \code
            /// target.mapPixelToCoords(point, target.getView());
            /// \endcode
            ///
            /// \param point Pixel to convert
            ///
            /// \return The converted point, in "world" coordinates
            ///
            /// \see mapCoordsToPixel
            ///
            ////////////////////////////////////////////////////////////
            math::Vec3f mapPixelToCoords(const math::Vec3f& point);

            ////////////////////////////////////////////////////////////
            /// \brief Convert a point from target coordinates to world coordinates
            ///
            /// This function finds the 2D position that matches the
            /// given pixel of the render-target. In other words, it does
            /// the inverse of what the graphics card does, to find the
            /// initial position of a rendered pixel.
            ///
            /// Initially, both coordinate systems (world units and target pixels)
            /// match perfectly. But if you define a custom view or resize your
            /// render-target, this assertion is not true anymore, ie. a point
            /// located at (10, 50) in your render-target may map to the point
            /// (150, 75) in your 2D world -- if the view is translated by (140, 25).
            ///
            /// For render-windows, this function is typically used to find
            /// which point (or object) is located below the mouse cursor.
            ///
            /// This version uses a custom view for calculations, see the other
            /// overload of the function if you want to use the current view of the
            /// render-target.
            ///
            /// \param point Pixel to convert
            /// \param view The view to use for converting the point
            ///
            /// \return The converted point, in "world" units
            ///
            /// \see mapCoordsToPixel
            ///
            ////////////////////////////////////////////////////////////
            math::Vec3f mapPixelToCoords(const math::Vec3f& point, View& view);

            ////////////////////////////////////////////////////////////
            /// \brief Convert a point from world coordinates to target
            ///        coordinates, using the current view
            ///
            /// This function is an overload of the mapCoordsToPixel
            /// function that implicitely uses the current view.
            /// It is equivalent to:
            /// \code
            /// target.mapCoordsToPixel(point, target.getView());
            /// \endcode
            ///
            /// \param point Point to convert
            ///
            /// \return The converted point, in target coordinates (pixels)
            ///
            /// \see mapPixelToCoords
            ///
            ////////////////////////////////////////////////////////////
            math::Vec3f mapCoordsToPixel(const math::Vec3f& point);

            ////////////////////////////////////////////////////////////
            /// \brief Convert a point from world coordinates to target coordinates
            ///
            /// This function finds the pixel of the render-target that matches
            /// the given 2D point. In other words, it goes through the same process
            /// as the graphics card, to compute the final position of a rendered point.
            ///
            /// Initially, both coordinate systems (world units and target pixels)
            /// match perfectly. But if you define a custom view or resize your
            /// render-target, this assertion is not true anymore, ie. a point
            /// located at (150, 75) in your 2D world may map to the pixel
            /// (10, 50) of your render-target -- if the view is translated by (140, 25).
            ///
            /// This version uses a custom view for calculations, see the other
            /// overload of the function if you want to use the current view of the
            /// render-target.
            ///
            /// \param point Point to convert
            /// \param view The view to use for converting the point
            ///
            /// \return The converted point, in target coordinates (pixels)
            ///
            /// \see mapPixelToCoords
            ///
            ////////////////////////////////////////////////////////////
            math::Vec3f mapCoordsToPixel(const math::Vec3f& point, View& view);
            void drawInstanced(VertexBuffer& vertexBuffer, enum sf::PrimitiveType type, unsigned int start, unsigned int nb, unsigned int nbInstances, RenderStates states, unsigned int vboMatrix1=0, unsigned int vboMatrix2=0);

            ////////////////////////////////////////////////////////////
            /// \brief Draw a drawable object to the render-target
            ///
            /// \param drawable Object to draw
            /// \param states   Render states to use for drawing
            ///
            ////////////////////////////////////////////////////////////
            void draw(Drawable& drawable, RenderStates states = RenderStates::Default);

            ////////////////////////////////////////////////////////////
            /// \brief Draw primitives defined by an array of vertices
            ///
            /// \param vertices    Pointer to the vertices
            /// \param vertexCount Number of vertices in the array
            /// \param type        Type of primitives to draw
            /// \param states      Render states to use for drawing
            ///
            ////////////////////////////////////////////////////////////
            void draw(const Vertex* vertices, unsigned int vertexCount, sf::PrimitiveType type,
                      RenderStates states = RenderStates::Default);
            void drawVertexBuffer(VertexBuffer& vertexBuffer, RenderStates states = RenderStates::Default);
            void drawIndirect(VertexBuffer& vb, sf::PrimitiveType primitiveType, unsigned int size, RenderStates states, unsigned int vboIndirect, unsigned int vboMatrices=0, unsigned int vboMatrices2=0);
            void drawVBOBindlessIndirect(sf::PrimitiveType type, unsigned int nbIndirectCommands, RenderStates states, unsigned int vboIndirect);
                       ////////////////////////////////////////////////////////////
            /// \brief Return the size of the rendering region of the target
            ///
            /// \return Size in pixels
            ///
            ////////////////////////////////////////////////////////////
            virtual sf::Vector2u getSize() const = 0;
             ////////////////////////////////////////////////////////////
            /// \brief Save the current OpenGL render states and matrices
            ///
            /// This function can be used when you mix SFML drawing
            /// and direct OpenGL rendering. Combined with PopGLStates,
            /// it ensures that:
            /// \li SFML's internal states are not messed up by your OpenGL code
            /// \li your OpenGL states are not modified by a call to a SFML function
            ///
            /// More specifically, it must be used around code that
            /// calls Draw functions. Example:
            /// \code
            /// // OpenGL code here...
            /// window.pushGLStates();
            /// window.draw(...);
            /// window.draw(...);
            /// window.popGLStates();
            /// // OpenGL code here...
            /// \endcode
            ///
            /// Note that this function is quite expensive: it saves all the
            /// possible OpenGL states and matrices, even the ones you
            /// don't care about. Therefore it should be used wisely.
            /// It is provided for convenience, but the best results will
            /// be achieved if you handle OpenGL states yourself (because
            /// you know which states have really changed, and need to be
            /// saved and restored). Take a look at the ResetGLStates
            /// function if you do so.
            ///
            /// \see popGLStates
            ///
            ////////////////////////////////////////////////////////////
            void pushGLStates();

             ////////////////////////////////////////////////////////////
            /// \brief Restore the previously saved OpenGL render states and matrices
            ///
            /// See the description of pushGLStates to get a detailed
            /// description of these functions.
            ///
            /// \see pushGLStates
            ///
            ////////////////////////////////////////////////////////////
            void popGLStates();

            ////////////////////////////////////////////////////////////
            /// \brief Reset the internal OpenGL states so that the target is ready for drawing
            ///
            /// This function can be used when you mix SFML drawing
            /// and direct OpenGL rendering, if you choose not to use
            /// pushGLStates/popGLStates. It makes sure that all OpenGL
            /// states needed by SFML are set, so that subsequent draw()
            /// calls will work as expected.
            ///
            /// Example:
            /// \code
            /// // OpenGL code here...
            /// glPushAttrib(...);
            /// window.resetGLStates();
            /// window.draw(...);
            /// window.draw(...);
            /// glPopAttrib(...);
            /// // OpenGL code here...
            /// \endcode
            ///
            ////////////////////////////////////////////////////////////
            void resetGLStates();
            unsigned int getVersionMajor();
            unsigned int getVersionMinor();
            void setAlphaTestEnable(bool enabled);
            void setName(std::string name) {
                m_name = name;
            }
            std::string getName() {
                return m_name;
            }

            void setEnableCubeMap(bool enableCubeMap);
        protected :
            void setVersionMajor(unsigned int version);
            void setVersionMinor(unsigned int version);
            ////////////////////////////////////////////////////////////
            /// \brief Default constructor
            ///
            ////////////////////////////////////////////////////////////
            RenderTarget();

            ////////////////////////////////////////////////////////////
            /// \brief Performs the common initialization step after creation
            ///
            /// The derived classes must call this function after the
            /// target is created and ready for drawing.
            ///
            ////////////////////////////////////////////////////////////
            void initialize (unsigned int framebufferId);
        private :


            ////////////////////////////////////////////////////////////
            /// \brief Apply the current view
            ///
            ////////////////////////////////////////////////////////////
            void applyCurrentView();

             ////////////////////////////////////////////////////////////
            /// \brief Apply a new blending mode
            ///
            /// \param mode Blending mode to apply
            ///
            ////////////////////////////////////////////////////////////
            void applyBlendMode(const sf::BlendMode& mode);

            ////////////////////////////////////////////////////////////
            /// \brief Apply a new transform
            ///
            /// \param transform Transform to apply
            ///
            ////////////////////////////////////////////////////////////
            void applyTransform(TransformMatrix& transform);

            ////////////////////////////////////////////////////////////
            /// \brief Apply a new texture
            ///
            /// \param texture Texture to apply
            ///
            ////////////////////////////////////////////////////////////
            void applyTexture(const Texture* texture);

            ////////////////////////////////////////////////////////////
            /// \brief Apply a new shader
            ///
            /// \param shader Shader to apply
            ///
            ////////////////////////////////////////////////////////////
            void applyShader(const Shader* shader);

             ////////////////////////////////////////////////////////////
            /// \brief Activate the target for rendering
            ///
            /// This function must be implemented by derived classes to make
            /// their OpenGL context current; it is called by the base class
            /// everytime it's going to use OpenGL calls.
            ///
            /// \param active True to make the target active, false to deactivate it
            ///
            /// \return True if the function succeeded
            ///
            ////////////////////////////////////////////////////////////
            virtual bool activate(bool active) = 0;

             ////////////////////////////////////////////////////////////
            /// \brief Render states cache
            ///
            ////////////////////////////////////////////////////////////
            struct StatesCache
            {
                enum {VertexCacheSize = 4};
                bool      glStatesSet; ///< Are our internal GL states set yet?
                bool      viewChanged; ///< Has the current view changed since last draw?
                bool vboPointerSets;
                sf::BlendMode lastBlendMode; ///< Cached blending mode
                sf::Uint64    lastTextureId; ///< Cached texture
                bool      useVertexCache; ///< Did we previously use the vertex cache?
                Vertex vertexCache[VertexCacheSize]; ///< Pre-transformed vertices cache
                VertexBuffer* lastVboBuffer;
            };

             ////////////////////////////////////////////////////////////
            // Member data
            ////////////////////////////////////////////////////////////
            View        m_defaultView; ///< Default view
            View        m_view;  ///< Current view
            StatesCache m_cache;  ///< Render states cache
            unsigned int m_vao, m_versionMajor, m_versionMinor, m_framebufferId;
            bool enableAlphaTest, enableCubeMap;
            std::string m_name;

        };
        #endif
    }
}
#endif // RENDER_TARGET
