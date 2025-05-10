#include "skill.hpp"
using namespace odfaeg::core;
namespace sorrok {
    Skill::Skill() {
        name = "";
        manaCost = 0;
        skillBehaviour=nullptr;
    }
    Skill::Skill (std::string name, unsigned int manaCost, std::string requiredClass) : name(name), manaCost(manaCost), requiredClass(requiredClass) {
        skillBehaviour=nullptr;
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
    void Skill::setSkillBehaviour(FastDelegate<void>* skillBehaviour) {
        this->skillBehaviour = skillBehaviour;
    }
    void Skill::applySkillBehaviour() {
        (*skillBehaviour)();
    }
    Skill::~Skill() {
        /*if (skillBehaviour != nullptr) {
            delete skillBehaviour;
        }*/
    }
}
