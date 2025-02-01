#ifndef EQUIPMENT_HPP
#define EQUIPMENT_HPP
#include <string>
#include <map>
namespace sorrok {
    class Equipment {
        enum Type {
            HEAD, ARMOR, GLOVES, BOOTS, WEAPON_RIGHT, WEAPON_LEFT
        };
        Equipment();
        Equipment(std::string name, Type type, std::string requiredClass);
        Type getType();
        std::string getName();
        std::string getRequiredClass();
        template <typename Archive>
        void serialize(Archive& ar) {
            ar(name);
            ar(type);
            ar(requiredClass);
        }
    private :
        std::string name, requiredClass;
        Type type;
    };
}
#endif // EQUIPMENT_HPP
