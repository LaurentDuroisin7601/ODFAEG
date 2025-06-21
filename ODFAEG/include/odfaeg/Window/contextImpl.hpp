#ifndef ODFAEG_CONTEXTIMPL_HPP
#define ODFAEG_CONTEXTIMPL_HPP
#include "contextSettings.hpp"
#include "../../../include/odfaeg/config.hpp"
#if defined(ODFAEG_SYSTEM_LINUX)
#include "../../../include/odfaeg/Window/Linux/glxContext.hpp"
typedef odfaeg::window::GlxContext ContextImplType;
#elif defined(ODFAEG_SYSTEM_WINDOWS)
#include "../../../include/odfaeg/Window/Windows/wglContext.hpp"
typedef odfaeg::window::WglContext ContextImplType;
#endif
#include <vector>
#include <string>
#include "../../../include/odfaeg/Window/context.hpp"
#include "glResource.hpp"
#include "windowHandle.hpp"
#include <set>
namespace odfaeg {
    namespace window {
        class ThreadLocalContext {
        public:
            static ContextImplType*& get() {
                thread_local ContextImplType* ctx = nullptr;
                if (!ctx)
                    ctx = new ContextImplType(); // ou via ThreadLocalContext::get()
                return ctx;
            }
        };
        class ODFAEG_WINDOW_API ContextImpl : public ContextImplType {
            public :
                ContextImpl();

                static void initResource();
                static void cleanupResource();
                static GlFunctionPointer getFunction(const char* name);
                static int evaluateFormat(unsigned int bitsPerPixel, const ContextSettings& settings, int colorBits, int depthBits, int stencilBits, int antialiasing, bool accelerated, bool sRgb);
                static bool isExtensionAvalaible(const char* name);
                void create(IContext* shared);
                void create(WindowHandle handle, const ContextSettings& settings, IContext* shared = nullptr, unsigned int bitsPerPixel = 32);
                void create(const ContextSettings& settings, unsigned int width, unsigned int height, IContext* shared = nullptr);
                bool setActive(bool active);
                const ContextSettings& getSettings() const;
                void display();
                std::uint64_t getActiveContextId();
                void cleanupUnsharedResources();
                void setVerticalSyncEnabled(bool enabled);
                /*static void registerContextDestroyCallback(ContextDestroyCallback contextDestoyCallback, void* arg);
                static void acquireTransientContext();
                static void releaseTransientContext();*/
                void checkSettings(const ContextSettings& settings);
                ~ContextImpl();
                static IContext* sharedContext;
            private :
                bool parseVersionString(const char* version, const char* prefix, unsigned int &major, unsigned int &minor);
                void initialize(const ContextSettings& requestedSettings);
                // OpenGL resources counter
                static unsigned int resourceCount;
                static std::vector<std::string> extensions;
                static unsigned int nbContexts;

                static odfaeg::window::ContextImpl* current_ContextImpl;
                static std::recursive_mutex rec_mutex;
                // This structure contains all the state necessary to
                // track TransientContext usage
                // This per-thread variable tracks if and how a transient
                // context is currently being used on the current thread
                /*struct TransientContext : private sf::NonCopyable
                {
                    ////////////////////////////////////////////////////////////
                    /// \brief Constructor
                    ///
                    ////////////////////////////////////////////////////////////
                    TransientContext() :
                    referenceCount   (0),
                    context          (0),
                    sharedContextLock(0),
                    useSharedContext (false)
                    {
                        if (resourceCount == 0)
                        {
                            context = new odfaeg::window::Context;
                        }
                        else if (!current_ContextImpl)
                        {
                            sharedContextLock = new sf::Lock(rec_mutex);
                            useSharedContext = true;
                            sharedContext->setActive(true);
                        }
                    }

                    ////////////////////////////////////////////////////////////
                    /// \brief Destructor
                    ///
                    ////////////////////////////////////////////////////////////
                    ~TransientContext()
                    {
                        if (useSharedContext)
                            sharedContext->setActive(false);

                        delete sharedContextLock;
                        delete context;
                    }

                    ///////////////////////////////////////////////////////////
                    // Member data
                    ////////////////////////////////////////////////////////////
                    unsigned int referenceCount;
                    Context* context;
                    sf::Lock*    sharedContextLock;
                    bool         useSharedContext;
                };
                static sf::ThreadLocalPtr<TransientContext> transientContext;
                typedef std::set<std::pair<ContextDestroyCallback, void*> > ContextDestroyCallbacks;
                static ContextDestroyCallbacks contextDestroyCallbacks;*/
                static std::uint64_t id;
                const std::uint64_t m_id; ///< Unique number that identifies the context
        };
    }
}
#endif
