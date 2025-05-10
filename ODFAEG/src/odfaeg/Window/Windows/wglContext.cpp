#include "../../../../include/odfaeg/Window/Windows/wglContext.hpp"
#include "../../../../include/odfaeg/Window/contextImpl.hpp"
#include <SFML/System/ThreadLocalPtr.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <sstream>
#include <vector>
#include <SFML/OpenGL.hpp>
#include <iostream>
namespace
{
    // Some drivers are bugged and don't track the current HDC/HGLRC properly
    // In order to deactivate successfully, we need to track it ourselves as well
    sf::ThreadLocalPtr<odfaeg::window::WglContext> currentContext(NULL);
}

namespace odfaeg {
    namespace window {
        using namespace sf;
        ////////////////////////////////////////////////////////////
        void ensureExtensionsInit(HDC deviceContext)
        {
            static bool initialized = false;
            if (!initialized)
            {
                initialized = true;

                // We don't check the return value since the extension
                // flags are cleared even if loading fails
                sfwgl_LoadFunctions(deviceContext);
            }
        }


        ////////////////////////////////////////////////////////////
        String getErrorString(DWORD errorCode)
        {
            std::basic_ostringstream<TCHAR, std::char_traits<TCHAR> > ss;
            TCHAR errBuff[256];
            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errorCode, 0, errBuff, sizeof(errBuff), NULL);
            ss << errBuff;
            String errMsg(ss.str());

            return errMsg;
        }

        WglContext::WglContext() :
        m_window       (NULL),
        m_pbuffer      (NULL),
        m_deviceContext(NULL),
        m_context      (NULL),
        m_ownsWindow   (false) {
        }
        ////////////////////////////////////////////////////////////
        void WglContext::create(IContext* shared)
        {
            // Save the creation settings
            m_settings = ContextSettings();
            WglContext* w32shared = (shared != nullptr) ? static_cast<WglContext*>(shared) : nullptr;

            // Make sure that extensions are initialized if this is not the shared context
            // The shared context is the context used to initialize the extensions
            if (w32shared && w32shared->m_deviceContext)
                ensureExtensionsInit(w32shared->m_deviceContext);

            // Create the rendering surface (window or pbuffer if supported)

            createSurface(w32shared, 1, 1, VideoMode::getDesktopMode().bitsPerPixel);

            // Create the context
            if (m_deviceContext)
                createContext(w32shared);
        }


        ////////////////////////////////////////////////////////////
        void WglContext::create(HWND owner,  const ContextSettings& settings, IContext* shared, unsigned int bitsPerPixel)
        {
            // Save the creation settings
            m_settings = settings;

            // Make sure that extensions are initialized if this is not the shared context
            // The shared context is the context used to initialize the extensions
            WglContext* w32shared = (shared != nullptr) ? static_cast<WglContext*>(shared) : nullptr;
            if (w32shared && w32shared->m_deviceContext)
                ensureExtensionsInit(w32shared->m_deviceContext);

            // Create the rendering surface from the owner window
            createSurface(owner, bitsPerPixel);

            // Create the context
            if (m_deviceContext) {
                createContext(w32shared);
            }
        }


        ////////////////////////////////////////////////////////////
        void WglContext::create(const ContextSettings& settings, unsigned int width, unsigned int height, IContext* shared)
        {
            // Save the creation settings
            m_settings = settings;

            // Make sure that extensions are initialized if this is not the shared context
            // The shared context is the context used to initialize the extensions
            WglContext* w32shared = (shared != nullptr) ? static_cast<WglContext*>(shared) : nullptr;
            if (w32shared && w32shared->m_deviceContext)
                ensureExtensionsInit(w32shared->m_deviceContext);

            // Create the rendering surface (window or pbuffer if supported)
            createSurface(w32shared, width, height, VideoMode::getDesktopMode().bitsPerPixel);

            // Create the context
            if (m_deviceContext)
                createContext(w32shared);
        }


