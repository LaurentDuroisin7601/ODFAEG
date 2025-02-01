#include "../../../include/odfaeg/Window/contextImpl.hpp"
#include "../../../include/odfaeg/Window/windowImpl.hpp"
#include <SFML/System/Mutex.hpp>
#include <SFML/System/Lock.hpp>
#include <SFML/OpenGL.hpp>
#include <iostream>
#include <cassert>
#if defined(ODFAEG_SYSTEM_WINDOWS)

    typedef const GLubyte* (APIENTRY *glGetStringiFuncType)(GLenum, GLuint);

#else

    typedef const GLubyte* (*glGetStringiFuncType)(GLenum, GLuint);

#endif
#if !defined(GL_MULTISAMPLE)
    #define GL_MULTISAMPLE 0x809D
#endif

#if !defined(GL_MAJOR_VERSION)
    #define GL_MAJOR_VERSION 0x821B
#endif

#if !defined(GL_MINOR_VERSION)
    #define GL_MINOR_VERSION 0x821C
#endif

#if !defined(GL_NUM_EXTENSIONS)
    #define GL_NUM_EXTENSIONS 0x821D
#endif

#if !defined(GL_CONTEXT_FLAGS)
    #define GL_CONTEXT_FLAGS 0x821E
#endif

#if !defined(GL_FRAMEBUFFER_SRGB)
    #define GL_FRAMEBUFFER_SRGB 0x8DB9
#endif

#if !defined(GL_CONTEXT_FLAG_DEBUG_BIT)
    #define GL_CONTEXT_FLAG_DEBUG_BIT 0x00000002
#endif

#if !defined(GL_CONTEXT_PROFILE_MASK)
    #define GL_CONTEXT_PROFILE_MASK 0x9126
#endif

#if !defined(GL_CONTEXT_CORE_PROFILE_BIT)
    #define GL_CONTEXT_CORE_PROFILE_BIT 0x00000001
#endif

#if !defined(GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)
    #define GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0x00000002
#endif
using namespace sf;
namespace odfaeg {
    namespace window {
        std::vector<std::string> ContextImpl::extensions = std::vector<std::string>();
        unsigned int ContextImpl::nbContexts = 0;
        unsigned int ContextImpl::resourceCount = 0;
        sf::Uint64 ContextImpl::id = 1;
        sf::Mutex ContextImpl::mutex;
        sf::ThreadLocalPtr<ContextImpl> ContextImpl::current_ContextImpl(nullptr);
        //sf::ThreadLocalPtr<ContextImpl::TransientContext> ContextImpl::transientContext(nullptr);
        IContext* ContextImpl::sharedContext = nullptr;
        //std::set<std::pair<ContextDestroyCallback, void*>> ContextImpl::contextDestroyCallbacks = std::set<std::pair<ContextDestroyCallback, void*>>();
        // Helper to parse OpenGL version strings
        bool ContextImpl::parseVersionString(const char* version, const char* prefix, unsigned int &major, unsigned int &minor)
        {
            std::size_t prefixLength = strlen(prefix);

            if ((strlen(version) >= (prefixLength + 3)) &&
                (strncmp(version, prefix, prefixLength) == 0) &&
                std::isdigit(version[prefixLength]) &&
                (version[prefixLength + 1] == '.') &&
                std::isdigit(version[prefixLength + 2]))
            {
                major = version[prefixLength] - '0';
                minor = version[prefixLength + 2] - '0';

                return true;
            }

            return false;
        }
        void ContextImpl::initResource() {
            // Protect from concurrent access
            sf::Lock lock(mutex);
            if (nbContexts == 0) {
                sharedContext = new ContextImplType();
                ContextSettings settings;
                sharedContext->create(settings, 1, 1);
                sharedContext->setActive(true);
                // Load our extensions vector
                extensions.clear();

                // Check whether a >= 3.0 context is available
                int majorVersion = 0;
                glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);

                if (glGetError() == GL_INVALID_ENUM)
                {
                    // Try to load the < 3.0 way
                    const char* extensionString = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));

