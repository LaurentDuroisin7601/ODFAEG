#ifndef ODFAEG_CONTEXT_HPP
#define ODFAEG_CONTEXT_HPP
#include "iContext.hpp"
#include "contextFactory.hpp"
namespace odfaeg {
    namespace window {
        class Context {
            public :
                Context();
                Context(ContextSettings settings, unsigned int width, unsigned int height);
                Context(sf::WindowHandle window, ContextSettings settings);
                void create (ContextSettings settings, unsigned int width, unsigned int height);
                void create (sf::WindowHandle window, ContextSettings settings);
                /*void display();
                void setVerticalSyncEnabled(bool enabled);*/
                bool setActive(bool active);
                const ContextSettings& getSettings() const;
                static GlFunctionPointer getFunction(const char* name);
                static bool isExtensionAvailable(const char* name);
                ~Context();
            private :
                IContext* m_context;
        };
    }
}
#endif