        ////////////////////////////////////////////////////////////
        WglContext::~WglContext()
        {
            // Notify unshared OpenGL resources of context destruction
            //cleanupUnsharedResources();

            // Destroy the OpenGL context
            if (m_context)
            {
                if (currentContext == this)
                {
                    if (wglMakeCurrent(m_deviceContext, NULL) == TRUE)
                        currentContext = NULL;
                }

                wglDeleteContext(m_context);
            }

            // Destroy the device context
            if (m_deviceContext)
            {
                if (m_pbuffer)
                {
                    wglReleasePbufferDCARB(m_pbuffer, m_deviceContext);
                    wglDestroyPbufferARB(m_pbuffer);
                }
                else
                {
                    ReleaseDC(m_window, m_deviceContext);
                }
            }

            // Destroy the window if we own it
            if (m_window && m_ownsWindow)
                DestroyWindow(m_window);
        }


        ////////////////////////////////////////////////////////////
        GlFunctionPointer WglContext::getFunction(const char* name)
        {
            GlFunctionPointer address = reinterpret_cast<GlFunctionPointer>(wglGetProcAddress(reinterpret_cast<LPCSTR>(name)));

            if (address)
            {
                // Test whether the returned value is a valid error code
                ptrdiff_t errorCode = reinterpret_cast<ptrdiff_t>(address);

                if ((errorCode != -1) && (errorCode != 1) && (errorCode != 2) && (errorCode != 3))
                    return address;
            }

            static HMODULE module = NULL;

            if (!module)
                module = GetModuleHandleA("OpenGL32.dll");

            if (module)
                return reinterpret_cast<GlFunctionPointer>(GetProcAddress(module, reinterpret_cast<LPCSTR>(name)));

            return 0;
        }


        ////////////////////////////////////////////////////////////
        bool WglContext::setActive(bool current)
        {
            if (!m_deviceContext || !m_context)
                return false;

            if (wglMakeCurrent(m_deviceContext, current ? m_context : NULL) == FALSE)
            {
                err() << "Failed to " << (current ? "activate" : "deactivate") << " OpenGL context: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
                return false;
            }

            currentContext = (current ? this : NULL);

            return true;
        }


        ////////////////////////////////////////////////////////////
        void WglContext::display()
        {
            if (m_deviceContext && m_context)
                SwapBuffers(m_deviceContext);
        }


        ////////////////////////////////////////////////////////////
        void WglContext::setVerticalSyncEnabled(bool enabled)
        {
            // Make sure that extensions are initialized
            ensureExtensionsInit(m_deviceContext);

            if (sfwgl_ext_EXT_swap_control == sfwgl_LOAD_SUCCEEDED)
            {
                if (wglSwapIntervalEXT(enabled ? 1 : 0) == FALSE)
                    err() << "Setting vertical sync failed: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
            }
            else
            {
                static bool warned = false;

                if (!warned)
                {
                    // wglSwapIntervalEXT not supported
                    err() << "Setting vertical sync not supported" << std::endl;

                    warned = true;
                }
            }
        }


