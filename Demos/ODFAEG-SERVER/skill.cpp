#include "skill.hpp"
namespace sorrok {
    Skill::Skill() {
        name = "";
        manaCost = 0;
    }
    Skill::Skill (std::string name, unsigned int manaCost, std::string requiredClass) : name(name), manaCost(manaCost), requiredClass(requiredClass) {

    }
    unsigned int Skill::getManaCost() {
        return manaCost;
    }
    std::string Skill::getName() {
        return name;
    }
    std::string Skill::getRequiredClass() {
        return requiredClass;
    }
}