                    do
                    {
                        const char* extension = extensionString;

                        while(*extensionString && (*extensionString != ' '))
                            extensionString++;

                        extensions.push_back(std::string(extension, extensionString));
                    }
                    while (*extensionString++);
                }
                else
                {
                    // Try to load the >= 3.0 way
                    glGetStringiFuncType glGetStringiFunc = NULL;
                    glGetStringiFunc = reinterpret_cast<glGetStringiFuncType>(getFunction("glGetStringi"));

                    if (glGetStringiFunc)
                    {
                        int numExtensions = 0;
                        glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

                        if (numExtensions)
                        {
                            for (unsigned int i = 0; i < static_cast<unsigned int>(numExtensions); ++i)
                            {
                                const char* extensionString = reinterpret_cast<const char*>(glGetStringiFunc(GL_EXTENSIONS, i));

                                extensions.push_back(extensionString);
                            }
                        }
                    }
                }
                sharedContext->setActive(false);
            }
            nbContexts++;
        }
        void ContextImpl::cleanupResource() {
            nbContexts--;
            if (nbContexts == 0) {
                sharedContext->setActive(false);
                delete sharedContext;
            }
        }
        ContextImpl::ContextImpl() : m_id(id++) {
            /*if (nbContexts == 0) {
                std::cout<<"create the shared context"<<std::endl;
                sharedContext = new ContextImplType();
                ContextSettings settings;
                sharedContext->create(settings, 1, 1);
                sharedContext->setActive(true);
                // Load our extensions vector
                extensions.clear();

                // Check whether a >= 3.0 context is available
                int majorVersion = 0;
                glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);

                if (glGetError() == GL_INVALID_ENUM)
                {
                    // Try to load the < 3.0 way
                    const char* extensionString = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));

                    do
                    {
                        const char* extension = extensionString;

                        while(*extensionString && (*extensionString != ' '))
                            extensionString++;

                        extensions.push_back(std::string(extension, extensionString));
                    }
                    while (*extensionString++);
                }
                else
                {
                    // Try to load the >= 3.0 way
                    glGetStringiFuncType glGetStringiFunc = NULL;
                    glGetStringiFunc = reinterpret_cast<glGetStringiFuncType>(getFunction("glGetStringi"));

                    if (glGetStringiFunc)
                    {
                        int numExtensions = 0;
                        glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

                        if (numExtensions)
                        {
                            for (unsigned int i = 0; i < static_cast<unsigned int>(numExtensions); ++i)
                            {
                                const char* extensionString = reinterpret_cast<const char*>(glGetStringiFunc(GL_EXTENSIONS, i));

                                extensions.push_back(extensionString);
                            }
                        }
                    }
                }
                sharedContext->setActive(false);
            }
            nbContexts++;*/

        }
        void ContextImpl::create(IContext* shared) {
            sharedContext->setActive(true);
            ContextImplType::create((shared == nullptr) ? sharedContext : shared);
            sharedContext->setActive(false);
            initialize(ContextSettings());
        }
        void ContextImpl::create(const ContextSettings& settings, unsigned int width, unsigned int height, IContext* shared) {
            sharedContext->setActive(true);
            ContextImplType::create(settings, width, height, (shared == nullptr) ? sharedContext : shared);
            sharedContext->setActive(false);
            initialize(settings);
            checkSettings(settings);
        }
        void ContextImpl::create(sf::WindowHandle handle,const ContextSettings& settings, IContext* shared, unsigned int bitsPerPixel) {
            sharedContext->setActive(true);
            ContextImplType::create(handle, settings, (shared == nullptr) ? sharedContext : shared, bitsPerPixel);
            sharedContext->setActive(false);
            initialize(settings);
            //checkSettings(settings);*/
        }
        bool ContextImpl::setActive(bool active) {
            if (active) {
                if (this != current_ContextImpl) {
                    Lock lock(mutex);
                    if (ContextImplType::setActive(true)) {
                        current_ContextImpl = this;

                        return true;
                    } else {
                        return false;
                    }
                } else {
                    return true;
                }
            } else {
                if (this == current_ContextImpl) {
                    if (ContextImplType::setActive(false)) {
                        current_ContextImpl = nullptr;
                        return true;
                    } else {
                        return false;
                    }
                } else {
                    return true;
                }
            }
        }
        const ContextSettings& ContextImpl::getSettings() const {
            return m_settings;
        }
        void ContextImpl::display() {
            ContextImplType::display();
        }
        void ContextImpl::setVerticalSyncEnabled (bool enabled) {
            ContextImplType::setVerticalSyncEnabled(enabled);
        }
        ////////////////////////////////////////////////////////////
        int ContextImpl::evaluateFormat(unsigned int bitsPerPixel, const ContextSettings& settings, int colorBits, int depthBits, int stencilBits, int antialiasing, bool accelerated, bool sRgb)
        {
            int colorDiff        = static_cast<int>(bitsPerPixel)               - colorBits;
            int depthDiff        = static_cast<int>(settings.depthBits)         - depthBits;
            int stencilDiff      = static_cast<int>(settings.stencilBits)       - stencilBits;
            int antialiasingDiff = static_cast<int>(settings.antiAliasingLevel) - antialiasing;

            // Weight sub-scores so that better settings don't score equally as bad as worse settings
            colorDiff        *= ((colorDiff        > 0) ? 100000 : 1);
            depthDiff        *= ((depthDiff        > 0) ? 100000 : 1);
            stencilDiff      *= ((stencilDiff      > 0) ? 100000 : 1);
            antialiasingDiff *= ((antialiasingDiff > 0) ? 100000 : 1);

            // Aggregate the scores
            int score = std::abs(colorDiff) + std::abs(depthDiff) + std::abs(stencilDiff) + std::abs(antialiasingDiff);

            // If the user wants an sRGB capable format, try really hard to get one
            if (settings.sRgbCapable && !sRgb)
                score += 10000000;

            // Make sure we prefer hardware acceleration over features
            if (!accelerated)
                score += 100000000;

            return score;
        }
        ////////////////////////////////////////////////////////////
        GlFunctionPointer ContextImpl::getFunction(const char* name)
        {
            #if !defined(ODFAEG_OPENGL_ES)

                Lock lock(mutex);

                return ContextImplType::getFunction(name);

            #else

                return 0;

            #endif
        }
        bool ContextImpl::isExtensionAvalaible(const char* name) {
            for (unsigned int i = 0; i < extensions.size(); i++) {
                if (extensions[i] == name)
                    return true;
            }
            return false;
        }
        ////////////////////////////////////////////////////////////
       /* void ContextImpl::acquireTransientContext()
        {
            // Protect from concurrent access
            Lock lock(mutex);

            // If this is the first TransientContextLock on this thread
            // construct the state object
            if (!transientContext)
                transientContext = new TransientContext;

            // Increase the reference count
            transientContext->referenceCount++;
        }


        ////////////////////////////////////////////////////////////
        void ContextImpl::releaseTransientContext()
        {
            // Protect from concurrent access
            Lock lock(mutex);

            // Make sure a matching acquireTransientContext() was called
            assert(transientContext);

            // Decrease the reference count
            transientContext->referenceCount--;

            // If this is the last TransientContextLock that is released
            // destroy the state object
            if (transientContext->referenceCount == 0)
            {
                delete transientContext;
                transientContext = NULL;
            }
        }*/
        ////////////////////////////////////////////////////////////
        Uint64 ContextImpl::getActiveContextId()
        {
            return current_ContextImpl ? current_ContextImpl->m_id : 0;
        }

        ////////////////////////////////////////////////////////////
        /*void ContextImpl::registerContextDestroyCallback(ContextDestroyCallback callback, void* arg)
        {
            contextDestroyCallbacks.insert(std::make_pair(callback, arg));
        }
        ////////////////////////////////////////////////////////////
        void ContextImpl::cleanupUnsharedResources()
        {
            // Save the current context so we can restore it later
            ContextImpl* contextToRestore = current_ContextImpl;

            // If this context is already active there is no need to save it
            if (contextToRestore == this)
                contextToRestore = NULL;

            // Make this context active so resources can be freed
            setActive(true);

            // Call the registered destruction callbacks
            for (ContextDestroyCallbacks::iterator iter = contextDestroyCallbacks.begin(); iter != contextDestroyCallbacks.end(); ++iter)
                iter->first(iter->second);

            // Make the originally active context active again
            if (contextToRestore)
                contextToRestore->setActive(true);
        }*/
        ////////////////////////////////////////////////////////////
        void ContextImpl::initialize(const ContextSettings& requestedSettings)
        {
            // Activate the context
            setActive(true);

            // Retrieve the context version number
            int majorVersion = 0;
            int minorVersion = 0;
           /* m_settings.versionMajor = 3;
            m_settings.versionMinor = 3;
            m_settings.attributeFlags = ContextSettings::Default;*/

            // Try the new way first
            glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
            glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
            if (glGetError() != GL_INVALID_ENUM)
            {
                m_settings.versionMajor = static_cast<unsigned int>(majorVersion);
                m_settings.versionMinor = static_cast<unsigned int>(minorVersion);
            }
            else
            {
                // Try the old way

                // If we can't get the version number, assume 1.1
                m_settings.versionMajor = 1;
                m_settings.versionMinor = 1;

                const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
                if (version)
                {
                    // OpenGL ES Common Lite profile: The beginning of the returned string is "OpenGL ES-CL major.minor"
                    // OpenGL ES Common profile:      The beginning of the returned string is "OpenGL ES-CM major.minor"
                    // OpenGL ES Full profile:        The beginning of the returned string is "OpenGL ES major.minor"
                    // Desktop OpenGL:                The beginning of the returned string is "major.minor"

                    if (!parseVersionString(version, "OpenGL ES-CL ", m_settings.versionMajor, m_settings.versionMinor) &&
                        !parseVersionString(version, "OpenGL ES-CM ", m_settings.versionMajor, m_settings.versionMinor) &&
                        !parseVersionString(version, "OpenGL ES ",    m_settings.versionMajor, m_settings.versionMinor) &&
                        !parseVersionString(version, "",              m_settings.versionMajor, m_settings.versionMinor))
                    {
                        err() << "Unable to parse OpenGL version string: \"" << version << "\", defaulting to 1.1" << std::endl;
                    }
                }
                else
                {
                    err() << "Unable to retrieve OpenGL version string, defaulting to 1.1" << std::endl;
                }
            }

            // 3.0 contexts only deprecate features, but do not remove them yet
            // 3.1 contexts remove features if ARB_compatibility is not present
            // 3.2+ contexts remove features only if a core profile is requested

            // If the context was created with wglCreateContext, it is guaranteed to be compatibility.
            // If a 3.0 context was created with wglCreateContextAttribsARB, it is guaranteed to be compatibility.
            // If a 3.1 context was created with wglCreateContextAttribsARB, the compatibility flag
            // is set only if ARB_compatibility is present
            // If a 3.2+ context was created with wglCreateContextAttribsARB, the compatibility flag
            // would have been set correctly already depending on whether ARB_create_context_profile is supported.

            // If the user requests a 3.0 context, it will be a compatibility context regardless of the requested profile.
            // If the user requests a 3.1 context and its creation was successful, the specification
            // states that it will not be a compatibility profile context regardless of the requested
            // profile unless ARB_compatibility is present.

            m_settings.attributeFlags = ContextSettings::Default;

            if (m_settings.versionMajor >= 3)
            {
                // Retrieve the context flags
                int flags = 0;
                glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

                if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
                    m_settings.attributeFlags |= ContextSettings::Debug;

                if ((m_settings.versionMajor  == 3) && (m_settings.versionMajor  == 1))
                {
                    m_settings.attributeFlags |= ContextSettings::Core;

                    glGetStringiFuncType glGetStringiFunc = reinterpret_cast<glGetStringiFuncType>(getFunction("glGetStringi"));

                    if (glGetStringiFunc)
                    {
                        int numExtensions = 0;
                        glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

                        for (unsigned int i = 0; i < static_cast<unsigned int>(numExtensions); ++i)
                        {
                            const char* extensionString = reinterpret_cast<const char*>(glGetStringiFunc(GL_EXTENSIONS, i));

                            if (strstr(extensionString, "GL_ARB_compatibility"))
                            {
                                m_settings.attributeFlags &= ~static_cast<Uint32>(ContextSettings::Core);
                                break;
                            }
                        }
                    }
                }
                else if ((m_settings.versionMajor > 3) || (m_settings.versionMinor >= 2))
                {
                    // Retrieve the context profile
                    int profile = 0;
                    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile);

                    if (profile & GL_CONTEXT_CORE_PROFILE_BIT)
                        m_settings.attributeFlags |= ContextSettings::Core;
                }
            }

            // Enable anti-aliasing if requested by the user and supported
            if ((requestedSettings.antiAliasingLevel > 0) && (m_settings.antiAliasingLevel > 0))
            {
                glEnable(GL_MULTISAMPLE);
            }
            else
            {
                m_settings.antiAliasingLevel = 0;
            }

            // Enable sRGB if requested by the user and supported
            if (requestedSettings.sRgbCapable && m_settings.sRgbCapable)
            {
                glEnable(GL_FRAMEBUFFER_SRGB);

                // Check to see if the enable was successful
                if (glIsEnabled(GL_FRAMEBUFFER_SRGB) == GL_FALSE)
                {
                    err() << "Warning: Failed to enable GL_FRAMEBUFFER_SRGB" << std::endl;
                    m_settings.sRgbCapable = false;
                }
            }
            else
            {
                m_settings.sRgbCapable = false;
            }
        }
        ///////////////////////////////////////////////////////////
        void ContextImpl::checkSettings(const ContextSettings& requestedSettings)
        {
            // Perform checks to inform the user if they are getting a context they might not have expected

            // Detect any known non-accelerated implementations and warn
            const char* vendorName = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
            const char* rendererName = reinterpret_cast<const char*>(glGetString(GL_RENDERER));

            if (vendorName && rendererName)
            {
                if ((strcmp(vendorName, "Microsoft Corporation") == 0) && (strcmp(rendererName, "GDI Generic") == 0))
                {
                    err() << "Warning: Detected \"Microsoft Corporation GDI Generic\" OpenGL implementation" << std::endl
                          << "The current OpenGL implementation is not hardware-accelerated" << std::endl;
                }
            }
            int version = m_settings.versionMajor * 10 + m_settings.versionMinor;
            int requestedVersion = requestedSettings.versionMajor * 10 + requestedSettings.versionMinor;

            if ((m_settings.attributeFlags    != requestedSettings.attributeFlags)    ||
                (version                      <  requestedVersion)                    ||
                (m_settings.stencilBits       <  requestedSettings.stencilBits)       ||
                (m_settings.antiAliasingLevel <  requestedSettings.antiAliasingLevel) ||
                (m_settings.depthBits         <  requestedSettings.depthBits)         ||
                (!m_settings.sRgbCapable      && requestedSettings.sRgbCapable))
            {
                err() << "Warning: The created OpenGL context does not fully meet the settings that were requested" << std::endl;
                err() << "Requested: version = " << requestedSettings.versionMajor << "." << requestedSettings.versionMinor
                      << " ; depth bits = " << requestedSettings.depthBits
                      << " ; stencil bits = " << requestedSettings.stencilBits
                      << " ; AA level = " << requestedSettings.antiAliasingLevel
                      << std::boolalpha
                      << " ; core = " << ((requestedSettings.attributeFlags & ContextSettings::Core) != 0)
                      << " ; debug = " << ((requestedSettings.attributeFlags & ContextSettings::Debug) != 0)
                      << " ; sRGB = " << requestedSettings.sRgbCapable
                      << std::noboolalpha << std::endl;
                err() << "Created: version = " << m_settings.versionMajor << "." << m_settings.versionMinor
                      << " ; depth bits = " << m_settings.depthBits
                      << " ; stencil bits = " << m_settings.stencilBits
                      << " ; AA level = " << m_settings.antiAliasingLevel
                      << std::boolalpha
                      << " ; core = " << ((m_settings.attributeFlags & ContextSettings::Core) != 0)
                      << " ; debug = " << ((m_settings.attributeFlags & ContextSettings::Debug) != 0)
                      << " ; sRGB = " << m_settings.sRgbCapable
                      << std::noboolalpha << std::endl;
            }
        }
        ContextImpl::~ContextImpl() {
            //cleanupUnsharedResources();
            /*nbContexts--;
            if (nbContexts == 0) {
                sharedContext->setActive(false);
                delete sharedContext;
            }*/
        }
    }
}