        ////////////////////////////////////////////////////////////
        int WglContext::selectBestPixelFormat(HDC deviceContext, unsigned int bitsPerPixel, const ContextSettings& settings, bool pbuffer)
        {
            // Let's find a suitable pixel format -- first try with wglChoosePixelFormatARB
            int bestFormat = 0;
            if (sfwgl_ext_ARB_pixel_format == sfwgl_LOAD_SUCCEEDED)
            {
                // Define the basic attributes we want for our window
                int intAttributes[] =
                {
                    WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
                    WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
                    WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
                    WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
                    0,                      0
                };

                // Let's check how many formats are supporting our requirements
                int  formats[512];
                UINT nbFormats;
                bool isValid = wglChoosePixelFormatARB(deviceContext, intAttributes, NULL, 512, formats, &nbFormats) != FALSE;

                if (!isValid)
                    err() << "Failed to enumerate pixel formats: " << getErrorString(GetLastError()).toAnsiString() << std::endl;

                // Get the best format among the returned ones
                if (isValid && (nbFormats > 0))
                {
                    int bestScore = 0x7FFFFFFF;
                    for (UINT i = 0; i < nbFormats; ++i)
                    {
                        // Extract the components of the current format
                        int values[7];
                        const int attributes[] =
                        {
                            WGL_RED_BITS_ARB,
                            WGL_GREEN_BITS_ARB,
                            WGL_BLUE_BITS_ARB,
                            WGL_ALPHA_BITS_ARB,
                            WGL_DEPTH_BITS_ARB,
                            WGL_STENCIL_BITS_ARB,
                            WGL_ACCELERATION_ARB
                        };

                        if (wglGetPixelFormatAttribivARB(deviceContext, formats[i], PFD_MAIN_PLANE, 7, attributes, values) == FALSE)
                        {
                            err() << "Failed to retrieve pixel format information: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
                            break;
                        }

                        int sampleValues[2] = {0, 0};
                        if (sfwgl_ext_ARB_multisample == sfwgl_LOAD_SUCCEEDED)
                        {
                            const int sampleAttributes[] =
                            {
                                WGL_SAMPLE_BUFFERS_ARB,
                                WGL_SAMPLES_ARB
                            };

                            if (wglGetPixelFormatAttribivARB(deviceContext, formats[i], PFD_MAIN_PLANE, 2, sampleAttributes, sampleValues) == FALSE)
                            {
                                err() << "Failed to retrieve pixel format multisampling information: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
                                break;
                            }
                        }

                        int sRgbCapableValue = 0;
                        if ((sfwgl_ext_ARB_framebuffer_sRGB == sfwgl_LOAD_SUCCEEDED) || (sfwgl_ext_EXT_framebuffer_sRGB == sfwgl_LOAD_SUCCEEDED))
                        {
                            const int sRgbCapableAttribute = WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB;

                            if (wglGetPixelFormatAttribivARB(deviceContext, formats[i], PFD_MAIN_PLANE, 1, &sRgbCapableAttribute, &sRgbCapableValue) == FALSE)
                            {
                                err() << "Failed to retrieve pixel format sRGB capability information: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
                                break;
                            }
                        }

                        if (pbuffer)
                        {
                            const int pbufferAttributes[] =
                            {
                                WGL_DRAW_TO_PBUFFER_ARB
                            };

                            int pbufferValue = 0;

                            if (wglGetPixelFormatAttribivARB(deviceContext, formats[i], PFD_MAIN_PLANE, 1, pbufferAttributes, &pbufferValue) == FALSE)
                            {
                                err() << "Failed to retrieve pixel format pbuffer information: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
                                break;
                            }

                            if (pbufferValue != GL_TRUE)
                                continue;
                        }

                        // Evaluate the current configuration
                        int color = values[0] + values[1] + values[2] + values[3];
                        int score = ContextImpl::evaluateFormat(bitsPerPixel, settings, color, values[4], values[5], sampleValues[0] ? sampleValues[1] : 0, values[6] == WGL_FULL_ACCELERATION_ARB, sRgbCapableValue == TRUE);

                        // Keep it if it's better than the current best
                        if (score < bestScore)
                        {
                            bestScore  = score;
                            bestFormat = formats[i];
                        }
                    }
                }
            }

            // ChoosePixelFormat doesn't support pbuffers
            if (pbuffer)
                return bestFormat;

            // Find a pixel format with ChoosePixelFormat, if wglChoosePixelFormatARB is not supported
            if (bestFormat == 0)
            {
                // Setup a pixel format descriptor from the rendering settings
                PIXELFORMATDESCRIPTOR descriptor;
                ZeroMemory(&descriptor, sizeof(descriptor));
                descriptor.nSize        = sizeof(descriptor);
                descriptor.nVersion     = 1;
                descriptor.iLayerType   = PFD_MAIN_PLANE;
                descriptor.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
                descriptor.iPixelType   = PFD_TYPE_RGBA;
                descriptor.cColorBits   = static_cast<BYTE>(bitsPerPixel);
                descriptor.cDepthBits   = static_cast<BYTE>(settings.depthBits);
                descriptor.cStencilBits = static_cast<BYTE>(settings.stencilBits);
                descriptor.cAlphaBits   = bitsPerPixel == 32 ? 8 : 0;

                // Get the pixel format that best matches our requirements
                bestFormat = ChoosePixelFormat(deviceContext, &descriptor);
            }

            return bestFormat;
        }


