#include "../../../../include/odfaeg/Window/SFML/sfmlContextImpl.hpp"
#include <iostream>
namespace odfaeg {
    namespace window {
        bool SFMLContextImpl::setActive(bool active) {
            if (context) {
                return context->setActive(active);
            }
            return false;
        }
        void SFMLContextImpl::create(IContext* sharedContext) {
            m_settings = ContextSettings (0, 0, 0, 0, 0);
        }
        void SFMLContextImpl::create(sf::WindowHandle handle, const ContextSettings& settings, IContext* sharedContext, unsigned int bitsPerPixel) {
            m_settings = settings;
        }
        void SFMLContextImpl::create(const ContextSettings& settings, unsigned int width, unsigned int height, IContext* sharedContext) {
            context = new sf::Context(sf::ContextSettings(settings.depthBits, settings.stencilBits, settings.antiAliasingLevel, settings.versionMajor, settings.versionMinor), width, height);
            m_settings = settings;
        }
        const ContextSettings& SFMLContextImpl::getSettings() const {
            if (context) {
                return m_settings;
            } else {
                static ContextSettings empty(0, 0, 0, 0, 0);
                return empty;
            }
        }
        void SFMLContextImpl::display() {

        }
        void SFMLContextImpl::initialize(const ContextSettings& settings) {
        }
        void SFMLContextImpl::setVerticalSyncEnabled(bool enabled) {

        }
    }
}
