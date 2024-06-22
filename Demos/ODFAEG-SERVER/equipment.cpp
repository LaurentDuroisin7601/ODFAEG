#include "equipement.hpp"
namespace sorrok {
    Equipment::Equipment() {
    }
    Equipment::Equipment(std::string name,Type type, std::string requiredClass) : name(name), type(type), requiredClass(requiredClass) {

    }
    Equipment::Type Item::getType() {
        return type;
    }
    std::string Equipment::getName() {
        return name;
    }
    std::string Equipment::getRequiredClass() {
        return requiredClass;
    }
}