        ////////////////////////////////////////////////////////////
        void WglContext::setDevicePixelFormat(unsigned int bitsPerPixel)
        {
            int bestFormat = selectBestPixelFormat(m_deviceContext, bitsPerPixel, m_settings);
            if (bestFormat == 0)
            {
                err() << "Failed to find a suitable pixel format for device context: " << getErrorString(GetLastError()).toAnsiString() << std::endl
                      << "Cannot create OpenGL context" << std::endl;
                return;
            }

            // Extract the depth and stencil bits from the chosen format
            PIXELFORMATDESCRIPTOR actualFormat;
            actualFormat.nSize    = sizeof(actualFormat);
            actualFormat.nVersion = 1;
            DescribePixelFormat(m_deviceContext, bestFormat, sizeof(actualFormat), &actualFormat);

            // Set the chosen pixel format
            if (SetPixelFormat(m_deviceContext, bestFormat, &actualFormat) == FALSE)
            {
                err() << "Failed to set pixel format for device context: " << getErrorString(GetLastError()).toAnsiString() << std::endl
                      << "Cannot create OpenGL context" << std::endl;
                return;
            }
        }


        ////////////////////////////////////////////////////////////
        void WglContext::updateSettingsFromPixelFormat()
        {
            int format = GetPixelFormat(m_deviceContext);

            if (format == 0)
            {
                err() << "Failed to get selected pixel format: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
                return;
            }

            PIXELFORMATDESCRIPTOR actualFormat;
            actualFormat.nSize    = sizeof(actualFormat);
            actualFormat.nVersion = 1;

            if (DescribePixelFormat(m_deviceContext, format, sizeof(actualFormat), &actualFormat) == 0)
            {
                err() << "Failed to retrieve pixel format information: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
                return;
            }

            if (sfwgl_ext_ARB_pixel_format == sfwgl_LOAD_SUCCEEDED)
            {
                const int attributes[] = {WGL_DEPTH_BITS_ARB, WGL_STENCIL_BITS_ARB};
                int values[2];

                if (wglGetPixelFormatAttribivARB(m_deviceContext, format, PFD_MAIN_PLANE, 2, attributes, values) == TRUE)
                {
                    m_settings.depthBits   = values[0];
                    m_settings.stencilBits = values[1];
                }
                else
                {
                    err() << "Failed to retrieve pixel format information: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
                    m_settings.depthBits   = actualFormat.cDepthBits;
                    m_settings.stencilBits = actualFormat.cStencilBits;
                }

                if (sfwgl_ext_ARB_multisample == sfwgl_LOAD_SUCCEEDED)
                {
                    const int sampleAttributes[] = {WGL_SAMPLE_BUFFERS_ARB, WGL_SAMPLES_ARB};
                    int sampleValues[2];

                    if (wglGetPixelFormatAttribivARB(m_deviceContext, format, PFD_MAIN_PLANE, 2, sampleAttributes, sampleValues) == TRUE)
                    {
                        m_settings.antiAliasingLevel = sampleValues[0] ? sampleValues[1] : 0;
                    }
                    else
                    {
                        err() << "Failed to retrieve pixel format multisampling information: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
                        m_settings.antiAliasingLevel = 0;
                    }
                }
                else
                {
                    m_settings.antiAliasingLevel = 0;
                }

                if ((sfwgl_ext_ARB_framebuffer_sRGB == sfwgl_LOAD_SUCCEEDED) || (sfwgl_ext_EXT_framebuffer_sRGB == sfwgl_LOAD_SUCCEEDED))
                {
                    const int sRgbCapableAttribute = WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB;
                    int sRgbCapableValue = 0;

                    if (wglGetPixelFormatAttribivARB(m_deviceContext, format, PFD_MAIN_PLANE, 1, &sRgbCapableAttribute, &sRgbCapableValue) == TRUE)
                    {
                        m_settings.sRgbCapable = (sRgbCapableValue == TRUE);
                    }
                    else
                    {
                        err() << "Failed to retrieve pixel format sRGB capability information: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
                        m_settings.sRgbCapable = false;
                    }
                }
                else
                {
                    m_settings.sRgbCapable = false;
                }
            }
            else
            {
                m_settings.depthBits   = actualFormat.cDepthBits;
                m_settings.stencilBits = actualFormat.cStencilBits;
                m_settings.antiAliasingLevel = 0;
            }
        }


