#ifndef ITEM_HPP
#define ITEM_HPP
#include <string>
#include <map>
namespace sorrok {
    class Item {
    public :
        enum Type {
            HP_POTION
        };
        Item();
        Item (std::string name, Type type, std::string requiredClass);
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
#endif // ITEM_HPP
