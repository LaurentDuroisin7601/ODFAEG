#ifndef ODFAEG_SFML_CONTEXT_IMPL_HPP
#define ODFAEG_SFML_CONTEXT_IMPL_HPP
#include <SFML/Window/Context.hpp>
#include "../iContext.hpp"
namespace odfaeg {
    namespace window {
        class SFMLContextImpl : public IContext {
        public :
            void create(IContext* sharedContext=nullptr);
            bool setActive(bool active);
            void create(sf::WindowHandle handle, const ContextSettings& settings, IContext* sharedContext = nullptr, unsigned int bitsPerPixels = 32);
            void create(const ContextSettings& settings, unsigned int width, unsigned int height, IContext* sharedContext = nullptr);
            const ContextSettings& getSettings() const;
            void initialize(const ContextSettings& settings);
            void display();
            void setVerticalSyncEnabled(bool enabled);
        private :
            sf::Context* context;
            ContextSettings m_settings;
        };
    }
}
#endif