        ////////////////////////////////////////////////////////////
        void WglContext::createSurface(WglContext* shared, unsigned int width, unsigned int height, unsigned int bitsPerPixel)
        {

            // Check if the shared context already exists and pbuffers are supported
            if (shared && shared->m_deviceContext && (sfwgl_ext_ARB_pbuffer == sfwgl_LOAD_SUCCEEDED))
            {
                int bestFormat = selectBestPixelFormat(shared->m_deviceContext, bitsPerPixel, m_settings, true);

                if (bestFormat > 0)
                {
                    int attributes[] = {0, 0};

                    m_pbuffer = wglCreatePbufferARB(shared->m_deviceContext, bestFormat, width, height, attributes);

                    if (m_pbuffer)
                    {
                        m_window = shared->m_window;
                        m_deviceContext = wglGetPbufferDCARB(m_pbuffer);

                        if (!m_deviceContext)
                        {
                            err() << "Failed to retrieve pixel buffer device context: " << getErrorString(GetLastError()).toAnsiString() << std::endl;

                            wglDestroyPbufferARB(m_pbuffer);
                            m_pbuffer = NULL;
                        }
                    }
                    else
                    {
                        err() << "Failed to create pixel buffer: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
                    }
                }
            }

            // If pbuffers are not available we use a hidden window as the off-screen surface to draw to
            if (!m_deviceContext)
            {
                // We can't create a memory DC, the resulting context wouldn't be compatible
                // with other contexts and thus wglShareLists would always fail

                // Create the hidden window
                m_window = CreateWindowA("STATIC", "", WS_POPUP | WS_DISABLED, 0, 0, width, height, NULL, NULL, GetModuleHandle(NULL), NULL);
                ShowWindow(m_window, SW_HIDE);
                m_deviceContext = GetDC(m_window);

                m_ownsWindow = true;

                // Set the pixel format of the device context
                setDevicePixelFormat(bitsPerPixel);
            }

            // Update context settings from the selected pixel format
            updateSettingsFromPixelFormat();
        }


        ////////////////////////////////////////////////////////////
        void WglContext::createSurface(HWND window, unsigned int bitsPerPixel)
        {
            m_window = window;
            m_deviceContext = GetDC(window);

            // Set the pixel format of the device context
            setDevicePixelFormat(bitsPerPixel);

            // Update context settings from the selected pixel format
            updateSettingsFromPixelFormat();
        }


