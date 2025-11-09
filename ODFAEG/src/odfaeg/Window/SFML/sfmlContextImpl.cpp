#include "../../../../include/odfaeg/Window/ODFAEG/sfmlContextImpl.hpp"
#include <iostream>
namespace odfaeg {
    namespace window {
        bool ODFAEGContextImpl::setActive(bool active) {
            if (context) {
                return context->setActive(active);
            }
            return false;
        }
        void ODFAEGContextImpl::create(IContext* sharedContext) {
            m_settings = ContextSettings (0, 0, 0, 0, 0);
        }
        void ODFAEGContextImpl::create(sf::WindowHandle handle, const ContextSettings& settings, IContext* sharedContext, unsigned int bitsPerPixel) {
            m_settings = settings;
        }
        void ODFAEGContextImpl::create(const ContextSettings& settings, unsigned int width, unsigned int height, IContext* sharedContext) {
            context = new sf::Context(sf::ContextSettings(settings.depthBits, settings.stencilBits, settings.antiAliasingLevel, settings.versionMajor, settings.versionMinor), width, height);
            m_settings = settings;
        }
        const ContextSettings& ODFAEGContextImpl::getSettings() const {
            if (context) {
                return m_settings;
            } else {
                static ContextSettings empty(0, 0, 0, 0, 0);
                return empty;
            }
        }
        void ODFAEGContextImpl::display() {

        }
        void ODFAEGContextImpl::initialize(const ContextSettings& settings) {
        }
        void ODFAEGContextImpl::setVerticalSyncEnabled(bool enabled) {

        }
    }
}
