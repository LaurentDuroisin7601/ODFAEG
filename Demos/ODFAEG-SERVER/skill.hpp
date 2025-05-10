#ifndef SORROK_SKILL_HPP
#define SORROK_SKILL_HPP
#include <string>
namespace sorrok {
    class Skill {
        public :
        enum Stat {
            NONE, HP
        };
        Skill();
        Skill(std::string name, unsigned int manaCost, std::string requiredClass);
        std::string getName();
        template <typename Archive>
        void serialize(Archive &ar) {
            ar(name);
            ar(manaCost);
            ar(requiredClass);
        }
        unsigned int getManaCost();
        std::string getRequiredClass();
        private :
        std::string name, requiredClass;
        unsigned int manaCost;
    };
}
#endif