        ////////////////////////////////////////////////////////////
        void WglContext::createContext(WglContext* shared)
        {
            // Get a working copy of the context settings
            ContextSettings settings = m_settings;

            // Get the context to share display lists with
            HGLRC sharedContext = shared ? shared->m_context : NULL;

            // Create the OpenGL context -- first try using wglCreateContextAttribsARB
            while (!m_context && m_settings.versionMajor)
            {
                if (sfwgl_ext_ARB_create_context == sfwgl_LOAD_SUCCEEDED)
                {
                    std::vector<int> attributes;

                    // Check if the user requested a specific context version (anything > 1.1)
                    if ((m_settings.versionMajor > 1) || ((m_settings.versionMajor == 1) && (m_settings.versionMinor > 1)))
                    {
                        attributes.push_back(WGL_CONTEXT_MAJOR_VERSION_ARB);
                        attributes.push_back(m_settings.versionMajor);
                        attributes.push_back(WGL_CONTEXT_MINOR_VERSION_ARB);
                        attributes.push_back(m_settings.versionMinor);
                    }

                    // Check if setting the profile is supported
                    if (sfwgl_ext_ARB_create_context_profile == sfwgl_LOAD_SUCCEEDED)
                    {
                        int profile = (m_settings.attributeFlags & ContextSettings::Core) ? WGL_CONTEXT_CORE_PROFILE_BIT_ARB : WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
                        int debug = (m_settings.attributeFlags & ContextSettings::Debug) ? WGL_CONTEXT_DEBUG_BIT_ARB : 0;

                        attributes.push_back(WGL_CONTEXT_PROFILE_MASK_ARB);
                        attributes.push_back(profile);
                        attributes.push_back(WGL_CONTEXT_FLAGS_ARB);
                        attributes.push_back(debug);
                    }
                    else
                    {
                        if ((m_settings.attributeFlags & ContextSettings::Core) || (m_settings.attributeFlags & ContextSettings::Debug))
                            err() << "Selecting a profile during context creation is not supported,"
                                  << "disabling comptibility and debug" << std::endl;

                        m_settings.attributeFlags = ContextSettings::Default;
                    }

                    // Append the terminating 0
                    attributes.push_back(0);
                    attributes.push_back(0);

                    if (sharedContext)
                    {
                        static Mutex mutex;
                        Lock lock(mutex);

                        if (currentContext == shared)
                        {
                            if (wglMakeCurrent(shared->m_deviceContext, NULL) == FALSE)
                            {
                                err() << "Failed to deactivate shared context before sharing: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
                                return;
                            }

                            currentContext = NULL;
                        }
                    }

                    // Create the context
                    m_context = wglCreateContextAttribsARB(m_deviceContext, sharedContext, &attributes[0]);
                }
                else
                {
                    // If wglCreateContextAttribsARB is not supported, there is no need to keep trying
                    break;
                }

                // If we couldn't create the context, first try disabling flags,
                // then lower the version number and try again -- stop at 0.0
                // Invalid version numbers will be generated by this algorithm (like 3.9), but we really don't care
                if (!m_context)
                {
                    if (m_settings.attributeFlags != ContextSettings::Default)
                    {
                        m_settings.attributeFlags = ContextSettings::Default;
                    }
                    else if (m_settings.versionMinor > 0)
                    {
                        // If the minor version is not 0, we decrease it and try again
                        m_settings.versionMinor--;

                        m_settings.attributeFlags = settings.attributeFlags;
                    }
                    else
                    {
                        // If the minor version is 0, we decrease the major version
                        m_settings.versionMajor--;
                        m_settings.versionMinor = 9;

                        m_settings.attributeFlags = settings.attributeFlags;
                    }
                }
            }

            // If wglCreateContextAttribsARB failed, use wglCreateContext
            if (!m_context)
            {
                // set the context version to 1.1 (arbitrary) and disable flags
                m_settings.versionMajor = 1;
                m_settings.versionMinor = 1;
                m_settings.attributeFlags = ContextSettings::Default;

                m_context = wglCreateContext(m_deviceContext);
                if (!m_context)
                {
                    err() << "Failed to create an OpenGL context for this window: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
                    return;
                }

                // Share this context with others
                if (sharedContext)
                {
                    // wglShareLists doesn't seem to be thread-safe
                    static Mutex mutex;
                    Lock lock(mutex);

                    if (currentContext == shared)
                    {
                        if (wglMakeCurrent(shared->m_deviceContext, NULL) == FALSE)
                        {
                            err() << "Failed to deactivate shared context before sharing: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
                            return;
                        }

                        currentContext = NULL;
                    }

                    if (wglShareLists(sharedContext, m_context) == FALSE)
                        err() << "Failed to share the OpenGL context: " << getErrorString(GetLastError()).toAnsiString() << std::endl;
                }
            }
        }
    }
}
