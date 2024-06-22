#ifndef ITEM_HPP
#define ITEM_HPP
#include <string>
#include <map>
#include "odfaeg/Graphics/GUI/icon.hpp"
#include "odfaeg/Core/fastDelegate.h"
namespace sorrok {
    class Item {
    public :
        enum Type {
            HP_POTION, HEAD, ARMOR, GLOVES, BOOTS, WEAPON_RIGHT, WEAPON_LEFT
        };
        Item () {itemBehaviour=nullptr;}
        Item (std::string name, Type type, std::string requiredClass);
        Type getType();
        std::string getName();
        template <typename Archive>
        void serialize(Archive& ar) {
            ar(name);
            ar(type);
            ar(requiredClass);
        }
        std::string getRequiredClass();
        void setItemBehaviour(odfaeg::core::FastDelegate<void>* itemBehaviour);
        void applyBehaviour();
        void setIcon(odfaeg::graphic::gui::Icon* icon);
        odfaeg::graphic::gui::Icon* getIcon();
        ~Item();
    private :
        std::string name, requiredClass;
        Type type;
        odfaeg::graphic::gui::Icon* icon;
        odfaeg::core::FastDelegate<void>* itemBehaviour;
    };
}
#endif // ITEM_HPP
