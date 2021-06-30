#include "glxExtensions.hpp"
#include "../../../../include/odfaeg/Window/glxContext.hpp"
#include "../../../../include/odfaeg/Window/contextImpl.hpp"
#include "../../../../include/odfaeg/Window/display.hpp"
#include <SFML/Window/VideoMode.hpp>
#include <cstring>
#include <iostream>
typedef GLXContext (*glXCreateContextAttribsARBProc)(::Display*, GLXFBConfig, GLXContext, Bool, const int*);
typedef Bool (*glXMakeContextCurrentARBProc)(::Display*, GLXDrawable, GLXDrawable, GLXContext);
namespace
{
    sf::Mutex glxErrorMutex;
    bool glxErrorOccurred = false;

    int HandleXError(::Display*, XErrorEvent*)
    {
        glxErrorOccurred = true;
        return 0;
    }

    class GlxErrorHandler
    {
    public:

        GlxErrorHandler(::Display* display) :
        m_lock   (glxErrorMutex),
        m_display(display)
        {
            glxErrorOccurred = false;
            m_previousHandler = XSetErrorHandler(HandleXError);
        }

        ~GlxErrorHandler()
        {
            XSync(m_display, False);
            XSetErrorHandler(m_previousHandler);
        }

    private:
        sf::Lock   m_lock;
        ::Display* m_display;
        int      (*m_previousHandler)(::Display*, XErrorEvent*);
    };
}
namespace odfaeg {
    namespace window {
        using namespace sf;
        ////////////////////////////////////////////////////////////
        void ensureExtensionsInit(::Display* display, int screen)
        {
            static bool initialized = false;
            if (!initialized)
            {
                initialized = true;
                // We don't check the return value since the extension
                // flags are cleared even if loading fails
                sfglx_LoadFunctions(display, screen);
            }
        }
        ////////////////////////////////////////////////////////////
        GlFunctionPointer GlxContext::getFunction(const char* name)
        {
            return reinterpret_cast<GlFunctionPointer>(glXGetProcAddressARB(reinterpret_cast<const GLubyte*>(name)));
        }
        bool GlxContext::ctxErrorOccurred = false;
        // Helper to check for extension string presence.  Adapted from:
        //   http://www.opengl.org/resources/features/OGLextensions/
        bool GlxContext::isExtensionSupported(const char *extList, const char *extension)
        {
          const char *start;
          const char *where, *terminator;

          /* Extension names should not have spaces. */
          where = strchr(extension, ' ');
          if (where || *extension == '\0')
            return false;

          /* It takes a bit of care to be fool-proof about parsing the
             OpenGL extensions string. Don't be fooled by sub-strings,
             etc. */
          for (start=extList;;) {
            where = strstr(start, extension);

            if (!where)
              break;

            terminator = where + strlen(extension);

            if ( where == start || *(where - 1) == ' ' )
              if ( *terminator == ' ' || *terminator == '\0' )
                return true;

            start = terminator;
          }

          return false;
        }
        int GlxContext::ctxErrorHandler(::Display *dpy, XErrorEvent *ev )
        {
            ctxErrorOccurred = true;
            return 0;
        }
        GlxContext::GlxContext() {
            m_display = Display::openDisplay();
            ensureExtensionsInit(m_display, DefaultScreen(m_display));
            m_windowLess = true;
            m_pbuffer = 0;
            m_context = nullptr;
        }
        ////////////////////////////////////////////////////////////
        XVisualInfo GlxContext::selectBestVisual(::Display* display, unsigned int bitsPerPixel, const ContextSettings& settings)
        {
            // Make sure that extensions are initialized
            ensureExtensionsInit(display, DefaultScreen(display));

            // Retrieve all the visuals
            int count;
            XVisualInfo* visuals = XGetVisualInfo(display, 0, NULL, &count);
            if (visuals)
            {
                // Evaluate all the returned visuals, and pick the best one
                int bestScore = 0x7FFFFFFF;
                XVisualInfo bestVisual = XVisualInfo();
                for (int i = 0; i < count; ++i)
                {
                    // Check mandatory attributes
                    int doubleBuffer;
                    glXGetConfig(display, &visuals[i], GLX_DOUBLEBUFFER, &doubleBuffer);
                    if (!doubleBuffer)
                        continue;

                    // Extract the components of the current visual
                    int red, green, blue, alpha, depth, stencil, multiSampling, samples, sRgb;
                    glXGetConfig(display, &visuals[i], GLX_RED_SIZE,     &red);
                    glXGetConfig(display, &visuals[i], GLX_GREEN_SIZE,   &green);
                    glXGetConfig(display, &visuals[i], GLX_BLUE_SIZE,    &blue);
                    glXGetConfig(display, &visuals[i], GLX_ALPHA_SIZE,   &alpha);
                    glXGetConfig(display, &visuals[i], GLX_DEPTH_SIZE,   &depth);
                    glXGetConfig(display, &visuals[i], GLX_STENCIL_SIZE, &stencil);

                    if (sfglx_ext_ARB_multisample == sfglx_LOAD_SUCCEEDED)
                    {
                        glXGetConfig(display, &visuals[i], GLX_SAMPLE_BUFFERS_ARB, &multiSampling);
                        glXGetConfig(display, &visuals[i], GLX_SAMPLES_ARB,        &samples);
                    }
                    else
                    {
                        multiSampling = 0;
                        samples = 0;
                    }

                    if ((sfglx_ext_EXT_framebuffer_sRGB == sfglx_LOAD_SUCCEEDED) || (sfglx_ext_ARB_framebuffer_sRGB == sfglx_LOAD_SUCCEEDED))
                    {
                        glXGetConfig(display, &visuals[i], GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB, &sRgb);
                    }
                    else
                    {
                        sRgb = 0;
                    }

                    // TODO: Replace this with proper acceleration detection
                    bool accelerated = true;

                    // Evaluate the visual
                    int color = red + green + blue + alpha;
                    int score = ContextImpl::evaluateFormat(bitsPerPixel, settings, color, depth, stencil, multiSampling ? samples : 0, accelerated, sRgb == True);

                    // If it's better than the current best, make it the new best
                    if (score < bestScore)
                    {
                        bestScore = score;
                        bestVisual = visuals[i];
                    }
                }

                // Free the array of visuals
                XFree(visuals);

                return bestVisual;
            }
            else
            {
                // Should never happen...
                err() << "No GLX visual found. You should check your graphics driver" << std::endl;

                return XVisualInfo();
            }
        }
        void GlxContext::create(IContext* sharedContext) {
         // Save the creation settings
            m_settings = ContextSettings();

            // Open the connection with the X server
            m_display = Display::openDisplay();
            // Create the rendering surface (window or pbuffer if supported)
            createSurface(static_cast<GlxContext*>(sharedContext), 1, 1, sf::VideoMode::getDesktopMode().bitsPerPixel);
            createContext(static_cast<GlxContext*>(sharedContext));
        }
        void GlxContext::create(const ContextSettings& settings, unsigned int width, unsigned int height, IContext* sharedContext) {
            //GLXContext shared = (sharedContext != nullptr) ? static_cast<GlxContext*>(sharedContext)->m_context : nullptr;
            /*XVisualInfo vi = selectBestVisual(m_display, VideoMode::getDesktopMode().bitsPerPixel, settings);
            updateSettingsFromVisualInfo(&vi);


            int versionMajor=0, versionMinor=0, depthBits=0, stencilBits=0, antiAliasingLevel=0;
            // Get a GLXFBConfig that matches the visual
            GLXFBConfig* bestFbc = nullptr;
            // We don't supply attributes to match against, since
            // the visual we are matching against was already
            // deemed suitable in selectBestVisual()
            int nbConfigs = 0;
            GLXFBConfig* configs = glXChooseFBConfig(m_display, DefaultScreen(m_display), NULL, &nbConfigs);
            int bestDepthBits = 0, bestStencilBits = 0, bestAntiAliasingLevel = 0;
            for (int i = 0; configs && (i < nbConfigs); ++i)
            {
                XVisualInfo* visual = glXGetVisualFromFBConfig(m_display, configs[i]);

                if (!visual)
                    continue;

                if (visual->visualid == vi.visualid)
                {
                    bestFbc = &configs[i];
                    XFree(visual);
                    break;
                }
                XFree(visual);
            }
            int pbuffer_attribs[] = {
                            GLX_PBUFFER_WIDTH, (int) width,
                            GLX_PBUFFER_HEIGHT, (int) height,
                            None
                    };
                    m_pbuffer = glXCreatePbuffer(m_display, *bestFbc, pbuffer_attribs);
                    XSync(m_display, False);
            m_window = DefaultRootWindow(m_display);
            m_windowLess = true;*/
            m_settings = settings;
            createSurface(static_cast<GlxContext*>(sharedContext), width, height, sf::VideoMode::getDesktopMode().bitsPerPixel);
            createContext(static_cast<GlxContext*>(sharedContext));
            /*glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
            glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
            glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );
            glXMakeContextCurrentARBProc glXMakeContextCurrentARB = 0;
            glXMakeContextCurrentARB = (glXMakeContextCurrentARBProc)
            glXGetProcAddressARB( (const GLubyte *) "glXMakeContextCurrent");
            // Install an X error handler so the application won't exit if GL 3.0
            // context allocation fails.
            //
            // Note this error handler is global.  All display connections in all threads
            // of a process use the same error handler, so be sure to guard against other
            // threads issuing X commands while this code is running.
            int (*oldHandler)(::Display*, XErrorEvent*) =
              XSetErrorHandler(&ctxErrorHandler);
            // Get the default screen's GLX extension list
            const char *glxExts = glXQueryExtensionsString(m_display,
                                                          DefaultScreen(m_display ) );
            // Check for the GLX_ARB_create_context extension string and the function.
            // If either is not present, use GLX 1.3 context creation method.
            if ( !isExtensionSupported( glxExts, "GLX_ARB_create_context") ||
               !glXCreateContextAttribsARB || !glXMakeContextCurrentARB)
            {
                std::cout<<"glXCreateContextAttribsARB() not found"
                " ... using old-style GLX context"<<std::endl;
                m_context = glXCreateContext(m_display, &vi, NULL, GL_TRUE);
            } else {
                int context_attribs[] = {
                        GLX_CONTEXT_MAJOR_VERSION_ARB, (int) settings.versionMajor,
                        GLX_CONTEXT_MINOR_VERSION_ARB, (int) settings.versionMinor,
                        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
                        None
                };
                m_context = glXCreateContextAttribsARB(m_display, *bestFbc, shared, True, context_attribs);
                // Sync to ensure any errors generated are processed.
                XSync(m_display, False );
                if ( !ctxErrorOccurred && m_context ) {
                    int pbuffer_attribs[] = {
                            GLX_PBUFFER_WIDTH, (int) width,
                            GLX_PBUFFER_HEIGHT, (int) height,
                            None
                    };
                    m_pbuffer = glXCreatePbuffer(m_display, *bestFbc, pbuffer_attribs);
                    XSync(m_display, False);
                } else
                {
                  // Couldn't create GL 3.0 context.  Fall back to old-style 2.x context.
                  // When a context version below 3.0 is requested, implementations will
                  // return the newest context version compatible with OpenGL versions less
                  // than version 3.0.
                  // GLX_CONTEXT_MAJOR_VERSION_ARB = 1
                  context_attribs[1] = settings.versionMajor;
                  // GLX_CONTEXT_MINOR_VERSION_ARB = 0
                  context_attribs[3] = settings.versionMinor;

                  ctxErrorOccurred = false;
                  std::cout<<"Failed to create GL 3.0 context"
                          " ... using old-style GLX context"<<std::endl;
                  m_context = glXCreateContext(m_display, &vi, shared, GL_TRUE);
                }
                // Sync to ensure any errors generated are processed.
                XSync(m_display, False );

                // Restore the original error handler
                XSetErrorHandler( oldHandler );
                if ( ctxErrorOccurred || !m_context )
                {
                    std::cerr<<"Failed to create an OpenGL context"<<std::endl;
                    exit(1);
                }
                // Verifying that context is a direct context
                if ( ! glXIsDirect (m_display, m_context ) )
                {
                    std::cout<<"Indirect GLX rendering context obtained"<<std::endl;
                }
                else
                {
                    std::cout<<"Direct GLX rendering context obtained"<<std::endl;
                }
            }*/
        }
        void GlxContext::create(::Window win, const ContextSettings& settings, IContext* sharedContext, unsigned int bitsPerPixel) {


              //m_window = win;
              // Get a matching FB config
              /*static int visual_attribs[] =
                {
                  GLX_X_RENDERABLE    , True,
                  GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
                  GLX_RENDER_TYPE     , GLX_RGBA_BIT,
                  GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
                  GLX_RED_SIZE        , 8,
                  GLX_GREEN_SIZE      , 8,
                  GLX_BLUE_SIZE       , 8,
                  GLX_ALPHA_SIZE      , 8,
                  GLX_DEPTH_SIZE      , 24,
                  GLX_STENCIL_SIZE    , 8,
                  GLX_DOUBLEBUFFER    , True,
                  //GLX_SAMPLE_BUFFERS  , 1,
                  //GLX_SAMPLES         , 4,
                  None
                };

              int glx_major, glx_minor;

              // FBConfigs were added in GLX version 1.3.
              if ( !glXQueryVersion(m_display, &glx_major, &glx_minor ) ||
                   ( ( glx_major == 1 ) && ( glx_minor < 3 ) ) || ( glx_major < 1 ) )
              {
                printf("Invalid GLX version");
                exit(1);
              }

              printf( "Getting matching framebuffer configs\n" );
              int fbcount;
              GLXFBConfig* fbc = glXChooseFBConfig(m_display, DefaultScreen(m_display), visual_attribs, &fbcount);
              if (!fbc)
              {
                printf( "Failed to retrieve a framebuffer config\n" );
                exit(1);
              }
              printf( "Found %d matching FB configs.\n", fbcount );

              // Pick the FB config/visual with the most samples per pixel
              printf( "Getting XVisualInfos\n" );
              int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;

              int i;
              for (i=0; i<fbcount; ++i)
              {
                XVisualInfo *vi = glXGetVisualFromFBConfig(m_display, fbc[i] );
                if ( vi )
                {
                  int samp_buf, samples;
                  glXGetFBConfigAttrib(m_display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf );
                  glXGetFBConfigAttrib(m_display, fbc[i], GLX_SAMPLES       , &samples  );

                  printf( "  Matching fbconfig %d, visual ID 0x%2x: SAMPLE_BUFFERS = %d,"
                          " SAMPLES = %d\n",
                          i, vi -> visualid, samp_buf, samples );

                  if ( best_fbc < 0 || samp_buf && samples > best_num_samp )
                    best_fbc = i, best_num_samp = samples;
                  if ( worst_fbc < 0 || !samp_buf || samples < worst_num_samp )
                    worst_fbc = i, worst_num_samp = samples;
                }
                XFree( vi );
              }

              GLXFBConfig bestFbc = fbc[ best_fbc ];

              // Be sure to free the FBConfig list allocated by glXChooseFBConfig()
              XFree( fbc );

              // Get a visual
              XVisualInfo *vi = glXGetVisualFromFBConfig(m_display, bestFbc );
              XSetWindowAttributes swa;
              Colormap cmap;
              swa.colormap = cmap = XCreateColormap(m_display,
                                                     RootWindow(m_display, vi->screen ),
                                                     vi->visual, AllocNone );
              swa.background_pixmap = None ;
              swa.border_pixel      = 0;
              swa.event_mask        = StructureNotifyMask;

              printf( "Creating window\n" );
              m_window = XCreateWindow(m_display, RootWindow(m_display, vi->screen ),
                                          0, 0, 800, 600, 0, vi->depth, InputOutput,
                                          vi->visual,
                                          CWBorderPixel|CWColormap|CWEventMask, &swa );
              if ( !m_window )
              {
                printf( "Failed to create window.\n" );
                exit(1);
              }

              // Done with the visual info data
              XFree( vi );

              XStoreName(m_display, m_window, "GL 3.0 Window" );

              printf( "Mapping window\n" );
              XMapWindow(m_display, m_window );


              // Done with the visual info data


              // Get the default screen's GLX extension list
              const char *glxExts = glXQueryExtensionsString(m_display,
                                                              DefaultScreen(m_display ) );

              // NOTE: It is not necessary to create or make current to a context before
              // calling glXGetProcAddressARB
              glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
              glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
                       glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );



              // Install an X error handler so the application won't exit if GL 3.0
              // context allocation fails.
              //
              // Note this error handler is global.  All display connections in all threads
              // of a process use the same error handler, so be sure to guard against other
              // threads issuing X commands while this code is running.
              ctxErrorOccurred = false;
              /*int (*oldHandler)(Display*, XErrorEvent*) =
                  XSetErrorHandler(&ctxErrorHandler);*/

              // Check for the GLX_ARB_create_context extension string and the function.
              // If either is not present, use GLX 1.3 context creation method.
             /* if ( !isExtensionSupported( glxExts, "GLX_ARB_create_context" ) ||
                   !glXCreateContextAttribsARB )
              {
                printf( "glXCreateContextAttribsARB() not found"
                        " ... using old-style GLX context\n" );
                m_context = glXCreateNewContext(m_display, bestFbc, GLX_RGBA_TYPE, 0, True );
              }

              // If it does, try to get a GL 3.0 context!
              else
              {
                int context_attribs[] =
                  {
                    GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
                    GLX_CONTEXT_MINOR_VERSION_ARB, 0,
                    //GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
                    None
                  };

                printf( "Creating context\n" );
                m_context = glXCreateContextAttribsARB(m_display, bestFbc, 0,
                                                  True, context_attribs );

                // Sync to ensure any errors generated are processed.
                XSync(m_display, False );
                if ( !ctxErrorOccurred && m_context )
                  printf( "Created GL 3.0 context\n" );
                else
                {
                  // Couldn't create GL 3.0 context.  Fall back to old-style 2.x context.
                  // When a context version below 3.0 is requested, implementations will
                  // return the newest context version compatible with OpenGL versions less
                  // than version 3.0.
                  // GLX_CONTEXT_MAJOR_VERSION_ARB = 1
                  context_attribs[1] = 1;
                  // GLX_CONTEXT_MINOR_VERSION_ARB = 0
                  context_attribs[3] = 0;

                  ctxErrorOccurred = false;

                  printf( "Failed to create GL 3.0 context"
                          " ... using old-style GLX context\n" );
                  m_context = glXCreateContextAttribsARB(m_display, bestFbc, 0,
                                                    True, context_attribs );
                }
              }

              // Sync to ensure any errors generated are processed.
              XSync(m_display, False );

              // Restore the original error handler
              //XSetErrorHandler( oldHandler );

              if ( ctxErrorOccurred || !m_context )
              {
                printf( "Failed to create an OpenGL context\n" );
                exit(1);
              }

              // Verifying that context is a direct context
              if ( ! glXIsDirect (m_display, m_context ) )
              {
                printf( "Indirect GLX rendering context obtained\n" );
              }
              else
              {
                printf( "Direct GLX rendering context obtained\n" );
              }
              glXMakeCurrent(m_display, m_window, m_context );*/
            GLXContext shared = (sharedContext != nullptr) ? static_cast<GlxContext*>(sharedContext)->m_context : nullptr;
            m_settings = settings;
            createSurface(win);
            createContext(static_cast<GlxContext*>(sharedContext));
           /* m_window = win;
            m_windowLess = false;

            //updateSettingsFromWindow();
            XWindowAttributes windowAttributes;
            if (XGetWindowAttributes(m_display, m_window, &windowAttributes) == 0)
            {
                std::cerr << "Failed to get the window attributes" << std::endl;
                return;
            }

            // Get its visuals
            XVisualInfo tpl;
            tpl.screen   = DefaultScreen(m_display);
            tpl.visualid = XVisualIDFromVisual(windowAttributes.visual);
            int nbVisuals = 0;
            XVisualInfo* vi = XGetVisualInfo(m_display, VisualIDMask | VisualScreenMask, &tpl, &nbVisuals);
            //createSurface(win);
            //createContext(static_cast<GlxContext*>(sharedContext));
            // Get a GLXFBConfig that matches the visual
            GLXFBConfig* bestFbc = nullptr;

            // We don't supply attributes to match against, since
            // the visual we are matching against was already
            // deemed suitable in selectBestVisual()
            int nbConfigs = 0;
            GLXFBConfig* configs = glXChooseFBConfig(m_display, DefaultScreen(m_display), NULL, &nbConfigs);
            int bestDepthBits = 0, bestStencilBits = 0, bestAntiAliasingLevel=0;
            for (int i = 0; configs && (i < nbConfigs); ++i)
            {
                XVisualInfo* visual = glXGetVisualFromFBConfig(m_display, configs[i]);

                if (!visual)
                    continue;

                if (visual->visualid == vi->visualid)
                {
                    bestFbc = &configs[i];
                    XFree(visual);
                    break;
                }
                XFree(visual);
            }
            // Get the default screen's GLX extension list
            const char *glxExts = glXQueryExtensionsString(m_display,
                                                          DefaultScreen(m_display ) );
            // NOTE: It is not necessary to create or make current to a context before
            // calling glXGetProcAddressARB

            glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
            glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
            glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );
            // Install an X error handler so the application won't exit if GL 3.0
            // context allocation fails.
            //
            // Note this error handler is global.  All display connections in all threads
            // of a process use the same error handler, so be sure to guard against other
            // threads issuing X commands while this code is running.
            int (*oldHandler)(::Display*, XErrorEvent*) =
              XSetErrorHandler(&ctxErrorHandler);
            // Check for the GLX_ARB_create_context extension string and the function.
            // If either is not present, use GLX 1.3 context creation method.
            if ( !isExtensionSupported( glxExts, "GLX_ARB_create_context" ) ||
               !glXCreateContextAttribsARB )
            {
                 std::cout<<"glXCreateContextAttribsARB() not found"
                       " ... using old-style GLX context"<<std::endl;
                 m_context = glXCreateNewContext(m_display, *bestFbc, GLX_RGBA_TYPE, shared, True );
            }
            // If it does, try to get a GL 3.0 context!
            else
            {
                int context_attribs[] =
                {
                    GLX_CONTEXT_MAJOR_VERSION_ARB, (int) settings.versionMajor,
                    GLX_CONTEXT_MINOR_VERSION_ARB, (int) settings.versionMinor,
                    GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
                    None
                };
                m_context = glXCreateContextAttribsARB(m_display, *bestFbc, shared,
                                                  True, context_attribs );

                // Sync to ensure any errors generated are processed.
                XSync(m_display, False );
                if ( !ctxErrorOccurred && m_context ) {
                    std::cout<<"Created GL 3.0 context"<<std::endl;
                } else
                {
                  // Couldn't create GL 3.0 context.  Fall back to old-style 2.x context.
                  // When a context version below 3.0 is requested, implementations will
                  // return the newest context version compatible with OpenGL versions less
                  // than version 3.0.
                  // GLX_CONTEXT_MAJOR_VERSION_ARB = 1
                  context_attribs[1] = settings.versionMajor;
                  // GLX_CONTEXT_MINOR_VERSION_ARB = 0
                  context_attribs[3] = settings.versionMinor;

                  ctxErrorOccurred = false;

                  std::cout<<"Failed to create GL 3.0 context"
                          " ... using old-style GLX context"<<std::endl;
                  m_context = glXCreateContextAttribsARB(m_display, *bestFbc, shared,
                                                    True, context_attribs );
                }
                // Sync to ensure any errors generated are processed.
                XSync(m_display, False );

                // Restore the original error handler
                XSetErrorHandler( oldHandler );

                if ( ctxErrorOccurred || !m_context )
                {
                    std::cerr<<"Failed to create an OpenGL context"<<std::endl;
                    exit(1);
                }
                // Verifying that context is a direct context
                if ( ! glXIsDirect (m_display, m_context ) )
                {
                    std::cout<<"Indirect GLX rendering context obtained"<<std::endl;
                }
                else
                {
                    std::cout<<"Direct GLX rendering context obtained"<<std::endl;
                }
            }*/
        }
        ////////////////////////////////////////////////////////////
        void GlxContext::createSurface(GlxContext* shared, unsigned int width, unsigned int height, unsigned int bitsPerPixel)
        {
            // Choose the visual according to the context settings
            XVisualInfo visualInfo = selectBestVisual(m_display, bitsPerPixel, m_settings);
            updateSettingsFromVisualInfo(&visualInfo);

            // Check if the shared context already exists and pbuffers are supported
            if (shared && (sfglx_ext_SGIX_pbuffer == sfglx_LOAD_SUCCEEDED))
            {
                // There are no GLX versions prior to 1.0
                int major = 0;
                int minor = 0;

                glXQueryVersion(m_display, &major, &minor);

                // Check if glXCreatePbuffer is available (requires GLX 1.3 or greater)
                bool hasCreatePbuffer = ((major > 1) || (minor >= 3));

                if (hasCreatePbuffer)
                {
                    // Get a GLXFBConfig that matches the visual
                    GLXFBConfig* bestFbc = nullptr;

                    // We don't supply attributes to match against, since
                    // the visual we are matching against was already
                    // deemed suitable in selectBestVisual()
                    int nbConfigs = 0;
                    GLXFBConfig* configs = glXChooseFBConfig(m_display, DefaultScreen(m_display), NULL, &nbConfigs);
                    int bestDepthBits = 0, bestStencilBits = 0, bestAntiAliasingLevel=0;
                    for (int i = 0; configs && (i < nbConfigs); ++i)
                    {
                        XVisualInfo* visual = glXGetVisualFromFBConfig(m_display, configs[i]);

                        if (!visual)
                            continue;

                        if (visual->visualid == visualInfo.visualid)
                        {
                            bestFbc = &configs[i];
                            XFree(visual);
                            break;
                        }
                        XFree(visual);
                    }
                    if (bestFbc)
                    {
                        int attributes[] =
                        {
                            GLX_PBUFFER_WIDTH,  static_cast<int>(width),
                            GLX_PBUFFER_HEIGHT, static_cast<int>(height),
                            None
                        };

                        m_pbuffer = glXCreatePbuffer(m_display, *bestFbc, attributes);

                        XFree(configs);

                        return;
                    }

                    if (configs)
                        XFree(configs);
                }
            }
             // If pbuffers are not available we use a hidden window as the off-screen surface to draw to
            int screen = DefaultScreen(m_display);

            // Define the window attributes
            XSetWindowAttributes attributes;
            attributes.colormap = XCreateColormap(m_display, RootWindow(m_display, screen), visualInfo.visual, AllocNone);

            m_window = XCreateWindow(m_display,
                                     RootWindow(m_display, screen),
                                     0, 0,
                                     width, height,
                                     0,
                                     DefaultDepth(m_display, screen),
                                     InputOutput,
                                     visualInfo.visual,
                                     CWColormap,
                                     &attributes);

            m_ownsWindow = true;

            updateSettingsFromWindow();
        }
        ////////////////////////////////////////////////////////////
        void GlxContext::createSurface(::Window window)
        {
            // A window already exists, so just use it
            m_window = window;

            updateSettingsFromWindow();
        }
        void GlxContext::createContext(GlxContext* shared)
        {
            // Get a working copy of the context settings
            ContextSettings settings = m_settings;

            XVisualInfo* visualInfo = NULL;

            if (m_pbuffer)
            {
                unsigned int fbConfigId = 0;

                glXQueryDrawable(m_display, m_pbuffer, GLX_FBCONFIG_ID, &fbConfigId);

                int attributes[] =
                {
                    GLX_FBCONFIG_ID, static_cast<int>(fbConfigId),
                    0,               0
                };

                int count = 0;
                GLXFBConfig* fbconfig = glXChooseFBConfig(m_display, DefaultScreen(m_display), attributes, &count);

                if (count == 1)
                    visualInfo = glXGetVisualFromFBConfig(m_display, *fbconfig);

                if (fbconfig)
                    XFree(fbconfig);
            }
            else
            {
                // Retrieve the attributes of the target window
                XWindowAttributes windowAttributes;
                if (XGetWindowAttributes(m_display, m_window, &windowAttributes) == 0)
                {
                    err() << "Failed to get the window attributes" << std::endl;
                    return;
                }

                // Get its visuals
                XVisualInfo tpl;
                tpl.screen   = DefaultScreen(m_display);
                tpl.visualid = XVisualIDFromVisual(windowAttributes.visual);
                int nbVisuals = 0;
                visualInfo = XGetVisualInfo(m_display, VisualIDMask | VisualScreenMask, &tpl, &nbVisuals);
            }

            if (!visualInfo)
            {
                err() << "Failed to get visual info" << std::endl;
                return;
            }

            // Get the context to share display lists with
            GLXContext toShare = shared ? shared->m_context : NULL;

                // There are no GLX versions prior to 1.0
                int major = 0;
                int minor = 0;

                if (!glXQueryVersion(m_display, &major, &minor))
                    err() << "Failed to query GLX version, limited to legacy context creation" << std::endl;

                // Check if glXCreateContextAttribsARB is available (requires GLX 1.3 or greater)
                bool hasCreateContextArb = (sfglx_ext_ARB_create_context == sfglx_LOAD_SUCCEEDED) && ((major > 1) || (minor >= 3));

                // Create the OpenGL context -- first try using glXCreateContextAttribsARB
                if (hasCreateContextArb)
                {
                    // Get a GLXFBConfig that matches the window's visual, for glXCreateContextAttribsARB
                    GLXFBConfig* config = NULL;

                    // We don't supply attributes to match against, since
                    // the visual we are matching against was already
                    // deemed suitable in selectBestVisual()
                    int nbConfigs = 0;
                    GLXFBConfig* configs = glXChooseFBConfig(m_display, DefaultScreen(m_display), NULL, &nbConfigs);

                    for (int i = 0; configs && (i < nbConfigs); ++i)
                    {
                        XVisualInfo* visual = glXGetVisualFromFBConfig(m_display, configs[i]);

                        if (!visual)
                            continue;

                        if (visual->visualid == visualInfo->visualid)
                        {
                            config = &configs[i];
                            XFree(visual);
                            break;
                        }

                        XFree(visual);
                    }

                    if (!config)
                        err() << "Failed to get GLXFBConfig which corresponds to the window's visual" << std::endl;

                    while (config && !m_context && m_settings.versionMajor)
                    {
                        std::vector<int> attributes;

                        // Check if the user requested a specific context version (anything > 1.1)
                        if ((m_settings.versionMajor > 1) || ((m_settings.versionMajor == 1) && (m_settings.versionMinor > 1)))
                        {
                            attributes.push_back(GLX_CONTEXT_MAJOR_VERSION_ARB);
                            attributes.push_back(m_settings.versionMajor);
                            attributes.push_back(GLX_CONTEXT_MINOR_VERSION_ARB);
                            attributes.push_back(m_settings.versionMinor);
                        }

                        // Check if setting the profile is supported
                        if (sfglx_ext_ARB_create_context_profile == sfglx_LOAD_SUCCEEDED)
                        {
                            int profile = (m_settings.attributeFlags & ContextSettings::Core) ? GLX_CONTEXT_CORE_PROFILE_BIT_ARB : GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
                            int debug = (m_settings.attributeFlags & ContextSettings::Debug) ? GLX_CONTEXT_DEBUG_BIT_ARB : 0;

                            attributes.push_back(GLX_CONTEXT_PROFILE_MASK_ARB);
                            attributes.push_back(profile);
                            attributes.push_back(GLX_CONTEXT_FLAGS_ARB);
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

                        // RAII GLX error handler (we simply ignore errors here)
                        // On an error, glXCreateContextAttribsARB will return 0 anyway
                        GlxErrorHandler handler(m_display);

                        if (toShare)
                        {
                            if (!glXMakeCurrent(m_display, None, NULL))
                            {
                                err() << "Failed to deactivate shared context before sharing" << std::endl;
                                return;
                            }
                        }

                        // Create the context
                        m_context = glXCreateContextAttribsARB(m_display, *config, toShare, true, &attributes[0]);

                        if (!m_context)
                        {
                            std::cout<<"failed to create the context"<<std::endl;
                            // If we couldn't create the context, first try disabling flags,
                            // then lower the version number and try again -- stop at 0.0
                            // Invalid version numbers will be generated by this algorithm (like 3.9), but we really don't care
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

                    if (configs)
                        XFree(configs);
                }

                // If glXCreateContextAttribsARB failed, use glXCreateContext
                if (!m_context)
                {
                    // set the context version to 2.1 (arbitrary) and disable flags
                    m_settings.versionMajor = 2;
                    m_settings.versionMinor = 1;
                    m_settings.attributeFlags = ContextSettings::Default;

                    #if defined(GLX_DEBUGGING)
                        GlxErrorHandler handler(m_display);
                    #endif

                    if (toShare)
                    {
                        if (!glXMakeCurrent(m_display, None, NULL))
                        {
                            err() << "Failed to deactivate shared context before sharing" << std::endl;
                            return;
                        }
                    }

                    // Create the context, using the target window's visual
                    m_context = glXCreateContext(m_display, visualInfo, toShare, true);

                #if defined(GLX_DEBUGGING)
                    if (glxErrorOccurred)
                        err() << "GLX error in GlxContext::createContext()" << std::endl;
                #endif
            }

            if (!m_context)
                err() << "Failed to create an OpenGL context for this window" << std::endl;

            // Free the visual info
            XFree(visualInfo);
        }
        bool GlxContext::setActive(bool current) {
            if (!m_context)
                return false;

            #if defined(GLX_DEBUGGING)
                GlxErrorHandler handler(m_display);
            #endif

            bool result = false;

            if (current)
            {
                if (m_pbuffer)
                {
                    result = glXMakeContextCurrent(m_display, m_pbuffer, m_pbuffer, m_context);
                }
                else if (m_window)
                {
                    result = glXMakeCurrent(m_display, m_window, m_context);
                }
            }
            else
            {
                result = glXMakeCurrent(m_display, None, NULL);
            }

            #if defined(GLX_DEBUGGING)
                if (glxErrorOccurred)
                    err() << "GLX error in GlxContext::makeCurrent()" << std::endl;
            #endif

            return result;
        }
        const ContextSettings& GlxContext::getSettings() const {
            return m_settings;
        }
        void GlxContext::updateSettingsFromWindow() {
            // Retrieve the attributes of the target window
            XWindowAttributes windowAttributes;
            if (XGetWindowAttributes(m_display, m_window, &windowAttributes) == 0)
            {
                std::cerr << "Failed to get the window attributes" << std::endl;
                return;
            }

            // Get its visuals
            XVisualInfo tpl;
            tpl.screen   = DefaultScreen(m_display);
            tpl.visualid = XVisualIDFromVisual(windowAttributes.visual);
            int nbVisuals = 0;
            XVisualInfo* vi = XGetVisualInfo(m_display, VisualIDMask | VisualScreenMask, &tpl, &nbVisuals);
            updateSettingsFromVisualInfo(vi);
        }
        ////////////////////////////////////////////////////////////
        void GlxContext::updateSettingsFromVisualInfo(XVisualInfo* visualInfo)
        {
            // Update the creation settings from the chosen format
            int depth, stencil, multiSampling, samples, sRgb;
            glXGetConfig(m_display, visualInfo, GLX_DEPTH_SIZE,   &depth);
            glXGetConfig(m_display, visualInfo, GLX_STENCIL_SIZE, &stencil);

            if (sfglx_ext_ARB_multisample == sfglx_LOAD_SUCCEEDED)
            {
                glXGetConfig(m_display, visualInfo, GLX_SAMPLE_BUFFERS_ARB, &multiSampling);
                glXGetConfig(m_display, visualInfo, GLX_SAMPLES_ARB,        &samples);
            }
            else
            {
                multiSampling = 0;
                samples = 0;
            }

            if ((sfglx_ext_EXT_framebuffer_sRGB == sfglx_LOAD_SUCCEEDED) || (sfglx_ext_ARB_framebuffer_sRGB == sfglx_LOAD_SUCCEEDED))
            {
                glXGetConfig(m_display, visualInfo, GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB, &sRgb);
            }
            else
            {
                sRgb = 0;
            }

            m_settings.depthBits         = static_cast<unsigned int>(depth);
            m_settings.stencilBits       = static_cast<unsigned int>(stencil);
            m_settings.antiAliasingLevel = multiSampling ? samples : 0;
            m_settings.sRgbCapable       = (sRgb == True);
        }
        void GlxContext::display() {
            if (m_pbuffer)
                glXSwapBuffers(m_display, m_pbuffer);
            else if (m_window)
                glXSwapBuffers(m_display, m_window);
        }
        void GlxContext::setVerticalSyncEnabled(bool enabled) {
             int result = 0;

            // Prioritize the EXT variant and fall back to MESA or SGI if needed
            // We use the direct pointer to the MESA entry point instead of the alias
            // because glx.h declares the entry point as an external function
            // which would require us to link in an additional library
            if (sfglx_ext_EXT_swap_control == sfglx_LOAD_SUCCEEDED)
            {
                glXSwapIntervalEXT(m_display, m_pbuffer ? m_pbuffer : m_window, enabled ? 1 : 0);
            }
            else if (sfglx_ext_MESA_swap_control == sfglx_LOAD_SUCCEEDED)
            {
                result = sf_ptrc_glXSwapIntervalMESA(enabled ? 1 : 0);
            }
            else if (sfglx_ext_SGI_swap_control == sfglx_LOAD_SUCCEEDED)
            {
                result = glXSwapIntervalSGI(enabled ? 1 : 0);
            }
            else
            {
                static bool warned = false;

                if (!warned)
                {
                    err() << "Setting vertical sync not supported" << std::endl;

                    warned = true;
                }
            }

            if (result != 0)
                err() << "Setting vertical sync failed" << std::endl;
        }
        GlxContext::~GlxContext() {
             // Destroy the context
            if (m_context)
            {
                #if defined(GLX_DEBUGGING)
                        GlxErrorHandler handler(m_display);
                #endif

                if (glXGetCurrentContext() == m_context)
                    glXMakeCurrent(m_display, None, NULL);
                glXDestroyContext(m_display, m_context);

                #if defined(GLX_DEBUGGING)
                        if (glxErrorOccurred)
                            err() << "GLX error in GlxContext::~GlxContext()" << std::endl;
                #endif
            }

            if (m_pbuffer)
            {
                glXDestroyPbuffer(m_display, m_pbuffer);
            }

            // Destroy the window if we own it
            if (m_window && m_ownsWindow)
            {
                XDestroyWindow(m_display, m_window);
                XFlush(m_display);
            }
            Display::closeDisplay(m_display);
        }
    }
}
