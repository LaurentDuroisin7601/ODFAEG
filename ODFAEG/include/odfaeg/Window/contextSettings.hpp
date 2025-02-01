#ifndef ODFAEG_CONTEXT_SETTINGS_HPP
#define ODFAEG_CONTEXT_SETTINGS_HPP
#include <SFML/System.hpp>
namespace odfaeg {
    namespace window {
        class ContextSettings {
            public :
            ////////////////////////////////////////////////////////////
            /// \brief Enumeration of the context attribute flags
            ///
            ////////////////////////////////////////////////////////////
            enum Attribute
            {
                Default = 0,      ///< Non-debug, compatibility context (this and the core attribute are mutually exclusive)
                Core    = 1 << 0, ///< Core attribute
                Debug   = 1 << 2  ///< Debug attribute
            };
            explicit ContextSettings(unsigned int depth = 0 , unsigned int stencil = 0, unsigned int antiAliasing = 0, unsigned int major = 1, unsigned int minor = 1, unsigned int attributes = Default, bool sRgb = false);
            unsigned int versionMajor, versionMinor, depthBits, stencilBits, antiAliasingLevel;
            bool sRgbCapable;
            sf::Uint32       attributeFlags;
        };
    }
}
#endif

