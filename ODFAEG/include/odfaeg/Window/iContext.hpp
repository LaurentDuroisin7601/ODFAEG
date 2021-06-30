#ifndef ODFAEG_ICONTEXT_HPP
#define ODFAEG_ICONTEXT_HPP
#include "contextSettings.hpp"
#include <SFML/Window/WindowHandle.hpp>
namespace odfaeg {
    namespace window {
        typedef void (*GlFunctionPointer)();
        class IContext {
            public :
            virtual bool setActive(bool active) = 0;
            virtual void create(IContext* sharedContext=nullptr) = 0;
            virtual void create(sf::WindowHandle handle, const ContextSettings& settings, IContext* sharedContext = nullptr, unsigned int bitsPerPixel = 32) = 0;
            virtual void create(const ContextSettings& settings, unsigned int width, unsigned int height, IContext* sharedContext = nullptr) = 0;
            virtual const ContextSettings& getSettings() const = 0;
            virtual void initialize(const ContextSettings& settings) = 0;
        };
    }
}
#endif
