#ifndef ODFAEG_STATE_HPP
#define ODFAEG_STATE_HPP
#include "any.h"
#include "erreur.h"
#include "../../../include/odfaeg/Core/resourceManager.h"
#include <ODFAEG/System.hpp>
#include <ODFAEG/Graphics.hpp>
#include "export.hpp"
namespace odfaeg {
class Application;
class ODFAEG_CORE_API State {
public :
    explicit State (Application &app) : app(app) {}
    virtual void init () = 0;
    virtual void draw () = 0;
    virtual void update (core::Time dt) = 0;
    virtual void handleEvent (sf::Event &event) = 0;
    void addOption (std::string name, any value) {
        std::map<std::string, any>::iterator it = options.find(name);
        if (it != options.end())
            throw Erreur (11, "This option already exists!", 0);
        options.insert(std::make_pair(name, value));
    }
    any getOption (std::string name) {
        std::map<std::string, any>::iterator it = options.find(name);
        if (it == options.end())
            throw Erreur (12, "No such option!", 0);
        return it->second;
    }
private :
    std::map<std::string, any> options;
    Application &app;
};
}
#endif // GAME
