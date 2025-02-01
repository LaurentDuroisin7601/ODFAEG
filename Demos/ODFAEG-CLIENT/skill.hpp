#ifndef SORROK_SKILL_HPP
#define SORROK_SKILL_HPP
#include <string>
#include "odfaeg/Core/fastDelegate.h"
namespace sorrok {
    class Skill {
        public :
        enum Type {
            LAST_HEAL
        };
        Skill();
        Skill(std::string name, unsigned int manaCost, std::string requiredClass);
        void setSkillBehaviour(odfaeg::core::FastDelegate<void>* skillBehaviour);
        void applySkillBehaviour();
        std::string getName();
        unsigned int getManaCost();
        template <typename Archive>
        void serialize (Archive &ar) {
            ar(name);
            ar(manaCost);
            ar(requiredClass);
        }
        ~Skill();
        std::string getRequiredClass();
        private :
        std::string name, requiredClass;
        unsigned int manaCost;
        odfaeg::core::FastDelegate<void>* skillBehaviour;
    };
}
#endif
