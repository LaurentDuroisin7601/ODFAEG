#ifndef ODFAEG_WGLCONTEXT_HPP
#define ODFAEG_WGLCONTEXT_HPP
#include "../contextSettings.hpp"
#include "../iContext.hpp"
#include <windows.h>
#include "wglExtensions.hpp"
namespace odfaeg {
    namespace window {
        ////////////////////////////////////////////////////////////
        /// \brief Windows (WGL) implementation of OpenGL contexts
        ///
        ////////////////////////////////////////////////////////////
        class WglContext : public IContext
        {
        public:
            WglContext();
            ////////////////////////////////////////////////////////////
            /// \brief Create a new default context
            ///
            /// \param shared Context to share the new one with (can be NULL)
            ///
            ////////////////////////////////////////////////////////////
            virtual void create(IContext* sharedContext = nullptr);
            virtual void create(HWND window, const ContextSettings& settings, IContext* shared = nullptr, unsigned int bitsPerPixel=32);
            virtual void create(const ContextSettings& settings, unsigned int width, unsigned int height, IContext* shared = nullptr);

            ////////////////////////////////////////////////////////////
            /// \brief Destructor
            ///
            ////////////////////////////////////////////////////////////
            ~WglContext();

            ////////////////////////////////////////////////////////////
            /// \brief Get the address of an OpenGL function
            ///
            /// \param name Name of the function to get the address of
            ///
            /// \return Address of the OpenGL function, 0 on failure
            ///
            ////////////////////////////////////////////////////////////
            static GlFunctionPointer getFunction(const char* name);

            ////////////////////////////////////////////////////////////
            /// \brief Activate the context as the current target for rendering
            ///
            /// \param current Whether to make the context current or no longer current
            ///
            /// \return True on success, false if any error happened
            ///
            ////////////////////////////////////////////////////////////
            virtual bool setActive(bool current);

            ////////////////////////////////////////////////////////////
            /// \brief Display what has been rendered to the context so far
            ///
            ////////////////////////////////////////////////////////////
            virtual void display();

            ////////////////////////////////////////////////////////////
            /// \brief Enable or disable vertical synchronization
            ///
            /// Activating vertical synchronization will limit the number
            /// of frames displayed to the refresh rate of the monitor.
            /// This can avoid some visual artifacts, and limit the framerate
            /// to a good value (but not constant across different computers).
            ///
            /// \param enabled: True to enable v-sync, false to deactivate
            ///
            ////////////////////////////////////////////////////////////
            virtual void setVerticalSyncEnabled(bool enabled);

            ////////////////////////////////////////////////////////////
            /// \brief Select the best pixel format for a given set of settings
            ///
            /// \param deviceContext Device context
            /// \param bitsPerPixel  Pixel depth, in bits per pixel
            /// \param settings      Requested context settings
            /// \param pbuffer       Whether the pixel format should support pbuffers
            ///
            /// \return The best pixel format
            ///
            ////////////////////////////////////////////////////////////
            static int selectBestPixelFormat(HDC deviceContext, unsigned int bitsPerPixel, const ContextSettings& settings, bool pbuffer = false);
            const ContextSettings& getSettings() const {
                return m_settings;
            }
        protected :
            virtual void initialize(const ContextSettings& settings) {}
            ContextSettings m_settings;
        private:

            ////////////////////////////////////////////////////////////
            /// \brief Set the pixel format of the device context
            ///
            /// \param bitsPerPixel Pixel depth, in bits per pixel
            ///
            ////////////////////////////////////////////////////////////
            void setDevicePixelFormat(unsigned int bitsPerPixel);

            ////////////////////////////////////////////////////////////
            /// \brief Update the context settings from the selected pixel format
            ///
            ////////////////////////////////////////////////////////////
            void updateSettingsFromPixelFormat();

            ////////////////////////////////////////////////////////////
            /// \brief Create the context's drawing surface
            ///
            /// \param shared       Shared context (can be NULL)
            /// \param width        Back buffer width, in pixels
            /// \param height       Back buffer height, in pixels
            /// \param bitsPerPixel Pixel depth, in bits per pixel
            ///
            ////////////////////////////////////////////////////////////
            void createSurface(WglContext* shared, unsigned int width, unsigned int height, unsigned int bitsPerPixel);

            ////////////////////////////////////////////////////////////
            /// \brief Create the context's drawing surface from an existing window
            ///
            /// \param window       Window handle of the owning window
            /// \param bitsPerPixel Pixel depth, in bits per pixel
            ///
            ////////////////////////////////////////////////////////////
            void createSurface(HWND window, unsigned int bitsPerPixel);

            ////////////////////////////////////////////////////////////
            /// \brief Create the context
            ///
            /// \param shared Context to share the new one with (can be NULL)
            ///
            ////////////////////////////////////////////////////////////
            void createContext(WglContext* shared);

            ////////////////////////////////////////////////////////////
            // Member data
            ////////////////////////////////////////////////////////////
            HWND        m_window;        ///< Window to which the context is attached
            HPBUFFERARB m_pbuffer;       ///< Handle to a pbuffer if one was created
            HDC         m_deviceContext; ///< Device context associated to the context
            HGLRC       m_context;       ///< OpenGL context
            bool        m_ownsWindow;    ///< Do we own the target window?
        };
    }
}
#endif // ODFAEG_WGLCONTEXT_HPP
