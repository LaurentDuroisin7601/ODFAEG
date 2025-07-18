#include "../../../include/odfaeg/Window/contextSettings.hpp"
namespace odfaeg {
    namespace window {
        ContextSettings::ContextSettings (unsigned int depthBits, unsigned int stencilBits, unsigned int antiAliasingLevel, unsigned  int versionMajor, unsigned int versionMinor, std::uint32_t attributes, bool sRgbCapable)  :
        versionMajor(versionMajor), versionMinor(versionMinor), depthBits(depthBits), stencilBits(stencilBits), antiAliasingLevel (antiAliasingLevel), attributeFlags(attributes), sRgbCapable(sRgbCapable) {
        }
    }
}
