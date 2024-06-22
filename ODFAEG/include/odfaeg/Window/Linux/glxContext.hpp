#ifndef ODFAEG_GLX_CONTEXT
#define ODFAEG_GLX_CONTEXT
#include <GL/glx.h>
#include "../contextSettings.hpp"
#include "../iContext.hpp"


namespace odfaeg {
    namespace window {
        class GlxContext : public IContext {
            public :
                GlxContext();
                static GlFunctionPointer getFunction(const char* name);
                static XVisualInfo selectBestVisual(::Display* display, unsigned int bitsPerPixel, const ContextSettings& settings);
                virtual bool setActive(bool current);
                virtual void create(IContext* sharedContext = nullptr);
                virtual void create(::Window window, const ContextSettings& settings, IContext* shared = nullptr, unsigned int bitsPerPixel = 32);
                virtual void create(const ContextSettings& settings, unsigned int width, unsigned int height, IContext* shared = nullptr);
                virtual const ContextSettings& getSettings() const;
                virtual void display();
                void setVerticalSyncEnabled(bool enabled);
                ~GlxContext();
                bool m_windowLess;
            protected :
                virtual void initialize(const ContextSettings& settings) {}
                ContextSettings m_settings;
            private :
                void updateSettingsFromWindow();
                void createSurface(GlxContext* sharedContext, unsigned int width, unsigned int height, unsigned int bitsPerPixel);
                void createSurface(::Window window);
                void createContext(GlxContext* sharedContext);
                void updateSettingsFromVisualInfo(XVisualInfo* visualInfo);
                void checkSettings ();
                static bool isExtensionSupported(const char *extList, const char *extension);
                static int ctxErrorHandler(_XDisplay *dpy, XErrorEvent *ev );
                static bool ctxErrorOccurred;
                bool m_ownsWindow;
                ::Display* m_display;
                ::Window m_window;
                GLXContext m_context;
                GLXPbuffer m_pbuffer;
        };
    }
}
#endif
