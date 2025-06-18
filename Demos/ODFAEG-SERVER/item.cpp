#include "item.hpp"
namespace sorrok {
    Item::Item() {
    }
    Item::Item(std::string name,Type type, std::string requiredClass) : name(name), type(type), requiredClass(requiredClass) {

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
}
