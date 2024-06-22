#include "item.hpp"
using namespace odfaeg::graphic::gui;
using namespace odfaeg::core;
namespace sorrok {
    Item::Item(std::string name,Type type,std::string requiredClass) : name(name), type(type), requiredClass(requiredClass) {
        itemBehaviour = nullptr;
    }
    void Item::setItemBehaviour(FastDelegate<void>* itemBehaviour) {
        this->itemBehaviour = itemBehaviour;
    }
    void Item::applyBehaviour() {
        (*itemBehaviour)();
    }
    Item::Type Item::getType() {
        return type;
    }
    std::string Item::getName() {
        return name;
    }
    std::string Item::getRequiredClass() {
        return requiredClass;
    }
    void Item::setIcon(Icon* icon) {
        this->icon = icon;
    }
    Icon* Item::getIcon() {
        return icon;
    }
    Item::~Item() {
        /*if (itemBehaviour != nullptr) {
            delete itemBehaviour;
        }*/
    }
}
